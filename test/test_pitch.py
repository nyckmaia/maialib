import contextlib
import io
import math
import struct
import unittest

import maialib as ml

# ===== TEST PITCH CLASS ===== #
#
# Task 11: Python parity for the C++ Pitch class (tests-cpp/src/pitch-test.cpp), exercised through
# the real binding. Each test names the C++ test it mirrors. Three C++ tests have no Python
# counterpart, on purpose: Characterization.semitoneBehaviourIsUnchanged characterises
# Helper.pitch2midiNote() and Note.getEnharmonicPitch() against a 453-entry C++ data table rather
# than Pitch, and the two *ThrowsForNonEqualTemperament tests need a tuning system other than equal
# temperament, which Python cannot select (the tuning system is not bound).


def firstLine(exception):
    """The message proper: the lines after it are a stack trace."""
    return str(exception).splitlines()[0]


def capturedStdout(action):
    """Run action() and return what it printed. The binding redirects the C++ LOG_WARN output
    (std::cout) to sys.stdout, so a refusal's warning is observable from Python."""
    buffer = io.StringIO()
    with contextlib.redirect_stdout(buffer):
        action()
    return buffer.getvalue()


def float32(value):
    """Round a Python float to the nearest C++ float, the type every frequency travels as."""
    return struct.unpack("f", struct.pack("f", value))[0]


class PitchConstruction(unittest.TestCase):
    # The user's decision (Task 11, section A): the pitch string is the ONLY constructor. The
    # int (MIDI) and float (frequency) C++ constructors are the named factories fromMidi() and
    # fromFrequency(), because as __init__ overloads pybind11 would pick between them by
    # int-versus-float alone, and ml.Pitch(110) meant as 110 Hz would silently become MIDI 110.
    def testNumericConstructorIsRejected(self):
        for value in (60, 110, 440.0):
            with self.subTest(value=value), self.assertRaises(TypeError):
                ml.Pitch(value)

    def testDefaultConstructorIsARest(self):
        self.assertTrue(ml.Pitch().isRest())
        self.assertEqual(ml.Pitch().getPitch(), "rest")

    def testFactoriesDisambiguateMidiFromFrequency(self):
        self.assertEqual(ml.Pitch.fromMidi(110).getPitch(), "D8")
        self.assertEqual(ml.Pitch.fromFrequency(110).getPitch(), "A2")

    def testFromMidiRejectsAFloat(self):
        with self.assertRaises(TypeError):
            ml.Pitch.fromMidi(60.0)
        self.assertEqual(ml.Pitch.fromMidi(60).getPitch(), "C4")

    def testFromFrequencyAcceptsAnInt(self):
        self.assertEqual(ml.Pitch.fromFrequency(440).getPitch(), "A4")
        self.assertEqual(ml.Pitch.fromFrequency(440.0).getPitch(), "A4")

    # Mirrors Pitch.midiNumberConstructorHonoursAccType.
    def testFromMidiHonoursAccType(self):
        self.assertEqual(ml.Pitch.fromMidi(61, "b").getPitch(), "Db4")
        self.assertEqual(ml.Pitch.fromMidi(61, "#").getPitch(), "C#4")
        self.assertTrue(ml.Pitch.fromMidi(-1).isRest())

    def testFromMidiRejectsAnAccTypeThatCannotSpellTheNote(self):
        # MIDI 60 is a white key: it has no flat spelling.
        with self.assertRaises(RuntimeError) as context:
            ml.Pitch.fromMidi(60, "b")
        self.assertIn("'60'", firstLine(context.exception))

    def testIsTheCompiledClassNotShadowed(self):
        # maialib/__init__.py re-exports maiapy after maiacore; nothing there may shadow Pitch.
        self.assertIs(ml.Pitch, ml.maiacore.Pitch)

    def testRepr(self):
        self.assertEqual(repr(ml.Pitch("C1x4")), "<Pitch C1x4>")
        self.assertEqual(repr(ml.Pitch()), "<Pitch rest>")


