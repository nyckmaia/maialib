"""Comprehensive test suite for Interval class

This module provides extensive testing for the maialib Interval class,
covering creation, properties, quality, direction, and specific interval types.
"""

import unittest
import maialib as ml


class IntervalCreationTestCase(unittest.TestCase):
    """Tests for Interval creation and initialization"""

    def test_create_interval_default(self):
        """Test creating an interval with default values"""
        interval = ml.Interval()
        self.assertIsNotNone(interval)

    def test_create_interval_with_pitches(self):
        """Test creating an interval with pitch strings"""
        interval = ml.Interval("C4", "E4")
        self.assertIsNotNone(interval)
        self.assertEqual(interval.getName(), "M3")  # Abbreviated name

    def test_create_interval_with_notes(self):
        """Test creating an interval with Note objects"""
        note_a = ml.Note("D4")
        note_b = ml.Note("F4")
        interval = ml.Interval(note_a, note_b)
        self.assertIsNotNone(interval)


class IntervalPropertiesTestCase(unittest.TestCase):
    """Tests for Interval basic properties"""

    def test_get_name(self):
        """Test getting interval name"""
        interval = ml.Interval("C4", "G4")
        name = interval.getName()
        self.assertIsInstance(name, str)
        self.assertEqual(name, "P5")  # Abbreviated name

    def test_get_num_semitones(self):
        """Test getting number of semitones"""
        interval = ml.Interval("C4", "E4")
        semitones = interval.getNumSemitones()
        self.assertEqual(semitones, 4)  # Major third = 4 semitones

    def test_get_num_semitones_absolute(self):
        """Test getting absolute value of semitones"""
        interval = ml.Interval("G4", "C4")  # Descending fifth
        semitones = interval.getNumSemitones(absoluteValue=True)
        self.assertEqual(semitones, 7)

    def test_get_num_octaves(self):
        """Test getting number of octaves"""
        interval = ml.Interval("C4", "C5")
        octaves = interval.getNumOctaves()
        self.assertEqual(octaves, 1)

    def test_get_diatonic_interval(self):
        """Test getting diatonic interval"""
        interval = ml.Interval("C4", "E4")
        diatonic = interval.getDiatonicInterval()
        self.assertIsInstance(diatonic, int)

    def test_get_notes(self):
        """Test getting the two notes of the interval"""
        interval = ml.Interval("C4", "F4")
        notes = interval.getNotes()
        self.assertIsInstance(notes, list)
        self.assertEqual(len(notes), 2)


class IntervalDirectionTestCase(unittest.TestCase):
    """Tests for Interval direction"""

    def test_is_ascendant(self):
        """Test ascending interval detection"""
        interval = ml.Interval("C4", "E4")
        self.assertTrue(interval.isAscendant())

    def test_is_descendant(self):
        """Test descending interval detection"""
        interval = ml.Interval("G4", "D4")
        self.assertTrue(interval.isDescendant())

    def test_get_direction(self):
        """Test getting interval direction"""
        interval = ml.Interval("C4", "F4")
        direction = interval.getDirection()
        self.assertIsInstance(direction, str)


class IntervalQualityTestCase(unittest.TestCase):
    """Tests for Interval quality (major, minor, perfect, etc.)"""

    def test_is_major(self):
        """Test major interval detection"""
        interval = ml.Interval("C4", "E4")  # Major third
        self.assertTrue(interval.isMajor())

    def test_is_minor(self):
        """Test minor interval detection"""
        interval = ml.Interval("E4", "G4")  # Minor third
        self.assertTrue(interval.isMinor())

    def test_is_perfect(self):
        """Test perfect interval detection"""
        interval = ml.Interval("C4", "G4")  # Perfect fifth
        self.assertTrue(interval.isPerfect())

    def test_is_diminished(self):
        """Test diminished interval detection"""
        interval = ml.Interval("C4", "Gb4")  # Diminished fifth
        self.assertTrue(interval.isDiminished())

    def test_is_augmented(self):
        """Test augmented interval detection"""
        interval = ml.Interval("C4", "F#4")  # Augmented fourth
        self.assertTrue(interval.isAugmented())


class IntervalSimpleCompoundTestCase(unittest.TestCase):
    """Tests for simple vs compound intervals"""

    def test_is_simple(self):
        """Test simple interval (within one octave)"""
        interval = ml.Interval("C4", "G4")  # Fifth
        self.assertTrue(interval.isSimple())

    def test_is_compound(self):
        """Test compound interval (more than one octave)"""
        interval = ml.Interval("C4", "D5")  # Ninth
        self.assertTrue(interval.isCompound())


