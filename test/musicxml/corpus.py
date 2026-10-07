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
import tempfile
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
STDERR_TAIL = 2000
# The size above which a ledger keeps its "codes" in a sidecar file instead, below the 500 KB
# from which the repository's pre-commit hook refuses a file.
LEDGER_LIMIT = 500_000

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


def default_workers() -> int:
    """The number of files examined at once unless told otherwise: half the CPUs, at least 2."""
    return max(2, (os.cpu_count() or 2) // 2)


def select(
    names: Iterable[str], substring: str | None = None, skip_slow: bool = False
) -> list[str]:
    """The names that contain ``substring`` (all without one), the slow files left out with
    ``skip_slow``."""
    return [
        name
        for name in names
        if (substring is None or substring in name) and not (skip_slow and is_slow(name))
    ]


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


def run_one(
    name: str,
    analyses: bool = False,
    timeout: float | None = None,
    diagnostics: dict[str, Any] | None = None,
) -> Record:
    """Examine one file, named relative to the repository or by an absolute path, in a fresh
    process and return its record.

    The worker keeps its temporary files in a directory of its own, removed when the worker has
    ended, so that a worker killed at the timeout or ended by a crash leaves none behind. With
    ``diagnostics``, also fill it with the worker's ``exit_code`` (None when the timeout stopped
    it) and ``stderr_tail``, the last STDERR_TAIL characters of its standard error. A worker that
    ended with a status other than 0 has it in the record's ``exit``, also after its final record.
    """
    command = [sys.executable, str(WORKER), str(REPO_ROOT / name)]
    if analyses:
        command.append("--analyses")
    limit = timeout_for(name) if timeout is None else timeout
    with tempfile.TemporaryDirectory() as temporary:
        # Python's tempfile reads TMPDIR, TEMP and TMP; Windows' own functions read TMP and TEMP.
        variables = dict.fromkeys(("TMPDIR", "TEMP", "TMP"), temporary)
        try:
            completed = subprocess.run(
                command,
                cwd=str(REPO_ROOT),
                env={**os.environ, **variables},
                capture_output=True,
                timeout=limit,
            )
        except subprocess.TimeoutExpired as expired:
            # subprocess.run has killed the worker and waited for it.
            exit_code, stdout, stderr = None, expired.stdout, expired.stderr
        else:
            exit_code, stdout, stderr = completed.returncode, completed.stdout, completed.stderr
    if diagnostics is not None:
        diagnostics["exit_code"] = exit_code
        diagnostics["stderr_tail"] = (stderr or b"").decode("utf-8", "replace")[-STDERR_TAIL:]
    record = _last_record(stdout or b"") or corpus_worker.new_record(analyses)
    if exit_code is not None and exit_code != 0:
        record["exit"] = exit_code
    return ended(record, "timeout" if exit_code is None else "crash")


def run_corpus(
    names: Iterable[str], analyses: bool = False, workers: int | None = None
) -> dict[str, Record]:
    """Examine the files in parallel and return their records by name."""
    names = list(names)
    with ThreadPoolExecutor(max_workers=workers or default_workers()) as pool:
        records = list(pool.map(lambda name: run_one(name, analyses), names))
    return dict(zip(names, records))


def codes_path(path: Path) -> Path:
    """The sidecar file of a ledger, which holds its "codes" when the ledger would be too big."""
    return path.with_name(path.stem + "-codes.json")


def load_ledger(path: Path) -> dict[str, Record]:
    """The ledger's records, with the "codes" of its sidecar file, if it has one."""
    records = json.loads(path.read_text(encoding="utf-8"))["files"]
    sidecar = codes_path(path)
    if sidecar.is_file():
        for name, codes in json.loads(sidecar.read_text(encoding="utf-8"))["files"].items():
            records.setdefault(name, {})["codes"] = codes
    return records


def _ledger_bytes(entries: dict[str, Any]) -> bytes:
    """One file per line, sorted, so that a change shows as a one-line diff; LF line endings on
    every platform (Path.write_text has no newline argument before Python 3.10)."""
    lines = [
        f"  {json.dumps(name)}: {json.dumps(entries[name], sort_keys=True)}"
        for name in sorted(entries)
    ]
    return ('{"files": {\n' + ",\n".join(lines) + "\n}}\n").encode("utf-8")


def write_ledger(path: Path, records: dict[str, Record]) -> None:
    """Write the ledger, one file per line. When it would exceed LEDGER_LIMIT bytes, the "codes"
    of its records go to its sidecar file (codes_path), in the same format; otherwise a sidecar
    left from an earlier write is removed."""
    data = _ledger_bytes(records)
    sidecar = codes_path(path)
    if len(data) > LEDGER_LIMIT:
        codes = {name: record["codes"] for name, record in records.items() if "codes" in record}
        rest = {
            name: {key: value for key, value in record.items() if key != "codes"}
            for name, record in records.items()
        }
        data = _ledger_bytes(rest)
        sidecar.write_bytes(_ledger_bytes(codes))
    elif sidecar.is_file():
        sidecar.unlink()
    path.write_bytes(data)


def ledger_after(
    old: dict[str, Record], actual: dict[str, Record], complete: bool
) -> dict[str, Record]:
    """The ledger to write after examining the files of ``actual``: their updated entries, and,
    when the run examined only some of the corpus (``complete`` false), the old entries of every
    other file."""
    ledger = {} if complete else dict(old)
    ledger.update(updated_ledger(old, actual))
    return ledger


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
