#include "maiacore/helper.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>  // std::ofstream
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "cherno/instrumentor.h"
#include "maiacore/interval.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

std::vector<std::string> Helper::splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string Helper::formatFloat(float floatValue, int digits) {
    std::ostringstream ss;
    ss.precision(digits);
    ss << floatValue;
    return ss.str();
}

int Helper::semitonesBetweenPitches(const std::string& pitch_A, const std::string& pitch_B) {
    // Error checking
    if (pitch_A.empty() || pitch_B.empty()) {
        return 0;
    }

    // Rest case
    if (pitch_A == MUSIC_XML::PITCH::REST || pitch_B == MUSIC_XML::PITCH::REST) {
        return 0;
    }

    const int midiNumber_A = static_cast<int>(pitch2midiNote(pitch_A));
    const int midiNumber_B = static_cast<int>(pitch2midiNote(pitch_B));

    return midiNumber_B - midiNumber_A;
}

const std::string Helper::midiNote2pitch(const int midiNote, const std::string& accType) {
    // Rest case
    if (midiNote < 0) {
        return "rest";
    }

    // Validate accType value
    if (!accType.empty() && (accType != MUSIC_XML::ACCIDENT::SHARP) &&
        (accType != MUSIC_XML::ACCIDENT::FLAT) && (accType != MUSIC_XML::ACCIDENT::NONE) &&
        (accType != MUSIC_XML::ACCIDENT::DOUBLE_SHARP) &&
        (accType != MUSIC_XML::ACCIDENT::DOUBLE_FLAT)) {
        LOG_ERROR("Unknown accident type: " + accType);
        return {};
    }

    // ===== VALIDADE ACCTYPE FOR THIS SPECIFIC MIDI NOTE ===== //

    // Get this midi note inside the first octave range
    const int firstOctaveMidiNote = midiNote % 12;

    const bool canBeDoubleFlat =
        (firstOctaveMidiNote == 0 || firstOctaveMidiNote == 2 || firstOctaveMidiNote == 3 ||
         firstOctaveMidiNote == 5 || firstOctaveMidiNote == 7 || firstOctaveMidiNote == 9 ||
         firstOctaveMidiNote == 10)
            ? true
            : false;

    const bool canBeFlat =
        (firstOctaveMidiNote == 1 || firstOctaveMidiNote == 3 || firstOctaveMidiNote == 4 ||
         firstOctaveMidiNote == 6 || firstOctaveMidiNote == 8 || firstOctaveMidiNote == 10 ||
         firstOctaveMidiNote == 11)
            ? true
            : false;

    const bool canBeSharp =
        (firstOctaveMidiNote == 0 || firstOctaveMidiNote == 1 || firstOctaveMidiNote == 3 ||
         firstOctaveMidiNote == 5 || firstOctaveMidiNote == 6 || firstOctaveMidiNote == 8 ||
         firstOctaveMidiNote == 10)
            ? true
            : false;

    const bool canBeDoubleSharp =
        (firstOctaveMidiNote == 1 || firstOctaveMidiNote == 2 || firstOctaveMidiNote == 4 ||
         firstOctaveMidiNote == 6 || firstOctaveMidiNote == 7 || firstOctaveMidiNote == 9 ||
         firstOctaveMidiNote == 11)
            ? true
            : false;

    if ((accType == MUSIC_XML::ACCIDENT::DOUBLE_FLAT && !canBeDoubleFlat) ||
        (accType == MUSIC_XML::ACCIDENT::FLAT && !canBeFlat) ||
        (accType == MUSIC_XML::ACCIDENT::SHARP && !canBeSharp) ||
        (accType == MUSIC_XML::ACCIDENT::DOUBLE_SHARP && !canBeDoubleSharp)) {
        LOG_ERROR("The MIDI Note '" + std::to_string(midiNote) + "' cannot be wrote using '" +
                  accType + "' accident type");
    }

    // ===== GET THE PITCHCLASS ===== //
    std::string firstOctavePitchClass;

    if (accType == MUSIC_XML::ACCIDENT::SHARP) {
        firstOctavePitchClass =
            (firstOctaveMidiNote != 0) ? c_chromaticSharpScale[firstOctaveMidiNote] : "B#";
        firstOctavePitchClass = (firstOctaveMidiNote == 5) ? "E#" : firstOctavePitchClass;
    } else if (accType == MUSIC_XML::ACCIDENT::FLAT) {
        firstOctavePitchClass = c_chromaticFlatScale[firstOctaveMidiNote];
        firstOctavePitchClass = (firstOctaveMidiNote == 11) ? "Cb" : firstOctavePitchClass;
    } else if (accType == MUSIC_XML::ACCIDENT::DOUBLE_SHARP) {
        firstOctavePitchClass = c_chromaticDoubleSharpScale[firstOctaveMidiNote];
    } else if (accType == MUSIC_XML::ACCIDENT::DOUBLE_FLAT) {
        firstOctavePitchClass =
            (firstOctaveMidiNote != 0) ? c_chromaticDoubleFlatScale[firstOctaveMidiNote] : "Dbb";
        firstOctavePitchClass = (firstOctaveMidiNote == 10) ? "Cbb" : firstOctavePitchClass;
    } else {  // accType is empty
        firstOctavePitchClass =
            (firstOctaveMidiNote == 0) ? "C" : c_chromaticSharpScale[firstOctaveMidiNote];
    }

    // ===== COMPUTE THE OCTAVE ===== //
    int octave = floor(midiNote / 12) - 1;

    // Check if there is a octave transition
    if ((firstOctaveMidiNote == 0 && accType == MUSIC_XML::ACCIDENT::SHARP) ||
        (firstOctaveMidiNote == 1 && accType == MUSIC_XML::ACCIDENT::DOUBLE_SHARP)) {
        octave--;
    }

    if ((firstOctaveMidiNote == 10 && accType == MUSIC_XML::ACCIDENT::DOUBLE_FLAT) ||
        (firstOctaveMidiNote == 11 && accType == MUSIC_XML::ACCIDENT::FLAT)) {
        octave++;
    }

    return firstOctavePitchClass + std::to_string(octave);
}

const std::vector<std::string> Helper::midiNote2pitches(const int midiNote) {
    std::vector<std::string> pitches;

    const std::vector<std::string> accType{
        MUSIC_XML::ACCIDENT::DOUBLE_FLAT, MUSIC_XML::ACCIDENT::FLAT, MUSIC_XML::ACCIDENT::NONE,
        MUSIC_XML::ACCIDENT::SHARP, MUSIC_XML::ACCIDENT::DOUBLE_SHARP};

    for (const auto& acc : accType) {
        try {
            const std::string pitch = midiNote2pitch(midiNote, acc);
            pitches.push_back(pitch);
        } catch (const std::runtime_error& error) {
            // Nothing to do. Just ignore
            ignore(error);
        }
    }

    std::sort(pitches.begin(), pitches.end());
    pitches.erase(std::unique(pitches.begin(), pitches.end()), pitches.end());

    return pitches;
}

std::vector<Interval> Helper::notes2Intervals(const std::vector<Note>& notes,
                                              const bool firstNoteAsReference) {
    const int notesSize = notes.size();
    if (notesSize <= 1) {
        LOG_ERROR("You should pass two or more notes to create an interval vector");
    }

    const int numIntervals = notesSize - 1;
    std::vector<Interval> intervals(numIntervals);

    if (firstNoteAsReference) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(notes[0], notes[i + 1]);
        }

        return intervals;
    }

    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(notes[i], notes[i + 1]);
    }

    return intervals;
}

std::vector<Interval> Helper::notes2Intervals(const std::vector<std::string>& pitches,
    const bool firstNoteAsReference) {
    const int notesSize = pitches.size();
    std::vector<Note> notes;
    notes.reserve(notesSize);

    for (const auto& pitch : pitches) {
        notes.emplace_back(Note(pitch));
    }

    return Helper::notes2Intervals(notes, firstNoteAsReference);
}