class PitchBindingSurface(unittest.TestCase):
    # Every public member of maiacore/pitch.h is bound, and each carries a numpydoc docstring with
    # an Examples section (Task 11, section B).
    EXPECTED = {
        "fromMidi",
        "fromFrequency",
        "maxRepresentableMidi",
        "clampToRepresentableMidi",
        "getPitch",
        "getPitchClass",
        "getPitchStep",
        "getAlterSymbol",
        "getAlter",
        "getOctave",
        "getMidiNumber",
        "getQuarterToneSteps",
        "getFrequency",
        "isRest",
        "setStep",
        "setAlter",
        "setOctave",
        "setPitch",
        "setPitchClass",
        "setMidiNumber",
        "setFrequency",
        "roundToSemitone",
    }

    def testBindsThePublicSurface(self):
        public = {name for name in dir(ml.Pitch) if not name.startswith("_")}
        self.assertEqual(public, self.EXPECTED)

    def testEveryMemberIsDocumentedWithAnExample(self):
        for name in sorted(self.EXPECTED | {"__init__"}):
            with self.subTest(member=name):
                self.assertIn("Examples\n", getattr(ml.Pitch, name).__doc__)


class PitchComponents(unittest.TestCase):
    # Mirrors Pitch.quarterToneStringRoundTrip.
    def testQuarterToneStringRoundTrip(self):
        pitch = ml.Pitch("C1x4")
        self.assertEqual(pitch.getPitchStep(), "C")
        self.assertEqual(pitch.getAlter(), 0.5)
        self.assertEqual(pitch.getOctave(), 4)
        self.assertEqual(pitch.getPitch(), "C1x4")
        self.assertEqual(pitch.getMidiNumber(), 61)  # ties round up
        self.assertEqual(pitch.getQuarterToneSteps(), 60.5)

    def testPitchClassAndAlterSymbol(self):
        pitch = ml.Pitch("D3b4")
        self.assertEqual(pitch.getPitchClass(), "D3b")
        self.assertEqual(pitch.getAlterSymbol(), "3b")
        self.assertEqual(pitch.getAlter(), -1.5)
        self.assertEqual(pitch.getQuarterToneSteps(), 60.5)

    # Mirrors Pitch.restHasNoOctave: a rest has no octave, and Python sees None, not a sentinel.
    def testRestHasNoOctave(self):
        self.assertIsNone(ml.Pitch("rest").getOctave())
        rest = ml.Pitch("rest")
        self.assertTrue(rest.isRest())
        self.assertEqual(rest.getMidiNumber(), -1)
        self.assertEqual(rest.getPitch(), "rest")
        self.assertEqual(rest.getPitchClass(), "rest")
        self.assertEqual(rest.getAlterSymbol(), "")

    def testEveryRestSpellingIsARest(self):
        self.assertTrue(ml.Pitch("").isRest())
        self.assertFalse(ml.Pitch("C4").isRest())

    def testInvalidPitchStringRaises(self):
        with self.assertRaises(RuntimeError) as context:
            ml.Pitch("H4")
        self.assertIn("Unknown diatonic pitch", firstLine(context.exception))

        with self.assertRaises(RuntimeError) as context:
            ml.Pitch("Cb-1")
        self.assertIn("below MIDI note 0", firstLine(context.exception))

    # Mirrors Pitch.midiNumberRoundsHalfUpOnFlatSide: every rounding test on the sharp side agrees
    # with ties-away-from-zero too; only the flat side tells the two rules apart.
    def testMidiNumberRoundsHalfUpOnFlatSide(self):
        self.assertEqual(ml.Pitch("D1b4").getMidiNumber(), 62)  # 61.5 -> 62, not 61
        self.assertEqual(ml.Pitch("D3b4").getMidiNumber(), 61)  # 60.5 -> 61, not 60
        self.assertEqual(ml.Pitch("C1x-1").getMidiNumber(), 1)
        self.assertEqual(ml.Pitch("D1b4").getQuarterToneSteps(), 61.5)

    # The lowest pitch is a quarter tone below C-1: it still rounds, ties upward, to MIDI 0.
    def testLowestPitchIsC1bMinus1(self):
        lowest = ml.Pitch("C1b-1")
        self.assertEqual(lowest.getQuarterToneSteps(), -0.5)
        self.assertEqual(lowest.getMidiNumber(), 0)


