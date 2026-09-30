import contextlib
import io
import math
import unittest
from pathlib import Path

import maialib as ml


def isFloatEq(x, y, epslon=1e-3):
    return abs(x - y) < epslon


# ===== TEST MAIALIB FUNCTIONS ===== #


class midiNote2freq(unittest.TestCase):
    def testMidiValuesTable(self):
        # Special Cases
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(-1), 0.00), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(0), 8.175), True)

        # Piano First Octave: From A0 to A1
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(21), 27.500), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(22), 29.135), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(23), 30.868), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(24), 32.703), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(25), 34.648), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(26), 36.708), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(27), 38.891), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(28), 41.203), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(29), 43.654), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(30), 46.249), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(31), 48.999), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(32), 51.913), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(33), 55.000), True)

        # Piano Middle Octave: From C4 to C5
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(60), 261.626), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(61), 277.183), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(62), 293.665), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(63), 311.127), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(64), 329.628), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(65), 349.228), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(66), 369.994), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(67), 391.995), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(68), 415.305), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(69), 440.000), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(70), 466.164), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(71), 493.883), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(72), 523.251), True)

        # Piano Higher Octave: From C7 to C8
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(96), 2093.005), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(97), 2217.461), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(98), 2349.318), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(99), 2489.016), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(100), 2637.020), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(101), 2793.826), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(102), 2959.955), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(103), 3135.963), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(104), 3322.438), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(105), 3520.000), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(106), 3729.310), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(107), 3951.066), True)
        self.assertEqual(isFloatEq(ml.Helper.midiNote2freq(108), 4186.009), True)


class Freq2midiNote(unittest.TestCase):
    def test_octaves_values(self):
        indexes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        for i in indexes:
            result = ml.Helper.freq2midiNote((pow(2, i)) * 110.0)
            self.assertEqual(result[0], 45 + (12 * i))
            self.assertEqual(result[1], 0)

    def test_cents_values(self):
        # base frequency = 29.135, base midiNote = 22
        result01 = ml.Helper.freq2midiNote(30.1)
        self.assertEqual(result01[0], 23)
        self.assertEqual(result01[1], -44)

        # base frequency = 195.998, base midiNote = 55
        result02 = ml.Helper.freq2midiNote(202.0)
        self.assertEqual(result02[0], 56)
        self.assertEqual(result02[1], -48)

        # base frequency = 466.164, base midiNote = 70
        result03 = ml.Helper.freq2midiNote(481.0)
        self.assertEqual(result03[0], 71)
        self.assertEqual(result03[1], -46)

        # base frequency = 1661.219, base midiNote = 92
        result04 = ml.Helper.freq2midiNote(1712.0)
        self.assertEqual(result04[0], 93)
        self.assertEqual(result04[1], -48)

        # base frequency = 3135.963, base midiNote = 103
        result05 = ml.Helper.freq2midiNote(3270.0)
        self.assertEqual(result05[0], 104)
        self.assertEqual(result05[1], -28)

        # base frequency =3951.056, base midiNote = 107
        result06 = ml.Helper.freq2midiNote(4080.0)
        self.assertEqual(result06[0], 108)
        self.assertEqual(result06[1], -44)

        # b) Testing for frequencies with values slighter smaller than the base frequency.
        # the output note must be the base MidiNote -1. So cents are positive and smaller than 50.

        # base frequency = 29.135, base midiNote = 22
        result07 = ml.Helper.freq2midiNote(28.0)
        self.assertEqual(result07[0], 21)
        self.assertEqual(result07[1], 31)

        # base frequency = 195.998, base midiNote = 55
        result08 = ml.Helper.freq2midiNote(189.0)
        self.assertEqual(result08[0], 54)
        self.assertEqual(result08[1], 37)

        # base frequency = 466.164, base midiNote = 70
        result09 = ml.Helper.freq2midiNote(450.0)
        self.assertEqual(result09[0], 69)
        self.assertEqual(result09[1], 39)

        # base frequency = 1661.219, base midiNote = 92
        result10 = ml.Helper.freq2midiNote(1605.0)
        self.assertEqual(result10[0], 91)
        self.assertEqual(result10[1], 40)

        # base frequency = 3135.963, base midiNote = 103
        result11 = ml.Helper.freq2midiNote(3035.0)
        self.assertEqual(result11[0], 102)
        self.assertEqual(result11[1], 43)

        # base frequency =3951.056, base midiNote = 107
        result12 = ml.Helper.freq2midiNote(3828.0)
        self.assertEqual(result12[0], 106)
        self.assertEqual(result12[1], 45)


