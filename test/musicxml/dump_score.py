"""A canonical JSON dump of a maialib Score, to compare the model before and after a change.

Command line: ``python dump_score.py SCORE [OUTPUT]`` writes the dump to OUTPUT or prints it, as
ASCII with LF line endings on every platform. What maialib prints while it loads the score and
the dump reads it, such as the summary line of its import report, goes to stderr, so that
stdout holds only the dump.
Every value comes from maialib's public API through ``_safe``: a getter that raises, such as a
string getter whose bytes are not UTF-8, is recorded as {"error": "<exception type>"} instead of
stopping the dump. So is a list that cannot be read, such as the notes of a staff whose note
list is gone.
"""

from __future__ import annotations

import contextlib
import json
import sys
from pathlib import Path
from typing import Any, Callable


def _safe(getter: Callable[[], Any]) -> Any:
    try:
        return getter()
    except Exception as error:  # the exception type is the recorded value
        return {"error": type(error).__name__}


def _list(getter: Callable[[], Any]) -> Any:
    """A getter's sequence as a list (pybind11 returns a pair as a tuple)."""
    return _safe(lambda: list(getter()))


def _items(count: Callable[[], int], item: Callable[[int], Any]) -> Any:
    """``[item(0), ..., item(count() - 1)]``, or its error record."""
    return _safe(lambda: [item(index) for index in range(count())])


def _clef(measure: Any, staff: int) -> Any:
    def read() -> dict[str, Any]:
        clef = measure.getClef(staff)
        return {"sign": clef.getSign().name, "line": clef.getLine()}

    return _safe(read)


def _barline(getter: Callable[[], Any]) -> Any:
    def read() -> dict[str, Any]:
        barline = getter()
        return {
            "location": _safe(barline.getLocation),
            "style": _safe(barline.getBarStyle),
            "repeat": _safe(barline.getDirection),
        }

    return _safe(read)


def note_record(note: Any) -> dict[str, Any]:
    """One note as plain data: written and sounding pitch, rhythm, voice and staff, flags and
    notations."""
    return {
        "pitch": _safe(note.getWrittenPitch),
        "sounding": _safe(note.getSoundingPitch),
        "on": _safe(note.isNoteOn),
        "pitched": _safe(note.isPitched),
        "ticks": _safe(note.getDurationTicks),
        "divisions": _safe(note.getDivisionsPerQuarterNote),
        "type": _safe(note.getType),
        "dots": _safe(note.getNumDots),
        "voice": _safe(note.getVoice),
        "staff": _safe(note.getStaff),
        "chord": _safe(note.inChord),
        "grace": _safe(note.isGraceNote),
        "transpose": [_safe(note.getTransposeDiatonic), _safe(note.getTransposeChromatic)],
        "octaveDoubling": _safe(lambda: note.getOctaveDoubling().name),
        "ties": _list(note.getTie),
        "slur": _list(note.getSlur),
        "beams": _list(note.getBeam),
        "articulations": _list(note.getArticulation),
        "stem": _safe(note.getStem),
        "unpitched_index": _safe(note.getUnpitchedIndex),
    }


def _staff(measure: Any, staff: int) -> dict[str, Any]:
    return {
        "clef": _clef(measure, staff),
        "notes": _items(
            lambda: measure.getNumNotes(staff),
            lambda index: note_record(measure.getNote(index, staff)),
        ),
    }


def _measure(measure: Any) -> dict[str, Any]:
    return {
        "number": _safe(measure.getNumber),
        "divisions": _safe(measure.getDivisionsPerQuarterNote),
        "key": _safe(
            lambda: {
                "fifths": measure.getKey().getFifthCircle(),
                "major": bool(measure.getKey().isMajorMode()),
            }
        ),
        "time": _safe(
            lambda: [
                measure.getTimeSignature().getUpperValue(),
                measure.getTimeSignature().getLowerValue(),
            ]
        ),
        # Whether the measure states each attribute anew; the writer emits attributes from these.
        "changed": {
            "divisions": _safe(measure.divisionsPerQuarterNoteChanged),
            "key": _safe(measure.keySignatureChanged),
            "time": _safe(measure.timeSignatureChanged),
            "clef": _safe(measure.isClefChanged),
            "metronome": _safe(measure.metronomeChanged),
        },
        "barlines": [_barline(measure.getBarlineLeft), _barline(measure.getBarlineRight)],
        "staves": _items(measure.getNumStaves, lambda staff: _staff(measure, staff)),
    }


def _part(part: Any) -> dict[str, Any]:
    return {
        "name": _safe(part.getName),
        "short_name": _safe(part.getShortName),
        "staves": _safe(part.getNumStaves),
        "pitched": _safe(part.isPitched),
        "staff_lines": _safe(part.getStaffLines),
        # The MIDI numbers that each note's unpitched_index points into.
        "midi_unpitched": _list(part.getMidiUnpitched),
        "measures": _items(part.getNumMeasures, lambda index: _measure(part.getMeasure(index))),
    }


def dump_score(score: Any) -> dict[str, Any]:
    """The score as plain data: the header, then every part, measure, staff and note in order."""
    return {
        "title": _safe(score.getTitle),
        "composer": _safe(score.getComposerName),
        "anacrusis": _safe(score.haveAnacrusisMeasure),
        "parts": _items(score.getNumParts, lambda index: _part(score.getPart(index))),
    }


def main(argv: list[str]) -> int:
    if len(argv) not in (1, 2):
        print("usage: dump_score.py SCORE [OUTPUT]", file=sys.stderr)
        return 2
    # maialib's binding sends what its C++ code prints to Python's sys.stdout, so redirecting
    # sys.stdout moves it to stderr.
    with contextlib.redirect_stdout(sys.stderr):
        import maialib as ml

        dump = dump_score(ml.Score(argv[0]))
    # Bytes, so that no platform rewrites the line endings: text-mode output on Windows turns
    # each LF into CRLF. json.dumps escapes every non-ASCII character, so the text is ASCII.
    text = json.dumps(dump, indent=1, sort_keys=True) + "\n"
    data = text.encode("ascii")
    if len(argv) == 2:
        Path(argv[1]).write_bytes(data)
    else:
        sys.stdout.buffer.write(data)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
