import io
import unittest
from contextlib import redirect_stdout

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

    def testReturnsACopyNotAReference(self):
        # getNote() must return a copy: py_chord.cpp used to register a second, dead
        # `reference_internal` overload with an identical signature (final review, item 3). A
        # reference-returning binding would let mutating the returned Note desync the chord's
        # cached stacked-in-thirds analysis without going through any mutator (see chord.h's
        # operator[] docs). Pinning copy semantics here guards against that binding coming back.
        myChord = ml.Chord(["C4", "E4", "G4"])
        note = myChord.getNote(0)
        note.setPitch("F#4")
        self.assertEqual(myChord.getNote(0).getPitch(), "C4")


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

    def testClearInvalidatesCacheReflectedByNextMutation(self):
        # Renamed and rewritten: this used to be worded around the round-1 bug where clear()
        # left _closeStack stale. Since chord.cpp's invalidateStackCache() fix, clear() clears
        # it too, so that stale state is no longer reachable through the public API, and a bare
        # isInRootPosition()==False check here would only duplicate testEmptyChordReturnsFalse.
        #
        # Re-review found that first rewrite still didn't discriminate: addNote() calls
        # invalidateStackCache() unconditionally, so the three addNote() calls further down
        # would mask a broken clear() regardless of what clear() itself did. The assertion that
        # actually pins clear()'s own invalidation is the one immediately below, BEFORE any
        # further mutation runs: with the fix, getName() re-stacks on the now-empty chord and
        # returns "" (no minor/major third); without it, getName() would skip re-stacking and
        # return the STALE pre-clear() name "C" instead. Confirmed by deletion on the C++ side
        # (see chord-test.cpp / the implementation report) that this assertion actually fails
        # without clear()'s invalidateStackCache() call and passes with it restored.
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.getName()  # populate the stack cache for the 3-note chord
        myChord.clear()
        self.assertEqual(myChord.getName(), "")  # pins clear()'s own invalidation, before addNote() runs
        self.assertEqual(myChord.isInRootPosition(), False)  # empty chord; defense in depth

        # Secondary check: confirms the chord behaves correctly after further mutation. Since
        # addNote() invalidates unconditionally on its own, this does NOT by itself prove clear()
        # invalidated anything -- the assertion above does that.
        myChord.addNote("D4")
        myChord.addNote("F4")
        myChord.addNote("A4")
        self.assertEqual(myChord.getName(), "Dm")  # reflects the new chord, not any pre-clear cache

    def testRemoveNoteToEmptyInvalidatesCacheReflectedByNextMutation(self):
        # Renamed and rewritten for the same reason as the test above, via removeNote() instead
        # of clear(): it now also invalidates the cache, so the old stale-_closeStack scenario
        # this test used to set up is no longer reachable.
        #
        # Unlike clear()'s test above, this one deliberately does NOT add a getName() == ""
        # check immediately after removeNote() -- that crashes the interpreter process, not just
        # fails: clear() explicitly clears _openStack itself, but removeNote() does not (only
        # invalidateStackCache() runs, which deliberately excludes _openStack). After
        # removeNote()-to-empty, _openStack is therefore stale (still the pre-removal size)
        # while _closeStack is correctly emptied; isTonal()'s loop, bounded by the stale
        # stackSize()/_openStack, then reads the now-empty _closeStack out of bounds. This is a
        # real, currently-reachable, pre-existing bug found incidentally while fixing this test
        # (reported, not fixed here -- it is the _originalNotes/_openStack dual-representation
        # drift this branch is explicitly not chartered to redesign). Confirmed on the C++ side
        # (see chord-test.cpp) that the crash reproduces identically whether or not clear()'s own
        # invalidation is present, proving it is unrelated to clear() and exists even with every
        # fix on this branch applied.
        #
        # removeNote()'s invalidation is independently and safely pinned by
        # ChordMutationCacheInvalidation.testGetNameAfterRemoveNoteReflectsNewChord, which
        # shrinks a chord from 4 notes to 3 (not to empty) and queries getName() directly
        # afterward -- that path does not hit this crash.
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.getName()  # populate the stack cache for the 3-note chord
        myChord.removeNote(0)
        myChord.removeNote(0)
        myChord.removeNote(0)
        self.assertEqual(myChord.isInRootPosition(), False)  # empty chord; defense in depth

        myChord.addNote("D4")
        myChord.addNote("F4")
        myChord.addNote("A4")
        self.assertEqual(myChord.getName(), "Dm")  # reflects the new chord, not the stale cache


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


class SetDuration(unittest.TestCase):
    """Final review found a real, Python-reachable out-of-bounds WRITE: setDuration() indexed
    both _originalNotes and _openStack by _originalNotes.size(), but _openStack can be
    strictly smaller after stackInThirds() dedups by pitch class."""

    def testFloatOverloadSafeAfterPitchClassDedupShrinksOpenStack(self):
        # Exact 3-line repro from the final review. (The Duration& overload is not reachable
        # from Python at all -- Duration has no Python binding -- so only this overload, the
        # one actually reachable from Python, needs a Python-side test; the Duration& overload
        # is covered in chord-test.cpp.)
        myChord = ml.Chord(["C4", "C5"])
        myChord.getName()
        myChord.setDuration(1.0, 256)

        self.assertEqual(myChord.size(), 2)
        self.assertEqual(myChord.stackSize(), 1)


