#ifndef HELPERS_H
#define HELPERS_H

#include <math.h>

#include <optional>
#include <string>

#include "maiacore/constants.h"
#include "maiacore/note.h"
#include "nlohmann/json.hpp"
#include "pugi/pugixml.hpp"

class Interval;

/**
 * @brief Helper class with static utility functions for music analysis, pitch/duration conversion,
 * and MusicXML processing.
 *
 * This class provides a wide range of static methods for manipulating musical data, including pitch
 * and rhythm conversions, MusicXML node handling, similarity calculations, and more. All methods
 * are designed for use in music research and computational musicology.
 */
class Helper {
   public:
    /**
     * @brief Returns the maiacore library version.
     * @details Reads the version baked into the compiled library from the `MAIALIB_VERSION_INFO`
     *          macro (set by CMake from the repo-root `VERSION` file). Returns `"dev"` when the
     *          macro is undefined, e.g. a translation unit built outside the project's CMake setup.
     * @return Library version string (e.g. "1.10.3"), or "dev" if unavailable.
     */
    static std::string getLibraryVersion();

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
     * @return Octave number, or an empty optional if midiNote < 0 (a rest has no octave).
     */
    static std::optional<int> midiNote2octave(const int midiNote);

    /**
     * @brief Converts a pitch string (e.g., "C4") to a MIDI note number.
     * @details Computed as `12 * (octave + 1) + stepSemitone + alterValue`, so every spelling
     *          accepted by splitPitch() is supported (e.g. "C-1" = 0, "C4" = 60, "Bx11" = 157).
     * @param pitch Pitch string. An empty string or any string containing "rest" is a rest.
     * @return MIDI note number, or -1 (MUSIC_XML::MIDI::NUMBER::MIDI_REST) for a rest.
     * @throws std::runtime_error If the pitch string is invalid (see splitPitch()).
     */
    static int pitch2midiNote(const std::string& pitch);

    /**
     * @brief Computes the MIDI note number from already-parsed pitch spelling components.
     * @details Single implementation of `12 * (octave + 1) + stepSemitone + alterValue`, used by
     *          pitch2midiNote() and splitPitch() so the formula is evaluated in one place. Intended
     *          for callers (e.g. Note) that already parsed a pitch string with splitPitch() and
     * want to avoid rebuilding and re-parsing a pitch string just to get its MIDI number.
     *          A quarter-tone alterValue leaves the sum halfway between two MIDI numbers; it
     *          rounds to the upper one (roundTiesUpward(), utils.h).
     * @param pitchStep Diatonic step, one of "A".."G" (see splitPitch()).
     * @param alterValue Accidental value in semitones, from -2 to 2 (e.g., -2.0 for "bb").
     * @param octave Octave number, from -1 to 11.
     * @return MIDI note number.
     * @throws std::runtime_error If pitchStep is not a valid diatonic step, if alterValue is NaN,
     *         infinite or outside [-2, 2], or if octave is outside [-1, 11].
     */
    static int spelling2midiNote(const std::string& pitchStep, const float alterValue,
                                 const int octave);

    /**
     * @brief Converts a MIDI note number to a pitch string, with optional accidental type.
     * @param midiNote MIDI note number (negative values return "rest").
     * @param accType Accidental type: "" (natural for white keys, "#" for black keys), "#", "b",
     *        "x" or "bb".
     * @return Pitch string within octaves -1..11 (e.g., 5 -> "F-1"; 157 with "x" -> "Bx11").
     * @throws std::runtime_error If accType is unknown, if the MIDI note cannot be written with
     *         accType, or if the resulting octave falls outside -1..11.
     */
    static const std::string midiNote2pitch(const int midiNote, const std::string& accType = {});

