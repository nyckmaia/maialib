#include <gtest/gtest.h>

#include <regex>

#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"
#include "pitch-spelling-legacy-data.h"

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
std::pair<int, int> result01 = Helper::freq2midiNote(
    30.1f);  // base frequency = 29.135, base midiNote = 22
EXPECT_EQ(result01.first, 23);
EXPECT_EQ(result01.second, -44);

std::pair<int, int> result02 = Helper::freq2midiNote(
    202.0f);  // base frequency = 195.998, base midiNote = 55
EXPECT_EQ(result02.first, 56);
EXPECT_EQ(result02.second, -48);

std::pair<int, int> result03 = Helper::freq2midiNote(
    481.0f);  // base frequency = 466.164, base midiNote = 70
EXPECT_EQ(result03.first, 71);
EXPECT_EQ(result03.second, -46);

std::pair<int, int> result04 = Helper::freq2midiNote(
    1712.0f);  // base frequency = 1661.219, base midiNote = 92
EXPECT_EQ(result04.first, 93);
EXPECT_EQ(result04.second, -48);

std::pair<int, int> result05 = Helper::freq2midiNote(
    3270.0f);  // base frequency = 3135.963, base midiNote = 103
EXPECT_EQ(result05.first, 104);
EXPECT_EQ(result05.second, -28);

std::pair<int, int> result06 = Helper::freq2midiNote(
    4080.0f);  // base frequency =3951.056, base midiNote = 107
EXPECT_EQ(result06.first, 108);
EXPECT_EQ(result06.second, -44);

// b) Testing for frequencies with values slighter smaller than the base
// frequency. the output note must be the base MidiNote -1. So cents are
// positive and smaller than 50.

std::pair<int, int> result07 = Helper::freq2midiNote(
    28.0f);  // base frequency = 29.135, base midiNote = 22
EXPECT_EQ(result07.first, 21);
EXPECT_EQ(result07.second, 31);

std::pair<int, int> result08 = Helper::freq2midiNote(
    189.0f);  // base frequency = 195.998, base midiNote = 55
EXPECT_EQ(result08.first, 54);
EXPECT_EQ(result08.second, 37);

std::pair<int, int> result09 = Helper::freq2midiNote(
    450.0f);  // base frequency = 466.164, base midiNote = 70
EXPECT_EQ(result09.first, 69);
EXPECT_EQ(result09.second, 39);

std::pair<int, int> result10 = Helper::freq2midiNote(
    1605.0f);  // base frequency = 1661.219, base midiNote = 92
EXPECT_EQ(result10.first, 91);
EXPECT_EQ(result10.second, 40);

std::pair<int, int> result11 = Helper::freq2midiNote(
    3035.0f);  // base frequency = 3135.963, base midiNote = 103
EXPECT_EQ(result11.first, 102);
EXPECT_EQ(result11.second, 43);

std::pair<int, int> result12 = Helper::freq2midiNote(
    3828.0f);  // base frequency =3951.056, base midiNote = 107
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
EXPECT_EQ(Helper::midiNote2pitch(60, "bb"), "Dbb4"); // Double flat
EXPECT_THROW(Helper::midiNote2pitch(60, "b"), std::runtime_error); // Flat 
EXPECT_EQ(Helper::midiNote2pitch(60), "C4"); // Natural
EXPECT_EQ(Helper::midiNote2pitch(60, "#"), "B#3"); // Sharp
EXPECT_THROW(Helper::midiNote2pitch(60, "x"), std::runtime_error); // Double Sharp


// ===== MIDI Note 61 - C#4 =====
EXPECT_THROW(Helper::midiNote2pitch(61, "bb"), std::runtime_error);        // Double flat
EXPECT_EQ(Helper::midiNote2pitch(61, "b"), "Db4");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(61), "C#4");                               // Natural
EXPECT_EQ(Helper::midiNote2pitch(61, "#"), "C#4");                          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(61, "x"), "Bx3");                          // Double sharp