class RemoveTopNote(unittest.TestCase):
    def testEmptyChordRaises(self):
        myChord = ml.Chord()
        with self.assertRaises(RuntimeError):
            myChord.removeTopNote()

    def testValidCallDoesNotRaise(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.removeTopNote()
        self.assertEqual(myChord.size(), 2)


class InsertNote(unittest.TestCase):
    def testNegativeIndexRaises(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        with self.assertRaises(RuntimeError):
            myChord.insertNote(ml.Note("D4"), -1)

    def testIndexPastSizeRaises(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        with self.assertRaises(RuntimeError):
            myChord.insertNote(ml.Note("D4"), 4)

    def testIndexEqualToSizeAppendsWithoutRaising(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        myChord.insertNote(ml.Note("B4"), 3)
        self.assertEqual(myChord.size(), 4)


class RemoveNote(unittest.TestCase):
    def testNegativeIndexRaises(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        with self.assertRaises(RuntimeError):
            myChord.removeNote(-1)

    def testIndexEqualToSizeRaises(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        with self.assertRaises(RuntimeError):
            myChord.removeNote(3)

    def testEmptyChordRaises(self):
        myChord = ml.Chord()
        with self.assertRaises(RuntimeError):
            myChord.removeNote(0)


class QuarterToneAnalysisGuard(unittest.TestCase):
    """Harmonic analysis rejects quarter tones; roundQuarterTones() is the escape hatch.

    Every answer the stacked-in-thirds analysis computes is defined over twelve-tone equal
    temperament, so a quarter tone did not make it fail, it made it answer wrongly.
    """

    def testGetNameRaises(self):
        # Asserting only that it raises would NOT discriminate the guard: the pre-existing
        # enharmonic-spelling guard also raises RuntimeError for a quarter tone, reached from
        # deep inside the stacking code. The diagnosable message is what this guard adds.
        myChord = ml.Chord(["C4", "E1b4", "G4"])
        with self.assertRaises(RuntimeError) as context:
            myChord.getName()

        self.assertIn("roundQuarterTones", str(context.exception))

    def testErrorMessageNamesTheNoteAndTheEscapeHatch(self):
        myChord = ml.Chord(["C4", "E1b4", "G4"])
        with self.assertRaises(RuntimeError) as context:
            myChord.getName()

        message = str(context.exception)
        self.assertIn("E1b4", message)
        self.assertIn("roundQuarterTones", message)

    def testEveryAnalysisMethodRaises(self):
        myChord = ml.Chord(["C4", "E1b4", "G4"])
        analysisMethods = (
            "getName",
            "getQuality",
            "getRoot",
            "getBassNote",
            "stackSize",
            "getOpenStackNotes",
            "getOpenStackIntervals",
            "getCloseStackIntervals",
            "getOpenStackChord",
            "getCloseStackChord",
            "getCloseChord",
            "isTonal",
            "isMajorChord",
            "isMinorChord",
            "isSus",
            "isInRootPosition",
            "haveMajorThird",
            "haveMinorThird",
            "havePerfectFifth",
            "haveMajorSeventh",
        )

        for methodName in analysisMethods:
            with self.subTest(method=methodName):
                with self.assertRaises(RuntimeError) as context:
                    getattr(myChord, methodName)()

                # The remedy named in the message is what identifies this as the analysis
                # guard rather than some deeper failure (see testGetNameRaises).
                self.assertIn("roundQuarterTones", str(context.exception))

    def testAccessorsAndMutatorsKeepWorking(self):
        # The reason the guard sits at the analysis chokepoint and not on the class as a whole:
        # a user must still be able to hold, inspect and repair a quarter-tone chord.
        myChord = ml.Chord(["C4", "E1b4", "G4"])

        self.assertEqual(myChord.size(), 3)
        self.assertEqual(myChord.getNote(1).getPitch(), "E1b4")
        self.assertEqual(len(myChord.getNotes()), 3)
        myChord.getDuration()
        myChord.setDuration(2.0)
        myChord.addNote("B4")
        self.assertEqual(myChord.size(), 4)
        myChord.removeNote(3)
        self.assertEqual(myChord.size(), 3)

    def testRoundQuarterTonesEnablesAnalysis(self):
        myChord = ml.Chord(["C4", "E1b4", "G4"])
        self.assertEqual(myChord.roundQuarterTones(), 1)
        self.assertEqual(myChord.getNote(1).getPitch(), "E4")
        self.assertEqual(myChord.getName(), "C")

    def testRoundQuarterTonesReturnsZeroOnASemitoneChord(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        self.assertEqual(myChord.roundQuarterTones(), 0)
        self.assertEqual(myChord.getName(), "C")

    def testInfoDegradesInsteadOfRaising(self):
        # info() is the diagnostic a user reaches for precisely when holding a chord they do not
        # understand, so it must work on any chord that can be built.
        myChord = ml.Chord(["C4", "E1b4", "G4"])

        buffer = io.StringIO()
        with redirect_stdout(buffer):
            myChord.info()
        output = buffer.getvalue()

        self.assertIn("Size:3", output)
        self.assertIn("[C4, E1b4, G4]", output)
        self.assertIn("unavailable", output)
        self.assertIn("roundQuarterTones", output)
        self.assertNotIn("Open Stack Size", output)

    def testInfoStillPrintsTheFullAnalysisForASemitoneChord(self):
        myChord = ml.Chord(["C4", "E4", "G4"])

        buffer = io.StringIO()
        with redirect_stdout(buffer):
            myChord.info()
        output = buffer.getvalue()

        self.assertIn("Name: C", output)
        self.assertIn("Open Stack Size", output)
        self.assertNotIn("unavailable", output)


if __name__ == "__main__":
    unittest.main()