    /**
     * @brief Converts an exact, unrounded pitch position in quarter-tone steps to a pitch string.
     * @details The fractional counterpart of midiNote2pitch(), and the single place a quarter tone
     *          is spelled from a numeric position. The position is split into the base semitone it
     *          rounds to (ties upward, the rule spelling2midiNote() owns) and the remaining
     *          quarter tone, which is always 0 or -0.5; the base semitone is spelled with
     *          midiNote2pitch() and the remainder is then folded into that spelling's accidental
     *          (e.g. 60.5 with accType "" gives "C1x4", since "C4" carries an alter of 0).
     *
     *          Defined for every float: a non-finite, off-grid or too-high position throws, a
     *          position below MIDI note 0 returns "rest", and only a position inside the
     *          representable range is ever converted to an int. The representable range runs from
     *          -0.5 ("C1b-1", the lowest position that still rounds, ties upward, to MIDI note 0)
     *          to Pitch::maxRepresentableMidi() (157, "Bx11").
     * @param exactSteps Exact pitch position in semitones (e.g. 60.5 for "C1x4"); must be finite
     *        and a multiple of 0.5. A position below MIDI note 0 -- one that rounds to a negative
     *        MIDI number, i.e. anything below -0.5 -- returns "rest", deliberately parallel to
     *        midiNote2pitch()'s contract for a negative MIDI number; -0.5 itself is "C1b-1".
     * @param accType Preferred accidental type for the BASE semitone: "", "#", "b", "x" or "bb".
     *        It is a preference, not a demand: when the requested type cannot absorb the quarter
     *        tone (a "bb" base is already at the -2 limit, so -2.5 has no spelling), the default
     *        spelling is used instead and a warning is logged.
     * @return Pitch string within octaves -1..11, or "rest" for a position below MIDI note 0.
     * @throws std::runtime_error If exactSteps is not finite (the message names the value and the
     *         representable range), is not a multiple of 0.5 (names the value), or lies above the
     *         representable range (names the value and the range). Also, from midiNote2pitch(),
     *         if the base semitone cannot be spelled with accType or its octave falls outside
     *         -1..11: e.g. 157 is only reachable as "Bx11", so it throws for any other accType.
     */
    static const std::string steps2pitch(const float exactSteps, const std::string& accType = {});

    /**
     * @brief Returns all possible pitch spellings for a given MIDI note.
     * @param midiNote MIDI note number.
     * @return Vector of pitch strings.
     */
    static const std::vector<std::string> midiNote2pitches(const int midiNote);

    /**
     * @brief Computes intervals between a sequence of notes.
     * @param notes Vector of Note objects.
     * @param firstNoteAsReference If true, intervals are from the first note to each subsequent
     * note.
     * @return Vector of Interval objects.
     */
    static std::vector<Interval> notes2Intervals(const std::vector<Note>& notes,
                                                 const bool firstNoteAsReference = false);

    /**
     * @brief Computes intervals between a sequence of pitch strings.
     * @param pitches Vector of pitch strings.
     * @param firstNoteAsReference If true, intervals are from the first note to each subsequent
     * note.
     * @return Vector of Interval objects.
     */
    static std::vector<Interval> notes2Intervals(const std::vector<std::string>& pitches,
                                                 const bool firstNoteAsReference = false);