int Helper::pitch2midiNote(const std::string& pitch) {
    switch (hash(pitch.c_str())) {
        case hash("C0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_012;
        case hash("Dbb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_012;

        case hash("Db0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_013;
        case hash("C#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_013;

        case hash("D0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_014;
        case hash("Cx0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_014;
        case hash("Ebb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_014;

        case hash("Eb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_015;
        case hash("D#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_015;
        case hash("Fbb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_015;

        case hash("E0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_016;
        case hash("Dx0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_016;
        case hash("Fb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_016;

        case hash("F0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_017;
        case hash("E#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_017;
        case hash("Gbb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_017;

        case hash("Gb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_018;
        case hash("F#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_018;
        case hash("Ex0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_018;

        case hash("G0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_019;
        case hash("Abb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_019;
        case hash("Fx0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_019;

        case hash("Ab0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_020;
        case hash("G#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_020;

        case hash("A0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_021;
        case hash("Bbb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_021;
        case hash("Gx0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_021;

        case hash("Bb0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_022;
        case hash("A#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_022;
        case hash("Cbb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_022;

        case hash("B0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_023;
        case hash("Cb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_023;
        case hash("Ax0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_023;

            // ==================== OCTAVE 1 ================ //

        case hash("C1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_024;
        case hash("Dbb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_024;
        case hash("B#0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_024;

        case hash("Db1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_025;
        case hash("C#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_025;
        case hash("Bx0"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_025;

        case hash("D1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_026;
        case hash("Cx1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_026;
        case hash("Ebb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_026;

        case hash("Eb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_027;
        case hash("D#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_027;
        case hash("Fbb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_027;

        case hash("E1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_028;
        case hash("Dx1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_028;
        case hash("Fb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_028;

        case hash("F1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_029;
        case hash("E#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_029;
        case hash("Gbb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_029;

        case hash("Gb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_030;
        case hash("F#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_030;
        case hash("Ex1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_030;

        case hash("G1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_031;
        case hash("Abb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_031;
        case hash("Fx1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_031;

        case hash("Ab1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_032;
        case hash("G#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_032;

        case hash("A1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_033;
        case hash("Bbb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_033;
        case hash("Gx1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_033;

        case hash("Bb1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_034;
        case hash("A#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_034;
        case hash("Cbb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_034;

        case hash("B1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_035;
        case hash("Cb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_035;
        case hash("Ax1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_035;

            // ==================== OCTAVE 2 ================ //

        case hash("C2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_036;
        case hash("Dbb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_036;
        case hash("B#1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_036;

        case hash("Db2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_037;
        case hash("C#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_037;
        case hash("Bx1"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_037;

        case hash("D2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_038;
        case hash("Cx2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_038;
        case hash("Ebb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_038;

        case hash("Eb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_039;
        case hash("D#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_039;
        case hash("Fbb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_039;

        case hash("E2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_040;
        case hash("Dx2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_040;
        case hash("Fb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_040;

        case hash("F2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_041;
        case hash("E#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_041;
        case hash("Gbb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_041;

        case hash("Gb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_042;
        case hash("F#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_042;
        case hash("Ex2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_042;

        case hash("G2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_043;
        case hash("Abb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_043;
        case hash("Fx2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_043;

        case hash("Ab2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_044;
        case hash("G#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_044;

        case hash("A2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_045;
        case hash("Bbb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_045;
        case hash("Gx2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_045;

        case hash("Bb2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_046;
        case hash("A#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_046;
        case hash("Cbb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_046;

        case hash("B2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_047;
        case hash("Cb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_047;
        case hash("Ax2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_047;

            // ==================== OCTAVE 3 ================ //

        case hash("C3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_048;
        case hash("Dbb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_048;
        case hash("B#2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_048;

        case hash("Db3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_049;
        case hash("C#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_049;
        case hash("Bx2"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_049;

        case hash("D3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_050;
        case hash("Cx3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_050;
        case hash("Ebb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_050;

        case hash("Eb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_051;
        case hash("D#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_051;
        case hash("Fbb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_051;

        case hash("E3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_052;
        case hash("Dx3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_052;
        case hash("Fb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_052;

        case hash("F3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_053;
        case hash("E#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_053;
        case hash("Gbb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_053;

        case hash("Gb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_054;
        case hash("F#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_054;
        case hash("Ex3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_054;

        case hash("G3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_055;
        case hash("Abb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_055;
        case hash("Fx3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_055;

        case hash("Ab3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_056;
        case hash("G#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_056;

        case hash("A3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_057;
        case hash("Bbb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_057;
        case hash("Gx3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_057;

        case hash("Bb3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_058;
        case hash("A#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_058;
        case hash("Cbb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_058;

        case hash("B3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_059;
        case hash("Cb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_059;
        case hash("Ax3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_059;

            // ==================== OCTAVE 4 ================ //

        case hash("C4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_060;
        case hash("Dbb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_060;
        case hash("B#3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_060;

        case hash("Db4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_061;
        case hash("C#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_061;
        case hash("Bx3"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_061;

        case hash("D4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_062;
        case hash("Cx4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_062;
        case hash("Ebb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_062;

        case hash("Eb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_063;
        case hash("D#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_063;
        case hash("Fbb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_063;

        case hash("E4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_064;
        case hash("Dx4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_064;
        case hash("Fb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_064;

        case hash("F4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_065;
        case hash("E#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_065;
        case hash("Gbb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_065;

        case hash("Gb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_066;
        case hash("F#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_066;
        case hash("Ex4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_066;

        case hash("G4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_067;
        case hash("Abb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_067;
        case hash("Fx4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_067;

        case hash("Ab4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_068;
        case hash("G#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_068;

        case hash("A4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_069;
        case hash("Bbb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_069;
        case hash("Gx4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_069;

        case hash("Bb4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_070;
        case hash("A#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_070;
        case hash("Cbb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_070;

        case hash("B4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_071;
        case hash("Cb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_071;
        case hash("Ax4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_071;

            // ==================== OCTAVE 5 ================ //

        case hash("C5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_072;
        case hash("Dbb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_072;
        case hash("B#4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_072;

        case hash("Db5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_073;
        case hash("C#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_073;
        case hash("Bx4"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_073;

        case hash("D5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_074;
        case hash("Cx5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_074;
        case hash("Ebb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_074;

        case hash("Eb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_075;
        case hash("D#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_075;
        case hash("Fbb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_075;

        case hash("E5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_076;
        case hash("Dx5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_076;
        case hash("Fb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_076;

        case hash("F5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_077;
        case hash("E#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_077;
        case hash("Gbb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_077;

        case hash("Gb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_078;
        case hash("F#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_078;
        case hash("Ex5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_078;

        case hash("G5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_079;
        case hash("Abb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_079;
        case hash("Fx5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_079;

        case hash("Ab5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_080;
        case hash("G#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_080;

        case hash("A5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_081;
        case hash("Bbb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_081;
        case hash("Gx5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_081;

        case hash("Bb5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_082;
        case hash("A#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_082;
        case hash("Cbb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_082;

        case hash("B5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_083;
        case hash("Cb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_083;
        case hash("Ax5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_083;

            // ==================== OCTAVE 6 ================ //

        case hash("C6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_084;
        case hash("Dbb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_084;
        case hash("B#5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_084;

        case hash("Db6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_085;
        case hash("C#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_085;
        case hash("Bx5"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_085;

        case hash("D6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_086;
        case hash("Cx6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_086;
        case hash("Ebb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_086;

        case hash("Eb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_087;
        case hash("D#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_087;
        case hash("Fbb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_087;

        case hash("E6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_088;
        case hash("Dx6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_088;
        case hash("Fb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_088;

        case hash("F6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_089;
        case hash("E#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_089;
        case hash("Gbb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_089;

        case hash("Gb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_090;
        case hash("F#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_090;
        case hash("Ex6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_090;

        case hash("G6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_091;
        case hash("Abb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_091;
        case hash("Fx6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_091;

        case hash("Ab6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_092;
        case hash("G#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_092;

        case hash("A6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_093;
        case hash("Bbb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_093;
        case hash("Gx6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_093;

        case hash("Bb6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_094;
        case hash("A#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_094;
        case hash("Cbb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_094;

        case hash("B6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_095;
        case hash("Cb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_095;
        case hash("Ax6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_095;

            // ==================== OCTAVE 7 ================ //

        case hash("C7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_096;
        case hash("Dbb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_096;
        case hash("B#6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_096;

        case hash("Db7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_097;
        case hash("C#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_097;
        case hash("Bx6"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_097;

        case hash("D7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_098;
        case hash("Cx7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_098;
        case hash("Ebb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_098;

        case hash("Eb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_099;
        case hash("D#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_099;
        case hash("Fbb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_099;

        case hash("E7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_100;
        case hash("Dx7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_100;
        case hash("Fb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_100;

        case hash("F7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_101;
        case hash("E#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_101;
        case hash("Gbb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_101;

        case hash("Gb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_102;
        case hash("F#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_102;
        case hash("Ex7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_102;

        case hash("G7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_103;
        case hash("Abb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_103;
        case hash("Fx7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_103;

        case hash("Ab7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_104;
        case hash("G#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_104;

        case hash("A7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_105;
        case hash("Bbb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_105;
        case hash("Gx7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_105;

        case hash("Bb7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_106;
        case hash("A#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_106;
        case hash("Cbb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_106;

        case hash("B7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_107;
        case hash("Cb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_107;
        case hash("Ax7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_107;

            // ==================== OCTAVE 8 ================ //

        case hash("C8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_108;
        case hash("Dbb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_108;
        case hash("B#7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_108;

        case hash("Db8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_109;
        case hash("C#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_109;
        case hash("Bx7"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_109;

        case hash("D8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_110;
        case hash("Cx8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_110;
        case hash("Ebb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_110;

        case hash("Eb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_111;
        case hash("D#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_111;
        case hash("Fbb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_111;

        case hash("E8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_112;
        case hash("Dx8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_112;
        case hash("Fb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_112;

        case hash("F8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_113;
        case hash("E#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_113;
        case hash("Gbb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_113;

        case hash("Gb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_114;
        case hash("F#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_114;
        case hash("Ex8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_114;

        case hash("G8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_115;
        case hash("Abb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_115;
        case hash("Fx8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_115;

        case hash("Ab8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_116;
        case hash("G#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_116;

        case hash("A8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_117;
        case hash("Bbb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_117;
        case hash("Gx8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_117;

        case hash("Bb8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_118;
        case hash("A#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_118;
        case hash("Cbb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_118;

        case hash("B8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_119;
        case hash("Cb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_119;
        case hash("Ax8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_119;

            // ==================== OCTAVE 9 ================ //

        case hash("C9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_120;
        case hash("Dbb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_120;
        case hash("B#8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_120;

        case hash("Db9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_121;
        case hash("C#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_121;
        case hash("Bx8"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_121;

        case hash("D9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_122;
        case hash("Cx9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_122;
        case hash("Ebb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_122;

        case hash("Eb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_123;
        case hash("D#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_123;
        case hash("Fbb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_123;

        case hash("E9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_124;
        case hash("Dx9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_124;
        case hash("Fb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_124;

        case hash("F9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_125;
        case hash("E#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_125;
        case hash("Gbb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_125;

        case hash("Gb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_126;
        case hash("F#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_126;
        case hash("Ex9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_126;

        case hash("G9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_127;
        case hash("Abb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_127;
        case hash("Fx9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_127;

        case hash("Ab9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_128;
        case hash("G#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_128;

        case hash("A9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_129;
        case hash("Bbb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_129;
        case hash("Gx9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_129;

        case hash("Bb9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_130;
        case hash("A#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_130;
        case hash("Cbb10"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_130;

        case hash("B9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_131;
        case hash("Cb10"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_131;
        case hash("Ax9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_131;

            // ==================== OCTAVE 10 ================ //

        case hash("C10"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_132;
        case hash("Db10"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_132;
        case hash("B#9"):
            return MUSIC_XML::MIDI::NUMBER::MIDI_132;

        default:
            LOG_ERROR("Unknown pitch: " + pitch);
            break;
    }

    // If pitch no found:
    return -1;
}

std::pair<int, int> Helper::freq2midiNote(const float freq, std::function<int(float)> modelo) {
    if (freq <= 0.0f) {
        return std::make_pair(MUSIC_XML::MIDI::NUMBER::MIDI_REST, 0);
    }

    int closestMIDINumber = 0;
    if (modelo == nullptr) {
        closestMIDINumber = std::round(12.0f * log2f(freq / 440.0f) + 69.0f);
    } else {
        closestMIDINumber = modelo(freq);
    }

    const float closestNoteFrequency = midiNote2freq(closestMIDINumber);

    const float freqRatio = freq / closestNoteFrequency;

    const int centsOffset = std::round(1200.0f * log2f(freqRatio));

    return std::make_pair(closestMIDINumber, centsOffset);
}

float Helper::midiNote2freq(const int midiNote, const float freqA4) {
    if (midiNote < 0) {
        return 0.0f;
    }  // rest

    return powf(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f) * freqA4;
}

int Helper::midiNote2octave(const int midiNote) {
    if (midiNote < 0) {
        return -2;
    }  // rest octave

    const int octave = (midiNote / 12) - 1;

    return octave;
}

float Helper::alterSymbol2Value(const std::string& alterSymbol) {
    switch (hash(alterSymbol.c_str())) {
        case hash("bb"):
            return -2.0f;
        case hash("3b"):
            return -1.5f;
        case hash("b"):
            return -1.0f;
        case hash("1b"):
            return -0.5f;
        case hash(""):
            return 0.0f;
        case hash("1x"):
            return 0.5f;
        case hash("#"):
            return 1.0f;
        case hash("3x"):
            return 1.5f;
        case hash("x"):
            return 2.0f;

        default:
            LOG_ERROR("Unknown accident symbol: " + alterSymbol);
            break;
    }

    return {};
}

const std::string Helper::alterValue2Name(const float alterValue) {
    std::ostringstream streamObj;

    // Set Fixed-Point Notation
    streamObj << std::fixed;
    streamObj << std::setprecision(1);

    streamObj << alterValue;

    const std::string alterStr = streamObj.str().c_str();

    switch (hash(alterStr.c_str())) {
        case hash("-2.0"):
            return "flat-flat";
        case hash("-1.5"):
            return "flat-down";
        case hash("-1.0"):
            return "flat";
        case hash("-0.5"):
            return "flat-up";
        case hash("0.0"):
            return "natural";
        case hash("0.5"):
            return "sharp-down";
        case hash("1.0"):
            return "sharp";
        case hash("1.5"):
            return "sharp-up";
        case hash("2.0"):
            return "double-sharp";

        default:
            LOG_ERROR("Unknown accidental alter value: " + alterStr);
            break;
    }

    return {};
}

const std::string Helper::alterValue2symbol(const float alterValue) {
    std::ostringstream streamObj;

    // Set Fixed-Point Notation
    streamObj << std::fixed;
    streamObj << std::setprecision(1);

    streamObj << alterValue;

    std::string value = streamObj.str();

    switch (hash(value.c_str())) {
        case hash("-2.0"):
            return "bb";
        case hash("-1.5"):
            return "3b";
        case hash("-1.0"):
            return "b";
        case hash("-0.5"):
            return "1b";
        case hash("0.0"):
            return "";
        case hash("0.5"):
            return "1x";
        case hash("1.0"):
            return "#";
        case hash("1.5"):
            return "3x";
        case hash("2.0"):
            return "x";

        default:
            LOG_ERROR("Unknown accidental alter value: " + value);
            break;
    }

    return {};
}

const std::string Helper::alterName2symbol(const std::string& alterName) {
    switch (hash(alterName.c_str())) {
        case hash("natural"):
            return "";
            break;
        case hash("sharp-down"):
            return "1x";
            break;
        case hash("sharp"):
            return "#";
            break;
        case hash("sharp-up"):
            return "3x";
            break;
        case hash("double-sharp"):
            return "x";
            break;
        case hash("flat-up"):
            return "1b";
            break;
        case hash("flat"):
            return "b";
            break;
        case hash("flat-down"):
            return "3b";
            break;
        case hash("flat-flat"):
            return "bb";
            break;

        default:
            LOG_ERROR("Unknown accidental name: " + alterName);
            break;
    }

    return {};
}

const nlohmann::json Helper::getPercentiles(const nlohmann::json& table,
                                            const std::vector<float>& desiredPercentiles) {
    nlohmann::json output;
    const size_t tableSize = table.size();
    const size_t desiredPercentilesSize = desiredPercentiles.size();

    // Check input values:
    for (size_t i = 0; i < desiredPercentilesSize; i++) {
        if (desiredPercentiles[i] > 1.0f) {
            LOG_ERROR("All desired percentiles MUST BE smaller than 1.0");
            return nlohmann::json();
        }
    }

    // For each input percentile value:
    for (size_t i = 0; i < desiredPercentilesSize; i++) {
        float sum = 0.0f;

        // For each table input value:
        for (size_t j = 0; j < tableSize; j++) {
            const float tableSimilarity = table[j]["averageSimilarity"].get<float>();

            if (tableSimilarity >= desiredPercentiles[i]) {
                sum++;
            }
        }

        // Compute valule:
        const float value = (sum * 100.0f) / (static_cast<float>(tableSize) - 1.0f);

        // Store percentile value:
        nlohmann::json tableLine;
        tableLine["percentile"] = desiredPercentiles[i];
        tableLine["value"] = value;

        output.push_back(tableLine);
    }

    return output;
}

int Helper::noteType2ticks(std::string noteType, const int divisionsPerQuarterNote) {
    int ticks = 0;

    // Set string to lower case
    for (auto& c : noteType) {
        c = tolower(c);
    }

    switch (hash(noteType.c_str())) {
        case hash("maxima-dot-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 5) + divisionsPerQuarterNote * pow(2, 4) +
                    divisionsPerQuarterNote * pow(2, 3);
            break;

        case hash("maxima-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 5) + divisionsPerQuarterNote * pow(2, 4);
            break;

        case hash("maxima"):
            ticks = divisionsPerQuarterNote * pow(2, 5);
            break;

        case hash("long-dot-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 4) + divisionsPerQuarterNote * pow(2, 3) +
                    divisionsPerQuarterNote * pow(2, 2);
            break;

        case hash("long-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 4) + divisionsPerQuarterNote * pow(2, 3);
            break;

        case hash("long"):
            ticks = divisionsPerQuarterNote * pow(2, 4);
            break;

        case hash("breve-dot-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 3) + divisionsPerQuarterNote * pow(2, 2) +
                    divisionsPerQuarterNote * pow(2, 1);
            break;

        case hash("breve-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 3) + divisionsPerQuarterNote * pow(2, 2);
            break;

        case hash("breve"):
            ticks = divisionsPerQuarterNote * pow(2, 3);
            break;

        case hash("whole-dot-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 2) + divisionsPerQuarterNote * pow(2, 1) +
                    divisionsPerQuarterNote * pow(2, 0);
            break;

        case hash("whole-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 2) + divisionsPerQuarterNote * pow(2, 1);
            break;

        case hash("whole"):
            ticks = divisionsPerQuarterNote * pow(2, 2);
            break;

        case hash("half-dot-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 1) + divisionsPerQuarterNote * pow(2, 0) +
                    divisionsPerQuarterNote * pow(2, -1);
            break;

        case hash("half-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 1) + divisionsPerQuarterNote * pow(2, 0);
            break;

        case hash("half"):
            ticks = divisionsPerQuarterNote * pow(2, 1);
            break;

        case hash("quarter-dot-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 0) + divisionsPerQuarterNote * pow(2, -1) +
                    divisionsPerQuarterNote * pow(2, -2);
            break;

        case hash("quarter-dot"):
            ticks = divisionsPerQuarterNote * pow(2, 0) + divisionsPerQuarterNote * pow(2, -1);
            break;

        case hash("quarter"):
            ticks = divisionsPerQuarterNote * pow(2, 0);
            break;

        case hash("eighth-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -1) +
                               divisionsPerQuarterNote * pow(2, -2) +
                               divisionsPerQuarterNote * pow(2, -3));
            break;

        case hash("eighth-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -1) +
                               divisionsPerQuarterNote * pow(2, -2));
            break;

        case hash("eighth"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -1));
            break;

        case hash("16th-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -2) +
                               divisionsPerQuarterNote * pow(2, -3) +
                               divisionsPerQuarterNote * pow(2, -4));
            break;

        case hash("16th-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -2) +
                               divisionsPerQuarterNote * pow(2, -3));
            break;

        case hash("16th"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -2));
            break;

        case hash("32nd-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -3) +
                               divisionsPerQuarterNote * pow(2, -4) +
                               divisionsPerQuarterNote * pow(2, -5));
            break;

        case hash("32nd-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -3) +
                               divisionsPerQuarterNote * pow(2, -4));
            break;

        case hash("32nd"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -3));
            break;

        case hash("64th-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -4) +
                               divisionsPerQuarterNote * pow(2, -5) +
                               divisionsPerQuarterNote * pow(2, -6));
            break;

        case hash("64th-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -4) +
                               divisionsPerQuarterNote * pow(2, -5));
            break;

        case hash("64th"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -4));
            break;

        case hash("128th-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -5) +
                               divisionsPerQuarterNote * pow(2, -6) +
                               divisionsPerQuarterNote * pow(2, -7));
            break;

        case hash("128th-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -5) +
                               divisionsPerQuarterNote * pow(2, -6));
            break;

        case hash("128th"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -5));
            break;

        case hash("256th-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -6) +
                               divisionsPerQuarterNote * pow(2, -7) +
                               divisionsPerQuarterNote * pow(2, -8));
            break;

        case hash("256th-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -6) +
                               divisionsPerQuarterNote * pow(2, -7));
            break;

        case hash("256th"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -6));
            break;

        case hash("512th-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -7) +
                               divisionsPerQuarterNote * pow(2, -7) +
                               divisionsPerQuarterNote * pow(2, -8));
            break;

        case hash("512th-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -7) +
                               divisionsPerQuarterNote * pow(2, -7));
            break;

        case hash("512th"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -7));
            break;

        case hash("1024th-dot-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -8) +
                               divisionsPerQuarterNote * pow(2, -8) +
                               divisionsPerQuarterNote * pow(2, -9));
            break;

        case hash("1024th-dot"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -8) +
                               divisionsPerQuarterNote * pow(2, -8));
            break;

        case hash("1024th"):
            ticks = std::round(divisionsPerQuarterNote * pow(2, -8));
            break;

        default:
            LOG_ERROR("Unknown note type called: " + noteType);
            break;
    }

    return ticks;
}

std::pair<std::string, int> Helper::ticks2noteType(int durationTicks, int divisionsPerQuarterNote,
                                                   int actualNotes, int normalNotes) {
    std::pair<std::string, int> result;
    int baseDuration = durationTicks;
    int dotCount = 0;

    // Adjust the durationTicks for tuplets
    if (actualNotes != normalNotes) {
        durationTicks = (durationTicks * normalNotes) / actualNotes;
    }

    // Determine base note type and dot count
    if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 5)) {
        result.first = "maxima";
        baseDuration = divisionsPerQuarterNote * std::pow(2, 5);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 4)) {
        result.first = "long";
        baseDuration = divisionsPerQuarterNote * std::pow(2, 4);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 3)) {
        result.first = "breve";
        baseDuration = divisionsPerQuarterNote * std::pow(2, 3);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 2)) {
        result.first = "whole";
        baseDuration = divisionsPerQuarterNote * std::pow(2, 2);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 1)) {
        result.first = "half";
        baseDuration = divisionsPerQuarterNote * std::pow(2, 1);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 0)) {
        result.first = "quarter";
        baseDuration = divisionsPerQuarterNote * std::pow(2, 0);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -1)) {
        result.first = "eighth";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -1);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -2)) {
        result.first = "16th";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -2);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -3)) {
        result.first = "32nd";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -3);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -4)) {
        result.first = "64th";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -4);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -5)) {
        result.first = "128th";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -5);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -6)) {
        result.first = "256th";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -6);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -7)) {
        result.first = "512th";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -7);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -8)) {
        result.first = "1024th";
        baseDuration = divisionsPerQuarterNote * std::pow(2, -8);
    } else {
        std::string errMsg = "Unable to convert durationTick to noteType\n";
        errMsg += "durationTicks: " + std::to_string(durationTicks) + "\n";
        errMsg += "divisionsPerQuarterNote: " + std::to_string(divisionsPerQuarterNote) + "\n";
        errMsg += "actualNotes: " + std::to_string(actualNotes) + "\n";
        errMsg += "normalNotes: " + std::to_string(normalNotes) + "\n";
        LOG_ERROR(errMsg);
    }

    // Calculate dot count
    while (durationTicks > baseDuration) {
        baseDuration += baseDuration / 2;
        dotCount++;
    }

    result.second = dotCount;
    return result;
}

std::pair<RhythmFigure, int> Helper::ticks2rhythmFigure(int durationTicks,
                                                        int divisionsPerQuarterNote,
                                                        int actualNotes, int normalNotes) {
    std::pair<RhythmFigure, int> result;
    int baseDuration = durationTicks;
    int dotCount = 0;

    const bool isTuplet = (actualNotes != normalNotes);
    const bool isPerfectMultipleTuple = (divisionsPerQuarterNote % actualNotes == 0);

    // Adjust the durationTicks for tuplets
    if (isTuplet) {
        if (isPerfectMultipleTuple) {
            durationTicks =
                std::floor(static_cast<float>(durationTicks) *
                           (static_cast<float>(actualNotes) / static_cast<float>(normalNotes)));
        } else {
            durationTicks =
                std::floor((static_cast<float>(durationTicks) * static_cast<float>(actualNotes)) /
                           static_cast<float>(normalNotes));
        }
    }

    // Determine base note type and dot count
    if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 5)) {
        result.first = RhythmFigure::MAXIMA;
        baseDuration = divisionsPerQuarterNote * std::pow(2, 5);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 4)) {
        result.first = RhythmFigure::LONG;
        baseDuration = divisionsPerQuarterNote * std::pow(2, 4);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 3)) {
        result.first = RhythmFigure::BREVE;
        baseDuration = divisionsPerQuarterNote * std::pow(2, 3);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 2)) {
        result.first = RhythmFigure::WHOLE;
        baseDuration = divisionsPerQuarterNote * std::pow(2, 2);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 1)) {
        result.first = RhythmFigure::HALF;
        baseDuration = divisionsPerQuarterNote * std::pow(2, 1);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, 0)) {
        result.first = RhythmFigure::QUARTER;
        baseDuration = divisionsPerQuarterNote * std::pow(2, 0);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -1)) {
        result.first = RhythmFigure::EIGHTH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -1);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -2)) {
        result.first = RhythmFigure::N16TH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -2);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -3)) {
        result.first = RhythmFigure::N32ND;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -3);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -4)) {
        result.first = RhythmFigure::N64TH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -4);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -5)) {
        result.first = RhythmFigure::N128TH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -5);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -6)) {
        result.first = RhythmFigure::N256TH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -6);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -7)) {
        result.first = RhythmFigure::N512TH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -7);
    } else if (durationTicks >= divisionsPerQuarterNote * std::pow(2, -8)) {
        result.first = RhythmFigure::N1024TH;
        baseDuration = divisionsPerQuarterNote * std::pow(2, -8);
    } else {
        std::string errMsg = "Unable to convert durationTick to RhythmFigure\n";
        errMsg += "durationTicks: " + std::to_string(durationTicks) + "\n";
        errMsg += "divisionsPerQuarterNote: " + std::to_string(divisionsPerQuarterNote) + "\n";
        errMsg += "actualNotes: " + std::to_string(actualNotes) + "\n";
        errMsg += "normalNotes: " + std::to_string(normalNotes) + "\n";
        LOG_ERROR(errMsg);
    }

    // Calculate dot count
    while (durationTicks > baseDuration) {
        baseDuration += baseDuration / 2;
        dotCount++;
    }

    result.second = dotCount;
    return result;
}

// std::pair<std::string, int> Helper::ticks2noteType(const int ticks,
//                                                    const int divisionsPerQuarterNote) {
//     const float ratio = static_cast<float>(ticks) / static_cast<float>(divisionsPerQuarterNote);
//     const int scaledRatio =
//         std::round(ratio * 1000000.0f);  // Round to the nearest integer, like MusicXML scores

//     // Microsoft Visual C++ Compiler doesn't support 'case range' like GCC and
//     // Clang/LLVM Replaced this feature using multiple 'if' statements
// #ifdef _MSC_VER
//     if ((scaledRatio >= 56000000) && (scaledRatio <= 63999999)) {
//         return {MUSIC_XML::NOTE_TYPE::MAXIMA_DOT_DOT, 2};
//     }  // 32:1
//     if ((scaledRatio >= 48000000) && (scaledRatio <= 55999999)) {
//         return {MUSIC_XML::NOTE_TYPE::MAXIMA_DOT, 1};
//     }  // 32:1
//     if ((scaledRatio >= 32000000) && (scaledRatio <= 47999999)) {
//         return {MUSIC_XML::NOTE_TYPE::MAXIMA, 0};
//     }  // 32:1

//     if ((scaledRatio >= 28000000) && (scaledRatio <= 31999999)) {
//         return {MUSIC_XML::NOTE_TYPE::LONG_DOT_DOT, 2};
//     }  // 16:1
//     if ((scaledRatio >= 24000000) && (scaledRatio <= 27999999)) {
//         return {MUSIC_XML::NOTE_TYPE::LONG_DOT, 1};
//     }  // 16:1
//     if ((scaledRatio >= 16000000) && (scaledRatio <= 23999999)) {
//         return {MUSIC_XML::NOTE_TYPE::LONG, 0};
//     }  // 16:1

//     if ((scaledRatio >= 14000000) && (scaledRatio <= 15999999)) {
//         return {MUSIC_XML::NOTE_TYPE::BREVE_DOT_DOT, 2};
//     }  // 8:1
//     if ((scaledRatio >= 12000000) && (scaledRatio <= 13999999)) {
//         return {MUSIC_XML::NOTE_TYPE::BREVE_DOT, 1};
//     }  // 8:1
//     if ((scaledRatio >= 8000000) && (scaledRatio <= 11999999)) {
//         return {MUSIC_XML::NOTE_TYPE::BREVE, 0};
//     }  // 8:1

//     if ((scaledRatio >= 7000000) && (scaledRatio <= 7999999)) {
//         return {MUSIC_XML::NOTE_TYPE::WHOLE_DOT_DOT, 2};
//     }  // 4:1
//     if ((scaledRatio >= 6000000) && (scaledRatio <= 6999999)) {
//         return {MUSIC_XML::NOTE_TYPE::WHOLE_DOT, 1};
//     }  // 4:1
//     if ((scaledRatio >= 4000000) && (scaledRatio <= 5999999)) {
//         return {MUSIC_XML::NOTE_TYPE::WHOLE, 0};
//     }  // 4:1

//     if ((scaledRatio >= 3500000) && (scaledRatio <= 3999999)) {
//         return {MUSIC_XML::NOTE_TYPE::HALF_DOT_DOT, 2};
//     }  // 2:1
//     if ((scaledRatio >= 3000000) && (scaledRatio <= 3499999)) {
//         return {MUSIC_XML::NOTE_TYPE::HALF_DOT, 1};
//     }  // 2:1
//     if ((scaledRatio >= 2000000) && (scaledRatio <= 2999999)) {
//         return {MUSIC_XML::NOTE_TYPE::HALF, 0};
//     }  // 2:1

//     if ((scaledRatio >= 1750000) && (scaledRatio <= 1999999)) {
//         return {MUSIC_XML::NOTE_TYPE::QUARTER_DOT_DOT, 2};
//     }  // 1:1
//     if ((scaledRatio >= 1500000) && (scaledRatio <= 1749999)) {
//         return {MUSIC_XML::NOTE_TYPE::QUARTER_DOT, 1};
//     }  // 1:1
//     if ((scaledRatio >= 1000000) && (scaledRatio <= 1499999)) {
//         return {MUSIC_XML::NOTE_TYPE::QUARTER, 0};
//     }  // 1:1

//     if ((scaledRatio >= 875000) && (scaledRatio <= 999999)) {
//         return {MUSIC_XML::NOTE_TYPE::EIGHTH_DOT_DOT, 2};
//     }  // 1:2
//     if ((scaledRatio >= 750000) && (scaledRatio <= 874999)) {
//         return {MUSIC_XML::NOTE_TYPE::EIGHTH_DOT, 1};
//     }  // 1:2
//     if ((scaledRatio >= 500000) && (scaledRatio <= 749999)) {
//         return {MUSIC_XML::NOTE_TYPE::EIGHTH, 0};
//     }  // 1:2

//     if ((scaledRatio >= 437500) && (scaledRatio <= 499999)) {
//         return {MUSIC_XML::NOTE_TYPE::N16TH_DOT_DOT, 2};
//     }  // 1:4
//     if ((scaledRatio >= 375000) && (scaledRatio <= 437499)) {
//         return {MUSIC_XML::NOTE_TYPE::N16TH_DOT, 1};
//     }  // 1:4
//     if ((scaledRatio >= 250000) && (scaledRatio <= 374999)) {
//         return {MUSIC_XML::NOTE_TYPE::N16TH, 0};
//     }  // 1:4

//     if ((scaledRatio >= 218750) && (scaledRatio <= 249999)) {
//         return {MUSIC_XML::NOTE_TYPE::N32ND_DOT_DOT, 2};
//     }  // 1:8
//     if ((scaledRatio >= 187500) && (scaledRatio <= 218749)) {
//         return {MUSIC_XML::NOTE_TYPE::N32ND_DOT, 1};
//     }  // 1:8
//     if ((scaledRatio >= 125000) && (scaledRatio <= 187499)) {
//         return {MUSIC_XML::NOTE_TYPE::N32ND, 0};
//     }  // 1:8

//     if ((scaledRatio >= 109375) && (scaledRatio <= 124999)) {
//         return {MUSIC_XML::NOTE_TYPE::N64TH_DOT_DOT, 2};
//     }  // 1:16
//     if ((scaledRatio >= 93750) && (scaledRatio <= 109374)) {
//         return {MUSIC_XML::NOTE_TYPE::N64TH_DOT, 1};
//     }  // 1:16
//     if ((scaledRatio >= 62500) && (scaledRatio <= 93749)) {
//         return {MUSIC_XML::NOTE_TYPE::N64TH, 0};
//     }  // 1:16

//     if ((scaledRatio >= 54688) && (scaledRatio <= 62499)) {
//         return {MUSIC_XML::NOTE_TYPE::N128TH_DOT_DOT, 2};
//     }  // 1:32
//     if ((scaledRatio >= 46875) && (scaledRatio <= 54687)) {
//         return {MUSIC_XML::NOTE_TYPE::N128TH_DOT, 1};
//     }  // 1:32
//     if ((scaledRatio >= 31250) && (scaledRatio <= 46874)) {
//         return {MUSIC_XML::NOTE_TYPE::N128TH, 0};
//     }  // 1:32

//     if ((scaledRatio >= 27344) && (scaledRatio <= 31249)) {
//         return {MUSIC_XML::NOTE_TYPE::N256TH_DOT_DOT, 2};
//     }  // 1:64
//     if ((scaledRatio >= 23438) && (scaledRatio <= 27343)) {
//         return {MUSIC_XML::NOTE_TYPE::N256TH_DOT, 1};
//     }  // 1:64
//     if ((scaledRatio >= 15625) && (scaledRatio <= 23437)) {
//         return {MUSIC_XML::NOTE_TYPE::N256TH, 0};
//     }  // 1:64

//     if ((scaledRatio >= 13672) && (scaledRatio <= 15624)) {
//         return {MUSIC_XML::NOTE_TYPE::N512TH_DOT_DOT, 2};
//     }  // 1:128
//     if ((scaledRatio >= 11719) && (scaledRatio <= 13671)) {
//         return {MUSIC_XML::NOTE_TYPE::N512TH_DOT, 1};
//     }  // 1:128
//     if ((scaledRatio >= 7813) && (scaledRatio <= 11718)) {
//         return {MUSIC_XML::NOTE_TYPE::N512TH, 0};
//     }  // 1:128

//     if ((scaledRatio >= 6836) && (scaledRatio <= 7812)) {
//         return {MUSIC_XML::NOTE_TYPE::N1024TH_DOT_DOT, 2};
//     }  // 1:256
//     if ((scaledRatio >= 5859) && (scaledRatio <= 6835)) {
//         return {MUSIC_XML::NOTE_TYPE::N1024TH_DOT, 1};
//     }  // 1:256
//     if ((scaledRatio >= 3906) && (scaledRatio <= 5858)) {
//         return {MUSIC_XML::NOTE_TYPE::N1024TH, 0};
//     }  // 1:256

//     // switch/case default option
//     LOG_ERROR(
//         "Unable to convert " + std::to_string(ticks) + " 'ticks' value to 'noteType' string\n " +
//         "using 'divisionPerQuarterNote'=" + std::to_string(divisionsPerQuarterNote) +
//         " and 'scaledRatio'=" + std::to_string(scaledRatio) + "\n\n" +
//         "If you are working based on a XML file, this file can be corrupted!\nYou can try to "
//         "fix this problem: \na) Open/Import this XML file in a modern score editor software "
//         "(like MuseScore 4, Avid Sibelius 2023 or Makemusic Finale 27)\nb) Generate a new "
//         "version of this sheet music by exporting this file as a new *.xml or *.musicxml "
//         "file\nc) Return to your 'maialib environment' and update your Score object "
//         "constructor file path to point to the new generated file\nd) Run your code again!
//         Done!");
//     return {};

// #else
//     // GCC and Clang/LLVM compilers have 'case range' support
//     switch (scaledRatio) {
//         case 56000000 ... 63999999:
//             return {MUSIC_XML::NOTE_TYPE::MAXIMA_DOT_DOT, 2};  // 32:1
//         case 48000000 ... 55999999:
//             return {MUSIC_XML::NOTE_TYPE::MAXIMA_DOT, 1};  // 32:1
//         case 32000000 ... 47999999:
//             return {MUSIC_XML::NOTE_TYPE::MAXIMA, 0};  // 32:1

//         case 28000000 ... 31999999:
//             return {MUSIC_XML::NOTE_TYPE::LONG_DOT_DOT, 2};  // 16:1
//         case 24000000 ... 27999999:
//             return {MUSIC_XML::NOTE_TYPE::LONG_DOT, 1};  // 16:1
//         case 16000000 ... 23999999:
//             return {MUSIC_XML::NOTE_TYPE::LONG, 0};  // 16:1

//         case 14000000 ... 15999999:
//             return {MUSIC_XML::NOTE_TYPE::BREVE_DOT_DOT, 2};  // 8:1
//         case 12000000 ... 13999999:
//             return {MUSIC_XML::NOTE_TYPE::BREVE_DOT, 1};  // 8:1
//         case 8000000 ... 11999999:
//             return {MUSIC_XML::NOTE_TYPE::BREVE, 0};  // 8:1

//         case 7000000 ... 7999999:
//             return {MUSIC_XML::NOTE_TYPE::WHOLE_DOT_DOT, 2};  // 4:1
//         case 6000000 ... 6999999:
//             return {MUSIC_XML::NOTE_TYPE::WHOLE_DOT, 1};  // 4:1
//         case 4000000 ... 5999999:
//             return {MUSIC_XML::NOTE_TYPE::WHOLE, 0};  // 4:1

//         case 3500000 ... 3999999:
//             return {MUSIC_XML::NOTE_TYPE::HALF_DOT_DOT, 2};  // 2:1
//         case 3000000 ... 3499999:
//             return {MUSIC_XML::NOTE_TYPE::HALF_DOT, 1};  // 2:1
//         case 2000000 ... 2999999:
//             return {MUSIC_XML::NOTE_TYPE::HALF, 0};  // 2:1

//         case 1750000 ... 1999999:
//             return {MUSIC_XML::NOTE_TYPE::QUARTER_DOT_DOT, 2};  // 1:1
//         case 1500000 ... 1749999:
//             return {MUSIC_XML::NOTE_TYPE::QUARTER_DOT, 1};  // 1:1
//         case 1000000 ... 1499999:
//             return {MUSIC_XML::NOTE_TYPE::QUARTER, 0};  // 1:1

//         case 875000 ... 999999:
//             return {MUSIC_XML::NOTE_TYPE::EIGHTH_DOT_DOT, 2};  // 1:2
//         case 750000 ... 874999:
//             return {MUSIC_XML::NOTE_TYPE::EIGHTH_DOT, 1};  // 1:2
//         case 500000 ... 749999:
//             return {MUSIC_XML::NOTE_TYPE::EIGHTH, 0};  // 1:2

//         case 437500 ... 499999:
//             return {MUSIC_XML::NOTE_TYPE::N16TH_DOT_DOT, 2};  // 1:4
//         case 375000 ... 437499:
//             return {MUSIC_XML::NOTE_TYPE::N16TH_DOT, 1};  // 1:4
//         case 250000 ... 374999:
//             return {MUSIC_XML::NOTE_TYPE::N16TH, 0};  // 1:4

//         case 218750 ... 249999:
//             return {MUSIC_XML::NOTE_TYPE::N32ND_DOT_DOT, 2};  // 1:8
//         case 187500 ... 218749:
//             return {MUSIC_XML::NOTE_TYPE::N32ND_DOT, 1};  // 1:8
//         case 125000 ... 187499:
//             return {MUSIC_XML::NOTE_TYPE::N32ND, 0};  // 1:8

//         case 109375 ... 124999:
//             return {MUSIC_XML::NOTE_TYPE::N64TH_DOT_DOT, 2};  // 1:16
//         case 93750 ... 109374:
//             return {MUSIC_XML::NOTE_TYPE::N64TH_DOT, 1};  // 1:16
//         case 62500 ... 93749:
//             return {MUSIC_XML::NOTE_TYPE::N64TH, 0};  // 1:16

//         case 54688 ... 62499:
//             return {MUSIC_XML::NOTE_TYPE::N128TH_DOT_DOT, 2};  // 1:32
//         case 46875 ... 54687:
//             return {MUSIC_XML::NOTE_TYPE::N128TH_DOT, 1};  // 1:32
//         case 31250 ... 46874:
//             return {MUSIC_XML::NOTE_TYPE::N128TH, 0};  // 1:32

//         case 27344 ... 31249:
//             return {MUSIC_XML::NOTE_TYPE::N256TH_DOT_DOT, 2};  // 1:64
//         case 23438 ... 27343:
//             return {MUSIC_XML::NOTE_TYPE::N256TH_DOT, 1};  // 1:64
//         case 15625 ... 23437:
//             return {MUSIC_XML::NOTE_TYPE::N256TH, 0};  // 1:64

//         case 13672 ... 15624:
//             return {MUSIC_XML::NOTE_TYPE::N512TH_DOT_DOT, 2};  // 1:128
//         case 11719 ... 13671:
//             return {MUSIC_XML::NOTE_TYPE::N512TH_DOT, 1};  // 1:128
//         case 7813 ... 11718:
//             return {MUSIC_XML::NOTE_TYPE::N512TH, 0};  // 1:128

//         case 6836 ... 7812:
//             return {MUSIC_XML::NOTE_TYPE::N1024TH_DOT_DOT, 2};  // 1:256
//         case 5859 ... 6835:
//             return {MUSIC_XML::NOTE_TYPE::N1024TH_DOT, 1};  // 1:256
//         case 3906 ... 5858:
//             return {MUSIC_XML::NOTE_TYPE::N1024TH, 0};  // 1:256
//         default:
//             LOG_ERROR(
//                 "Unable to convert " + std::to_string(ticks) +
//                 " 'ticks' value to 'noteType' string\n " +
//                 "using 'divisionPerQuarterNote'=" + std::to_string(divisionsPerQuarterNote) +
//                 " and 'scaledRatio'=" + std::to_string(scaledRatio) + "\n\n" +
//                 "If you are working based on a XML file, this file can be corrupted!\nYou can try
//                 " "to fix this problem: \na) Open/Import this XML file in a modern score editor "
//                 "software "
//                 "(like MuseScore 4, Avid Sibelius 2023 or Makemusic Finale 27)\nb) Generate a new
//                 " "version of this sheet music by exporting this file as a new *.xml or
//                 *.musicxml " "file\nc) Return to your 'maialib environment' and update your Score
//                 object " "constructor file path to point to the new generated file\nd) Run your
//                 code again! " "Done!");
//             break;
//     }

//     return {};
// #endif
// }

float Helper::noteSimilarity(std::string& pitchClass_A, int octave_A, const float duration_A,
                             std::string& pitchClass_B, int octave_B, const float duration_B,
                             float& durRatio, float& pitRatio, const bool enableEnharmonic) {
    // Special case: Mixing unknown pitches comparison:
    if ((pitchClass_A == MUSIC_XML::PITCH::ALL) && (pitchClass_B == MUSIC_XML::PITCH::ALL)) {
        // Set default octaves:
        pitchClass_A = "A";
        pitchClass_B = "A";
    } else if ((pitchClass_A == MUSIC_XML::PITCH::ALL) && (pitchClass_B != MUSIC_XML::PITCH::ALL)) {
        pitchClass_A = pitchClass_B;
    } else if ((pitchClass_A != MUSIC_XML::PITCH::ALL) && (pitchClass_B == MUSIC_XML::PITCH::ALL)) {
        pitchClass_B = pitchClass_A;
    } else if ((pitchClass_A == MUSIC_XML::PITCH::REST) &&
               (pitchClass_B == MUSIC_XML::PITCH::ALL)) {
        pitchClass_B = pitchClass_A;
    } else if ((pitchClass_A == MUSIC_XML::PITCH::ALL) &&
               (pitchClass_B == MUSIC_XML::PITCH::REST)) {
        pitchClass_A = pitchClass_B;
    }

    // Special case: Mixing unknown octaves comparison:
    if ((octave_A == MUSIC_XML::OCTAVE::ALL) && (octave_B == MUSIC_XML::OCTAVE::ALL)) {
        // Set default octaves:
        octave_A = 4;
        octave_B = 4;
    } else if ((octave_A == MUSIC_XML::OCTAVE::ALL) && (octave_B != MUSIC_XML::OCTAVE::ALL)) {
        octave_A = octave_B;
    } else if ((octave_A != MUSIC_XML::OCTAVE::ALL) && (octave_B == MUSIC_XML::OCTAVE::ALL)) {
        octave_B = octave_A;
    }

    // Pitch Concatenate: pitchClass + Octave:
    std::string pitch_A, pitch_B;

    pitch_A = (pitchClass_A == "rest") ? pitchClass_A : pitchClass_A + std::to_string(octave_A);
    pitch_B = (pitchClass_B == "rest") ? pitchClass_B : pitchClass_B + std::to_string(octave_B);

    bool is_enharmonic = false;
    if (enableEnharmonic) {
        is_enharmonic = isEnharmonic(pitch_A, pitch_B);
    }

    // Compute the duration ratio between these 2 musicNotes:
    durRatio = durationRatio(duration_A, duration_B);

    // Compute the pitch ration between these 2 musicNotes:
    pitRatio = (is_enharmonic) ? 1.0f : pitchRatio(pitch_A, pitch_B);

    // Compute the distance between these 2 musicNotes:
    const float averageSimilarity = (durRatio + pitRatio) / 2.0f;

    return averageSimilarity;
}

float Helper::pitch2freq(const std::string& pitch) {
    // Special case: rest
    if (pitch.empty()) {
        return 0.0f;
    }

    // Get splited data from pitch string:
    std::string pitchClass, pitchStep, accidental;
    int octave;
    float alter;
    splitPitch(pitch, pitchClass, pitchStep, octave, alter, accidental);

    // Verify if there is a quarter accident in this note:
    float quarterRatio = 1.0f;
    if (pitch.size() > 2) {
        if (accidental == "1x") {
            quarterRatio = 1.005f;  // Empirical ratio
        } else if (accidental == "3x") {
            quarterRatio = 1.015f;  // Empirical ratio
        } else if (accidental == "1b") {
            quarterRatio = 0.095f;  // Empirical ratio
        } else if (accidental == "3b") {
            quarterRatio = 0.085f;  // Empirical ratio
        } else {
            // No quarter accident:
            quarterRatio = 1.0f;
        }
    }

    // Compute the frequency:
    float freq = 0.0f;
    switch (hash(pitchClass.c_str())) {
        case hash("C"):
            freq = 16.35f * pow(2, octave);
            break;
        case hash("Dbb"):
            freq = 16.40f * pow(2, octave);  // Aproximation of C
            break;
        case hash("Db"):
            freq = 17.16f * pow(2, octave);
            break;
        case hash("C#"):
            freq = 17.40f * pow(2, octave);
            break;
        case hash("Cx"):
            freq = 18.30f * pow(2, octave);  // Aproximation of D
            break;
        case hash("D"):
            freq = 18.35f * pow(2, octave);
            break;
        case hash("Ebb"):
            freq = 18.40f * pow(2, octave);  // Aproximation of D
            break;
        case hash("Eb"):
            freq = 19.31f * pow(2, octave);
            break;
        case hash("Fbb"):
            freq = 19.40f * pow(2, octave);  // Aproximation of Eb
            break;
        case hash("D#"):
            freq = 19.57f * pow(2, octave);
            break;
        case hash("Dx"):
            freq = 20.55f * pow(2, octave);  // Aproximation of E
            break;
        case hash("E"):
            freq = 20.60f * pow(2, octave);
            break;
        case hash("Fb"):
            freq = 20.34f * pow(2, octave);
            break;
        case hash("E#"):
            freq = 22.02f * pow(2, octave);
            break;
        case hash("F"):
            freq = 21.83f * pow(2, octave);
            break;
        case hash("Gbb"):
            freq = 22.33f * pow(2, octave);  // Aproximation of F
            break;
        case hash("Gb"):
            freq = 22.89f * pow(2, octave);
            break;
        case hash("Ex"):
            freq = 23.00f * pow(2, octave);  // Aproximation of F#
            break;
        case hash("F#"):
            freq = 23.20f * pow(2, octave);
            break;
        case hash("Fx"):
            freq = 24.00f * pow(2, octave);  // Aproximation of G
            break;
        case hash("G"):
            freq = 24.50f * pow(2, octave);
            break;
        case hash("Abb"):
            freq = 25.00f * pow(2, octave);  // Aproximation of G
            break;
        case hash("Ab"):
            freq = 25.75f * pow(2, octave);
            break;
        case hash("G#"):
            freq = 26.10f * pow(2, octave);
            break;
        case hash("Gx"):
            freq = 27.00f * pow(2, octave);  // Aproximation of A
            break;
        case hash("A"):
            freq = 27.50f * pow(2, octave);
            break;
        case hash("Bbb"):
            freq = 28.00f * pow(2, octave);  // Aproximation of A
            break;
        case hash("Bb"):
            freq = 28.43f * pow(2, octave);
            break;
        case hash("Cbb"):
            freq = 28.60f * pow(2, octave);  // Aproximation of Bb
            break;
        case hash("A#"):
            freq = 28.97f * pow(2, octave);
            break;
        case hash("Ax"):
            freq = 29.10f * pow(2, octave);  // Aproximation of B
            break;
        case hash("B"):
            freq = 30.36f * pow(2, octave);
            break;
        case hash("Cb"):
            freq = 30.52f * pow(2, octave);
            break;
        case hash("B#"):
            freq = 33.03f * pow(2, octave);
            break;
        case hash("Bx"):
            freq = 34.30f * pow(2, octave);  // Aproximation of C#
            break;
        default:
            LOG_ERROR("Pitch not found!");
    }

    // Apply the quarter accidental to the pure tone frequency value:
    freq *= quarterRatio;

    return freq;
}

std::pair<std::string, int> Helper::freq2pitch(const float freq, const std::string& accType) {
    const std::pair<int, int> result = freq2midiNote(freq);

    const int& closestMIDINote = result.first;
    const int& centsOffset = result.second;

    const std::string pitch = midiNote2pitch(closestMIDINote, accType);

    return std::make_pair(pitch, centsOffset);
}

float Helper::pitchRatio(const std::string& pitch_A, const std::string& pitch_B) {
    // SPECIAL CASES:
    // Special case 01: Well-known with Unknown pitch value:
    if ((pitch_A == MUSIC_XML::PITCH::ALL || pitch_B == MUSIC_XML::PITCH::ALL) &&
        (pitch_A != "rest") && (pitch_B != "rest")) {
        return 1.0f;
    }

    // Special case 02: Compare a Unknown pitch with a rest:
    if ((pitch_A == MUSIC_XML::PITCH::ALL) && (pitch_B == "rest")) {
        return 0.0f;
    } else if ((pitch_A == "rest") && (pitch_B == MUSIC_XML::PITCH::ALL)) {
        return 0.0f;
    }

    // Get the frequency of these absolute pitches:
    float freq_A = pitch2freq(pitch_A);
    float freq_B = pitch2freq(pitch_B);

    if (freq_A > freq_B) {
        return freq_B / freq_A;
    } else if (freq_A < freq_B) {
        return freq_A / freq_B;
    } else {
        return 1.0f;
    }
}

void Helper::splitPitch(const std::string& pitch, std::string& pitchClass, std::string& pitchStep,
                        int& octave, float& alterValue, std::string& alterSymbol) {
    // Rest case: This is necessary to prevent: empty pitchClass + alterSymbol
    if (pitch.empty() || (pitch.find(MUSIC_XML::PITCH::REST) != std::string::npos)) {
        pitchClass = "rest";
        pitchStep = "rest";
        octave = 0;
        alterValue = 0;
        alterSymbol = "";
        return;
    }

    const size_t pitchSize = pitch.size();

    // Error checking:
    if (pitchSize > 4) {
        LOG_ERROR("The pitch '" + pitch + "' have a invalid length: " + std::to_string(pitchSize));
    }

    const std::string diatonicPitch = pitch.substr(0, 1);
    bool foundPitch = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(),
                                diatonicPitch) != c_C_diatonicScale.end();

    // Error checking:
    if (!foundPitch && !pitch.empty()) {
        LOG_ERROR("Unknown diatonc pitch: " + diatonicPitch);
    }

    // Verify if the input data is a full pitch or just a pitchClass. Ex.: "A4"
    // or "A"
    const bool isFullPitch = (isdigit(pitch.back())) ? true : false;

    pitchStep = pitch.substr(0, 1);
    pitchClass = (isFullPitch) ? pitch.substr(0, pitchSize - 1) : pitch;
    octave = (isFullPitch) ? static_cast<int>(pitch.back()) - 48 : 4;

    const size_t pitchClassSize = pitchClass.size();

    // ===== GET ALTER SYMBOL AND ALTER VALUE ===== //
    // Store the alter symbol: # / b / x / bb
    if (pitchClassSize > 1) {
        alterSymbol = pitchClass.substr(1, pitchClassSize);
    }

    bool foundAlterSymbol =
        std::find(c_alterSymbol.begin(), c_alterSymbol.end(), alterSymbol) != c_alterSymbol.end();
    if (!foundAlterSymbol && !alterSymbol.empty()) {
        LOG_ERROR("Unknown alter symbol: " + alterSymbol);
    }

    switch (pitchSize) {
        case 2:
            alterSymbol = "";
            break;
        case 3:
            alterSymbol = pitch.substr(1, 1);
            break;
        case 4:
            alterSymbol = pitch.substr(1, 2);
            break;
    }

    alterValue = alterSymbol2Value(alterSymbol);

    return;
}

float Helper::durationRatio(float duration_A, float duration_B) {
    // Special case: Two Unknown durations:
    if ((duration_A == MUSIC_XML::DURATION::ALL) && (duration_B == MUSIC_XML::DURATION::ALL)) {
        return 1.0f;
    }

    // Special case: Mix well-known and Unknown durations:
    if ((duration_A == MUSIC_XML::DURATION::ALL) && (duration_B != MUSIC_XML::DURATION::ALL)) {
        duration_A = duration_B;
    } else if ((duration_A != MUSIC_XML::DURATION::ALL) &&
               (duration_B == MUSIC_XML::DURATION::ALL)) {
        duration_B = duration_A;
    }

    // Error checking: negative values
    if (duration_A < 0 || duration_B < 0) {
        LOG_ERROR("Both duration values must be positive!");
        return 0.0f;
    }

    if (duration_A == duration_B) {
        return 1.0f;
    }

    // Compute the ratio from 0 to 100%:
    return (duration_A > duration_B) ? duration_B / duration_A : duration_A / duration_B;
}

std::string Helper::rhythmFigure2noteType(const RhythmFigure rhythmFigure) {
    switch (rhythmFigure) {
        case RhythmFigure::MAXIMA:
            return MUSIC_XML::NOTE_TYPE::MAXIMA;
            break;
        case RhythmFigure::LONG:
            return MUSIC_XML::NOTE_TYPE::LONG;
            break;
        case RhythmFigure::BREVE:
            return MUSIC_XML::NOTE_TYPE::BREVE;
            break;
        case RhythmFigure::WHOLE:
            return MUSIC_XML::NOTE_TYPE::WHOLE;
            break;
        case RhythmFigure::HALF:
            return MUSIC_XML::NOTE_TYPE::HALF;
            break;
        case RhythmFigure::QUARTER:
            return MUSIC_XML::NOTE_TYPE::QUARTER;
            break;
        case RhythmFigure::EIGHTH:
            return MUSIC_XML::NOTE_TYPE::EIGHTH;
            break;
        case RhythmFigure::N16TH:
            return MUSIC_XML::NOTE_TYPE::N16TH;
            break;
        case RhythmFigure::N32ND:
            return MUSIC_XML::NOTE_TYPE::N32ND;
            break;
        case RhythmFigure::N64TH:
            return MUSIC_XML::NOTE_TYPE::N64TH;
            break;
        case RhythmFigure::N128TH:
            return MUSIC_XML::NOTE_TYPE::N128TH;
            break;
        case RhythmFigure::N256TH:
            return MUSIC_XML::NOTE_TYPE::N256TH;
            break;
        case RhythmFigure::N512TH:
            return MUSIC_XML::NOTE_TYPE::N512TH;
            break;
        case RhythmFigure::N1024TH:
            return MUSIC_XML::NOTE_TYPE::N1024TH;
            break;
    }

    LOG_ERROR("Unknown Duration type");
    return {};
}

int Helper::rhythmFigure2Ticks(const RhythmFigure rhythmFigure, const int divisionsPerQuarterNote) {
    switch (rhythmFigure) {
        case RhythmFigure::MAXIMA:
            return divisionsPerQuarterNote * std::pow(2, 5);
        case RhythmFigure::LONG:
            return divisionsPerQuarterNote * std::pow(2, 4);
        case RhythmFigure::BREVE:
            return divisionsPerQuarterNote * std::pow(2, 3);
        case RhythmFigure::WHOLE:
            return divisionsPerQuarterNote * std::pow(2, 2);
        case RhythmFigure::HALF:
            return divisionsPerQuarterNote * std::pow(2, 1);
        case RhythmFigure::QUARTER:
            return divisionsPerQuarterNote * std::pow(2, 0);
        case RhythmFigure::EIGHTH:
            return divisionsPerQuarterNote * std::pow(2, -1);
        case RhythmFigure::N16TH:
            return divisionsPerQuarterNote * std::pow(2, -2);
        case RhythmFigure::N32ND:
            return divisionsPerQuarterNote * std::pow(2, -3);
        case RhythmFigure::N64TH:
            return divisionsPerQuarterNote * std::pow(2, -4);
        case RhythmFigure::N128TH:
            return divisionsPerQuarterNote * std::pow(2, -5);
        case RhythmFigure::N256TH:
            return divisionsPerQuarterNote * std::pow(2, -6);
        case RhythmFigure::N512TH:
            return divisionsPerQuarterNote * std::pow(2, -7);
        case RhythmFigure::N1024TH:
            return divisionsPerQuarterNote * std::pow(2, -8);
    }

    LOG_ERROR("Unknown rhythmFigure: " + Helper::toString(rhythmFigure));
    return {};
}

RhythmFigure Helper::noteType2RhythmFigure(const std::string& noteType) {
    switch (hash(noteType.c_str())) {
        case hash("maxima"):
            return RhythmFigure::MAXIMA;
        case hash("long"):
            return RhythmFigure::LONG;
        case hash("breve"):
            return RhythmFigure::BREVE;
        case hash("whole"):
            return RhythmFigure::WHOLE;
        case hash("half"):
            return RhythmFigure::HALF;
        case hash("quarter"):
            return RhythmFigure::QUARTER;
        case hash("eighth"):
            return RhythmFigure::EIGHTH;
        case hash("16th"):
            return RhythmFigure::N16TH;
        case hash("32nd"):
            return RhythmFigure::N32ND;
        case hash("64th"):
            return RhythmFigure::N64TH;
        case hash("128th"):
            return RhythmFigure::N128TH;
        case hash("256th"):
            return RhythmFigure::N256TH;
        case hash("512th"):
            return RhythmFigure::N512TH;
        case hash("1024th"):
            return RhythmFigure::N1024TH;

        default:
            LOG_ERROR("Unknown note type: " + noteType);
            break;
    }

    return {};
}

const std::string Helper::number2pitch(const float number, const std::string& accType) {
    if ((number == 0.0f) || static_cast<int>(number) % 10 == 0) {
        return "rest";
    }

    // Get the pure pitch number without the octave:
    float purePitchAcc = fmod(number, 10.0f);

    // Get the pure pitch number:
    int purePitch = 0;
    if (accType == MUSIC_XML::ACCIDENT::SHARP) {
        purePitch = floor(purePitchAcc);
    } else {
        purePitch = floor(purePitchAcc) + 1;
    }

    // Get the accident number:
    float acc = purePitchAcc - static_cast<float>(purePitch);

    // Get the octave number:
    std::string octStr = std::to_string(static_cast<int>(floor(number / 10.0f)));

    // Select the accident type:
    std::string accStr;
    if (acc == 0.0f) {
        accStr = "";
    } else if (acc == 0.25f) {
        accStr = "1x";
    } else if (acc == 0.5f) {
        accStr = "#";
    } else if (acc == 0.75f) {
        accStr = "3x";
    } else if (acc == 1.0f) {
        accStr = "x";
    } else if (acc == -0.25f) {
        accStr = "1b";
    } else if (acc == -0.5f) {
        accStr = "b";
    } else if (acc == -0.75f) {
        accStr = "3b";
    } else if (acc == -1.0f) {
        accStr = "bb";
    } else {
        LOG_ERROR("Unknown accident symbol");
        return {};
    }

    // Pitch: Select a pure pitch value:
    std::string purePitchStr;
    switch (purePitch) {
        case 1:
            purePitchStr = "C";
            break;
        case 2:
            purePitchStr = "D";
            break;
        case 3:
            purePitchStr = "E";
            break;
        case 4:
            purePitchStr = "F";
            break;
        case 5:
            purePitchStr = "G";
            break;
        case 6:
            purePitchStr = "A";
            break;
        case 7:
            purePitchStr = "B";
            break;

        default:
            LOG_ERROR("Unknown pure pitch value");
            return {};
    }

    // Result:
    std::string pitch = purePitchStr + accStr + octStr;

    return pitch;
}

const std::string Helper::transposePitch(const std::string& pitch, const int semitones,
                                         const std::string& accType) {
    if (semitones == 0) {
        return pitch;
    }

    if (pitch == MUSIC_XML::PITCH::REST) {
        return MUSIC_XML::PITCH::REST;
    }

    // Get the pitch MIDI note number:
    const int midiNote = pitch2midiNote(pitch);

    // Transpose:
    const int transposedMidiNote = midiNote + semitones;

    // Return the Pitch transposed:
    return midiNote2pitch(transposedMidiNote, accType);
}

bool Helper::isEnharmonic(const std::string& pitch_A, const std::string& pitch_B) {
    // Get the pitch numbers:
    const float number_A = pitch2number(pitch_A);
    const float number_B = pitch2number(pitch_B);

    // Verify if these pitches are equal each other:
    if (number_A == number_B) {
        return true;
    } else {
        return false;
    }
}

float Helper::pitch2number(const std::string& pitch) {
    // Special case:
    if (pitch == "rest") {
        return 0.0f;
    }

    // Split pitch string data:
    const std::string purePitch = pitch.substr(0, 1);
    std::string acc;

    // Verify if there the pitch have a accident:
    if (pitch.size() > 2) {
        // Get the accident:
        acc = pitch.substr(1, pitch.size() - 2);
    }

    // Get the octave:
    const size_t octave = atoi(pitch.substr(pitch.size() - 1, 1).c_str());

    // Select the accident type:
    float accValue = 0.0f;
    if (!acc.empty()) {
        if (acc == "#") {
            accValue = 0.5f;
        } else if (acc == "b") {
            accValue = -0.5f;
        } else if (acc == "x") {
            accValue = 1.0f;
        } else if (acc == "bb") {
            accValue = -1.0f;
        } else if (acc == "1x") {
            accValue = 0.25f;
        } else if (acc == "3x") {
            accValue = 0.75f;
        } else if (acc == "1b") {
            accValue = -0.25f;
        } else if (acc == "3b") {
            accValue = -0.75;
        } else {
            LOG_ERROR("Unknown accident symbol");
            return 0.0f;
        }
    }

    // Select a pure pitch value:
    float basePitch = 0.0f;
    if (purePitch == "C") {
        basePitch = 1.0f;
    } else if (purePitch == "D") {
        basePitch = 2.0f;
    } else if (purePitch == "E") {
        basePitch = 3.0f;
    } else if (purePitch == "F") {
        basePitch = 4.0f;
    } else if (purePitch == "G") {
        basePitch = 5.0f;
    } else if (purePitch == "A") {
        basePitch = 6.0f;
    } else if (purePitch == "B") {
        basePitch = 7.0f;
    } else {
        LOG_ERROR("Unknown pure pitch value: " + purePitch);
        return 0.0f;
    }

    // Result:
    float pitchNumber = (basePitch + accValue) + (10.0f * static_cast<float>(octave));

    return pitchNumber;
}

const pugi::xpath_node_set Helper::getNodeSet(const pugi::xml_document& doc,
                                              const std::string& xPath) {
    PROFILE_FUNCTION();

    return doc.select_nodes(xPath.c_str());
}

int Helper::frequencies2cents(const float freq_A, const float freq_B) {
    const float cents = 1200.0f * log2f(freq_B / freq_A);
    return static_cast<int>(cents);
}

const std::string Helper::generateIdentation(int identPosition, int identSize) {
    std::string ident;

    const int numSpaces = identPosition * identSize;

    ident.append(numSpaces, ' ');

    return ident;
}

float Helper::freq2equalTemperament(const float freq, const float referenceFreq) {
    return referenceFreq * powf(2, (round(12.0f * log2f(freq / referenceFreq)) / 12.0f));
}

std::string Helper::toString(const RhythmFigure rhythmFigure) {
    switch (rhythmFigure) {
        case RhythmFigure::MAXIMA:
            return MUSIC_XML::NOTE_TYPE::MAXIMA;
        case RhythmFigure::LONG:
            return MUSIC_XML::NOTE_TYPE::LONG;
        case RhythmFigure::BREVE:
            return MUSIC_XML::NOTE_TYPE::BREVE;
        case RhythmFigure::WHOLE:
            return MUSIC_XML::NOTE_TYPE::WHOLE;
        case RhythmFigure::HALF:
            return MUSIC_XML::NOTE_TYPE::HALF;
        case RhythmFigure::QUARTER:
            return MUSIC_XML::NOTE_TYPE::QUARTER;
        case RhythmFigure::EIGHTH:
            return MUSIC_XML::NOTE_TYPE::EIGHTH;
        case RhythmFigure::N16TH:
            return MUSIC_XML::NOTE_TYPE::N16TH;
        case RhythmFigure::N32ND:
            return MUSIC_XML::NOTE_TYPE::N32ND;
        case RhythmFigure::N64TH:
            return MUSIC_XML::NOTE_TYPE::N64TH;
        case RhythmFigure::N128TH:
            return MUSIC_XML::NOTE_TYPE::N128TH;
        case RhythmFigure::N256TH:
            return MUSIC_XML::NOTE_TYPE::N256TH;
        case RhythmFigure::N512TH:
            return MUSIC_XML::NOTE_TYPE::N512TH;
        case RhythmFigure::N1024TH:
            return MUSIC_XML::NOTE_TYPE::N1024TH;
    }

    std::string errMgs = "Unable to convert rhythmFigure to a noteType string\n";
    errMgs += "rhythmFigure: " + std::to_string((int)rhythmFigure);
    LOG_ERROR(errMgs);
}

std::vector<float> Helper::getSemitonesDifferenceBetweenMelodies(
    const std::vector<Note>& referenceMelody, const std::vector<Note>& otherMelody) {
    const int referenceMelodySize = referenceMelody.size();
    const int otherMelodySize = otherMelody.size();

    if (referenceMelodySize < 2 || otherMelodySize < 2) {
        LOG_ERROR("referenceMelody and otherMelody must have 2 elements minimum!");
    }

    const int minSize = std::min(referenceMelodySize, otherMelodySize);
    const int numValidSemitones = minSize - 1;
    std::vector<float> semitones(numValidSemitones, 0.0f);

    for (int i = 0; i < numValidSemitones; i++) {
        // For the Reference Melody
        const Note& firstReferenceNote = referenceMelody.at(i);
        const Note& secondReferenceNote = referenceMelody.at(i + 1);

        int referenceSemitones = 0;
        if (firstReferenceNote.isNoteOff() || secondReferenceNote.isNoteOff()) {
            referenceSemitones = 0;
        } else {
            const Interval& referenceInterval = Interval(firstReferenceNote, secondReferenceNote);
            referenceSemitones = referenceInterval.getNumSemitones();
        }

        // For the Other Melody
        const Note& firstOtherNote = otherMelody.at(i);
        const Note& secondOtherNote = otherMelody.at(i + 1);

        int otherSemitones = 0;
        if (firstOtherNote.isNoteOff() || secondOtherNote.isNoteOff()) {
            otherSemitones = 0;
        } else {
            const Interval& otherInterval = Interval(firstOtherNote, secondOtherNote);
            otherSemitones = otherInterval.getNumSemitones();
        }

        semitones[i] = (float)referenceSemitones - (float)otherSemitones;
    }

    return semitones;
}

float Helper::calculateMelodyEuclideanSimilarity(const std::vector<Note>& melodyPattern,
                                                 const std::vector<Note>& otherMelody) {
    std::vector<float> semitones =
        Helper::getSemitonesDifferenceBetweenMelodies(melodyPattern, otherMelody);

    return calculateMelodyEuclideanSimilarity(semitones);
}

float Helper::calculateMelodyEuclideanSimilarity(const std::vector<float>& semitonesDifference) {
    // To power of 2
    std::vector<float> semitonesDiffPow2 = semitonesDifference;
    for (auto& semitone : semitonesDiffPow2) {
        semitone = std::pow(semitone, 2);
    }

    const float sumSquares =
        std::accumulate(semitonesDiffPow2.begin(), semitonesDiffPow2.end(), 0.0f);

    const float euclideanDistance = std::sqrt(sumSquares);
    const float similarity = 1.0f / (1.0f + euclideanDistance);  // Inverso da distância euclidiana

    return similarity;
}

std::vector<float> Helper::getDurationDifferenceBetweenRhythms(
    const std::vector<Note>& referenceRhythm, const std::vector<Note>& otherRhythm) {
    const int referenceRhythmSize = referenceRhythm.size();
    const int otherRhythmSize = otherRhythm.size();

    if (referenceRhythmSize < 2 || otherRhythmSize < 2) {
        LOG_ERROR("referenceRhythm and otherRhythm must have 2 elements minimum!");
    }

    // Normaliza cada duração pelo maior valor de duração na sequência
    float maxReferenceDuration = 0.0f;
    float maxOtherDuration = 0.0f;

    for (const auto& note : referenceRhythm) {
        maxReferenceDuration =
            std::max(maxReferenceDuration, note.getDuration().getQuarterDuration());
    }
    for (const auto& note : otherRhythm) {
        maxOtherDuration = std::max(maxOtherDuration, note.getDuration().getQuarterDuration());
    }

    const int minSize = std::min(referenceRhythmSize, otherRhythmSize);
    std::vector<float> durationDifferences(minSize, 0.0f);

    for (int i = 0; i < minSize; i++) {
        // Normaliza as durações pelo maior valor
        float normalizedReferenceDuration =
            referenceRhythm.at(i).getDuration().getQuarterDuration() / maxReferenceDuration;
        float normalizedOtherDuration =
            otherRhythm.at(i).getDuration().getQuarterDuration() / maxOtherDuration;

        // Calcula a diferença ao quadrado entre durações normalizadas
        durationDifferences[i] = normalizedReferenceDuration - normalizedOtherDuration;
    }

    return durationDifferences;
}

float Helper::calculateRhythmicEuclideanSimilarity(const std::vector<Note>& rhythmPattern,
                                                   const std::vector<Note>& otherRhythm) {
    const std::vector<float> durationDifferences =
        Helper::getDurationDifferenceBetweenRhythms(rhythmPattern, otherRhythm);

    return calculateRhythmicEuclideanSimilarity(durationDifferences);
}

float Helper::calculateRhythmicEuclideanSimilarity(const std::vector<float>& durationDifferences) {
    // To power of 2
    std::vector<float> durationDifferencesPow2 = durationDifferences;
    for (auto& diff : durationDifferencesPow2) {
        diff = std::pow(diff, 2);
    }

    // Soma dos quadrados das diferenças de durações normalizadas
    const float sumSquares =
        std::accumulate(durationDifferencesPow2.begin(), durationDifferencesPow2.end(), 0.0f);

    // Distância Euclidiana das diferenças de duração
    const float euclideanDistance = std::sqrt(sumSquares);

    // Similaridade é o inverso da distância euclidiana
    const float similarity = 1.0f / (1.0f + euclideanDistance);

    return similarity;
}
