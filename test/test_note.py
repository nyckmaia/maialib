import unittest

import maialib as ml

# ===== TEST NOTE CLASS ===== #


class NoteConstructor(unittest.TestCase):
    def testOnlyPitchClass_getPitchTypesOctaveAndDuration(self):
        noteDoubleFlat = ml.Note("Abb")
        noteFlat = ml.Note("Ab")
        noteNatural = ml.Note("A")
        noteSharp = ml.Note("A#")
        noteDoubleSharp = ml.Note("Ax")

        # ===== PITCHSTEP ===== #
        self.assertEqual(noteDoubleFlat.getPitchStep(), "A")
        self.assertEqual(noteFlat.getPitchStep(), "A")
        self.assertEqual(noteNatural.getPitchStep(), "A")
        self.assertEqual(noteSharp.getPitchStep(), "A")
        self.assertEqual(noteDoubleSharp.getPitchStep(), "A")

        # ===== PITCHCLASS ===== #
        self.assertEqual(noteDoubleFlat.getPitchClass(), "Abb")
        self.assertEqual(noteFlat.getPitchClass(), "Ab")
        self.assertEqual(noteNatural.getPitchClass(), "A")
        self.assertEqual(noteSharp.getPitchClass(), "A#")
        self.assertEqual(noteDoubleSharp.getPitchClass(), "Ax")

        # ===== PITCH ===== #
        self.assertEqual(noteDoubleFlat.getPitch(), "Abb4")
        self.assertEqual(noteFlat.getPitch(), "Ab4")
        self.assertEqual(noteNatural.getPitch(), "A4")
        self.assertEqual(noteSharp.getPitch(), "A#4")
        self.assertEqual(noteDoubleSharp.getPitch(), "Ax4")

        # ===== DEFAULT OCTAVE ===== #
        self.assertEqual(noteDoubleFlat.getOctave(), 4)
        self.assertEqual(noteFlat.getOctave(), 4)
        self.assertEqual(noteNatural.getOctave(), 4)
        self.assertEqual(noteSharp.getOctave(), 4)
        self.assertEqual(noteDoubleSharp.getOctave(), 4)

        # ===== DEFAULT DURATION TICKS ===== #
        self.assertEqual(noteDoubleFlat.getDurationTicks(), 256)
        self.assertEqual(noteFlat.getDurationTicks(), 256)
        self.assertEqual(noteNatural.getDurationTicks(), 256)
        self.assertEqual(noteSharp.getDurationTicks(), 256)
        self.assertEqual(noteDoubleSharp.getDurationTicks(), 256)


class NoteEqualsOperator(unittest.TestCase):
    def testNonEnharmonicNotes(self):
        a = ml.Note("A")
        b = ml.Note("A")

        self.assertEqual(a == b, True)
        self.assertEqual(a.getMidiNumber() == b.getMidiNumber(), True)

    def testEnharmonicNotes(self):
        a = ml.Note("A")
        b = ml.Note("Gx")

        self.assertEqual(a == b, False)
        self.assertEqual(a.getMidiNumber() == b.getMidiNumber(), True)


