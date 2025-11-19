#pragma once

#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include "SQLiteCpp/SQLiteCpp.h"
#include "maiacore/chord.h"
#include "maiacore/constants.h"
#include "maiacore/key.h"
#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/part.h"
#include "nlohmann/json.hpp"
#include "pugi/pugixml.hpp"

/**
 * @brief Represents a complete musical score, including metadata, parts, measures, and notes.
 * 
 * The Score class provides methods for creating, loading, editing, analyzing, and exporting musical scores.
 * It supports MusicXML import/export, part and measure management, note access, metadata handling, and advanced
 * musicological analysis such as melodic pattern search and chord extraction.
 */
class Score {
   private:
    std::string _title; ///< Title of the score.
    std::string _composerName; ///< Composer's name.
    std::vector<Part> _part; ///< List of parts (instruments/voices).
    std::string _filePath; ///< Path to the loaded MusicXML file.
    std::string _fileName; ///< Name of the loaded MusicXML file.

    pugi::xml_document _doc; ///< Internal XML document representation.
    int _numParts; ///< Number of parts in the score.
    int _numMeasures; ///< Number of measures in the score.
    int _numNotes; ///< Number of notes in the score.
    bool _isValidXML; ///< True if the XML was loaded and parsed successfully.
    bool _haveTypeTag; ///< True if the MusicXML contains <type> tags for notes.
    bool _isLoadedXML; ///< True if the score was loaded from a file.
    std::vector<Chord> _stackedChords; ///< Cached vertical chords.
    int _lcmDivisionsPerQuarterNote; ///< Least common multiple of all 'divisions' tags in the XML file.
    bool _haveAnacrusisMeasure; ///< True if the score contains an anacrusis (pickup) measure.

    /**
     * @brief Internal structure to represent a note event in the score.
     */
    struct NoteEvent {
        std::string partName;
        int measureIdx;
        int staveIdx;
        int noteIdx;
        const std::string keyName;
        const Note* notePtr;
    };

    mutable std::vector<NoteEvent> _cachedNoteEvents; ///< Cache for note events.
    mutable bool _isNoteEventsCached = false; ///< True if note events cache is filled.
    /**
     * @brief Collects all note events in the score for fast access and analysis.
     * @return Vector of NoteEvent structures.
     */
    std::vector<NoteEvent> collectNoteEvents() const;

    mutable std::vector<std::vector<NoteEvent>> _cachedNoteEventsPerPart; ///< Cache for note events per part.
    mutable bool _isNoteEventsPerPartCached = false; ///< True if per-part note events cache is filled.
    /**
     * @brief Collects note events grouped by part.
     * @return Vector of vectors of NoteEvent, one vector per part.
     */
    std::vector<std::vector<NoteEvent>> collectNoteEventsPerPart() const;

    /**
     * @brief Removes duplicate melodic patterns from a vector of patterns.
     * @details Compares patterns by MIDI pitch and duration differences, keeping only unique patterns.
     * @param patterns Pointer to the vector of note patterns to be filtered.
     */
    void removeDuplicatePatterns(std::vector<std::vector<Note>>* patterns) const;

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
     * @details Parses the XML, extracts metadata, parts, measures, and notes, and fills internal structures.
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
     * @brief Constructs a new blank Score object with specified part names and initial measure count.
     * @param partsName List of instrument/part names.
     * @param numMeasures Initial number of measures (default: 20).
     */
    explicit Score(const std::initializer_list<std::string>& partsName, const int numMeasures = 20);

    /**
     * @brief Constructs a new blank Score object with specified part names and initial measure count.
     * @param partsName Vector of instrument/part names.
     * @param numMeasures Initial number of measures (default: 20).
     */
    explicit Score(const std::vector<std::string>& partsName, const int numMeasures = 20);