class PitchRounding(unittest.TestCase):
    # Mirrors Pitch.roundToSemitoneTiesUp.
    def testRoundToSemitoneTiesUp(self):
        pitch = ml.Pitch("C1x4")
        pitch.roundToSemitone()
        self.assertEqual(pitch.getPitch(), "C#4")

    # Mirrors Pitch.roundToSemitoneTiesUpFlatSide.
    def testRoundToSemitoneTiesUpFlatSide(self):
        pitch = ml.Pitch("D1b4")
        pitch.roundToSemitone()
        self.assertEqual(pitch.getPitch(), "D4")  # std::round(-0.5) would give "Db4"

        pitch = ml.Pitch("D3b4")
        pitch.roundToSemitone()
        self.assertEqual(pitch.getPitch(), "Db4")  # std::round(-1.5) would give "Dbb4"


class PitchSetStep(unittest.TestCase):
    # Mirrors Pitch.setStepResurrectsRestToOctave4.
    def testSetStepResurrectsRestToOctave4(self):
        rest = ml.Pitch("rest")
        rest.setStep("D")
        self.assertFalse(rest.isRest())
        self.assertEqual(rest.getPitch(), "D4")
        self.assertEqual(rest.getOctave(), 4)

    # Mirrors Pitch.setStepValidTransitionKeepsOctave.
    def testSetStepValidTransitionKeepsOctave(self):
        pitch = ml.Pitch("C1x5")
        pitch.setStep("D")
        self.assertEqual(pitch.getPitch(), "D1x5")

    # Mirrors Pitch.setStepRejectsInvalidStep.
    def testSetStepRejectsInvalidStep(self):
        pitch = ml.Pitch("C4")
        with self.assertRaises(RuntimeError) as context:
            pitch.setStep("H")
        self.assertIn("Unknown diatonic pitch step: H", firstLine(context.exception))
        self.assertEqual(pitch.getPitchStep(), "C")  # unchanged

    # Mirrors Pitch.setStepBelowMidiZeroIsRefusedAndWarns: a boundary condition, not a caller
    # error -- it warns and leaves the pitch unchanged instead of raising.
    def testSetStepBelowMidiZeroIsRefusedAndWarns(self):
        pitch = ml.Pitch("Db-1")  # MIDI 1
        printed = capturedStdout(lambda: pitch.setStep("C"))  # C(0) - 1 would be MIDI -1
        self.assertIn("[WARN] Pitch::setStep", printed)
        self.assertEqual(pitch.getPitchStep(), "D")
        self.assertEqual(pitch.getMidiNumber(), 1)


class PitchSetAlter(unittest.TestCase):
    # Mirrors Pitch.setAlterRejectsNonMultipleOfHalf.
    def testSetAlterRejectsNonMultipleOfHalf(self):
        pitch = ml.Pitch("C4")
        with self.assertRaises(RuntimeError) as context:
            pitch.setAlter(0.3)
        self.assertIn("multiple of 0.5", firstLine(context.exception))
        self.assertEqual(pitch.getAlter(), 0.0)

    # Mirrors Pitch.setAlterRejectsOutOfRange.
    def testSetAlterRejectsOutOfRange(self):
        pitch = ml.Pitch("C4")
        with self.assertRaises(RuntimeError) as context:
            pitch.setAlter(2.5)
        self.assertIn("out of range", firstLine(context.exception))
        self.assertEqual(pitch.getAlter(), 0.0)

    def testSetAlterSetsAQuarterTone(self):
        pitch = ml.Pitch("C4")
        pitch.setAlter(0.5)
        self.assertEqual(pitch.getPitch(), "C1x4")

    # Mirrors Pitch.setAlterOnRestIsRefusedAndWarns: before this was guarded, a rest given an
    # alter reported the pitch class "rest1x".
    def testSetAlterOnRestIsRefusedAndWarns(self):
        rest = ml.Pitch("rest")
        printed = capturedStdout(lambda: rest.setAlter(0.5))  # must not raise
        self.assertIn("[WARN] Pitch::setAlter: cannot set the alter of a rest", printed)
        self.assertTrue(rest.isRest())
        self.assertEqual(rest.getAlter(), 0.0)
        self.assertEqual(rest.getPitchClass(), "rest")  # not "rest1x"

    # Mirrors Pitch.setAlterBelowMidiZeroIsRefusedAndWarns.
    def testSetAlterBelowMidiZeroIsRefusedAndWarns(self):
        pitch = ml.Pitch("C-1")
        printed = capturedStdout(lambda: pitch.setAlter(-1.0))  # must not raise
        self.assertIn("[WARN] Pitch::setAlter", printed)
        self.assertEqual(pitch.getAlter(), 0.0)
        self.assertEqual(pitch.getMidiNumber(), 0)
        self.assertFalse(pitch.isRest())

    # Mirrors PitchSetAlter.rejectsNonFiniteValues: NaN passes a range test, since every
    # comparison with it is false, and an infinity is more than "out of range".
    def testSetAlterRejectsNonFiniteValues(self):
        for alter in (math.nan, math.inf, -math.inf):
            with self.subTest(alter=alter):
                pitch = ml.Pitch("C4")
                with self.assertRaises(RuntimeError) as context:
                    pitch.setAlter(alter)
                self.assertIn("must be a finite number", firstLine(context.exception))
                self.assertEqual(pitch.getPitch(), "C4")

    # Mirrors PitchSetAlter.rejectsAValueNearTheGrid: exact membership, never snapped.
    def testSetAlterRejectsAValueNearTheGrid(self):
        for alter in (0.99996, 1.00004, 0.46):
            with self.subTest(alter=alter):
                pitch = ml.Pitch("C4")
                with self.assertRaises(RuntimeError) as context:
                    pitch.setAlter(alter)
                self.assertIn("multiple of 0.5", firstLine(context.exception))
                self.assertEqual(pitch.getAlter(), 0.0)

    # Mirrors PitchSetAlter.storesNegativeZeroAsPositiveZero.
    def testSetAlterStoresNegativeZeroAsZero(self):
        pitch = ml.Pitch("C1x4")
        pitch.setAlter(-0.0)
        self.assertEqual(math.copysign(1.0, pitch.getAlter()), 1.0)
        self.assertEqual(pitch.getPitch(), "C4")