class NoteSetPitch(unittest.TestCase):
    def testWrittenAndSoundingPitchTypesAndOctave(self):
        note = ml.Note("A")

        note.setPitchClass("D")

        # ===== WRITTEN ATTRIBUTES ===== #
        self.assertEqual(note.getWrittenPitchClass(), "D")
        self.assertEqual(note.getWrittenPitch(), "D4")
        self.assertEqual(note.getWrittenOctave(), 4)

        # ===== SOUNDING ATTRIBUTES ===== #
        self.assertEqual(note.getSoundingPitchClass(), "D")
        self.assertEqual(note.getSoundingPitch(), "D4")
        self.assertEqual(note.getSoundingOctave(), 4)

        # ===== ALIAS WRITTEN ATTRIBUTES ===== #
        self.assertEqual(note.getPitchClass(), note.getWrittenPitchClass())
        self.assertEqual(note.getPitch(), note.getWrittenPitch())
        self.assertEqual(note.getOctave(), note.getWrittenOctave())

    def testWrittenAndSoundingPitchTypesAndOctave_transposeInstrument(self):
        note = ml.Note("A")

        note.setPitchClass("D")
        note.setTransposingInterval(-1, -2)

        # ===== WRITTEN ATTRIBUTES ===== #
        self.assertEqual(note.getWrittenPitchClass(), "D")
        self.assertEqual(note.getWrittenPitch(), "D4")
        self.assertEqual(note.getWrittenOctave(), 4)

        # ===== SOUNDING ATTRIBUTES ===== #
        self.assertEqual(note.getSoundingPitchClass(), "C")
        self.assertEqual(note.getSoundingPitch(), "C4")
        self.assertEqual(note.getSoundingOctave(), 4)

    def testWrittenAndSoundingPitchTypesAndOctave_transposeInstrumentChangeOctave(self):
        note = ml.Note("C4")

        note.setTransposingInterval(-1, -2)

        # ===== WRITTEN ATTRIBUTES ===== #
        self.assertEqual(note.getWrittenPitchClass(), "C")
        self.assertEqual(note.getWrittenPitch(), "C4")
        self.assertEqual(note.getWrittenOctave(), 4)

        # ===== SOUNDING ATTRIBUTES ===== #
        self.assertEqual(note.getSoundingPitchClass(), "Bb")
        self.assertEqual(note.getSoundingPitch(), "Bb3")
        self.assertEqual(note.getSoundingOctave(), 3)

    def testGetPitchTypesAndOctave(self):
        noteDoubleFlat = ml.Note("Dbb")
        noteFlat = ml.Note("Db")
        noteNatural = ml.Note("D")
        noteSharp = ml.Note("D#")
        noteDoubleSharp = ml.Note("Dx")

        noteDoubleFlat.setPitch("A")
        noteFlat.setPitch("A")
        noteNatural.setPitch("A")
        noteSharp.setPitch("A")
        noteDoubleSharp.setPitch("A")

        # ===== PITCHSTEP ===== #
        self.assertEqual(noteDoubleFlat.getPitchStep(), "A")
        self.assertEqual(noteFlat.getPitchStep(), "A")
        self.assertEqual(noteNatural.getPitchStep(), "A")
        self.assertEqual(noteSharp.getPitchStep(), "A")
        self.assertEqual(noteDoubleSharp.getPitchStep(), "A")

        # ===== PITCHCLASS ===== #
        self.assertEqual(noteDoubleFlat.getPitchClass(), "A")
        self.assertEqual(noteFlat.getPitchClass(), "A")
        self.assertEqual(noteNatural.getPitchClass(), "A")
        self.assertEqual(noteSharp.getPitchClass(), "A")
        self.assertEqual(noteDoubleSharp.getPitchClass(), "A")

        # ===== PITCH ===== #
        self.assertEqual(noteDoubleFlat.getPitch(), "A4")
        self.assertEqual(noteFlat.getPitch(), "A4")
        self.assertEqual(noteNatural.getPitch(), "A4")
        self.assertEqual(noteSharp.getPitch(), "A4")
        self.assertEqual(noteDoubleSharp.getPitch(), "A4")

        # ===== OCTAVE ===== #
        self.assertEqual(noteDoubleFlat.getOctave(), 4)
        self.assertEqual(noteFlat.getOctave(), 4)
        self.assertEqual(noteNatural.getOctave(), 4)
        self.assertEqual(noteSharp.getOctave(), 4)
        self.assertEqual(noteDoubleSharp.getOctave(), 4)


class NotePitchSpellingRange(unittest.TestCase):
    def testFullRangeEdges(self):
        self.assertEqual(ml.Note("Cbb0").getMidiNumber(), 10)
        self.assertEqual(ml.Note("Bx9").getMidiNumber(), 133)
        self.assertEqual(ml.Note("C-1").getMidiNumber(), 0)
        self.assertEqual(ml.Note(5).getPitch(), "F-1")
        with self.assertRaises(RuntimeError):
            ml.Note("Cb-1")

    def testSetPitchResetsAccidental(self):
        note = ml.Note("C#4")
        note.setPitch("D4")
        self.assertEqual(note.getAlterSymbol(), "")
        self.assertEqual(note.getMidiNumber(), 62)

    def testEnharmonicRangeFallback(self):
        self.assertEqual(ml.Note("Bx11").getEnharmonicPitch(), "Bx11")
        self.assertEqual(ml.Note("B11").getEnharmonicPitch(True), "Ax11")
        self.assertEqual(ml.Note("C-1").getEnharmonicPitches(True), ["C-1", "Dbb-1", "Dbb-1"])


