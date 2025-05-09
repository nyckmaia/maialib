#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <array>
#include <cmath>  // std::pow
#include <map>
#include <string>
#include <vector>

#include "maiacore/log.h"

const float HAVE_THIRD_VALUE = 1.0f;         // 2^0
const float HAVE_FIFTH_VALUE = 2.0f;         // 2^1
const float HAVE_SEVENTH_VALUE = 4.0f;       // 2^2
const float HAVE_NINTH_VALUE = 8.0f;         // 2^3
const float HAVE_ELEVENTH_VALUE = 16.0f;     // 2^4
const float HAVE_THIRDTEENTH_VALUE = 32.0f;  // 2^5

const std::vector<float> c_haveComplexityValues{HAVE_THIRD_VALUE,    HAVE_FIFTH_VALUE,
                                                HAVE_SEVENTH_VALUE,  HAVE_NINTH_VALUE,
                                                HAVE_ELEVENTH_VALUE, HAVE_THIRDTEENTH_VALUE};

const std::map<int, std::pair<std::string, std::string>> c_mapAscFifthTonality = {
    {0, std::make_pair("C", "Am")},   {1, std::make_pair("G", "Em")},
    {2, std::make_pair("D", "Bm")},   {3, std::make_pair("A", "F#m")},
    {4, std::make_pair("E", "C#m")},  {5, std::make_pair("B", "G#m")},
    {6, std::make_pair("F#", "D#m")}, {7, std::make_pair("C#", "Bbm")},
    {8, std::make_pair("G#", "Fm")},  {9, std::make_pair("D#", "Cm")},
    {10, std::make_pair("A#", "Gm")}, {11, std::make_pair("E#", "Dm")},
};

const std::map<int, std::pair<std::string, std::string>> c_mapDescFifthTonality = {
    {0, std::make_pair("C", "Am")},   {1, std::make_pair("G", "Em")},
    {2, std::make_pair("D", "Bm")},   {3, std::make_pair("A", "F#m")},
    {4, std::make_pair("E", "C#m")},  {5, std::make_pair("B", "G#m")},
    {6, std::make_pair("Gb", "D#m")}, {7, std::make_pair("Db", "Bbm")},
    {8, std::make_pair("Ab", "Fm")},  {9, std::make_pair("Eb", "Cm")},
    {10, std::make_pair("Bb", "Gm")}, {11, std::make_pair("F", "Dm")},
};

const std::map<std::string, std::pair<int, bool>> c_mapKeyName2FifthCirCleMode = {
    {"C", std::make_pair(0, true)},    {"G", std::make_pair(1, true)},
    {"D", std::make_pair(2, true)},    {"A", std::make_pair(3, true)},
    {"E", std::make_pair(4, true)},    {"B", std::make_pair(5, true)},
    {"F#", std::make_pair(6, true)},   {"C#", std::make_pair(7, true)},
    {"G#", std::make_pair(8, true)},   {"D#", std::make_pair(9, true)},
    {"A#", std::make_pair(10, true)},  {"E#", std::make_pair(11, true)},
    {"Gb", std::make_pair(-6, true)},  {"Db", std::make_pair(-5, true)},
    {"Ab", std::make_pair(-4, true)},  {"Eb", std::make_pair(-3, true)},
    {"Bb", std::make_pair(-2, true)},  {"F", std::make_pair(-1, true)},
    {"Am", std::make_pair(0, false)},  {"Em", std::make_pair(1, false)},
    {"Bm", std::make_pair(2, false)},  {"F#m", std::make_pair(3, false)},
    {"C#m", std::make_pair(4, false)}, {"G#m", std::make_pair(5, false)},
    {"D#m", std::make_pair(6, false)}, {"Bbm", std::make_pair(-5, false)},
    {"Fm", std::make_pair(-4, false)}, {"Cm", std::make_pair(-3, false)},
    {"Gm", std::make_pair(-2, false)}, {"Dm", std::make_pair(-1, false)}};

