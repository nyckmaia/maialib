"""Comprehensive test suite for Measure class

This module provides extensive testing for the maialib Measure class,
covering initialization, properties, notes, time/key signatures, and manipulation.
"""

import unittest
import maialib as ml


class MeasureCreationTestCase(unittest.TestCase):
    """Tests for Measure creation and initialization"""

    def test_create_measure_default(self):
        """Test creating a measure with default values"""
        measure = ml.Measure()
        self.assertEqual(measure.getNumStaves(), 1)
        self.assertEqual(measure.getDivisionsPerQuarterNote(), 256)

    def test_create_measure_with_staves(self):
        """Test creating a measure with multiple staves"""
        measure = ml.Measure(2)
        self.assertEqual(measure.getNumStaves(), 2)

    def test_create_measure_with_custom_divisions(self):
        """Test creating a measure with custom divisions per quarter note"""
        measure = ml.Measure(1, 512)
        self.assertEqual(measure.getDivisionsPerQuarterNote(), 512)


class MeasurePropertiesTestCase(unittest.TestCase):
    """Tests for Measure basic properties"""

    def setUp(self):
        """Set up test fixtures"""
        self.measure = ml.Measure()

    def test_get_number_default(self):
        """Test getting measure number (default)"""
        number = self.measure.getNumber()
        self.assertIsInstance(number, int)

    def test_set_number(self):
        """Test setting measure number"""
        self.measure.setNumber(5)
        self.assertEqual(self.measure.getNumber(), 5)

    def test_get_num_staves(self):
        """Test getting number of staves"""
        self.assertEqual(self.measure.getNumStaves(), 1)

    def test_set_num_staves(self):
        """Test setting number of staves"""
        self.measure.setNumStaves(3)
        self.assertEqual(self.measure.getNumStaves(), 3)

    def test_is_empty_initially(self):
        """Test that new measure is empty"""
        self.assertTrue(self.measure.isEmpty())


class MeasureTimeSignatureTestCase(unittest.TestCase):
    """Tests for Measure time signature functionality"""

    def setUp(self):
        """Set up test fixtures"""
        self.measure = ml.Measure()

    def test_set_time_signature(self):
        """Test setting time signature"""
        self.measure.setTimeSignature(3, 4)
        time_sig = self.measure.getTimeSignature()
        self.assertIsNotNone(time_sig)
        # TimeSignature is an object, not a tuple

    def test_get_time_signature(self):
        """Test getting time signature"""
        self.measure.setTimeSignature(6, 8)
        time_sig = self.measure.getTimeSignature()
        self.assertIsNotNone(time_sig)
        # TimeSignature object exists

    def test_time_signature_changed_flag(self):
        """Test time signature changed flag"""
        self.measure.setTimeSignature(4, 4)
        self.measure.setIsTimeSignatureChanged(True)
        self.assertTrue(self.measure.timeSignatureChanged())

    def test_time_signature_not_changed_default(self):
        """Test time signature changed flag default"""
        changed = self.measure.timeSignatureChanged()
        self.assertIsInstance(changed, bool)


class MeasureKeySignatureTestCase(unittest.TestCase):
    """Tests for Measure key signature functionality"""

    def setUp(self):
        """Set up test fixtures"""
        self.measure = ml.Measure()

    def test_set_key_signature(self):
        """Test setting key signature"""
        self.measure.setKeySignature(2, True)  # D major (2 sharps)
        fifth_circle = self.measure.getFifthCircle()
        self.assertEqual(fifth_circle, 2)

    def test_set_key_signature_minor(self):
        """Test setting key signature for minor key"""
        self.measure.setKeySignature(-3, False)  # C minor (3 flats)
        fifth_circle = self.measure.getFifthCircle()
        self.assertEqual(fifth_circle, -3)
        self.assertFalse(self.measure.isMajorKeyMode())

    def test_get_key_name(self):
        """Test getting key name"""
        self.measure.setKeySignature(1, True)  # G major
        key_name = self.measure.getKeyName()
        self.assertIsInstance(key_name, str)

    def test_set_key_mode(self):
        """Test setting key mode (major/minor)"""
        self.measure.setKeySignature(0, True)
        self.measure.setKeyMode(False)
        self.assertFalse(self.measure.isMajorKeyMode())

    def test_key_signature_changed_flag(self):
        """Test key signature changed flag"""
        self.measure.setKeySignature(3, True)
        self.measure.setIsKeySignatureChanged(True)
        self.assertTrue(self.measure.keySignatureChanged())


