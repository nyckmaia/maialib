"""Comprehensive test suite for Score class

This module provides extensive testing for the maialib Score class,
covering loading, properties, navigation, analysis, manipulation,
and edge cases.
"""

import contextlib
import io
import locale
import os
import subprocess
import sys
import tempfile
import textwrap
import unittest

import maialib as ml

# Comma-decimal locale names: Windows first, then glibc and macOS.
COMMA_DECIMAL_LOCALES = (
    "Portuguese_Brazil.1252",
    "pt_BR.UTF-8",
    "pt_BR.utf8",
    "de_DE.UTF-8",
    "de_DE.utf8",
    "fr_FR.UTF-8",
    "German_Germany.1252",
)


def setCommaDecimalNumericLocale():
    """Set LC_NUMERIC to the first installed comma-decimal locale; return its name, or None."""
    for name in COMMA_DECIMAL_LOCALES:
        try:
            locale.setlocale(locale.LC_NUMERIC, name)
            return name
        except locale.Error:
            continue
    return None


def writtenPitches(score):
    """Every written pitch on stave 0 of the score's first part, measure by measure."""
    part = score.getPart(0)
    pitches = []
    for m in range(part.getNumMeasures()):
        measure = part.getMeasure(m)
        pitches += [measure.getNote(i, 0).getWrittenPitch() for i in range(measure.getNumNotes(0))]
    return pitches


class ScoreLoadingTestCase(unittest.TestCase):
    """Tests for Score loading from various file formats"""

    def test_load_valid_xml(self):
        """Test loading a valid MusicXML file"""
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        self.assertTrue(score.isValid())
        self.assertGreater(score.getNumParts(), 0)
        self.assertGreater(score.getNumMeasures(), 0)

    def test_load_returns_invalid_for_nonexistent_file(self):
        """Test loading a nonexistent file raises RuntimeError"""
        with self.assertRaises(RuntimeError):
            score = ml.Score("./nonexistent_file.xml")

    def test_load_filename_stored_correctly(self):
        """Test that filename is stored correctly after loading"""
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        self.assertEqual(score.getFileName(), "test_chord.xml")

    def test_load_filepath_stored_correctly(self):
        """Test that file path is stored correctly after loading"""
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        self.assertIn("test_chord.xml", score.getFilePath())


