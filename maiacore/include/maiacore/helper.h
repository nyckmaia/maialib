#ifndef HELPERS_H
#define HELPERS_H

#include <math.h>
#include <string>
#include "maiacore/constants.h"
#include "maiacore/note.h"
#include "nlohmann/json.hpp"
#include "pugi/pugixml.hpp"

class Interval;

/**
 * @brief Helper class with static utility functions for music analysis, pitch/duration conversion, and MusicXML processing.
 *
 * This class provides a wide range of static methods for manipulating musical data, including pitch and rhythm conversions,
 * MusicXML node handling, similarity calculations, and more. All methods are designed for use in music research and computational musicology.
 */
class Helper {
   public:
    /**
     * @brief Splits a string into tokens using a specified delimiter.
     * @param s The input string.
     * @param delimiter The character used to split the string.
     * @return Vector of substrings.
     */
    static std::vector<std::string> splitString(const std::string& s, char delimiter);

    /**
     * @brief Formats a floating-point number as a string with a given number of decimal digits.
     * @param floatValue The value to format.
     * @param digits Number of decimal digits.
     * @return Formatted string.
     */
    static std::string formatFloat(float floatValue, int digits);

    /**
     * @brief Generates a string of spaces for indentation, useful for pretty-printing MusicXML.
     * @param identPosition Indentation level.
     * @param identSize Number of spaces per level (default: 2).
     * @return String with the requested indentation.
     */
    static const std::string generateIdentation(int identPosition, int identSize = 2);

    /**
     * @brief Converts a duration in ticks to a MusicXML note type and dot count.
     * @param durationTicks Duration in ticks.
     * @param divisionsPerQuarterNote Divisions per quarter note.
     * @param actualNotes Tuplet numerator (default: 1).
     * @param normalNotes Tuplet denominator (default: 1).
     * @return Pair of note type string and number of dots.
     */
    static std::pair<std::string, int> ticks2noteType(int durationTicks,
                                                      int divisionsPerQuarterNote,
                                                      int actualNotes = 1, int normalNotes = 1);

    /**
     * @brief Converts a duration in ticks to a RhythmFigure and dot count.
     * @param durationTicks Duration in ticks.
     * @param divisionsPerQuarterNote Divisions per quarter note.
     * @param actualNotes Tuplet numerator.
     * @param normalNotes Tuplet denominator.
     * @return Pair of RhythmFigure and number of dots.
     */
    static std::pair<RhythmFigure, int> ticks2rhythmFigure(int durationTicks,
                                                           int divisionsPerQuarterNote,
                                                           int actualNotes, int normalNotes);

