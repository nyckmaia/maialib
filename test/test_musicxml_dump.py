"""dump_score: a canonical, deterministic JSON dump of a maialib Score."""

import contextlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import maialib as ml

MUSICXML = Path(__file__).resolve().parent / "musicxml"
sys.path.insert(0, str(MUSICXML))

import dump_score  # noqa: E402
from fixtures import MINIMAL_SCORE  # noqa: E402

# Two staves in every measure, so a dump that left out a staff would lose notes.
FIXTURE = Path(__file__).resolve().parent / "xml_examples" / "unit_test" / "test_staves.xml"

# Its <accidental> name is unknown to maialib, which records a correction while it loads the score
# and prints the summary line of its import report.
WARNING_FIXTURE = (
    Path(__file__).resolve().parent
    / "xml_examples"
    / "unit_test"
    / "quarter_tone_unknown_accidental_name.xml"
)

# The command line's dump of FIXTURE, reviewed by hand. When the dump changes on purpose, write it
# again from test/ with `python musicxml/dump_score.py xml_examples/unit_test/test_staves.xml
# musicxml/golden/test_staves.dump.json` and review the diff.
GOLDEN = MUSICXML / "golden" / "test_staves.dump.json"

# The minimal score declared UTF-8, with byte 0xE9 (e acute in Latin-1, not UTF-8) in every kind
# of string the dump reads: the header, the part name, the note's tie, stem, beam, slur and
# articulation, and the right barline's location, style and repeat direction.
NOT_UTF8 = (
    (
        b'<score-partwise version="4.0">',
        b'<score-partwise version="4.0"><work><work-title>Caf\xe9</work-title></work>'
        b'<identification><creator type="composer">Caf\xe9</creator></identification>',
    ),
    (b"<part-name>Music</part-name>", b"<part-name>Caf\xe9</part-name>"),
    (
        b"<duration>4</duration><type>whole</type>",
        b'<duration>4</duration><tie type="start\xe9"/><type>whole</type><stem>up\xe9</stem>'
        b'<beam number="1">begin\xe9</beam>'
        b'<notations><slur type="start\xe9"/><articulations><accent\xe9/></articulations>'
        b"</notations>",
    ),
    (
        b"</measure>",
        b'<barline location="right\xe9"><bar-style>light-heavy\xe9</bar-style>'
        b'<repeat direction="backward\xe9"/></barline></measure>',
    ),
)


def run_command_line(*arguments: str) -> bytes:
    """What ``dump_score.py FIXTURE *arguments`` prints."""
    command = [sys.executable, str(MUSICXML / "dump_score.py"), str(FIXTURE), *arguments]
    return subprocess.run(command, check=True, stdout=subprocess.PIPE).stdout


