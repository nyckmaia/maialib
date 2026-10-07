"""ScoreCollection: construction, directory discovery, edits and the melody search's DataFrames."""

import contextlib
import getpass
import io
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

import maialib as ml

HERE = os.path.dirname(os.path.abspath(__file__))
UNIT_TEST = os.path.join(HERE, "xml_examples", "unit_test")
LAST_WINDOW = os.path.join(UNIT_TEST, "melody_last_window.musicxml")
DUPLICATES = os.path.join(UNIT_TEST, "melody_duplicate_patterns.musicxml")
BACH = os.path.join(HERE, "xml_examples", "Bach")
BEETHOVEN = os.path.join(HERE, "xml_examples", "Beethoven")

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
SCORE_COLUMNS = ["fileName", "composerName", "scoreTitle"]


def fileNames(collection):
    return [score.getFileName() for score in collection.getScores()]


def runChild(code, environment=None):
    """Run the Python source 'code' in a child process, from this directory, so that a crash of
    the interpreter shows as the child's exit code instead of ending the test run; 'environment'
    adds variables to the child's environment."""
    return subprocess.run(
        [sys.executable, "-c", code],
        cwd=HERE,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        timeout=120,
        env={**os.environ, **(environment or {})},
    )


@contextlib.contextmanager
def unreadable(path):
    """Deny the current user the listing of the directory 'path' while the block runs, then
    restore its permissions. Skips the test when the platform does not deny it."""
    if os.name == "nt":
        user = getpass.getuser()
        denied = subprocess.run(["icacls", path, "/deny", f"{user}:(RD)"], capture_output=True)
        if denied.returncode != 0:
            raise unittest.SkipTest(f"icacls cannot deny {user} the listing of a directory")

        def restore():
            subprocess.run(["icacls", path, "/remove:d", user], capture_output=True, check=True)
    else:
        mode = os.stat(path).st_mode
        os.chmod(path, 0)

        def restore():
            os.chmod(path, mode)

    try:
        try:
            os.listdir(path)
        except PermissionError:
            pass
        else:
            raise unittest.SkipTest("this user can list a directory it has no permission to list")
        yield
    finally:
        restore()


def twoScores():
    """A collection of the two small fixtures, the last-window one titled B, the other A."""
    collection = ml.ScoreCollection()
    collection.addScore(LAST_WINDOW)
    collection.addScore(DUPLICATES)
    collection.getScores()[0].setTitle("B")
    collection.getScores()[1].setTitle("A")
    return collection