    /**
     * @brief Constructs a new Score object by loading a MusicXML file.
     * @details Supported formats: *.xml, *.musicxml, *.mxl (compressed).
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
    void setKeySignature(const int fifthCicle, const bool isMajorMode = true, const int measureId = 0);

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
    void setMetronomeMark(int bpm, const RhythmFigure duration = RhythmFigure::QUARTER, int measureStart = 0);

    /**
     * @brief Exports the score to MusicXML format.
     * @details Generates a complete MusicXML string, including metadata and all parts.
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string.
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
     */
    void toFile(std::string fileName, bool compressedXML = false, const int identSize = 2) const;

    /**
     * @brief Prints summary information about the score to the log.
     * @details Includes title, composer, key, time signature, note count, measure count, and part names.
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
     * @brief Returns true if the MusicXML file contains <type> tags for notes.
     * @return True if <type> tags are present.
     */
    bool haveTypeTag(void) const;

    /**
     * @brief Retrieves detailed information about a specific note in the score.
     * @details Accesses attributes such as pitch, duration, voice, type, stem, and staff.
     * @param part Part index.
     * @param measure Measure index.
     * @param note Note index.
     * @param pitch Output: pitch string.
     * @param step Output: step string.
     * @param octave Output: octave integer.
     * @param duration Output: duration integer.
     * @param voice Output: voice integer.
     * @param type Output: type string.
     * @param steam Output: stem string.
     * @param staff Output: staff integer.
     * @return True if the note was found.
     */
    bool getNote(const int part, const int measure, const int note, std::string& pitch,
                 std::string& step, int& octave, int& duration, int& voice, std::string& type,
                 std::string& steam, int& staff) const;

    /**
     * @brief Retrieves basic information about a specific note (pitch, step, octave).
     * @param part Part index.
     * @param measure Measure index.
     * @param note Note index.
     * @param pitch Output: pitch string.
     * @param step Output: step string.
     * @param octave Output: octave integer.
     * @return True if the note was found.
     */
    bool getNote(const int part, const int measure, const int note, std::string& pitch,
                 std::string& step, int& octave) const;

    /**
     * @brief Retrieves only the pitch of a specific note.
     * @param part Part index.
     * @param measure Measure index.
     * @param note Note index.
     * @param pitch Output: pitch string.
     * @return True if the note was found.
     */
    bool getNote(const int part, const int measure, const int note, std::string& pitch) const;

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
     * @brief Retrieves detailed information from a note XML node.
     * @param node XML node of the note.
     * @param partName Output: part name.
     * @param measure Output: measure number.
     * @param pitch Output: pitch string.
     * @param pitchClass Output: pitch class string.
     * @param alterSymbol Output: accidental symbol.
     * @param alterValue Output: accidental value.
     * @param octave Output: octave.
     * @param type Output: note type.
     * @param duration Output: note duration.
     */
    void getNoteNodeData(const pugi::xml_node& node, std::string& partName, int& measure,
                         std::string& pitch, std::string& pitchClass, std::string& alterSymbol,
                         int& alterValue, int& octave, std::string& type, float& duration) const;

    /**
     * @brief Sets repeat barlines for a range of measures.
     * @param measureStart Starting measure index.
     * @param measureEnd Ending measure index (-1 for last measure).
     */
    void setRepeat(int measureStart, int measureEnd = -1);

