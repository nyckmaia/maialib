#pragma once

#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "SQLiteCpp/SQLiteCpp.h"
#include "maiacore/chord.h"
#include "maiacore/constants.h"
#include "maiacore/import-issue.h"
#include "maiacore/key.h"
#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/part.h"
#include "nlohmann/json.hpp"
#include "pugi/pugixml.hpp"

/**
 * @brief Represents a complete musical score, including metadata, parts, measures, and notes.
 *
 * The Score class provides methods for creating, loading, editing, analyzing, and exporting musical
 * scores. It supports MusicXML import/export, part and measure management, note access, metadata
 * handling, and advanced musicological analysis such as melodic pattern search and chord
 * extraction.
 */
class Score {
   private:
    std::string _title;         ///< Title of the score.
    std::string _composerName;  ///< Composer's name.
    std::vector<Part> _part;    ///< List of parts (instruments/voices).
    std::string _filePath;      ///< Path to the loaded MusicXML file.
    std::string _fileName;      ///< Name of the loaded MusicXML file.

    pugi::xml_document _doc;            ///< Internal XML document representation.
    int _numParts;                      ///< Number of parts in the score.
    int _numMeasures;                   ///< Number of measures in the score.
    int _numNotes;                      ///< Number of notes in the score.
    bool _isValidXML;                   ///< True if the XML was loaded and parsed successfully.
    bool _haveTypeTag;                  ///< True if the MusicXML contains <type> tags for notes.
    bool _isLoadedXML;                  ///< True if the score was loaded from a file.
    std::vector<Chord> _stackedChords;  ///< Cached vertical chords.
    int _lcmDivisionsPerQuarterNote;  ///< Least common multiple of all 'divisions' tags in the XML
                                      ///< file.
    bool _haveAnacrusisMeasure;       ///< True if the score contains an anacrusis (pickup) measure.
    std::vector<ImportIssue> _importIssues;  ///< The import report of the loaded file.

    typedef struct noteData_st {
        float currentTimeValue = 0.0f;
        const Note* notePtr = nullptr;
        float floatBeatStartTime = 0.0f;
        float floatBeatEndTime = 0.0f;
    } NoteData;

    typedef struct chordData_st {
        std::vector<NoteData> noteData;
        const Measure* measurePtr = nullptr;
        bool isHomophonicChord = true;
        float beatEndTimeHigherLimit = 0.0f;
        float beatStartTimeLowerLimit = 1000.0f;
        float chordQuarterDuration = 0.0f;
    } ChordData;

    /**
     * @brief Loads a MusicXML file (*.xml, *.musicxml, *.mxl) into the Score object.
     * @details Parses the XML, extracts metadata, parts, measures, and notes, and fills internal
     * structures.
     * @param filePath Path to the MusicXML file (absolute or relative).
     */
    void loadXMLFile(const std::string& filePath);

    /**
     * @brief Extracts vertical chords for each note event using an in-memory SQLite database.
     * @param db SQLite database with note events.
     * @param includeDuplicates If true, duplicate notes are included in chords.
     * @return Vector of tuples: {measure, floatMeasure, Key, Chord, isHomophonic}.
     */
    std::vector<std::tuple<int, float, Key, Chord, bool>> getChordsPerEachNoteEvent(
        SQLite::Database& db, const bool includeDuplicates);

   public:
    /**
     * @brief Constructs a new blank Score object with specified part names and initial measure
     * count.
     * @param partsName List of instrument/part names.
     * @param numMeasures Initial number of measures (default: 20).
     */
    explicit Score(const std::initializer_list<std::string>& partsName, const int numMeasures = 20);

    /**
     * @brief Constructs a new blank Score object with specified part names and initial measure
     * count.
     * @param partsName Vector of instrument/part names.
     * @param numMeasures Initial number of measures (default: 20).
     */
    explicit Score(const std::vector<std::string>& partsName, const int numMeasures = 20);

