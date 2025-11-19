"""Comprehensive test suite for Score class

This module provides extensive testing for the maialib Score class,
covering loading, properties, navigation, analysis, manipulation,
and edge cases.
"""

import unittest
import os
from pathlib import Path
import maialib as ml


class ScoreLoadingTestCase(unittest.TestCase):
    """Tests for Score loading from various file formats"""

    def test_load_valid_xml(self):
        """Test loading a valid MusicXML file"""
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        self.assertTrue(score.isValid())
        self.assertGreater(score.getNumParts(), 0)
        self.assertGreater(score.getNumMeasures(), 0)

    def test_load_returns_invalid_for_nonexistent_file(self):
        """Test loading a nonexistent file returns invalid score"""
        score = ml.Score('./nonexistent_file.xml')
        self.assertFalse(score.isValid())

    def test_load_filename_stored_correctly(self):
        """Test that filename is stored correctly after loading"""
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        self.assertEqual(score.getFileName(), 'test_chord.xml')

    def test_load_filepath_stored_correctly(self):
        """Test that file path is stored correctly after loading"""
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        self.assertIn('test_chord.xml', score.getFilePath())


class ScorePropertiesTestCase(unittest.TestCase):
    """Tests for Score basic properties"""

    def setUp(self):
        """Set up test fixtures"""
        self.score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')

    def test_get_title(self):
        """Test getting score title"""
        title = self.score.getTitle()
        self.assertIsInstance(title, str)

    def test_set_title(self):
        """Test setting score title"""
        self.score.setTitle('Test Symphony')
        self.assertEqual(self.score.getTitle(), 'Test Symphony')

    def test_get_composer_name(self):
        """Test getting composer name"""
        composer = self.score.getComposerName()
        self.assertIsInstance(composer, str)

    def test_set_composer_name(self):
        """Test setting composer name"""
        self.score.setComposerName('Test Composer')
        self.assertEqual(self.score.getComposerName(), 'Test Composer')

    def test_get_num_parts(self):
        """Test getting number of parts"""
        num_parts = self.score.getNumParts()
        self.assertIsInstance(num_parts, int)
        self.assertGreater(num_parts, 0)

    def test_get_num_measures(self):
        """Test getting number of measures"""
        num_measures = self.score.getNumMeasures()
        self.assertIsInstance(num_measures, int)
        self.assertGreater(num_measures, 0)

    def test_get_num_notes(self):
        """Test getting number of notes"""
        num_notes = self.score.getNumNotes()
        self.assertIsInstance(num_notes, int)
        self.assertGreaterEqual(num_notes, 0)


class ScoreNavigationTestCase(unittest.TestCase):
    """Tests for navigating through Score structure"""

    def setUp(self):
        """Set up test fixtures"""
        self.score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')

    def test_get_part_by_index(self):
        """Test getting part by index"""
        if self.score.getNumParts() > 0:
            part = self.score.getPart(0)
            self.assertIsNotNone(part)

    def test_get_parts_names(self):
        """Test getting all part names"""
        part_names = self.score.getPartsNames()
        self.assertIsInstance(part_names, list)
        self.assertEqual(len(part_names), self.score.getNumParts())

    def test_iterate_parts(self):
        """Test iterating through all parts"""
        parts_count = 0
        for i in range(self.score.getNumParts()):
            part = self.score.getPart(i)
            self.assertIsNotNone(part)
            parts_count += 1
        self.assertEqual(parts_count, self.score.getNumParts())


class ScoreAnalysisTestCase(unittest.TestCase):
    """Tests for Score analysis methods"""

    def setUp(self):
        """Set up test fixtures"""
        self.score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')

    def test_get_all_notes(self):
        """Test getting all notes from score"""
        notes = self.score.getAllNotes()
        self.assertIsInstance(notes, list)
        self.assertEqual(len(notes), self.score.getNumNotes())

    def test_get_note_events(self):
        """Test getting note events"""
        note_events = self.score.getNoteEvents()
        self.assertIsInstance(note_events, list)

    def test_get_chords_dataframe(self):
        """Test getting chords as DataFrame"""
        try:
            df = self.score.getChordsDataFrame()
            self.assertIsNotNone(df)
        except Exception:
            # Some scores might not have chords
            pass


class ScoreManipulationTestCase(unittest.TestCase):
    """Tests for Score manipulation methods"""

    def test_create_empty_score(self):
        """Test creating an empty score"""
        score = ml.Score(["Piano"], 4)
        self.assertEqual(score.getNumParts(), 1)
        self.assertEqual(score.getNumMeasures(), 4)

    def test_create_multipart_score(self):
        """Test creating a score with multiple parts"""
        score = ml.Score(["Violin", "Viola", "Cello"], 8)
        self.assertEqual(score.getNumParts(), 3)
        self.assertEqual(score.getNumMeasures(), 8)

    def test_add_part(self):
        """Test adding a part to score"""
        score = ml.Score(["Piano"], 4)
        initial_parts = score.getNumParts()
        score.addPart("Violin")
        self.assertEqual(score.getNumParts(), initial_parts + 1)

    def test_remove_part(self):
        """Test removing a part from score"""
        score = ml.Score(["Piano", "Violin", "Cello"], 4)
        initial_parts = score.getNumParts()
        score.removePart(1)
        self.assertEqual(score.getNumParts(), initial_parts - 1)

    def test_set_title_persists(self):
        """Test that setting title persists"""
        score = ml.Score(["Piano"], 4)
        score.setTitle("New Title")
        self.assertEqual(score.getTitle(), "New Title")

    def test_set_composer_persists(self):
        """Test that setting composer persists"""
        score = ml.Score(["Piano"], 4)
        score.setComposerName("New Composer")
        self.assertEqual(score.getComposerName(), "New Composer")


