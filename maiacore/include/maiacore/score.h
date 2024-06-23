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

class Score {
   private:
    std::string _title;
    std::string _composerName;
    std::vector<Part> _part;
    std::string _filePath;
    std::string _fileName;

    pugi::xml_document _doc;
    int _numParts;
    int _numMeasures;
    int _numNotes;
    bool _isValidXML;
    bool _haveTypeTag;
    bool _isLoadedXML;
    std::vector<Chord> _stackedChords;
    int _lcmDivisionsPerQuarterNote; /* Least Common Multiple of all 'divisions'
                                        tags in XML file */
    bool _haveAnacrusisMeasure;

    /**
     * @brief Import a XML sheet music file: *.xml, *.musicxml or *.mxml
     *
     * Obs.: Use foward-slash (/) folder separator for all operational systems or double back-slash
     * (\\) in Windows-only system
     *
     * @param filePath XML absolute or relative file path
     */
    void loadXMLFile(const std::string& filePath);

    /**
     * @brief Get a chord list where each chord contains notes have the same attack in time (aligned
     * chord)
     *
     * @param db
     * @param minDurationTicks
     * @param maxDurationTicks
     * @param includeDuplicates
     * @return std::vector<std::tuple<int, float, Chord>>
     */
    std::vector<std::tuple<int, float, Key, Chord, bool>> getSameAttackChords(
        SQLite::Database& db, const int minDurationTicks, const int maxDurationTicks,
        const bool includeDuplicates);

    /**
     * @brief Get the Chords Per Each Note Event object
     *
     * @param db
     * @param minDurationTicks
     * @param maxDurationTicks
     * @param includeDuplicates
     * @return std::vector<std::tuple<int, float, Chord>>
     */
    std::vector<std::tuple<int, float, Key, Chord, bool>> getChordsPerEachNoteEvent(
        SQLite::Database& db, const int minDurationTicks, const int maxDurationTicks,
        const bool includeDuplicates);

   public:
    /**
     * @brief Construct a new blank Score object
     *
     * @param partsName A musical instrument names list
     * @param numMeasures Initial measures amount
     */
    explicit Score(const std::initializer_list<std::string>& partsName, const int numMeasures = 20);

    /**
     * @brief Construct a new Score object
     *
     * @param partsName
     * @param numMeasures
     */
    explicit Score(const std::vector<std::string>& partsName, const int numMeasures = 20);

    /**
     * @brief Construct a new Score object from a loaded sheet music file
     *
     * The supported input sheet music files are: \n
     * - *.xml (Standard for some music editors like: Sibelius and Finale) \n
     * - *.musicxml (Standard for other music editors like: MuseScore) \n
     * - *.mxml (Compressed/Zipped XML file)
     *
     * @param filePath Path to the XML sheet music file
     */
    explicit Score(const std::string& filePath);

    /**
     * @brief Construct a new Score object
     *
     */
    Score(Score&&) = default;

    /**
     * @brief Destroy the Score object
     *
     */
    ~Score();

    /**
     * @brief Clear Score object
     *
     * Clear all content inside of Score object
     */
    void clear();

    /**
     * @brief
     *
     * @param partName
     * @param numStaves
     */
    void addPart(const std::string& partName, const int numStaves = 1);

    /**
     * @brief
     *
     * @param partId
     */
    void removePart(const int partId);

    /**
     * @brief
     *
     * @param numMeasures
     */
    void addMeasure(const int numMeasures);

    /**
     * @brief
     *
     * @param measureStart
     * @param measureEnd
     */
    void removeMeasure(const int measureStart, const int measureEnd);

    /**
     * @brief Get the Part object
     *
     * @param partId
     * @return Part&
     */
    Part& getPart(const int partId);

    /**
     * @brief Get the Part object
     *
     * @param partName
     * @return Part&
     */
    Part& getPart(const std::string& partName);

    /**
     * @brief Get the Num Parts object
     *
     * @return int
     */
    int getNumParts() const;