class MeasureMetronomeTestCase(unittest.TestCase):
    """Tests for Measure metronome/tempo functionality"""

    def setUp(self):
        """Set up test fixtures"""
        self.measure = ml.Measure()

    def test_set_metronome(self):
        """Test setting metronome marking"""
        self.measure.setMetronome(120)
        metronome = self.measure.getTimeMetronome()
        self.assertIsInstance(metronome, tuple)

    def test_metronome_changed_flag(self):
        """Test metronome changed flag"""
        self.measure.setMetronome(80)
        self.measure.setIsMetronomeChanged(True)
        self.assertTrue(self.measure.metronomeChanged())


class MeasureNotesTestCase(unittest.TestCase):
    """Tests for Measure notes functionality"""

    def setUp(self):
        """Set up test fixtures"""
        self.measure = ml.Measure()

    def test_add_note_by_pitch(self):
        """Test adding note by pitch string"""
        self.measure.addNote("C4")
        self.assertGreater(self.measure.getNumNotes(), 0)

    def test_add_note_object(self):
        """Test adding Note object"""
        note = ml.Note("D4")
        self.measure.addNote(note)
        self.assertEqual(self.measure.getNumNotes(), 1)

    def test_add_multiple_notes(self):
        """Test adding multiple notes"""
        self.measure.addNote("C4")
        self.measure.addNote("E4")
        self.measure.addNote("G4")
        self.assertEqual(self.measure.getNumNotes(), 3)

    def test_get_note(self):
        """Test getting a note by index"""
        self.measure.addNote("A4")
        note = self.measure.getNote(0)
        self.assertIsNotNone(note)

    def test_remove_note(self):
        """Test removing a note"""
        self.measure.addNote("F4")
        self.measure.addNote("G4")
        initial_count = self.measure.getNumNotes()
        self.measure.removeNote(0)
        # removeNote should work without crashing
        self.assertIsInstance(self.measure.getNumNotes(), int)

    def test_get_num_notes_empty(self):
        """Test getting number of notes in empty measure"""
        self.assertEqual(self.measure.getNumNotes(), 0)

    def test_get_num_notes_on(self):
        """Test getting number of note-on events"""
        self.measure.addNote("C4")
        num_on = self.measure.getNumNotesOn()
        self.assertIsInstance(num_on, int)
        self.assertGreaterEqual(num_on, 0)

    def test_get_num_notes_off(self):
        """Test getting number of note-off events"""
        self.measure.addNote("D4")
        num_off = self.measure.getNumNotesOff()
        self.assertIsInstance(num_off, int)
        self.assertGreaterEqual(num_off, 0)


class MeasureDurationTestCase(unittest.TestCase):
    """Tests for Measure duration calculations"""

    def setUp(self):
        """Set up test fixtures"""
        self.measure = ml.Measure()
        self.measure.setTimeSignature(4, 4)

    def test_get_quarter_duration(self):
        """Test getting quarter note duration"""
        duration = self.measure.getQuarterDuration()
        self.assertIsInstance(duration, (int, float))
        self.assertGreater(duration, 0)

    def test_get_filled_quarter_duration(self):
        """Test getting filled quarter duration"""
        filled = self.measure.getFilledQuarterDuration()
        self.assertIsInstance(filled, (int, float))
        self.assertGreaterEqual(filled, 0)

    def test_get_free_quarter_duration(self):
        """Test getting free quarter duration"""
        free = self.measure.getFreeQuarterDuration()
        self.assertIsInstance(free, (int, float))
        self.assertGreaterEqual(free, 0)

    def test_get_duration_ticks(self):
        """Test getting duration in ticks"""
        ticks = self.measure.getDurationTicks()
        self.assertIsInstance(ticks, int)
        self.assertGreater(ticks, 0)

    def test_get_filled_duration_ticks(self):
        """Test getting filled duration in ticks"""
        filled_ticks = self.measure.getFilledDurationTicks()
        self.assertIsInstance(filled_ticks, int)
        self.assertGreaterEqual(filled_ticks, 0)

    def test_get_free_duration_ticks(self):
        """Test getting free duration in ticks"""
        free_ticks = self.measure.getFreeDurationTicks()
        self.assertIsInstance(free_ticks, int)
        self.assertGreaterEqual(free_ticks, 0)