class ScoreCollectionConstructionTestCase(unittest.TestCase):
    def test_the_default_constructor_builds_an_empty_collection(self):
        collection = ml.ScoreCollection()
        self.assertEqual(collection.getNumScores(), 0)
        self.assertEqual(collection.getNumDirectories(), 0)

    def test_a_path_that_is_not_a_directory_raises_runtime_error_naming_it(self):
        for path in ("no-such-directory", LAST_WINDOW, ""):
            with self.subTest(path=path), self.assertRaises(RuntimeError) as context:
                ml.ScoreCollection(path)
            self.assertEqual(
                str(context.exception).splitlines()[0],
                f"[maiacore] ScoreCollection: '{path}' is not a directory, or does not exist",
            )

    def test_discovery_ignores_case_sorts_and_recurses_on_request(self):
        with tempfile.TemporaryDirectory() as directory:
            for name in ("c.MusicXML", "a.xml", "B.XML", "notes.txt", os.path.join("sub", "d.xml")):
                os.makedirs(os.path.dirname(os.path.join(directory, name)), exist_ok=True)
                shutil.copyfile(LAST_WINDOW, os.path.join(directory, name))

            self.assertEqual(
                fileNames(ml.ScoreCollection(directory)), ["B.XML", "a.xml", "c.MusicXML"]
            )
            self.assertEqual(
                fileNames(ml.ScoreCollection(directory, recursive=True)),
                ["B.XML", "a.xml", "c.MusicXML", "d.xml"],
            )
            collection = ml.ScoreCollection()
            collection.setDirectoriesPaths([directory], recursive=True)
            self.assertEqual(fileNames(collection), ["B.XML", "a.xml", "c.MusicXML", "d.xml"])

    def test_a_directory_named_like_a_score_and_a_non_ascii_name_are_skipped(self):
        with tempfile.TemporaryDirectory() as directory:
            os.makedirs(os.path.join(directory, "old.xml"))
            with open(os.path.join(directory, "notes.ωδή"), "w", encoding="utf-8") as text:
                text.write("not a score")
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))

            self.assertEqual(fileNames(ml.ScoreCollection(directory, recursive=True)), ["a.xml"])

    def test_an_unreadable_subdirectory_is_skipped_and_an_unreadable_directory_raises(self):
        with tempfile.TemporaryDirectory() as directory:
            locked = os.path.join(directory, "locked")
            os.makedirs(locked)
            shutil.copyfile(LAST_WINDOW, os.path.join(locked, "b.xml"))
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))

            with unreadable(locked):
                self.assertEqual(
                    fileNames(ml.ScoreCollection(directory, recursive=True)), ["a.xml"]
                )
                for flag in (False, True):
                    with self.subTest(recursive=flag), self.assertRaises(RuntimeError) as raised:
                        ml.ScoreCollection(locked, flag)
                    self.assertEqual(
                        str(raised.exception).splitlines()[0],
                        f"[maiacore] ScoreCollection: cannot read the directory '{locked}'",
                    )

    @unittest.skipUnless(os.name == "nt", "only Windows fails to open a name ending in a dot")
    def test_a_subdirectory_that_cannot_be_read_is_named_in_the_error(self):
        """Windows strips the trailing dot of 'sub.' when the directory is opened by its plain
        path, so listing it fails, with an error other than a denied permission."""
        with tempfile.TemporaryDirectory() as directory:
            subdirectory = os.path.join(directory, "sub.")
            os.makedirs("\\\\?\\" + subdirectory)
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))
            try:
                with self.assertRaises(RuntimeError) as context:
                    ml.ScoreCollection(directory, recursive=True)
                self.assertEqual(
                    str(context.exception).splitlines()[0],
                    f"[maiacore] ScoreCollection: cannot read the directory '{subdirectory}'",
                )
            finally:
                os.rmdir("\\\\?\\" + subdirectory)

    def test_a_file_that_fails_to_load_is_skipped_and_listed(self):
        with tempfile.TemporaryDirectory() as directory:
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))
            broken = os.path.join(directory, "b.xml")
            with open(broken, "w", encoding="utf-8") as text:
                text.write("not a score")
            collection = ml.ScoreCollection()
            collection.addScore(DUPLICATES)

            printed = io.StringIO()
            with contextlib.redirect_stdout(printed):
                collection.setDirectoriesPaths([directory])
            self.assertEqual(fileNames(collection), ["a.xml"])
            self.assertEqual(collection.getDirectoriesPaths(), [directory])
            self.assertEqual(
                collection.getLoadErrors(),
                [
                    (
                        broken,
                        f"[maiacore] Score: '{broken}' is not well-formed XML: No document "
                        "element found (byte offset 11)",
                    )
                ],
            )
            self.assertIn(
                "[maiacore] ScoreCollection: 1 of 2 files failed to load; see "
                "ScoreCollection.getLoadErrors()",
                printed.getvalue(),
            )

    def test_add_score_by_path_lists_a_failure_and_prints_to_sys_stdout(self):
        """addScore prints to sys.stdout, which redirect_stdout catches."""
        collection = ml.ScoreCollection()
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            collection.addScore("./missing.xml")
            collection.addScore([LAST_WINDOW, "./missing.xml"])
        self.assertEqual(fileNames(collection), ["melody_last_window.musicxml"])
        self.assertEqual(
            collection.getLoadErrors(),
            [("./missing.xml", "[maiacore] Score: cannot open './missing.xml'")],
        )
        self.assertEqual(
            printed.getvalue().count("files failed to load; see ScoreCollection.getLoadErrors()"),
            2,
        )

    def test_a_non_ascii_directory_lists_its_failures_with_utf8_paths(self):
        """Run in a child process whose standard output is ASCII: a directory 'músicas' holding
        '日本.xml', which loads, and 'ruim.xml', which does not."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            "    folder = os.path.join(directory, 'm\\u00fasicas')\n"
            "    os.makedirs(folder)\n"
            f"    shutil.copyfile({LAST_WINDOW!r}, os.path.join(folder, '\\u65e5\\u672c.xml'))\n"
            "    with open(os.path.join(folder, 'ruim.xml'), 'w') as text:\n"
            "        text.write('not a score')\n"
            "    collection = ml.ScoreCollection(folder)\n"
            "    paths = [path for path, _ in collection.getLoadErrors()]\n"
            "    names = [score.getFileName() for score in collection.getScores()]\n"
            "    print('RESULT', ascii(names), paths == [os.path.join(folder, 'ruim.xml')])\n"
        )
        completed = runChild(code, {"PYTHONIOENCODING": "ascii"})
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, ["RESULT ['\\u65e5\\u672c.xml'] True"])

    def test_set_directories_paths_replaces_the_scores(self):
        collection = ml.ScoreCollection(BACH)
        collection.addScore(LAST_WINDOW)
        collection.setDirectoriesPaths([BEETHOVEN])
        collection.setDirectoriesPaths([BEETHOVEN])
        self.assertEqual(
            fileNames(collection),
            ["Beethoven_quartet_133.xml", "Beethoven_quartet_Op133.xml", "Symphony_5th_1Mov.xml"],
        )

    def test_a_failed_reload_keeps_the_scores_and_the_load_errors(self):
        collection = ml.ScoreCollection()
        with contextlib.redirect_stdout(io.StringIO()):
            collection.addScore([LAST_WINDOW, "./missing.xml"])
        errors = collection.getLoadErrors()
        self.assertEqual(len(errors), 1)

        with self.assertRaises(RuntimeError):
            collection.setDirectoriesPaths([BACH, "no-such-directory"])
        self.assertEqual(fileNames(collection), ["melody_last_window.musicxml"])
        self.assertEqual(collection.getNumDirectories(), 0)
        self.assertEqual(collection.getLoadErrors(), errors)

    def test_remove_score_outside_the_collection_raises_index_error(self):
        """Run in a child process: an unchecked negative index crashes the interpreter."""
        code = (
            "import maialib as ml\n"
            "c = ml.ScoreCollection()\n"
            f"c.addScore({LAST_WINDOW!r})\n"
            "for i in (-1, 1, 5):\n"
            "    try:\n"
            "        c.removeScore(i)\n"
            "        print('RESULT nothing-raised', i)\n"
            "    except IndexError:\n"
            "        pass\n"
            "print('RESULT', c.getNumScores())\n"
        )
        completed = runChild(code)
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, ["RESULT 1"])

    def test_non_ascii_file_names_load_with_their_utf8_names(self):
        """Run in a child process whose standard output is ASCII: 'canção.xml' and '日本.xml'
        load, keep their names as UTF-8, and name the search's rows; the "Loading:" lines that
        name them are written with backslash escapes."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            "    for name in ('can\\u00e7\\u00e3o.xml', '\\u65e5\\u672c.xml'):\n"
            f"        shutil.copyfile({LAST_WINDOW!r}, os.path.join(directory, name))\n"
            "    collection = ml.ScoreCollection(directory)\n"
            "    table = collection.findMelodyPatternDataFrame([ml.Note('C4'), ml.Note('D4')])\n"
            "    names = [score.getFileName() for score in collection.getScores()]\n"
            "    print('RESULT', ascii(names), ascii(sorted(set(table['fileName']))))\n"
        )
        completed = runChild(code, {"PYTHONIOENCODING": "ascii"})
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        names = "['can\\xe7\\xe3o.xml', '\\u65e5\\u672c.xml']"
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, [f"RESULT {names} {names}"])
        self.assertIn("Loading: \\u65e5\\u672c.xml", completed.stdout)