    /**
     * @brief Analyzes instrumental fragmentation patterns across the score timeline.
     * @param config JSON configuration object specifying analysis parameters (parts, measures, etc).
     * @return JSON structure with activation, fragmentation, and execution metrics per instrument.
     * @details Performs temporal analysis of instrumental activity distribution, quantifying how
     *          melodic lines are fragmented across different instruments over time. This analysis
     *          is essential for orchestration studies, texture evolution tracking, and compositional
     *          strategy research.
     *
     *          **Fragmentation Metrics Computed**:
     *          1. **Activation Lines**: Temporal intervals where each instrument is active (sounding notes)
     *          2. **Fragmentation Patterns**: Transitions of melodic material between instruments
     *             (e.g., melody alternating between violin and flute—klangfarbenmelodie analysis)
     *          3. **Execution Density**: Proportion of time each instrument participates in texture
     *
     *          **Analysis Categories**:
     *          - **Continuous Execution**: Sustained melodic lines without rests (legato passages)
     *          - **Fragmented Execution**: Interrupted melodic lines with rests (staccato, pointillism)
     *          - **Hand-off Patterns**: Melodic continuation across instrument changes (orchestral dialogue)
     *          - **Tutti vs. Solo Distributions**: Ensemble density fluctuations
     *
     *          **Configuration Parameters** (config JSON):
     *          - `partNames` (list): Restrict analysis to specific instrumental parts
     *          - `measureStart`, `measureEnd` (int): Define temporal analysis window
     *          - `timeResolution` (float): Granularity for temporal slicing (e.g., 0.25 = sixteenth note)
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
     *          - Compositional fingerprinting (characteristic orchestration strategies per composer)
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

        // Deep copy of XML document
        _doc.reset(other._doc);

        // Invalidate caches - they will be rebuilt when needed
        _isNoteEventsCached = false;
        _isNoteEventsPerPartCached = false;
        _cachedNoteEvents.clear();
        _cachedNoteEventsPerPart.clear();
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

        // Deep copy of XML document
        _doc.reset(other._doc);

        // Invalidate caches - they will be rebuilt when needed
        _isNoteEventsCached = false;
        _isNoteEventsPerPartCached = false;
        _cachedNoteEvents.clear();
        _cachedNoteEventsPerPart.clear();

        return *this;
    }

    // ====== MELODIC PATTERN ANALYSIS ======

    /**
     * @brief Table row type for melodic pattern search results.
     * @details Contains part name, measure, stave, key, transposition, interval/rhythm differences, and similarity scores.
     */
    typedef std::tuple<std::string, int, int, std::string, std::string, std::vector<std::string>, std::vector<float>, std::vector<float>,
                       float, float, float> MelodyPatternRow;
    /**
     * @brief Table type for melodic pattern search results.
     */
    typedef std::vector<MelodyPatternRow> MelodyPatternTable;