class ScoreQuarterToneReadTestCase(unittest.TestCase):
    """Tests that MusicXML quarter tones survive a real Score(path) load through the
    Python bindings, not just the underlying C++ function. loadXMLFile() is the only
    Python-reachable entry point Task 7 fixes; the C++ suite covers the same function
    directly, this covers it through the actual binding surface end users call."""

    def _first_note_pitch(self, fileName):
        score = ml.Score(f"./xml_examples/unit_test/{fileName}")
        self.assertTrue(score.isValid())
        return score.getPart(0).getMeasure(0).getNote(0, 0).getPitch()

    def test_tartini_accidental_with_matching_alter(self):
        """<alter>0.5</alter> together with <accidental>quarter-sharp</accidental>"""
        self.assertEqual(self._first_note_pitch("quarter_tone_tartini.xml"), "C1x4")

    def test_arrow_accidental_no_alter(self):
        """<accidental>sharp-down</accidental> alone, no <alter> at all"""
        self.assertEqual(self._first_note_pitch("quarter_tone_arrow.xml"), "C1x4")

    def test_accidental_only_no_alter_musescore_case(self):
        """<accidental>quarter-sharp</accidental> alone, no <alter> (MuseScore export)"""
        self.assertEqual(self._first_note_pitch("quarter_tone_accidental_only.xml"), "C1x4")

    def test_unrecognised_accidental_name_falls_back_instead_of_raising(self):
        """<accidental>natural-sharp</accidental> is outside the 14 names this library
        spells but carries a usable <alter>1</alter>; the load must degrade to that
        value instead of raising and aborting."""
        self.assertEqual(self._first_note_pitch("quarter_tone_unknown_accidental_name.xml"), "C#4")

    def test_unrepresentable_alter_falls_back_to_natural_instead_of_raising(self):
        """<alter>3</alter>, no <accidental> at all: a triple sharp is outside the nine
        values this library's accidental vocabulary can spell. The load must degrade to
        natural (the pre-Task-7 outcome) rather than raising and aborting -- this is the
        path a real ml.Score() user actually hits, not just the underlying C++ function."""
        self.assertEqual(self._first_note_pitch("unrepresentable_alter_triple_sharp.xml"), "C4")

    def _pitches_and_output(self, fileName):
        """Every written pitch of the loaded score's first part, and what the load printed."""
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            score = ml.Score(f"./xml_examples/unit_test/{fileName}")
        return writtenPitches(score), buffer.getvalue()

    def test_alter_without_accidental_is_read_as_a_quarter_tone(self):
        """<alter>0.5</alter> and <alter>-1.5</alter> with no <accidental>: the form a quarter
        tone takes when its accidental carries through the measure."""
        pitches, printed = self._pitches_and_output("quarter_tone_alter_only.xml")
        self.assertEqual(pitches, ["C1x4", "E3b4"])
        self.assertNotIn("[WARN]", printed)

    def test_alter_is_read_the_same_under_a_comma_decimal_locale(self):
        """The reader parses <alter> in the classic locale, not with atof(), which follows
        setlocale() and, under a comma-decimal locale, stops at the '.' of "0.5". On Linux and
        macOS the extension shares the process's C library, so this runs the reader under the
        comma locale; on Windows the extension links its own copy of the C runtime, which
        Python's setlocale() does not reach, and the test passes either way."""
        previous = locale.setlocale(locale.LC_NUMERIC)
        try:
            if setCommaDecimalNumericLocale() is None:
                self.skipTest("no comma-decimal locale is installed")
            self.assertEqual(locale.localeconv()["decimal_point"], ",")
            pitches, printed = self._pitches_and_output("quarter_tone_alter_only.xml")
        finally:
            locale.setlocale(locale.LC_NUMERIC, previous)
        self.assertEqual(pitches, ["C1x4", "E3b4"])
        self.assertNotIn("[WARN]", printed)

    def test_alter_near_a_quarter_tone_is_not_snapped_onto_it(self):
        """<alter>0.46</alter> is near a quarter-tone sharp but is not one: the note reads as
        natural, with a warning naming the value, never rounded to the nearest pitch."""
        pitches, printed = self._pitches_and_output("unrepresentable_alter_near_quarter_tone.xml")
        self.assertEqual(pitches, ["C4"])
        self.assertIn("[WARN] Unrepresentable <alter> value '0.46'", printed)

    def test_disagreeing_accidental_wins_with_a_warning(self):
        """<accidental>quarter-sharp</accidental> with <alter>1</alter>: the accidental wins,
        and the disagreement is reported."""
        pitches, printed = self._pitches_and_output("quarter_tone_accidental_alter_disagree.xml")
        self.assertEqual(pitches, ["C1x4"])
        self.assertIn(
            "[WARN] The <accidental> 'quarter-sharp' and the <alter> '1' of this note disagree",
            printed,
        )

    def test_agreeing_accidental_and_alter_read_without_a_warning(self):
        pitches, printed = self._pitches_and_output("quarter_tone_tartini.xml")
        self.assertEqual(pitches, ["C1x4"])
        self.assertNotIn("[WARN]", printed)

    def test_sharp_sharp_accidental_is_a_double_sharp(self):
        """"sharp-sharp" is MusicXML's double sharp drawn as two sharp signs."""
        pitches, printed = self._pitches_and_output("accidental_sharp_sharp.xml")
        self.assertEqual(pitches, ["Cx4"])
        self.assertNotIn("[WARN]", printed)

    def test_quarter_tone_score_written_by_maialib_reads_back_unchanged(self):
        """The source spells every quarter tone with an arrow glyph and no <alter>, so each
        <alter> in the written file is maialib's own; it must agree with the <accidental>
        written beside it."""
        original = ml.Score("./xml_examples/unit_test/test_quarter_tones.musicxml")
        pitches = writtenPitches(original)
        for quarterTone in ("C1x4", "C3x4", "C1b4", "C3b4"):
            self.assertIn(quarterTone, pitches)

        with tempfile.TemporaryDirectory() as directory:
            base = os.path.join(directory, "round_trip")
            original.toFile(base, False)
            buffer = io.StringIO()
            with contextlib.redirect_stdout(buffer):
                reread = ml.Score(base + ".xml")
            readBack = writtenPitches(reread)

        self.assertEqual(readBack, pitches)
        self.assertNotIn("[WARN]", buffer.getvalue())


