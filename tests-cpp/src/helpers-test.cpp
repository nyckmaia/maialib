#include <gtest/gtest.h>

#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>
#include <vector>

#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/note.h"
#include "maiacore/utils.h"
#include "pitch-spelling-legacy-data.h"
#include "test-capture.h"
#include "test-locale.h"

using namespace testing;

TEST(midiNote2freq, midiValuesTable) {
    // Special Cases
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(-1), 0.00f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(0), 8.175f), true);

    // Piano First Octave: From A0 to A1
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(21), 27.500f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(22), 29.135f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(23), 30.868f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(24), 32.703f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(25), 34.648f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(26), 36.708f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(27), 38.891f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(28), 41.203f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(29), 43.654f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(30), 46.249f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(31), 48.999f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(32), 51.913f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(33), 55.000f), true);

    // Piano Middle Octave: From C4 to C5
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(60), 261.626f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(61), 277.183f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(62), 293.665f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(63), 311.127f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(64), 329.628f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(65), 349.228f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(66), 369.994f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(67), 391.995f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(68), 415.305f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(69), 440.000f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(70), 466.164f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(71), 493.883f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(72), 523.251f), true);

    // Piano Higher Octave: From C7 to C8
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(96), 2093.005f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(97), 2217.461f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(98), 2349.318f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(99), 2489.016f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(100), 2637.020f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(101), 2793.826f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(102), 2959.955f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(103), 3135.963f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(104), 3322.438f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(105), 3520.000f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(106), 3729.310f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(107), 3951.066f), true);
    EXPECT_EQ(isFloatEqual(Helper::midiNote2freq(108), 4186.009f), true);
}

TEST(freq2midiNote, octavesValues) {
    for (int i = 0; i < 10; i++) {
        std::pair<int, int> result = Helper::freq2midiNote((pow(2, i)) * 110.0);
        EXPECT_EQ(result.first, 45 + (12 * i));
        EXPECT_EQ(result.second, 0);
    }
}

TEST(freq2midiNote, centsValues) {
    // Obs: In this case the (non zero) cents were calculated separately (with
    // a calculator) Enter with a given frequency (not in the table) and
    // calculate the deviation (in CENTS) from the closest  frequency (up or down) from
    // the table. float midi_50cents_up = 1.029302; this factor increase 50
    // cents (f2= a*f1 ) ==> this makes round up the value of MIDI note float
    // midi_50cents_down = 0.971532; this factor decrease 50 cents on the
    // value of MidiNumber ==> makes round down the value of MIDI note

    // a) Testing for frequencies with values slighter greater than the base
    // frequency the output note must be the base MidiNote + 1. So cents are
    // negative and greater than -50.
    std::pair<int, int> result01 =
        Helper::freq2midiNote(30.1f);  // base frequency = 29.135, base midiNote = 22
    EXPECT_EQ(result01.first, 23);
    EXPECT_EQ(result01.second, -44);

    std::pair<int, int> result02 =
        Helper::freq2midiNote(202.0f);  // base frequency = 195.998, base midiNote = 55
    EXPECT_EQ(result02.first, 56);
    EXPECT_EQ(result02.second, -48);

    std::pair<int, int> result03 =
        Helper::freq2midiNote(481.0f);  // base frequency = 466.164, base midiNote = 70
    EXPECT_EQ(result03.first, 71);
    EXPECT_EQ(result03.second, -46);

    std::pair<int, int> result04 =
        Helper::freq2midiNote(1712.0f);  // base frequency = 1661.219, base midiNote = 92
    EXPECT_EQ(result04.first, 93);
    EXPECT_EQ(result04.second, -48);

    std::pair<int, int> result05 =
        Helper::freq2midiNote(3270.0f);  // base frequency = 3135.963, base midiNote = 103
    EXPECT_EQ(result05.first, 104);
    EXPECT_EQ(result05.second, -28);

    std::pair<int, int> result06 =
        Helper::freq2midiNote(4080.0f);  // base frequency =3951.056, base midiNote = 107
    EXPECT_EQ(result06.first, 108);
    EXPECT_EQ(result06.second, -44);

    // b) Testing for frequencies with values slighter smaller than the base
    // frequency. the output note must be the base MidiNote -1. So cents are
    // positive and smaller than 50.

    std::pair<int, int> result07 =
        Helper::freq2midiNote(28.0f);  // base frequency = 29.135, base midiNote = 22
    EXPECT_EQ(result07.first, 21);
    EXPECT_EQ(result07.second, 31);

    std::pair<int, int> result08 =
        Helper::freq2midiNote(189.0f);  // base frequency = 195.998, base midiNote = 55
    EXPECT_EQ(result08.first, 54);
    EXPECT_EQ(result08.second, 37);

    std::pair<int, int> result09 =
        Helper::freq2midiNote(450.0f);  // base frequency = 466.164, base midiNote = 70
    EXPECT_EQ(result09.first, 69);
    EXPECT_EQ(result09.second, 39);

    std::pair<int, int> result10 =
        Helper::freq2midiNote(1605.0f);  // base frequency = 1661.219, base midiNote = 92
    EXPECT_EQ(result10.first, 91);
    EXPECT_EQ(result10.second, 40);

    std::pair<int, int> result11 =
        Helper::freq2midiNote(3035.0f);  // base frequency = 3135.963, base midiNote = 103
    EXPECT_EQ(result11.first, 102);
    EXPECT_EQ(result11.second, 43);

    std::pair<int, int> result12 =
        Helper::freq2midiNote(3828.0f);  // base frequency =3951.056, base midiNote = 107
    EXPECT_EQ(result12.first, 106);
    EXPECT_EQ(result12.second, 45);
}