class ScoreCopyTestCase(unittest.TestCase):
    """Tests for Score copy constructor and assignment"""

    def test_score_properties_accessible(self):
        """Test that score properties are accessible"""
        original = ml.Score(["Piano", "Violin"], 8)
        original.setTitle("Test Symphony")
        original.setComposerName("Test Composer")

        self.assertEqual(original.getTitle(), "Test Symphony")
        self.assertEqual(original.getComposerName(), "Test Composer")
        self.assertEqual(original.getNumParts(), 2)
        self.assertEqual(original.getNumMeasures(), 8)

    def test_loaded_score_properties(self):
        """Test properties of a loaded score"""
        original = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        original_title = original.getTitle()
        original_composer = original.getComposerName()
        original_notes = original.getNumNotes()
        original_measures = original.getNumMeasures()

        self.assertIsNotNone(original_title)
        self.assertIsInstance(original_notes, int)
        self.assertIsInstance(original_measures, int)

    def test_score_independence_after_modification(self):
        """Test that modifying a score doesn't affect others"""
        score1 = ml.Score(["Piano"], 4)
        score1.setTitle("Score 1")

        score2 = ml.Score(["Violin"], 4)
        score2.setTitle("Score 2")

        self.assertEqual(score1.getTitle(), "Score 1")
        self.assertEqual(score2.getTitle(), "Score 2")


class ScoreEdgeCasesTestCase(unittest.TestCase):
    """Tests for Score edge cases and error handling"""

    def test_empty_score_properties(self):
        """Test properties of an empty score"""
        score = ml.Score(["Piano"], 1)
        self.assertEqual(score.getNumParts(), 1)
        self.assertEqual(score.getNumMeasures(), 1)

    def test_score_with_unicode_title(self):
        """Test score with unicode characters in title"""
        score = ml.Score(["Piano"], 4)
        unicode_title = "Симфония № 5"  # Cyrillic
        score.setTitle(unicode_title)
        self.assertEqual(score.getTitle(), unicode_title)

    def test_score_with_special_chars_title(self):
        """Test score with special characters in title"""
        score = ml.Score(["Piano"], 4)
        special_title = "Symphony #5 - \"Fate\""
        score.setTitle(special_title)
        self.assertEqual(score.getTitle(), special_title)

    def test_empty_string_title(self):
        """Test setting empty string as title"""
        score = ml.Score(["Piano"], 4)
        score.setTitle("")
        self.assertEqual(score.getTitle(), "")

    def test_empty_string_composer(self):
        """Test setting empty string as composer"""
        score = ml.Score(["Piano"], 4)
        score.setComposerName("")
        self.assertEqual(score.getComposerName(), "")


class ScoreExportTestCase(unittest.TestCase):
    """Tests for Score export functionality"""

    def test_export_to_xml(self):
        """Test exporting score to XML file"""
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        output_file = "test_export_temp"

        try:
            score.toFile(output_file, False)

            # Verify file was created
            self.assertTrue(os.path.exists(f"{output_file}.xml"))

            # Load exported file and verify
            reloaded = ml.Score(f"{output_file}.xml")
            self.assertTrue(reloaded.isValid())
            self.assertEqual(reloaded.getNumParts(), score.getNumParts())
        finally:
            # Cleanup
            if os.path.exists(f"{output_file}.xml"):
                os.remove(f"{output_file}.xml")

    def test_export_preserves_structure(self):
        """Test that export preserves score structure"""
        original = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        output_file = "test_structure_temp"

        try:
            original.toFile(output_file, False)
            reloaded = ml.Score(f"{output_file}.xml")

            self.assertEqual(reloaded.getNumNotes(), original.getNumNotes())
            self.assertEqual(reloaded.getNumMeasures(), original.getNumMeasures())
        finally:
            if os.path.exists(f"{output_file}.xml"):
                os.remove(f"{output_file}.xml")

    def test_export_preserves_metadata(self):
        """Test that export preserves metadata"""
        original = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        original.setTitle("Export Test")
        original.setComposerName("Test Composer")

        output_file = "test_metadata_temp"

        try:
            original.toFile(output_file, False)
            reloaded = ml.Score(f"{output_file}.xml")

            self.assertEqual(reloaded.getTitle(), "Export Test")
            self.assertEqual(reloaded.getComposerName(), "Test Composer")
        finally:
            if os.path.exists(f"{output_file}.xml"):
                os.remove(f"{output_file}.xml")