    /**
     * @brief Constructs a new Score object by loading a MusicXML file.
     * @details Supported formats: *.xml, *.musicxml, *.mxl (compressed).
     *
     *          A note's accidental is read from its `<accidental>` element first, then from the
     *          decimal `<alter>` value, and is natural otherwise. `<alter>` is parsed with a '.'
     *          decimal point whatever the C or C++ locale, and must be exactly one of the nine
     *          alters this library can spell (a multiple of 0.5 from -2 to 2). An `<accidental>`
     *          name this library cannot spell falls back to `<alter>` (record
     *          ACCIDENTAL_NAME_UNKNOWN); an `<alter>` it cannot spell (3, the eighth tone 0.25, or
     *          0.46, near a quarter tone but not one) leaves the note natural, never rounded to
     *          the nearest pitch (ALTER_OFF_GRID). When a recognised `<accidental>` and the
     *          `<alter>` disagree, the `<accidental>` is used (ACCIDENTAL_ALTER_MISMATCH). None of
     *          these aborts the load.
     *
     *          Every value the reader corrects is a record of the import report
     *          (getImportIssues()), with its part, its measure, the element, the value found and
     *          the value used. When the report is not empty, the load prints one line:
     *          "[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on
     *          export); see Score.getImportIssues()". Text taken from the file reaches a record,
     *          a message or the console as valid UTF-8: a byte that is not is replaced by U+FFFD.
     *
     *          A `<transpose>` is read in every measure. It applies to the pitched notes written
     *          after it, in its measure and the following ones, on the staff its `number` names or,
     *          without `number`, on every staff of the part, until the next `<transpose>` for that
     *          staff; inside a measure document order decides, and a chord is read as a unit: a
     *          `<transpose>` between the notes of a chord applies from the first note after the
     *          chord. Each such note is given the interval with `<octave-change>` folded in, 7
     *          letters and 12 semitones per octave (a B-flat bass clarinet's -1, -2 and -1 make
     *          (-8, -14)), and the octave doubling of `<double>` (Note::getOctaveDoubling()); rests
     *          and unpitched notes are left alone. A `<transpose>` without `<diatonic>` is given
     *          the conventional diatonic interval of its `<chromatic>` one (see
     *          Note::getSoundingPitch()). One whose `<chromatic>` or `<octave-change>` is not a
     *          whole number is ignored, leaving the previous transposition in force (records
     *          TRANSPOSE_CHROMATIC_NOT_INTEGER and TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER). A
     *          `<diatonic>` that does not match `<chromatic>` is replaced by the conventional
     *          diatonic interval, so that nothing sounds different (TRANSPOSE_PAIR_CORRECTED); for
     *          a tritone both the augmented fourth and the diminished fifth match, and an explicit
     *          0 with a non-zero `<chromatic>` does not. A `<transpose>` with which a note of its
     *          scope -- the notes it would apply to, up to the next `<transpose>` for their staff
     *          -- would have no sounding pitch (below C1b-1, or above B11 where its letter cannot
     *          spell it) is ignored for its whole scope (TRANSPOSE_OUT_OF_RANGE); there the
     *          previous transposition stays in force, and a chord with a note it cannot sound
     *          either is read untransposed. `<for-part>` is not modelled: it is dropped
     *          (FOR_PART_NOT_MODELLED).
     *
     *          The reader also records these corrections: parts that share a name
     *          are told apart by a suffix, " 1", " 2", ... (PART_NAME_DUPLICATE); a part whose
     *          first measure has no `<divisions>` is read at 256 divisions per quarter note
     *          (DIVISIONS_MISSING); a `<voice>` or a `<staff>` that is not a positive whole number
     *          is read as voice 1 or the first staff, or as the digits its text starts with
     *          (VOICE_NOT_POSITIVE, STAFF_CLAMPED); an `<actual-notes>` or `<normal-notes>` of a
     *          `<time-modification>` that is not a positive whole number is read as 1
     *          (TUPLET_CLAMPED).
     * @param filePath Path to the MusicXML file.
     */
    explicit Score(const std::string& filePath);

    /**
     * @brief Move constructor for Score.
     */
    Score(Score&&) = default;

    /**
     * @brief Destructor. Releases resources associated with the score.
     */
    ~Score();

    /**
     * @brief Clears all content from the score, removing parts, measures, and metadata.
     * @details Useful for reusing the Score object without creating a new one.
     */
    void clear();

    /**
     * @brief Adds a new part (instrument/voice) to the score.
     * @details Optionally specify the number of staves for the part.
     * @param partName Name of the part/instrument.
     * @param numStaves Number of staves (default: 1).
     */
    void addPart(const std::string& partName, const int numStaves = 1);

