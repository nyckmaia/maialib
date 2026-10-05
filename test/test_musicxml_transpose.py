"""<transpose> through maialib's MusicXML writer, and through an export and an import."""

import contextlib
import io
import os
import re
import sys
import tempfile
import unittest
from pathlib import Path

import maialib as ml

TEST = Path(__file__).resolve().parent
sys.path.insert(0, str(TEST / "musicxml"))

import corpus  # noqa: E402
import musicxml_check  # noqa: E402

UNIT_TEST = TEST / "xml_examples" / "unit_test"
REPO = TEST.parent
SLOW_TESTS = os.environ.get("MAIALIB_SLOW_TESTS") == "1"
TRANSPOSE_ELEMENT = re.compile(r"Element '(transpose|diatonic|chromatic|octave-change|double)'")

# Every corpus file with a <transpose>, the transpose_*.musicxml fixtures aside, as
# repository-relative paths.
CORPUS_WITH_TRANSPOSE = (
    "maialib/xml-scores-examples/Beethoven_Symphony_5_mov_1.xml",
    "maialib/xml-scores-examples/Dvorak_Symphony_9_mov_4.mxl",
    "maialib/xml-scores-examples/Mahler_Symphony_8_Finale.mxl",
    "maialib/xml-scores-examples/Mozart_Requiem_Introitus.mxl",
    "maialib/xml-scores-examples/Strauss_Also_Sprach_Zarathustra.mxl",
    "test/musicxml/w3c-test-suite/xmlFiles/41c-StaffGroups.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72a-TransposingInstruments.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72b-TransposingInstruments-Full.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72c-TransposingInstruments-Change.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72d-TransposingInstruments-scorePitch.musicxml",
    "test/xml_examples/Beethoven/Symphony_5th_1Mov.xml",
    "test/xml_examples/Beethoven/big_files/Symphony_9th.xml",
    "test/xml_examples/unit_test/test_compressed_file.mxl",
    "test/xml_examples/unit_test/test_getChords.xml",
    "test/xml_examples/unit_test/test_getchords_poly.musicxml",
    "test/xml_examples/unit_test/test_multiple_instruments2.xml",
    "test/xml_examples/unit_test/test_multiple_instruments3.musicxml",
    "test/xml_examples/unit_test/test_pattern.musicxml",
    "test/xml_examples/unit_test/test_stack_chords.xml",
    "test/xml_examples/unit_test/test_stack_multiple_staves.xml",
    "test/xml_examples/unit_test/xakypueri.xml",
)

# They do not load: a first measure without <key>, two clefs in one measure; W3C 72d covers
# 72b's transpositions.
NOT_LOADABLE = (
    "maialib/xml-scores-examples/Mozart_Requiem_Introitus.mxl",
    "test/musicxml/w3c-test-suite/xmlFiles/72b-TransposingInstruments-Full.musicxml",
)

# Its only <transpose> is for a <for-part>, which the reader does not model, so its notes are
# untransposed.
UNTRANSPOSED_FIXTURES = ("transpose_for_part.musicxml",)


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


def note_transpositions(score):
    """Each sounding pitched note's written pitch, sounding pitch, interval and octave doubling,
    keyed by its part, measure, staff and index."""
    found = {}
    for p in range(score.getNumParts()):
        part = score.getPart(p)
        for m in range(part.getNumMeasures()):
            measure = part.getMeasure(m)
            for s in range(measure.getNumStaves()):
                for n in range(measure.getNumNotes(s)):
                    note = measure.getNote(n, s)
                    if note.isNoteOn() and note.isPitched():
                        found[(p, m, s, n)] = (
                            note.getWrittenPitch(),
                            note.getSoundingPitch(),
                            note.getTransposeDiatonic(),
                            note.getTransposeChromatic(),
                            note.getOctaveDoubling().name,
                        )
    return found


def reloaded(data):
    """The score an export loads back as, and what the load printed."""
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "round-trip.musicxml"
        path.write_bytes(data)
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            score = ml.Score(str(path))
    return score, printed.getvalue()


