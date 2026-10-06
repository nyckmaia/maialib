"""ScoreCollection: construction, directory discovery, edits and the melody search's DataFrames."""

import contextlib
import getpass
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


def runChild(code):
    """Run the Python source 'code' in a child process, from this directory, so that a crash of
    the interpreter shows as the child's exit code instead of ending the test run."""
    return subprocess.run(
        [sys.executable, "-c", code],
        cwd=HERE,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        timeout=120,
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


def fileNamesAreUtf8():
    """Whether Score keeps a file name as UTF-8. It keeps the name in the ANSI code page, which
    is UTF-8 everywhere except on Windows with a code page other than 65001."""
    if os.name != "nt":
        return True
    import ctypes

    return ctypes.windll.kernel32.GetACP() == 65001


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

    def test_a_file_that_fails_to_load_raises_naming_it_and_changes_nothing(self):
        with tempfile.TemporaryDirectory() as directory:
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))
            broken = os.path.join(directory, "b.xml")
            with open(broken, "w", encoding="utf-8") as text:
                text.write("not a score")
            collection = ml.ScoreCollection()
            collection.addScore(DUPLICATES)

            with self.assertRaises(RuntimeError) as context:
                collection.setDirectoriesPaths([directory])
            self.assertTrue(
                str(context.exception).startswith(f"{broken}: [maiacore] "),
                str(context.exception).splitlines()[0],
            )
            self.assertEqual(fileNames(collection), ["melody_duplicate_patterns.musicxml"])
            self.assertEqual(collection.getNumDirectories(), 0)

    def test_set_directories_paths_replaces_the_scores(self):
        collection = ml.ScoreCollection(BACH)
        collection.addScore(LAST_WINDOW)
        collection.setDirectoriesPaths([BEETHOVEN])
        collection.setDirectoriesPaths([BEETHOVEN])
        self.assertEqual(
            fileNames(collection),
            ["Beethoven_quartet_133.xml", "Beethoven_quartet_Op133.xml", "Symphony_5th_1Mov.xml"],
        )

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

    def test_a_non_ascii_file_name_does_not_crash_the_interpreter(self):
        """Run in a child process: logging the name of a file such as 'canção.xml'
        must not end the interpreter. The file either loads or raises RuntimeError.

        A search of the loaded collection builds its fileName column from the name in the ANSI
        code page: it succeeds where that is UTF-8, and raises UnicodeDecodeError on Windows
        with another code page. This pins the current behaviour of non-ASCII paths, which are
        not supported yet, so that a change to it is noticed."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            f"    shutil.copyfile({LAST_WINDOW!r}, os.path.join(directory, 'can\u00e7\u00e3o.xml'))\n"
            "    try:\n"
            "        collection = ml.ScoreCollection(directory)\n"
            "        print('RESULT loaded', collection.getNumScores())\n"
            "    except RuntimeError:\n"
            "        print('RESULT RuntimeError')\n"
            "    else:\n"
            "        try:\n"
            "            collection.findMelodyPatternDataFrame([ml.Note('C4'), ml.Note('D4')])\n"
            "            print('RESULT searched')\n"
            "        except UnicodeDecodeError:\n"
            "            print('RESULT UnicodeDecodeError')\n"
        )
        completed = runChild(code)
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        search = "RESULT searched" if fileNamesAreUtf8() else "RESULT UnicodeDecodeError"
        self.assertIn(results, (["RESULT loaded 1", search], ["RESULT RuntimeError"]))


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
