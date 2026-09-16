#include "maiacore/helper.h"

#include <algorithm>
#include <array>
#include <cctype>
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

    if (octave < c_minPitchOctave || octave > c_maxPitchOctave) {
        LOG_ERROR("The MIDI Note '" + std::to_string(midiNote) + "' cannot be written using '" +
                  accType + "' accident type within octaves " + std::to_string(c_minPitchOctave) +
                  ".." + std::to_string(c_maxPitchOctave));
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
    if (pitch.empty() || (pitch.find(MUSIC_XML::PITCH::REST) != std::string::npos)) {
        return MUSIC_XML::MIDI::NUMBER::MIDI_REST;
    }

    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;
    splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

    return spelling2midiNote(pitchStep, alterValue, octave);
}

int Helper::spelling2midiNote(const std::string& pitchStep, const float alterValue,
                              const int octave) {
    const auto stepIt = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), pitchStep);
    if (stepIt == c_C_diatonicScale.end()) {
        LOG_ERROR("Unknown diatonic pitch step: " + pitchStep);
    }
    const auto stepIdx = static_cast<size_t>(std::distance(c_C_diatonicScale.begin(), stepIt));
    // SP2: alterValue is integral today because splitPitch rejects quarter-tone symbols;
    // widening c_alterSymbol must replace this truncation.
    return 12 * (octave + 1) + c_diatonicStepSemitones[stepIdx] + static_cast<int>(alterValue);
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

    // ===== STEP ===== //
    const std::string step = pitch.substr(0, 1);
    const bool isValidStep = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(),
                                       step) != c_C_diatonicScale.end();
    if (!isValidStep) {
        LOG_ERROR("Unknown diatonic pitch: " + step);
    }

    // ===== OCTAVE ===== //
    // Trailing digits, optionally preceded by a single '-' (e.g. "C-1"). Default octave: 4
    size_t octaveStart = pitch.size();
    while (octaveStart > 1 && std::isdigit(static_cast<unsigned char>(pitch[octaveStart - 1]))) {
        octaveStart--;
    }
    const bool hasOctave = octaveStart < pitch.size();
    if (hasOctave && octaveStart > 1 && pitch[octaveStart - 1] == '-') {
        octaveStart--;
    }

    int parsedOctave = 4;
    if (hasOctave) {
        const std::string octaveToken = pitch.substr(octaveStart);
        parsedOctave = (octaveToken.size() <= 3) ? std::stoi(octaveToken) : c_maxPitchOctave + 1;
        if (parsedOctave < c_minPitchOctave || parsedOctave > c_maxPitchOctave) {
            LOG_ERROR("Invalid octave value: " + octaveToken);
        }
    }

    // ===== ACCIDENTAL ===== //
    const std::string symbol = pitch.substr(1, octaveStart - 1);
    const bool isValidSymbol =
        symbol.empty() ||
        std::find(c_alterSymbol.begin(), c_alterSymbol.end(), symbol) != c_alterSymbol.end();
    if (!isValidSymbol) {
        LOG_ERROR("Unknown alter symbol: " + symbol);
    }

    const float value = alterSymbol2Value(symbol);
    if (spelling2midiNote(step, value, parsedOctave) < 0) {
        LOG_ERROR("The pitch '" + pitch + "' is below MIDI note 0");
    }

    pitchClass = step + symbol;
    pitchStep = step;
    octave = parsedOctave;
    alterValue = value;
    alterSymbol = symbol;
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
    return pitch2midiNote(pitch_A) == pitch2midiNote(pitch_B);
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