def contains_transpose(name):
    """Whether a corpus file's MusicXML document holds a <transpose>."""
    data = (REPO / name).read_bytes()
    if data[:2] == b"PK":
        data = musicxml_check.read_mxl(data)[0] or b""
    return b"<transpose" in data


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


class TransposeRoundTripTestCase(unittest.TestCase):
    """Export -> import keeps each note's sounding pitch, spelling, interval and doubling, and
    each corpus file's export is valid where the 4a ledger says the file's export is."""

    def check_round_trip(self, path, ledger_record=None):
        """The round trip of a file; with its ledger record, also the export's validity. The
        fixtures' exports are validated by TransposeWriterTestCase."""
        score = load(path)
        before = note_transpositions(score)
        if Path(path).name not in UNTRANSPOSED_FIXTURES:
            self.assertTrue(
                any(
                    diatonic or chromatic or doubling != "NONE"
                    for _, _, diatonic, chromatic, doubling in before.values()
                ),
                "no note is transposed or doubled",
            )
        data = export(score)
        again, _ = reloaded(data)
        after = note_transpositions(again)
        # A summary, not assertEqual: difflib over the notes of a large score takes hours.
        diff = {
            key: (before.get(key), after.get(key))
            for key in before.keys() | after.keys()
            if before.get(key) != after.get(key)
        }
        first = sorted(diff.items(), key=lambda item: item[0])[:5]
        self.assertEqual(0, len(diff), f"{len(diff)} notes differ; first: {first}")
        if ledger_record is None:
            return
        report = musicxml_check.check_bytes(data)
        self.assertEqual(
            [], [error for error in report.xsd_errors if TRANSPOSE_ELEMENT.search(error)]
        )
        if ledger_record.get("export_xsd") == "valid":
            self.assertTrue(report.xsd_valid, report.xsd_errors[:3])
        self.assertEqual(ledger_record.get("export_errors", []), report.errors)

    def test_every_fixture_keeps_its_transpositions(self):
        fixtures = sorted(UNIT_TEST.glob("transpose_*.musicxml"))
        self.assertEqual(14, len(fixtures))
        for path in fixtures:
            with self.subTest(fixture=path.name):
                self.check_round_trip(path)

    def test_the_files_left_out_still_do_not_load(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        for name in NOT_LOADABLE:
            with self.subTest(file=name):
                self.assertNotEqual("ok", ledger[name]["load"])

    def test_every_corpus_file_with_a_transpose_keeps_its_transpositions(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        for name in CORPUS_WITH_TRANSPOSE:
            if name in NOT_LOADABLE or corpus.is_slow(name):
                continue
            with self.subTest(file=name):
                self.check_round_trip(REPO / name, ledger[name])

    @unittest.skipUnless(SLOW_TESTS, "the slow corpus files run under `make corpus`")
    def test_the_slow_corpus_files_keep_their_transpositions(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        for name in CORPUS_WITH_TRANSPOSE:
            if corpus.is_slow(name):
                with self.subTest(file=name):
                    self.check_round_trip(REPO / name, ledger[name])

    def test_the_list_holds_every_corpus_file_with_a_transpose(self):
        found = {
            name
            for name in corpus.corpus_files()
            if "/transpose_" not in name and contains_transpose(name)
        }
        self.assertEqual(set(CORPUS_WITH_TRANSPOSE), found)

    def test_a_stored_diatonic_interval_of_zero_comes_back_as_the_conventional_one(self):
        score = single_part_score("Clarinet in Bb", 1)
        score.getPart(0).getMeasure(0).addNote(ml.Note("F#4", transposeChromatic=-2))
        again, printed = reloaded(export(score))
        note = again.getPart(0).getMeasure(0).getNote(0)
        self.assertEqual(
            (-1, -2, "E4"),
            (note.getTransposeDiatonic(), note.getTransposeChromatic(), note.getSoundingPitch()),
        )
        self.assertNotIn("[WARN]", printed)


if __name__ == "__main__":
    unittest.main()
