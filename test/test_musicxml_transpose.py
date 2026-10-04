"""<transpose> through maialib's MusicXML writer, and through an export and an import."""

import contextlib
import io
import sys
import unittest
from pathlib import Path

import maialib as ml

TEST = Path(__file__).resolve().parent
sys.path.insert(0, str(TEST / "musicxml"))

import musicxml_check  # noqa: E402

UNIT_TEST = TEST / "xml_examples" / "unit_test"


def load(path):
    """The score of a MusicXML file, loaded without printing its warnings."""
    with contextlib.redirect_stdout(io.StringIO()):
        return ml.Score(str(path))


def export(score):
    """The score's MusicXML export, as bytes."""
    return score.toXML().encode("utf-8")


def transposes(data, part_index=0):
    """Every <transpose> of a part of an export, in document order: the number of its measure, how
    many <note> elements come before its <attributes> in the measure, its number attribute (or
    None), the text of <diatonic>, <chromatic> and <octave-change> (None when absent), and its
    <double> ("below", "above" or None)."""
    part = musicxml_check.parse_document(data).findall("part")[part_index]
    found = []
    for measure in part.findall("measure"):
        notes = 0
        for child in measure:
            if child.tag == "note":
                notes += 1
            elif child.tag == "attributes":
                for transpose in child.findall("transpose"):
                    double = transpose.find("double")
                    if double is None:
                        doubling = None
                    else:
                        doubling = "above" if double.get("above") == "yes" else "below"
                    found.append(
                        (
                            measure.get("number"),
                            notes,
                            transpose.get("number"),
                            transpose.findtext("diatonic"),
                            transpose.findtext("chromatic"),
                            transpose.findtext("octave-change"),
                            doubling,
                        )
                    )
    return found


def single_part_score(name, measures):
    """A one-part score with a C major key signature in measure 1, which its export needs to be
    loaded again."""
    score = ml.Score([name], measures)
    score.setKeySignature(0, True, 0)
    return score


class TransposeWriterTestCase(unittest.TestCase):
    """The export writes the transpositions its notes hold as <transpose> elements."""

    def test_each_change_is_written_at_the_start_of_its_measure(self):
        data = export(load(UNIT_TEST / "transpose_change_mid_part.musicxml"))
        self.assertEqual(
            [
                ("1", 0, None, "-1", "-2", None, None),
                ("2", 0, None, "-2", "-3", None, None),
                ("4", 0, None, "0", "0", None, None),
            ],
            transposes(data),
        )

    def test_a_change_brought_by_the_first_pitched_note_is_written_at_the_measure_start(self):
        # Measure 2 begins with a rest; its first pitched note brings the change.
        data = export(load(UNIT_TEST / "transpose_later_measure.musicxml"))
        self.assertEqual([("2", 0, None, "-4", "-7", None, None)], transposes(data))

    def test_measure_one_states_the_interval_of_a_first_note_in_a_later_measure(self):
        score = single_part_score("Clarinet in Bb", 2)
        score.getPart(0).getMeasure(1).addNote(
            ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        )
        self.assertEqual([("1", 0, None, "-1", "-2", None, None)], transposes(export(score)))

    def test_number_is_written_only_where_the_staves_differ(self):
        data = export(load(UNIT_TEST / "transpose_per_staff.musicxml"))
        self.assertEqual(
            [
                ("1", 0, "1", "-1", "-2", None, None),
                ("1", 0, "2", "-2", "-3", None, None),
                ("2", 0, "2", "-4", "-7", None, None),
                ("3", 0, None, "-1", "-2", None, None),
            ],
            transposes(data),
        )

    def test_an_octave_transposition_is_written_as_octave_change(self):
        data = export(load(UNIT_TEST / "transpose_octave_change.musicxml"))
        self.assertEqual([("1", 0, None, "0", "0", "1", None)], transposes(data, 0))
        self.assertEqual([("1", 0, None, "-1", "-2", "-1", None)], transposes(data, 1))
        self.assertEqual([("1", 0, None, "0", "0", "-1", None)], transposes(data, 2))

    def test_the_doubling_is_written_as_double(self):
        data = export(load(UNIT_TEST / "transpose_double.musicxml"))
        self.assertEqual([("1", 0, None, "0", "0", None, "below")], transposes(data, 0))
        self.assertEqual([("1", 0, None, "0", "0", None, "above")], transposes(data, 1))

    def test_a_change_after_the_first_pitched_note_is_written_just_before_its_note(self):
        data = export(load(UNIT_TEST / "transpose_after_notes.musicxml"))
        self.assertEqual(
            [("1", 2, None, "-3", "-5", None, None), ("3", 0, None, "0", "0", None, None)],
            transposes(data),
        )
        measure = musicxml_check.parse_document(data).find("part").find("measure")
        self.assertEqual(
            ["attributes", "note", "note", "attributes", "note", "note"],
            [child.tag for child in measure],
        )
        self.assertEqual("E", measure[4].findtext("pitch/step"))

    def test_a_diatonic_interval_of_zero_is_written_as_the_conventional_one(self):
        # A stored 0 and the conventional interval it stands for are one transposition, so the
        # second measure changes nothing.
        score = single_part_score("Clarinet in Bb", 2)
        score.getPart(0).getMeasure(0).addNote(ml.Note("D4", transposeChromatic=-2))
        score.getPart(0).getMeasure(1).addNote(
            ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        )
        self.assertEqual([("1", 0, None, "-1", "-2", None, None)], transposes(export(score)))

    def test_attributes_are_opened_for_a_transpose_alone(self):
        score = single_part_score("Clarinet", 2)
        score.getPart(0).getMeasure(0).addNote(
            ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2)
        )
        score.getPart(0).getMeasure(1).addNote(
            ml.Note("C4", transposeDiatonic=-2, transposeChromatic=-3)
        )
        second = musicxml_check.parse_document(export(score)).find("part").findall("measure")[1]
        self.assertEqual(["transpose"], [child.tag for child in second.find("attributes")])

    def test_a_chord_whose_notes_transpose_differently_cannot_be_written(self):
        score = single_part_score("Clarinets", 1)
        measure = score.getPart(0).getMeasure(0)
        measure.addNote(ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2))
        measure.addNote(ml.Note("E4", inChord=True, transposeDiatonic=-2, transposeChromatic=-3))
        with self.assertRaises(RuntimeError) as context:
            score.toXML()
        self.assertIn("Part 'Clarinets', measure 1, staff 1", str(context.exception))

    def test_every_fixture_exports_valid_musicxml(self):
        for path in sorted(UNIT_TEST.glob("transpose_*.musicxml")):
            with self.subTest(fixture=path.name):
                report = musicxml_check.check_bytes(export(load(path)))
                self.assertTrue(report.xsd_valid, report.xsd_errors[:3])
                self.assertEqual([], report.errors)

    def test_the_hash_of_a_score_follows_its_transpositions(self):
        score = single_part_score("Clarinet", 1)
        score.getPart(0).getMeasure(0).addNote(ml.Note("C4"))
        untransposed = hash(score)
        score.getPart(0).setTransposingInterval(-1, -2)
        self.assertNotEqual(untransposed, hash(score))


if __name__ == "__main__":
    unittest.main()
