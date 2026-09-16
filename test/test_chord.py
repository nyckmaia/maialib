import unittest

import maialib as ml

# ===== TEST CHORD CLASS ===== #


class GetName(unittest.TestCase):
    def testMajorTriad01(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        self.assertEqual(myChord.getName(), "C")

    def testMajorTriad02(self):
        myChord = ml.Chord(["C4", "D7", "G6", "E4"])
        self.assertEqual(myChord.getName(), "C9")

    def testMajorWithBass(self):
        myChord = ml.Chord(["C4", "E4", "G3", "B4"])
        self.assertEqual(myChord.getName(), "C7M/G")

    def testMajorMinorSeventh(self):
        myChord = ml.Chord(["E5", "C5", "G3", "Bb3"])
        self.assertEqual(myChord.getName(), "C7/G")

    def testMajorMajorSeventh(self):
        myChord = ml.Chord(["C", "E", "G", "B"])
        self.assertEqual(myChord.getName(), "C7M")

    def testMinorTriad01(self):
        myChord = ml.Chord(["A4", "C5", "E7"])
        self.assertEqual(myChord.getName(), "Am")

    def testMinorTriad02(self):
        myChord = ml.Chord(["C5", "Eb5", "G3"])
        self.assertEqual(myChord.getName(), "Cm/G")

    def testMinorSeventhWithBass(self):
        myChord = ml.Chord(["A5", "C5", "E7", "G3"])
        self.assertEqual(myChord.getName(), "Am7/G")

    def testEightDistinctPitchClassesRaises(self):
        # A 10th out-of-bounds site the original audit missed: no enharmonic respelling of this
        # chord's 8 distinct pitch classes can give every note a distinct letter (only 7 exist,
        # A-G) and form a valid stacked-in-thirds heap, so the internal computation finds none at
        # all. More than 8 distinct pitch classes throws earlier and unconditionally elsewhere;
        # see testThreeNoteClusterRaises below for why "8 distinct pitch classes" is not itself a
        # real threshold, just one sufficient case.
        myChord = ml.Chord(["C4", "D4", "E4", "F4", "G4", "A4", "B4", "Db5"])
        with self.assertRaises(RuntimeError):
            myChord.getName()

    def testThreeNoteClusterRaises(self):
        # Pitch classes are de-duplicated by spelling string, not by enharmonic-equivalent pitch:
        # C, C#, Db and B# are four different pitch classes that all draw their possible letters
        # from just {B, C, D}. This 3-note chord already can't find a respelling that gives every
        # note a distinct letter and forms a valid stacked-in-thirds heap -- there is no simple
        # "N distinct pitch classes" threshold for when this guard fires.
        myChord = ml.Chord(["C4", "C#4", "Db4"])
        with self.assertRaises(RuntimeError):
            myChord.getName()


class IsTonal(unittest.TestCase):
    def testTonalMajorChord01(self):
        myChord = ml.Chord(["C", "E", "G"])
        self.assertEqual(myChord.isTonal(), True)

    def testTonalMinorChord01(self):
        myChord = ml.Chord(["D", "F", "A"])
        self.assertEqual(myChord.isTonal(), True)

    def testNotTonalMajorChord01(self):
        myChord = ml.Chord(["D", "F", "A#"])
        self.assertEqual(myChord.isTonal(), False)


class GetCloseStackChord(unittest.TestCase):
    def testMajorAndMinorChords(self):
        # Exact the same chords
        chord01_test = ml.Chord(["C4", "E4", "G4", "B4"])
        chord01_answ = ml.Chord(["C4", "E4", "G4", "B4"])
        self.assertEqual(chord01_test.getCloseStackChord().getNotes(), chord01_answ.getNotes())

        # Different note sequency
        chord02_test = ml.Chord(["Eb4", "C4", "B4", "G4"])
        chord02_answ = ml.Chord(["C4", "Eb4", "G4", "B4"])
        self.assertEqual(chord02_test.getCloseStackChord().getNotes(), chord02_answ.getNotes())

        # Both different note sequency and octave
        chord03_test = ml.Chord(["Eb3", "C5", "B2", "G9"])
        chord03_answ = ml.Chord(["C4", "Eb4", "G4", "B4"])
        self.assertEqual(chord03_test.getCloseStackChord().getNotes(), chord03_answ.getNotes())

    def testComplexChords(self):
        # # Inverted major-seven-nine (5th ommited) chord
        # chord01_test = ml.Chord(["D2", "C4", "E5", "B4"])
        # chord01_answ = ml.Chord(["C4", "E4", "B4", "D5"])
        # self.assertEqual(chord01_test.getCloseStackChord().getNotes(), chord01_answ.getNotes())

        # # Inverted minor-seven-nine (5th ommited) chord
        # chord02_test = ml.Chord(["F2", "E5", "D3", "C4"])
        # chord02_answ = ml.Chord(["D4", "F4", "C5", "E5"])
        # self.assertEqual(chord02_test.getCloseStackChord().getNotes(), chord02_answ.getNotes())

        # # Inverted minor-seven-eleven (5th ommited) chord
        # chord03_test = ml.Chord(["F2", "G5", "D3", "C4"])
        # chord03_answ = ml.Chord(["D4", "F4", "C5", "G5"])
        # self.assertEqual(chord03_test.getCloseStackChord().getNotes(), chord03_answ.getNotes())

        # Four notes chromatic cluster
        chord04_test = ml.Chord(["C4", "C#4", "D4", "D#4"])
        # [C4, Eb4, <G4>, Bx4, D5] => Cm(7aug)9
        chord04_answ = ml.Chord(["C4", "Eb4", "Bx4", "D5"])
        self.assertEqual(chord04_test.getCloseStackChord().getNotes(), chord04_answ.getNotes())

        # Five notes diatonic cluster
        chord05a_test = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        chord05a_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        self.assertEqual(chord05a_test.getCloseStackChord().getNotes(), chord05a_answ.getNotes())

        # Five notes diatonic cluster
        chord05b_test = ml.Chord(["C4", "Ebb7", "Fx5", "Ax3", "E4"])
        chord05b_answ = ml.Chord(["Ax4", "C5", "E5", "G5", "D6"])
        self.assertEqual(chord05b_test.getCloseStackChord().getNotes(), chord05b_answ.getNotes())

        # Five notes diatonic cluster
        chord05c_test = ml.Chord(["C4", "G4", "A4", "E4", "B5"])
        chord05c_answ = ml.Chord(["A4", "C5", "E5", "G5", "B5"])
        self.assertEqual(chord05c_test.getCloseStackChord().getNotes(), chord05c_answ.getNotes())

        # Five notes diatonic cluster
        chord05d_test = ml.Chord(["D3", "C4", "E4", "G7", "B5"])
        chord05d_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        self.assertEqual(chord05d_test.getCloseStackChord().getNotes(), chord05d_answ.getNotes())

        # Five notes diatonic cluster
        chord05e_test = ml.Chord(["C4", "Ab8", "Eb3", "G4", "B5"])
        chord05e_answ = ml.Chord(["Ab4", "C5", "Eb5", "G5", "B5"])
        self.assertEqual(chord05e_test.getCloseStackChord().getNotes(), chord05e_answ.getNotes())

        # Six notes diatonic cluster
        chord06_test = ml.Chord(["C4", "E4", "G4", "B4", "D5", "F5"])
        chord06_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5", "F5"])
        self.assertEqual(chord06_test.getCloseStackChord().getNotes(), chord06_answ.getNotes())

        # Six notes diatonic cluster
        chord06b_test = ml.Chord(["C4", "D4", "E4", "F4", "G4", "B4"])
        chord06b_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5", "F5"])
        self.assertEqual(chord06b_test.getCloseStackChord().getNotes(), chord06b_answ.getNotes())


class ChordOperator(unittest.TestCase):
    def testPlus(self):
        myChord1 = ml.Chord(["C", "E", "G"])
        myChord2 = ml.Chord(["B", "D", "F"])

        myChord3 = myChord1 + myChord2

        self.assertEqual(myChord3.size(), myChord1.size() + myChord2.size())


class GetNote(unittest.TestCase):
    def testValidIndex(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        self.assertEqual(myChord.getNote(0).getPitch(), "C4")

    def testNegativeIndexRaises(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        with self.assertRaises(RuntimeError):
            myChord.getNote(-1)

    def testIndexEqualToSizeRaises(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        with self.assertRaises(RuntimeError):
            myChord.getNote(3)

    def testEmptyChordRaises(self):
        myChord = ml.Chord()
        with self.assertRaises(RuntimeError):
            myChord.getNote(0)


class Info(unittest.TestCase):
    def testEmptyChordRaises(self):
        myChord = ml.Chord()
        with self.assertRaises(RuntimeError):
            myChord.info()


class ToInversion(unittest.TestCase):
    def testEmptyChordRaises(self):
        myChord = ml.Chord()
        with self.assertRaises(RuntimeError):
            myChord.toInversion(1)


class IsInRootPosition(unittest.TestCase):
    def testEmptyChordReturnsFalse(self):
        myChord = ml.Chord()
        self.assertEqual(myChord.isInRootPosition(), False)

    def testStaleCloseStackAfterClearReturnsFalse(self):
        # clear() empties _originalNotes/_openStack but leaves _closeStack (and the
        # "already stacked" flag) untouched, so a guard that only checks _closeStack
        # would still read out of bounds here.
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.getName()  # populate _closeStack
        myChord.clear()
        self.assertEqual(myChord.isInRootPosition(), False)

    def testStaleCloseStackAfterRemoveNoteToEmptyReturnsFalse(self):
        # removeNote() resets the "already stacked" flag, so isInRootPosition() re-runs
        # stackInThirds(), which early-returns on an empty chord without clearing the
        # (still non-empty, stale) _closeStack from before the notes were removed.
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.getName()  # populate _closeStack
        myChord.removeNote(0)
        myChord.removeNote(0)
        myChord.removeNote(0)
        self.assertEqual(myChord.isInRootPosition(), False)


class GetOpenAndCloseStackIntervals(unittest.TestCase):
    def testEmptyChordReturnsEmptyList(self):
        myChord = ml.Chord()
        self.assertEqual(myChord.getOpenStackIntervals(), [])
        self.assertEqual(myChord.getCloseStackIntervals(), [])


class ToCents(unittest.TestCase):
    def testEmptyChordReturnsEmptyList(self):
        myChord = ml.Chord()
        self.assertEqual(myChord.toCents(), [])

    def testSingleNoteChordReturnsEmptyList(self):
        myChord = ml.Chord(["C4"])
        self.assertEqual(myChord.toCents(), [])


class ChordMutationCacheInvalidation(unittest.TestCase):
    """Build -> query (populate the stack cache) -> mutate -> query again.

    Re-review found stackInThirds() never cleared the internal '_stackedHeaps' cache, so a
    mutated chord could reuse a stale, wrong-sized heap instead of recomputing -- silently
    wrong answers on the grow side, an out-of-bounds read on the shrink side. Fixed with a
    private invalidation helper called from every mutator. These pin the sequence.
    """

    def testGetNameAfterAddNoteReflectsNewChord(self):
        # The reviewer's exact repro sequence, which aborted the process before this fix.
        myChord = ml.Chord(["C4", "E4", "G4"])
        self.assertEqual(myChord.getName(), "C")

        myChord.addNote("B4")
        self.assertEqual(myChord.getName(), "C7M")

    def testGetNameAfterRemoveNoteReflectsNewChord(self):
        myChord = ml.Chord(["C4", "E4", "G4", "B4"])
        self.assertEqual(myChord.getName(), "C7M")

        myChord.removeNote(3)
        self.assertEqual(myChord.getName(), "C")

    def testGrowingPastValidHeapAfterStackingStillThrows(self):
        # A chord stacked once, then grown to 8 distinct pitch classes, used to keep the stale
        # 3-note heap and skip the "no valid heap" guard entirely.
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.getName()

        myChord.addNote("D4")
        myChord.addNote("F4")
        myChord.addNote("A4")
        myChord.addNote("B4")
        myChord.addNote("Db5")

        self.assertEqual(myChord.size(), 8)
        with self.assertRaises(RuntimeError):
            myChord.getName()

    def testShrinkingAfterStackingDoesNotReadStaleHeap(self):
        myChord = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        myChord.getName()

        myChord.removeNote(4)
        myChord.removeNote(3)
        self.assertEqual(myChord.size(), 3)

        self.assertEqual(myChord.getName(), "C")

    def testGetCloseStackIntervalsAfterGrowingMatchesNewSize(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.getName()

        myChord.addNote("B4")
        myChord.addNote("D5")

        intervals = myChord.getCloseStackIntervals()
        self.assertEqual(len(intervals), myChord.stackSize() - 1)


if __name__ == "__main__":
    unittest.main()