class PitchSetOctave(unittest.TestCase):
    # Mirrors Pitch.setOctaveValidTransition.
    def testSetOctaveValidTransition(self):
        pitch = ml.Pitch("C1x4")
        pitch.setOctave(5)
        self.assertEqual(pitch.getPitch(), "C1x5")

    # Mirrors Pitch.setOctaveRejectsOutOfRange.
    def testSetOctaveRejectsOutOfRange(self):
        pitch = ml.Pitch("C4")
        with self.assertRaises(RuntimeError) as context:
            pitch.setOctave(12)
        self.assertIn("Invalid octave value: 12", firstLine(context.exception))
        self.assertEqual(pitch.getOctave(), 4)

    # Mirrors Pitch.setOctaveOnRestIsRefusedAndWarns.
    def testSetOctaveOnRestIsRefusedAndWarns(self):
        rest = ml.Pitch("rest")
        printed = capturedStdout(lambda: rest.setOctave(4))
        self.assertIn("[WARN] Pitch::setOctave: cannot set the octave of a rest", printed)
        self.assertTrue(rest.isRest())
        self.assertIsNone(rest.getOctave())

    # Mirrors Pitch.setOctaveOutOfRangeOnRestIsRefusedAndWarnsNotThrows: the rest check runs
    # before the range check, so an out-of-range octave on a rest warns instead of raising.
    def testSetOctaveOutOfRangeOnRestIsRefusedAndWarnsNotThrows(self):
        rest = ml.Pitch("rest")
        for octave in (-2, 12):
            with self.subTest(octave=octave):
                printed = capturedStdout(lambda octave=octave: rest.setOctave(octave))
                self.assertIn("[WARN] Pitch::setOctave", printed)
                self.assertTrue(rest.isRest())
                self.assertIsNone(rest.getOctave())

    # Mirrors Pitch.setOctaveBelowMidiZeroIsRefusedAndWarns.
    def testSetOctaveBelowMidiZeroIsRefusedAndWarns(self):
        pitch = ml.Pitch("Cbb4")  # MIDI 58
        printed = capturedStdout(lambda: pitch.setOctave(-1))  # C - 2 at octave -1 is MIDI -2
        self.assertIn("[WARN] Pitch::setOctave", printed)
        self.assertEqual(pitch.getOctave(), 4)
        self.assertEqual(pitch.getMidiNumber(), 58)

    # A rest's octave is None, and None is not an octave: the round trip is a TypeError from the
    # argument conversion, never a silent sentinel.
    def testSetOctaveOfARestsOctaveIsATypeError(self):
        rest = ml.Pitch("rest")
        with self.assertRaises(TypeError):
            rest.setOctave(rest.getOctave())
        self.assertTrue(rest.isRest())