class ConsoleRedirectTestCase(unittest.TestCase):
    """The loading methods write to Python's sys.stdout; a stream that fails cannot end the
    interpreter or change what the call does. Each test runs in a child process, where a crash
    shows as the exit code."""

    PRINTS = os.path.join(UNIT_TEST, "unrepresentable_alter_near_quarter_tone.xml")

    def test_a_stream_that_raises_value_error_drops_the_text(self):
        code = (
            "import sys\n"
            "import maialib as ml\n"
            "class Closed:\n"
            "    encoding = 'utf-8'\n"
            "    def write(self, text):\n"
            "        raise ValueError('I/O operation on closed file.')\n"
            "    def flush(self):\n"
            "        raise ValueError('I/O operation on closed file.')\n"
            "standard = sys.stdout\n"
            "sys.stdout = Closed()\n"
            "collection = ml.ScoreCollection()\n"
            "collection.addScore('./missing.xml')\n"
            f"score = ml.Score({self.PRINTS!r})\n"
            "sys.stdout = standard\n"
            "print('RESULT', collection.getNumScores(), collection.getLoadErrors(),\n"
            "      len(score.getImportIssues()))\n"
        )
        completed = runChild(code)
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(
            results,
            ["RESULT 0 [('./missing.xml', \"[maiacore] Score: cannot open './missing.xml'\")] 1"],
        )

    def test_a_stream_that_raises_keyboard_interrupt_interrupts_after_the_call(self):
        """The interrupt is raised once the call has returned with its whole result, and once."""
        code = (
            "import sys\n"
            "import maialib as ml\n"
            "class Interrupting:\n"
            "    encoding = 'utf-8'\n"
            "    def write(self, text):\n"
            "        raise KeyboardInterrupt\n"
            "    def flush(self):\n"
            "        pass\n"
            "standard = sys.stdout\n"
            "sys.stdout = Interrupting()\n"
            "outcomes = []\n"
            "collection = ml.ScoreCollection()\n"
            "for call in (lambda: collection.addScore('./missing.xml'),\n"
            f"             lambda: ml.Score({self.PRINTS!r})):\n"
            "    try:\n"
            "        call()\n"
            "        for _ in range(1000):\n"
            "            pass\n"
            "        outcomes.append('not interrupted')\n"
            "    except KeyboardInterrupt:\n"
            "        outcomes.append('interrupted')\n"
            "    for _ in range(1000):\n"
            "        pass\n"
            "sys.stdout = standard\n"
            "print('RESULT', outcomes, collection.getNumScores(), len(collection.getLoadErrors()))\n"
        )
        completed = runChild(code)
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, ["RESULT ['interrupted', 'interrupted'] 0 1"])