TEST(midiNote2pitch, negativeValue_restCase) {
    // Any negative number - rest
    EXPECT_EQ(Helper::midiNote2pitch(-1, "bb"), "rest");
    EXPECT_EQ(Helper::midiNote2pitch(-1, "b"), "rest");
    EXPECT_EQ(Helper::midiNote2pitch(-1), "rest");
    EXPECT_EQ(Helper::midiNote2pitch(-1, "#"), "rest");
    EXPECT_EQ(Helper::midiNote2pitch(-1, "x"), "rest");
}

TEST(midiNote2pitch, twelveTonesOctave4) {
    // ===== MIDI Note 60 - C4 =====
    EXPECT_EQ(Helper::midiNote2pitch(60, "bb"), "Dbb4");                // Double flat
    EXPECT_THROW(Helper::midiNote2pitch(60, "b"), std::runtime_error);  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(60), "C4");                        // Natural
    EXPECT_EQ(Helper::midiNote2pitch(60, "#"), "B#3");                  // Sharp
    EXPECT_THROW(Helper::midiNote2pitch(60, "x"), std::runtime_error);  // Double Sharp

    // ===== MIDI Note 61 - C#4 =====
    EXPECT_THROW(Helper::midiNote2pitch(61, "bb"), std::runtime_error);  // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(61, "b"), "Db4");                   // Flat
    EXPECT_EQ(Helper::midiNote2pitch(61), "C#4");                        // Natural
    EXPECT_EQ(Helper::midiNote2pitch(61, "#"), "C#4");                   // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(61, "x"), "Bx3");                   // Double sharp

    // ===== MIDI Note 62 - D4 =====
    EXPECT_EQ(Helper::midiNote2pitch(62, "bb"), "Ebb4");                // Double flat
    EXPECT_THROW(Helper::midiNote2pitch(62, "b"), std::runtime_error);  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(62), "D4");                        // Natural
    EXPECT_THROW(Helper::midiNote2pitch(62, "#"), std::runtime_error);  // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(62, "x"), "Cx4");                  // Double sharp

    // ===== MIDI Note 63 - D#4 =====
    EXPECT_EQ(Helper::midiNote2pitch(63, "bb"), "Fbb4");                // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(63, "b"), "Eb4");                  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(63), "D#4");                       // Natural
    EXPECT_EQ(Helper::midiNote2pitch(63, "#"), "D#4");                  // Sharp
    EXPECT_THROW(Helper::midiNote2pitch(63, "x"), std::runtime_error);  // Double sharp

    // ===== MIDI Note 64 - E4 =====
    EXPECT_THROW(Helper::midiNote2pitch(64, "bb"), std::runtime_error);  // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(64, "b"), "Fb4");                   // Flat
    EXPECT_EQ(Helper::midiNote2pitch(64), "E4");                         // Natural
    EXPECT_THROW(Helper::midiNote2pitch(64, "#"), std::runtime_error);   // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(64, "x"), "Dx4");                   // Double sharp

    // ===== MIDI Note 65 - F4 =====
    EXPECT_EQ(Helper::midiNote2pitch(65, "bb"), "Gbb4");                // Double flat
    EXPECT_THROW(Helper::midiNote2pitch(65, "b"), std::runtime_error);  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(65), "F4");                        // Natural
    EXPECT_EQ(Helper::midiNote2pitch(65, "#"), "E#4");                  // Sharp
    EXPECT_THROW(Helper::midiNote2pitch(65, "x"), std::runtime_error);  // Double sharp

    // ===== MIDI Note 66 - F#4 =====
    EXPECT_THROW(Helper::midiNote2pitch(66, "bb"), std::runtime_error);  // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(66, "b"), "Gb4");                   // Flat
    EXPECT_EQ(Helper::midiNote2pitch(66), "F#4");                        // Natural
    EXPECT_EQ(Helper::midiNote2pitch(66, "#"), "F#4");                   // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(66, "x"), "Ex4");                   // Double sharp

    // ===== MIDI Note 67 - G4 =====
    EXPECT_EQ(Helper::midiNote2pitch(67, "bb"), "Abb4");                // Double flat
    EXPECT_THROW(Helper::midiNote2pitch(67, "b"), std::runtime_error);  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(67), "G4");                        // Natural
    EXPECT_THROW(Helper::midiNote2pitch(67, "#"), std::runtime_error);  // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(67, "x"), "Fx4");                  // Double sharp

    // ===== MIDI Note 68 - G#4 =====
    EXPECT_THROW(Helper::midiNote2pitch(68, "bb"), std::runtime_error);  // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(68, "b"), "Ab4");                   // Flat
    EXPECT_EQ(Helper::midiNote2pitch(68), "G#4");                        // Natural
    EXPECT_EQ(Helper::midiNote2pitch(68, "#"), "G#4");                   // Sharp
    EXPECT_THROW(Helper::midiNote2pitch(68, "x"), std::runtime_error);   // Double sharp

    // ===== MIDI Note 69 - A4 =====
    EXPECT_EQ(Helper::midiNote2pitch(69, "bb"), "Bbb4");                // Double flat
    EXPECT_THROW(Helper::midiNote2pitch(69, "b"), std::runtime_error);  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(69), "A4");                        // Natural
    EXPECT_THROW(Helper::midiNote2pitch(69, "#"), std::runtime_error);  // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(69, "x"), "Gx4");                  // Double sharp

    // ===== MIDI Note 70 - A#4 =====
    EXPECT_EQ(Helper::midiNote2pitch(70, "bb"), "Cbb5");                // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(70, "b"), "Bb4");                  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(70), "A#4");                       // Natural
    EXPECT_EQ(Helper::midiNote2pitch(70, "#"), "A#4");                  // Sharp
    EXPECT_THROW(Helper::midiNote2pitch(70, "x"), std::runtime_error);  // Double sharp

    // ===== MIDI Note 71 - B4 =====
    EXPECT_THROW(Helper::midiNote2pitch(71, "bb"), std::runtime_error);  // Double flat
    EXPECT_EQ(Helper::midiNote2pitch(71, "b"), "Cb5");                   // Flat
    EXPECT_EQ(Helper::midiNote2pitch(71), "B4");                         // Natural
    EXPECT_THROW(Helper::midiNote2pitch(71, "#"), std::runtime_error);   // Sharp
    EXPECT_EQ(Helper::midiNote2pitch(71, "x"), "Ax4");                   // Double sharp

    // ===== MIDI Note 72 - C5 =====
    EXPECT_EQ(Helper::midiNote2pitch(72, "bb"), "Dbb5");                // Double flat
    EXPECT_THROW(Helper::midiNote2pitch(72, "b"), std::runtime_error);  // Flat
    EXPECT_EQ(Helper::midiNote2pitch(72), "C5");                        // Natural
    EXPECT_EQ(Helper::midiNote2pitch(72, "#"), "B#4");                  // Sharp
    EXPECT_THROW(Helper::midiNote2pitch(72, "x"), std::runtime_error);  // Double sharp
}