    /**
     * @brief Converts a MusicXML note type string to a duration in ticks.
     * @param noteType Note type string (e.g., "quarter").
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     * @return Duration in ticks.
     */
    static int noteType2ticks(std::string noteType, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Converts a pitch string (e.g., "C4") to a floating-point number representation.
     * @param pitch Pitch string.
     * @return Numeric representation of the pitch.
     */
    static float pitch2number(const std::string& pitch);

    /**
     * @brief Converts a floating-point pitch number to a pitch string, with optional accidental type.
     * @param number Numeric pitch representation.
     * @param accType Accidental type (e.g., "#", "b").
     * @return Pitch string.
     */
    static const std::string number2pitch(const float number,
                                          const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    /**
     * @brief Converts a frequency in Hz to the closest MIDI note and cents deviation.
     * @param freq Frequency in Hz.
     * @param modelo Optional custom mapping function.
     * @return Pair of MIDI note number and cents deviation.
     */
    static std::pair<int, int> freq2midiNote(const float freq,
                                             std::function<int(float)> modelo = nullptr);

    /**
     * @brief Converts a MIDI note number to frequency in Hz.
     * @param midiNote MIDI note number.
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Frequency in Hz.
     */
    static float midiNote2freq(const int midiNote, const float freqA4 = 440.0f);

    /**
     * @brief Converts a MIDI note number to its octave number.
     * @param midiNote MIDI note number.
     * @return Octave number.
     */
    static int midiNote2octave(const int midiNote);

    /**
     * @brief Converts a pitch string (e.g., "C4") to a MIDI note number.
     * @param pitch Pitch string.
     * @return MIDI note number.
     */
    static int pitch2midiNote(const std::string& pitch);

    /**
     * @brief Converts a MIDI note number to a pitch string, with optional accidental type.
     * @param midiNote MIDI note number.
     * @param accType Accidental type (e.g., "#", "b").
     * @return Pitch string.
     */
    static const std::string midiNote2pitch(const int midiNote, const std::string& accType = {});

    /**
     * @brief Returns all possible pitch spellings for a given MIDI note.
     * @param midiNote MIDI note number.
     * @return Vector of pitch strings.
     */
    static const std::vector<std::string> midiNote2pitches(const int midiNote);

    /**
     * @brief Computes intervals between a sequence of notes.
     * @param notes Vector of Note objects.
     * @param firstNoteAsReference If true, intervals are from the first note to each subsequent note.
     * @return Vector of Interval objects.
     */
    static std::vector<Interval> notes2Intervals(const std::vector<Note>& notes,
                                                 const bool firstNoteAsReference = false);

    /**
     * @brief Computes intervals between a sequence of pitch strings.
     * @param pitches Vector of pitch strings.
     * @param firstNoteAsReference If true, intervals are from the first note to each subsequent note.
     * @return Vector of Interval objects.
     */
    static std::vector<Interval> notes2Intervals(const std::vector<std::string>& pitches,
                                                 const bool firstNoteAsReference = false);

    /**
     * @brief Calculates the number of semitones between two pitch strings.
     * @param pitch_A First pitch string.
     * @param pitch_B Second pitch string.
     * @return Number of semitones from pitch_A to pitch_B.
     */
    static int semitonesBetweenPitches(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Computes the similarity between two notes based on pitch class, octave, and duration.
     * @details Used for melodic pattern matching and musicological analysis.
     * @param pitchClass_A Pitch class of note A.
     * @param octave_A Octave of note A.
     * @param duration_A Duration of note A.
     * @param pitchClass_B Pitch class of note B.
     * @param octave_B Octave of note B.
     * @param duration_B Duration of note B.
     * @param durRatio Output: duration similarity ratio.
     * @param pitRatio Output: pitch similarity ratio.
     * @param enableEnharmonic If true, considers enharmonic equivalence.
     * @return Similarity value in [0,1], where 1 is identical.
     */
    static float noteSimilarity(std::string& pitchClass_A, int octave_A, const float duration_A,
                                std::string& pitchClass_B, int octave_B, const float duration_B,
                                float& durRatio, float& pitRatio,
                                const bool enableEnharmonic = false);

    /**
     * @brief Converts a pitch string to its frequency in Hz.
     * @param pitch Pitch string (e.g., "A4").
     * @return Frequency in Hz.
     */
    static float pitch2freq(const std::string& pitch);

    /**
     * @brief Converts a frequency ratio between two pitches to cents.
     * @param freq_A Frequency of the first pitch.
     * @param freq_B Frequency of the second pitch.
     * @return Interval in cents.
     */
    static int frequencies2cents(const float freq_A, const float freq_B);

    /**
     * @brief Converts a frequency to the nearest equal-tempered frequency.
     * @param freq Input frequency.
     * @param referenceFreq Reference frequency (default: 440.0 Hz).
     * @return Nearest equal-tempered frequency.
     */
    static float freq2equalTemperament(const float freq, const float referenceFreq = 440.0f);

    /**
     * @brief Converts a RhythmFigure enum to a MusicXML note type string.
     * @param rhythmFigure RhythmFigure value.
     * @return Note type string (e.g., "quarter").
     */
    static std::string rhythmFigure2noteType(const RhythmFigure rhythmFigure);

    /**
     * @brief Converts a RhythmFigure to a duration in ticks.
     * @param rhythmFigure RhythmFigure value.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     * @return Duration in ticks.
     */
    static int rhythmFigure2Ticks(const RhythmFigure rhythmFigure,
                                  const int divisionsPerQuarterNote = 256);

    /**
     * @brief Converts a MusicXML note type string to a RhythmFigure enum.
     * @param noteType Note type string (e.g., "quarter").
     * @return RhythmFigure value.
     */
    static RhythmFigure noteType2RhythmFigure(const std::string& noteType);

    /**
     * @brief Transposes a pitch string by a number of semitones.
     * @param pitch Input pitch string.
     * @param semitones Number of semitones to transpose.
     * @param accType Accidental type for output pitch (default: "#").
     * @return Transposed pitch string.
     */
    static const std::string transposePitch(
        const std::string& pitch, const int semitones,
        const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    /**
     * @brief Checks if two pitch strings are enharmonically equivalent.
     * @param pitch_A First pitch string.
     * @param pitch_B Second pitch string.
     * @return True if pitches are enharmonic.
     */
    static bool isEnharmonic(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Splits a pitch string into its components: pitch class, step, octave, accidental value, and symbol.
     * @param pitch Input pitch string.
     * @param pitchClass Output: pitch class (e.g., "C#", "Bb").
     * @param pitchStep Output: step (e.g., "C", "D").
     * @param octave Output: octave number.
     * @param alterValue Output: accidental value as float.
     * @param alterSymbol Output: accidental symbol (e.g., "#", "b").
     */
    static void splitPitch(const std::string& pitch, std::string& pitchClass,
                           std::string& pitchStep, int& octave, float& alterValue,
                           std::string& alterSymbol);

    /**
     * @brief Computes the ratio between two durations.
     * @param duration_A Duration of note A.
     * @param duration_B Duration of note B.
     * @return Ratio in [0,1], where 1 means identical durations.
     */
    static float durationRatio(float duration_A, float duration_B);

    // /**
    //  * @brief Converts an accidental RhythmFigure to a MusicXML note type string.
    //  * @param rhythmFigure RhythmFigure value.
    //  * @return Note type string.
    //  */
    // static std::string rhythmFigure2noteType(const RhythmFigure rhythmFigure);

    // /**
    //  * @brief Converts a MusicXML note type string to a RhythmFigure enum.
    //  * @param noteType Note type string.
    //  * @return RhythmFigure value.
    //  */
    // static RhythmFigure noteType2RhythmFigure(const std::string& noteType);

    /**
     * @brief Converts an accidental name (e.g., "sharp") to its symbol (e.g., "#").
     * @param alterName Accidental name.
     * @return Accidental symbol.
     */
    static const std::string alterName2symbol(const std::string& alterName);

    /**
     * @brief Converts an accidental symbol (e.g., "#") to its numeric value.
     * @param alterSymbol Accidental symbol.
     * @return Numeric value (e.g., 1.0 for "#").
     */
    static float alterSymbol2Value(const std::string& alterSymbol);

    /**
     * @brief Converts an accidental value to its symbol (e.g., 1.0 -> "#").
     * @param alterValue Accidental value.
     * @return Accidental symbol.
     */
    static const std::string alterValue2symbol(const float alterValue);

    /**
     * @brief Converts an accidental value to its name (e.g., 1.0 -> "sharp").
     * @param alterValue Accidental value.
     * @return Accidental name.
     */
    static const std::string alterValue2Name(const float alterValue);

    /**
     * @brief Retrieves detailed information from a MusicXML note node.
     * @param node XML node of the note.
     * @param pitch Output: pitch string.
     * @param pitchClass Output: pitch class.
     * @param alterSymbol Output: accidental symbol.
     * @param alterValue Output: accidental value.
     * @param octave Output: octave.
     * @param type Output: note type.
     * @param duration Output: duration.
     */
    static void getNoteNodeData(const pugi::xml_node& node, std::string& pitch,
                                std::string& pitchClass, std::string& alterSymbol, int& alterValue,
                                int& octave, std::string& type, float& duration);

    /**
     * @brief Selects nodes from a MusicXML document using an XPath expression.
     * @param doc XML document.
     * @param xPath XPath query string.
     * @return Set of matching XML nodes.
     */
    static const pugi::xpath_node_set getNodeSet(const pugi::xml_document& doc,
                                                 const std::string& xPath);

    /**
     * @brief Returns a JSON object with percentiles for a given table and desired percentile values.
     * @param table Input JSON table.
     * @param desiredPercentiles Vector of percentiles (0.0 to 1.0).
     * @return JSON object with percentile values.
     */
    static const nlohmann::json getPercentiles(const nlohmann::json& table,
                                               const std::vector<float>& desiredPercentiles);

    // /**
    //  * @brief Converts a pitch string to its frequency in Hz.
    //  * @param pitch Pitch string.
    //  * @return Frequency in Hz.
    //  */
    // static float pitch2freq(const std::string& pitch);

    /**
     * @brief Converts a frequency in Hz to the closest pitch and cents deviation.
     * @param freq Frequency in Hz.
     * @param accType Accidental type for output pitch.
     * @return Pair of pitch string and cents deviation.
     */
    static std::pair<std::string, int> freq2pitch(
        const float freq, const std::string& accType = MUSIC_XML::ACCIDENT::NONE);

    /**
     * @brief Computes the ratio between two pitch strings as a float.
     * @param pitch_A First pitch string.
     * @param pitch_B Second pitch string.
     * @return Ratio in [0,1].
     */
    static float pitchRatio(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Converts a RhythmFigure to a string.
     * @param rhythmFigure RhythmFigure value.
     * @return String representation.
     */
    static std::string toString(const RhythmFigure rhythmFigure);

    /**
     * @brief Computes the vector of semitone differences between two melodies.
     * @param referenceMelody Vector of reference notes.
     * @param otherMelody Vector of comparison notes.
     * @return Vector of semitone differences.
     */
    static std::vector<float> getSemitonesDifferenceBetweenMelodies(
        const std::vector<Note>& referenceMelody, const std::vector<Note>& otherMelody);

    /**
     * @brief Calculates the Euclidean similarity between two melodies based on semitone differences.
     * @param melodyPattern Reference melody.
     * @param otherMelody Comparison melody.
     * @return Similarity value in [0,1].
     */
    static float calculateMelodyEuclideanSimilarity(const std::vector<Note>& melodyPattern,
                                                    const std::vector<Note>& otherMelody);

    /**
     * @brief Calculates the Euclidean similarity from a vector of semitone differences.
     * @param semitonesDifference Vector of semitone differences.
     * @return Similarity value in [0,1].
     */
    static float calculateMelodyEuclideanSimilarity(const std::vector<float>& semitonesDifference);

    /**
     * @brief Computes the vector of normalized duration differences between two rhythms.
     * @param referenceRhythm Vector of reference notes.
     * @param otherRhythm Vector of comparison notes.
     * @return Vector of duration differences.
     */
    static std::vector<float> getDurationDifferenceBetweenRhythms(
        const std::vector<Note>& referenceRhythm, const std::vector<Note>& otherRhythm);

    /**
     * @brief Calculates the Euclidean similarity between two rhythms based on duration differences.
     * @param rhythmPattern Reference rhythm.
     * @param otherRhythm Comparison rhythm.
     * @return Similarity value in [0,1].
     */
    static float calculateRhythmicEuclideanSimilarity(const std::vector<Note>& rhythmPattern,
                                                      const std::vector<Note>& otherRhythm);

    /**
     * @brief Calculates the Euclidean similarity from a vector of duration differences.
     * @param durationDifferences Vector of duration differences.
     * @return Similarity value in [0,1].
     */
    static float calculateRhythmicEuclideanSimilarity(const std::vector<float>& durationDifferences);
};
#endif  // HELPERS_H
