"""Seeded mutation fuzzing of maialib's MusicXML reader and writer.

Case N of seed S picks a small corpus file and one mutation with a random generator seeded with
"S:N", so the same seed and case number give the same mutant in the same checkout of the same
corpus (README.md says what changes it). Each mutant runs through corpus_worker.py --analyses in
its own process: load, chords and intervals, export, the export's checks and the round trip.
"""

from __future__ import annotations

import copy
import io
import json
import random
import re
import time
import zipfile
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict

import corpus
import musicxml_check
from lxml import etree

SEED_MAX_BYTES = 200_000
WORK = corpus.HERE / "fuzz-work"
REGRESSIONS = corpus.HERE / "fuzz-regressions"
XML_MUTATIONS = (
    "delete-element",
    "empty-text",
    "non-numeric-text",
    "negative-number",
    "zero-number",
    "huge-number",
    "duplicate-element",
    "reorder-siblings",
    "invalid-enum",
    "utf16",
    "byte-order-mark",
    "latin1-declaration",
    "truncate",
)
MXL_MUTATIONS = ("mxl-drop-container", "mxl-wrong-rootfile", "mxl-corrupt-zip")
# Mutants whose fault lies in their bytes or their archive: a truncated document does not parse,
# and re-serialising a re-encoded one or an archive's document after removing elements would undo
# the mutation, so minimize() returns them as they are.
UNMINIMISABLE = ("utf16", "byte-order-mark", "latin1-declaration", "truncate") + MXL_MUTATIONS
# Outcomes whose cause removing elements cannot keep, so minimize() returns them as they are too.
# minimize() keeps a removal while the outcome class stays the same, and maialib refuses with
# RuntimeError many documents it cannot read, a score without any <part> among them ("Unable to
# locate the MusicXML XPath: /score-partwise/part"): a readable load:RuntimeError would shrink to
# an empty score, whatever its cause was. A finer signature, such as the exception's message,
# would tell the causes apart.
UNMINIMISABLE_OUTCOMES = ("load:RuntimeError",)
ENUM_ELEMENTS = {
    "step",
    "type",
    "mode",
    "sign",
    "bar-style",
    "stem",
    "accidental",
    "notehead",
    "beam",
    "syllabic",
}
ENUM_ATTRIBUTES = ("type", "location", "direction", "placement", "orientation")
NUMBER = re.compile(r"\s*-?\d+(\.\d+)?\s*")
# "ok" and the expected rejections. maialib may refuse with RuntimeError a file that is not
# readable MusicXML, which the validator cannot read either; a RuntimeError on a file the
# validator reads, valid against the schema or not, is a finding like any other exception. A file
# whose root is not a score, which maialib may refuse as well, would be readable, but no corpus
# file has one and no mutation renames the root.
EXPECTED_OUTCOMES = (
    "ok",
    "load:RuntimeError:unreadable",
    "export:xsd-invalid",
    "export:semantic-errors",
    "roundtrip:unstable",
)
# The stages that can hold "crash" or "timeout", in order: the export's checks are one step,
# recorded under export_xml, so export_xsd never does.
CRASH_STAGES = ("input", "load", "analyses", "export", "export_xml", "roundtrip")
# The values of each stage that are results rather than the type of an exception.
RESULTS = {
    "input": ("valid", "invalid", "unreadable"),
    "load": ("ok",),
    "analyses": ("ok",),
    "export": ("ok",),
    "roundtrip": ("stable", "unstable"),
}

Record = Dict[str, Any]


@dataclass
class Case:
    """One mutant: the seed and case number that made it, its source file, mutation and bytes."""

    seed: int
    index: int
    source: str
    mutation: str
    data: bytes
    suffix: str


def seed_files() -> list[str]:
    """The corpus files small enough to fuzz quickly."""
    return [
        name
        for name in corpus.corpus_files()
        if (corpus.REPO_ROOT / name).stat().st_size <= SEED_MAX_BYTES
    ]


def make_case(seed: int, index: int, seeds: list[str]) -> Case:
    rng = random.Random(f"{seed}:{index}")
    source = seeds[rng.randrange(len(seeds))]
    raw = (corpus.REPO_ROOT / source).read_bytes()
    is_archive = raw[:2] == b"PK"
    mutation = rng.choice(XML_MUTATIONS + (MXL_MUTATIONS if is_archive else ()))
    if mutation in MXL_MUTATIONS:
        return Case(seed, index, source, mutation, mutate_archive(raw, mutation, rng), ".mxl")
    document = musicxml_check.read_mxl(raw)[0] if is_archive else raw
    return Case(
        seed, index, source, mutation, mutate_xml(document or raw, mutation, rng), ".musicxml"
    )


def _declared(text: str, encoding: str) -> str:
    """The document with an XML declaration naming ``encoding``."""
    if text.startswith("<?xml"):
        end = text.index("?>")
        declaration = re.sub(
            r"encoding\s*=\s*[\"'][^\"']*[\"']", f'encoding="{encoding}"', text[:end]
        )
        if "encoding" not in declaration:
            declaration += f' encoding="{encoding}"'
        return declaration + text[end:]
    return f'<?xml version="1.0" encoding="{encoding}"?>\n' + text