    /**
     * @brief Computes multidimensional similarity between two notes using pitch-space and rhythmic
     * metrics.
     * @param pitchClass_A Pitch class of note A (e.g., "C", "F#", "Bb").
     * @param octave_A Octave register of note A (MIDI octave numbering: A4=440Hz).
     * @param duration_A Duration of note A in quarter-note units (1.0 = quarter, 0.5 = eighth).
     * @param pitchClass_B Pitch class of note B.
     * @param octave_B Octave register of note B.
     * @param duration_B Duration of note B in quarter-note units.
     * @param durRatio Output parameter: rhythmic similarity ratio in [0,1].
     * @param pitRatio Output parameter: pitch proximity ratio in [0,1].
     * @param enableEnharmonic If true, treats enharmonic equivalents as identical (C# ≡ Db).
     * @return Overall similarity score in [0,1] where 1.0 = perfect identity, 0.0 = maximal
     * dissimilarity.
     * @details Calculates composite similarity by combining pitch-space distance and rhythmic
     * congruence, enabling flexible note-matching for melodic pattern recognition, variation
     * analysis, and approximate music information retrieval.
     *
     *          **Similarity Components**:
     *          1. **Pitch Similarity (pitRatio)**:
     *             - Computed as inverse exponential decay function of semitone distance
     *             - Adjacent semitones (C→C#) → high similarity (≈0.9)
     *             - Octave displacement (C4→C5) → moderate similarity (≈0.6-0.7)
     *             - Large intervals (C4→G5) → low similarity (≈0.2-0.4)
     *             - When enableEnharmonic=true: C# and Db are treated as identical (distance=0)
     *
     *          2. **Rhythmic Similarity (durRatio)**:
     *             - Computed as inverse ratio of duration difference
     *             - Identical durations → durRatio = 1.0
     *             - Augmentation/diminution by factor of 2 (quarter vs. half) → durRatio ≈ 0.5
     *             - Large durational differences (whole vs. sixteenth) → durRatio → 0.0
     *
     *          3. **Combined Similarity (return value)**:
     *             - Weighted combination: similarity = α × pitRatio + β × durRatio
     *             - Default weighting emphasizes pitch over rhythm (typical: α=0.7, β=0.3)
     *
     *          **Applications**:
     *          - Melodic pattern matching with tolerance for ornamentation and metric variation
     *          - Variation analysis (comparing theme vs. variations in sonata/rondo forms)
     *          - Approximate music search (query-by-humming with pitch/rhythm flexibility)
     *          - Voice-leading analysis (tracking note continuity across harmonic changes)
     *          - Motivic analysis with rhythmic transformation (augmentation, diminution)
     *
     * @note This is a local (note-to-note) similarity metric. For global melodic similarity
     *       (entire phrase comparison), use calculateMelodyEuclideanSimilarity() instead.
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
     * @brief Validates that a transposition interval lands on the quarter-tone grid.
     * @details Transposition is defined on multiples of 0.5 semitones, because that is the finest
     *          interval this library can spell: there is no pitch between "C1x4" and "C#4" for a
     *          0.3-semitone transposition to land on. Shared by all four transposition entry
     *          points (Note::transpose(), Chord::transpose(), Chord::transposeStackOnly() and
     *          transposePitch()) so the rule and its message are stated once.
     * @param semitones Transposition interval in semitones.
     * @throws std::runtime_error If semitones is not finite (an infinity or NaN), or is finite but
     *         not a multiple of 0.5. Both messages name the offending value. The non-finite check
     *         is separate because an infinity passes the multiple-of-0.5 test (`inf * 2 == inf ==
     *         std::floor(inf)`), and would otherwise reach the spelling code — where `-inf`
     *         silently produced a rest.
     */
    static void validateTransposeSemitones(const float semitones);

    /**
     * @brief Transposes a pitch string by a number of semitones.
     * @details Computed on exact pitch positions (see steps2pitch()), so a quarter tone survives
     *          the transposition instead of being rounded away first: "C1x4" transposed by 2
     *          gives "D1x4". The interval is validated first and the pitch string parsed second,
     *          so an invalid argument is rejected whatever the other one is.
     *
     *          A real pitch never silently becomes a rest: a result outside the representable
     *          range, -0.5 ("C1b-1") to Pitch::maxRepresentableMidi() (157, "Bx11"), throws. This
     *          differs deliberately from steps2pitch(), whose rest sentinel below MIDI note 0
     *          exists for a sounding pitch computed from a transposing instrument.
     * @param pitch Input pitch string. A rest (an empty string or any string containing "rest")
     *        transposes to "rest".
     * @param semitones Number of semitones to transpose; must be finite and a multiple of 0.5
     *        (e.g. 0.5 for one quarter tone up, -2 for a whole tone down). 0 returns the pitch
     *        string unchanged.
     * @param accType Preferred accidental type for the result's base semitone (default: "#"); see
     *        steps2pitch().
     * @return Transposed pitch string.
     * @throws std::runtime_error If semitones is not finite or not a multiple of 0.5 (see
     *         validateTransposeSemitones()); if the pitch string is invalid; if a non-rest pitch
     *         transposes outside the representable range (the message names the pitch, the
     *         interval and the range); or if the result cannot be spelled with accType within
     *         octaves -1..11 (e.g. MIDI 0 with "#", which would be "B#-2").
     */
    static const std::string transposePitch(
        const std::string& pitch, const float semitones,
        const std::string& accType = MUSIC_XML::ACCIDENT::SHARP);

