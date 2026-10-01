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

    # A B-flat clarinet's written C1x4 sounds B1b3; the family respells the written pitch.
    def testTheWrittenPitchIsRespelled(self):
        clarinet = ml.Note("C1x4", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertEqual(clarinet.getPitch(), "C1x4")
        self.assertEqual(clarinet.getSoundingPitch(), "B1b3")
        self.assertEqual(enharmonicsOf(clarinet), ("D3b4", "B3x3"))

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

    # getAlterSymbol() reads the written pitch, like every unprefixed pitch getter. B-flat
    # clarinet: written "C4" (alter symbol "") transposed by (transposeDiatonic=-1,
    # transposeChromatic=-2) sounds "Bb3" -- the same construction and values pinned in
    # note-test.cpp's NoteComposesPitch.GetAlterSymbolIsTheWrittenAccidentalOnTransposedNote.
    # Written "C#4" shows the same in the opposite direction.
    def testGetAlterSymbolIsTheWrittenAccidentalOnTransposedNote(self):
        transposed = ml.Note(
            "C4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertEqual(transposed.getSoundingPitch(), "Bb3")
        self.assertEqual(transposed.getMidiNumber(), 58)
        self.assertEqual(transposed.getAlterSymbol(), "")
        self.assertEqual(transposed.getSoundingPitchClass(), "Bb")

        # The opposite direction: written "C#4" (alter symbol "#") sounds "B3".
        sharpTransposed = ml.Note(
            "C#4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertEqual(sharpTransposed.getSoundingPitch(), "B3")
        self.assertEqual(sharpTransposed.getMidiNumber(), 59)
        self.assertEqual(sharpTransposed.getAlterSymbol(), "#")
        self.assertEqual(sharpTransposed.getSoundingPitchClass(), "B")

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
    # keeps its transposing interval, so isTransposed() stays True. Every view still answers the
    # well-formed "rest", never a malformed string such as "rest-2", which the Note constructor
    # rejects: getSoundingPitch() spells a whole sounding pitch, and the sounding pitch of a rest
    # is the rest itself, whatever the interval; getPitch() and getWrittenPitch() answer the
    # written pitch, a rest. Mirrors the C++
    # NoteComposesPitch.GetPitchIsWellFormedRestForTransposedNoteTurnedOff test.
    def testSilencedTransposedNoteReportsWellFormedRest(self):
        note = ml.Note(
            "C#4", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        note.setIsNoteOn(False)
        self.assertTrue(note.isTransposed())  # the interval survives the silencing
        self.assertTrue(note.isNoteOff())
        self.assertEqual(note.getPitch(), "rest")
        self.assertEqual(note.getSoundingPitch(), "rest")
        self.assertEqual(note.getWrittenPitch(), "rest")
        self.assertEqual(note.getMidiNumber(), -1)

    # A written "C#-1" on a B-flat clarinet sounds below the lowest representable pitch, C1b-1.
    # The note is constructible and is a note, not a rest; its sounding pitch fails loudly and
    # diagnosably, not as a rest and not with an unexplained internal error. Its written pitch
    # still answers.
    def testGetSoundingPitchBelowMidiZeroFailsDiagnosably(self):
        note = ml.Note(
            "C#-1", isNoteOn=True, inChord=False, transposeDiatonic=-1, transposeChromatic=-2
        )
        self.assertTrue(note.isNoteOn())
        self.assertFalse(note.isNoteOff())
        self.assertEqual(note.getPitch(), "C#-1")

        with self.assertRaises(RuntimeError) as ctx:
            note.getSoundingPitch()
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
    # Mirrors NoteSoundingPitchBelowFloor.everySoundingGetterFailsTheSameWay: the Sounding view and
    # the acoustic getters.
    GETTERS = (
        "getSoundingPitch",
        "getSoundingPitchClass",
        "getSoundingPitchStep",
        "getDiatonicSoundingPitchClass",
        "getSoundingOctave",
        "getMidiNumber",
        "getQuarterToneSteps",
        "getFrequency",
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

    # The written getters, the unprefixed ones that are shortcuts for them, and the enharmonic
    # family, which respells the written pitch, all answer.
    def testTheWrittenPitchStillAnswers(self):
        note = ml.Note("C#-1", transposeDiatonic=-1, transposeChromatic=-2)
        self.assertTrue(note.isNoteOn())
        self.assertEqual(note.getWrittenPitch(), "C#-1")
        self.assertEqual(note.getWrittenOctave(), -1)
        self.assertEqual(note.getPitch(), "C#-1")
        self.assertEqual(note.getPitchClass(), "C#")
        self.assertEqual(note.getPitchStep(), "C")
        self.assertEqual(note.getAlterSymbol(), "#")
        self.assertEqual(note.getOctave(), -1)
        self.assertEqual(note.getEnharmonicPitch(), "Db-1")
        self.assertEqual(repr(note), "<Note C#-1>")
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
        sounding = note.getSoundingPitch()
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
    'position' (as C++'s std::to_string() writes it), above B11, where no spelling within octaves
    -1..11 reaches it: only B1x11, B#11, B3x11 and Bx11 lie above B11, and only a diatonic
    interval that moves the written letter to the B of octave 11 spells them."""
    return (
        f"[maiacore] The sounding pitch of the written pitch '{written}' with "
        f"transposeDiatonic={diatonic} and transposeChromatic={chromatic} is at position "
        f"{position}, above B11 (MIDI note 155), and has no sounding spelling within octaves "
        "-1..11: above B11 only B1x11, B#11, B3x11 and Bx11 can be spelled, when the diatonic "
        "interval moves the written letter to the B of octave 11. A lower written pitch or a "
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
        self.assertEqual(note.getSoundingPitch(), "D#4")

    # The written C4 moved up 95 semitones is B11, which on a (1, 3) instrument would sound at
    # 158, above Bx11 (157), the highest pitch any letter spells.
    def testTransposeLeavesTheNoteUnchanged(self):
        note = ml.Note("C4", transposeDiatonic=1, transposeChromatic=3)
        self.assertRaisesLeavingTheNote(
            note, aboveTheCeiling("B11", 1, 3, "158.000000"), note.transpose, 95
        )
        self.assertEqual(note.getSoundingPitch(), "D#4")

    def testToEnharmonicPitchLeavesTheNoteUnchanged(self):
        for alternative in (False, True):
            with self.subTest(alternative=alternative):
                note = ml.Note("A#11", transposeDiatonic=1, transposeChromatic=2)
                # A#11 sounds B#11, a spelling its letter reaches; its respelling Bb11 (the
                # default, and the alternative too: A#11 has no double-accidental spelling within
                # octave 11) would need the letter C of octave 12.
                self.assertRaisesLeavingTheNote(
                    note,
                    aboveTheCeiling("Bb11", 1, 2, "156.000000"),
                    note.toEnharmonicPitch,
                    alternative,
                )
                self.assertEqual(note.getSoundingPitch(), "B#11")

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
            "B11",
            aboveTheCeiling("B11", 1, 3, "158.000000"),
            "setStep",
            "B",
        )
        self.assertEqual(note.getSoundingPitch(), "D#11")

    def testSetPitchClassChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("C11", transposeDiatonic=1, transposeChromatic=3),
            "B11",
            aboveTheCeiling("B11", 1, 3, "158.000000"),
            "setPitchClass",
            "B",
        )
        self.assertEqual(note.getSoundingPitch(), "D#11")

    def testSetOctaveChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("B4", transposeDiatonic=1, transposeChromatic=3),
            "B11",
            aboveTheCeiling("B11", 1, 3, "158.000000"),
            "setOctave",
            11,
        )
        self.assertEqual(note.getSoundingPitch(), "D5")

    # Bb11 moved up a semitone without a diatonic interval sounds B11: the minor second inferred
    # for that semitone would reach Cb12, so the chromatic rule spells it. B11 moved up a semitone
    # (156) can be spelled by no letter.
    def testSetAlterChecksTheSoundingPitchLikeSetPitch(self):
        note = self.assertChecksLikeSetPitch(
            lambda: ml.Note("Bb11", transposeDiatonic=0, transposeChromatic=1),
            "B11",
            aboveTheCeiling("B11", 0, 1, "156.000000"),
            "setAlter",
            0,
        )
        self.assertEqual(note.getSoundingPitch(), "B11")

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
                    note.getSoundingPitch()
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


def transposing(written, diatonic, chromatic):
    """A note written 'written' in the part of an instrument that sounds 'diatonic' letters and
    'chromatic' semitones away from what it reads."""
    return ml.Note(written, transposeDiatonic=diatonic, transposeChromatic=chromatic)


class NotePitchViews(unittest.TestCase):
    """Written (the unprefixed getters are shortcuts for it), Sounding (the simplest spelling of
    what sounds) and acoustic. Mirrors the C++ NotePitchViews tests. The design these tests cite,
    with its decisions D1-D7 and the examples of its section 3, is
    docs/superpowers/specs/2026-09-30-note-pitch-views-design.md."""

    # The design's section 3 through the public API, plus decision D4's tie example.
    def testTheSpecExamplesTable(self):
        rows = (
            (ml.Note("Cb4"), "Cb4", "Cb4", "B3", 3, 59),
            (transposing("F#4", -1, -2), "F#4", "E4", "E4", 4, 64),
            (transposing("Db4", -1, -2), "Db4", "Cb4", "B3", 3, 59),
            (transposing("C1x4", -1, -2), "C1x4", "B1b3", "B1b3", 3, 59),
            (transposing("B4", -4, -7), "B4", "E4", "E4", 4, 64),
            (transposing("Bb4", 7, 12), "Bb4", "Bb5", "Bb5", 5, 82),
            (transposing("C4", 0, -2), "C4", "Bb3", "Bb3", 3, 58),
            (transposing("F#4", 0, -2), "F#4", "E4", "E4", 4, 64),
            (transposing("Cbb4", -1, -2), "Cbb4", "Ab3", "Ab3", 3, 56),
            (transposing("Eb4", -1, -2), "Eb4", "Db4", "Db4", 4, 61),
        )
        for note, written, concert, sounding, sounding_octave, midi_number in rows:
            with self.subTest(written=written, interval=note.getTransposeChromatic()):
                pitch = ml.Pitch(written)
                self.assertEqual(note.getPitch(), written)
                self.assertEqual(note.getWrittenPitch(), written)
                self.assertEqual(note.getOctave(), pitch.getOctave())
                self.assertEqual(note.getPitchClass(), pitch.getPitchClass())
                self.assertEqual(note.getPitchStep(), pitch.getPitchStep())
                self.assertEqual(note.getAlterSymbol(), pitch.getAlterSymbol())
                self.assertTrue(note == ml.Note(concert))
                self.assertEqual(note.getSoundingPitch(), sounding)
                self.assertEqual(note.getSoundingOctave(), sounding_octave)
                self.assertEqual(note.getMidiNumber(), midi_number)

        silenced = transposing("C4", -1, -2)
        silenced.setIsNoteOn(False)
        for rest in (ml.Note("rest"), silenced):
            self.assertEqual(rest.getPitch(), "rest")
            self.assertEqual(rest.getSoundingPitch(), "rest")
            self.assertIsNone(rest.getOctave())
            self.assertIsNone(rest.getSoundingOctave())
            self.assertEqual(rest.getMidiNumber(), -1)

    # Decision D3: an untransposed note sounds its simplest spelling, with that spelling's octave.
    # Mirrors NotePitchViews.anUntransposedNoteSoundsItsSimplestSpelling: every example the design
    # gives, then B1x3, B3x3 and Bx11, whose octave is the simplest spelling's own.
    def testAnUntransposedNoteSoundsItsSimplestSpelling(self):
        for written, sounding, octave in (
            ("Db4", "Db4", 4),
            ("C#4", "C#4", 4),
            ("Cb4", "B3", 3),
            ("E#4", "F4", 4),
            ("B#3", "C4", 4),
            ("Fb4", "E4", 4),
            ("Ebb4", "D4", 4),
            ("Fx4", "G4", 4),
            ("Bbb4", "A4", 4),
            ("C1x4", "C1x4", 4),
            ("C3x4", "D1b4", 4),
            ("E1x4", "E1x4", 4),
            ("B1b3", "B1b3", 3),
            ("B1x3", "B1x3", 3),
            ("B3x3", "C1x4", 4),
            ("Bx11", "Bx11", 11),
        ):
            with self.subTest(written=written):
                note = ml.Note(written)
                self.assertEqual(note.getPitch(), written)
                self.assertEqual(note.getSoundingPitch(), sounding)
                self.assertEqual(note.getSoundingOctave(), octave)

    # Decision D6: transpose() moves the written pitch once.
    def testTransposeMovesTheWrittenPitchOnce(self):
        clarinet = transposing("C4", -1, -2)
        clarinet.transpose(0)
        self.assertEqual(clarinet.getWrittenPitch(), "C4")
        self.assertEqual(clarinet.getSoundingPitch(), "Bb3")

        clarinet.transpose(2)
        self.assertEqual(clarinet.getWrittenPitch(), "D4")
        self.assertEqual(clarinet.getSoundingPitch(), "C4")
        self.assertEqual(clarinet.getMidiNumber(), 60)

    # Decision D6: toEnharmonicPitch() respells the written pitch, so the note sounds as before.
    def testToEnharmonicPitchKeepsWhatATransposedNoteSounds(self):
        for alternative, respelled in ((False, "Db4"), (True, "Bx3")):
            with self.subTest(alternative=alternative):
                clarinet = transposing("C#4", -1, -2)
                clarinet.toEnharmonicPitch(alternative)
                self.assertEqual(clarinet.getWrittenPitch(), respelled)
                self.assertEqual(clarinet.getMidiNumber(), 59)
                self.assertEqual(clarinet.getSoundingPitch(), "B3")

    # Mirrors NotePitchViews.enharmonicNotesKeepTheTransposingInterval: each enharmonic note is a
    # respelling of the written pitch that keeps the transposing interval, so it sounds what the
    # note sounds.
    def testEnharmonicNotesKeepTheTransposingInterval(self):
        for note, written in (
            (transposing("D4", -1, -2), ["D4", "Ebb4", "Cx4"]),
            (transposing("C1x4", -1, -2), ["C1x4", "D3b4", "B3x3"]),
            (transposing("F#4", -4, -7), ["F#4", "Gb4", "Ex4"]),
        ):
            respelled = (
                note.getEnharmonicNotes(True)
                + note.getEnharmonicNotes()
                + [note.getEnharmonicNote(), note.getEnharmonicNote(True)]
            )
            self.assertEqual([other.getPitch() for other in respelled], written + written[1:] * 2)
            for other in respelled:
                with self.subTest(written=note.getPitch(), respelled=other.getPitch()):
                    self.assertEqual(other.getTransposeDiatonic(), note.getTransposeDiatonic())
                    self.assertEqual(other.getTransposeChromatic(), note.getTransposeChromatic())
                    self.assertEqual(other.getMidiNumber(), note.getMidiNumber())
                    self.assertEqual(other.getQuarterToneSteps(), note.getQuarterToneSteps())
                    self.assertEqual(other.getSoundingPitch(), note.getSoundingPitch())

    # Mirrors NotePitchViews.anEnharmonicNoteThatCannotBeSpelledIsRejected: A#11 a major second up
    # sounds B#11, but its respelling Bb11 would need the letter C of octave 12.
    def testAnEnharmonicNoteThatCannotBeSpelledIsRejected(self):
        note = transposing("A#11", 1, 2)
        self.assertEqual(note.getSoundingPitch(), "B#11")
        for alternative in (False, True):
            with self.subTest(alternative=alternative):
                with self.assertRaises(RuntimeError) as context:
                    note.getEnharmonicNote(alternative)
                self.assertEqual(
                    firstLine(context.exception), aboveTheCeiling("Bb11", 1, 2, "156.000000")
                )

    # Above B11 the diatonic interval spells B#11 and Bx11, and every entry point accepts them.
    def testASoundingPitchTheDiatonicIntervalSpellsAboveB11IsAccepted(self):
        self.assertEqual(transposing("A#11", 1, 2).getSoundingPitch(), "B#11")
        self.assertEqual(transposing("A#11", 1, 3).getSoundingPitch(), "Bx11")
        self.assertEqual(transposing("A#11", 1, 2).getMidiNumber(), 156)

        note = ml.Note("A#11")
        note.setTransposingInterval(1, 2)
        self.assertEqual(note.getSoundingPitch(), "B#11")

        for start, mutator, argument, sounding in (
            ("C4", "setPitch", "A#11", "B#11"),
            ("G#11", "setStep", "A", "B#11"),
            ("G11", "setPitchClass", "A#", "B#11"),
            ("A#10", "setOctave", 11, "B#11"),
            ("A11", "setAlter", 2, "Bx11"),
        ):
            with self.subTest(mutator=mutator):
                note = transposing(start, 1, 2)
                getattr(note, mutator)(argument)
                self.assertEqual(note.getSoundingPitch(), sounding)

    # Mirrors NotePitchViews.anInferredDiatonicIntervalSpellsAboveB11Too: two semitones up without
    # a diatonic interval are read as a major second, so a written A#11 sounds B#11, while the
    # note keeps the diatonic interval it was given, 0.
    def testAnInferredDiatonicIntervalSpellsAboveB11Too(self):
        sharp = transposing("A#11", 0, 2)
        self.assertEqual(sharp.getSoundingPitch(), "B#11")
        self.assertEqual(sharp.getMidiNumber(), 156)
        self.assertEqual(sharp.getTransposeDiatonic(), 0)

        note = ml.Note("A#11")
        note.setTransposingInterval(0, 2)
        self.assertEqual(note.getSoundingPitch(), "B#11")

        for start, mutator, argument in (("A11", "setAlter", 1), ("C4", "setPitch", "A#11")):
            with self.subTest(mutator=mutator):
                note = transposing(start, 0, 2)
                getattr(note, mutator)(argument)
                self.assertEqual(note.getSoundingPitch(), "B#11")

    # Mirrors NotePitchViews.aChromaticIntervalAloneIsReadWithItsConventionalDiatonicInterval
    # (decision D4): a note given only transposeChromatic is read with the diatonic interval
    # conventionally written for those semitones -- two semitones down are a B-flat clarinet's
    # major second -- and keeps the transposeDiatonic it was given, 0.
    def testAChromaticIntervalAloneIsReadWithItsConventionalDiatonicInterval(self):
        note = ml.Note("F#4", transposeChromatic=-2)
        self.assertEqual(note.getTransposeDiatonic(), 0)
        self.assertEqual(note.getTransposeChromatic(), -2)
        self.assertTrue(note.isTransposed())
        self.assertEqual(note.getPitch(), "F#4")
        self.assertEqual(note.getSoundingPitch(), "E4")
        self.assertEqual(note.getMidiNumber(), 64)
        self.assertTrue(note == ml.Note("E4"))
        self.assertTrue(note != ml.Note("Fb4"))
        self.assertEqual(hash(note), hash(ml.Note("E4")))

        # A semitone is a minor second, in either direction.
        self.assertEqual(ml.Note("D4", transposeChromatic=1).getSoundingPitch(), "Eb4")
        self.assertEqual(ml.Note("D4", transposeChromatic=-1).getSoundingPitch(), "C#4")

    # The diatonic interval inferred for every number of semitones within an octave, in both
    # directions, and for compound intervals, compared at the spelling == and the analyses use: a
    # written C4 moved by it. Six semitones are an augmented fourth, not a diminished fifth.
    # Mirrors the C++ ConcertSpelling tests of the inferred interval.
    def testEverySemitoneCountIsReadWithItsConventionalDiatonicInterval(self):
        for chromatic, concert in (
            (1, "Db4"),
            (-1, "B3"),
            (2, "D4"),
            (-2, "Bb3"),
            (3, "Eb4"),
            (-3, "A3"),
            (4, "E4"),
            (-4, "Ab3"),
            (5, "F4"),
            (-5, "G3"),
            (6, "F#4"),
            (-6, "Gb3"),
            (7, "G4"),
            (-7, "F3"),
            (8, "Ab4"),
            (-8, "E3"),
            (9, "A4"),
            (-9, "Eb3"),
            (10, "Bb4"),
            (-10, "D3"),
            (11, "B4"),
            (-11, "Db3"),
            (12, "C5"),
            (-12, "C3"),
            (14, "D5"),
            (-14, "Bb2"),
            (21, "A5"),
            (-21, "Eb2"),
            (24, "C6"),
            (-24, "C2"),
        ):
            with self.subTest(chromatic=chromatic):
                note = ml.Note("C4", transposeChromatic=chromatic)
                self.assertTrue(
                    note == ml.Note(concert), f"C4 moved by {chromatic} is not {concert}"
                )
                self.assertEqual(note.getTransposeDiatonic(), 0)

        tritone = ml.Note("B3", transposeChromatic=6)
        self.assertTrue(tritone == ml.Note("E#4"))
        self.assertEqual(tritone.getSoundingPitch(), "F4")

    # getScaleDegree() reads the written step, in the key the part is written in.
    def testGetScaleDegreeReadsTheWrittenStep(self):
        self.assertEqual(transposing("D4", -1, -2).getScaleDegree(ml.Key("C")), 2)
        self.assertEqual(transposing("G4", -4, -7).getScaleDegree(ml.Key("C")), 5)

    # repr() shows the written pitch, the one the note was constructed with, and never raises.
    def testReprShowsTheWrittenPitch(self):
        self.assertEqual(repr(transposing("D4", -1, -2)), "<Note D4>")
        self.assertEqual(repr(transposing("C#-1", -1, -2)), "<Note C#-1>")
        self.assertEqual(repr(ml.Note("Cb4")), "<Note Cb4>")

    # Decision D7: the Sounding getters document their silent fallback.
    def testEverySoundingGetterDocumentsTheFallback(self):
        for name in (
            "getSoundingPitch",
            "getSoundingPitchClass",
            "getSoundingPitchStep",
            "getDiatonicSoundingPitchClass",
            "getSoundingOctave",
        ):
            with self.subTest(getter=name):
                doc = " ".join(getattr(ml.Note, name).__doc__.split()).lower()
                self.assertIn("fallback", doc)
                self.assertIn("silent", doc)

    # Decision D7: the Sounding getters document the diatonic interval they infer when only the
    # chromatic one is given.
    def testEverySoundingGetterDocumentsTheInferredDiatonicInterval(self):
        for name in (
            "getSoundingPitch",
            "getSoundingPitchClass",
            "getSoundingPitchStep",
            "getDiatonicSoundingPitchClass",
            "getSoundingOctave",
        ):
            with self.subTest(getter=name):
                doc = " ".join(getattr(ml.Note, name).__doc__.split()).lower()
                self.assertIn("inferred diatonic interval", doc)


if __name__ == "__main__":
    unittest.main()