def _serialize(root: etree._Element) -> bytes:
    return etree.tostring(root.getroottree(), xml_declaration=True, encoding="UTF-8")


def mutate_xml(data: bytes, mutation: str, rng: random.Random) -> bytes:
    if mutation == "truncate":
        return data[: rng.randrange(1, max(2, len(data)))]
    if mutation == "byte-order-mark":
        return b"\xef\xbb\xbf" + data
    if mutation == "utf16":
        return _declared(data.decode("utf-8", errors="replace"), "UTF-16").encode("utf-16")
    if mutation == "latin1-declaration":
        return _declared(data.decode("utf-8", errors="replace"), "ISO-8859-1").encode("utf-8")
    try:
        root = musicxml_check.parse_document(data)
    except etree.XMLSyntaxError:
        return data[: max(1, len(data) // 2)]
    _mutate_tree(root, mutation, rng)
    return _serialize(root)


def _mutate_tree(root: etree._Element, mutation: str, rng: random.Random) -> None:
    elements = [
        element for element in root.iter() if isinstance(element.tag, str) and element is not root
    ]
    numbers = [
        element
        for element in elements
        if element.text is not None and NUMBER.fullmatch(element.text)
    ]
    pools = {
        "empty-text": [element for element in elements if element.text and element.text.strip()],
        "non-numeric-text": numbers,
        "negative-number": numbers,
        "zero-number": [element for element in numbers if float(element.text) != 0],
        "huge-number": numbers,
        "invalid-enum": [
            element
            for element in elements
            if element.tag in ENUM_ELEMENTS
            or any(name in element.attrib for name in ENUM_ATTRIBUTES)
        ],
        "reorder-siblings": [element for element in [root, *elements] if len(element) >= 2],
    }
    pool = pools.get(mutation, elements)
    if not pool:  # nothing this mutation applies to: delete an element instead
        mutation, pool = "delete-element", elements
    if not pool:
        return
    target = rng.choice(pool)
    if mutation == "delete-element":
        target.getparent().remove(target)
    elif mutation == "empty-text":
        target.text = ""
    elif mutation == "non-numeric-text":
        target.text = "abc"
    elif mutation == "negative-number":
        target.text = "-" + target.text.strip().lstrip("-")
    elif mutation == "zero-number":
        target.text = "0"
    elif mutation == "huge-number":
        target.text = "99999999999999999999"
    elif mutation == "duplicate-element":
        target.addnext(copy.deepcopy(target))
    elif mutation == "reorder-siblings":
        children = list(target)
        shuffled = children[:]
        rng.shuffle(shuffled)
        if shuffled == children:
            shuffled.reverse()
        for child in shuffled:
            target.append(child)  # appending an existing child moves it to the end
    elif mutation == "invalid-enum":
        names = [name for name in ENUM_ATTRIBUTES if name in target.attrib]
        if target.tag in ENUM_ELEMENTS and (not names or rng.random() < 0.5):
            target.text = "zzz"
        else:
            target.set(rng.choice(names), "zzz")


def mutate_archive(data: bytes, mutation: str, rng: random.Random) -> bytes:
    if mutation == "mxl-corrupt-zip":
        position = rng.randrange(len(data))
        flipped = bytes(255 - byte for byte in data[position : position + 16])
        return data[:position] + flipped + data[position + 16 :]
    with zipfile.ZipFile(io.BytesIO(data)) as source:
        entries = [(info, source.read(info.filename)) for info in source.infolist()]
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as target:
        for info, content in entries:
            if info.filename == "META-INF/container.xml":
                if mutation == "mxl-drop-container":
                    continue
                content = re.sub(
                    rb'full-path="[^"]*"', b'full-path="missing.musicxml"', content, count=1
                )
            target.writestr(info, content)
    return buffer.getvalue()


def classify(record: Record) -> str:
    """The outcome class of a record: a crash or timeout first, in a stage or, by the worker's
    diagnostics, after its final record ("crash:exit", "timeout:exit"); then an exception, where
    a RuntimeError from loading a file the validator could not read is
    "load:RuntimeError:unreadable"; then the export's validity; then an unstable round trip; "ok"
    when nothing failed. Each step looks at the stages in their order, the input first."""
    for stage in CRASH_STAGES:
        if record.get(stage) in ("crash", "timeout"):
            return f"{record[stage]}:{stage}"
    exit_code = record.get("diagnostics", {}).get("exit_code", 0)
    if exit_code is None:
        return "timeout:exit"
    if exit_code != 0:
        return "crash:exit"
    for stage, results in RESULTS.items():
        value = record.get(stage, "n/a")
        if value not in results and value != "n/a":
            if stage == "load" and value == "RuntimeError" and record.get("input") == "unreadable":
                return "load:RuntimeError:unreadable"
            return f"{stage}:{value}"
    if record.get("export_xml") == "ill-formed":
        return "export:ill-formed"
    if record.get("export_xsd") == "invalid":
        return "export:xsd-invalid"
    if record.get("export_errors"):
        return "export:semantic-errors"
    if record.get("roundtrip") == "unstable":
        return "roundtrip:unstable"
    return "ok"


def worth_minimising(outcome: str) -> bool:
    """Crashes, hangs, exceptions and ill-formed exports; not "ok" or the expected rejections,
    among which a RuntimeError is only maialib refusing a file the validator could not read.
    make fuzz-minimize saves these cases; minimize() returns some of them as they are."""
    return outcome not in EXPECTED_OUTCOMES


def run_case(case: Case, timeout: float, tag: str = "") -> Record:
    """The worker's record of the mutant, with its diagnostics (exit code, end of stderr)."""
    WORK.mkdir(exist_ok=True)
    path = WORK / f"case-{case.seed}-{case.index}{tag}{case.suffix}"
    path.write_bytes(case.data)
    diagnostics: Record = {}
    try:
        record = corpus.run_one(str(path), analyses=True, timeout=timeout, diagnostics=diagnostics)
    finally:
        path.unlink()
    record["diagnostics"] = diagnostics
    return record


def run(
    seed: int,
    cases: int,
    minutes: float | None = None,
    timeout: float = 30.0,
    workers: int | None = None,
) -> list[tuple[Case, Record, str]]:
    """Run cases 0..cases-1 of ``seed``; with ``minutes``, stop starting batches after that."""
    seeds = seed_files()
    deadline = None if minutes is None else time.monotonic() + minutes * 60
    count = workers or corpus.default_workers()
    results: list[tuple[Case, Record, str]] = []
    index = 0
    with ThreadPoolExecutor(max_workers=count) as pool:
        while index < cases and (deadline is None or time.monotonic() < deadline):
            batch = [make_case(seed, i, seeds) for i in range(index, min(cases, index + count))]
            for case, record in zip(batch, pool.map(lambda c: run_case(c, timeout), batch)):
                results.append((case, record, classify(record)))
            index += len(batch)
    return results


def summary(results: list[tuple[Case, Record, str]]) -> dict[str, int]:
    return dict(sorted(Counter(outcome for _, _, outcome in results).items()))


def write_report(seed: int, results: list[tuple[Case, Record, str]]) -> Path:
    """Write the outcome counts and every case that is not ok, with its record and diagnostics."""
    WORK.mkdir(exist_ok=True)
    path = WORK / f"report-seed-{seed}.json"
    findings = [
        {
            "case": case.index,
            "source": case.source,
            "mutation": case.mutation,
            "outcome": outcome,
            "record": record,
        }
        for case, record, outcome in results
        if outcome != "ok"
    ]
    report = {
        "seed": seed,
        "cases": len(results),
        "outcomes": summary(results),
        "findings": findings,
    }
    path.write_text(json.dumps(report, indent=1) + "\n", encoding="utf-8")
    return path


def _attached(element: etree._Element, root: etree._Element) -> bool:
    while element is not None:
        if element is root:
            return True
        element = element.getparent()
    return False


def minimize(case: Case, outcome: str, timeout: float, budget: int = 200) -> bytes:
    """Remove elements, larger subtrees first, while the mutant keeps its outcome. A mutant whose
    mutation is in UNMINIMISABLE, or whose outcome is in UNMINIMISABLE_OUTCOMES, is returned as it
    is."""
    if case.mutation in UNMINIMISABLE or outcome in UNMINIMISABLE_OUTCOMES:
        return case.data
    try:
        root = musicxml_check.parse_document(case.data)
    except etree.XMLSyntaxError:
        return case.data
    trials = 0
    progress = True
    while progress and trials < budget:
        progress = False
        candidates = sorted(
            (
                element
                for element in root.iter()
                if isinstance(element.tag, str) and element is not root
            ),
            key=lambda element: -sum(1 for _ in element.iter()),
        )
        for element in candidates:
            if trials >= budget:
                break
            parent = element.getparent()
            if parent is None or not _attached(element, root):
                continue
            position = parent.index(element)
            parent.remove(element)
            trials += 1
            trial = Case(
                case.seed, case.index, case.source, case.mutation, _serialize(root), case.suffix
            )
            if classify(run_case(trial, timeout, tag=f"-min{trials}")) == outcome:
                progress = True
            else:
                parent.insert(position, element)
    return _serialize(root)


def save_regression(case: Case, outcome: str, data: bytes) -> Path:
    REGRESSIONS.mkdir(exist_ok=True)
    stem = (
        re.sub(r"[^a-z0-9]+", "-", outcome.lower()).strip("-")
        + f"-seed{case.seed}-case{case.index}"
    )
    path = REGRESSIONS / (stem + case.suffix)
    path.write_bytes(data)
    description = {
        "seed": case.seed,
        "case": case.index,
        "source": case.source,
        "mutation": case.mutation,
        "outcome": outcome,
    }
    path.with_suffix(".json").write_text(json.dumps(description, indent=1) + "\n", encoding="utf-8")
    return path