class PitchWholeStateSetters(unittest.TestCase):
    def testSetPitchReplacesTheWholePitch(self):
        pitch = ml.Pitch()
        pitch.setPitch("E1b4")
        self.assertEqual(pitch.getPitch(), "E1b4")
        self.assertEqual(pitch.getQuarterToneSteps(), 63.5)

        pitch.setPitch("rest")
        self.assertTrue(pitch.isRest())

    # Mirrors Pitch.setPitchClassPreservesOctave.
    def testSetPitchClassPreservesOctave(self):
        pitch = ml.Pitch("C5")
        pitch.setPitchClass("D1x")
        self.assertEqual(pitch.getPitch(), "D1x5")

    # Mirrors Pitch.setPitchClassCanBecomeRest.
    def testSetPitchClassCanBecomeRest(self):
        pitch = ml.Pitch("C4")
        pitch.setPitchClass("rest")
        self.assertTrue(pitch.isRest())
        self.assertEqual(pitch.getPitch(), "rest")

    # Mirrors Pitch.setMidiNumberRoundTrip.
    def testSetMidiNumberRoundTrip(self):
        pitch = ml.Pitch("rest")
        pitch.setMidiNumber(61)
        self.assertEqual(pitch.getPitch(), "C#4")
        self.assertEqual(pitch.getMidiNumber(), 61)

    # Mirrors Pitch.setMidiNumberNegativeIsRest.
    def testSetMidiNumberNegativeIsRest(self):
        pitch = ml.Pitch("C4")
        pitch.setMidiNumber(-1)
        self.assertTrue(pitch.isRest())

    def testSetFrequencyReplacesTheWholePitch(self):
        pitch = ml.Pitch("C4")
        pitch.setFrequency(466.16)
        self.assertEqual(pitch.getPitch(), "A#4")

        pitch.setFrequency(449.0, "#", 440.0, True)
        self.assertEqual(pitch.getPitch(), "A1x4")

        pitch.setFrequency(0.0)
        self.assertTrue(pitch.isRest())


class PitchRange(unittest.TestCase):
    # Mirrors Pitch.clampToRepresentableMidiClampsAnyMidiToTheRepresentableRange.
    def testClampToRepresentableMidi(self):
        self.assertEqual(ml.Pitch.clampToRepresentableMidi(1000000), 157)  # "Bx11"
        self.assertEqual(ml.Pitch.clampToRepresentableMidi(157), 157)
        self.assertEqual(ml.Pitch.clampToRepresentableMidi(156), 156)
        self.assertEqual(ml.Pitch.clampToRepresentableMidi(0), 0)
        self.assertEqual(ml.Pitch.clampToRepresentableMidi(-1000000), 0)  # "C-1"

    def testMaxRepresentableMidiIsBDoubleSharpInOctave11(self):
        self.assertEqual(ml.Pitch.maxRepresentableMidi(), 157)
        self.assertEqual(ml.Pitch("Bx11").getMidiNumber(), ml.Pitch.maxRepresentableMidi())


class PitchReferenceFrequency(unittest.TestCase):
    # Mirrors the PitchReferenceFrequency tests: freqA4 must be a finite number of Hz greater than
    # 0, checked before any arithmetic, also when the result would be a rest.
    INVALID = (0.0, -440.0, math.nan, math.inf, -math.inf)

    def testFromFrequencyRejectsAnInvalidFreqA4(self):
        for freqA4 in self.INVALID:
            for frequency in (440.0, 0.0):
                with self.subTest(freqA4=freqA4, frequency=frequency):
                    with self.assertRaises(RuntimeError) as context:
                        ml.Pitch.fromFrequency(frequency, freqA4=freqA4)
                    self.assertIn("reference frequency freqA4", firstLine(context.exception))

    def testSetFrequencyRejectsAnInvalidFreqA4AndLeavesThePitch(self):
        pitch = ml.Pitch("C4")
        with self.assertRaises(RuntimeError) as context:
            pitch.setFrequency(440.0, freqA4=0.0)
        self.assertIn("reference frequency freqA4", firstLine(context.exception))
        self.assertEqual(pitch.getPitch(), "C4")

    def testGetFrequencyRejectsAnInvalidFreqA4(self):
        for freqA4 in self.INVALID:
            for pitch in ("A4", "rest"):
                with self.subTest(freqA4=freqA4, pitch=pitch):
                    with self.assertRaises(RuntimeError) as context:
                        ml.Pitch(pitch).getFrequency(freqA4)
                    self.assertIn("reference frequency freqA4", firstLine(context.exception))
        self.assertAlmostEqual(ml.Pitch("A4").getFrequency(442.0), 442.0, places=3)