class midiNote2pitch(unittest.TestCase):
    def testNegativeValue_restCase(self):
        # Any negative number - rest
        self.assertEqual(ml.Helper.midiNote2pitch(-1, "bb"), "rest")
        self.assertEqual(ml.Helper.midiNote2pitch(-1, "b"), "rest")
        self.assertEqual(ml.Helper.midiNote2pitch(-1), "rest")
        self.assertEqual(ml.Helper.midiNote2pitch(-1, "#"), "rest")
        self.assertEqual(ml.Helper.midiNote2pitch(-1, "x"), "rest")

    def testTwelveTonesOctave4(self):
        # A spelling the note cannot take raises RuntimeError. Each pattern pins the MIDI number
        # and the accidental but leaves the verb open ("cannot be .* using"), so the test does not
        # depend on how that verb is spelled.

        # ===== MIDI Note 60 - C4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(60, "bb"), "Dbb4")

        # Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '60' cannot be .* using 'b' accident type"
        ):
            ml.Helper.midiNote2pitch(60, "b")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(60), "C4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(60, "#"), "B#3")

        # Double Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '60' cannot be .* using 'x' accident type"
        ):
            ml.Helper.midiNote2pitch(60, "x")

        # ===== MIDI Note 61 - C#4 ===== #
        # Double Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '61' cannot be .* using 'bb' accident type"
        ):
            ml.Helper.midiNote2pitch(61, "bb")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(61, "b"), "Db4")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(61), "C#4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(61, "#"), "C#4")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(61, "x"), "Bx3")

        # ===== MIDI Note 62 - D4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(62, "bb"), "Ebb4")

        # Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '62' cannot be .* using 'b' accident type"
        ):
            ml.Helper.midiNote2pitch(62, "b")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(62), "D4")

        # Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '62' cannot be .* using '#' accident type"
        ):
            ml.Helper.midiNote2pitch(62, "#")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(62, "x"), "Cx4")

        # ===== MIDI Note 63 - D#4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(63, "bb"), "Fbb4")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(63, "b"), "Eb4")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(63), "D#4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(63, "#"), "D#4")

        # Double Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '63' cannot be .* using 'x' accident type"
        ):
            ml.Helper.midiNote2pitch(63, "x")

        # ===== MIDI Note 64 - E4 ===== #
        # Double Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '64' cannot be .* using 'bb' accident type"
        ):
            ml.Helper.midiNote2pitch(64, "bb")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(64, "b"), "Fb4")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(64), "E4")

        # Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '64' cannot be .* using '#' accident type"
        ):
            ml.Helper.midiNote2pitch(64, "#")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(64, "x"), "Dx4")

        # ===== MIDI Note 65 - F4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(65, "bb"), "Gbb4")

        # Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '65' cannot be .* using 'b' accident type"
        ):
            ml.Helper.midiNote2pitch(65, "b")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(65), "F4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(65, "#"), "E#4")

        # Double Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '65' cannot be .* using 'x' accident type"
        ):
            ml.Helper.midiNote2pitch(65, "x")

        # ===== MIDI Note 66 - F#4 ===== #
        # Double Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '66' cannot be .* using 'bb' accident type"
        ):
            ml.Helper.midiNote2pitch(66, "bb")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(66, "b"), "Gb4")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(66), "F#4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(66, "#"), "F#4")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(66, "x"), "Ex4")

        # ===== MIDI Note 67 - G4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(67, "bb"), "Abb4")

        # Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '67' cannot be .* using 'b' accident type"
        ):
            ml.Helper.midiNote2pitch(67, "b")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(67), "G4")

        # Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '67' cannot be .* using '#' accident type"
        ):
            ml.Helper.midiNote2pitch(67, "#")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(67, "x"), "Fx4")

        # ===== MIDI Note 68 - G#4 ===== #
        # Double Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '68' cannot be .* using 'bb' accident type"
        ):
            ml.Helper.midiNote2pitch(68, "bb")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(68, "b"), "Ab4")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(68), "G#4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(68, "#"), "G#4")

        # Double Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '68' cannot be .* using 'x' accident type"
        ):
            ml.Helper.midiNote2pitch(68, "x")

        # ===== MIDI Note 69 - A4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(69, "bb"), "Bbb4")

        # Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '69' cannot be .* using 'b' accident type"
        ):
            ml.Helper.midiNote2pitch(69, "b")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(69), "A4")

        # Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '69' cannot be .* using '#' accident type"
        ):
            ml.Helper.midiNote2pitch(69, "#")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(69, "x"), "Gx4")

        # ===== MIDI Note 70 - A#4 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(70, "bb"), "Cbb5")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(70, "b"), "Bb4")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(70), "A#4")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(70, "#"), "A#4")

        # Double Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '70' cannot be .* using 'x' accident type"
        ):
            ml.Helper.midiNote2pitch(70, "x")

        # ===== MIDI Note 71 - B4 ===== #
        # Double Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '71' cannot be .* using 'bb' accident type"
        ):
            ml.Helper.midiNote2pitch(71, "bb")

        # Flat
        self.assertEqual(ml.Helper.midiNote2pitch(71, "b"), "Cb5")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(71), "B4")

        # Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '71' cannot be .* using '#' accident type"
        ):
            ml.Helper.midiNote2pitch(71, "#")

        # Double Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(71, "x"), "Ax4")

        # ===== MIDI Note 72 - C5 ===== #
        # Double Flat
        self.assertEqual(ml.Helper.midiNote2pitch(72, "bb"), "Dbb5")

        # Flat
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '72' cannot be .* using 'b' accident type"
        ):
            ml.Helper.midiNote2pitch(72, "b")

        # Natural
        self.assertEqual(ml.Helper.midiNote2pitch(72), "C5")

        # Sharp
        self.assertEqual(ml.Helper.midiNote2pitch(72, "#"), "B#4")

        # Double Sharp
        with self.assertRaisesRegex(
            RuntimeError, r"MIDI Note '72' cannot be .* using 'x' accident type"
        ):
            ml.Helper.midiNote2pitch(72, "x")