    /**
     * @brief Removes a part from the score by its index.
     * @param partId Index of the part to remove.
     */
    void removePart(const int partId);

    /**
     * @brief Adds a specified number of measures to all parts in the score.
     * @param numMeasures Number of measures to add.
     */
    void addMeasure(const int numMeasures);

    /**
     * @brief Removes a range of measures from all parts in the score.
     * @param measureStart Index of the first measure to remove.
     * @param measureEnd Index of the last measure to remove.
     */
    void removeMeasure(const int measureStart, const int measureEnd);

    /**
     * @brief Returns a reference to a part by its index.
     * @param partId Index of the part.
     * @return Reference to the Part object.
     */
    Part& getPart(const int partId);

    /**
     * @brief Returns a reference to a part by its name.
     * @param partName Name of the part.
     * @return Reference to the Part object.
     */
    Part& getPart(const std::string& partName);

    /**
     * @brief Returns the number of parts (instruments/voices) in the score.
     * @return Number of parts.
     */
    int getNumParts() const;

    /**
     * @brief Returns the number of measures in the score.
     * @return Number of measures.
     */
    int getNumMeasures() const;

    /**
     * @brief Returns the total number of notes in the score.
     * @return Number of notes.
     */
    int getNumNotes() const;

    /**
     * @brief Returns a vector with the names of all parts/instruments.
     * @return Vector of part names.
     */
    const std::vector<std::string> getPartsNames() const;

    /**
     * @brief Returns the title of the score.
     * @return Title string.
     */
    std::string getTitle() const;

    /**
     * @brief Sets the title of the score.
     * @param scoreTitle New title.
     */
    void setTitle(const std::string& scoreTitle);

    /**
     * @brief Returns the composer's name.
     * @return Composer name string.
     */
    std::string getComposerName() const;

    /**
     * @brief Sets the composer's name.
     * @param composerName New composer name.
     */
    void setComposerName(const std::string& composerName);

    /**
     * @brief Sets the key signature for all parts at a specific measure.
     * @details Specify the number of accidentals (fifths) and mode (major/minor).
     * @param fifthCicle Number of accidentals in the circle of fifths.
     * @param isMajorMode True for major, false for minor.
     * @param measureId Measure index (default: 0).
     */
    void setKeySignature(const int fifthCicle, const bool isMajorMode = true,
                         const int measureId = 0);

    /**
     * @brief Sets the key signature using the key name (e.g., "C", "Gm").
     * @param key Key name.
     * @param measureId Measure index (default: 0).
     */
    void setKeySignature(const std::string& key, const int measureId = 0);

    /**
     * @brief Sets the time signature for all or part of the measures.
     * @details Can be applied to all measures or from a specific measure onward.
     * @param timeUpper Numerator of the time signature.
     * @param timeLower Denominator of the time signature.
     * @param measureId Starting measure index (-1 for all).
     */
    void setTimeSignature(const int timeUpper, const int timeLower, const int measureId = -1);

    /**
     * @brief Sets the metronome mark (BPM) for a specific measure.
     * @param bpm Beats per minute.
     * @param duration Rhythm figure associated with the BPM (default: QUARTER).
     * @param measureStart Starting measure (default: 0).
     */
    void setMetronomeMark(int bpm, const RhythmFigure duration = RhythmFigure::QUARTER,
                          int measureStart = 0);

    /**
     * @brief Exports the score to MusicXML format.
     * @details Generates a complete MusicXML string, including metadata and all parts. Each
     *          part's transpositions are written as `<transpose>` elements (see Part::toXML()).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string.
     * @throws std::runtime_error If a part cannot be written (see Part::toXML()).
     */
    const std::string toXML(const int identSize = 2) const;

    /**
     * @brief Exports the score to JSON format.
     * @details Useful for integration with analysis and visualization tools.
     * @return JSON string representing the score.
     */
    const std::string toJSON() const;

    /**
     * @brief Saves the score to a file in XML or compressed MXL format.
     * @param fileName Output file name.
     * @param compressedXML True to save as .mxl (compressed).
     * @param identSize Indentation size (default: 2).
     * @throws std::runtime_error If fileName is empty, the file cannot be opened, or a part
     *         cannot be written (see Part::toXML()).
     */
    void toFile(std::string fileName, bool compressedXML = false, const int identSize = 2) const;