class NoteComposesPitch(unittest.TestCase):
    # Step 1 bug fix, through the bindings: setPitchClass() used to leave getMidiNumber()
    # reporting the note's previous pitch.
    def testSetPitchClassUpdatesAccidentalAndMidi(self):
        note = ml.Note("C4")
        note.setPitchClass("Eb")
        self.assertEqual(note.getAlterSymbol(), "b")
        self.assertEqual(note.getMidiNumber(), 63)

    # T5 (Python parity for T1): Note(pitch, isNoteOn=False) is a fully consistent rest.
    def testConstructorIsNoteOnFalseIsAFullyConsistentRest(self):
        note = ml.Note("C4", isNoteOn=False)
        self.assertFalse(note.isNoteOn())
        self.assertTrue(note.isNoteOff())
        self.assertEqual(note.getPitchClass(), "rest")
        self.assertEqual(note.getPitch(), "rest")
        self.assertEqual(note.getMidiNumber(), -1)
        self.assertEqual(note.getPitchStep(), "rest")
        # Task 6b: was self.assertEqual(note.getOctave(), -2); getOctave() now returns
        # int | None, None for a rest instead of the -2 sentinel.
        self.assertIsNone(note.getOctave())

    # T5 (Python parity for T2): setIsNoteOn(False) makes every getter report a rest, not just
    # isNoteOn(). Before this task, getPitchClass()/getMidiNumber() kept reporting the note's
    # previous sounding pitch ("C#"/60) after this call.
    def testSetIsNoteOnFalseReportsRestEverywhere(self):
        note = ml.Note("C#4")
        note.setIsNoteOn(False)
        self.assertTrue(note.isNoteOff())
        self.assertFalse(note.isNoteOn())
        self.assertEqual(note.getPitchClass(), "rest")
        self.assertEqual(note.getPitch(), "rest")
        self.assertEqual(note.getMidiNumber(), -1)
        self.assertEqual(note.getPitchStep(), "rest")
        # Task 6b: was self.assertEqual(note.getOctave(), -2).
        self.assertIsNone(note.getOctave())

    # T5 (Python parity for T3): setIsNoteOn(True) on a rest refuses (warns, does not throw)
    # instead of flipping the flag under a still-empty pitch.
    def testSetIsNoteOnTrueOnRestRefusesAndWarns(self):
        note = ml.Note("")
        self.assertTrue(note.isNoteOff())
        note.setIsNoteOn(True)  # must not raise
        self.assertTrue(note.isNoteOff())
        self.assertFalse(note.isNoteOn())
        self.assertEqual(note.getWrittenPitchStep(), "rest")
        self.assertEqual(note.getPitchClass(), "rest")

    # Fix round 1 (controller ruling on Task 6 concern 2), Python parity: getAlterSymbol()
    # forwards to the sounding pitch, not the written one. B-flat clarinet: written "C4"
    # (alter symbol "") transposed by (transposeDiatonic=-1, transposeChromatic=-2) sounds
    # "Bb3" (alter symbol "b") -- the same construction and measured values pinned in
    # note-test.cpp's NoteComposesPitch.GetAlterSymbolForwardsToSoundingPitchOnTransposedNote
    # (see that test's comment for why the controller's suggested "written C#4" case was
    # discarded: it triggers an unrelated, pre-existing bug in the transpose scale lookup).
    def testGetAlterSymbolForwardsToSoundingPitchOnTransposedNote(self):
        written = ml.Note("C4")
        self.assertEqual(written.getAlterSymbol(), "")

        transposed = ml.Note(
            "C4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertEqual(transposed.getSoundingPitch(), "Bb3")
        self.assertEqual(transposed.getMidiNumber(), 58)
        self.assertEqual(transposed.getAlterSymbol(), "b")
        self.assertNotEqual(transposed.getAlterSymbol(), written.getAlterSymbol())

    # Fix round 2 (I2), Python parity for T7's sibling on the C++ side: setStep()/setAlter()
    # were new public methods added by Task 6 with no pybind11 wrapper. Now bound; mirror the
    # C++ NoteComposesPitch.SetAlterOnRestRefusesAndWarns (T8) and
    # .SetStepResurrectsRestToOctave4 (T9) tests.
    def testSetAlterOnRestRefusesAndWarns(self):
        note = ml.Note("rest")
        note.setAlter(0.5)  # must not raise
        self.assertTrue(note.isNoteOff())
        # Task 6b: was self.assertEqual(note.getOctave(), -2).
        self.assertIsNone(note.getOctave())
        self.assertEqual(note.getPitchClass(), "rest")

    def testSetStepResurrectsRestToOctave4(self):
        note = ml.Note("rest")
        note.setStep("C")
        self.assertTrue(note.isNoteOn())
        self.assertFalse(note.isNoteOff())
        self.assertEqual(note.getPitch(), "C4")
        self.assertEqual(note.getOctave(), 4)

    # Fix round 5 (F1), Python parity: silencing a TRANSPOSING instrument with
    # setIsNoteOn(False) makes it a rest but deliberately keeps its transposing interval, so
    # isTransposed() stays True. getSoundingPitch() therefore used to take its transposition
    # branch and concatenate the pitch class "rest" with the octave -2, returning the malformed
    # string "rest-2" -- not a valid pitch, and rejected by the Note constructor. Measured
    # against a d26aa67 worktree, which never produced a malformed pitch string, so this was a
    # Task 6 regression rather than pre-existing. Mirrors the C++
    # NoteComposesPitch.GetPitchIsWellFormedRestForTransposedNoteTurnedOff test.
    def testSilencedTransposedNoteReportsWellFormedRest(self):
        note = ml.Note(
            "C#4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        note.setIsNoteOn(False)
        self.assertTrue(note.isTransposed())  # the interval survives; hence the guard
        self.assertTrue(note.isNoteOff())
        self.assertEqual(note.getPitch(), "rest")
        self.assertEqual(note.getSoundingPitch(), "rest")
        self.assertEqual(note.getWrittenPitch(), "rest")
        self.assertEqual(note.getMidiNumber(), -1)

    # Task 6b, Python parity: a rest has no octave. Note's three octave getters used to
    # collapse that absence to the numeric sentinel -2; they now return int | None, None for a
    # rest, unchanged for every non-rest input.
    def testRestHasNoOctave(self):
        rest = ml.Note("", isNoteOn=False)
        self.assertIsNone(rest.getOctave())
        self.assertIsNone(rest.getWrittenOctave())
        self.assertIsNone(rest.getSoundingOctave())
        self.assertEqual(ml.Note("C#4").getOctave(), 4)

    # Task 6b, section K, Python parity: the rest check in Pitch::setOctave() (reached through
    # Note::setOctave()) now runs before the range check, so a rest given an out-of-range
    # octave -- including the literal old -2 sentinel -- warns and does not mutate, instead of
    # throwing. A genuine out-of-range octave on a real (non-rest) note still throws.
    def testSetOctaveOutOfRangeOnRestWarnsAndDoesNotThrow(self):
        note = ml.Note("rest")
        note.setOctave(-2)  # must not raise
        self.assertTrue(note.isNoteOff())
        self.assertIsNone(note.getOctave())

        note.setOctave(12)  # must not raise
        self.assertTrue(note.isNoteOff())
        self.assertIsNone(note.getOctave())

        sounding = ml.Note("C4")
        with self.assertRaises(RuntimeError):
            sounding.setOctave(-2)
        self.assertEqual(sounding.getOctave(), 4)

    # Task 6b, finding N1, Python parity (see note-test.cpp's
    # NoteComposesPitch.SetStepAfterSetIsNoteOnFalseKeepsTransposingInterval for the full
    # reasoning): setIsNoteOn(False) deliberately keeps a transposing note's interval, and
    # setStep() resurrecting a rest does not clear it either. The revived note is not leaking
    # stale state -- it is identical to constructing the same written pitch on the same
    # transposing instrument from scratch. Pinned as defensible, not a bug.
    def testSetStepAfterSetIsNoteOnFalseKeepsTransposingInterval(self):
        note = ml.Note(
            "C4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertEqual(note.getSoundingPitch(), "Bb3")

        note.setIsNoteOn(False)
        self.assertTrue(note.isNoteOff())
        self.assertTrue(note.isTransposed())

        note.setStep("C")
        self.assertTrue(note.isNoteOn())
        self.assertTrue(note.isTransposed())
        self.assertEqual(note.getTransposeDiatonic(), -1)
        self.assertEqual(note.getTransposeChromatic(), -2)
        self.assertEqual(note.getWrittenPitch(), "C4")
        self.assertEqual(note.getSoundingPitch(), "Bb3")


if __name__ == "__main__":
    unittest.main()
