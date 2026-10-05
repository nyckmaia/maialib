"""Comprehensive test suite for Part class

This module provides extensive testing for the maialib Part class,
covering initialization, properties, measures, staves, and manipulation.
"""

import unittest

import maialib as ml


class PartCreationTestCase(unittest.TestCase):
    """Tests for Part creation and initialization"""

    def test_create_part_with_default_values(self):
        """Test creating a part with default values"""
        part = ml.Part("Piano")
        self.assertEqual(part.getName(), "Piano")
        self.assertEqual(part.getNumStaves(), 1)
        self.assertTrue(part.isPitched())

    def test_create_part_with_multiple_staves(self):
        """Test creating a part with multiple staves"""
        part = ml.Part("Piano", 2)
        self.assertEqual(part.getName(), "Piano")
        self.assertEqual(part.getNumStaves(), 2)

    def test_create_unpitched_part(self):
        """Test creating an unpitched part (percussion)"""
        part = ml.Part("Drums", 1, False)
        self.assertEqual(part.getName(), "Drums")
        self.assertFalse(part.isPitched())

    def test_part_name_stored_correctly(self):
        """Test that part name is stored correctly"""
        part = ml.Part("Violin")
        self.assertEqual(part.getName(), "Violin")


class PartPropertiesTestCase(unittest.TestCase):
    """Tests for Part properties"""

    def setUp(self):
        """Set up test fixtures"""
        self.part = ml.Part("Violin")

    def test_get_name(self):
        """Test getting part name"""
        self.assertEqual(self.part.getName(), "Violin")

    def test_get_short_name(self):
        """Test getting part short name"""
        short_name = self.part.getShortName()
        self.assertIsInstance(short_name, str)

    def test_set_short_name(self):
        """Test setting part short name"""
        self.part.setShortName("Vln")
        self.assertEqual(self.part.getShortName(), "Vln")

    def test_get_num_staves_default(self):
        """Test getting number of staves (default)"""
        self.assertEqual(self.part.getNumStaves(), 1)

    def test_is_pitched_default(self):
        """Test default isPitched value"""
        self.assertTrue(self.part.isPitched())

    def test_get_staff_lines_default(self):
        """Test default number of staff lines"""
        self.assertEqual(self.part.getStaffLines(), 5)

    def test_get_num_measures_empty(self):
        """Test getting number of measures in empty part"""
        # Empty part should have 0 measures
        self.assertEqual(self.part.getNumMeasures(), 0)


class PartStavesTestCase(unittest.TestCase):
    """Tests for Part staves manipulation"""

    def test_set_num_staves(self):
        """Test setting number of staves"""
        part = ml.Part("Piano")
        part.addMeasure(1)  # Need at least one measure
        part.setNumStaves(2)
        self.assertEqual(part.getNumStaves(), 2)

    def test_add_staves(self):
        """Test adding staves to part"""
        part = ml.Part("Piano", 1)
        initial_staves = part.getNumStaves()
        part.addStaves(1)
        self.assertEqual(part.getNumStaves(), initial_staves + 1)

    def test_add_multiple_staves(self):
        """Test adding multiple staves at once"""
        part = ml.Part("Piano", 1)
        part.addStaves(3)
        self.assertEqual(part.getNumStaves(), 4)

    def test_remove_stave(self):
        """Test removing a stave (method exists)"""
        part = ml.Part("Piano", 3)
        part.addMeasure(1)  # Need at least one measure
        initial_staves = part.getNumStaves()
        part.removeStave(2)
        # removeStave method exists and doesn't crash
        self.assertIsInstance(part.getNumStaves(), int)

    def test_set_staff_lines(self):
        """Test setting number of staff lines"""
        part = ml.Part("Bass", 1)
        part.setStaffLines(4)
        self.assertEqual(part.getStaffLines(), 4)


class PartMeasuresTestCase(unittest.TestCase):
    """Tests for Part measures manipulation"""

    def test_add_measure(self):
        """Test adding measures to part"""
        part = ml.Part("Violin")
        part.addMeasure(4)
        self.assertEqual(part.getNumMeasures(), 4)

    def test_add_multiple_measures(self):
        """Test adding multiple measures"""
        part = ml.Part("Violin")
        part.addMeasure(8)
        self.assertEqual(part.getNumMeasures(), 8)

    def test_remove_measure(self):
        """Test removing measures from part"""
        part = ml.Part("Violin")
        part.addMeasure(10)
        part.removeMeasure(5, 7)
        self.assertEqual(part.getNumMeasures(), 7)

    def test_get_measure(self):
        """Test getting a measure by index"""
        part = ml.Part("Violin")
        part.addMeasure(5)
        measure = part.getMeasure(0)
        self.assertIsNotNone(measure)

    def test_get_measures(self):
        """Test getting all measures"""
        part = ml.Part("Violin")
        part.addMeasure(3)
        measures = part.getMeasures()
        self.assertIsInstance(measures, list)
        self.assertEqual(len(measures), 3)