    /**
     * @brief Prints summary information about the score to the log.
     * @details Includes title, composer, key, time signature, note count, measure count, and part
     * names.
     */
    void info() const;

    /**
     * @brief Iterates over all notes in the score, applying a callback function.
     * @details Supports optional filtering by measure range and part names.
     * @param callback Function to call for each note.
     * @param measureStart Starting measure (default: 0).
     * @param measureEnd Ending measure (default: -1, until the end).
     * @param partNames List of part names to process (default: all).
     */
    void forEachNote(
        std::function<void(Part* part, Measure* measure, int staveId, Note* note)> callback,
        int measureStart = 0, int measureEnd = -1, std::vector<std::string> partNames = {});

    /**
     * @brief Returns true if the score is valid (MusicXML loaded correctly).
     * @return True if valid.
     */
    bool isValid(void) const;

    /**
     * @brief Returns the file path of the loaded MusicXML file.
     * @return File path string.
     */
    std::string getFilePath() const;

    /**
     * @brief Returns the file name of the loaded MusicXML file.
     * @return File name string.
     */
    std::string getFileName() const;

    /**
     * @brief Returns the import report: what the reader corrected or dropped while loading the
     *        file, in the order it did.
     * @details The report describes the load. It is copied with the score, emptied by clear(),
     *          and left unchanged by edits and exports; a score built through the API has an empty
     *          report.
     * @return The records, one ImportIssue each.
     */
    const std::vector<ImportIssue>& getImportIssues() const;

    /**
     * @brief Returns true if the MusicXML file contains <type> tags for notes.
     * @return True if <type> tags are present.
     */
    bool haveTypeTag(void) const;

    /**
     * @brief Prints the names of all parts/instruments in the score to the terminal.
     */
    void printPartNames() const;

    /**
     * @brief Counts the number of XML nodes matching a given XPath expression.
     * @param xPath XPath expression.
     * @return Number of nodes found.
     */
    int xPathCountNodes(const std::string& xPath) const;

    /**
     * @brief Returns the name of a part by its index.
     * @param partId Part index.
     * @return Part name string.
     */
    const std::string getPartName(const int partId) const;

    /**
     * @brief Gets the index of a part by its name.
     * @param partName Name of the part.
     * @param index Output: index integer.
     * @return True if the part was found.
     */
    bool getPartIndex(const std::string& partName, int* index) const;

    /**
     * @brief Returns true if the score contains an anacrusis (pickup) measure.
     * @return True if anacrusis is present.
     */
    bool haveAnacrusisMeasure() const;

    /**
     * @brief Sets repeat barlines for a range of measures.
     * @param measureStart Starting measure index.
     * @param measureEnd Ending measure index (-1 for last measure).
     */
    void setRepeat(int measureStart, int measureEnd = -1);

    /**
     * @brief Analyzes instrumental fragmentation patterns across the score timeline.
     * @param config JSON configuration object specifying analysis parameters (parts, measures,
     * etc).
     * @return JSON structure with activation, fragmentation, and execution metrics per instrument.
     * @details Performs temporal analysis of instrumental activity distribution, quantifying how
     *          melodic lines are fragmented across different instruments over time. This analysis
     *          is essential for orchestration studies, texture evolution tracking, and
     * compositional strategy research.
     *
     *          **Fragmentation Metrics Computed**:
     *          1. **Activation Lines**: Temporal intervals where each instrument is active
     * (sounding notes)
     *          2. **Fragmentation Patterns**: Transitions of melodic material between instruments
     *             (e.g., melody alternating between violin and flute—klangfarbenmelodie analysis)
     *          3. **Execution Density**: Proportion of time each instrument participates in texture
     *
     *          **Analysis Categories**:
     *          - **Continuous Execution**: Sustained melodic lines without rests (legato passages)
     *          - **Fragmented Execution**: Interrupted melodic lines with rests (staccato,
     * pointillism)
     *          - **Hand-off Patterns**: Melodic continuation across instrument changes (orchestral
     * dialogue)
     *          - **Tutti vs. Solo Distributions**: Ensemble density fluctuations
     *
     *          **Configuration Parameters** (config JSON):
     *          - `partNames` (list): Restrict analysis to specific instrumental parts
     *          - `measureStart`, `measureEnd` (int): Define temporal analysis window
     *          - `timeResolution` (float): Granularity for temporal slicing (e.g., 0.25 = sixteenth
     * note)
     *
     *          **Return JSON Structure**:
     *          \code{.json}
     *          {
     *            "instruments": [
     *              {
     *                "name": "Violin I",
     *                "activationIntervals": [[1.0, 4.5], [8.0, 12.5]],
     *                "fragmentationCount": 3,
     *                "executionDensity": 0.67
     *              },
     *              ...
     *            ],
     *            "fragmentationEvents": [
     *              {"measure": 4, "fromInstrument": "Violin I", "toInstrument": "Flute"},
     *              ...
     *            ]
     *          }
     *          \endcode
     *
     *          **Applications**:
     *          - Orchestration analysis (instrument usage patterns in Mahler, Ravel, Stravinsky)
     *          - Klangfarbenmelodie detection (Schoenberg, Webern timbral melody techniques)
     *          - Texture evolution studies (gradual thickening/thinning of orchestral fabric)
     *          - Compositional fingerprinting (characteristic orchestration strategies per
     * composer)
     *          - Performance part difficulty assessment (rest distribution, endurance requirements)
     *
     * @note This function is computationally intensive for large orchestral scores.
     *       Consider restricting analysis to specific measure ranges or part subsets.
     */
    nlohmann::json instrumentFragmentation(nlohmann::json config = nlohmann::json());