// ===== MIDI Note 62 - D4 =====
EXPECT_EQ(Helper::midiNote2pitch(62, "bb"), "Ebb4");                        // Double flat
EXPECT_THROW(Helper::midiNote2pitch(62, "b"), std::runtime_error);          // Flat
EXPECT_EQ(Helper::midiNote2pitch(62), "D4");                                // Natural
EXPECT_THROW(Helper::midiNote2pitch(62, "#"), std::runtime_error);          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(62, "x"), "Cx4");                          // Double sharp

// ===== MIDI Note 63 - D#4 =====
EXPECT_EQ(Helper::midiNote2pitch(63, "bb"), "Fbb4");                        // Double flat
EXPECT_EQ(Helper::midiNote2pitch(63, "b"), "Eb4");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(63), "D#4");                               // Natural
EXPECT_EQ(Helper::midiNote2pitch(63, "#"), "D#4");                          // Sharp
EXPECT_THROW(Helper::midiNote2pitch(63, "x"), std::runtime_error);          // Double sharp

// ===== MIDI Note 64 - E4 =====
EXPECT_THROW(Helper::midiNote2pitch(64, "bb"), std::runtime_error);         // Double flat
EXPECT_EQ(Helper::midiNote2pitch(64, "b"), "Fb4");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(64), "E4");                                // Natural
EXPECT_THROW(Helper::midiNote2pitch(64, "#"), std::runtime_error);          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(64, "x"), "Dx4");                          // Double sharp

// ===== MIDI Note 65 - F4 =====
EXPECT_EQ(Helper::midiNote2pitch(65, "bb"), "Gbb4");                        // Double flat
EXPECT_THROW(Helper::midiNote2pitch(65, "b"), std::runtime_error);          // Flat
EXPECT_EQ(Helper::midiNote2pitch(65), "F4");                                // Natural
EXPECT_EQ(Helper::midiNote2pitch(65, "#"), "E#4");                          // Sharp
EXPECT_THROW(Helper::midiNote2pitch(65, "x"), std::runtime_error);          // Double sharp

// ===== MIDI Note 66 - F#4 =====
EXPECT_THROW(Helper::midiNote2pitch(66, "bb"), std::runtime_error);         // Double flat
EXPECT_EQ(Helper::midiNote2pitch(66, "b"), "Gb4");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(66), "F#4");                               // Natural
EXPECT_EQ(Helper::midiNote2pitch(66, "#"), "F#4");                          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(66, "x"), "Ex4");                          // Double sharp

// ===== MIDI Note 67 - G4 =====
EXPECT_EQ(Helper::midiNote2pitch(67, "bb"), "Abb4");                        // Double flat
EXPECT_THROW(Helper::midiNote2pitch(67, "b"), std::runtime_error);          // Flat
EXPECT_EQ(Helper::midiNote2pitch(67), "G4");                                // Natural
EXPECT_THROW(Helper::midiNote2pitch(67, "#"), std::runtime_error);          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(67, "x"), "Fx4");                          // Double sharp

// ===== MIDI Note 68 - G#4 =====
EXPECT_THROW(Helper::midiNote2pitch(68, "bb"), std::runtime_error);         // Double flat
EXPECT_EQ(Helper::midiNote2pitch(68, "b"), "Ab4");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(68), "G#4");                               // Natural
EXPECT_EQ(Helper::midiNote2pitch(68, "#"), "G#4");                          // Sharp
EXPECT_THROW(Helper::midiNote2pitch(68, "x"), std::runtime_error);          // Double sharp

// ===== MIDI Note 69 - A4 =====
EXPECT_EQ(Helper::midiNote2pitch(69, "bb"), "Bbb4");                        // Double flat
EXPECT_THROW(Helper::midiNote2pitch(69, "b"), std::runtime_error);          // Flat
EXPECT_EQ(Helper::midiNote2pitch(69), "A4");                                // Natural
EXPECT_THROW(Helper::midiNote2pitch(69, "#"), std::runtime_error);          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(69, "x"), "Gx4");                          // Double sharp

