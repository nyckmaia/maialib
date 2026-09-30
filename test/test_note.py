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

    # Mirrors NoteEqualsOperator.aTransposedNoteIsComparedAtConcertPitch.
    def testATransposedNoteIsComparedAtConcertPitch(self):
        clarinet_d4 = ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertTrue(clarinet_d4 == ml.Note("C4"))
        self.assertFalse(clarinet_d4 != ml.Note("C4"))
        self.assertFalse(clarinet_d4 == ml.Note("D4"))

        clarinet_db4 = ml.Note("Db4", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertTrue(clarinet_db4 == ml.Note("Cb4"))
        self.assertFalse(clarinet_db4 == ml.Note("B3"))
        self.assertTrue(clarinet_db4 != ml.Note("B3"))


class NoteHash(unittest.TestCase):
    """Notes that compare equal hash equally, as Python requires."""

    def testEqualNotesHashEqually(self):
        clarinet_d4 = ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        for a, b in (
            (ml.Note("C4"), ml.Note("C4", ml.RhythmFigure.HALF)),
            (ml.Note("C4"), ml.Note("C4", inChord=True)),
            (clarinet_d4, ml.Note("C4")),
            (ml.Note("rest"), ml.Note("C4", isNoteOn=False)),
        ):
            with self.subTest(a=repr(a), b=repr(b)):
                self.assertTrue(a == b)
                self.assertEqual(hash(a), hash(b))

    def testASetKeepsOneOfEqualNotes(self):
        clarinet_d4 = ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        notes = {ml.Note("C4"), clarinet_d4, ml.Note("C4", ml.RhythmFigure.HALF)}
        self.assertEqual(len(notes), 1)


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
    # setPitchClass() changes the MIDI number together with the accidental, through the bindings.
    def testSetPitchClassUpdatesAccidentalAndMidi(self):
        note = ml.Note("C4")
        note.setPitchClass("Eb")
        self.assertEqual(note.getAlterSymbol(), "b")
        self.assertEqual(note.getMidiNumber(), 63)

    # Note(pitch, isNoteOn=False) is a fully consistent rest.
    def testConstructorIsNoteOnFalseIsAFullyConsistentRest(self):
        note = ml.Note("C4", isNoteOn=False)
        self.assertFalse(note.isNoteOn())
        self.assertTrue(note.isNoteOff())
        self.assertEqual(note.getPitchClass(), "rest")
        self.assertEqual(note.getPitch(), "rest")
        self.assertEqual(note.getMidiNumber(), -1)
        self.assertEqual(note.getPitchStep(), "rest")
        self.assertIsNone(note.getOctave())  # a rest has no octave: None, not a sentinel

    # setIsNoteOn(False) makes every getter report a rest, not just isNoteOn().
    def testSetIsNoteOnFalseReportsRestEverywhere(self):
        note = ml.Note("C#4")
        note.setIsNoteOn(False)
        self.assertTrue(note.isNoteOff())
        self.assertFalse(note.isNoteOn())
        self.assertEqual(note.getPitchClass(), "rest")
        self.assertEqual(note.getPitch(), "rest")
        self.assertEqual(note.getMidiNumber(), -1)
        self.assertEqual(note.getPitchStep(), "rest")
        self.assertIsNone(note.getOctave())

    # setIsNoteOn(True) on a rest refuses (warns, does not throw) and leaves the rest a rest.
    def testSetIsNoteOnTrueOnRestRefusesAndWarns(self):
        note = ml.Note("")
        self.assertTrue(note.isNoteOff())
        note.setIsNoteOn(True)  # must not raise
        self.assertTrue(note.isNoteOff())
        self.assertFalse(note.isNoteOn())
        self.assertEqual(note.getWrittenPitchStep(), "rest")
        self.assertEqual(note.getPitchClass(), "rest")

    # getAlterSymbol() reads the sounding pitch, not the written one. B-flat clarinet: written
    # "C4" (alter symbol "") transposed by (transposeDiatonic=-1, transposeChromatic=-2) sounds
    # "Bb3" (alter symbol "b") -- the same construction and values pinned in note-test.cpp's
    # NoteComposesPitch.GetAlterSymbolForwardsToSoundingPitchOnTransposedNote. Written "C#4" shows
    # the same in the opposite direction.
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

        # The opposite direction: written "C#4" (alter symbol "#") sounds "B3" (alter symbol "").
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

    # Mirror the C++ NoteComposesPitch.SetAlterOnRestRefusesAndWarns and
    # .SetStepResurrectsRestToOctave4 tests through the bindings.
    def testSetAlterOnRestRefusesAndWarns(self):
        note = ml.Note("rest")
        note.setAlter(0.5)  # must not raise
        self.assertTrue(note.isNoteOff())
        self.assertIsNone(note.getOctave())
        self.assertEqual(note.getPitchClass(), "rest")

    def testSetStepResurrectsRestToOctave4(self):
        note = ml.Note("rest")
        note.setStep("C")
        self.assertTrue(note.isNoteOn())
        self.assertFalse(note.isNoteOff())
        self.assertEqual(note.getPitch(), "C4")
        self.assertEqual(note.getOctave(), 4)

    # Note.transpose() takes a float interval and computes on exact positions, so it transposes by
    # a quarter tone and keeps a quarter tone it transposes. Mirrors the C++ NoteTransposition
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
        self.assertEqual(note.getPitch(), "D1x4")  # not rounded to "D4"
        self.assertTrue(note.isQuarterTone())

    def testTransposeRejectsIntervalOffTheQuarterToneGrid(self):
        note = ml.Note("C4")
        with self.assertRaises(RuntimeError) as ctx:
            note.transpose(0.3)
        self.assertIn("multiple of 0.5", str(ctx.exception))
        self.assertEqual(note.getPitch(), "C4")  # the refused call changed nothing

    # A note transposed below MIDI 0 raises and is left as it was, never silently turned into a
    # rest. Mirrors the C++ NoteTransposition.TransposeOutOfRangeRaisesAndLeavesTheNoteUnchanged
    # test.
    def testTransposeOutOfRangeRaisesAndLeavesTheNoteUnchanged(self):
        note = ml.Note("C4")
        with self.assertRaises(RuntimeError) as ctx:
            note.transpose(-61)
        self.assertIn("outside the representable range", str(ctx.exception).splitlines()[0])
        self.assertEqual(note.getPitch(), "C4")
        self.assertTrue(note.isNoteOn())

    # Silencing a TRANSPOSING instrument with setIsNoteOn(False) makes it a rest but deliberately
    # keeps its transposing interval, so isTransposed() stays True. getSoundingPitch() must still
    # answer the well-formed "rest", never compose the pitch class "rest" with an octave into a
    # malformed string such as "rest-2", which the Note constructor rejects. Mirrors the C++
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

    # A rest has no octave: Note's three octave getters return int | None, None for a rest.
    def testRestHasNoOctave(self):
        rest = ml.Note("", isNoteOn=False)
        self.assertIsNone(rest.getOctave())
        self.assertIsNone(rest.getWrittenOctave())
        self.assertIsNone(rest.getSoundingOctave())
        self.assertEqual(ml.Note("C#4").getOctave(), 4)

    # The rest check in Pitch::setOctave() (reached through Note::setOctave()) runs before the
    # range check, so a rest given an out-of-range octave warns and does not mutate, instead of
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

    # The round trip setOctave(getOctave()) on a rest, which C++ cannot even express
    # (Note::setOctave(int) has no overload accepting the empty optional getOctave() returns for
    # a rest, so the equivalent C++ call is a compile error). In Python it raises TypeError from
    # the pybind11 argument conversion (None cannot become int), and mutates nothing.
    def testSetOctaveOfGetOctaveRoundTripOnRestRaisesTypeError(self):
        note = ml.Note("rest")
        self.assertIsNone(note.getOctave())
        with self.assertRaises(TypeError):
            note.setOctave(note.getOctave())
        self.assertTrue(note.isNoteOff())
        self.assertIsNone(note.getOctave())

    # See note-test.cpp's NoteComposesPitch.SetStepAfterSetIsNoteOnFalseKeepsTransposingInterval
    # for the full reasoning, including why the note starts at C5: from C4, "octave preserved"
    # and "octave defaulted to 4" by setStep() would look the same. What is pinned: the revived
    # note is SELF-CONSISTENT (freshly derived from its current, defaulted written pitch and the
    # surviving interval), not a stale leftover. Not "revived equals a note that already carries
    # the interval" -- that would assume the answer.
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


def noteState(note):
    """Everything a mutator can change: the written pitch, the transposing interval, the sounding
    pitch (or the first line of the error a sounding getter raises), the duration, and the
    MusicXML the note writes, which also carries its tuplet values."""
    try:
        sounding = note.getPitch()
    except RuntimeError as error:
        sounding = "raises: " + firstLine(error)
    return (
        note.getWrittenPitch(),
        note.getTransposeDiatonic(),
        note.getTransposeChromatic(),
        sounding,
        note.getDurationTicks(),
        note.getLongType(),
        note.toXML(),
    )


def aboveTheCeiling(written, diatonic, chromatic, position):
    """The error for 'written' on a (diatonic, chromatic) instrument, whose sounding pitch lies at
    'position' (as C++'s std::to_string() writes it), above B11, the highest sounding pitch that
    can be spelled within octaves -1..11."""
    return (
        f"[maiacore] The sounding pitch of the written pitch '{written}' with "
        f"transposeDiatonic={diatonic} and transposeChromatic={chromatic} is at position "
        f"{position}, above B11 (MIDI note 155), the highest sounding pitch that can be spelled "
        "within octaves -1..11, so it has no sounding spelling. A lower written pitch or a "
        "smaller transposing interval keeps the sounding pitch at or below B11."
    )


class NoteMutatorThatRaises(unittest.TestCase):
    """A Note mutator that raises leaves the note exactly as it was. Mirrors the C++
    NoteMutatorThatThrows tests."""

    def assertRaisesLeavingTheNote(self, note, message, mutator, *arguments):
        """mutator(*arguments) raises RuntimeError with this message, and the note is as before."""
        before = noteState(note)
        with self.assertRaises(RuntimeError) as context:
            mutator(*arguments)
        self.assertEqual(firstLine(context.exception), message)
        self.assertEqual(noteState(note), before)

    def testSetTransposingIntervalLeavesTheNoteUnchanged(self):
        note = ml.Note("B11")
        self.assertRaisesLeavingTheNote(
            note, aboveTheCeiling("B11", 1, 3, "158.000000"), note.setTransposingInterval, 1, 3
        )
        self.assertEqual(note.getPitch(), "B11")

    def testSetPitchLeavesTheNoteUnchanged(self):
        note = ml.Note("C4", transposeDiatonic=1, transposeChromatic=3)
        self.assertRaisesLeavingTheNote(
            note, aboveTheCeiling("B11", 1, 3, "158.000000"), note.setPitch, "B11"
        )
        self.assertEqual(note.getPitch(), "D#4")

    def testTransposeLeavesTheNoteUnchanged(self):
        note = ml.Note("C4", transposeDiatonic=1, transposeChromatic=3)
        self.assertRaisesLeavingTheNote(
            note, aboveTheCeiling("A11", 1, 3, "156.000000"), note.transpose, 90
        )
        self.assertEqual(note.getPitch(), "D#4")

    def testToEnharmonicPitchLeavesTheNoteUnchanged(self):
        for alternative in (False, True):
            with self.subTest(alternative=alternative):
                note = ml.Note("A11", transposeDiatonic=1, transposeChromatic=2)
                # The default respelling of the sounding B11 is B11 itself (no flat partner
                # within octave 11), the alternative Ax11; each sounds at 157 once written.
                self.assertRaisesLeavingTheNote(
                    note,
                    aboveTheCeiling("Ax11" if alternative else "B11", 1, 2, "157.000000"),
                    note.toEnharmonicPitch,
                    alternative,
                )
                self.assertEqual(note.getPitch(), "B11")

    def testSetDurationLeavesTheNoteUnchanged(self):
        for duration in (0.0, -1.0):
            with self.subTest(duration=duration):
                note = ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2)
                self.assertRaisesLeavingTheNote(
                    note,
                    "[maiacore] Unable to convert durationTick to RhythmFigure",
                    note.setDuration,
                    duration,
                )
                self.assertEqual(note.getDurationTicks(), 256)

    def testSetTupleValuesLeavesTheNoteUnchanged(self):
        note = ml.Note("C4")
        note.setIsTuplet(True)
        note.setTupleValues(3, 2, "eighth")
        self.assertRaisesLeavingTheNote(
            note, "[maiacore] Unknown note type: garbage", note.setTupleValues, 5, 4, "garbage"
        )
        self.assertIn("<actual-notes>3</actual-notes>", note.toXML())

    # The partial pitch setters check the resulting sounding pitch as setPitch() checks a whole
    # pitch, and raise the error setPitch() raises for that pitch. Mirrors the C++
    # NoteMutatorThatThrows.*ChecksTheSoundingPitchLikeSetPitch tests.
    def assertChecksLikeSetPitch(self, make, pitch, message, mutator, *arguments):
        reference = make()
        with self.assertRaises(RuntimeError) as context:
            reference.setPitch(pitch)
        self.assertEqual(firstLine(context.exception), message)
        note = make()
        self.assertRaisesLeavingTheNote(note, message, getattr(note, mutator), *arguments)
        return note

    def testSetStepChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("C11", transposeDiatonic=1, transposeChromatic=3),
            "A11",
            aboveTheCeiling("A11", 1, 3, "156.000000"),
            "setStep",
            "A",
        )
        self.assertEqual(note.getPitch(), "D#11")

    def testSetPitchClassChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("C11", transposeDiatonic=1, transposeChromatic=3),
            "A11",
            aboveTheCeiling("A11", 1, 3, "156.000000"),
            "setPitchClass",
            "A",
        )
        self.assertEqual(note.getPitch(), "D#11")

    def testSetOctaveChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("B4", transposeDiatonic=1, transposeChromatic=3),
            "B11",
            aboveTheCeiling("B11", 1, 3, "158.000000"),
            "setOctave",
            11,
        )
        self.assertEqual(note.getPitch(), "D5")

    def testSetAlterChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("A11", transposeDiatonic=1, transposeChromatic=2),
            "A#11",
            aboveTheCeiling("A#11", 1, 2, "156.000000"),
            "setAlter",
            1,
        )
        self.assertEqual(note.getPitch(), "B11")

    def testThePartialSettersStillAcceptASoundingPitchBelowTheFloor(self):
        for written, diatonic, chromatic, mutator, argument, result in (
            ("D#-1", -1, -3, "setStep", "C", "C#-1"),
            ("D#-1", -1, -3, "setPitchClass", "C#", "C#-1"),
            ("C#0", -7, -12, "setOctave", -1, "C#-1"),
            ("D-1", -1, -2, "setAlter", -1, "Db-1"),
        ):
            with self.subTest(mutator=mutator):
                note = ml.Note(written, transposeDiatonic=diatonic, transposeChromatic=chromatic)
                getattr(note, mutator)(argument)
                self.assertEqual(note.getWrittenPitch(), result)
                with self.assertRaises(RuntimeError) as context:
                    note.getPitch()
                self.assertIn(
                    "below the lowest representable pitch C1b-1", firstLine(context.exception)
                )

    def testARestIsNotGivenAPitchThatCannotSound(self):
        # A rest keeps its transposing interval: B4 on a (0, 90) instrument would sound at 161.
        for mutator in ("setStep", "setPitchClass"):
            with self.subTest(mutator=mutator):
                rest = ml.Note("C4", transposeDiatonic=0, transposeChromatic=90)
                rest.setIsNoteOn(False)
                self.assertRaisesLeavingTheNote(
                    rest,
                    aboveTheCeiling("B4", 0, 90, "161.000000"),
                    getattr(rest, mutator),
                    "B",
                )
                self.assertTrue(rest.isNoteOff())
                self.assertEqual(rest.getTransposeChromatic(), 90)


if __name__ == "__main__":
    unittest.main()