    // /**
    //  * @brief Returns the vertical (stacked) chords of the score, with configurable filters.
    //  * @details Extracts chords considering parts, measures, min/max duration, duplicates, etc.
    //  * @param config JSON object with configuration parameters.
    //  * @return Vector of tuples: {measure number, time, key, chord, homophony}.
    //  */
    // std::vector<std::tuple<int, float, Key, Chord, bool>> getChords(nlohmann::json config = {});

    /**
     * @brief Copy constructor for Score.
     * @param other Score to copy.
     */
    Score(const Score& other) {
        _title = other._title;
        _composerName = other._composerName;
        _filePath = other._filePath;
        _fileName = other._fileName;
        _part = other._part;
        _numParts = other._numParts;
        _numMeasures = other._numMeasures;
        _numNotes = other._numNotes;
        _isValidXML = other._isValidXML;
        _haveTypeTag = other._haveTypeTag;
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
        _stackedChords = other._stackedChords;
        _haveAnacrusisMeasure = other._haveAnacrusisMeasure;
        _importIssues = other._importIssues;

        // Deep copy of XML document
        _doc.reset(other._doc);
    }

    /**
     * @brief Assignment operator for Score.
     * @param other Score to assign from.
     * @return Reference to this Score.
     */
    Score& operator=(const Score& other) {
        if (this == &other) return *this;

        _title = other._title;
        _composerName = other._composerName;
        _filePath = other._filePath;
        _fileName = other._fileName;
        _part = other._part;
        _numParts = other._numParts;
        _numMeasures = other._numMeasures;
        _numNotes = other._numNotes;
        _isValidXML = other._isValidXML;
        _haveTypeTag = other._haveTypeTag;
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
        _stackedChords = other._stackedChords;
        _haveAnacrusisMeasure = other._haveAnacrusisMeasure;
        _importIssues = other._importIssues;

        // Deep copy of XML document
        _doc.reset(other._doc);

        return *this;
    }

    // ====== MELODIC PATTERN ANALYSIS ======

    /**
     * @brief One match of a melodic pattern: a window of a melodic line of the score.
     * @details The melody search reads every voice of every staff of every part as a melodic
     *          line of events -- a note, a chord by its highest sounding note, or a rest; a
     *          tied note extends the event it is tied to -- and compares the pattern with every
     *          window of as many consecutive events of one line (see findMelodyPattern()).
     */
    struct MelodyPatternRow {
        std::string partName;    ///< The part.
        int measure = 0;         ///< 0-based index of the measure of the window's first event.
        int staff = 0;           ///< 0-based staff of the line.
        int voice = 0;           ///< Voice of the line, as written.
        std::string writtenKey;  ///< The part's written key at that measure (Key::getName()).
        /// The score's concert key at that measure, by the rule of getChords().
        std::string concertKey;
        /// The interval from the pattern's first sounding note to the window's, named at concert
        /// spelling with its direction ("M2 asc", "P1"); empty when it has no name or when the
        /// pattern or the window has no sounding note.
        std::string transposeInterval;
        /// That interval in exact semitones (quarter tones included); NaN when the pattern or
        /// the window has no sounding note.
        float transposeSemitones = 0.0f;
        std::vector<std::string> writtenPitches;   ///< The window's written pitches ("rest").
        std::vector<std::string> soundingPitches;  ///< The window's sounding pitches ("rest").
        std::vector<float> semitonesDiff;          ///< Per-interval differences (pattern size - 1).
        std::vector<float> rhythmDiff;             ///< Per-duration differences (pattern size).
        float intervalSimilarity = 0.0f;           ///< The interval similarity.
        float rhythmSimilarity = 0.0f;             ///< The rhythm similarity.
        float totalSimilarity = 0.0f;              ///< The combined similarity.