    /**
     * @brief Searches for a melodic pattern throughout the score, returning detailed results.
     * @param melodyPattern Vector of notes representing the pattern to search for.
     * @param totalIntervalsSimilarityThreshold Minimum interval similarity threshold (0.0-1.0).
     * @param totalRhythmSimilarityThreshold Minimum rhythm similarity threshold (0.0-1.0).
     * @param intervalsSimilarityCallback Custom function to calculate interval similarity.
     * @param rhythmSimilarityCallback Custom function to calculate rhythm similarity.
     * @param totalIntervalSimilarityCallback Function to aggregate interval similarity.
     * @param totalRhythmSimilarityCallback Function to aggregate rhythm similarity.
     * @param totalSimilarityCallback Function to combine total similarities.
     * @return Table of results with detailed information about found patterns.
     * @details Performs comprehensive melodic pattern matching across all parts and measures of the score,
     *          supporting flexible similarity metrics for both intervallic contour and rhythmic structure.
     *          This function enables motivic analysis, thematic transformation studies, and computational
     *          detection of melodic recurrence.
     *
     *          **Pattern Matching Process**:
     *          1. Sliding window search across all melodic sequences in the score
     *          2. For each candidate match:
     *             a. Compute intervallic similarity (pitch contour matching)
     *             b. Compute rhythmic similarity (durational pattern matching)
     *             c. Aggregate similarities using custom or default models
     *          3. Filter results by threshold criteria
     *          4. Return matches with positional metadata (part, measure, beat)
     *
     *          **Similarity Calculation**:
     *          - **Default Interval Similarity**: Normalized edit distance or contour correlation
     *            between semitone interval sequences. Allows approximate matching (transposition,
     *            modal mutation, chromatic alteration).
     *          - **Default Rhythm Similarity**: Durational ratio comparison with tolerance for
     *            rhythmic augmentation/diminution and metric displacement.
     *          - **Custom Callbacks**: User-defined similarity models enable:
     *            - Weighted interval classes (emphasize melodic leaps vs. stepwise motion)
     *            - Parsons code contour matching (directional contour only)
     *            - Fuzzy matching with configurable tolerance bands
     *
     *          **Threshold Parameters**:
     *          - Values near 1.0: Require near-exact matches (strict motivic repetition)
     *          - Values near 0.5: Allow moderate variation (thematic transformation, development)
     *          - Values near 0.0: Detect loose similarity (distant motivic relationships)
     *
     *          **Applications**:
     *          - Motivic analysis (leitmotif tracking in Wagner, Brahms developing variation)
     *          - Thematic cataloging (identifying subject entries in fugues, variation themes)
     *          - Computational musicology (corpus-wide melodic similarity studies)
     *          - Plagiarism detection (melodic borrowing, paraphrase identification)
     *          - Style analysis (characteristic melodic gestures across composers/periods)
     *
     * @note Computational complexity is O(n × m) where n = total notes in score, m = pattern length.
     *       For large scores, consider restricting search to specific parts or measure ranges.
     */
    MelodyPatternTable findMelodyPattern(
        const std::vector<Note>& melodyPattern, const float totalIntervalsSimilarityThreshold = 0.5,
        const float totalRhythmSimilarityThreshold = 0.5,
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
     * @brief Searches for multiple melodic patterns in the score, returning a table for each pattern.
     * @details Allows parallel analysis of several patterns, useful for comparative research.
     * @param melodyPatterns Vector of melodic patterns.
     * @param totalIntervalsSimilarityThreshold Minimum interval similarity threshold.
     * @param totalRhythmSimilarityThreshold Minimum rhythm similarity threshold.
     * @param intervalsSimilarityCallback Custom function to calculate interval similarity.
     * @param rhythmSimilarityCallback Custom function to calculate rhythm similarity.
     * @param totalIntervalSimilarityCallback Function to aggregate interval similarity.
     * @param totalRhythmSimilarityCallback Function to aggregate rhythm similarity.
     * @param totalSimilarityCallback Function to combine total similarities.
     * @return Vector of result tables, one for each pattern.
     */
    std::vector<MelodyPatternTable> findMelodyPattern(
        const std::vector<std::vector<Note>>& melodyPatterns, const float totalIntervalsSimilarityThreshold = 0.5,
        const float totalRhythmSimilarityThreshold = 0.5,
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
     * @brief Finds all possible melodic patterns of a given length in the score.
     * @details Useful for exploratory analysis of motives and recurring melodic cells.
     * @param patternNumNotes Number of notes in each pattern.
     * @param totalIntervalsSimilarityThreshold Minimum interval similarity threshold.
     * @param totalRhythmSimilarityThreshold Minimum rhythm similarity threshold.
     * @param intervalsSimilarityCallback Custom function to calculate interval similarity.
     * @param rhythmSimilarityCallback Custom function to calculate rhythm similarity.
     * @param totalIntervalSimilarityCallback Function to aggregate interval similarity.
     * @param totalRhythmSimilarityCallback Function to aggregate rhythm similarity.
     * @param totalSimilarityCallback Function to combine total similarities.
     * @return Vector of result tables for each found pattern.
     */
    std::vector<MelodyPatternTable> findAnyMelodyPattern(const int patternNumNotes = 5,
        const float totalIntervalsSimilarityThreshold = 1.0f,
        const float totalRhythmSimilarityThreshold = 1.0f,
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
     * @brief Extracts vertical chord structures from the score with configurable analysis parameters.
     * @param config Optional JSON configuration object controlling chord extraction criteria.
     * @return Vector of tuples: {measure number, beat position, Key, Chord, homophony flag}.
     * @details Performs vertical harmonic analysis by extracting simultaneities (vertical chord slices)
     *          from the polyphonic texture, with extensive filtering and processing options for
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
     *          - `includeDuplicates` (boolean): Octave doubling handling
     *            - **true**: Preserve pitch-class duplications (["C4", "C4", "E4", "G4"])
     *            - **false**: Remove duplicate pitch classes (["C4", "E4", "G4"])
     *          - `includeUnpitched` (boolean): Include percussion/unpitched elements
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
     *          3. **Key** (Key object): Prevailing key signature at this position
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
