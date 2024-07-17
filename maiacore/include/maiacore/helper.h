#ifndef HELPERS_H
#define HELPERS_H

#include <math.h>

#include <string>

#include "maiacore/constants.h"
#include "maiacore/note.h"
#include "nlohmann/json.hpp"
#include "pugi/pugixml.hpp"

class Interval;

class Helper {
   public:
    /**
     * @brief
     *
     * @param s
     * @param delimiter
     * @return std::vector<std::string>
     */
    static std::vector<std::string> splitString(const std::string& s, char delimiter);

    /**
     * @brief
     *
     * @param floatValue
     * @param digits
     * @return std::string
     */
    static std::string formatFloat(float floatValue, int digits);

    /**
     * @brief
     *
     * @param identPosition
     * @param identSize
     * @return const std::string
     */
    static const std::string generateIdentation(int identPosition, int identSize = 2);

    /**
     * @brief
     *
     * @param durationTicks
     * @param divisionsPerQuarterNote
     * @param actualNotes
     * @param normalNotes
     * @return std::pair<std::string, int>
     */
    static std::pair<std::string, int> ticks2noteType(int durationTicks,
                                                      int divisionsPerQuarterNote,
                                                      int actualNotes = 1, int normalNotes = 1);

    /**
     * @brief
     *
     * @param durationTicks
     * @param divisionsPerQuarterNote
     * @param actualNotes
     * @param normalNotes
     * @return std::pair<RhythmFigure, int>
     */
    static std::pair<RhythmFigure, int> ticks2rhythmFigure(int durationTicks,
                                                           int divisionsPerQuarterNote,
                                                           int actualNotes, int normalNotes);
    /**
     * @brief
     *
     * @param noteType
     * @param divisionsPerQuarterNote
     * @return int
     */
    static int noteType2ticks(std::string noteType, const int divisionsPerQuarterNote = 256);

    /**
     * @brief
     *
     * @param pitch
     * @return float
     */
    static float pitch2number(const std::string& pitch);