        /**
         * @brief Field-by-field equality; a NaN transposeSemitones equals no value, NaN included.
         */
        bool operator==(const MelodyPatternRow& other) const {
            return std::tie(partName, measure, staff, voice, writtenKey, concertKey,
                            transposeInterval, transposeSemitones, writtenPitches, soundingPitches,
                            semitonesDiff, rhythmDiff, intervalSimilarity, rhythmSimilarity,
                            totalSimilarity) ==
                   std::tie(other.partName, other.measure, other.staff, other.voice,
                            other.writtenKey, other.concertKey, other.transposeInterval,
                            other.transposeSemitones, other.writtenPitches, other.soundingPitches,
                            other.semitonesDiff, other.rhythmDiff, other.intervalSimilarity,
                            other.rhythmSimilarity, other.totalSimilarity);
        }
    };

    /**
     * @brief The matches of one pattern, sorted stably by measure: matches of one measure keep
     *        the order of their lines (part, staff, voice) and windows.
     */
    typedef std::vector<MelodyPatternRow> MelodyPatternTable;

    /**
     * @brief A pattern that findAnyMelodyPattern() found in the score, with its matches.
     */
    struct FoundMelodyPattern {
        std::vector<Note> pattern;   ///< The window's events, as the search compares them.
        MelodyPatternTable matches;  ///< Its matches, as findMelodyPattern() returns them.
    };