class PitchSpelling(unittest.TestCase):
    def testPitch2midiNoteEdgeCases(self):
        self.assertEqual(ml.Helper.pitch2midiNote("Cbb0"), 10)
        self.assertEqual(ml.Helper.pitch2midiNote("Cb0"), 11)
        self.assertEqual(ml.Helper.pitch2midiNote("Bx9"), 133)
        self.assertEqual(ml.Helper.pitch2midiNote("Db10"), 133)
        self.assertEqual(ml.Helper.pitch2midiNote("C-1"), 0)
        self.assertEqual(ml.Helper.pitch2midiNote("Bx11"), 157)
        self.assertEqual(ml.Helper.pitch2midiNote("rest"), -1)

    def testPitch2midiNoteRejectsInvalidPitches(self):
        for pitch in ["Cb-1", "Cbb-1", "C12", "H4"]:
            with self.subTest(pitch=pitch), self.assertRaises(RuntimeError):
                ml.Helper.pitch2midiNote(pitch)

    def testPitch2midiNoteAcceptsQuarterTones(self):
        # Quarter-tone accidentals round to the nearest MIDI number with ties broken upward
        # (spec section 4.5); values match the C++ PitchSpelling.QuarterToneRoundsTiesUpward test.
        self.assertEqual(ml.Helper.pitch2midiNote("C1x4"), 61)
        self.assertEqual(ml.Helper.pitch2midiNote("C3x4"), 62)
        self.assertEqual(ml.Helper.pitch2midiNote("D1b4"), 62)
        self.assertEqual(ml.Helper.pitch2midiNote("D3b4"), 61)

    def testIsEnharmonic(self):
        self.assertTrue(ml.Helper.isEnharmonic("E#4", "F4"))
        self.assertTrue(ml.Helper.isEnharmonic(pitch_A="B#3", pitch_B="C4"))
        self.assertFalse(ml.Helper.isEnharmonic("C4", "D4"))

    # isEnharmonic() compares exact positions: by rounded MIDI numbers, a quarter tone and the
    # semitone it rounds to would collide on one integer and be reported as the same pitch.
    # Mirrors the C++ PitchSpelling.IsEnharmonicComparesExactPitchNotRoundedMidi test.
    def testIsEnharmonicComparesExactPitchNotRoundedMidi(self):
        # The two spellings of one quarter tone, both exactly 60.5.
        self.assertTrue(ml.Helper.isEnharmonic("C1x4", "D3b4"))
        # A quarter tone is not the semitone it rounds to: 60.5 is not 61.
        self.assertFalse(ml.Helper.isEnharmonic("C1x4", "C#4"))
        self.assertFalse(ml.Helper.isEnharmonic("C1x4", "C4"))

    # transposePitch() takes a float interval and computes on exact positions, never rounding the
    # pitch to a MIDI integer before applying the interval. Mirrors the C++
    # PitchSpelling.TransposePitchMovesByAndPreservesQuarterTones test.
    def testTransposePitchMovesByAndPreservesQuarterTones(self):
        self.assertEqual(ml.Helper.transposePitch("C4", 0.5, ""), "C1x4")
        self.assertEqual(ml.Helper.transposePitch("C1x4", 2, ""), "D1x4")
        self.assertEqual(ml.Helper.transposePitch("C4", 2, ""), "D4")

    def testTransposePitchRejectsIntervalOffTheQuarterToneGrid(self):
        with self.assertRaises(RuntimeError) as ctx:
            ml.Helper.transposePitch("C4", 0.3)
        self.assertIn("multiple of 0.5", str(ctx.exception))

    # An infinity must be rejected as non-finite: a multiple-of-0.5 check alone passes it
    # (inf * 2 == inf == floor(inf)), after which +inf would fail far away with an unrelated
    # message, and -inf would SILENTLY answer "rest" -- an infinite transposition quietly turning
    # a note into a rest. Mirrors the C++ PitchSpelling.TransposePitchRejectsNonFiniteIntervals
    # test.
    #
    # The message must also NAME the offending value, as the sibling off-grid test requires of
    # 0.3. Only the first line is read: the rest is a stack trace.
    def testTransposePitchRejectsNonFiniteIntervals(self):
        for label, value, shown in (
            ("+inf", float("inf"), "'inf'"),
            ("-inf", float("-inf"), "'-inf'"),
            ("nan", float("nan"), "nan"),
        ):
            with self.subTest(interval=label):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.transposePitch("C4", value)
                message = str(ctx.exception).splitlines()[0]
                self.assertIn("finite", message)
                self.assertIn(shown, message)

    # A real pitch transposed out of the representable range raises at both ends, naming the
    # pitch and the range, rather than answering "rest" below MIDI 0. Mirrors the C++
    # PitchSpelling.TransposePitchRejectsAResultOutsideTheRepresentableRange test.
    def testTransposePitchRejectsResultOutsideRepresentableRange(self):
        for pitch, semitones in (
            ("C4", -61),
            ("C4", -3e9),
            ("C1b-1", -0.5),
            ("C4", 3e9),
            ("B11", 2.5),
        ):
            with self.subTest(pitch=pitch, semitones=semitones):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.transposePitch(pitch, semitones)
                message = str(ctx.exception).splitlines()[0]
                self.assertIn("outside the representable range", message)
                self.assertIn(f"'{pitch}'", message)
                self.assertIn("C1b-1", message)
                self.assertIn("Bx11", message)

        # Both ends of the range stay reachable.
        self.assertEqual(ml.Helper.transposePitch("C-1", -0.5, ""), "C1b-1")
        self.assertEqual(ml.Helper.transposePitch("B11", 2, "x"), "Bx11")

    # transposePitch() reads its input as a pitch first: an empty string is a rest (not "C#-1",
    # two semitones above MIDI_REST), and an invalid pitch string raises even for an interval of
    # 0.
    def testTransposePitchTreatsEveryRestSpellingAsARestAndParsesTheInputFirst(self):
        self.assertEqual(ml.Helper.transposePitch("", 2), "rest")
        self.assertEqual(ml.Helper.transposePitch("rest", 2), "rest")
        with self.assertRaises(RuntimeError) as ctx:
            ml.Helper.transposePitch("H4", 0)
        self.assertIn("Unknown diatonic pitch", str(ctx.exception).splitlines()[0])

    def testSplitPitch(self):
        self.assertEqual(ml.Helper.splitPitch("Dbb-1"), ("Dbb", "D", -1, -2.0, "bb"))
        self.assertEqual(ml.Helper.splitPitch("E"), ("E", "E", 4, 0.0, ""))
        self.assertEqual(ml.Helper.splitPitch(pitch="rest"), ("rest", "rest", None, 0.0, ""))

    def testPitch2numberWasRemoved(self):
        self.assertFalse(hasattr(ml.Helper, "pitch2number"))


