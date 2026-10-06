"""ScoreCollection: construction, directory discovery, edits and the melody search's DataFrames."""

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
        completed = subprocess.run(
            [sys.executable, "-c", code], capture_output=True, encoding="utf-8", timeout=120
        )
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, ["RESULT 1"])


class ScoreCollectionMelodySearchTestCase(unittest.TestCase):
    def test_the_columns_start_with_the_score(self):
        table = twoScores().findMelodyPatternDataFrame([ml.Note("C4"), ml.Note("D4")], 1.0, 1.0)
        self.assertEqual(list(table.columns), SCORE_COLUMNS + MATCH_COLUMNS)
        self.assertEqual(list(table["scoreTitle"]), ["A", "A", "A", "B", "B"])
        self.assertEqual(list(table.index), [0, 1, 2, 3, 4])

    def test_the_list_overload_adds_the_pattern_index_first(self):
        """Both patterns match in both scores, so the rows sort by title, then pattern, then
        measure, unlike the pattern-by-pattern order the search gives them in."""
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