// NOTE: This test is commented out because the MUSIC_XML::NOTE_TYPE enum
// does not include the _DOT and _DOT_DOT variants (MAXIMA_DOT, MAXIMA_DOT_DOT, etc.)
// These would need to be added to maiacore/include/maiacore/constants.h first
//
// TEST(ticks2noteType, basicNoteTypes) {
//   const int divisionsPerQuarterNote = 1024;
//   // Test cases commented out - enum values not available
// }

TEST(midiNote2octave, midiValues) {
    // Special case: rest -- a rest has no octave (empty optional, not a -2 sentinel)
    EXPECT_FALSE(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_REST).has_value());

    // Octave -1
    EXPECT_EQ(Helper::midiNote2octave(0), -1);
    EXPECT_EQ(Helper::midiNote2octave(1), -1);
    EXPECT_EQ(Helper::midiNote2octave(2), -1);
    EXPECT_EQ(Helper::midiNote2octave(3), -1);
    EXPECT_EQ(Helper::midiNote2octave(4), -1);
    EXPECT_EQ(Helper::midiNote2octave(5), -1);
    EXPECT_EQ(Helper::midiNote2octave(6), -1);
    EXPECT_EQ(Helper::midiNote2octave(7), -1);
    EXPECT_EQ(Helper::midiNote2octave(8), -1);
    EXPECT_EQ(Helper::midiNote2octave(9), -1);
    EXPECT_EQ(Helper::midiNote2octave(10), -1);
    EXPECT_EQ(Helper::midiNote2octave(11), -1);

    // Octave 0
    EXPECT_EQ(Helper::midiNote2octave(12), 0);
    EXPECT_EQ(Helper::midiNote2octave(13), 0);
    EXPECT_EQ(Helper::midiNote2octave(14), 0);
    EXPECT_EQ(Helper::midiNote2octave(15), 0);
    EXPECT_EQ(Helper::midiNote2octave(16), 0);
    EXPECT_EQ(Helper::midiNote2octave(17), 0);
    EXPECT_EQ(Helper::midiNote2octave(18), 0);
    EXPECT_EQ(Helper::midiNote2octave(19), 0);
    EXPECT_EQ(Helper::midiNote2octave(20), 0);
    EXPECT_EQ(Helper::midiNote2octave(21), 0);
    EXPECT_EQ(Helper::midiNote2octave(22), 0);
    EXPECT_EQ(Helper::midiNote2octave(23), 0);

    // Octave 1
    EXPECT_EQ(Helper::midiNote2octave(24), 1);
    EXPECT_EQ(Helper::midiNote2octave(25), 1);
    EXPECT_EQ(Helper::midiNote2octave(26), 1);
    EXPECT_EQ(Helper::midiNote2octave(27), 1);
    EXPECT_EQ(Helper::midiNote2octave(28), 1);
    EXPECT_EQ(Helper::midiNote2octave(29), 1);
    EXPECT_EQ(Helper::midiNote2octave(30), 1);
    EXPECT_EQ(Helper::midiNote2octave(31), 1);
    EXPECT_EQ(Helper::midiNote2octave(32), 1);
    EXPECT_EQ(Helper::midiNote2octave(33), 1);
    EXPECT_EQ(Helper::midiNote2octave(34), 1);
    EXPECT_EQ(Helper::midiNote2octave(35), 1);

    // Octave 2
    EXPECT_EQ(Helper::midiNote2octave(36), 2);
    EXPECT_EQ(Helper::midiNote2octave(37), 2);
    EXPECT_EQ(Helper::midiNote2octave(38), 2);
    EXPECT_EQ(Helper::midiNote2octave(39), 2);
    EXPECT_EQ(Helper::midiNote2octave(40), 2);
    EXPECT_EQ(Helper::midiNote2octave(41), 2);
    EXPECT_EQ(Helper::midiNote2octave(42), 2);
    EXPECT_EQ(Helper::midiNote2octave(43), 2);
    EXPECT_EQ(Helper::midiNote2octave(44), 2);
    EXPECT_EQ(Helper::midiNote2octave(45), 2);
    EXPECT_EQ(Helper::midiNote2octave(46), 2);
    EXPECT_EQ(Helper::midiNote2octave(47), 2);

    // Octave 3
    EXPECT_EQ(Helper::midiNote2octave(48), 3);
    EXPECT_EQ(Helper::midiNote2octave(49), 3);
    EXPECT_EQ(Helper::midiNote2octave(50), 3);
    EXPECT_EQ(Helper::midiNote2octave(51), 3);
    EXPECT_EQ(Helper::midiNote2octave(52), 3);
    EXPECT_EQ(Helper::midiNote2octave(53), 3);
    EXPECT_EQ(Helper::midiNote2octave(54), 3);
    EXPECT_EQ(Helper::midiNote2octave(55), 3);
    EXPECT_EQ(Helper::midiNote2octave(56), 3);
    EXPECT_EQ(Helper::midiNote2octave(57), 3);
    EXPECT_EQ(Helper::midiNote2octave(58), 3);
    EXPECT_EQ(Helper::midiNote2octave(59), 3);

    // Octave 4
    EXPECT_EQ(Helper::midiNote2octave(60), 4);
    EXPECT_EQ(Helper::midiNote2octave(61), 4);
    EXPECT_EQ(Helper::midiNote2octave(62), 4);
    EXPECT_EQ(Helper::midiNote2octave(63), 4);
    EXPECT_EQ(Helper::midiNote2octave(64), 4);
    EXPECT_EQ(Helper::midiNote2octave(65), 4);
    EXPECT_EQ(Helper::midiNote2octave(66), 4);
    EXPECT_EQ(Helper::midiNote2octave(67), 4);
    EXPECT_EQ(Helper::midiNote2octave(68), 4);
    EXPECT_EQ(Helper::midiNote2octave(69), 4);
    EXPECT_EQ(Helper::midiNote2octave(70), 4);
    EXPECT_EQ(Helper::midiNote2octave(71), 4);
}