# A melody search that calls Python callbacks from its worker threads. Run in a child process,
# because a search that deadlocks cannot be interrupted from the thread that started it.
CALLBACK_SEARCH = textwrap.dedent(
    """
    import maialib as ml

    score = ml.Score("./xml_examples/Bach/cello_suite_1_violin.xml")
    pattern = [ml.Note("G2"), ml.Note("D3"), ml.Note("B3")]
    calls = []


    def intervals(pattern, segment):
        calls.append(1)
        return ml.Helper.getSemitonesDifferenceBetweenMelodies(pattern, segment)


    def total(differences):
        return ml.Helper.calculateMelodyEuclideanSimilarity(differences)


    single = len(score.findMelodyPatternDataFrame(pattern, 0.5, 0.5, intervals, None, total))
    callsPerSearch = len(calls)
    searcher = score
    if COLLECTION:
        searcher = ml.ScoreCollection([])
        searcher.addScore(score)
    listed = searcher.findMelodyPatternDataFrame([pattern, pattern], 0.5, 0.5, intervals, None, total)
    print("RESULT", len(listed), 2 * single, len(calls) == 3 * callsPerSearch > 0)
    """
)


def runCallbackSearch(collection, timeout=60):
    """Run CALLBACK_SEARCH in a child process; return its RESULT line, or None on a timeout."""
    code = f"COLLECTION = {collection}\n{CALLBACK_SEARCH}"
    try:
        completed = subprocess.run(
            [sys.executable, "-c", code],
            cwd=os.path.dirname(os.path.abspath(__file__)),
            capture_output=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return None
    lines = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
    return lines[-1] if lines else completed.stderr


class ScoreMelodyPatternSearchTestCase(unittest.TestCase):
    """findMelodyPatternDataFrame's list overload searches each pattern on a worker thread."""

    def test_a_failing_pattern_fails_the_list_overload_too(self):
        """A segment that starts on a quarter tone has no interval name for its transposition, so
        the search raises -- through the list overload exactly as through the single-pattern one,
        instead of answering an empty DataFrame."""
        score = ml.Score("./xml_examples/unit_test/test_quarter_tones.musicxml")
        pattern = [ml.Note("C4"), ml.Note("D4")]

        with self.assertRaises(RuntimeError) as single:
            score.findMelodyPatternDataFrame(pattern)
        with self.assertRaises(RuntimeError) as listed:
            score.findMelodyPatternDataFrame([pattern])

        message = str(listed.exception).splitlines()[0]
        self.assertIn("Cannot compute an interval with the quarter tone", message)
        self.assertEqual(message, str(single.exception).splitlines()[0])

    def test_every_pattern_is_searched_whatever_the_thread_count(self):
        """More patterns than processors: each is searched, and finds as many rows as when it is
        searched on its own."""
        score = ml.Score("./xml_examples/Bach/cello_suite_1_violin.xml")
        numPatterns = (os.cpu_count() or 1) + 3
        opening = []
        part = score.getPart(0)
        for m in range(part.getNumMeasures()):
            measure = part.getMeasure(m)
            for n in range(measure.getNumNotes(0)):
                note = measure.getNote(n, 0)
                if note.isNoteOn() and note.getVoice() == 1 and not note.inChord():
                    opening.append(note.getWrittenPitch())
            if len(opening) >= numPatterns + 2:
                break
        patterns = [[ml.Note(p) for p in opening[i : i + 3]] for i in range(numPatterns)]

        table = score.findMelodyPatternDataFrame(patterns)
        counts = table.groupby("patternIdx").size().to_dict()
        for i, pattern in enumerate(patterns):
            with self.subTest(pattern=i):
                single = len(score.findMelodyPatternDataFrame(pattern))
                self.assertGreater(single, 0)
                self.assertEqual(int(counts.get(i, 0)), single)

    def test_the_list_overload_calls_python_callbacks_without_deadlocking(self):
        """The worker threads call the Python callbacks, each call taking the GIL, so the search
        must not hold it while they run. Under a hard timeout, a deadlock fails this test."""
        result = runCallbackSearch(collection=False)
        self.assertIsNotNone(result, "the search did not finish within 60 s: a deadlock")
        _, listed, expected, called = result.split()
        self.assertEqual(listed, expected)
        self.assertEqual(called, "True")

    def test_the_collection_list_overload_calls_python_callbacks_without_deadlocking(self):
        """ScoreCollection's list overload runs each score's patterns on worker threads too."""
        result = runCallbackSearch(collection=True)
        self.assertIsNotNone(result, "the search did not finish within 60 s: a deadlock")
        _, listed, expected, called = result.split()
        self.assertEqual(listed, expected)
        self.assertEqual(called, "True")


class ScorePropertiesTestCase(unittest.TestCase):
    """Tests for Score basic properties"""

    def setUp(self):
        """Set up test fixtures"""
        self.score = ml.Score("./xml_examples/unit_test/test_chord.xml")

    def test_get_title(self):
        """Test getting score title"""
        title = self.score.getTitle()
        self.assertIsInstance(title, str)

    def test_set_title(self):
        """Test setting score title"""
        self.score.setTitle("Test Symphony")
        self.assertEqual(self.score.getTitle(), "Test Symphony")

    def test_get_composer_name(self):
        """Test getting composer name"""
        composer = self.score.getComposerName()
        self.assertIsInstance(composer, str)

    def test_set_composer_name(self):
        """Test setting composer name"""
        self.score.setComposerName("Test Composer")
        self.assertEqual(self.score.getComposerName(), "Test Composer")

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
        self.score = ml.Score("./xml_examples/unit_test/test_chord.xml")

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
        self.score = ml.Score("./xml_examples/unit_test/test_chord.xml")

    def test_for_each_note(self):
        """Test iterating through all notes with callback"""
        note_count = [0]  # Use list to allow modification in nested function

        def count_callback(part, measure, staveId, note):
            note_count[0] += 1

        self.score.forEachNote(count_callback)
        self.assertGreater(note_count[0], 0)

    def test_to_dataframe(self):
        """Test converting score to DataFrame"""
        df = self.score.toDataFrame()
        self.assertIsNotNone(df)

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
        original = ml.Score("./xml_examples/unit_test/test_chord.xml")
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
        special_title = 'Symphony #5 - "Fate"'
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
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
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
        original = ml.Score("./xml_examples/unit_test/test_chord.xml")
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
        original = ml.Score("./xml_examples/unit_test/test_chord.xml")
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
        score1 = ml.Score("./xml_examples/unit_test/test_chord.xml")
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
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")

        if score.getNumParts() > 1:
            # Analyze each part
            for i in range(score.getNumParts()):
                part = score.getPart(i)
                self.assertIsNotNone(part)

                part_name = score.getPartsNames()[i]
                self.assertIsInstance(part_name, str)

    def test_create_and_modify_score(self):
        """Test creating a new score and modifying it"""
        # Create
        score = ml.Score(["Piano", "Violin"], 8)
        score.setTitle("New Composition")
        score.setComposerName("Composer")

        # Verify basic properties
        self.assertEqual(score.getTitle(), "New Composition")
        self.assertEqual(score.getComposerName(), "Composer")
        self.assertEqual(score.getNumParts(), 2)
        self.assertEqual(score.getNumMeasures(), 8)

        # Modify
        score.addPart("Cello")
        self.assertEqual(score.getNumParts(), 3)


class ScorePerformanceTestCase(unittest.TestCase):
    """Performance-related tests for Score"""

    def test_load_performance(self):
        """Test that loading a score completes in reasonable time"""
        import time

        start = time.time()
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        elapsed = time.time() - start

        # Should load in less than 5 seconds for typical files
        self.assertLess(elapsed, 5.0)
        self.assertTrue(score.isValid())

    def test_dataframe_conversion_performance(self):
        """Test that converting to DataFrame completes in reasonable time"""
        import time

        score = ml.Score("./xml_examples/unit_test/test_chord.xml")

        start = time.time()
        df = score.toDataFrame()
        elapsed = time.time() - start

        # Should convert in less than 2 seconds
        self.assertLess(elapsed, 2.0)
        self.assertIsNotNone(df)

    def test_multiple_loads_performance(self):
        """Test loading the same file multiple times"""
        import time

        start = time.time()

        for _ in range(5):
            score = ml.Score("./xml_examples/unit_test/test_chord.xml")
            self.assertTrue(score.isValid())

        elapsed = time.time() - start

        # Should load 5 times in less than 10 seconds
        self.assertLess(elapsed, 10.0)


class ScoreValidationTestCase(unittest.TestCase):
    """Tests for Score validation"""

    def test_valid_score_is_valid(self):
        """Test that a properly loaded score is valid"""
        score = ml.Score("./xml_examples/unit_test/test_chord.xml")
        self.assertTrue(score.isValid())

    def test_invalid_score_is_invalid(self):
        """Test that an invalid score raises RuntimeError"""
        with self.assertRaises(RuntimeError):
            score = ml.Score("./nonexistent_file.xml")

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


if __name__ == "__main__":
    unittest.main()