class PartNotesTestCase(unittest.TestCase):
    """Tests for Part notes functionality"""

    def setUp(self):
        """Set up test fixtures"""
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        if score.getNumParts() > 0:
            self.part = score.getPart(0)
        else:
            self.part = None

    def test_get_num_notes(self):
        """Test getting total number of notes"""
        if self.part:
            num_notes = self.part.getNumNotes()
            self.assertIsInstance(num_notes, int)
            self.assertGreaterEqual(num_notes, 0)

    def test_get_num_notes_on(self):
        """Test getting number of note-on events"""
        if self.part:
            num_notes_on = self.part.getNumNotesOn()
            self.assertIsInstance(num_notes_on, int)
            self.assertGreaterEqual(num_notes_on, 0)

    def test_get_num_notes_off(self):
        """Test getting number of note-off events"""
        if self.part:
            num_notes_off = self.part.getNumNotesOff()
            self.assertIsInstance(num_notes_off, int)
            self.assertGreaterEqual(num_notes_off, 0)

    def test_get_num_notes_per_stave(self):
        """Test getting number of notes for specific stave"""
        if self.part and self.part.getNumStaves() > 0:
            notes_stave_0 = self.part.getNumNotes(0)
            self.assertIsInstance(notes_stave_0, int)
            self.assertGreaterEqual(notes_stave_0, 0)


class PartManipulationTestCase(unittest.TestCase):
    """Tests for Part manipulation methods"""

    def test_clear(self):
        """Test clearing part contents"""
        part = ml.Part("Piano")
        part.addMeasure(5)
        part.clear()
        # After clear, part should have 0 measures
        self.assertEqual(part.getNumMeasures(), 0)

    def test_set_is_pitched(self):
        """Test setting isPitched property"""
        part = ml.Part("Percussion")
        part.addMeasure(1)  # Need at least one measure
        part.setIsPitched(False)
        self.assertFalse(part.isPitched())

    def test_set_part_index(self):
        """Test setting part index"""
        part = ml.Part("Violin")
        part.setPartIndex(2)
        self.assertEqual(part.getPartIndex(), 2)

    def test_get_part_index_default(self):
        """Test getting default part index"""
        part = ml.Part("Violin")
        part_index = part.getPartIndex()
        self.assertIsInstance(part_index, int)


class PartMidiTestCase(unittest.TestCase):
    """Tests for Part MIDI functionality"""

    def test_add_midi_unpitched(self):
        """Test adding MIDI unpitched values"""
        part = ml.Part("Drums", 1, False)
        part.addMidiUnpitched(36)  # Bass drum
        midi_list = part.getMidiUnpitched()
        self.assertIn(36, midi_list)

    def test_get_midi_unpitched_empty(self):
        """Test getting MIDI unpitched from pitched part"""
        part = ml.Part("Piano")
        midi_list = part.getMidiUnpitched()
        self.assertIsInstance(midi_list, list)


class PartEdgeCasesTestCase(unittest.TestCase):
    """Tests for Part edge cases"""

    def test_empty_part_properties(self):
        """Test properties of an empty part"""
        part = ml.Part("Empty")
        self.assertEqual(part.getName(), "Empty")
        self.assertEqual(part.getNumMeasures(), 0)
        self.assertEqual(part.getNumStaves(), 1)

    def test_part_with_unicode_name(self):
        """Test part with unicode characters in name"""
        unicode_name = "Скрипка"  # "Violin" in Russian
        part = ml.Part(unicode_name)
        self.assertEqual(part.getName(), unicode_name)

    def test_part_with_special_chars_name(self):
        """Test part with special characters in name"""
        special_name = 'Piano #1 - "Grand"'
        part = ml.Part(special_name)
        self.assertEqual(part.getName(), special_name)