    /**
     * @brief Get the Num Measures object
     *
     * @return int
     */
    int getNumMeasures() const;

    /**
     * @brief Get the Num Notes object
     *
     * @return int
     */
    int getNumNotes() const;

    /**
     * @brief Get the Part Names object
     *
     * @return const std::vector<std::string>
     */
    const std::vector<std::string> getPartsNames() const;

    /**
     * @brief Get the Title object
     *
     * @return std::string
     */
    std::string getTitle() const;

    /**
     * @brief Set the Title object
     *
     * @param scoreTitle
     */
    void setTitle(const std::string& scoreTitle);

    /**
     * @brief Get the Composer Name object
     *
     * @return std::string
     */
    std::string getComposerName() const;

    /**
     * @brief Set the Composer Name object
     *
     * @param composerName
     */
    void setComposerName(const std::string& composerName);

    /**
     * @brief Set the Key Signature object
     *
     * @param fifthCicle
     * @param isMajorMode
     * @param measureId
     */
    void setKeySignature(const int fifthCicle, const bool isMajorMode = true,
                         const int measureId = 0);
    /**
     * @brief Set the Key Signature object
     *
     * @param key
     * @param measureId
     */
    void setKeySignature(const std::string& key, const int measureId = 0);

    /**
     * @brief Set the Time Signature object
     *
     * @param timeUpper
     * @param timeLower
     * @param measureId
     */
    void setTimeSignature(const int timeUpper, const int timeLower, const int measureId = -1);

    /**
     * @brief Set the Metronome Mark object
     *
     * @param bpm
     * @param duration
     * @param measureStart
     */
    void setMetronomeMark(int bpm, const Duration duration = Duration::QUARTER,
                          int measureStart = 0);

    /**
     * @brief
     *
     * @param identSize
     * @return const std::string
     */
    const std::string toXML(const int identSize = 2) const;

    /**
     * @brief
     *
     * @return const std::string
     */
    const std::string toJSON() const;

    /**
     * @brief
     *
     * @param fileName
     * @param compressedXML
     * @param identSize
     */
    void toFile(std::string fileName, bool compressedXML = false, const int identSize = 2) const;

    /**
     * @brief
     *
     */
    void info() const;

    /**
     * @brief
     *
     * @param callback
     * @param measureStart
     * @param measureEnd
     * @param partNames
     */
    void forEachNote(
        std::function<void(Part* part, Measure* measure, int staveId, Note* note)> callback,
        int measureStart = 0, int measureEnd = -1, std::vector<std::string> partNames = {});

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isValid(void) const;

    std::string getFilePath() const;

    std::string getFileName() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveTypeTag(void) const;

    /**
     * @brief Get the Note object
     *
     * @param part
     * @param measure
     * @param note
     * @param pitch
     * @param step
     * @param octave
     * @param duration
     * @param voice
     * @param type
     * @param steam
     * @param staff
     * @return true
     * @return false
     */
    bool getNote(const int part, const int measure, const int note, std::string& pitch,
                 std::string& step, int& octave, int& duration, int& voice, std::string& type,
                 std::string& steam, int& staff) const;

    /**
     * @brief Get the Note object
     *
     * @param part
     * @param measure
     * @param note
     * @param pitch
     * @param step
     * @param octave
     * @return true
     * @return false
     */
    bool getNote(const int part, const int measure, const int note, std::string& pitch,
                 std::string& step, int& octave) const;

    /**
     * @brief Get the Note object
     *
     * @param part
     * @param measure
     * @param note
     * @param pitch
     * @return true
     * @return false
     */
    bool getNote(const int part, const int measure, const int note, std::string& pitch) const;

    /**
     * @brief
     *
     */
    void printPartNames() const;

    /**
     * @brief
     *
     * @param xPath
     * @return int
     */
    int xPathCountNodes(const std::string& xPath) const;