enum class RhythmFigure {
    MAXIMA,
    LONG,
    BREVE,
    WHOLE,
    HALF,
    QUARTER,
    EIGHTH,
    N16TH,
    N32ND,
    N64TH,
    N128TH,
    N256TH,
    N512TH,
    N1024TH,
};

const std::map<int, RhythmFigure> c_mapTimeSignatureLower_Duration = {
    {1, RhythmFigure::WHOLE},    {2, RhythmFigure::HALF},      {4, RhythmFigure::QUARTER},
    {8, RhythmFigure::EIGHTH},   {16, RhythmFigure::N16TH},    {32, RhythmFigure::N32ND},
    {64, RhythmFigure::N64TH},   {128, RhythmFigure::N128TH},  {256, RhythmFigure::N256TH},
    {512, RhythmFigure::N512TH}, {1024, RhythmFigure::N1024TH}};

// ===== HARMONY DEGREES ===== //
const std::array<int, 7> c_harmonyDegrees = {1, 2, 3, 4, 5, 6, 7};
const std::array<std::string, 7> c_harmonyKeyDegrees = {"I", "II", "III", "IV", "V", "VI", "VII"};

// ===== SCALES ===== //
// Diatonic Scales (Pitch Steps)
const std::array<std::string, 7> c_C_diatonicScale = {"C", "D", "E", "F", "G", "A", "B"};
const std::array<std::string, 7> c_D_diatonicScale = {"D", "E", "F", "G", "A", "B", "C"};
const std::array<std::string, 7> c_E_diatonicScale = {"E", "F", "G", "A", "B", "C", "D"};
const std::array<std::string, 7> c_F_diatonicScale = {"F", "G", "A", "B", "C", "D", "E"};
const std::array<std::string, 7> c_G_diatonicScale = {"G", "A", "B", "C", "D", "E", "F"};
const std::array<std::string, 7> c_A_diatonicScale = {"A", "B", "C", "D", "E", "F", "G"};
const std::array<std::string, 7> c_B_diatonicScale = {"B", "C", "D", "E", "F", "G", "A"};

const std::array<std::string, 12> c_chromaticSharpScale = {"C",  "C#", "D",  "D#", "E",  "F",
                                                           "F#", "G",  "G#", "A",  "A#", "B"};
const std::array<std::string, 12> c_chromaticFlatScale = {"C",  "Db", "D",  "Eb", "Fb", "F",
                                                          "Gb", "G",  "Ab", "A",  "Bb", "B"};
const std::array<std::string, 12> c_chromaticDoubleSharpScale = {
    "C", "Bx", "Cx", "D#", "Dx", "E#", "Ex", "Fx", "G#", "Gx", "A#", "Ax"};
const std::array<std::string, 12> c_chromaticDoubleFlatScale = {
    "C", "Db", "Ebb", "Fbb", "Fb", "Gbb", "Gb", "Abb", "Ab", "Bbb", "Bb", "Cb"};
const std::array<std::string, 4> c_alterSymbol = {"bb", "b", "#", "x"};

// ===== PIANO WHITE KEYS ===== //
const std::array<std::string, 71> c_pianoWhiteKeys = {
    "C0", "D0", "E0", "F0", "G0", "A0", "B0", "C1", "D1", "E1", "F1", "G1", "A1", "B1", "C2",
    "D2", "E2", "F2", "G2", "A2", "B2", "C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4", "D4",
    "E4", "F4", "G4", "A4", "B4", "C5", "D5", "E5", "F5", "G5", "A5", "B5", "C6", "D6", "E6",
    "F6", "G6", "A6", "B6", "C7", "D7", "E7", "F7", "G7", "A7", "B7", "C8", "D8", "E8", "F8",
    "G8", "A8", "B8", "C9", "D9", "E9", "F9", "G9", "A9", "B9", "C10"};

// ===== STACK POSITION WEIGHT MAP ===== //
const std::map<char, int> c_C_stackPositionWeightMap = {
    {'C', std::pow(2, 7)}, {'E', std::pow(2, 6)}, {'G', std::pow(2, 5)}, {'B', std::pow(2, 4)},
    {'D', std::pow(2, 3)}, {'F', std::pow(2, 2)}, {'A', std::pow(2, 1)}};

