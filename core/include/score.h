#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <initializer_list>
#include <chrono>

#include "pugixml.hpp"
#include "nlohmann/json.hpp"

#include "part.h"
#include "chord.h"
#include "helper.h"

#include <zip.h>


typedef std::multimap<float, std::pair<float, Note>> PartTableMap; // [timeStart, timeEnd, Note] For a single part (2D table)
typedef std::vector<PartTableMap> ScoreTable; // multiple parts (3D table)

class Score
{
private:
    std::string _title;
    std::string _composerName;
    std::vector<Part> _part;

    // ===== OLD MUSICXML class members ===== //
    pugi::xml_document _doc;
    int _numParts;
    int _numMeasures;
    int _numNotes;
    std::map<int, std::string> _partsName;
    int _divisionsPerQuarterNote;
    bool _isValidXML;
    bool _haveTypeTag;
    bool _isLoadedXML;

public:
    /**
     * @brief Construct a new blank Score object
     * 
     * @param partsName A musical instrument names list
     * @param numMeasures Initial measures amount
     * @param divisionsPerQuarterNote The number of internal divisions for each note (MusicXML protocol)
     */
    Score(const std::initializer_list<std::string>& partsName, const int numMeasures = 20, const int divisionsPerQuarterNote = 256);
    
    Score(const std::vector<std::string>& partsName, const int numMeasures = 20, const int divisionsPerQuarterNote = 256);

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
    Score(const std::string& filePath);
    ~Score();

    /**
     * @brief Clear Score object
     * 
     * Clear all content inside of Score object
     */
    void clear();
    void loadXMLFile(const std::string& filePath);

    void addPart(const std::string& partName, const int numStaves = 1);
    void removePart(const int partId);

    void addMeasure(const int numMeasures);
    void removeMeasure(const int measureStart, const int measureEnd);

    Part& getPart(const int partId);

    int getNumParts() const;
    int getNumMeasures() const;
    int getNumNotes() const; // overload like MusicXML class

    void setTitle(const std::string& title);
    void setComposerName(const std::string& composerName);

    void setKeySignature(const int fifthCicle, const bool isMajorMode = true, const int measureId = 0);
    void setKeySignature(const std::string& key, const int measureId = 0);
    void setTimeSignature(const int timeUpper, const int timeLower, const int measureId = -1);
    void setMetronomeMark(int bpm, const std::string& rhythmFigure = "quarter", int measureStart = 0);

    const std::string toXML(const int identSize = 2) const;
    const std::string toJSON() const;

    void toFile(std::string fileName, const int identSize = 2) const;
    void info() const;
    void forEachNote(std::function<void (Note& note)> callback, int measureStart = 0, int measureEnd = -1, std::vector<std::string> partNames = {});

    // ===== OLD MUSIC_XML CLASS METHODS ===== //
    bool isValid(void) const;
    bool haveTypeTag(void) const;
    bool getNote(const int part, const int measure, const int note, std::string& pitch, std::string& step, int& octave, int& duration, int& voice, std::string& type, std::string& steam, int& staff) const;
    bool getNote(const int part, const int measure, const int note, std::string& pitch, std::string& step, int& octave) const;
    bool getNote(const int part, const int measure, const int note, std::string& pitch) const;
    int getDivisionsPerQuarterNote() const;
    void printPartNames() const;

    int xPathCountNodes(const std::string& xPath) const;
    const std::map<int, std::string> getPartsName() const;
    const std::string getPartName(const int partId) const;
    bool getPartIndex(const std::string& partName, int& index) const;
    nlohmann::json selectNotes(nlohmann::json& config) const;

    const nlohmann::json findPattern(nlohmann::json& pattern) const;
    int countNotes(nlohmann::json& config) const;
    void getNoteNodeData(const pugi::xml_node& node, std::string& partName, int& measure, std::string& pitch, std::string& pitchClass, std::string& alterSymbol, int& alterValue, int& octave, std::string& type, float& duration) const;
    
    /**
     * @brief Get the called score 'instrument fragmentation' data
     * 
     * This method is used only to get the input data to a specific Python plot graph.\n
     * The final Python plot shows the score 'instrument fragmentation' analysis graphically 
     * 
     * \b Paper: Uma análise da organização e fragmentação de Farben de Arnold Schoenberg \n
     * \b Author: Igor Leão Maia (UNICAMP) \n
     * \b C++/Python \b Implementation \b by: Prof. Adolfo Maia Junior (UNICAMP) \n \n
     * 
     * To get more information about it: \n 
     * \link https://www.researchgate.net/publication/321335427_Uma_analise_da_organizacao_e_fragmentacao_de_Farben_de_Arnold_Schoenberg \endlink
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
    nlohmann::json instrumentFragmentation(const nlohmann::json config);

    /**
     * @brief Get the musical chords inside the score object
     * 
     * @param config (Optional) A JSON object that contains: \n \n
     * \c partNumber: list of integers \n
     * \c measureStart: integer \n
     * \c measureEnd: integer \n 
     * \c minStackedNotes: integer \n 
     * \c maxStackedNotes: integer \n \n
     * \b Example:
     * \code{.json}
     * {
     *   "partNumber": [1, 3, 7],
     *   "measureStart": 4,
     *   "measureEnd": 10,
     *   "minStackedNotes": 3,
     *   "maxStackedNotes": 5
     * }
     * \endcode
     * @param sameAttackNotes (Optional) Filters only for chords that have all notes with the same attack time (synchronized) 
     * @return A list of Chord objects
     */
    std::vector<Chord> getChords(nlohmann::json config = nlohmann::json(), const bool sameAttackNotes = true) const;

    void getSameAttackChords(std::vector<Chord>& stackedChords, ScoreTable& scoreTable, const int minStackedNotes, const int maxStackedNotes) const;
    void getChordsPerDeltaTime(std::vector<Chord>& stackedChords, ScoreTable& scoreTable, const int minStackedNotes, const int maxStackedNotes) const;

    nlohmann::json getPianoRoll(nlohmann::json& config) const;
};