    /**
     * @brief
     *
     * @param number
     * @param accType
     * @return const std::string
     */
    static const std::string number2pitch(const float number,
                                          const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    /**
     * @brief
     *
     * @param freq
     * @param modelo
     * @return std::pair<int, int>
     */
    static std::pair<int, int> freq2midiNote(const float freq,
                                             std::function<int(float)> modelo = nullptr);

    /**
     * @brief
     *
     * @param midiNote
     * @param freqA4
     * @return float
     */
    static float midiNote2freq(const int midiNote, const float freqA4 = 440.0f);

    /**
     * @brief
     *
     * @param midiNote
     * @return int
     */
    static int midiNote2octave(const int midiNote);

    /**
     * @brief
     *
     * @param pitch
     * @return int
     */
    static int pitch2midiNote(const std::string& pitch);

    /**
     * @brief
     *
     * @param midiNote
     * @param accType
     * @return const std::string
     */
    static const std::string midiNote2pitch(const int midiNote, const std::string& accType = {});

    /**
     * @brief
     *
     * @param midiNote
     * @return const std::vector<std::string>
     */
    static const std::vector<std::string> midiNote2pitches(const int midiNote);

    /**
     * @brief
     *
     * @param notes
     * @param firstNoteAsReference
     * @return std::vector<Interval>
     */
    static std::vector<Interval> notes2Intervals(const std::vector<Note>& notes,
                                                 const bool firstNoteAsReference = false);

    /**
     * @brief
     *
     * @param pitch_A
     * @param pitch_B
     * @return int
     */
    static int semitonesBetweenPitches(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief
     *
     * @param pitchClass_A
     * @param octave_A
     * @param duration_A
     * @param pitchClass_B
     * @param octave_B
     * @param duration_B
     * @param durRatio
     * @param pitRatio
     * @param enableEnharmonic
     * @return float
     */
    static float noteSimilarity(std::string& pitchClass_A, int octave_A, const float duration_A,
                                std::string& pitchClass_B, int octave_B, const float duration_B,
                                float& durRatio, float& pitRatio,
                                const bool enableEnharmonic = false);

    /**
     * @brief
     *
     * @param pitch
     * @return float
     */
    static float pitch2freq(const std::string& pitch);

    /**
     * @brief
     *
     * @param freq
     * @param accType
     * @return std::pair<std::string, int>
     */
    static std::pair<std::string, int> freq2pitch(
        const float freq, const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    /**
     * @brief
     *
     * @param pitch_A
     * @param pitch_B
     * @return float
     */
    static float pitchRatio(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief
     *
     * @param pitch
     * @param pitchClass
     * @param pitchStep
     * @param octave
     * @param alterValue
     * @param alterSymbol
     */
    static void splitPitch(const std::string& pitch, std::string& pitchClass,
                           std::string& pitchStep, int& octave, float& alterValue,
                           std::string& alterSymbol);

    /**
     * @brief
     *
     * @param duration_A
     * @param duration_B
     * @return float
     */
    static float durationRatio(float duration_A, float duration_B);

    /**
     * @brief
     *
     * @param rhythmFigure
     * @return std::string
     */
    static std::string rhythmFigure2noteType(const RhythmFigure rhythmFigure);

    /**
     * @brief
     *
     * @param rhythmFigure
     * @param divisionsPerQuarterNote
     * @return int
     */
    static int rhythmFigure2Ticks(const RhythmFigure rhythmFigure,
                                  const int divisionsPerQuarterNote = 256);

    /**
     * @brief
     *
     * @param noteType
     * @return Duration
     */
    static RhythmFigure noteType2RhythmFigure(const std::string& noteType);

    /**
     * @brief
     *
     * @param pitch
     * @param semitones
     * @param accType
     * @return const std::string
     */
    static const std::string transposePitch(
        const std::string& pitch, const int semitones,
        const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    /**
     * @brief
     *
     * @param pitch_A
     * @param pitch_B
     * @return true
     * @return false
     */
    static bool isEnharmonic(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Get the Percentiles object
     *
     * @param table
     * @param desiredPercentiles
     * @return const nlohmann::json
     */
    static const nlohmann::json getPercentiles(const nlohmann::json& table,
                                               const std::vector<float>& desiredPercentiles);

    /**
     * @brief
     *
     * @param alterName
     * @return const std::string
     */
    static const std::string alterName2symbol(const std::string& alterName);

    /**
     * @brief
     *
     * @param alterSymbol
     * @return float
     */
    static float alterSymbol2Value(const std::string& alterSymbol);

    /**
     * @brief
     *
     * @param alterValue
     * @return const std::string
     */
    static const std::string alterValue2symbol(const float alterValue);

    /**
     * @brief
     *
     * @param alterValue
     * @return const std::string
     */
    static const std::string alterValue2Name(const float alterValue);

    /**
     * @brief Get the Note Node Data object
     *
     * @param node
     * @param pitch
     * @param pitchClass
     * @param alterSymbol
     * @param alterValue
     * @param octave
     * @param type
     * @param duration
     */
    static void getNoteNodeData(const pugi::xml_node& node, std::string& pitch,
                                std::string& pitchClass, std::string& alterSymbol, int& alterValue,
                                int& octave, std::string& type, float& duration);

    /**
     * @brief Get the Node Set object
     *
     * @param doc
     * @param xPath
     * @return const pugi::xpath_node_set
     */
    static const pugi::xpath_node_set getNodeSet(const pugi::xml_document& doc,
                                                 const std::string& xPath);

    /**
     * @brief
     *
     * @param freq_A
     * @param freq_B
     * @return int
     */
    static int frequencies2cents(const float freq_A, const float freq_B);

    /**
     * @brief
     *
     * @param freq
     * @param referenceFreq
     * @return float
     */
    static float freq2equalTemperament(const float freq, const float referenceFreq = 440.0f);

    /**
     * @brief
     *
     * @param rhythmFigure
     * @return std::string
     */
    static std::string toString(const RhythmFigure rhythmFigure);
};
#endif  // HELPERS_H