    /**
     * @brief Checks if two pitch strings are enharmonically equivalent.
     * @details Compares the exact, unrounded pitch positions of both pitches, so "E#4" and "F4",
     *          or "B#3" and "C4", are enharmonic, and so are the two spellings of a quarter tone
     *          ("C1x4" and "D3b4"). A quarter tone is NOT enharmonic with the semitone it rounds
     *          to: "C1x4" (60.5) and "C#4" (61) are different pitches. Two rests are considered
     *          enharmonic.
     * @param pitch_A First pitch string.
     * @param pitch_B Second pitch string.
     * @return True if both pitches denote the same exact pitch position.
     * @throws std::runtime_error If a pitch string is invalid (see splitPitch()).
     */
    static bool isEnharmonic(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Parses a pitch string into its components (the single pitch-string parser).
     * @details Accepted grammar: `step accidental? octave?`, where `step` is A-G, `accidental`
     *          is one of "bb", "b", "#", "x", and `octave` is an integer in [-1, 11]
     *          ("C-1" is MIDI 0). A missing octave defaults to 4. An empty string or any string
     *          containing "rest" yields the rest components ("rest", "rest", empty optional,
     *          0.0, ""): a rest has no octave.
     * @param pitch Input pitch string (e.g., "C4", "F#11", "Dbb-1", "Eb").
     * @param pitchClass Output: pitch class (e.g., "C#", "Bb").
     * @param pitchStep Output: diatonic step (e.g., "C", "D").
     * @param octave Output: octave number, left empty for a rest.
     * @param alterValue Output: accidental value in semitones (e.g., -2.0 for "bb").
     * @param alterSymbol Output: accidental symbol (e.g., "#", "b", or "" for natural).
     * @throws std::runtime_error If the step, accidental or octave is invalid, or if the pitch
     *         is below MIDI note 0 (e.g., "Cb-1").
     */
    static void splitPitch(const std::string& pitch, std::string& pitchClass,
                           std::string& pitchStep, std::optional<int>& octave, float& alterValue,
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
     * @brief Selects nodes from a MusicXML document using an XPath expression.
     * @param doc XML document.
     * @param xPath XPath query string.
     * @return Set of matching XML nodes.
     */
    static const pugi::xpath_node_set getNodeSet(const pugi::xml_document& doc,
                                                 const std::string& xPath);

    /**
     * @brief Returns a JSON object with percentiles for a given table and desired percentile
     * values.
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
     * @brief Computes the intervallic contour difference vector between two melodic sequences.
     * @param referenceMelody Vector of notes representing the reference melodic pattern.
     * @param otherMelody Vector of notes to compare against the reference melody.
     * @return Vector of signed semitone differences (positive = upward transposition, negative =
     * downward).
     * @details Calculates the interval-by-interval pitch displacement between two melodies of equal
     * length, producing a difference vector that quantifies melodic transposition, contour
     * divergence, and pitch-space transformation. This function is fundamental for melodic
     * similarity analysis, thematic variation studies, and computational pattern matching.
     *
     *          **Computation Process**:
     *          For each aligned note pair (referenceMelody[i], otherMelody[i]):
     *          - Compute signed semitone distance: otherMelody[i].pitch - referenceMelody[i].pitch
     *          - Positive values indicate upward transposition (otherMelody higher than reference)
     *          - Negative values indicate downward transposition (otherMelody lower than reference)
     *          - Zero values indicate exact pitch match at that position
     *
     *          **Example**:
     *          \code
     *          referenceMelody: C4-E4-G4 (semitone sequence: 60-64-67)
     *          otherMelody:     D4-F#4-A4 (semitone sequence: 62-66-69)
     *          Difference vector: [+2, +2, +2] → uniform transposition up by major second
     *          \endcode
     *
     *          **Interpretation of Results**:
     *          - **Constant difference vector** (e.g., [+5, +5, +5]): Exact transposition
     *          - **Zero-centered fluctuations** (e.g., [-1, 0, +1]): Approximate contour match with
     * chromatic variation
     *          - **Large absolute values** (e.g., [+12, +7, -3]): Significant contour divergence,
     * octave displacements
     *          - **Alternating signs**: Contour inversion or melodic inversion transformation
     *
     *          **Applications**:
     *          - Melodic variation analysis (identifying theme-and-variation relationships)
     *          - Fugue analysis (detecting transposed subject entries at different pitch levels)
     *          - Melodic similarity scoring (input to calculateMelodyEuclideanSimilarity)
     *          - Contour analysis (studying intervallic contour preservation vs. transformation)
     *          - Computational musicology (corpus-wide melodic relationship detection)
     *
     * @note Both melodies must have the same length. If lengths differ, the function processes
     *       min(referenceMelody.size(), otherMelody.size()) notes and ignores excess notes.
     *
     * @warning This function compares absolute pitch, not pitch-class. C4 and C5 differ by 12
     * semitones. For pitch-class comparison (octave-invariant), reduce results modulo 12.
     */
    static std::vector<float> getSemitonesDifferenceBetweenMelodies(
        const std::vector<Note>& referenceMelody, const std::vector<Note>& otherMelody);

    /**
     * @brief Calculates Euclidean distance-based melodic similarity from intervallic contour
     * comparison.
     * @param melodyPattern Reference melodic pattern to match against.
     * @param otherMelody Candidate melody to compare for similarity.
     * @return Normalized similarity score in [0,1] where 1.0 = identical contour, 0.0 = maximal
     * divergence.
     * @details Computes global melodic similarity by measuring the Euclidean distance in
     * pitch-space between two melodic sequences, accounting for transposition, intervallic
     * distortion, and contour preservation. This metric is transposition-sensitive, meaning exact
     * transpositions yield high similarity while contour-preserving transformations with interval
     * alterations yield moderate similarity.
     *
     *          **Computation Process**:
     *          1. Extract semitone difference vector:
     * getSemitonesDifferenceBetweenMelodies(melodyPattern, otherMelody)
     *          2. Compute Euclidean distance: sqrt(Σ(difference[i]²))
     *          3. Normalize to [0,1] similarity: similarity = 1 / (1 + distance/scaling_factor)
     *
     *          **Interpretation of Results**:
     *          - **similarity ≈ 1.0** (0.95-1.0): Near-identical melodies, possibly exact
     * transposition
     *          - **similarity ≈ 0.7-0.9**: High similarity with minor intervallic variations
     *            (e.g., chromatic alterations, octave displacements in 1-2 notes)
     *          - **similarity ≈ 0.4-0.7**: Moderate similarity, recognizable contour with
     * significant transformations (e.g., modal transposition, rhythmic variation, ornamentation)
     *          - **similarity < 0.4**: Low similarity, different melodic material or inverted
     * contours
     *
     *          **Transposition Invariance**:
     *          This metric is NOT fully transposition-invariant. Melodies transposed by a constant
     * interval will have high but not perfect similarity (distance proportional to transposition
     * distance). For exact transposition-invariance, compute interval sequence differences instead.
     *
     *          **Applications**:
     *          - Melodic pattern matching in thematic analysis (identifying motivic recurrence)
     *          - Theme-and-variation studies (quantifying melodic transformation degree)
     *          - Plagiarism detection (measuring melodic borrowing/paraphrase)
     *          - Music information retrieval (query-by-example melodic search)
     *          - Corpus analysis (clustering melodies by contour similarity)
     *
     * @note Both melodies must have the same length for meaningful comparison. If lengths differ,
     *       the function processes min(melodyPattern.size(), otherMelody.size()) notes.
     *
     * @warning This metric emphasizes pitch contour over rhythmic structure. For combined
     * pitch+rhythm similarity, use findMelodyPattern() with custom similarity callbacks.
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
    static float calculateRhythmicEuclideanSimilarity(
        const std::vector<float>& durationDifferences);
};
#endif  // HELPERS_H
