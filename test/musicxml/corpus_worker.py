"""Examine one MusicXML file with maialib, for the corpus ledger and the fuzz driver.

Usage: ``python corpus_worker.py PATH [--analyses]``

After each stage the worker prints a line "CORPUS-RECORD <json>" holding the record so far, so
the parent still learns the finished stages when a later one crashes or hangs. The stages, in
order: input (the file itself against the MusicXML 4.0 schema), load (maialib.Score), analyses
(only with --analyses: chords and the intervals between consecutive notes), export
(Score.toXML), the export's checks (well-formed, schema, semantic errors), and roundtrip (the
export loaded and exported again, compared without its encoding date).
"""

from __future__ import annotations

import json
import re
import sys
import tempfile
from pathlib import Path
from typing import Any, Dict

import musicxml_check

PREFIX = "CORPUS-RECORD "
PENDING = "pending"
NOT_APPLICABLE = "n/a"
STAGE_ORDER = ("input", "load", "analyses", "export", "export_xml", "export_xsd", "roundtrip")
_ENCODING_DATE = re.compile(r"<encoding-date>[^<]*</encoding-date>")

Record = Dict[str, Any]


def new_record(analyses: bool = False) -> Record:
    """A record with every stage pending; "analyses" only when that stage runs."""
    record: Record = {name: PENDING for name in STAGE_ORDER if analyses or name != "analyses"}
    record["export_errors"] = []
    return record


def emit(record: Record) -> None:
    print(PREFIX + json.dumps(record, sort_keys=True), flush=True)


def finish(record: Record) -> None:
    """Mark the stages that did not run as not applicable, then emit the final record."""
    for name in STAGE_ORDER:
        if record.get(name) == PENDING:
            record[name] = NOT_APPLICABLE
    emit(record)


def input_status(path: Path) -> str:
    report = musicxml_check.check_file(path)
    if not report.readable:
        return "unreadable"
    return "valid" if report.xsd_valid else "invalid"


def without_encoding_date(text: str) -> str:
    return _ENCODING_DATE.sub("", text)


def run_analyses(ml: Any, score: Any) -> str:
    """Chord extraction, and the interval between each pair of consecutive pitched notes."""
    try:
        score.getChords()
        for p in range(score.getNumParts()):
            part = score.getPart(p)
            previous = None
            for m in range(part.getNumMeasures()):
                measure = part.getMeasure(m)
                for s in range(measure.getNumStaves()):
                    for n in range(measure.getNumNotes(s)):
                        note = measure.getNote(n, s)
                        if not note.isNoteOn() or not note.isPitched():
                            continue
                        if previous is not None:
                            ml.Interval(previous, note).getName()
                        previous = note
    except Exception as error:  # the exception type is the result
        return type(error).__name__
    return "ok"


def roundtrip_status(ml: Any, exported: str) -> str:
    with tempfile.TemporaryDirectory() as folder:
        path = Path(folder) / "roundtrip.musicxml"
        path.write_text(exported, encoding="utf-8")
        try:
            again = ml.Score(str(path)).toXML()
        except Exception as error:  # the exception type is the result
            return type(error).__name__
    return (
        "stable" if without_encoding_date(again) == without_encoding_date(exported) else "unstable"
    )


def examine(path: Path, analyses: bool) -> None:
    record = new_record(analyses)
    record["input"] = input_status(path)
    emit(record)

    import maialib as ml

    try:
        score = ml.Score(str(path))
    except Exception as error:  # the exception type is the result
        record["load"] = type(error).__name__
        finish(record)
        return
    record["load"] = "ok"
    emit(record)

    if analyses:
        record["analyses"] = run_analyses(ml, score)
        emit(record)

    try:
        exported = score.toXML()
    except Exception as error:  # the exception type is the result
        record["export"] = type(error).__name__
        finish(record)
        return
    record["export"] = "ok"
    report = musicxml_check.check_bytes(exported.encode("utf-8"))
    record["export_xml"] = "well-formed" if report.readable else "ill-formed"
    if report.readable:
        record["export_xsd"] = "valid" if report.xsd_valid else "invalid"
        record["export_errors"] = report.errors
    emit(record)

    record["roundtrip"] = roundtrip_status(ml, exported)
    finish(record)


def main(argv: list[str]) -> int:
    if sys.platform == "win32":
        import ctypes

        # A crash ends the process at once instead of opening the Windows error-reporting
        # dialog, which would hold it until the parent's timeout.
        sem_failcriticalerrors, sem_nogpfaulterrorbox = 0x0001, 0x0002
        ctypes.windll.kernel32.SetErrorMode(sem_failcriticalerrors | sem_nogpfaulterrorbox)
    analyses = "--analyses" in argv
    paths = [argument for argument in argv if argument != "--analyses"]
    if len(paths) != 1:
        print("usage: corpus_worker.py PATH [--analyses]", file=sys.stderr)
        return 2
    examine(Path(paths[0]), analyses)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