TEST(PitchSpellingLegacy, MidiTableMatches) {
    for (const auto& entry : kLegacyMidiTable) {
        EXPECT_EQ(Helper::pitch2midiNote(entry.pitch), entry.midiNumber)
            << "pitch: " << entry.pitch;
    }
}

TEST(PitchSpelling, FullRangeMidiTable) {
    for (const auto& entry : kFullRangeMidiTable) {
        EXPECT_EQ(Helper::pitch2midiNote(entry.pitch), entry.midiNumber)
            << "pitch: " << entry.pitch;
    }
}

TEST(PitchSpelling, SplitPitchComponents) {
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    std::optional<int> octave;
    float alterValue = 0.0f;

    Helper::splitPitch("Dbb-1", pitchClass, pitchStep, octave, alterValue, alterSymbol);
    EXPECT_EQ(pitchClass, "Dbb");
    EXPECT_EQ(pitchStep, "D");
    EXPECT_EQ(octave, -1);
    EXPECT_FLOAT_EQ(alterValue, -2.0f);
    EXPECT_EQ(alterSymbol, "bb");

    Helper::splitPitch("F#11", pitchClass, pitchStep, octave, alterValue, alterSymbol);
    EXPECT_EQ(pitchClass, "F#");
    EXPECT_EQ(pitchStep, "F");
    EXPECT_EQ(octave, 11);
    EXPECT_FLOAT_EQ(alterValue, 1.0f);
    EXPECT_EQ(alterSymbol, "#");

    Helper::splitPitch("E", pitchClass, pitchStep, octave, alterValue, alterSymbol);
    EXPECT_EQ(pitchClass, "E");
    EXPECT_EQ(pitchStep, "E");
    EXPECT_EQ(octave, 4);
    EXPECT_FLOAT_EQ(alterValue, 0.0f);
    EXPECT_EQ(alterSymbol, "");

    Helper::splitPitch("rest", pitchClass, pitchStep, octave, alterValue, alterSymbol);
    EXPECT_EQ(pitchClass, "rest");
    EXPECT_EQ(pitchStep, "rest");
    // A rest has no octave (spec section 4.4/12.1): an empty optional, not a sentinel value such
    // as 0, which is a real octave.
    EXPECT_FALSE(octave.has_value());
    EXPECT_FLOAT_EQ(alterValue, 0.0f);
    EXPECT_EQ(alterSymbol, "");
}

TEST(splitPitch, acceptsQuarterTones) {
    std::string pc, step, sym; std::optional<int> oct; float alter;
    Helper::splitPitch("C1x4", pc, step, oct, alter, sym);
    EXPECT_EQ(sym, "1x");
    EXPECT_FLOAT_EQ(alter, 0.5f);
    EXPECT_EQ(oct, 4);
}

// Quarter-tone alters must round to the nearest MIDI number with ties broken upward (spec
// section 4.5), for both signs and across an octave boundary at 0.
TEST(PitchSpelling, QuarterToneRoundsTiesUpward) {
    EXPECT_EQ(Helper::pitch2midiNote("C1x4"), 61);
    EXPECT_EQ(Helper::pitch2midiNote("C3x4"), 62);
    EXPECT_EQ(Helper::pitch2midiNote("D1b4"), 62);
    EXPECT_EQ(Helper::pitch2midiNote("D3b4"), 61);
    EXPECT_EQ(Helper::pitch2midiNote("D1x4"), 63);
    EXPECT_EQ(Helper::pitch2midiNote("C1x-1"), 1);
}

TEST(PitchSpelling, Spelling2MidiNote) {
    EXPECT_EQ(Helper::spelling2midiNote("C", 0.0f, -1), 0);
    EXPECT_EQ(Helper::spelling2midiNote("C", 0.0f, 4), 60);
    EXPECT_EQ(Helper::spelling2midiNote("B", 2.0f, 11), 157);
    EXPECT_EQ(Helper::spelling2midiNote("D", -1.0f, 10), 133);

    // Must agree with Helper::pitch2midiNote() for the already-parsed components of the same pitch
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    std::optional<int> octave;
    float alterValue = 0.0f;
    Helper::splitPitch("F#11", pitchClass, pitchStep, octave, alterValue, alterSymbol);
    EXPECT_EQ(Helper::spelling2midiNote(pitchStep, alterValue, octave.value()),
              Helper::pitch2midiNote("F#11"));

    try {
        Helper::spelling2midiNote("H", 0.0f, 4);
        FAIL() << "Expected std::runtime_error for pitchStep: H";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string::npos, std::string(e.what()).find("Unknown diatonic pitch step"))
            << "message: " << e.what();
    }
}

