"""The MusicXML corpus: which files it holds, how they are examined, and the ledger of results.

The in-repository corpus is every MusicXML file under test/xml_examples, the samples bundled in
maialib/xml-scores-examples and the vendored W3C test suite; `make corpus-fetch` adds the
external corpus under test/musicxml/external. corpus_worker.py examines each file in its own
process, so a crash or a hang becomes a result instead of ending the run. README.md describes
the ledger.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Any, Dict, Iterable

import corpus_worker

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parents[1]
WORKER = HERE / "corpus_worker.py"
LEDGER = HERE / "ledger.json"
EXTERNAL_ROOT = HERE / "external"
EXTERNAL_LEDGER = HERE / "ledger-external.json"
CORPUS_ROOTS = ("test/xml_examples", "maialib/xml-scores-examples", "test/musicxml/w3c-test-suite")
SUFFIXES = (".xml", ".musicxml", ".mxl")
SLOW_BYTES = 10_000_000
SLOW_TIMEOUT = 3600.0
IGNORED_KEYS = ("slow", "note")

Record = Dict[str, Any]


def _files_under(root: Path) -> list[str]:
    return sorted(
        path.relative_to(REPO_ROOT).as_posix()
        for path in root.rglob("*")
        if path.is_file() and ".git" not in path.parts and path.name.lower().endswith(SUFFIXES)
    )


def corpus_files() -> list[str]:
    """Repository-relative paths of the in-repository corpus, sorted."""
    return sorted(name for root in CORPUS_ROOTS for name in _files_under(REPO_ROOT / root))


def external_files() -> list[str]:
    """Repository-relative paths of the fetched external corpus; empty until it is fetched."""
    return _files_under(EXTERNAL_ROOT) if EXTERNAL_ROOT.is_dir() else []


def is_slow(name: str) -> bool:
    """Files of 10 MB or more: `make py-tests` skips them, `make corpus` runs them."""
    return (REPO_ROOT / name).stat().st_size >= SLOW_BYTES


def timeout_for(name: str) -> float:
    """Seconds allowed for one file: 30 s plus 10 s per megabyte, and an hour for a slow file."""
    if is_slow(name):
        return SLOW_TIMEOUT
    return 30 + 10 * (REPO_ROOT / name).stat().st_size / 1_000_000


def _last_record(output: bytes) -> Record | None:
    last = None
    for line in output.decode("utf-8", errors="replace").splitlines():
        if line.startswith(corpus_worker.PREFIX):
            last = json.loads(line[len(corpus_worker.PREFIX) :])
    return last


def ended(record: Record, outcome: str) -> Record:
    """Mark the first unfinished stage with ``outcome`` ("crash" or "timeout"), later ones n/a."""
    marked = False
    for name in corpus_worker.STAGE_ORDER:
        if record.get(name) == corpus_worker.PENDING:
            record[name] = corpus_worker.NOT_APPLICABLE if marked else outcome
            marked = True
    return record


def run_one(name: str, analyses: bool = False, timeout: float | None = None) -> Record:
    """Examine one repository-relative file in a fresh process and return its record."""
    command = [sys.executable, str(WORKER), str(REPO_ROOT / name)]
    if analyses:
        command.append("--analyses")
    limit = timeout_for(name) if timeout is None else timeout
    try:
        completed = subprocess.run(command, cwd=str(REPO_ROOT), capture_output=True, timeout=limit)
    except subprocess.TimeoutExpired as expired:
        record = _last_record(expired.stdout or b"") or corpus_worker.new_record(analyses)
        return ended(record, "timeout")
    record = _last_record(completed.stdout) or corpus_worker.new_record(analyses)
    return ended(record, "crash")


def run_corpus(
    names: Iterable[str], analyses: bool = False, workers: int | None = None
) -> dict[str, Record]:
    """Examine the files in parallel and return their records by name."""
    names = list(names)
    count = workers or max(2, (os.cpu_count() or 2) // 2)
    with ThreadPoolExecutor(max_workers=count) as pool:
        records = list(pool.map(lambda name: run_one(name, analyses), names))
    return dict(zip(names, records))


def load_ledger(path: Path) -> dict[str, Record]:
    return json.loads(path.read_text(encoding="utf-8"))["files"]


def write_ledger(path: Path, records: dict[str, Record]) -> None:
    """Write one file per line, sorted, so that a change shows as a one-line diff."""
    lines = [
        f"  {json.dumps(name)}: {json.dumps(records[name], sort_keys=True)}"
        for name in sorted(records)
    ]
    # Bytes, so the line endings are LF on every platform (Path.write_text has no newline
    # argument before Python 3.10).
    path.write_bytes(('{"files": {\n' + ",\n".join(lines) + "\n}}\n").encode("utf-8"))


def updated_ledger(old: dict[str, Record], actual: dict[str, Record]) -> dict[str, Record]:
    """The ledger to write for these results: where the old entry allowed several values and the
    new one is among them, the alternatives stay, and the note with them while any do; slow files
    are marked. Alternatives never narrow here: prune them by hand once their cause is fixed."""
    ledger: dict[str, Record] = {}
    for name, record in actual.items():
        entry = dict(record)
        previous = old.get(name, {})
        kept_alternatives = False
        for key, allowed in previous.items():
            if isinstance(allowed, dict) and record.get(key) in allowed.get("any_of", []):
                entry[key] = allowed
                kept_alternatives = True
        if kept_alternatives and "note" in previous:
            entry["note"] = previous["note"]
        if is_slow(name):
            entry["slow"] = True
        ledger[name] = entry
    return ledger


def _matches(allowed: Any, value: Any) -> bool:
    if isinstance(allowed, dict):
        return value in allowed.get("any_of", [])
    return allowed == value


def compare(expected: dict[str, Record], actual: dict[str, Record]) -> list[str]:
    """Every difference between the ledger entries and this run's records, sorted by file."""
    problems: list[str] = []
    for name in sorted(actual):
        if name not in expected:
            problems.append(f"{name}: not in the ledger")
            continue
        want = {key: value for key, value in expected[name].items() if key not in IGNORED_KEYS}
        got = actual[name]
        for key in sorted(set(want) | set(got)):
            if not _matches(want.get(key), got.get(key)):
                problems.append(
                    f"{name}: {key} is {got.get(key)!r}, the ledger says {want.get(key)!r}"
                )
    return problems