class QuarterToneHelpers(unittest.TestCase):
    """The quarter-tone Helper functions, through the bindings."""

    ALTERS = (
        # (value, symbol, Tartini or whole-tone name)
        (-2.0, "bb", "flat-flat"),
        (-1.5, "3b", "three-quarters-flat"),
        (-1.0, "b", "flat"),
        (-0.5, "1b", "quarter-flat"),
        (0.0, "", "natural"),
        (0.5, "1x", "quarter-sharp"),
        (1.0, "#", "sharp"),
        (1.5, "3x", "three-quarters-sharp"),
        (2.0, "x", "double-sharp"),
    )

    def testEveryNewBindingIsDocumentedWithAnExample(self):
        for name in (
            "steps2pitch",
            "validateTransposeSemitones",
            "alterName2symbol",
            "alterSymbol2Value",
            "alterValue2symbol",
            "alterValue2Name",
        ):
            with self.subTest(function=name):
                self.assertIn("Examples\n", getattr(ml.Helper, name).__doc__)

    def testSteps2PitchSpellsAnExactPosition(self):
        self.assertEqual(ml.Helper.steps2pitch(60.0), "C4")
        self.assertEqual(ml.Helper.steps2pitch(60.5), "C1x4")
        self.assertEqual(ml.Helper.steps2pitch(63.5), "E1b4")
        # accType spells the semitone the position rounds UP to: 64 as "Fb4", so "F3b4".
        self.assertEqual(ml.Helper.steps2pitch(63.5, "b"), "F3b4")
        self.assertEqual(ml.Helper.steps2pitch(157.0, "x"), "Bx11")

    # The rest sentinel only below MIDI 0: -0.5 rounds, ties upward, to MIDI 0 and is "C1b-1".
    def testSteps2PitchAnswersARestOnlyBelowMidiZero(self):
        self.assertEqual(ml.Helper.steps2pitch(-0.5), "C1b-1")
        self.assertEqual(ml.Helper.steps2pitch(-1.0), "rest")
        self.assertEqual(ml.Helper.steps2pitch(-3e9), "rest")

    # Section N: without these checks a position was converted with an undefined-behaviour
    # static_cast<int> (non-finite or huge), or silently snapped to the grid (off-grid).
    def testSteps2PitchRejectsNonFiniteOffGridAndTooHighPositions(self):
        for label, value, expected in (
            ("+inf", float("inf"), "'inf'"),
            ("-inf", float("-inf"), "'-inf'"),
            ("nan", float("nan"), "nan"),
        ):
            with self.subTest(position=label):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.steps2pitch(value)
                message = str(ctx.exception).splitlines()[0]
                self.assertIn("finite", message)
                self.assertIn(expected, message)

        for value in (60.25, 60.45):
            with self.subTest(position=value):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.steps2pitch(value)
                self.assertIn("multiple of 0.5", str(ctx.exception).splitlines()[0])

        for value in (157.5, 3e9):
            with self.subTest(position=value):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.steps2pitch(value, "x")
                message = str(ctx.exception).splitlines()[0]
                self.assertIn("above the representable range", message)
                self.assertIn("Bx11", message)

    # accType is a preference for the quarter tone: a "bb" spelling (62 as "Ebb4") is already at
    # the -2 limit and cannot absorb -0.5, so the default spelling is used, with a warning that
    # the binding redirects to sys.stdout.
    def testSteps2PitchFallsBackAndWarnsWhenAccTypeCannotAbsorbTheQuarterTone(self):
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            pitch = ml.Helper.steps2pitch(61.5, "bb")
        self.assertEqual(pitch, "D1b4")
        self.assertIn("[WARN] Helper::steps2pitch", buffer.getvalue())

    # The components of a pitch this library can spell: an alter from -2 to 2 and an octave from
    # -1 to 11. Anything else is rejected before the sum is converted to an int.
    def testSpelling2midiNoteRejectsComponentsNoPitchCanHave(self):
        for alter in (math.nan, math.inf, -math.inf, 2.5, -2.5, 3.0e9):
            with self.subTest(alter=alter):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.spelling2midiNote("C", alter, 4)
                self.assertIn(
                    "the alter value must be a finite number of semitones from -2 to 2",
                    str(ctx.exception).splitlines()[0],
                )

        for octave in (-2, 12, 100000):
            with self.subTest(octave=octave):
                with self.assertRaises(RuntimeError) as ctx:
                    ml.Helper.spelling2midiNote("C", 0.0, octave)
                self.assertIn("the octave must be from -1 to 11", str(ctx.exception).splitlines()[0])

        self.assertEqual(ml.Helper.spelling2midiNote("B", 2.0, 11), 157)
        self.assertEqual(ml.Helper.spelling2midiNote("D", -2.0, -1), 0)

    def testValidateTransposeSemitones(self):
        self.assertIsNone(ml.Helper.validateTransposeSemitones(0.5))
        self.assertIsNone(ml.Helper.validateTransposeSemitones(-12))

        with self.assertRaises(RuntimeError) as ctx:
            ml.Helper.validateTransposeSemitones(0.3)
        message = str(ctx.exception).splitlines()[0]
        self.assertIn("multiple of 0.5", message)
        self.assertIn("0.3", message)

        with self.assertRaises(RuntimeError) as ctx:
            ml.Helper.validateTransposeSemitones(float("inf"))
        message = str(ctx.exception).splitlines()[0]
        self.assertIn("finite", message)
        self.assertIn("'inf'", message)

    def testAlterSymbol2ValueAndAlterValue2symbolAreInverse(self):
        for value, symbol, _ in self.ALTERS:
            with self.subTest(symbol=symbol):
                self.assertEqual(ml.Helper.alterSymbol2Value(symbol), value)
                self.assertEqual(ml.Helper.alterValue2symbol(value), symbol)

    def testAlterValue2NameUsesTheTartiniNames(self):
        for value, _, name in self.ALTERS:
            with self.subTest(value=value):
                self.assertEqual(ml.Helper.alterValue2Name(value), name)

    # All 13 names, the arrow spellings included, and each Tartini name round-trips through
    # alterValue2Name().
    def testAlterName2symbolAcceptsTheThirteenNames(self):
        arrows = {"flat-up": "1b", "flat-down": "3b", "sharp-down": "1x", "sharp-up": "3x"}
        for name, symbol in arrows.items():
            with self.subTest(name=name):
                self.assertEqual(ml.Helper.alterName2symbol(name), symbol)
        for _, symbol, name in self.ALTERS:
            with self.subTest(name=name):
                self.assertEqual(ml.Helper.alterName2symbol(name), symbol)

    # Exactly one of the nine alters, or an error naming the value: a value near one is not
    # rounded onto it, as a one-decimal text match would.
    def testAlterValueMustBeExactlyOneOfTheNine(self):
        for function in (ml.Helper.alterValue2symbol, ml.Helper.alterValue2Name):
            for alter in (0.46, 0.54, 1.04, 0.25, 2.5, math.nan):
                with self.subTest(function=function.__name__, alter=alter):
                    with self.assertRaises(RuntimeError) as ctx:
                        function(alter)
                    message = str(ctx.exception).splitlines()[0]
                    self.assertIn("Unknown accidental alter value", message)
                    self.assertIn("exactly a multiple of 0.5 from -2 to 2", message)

    def testNegativeZeroIsTheNatural(self):
        self.assertEqual(ml.Helper.alterValue2symbol(-0.0), "")
        self.assertEqual(ml.Helper.alterValue2Name(-0.0), "natural")

    def testSharpSharpIsADoubleSharp(self):
        self.assertEqual(ml.Helper.alterName2symbol("sharp-sharp"), "x")

    def testUnknownAlterInputsRaise(self):
        with self.assertRaises(RuntimeError) as ctx:
            ml.Helper.alterName2symbol("slash-flat")
        self.assertIn("Unknown accidental name: slash-flat", str(ctx.exception).splitlines()[0])

        with self.assertRaises(RuntimeError) as ctx:
            ml.Helper.alterSymbol2Value("2x")
        self.assertIn("Unknown accident symbol: 2x", str(ctx.exception).splitlines()[0])

        for function in (ml.Helper.alterValue2symbol, ml.Helper.alterValue2Name):
            with self.subTest(function=function.__name__):
                with self.assertRaises(RuntimeError) as ctx:
                    function(0.25)  # an eighth tone: no spelling
                self.assertIn("Unknown accidental alter value", str(ctx.exception).splitlines()[0])