// The components of a pitch this library can spell are an alter from -2 to 2 and an octave from
// -1 to 11. Anything else is rejected before the sum is converted to int, which is undefined
// behaviour for a value out of int's range, NaN and infinity included.
TEST(PitchSpelling, Spelling2MidiNoteRejectsComponentsNoPitchCanHave) {
    for (const float alter :
         {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
          -std::numeric_limits<float>::infinity(), 2.5f, -2.5f, 3.0e9f}) {
        const std::string message =
            thrownFirstLine([&] { Helper::spelling2midiNote("C", alter, 4); });
        EXPECT_NE(message.find("the alter value must be a finite number of semitones from -2 to 2"),
                  std::string::npos)
            << "alter " << alter << ": " << message;
    }

    for (const int octave : {-2, 12, 100000, -100000}) {
        const std::string message =
            thrownFirstLine([&] { Helper::spelling2midiNote("C", 0.0f, octave); });
        EXPECT_NE(message.find("the octave must be from -1 to 11"), std::string::npos)
            << "octave " << octave << ": " << message;
    }

    // The limits themselves are components of real spellings.
    EXPECT_EQ(Helper::spelling2midiNote("B", 2.0f, 11), 157);
    EXPECT_EQ(Helper::spelling2midiNote("D", -2.0f, -1), 0);
}

TEST(PitchSpelling, AcceptedEdgeCases) {
    EXPECT_EQ(Helper::pitch2midiNote("Cbb0"), 10);
    EXPECT_EQ(Helper::pitch2midiNote("Cb0"), 11);
    EXPECT_EQ(Helper::pitch2midiNote("Bx9"), 133);
    EXPECT_EQ(Helper::pitch2midiNote("Db10"), 133);
    EXPECT_EQ(Helper::pitch2midiNote("C-1"), 0);
    EXPECT_EQ(Helper::pitch2midiNote("Bx11"), 157);
    EXPECT_EQ(Helper::pitch2midiNote("C04"), 60);
    EXPECT_EQ(Helper::pitch2midiNote("rest"), -1);
    EXPECT_EQ(Helper::pitch2midiNote(""), -1);
}

TEST(PitchSpelling, RejectedEdgeCases) {
    for (const std::string pitch :
         {"Cb-1", "Cbb-1", "C12", "C#123", "H4", "C#4x", "C-", "C--1", "C-2", "C2x4", "C1234"}) {
        EXPECT_THROW(Helper::pitch2midiNote(pitch), std::runtime_error) << "pitch: " << pitch;
    }

    // Pin the exact Helper::splitPitch error message for one representative pitch per failure mode
    try {
        Helper::pitch2midiNote("H4");
        FAIL() << "Expected std::runtime_error for pitch: H4";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string::npos, std::string(e.what()).find("Unknown diatonic pitch"))
            << "message: " << e.what();
    }

    try {
        Helper::pitch2midiNote("C12");
        FAIL() << "Expected std::runtime_error for pitch: C12";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string::npos, std::string(e.what()).find("Invalid octave value"))
            << "message: " << e.what();
    }

    try {
        Helper::pitch2midiNote("C-");
        FAIL() << "Expected std::runtime_error for pitch: C-";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string::npos, std::string(e.what()).find("Unknown alter symbol"))
            << "message: " << e.what();
    }

    try {
        Helper::pitch2midiNote("Cb-1");
        FAIL() << "Expected std::runtime_error for pitch: Cb-1";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string::npos, std::string(e.what()).find("is below MIDI note 0"))
            << "message: " << e.what();
    }
}

TEST(PitchSpelling, MidiNote2PitchFullRange) {
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    std::optional<int> octave;
    float alterValue = 0.0f;

    for (const auto& entry : kFullRangeMidiTable) {
        Helper::splitPitch(entry.pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);
        EXPECT_EQ(Helper::midiNote2pitch(entry.midiNumber, alterSymbol), entry.pitch)
            << "pitch: " << entry.pitch;
    }
}

TEST(PitchSpelling, MidiNote2PitchRange) {
    EXPECT_EQ(Helper::midiNote2pitch(157, "x"), "Bx11");
    EXPECT_EQ(Helper::midiNote2pitch(5), "F-1");
    EXPECT_THROW(Helper::midiNote2pitch(158), std::runtime_error);
    EXPECT_THROW(Helper::midiNote2pitch(0, "#"), std::runtime_error);
    EXPECT_EQ(Helper::midiNote2pitches(0), std::vector<std::string>({"C-1", "Dbb-1"}));
    EXPECT_EQ(Helper::transposePitch("B11", 1), "B#11");
    EXPECT_THROW(Helper::transposePitch("B11", 2), std::runtime_error);
}

TEST(PitchSpelling, IsEnharmonic) {
    EXPECT_TRUE(Helper::isEnharmonic("E#4", "F4"));
    EXPECT_TRUE(Helper::isEnharmonic("B#3", "C4"));
    EXPECT_TRUE(Helper::isEnharmonic("Cb4", "B3"));
    EXPECT_TRUE(Helper::isEnharmonic("C#4", "Db4"));
    EXPECT_TRUE(Helper::isEnharmonic("rest", "rest"));
    EXPECT_FALSE(Helper::isEnharmonic("C4", "D4"));
    EXPECT_FALSE(Helper::isEnharmonic("C4", "C5"));

    for (const auto& entry : kFullRangeEnharmonicTable) {
        EXPECT_TRUE(Helper::isEnharmonic(entry.pitch, entry.defaultPitch))
            << "pitch: " << entry.pitch;
        EXPECT_TRUE(Helper::isEnharmonic(entry.pitch, entry.alternativePitch))
            << "pitch: " << entry.pitch;
    }
}

