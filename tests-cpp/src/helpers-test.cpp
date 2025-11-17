#include <gtest/gtest.h>

#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

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
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_000), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_001), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_002), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_003), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_004), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_005), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_006), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_007), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_008), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_009), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_010), -1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_011), -1);

// Octave 0
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_012), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_013), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_014), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_015), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_016), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_017), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_018), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_019), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_020), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_021), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_022), 0);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_023), 0);

// Octave 1
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_024), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_025), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_026), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_027), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_028), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_029), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_030), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_031), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_032), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_033), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_034), 1);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_035), 1);

// Octave 2
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_036), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_037), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_038), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_039), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_040), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_041), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_042), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_043), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_044), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_045), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_046), 2);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_047), 2);

// Octave 3
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_048), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_049), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_050), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_051), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_052), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_053), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_054), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_055), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_056), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_057), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_058), 3);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_059), 3);

// Octave 4
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_060), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_061), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_062), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_063), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_064), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_065), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_066), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_067), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_068), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_069), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_070), 4);
EXPECT_EQ(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_071), 4);
}