const std::map<char, int> c_D_stackPositionWeightMap = {
    {'D', std::pow(2, 7)}, {'F', std::pow(2, 6)}, {'A', std::pow(2, 5)}, {'C', std::pow(2, 4)},
    {'E', std::pow(2, 3)}, {'G', std::pow(2, 2)}, {'B', std::pow(2, 1)}};

const std::map<char, int> c_E_stackPositionWeightMap = {
    {'E', std::pow(2, 7)}, {'G', std::pow(2, 6)}, {'B', std::pow(2, 5)}, {'D', std::pow(2, 4)},
    {'F', std::pow(2, 3)}, {'A', std::pow(2, 2)}, {'C', std::pow(2, 1)}};

const std::map<char, int> c_F_stackPositionWeightMap = {
    {'F', std::pow(2, 7)}, {'A', std::pow(2, 6)}, {'C', std::pow(2, 5)}, {'E', std::pow(2, 4)},
    {'G', std::pow(2, 3)}, {'B', std::pow(2, 2)}, {'D', std::pow(2, 1)}};

const std::map<char, int> c_G_stackPositionWeightMap = {
    {'G', std::pow(2, 7)}, {'B', std::pow(2, 6)}, {'D', std::pow(2, 5)}, {'F', std::pow(2, 4)},
    {'A', std::pow(2, 3)}, {'C', std::pow(2, 2)}, {'E', std::pow(2, 1)}};

const std::map<char, int> c_A_stackPositionWeightMap = {
    {'A', std::pow(2, 7)}, {'C', std::pow(2, 6)}, {'E', std::pow(2, 5)}, {'G', std::pow(2, 4)},
    {'B', std::pow(2, 3)}, {'D', std::pow(2, 2)}, {'F', std::pow(2, 1)}};

const std::map<char, int> c_B_stackPositionWeightMap = {
    {'B', std::pow(2, 7)}, {'D', std::pow(2, 6)}, {'F', std::pow(2, 5)}, {'A', std::pow(2, 4)},
    {'C', std::pow(2, 3)}, {'E', std::pow(2, 2)}, {'G', std::pow(2, 1)}};

// ===== CLOSE STACK IN THIRDS OCTAVES MAP ===== //
const std::map<char, int> c_C_closeStackOctavesMap = {{'C', 4}, {'E', 4}, {'G', 4}, {'B', 4},
                                                      {'D', 5}, {'F', 5}, {'A', 5}};

const std::map<char, int> c_D_closeStackOctavesMap = {{'D', 4}, {'F', 4}, {'A', 4}, {'C', 5},
                                                      {'E', 5}, {'G', 5}, {'B', 5}};

const std::map<char, int> c_E_closeStackOctavesMap = {{'E', 4}, {'G', 4}, {'B', 4}, {'D', 5},
                                                      {'F', 5}, {'A', 5}, {'C', 6}};

const std::map<char, int> c_F_closeStackOctavesMap = {{'F', 4}, {'A', 4}, {'C', 5}, {'E', 5},
                                                      {'G', 5}, {'B', 5}, {'D', 6}};

const std::map<char, int> c_G_closeStackOctavesMap = {{'G', 4}, {'B', 4}, {'D', 5}, {'F', 5},
                                                      {'A', 5}, {'C', 6}, {'E', 6}};

const std::map<char, int> c_A_closeStackOctavesMap = {{'A', 4}, {'C', 5}, {'E', 5}, {'G', 5},
                                                      {'B', 5}, {'D', 6}, {'F', 6}};

const std::map<char, int> c_B_closeStackOctavesMap = {{'B', 4}, {'D', 5}, {'F', 5}, {'A', 5},
                                                      {'C', 6}, {'E', 6}, {'G', 6}};

