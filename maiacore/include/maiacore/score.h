#pragma once

#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include "SQLiteCpp/SQLiteCpp.h"
#include "maiacore/chord.h"
#include "maiacore/constants.h"
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

    pugi::xml_document _doc;
    int _numParts;
    int _numMeasures;
    int _numNotes;
    std::map<int, std::string> _partsName;
    bool _isValidXML;
    bool _haveTypeTag;
    bool _isLoadedXML;
    std::vector<Chord> _stackedChords;
    int _lcmDivisionsPerQuarterNote; /* Least Common Multiple of all 'divisions'
                                        tags in XML file */

    void loadXMLFile(const std::string& filePath);

    std::vector<std::pair<int, Chord>> getSameAttackChords(
        SQLite::Database& db, const int minStackedNotes, const int maxStackedNotes,
        const int minDurationTicks, const int maxDurationTicks, const bool includeDuplicates);
    std::vector<std::pair<int, Chord>> getChordsPerEachNoteEvent(
        SQLite::Database& db, const int minStackedNotes, const int maxStackedNotes,
        const int minDurationTicks, const int maxDurationTicks, const bool includeDuplicates);

   public:
    /**
     * @brief Construct a new blank Score object
     *
     * @param partsName A musical instrument names list
     * @param numMeasures Initial measures amount
     */
    explicit Score(const std::initializer_list<std::string>& partsName, const int numMeasures = 20);

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
    Score(Score&&) = default;
    ~Score();

    /**
     * @brief Clear Score object
     *
     * Clear all content inside of Score object
     */
    void clear();

    void addPart(const std::string& partName, const int numStaves = 1);
    void removePart(const int partId);

    void addMeasure(const int numMeasures);
    void removeMeasure(const int measureStart, const int measureEnd);

    Part& getPart(const int partId);
    Part& getPart(const std::string& partName);

    int getNumParts() const;
    int getNumMeasures() const;
    int getNumNotes() const;

    const std::vector<std::string> getPartNames() const;
    std::string getTitle() const;

    void setTitle(const std::string& scoreTitle);
    void setComposerName(const std::string& composerName);

    void setKeySignature(const int fifthCicle, const bool isMajorMode = true,
                         const int measureId = 0);
    void setKeySignature(const std::string& key, const int measureId = 0);
    void setTimeSignature(const int timeUpper, const int timeLower, const int measureId = -1);
    void setMetronomeMark(int bpm, const Duration duration = Duration::QUARTER,
                          int measureStart = 0);

    const std::string toXML(const int identSize = 2) const;
    const std::string toJSON() const;

    void toFile(std::string fileName, const int identSize = 2) const;
    void info() const;
    void forEachNote(
        std::function<void(Part* part, Measure* measure, int staveId, Note* note)> callback,
        int measureStart = 0, int measureEnd = -1, std::vector<std::string> partNames = {});

    bool isValid(void) const;
    bool haveTypeTag(void) const;
    bool getNote(const int part, const int measure, const int note, std::string& pitch,
                 std::string& step, int& octave, int& duration, int& voice, std::string& type,
                 std::string& steam, int& staff) const;
    bool getNote(const int part, const int measure, const int note, std::string& pitch,
                 std::string& step, int& octave) const;
    bool getNote(const int part, const int measure, const int note, std::string& pitch) const;
    void printPartNames() const;

    int xPathCountNodes(const std::string& xPath) const;
    const std::map<int, std::string> getPartsName() const;
    const std::string getPartName(const int partId) const;
    bool getPartIndex(const std::string& partName, int& index) const;
    nlohmann::json selectNotes(nlohmann::json& config) const;

    const nlohmann::json findPattern(nlohmann::json& pattern) const;
    int countNotes(nlohmann::json& config) const;
    void getNoteNodeData(const pugi::xml_node& node, std::string& partName, int& measure,
                         std::string& pitch, std::string& pitchClass, std::string& alterSymbol,
                         int& alterValue, int& octave, std::string& type, float& duration) const;
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
     *   "minStack": 3,
     *   "maxStack": 5,
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
     * @return A list of pairs: {measureId, Chord object}
     */
    std::vector<std::pair<int, Chord>> getChords(nlohmann::json config = {});

    Score(const Score& other) {
        _title = other._title;
        _composerName = other._composerName;
        _part = other._part;

        //_doc = other._doc;
        _numParts = other._numParts;
        _numMeasures = other._numMeasures;
        _numNotes = other._numNotes;
        _partsName = other._partsName;
        _isValidXML = other._isValidXML;
        _haveTypeTag = other._haveTypeTag;
        _isLoadedXML = other._isLoadedXML;
        // _stackedChords = other._stackedChords;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
    }

    // copy assignment
    Score& operator=(const Score& other) {
        // Guard self assignment
        if (this == &other) {
            return *this;
        }

        return *this;
    }
};