// ===== MIDI Note 70 - A#4 =====
EXPECT_EQ(Helper::midiNote2pitch(70, "bb"), "Cbb5");                        // Double flat
EXPECT_EQ(Helper::midiNote2pitch(70, "b"), "Bb4");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(70), "A#4");                               // Natural
EXPECT_EQ(Helper::midiNote2pitch(70, "#"), "A#4");                          // Sharp
EXPECT_THROW(Helper::midiNote2pitch(70, "x"), std::runtime_error);          // Double sharp

// ===== MIDI Note 71 - B4 =====
EXPECT_THROW(Helper::midiNote2pitch(71, "bb"), std::runtime_error);         // Double flat
EXPECT_EQ(Helper::midiNote2pitch(71, "b"), "Cb5");                          // Flat
EXPECT_EQ(Helper::midiNote2pitch(71), "B4");                                // Natural
EXPECT_THROW(Helper::midiNote2pitch(71, "#"), std::runtime_error);          // Sharp
EXPECT_EQ(Helper::midiNote2pitch(71, "x"), "Ax4");                          // Double sharp

// ===== MIDI Note 72 - C5 =====
EXPECT_EQ(Helper::midiNote2pitch(72, "bb"), "Dbb5");                        // Double flat
EXPECT_THROW(Helper::midiNote2pitch(72, "b"), std::runtime_error);          // Flat
EXPECT_EQ(Helper::midiNote2pitch(72), "C5");                                // Natural
EXPECT_EQ(Helper::midiNote2pitch(72, "#"), "B#4");                          // Sharp
EXPECT_THROW(Helper::midiNote2pitch(72, "x"), std::runtime_error);          // Double sharp

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
// Special case: rest
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_REST), -2);

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
    EXPECT_EQ(Helper::pitch2midiNote(entry.pitch), entry.midiNumber) << "pitch: " << entry.pitch;
}
}

TEST(PitchSpelling, FullRangeMidiTable) {
for (const auto& entry : kFullRangeMidiTable) {
    EXPECT_EQ(Helper::pitch2midiNote(entry.pitch), entry.midiNumber) << "pitch: " << entry.pitch;
}
}

TEST(PitchSpelling, SplitPitchComponents) {
std::string pitchClass;
std::string pitchStep;
std::string alterSymbol;
int octave = 0;
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
EXPECT_EQ(octave, 0);
EXPECT_FLOAT_EQ(alterValue, 0.0f);
EXPECT_EQ(alterSymbol, "");
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
int octave = 0;
float alterValue = 0.0f;
Helper::splitPitch("F#11", pitchClass, pitchStep, octave, alterValue, alterSymbol);
EXPECT_EQ(Helper::spelling2midiNote(pitchStep, alterValue, octave),
          Helper::pitch2midiNote("F#11"));

try {
    Helper::spelling2midiNote("H", 0.0f, 4);
    FAIL() << "Expected std::runtime_error for pitchStep: H";
} catch (const std::runtime_error& e) {
    EXPECT_NE(std::string::npos, std::string(e.what()).find("Unknown diatonic pitch step"))
        << "message: " << e.what();
}
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
     {"Cb-1", "Cbb-1", "C12", "C#123", "H4", "C#4x", "C-", "C--1", "C-2", "C1x4", "C1234"}) {
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
int octave = 0;
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

TEST(Helper, GetLibraryVersion) {
const std::string version = Helper::getLibraryVersion();
EXPECT_FALSE(version.empty());
EXPECT_EQ(version.find('"'), std::string::npos);
EXPECT_TRUE(std::regex_match(version, std::regex("^[0-9]+\\.[0-9]+\\.[0-9]+$")))
    << "version: " << version;
}