const std::map<int, RhythmFigure> c_TimeSignatureLowerValueMap = {
    {32, RhythmFigure::N32ND},  {16, RhythmFigure::N16TH}, {8, RhythmFigure::EIGHTH},
    {4, RhythmFigure::QUARTER}, {2, RhythmFigure::HALF},   {1, RhythmFigure::WHOLE}};

namespace MUSIC_XML {

namespace PART {
const std::string ALL = "all";
const int FIRST = 0;
}  // namespace PART

namespace OCTAVE {
const int ALL = -1;
const std::string ALL_STR = "all";
}  // namespace OCTAVE

namespace STEP {
const std::string ALL = "all";
}

namespace PITCH {
const std::string ALL = "all";
const std::string REST = "rest";
}  // namespace PITCH

namespace DURATION {
const float ALL = -1.0f;
}

namespace MEASURE {
const int START = 0;
const std::string END = "all";
}  // namespace MEASURE

namespace VOICE {
const std::string ALL = "all";
}

namespace STAFF {
const std::string ALL = "all";
}

namespace NOTE_TYPE {
const std::string ALL = "all";
const std::string MAXIMA = "maxima";
const std::string LONG = "long";
const std::string BREVE = "breve";
const std::string WHOLE = "whole";
const std::string HALF = "half";
const std::string QUARTER = "quarter";
const std::string EIGHTH = "eighth";
const std::string N16TH = "16th";
const std::string N32ND = "32nd";
const std::string N64TH = "64th";
const std::string N128TH = "128th";
const std::string N256TH = "256th";
const std::string N512TH = "512th";
const std::string N1024TH = "1024th";
}  // namespace NOTE_TYPE

namespace ACCIDENT {
const std::string SHARP = "#";
const std::string FLAT = "b";
const std::string NONE = "";
const std::string DOUBLE_SHARP = "x";
const std::string DOUBLE_FLAT = "bb";
}  // namespace ACCIDENT

namespace PATTERN_TYPE {
const std::string MELODY = "MELODY";
const std::string CHORD = "CHORD";
}  // namespace PATTERN_TYPE

namespace MIDI {
namespace NUMBER {
const int MIDI_REST = -1;
const int MIDI_000 = 0;
const int MIDI_001 = 1;
const int MIDI_002 = 2;
const int MIDI_003 = 3;
const int MIDI_004 = 4;
const int MIDI_005 = 5;
const int MIDI_006 = 6;
const int MIDI_007 = 7;
const int MIDI_008 = 8;
const int MIDI_009 = 9;
const int MIDI_010 = 10;
const int MIDI_011 = 11;
const int MIDI_012 = 12;
const int MIDI_013 = 13;
const int MIDI_014 = 14;
const int MIDI_015 = 15;
const int MIDI_016 = 16;
const int MIDI_017 = 17;
const int MIDI_018 = 18;
const int MIDI_019 = 19;
const int MIDI_020 = 20;
const int MIDI_021 = 21;
const int MIDI_022 = 22;
const int MIDI_023 = 23;
const int MIDI_024 = 24;
const int MIDI_025 = 25;
const int MIDI_026 = 26;
const int MIDI_027 = 27;
const int MIDI_028 = 28;
const int MIDI_029 = 29;
const int MIDI_030 = 30;
const int MIDI_031 = 31;
const int MIDI_032 = 32;
const int MIDI_033 = 33;
const int MIDI_034 = 34;
const int MIDI_035 = 35;
const int MIDI_036 = 36;
const int MIDI_037 = 37;
const int MIDI_038 = 38;
const int MIDI_039 = 39;
const int MIDI_040 = 40;
const int MIDI_041 = 41;
const int MIDI_042 = 42;
const int MIDI_043 = 43;
const int MIDI_044 = 44;
const int MIDI_045 = 45;
const int MIDI_046 = 46;
const int MIDI_047 = 47;
const int MIDI_048 = 48;
const int MIDI_049 = 49;
const int MIDI_050 = 50;
const int MIDI_051 = 51;
const int MIDI_052 = 52;
const int MIDI_053 = 53;
const int MIDI_054 = 54;
const int MIDI_055 = 55;
const int MIDI_056 = 56;
const int MIDI_057 = 57;
const int MIDI_058 = 58;
const int MIDI_059 = 59;
const int MIDI_060 = 60;
const int MIDI_061 = 61;
const int MIDI_062 = 62;
const int MIDI_063 = 63;
const int MIDI_064 = 64;
const int MIDI_065 = 65;
const int MIDI_066 = 66;
const int MIDI_067 = 67;
const int MIDI_068 = 68;
const int MIDI_069 = 69;
const int MIDI_070 = 70;
const int MIDI_071 = 71;
const int MIDI_072 = 72;
const int MIDI_073 = 73;
const int MIDI_074 = 74;
const int MIDI_075 = 75;
const int MIDI_076 = 76;
const int MIDI_077 = 77;
const int MIDI_078 = 78;
const int MIDI_079 = 79;
const int MIDI_080 = 80;
const int MIDI_081 = 81;
const int MIDI_082 = 82;
const int MIDI_083 = 83;
const int MIDI_084 = 84;
const int MIDI_085 = 85;
const int MIDI_086 = 86;
const int MIDI_087 = 87;
const int MIDI_088 = 88;
const int MIDI_089 = 89;
const int MIDI_090 = 90;
const int MIDI_091 = 91;
const int MIDI_092 = 92;
const int MIDI_093 = 93;
const int MIDI_094 = 94;
const int MIDI_095 = 95;
const int MIDI_096 = 96;
const int MIDI_097 = 97;
const int MIDI_098 = 98;
const int MIDI_099 = 99;
const int MIDI_100 = 100;
const int MIDI_101 = 101;
const int MIDI_102 = 102;
const int MIDI_103 = 103;
const int MIDI_104 = 104;
const int MIDI_105 = 105;
const int MIDI_106 = 106;
const int MIDI_107 = 107;
const int MIDI_108 = 108;
const int MIDI_109 = 109;
const int MIDI_110 = 110;
const int MIDI_111 = 111;
const int MIDI_112 = 112;
const int MIDI_113 = 113;
const int MIDI_114 = 114;
const int MIDI_115 = 115;
const int MIDI_116 = 116;
const int MIDI_117 = 117;
const int MIDI_118 = 118;
const int MIDI_119 = 119;
const int MIDI_120 = 120;
const int MIDI_121 = 121;
const int MIDI_122 = 122;
const int MIDI_123 = 123;
const int MIDI_124 = 124;
const int MIDI_125 = 125;
const int MIDI_126 = 126;
const int MIDI_127 = 127;
const int MIDI_128 = 128;
const int MIDI_129 = 129;
const int MIDI_130 = 130;
const int MIDI_131 = 131;
const int MIDI_132 = 132;
}  // namespace NUMBER

namespace PITCH {
const std::string C1_NEGATIVE = "C-1";
const std::string D1_FLAT_NEGATIVE = "Db-1";
const std::string C1_SHARP_NEGATIVE = "C#-1";
const std::string D1_NEGATIVE = "D-1";
const std::string E1_FLAT_NEGATIVE = "Eb-1";
const std::string D1_SHARP_NEGATIVE = "D#-1";
const std::string E1_NEGATIVE = "E-1";
const std::string F1_NEGATIVE = "F-1";
const std::string G1_FLAT_NEGATIVE = "Gb-1";
const std::string F1_SHARP_NEGATIVE = "F#-1";
const std::string G1_NEGATIVE = "G-1";
const std::string A1_FLAT_NEGATIVE = "Ab-1";
const std::string G1_SHARP_NEGATIVE = "G#-1";
const std::string A1_NEGATIVE = "A-1";
const std::string B1_FLAT_NEGATIVE = "Bb-1";
const std::string A1_SHARP_NEGATIVE = "A#-1";
const std::string B1_NEGATIVE = "B-1";
const std::string C0 = "C0";
const std::string D0_FLAT = "Db0";
const std::string C0_SHARP = "C#0";
const std::string D0 = "D0";
const std::string E0_FLAT = "Eb0";
const std::string D0_SHARP = "D#0";
const std::string E0 = "E0";
const std::string F0 = "F0";
const std::string G0_FLAT = "Gb0";
const std::string F0_SHARP = "F#0";
const std::string G0 = "G0";
const std::string A0_FLAT = "Ab0";
const std::string G0_SHARP = "G#0";
const std::string A0 = "A0";
const std::string B0_FLAT = "Bb0";
const std::string A0_SHARP = "A#0";
const std::string B0 = "B0";
const std::string C1 = "C1";
const std::string D1_FLAT = "Db1";
const std::string C1_SHARP = "C#1";
const std::string D1 = "D1";
const std::string E1_FLAT = "Eb1";
const std::string D1_SHARP = "D#1";
const std::string E1 = "E1";
const std::string F1 = "F1";
const std::string G1_FLAT = "Gb1";
const std::string F1_SHARP = "F#1";
const std::string G1 = "G1";
const std::string A1_FLAT = "Ab1";
const std::string G1_SHARP = "G#1";
const std::string A1 = "A1";
const std::string B1_FLAT = "Bb1";
const std::string A1_SHARP = "A#1";
const std::string B1 = "B1";
const std::string C2 = "C2";
const std::string D2_FLAT = "Db2";
const std::string C2_SHARP = "C#2";
const std::string D2 = "D2";
const std::string E2_FLAT = "Eb2";
const std::string D2_SHARP = "D#2";
const std::string E2 = "E2";
const std::string F2 = "F2";
const std::string G2_FLAT = "Gb2";
const std::string F2_SHARP = "F#2";
const std::string G2 = "G2";
const std::string A2_FLAT = "Ab2";
const std::string G2_SHARP = "G#2";
const std::string A2 = "A2";
const std::string B2_FLAT = "Bb2";
const std::string A2_SHARP = "A#2";
const std::string B2 = "B2";
const std::string C3 = "C3";
const std::string D3_FLAT = "Db3";
const std::string C3_SHARP = "C#3";
const std::string D3 = "D3";
const std::string E3_FLAT = "Eb3";
const std::string D3_SHARP = "D#3";
const std::string E3 = "E3";
const std::string F3 = "F3";
const std::string G3_FLAT = "Gb3";
const std::string F3_SHARP = "F#3";
const std::string G3 = "G3";
const std::string A3_FLAT = "Ab3";
const std::string G3_SHARP = "G#3";
const std::string A3 = "A3";
const std::string B3_FLAT = "Bb3";
const std::string A3_SHARP = "A#3";
const std::string B3 = "B3";
const std::string C4 = "C4";
const std::string D4_FLAT = "Db4";
const std::string C4_SHARP = "C#4";
const std::string D4 = "D4";
const std::string E4_FLAT = "Eb4";
const std::string D4_SHARP = "D#4";
const std::string E4 = "E4";
const std::string F4 = "F4";
const std::string G4_FLAT = "Gb4";
const std::string F4_SHARP = "F#4";
const std::string G4 = "G4";
const std::string A4_FLAT = "Ab4";
const std::string G4_SHARP = "G#4";
const std::string A4 = "A4";
const std::string B4_FLAT = "Bb4";
const std::string A4_SHARP = "A#4";
const std::string B4 = "B4";
const std::string C5 = "C5";
const std::string D5_FLAT = "Db5";
const std::string C5_SHARP = "C#5";
const std::string D5 = "D5";
const std::string E5_FLAT = "Eb5";
const std::string D5_SHARP = "D#5";
const std::string E5 = "E5";
const std::string F5 = "F5";
const std::string G5_FLAT = "Gb5";
const std::string F5_SHARP = "F#5";
const std::string G5 = "G5";
const std::string A5_FLAT = "Ab5";
const std::string G5_SHARP = "G#5";
const std::string A5 = "A5";
const std::string B5_FLAT = "Bb5";
const std::string A5_SHARP = "A#5";
const std::string B5 = "B5";
const std::string C6 = "C6";
const std::string D6_FLAT = "Db6";
const std::string C6_SHARP = "C#6";
const std::string D6 = "D6";
const std::string E6_FLAT = "Eb6";
const std::string D6_SHARP = "D#6";
const std::string E6 = "E6";
const std::string F6 = "F6";
const std::string G6_FLAT = "Gb6";
const std::string F6_SHARP = "F#6";
const std::string G6 = "G6";
const std::string A6_FLAT = "Ab6";
const std::string G6_SHARP = "G#6";
const std::string A6 = "A6";
const std::string B6_FLAT = "Bb6";
const std::string A6_SHARP = "A#6";
const std::string B6 = "B6";
const std::string C7 = "C7";
const std::string D7_FLAT = "Db7";
const std::string C7_SHARP = "C#7";
const std::string D7 = "D7";
const std::string E7_FLAT = "Eb7";
const std::string D7_SHARP = "D#7";
const std::string E7 = "E7";
const std::string F7 = "F7";
const std::string G7_FLAT = "Gb7";
const std::string F7_SHARP = "F#7";
const std::string G7 = "G7";
const std::string A7_FLAT = "Ab7";
const std::string G7_SHARP = "G#7";
const std::string A7 = "A7";
const std::string B7_FLAT = "Bb7";
const std::string A7_SHARP = "A#7";
const std::string B7 = "B7";
const std::string C8 = "C8";
const std::string D8_FLAT = "Db8";
const std::string C8_SHARP = "C#8";
const std::string D8 = "D8";
const std::string E8_FLAT = "Eb8";
const std::string D8_SHARP = "D#8";
const std::string E8 = "E8";
const std::string F8 = "F8";
const std::string G8_FLAT = "Gb8";
const std::string F8_SHARP = "F#8";
const std::string G8 = "G8";
const std::string A8_FLAT = "Ab8";
const std::string G8_SHARP = "G#8";
const std::string A8 = "A8";
const std::string B8_FLAT = "Bb8";
const std::string A8_SHARP = "A#8";
const std::string B8 = "B8";
const std::string C9 = "C9";
const std::string D9_FLAT = "Db9";
const std::string C9_SHARP = "C#9";
const std::string D9 = "D9";
const std::string E9_FLAT = "Eb9";
const std::string D9_SHARP = "D#9";
const std::string E9 = "E9";
const std::string F9 = "F9";
const std::string G9_FLAT = "Gb9";
const std::string F9_SHARP = "F#9";
const std::string G9 = "G9";
const std::string A9_FLAT = "Ab9";
const std::string G9_SHARP = "G#9";
const std::string A9 = "A9";
const std::string B9_FLAT = "Bb9";
const std::string A9_SHARP = "A#9";
const std::string B9 = "B9";
const std::string C10 = "C10";
}  // namespace PITCH

namespace INTERVAL {
const size_t PERFECT_UNISON = 0;
const size_t MINOR_SECOND = 1;
const size_t MAJOR_SECOND = 2;
const size_t MINOR_THIRD = 3;
const size_t MAJOR_THIRD = 4;
const size_t PERFECT_FOURTH = 5;
const size_t DIMINISHED_FIFTH = 6;
const size_t PERFECT_FIFTH = 7;
const size_t MINOR_SIXTH = 8;
const size_t MAJOR_SIXTH = 9;
const size_t MINOR_SEVENTH = 10;
const size_t MAJOR_SEVENTH = 11;
const size_t PERFECT_OCTAVE = 12;
const size_t MINOR_NINTH = 13;
const size_t MAJOR_NINTH = 14;
const size_t MINOR_TENTH = 15;
const size_t MAJOR_TENTH = 16;
const size_t PERFECT_ELEVENTH = 17;
const size_t SHARP_ELEVEN = 18;
const size_t PERFECT_TWELFTH = 19;
const size_t MINOR_THIRTEENTH = 20;
const size_t MAJOR_THIRTEENTH = 21;
}  // namespace INTERVAL
}  // namespace MIDI
}  // namespace MUSIC_XML

#endif  // CONSTANTS_H