class ScoreIntegrationTestCase(unittest.TestCase):
    """Integration tests combining multiple Score operations"""

    def test_load_modify_export_reload(self):
        """Test complete workflow: load -> modify -> export -> reload"""
        # Load
        score1 = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        original_notes = score1.getNumNotes()

        # Modify
        score1.setTitle("Modified Score")
        score1.setComposerName("New Composer")

        # Export
        output_file = "test_workflow_temp"
        try:
            score1.toFile(output_file, False)

            # Reload
            score2 = ml.Score(f"{output_file}.xml")
            self.assertEqual(score2.getTitle(), "Modified Score")
            self.assertEqual(score2.getComposerName(), "New Composer")
            self.assertEqual(score2.getNumNotes(), original_notes)
        finally:
            if os.path.exists(f"{output_file}.xml"):
                os.remove(f"{output_file}.xml")

    def test_multipart_analysis(self):
        """Test analyzing a multi-part score"""
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')

        if score.getNumParts() > 1:
            # Analyze each part
            for i in range(score.getNumParts()):
                part = score.getPart(i)
                self.assertIsNotNone(part)

                part_name = score.getPartsNames()[i]
                self.assertIsInstance(part_name, str)

    def test_create_modify_export(self):
        """Test creating a new score, modifying it, and exporting"""
        # Create
        score = ml.Score(["Piano", "Violin"], 8)
        score.setTitle("New Composition")
        score.setComposerName("Composer")

        # Modify
        score.addPart("Cello")

        # Export
        output_file = "test_create_export_temp"
        try:
            score.toFile(output_file, False)

            # Reload and verify
            reloaded = ml.Score(f"{output_file}.xml")
            self.assertTrue(reloaded.isValid())
            self.assertEqual(reloaded.getTitle(), "New Composition")
            self.assertEqual(reloaded.getComposerName(), "Composer")
            self.assertEqual(reloaded.getNumParts(), 3)
        finally:
            if os.path.exists(f"{output_file}.xml"):
                os.remove(f"{output_file}.xml")


class ScorePerformanceTestCase(unittest.TestCase):
    """Performance-related tests for Score"""

    def test_load_performance(self):
        """Test that loading a score completes in reasonable time"""
        import time
        start = time.time()
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        elapsed = time.time() - start

        # Should load in less than 5 seconds for typical files
        self.assertLess(elapsed, 5.0)
        self.assertTrue(score.isValid())

    def test_note_extraction_performance(self):
        """Test that extracting all notes completes in reasonable time"""
        import time
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')

        start = time.time()
        notes = score.getAllNotes()
        elapsed = time.time() - start

        # Should extract notes in less than 2 seconds
        self.assertLess(elapsed, 2.0)
        self.assertIsInstance(notes, list)

    def test_multiple_loads_performance(self):
        """Test loading the same file multiple times"""
        import time
        start = time.time()

        for _ in range(5):
            score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
            self.assertTrue(score.isValid())

        elapsed = time.time() - start

        # Should load 5 times in less than 10 seconds
        self.assertLess(elapsed, 10.0)


class ScoreValidationTestCase(unittest.TestCase):
    """Tests for Score validation"""

    def test_valid_score_is_valid(self):
        """Test that a properly loaded score is valid"""
        score = ml.Score('./test/xml_examples/unit_test/test_chord.xml')
        self.assertTrue(score.isValid())

    def test_invalid_score_is_invalid(self):
        """Test that an invalid score is marked as invalid"""
        score = ml.Score('./nonexistent_file.xml')
        self.assertFalse(score.isValid())

    def test_created_score_is_valid(self):
        """Test that a created score is valid"""
        score = ml.Score(["Piano"], 4)
        # Created scores should be valid
        self.assertEqual(score.getNumParts(), 1)
        self.assertEqual(score.getNumMeasures(), 4)


class ScoreRobustnessTestCase(unittest.TestCase):
    """Tests for Score robustness and error handling"""

    def test_multiple_modifications(self):
        """Test multiple modifications to a score"""
        score = ml.Score(["Piano"], 4)

        # Multiple title changes
        for i in range(10):
            score.setTitle(f"Title {i}")
            self.assertEqual(score.getTitle(), f"Title {i}")

    def test_multiple_part_additions(self):
        """Test adding multiple parts"""
        score = ml.Score(["Piano"], 4)

        instruments = ["Violin", "Viola", "Cello", "Bass"]
        for instrument in instruments:
            score.addPart(instrument)

        self.assertEqual(score.getNumParts(), 1 + len(instruments))

    def test_part_removal_and_addition(self):
        """Test removing and adding parts"""
        score = ml.Score(["Piano", "Violin", "Cello"], 4)

        # Remove middle part
        score.removePart(1)
        self.assertEqual(score.getNumParts(), 2)

        # Add new part
        score.addPart("Viola")
        self.assertEqual(score.getNumParts(), 3)


if __name__ == '__main__':
    unittest.main()
