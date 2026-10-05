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
    Python bindings, not just the underlying C++ function: loadXMLFile() is the
    Python-reachable entry point of the MusicXML reader. The C++ suite covers the same
    function directly; this covers it through the actual binding surface end users call."""

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
        natural, with a warning, rather than raising and aborting -- this is the path a
        real ml.Score() user actually hits, not just the underlying C++ function."""
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


def runChild(code, timeout=60):
    """Run the Python source 'code' in a child process, from this directory, under a hard timeout.
    Return the completed process, or None when the timeout expired."""
    try:
        return subprocess.run(
            [sys.executable, "-c", code],
            cwd=os.path.dirname(os.path.abspath(__file__)),
            capture_output=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return None


def resultLine(completed):
    """The last line the child printed that starts with RESULT, or its standard error."""
    lines = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
    return lines[-1] if lines else completed.stderr


def runCallbackSearch(collection, timeout=60):
    """Run CALLBACK_SEARCH in a child process; return its RESULT line, or None on a timeout."""
    completed = runChild(f"COLLECTION = {collection}\n{CALLBACK_SEARCH}", timeout)
    return None if completed is None else resultLine(completed)


# A melody search whose Python callback raises, called from the search's worker threads. Run in a
# child process, because an exception mishandled on a worker thread would kill the interpreter or
# hang it.
RAISING_CALLBACK_SEARCH = textwrap.dedent(
    """
    import maialib as ml


    class CallbackError(Exception):
        pass


    def intervals(pattern, segment):
        raise CallbackError("raised by the callback", len(segment))


    def total(differences):
        return 1.0


    score = ml.Score("./xml_examples/Bach/cello_suite_1_violin.xml")
    searcher = score
    if COLLECTION:
        searcher = ml.ScoreCollection([])
        searcher.addScore(score)
    pattern = [ml.Note("G2"), ml.Note("D3"), ml.Note("B3")]
    try:
        searcher.findMelodyPatternDataFrame([pattern, pattern], 0.5, 0.5, intervals, None, total)
        print("RESULT nothing-raised")
    except Exception as error:
        usable = len(score.findMelodyPatternDataFrame([pattern])) > 0
        print("RESULT", type(error).__name__, error.args == ("raised by the callback", 3), usable)
    """
)


def runRaisingCallbackSearch(collection, timeout=60):
    """Run RAISING_CALLBACK_SEARCH in a child process; return its RESULT line, or None on a
    timeout."""
    completed = runChild(f"COLLECTION = {collection}\n{RAISING_CALLBACK_SEARCH}", timeout)
    return None if completed is None else resultLine(completed)


# Several threads search one freshly loaded score at once, round after round, and every thread's
# table is compared with a serial search's. The list overload releases the GIL, so the searches
# really do overlap. Run in a child process, because a search that corrupted the heap would kill
# the interpreter running it.
CONCURRENT_SEARCH = textwrap.dedent(
    """
    import threading

    import maialib as ml

    path = ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)
    patterns = [[ml.Note("C4"), ml.Note("D4")], [ml.Note("G3"), ml.Note("D4"), ml.Note("B4")]]
    expected = ml.Score(path).findMelodyPatternDataFrame(patterns)
    numThreads = 4


    def search(score, barrier, tables, i):
        barrier.wait()
        tables[i] = score.findMelodyPatternDataFrame(patterns)


    differing = 0
    for _ in range(ROUNDS):
        score = ml.Score(path)
        barrier = threading.Barrier(numThreads)
        tables = [None] * numThreads
        threads = [
            threading.Thread(target=search, args=(score, barrier, tables, i))
            for i in range(numThreads)
        ]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        differing += sum(table is None or not table.equals(expected) for table in tables)
    print("RESULT", len(expected), differing)
    """
)


class ScoreMelodyPatternSearchTestCase(unittest.TestCase):
    """findMelodyPatternDataFrame's list overload searches each pattern on a worker thread."""

    def test_a_failing_pattern_fails_the_list_overload_too(self):
        """A pattern of one note has no melodic interval, so its search raises -- through the
        list overload exactly as through the single-pattern one, instead of answering an empty
        DataFrame."""
        score = ml.Score("./xml_examples/unit_test/test_quarter_tones.musicxml")
        pattern = [ml.Note("C4")]

        with self.assertRaises(RuntimeError) as single:
            score.findMelodyPatternDataFrame(pattern)
        with self.assertRaises(RuntimeError) as listed:
            score.findMelodyPatternDataFrame([[ml.Note("C4"), ml.Note("D4")], pattern])

        message = str(listed.exception).splitlines()[0]
        self.assertEqual(
            message,
            "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, and "
            "this one has 1",
        )
        self.assertEqual(message, str(single.exception).splitlines()[0])

    def test_a_quarter_tone_does_not_stop_the_search(self):
        """A window that starts on a quarter tone is compared exactly; its transposition has no
        name."""
        score = ml.Score("./xml_examples/unit_test/test_quarter_tones.musicxml")
        table = score.findMelodyPatternDataFrame([ml.Note("C4"), ml.Note("D4")], 0.0, 0.0)
        starts = table["writtenPitches"].map(lambda pitches: pitches[0] == "C1x4")
        self.assertGreater(int(starts.sum()), 0)
        self.assertEqual(set(table[starts]["transposeInterval"]), {""})
        self.assertEqual(set(table[starts]["transposeSemitones"]), {0.5})

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

    def test_a_callback_that_raises_on_a_worker_thread_raises_to_the_caller(self):
        """A Python callback's exception, raised on a worker thread, reaches the caller as itself
        -- the same type and arguments, not a RuntimeError describing it -- once every pattern
        has been searched, and the interpreter stays usable."""
        result = runRaisingCallbackSearch(collection=False)
        self.assertIsNotNone(result, "the search did not finish within 60 s: a deadlock")
        self.assertEqual(result, "RESULT CallbackError True True")

    def test_a_callback_that_raises_raises_through_the_collection_list_overload_too(self):
        """ScoreCollection's list overload propagates it the same way."""
        result = runRaisingCallbackSearch(collection=True)
        self.assertIsNotNone(result, "the search did not finish within 60 s: a deadlock")
        self.assertEqual(result, "RESULT CallbackError True True")

    def test_several_threads_can_search_one_score_at_once(self):
        """The list overload releases the GIL while it searches, so searches of one score from
        several Python threads run at the same time. A search only reads the score, so each finds
        exactly what a serial search finds, and the interpreter survives, however the threads
        interleave: four threads, released together by a barrier, search a freshly loaded score
        in each of ten rounds."""
        completed = runChild(CONCURRENT_SEARCH.replace("ROUNDS", "10"), timeout=120)
        self.assertIsNotNone(completed, "the searches did not finish within 120 s")
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        _, rows, differing = resultLine(completed).split()
        self.assertGreater(int(rows), 0)
        self.assertEqual(differing, "0")

    def test_a_pattern_longer_than_every_melody_finds_no_match(self):
        """A pattern longer than every melodic line finds nothing, however many notes the score
        has: voice 1 has one event and voice 2 two."""
        score = ml.Score(["Flute"], 1)
        measure = score.getPart(0).getMeasure(0)
        measure.addNote(ml.Note("C4"))
        for pitch in ("D4", "E4"):
            note = ml.Note(pitch)
            note.setVoice(2)
            measure.addNote(note)
        pattern = [ml.Note("C4"), ml.Note("D4"), ml.Note("E4")]
        self.assertEqual(score.getNumNotes(), 3)

        self.assertEqual(len(score.findMelodyPatternDataFrame(pattern)), 0)
        self.assertEqual(len(score.findMelodyPatternDataFrame([pattern])), 0)


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


class ScoreConcertKeyTestCase(unittest.TestCase):
    """getChordsDataFrame's key column is the concert key."""

    def test_the_key_column_is_the_concert_key(self):
        """W3C 72a's trumpet in B-flat is written in D major and its horn in E-flat in A major;
        its untransposed piano gives the concert key, C major."""
        score = ml.Score("./musicxml/w3c-test-suite/xmlFiles/72a-TransposingInstruments.musicxml")
        table = score.getChordsDataFrame()
        keys = {(key.getFifthCircle(), bool(key.isMajorMode())) for key in table["key"]}
        self.assertEqual({(0, True)}, keys)


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


MATCH_COLUMNS = [
    "partName",
    "measure",
    "staff",
    "voice",
    "writtenKey",
    "concertKey",
    "transposeInterval",
    "transposeSemitones",
    "writtenPitches",
    "soundingPitches",
    "semitonesDiff",
    "rhythmDiff",
    "intervalSimilarity",
    "rhythmSimilarity",
    "totalSimilarity",
]


def quarters(*pitches):
    return [ml.Note(pitch) for pitch in pitches]


class ScoreMelodyPatternDataFrameTestCase(unittest.TestCase):
    """The DataFrames of the melody search: columns, dtypes, order and the new rules."""

    def test_the_columns_of_a_match(self):
        score = ml.Score("./xml_examples/unit_test/melody_transposing_instrument.musicxml")
        table = score.findMelodyPatternDataFrame(quarters("C4", "D4", "E4"), 1.0, 1.0)
        self.assertEqual(list(table.columns), MATCH_COLUMNS)
        row = table.iloc[0].to_dict()
        self.assertEqual(
            {key: row[key] for key in MATCH_COLUMNS[:8]},
            {
                "partName": "Clarinet in Bb",
                "measure": 0,
                "staff": 0,
                "voice": 1,
                "writtenKey": "D",
                "concertKey": "C",
                "transposeInterval": "P1",
                "transposeSemitones": 0.0,
            },
        )
        self.assertEqual(list(row["writtenPitches"]), ["D4", "E4", "F#4"])
        self.assertEqual(list(row["soundingPitches"]), ["C4", "D4", "E4"])
        self.assertEqual(len(table), 1)

    def test_an_empty_result_has_every_column_and_dtype(self):
        score = ml.Score("./xml_examples/unit_test/melody_last_window.musicxml")
        matched = score.findMelodyPatternDataFrame(quarters("C4", "D4"), 1.0, 1.0)
        empty = score.findMelodyPatternDataFrame(quarters("C4", "C6"), 1.0, 1.0)
        self.assertEqual(len(empty), 0)
        self.assertEqual(list(empty.dtypes.items()), list(matched.dtypes.items()))
        for column in ("measure", "staff", "voice"):
            self.assertEqual(str(matched[column].dtype), "int64", column)
        for column in ("transposeSemitones", "intervalSimilarity", "totalSimilarity"):
            self.assertEqual(str(matched[column].dtype), "float64", column)

        listed = score.findMelodyPatternDataFrame([quarters("C4", "D4")], 1.0, 1.0)
        empty_list = score.findMelodyPatternDataFrame([quarters("C4", "C6")], 1.0, 1.0)
        self.assertEqual(list(listed.columns), ["patternIdx"] + MATCH_COLUMNS)
        self.assertEqual(list(empty_list.dtypes.items()), list(listed.dtypes.items()))

    def test_every_voice_of_every_staff_is_searched_and_rows_sort_by_measure(self):
        score = ml.Score("./xml_examples/unit_test/melody_staves_and_voices.musicxml")
        table = score.findMelodyPatternDataFrame(quarters("C4", "D4", "E4", "F4"), 1.0, 1.0)
        self.assertEqual(
            list(zip(table["measure"], table["staff"], table["voice"])),
            [(0, 0, 1), (0, 1, 5), (1, 1, 5)],
        )

    def test_the_list_overload_sorts_by_pattern_then_measure(self):
        score = ml.Score("./xml_examples/unit_test/melody_staves_and_voices.musicxml")
        table = score.findMelodyPatternDataFrame(
            [quarters("C4", "D4", "E4", "F4"), quarters("C5", "B4")], 1.0, 1.0
        )
        self.assertEqual(
            list(zip(table["patternIdx"], table["measure"], table["staff"])),
            [(0, 0, 0), (0, 0, 1), (0, 1, 1), (1, 0, 0)],
        )

    def test_a_transposition_without_a_name_does_not_stop_the_search(self):
        score = ml.Score("./xml_examples/unit_test/melody_unnameable_transposition.musicxml")
        table = score.findMelodyPatternDataFrame(quarters("C4", "D4", "E4"), 1.0, 1.0)
        self.assertEqual(list(table["transposeInterval"]), ["", ""])
        self.assertEqual(list(table["transposeSemitones"]), [14.0, 0.5])

    def test_a_pattern_of_fewer_than_two_notes_raises(self):
        score = ml.Score("./xml_examples/unit_test/melody_last_window.musicxml")
        for pattern in ([], quarters("C4")):
            with self.subTest(length=len(pattern)), self.assertRaises(RuntimeError) as context:
                score.findMelodyPatternDataFrame(pattern)
            self.assertEqual(
                str(context.exception).splitlines()[0],
                "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, "
                f"and this one has {len(pattern)}",
            )

    def test_the_thresholds_have_the_same_names_in_both_overloads(self):
        score = ml.Score("./xml_examples/unit_test/melody_last_window.musicxml")
        single = score.findMelodyPatternDataFrame(
            quarters("C4", "D4"), intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0
        )
        listed = score.findMelodyPatternDataFrame(
            [quarters("C4", "D4")], intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0
        )
        self.assertEqual((len(single), len(listed)), (2, 2))
        with self.assertRaises(TypeError):
            score.findMelodyPatternDataFrame(
                quarters("C4", "D4"), totalIntervalsSimilarityThreshold=1.0
            )


if __name__ == "__main__":
    unittest.main()
