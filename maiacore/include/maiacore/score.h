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
     * @brief Analyzes instrumental fragmentation in the score, returning data for visualization.
     * @details Generates a JSON with activation, fragmentation, and execution lines for each instrument.
     * @param config JSON object with configuration parameters (parts, measures, etc).
     * @return JSON with data for analysis and plotting.
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
        _part = other._part;
        _numParts = other._numParts;
        _numMeasures = other._numMeasures;
        _numNotes = other._numNotes;
        _isValidXML = other._isValidXML;
        _haveTypeTag = other._haveTypeTag;
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
    }

    /**
     * @brief Assignment operator for Score.
     * @param other Score to assign from.
     * @return Reference to this Score.
     */
    Score& operator=(const Score& other) {
        if (this == &other) {
            return *this;
        }
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
     * @details Allows searching for melodic patterns with interval and rhythmic similarity criteria.
     * @param melodyPattern Vector of notes representing the pattern to search for.
     * @param totalIntervalsSimilarityThreshold Minimum interval similarity threshold.
     * @param totalRhythmSimilarityThreshold Minimum rhythm similarity threshold.
     * @param intervalsSimilarityCallback Custom function to calculate interval similarity.
     * @param rhythmSimilarityCallback Custom function to calculate rhythm similarity.
     * @param totalIntervalSimilarityCallback Function to aggregate interval similarity.
     * @param totalRhythmSimilarityCallback Function to aggregate rhythm similarity.
     * @param totalSimilarityCallback Function to combine total similarities.
     * @return Table of results with detailed information about found patterns.
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
     * @brief Get the vertical stacked chords from the score object
     *
     * @param config (Optional) A JSON object that contains: \n \n
     * \c partNames: list of strings \n
     * \c measureStart: integer \n
     * \c measureEnd: integer \n
     * \c minStack: integer \n
     * \c maxStack: integer \n
     * \c minDuration: string \n
     * \c maxDuration: string \n
     * \c continuosMode: boolean \n
     * \c includeDuplicates: boolean \n
     * \c includeUnpitched: boolean \n \n
     * \b Example:
     * \code{.json}
     * {
     *   "partNames": ["Violino", "Viola", "Violoncelo"],
     *   "measureStart": 4,
     *   "measureEnd": 10,
     *   "minDuration": "quarter",
     *   "maxDuration": "whole",
     *   "continuosMode": true,
     *   "includeDuplicates": false,
     *   "includeUnpitched": false
     * }
     * \endcode \n
     * \b continuosMode \n
     * When \c continuosMode is \c true the algorithm will detect all vertical
     * chords, including counterpoint notes \n When \c continuosMode is \c false
     * the algorithm will detect only the chords formed by the same attack
     * notes, excluding counterpoint \n \n \b includeDuplicates \n When \c
     * includeDuplicates is \c true the return chords can have multiple notes
     * with the same pitch value (duplicate notes) \n
     * Example: ["C4", "C4", "E4", "G4"] \n
     * When \c includeDuplicates is \c false each chord will be filtered in post
     * processing, returning the same chord but removing the duplicate pitch
     * notes \n Example: The original chord ["C4", "C4", "E4", "G4"] will be
     * returned as ["C4", "E4", "G4"] \n \n \b includeUnpitched \n
     *
     * @return A list of tuples: {measure, floatMeasure, Key object, Chord object}
     */
    std::vector<std::tuple<int, float, Key, Chord, bool>> getChords(nlohmann::json config = {});
};
