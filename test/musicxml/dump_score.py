"""A canonical JSON dump of a maialib Score, to compare the model before and after a change.

Command line: ``python dump_score.py SCORE [OUTPUT]`` writes the dump to OUTPUT or prints it.
Every value comes from maialib's public API; a getter that raises is recorded as
{"error": "<exception type>"} instead of stopping the dump.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any, Callable


def _safe(getter: Callable[[], Any]) -> Any:
    try:
        return getter()
    except Exception as error:  # the exception type is the recorded value
        return {"error": type(error).__name__}


def _clef(measure: Any, staff: int) -> Any:
    def read() -> dict[str, Any]:
        clef = measure.getClef(staff)
        return {"sign": clef.getSign().name, "line": clef.getLine()}

    return _safe(read)


def _barline(barline: Any) -> dict[str, str]:
    return {
        "location": barline.getLocation(),
        "style": barline.getBarStyle(),
        "repeat": barline.getDirection(),
    }


def note_record(note: Any) -> dict[str, Any]:
    """One note as plain data: written pitch, rhythm, voice and staff, flags and notations."""
    return {
        "pitch": _safe(note.getWrittenPitch),
        "on": note.isNoteOn(),
        "pitched": note.isPitched(),
        "ticks": _safe(note.getDurationTicks),
        "divisions": _safe(note.getDivisionsPerQuarterNote),
        "type": _safe(note.getType),
        "dots": _safe(note.getNumDots),
        "voice": note.getVoice(),
        "staff": note.getStaff(),
        "chord": note.inChord(),
        "grace": note.isGraceNote(),
        "transpose": [note.getTransposeDiatonic(), note.getTransposeChromatic()],
        "ties": list(note.getTie()),
        "slur": list(note.getSlur()),
        "beams": list(note.getBeam()),
        "articulations": list(note.getArticulation()),
        "stem": note.getStem(),
        "unpitched_index": note.getUnpitchedIndex(),
    }


def _measure(measure: Any) -> dict[str, Any]:
    staves = []
    for staff in range(measure.getNumStaves()):
        notes = [
            note_record(measure.getNote(index, staff))
            for index in range(measure.getNumNotes(staff))
        ]
        staves.append({"clef": _clef(measure, staff), "notes": notes})
    return {
        "number": measure.getNumber(),
        "divisions": measure.getDivisionsPerQuarterNote(),
        "key": _safe(
            lambda: {
                "fifths": measure.getKey().getFifthCircle(),
                "major": measure.getKey().isMajorMode(),
            }
        ),
        "time": _safe(
            lambda: [
                measure.getTimeSignature().getUpperValue(),
                measure.getTimeSignature().getLowerValue(),
            ]
        ),
        "barlines": [_barline(measure.getBarlineLeft()), _barline(measure.getBarlineRight())],
        "staves": staves,
    }


def dump_score(score: Any) -> dict[str, Any]:
    """The score as plain data: the header, then every part, measure, staff and note in order."""
    parts: list[dict[str, Any]] = []
    for index in range(score.getNumParts()):
        part = score.getPart(index)
        parts.append(
            {
                "name": part.getName(),
                "staves": part.getNumStaves(),
                "pitched": part.isPitched(),
                "staff_lines": part.getStaffLines(),
                "measures": [_measure(part.getMeasure(m)) for m in range(part.getNumMeasures())],
            }
        )
    return {
        "title": score.getTitle(),
        "composer": score.getComposerName(),
        "anacrusis": score.haveAnacrusisMeasure(),
        "parts": parts,
    }


def main(argv: list[str]) -> int:
    if len(argv) not in (1, 2):
        print("usage: dump_score.py SCORE [OUTPUT]", file=sys.stderr)
        return 2
    import maialib as ml

    text = json.dumps(dump_score(ml.Score(argv[0])), indent=1, sort_keys=True) + "\n"
    if len(argv) == 2:
        Path(argv[1]).write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