class PitchFrequency(unittest.TestCase):
    # Mirrors Pitch.frequencyOfA4 (no tuning-system reset needed: Python cannot change it).
    def testFrequencyOfA4(self):
        self.assertAlmostEqual(ml.Pitch("A4").getFrequency(), 440.0, places=2)
        self.assertAlmostEqual(ml.Pitch("A4").getFrequency(432.0), 432.0, places=2)
        self.assertEqual(ml.Pitch("rest").getFrequency(), 0.0)

    # Mirrors Pitch.getFrequencyUsesExactQuarterToneSteps: a quarter tone's frequency must differ
    # from both neighbouring semitones', not collapse onto the one it rounds to.
    def testGetFrequencyUsesExactQuarterToneSteps(self):
        expected = 440.0 * 2.0 ** ((60.5 - 69.0) / 12.0)
        frequency = ml.Pitch("C1x4").getFrequency()
        self.assertAlmostEqual(frequency, expected, places=2)
        self.assertNotEqual(frequency, ml.Pitch("C4").getFrequency())
        self.assertNotEqual(frequency, ml.Pitch("C#4").getFrequency())

    # Mirrors Pitch.fromFrequencyRoundsToSemitoneByDefault: 69.4 straddles the quarter-tone
    # boundary (69.25) but not the semitone one (69.5).
    def testFromFrequencyRoundsToSemitoneByDefault(self):
        frequency = 440.0 * 2.0 ** (0.4 / 12.0)
        self.assertEqual(ml.Pitch.fromFrequency(frequency).getPitch(), "A4")

    # Mirrors Pitch.fromFrequencyRoundsToQuarterToneWhenEnabled.
    def testFromFrequencyRoundsToQuarterToneWhenEnabled(self):
        self.assertEqual(ml.Pitch.fromFrequency(449.0, "#", 440.0, True).getPitch(), "A1x4")

    # Mirrors Pitch.enableQuarterToneRoundDefaultsToFalse.
    def testEnableQuarterToneRoundDefaultsToFalse(self):
        self.assertEqual(ml.Pitch.fromFrequency(449.0).getPitch(), "A4")

    # Mirrors Pitch.nonPositiveFrequencyIsARest and Pitch.fromNegativeZeroFrequencyIsARest.
    def testNonPositiveFrequencyIsARest(self):
        for frequency in (0.0, -5.0, -0.0):
            with self.subTest(frequency=frequency):
                self.assertTrue(ml.Pitch.fromFrequency(frequency).isRest())

    # Mirrors Pitch.fromFrequencyAtFloorBoundaryIsNotARest.
    def testFromFrequencyAtFloorBoundaryIsNotARest(self):
        frequency = 440.0 * 2.0 ** ((-0.5 - 69.0) / 12.0)
        pitch = ml.Pitch.fromFrequency(frequency)
        self.assertFalse(pitch.isRest())
        self.assertEqual(pitch.getPitch(), "C-1")
        self.assertEqual(pitch.getMidiNumber(), 0)

    # Mirrors Pitch.fromFrequencyClampsBelowFloorInsteadOfSilentRest.
    def testFromFrequencyClampsBelowFloorInsteadOfSilentRest(self):
        created = []
        printed = capturedStdout(lambda: created.append(ml.Pitch.fromFrequency(1.0)))
        self.assertIn("[WARN] Pitch::setFrequency", printed)
        self.assertFalse(created[0].isRest())
        self.assertEqual(created[0].getPitch(), "C-1")

    # Mirrors Pitch.fromFrequencyRecoversExactQuarterToneAtFloor.
    def testFromFrequencyRecoversExactQuarterToneAtFloor(self):
        frequency = ml.Pitch("C1b-1").getFrequency()
        recovered = ml.Pitch.fromFrequency(frequency, "", 440.0, True)
        self.assertFalse(recovered.isRest())
        self.assertEqual(recovered.getPitch(), "C1b-1")

    # Mirrors Pitch.fromFrequencyClampsAboveCeilingInsteadOfThrowing.
    def testFromFrequencyClampsAboveCeilingInsteadOfThrowing(self):
        frequency = ml.Pitch("Bx11").getFrequency()
        self.assertEqual(ml.Pitch.fromFrequency(frequency).getPitch(), "B11")

    # Mirrors Pitch.fromFrequencyFallsBackWhenAccTypeCannotExpressResidual.
    def testFromFrequencyFallsBackWhenAccTypeCannotExpressResidual(self):
        frequency = 440.0 * 2.0 ** ((62.5 - 69.0) / 12.0)
        self.assertEqual(ml.Pitch.fromFrequency(frequency, "x", 440.0, True).getPitch(), "D1x4")

    # Mirrors PitchAccTypeFallback.warnsWhenTheAccidentalTypeCannotSpellTheRoundedPitch: the
    # documented example. 449 Hz rounds to the quarter tone 69.5, whose base semitone has no "#"
    # spelling, so the default spelling is used, with a warning.
    def testFromFrequencyWarnsWhenTheAccidentalTypeCannotSpellTheRoundedPitch(self):
        created = []
        printed = capturedStdout(
            lambda: created.append(ml.Pitch.fromFrequency(449.0, "#", 440.0, True))
        )
        self.assertEqual(created[0].getPitch(), "A1x4")
        self.assertIn("[WARN] Pitch::setFrequency: the accidental type '#' cannot spell", printed)

    # Mirrors PitchAccTypeFallback.anAccidentalTypeThatAppliesPrintsNothing.
    def testFromFrequencyWithAnAccidentalTypeThatAppliesPrintsNothing(self):
        created = []
        printed = capturedStdout(lambda: created.append(ml.Pitch.fromFrequency(466.16, "b")))
        self.assertEqual(created[0].getPitch(), "Bb4")
        self.assertEqual(printed, "")

    # Mirrors Pitch.fromFrequencyClampsAboveCeilingWhenAccTypeAlsoOverflowsAlter.
    def testFromFrequencyClampsAboveCeilingWhenAccTypeAlsoOverflowsAlter(self):
        pitch = ml.Pitch.fromFrequency(73038.0, "x", 440.0, True)
        self.assertFalse(pitch.isRest())
        self.assertEqual(pitch.getPitch(), "B11")

    # Mirrors Pitch.fromInfinityFrequencyClampsToCeilingInsteadOfUndefinedBehavior.
    def testFromInfinityFrequencyClampsToCeiling(self):
        pitch = ml.Pitch.fromFrequency(math.inf)
        self.assertFalse(pitch.isRest())
        self.assertEqual(pitch.getPitch(), "B11")
        self.assertTrue(ml.Pitch.fromFrequency(-math.inf).isRest())

    # Mirrors Pitch.fromNanFrequencyThrows: NaN is neither "zero or negative" nor "positive", so
    # it is a caller error rather than a pitch fabricated from the absence of a value.
    def testFromNanFrequencyRaises(self):
        with self.assertRaises(RuntimeError) as context:
            ml.Pitch.fromFrequency(math.nan)
        self.assertIn("NaN", firstLine(context.exception))

    # Mirrors Pitch.fromExtremeFiniteFrequencyClampsToCeiling. The largest finite C++ float is
    # used exactly, since a larger Python float has no float representation to convert to.
    def testFromExtremeFiniteFrequencyClampsToCeiling(self):
        largest = float.fromhex("0x1.fffffep+127")
        self.assertEqual(float32(largest), largest)
        self.assertEqual(ml.Pitch.fromFrequency(largest).getPitch(), "B11")

    # Mirrors Pitch.setFrequencyRejectsMalformedAccType.
    def testFromFrequencyRejectsMalformedAccType(self):
        with self.assertRaises(RuntimeError) as context:
            ml.Pitch.fromFrequency(277.18, "garbage")
        self.assertIn("Unknown accident type: garbage", firstLine(context.exception))

    # Mirrors Pitch.fromFrequencyQuarterToneFlatSide.
    def testFromFrequencyQuarterToneFlatSide(self):
        frequency = 440.0 * 2.0 ** ((68.5 - 69.0) / 12.0)
        pitch = ml.Pitch.fromFrequency(frequency, "b", 440.0, True)
        self.assertEqual(pitch.getPitch(), "A1b4")
        self.assertEqual(pitch.getMidiNumber(), 69)  # ties upward


if __name__ == "__main__":
    unittest.main()
