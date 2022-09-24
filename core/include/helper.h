#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <string>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <fstream> // std::ofstream
#include <array>
#include <functional>

#include "pugixml.hpp"
#include "nlohmann/json.hpp"

#include "interval.h"
#include "constants.h"
#include "utils.h"

class Note;

class Helper
{
public:

    static std::vector<std::string> splitString(const std::string& s, char delimiter);
    static const std::string generateIdentation(int identPosition, int identSize = 2);

    static std::pair<std::string, int> ticks2noteType(const int ticks, const int divisionsPerQuarterNote = 256);
    static int noteType2ticks(std::string noteType, const int divisionsPerQuarterNote = 256);

    static float pitch2number(const std::string& pitch);
    static const std::string number2pitch(const float number, const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    static std::pair<int, int> freq2midiNote(const float freq, std::function<int(float)> modelo = nullptr);
    static float midiNote2freq(const int midiNote);
    static int midiNote2octave(const int midiNote);

    static int pitch2midiNote(const std::string& pitch);
    static const std::string midiNote2pitch(const int midiNote, const std::string& accType = {});
    static const std::vector<std::string> midiNote2pitches(const int midiNote);
    static std::vector<Interval> notes2Intervals(const std::vector<Note>& notes, const bool firstNoteAsReference = false);

    static int semitonesBetweenPitches(const std::string& pitch_A, const std::string& pitch_B);

    static float noteSimilarity(std::string& pitchClass_A, int octave_A, const float duration_A, std::string& pitchClass_B, int octave_B, const float duration_B, float& durRatio, float& pitRatio, const bool enableEnharmonic = false);

    static float pitch2freq(const std::string& pitch);
    static std::pair<std::string, int> freq2pitch(const float freq, const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    static float pitchRatio(const std::string &pitch_A, const std::string &pitch_B);
    static void splitPitch(const std::string& pitch, std::string& pitchClass, std::string& pitchStep, int& octave, float& alterValue, std::string& alterSymbol);
    static float durationRatio(float duration_A, float duration_B);
    static std::string duration2noteType(const Duration duration);
    static Duration noteType2duration(const std::string& noteType);

    static const std::string transposePitch(const std::string& pitch, const int semitones, const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    static bool isEnharmonic(const std::string& pitch_A, const std::string& pitch_B);
    static const nlohmann::json getPercentiles(const nlohmann::json& table, const std::vector<float>& desiredPercentiles);
    static const std::string alterName2symbol(const std::string& alterName);
    static float alterSymbol2Value(const std::string& alterSymbol);
    static const std::string alterValue2symbol(const float alterValue);
    static const std::string alterValue2Name(const float alterValue);

    static void getNoteNodeData(const pugi::xml_node& node, std::string& pitch, std::string& pitchClass, std::string& alterSymbol, int& alterValue, int& octave, std::string& type, float& duration);
    static const pugi::xpath_node_set getNodeSet(const pugi::xml_document& doc, const std::string& xPath);
};
#endif // HELPERS_H
