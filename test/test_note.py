import contextlib
import io
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


def enharmonicsOf(note):
    """The default and the alternative enharmonic spelling of a note."""
    return (note.getEnharmonicPitch(False), note.getEnharmonicPitch(True))


class NoteQuarterToneEnharmonic(unittest.TestCase):
    """A quarter tone's partners are the white-key steps within 1.5 semitones of it, spelled with
    the quarter-tone accidental that separates them in that step's own octave. The default is the
    partner with the smaller alter, a tie going to the side opposite the note's own accidental.
    Mirrors the C++ QuarterToneEnharmonic tests."""

    def testAQuarterToneHasOneOrTwoPartners(self):
        self.assertEqual(enharmonicsOf(ml.Note("C1x4")), ("D3b4", "B3x3"))
        self.assertEqual(enharmonicsOf(ml.Note("C3x4")), ("D1b4", "D1b4"))
        self.assertEqual(enharmonicsOf(ml.Note("E1b4")), ("D3x4", "F3b4"))
        self.assertEqual(enharmonicsOf(ml.Note("E1x4")), ("F1b4", "F1b4"))

    def testTheDefaultIsTheSmallerAlterThenTheOppositeSide(self):
        self.assertEqual(enharmonicsOf(ml.Note("D3b4")), ("C1x4", "B3x3"))
        self.assertEqual(enharmonicsOf(ml.Note("B3x3")), ("C1x4", "D3b4"))
        self.assertEqual(enharmonicsOf(ml.Note("F3b4")), ("E1b4", "D3x4"))
        self.assertEqual(enharmonicsOf(ml.Note("B1b3")), ("A3x3", "C3b4"))

    def testAPartnerOutsideTheOctaveRangeDoesNotExist(self):
        self.assertEqual(enharmonicsOf(ml.Note("A3x11")), ("B1b11", "B1b11"))
        self.assertEqual(enharmonicsOf(ml.Note("B1x11")), ("B1x11", "B1x11"))
        self.assertEqual(enharmonicsOf(ml.Note("C1x-1")), ("D3b-1", "D3b-1"))
        self.assertEqual(enharmonicsOf(ml.Note("C1b-1")), ("C1b-1", "C1b-1"))

    def testTheWholeFamilyRespellsAQuarterTone(self):
        self.assertEqual(ml.Note("C1x4").getEnharmonicPitches(True), ["C1x4", "D3b4", "B3x3"])
        self.assertEqual(ml.Note("C1x4").getEnharmonicPitches(), ["D3b4", "B3x3"])
        self.assertEqual(ml.Note("C3x4").getEnharmonicPitches(True), ["C3x4", "D1b4", "D1b4"])
        self.assertEqual(ml.Note("E1b4").getEnharmonicNote().getPitch(), "D3x4")
        self.assertEqual(ml.Note("E1b4").getEnharmonicNote(True).getPitch(), "F3b4")
        self.assertEqual(
            [note.getPitch() for note in ml.Note("E1b4").getEnharmonicNotes(True)],
            ["E1b4", "D3x4", "F3b4"],
        )

        note = ml.Note("C1x4")
        note.toEnharmonicPitch()
        self.assertEqual(note.getPitch(), "D3b4")
        note.toEnharmonicPitch(alternativeEnharmonicPitch=True)
        self.assertEqual(note.getPitch(), "B3x3")

    def testTheSoundingPitchIsRespelled(self):
        clarinet = ml.Note("C1x4", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertEqual(clarinet.getPitch(), "B1b3")
        self.assertEqual(enharmonicsOf(clarinet), ("A3x3", "C3b4"))

    def testEveryRespellingDescribesTheSamePitch(self):
        numSpellings = 0
        for step in "CDEFGAB":
            for symbol in ("3b", "1b", "1x", "3x"):
                for octave in range(-1, 12):
                    if step == "C" and symbol == "3b" and octave == -1:
                        continue  # -1.5, below the lowest representable pitch, C1b-1
                    pitch = f"{step}{symbol}{octave}"
                    note = ml.Note(pitch)
                    numSpellings += 1
                    for respelling in enharmonicsOf(note):
                        with self.subTest(pitch=pitch, respelling=respelling):
                            respelled = ml.Note(respelling)
                            self.assertEqual(
                                respelled.getQuarterToneSteps(), note.getQuarterToneSteps()
                            )
                            self.assertTrue(respelled.isQuarterTone())
        self.assertEqual(numSpellings, 7 * 4 * 13 - 1)


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
    # note-test.cpp's NoteComposesPitch.GetAlterSymbolForwardsToSoundingPitchOnTransposedNote.
    #
    # TASK 10 RESTORED THE DISCARDED CASE. This comment used to end by saying the controller's
    # suggested "written C#4" case was discarded because "it triggers an unrelated, pre-existing
    # bug in the transpose scale lookup". That bug is the scale lookup Task 10 deleted, so the
    # case works now and is asserted below.
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

        # The restored case, forwarding in the opposite direction: written "C#4" (alter symbol
        # "#") sounds "B3" (alter symbol ""). Before Task 10 the scale lookup mis-spelled this
        # sounding pitch as "Bb3" and getAlterSymbol() answered "b".
        sharpWritten = ml.Note("C#4")
        self.assertEqual(sharpWritten.getAlterSymbol(), "#")

        sharpTransposed = ml.Note(
            "C#4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertEqual(sharpTransposed.getSoundingPitch(), "B3")
        self.assertEqual(sharpTransposed.getMidiNumber(), 59)
        self.assertEqual(sharpTransposed.getAlterSymbol(), "")
        self.assertNotEqual(
            sharpTransposed.getAlterSymbol(), sharpWritten.getAlterSymbol()
        )

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

    # Task 10, Python parity: Note.transpose() took an int, so a quarter-tone interval could not
    # be expressed, and it rounded the pitch to a MIDI integer before applying the interval, so a
    # quarter tone was destroyed by the first step. Mirrors the C++ NoteTransposition
    # .TransposeByQuarterTone family.
    def testTransposeByQuarterTone(self):
        note = ml.Note("C4")
        note.transpose(0.5)
        self.assertEqual(note.getPitch(), "C1x4")
        self.assertTrue(note.isQuarterTone())

        note.transpose(-0.5)
        self.assertEqual(note.getPitch(), "C4")
        self.assertFalse(note.isQuarterTone())

    def testTransposePreservesQuarterToneAcrossWholeToneInterval(self):
        note = ml.Note("C1x4")
        note.transpose(2)
        self.assertEqual(note.getPitch(), "D1x4")  # rounded to "D4" before Task 10
        self.assertTrue(note.isQuarterTone())

    def testTransposeRejectsIntervalOffTheQuarterToneGrid(self):
        note = ml.Note("C4")
        with self.assertRaises(RuntimeError) as ctx:
            note.transpose(0.3)
        self.assertIn("multiple of 0.5", str(ctx.exception))
        self.assertEqual(note.getPitch(), "C4")  # the refused call changed nothing

    # Task 11, section N, Python parity: Note.transpose() silently turned a note transposed below
    # MIDI 0 into a rest. Mirrors the C++
    # NoteTransposition.TransposeOutOfRangeRaisesAndLeavesTheNoteUnchanged test.
    def testTransposeOutOfRangeRaisesAndLeavesTheNoteUnchanged(self):
        note = ml.Note("C4")
        with self.assertRaises(RuntimeError) as ctx:
            note.transpose(-61)
        self.assertIn("outside the representable range", str(ctx.exception).splitlines()[0])
        self.assertEqual(note.getPitch(), "C4")
        self.assertTrue(note.isNoteOn())

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

    # A written "C#-1" on a B-flat clarinet sounds below the lowest representable pitch, C1b-1.
    # The note is constructible and is a note, not a rest; its sounding pitch fails loudly and
    # diagnosably, not as a rest and not with an unexplained internal error.
    def testGetPitchBelowMidiZeroFailsDiagnosably(self):
        note = ml.Note(
            "C#-1", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertTrue(note.isNoteOn())
        self.assertFalse(note.isNoteOff())

        with self.assertRaises(RuntimeError) as ctx:
            note.getPitch()
        message = str(ctx.exception).splitlines()[0]
        self.assertIn("below the lowest representable pitch C1b-1", message)
        self.assertIn("'C#-1'", message)
        self.assertNotIn("ptional access", message)

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

    # Task 6b, section L, fix round 1: pins the round trip the C++ side can no longer even
    # express (Note.setOctave(int) has no overload accepting the None getOctave() now returns
    # for a rest, so the equivalent C++ call is a compile error, not a runtime one -- this is
    # the Python-only half of closing fix round 5's finding F2). setOctave(getOctave()) on a
    # rest raises TypeError from the pybind11 argument conversion (None cannot become int),
    # never the old std::runtime_error round-trip hazard, and mutates nothing.
    def testSetOctaveOfGetOctaveRoundTripOnRestRaisesTypeError(self):
        note = ml.Note("rest")
        self.assertIsNone(note.getOctave())
        with self.assertRaises(TypeError):
            note.setOctave(note.getOctave())
        self.assertTrue(note.isNoteOff())
        self.assertIsNone(note.getOctave())

    # Task 6b, finding N1, Python parity (see note-test.cpp's
    # NoteComposesPitch.SetStepAfterSetIsNoteOnFalseKeepsTransposingInterval for the full
    # reasoning, including fix round 1's rebuild at C5 so the pin actually discriminates --
    # C4 could not tell "octave preserved" from "octave defaulted to 4" by setStep()). Pinned
    # finding: the revived note is SELF-CONSISTENT (freshly derived from its current, defaulted
    # written pitch and the surviving interval), not a stale-leftover defect. Not "revived
    # equals a note that already carries the interval" -- that would assume the answer.
    def testSetStepAfterSetIsNoteOnFalseKeepsTransposingInterval(self):
        note = ml.Note(
            "C5", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertEqual(note.getWrittenPitch(), "C5")
        self.assertEqual(note.getSoundingPitch(), "Bb4")

        note.setIsNoteOn(False)
        self.assertTrue(note.isNoteOff())
        self.assertTrue(note.isTransposed())

        note.setStep("C")
        self.assertTrue(note.isNoteOn())
        self.assertTrue(note.isTransposed())
        self.assertEqual(note.getTransposeDiatonic(), -1)
        self.assertEqual(note.getTransposeChromatic(), -2)
        # Octave is NOT preserved (would be "C5"); setStep()'s ordinary hardcoded-4 default.
        self.assertEqual(note.getWrittenPitch(), "C4")
        # Freshly derived, not the pre-silence sounding pitch ("Bb4").
        self.assertEqual(note.getSoundingPitch(), "Bb3")
        self.assertNotEqual(note.getSoundingPitch(), "Bb4")


def firstLine(exception):
    """The message proper: the lines after it are a stack trace."""
    return str(exception).splitlines()[0]


class NoteQuarterToneSteps(unittest.TestCase):
    # Mirrors NoteQuarterToneSteps.isTheExactPositionOfTheSoundingPitch.
    def testIsTheExactPositionOfTheSoundingPitch(self):
        self.assertEqual(ml.Note("C1x4").getQuarterToneSteps(), 60.5)
        self.assertEqual(ml.Note("E1b4").getQuarterToneSteps(), 63.5)
        self.assertEqual(ml.Note("C4").getQuarterToneSteps(), 60.0)
        self.assertEqual(ml.Note("rest").getQuarterToneSteps(), -1.0)

        # Sounding, not written: a B-flat clarinet's written C1x4 sounds a whole tone lower.
        clarinet = ml.Note("C1x4", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertEqual(clarinet.getQuarterToneSteps(), 58.5)
        self.assertEqual(clarinet.getMidiNumber(), 59)

    def testIsDocumentedWithAnExample(self):
        self.assertIn("Examples\n", ml.Note.getQuarterToneSteps.__doc__)

    # Mirrors NoteQuarterToneSteps.setAlterRejectsAValueNearTheGrid.
    def testSetAlterRejectsAValueNearTheGrid(self):
        note = ml.Note("C4")
        with self.assertRaises(RuntimeError) as context:
            note.setAlter(0.99996)
        self.assertIn("multiple of 0.5", firstLine(context.exception))
        self.assertEqual(note.getPitch(), "C4")
        self.assertFalse(note.isQuarterTone())

    # Mirrors NoteQuarterToneSteps.setAlterOfNegativeZeroLeavesAPlainNatural.
    def testSetAlterOfNegativeZeroLeavesAPlainNatural(self):
        note = ml.Note("C#4")
        note.setAlter(-0.0)
        self.assertEqual(note.getPitch(), "C4")
        self.assertNotIn("<alter>", note.toXML())


class NoteSoundingPitchBelowFloor(unittest.TestCase):
    # Mirrors NoteSoundingPitchBelowFloor.everySoundingGetterFailsTheSameWay.
    GETTERS = (
        "getPitch",
        "getSoundingPitch",
        "getPitchClass",
        "getSoundingPitchClass",
        "getPitchStep",
        "getSoundingPitchStep",
        "getDiatonicSoundingPitchClass",
        "getAlterSymbol",
        "getOctave",
        "getSoundingOctave",
        "getMidiNumber",
        "getQuarterToneSteps",
        "getFrequency",
        "getEnharmonicPitch",
        "getEnharmonicPitches",
    )

    def testEverySoundingGetterFailsTheSameWay(self):
        for written, chromatic in (("C#-1", -2), ("C1b-1", -1), ("C1x-1", -2)):
            note = ml.Note(written, transposeDiatonic=-1, transposeChromatic=chromatic)
            for name in self.GETTERS:
                with self.subTest(written=written, getter=name):
                    with self.assertRaises(RuntimeError) as ctx:
                        getattr(note, name)()
                    message = firstLine(ctx.exception)
                    self.assertIn("below the lowest representable pitch C1b-1", message)
                    self.assertIn(f"'{written}'", message)

    def testTheWrittenPitchStillAnswers(self):
        note = ml.Note("C#-1", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertTrue(note.isNoteOn())
        self.assertEqual(note.getWrittenPitch(), "C#-1")
        self.assertEqual(note.getWrittenOctave(), -1)
        self.assertIn("<step>C</step>", note.toXML())

    def testAnIntervalWithSuchANoteFailsDiagnosably(self):
        note = ml.Note("C#-1", transposeDiatonic=-1, transposeChromatic=-2)
        with self.assertRaises(RuntimeError) as ctx:
            ml.Interval(note, ml.Note("C4")).getNumOctaves()
        self.assertIn("below the lowest representable pitch C1b-1", firstLine(ctx.exception))

    # Each of them says so in its docstring.
    def testEverySoundingGetterDocumentsTheRaise(self):
        for name in self.GETTERS:
            with self.subTest(getter=name):
                self.assertIn("C1b-1", getattr(ml.Note, name).__doc__)


def capturedStdout(action):
    """Run action() and return what it printed on sys.stdout."""
    buffer = io.StringIO()
    with contextlib.redirect_stdout(buffer):
        action()
    return buffer.getvalue()


class NoteSetterWarnings(unittest.TestCase):
    """The setters that refuse a value print their warning on sys.stdout, as Pitch's do."""

    def testSetOctaveOnARestWarns(self):
        rest = ml.Note("rest")
        printed = capturedStdout(lambda: rest.setOctave(4))
        self.assertIn("[WARN] Pitch::setOctave: cannot set the octave of a rest", printed)
        self.assertTrue(rest.isNoteOff())

    def testSetStepBelowMidiZeroWarns(self):
        note = ml.Note("Db-1")
        printed = capturedStdout(lambda: note.setStep("C"))
        self.assertIn("[WARN] Pitch::setStep: step 'C' would move this pitch below MIDI", printed)
        self.assertEqual(note.getPitch(), "Db-1")

    def testSetAlterOnARestWarns(self):
        rest = ml.Note("rest")
        printed = capturedStdout(lambda: rest.setAlter(0.5))
        self.assertIn("[WARN] Pitch::setAlter: cannot set the alter of a rest", printed)
        self.assertTrue(rest.isNoteOff())

    def testSetIsNoteOnTrueOnARestWarns(self):
        rest = ml.Note("rest")
        printed = capturedStdout(lambda: rest.setIsNoteOn(True))
        self.assertIn("[WARN] Cannot turn a rest into a sounding note without a pitch", printed)
        self.assertTrue(rest.isNoteOff())


class NoteQuarterToneRoundingDocumentation(unittest.TestCase):
    """getMidiNumber() and getFrequency() round a quarter tone, and say so."""

    def testTheRoundingIsDocumented(self):
        for name in ("getMidiNumber", "getFrequency"):
            with self.subTest(method=name):
                doc = " ".join(getattr(ml.Note, name).__doc__.split())
                self.assertIn("rounded, ties upward", doc)
        self.assertEqual(ml.Note("A1x4").getMidiNumber(), ml.Note("A#4").getMidiNumber())
        self.assertEqual(ml.Note("A1x4").getFrequency(), ml.Note("A#4").getFrequency())


if __name__ == "__main__":
    unittest.main()