    /**
     * @brief Searches every melodic line of the score for a melodic pattern.
     * @param melodyPattern The pattern: at least 2 notes; rests are allowed.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match (0.0-1.0).
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match (0.0-1.0).
     * @param intervalsSimilarityCallback Replaces the interval differences
     *        (Helper::getSemitonesDifferenceBetweenMelodies()); give
     *        totalIntervalSimilarityCallback with it.
     * @param rhythmSimilarityCallback Replaces the duration differences
     *        (Helper::getDurationDifferenceBetweenRhythms()); give totalRhythmSimilarityCallback
     *        with it.
     * @param totalIntervalSimilarityCallback Reduces the interval differences to a similarity;
     *        used only with intervalsSimilarityCallback.
     * @param totalRhythmSimilarityCallback Reduces the duration differences to a similarity;
     *        used only with rhythmSimilarityCallback.
     * @param totalSimilarityCallback Combines the two similarities; the default is their mean.
     * @return One row per match, sorted stably by measure: rows of one measure keep the order of
     *         their lines (part, staff, voice) and windows.
     * @throws std::runtime_error If the pattern has fewer than 2 notes; the message names the
     *         method and the length.
     * @throws std::bad_function_call If intervalsSimilarityCallback or rhythmSimilarityCallback
     *         is given without its total callback.
     * @details **Melodic lines.** Every voice that occurs on a staff of a part is a line: its
     *          events in measure order. A note without `<chord/>` starts an event, and the chord
     *          notes written after it join it; the event stands for its highest note by sounding
     *          exact position. A note tied to the previous event of its line at the same sounding
     *          position extends that event, which keeps its first note's written pitch and
     *          measure and adds the durations. A rest is an event; a grace note is not.
     *
     *          **Windows.** Every window of as many consecutive events of one line as the pattern
     *          has notes, the last one included; no window spans two lines. A pattern longer
     *          than every line finds nothing.
     *
     *          **Comparison.** The melodic intervals of the pattern and of the window are
     *          compared at sounding exact positions, so the comparison is transposition-invariant
     *          and quarter tones count as half semitones; an interval to or from a rest is 0.
     *          Durations are divided by each sequence's longest. Similarity is 1 / (1 + the
     *          Euclidean norm of the differences) unless a callback replaces it, and a window
     *          matches when both similarities reach their thresholds.
     *
     *          **Transposition.** For a match only, transposeSemitones and transposeInterval
     *          relate the pattern's first sounding note to the window's; an interval without a
     *          name (an augmented ninth C4 -> Cx5, any quarter-tone interval) leaves
     *          transposeInterval empty and never stops the search.
     * @note Each call builds the melodic lines from the score as it is. The search reads the
     *       score and writes nothing another thread shares, so any number of threads may search
     *       the same score at once. Modifying the score, its parts, measures or notes while a
     *       search runs is not safe.
     */
    MelodyPatternTable findMelodyPattern(
        const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold = 0.5,
        const float rhythmSimilarityThreshold = 0.5,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;

    /**
     * @brief Searches the score for several melodic patterns, returning a table for each
     * pattern.
     * @details Builds the melodic lines once and searches each pattern on a worker thread, as
     *          the single-pattern overload searches it.
     * @param melodyPatterns The patterns.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match.
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match.
     * @param intervalsSimilarityCallback See the single-pattern overload.
     * @param rhythmSimilarityCallback See the single-pattern overload.
     * @param totalIntervalSimilarityCallback See the single-pattern overload.
     * @param totalRhythmSimilarityCallback See the single-pattern overload.
     * @param totalSimilarityCallback See the single-pattern overload.
     * @return One table per pattern, in pattern order.
     * @throws std::runtime_error Or std::bad_function_call, as the single-pattern overload throws
     *         them, for any pattern: the first such exception, in pattern order, is rethrown once
     *         every pattern has been searched.
     * @note Any number of threads may search the same score at once. Modifying the score, its
     * parts, measures or notes while a search runs is not safe.
     */
    std::vector<MelodyPatternTable> findMelodyPattern(
        const std::vector<std::vector<Note>>& melodyPatterns,
        const float intervalSimilarityThreshold = 0.5, const float rhythmSimilarityThreshold = 0.5,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;

    /**
     * @brief Finds every distinct melodic pattern of a given length in the score, with its
     *        matches.
     * @details Every window of patternNumNotes events of a melodic line (see
     *          findMelodyPattern()) is a pattern. Two windows are the same pattern when their
     *          events are the same notes and rests at the same exact positions relative to their
     *          first sounding note (quarter tones included) and have the same durations, exactly;
     *          of equal windows the first, in line order, is kept. Each pattern is then searched
     *          as the list overload of findMelodyPattern() searches it, and kept when it has at
     *          least minOccurrences matches, its own window included with the default
     *          comparison. At the default thresholds of 1 a match is an exact repetition,
     *          transposed or not.
     * @param patternNumNotes Number of events in each pattern: at least 2.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match (default 1).
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match (default 1).
     * @param minOccurrences Minimum number of matches of a kept pattern: at least 1 (default
     *        2, a pattern that repeats).
     * @param intervalsSimilarityCallback See findMelodyPattern().
     * @param rhythmSimilarityCallback See findMelodyPattern().
     * @param totalIntervalSimilarityCallback See findMelodyPattern().
     * @param totalRhythmSimilarityCallback See findMelodyPattern().
     * @param totalSimilarityCallback See findMelodyPattern().
     * @return The kept patterns in line order of their first window, each with its matches.
     * @throws std::runtime_error If patternNumNotes is less than 2 or minOccurrences less than 1,
     *         or as findMelodyPattern() throws.
     * @note Any number of threads may search the same score at once. Modifying the score, its
     * parts, measures or notes while a search runs is not safe.
     */
    std::vector<FoundMelodyPattern> findAnyMelodyPattern(
        const int patternNumNotes = 5, const float intervalSimilarityThreshold = 1.0f,
        const float rhythmSimilarityThreshold = 1.0f, const int minOccurrences = 2,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;

    /**
     * @brief Extracts vertical chord structures from the score with configurable analysis
     * parameters.
     * @param config Optional JSON configuration object controlling chord extraction criteria.
     * @return Vector of tuples: {measure number, beat position, Key, Chord, homophony flag}.
     * @details Performs vertical harmonic analysis by extracting simultaneities (vertical chord
     * slices) from the polyphonic texture, with extensive filtering and processing options for
     *          texture analysis, harmonic progression studies, and style-specific chord detection.
     *
     *          **Configuration Parameters** (all optional):
     *          - `partNames` (list of strings): Restrict analysis to specific instrumental parts
     *            (e.g., ["Violino", "Viola", "Violoncelo"] for string trio texture)
     *          - `measureStart`, `measureEnd` (integers): Define measure range for analysis
     *          - `minStack`, `maxStack` (integers): Filter by chord cardinality (number of notes)
     *            - minStack=3, maxStack=4 → only triads and seventh chords
     *          - `minDuration`, `maxDuration` (strings): Filter by note duration
     *            (e.g., "quarter", "half", "whole")
     *          - `continuosMode` (boolean): Texture analysis mode selector
     *            - **true**: Continuous vertical slicing—captures ALL vertical alignments including
     *              non-aligned notes (polyphonic/contrapuntal texture analysis)
     *            - **false**: Attack-point synchronization—only chords formed by simultaneous
     *              note onsets (homophonic texture, block chord analysis)
     *          - `includeDuplicates` (boolean): Duplicate pitch handling
     *            - **true**: Keep every note, unisons included (["C4", "C4", "E4", "G4"])
     *            - **false**: Remove duplicates as Chord::removeDuplicateNotes() does: the notes
     *              are sorted, then each note spelled, at concert pitch, exactly as the note
     *              before it is removed (["C4", "C4", "E4", "G4"] gives ["C4", "E4", "G4"]): a
     *              B-flat clarinet's written D4 duplicates a violin's C4, while C#4 and Db4 are
     *              both kept, and so are the C4 and C5 of an octave doubling
     *          - `includeUnpitched` (boolean): Include percussion/unpitched elements
     *
     *          A note with an octave doubling (Note::getOctaveDoubling()) also adds, to each chord
     *          it sounds in, an untransposed note at its concert pitch one octave below or above;
     *          a doubled octave outside octaves -1..11, or below C1b-1, is left out, with one
     *          warning per note.
     *
     *          **Example Configuration**:
     *          \code{.json}
     *          {
     *            "partNames": ["Violino", "Viola", "Violoncelo"],
     *            "measureStart": 4,
     *            "measureEnd": 10,
     *            "minDuration": "quarter",
     *            "maxDuration": "whole",
     *            "continuosMode": true,
     *            "includeDuplicates": false,
     *            "includeUnpitched": false
     *          }
     *          \endcode
     *
     *          **Return Value Structure**:
     *          Each tuple contains:
     *          1. **Measure number** (int): Absolute measure position in score
     *          2. **Beat position** (float): Fractional beat location within measure
     *          3. **Key** (Key object): the concert key of the chord's measure -- the most
     *             frequent written key among the pitched parts that are untransposed there, or
     *             transposed by whole octaves only (a key is its fifths and its mode; a tie goes
     *             to the part that comes first; every pitched part counts, also those `partNames`
     *             leaves out, and unpitched parts never do). A part's transposition at a measure
     *             is that of its first pitched note there; in a measure without one, that of its
     *             last pitched note before, or else of its first one after. When every pitched
     *             part transposes, the first pitched part's written key moved by its interval, by
     *             7 fifths per semitone less 12 per letter, and brought by twelves into -6..11
     *             fifths, the range Key accepts (13 becomes 1)
     *          4. **Chord** (Chord object): Extracted vertical sonority
     *          5. **Homophony flag** (bool): True if all voices share identical rhythm
     *             (homophonic texture indicator)
     *
     *          **Applications**:
     *          - Harmonic analysis (chord progression extraction, functional harmony labeling)
     *          - Texture classification (homophonic vs. polyphonic density analysis)
     *          - Voice-leading analysis (chord-to-chord motion studies)
     *          - Statistical harmony studies (chord frequency distributions, bigram models)
     *          - Dissonance trajectory analysis (tracking harmonic tension across time)
     *
     * @note The `continuosMode` parameter critically affects results:
     *       - For homophonic textures (hymns, chorales): use continuosMode=false
     *       - For contrapuntal textures (fugues, inventions): use continuosMode=true
     */
    std::vector<std::tuple<int, float, Key, Chord, bool>> getChords(nlohmann::json config = {});
};