// isEnharmonic() compares exact pitch positions: by pitch2midiNote()'s ROUNDED MIDI numbers, a
// quarter tone and the semitone it rounds to would collide on one integer and be reported as the
// same pitch.
TEST(PitchSpelling, IsEnharmonicComparesExactPitchNotRoundedMidi) {
    // The two spellings of a single quarter tone: "C1x4" and "D3b4" are both exactly 60.5.
    EXPECT_TRUE(Helper::isEnharmonic("C1x4", "D3b4"));
    EXPECT_TRUE(Helper::isEnharmonic("E1b4", "D3x4"));  // both exactly 63.5

    // A quarter tone is NOT the semitone it rounds to, although pitch2midiNote() rounds 60.5 up
    // to 61, which is C#4's own value.
    EXPECT_FALSE(Helper::isEnharmonic("C1x4", "C#4"));
    EXPECT_FALSE(Helper::isEnharmonic("C1x4", "C4"));
    EXPECT_FALSE(Helper::isEnharmonic("E1b4", "E4"));

    // Semitone pairs, for comparison.
    EXPECT_TRUE(Helper::isEnharmonic("E#4", "F4"));
    EXPECT_FALSE(Helper::isEnharmonic("C4", "D4"));
}

// transposePitch() takes a float interval and computes on exact positions, so it can transpose BY
// half a semitone and keeps a quarter tone it transposes: rounding the pitch to a MIDI integer
// BEFORE applying the interval would destroy the quarter tone at the very first step.
TEST(PitchSpelling, TransposePitchMovesByAndPreservesQuarterTones) {
    // Transposing BY a quarter tone.
    EXPECT_EQ(Helper::transposePitch("C4", 0.5f, ""), "C1x4");
    EXPECT_EQ(Helper::transposePitch("C1x4", 0.5f, ""), "C#4");

    // Transposing a quarter tone BY whole semitones keeps the quarter tone ("D1x4", not "D4").
    // The spelling follows the base semitone the exact position rounds
    // up to, so 62.5 is spelled from "D#4" (alter +1) as "D1x4", and 58.5 from "B3" (alter 0) as
    // "B1b3" -- two spellings of the same rule, not two rules.
    EXPECT_EQ(Helper::transposePitch("C1x4", 2.0f, ""), "D1x4");
    EXPECT_EQ(Helper::transposePitch("C1x4", -2.0f, ""), "B1b3");

    // A whole-tone transposition of a whole-tone pitch is untouched by any of this.
    EXPECT_EQ(Helper::transposePitch("C4", 2.0f, ""), "D4");
    EXPECT_EQ(Helper::transposePitch("C4", 0.0f, ""), "C4");
    EXPECT_EQ(Helper::transposePitch("rest", 2.0f, ""), "rest");
}

TEST(PitchSpelling, TransposePitchRejectsAnIntervalOffTheQuarterToneGrid) {
    try {
        Helper::transposePitch("C4", 0.3f);
        FAIL() << "Expected std::runtime_error for a transposition off the quarter-tone grid";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("multiple of 0.5"), std::string::npos) << "message: " << what;
        EXPECT_NE(what.find("0.3"), std::string::npos) << "message: " << what;
    }
}

// An infinity must be rejected as non-finite by the validator itself: a multiple-of-0.5 check of
// the form `x * 2 != floor(x * 2)` is false for it (inf * 2 == inf and std::floor(inf) == inf),
// and past the validator "+inf" would fail far away with the unrelated message "Unknown accidental
// alter value: inf", while "-inf" would SILENTLY answer "rest" -- an infinite transposition
// quietly turning a note into a rest.
//
// The "got '<result>'" branch is what pins the -inf case specifically: a returned value fails the
// test and prints what came back, so a silent "rest" is reported as the wrong ANSWER it is, rather
// than being indistinguishable from a wrong error message.
//
// The message must also NAME the offending value, as the sibling off-grid test above requires of
// '0.3': asserting "finite" alone would let the value drop out of the message unnoticed. "nan" is
// matched without its quotes because std::to_string() writes a NaN
// with its sign bit set as "-nan(ind)" on this platform's C runtime.
TEST(PitchSpelling, TransposePitchRejectsNonFiniteIntervals) {
    const auto expectRejected = [](const char* label, const float interval, const char* value) {
        try {
            const std::string result = Helper::transposePitch("C4", interval);
            ADD_FAILURE() << label << ": expected std::runtime_error, got '" << result << "'";
        } catch (const std::runtime_error& e) {
            const std::string what = e.what();
            EXPECT_NE(what.find("finite"), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find(value), std::string::npos) << label << " message: " << what;
            EXPECT_EQ(what.find("Unknown accidental"), std::string::npos)
                << label << " message: " << what;
        }
    };

    const float infinity = std::numeric_limits<float>::infinity();
    expectRejected("+inf", infinity, "'inf'");
    expectRejected("-inf", -infinity, "'-inf'");
    expectRejected("nan", std::numeric_limits<float>::quiet_NaN(), "nan");
}

// A real pitch transposed out of the representable range raises at both ends, naming the pitch,
// the interval and the range. Below MIDI 0 it must not answer steps2pitch()'s rest sentinel:
// through Note::transpose() and Chord::transpose(), a note transposed too low would be silently
// deleted. Far above the top, the position must not reach an out-of-range int conversion.
// Transposing a REST still answers "rest" (pinned in
// TransposePitchMovesByAndPreservesQuarterTones).
TEST(PitchSpelling, TransposePitchRejectsAResultOutsideTheRepresentableRange) {
    struct Case {
        const char* pitch;
        float semitones;
        const char* quotedPitch;
        const char* interval;
    };
    const Case cases[] = {
        {"C4", -61.0f, "'C4'", "-61"},         // position -1, not "rest"
        {"C4", -3e9f, "'C4'", "-3000000000"},  // far below, not "rest"
        {"C1b-1", -0.5f, "'C1b-1'", "-0.5"},   // one quarter tone below the lowest pitch
        {"C4", 3e9f, "'C4'", "3000000000"},    // far above
        {"B11", 2.5f, "'B11'", "2.5"},         // 157.5: one quarter tone above "Bx11"
    };

    for (const auto& c : cases) {
        const std::string label = std::string(c.pitch) + " by " + c.interval;
        try {
            const std::string result = Helper::transposePitch(c.pitch, c.semitones);
            ADD_FAILURE() << label << ": expected std::runtime_error, got '" << result << "'";
        } catch (const std::runtime_error& e) {
            const std::string what = e.what();
            EXPECT_NE(what.find("outside the representable range"), std::string::npos)
                << label << " message: " << what;
            EXPECT_NE(what.find(c.quotedPitch), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find(c.interval), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find("C1b-1"), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find("Bx11"), std::string::npos) << label << " message: " << what;
        }
    }

    // Both ends of the range are still reachable: the lowest pitch is "C1b-1" (-0.5, which rounds,
    // ties upward, to MIDI 0) and the highest is "Bx11" (157).
    EXPECT_EQ(Helper::transposePitch("C-1", -0.5f, ""), "C1b-1");
    EXPECT_EQ(Helper::transposePitch("B11", 2.0f, "x"), "Bx11");
}