class IntervalSpecificTypesTestCase(unittest.TestCase):
    """Tests for specific interval types"""

    def test_is_perfect_unisson(self):
        """Test perfect unison detection"""
        interval = ml.Interval("C4", "C4")
        self.assertTrue(interval.isPerfectUnisson())

    def test_is_minor_second(self):
        """Test minor second detection"""
        interval = ml.Interval("C4", "Db4")
        self.assertTrue(interval.isMinorSecond())

    def test_is_major_second(self):
        """Test major second detection"""
        interval = ml.Interval("C4", "D4")
        self.assertTrue(interval.isMajorSecond())

    def test_is_minor_third(self):
        """Test minor third detection"""
        interval = ml.Interval("C4", "Eb4")
        self.assertTrue(interval.isMinorThird())

    def test_is_major_third(self):
        """Test major third detection"""
        interval = ml.Interval("C4", "E4")
        self.assertTrue(interval.isMajorThird())

    def test_is_perfect_fourth(self):
        """Test perfect fourth detection"""
        interval = ml.Interval("C4", "F4")
        self.assertTrue(interval.isPerfectFourth())

    def test_is_augmented_fourth(self):
        """Test augmented fourth (tritone) detection"""
        interval = ml.Interval("C4", "F#4")
        self.assertTrue(interval.isAugmentedFourth())

    def test_is_diminished_fifth(self):
        """Test diminished fifth (tritone) detection"""
        interval = ml.Interval("C4", "Gb4")
        self.assertTrue(interval.isDiminishedFifth())

    def test_is_perfect_fifth(self):
        """Test perfect fifth detection"""
        interval = ml.Interval("C4", "G4")
        self.assertTrue(interval.isPerfectFifth())

    def test_is_minor_sixth(self):
        """Test minor sixth detection"""
        interval = ml.Interval("C4", "Ab4")
        self.assertTrue(interval.isMinorSixth())

    def test_is_major_sixth(self):
        """Test major sixth detection"""
        interval = ml.Interval("C4", "A4")
        self.assertTrue(interval.isMajorSixth())

    def test_is_minor_seventh(self):
        """Test minor seventh detection"""
        interval = ml.Interval("C4", "Bb4")
        self.assertTrue(interval.isMinorSeventh())

    def test_is_major_seventh(self):
        """Test major seventh detection"""
        interval = ml.Interval("C4", "B4")
        self.assertTrue(interval.isMajorSeventh())

    def test_is_perfect_octave(self):
        """Test perfect octave detection"""
        interval = ml.Interval("C4", "C5")
        self.assertTrue(interval.isPerfectOctave())


class IntervalGenericTypesTestCase(unittest.TestCase):
    """Tests for generic interval types (abstraction 2)"""

    def test_is_second(self):
        """Test second interval detection (any quality)"""
        interval = ml.Interval("C4", "D4")
        self.assertTrue(interval.isSecond())

    def test_is_third(self):
        """Test third interval detection (any quality)"""
        interval = ml.Interval("C4", "E4")
        self.assertTrue(interval.isThird())

    def test_is_fourth(self):
        """Test fourth interval detection (any quality)"""
        interval = ml.Interval("C4", "F4")
        self.assertTrue(interval.isFourth())

    def test_is_fifth(self):
        """Test fifth interval detection (any quality)"""
        interval = ml.Interval("C4", "G4")
        self.assertTrue(interval.isFifth())

    def test_is_sixth(self):
        """Test sixth interval detection (any quality)"""
        interval = ml.Interval("C4", "A4")
        self.assertTrue(interval.isSixth())

    def test_is_seventh(self):
        """Test seventh interval detection (any quality)"""
        interval = ml.Interval("C4", "B4")
        self.assertTrue(interval.isSeventh())

    def test_is_octave(self):
        """Test octave interval detection (any quality)"""
        interval = ml.Interval("C4", "C5")
        self.assertTrue(interval.isOctave())