class DumpScoreTestCase(unittest.TestCase):
    def test_the_dump_is_deterministic_json(self):
        first = json.dumps(dump_score.dump_score(ml.Score(str(FIXTURE))), sort_keys=True)
        second = json.dumps(dump_score.dump_score(ml.Score(str(FIXTURE))), sort_keys=True)
        self.assertEqual(first, second)

    def test_the_dump_holds_every_note_of_every_measure(self):
        score = ml.Score(str(FIXTURE))
        dump = dump_score.dump_score(score)
        self.assertEqual(score.getNumParts(), len(dump["parts"]))
        for p, part_dump in enumerate(dump["parts"]):
            part = score.getPart(p)
            self.assertEqual(part.getNumMeasures(), len(part_dump["measures"]))
            for m, measure_dump in enumerate(part_dump["measures"]):
                notes = sum(len(staff["notes"]) for staff in measure_dump["staves"])
                self.assertEqual(part.getMeasure(m).getNumNotes(), notes)

    def test_the_command_line_writes_the_golden_dump(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "dump.json"
            self.assertEqual(b"", run_command_line(str(output)))
            written = output.read_bytes()
        self.assertEqual(GOLDEN.read_bytes(), written)

    def test_the_command_line_prints_the_golden_dump(self):
        self.assertEqual(GOLDEN.read_bytes(), run_command_line())

    def test_the_command_line_prints_only_the_dump_and_the_load_summary_goes_to_stderr(self):
        command = [sys.executable, str(MUSICXML / "dump_score.py"), str(WARNING_FIXTURE)]
        done = subprocess.run(command, capture_output=True, timeout=120)
        self.assertEqual(0, done.returncode, done.stderr.decode("utf-8", "replace"))
        try:
            printed = json.loads(done.stdout)
        except ValueError:
            self.fail(f"stdout is not only the JSON dump: {done.stdout[:200]!r}")
        with contextlib.redirect_stdout(io.StringIO()):  # where the load's summary goes
            expected = dump_score.dump_score(ml.Score(str(WARNING_FIXTURE)))
        self.assertEqual(expected, printed)
        self.assertNotIn(b"\r", done.stdout)
        self.assertEqual(
            b"[maiacore] quarter_tone_unknown_accidental_name.xml: 1 corrections, 0 element types "
            b"not modelled (dropped on export); see Score.getImportIssues()",
            done.stderr.strip(),
        )

    def test_a_note_is_dumped_with_its_pitch_rhythm_and_flags(self):
        record = dump_score.note_record(ml.Note("C#4"))
        self.assertEqual("C#4", record["pitch"])
        self.assertTrue(record["on"])
        self.assertFalse(record["chord"])
        self.assertEqual([0, 0], record["transpose"])

    def test_a_transposed_note_is_dumped_with_its_interval_and_sounding_pitch(self):
        record = dump_score.note_record(ml.Note("C#4", transposeDiatonic=-1, transposeChromatic=-2))
        self.assertEqual("C#4", record["pitch"])
        self.assertEqual("B3", record["sounding"])
        self.assertEqual([-1, -2], record["transpose"])

    def test_a_note_is_dumped_with_its_octave_doubling(self):
        note = ml.Note("C3")
        self.assertEqual("NONE", dump_score.note_record(note)["octaveDoubling"])
        note.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        self.assertEqual("BELOW", dump_score.note_record(note)["octaveDoubling"])

    def test_the_attributes_and_change_flags_of_each_measure_are_dumped(self):
        score = ml.Score(["Drums"], 4)
        part = score.getPart(0)
        part.addMidiUnpitched(36)
        part.getMeasure(0).setIsDivisionsPerQuarterNoteChanged(True)
        part.getMeasure(1).setKeySignature(-2, False)
        part.getMeasure(2).setTimeSignature(3, 8)
        part.getMeasure(3).setIsMetronomeChanged(True)
        part_dump = dump_score.dump_score(score)["parts"][0]
        self.assertEqual([36], part_dump["midi_unpitched"])
        measures = part_dump["measures"]
        self.assertEqual({"fifths": -2, "major": False}, measures[1]["key"])
        self.assertEqual([3, 8], measures[2]["time"])
        for index, flag in enumerate(("divisions", "key", "time", "metronome")):
            expected = dict.fromkeys(("divisions", "key", "time", "clef", "metronome"), False)
            expected[flag] = True
            self.assertEqual(expected, measures[index]["changed"], flag)

    def test_a_getter_that_raises_is_recorded_instead_of_stopping_the_dump(self):
        def broken():
            raise IndexError("out of range")

        self.assertEqual({"error": "IndexError"}, dump_score._safe(broken))

    def test_a_string_that_is_not_utf8_is_recorded_instead_of_stopping_the_dump(self):
        data = MINIMAL_SCORE
        for old, new in NOT_UTF8:
            data = data.replace(old, new)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "not-utf8.musicxml"
            path.write_bytes(data)
            dump = dump_score.dump_score(ml.Score(str(path)))
        error = {"error": "UnicodeDecodeError"}
        part = dump["parts"][0]
        measure = part["measures"][0]
        note = measure["staves"][0]["notes"][0]
        # The title, the composer and the part name are read as valid UTF-8, U+FFFD in place of
        # each invalid byte; the short name, cut from the part name byte by byte, is not.
        self.assertEqual("Caf\ufffd", dump["title"])
        self.assertEqual("Caf\ufffd", dump["composer"])
        self.assertEqual("Caf\ufffd", part["name"])
        self.assertEqual(error, part["short_name"])
        for field in ("ties", "stem", "beams", "slur", "articulations"):
            self.assertEqual(error, note[field], field)
        self.assertEqual(
            {"location": error, "style": error, "repeat": error}, measure["barlines"][1]
        )
        self.assertEqual("C4", note["pitch"])
        self.assertEqual({"location": "left", "style": "", "repeat": ""}, measure["barlines"][0])

    def test_a_staff_whose_notes_cannot_be_read_is_recorded_instead_of_stopping_the_dump(self):
        class OneStaffTooMany:
            """A measure that counts one staff more than it has: reading the notes of that
            staff raises IndexError."""

            def __init__(self, measure):
                self._measure = measure

            def __getattr__(self, name):
                return getattr(self._measure, name)

            def getNumStaves(self):
                return self._measure.getNumStaves() + 1

        score = ml.Score(["Piano"], 1)
        score.getPart(0).getMeasure(0).addNote("C4")
        staves = dump_score._measure(OneStaffTooMany(score.getPart(0).getMeasure(0)))["staves"]
        self.assertEqual(2, len(staves))
        self.assertEqual({"error": "IndexError"}, staves[1]["notes"])
        self.assertEqual({"sign": "G", "line": 2}, staves[0]["clef"])
        self.assertEqual(["C4"], [note["pitch"] for note in staves[0]["notes"]])


if __name__ == "__main__":
    unittest.main()