// transposePitch() reads its input as a Pitch before anything else, so every rest spelling is a
// rest and an invalid pitch string is rejected whatever the interval.
TEST(PitchSpelling, TransposePitchTreatsEveryRestSpellingAsARestAndParsesTheInputFirst) {
    // An empty string is a rest (splitPitch()'s rule): comparing the string with "rest" would let
    // "" fall through to the arithmetic as MIDI_REST (-1), and transposed up two semitones it
    // would answer "C#-1", a pitch conjured out of a rest.
    EXPECT_EQ(Helper::transposePitch("", 2.0f), "rest");
    EXPECT_EQ(Helper::transposePitch("", -2.0f), "rest");

    // An interval of 0 still parses the input: returning it unchanged without parsing would let an
    // invalid pitch string pass straight through, although the documentation promises a throw.
    try {
        const std::string result = Helper::transposePitch("H4", 0.0f);
        ADD_FAILURE() << "expected std::runtime_error for 'H4', got '" << result << "'";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("Unknown diatonic pitch"), std::string::npos)
            << "message: " << e.what();
    }
    EXPECT_EQ(Helper::transposePitch("Eb", 0.0f), "Eb");  // a valid input is still returned as is
}

// steps2pitch() converts its argument with static_cast<int>, which is undefined behaviour for a
// non-finite position, and it is bound to Python, so such a position is rejected before any cast,
// naming the value and the representable range. Without the check, +inf and nan would throw with
// a different message and -inf would silently answer "rest".
TEST(PitchSpelling, Steps2PitchRejectsANonFinitePositionBeforeAnyCast) {
    const auto expectRejected = [](const char* label, const float position, const char* value) {
        try {
            const std::string result = Helper::steps2pitch(position);
            ADD_FAILURE() << label << ": expected std::runtime_error, got '" << result << "'";
        } catch (const std::runtime_error& e) {
            const std::string what = e.what();
            EXPECT_NE(what.find("finite"), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find(value), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find("C1b-1"), std::string::npos) << label << " message: " << what;
            EXPECT_NE(what.find("Bx11"), std::string::npos) << label << " message: " << what;
        }
    };

    const float infinity = std::numeric_limits<float>::infinity();
    expectRejected("+inf", infinity, "'inf'");
    expectRejected("-inf", -infinity, "'-inf'");
    expectRejected("nan", std::numeric_limits<float>::quiet_NaN(), "nan");
}

// Above the range too: a position above int's range would be an undefined-behaviour cast, and one
// merely above "Bx11" would fail with midiNote2pitch()'s unrelated octave message. Both are
// rejected, naming the value and the range.
TEST(PitchSpelling, Steps2PitchRejectsAPositionAboveTheRepresentableRange) {
    EXPECT_EQ(Helper::steps2pitch(157.0f, "x"), "Bx11");  // the ceiling itself is spellable

    const float positions[] = {157.5f, 158.0f, 3e9f, std::numeric_limits<float>::max()};
    for (const float position : positions) {
        const std::string value = std::to_string(position);
        try {
            const std::string result = Helper::steps2pitch(position, "x");
            ADD_FAILURE() << value << ": expected std::runtime_error, got '" << result << "'";
        } catch (const std::runtime_error& e) {
            const std::string what = e.what();
            EXPECT_NE(what.find("above the representable range"), std::string::npos)
                << value << " message: " << what;
            EXPECT_NE(what.find(value), std::string::npos) << value << " message: " << what;
            EXPECT_NE(what.find("Bx11"), std::string::npos) << value << " message: " << what;
        }
    }
}

// Every pitch position is a multiple of 0.5. steps2pitch() rejects any other position, naming
// it, rather than snapping it onto the grid (60.45 is not "C1x4") or failing with an unrelated
// message.
TEST(PitchSpelling, Steps2PitchRejectsAPositionOffTheQuarterToneGrid) {
    const float positions[] = {60.45f, 60.25f, -0.3f};
    for (const float position : positions) {
        const std::string value = std::to_string(position);
        try {
            const std::string result = Helper::steps2pitch(position);
            ADD_FAILURE() << value << ": expected std::runtime_error, got '" << result << "'";
        } catch (const std::runtime_error& e) {
            const std::string what = e.what();
            EXPECT_NE(what.find("multiple of 0.5"), std::string::npos)
                << value << " message: " << what;
            EXPECT_NE(what.find(value), std::string::npos) << value << " message: " << what;
        }
    }
}

// Below MIDI note 0 steps2pitch() answers the rest sentinel, parallel to midiNote2pitch() for a
// negative MIDI number, and "below MIDI 0" means what it means everywhere else in this library:
// the position ROUNDS, ties upward, to a negative MIDI number. So -0.5, which rounds to MIDI 0, is
// "C1b-1", a pitch this library holds (Pitch("C1b-1") is constructible), not a rest.
TEST(PitchSpelling, Steps2PitchAnswersARestOnlyBelowMidiZero) {
    EXPECT_EQ(Helper::steps2pitch(-0.5f), "C1b-1");
    EXPECT_EQ(Helper::steps2pitch(0.0f), "C-1");
    EXPECT_EQ(Helper::steps2pitch(-1.0f), "rest");
    EXPECT_EQ(Helper::steps2pitch(-3e9f), "rest");
    EXPECT_EQ(Helper::steps2pitch(-std::numeric_limits<float>::max()), "rest");
}