class IntervalAnyOctaveTestCase(unittest.TestCase):
    """Tests for any octave interval detection"""

    def test_is_any_octave_major_third(self):
        """Test any octave major third detection"""
        interval1 = ml.Interval("C4", "E4")  # Same octave
        interval2 = ml.Interval("C4", "E5")  # Different octave
        self.assertTrue(interval1.isAnyOctaveMajorThird())
        self.assertTrue(interval2.isAnyOctaveMajorThird())

    def test_is_any_octave_perfect_fifth(self):
        """Test any octave perfect fifth detection"""
        interval1 = ml.Interval("C4", "G4")
        interval2 = ml.Interval("C3", "G5")
        self.assertTrue(interval1.isAnyOctavePerfectFifth())
        self.assertTrue(interval2.isAnyOctavePerfectFifth())

    def test_is_any_octave_minor_third(self):
        """Test any octave minor third detection"""
        interval = ml.Interval("E4", "G5")
        self.assertTrue(interval.isAnyOctaveMinorThird())


class IntervalCompoundIntervalsTestCase(unittest.TestCase):
    """Tests for compound intervals (9th, 11th, 13th)"""

    def test_is_minor_ninth(self):
        """Test minor ninth detection"""
        interval = ml.Interval("C4", "Db5")
        self.assertTrue(interval.isMinorNinth())

    def test_is_major_ninth(self):
        """Test major ninth detection"""
        interval = ml.Interval("C4", "D5")
        self.assertTrue(interval.isMajorNinth())

    def test_is_perfect_eleventh(self):
        """Test perfect eleventh detection"""
        interval = ml.Interval("C4", "F5")
        self.assertTrue(interval.isPerfectEleventh())

    def test_is_sharp_eleventh(self):
        """Test sharp eleventh detection"""
        interval = ml.Interval("C4", "F#5")
        self.assertTrue(interval.isSharpEleventh())

    def test_is_major_thirdteenth(self):
        """Test major thirteenth detection"""
        interval = ml.Interval("C4", "A5")
        self.assertTrue(interval.isMajorThirdteenth())


class IntervalManipulationTestCase(unittest.TestCase):
    """Tests for Interval manipulation"""

    def test_set_notes_with_pitches(self):
        """Test setting notes with pitch strings"""
        interval = ml.Interval()
        interval.setNotes("C4", "G4")
        self.assertTrue(interval.isPerfectFifth())

    def test_set_notes_with_note_objects(self):
        """Test setting notes with Note objects"""
        interval = ml.Interval()
        note_a = ml.Note("D4")
        note_b = ml.Note("A4")
        interval.setNotes(note_a, note_b)
        self.assertTrue(interval.isPerfectFifth())


class IntervalTonalTestCase(unittest.TestCase):
    """Tests for tonal interval functionality"""

    def test_is_tonal(self):
        """Test tonal interval detection"""
        interval = ml.Interval("C4", "E4")
        is_tonal = interval.isTonal()
        self.assertIsInstance(is_tonal, bool)


class IntervalCentsTestCase(unittest.TestCase):
    """Tests for interval cents conversion"""

    def test_to_cents_default(self):
        """Test converting interval to cents (default A4=440Hz)"""
        interval = ml.Interval("C4", "C5")  # Octave
        cents = interval.toCents()
        self.assertIsInstance(cents, (int, float))
        self.assertAlmostEqual(cents, 1200.0, delta=1.0)  # Octave = 1200 cents

    def test_to_cents_custom_frequency(self):
        """Test converting interval to cents with custom A4 frequency"""
        interval = ml.Interval("C4", "C5")
        cents = interval.toCents(freqA4=442.0)
        self.assertIsInstance(cents, (int, float))


class IntervalEdgeCasesTestCase(unittest.TestCase):
    """Tests for Interval edge cases"""

    def test_descending_intervals(self):
        """Test descending intervals"""
        interval = ml.Interval("G4", "C4")  # Descending fifth
        self.assertTrue(interval.isDescendant())
        self.assertTrue(interval.isFifth())

    def test_large_compound_intervals(self):
        """Test large compound intervals"""
        interval = ml.Interval("C3", "C6")  # 3 octaves
        self.assertTrue(interval.isCompound())
        self.assertEqual(interval.getNumOctaves(), 3)

    def test_enharmonic_intervals(self):
        """Test enharmonic intervals with useEnharmony flag"""
        interval1 = ml.Interval("C4", "F#4")  # Augmented fourth
        interval2 = ml.Interval("C4", "Gb4")  # Diminished fifth

        # Without enharmony, they're different
        self.assertTrue(interval1.isAugmentedFourth(useEnharmony=False))
        self.assertTrue(interval2.isDiminishedFifth(useEnharmony=False))

        # With enharmony, they're the same (tritone)
        self.assertTrue(interval1.isAugmentedFourth(useEnharmony=True))
        self.assertTrue(interval2.isAugmentedFourth(useEnharmony=True))


if __name__ == '__main__':
    unittest.main()