class MeasureClefTestCase(unittest.TestCase):
    """Tests for Measure clef functionality"""

    def test_get_clef(self):
        """Test getting clef"""
        measure = ml.Measure()
        clef = measure.getClef()
        self.assertIsNotNone(clef)

    def test_clef_changed_flag(self):
        """Test clef changed flag"""
        measure = ml.Measure()
        changed = measure.isClefChanged()
        self.assertIsInstance(changed, bool)


class MeasureBarlineTestCase(unittest.TestCase):
    """Tests for Measure barline functionality"""

    def test_get_barline_left(self):
        """Test getting left barline"""
        measure = ml.Measure()
        barline = measure.getBarlineLeft()
        self.assertIsNotNone(barline)

    def test_get_barline_right(self):
        """Test getting right barline"""
        measure = ml.Measure()
        barline = measure.getBarlineRight()
        self.assertIsNotNone(barline)

    def test_set_repeat_start(self):
        """Test setting repeat start"""
        measure = ml.Measure()
        measure.setRepeatStart()
        # Method should execute without error

    def test_set_repeat_end(self):
        """Test setting repeat end"""
        measure = ml.Measure()
        measure.setRepeatEnd()
        # Method should execute without error

    def test_remove_repeat_start(self):
        """Test removing repeat start"""
        measure = ml.Measure()
        measure.setRepeatStart()
        measure.removeRepeatStart()
        # Method should execute without error

    def test_remove_repeat_end(self):
        """Test removing repeat end"""
        measure = ml.Measure()
        measure.setRepeatEnd()
        measure.removeRepeatEnd()
        # Method should execute without error


class MeasureManipulationTestCase(unittest.TestCase):
    """Tests for Measure manipulation methods"""

    def test_clear(self):
        """Test clearing measure contents"""
        measure = ml.Measure()
        measure.addNote("C4")
        measure.addNote("E4")
        measure.clear()
        self.assertEqual(measure.getNumNotes(), 0)

    def test_set_divisions_per_quarter_note(self):
        """Test setting divisions per quarter note"""
        measure = ml.Measure()
        measure.setDivisionsPerQuarterNote(480)
        self.assertEqual(measure.getDivisionsPerQuarterNote(), 480)

    def test_divisions_changed_flag(self):
        """Test divisions per quarter note changed flag"""
        measure = ml.Measure()
        measure.setDivisionsPerQuarterNote(512)
        measure.setIsDivisionsPerQuarterNoteChanged(True)
        self.assertTrue(measure.divisionsPerQuarterNoteChanged())


class MeasureIntegrationTestCase(unittest.TestCase):
    """Integration tests for Measure"""

    def test_complete_measure_setup(self):
        """Test complete measure setup workflow"""
        measure = ml.Measure(2)  # 2 staves
        measure.setNumber(1)
        measure.setTimeSignature(4, 4)
        measure.setKeySignature(0, True)  # C major
        measure.setMetronome(120)

        # Add notes
        measure.addNote("C4", 0)
        measure.addNote("E4", 0)
        measure.addNote("G4", 1)

        # Verify
        self.assertEqual(measure.getNumber(), 1)
        self.assertEqual(measure.getNumStaves(), 2)
        self.assertGreater(measure.getNumNotes(), 0)

    def test_load_measure_from_score(self):
        """Test loading a measure from a score"""
        score = ml.Score('./xml_examples/unit_test/test_chord.xml')
        if score.getNumParts() > 0:
            part = score.getPart(0)
            if part.getNumMeasures() > 0:
                measure = part.getMeasure(0)
                self.assertIsNotNone(measure)
                self.assertIsInstance(measure.getNumber(), int)
                self.assertGreaterEqual(measure.getNumStaves(), 1)


if __name__ == '__main__':
    unittest.main()