    /**
     * @brief Get the Part Name object
     *
     * @param partId
     * @return const std::string
     */
    const std::string getPartName(const int partId) const;

    /**
     * @brief Get the Part Index object
     *
     * @param partName
     * @param index
     * @return true
     * @return false
     */
    bool getPartIndex(const std::string& partName, int* index) const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnacrusisMeasure() const;

    // /**
    //  * @brief
    //  *
    //  * @param config
    //  * @return nlohmann::json
    //  */
    // nlohmann::json selectNotes(nlohmann::json& config) const;

    // /**
    //  * @brief
    //  *
    //  * @param pattern
    //  * @return const nlohmann::json
    //  */
    // const nlohmann::json findPattern(nlohmann::json& pattern) const;

    // /**
    //  * @brief
    //  *
    //  * @param config
    //  * @return int
    //  */
    // int countNotes(nlohmann::json& config) const;

    /**
     * @brief Get the Note Node Data object
     *
     * @param node
     * @param partName
     * @param measure
     * @param pitch
     * @param pitchClass
     * @param alterSymbol
     * @param alterValue
     * @param octave
     * @param type
     * @param duration
     */
    void getNoteNodeData(const pugi::xml_node& node, std::string& partName, int& measure,
                         std::string& pitch, std::string& pitchClass, std::string& alterSymbol,
                         int& alterValue, int& octave, std::string& type, float& duration) const;

    /**
     * @brief Set the Repeat object
     *
     * @param measureStart
     * @param measureEnd
     */
    void setRepeat(int measureStart, int measureEnd = -1);

    /**
     * @brief Get the called score 'instrument fragmentation' data
     *
     * This method is used only to get the input data to a specific Python plot
     * graph.\n The final Python plot shows the score 'instrument fragmentation'
     * analysis graphically
     *
     * \b Paper: Uma análise da organização e fragmentação de Farben de Arnold
     * Schoenberg \n \b Author: Igor Leão Maia (UNICAMP) \n \b C++/Python \b
     * Implementation \b by: Prof. Adolfo Maia Junior (UNICAMP) \n \n
     *
     * To get more information about it: \n
     * \link
     * https://www.researchgate.net/publication/321335427_Uma_analise_da_organizacao_e_fragmentacao_de_Farben_de_Arnold_Schoenberg
     * \endlink
     *
     * @param config A JSON object that contains: \n \n
     * \c partNumber: list of integers \n
     * \c measureStart: integer \n
     * \c measureEnd: integer \n \n
     * \b Example:
     * \code{.json}
     * {
     *   "partNumber": [1, 3, 7],
     *   "measureStart": 4,
     *   "measureEnd": 10
     * }
     * \endcode
     *
     * @return A JSON object that contains all data
     * to plot the 'instrument fragmentation' Python graph \n \n
     * \b Example:
     * \code{.json}
     * {
     *   "element":
     *   [
     *     {
     *       "partName": "Violin I",
     *       "measureStart": 1,
     *       "measureEnd": 5,
     *       "beatNumber": 3,
     *       "beatType": 4,
     *       "work": "My Score Title",
     *       "author": "Mozart",
     *       "Activation Rate": 2.5,
     *       "Number of Activations": 15,
     *       "lines": {
     *         {1.0f, 2.0f},
     *         {3.2f, 4.5f},
     *         {X, Y}
     *       }
     *       "lines_rests": {
     *         {2.0f, 3.2f},
     *         {3.5f, 5.7f},
     *         {W, Z}
     *       }
     *     },
     *     {
     *       // Other json object with the same fields
     *     }
     *   ]
     * }
     * \endcode
     */
    nlohmann::json instrumentFragmentation(nlohmann::json config = nlohmann::json());

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

    /**
     * @brief Construct a new Score object
     *
     * @param other
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
     * @brief
     *
     * @param other
     * @return Score&
     */
    Score& operator=(const Score& other) {
        // Guard self assignment
        if (this == &other) {
            return *this;
        }

        return *this;
    }
};