TEST(Helper, GetLibraryVersion) {
    const std::string version = Helper::getLibraryVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_EQ(version.find('"'), std::string::npos);
    EXPECT_TRUE(std::regex_match(version, std::regex("^[0-9]+\\.[0-9]+\\.[0-9]+$")))
        << "version: " << version;
}

// ===== Accidental spelling: exact, and independent of the locale ===== //

// Exactly one of the nine alters, or an error naming the value: a value near one is not rounded
// onto it, as a one-decimal text match would.
TEST(AccidentalSpelling, AlterValueMustBeExactlyOneOfTheNine) {
    for (const float alter :
         {0.46f, 0.54f, 1.04f, 0.25f, 2.5f, std::numeric_limits<float>::quiet_NaN()}) {
        for (const bool name : {false, true}) {
            const std::string message = thrownFirstLine(
                [&] { name ? Helper::alterValue2Name(alter) : Helper::alterValue2symbol(alter); });
            EXPECT_NE(message.find("Unknown accidental alter value: " + std::to_string(alter)),
                      std::string::npos)
                << "alter " << alter << ": " << message;
        }
    }
}

TEST(AccidentalSpelling, NegativeZeroIsTheNatural) {
    EXPECT_EQ(Helper::alterValue2symbol(-0.0f), "");
    EXPECT_EQ(Helper::alterValue2Name(-0.0f), "natural");
}

TEST(AccidentalSpelling, SharpSharpIsADoubleSharp) {
    EXPECT_EQ(Helper::alterName2symbol("sharp-sharp"), "x");
    EXPECT_EQ(Helper::alterName2symbol("double-sharp"), "x");
}

// A C++ host may set a comma-decimal global C++ locale. Nothing in the spelling of a pitch, or in
// the <alter> text Note::toXML() writes, may depend on it.
TEST(AccidentalSpelling, IsIndependentOfTheGlobalCppLocale) {
    const std::string localeName = installedCommaDecimalLocale();
    if (localeName.empty()) {
        GTEST_SKIP() << "no comma-decimal locale is installed";
    }

    const ScopedGlobalLocale commaLocale(localeName);
    std::ostringstream probe;
    probe << std::fixed << std::setprecision(1) << 0.5;
    ASSERT_EQ(probe.str(), "0,5") << "the locale does not use a decimal comma";

    EXPECT_EQ(Helper::alterValue2symbol(0.5f), "1x");
    EXPECT_EQ(Helper::alterValue2symbol(0.0f), "");
    EXPECT_EQ(Helper::alterValue2Name(-1.5f), "three-quarters-flat");
    EXPECT_EQ(Note("C4").getPitch(), "C4");
    EXPECT_EQ(Note("C1x4").getPitch(), "C1x4");

    const std::string quarterSharp = Note("C1x4").toXML();
    EXPECT_NE(quarterSharp.find("<alter>0.5</alter>"), std::string::npos) << quarterSharp;
    EXPECT_NE(quarterSharp.find("<accidental>quarter-sharp</accidental>"), std::string::npos)
        << quarterSharp;
    const std::string threeQuartersFlat = Note("E3b4").toXML();
    EXPECT_NE(threeQuartersFlat.find("<alter>-1.5</alter>"), std::string::npos)
        << threeQuartersFlat;
}

// ===== Melodic contour differences: exact for quarter tones ===== //

// The result's float type expresses a quarter tone exactly, so the difference is computed rather
// than rejected: a neutral third (3.5 semitones) against a major third (4) differs by half a
// semitone.
TEST(MelodyContour, QuarterToneIntervalsAreComputedExactly) {
    const std::vector<Note> reference = {Note("C4"), Note("E4"), Note("G4")};
    const std::vector<Note> neutralThird = {Note("C4"), Note("E1b4"), Note("G4")};
    EXPECT_EQ(Helper::getSemitonesDifferenceBetweenMelodies(reference, neutralThird),
              (std::vector<float>{0.5f, -0.5f}));

    // Transposed by a quarter tone and two semitones: the same contour, so no difference.
    const std::vector<Note> transposed = {Note("D1x4"), Note("F3x4"), Note("A1x4")};
    EXPECT_EQ(Helper::getSemitonesDifferenceBetweenMelodies(reference, transposed),
              (std::vector<float>{0.0f, 0.0f}));

    EXPECT_NEAR(Helper::calculateMelodyEuclideanSimilarity(reference, neutralThird),
                1.0f / (1.0f + std::sqrt(0.5f)), 1e-6f);
}

// A pair that includes a rest counts as an interval of 0.
TEST(MelodyContour, APairWithARestCountsAsNoInterval) {
    const std::vector<Note> reference = {Note("C4"), Note("E4"), Note("G4")};
    const std::vector<Note> withRest = {Note("C4"), Note("rest"), Note("G1x4")};
    EXPECT_EQ(Helper::getSemitonesDifferenceBetweenMelodies(reference, withRest),
              (std::vector<float>{4.0f, 3.0f}));
}

// ===== pitch2freq(): whole-tone spellings only ===== //

// pitch2freq() has a table entry only for pitch classes spelled with the whole-tone accidentals; a
// quarter tone is refused rather than answered with an approximate frequency.
TEST(PitchToFrequency, AQuarterToneIsRefusedNotApproximated) {
    EXPECT_NEAR(Helper::pitch2freq("A4"), 440.0f, 0.01f);
    for (const char* quarterTone : {"C1x4", "D3b4", "E1b4", "F3x4"}) {
        const std::string message = thrownFirstLine([&] { Helper::pitch2freq(quarterTone); });
        EXPECT_NE(message.find("Pitch not found"), std::string::npos)
            << quarterTone << ": " << message;
    }
}