class MelodyContour(unittest.TestCase):
    # Mirrors MelodyContour.QuarterToneIntervalsAreComputedExactly: the float result expresses a
    # quarter tone exactly, so the difference is computed rather than rejected.
    def testQuarterToneIntervalsAreComputedExactly(self):
        reference = [ml.Note("C4"), ml.Note("E4"), ml.Note("G4")]
        neutralThird = [ml.Note("C4"), ml.Note("E1b4"), ml.Note("G4")]
        self.assertEqual(
            ml.Helper.getSemitonesDifferenceBetweenMelodies(reference, neutralThird), [0.5, -0.5]
        )
        transposed = [ml.Note("D1x4"), ml.Note("F3x4"), ml.Note("A1x4")]
        self.assertEqual(
            ml.Helper.getSemitonesDifferenceBetweenMelodies(reference, transposed), [0.0, 0.0]
        )


class Version(unittest.TestCase):
    def testVersionHasNoQuoteCharacters(self):
        self.assertNotIn('"', ml.__version__)
        self.assertNotIn('"', ml.maiacore.__version__)
        self.assertNotIn('"', ml.Helper.getLibraryVersion())

    def testVersionMatchesVersionFile(self):
        versionFilePath = Path(__file__).resolve().parent.parent / "VERSION"
        expectedVersion = versionFilePath.read_text().strip()

        self.assertEqual(ml.__version__, expectedVersion)
        self.assertEqual(ml.maiacore.__version__, expectedVersion)
        self.assertEqual(ml.Helper.getLibraryVersion(), expectedVersion)

    def testGetLibraryVersionMatchesPythonVersion(self):
        self.assertEqual(ml.Helper.getLibraryVersion(), ml.__version__)
        self.assertEqual(ml.Helper.getLibraryVersion(), ml.maiacore.__version__)


if __name__ == "__main__":
    unittest.main()