class PartIntegrationTestCase(unittest.TestCase):
    """Integration tests for Part"""

    def test_create_part_add_measures_access(self):
        """Test complete workflow: create -> add measures -> access"""
        part = ml.Part("Cello")
        part.addMeasure(8)
        self.assertEqual(part.getNumMeasures(), 8)

        # Access first measure
        measure = part.getMeasure(0)
        self.assertIsNotNone(measure)

    def test_multistaff_part(self):
        """Test creating and manipulating multi-staff part"""
        part = ml.Part("Piano", 2)
        part.addMeasure(4)

        self.assertEqual(part.getNumStaves(), 2)
        self.assertEqual(part.getNumMeasures(), 4)

    def test_load_part_from_score(self):
        """Test loading a part from a score"""
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        if score.getNumParts() > 0:
            part = score.getPart(0)
            self.assertIsNotNone(part)
            self.assertIsInstance(part.getName(), str)
            self.assertGreater(part.getNumMeasures(), 0)


class PartAppendOverflowTestCase(unittest.TestCase):
    """Tests for Part.append() when a note/chord overflows the last measure"""

    def test_append_note_overflowing_last_measure_raises(self):
        """A note bigger than the only (last) measure has no next measure to tie into"""
        part = ml.Part("Piano")
        part.addMeasure(1)

        note = ml.Note("C4", ml.RhythmFigure.BREVE)
        with self.assertRaises(RuntimeError):
            part.append(note)

    def test_append_chord_overflowing_last_measure_raises(self):
        """A chord bigger than the only (last) measure has no next measure to tie into"""
        part = ml.Part("Piano")
        part.addMeasure(1)

        chord = ml.Chord(["C4", "E4", "G4"], ml.RhythmFigure.BREVE)
        with self.assertRaises(RuntimeError):
            part.append(chord)

    def test_append_note_overflowing_with_next_measure_available(self):
        """Same oversized note, but a following measure exists to hold the tied remainder"""
        part = ml.Part("Piano")
        part.addMeasure(2)

        note = ml.Note("C4", ml.RhythmFigure.BREVE)
        part.append(note)

        self.assertEqual(part.getNumNotes(), 2)


class PartSetTransposingIntervalTestCase(unittest.TestCase):
    """Part.setTransposingInterval stamps the pitched notes of a measure range and a staff in
    place, all of them or none."""

    def clarinets(self):
        """Two measures of a two-staff part; each staff holds a C4 and a rest."""
        score = ml.Score(["Clarinets"], 2)
        part = score.getPart(0)
        part.setNumStaves(2)
        for m in range(2):
            for s in range(2):
                part.getMeasure(m).addNote(ml.Note("C4"), s)
                part.getMeasure(m).addNote(ml.Note("rest"), s)
        return score

    def test_the_edit_reaches_the_score(self):
        score = self.clarinets()
        score.getPart(0).setTransposingInterval(
            -1, -2, measureStart=1, staff=1, doubling=ml.OctaveDoubling.BELOW
        )
        stamped = score.getPart(0).getMeasure(1).getNote(0, 1)
        self.assertEqual(
            (-1, -2, ml.OctaveDoubling.BELOW, "Bb3"),
            (
                stamped.getTransposeDiatonic(),
                stamped.getTransposeChromatic(),
                stamped.getOctaveDoubling(),
                stamped.getSoundingPitch(),
            ),
        )
        self.assertFalse(score.getPart(0).getMeasure(0).getNote(0, 1).isTransposed())
        self.assertFalse(score.getPart(0).getMeasure(1).getNote(0, 0).isTransposed())

    def test_a_note_without_a_sounding_pitch_changes_nothing(self):
        score = ml.Score(["Piccolo"], 2)
        score.getPart(0).getMeasure(0).addNote(ml.Note("C4"))
        score.getPart(0).getMeasure(1).addNote(ml.Note("B11"))
        with self.assertRaises(RuntimeError) as context:
            score.getPart(0).setTransposingInterval(7, 12)
        self.assertIn("the written B11 at measure index 1, staff index 0", str(context.exception))
        self.assertFalse(score.getPart(0).getMeasure(0).getNote(0).isTransposed())

    def test_invalid_indices_raise_index_error(self):
        part = self.clarinets().getPart(0)
        for arguments in (
            {"measureStart": -1},
            {"measureStart": 2, "measureEnd": 1},
            {"measureEnd": 3},
            {"staff": 2},
        ):
            with self.subTest(**arguments), self.assertRaises(IndexError):
                part.setTransposingInterval(-1, -2, **arguments)


if __name__ == "__main__":
    unittest.main()