class ScoreCollectionMelodySearchTestCase(unittest.TestCase):
    def test_the_columns_start_with_the_score(self):
        table = twoScores().findMelodyPatternDataFrame([ml.Note("C4"), ml.Note("D4")], 1.0, 1.0)
        self.assertEqual(list(table.columns), SCORE_COLUMNS + MATCH_COLUMNS)
        self.assertEqual(list(table["scoreTitle"]), ["A", "A", "A", "B", "B"])
        self.assertEqual(list(table.index), [0, 1, 2, 3, 4])

    def test_the_list_overload_adds_the_pattern_index_first(self):
        """Both patterns match in both scores, so the search's pattern-by-pattern order differs
        from the table's: the binding sorts the rows stably by title, and the pattern and measure
        order within a title follow from the order of the per-score results."""
        patterns = [[ml.Note("C4"), ml.Note("D4")], [ml.Note("E4"), ml.Note("F#4")]]
        table = twoScores().findMelodyPatternDataFrame(patterns, 1.0, 1.0)
        self.assertEqual(list(table.columns), ["patternIdx"] + SCORE_COLUMNS + MATCH_COLUMNS)
        self.assertEqual(
            list(zip(table["scoreTitle"], table["patternIdx"], table["measure"])),
            [
                ("A", 0, 0),
                ("A", 0, 0),
                ("A", 0, 1),
                ("A", 1, 0),
                ("A", 1, 0),
                ("A", 1, 1),
                ("B", 0, 0),
                ("B", 0, 0),
                ("B", 1, 0),
                ("B", 1, 0),
            ],
        )

    def test_an_empty_result_is_an_empty_dataframe_with_every_column_and_dtype(self):
        pattern = [ml.Note("C4"), ml.Note("D4")]
        matched = twoScores().findMelodyPatternDataFrame(pattern, 1.0, 1.0)
        listed = twoScores().findMelodyPatternDataFrame([pattern], 1.0, 1.0)
        searches = {
            "empty collection": ml.ScoreCollection().findMelodyPatternDataFrame(pattern),
            "no match": twoScores().findMelodyPatternDataFrame(
                [ml.Note("C4"), ml.Note("C6")], 1.0, 1.0
            ),
        }
        for name, table in searches.items():
            with self.subTest(search=name):
                self.assertEqual(len(table), 0)
                self.assertEqual(list(table.dtypes.items()), list(matched.dtypes.items()))
        empty = ml.ScoreCollection().findMelodyPatternDataFrame([pattern])
        self.assertEqual(len(empty), 0)
        self.assertEqual(list(empty.dtypes.items()), list(listed.dtypes.items()))
        self.assertEqual(str(matched["measure"].dtype), "int64")
        self.assertEqual(str(listed["patternIdx"].dtype), "int64")

    def test_a_pattern_of_fewer_than_two_notes_raises_even_in_an_empty_collection(self):
        with self.assertRaises(RuntimeError):
            ml.ScoreCollection().findMelodyPatternDataFrame([ml.Note("C4")])
        with self.assertRaises(RuntimeError):
            ml.ScoreCollection().findMelodyPatternDataFrame([[ml.Note("C4"), ml.Note("D4")], []])

    def test_the_thresholds_have_the_unified_names(self):
        table = twoScores().findMelodyPatternDataFrame(
            [ml.Note("C4"), ml.Note("D4")],
            intervalSimilarityThreshold=1.0,
            rhythmSimilarityThreshold=1.0,
        )
        self.assertEqual(len(table), 5)
        with self.assertRaises(TypeError):
            twoScores().findMelodyPatternDataFrame(
                [ml.Note("C4"), ml.Note("D4")], totalIntervalsSimilarityThreshold=1.0
            )


if __name__ == "__main__":
    unittest.main()
