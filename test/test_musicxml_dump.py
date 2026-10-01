"""dump_score: a canonical, deterministic JSON dump of a maialib Score."""

import json
import sys
import unittest
from pathlib import Path

import maialib as ml

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import dump_score  # noqa: E402

# Two staves in every measure, so a dump that left out a staff would lose notes.
FIXTURE = Path(__file__).resolve().parent / "xml_examples" / "unit_test" / "test_staves.xml"


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

    def test_a_note_is_dumped_with_its_pitch_rhythm_and_flags(self):
        record = dump_score.note_record(ml.Note("C#4"))
        self.assertEqual("C#4", record["pitch"])
        self.assertTrue(record["on"])
        self.assertFalse(record["chord"])
        self.assertEqual([0, 0], record["transpose"])

    def test_a_getter_that_raises_is_recorded_instead_of_stopping_the_dump(self):
        def broken():
            raise IndexError("out of range")

        self.assertEqual({"error": "IndexError"}, dump_score._safe(broken))


if __name__ == "__main__":
    unittest.main()
