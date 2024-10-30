#include "maiacore/score.h"

#include <filesystem>  // Para std::filesystem::absolute
#include <iostream>
#include <limits>  // std::numeric_limits
#include <set>
#include <tuple>

// #include "cherno/instrumentor.h"
#include "maiacore/clef.h"
#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"
#include "miniz-cpp/zip_file.hpp"
#include "nlohmann/json.hpp"

Score::Score(const std::initializer_list<std::string>& partsName, const int numMeasures)
    : _numParts(partsName.size()),
      _numMeasures(numMeasures),
      _numNotes(0),
      _isValidXML(false),
      _haveTypeTag(false),
      _isLoadedXML(false),
      _lcmDivisionsPerQuarterNote(0),
      _haveAnacrusisMeasure(false) {
    if (_numParts == 0) {
        LOG_ERROR("You MUST provide at least one part name");
    }

    for (const auto& part : partsName) {
        addPart(part);
    }
}

Score::Score(const std::vector<std::string>& partsName, const int numMeasures)
    : _numParts(partsName.size()),
      _numMeasures(numMeasures),
      _numNotes(0),
      _isValidXML(false),
      _haveTypeTag(false),
      _isLoadedXML(false),
      _lcmDivisionsPerQuarterNote(0),
      _haveAnacrusisMeasure(false) {
    if (_numParts == 0) {
        LOG_ERROR("You MUST provide at least one part name");
    }

    for (const auto& part : partsName) {
        addPart(part);
    }
}

Score::Score(const std::string& filePath)
    : _numParts(0),
      _numMeasures(0),
      _numNotes(0),
      _isValidXML(false),
      _haveTypeTag(false),
      _isLoadedXML(false),
      _lcmDivisionsPerQuarterNote(0),
      _haveAnacrusisMeasure(false) {
    // Instrumentor::Instance().beginSession("TEST");
    // PROFILE_FUNCTION();
    loadXMLFile(filePath);
    // Instrumentor::Instance().endSession();
}

Score::~Score() {}

void Score::clear() {
    // PROFILE_FUNCTION();

    _title.clear();
    _composerName.clear();
    _part.clear();
    _doc.reset();
    _numParts = 0;
    _numMeasures = 0;
    _numNotes = 0;
    // _partsName.clear();
    _isValidXML = false;
    _haveTypeTag = false;
    _isLoadedXML = false;
    _lcmDivisionsPerQuarterNote = 0;
}

void Score::info() const {
    LOG_INFO("Title: " << _title);
    LOG_INFO("Composer: " << _composerName);
    LOG_INFO("Key Signature: " << _part.at(0).getMeasure(0).getKeySignature());
    LOG_INFO("Time Signature: " << _part.at(0).getMeasure(0).getTimeSignature().getUpperValue()
                                << "/"
                                << _part.at(0).getMeasure(0).getTimeSignature().getLowerValue());
    LOG_INFO("Number of Notes: " << getNumNotes());
    LOG_INFO("Number of Measures: " << getNumMeasures());
    LOG_INFO("Number of Parts: " << getNumParts());

    // Get a part name list as a string
    const auto partsNames = getPartsNames();
    const int partsNamesSize = partsNames.size();
    std::string partNameList = "[";
    for (int i = 0; i < partsNamesSize; i++) {
        partNameList.append(partsNames[i]);
        if (i != partsNamesSize - 1) {
            partNameList.append(", ");
        }
    }

    partNameList.append("]");

    LOG_INFO("Parts: " << partNameList);
    LOG_INFO("Loaded from file: " << std::boolalpha << _isLoadedXML);
}

std::string Score::getFilePath() const { return _filePath; }

std::string Score::getFileName() const { return _fileName; }

void Score::loadXMLFile(const std::string& filePath) {
    clear();

    _filePath = filePath;

    _fileName = [](const std::string& path) -> std::string {
        // Encontrar a última ocorrência de '/' ou '\'
        size_t pos = path.find_last_of("/\\");

        // Se não encontrou, significa que a string é o nome do arquivo
        if (pos == std::string::npos) {
            return path;
        }

        // Retornar a substring após a última ocorrência de '/' ou '\'
        return path.substr(pos + 1);
    }(_filePath);

    const std::string fileExtension = filePath.substr(filePath.size() - 3, filePath.size());

    std::vector<std::string> result2 = Helper::splitString(filePath, '/');
    const std::string fileName = result2[result2.size() - 1];
    pugi::xml_parse_result isLoad;

    if (fileExtension == "mxl") {
        // LOG_DEBUG("Decompressing file...");
        miniz_cpp::zip_file file(filePath);

        // Read the internal META-INF/container.xml file
        const std::string containerFile = file.read("META-INF/container.xml");
        pugi::xml_document containerXML;
        containerXML.load_string(containerFile.c_str());

        const std::string xPathInternalXMLFile = "/container/rootfiles/rootfile";

        const std::string internalXMLFileName =
            containerXML.select_node(xPathInternalXMLFile.c_str())
                .node()
                .attribute("full-path")
                .value();

        const std::string fileContent = file.read(internalXMLFileName);
        isLoad = _doc.load_string(fileContent.c_str());
    } else {
        // Try to parse the XML file:
        isLoad = _doc.load_file(filePath.c_str());
    }

    // Error checking:
    if (!isLoad) {
        LOG_ERROR("Unable to load the file: " + filePath);
        return;
    }

    // Try to get the main MusicXML nodes:
    const std::string xPathParts = "/score-partwise/part";
    const std::string xPathMeasures = "/score-partwise/part[1]/measure";
    const std::string xPathNotes = "/score-partwise//part//measure//note";
    const std::string xPathPartsName = "/score-partwise/part-list//score-part/part-name";
    const std::string xPathDivisionsPerQuarterNote =
        "/score-partwise//part//measure/attributes/divisions";
    const std::string xPathHaveTypeName = "/score-partwise//part//measure//note/type";

    const pugi::xpath_node_set parts = _doc.select_nodes(xPathParts.c_str());
    const pugi::xpath_node_set measures = _doc.select_nodes(xPathMeasures.c_str());
    const pugi::xpath_node_set notes = _doc.select_nodes(xPathNotes.c_str());
    const pugi::xpath_node_set partsName = _doc.select_nodes(xPathPartsName.c_str());
    const pugi::xpath_node_set divisionsPerQuarterNote =
        _doc.select_nodes(xPathDivisionsPerQuarterNote.c_str());
    const pugi::xpath_node_set haveTypeName = _doc.select_nodes(xPathHaveTypeName.c_str());

    // Error checking:
    if (parts.empty()) {
        LOG_ERROR("Unable to locate the MusicXML XPath: " + xPathParts);
        return;
    }

    if (measures.empty()) {
        LOG_ERROR("Unable to locate the MusicXML XPath: " + xPathMeasures);
        return;
    }

    if (partsName.empty()) {
        LOG_ERROR("Unable to locate the MusicXML XPath: " + xPathPartsName);
        return;
    }

    // Detect Anacrusis Measure
    const std::string& xPathAnacrusisMeasure = "/score-partwise//part//measure[@number='0']";
    const pugi::xpath_node_set anacrusisMeasure = _doc.select_nodes(xPathAnacrusisMeasure.c_str());
    _haveAnacrusisMeasure = (anacrusisMeasure.empty()) ? false : true;

    // Get the all part names:
    const int partsNameSize = partsName.size();
    std::vector<std::string> partsNameVec;
    partsNameVec.reserve(partsNameSize);
    for (int n = 0; n < partsNameSize; n++) {
        const pugi::xml_node& name = partsName[n].node();
        std::string rawPartName = name.text().as_string();
        // Substitui todas as ocorrências de '\n' por ' '
        std::replace(rawPartName.begin(), rawPartName.end(), '\n', ' ');

        // Substitui os espaços duplos por apenas um espaço simples
        size_t pos = 0;
        while ((pos = rawPartName.find("  ", pos)) != std::string::npos) {
            rawPartName.replace(pos, 2, " ");
        }

        partsNameVec.push_back(rawPartName);
    }

    // Lambda function to detect duplacated part names
    auto hasDuplicatesPartNames = [](const std::vector<std::string>& pNames) -> bool {
        std::set<std::string> seen;
        for (const auto& item : pNames) {
            if (!seen.insert(item).second) {
                return true;
            }
        }
        return false;
    };

    if (hasDuplicatesPartNames(partsNameVec)) {
        LOG_INFO("Adding part names index suffix to better identification");

        // Adding part names index suffix to better identification
        auto modifyNames = [](std::vector<std::string>& vec) {
            std::unordered_map<std::string, int> countMap;
            std::unordered_map<std::string, int> indexMap;

            for (const auto& name : vec) {
                countMap[name]++;
            }

            for (auto& name : vec) {
                if (countMap[name] > 1) {
                    int& idx = indexMap[name];
                    idx++;
                    name += " " + std::to_string(idx);
                }
            }
        };

        // Adding part names index suffix to better identification
        modifyNames(partsNameVec);
    }

    // Get the parts and measures amounts:
    _numParts = parts.size();
    _numMeasures = measures.size();
    _numNotes = notes.size();

    // ===== GET THE DIVISIONS PER QUARTER NOTE ===== //
    if (divisionsPerQuarterNote.empty()) {
        _lcmDivisionsPerQuarterNote = 256;
    } else {
        const int divisionsPerQuarterNoteSize = divisionsPerQuarterNote.size();
        std::vector<int> div_vec(divisionsPerQuarterNoteSize, 0);

        for (int d = 0; d < divisionsPerQuarterNoteSize; d++) {
            const int divisions = divisionsPerQuarterNote[d].node().text().as_int();
            div_vec[d] = divisions;

            // LOG_DEBUG("div_vec[" << d << "]: " << div_vec[d]);
        }

        _lcmDivisionsPerQuarterNote = div_vec[0];
        for (int d = 0; d < divisionsPerQuarterNoteSize; d++) {
            _lcmDivisionsPerQuarterNote = std::lcm(_lcmDivisionsPerQuarterNote, div_vec[d]);
        }
    }

    // LOG_DEBUG("LCM: " << _lcmDivisionsPerQuarterNote);

    // ===== CHECK BASIC OBJECT VALIDATION ===== //
    if ((_numParts > 0) && (_numMeasures > 0) && (_lcmDivisionsPerQuarterNote > 0)) {
        _isValidXML = true;
        _isLoadedXML = true;
    }

    // ===== VERIFY IF THIS FILE HAVE TYPE NAME TAG ===== //
    if (!haveTypeName.empty()) {
        _haveTypeTag = true;
    }

    // ===== GET SCORE METADATA ===== //
    const std::string workTitle =
        _doc.select_node("/score-partwise/work/work-title").node().text().as_string();
    const std::string composerName =
        _doc.select_node("/score-partwise/identification/creator").node().text().as_string();

    setTitle(workTitle);
    setComposerName(composerName);

    // ===== PARSING THE FILE TO THE CLASS MEMBERS ===== //
    // For each part 'p'
    for (int p = 0; p < _numParts; p++) {
        addPart(partsNameVec[p]);

        const std::string xPathPartList = "/score-partwise/part-list/score-part[@id='P" +
                                          std::to_string(p + 1) + "']/midi-instrument";
        const std::string xPathPartListUnpitched = "/score-partwise/part-list/score-part[@id='P" +
                                                   std::to_string(p + 1) +
                                                   "']/midi-instrument/midi-unpitched";

        const pugi::xpath_node_set midiInstrumentUnpitched =
            _doc.select_nodes(xPathPartListUnpitched.c_str());
        const pugi::xpath_node_set midiInstrument = _doc.select_nodes(xPathPartList.c_str());

        for (const auto& node : midiInstrument) {
            int midiUnpitched = atoi(node.node().child_value("midi-unpitched"));
            _part[p].addMidiUnpitched(midiUnpitched);
        }

        if (!midiInstrumentUnpitched.empty()) {
            _part[p].setIsPitched(false);
        }

        // ==========================
        std::string xPathPart = "/score-partwise/part[" + std::to_string(p + 1) + "]";

        // ===== CHECK IF THERE MORE THAN ONE STAVES ===== //
        const std::string firstMeasure = xPathPart + "/measure[1]";
        const pugi::xpath_node staves =
            _doc.select_node(firstMeasure.c_str()).node().select_node("attributes/staves");

        if (!staves.node().empty()) {
            const int numStaves = atoi(staves.node().first_child().value());
            _part[p].setNumStaves(numStaves);
        }

        // ===== STEP 1: GET THE PART 'i' STAFF LINES ===== //
        const pugi::xpath_node staffLinesNode =
            _doc.select_node(firstMeasure.c_str())
                .node()
                .select_node("attributes/staff-details/staff-lines");

        if (!staffLinesNode.node().empty()) {
            const int staffLines = atoi(staffLinesNode.node().first_child().value());
            _part[p].setStaffLines(staffLines);
        }
        // ===== STEP 2: GET THE PART 'i' TRANSPOSE VALUES ===== //
        const std::string xPathTranspose = xPathPart + "/measure[1]/attributes/transpose";

        // Get the xPath result:
        const pugi::xpath_node_set numTransposeTag = Helper::getNodeSet(_doc, xPathTranspose);

        // Verify if the part 'i' is a transposed instrument or not:
        const bool isTransposedInstrument = (numTransposeTag.size() == 0) ? false : true;

        // Get the number of staves for the part 'i':
        int transposeDiatonic = 0;
        int transposeChromatic = 0;

        const std::string xPathDiatonic = "/diatonic";
        const std::string xPathChormatic = "/chromatic";

        if (isTransposedInstrument) {
            const pugi::xpath_node_set diatonic =
                Helper::getNodeSet(_doc, xPathTranspose + xPathDiatonic);
            const pugi::xpath_node_set chromatic =
                Helper::getNodeSet(_doc, xPathTranspose + xPathChormatic);

            transposeDiatonic = diatonic[0].node().text().as_int();
            transposeChromatic = chromatic[0].node().text().as_int();
        }

        // Get the part 'p' first measure divisions per quarter note
        const pugi::xpath_node firstMeasureDivisionsPerQuarterNote =
            _doc.select_node(firstMeasure.c_str()).node().select_node("attributes/divisions");

        const int firstDivisionsTemp = firstMeasureDivisionsPerQuarterNote.node().text().as_int();

        // If the XML file contains the DPQ info, use that information
        // But if the XML file does not contain this info, use the default value of 256
        const int firstDivisions = (firstDivisionsTemp != 0) ? firstDivisionsTemp : 256;

        // ===== STEP 3: GET THE PART 'i' CLEFS ===== //
        const std::string xPathDefaultClefs = xPathPart + "/measure[1]/attributes/clef";

        // Get the xPath result:
        const pugi::xpath_node_set defaultClefsNodes = Helper::getNodeSet(_doc, xPathDefaultClefs);
        const int numClefs = defaultClefsNodes.size();
        std::vector<Clef> defaultClefs(numClefs);

        for (int i = 0; i < numClefs; i++) {
            const std::string sign = defaultClefsNodes[i].node().child_value("sign");
            const int line = atoi(defaultClefsNodes[i].node().child_value("line"));
            defaultClefs[i].setSign(Clef::clefSignStr2ClefSign(sign));
            defaultClefs[i].setLine(line);
        }

        // For each measure 'm'
        for (int m = 0; m < _numMeasures; m++) {
            _part[p].getMeasure(m).setNumber(m);

            // Get the xPath for the measure 'm'
            const std::string xPathMeasure = xPathPart + "/measure[" + std::to_string(m + 1) + "]";

            // ===== DIVISIONS PER QUARTER NOTE CHANGES ===== //
            const pugi::xpath_node measureDivisionsPerQuarterNote =
                _doc.select_node(xPathMeasure.c_str()).node().select_node("attributes/divisions");

            if (!measureDivisionsPerQuarterNote.node().empty()) {
                _part[p].getMeasure(m).setIsDivisionsPerQuarterNoteChanged(true);
                const int divisions = measureDivisionsPerQuarterNote.node().text().as_int();
                _part[p].getMeasure(m).setDivisionsPerQuarterNote(divisions);
            } else {
                _part[p].getMeasure(m).setDivisionsPerQuarterNote(firstDivisions);
            }

            // ===== KEY SIGNATURE CHANGES ===== //
            const pugi::xpath_node measureKeyFifths =
                _doc.select_node(xPathMeasure.c_str()).node().select_node("attributes/key");
            if (!measureKeyFifths.node().empty()) {
                _part[p].getMeasure(m).setIsKeySignatureChanged(true);
                const int fifthCircle = atoi(measureKeyFifths.node().child_value("fifths"));
                _part[p].getMeasure(m).setKeySignature(fifthCircle);

                const std::string keyModeStr = measureKeyFifths.node().child_value("mode");

                if (keyModeStr.empty()) {
                    LOG_WARN("[XML MISSING TAG][" + _part[p].getName() + "][" + std::to_string(m) +
                             "] The key signature mode is empty. Auto-configing to 'major' mode");
                }

                const bool isMajorKey =
                    (keyModeStr.empty() || keyModeStr == "major") ? true : false;
                _part[p].getMeasure(m).setKeyMode(isMajorKey);

                _part[p].getMeasure(m).setKey(fifthCircle, isMajorKey);
            } else {
                const Key previusKey = _part[p].getMeasure(m - 1).getKey();
                _part[p].getMeasure(m).setKey(previusKey.getFifthCircle(),
                                              previusKey.isMajorMode());
            }

            // ===== TIME SIGNATURE CHANGES ===== //
            const pugi::xpath_node measureTimeSignature =
                _doc.select_node(xPathMeasure.c_str()).node().select_node("attributes/time");
            if (!measureTimeSignature.node().empty()) {
                _part[p].getMeasure(m).setIsTimeSignatureChanged(true);
                const int upper = atoi(measureTimeSignature.node().child_value("beats"));
                const int lower = atoi(measureTimeSignature.node().child_value("beat-type"));
                _part[p].getMeasure(m).setTimeSignature(upper, lower);
            }

            // ===== STAVES ===== //
            const pugi::xpath_node measureStaves =
                _doc.select_node(xPathMeasure.c_str()).node().select_node("attributes/staves");
            if (!measureStaves.node().empty()) {
                const int numStaves = measureStaves.node().text().as_int();
                _part[p].getMeasure(m).setNumStaves(numStaves);
            }

            // ===== CLEF CHANGES ===== //
            const std::string xPathClefs = xPathMeasure + "/attributes/clef";
            const pugi::xpath_node_set measureClef = _doc.select_nodes(xPathClefs.c_str());
            const int currentNumClefs = measureClef.size();
            bool currentClefsInMeasureChange = (currentNumClefs > 0) ? true : false;
            _part[p].getMeasure(m).getClefs().resize(numClefs);

            if (!currentClefsInMeasureChange) {
                _part[p].getMeasure(m).getClefs() = defaultClefs;
            } else {
                for (int c = 0; c < numClefs; c++) {
                    const std::string sign = measureClef[c].node().child_value("sign");
                    const int line = atoi(measureClef[c].node().child_value("line"));
                    _part[p].getMeasure(m).getClef(c).setSign(Clef::clefSignStr2ClefSign(sign));
                    _part[p].getMeasure(m).getClef(c).setLine(line);
                }
            }

            // ===== BARLINE CHANGES ===== //
            const std::string xPathBarline = xPathMeasure + "/barline";
            const pugi::xpath_node_set measureBarlines = _doc.select_nodes(xPathBarline.c_str());
            for (const auto& barline : measureBarlines) {
                const std::string barlineLocation = barline.node().attribute("location").value();
                const std::string barStyle = barline.node().child_value("bar-style");
                const std::string barDirection =
                    barline.node().child("repeat").attribute("direction").as_string();

                if (barlineLocation == "left") {
                    _part[p].getMeasure(m).getBarlineLeft().setLocation(barlineLocation);
                    _part[p].getMeasure(m).getBarlineLeft().setBarStyle(barStyle);
                    _part[p].getMeasure(m).getBarlineLeft().setDirection(barDirection);
                } else {
                    _part[p].getMeasure(m).getBarlineRight().setLocation(barlineLocation);
                    _part[p].getMeasure(m).getBarlineRight().setBarStyle(barStyle);
                    _part[p].getMeasure(m).getBarlineRight().setDirection(barDirection);
                }
            }
            // Get the xPath for all notes inside the measure 'm'
            const std::string xPathNotes = xPathMeasure + "//note";

            // Get all notes in this measure 'm'
            const pugi::xpath_node_set nodes = _doc.select_nodes(xPathNotes.c_str());
            const int noteSize = nodes.size();

            bool isNoteOn = false;
            bool inChord = false;
            bool isGraceNote = false;
            bool isTuple = false;
            bool isUnpitched = false;
            // int numDots = 0;
            int numTies = 0;
            int octave = 0;
            std::string step;
            std::string pitch;
            int durationTicks = 0;
            int voice = 0;
            std::string type;
            std::string stem;
            int staff = 0;
            int tupleActualNotes = 0;
            int tupleNormalNotes = 0;
            std::string tupleNormalType;
            int unpitchedIndex = 0;

            // For each note 'n'
            for (int n = 0; n < noteSize; n++) {
                auto node = nodes[n].node();

                // ===== GET NOTE DATA ===== //
                isNoteOn = !node.child("rest");
                inChord = node.child("chord");
                isTuple = node.child("time-modification");
                isGraceNote = node.child("grace");
                isUnpitched = node.child("unpitched");
                durationTicks = atoi(node.child_value("duration"));
                voice = atoi(node.child_value("voice"));
                type = node.child_value("type");
                stem = node.child_value("stem");
                staff = atoi(node.child_value("staff")) - 1;
                tupleActualNotes =
                    atoi(node.child("time-modification").child_value("actual-notes"));
                tupleNormalNotes =
                    atoi(node.child("time-modification").child_value("normal-notes"));
                tupleNormalType = node.child("time-modification").child_value("normal-type");
                if (tupleActualNotes == 0) {
                    tupleActualNotes = 1;
                }

                if (tupleNormalNotes == 0) {
                    tupleNormalNotes = 1;
                }

                if (tupleNormalType.empty()) {
                    tupleNormalType = type;
                }

                // ===== GET NOTE PITCH ===== //
                step = (!isUnpitched) ? node.child("pitch").child_value("step")
                                      : node.child("unpitched").child_value("display-step");

                if (isUnpitched) {
                    const auto idAttr = node.child("instrument").attribute("id").as_string();
                    unpitchedIndex = atoi(Helper::splitString(idAttr, '-')[1].substr(1).c_str());
                }

                const std::string alterTag = node.child("pitch").child_value("alter");

                std::string alterSymbol;
                if (!alterTag.empty()) {
                    switch (hash(alterTag.c_str())) {
                        case hash("-2"):
                            alterSymbol = "bb";
                            break;
                        case hash("-1"):
                            alterSymbol = "b";
                            break;
                        case hash("1"):
                            alterSymbol = "#";
                            break;
                        case hash("2"):
                            alterSymbol = "x";
                            break;
                    }
                }

                if (!isNoteOn) {
                    // octave = MUSIC_XML::OCTAVE::ALL;
                    pitch = MUSIC_XML::PITCH::REST;
                } else {
                    octave = (!isUnpitched)
                                 ? atoi(node.child("pitch").child_value("octave"))
                                 : atoi(node.child("unpitched").child_value("display-octave"));
                    pitch = step + alterSymbol + std::to_string(octave);
                }

                if (voice == 0) {
                    voice = 1;
                }

                if (staff <= 0) {
                    staff = 0;
                }

                // ===== CONSTRUCT A NOTE OBJECT AND STORE IT INSIDE THE SCORE ===== //

                Note note(pitch);
                note.setIsInChord(inChord);
                note.setTransposingInterval(transposeDiatonic, transposeChromatic);
                note.setVoice(voice);
                note.setStaff(staff);
                note.setIsGraceNote(isGraceNote);
                note.setStem(stem);
                note.setIsTuplet(isTuple);
                note.setIsPitched(!isUnpitched);
                // note.setTupleValues(tupleActualNotes, tupleNormalNotes, tupleNormalType);
                note.setUnpitchedIndex(unpitchedIndex);

                // ===== NOTE DURATION ===== //
                const int divPQN = _part[p].getMeasure(m).getDivisionsPerQuarterNote();
                if (!isGraceNote) {
                    note.setDuration({durationTicks, divPQN, tupleActualNotes, tupleNormalNotes});
                }

                // ===== ARTICULATIONS ===== //
                for (pugi::xml_node articulation :
                     node.child("notations").child("articulations").children()) {
                    note.addArticulation(articulation.name());
                }

                // ===== BEAMS ===== //
                const auto beamNodes = node.select_nodes("beam");
                const int numBeams = beamNodes.size();

                for (int b = 0; b < numBeams; b++) {
                    const std::string beam = beamNodes[b].node().text().as_string();
                    note.addBeam(beam);
                }
                // ===== TIES ===== //
                const auto tieNodes = node.select_nodes("tie");
                numTies = tieNodes.size();

                for (int t = 0; t < numTies; t++) {
                    const std::string tie = tieNodes[t].node().attribute("type").as_string();
                    note.addTie(tie);
                }

                // ===== SLUR ===== //
                const bool existSlur = node.child("notations").child("slur");

                if (existSlur) {
                    const std::string slurType =
                        node.child("notations").child("slur").attribute("type").as_string();
                    const std::string slurOrientation =
                        node.child("notations").child("slur").attribute("orientation").as_string();
                    note.addSlur(slurType, slurOrientation);
                }
                // ===== DOTS ===== //
                // const auto dotsNodes = node.select_nodes("dot");
                // numDots = dotsNodes.size();

                // switch (numDots) {
                //     case 0:
                //         note.removeDots();
                //         break;
                //     case 1:
                //         note.setSingleDot();
                //         break;
                //     case 2:
                //         note.setDoubleDot();
                //         break;
                //     default:
                //         LOG_ERROR("More than 2 dots in a note");
                //         break;
                // }

                _part[p].getMeasure(m).addNote(note, staff);
            }
        }
    }
}

void Score::addPart(const std::string& partName, const int numStaves) {
    // PROFILE_FUNCTION();

    _part.emplace_back(partName, numStaves);
    _part.back().addMeasure(_numMeasures);

    const int partIdx = _part.size() - 1;
    _part.back().setPartIndex(partIdx);
}

void Score::removePart(const int partId) {
    // PROFILE_FUNCTION();

    if (partId >= static_cast<int>(_part.size())) {
        LOG_ERROR("Invalid part index");
        return;
    }

    _part.erase(_part.begin() + partId);
}

void Score::addMeasure(const int numMeasures) {
    // PROFILE_FUNCTION();
    const int partSize = _part.size();

    for (int i = 0; i < partSize; i++) {
        _part[i].addMeasure(numMeasures);
    }
}

void Score::removeMeasure(const int measureStart, const int measureEnd) {
    // PROFILE_FUNCTION();
    if (measureEnd < measureStart) {
        LOG_ERROR("The 'measureEnd' MUST BE equal or greater than 'measureStart'");
        return;
    }

    const int partSize = _part.size();

    for (int i = 0; i < partSize; i++) {
        _part[i].removeMeasure(measureStart, measureEnd);
    }
}

Part& Score::getPart(const int partId) {
    // PROFILE_FUNCTION();

    const int partSize = _part.size();
    if (partId < 0 || partId > (partSize - 1)) {
        LOG_ERROR("Invalid partId: " + std::to_string(partId));
    }

    return _part.at(partId);
}

Part& Score::getPart(const std::string& partName) {
    // PROFILE_FUNCTION();
    int partIndex = 0;
    const bool isValid = getPartIndex(partName, &partIndex);

    if (!isValid) {
        printPartNames();
        LOG_ERROR("There is no '" + partName + "' in this score");
    }

    return getPart(partIndex);
}

int Score::getNumParts() const {
    // PROFILE_FUNCTION();

    return _part.size();
}

int Score::getNumMeasures() const {
    // PROFILE_FUNCTION();

    return _numMeasures;
}

int Score::getNumNotes() const {
    // PROFILE_FUNCTION();

    int numNotes = 0;

    for (const auto& part : _part) {
        for (int m = 0; m < part.getNumMeasures(); m++) {
            numNotes += part.getMeasure(m).getNumNotes();
        }
    }

    return numNotes;
}

const std::vector<std::string> Score::getPartsNames() const {
    // PROFILE_FUNCTION();

    const int numParts = getNumParts();
    std::vector<std::string> partNames(numParts);

    for (int p = 0; p < numParts; p++) {
        partNames[p] = _part[p].getName();
    }

    return partNames;
}

std::string Score::getTitle() const { return _title; }

void Score::setTitle(const std::string& scoreTitle) { _title = scoreTitle; }

std::string Score::getComposerName() const { return _composerName; }

void Score::setComposerName(const std::string& composerName) { _composerName = composerName; }

void Score::setKeySignature(const int fifthCicle, const bool isMajorMode, const int measureId) {
    // PROFILE_FUNCTION();
    const int partSize = _part.size();

    for (int i = 0; i < partSize; i++) {
        _part.at(i).getMeasure(measureId).setKeySignature(fifthCicle, isMajorMode);
    }
}

void Score::setKeySignature(const std::string& key, const int measureId) {
    // PROFILE_FUNCTION();
    const bool isMajorKey = (key.back() != 'm') ? true : false;

    const std::map<std::string, int> c_majorKeySignatureMap{
        std::make_pair("", 0),    std::make_pair("G", 1),   std::make_pair("D", 2),
        std::make_pair("A", 3),   std::make_pair("E", 4),   std::make_pair("B", 5),
        std::make_pair("F#", 6),  std::make_pair("C#", 7),  std::make_pair("G#", 8),
        std::make_pair("D#", 9),  std::make_pair("A#", 10), std::make_pair("E#", 11),
        std::make_pair("B#", 12), std::make_pair("Gb", -6), std::make_pair("Db", -5),
        std::make_pair("Ab", -4), std::make_pair("Eb", -3), std::make_pair("Bb", -2),
        std::make_pair("F", -1)};

    const int fifthCicle = c_majorKeySignatureMap.at(key);
    setKeySignature(fifthCicle, isMajorKey, measureId);
}

void Score::setTimeSignature(const int timeUpper, const int timeLower, const int measureId) {
    // PROFILE_FUNCTION();
    if (measureId < 0) {  // For all measures
        for (auto& part : _part) {
            for (int m = 0; m < _numMeasures; m++) {
                part.getMeasure(m).setTimeSignature(timeUpper, timeLower);

                // Remove the explicit time signature notation
                if (m != 0) {
                    part.getMeasure(m).setIsTimeSignatureChanged(false);
                }
            }
        }
    } else {  // Starting in a specific measure only
        for (auto& part : _part) {
            for (int m = measureId; m < _numMeasures; m++) {
                part.getMeasure(m).setTimeSignature(timeUpper, timeLower);
            }
        }
    }
}

void Score::setMetronomeMark(int bpm, const RhythmFigure duration, int measureStart) {
    // PROFILE_FUNCTION();
    // ===== VALIDADE INPUT ARGUMENTS ===== //
    // BPM validation
    if (bpm <= 0) {
        LOG_ERROR("BPM must be a positive value");
    }

    // measureStart validation
    measureStart = (measureStart < 0) ? 0 : measureStart;

    // ===== PROCESSING ===== //
    const int partSize = _part.size();

    for (int i = 0; i < partSize; i++) {
        _part[i].getMeasure(measureStart).setMetronome(bpm, duration);
    }
}

const std::string Score::toXML(const int identSize) const {
    // PROFILE_FUNCTION();
    const int numParts = getNumParts();

    std::string xml;

    // ===== HEADER ===== //
    xml.append("<?xml version=\"1.0\" encoding='UTF-8' standalone='no' ?>\n");
    xml.append(
        "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.0 "
        "Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n");
    xml.append("<score-partwise version=\"3.0\">\n");
    xml.append(Helper::generateIdentation(1, identSize) + "<work>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<work-title>" + _title +
               "</work-title>\n");
    xml.append(Helper::generateIdentation(1, identSize) + "</work>\n");
    xml.append(Helper::generateIdentation(1, identSize) + "<identification>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<creator type=\"composer\">" +
               _composerName + "</creator>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<rights>Copyright © </rights>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<encoding>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<encoding-date>2020-08-01</encoding-date>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<encoder>Maialib User</encoder>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<software>Maialib 1.0.0</software>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<software>Direct export, not from Dolet</software>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<encoding-description>Maialib / MusicXML "
               "3.0</encoding-description>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<supports element=\"print\" type=\"yes\" value=\"yes\" "
               "attribute=\"new-system\" />\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<supports element=\"print\" type=\"yes\" value=\"yes\" "
               "attribute=\"new-page\" />\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<supports element=\"accidental\" type=\"yes\" />\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<supports element=\"beam\" type=\"yes\" />\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<supports element=\"stem\" type=\"yes\" />\n");
    xml.append(Helper::generateIdentation(2, identSize) + "</encoding>\n");
    xml.append(Helper::generateIdentation(1, identSize) + "</identification>\n");
    xml.append(Helper::generateIdentation(1, identSize) + "<defaults>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<scaling>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<millimeters>210</millimeters>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<tenths>1200</tenths>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "</scaling>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<page-layout>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<page-height>1697</page-height>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<page-width>1200</page-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<page-margins type=\"both\">\n");
    xml.append(Helper::generateIdentation(4, identSize) + "<left-margin>72</left-margin>\n");
    xml.append(Helper::generateIdentation(4, identSize) + "<right-margin>72</right-margin>\n");
    xml.append(Helper::generateIdentation(4, identSize) + "<top-margin>72</top-margin>\n");
    xml.append(Helper::generateIdentation(4, identSize) + "<bottom-margin>72</bottom-margin>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "</page-margins>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "</page-layout>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<system-layout>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "<system-margins>\n");
    xml.append(Helper::generateIdentation(4, identSize) + "<left-margin>48</left-margin>\n");
    xml.append(Helper::generateIdentation(4, identSize) + "<right-margin>0</right-margin>\n");
    xml.append(Helper::generateIdentation(3, identSize) + "</system-margins>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<system-distance>92</system-distance>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "</system-layout>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<appearance>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"stem\">0.9375</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"beam\">5</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"staff\">0.9375</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"light barline\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"heavy barline\">5</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"leger\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"ending\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"wedge\">1.25</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"enclosure\">0.9375</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"tuplet bracket\">1.25</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"bracket\">5</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"dashes\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"extend\">0.9375</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"octave shift\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"pedal\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"slur middle\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"slur tip\">0.625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"tie middle\">1.5625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<line-width type=\"tie tip\">0.625</line-width>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<note-size type=\"cue\">75</note-size>\n");
    xml.append(Helper::generateIdentation(3, identSize) +
               "<note-size type=\"grace\">60</note-size>\n");
    xml.append(Helper::generateIdentation(2, identSize) + "</appearance>\n");
    xml.append(Helper::generateIdentation(2, identSize) +
               "<music-font font-family=\"Opus Std\" font-size=\"19.8425\" />\n");
    xml.append(Helper::generateIdentation(2, identSize) +
               "<lyric-font font-family=\"Plantin MT Std\" "
               "font-size=\"11.4715\" />\n");
    xml.append(Helper::generateIdentation(2, identSize) + "<lyric-language xml:lang=\"en\" />\n");
    xml.append(Helper::generateIdentation(1, identSize) + "</defaults>\n");

    // ===== PART LIST ===== //
    xml.append(Helper::generateIdentation(1) + "<part-list>\n");
    for (int p = 0; p < numParts; p++) {
        xml.append(Helper::generateIdentation(2, identSize) + "<score-part id=\"P" +
                   std::to_string(p + 1) + "\">\n");
        xml.append(Helper::generateIdentation(3, identSize) + "<part-name>" + _part[p].getName() +
                   "</part-name>\n");
        xml.append(Helper::generateIdentation(3, identSize) + "<part-name-display>\n");
        xml.append(Helper::generateIdentation(4, identSize) + "<display-text>" +
                   _part[p].getName() + "</display-text>\n");
        xml.append(Helper::generateIdentation(3, identSize) + "</part-name-display>\n");
        xml.append(Helper::generateIdentation(3, identSize) + "<part-abbreviation>" +
                   _part[p].getShortName() + "</part-abbreviation>\n");
        xml.append(Helper::generateIdentation(3, identSize) + "<part-abbreviation-display>\n");
        xml.append(Helper::generateIdentation(4, identSize) + "<display-text>" +
                   _part[p].getShortName() + "</display-text>\n");
        xml.append(Helper::generateIdentation(3, identSize) + "</part-abbreviation-display>\n");

        const std::vector<int> midiUnpitched = _part[p].getMidiUnpitched();

        if (_part[p].isPitched()) {
            xml.append(Helper::generateIdentation(3, identSize) + "<score-instrument id=\"P" +
                       std::to_string(p + 1) + "-I1\">\n");
            xml.append(Helper::generateIdentation(4, identSize) + "<instrument-name>" +
                       _part[p].getName() + "</instrument-name>\n");
            xml.append(Helper::generateIdentation(3, identSize) + "</score-instrument>\n");
        } else {
            if (midiUnpitched.size() == 0) {
                xml.append(Helper::generateIdentation(3, identSize) + "<score-instrument id=\"P" +
                           std::to_string(p + 1) + "-I1\">\n");
                xml.append(Helper::generateIdentation(4, identSize) + "<instrument-name>" +
                           _part[p].getName() + "</instrument-name>\n");
                xml.append(Helper::generateIdentation(3, identSize) + "</score-instrument>\n");

                xml.append(Helper::generateIdentation(3, identSize) + "<midi-instrument id=\"P" +
                           std::to_string(p + 1) + "-I1\">\n");
                xml.append(Helper::generateIdentation(4, identSize) + "<midi-unpitched>" +
                           std::to_string(82) + "</midi-unpitched>\n");
                xml.append(Helper::generateIdentation(3, identSize) + "</midi-instrument>\n");
            } else {
                for (const auto& m : midiUnpitched) {
                    xml.append(Helper::generateIdentation(3, identSize) +
                               "<score-instrument id=\"P" + std::to_string(p + 1) + "-I" +
                               std::to_string(m) + "\">\n");
                    xml.append(Helper::generateIdentation(4, identSize) + "<instrument-name>" +
                               _part[p].getName() + "</instrument-name>\n");
                    xml.append(Helper::generateIdentation(3, identSize) + "</score-instrument>\n");
                }

                for (const auto& m : midiUnpitched) {
                    xml.append(Helper::generateIdentation(3, identSize) +
                               "<midi-instrument id=\"P" + std::to_string(p + 1) + "-I" +
                               std::to_string(m) + "\">\n");
                    xml.append(Helper::generateIdentation(4, identSize) + "<midi-unpitched>" +
                               std::to_string(m) + "</midi-unpitched>\n");
                    xml.append(Helper::generateIdentation(3, identSize) + "</midi-instrument>\n");
                }
            }
        }

        xml.append(Helper::generateIdentation(2, identSize) + "</score-part>\n");
    }

    xml.append(Helper::generateIdentation(1, identSize) + "</part-list>\n");

    // ===== NOTES ===== //
    for (int p = 0; p < numParts; p++) {
        xml.append(Helper::generateIdentation(1, identSize) + "<part id=\"P" +
                   std::to_string(p + 1) + "\">\n");
        xml.append(_part[p].toXML(p));
        xml.append(Helper::generateIdentation(1, identSize) + "</part>\n");
    }

    // End of file
    xml.append("</score-partwise>\n");
    return xml;
}

const std::string Score::toJSON() const { return std::string(); }

void Score::toFile(std::string fileName, bool compressedXML, const int identSize) const {
    // Error checking
    if (fileName.empty()) {
        LOG_ERROR("fileName cannot be empty");
    }

    // MusicXML file extensions
    const std::string uncompressExt = ".xml";
    const std::string compressExt = ".mxl";

    // Compressed MusicXML flow
    if (compressedXML) {
        miniz_cpp::zip_file zipFile;
        zipFile.writestr(fileName + uncompressExt, toXML(identSize));
        const std::string fileNameWithExt = fileName + compressExt;

        std::string containerContent;
        containerContent.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        containerContent.append("<container>\n");
        containerContent.append("  <rootfiles>\n");
        // containerContent.append("    <rootfile full-path=\"" + fileName + uncompressExt +
        //                         " \" media-type=\"application/vnd.recordare.musicxml+xml\"/>\n");
        containerContent.append("    <rootfile full-path=\"" + fileName + uncompressExt + "\"/>\n");
        containerContent.append("  </rootfiles>\n");
        containerContent.append("</container>\n");
        zipFile.writestr("META-INF/container.xml", containerContent);

        zipFile.save(fileNameWithExt);
        LOG_INFO("Wrote file: " << fileNameWithExt);
        return;
    }

    // Uncompressed MusicXML flow
    const std::string fileNameWithExt = fileName + uncompressExt;
    std::string fullPath =
        std::filesystem::absolute(fileNameWithExt).string();  // Obtém o caminho absoluto

    std::ofstream buffer(fullPath, std::ofstream::out | std::ofstream::trunc);
    if (!buffer) {
        LOG_ERROR("Unable to open the file " + fullPath + " to write data");
        buffer.close();
        return;
    }

    const std::string& xml = toXML(identSize);
    buffer << xml;
    buffer.close();
    LOG_INFO("Wrote file: " << fullPath);
}

bool Score::isValid(void) const { return _isValidXML; }

bool Score::haveTypeTag(void) const { return _haveTypeTag; }

// int Score::getParts() const
//{
//     return _numParts;
// }

// int Score::getMeasures() const
//{
//     return _numMeasures;
// }

// int Score::countNotes(nlohmann::json& config) const {
//     PROFILE_FUNCTION();
//     // ===== CHECKING THE INPUT ARGUMENTS ===== //
//     // Parts:
//     if (!config.contains("parts")) {
//         LOG_INFO(
//             "'parts' field was not setted. Using the default 'all' "
//             "configuration");
//         config["parts"] = "all";
//     }

//     // Measures:
//     if (!config.contains("measures")) {
//         LOG_INFO(
//             "'measures' field was not setted. Using the default 'all' "
//             "configuration");
//         config["measures"] = "all";
//     }

//     if ((config["measures"] != "all") && (!config["measures"].is_array()) &&
//         (!config["measures"].is_number_integer())) {
//         LOG_ERROR(
//             "The 'measures' field MUST BE 'all', a integer number or an array "
//             "[start end] value!");
//     }

//     // pitchClass:
//     if (!config.contains("pitchClass")) {
//         LOG_INFO(
//             "'pitchClass' field was not setted. Using the default 'all' "
//             "configuration");
//         config["pitchClass"] = "all";
//     }

//     // Octave:
//     if (!config.contains("octave")) {
//         LOG_INFO(
//             "'octave' field was not setted. Using the default 'all' "
//             "configuration");
//         config["octave"] = "all";
//     }

//     // Voice:
//     if (!config.contains("voice")) {
//         LOG_INFO(
//             "'voice' field was not setted. Using the default 'all' "
//             "configuration");
//         config["voice"] = "all";
//     }

//     // Type:
//     if (!config.contains("type")) {
//         LOG_INFO(
//             "'type' field was not setted. Using the default 'all' "
//             "configuration");
//         config["type"] = "all";
//     }

//     // Staff:
//     if (!config.contains("staff")) {
//         LOG_INFO(
//             "'staff' field was not setted. Using the default 'all' "
//             "configuration");
//         config["staff"] = "all";
//     }

//     // ===== XPATH: ROOT ===== //
//     const std::string xPathRoot = "/score-partwise";

//     // ===== XPATH: PART ===== //
//     // Select the desired part:
//     std::string xPathPart;

//     // Set a alias:
//     const auto& partsField = config["parts"];

//     // Case 1: 'all' option:
//     if (partsField == MUSIC_XML::PART::ALL) {
//         xPathPart = "//part";

//         // Case 2: Single string:
//     } else if (partsField.is_string()) {
//         const std::string partName = partsField.get<std::string>();
//         int index = 0;
//         bool found = getPartIndex(partName, &index);

//         if (!found) {
//             printPartNames();
//             LOG_ERROR("This music doesn't have a part called: " + partName);
//             return 0;
//         }

//         xPathPart = "/part[" + std::to_string(index) + "]";

//         // Case 3: List of parts
//     } else if (partsField.is_array()) {
//         std::string positions;
//         const int partsFieldSize = partsField.size();
//         for (int p = 0; p < partsFieldSize; p++) {
//             auto& item = partsField[p];

//             if (!item.is_string()) {
//                 LOG_ERROR("All parts list elements MUST be strings");
//                 return 0;
//             }
//             const std::string partName = item.get<std::string>();

//             int index = 0;
//             bool found = getPartIndex(partName, &index);

//             if (!found) {
//                 printPartNames();
//                 LOG_ERROR("This music doesn't have a part called: " + partName);
//                 return 0;
//             }
//             if (p == 0) {
//                 positions += "position() = " + std::to_string(index);
//             } else {
//                 positions += " or position() = " + std::to_string(index);
//             }
//         }
//         // Concatenate all parts:
//         xPathPart = "/part[" + positions + "]";

//         // Error: None of the above options:
//     } else {
//         LOG_ERROR(
//             "The 'parts' field MUST BE 'all', a string or an array of "
//             "strings!");
//         return 0;
//     }

//     // ===== XPATH: MEASURES ===== //
//     int measureStart = 0;
//     int measureEnd = 0;

//     // Set a alias:
//     const auto& measuresField = config["measures"];

//     // Case 1: 'all' option
//     if (measuresField.is_string() && measuresField == MUSIC_XML::MEASURE::END) {
//         measureStart = 1;
//         measureEnd = getNumMeasures() + 1;

//         // Case 2: Single measure:
//     } else if (measuresField.is_number_integer()) {
//         measureStart = measuresField.get<int>();
//         measureEnd = measuresField.get<int>();

//         // Case 3: Array [start end]
//     } else if (measuresField.is_array() && measuresField.size() == 2) {
//         measureStart = measuresField[0].get<int>();
//         measureEnd = measuresField[1].get<int>();

//         // Error: None of the above options:
//     } else {
//         LOG_ERROR(
//             "The 'measures' field MUST BE 'all' or an array with 2 positive "
//             "interger values: [measureStart, measureEnd]");
//         return nlohmann::json();
//     }

//     // Error checking:
//     if (measureEnd < measureStart) {
//         LOG_ERROR(
//             "In the 'measures' field, the second element MUST BE greater than "
//             "the first one");
//         return nlohmann::json();
//     }

//     // Error checking:
//     if (measureStart == 0 || measureEnd == 0) {
//         LOG_ERROR("The 'measures' array values MUST BE greater then 0");
//         return nlohmann::json();
//     }

//     // Set the measures XPath:
//     const std::string xPathMeasures =
//         "/measure[" + std::to_string(measureStart) +
//         " <= position() and position() <= " + std::to_string(measureEnd) + "]";

//     std::vector<std::string> pitchFilter;
//     std::vector<std::string> noteFilter;

//     // ===== XPATH: STEP, ALTER AND ACCIDENTAL ===== //
//     std::string pitchClass2, pitchStep, alterSymbol;
//     int octave = 0;
//     float alterValue = 0.0f;
//     const std::string pitchClass = config["pitchClass"].get<std::string>();

//     if (pitchClass != MUSIC_XML::PITCH::ALL) {
//         std::string pitch;
//         if (config["octave"].is_number_integer()) {
//             const int tempOctave = config["octave"].get<int>();
//             pitch = pitchClass + std::to_string(tempOctave);

//             Helper::splitPitch(pitch, pitchClass2, pitchStep, octave, alterValue, alterSymbol);

//         } else if (config["octave"].is_string()) {
//             const std::string temp = config["octave"].get<std::string>();
//             if (temp == MUSIC_XML::OCTAVE::ALL_STR) {
//                 pitchStep = pitchClass.substr(0, 1);
//                 alterValue = Helper::alterSymbol2Value(pitchClass.substr(1, pitchClass.size()));
//             } else {
//                 LOG_ERROR("Unknown octave value");
//                 return nlohmann::json();
//             }
//         }
//     }

//     // Get basic pitch class without accident:
//     std::string xPathStep, xPathPitch;
//     if (pitchClass != MUSIC_XML::PITCH::ALL) {
//         xPathStep = "step/text()='" + pitchStep + "'";
//         xPathPitch = "/pitch";

//         pitchFilter.push_back(xPathStep);
//     }

//     // Get accident / alter:
//     std::string xPathPitchAlter, xPathAccidental;
//     // Check is this note is altered or natural:
//     if (alterValue != 0) {
//         // Check if this alter is a quarter-tone alter:
//         if (alterValue != floorf(alterValue)) {
//             // Quarter-tone alter:
//             const std::string alterName = Helper::alterValue2Name(alterValue);
//             xPathAccidental = "accidental/text()='" + alterName + "'";

//             noteFilter.push_back(xPathAccidental);
//         } else {
//             // Standard alter:
//             std::ostringstream streamObj;

//             // Set Fixed-Point Notation
//             streamObj << std::fixed;
//             streamObj << std::setprecision(0);
//             streamObj << floorf(alterValue);

//             const std::string alterStr = streamObj.str();

//             xPathPitchAlter = "alter/text()='" + alterStr + "'";

//             pitchFilter.push_back(xPathPitchAlter);
//         }
//     } else {
//         // Natural note:
//         // The 'pitch' tag cannot have a 'alter' tag:
//         pitchFilter.push_back("not(alter)");
//     }

//     // ===== XPATH: OCTAVE ===== //
//     std::string xPathOctave;
//     if (config["octave"].is_number()) {
//         const int octave = config["octave"].get<int>();
//         if (octave != MUSIC_XML::OCTAVE::ALL) {
//             xPathOctave = "octave/text()='" + std::to_string(octave) + "'";
//             pitchFilter.push_back(xPathOctave);
//         }

//     } else if (config["octave"].is_string()) {
//         const std::string octave = config["octave"].get<std::string>();
//         if (octave != MUSIC_XML::OCTAVE::ALL_STR) {
//             xPathOctave = "octave/text()='" + octave + "'";
//             pitchFilter.push_back(xPathOctave);
//         }
//     }

//     // ===== XPATH: TYPE ===== //
//     std::string xPathType;
//     const std::string type = config["type"].get<std::string>();
//     if (type != MUSIC_XML::NOTE_TYPE::ALL) {
//         xPathType = "type/text()='" + type + "'";
//         noteFilter.push_back(xPathType);
//     }

//     // ===== XPATH: VOICE ===== //
//     std::string xPathVoice;
//     const std::string voice = config["voice"].get<std::string>();
//     if (voice != MUSIC_XML::VOICE::ALL) {
//         xPathVoice = "voice/text()='" + voice + "'";
//         noteFilter.push_back(xPathVoice);
//     }

//     // ===== XPATH: STAFF ===== //
//     std::string xPathStaff;
//     const std::string staff = config["staff"].get<std::string>();
//     if (staff != MUSIC_XML::STAFF::ALL) {
//         xPathStaff = "staff/text()='" + staff + "'";
//         noteFilter.push_back(xPathStaff);
//     }

//     // ===== XPATH: NOTE ===== //
//     std::string xPathNote = "//note";

//     // ===== XPATH: AMOST DONE ===== //
//     std::string xPath = xPathRoot + xPathPart + xPathMeasures + xPathNote;

//     // ===== XPATH: ADD NOTE FILTERS ===== //
//     const int noteFilterSize = noteFilter.size();
//     for (int nf = 0; nf < noteFilterSize; nf++) {
//         if (nf == 0) {
//             xPath += "[" + noteFilter[nf];
//         } else {
//             xPath += " and " + noteFilter[nf];
//         }
//     }

//     if (noteFilter.size() > 0) {
//         xPath += "]";
//     }

//     xPath += xPathPitch;

//     const int pitchFilterSize = pitchFilter.size();
//     for (int f = 0; f < pitchFilterSize; f++) {
//         if (f == 0) {
//             xPath += "[" + pitchFilter[f];
//         } else {
//             xPath += " and " + pitchFilter[f];
//         }
//     }

//     if (pitchFilter.size() > 0) {
//         xPath += "]";
//     }

//     // LOG_DEBUG( "XPath: " << xPath);

//     const pugi::xpath_node_set nodes = _doc.select_nodes(xPath.c_str());

//     return nodes.size();
// }

bool Score::getNote(const int part, const int measure, const int note, std::string& pitch,
                    std::string& step, int& octave, int& duration, int& voice, std::string& type,
                    std::string& steam, int& staff) const {
    // PROFILE_FUNCTION();

    // Create a XPATH pointed to the desired note:
    const std::string xPath = "/score-partwise/part[" + std::to_string(part + 1) + "]/measure[" +
                              std::to_string(measure + 1) + "]/note[" + std::to_string(note + 1) +
                              "]";

    // Try to get the note node:
    const pugi::xml_node node = _doc.select_node(xPath.c_str()).node();

    // Verify if the note exists:
    if (node.empty()) {
        // Note not found!
        return false;
    }

    // Get note data:
    step = node.child("pitch").child_value("step");
    const std::string alterTag = node.child("pitch").child_value("alter");

    std::string alterSymbol;

    if (!alterTag.empty()) {
        switch (hash(alterTag.c_str())) {
            case hash("-2"):
                alterSymbol = "bb";
                break;
            case hash("-1"):
                alterSymbol = "b";
                break;
            case hash("1"):
                alterSymbol = "#";
                break;
            case hash("2"):
                alterSymbol = "x";
                break;
        }
    }

    if (step.empty()) {
        octave = MUSIC_XML::OCTAVE::ALL;
        pitch = MUSIC_XML::PITCH::REST;
    } else {
        octave = atoi(node.child("pitch").child_value("octave"));
        pitch = step + alterSymbol + std::to_string(octave);
    }

    duration = atoi(node.child_value("duration"));
    voice = atoi(node.child_value("voice"));
    type = node.child_value("type");
    steam = node.child_value("steam");
    staff = atoi(node.child_value("staff"));

    return true;
}

bool Score::getNote(const int part, const int measure, const int note, std::string& pitch,
                    std::string& step, int& octave) const {
    // PROFILE_FUNCTION();

    int duration, voice;
    std::string type;
    std::string steam;
    int staff;

    return getNote(part, measure, note, pitch, step, octave, duration, voice, type, steam, staff);
}

bool Score::getNote(const int part, const int measure, const int note, std::string& pitch) const {
    // PROFILE_FUNCTION();
    std::string step;
    int octave, duration, voice;
    std::string type;
    std::string steam;
    int staff;

    return getNote(part, measure, note, pitch, step, octave, duration, voice, type, steam, staff);
}

void Score::getNoteNodeData(const pugi::xml_node& node, std::string& partName, int& measure,
                            std::string& pitch, std::string& pitchClass, std::string& alterSymbol,
                            int& alterValue, int& octave, std::string& type,
                            float& duration) const {
    // PROFILE_FUNCTION();
    // ===== GET PART NAME ===== //
    const std::string partId = node.parent().parent().attribute("id").as_string();
    const int id = atoi(partId.substr(1, partId.size()).c_str());
    partName = getPartName(id);

    // ===== GET MEASURE ===== //
    measure = static_cast<int>(node.parent().attribute("number").as_int());

    // ===== GET ACCIDENTAL ===== //
    const std::string pitchAlter = node.child("pitch").child_value("alter");
    const std::string pitchAccidental = node.child_value("accidental");

    // Catch the quarter-tone accidental:
    if (!pitchAccidental.empty()) {
        alterSymbol = Helper::alterName2symbol(pitchAccidental);
        // Catch a standard acidental:
    } else if (!pitchAlter.empty()) {
        alterValue = atoi(pitchAlter.c_str());
        alterSymbol = Helper::alterValue2symbol(alterValue);
    }

    // ===== GET PITCH CLASS ===== //
    const std::string step = node.child("pitch").child_value("step");
    pitchClass = step + alterSymbol;

    // ===== GET OCTAVE AND PITCH ===== //
    if (pitchClass.empty()) {
        octave = MUSIC_XML::OCTAVE::ALL;
        pitchClass = MUSIC_XML::PITCH::REST;
        pitch = MUSIC_XML::PITCH::REST;
    } else {
        octave = atoi(node.child("pitch").child_value("octave"));
        pitch = pitchClass + std::to_string(octave);
    }

    // ===== GET DURATION ===== //
    duration = atof(node.child_value("duration"));

    // ===== GET TYPE ===== //
    type = node.child_value("type");
    if (type.empty()) {
        const int divisions = atoi(node.parent().child("attributes").child_value("divisions"));
        if (divisions <= 0) {
            LOG_ERROR("Unable to get the 'divisionsPerQuarterNote' value");
        }

        LOG_DEBUG("Nyck 02: " + std::to_string(duration) + " " + std::to_string(divisions));
        type = Helper::ticks2noteType(duration, divisions).first;
    }
}

// nlohmann::json Score::selectNotes(nlohmann::json& config) const {
//     PROFILE_FUNCTION();

//     // ===== XPATH: ROOT ===== //
//     const std::string xPathRoot = "/score-partwise";

//     // ===== XPATH: PART ===== //
//     // Select the desired part:
//     std::string xPathPart;

//     // Set a alias:
//     const auto& partsField = config["parts"];

//     // Case 1: 'all' option:
//     if (partsField == MUSIC_XML::PART::ALL) {
//         xPathPart = "//part";

//         // Case 2: Single string:
//     } else if (partsField.is_string()) {
//         const std::string partName = partsField.get<std::string>();
//         int index = 0;
//         bool found = getPartIndex(partName, &index);

//         if (!found) {
//             printPartNames();
//             LOG_ERROR("This music doesn't have a part called: " + partName);
//             return 0;
//         }

//         xPathPart = "/part[" + std::to_string(index) + "]";

//         // Case 3: List of parts
//     } else if (partsField.is_array()) {
//         std::string positions;
//         const int partsFieldSize = partsField.size();
//         for (int p = 0; p < partsFieldSize; p++) {
//             auto& item = partsField[p];

//             if (!item.is_string()) {
//                 LOG_ERROR("All parts list elements MUST be strings");
//                 return nlohmann::json();
//             }
//             const std::string partName = item.get<std::string>();

//             int index = 0;
//             bool found = getPartIndex(partName, &index);

//             if (!found) {
//                 printPartNames();
//                 LOG_ERROR("This music doesn't have a part called: " + partName);
//                 return nlohmann::json();
//             }

//             if (p == 0) {
//                 positions += "position() = " + std::to_string(index);
//             } else {
//                 positions += " or position() = " + std::to_string(index);
//             }
//         }
//         // Concatenate all parts:
//         xPathPart = "/part[" + positions + "]";

//         // Error: None of the above options:
//     } else {
//         LOG_ERROR(
//             "The 'parts' field MUST BE 'all', single string or a string "
//             "array!");
//         return nlohmann::json();
//     }

//     // ===== XPATH: MEASURES ===== //
//     int measureStart = 0;
//     int measureEnd = 0;

//     // Set a alias:
//     const auto& measuresField = config["measures"];

//     // Case 1: 'all' option
//     if (measuresField.is_string() && measuresField == MUSIC_XML::MEASURE::END) {
//         measureStart = 1;
//         measureEnd = getNumMeasures() + 1;

//         // Case 2: Single measure:
//     } else if (measuresField.is_number_integer()) {
//         measureStart = measuresField.get<int>();
//         measureEnd = measuresField.get<int>();

//         // Case 3: Array [start end]
//     } else if (measuresField.is_array() && measuresField.size() == 2) {
//         measureStart = measuresField[0].get<int>();
//         measureEnd = measuresField[1].get<int>();

//         // Error: None of the above options:
//     } else {
//         LOG_ERROR(
//             "The 'measures' field MUST BE 'all' or an array with 2 positive "
//             "integers [measureStart, measureEnd]");
//         return nlohmann::json();
//     }

//     // Error checking:
//     if (measureEnd < measureStart) {
//         LOG_ERROR(
//             "In the 'measures' field, the second element MUST BE greater than "
//             "the first one!");
//         return nlohmann::json();
//     }

//     // Error checking:
//     if (measureStart == 0 || measureEnd == 0) {
//         LOG_ERROR("The 'measures' [start end] array values MUST BE greater then 0");
//         return nlohmann::json();
//     }

//     // Set the measures XPath:
//     const std::string xPathMeasures =
//         "/measure[" + std::to_string(measureStart) +
//         " <= position() and position() <= " + std::to_string(measureEnd) + "]";

//     if (!config.contains("melodicOnly")) {
//         config["melodicOnly"] = true;
//     }

//     bool melodicOnly = config["melodicOnly"].get<bool>();

//     // ===== XPATH: NOTE ===== //
//     std::string xPathNote = "/note";

//     // ===== XPATH: ONLY MELODIC ===== //
//     std::string noteAttrFilter;
//     if (melodicOnly) {
//         noteAttrFilter += "[not(chord)]";
//     }

//     // Set the XPATH:
//     const std::string xPath = xPathRoot + xPathPart + xPathMeasures + xPathNote + noteAttrFilter;

//     const pugi::xpath_node_set musicNotes = _doc.select_nodes(xPath.c_str());

//     const int musicNotesSize = musicNotes.size();

//     nlohmann::json result;

//     std::string partName, musicPitch, musicPitchClass, alterSymbol, musicType;
//     int measure = 0;
//     int alterValue = 0;
//     int musicOctave = 0;
//     float duration = 0.0f;
//     // For each filtered music note:
//     for (int i = 0; i < musicNotesSize; i++) {
//         const pugi::xml_node& node = musicNotes[i].node();

//         partName = "";
//         musicPitch = "";
//         musicPitchClass = "";
//         alterSymbol = "";
//         musicType = "";
//         measure = 0;
//         alterValue = 0;
//         musicOctave = 0;
//         duration = 0.0f;

//         getNoteNodeData(node, partName, measure, musicPitch, musicPitchClass, alterSymbol,
//                         alterValue, musicOctave, musicType, duration);

//         // // ===== GET MEASURE ===== //
//         // const int measure =
//         // static_cast<int>(node.parent().attribute("number").as_int());

//         // // ===== GET MUSIC NOTES PITCH CLASS ===== //
//         // std::string musicPitchClass =
//         // node.child("pitch").child_value("step");

//         // // ===== GET MUSIC NOTES OCTAVE ===== //
//         // int musicOctave = 0;
//         // if (musicPitchClass.empty()) {
//         //     musicOctave = MUSIC_XML::OCTAVE::ALL;
//         //     musicPitchClass = MUSIC_XML::PITCH::REST;
//         // } else {
//         //     musicOctave = atoi(node.child("pitch").child_value("octave"));
//         // }

//         // // ===== GET MUSIC NOTES TYPE ===== //
//         // std::string musicType;
//         // musicType = node.child_value("type");
//         // if(musicType.empty()) {
//         //     const float musicDuration = atof(node.child_value("duration"));
//         //     const int divisionsPerQuarterNote = getDivisionsPerQuarterNote();
//         //     musicType = duration2noteType(musicDuration,
//         //     divisionsPerQuarterNote);
//         // }

//         // // ===== GET PART NAME ===== //
//         // const std::string partId =
//         // node.parent().parent().attribute("id").as_string(); const int id =
//         // atoi(partId.substr(1, partId.size()).c_str()); const std::string
//         // partName = getPartName(id);

//         // ===== APPEND THE RESULT ===== //
//         nlohmann::json outputLine;
//         outputLine["partName"] = partName;
//         outputLine["measure"] = measure;
//         outputLine["pitch"] = musicPitch;
//         outputLine["octave"] = musicOctave;
//         outputLine["type"] = musicType;

//         result.push_back(outputLine);
//     }

//     return result;
// }

void Score::printPartNames() const {
    for (const auto& part : _part) {
        std::cout << "P" << part.getPartIndex() << ": " << part.getName() << std::endl;
    }
}

// const std::map<int, std::string> Score::getPartsName() const { return _partsName; }

const std::string Score::getPartName(const int partId) const {
    const int partSize = _part.size();

    if (partId < 0 || partId > (partSize - 1)) {
        LOG_ERROR("partId out of range error: " + std::to_string(partId));
    }

    return _part.at(partId).getName();
}

bool Score::getPartIndex(const std::string& partName, int* index) const {
    // PROFILE_FUNCTION();

    bool foundIndex = false;

    const int partSize = _part.size();
    for (int idx = 0; idx < partSize; idx++) {
        if (_part.at(idx).getName() == partName) {
            foundIndex = true;
            *index = idx;
        }
    }

    if (!foundIndex) {
        LOG_ERROR("Unable to find '" + partName + "' inside this score");
    }

    return foundIndex;
}

Score::MelodyPatternTable Score::findMelodyPattern(
    const std::vector<Note>& melodyPattern, const float totalIntervalsSimilarityThreshold,
    const float totalRhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
    const std::function<float(float, float)> totalSimilarityCallback) const {
    const int totalNumNotes = getNumNotes();
    const int melodyPatternSize = melodyPattern.size();

    if (melodyPatternSize > totalNumNotes) {
        LOG_ERROR("The melody pattern is bigger than the score");
    }

    MelodyPatternTable resultTable;
    resultTable.reserve(totalNumNotes - melodyPatternSize);

    // Container em memória para armazenar informações de notas
    struct NoteEvent {
        std::string partName;
        int measureIdx;
        int staveIdx;
        int noteIdx;
        const std::string keyName;
        const Note* notePtr;
    };
    // ===== STEP 1: COLETAR TODAS AS NOTAS DA PARTITURA ===== //
    for (int partIdx = 0; partIdx < getNumParts(); partIdx++) {
        std::vector<NoteEvent> noteEvents;
        const int NUM_NOTES_PER_MEASURE = 16;
        noteEvents.reserve(_part[partIdx].getNumMeasures() * NUM_NOTES_PER_MEASURE);

        const Part& currentPart = _part[partIdx];
        const std::string& currentPartName = currentPart.getName();
        for (int measureIdx = 0; measureIdx < currentPart.getNumMeasures(); measureIdx++) {
            const Measure& currentMeasure = currentPart.getMeasure(measureIdx);
            const int numStaves = currentMeasure.getNumStaves();
            for (int staveIdx = 0; staveIdx < numStaves; staveIdx++) {
                const int numNotes = currentMeasure.getNumNotes(staveIdx);
                for (int noteIdx = 0; noteIdx < numNotes; noteIdx++) {
                    const Note& currentNote = currentMeasure.getNote(noteIdx, staveIdx);

                    // Skip all chords and multiple voices! To fix in the future
                    // Get only the top melody of each instrument/stave
                    if (currentNote.inChord() || currentNote.getVoice() != 1) {
                        continue;
                    }
                    const std::string& currentKeyName = currentMeasure.getKey().getName();
                    // Armazena o evento da nota em memória
                    noteEvents.push_back(
                        {currentPartName, measureIdx, staveIdx, noteIdx, currentKeyName, &currentNote});
                }
            }
        }

        const int patternMaxIterations = noteEvents.size() - melodyPatternSize;
        for (int i = 0; i < patternMaxIterations; i++) {
            // Extrai o segmento para comparação com o padrão
            std::vector<Note> segment;
            segment.reserve(melodyPatternSize);
            for (int offset = 0; offset < melodyPatternSize; offset++) {
                const Note& note = *noteEvents[i + offset].notePtr;
                segment.push_back(note);
            }


            // ==== COMPUTE THE TRANSPOSE SEMITONES ===== //
            const Note* patternFirstNoteOn = nullptr;
            bool melodyContainANoteOn = false;
            for (int i = 0; i < melodyPatternSize; i++) {
                if (melodyPattern[i].isNoteOff()) {
                    continue;
                }

                melodyContainANoteOn = true;
                patternFirstNoteOn = &melodyPattern[i];
                break;
            }

            const Note* segmentFirstNoteOn = nullptr;
            bool segmentContainANoteOn = false;
            for (int i = 0; i < melodyPatternSize; i++) {
                if (segment[i].isNoteOff()) {
                    continue;
                }

                segmentContainANoteOn = true;
                segmentFirstNoteOn = &segment[i];
                break;
            }

            // const int patternFirstNoteMidi = patternFirstNoteOn->getMIDINumber();
            // const int segmentFirstNoteMidi = segmentFirstNoteOn->getMIDINumber();
            // const int transposeSemitones = segmentFirstNoteMidi - patternFirstNoteMidi;

            std::string intervalName;
            if (melodyContainANoteOn && segmentContainANoteOn) {
                const std::string& patternFirstSoundingPitch = patternFirstNoteOn->getSoundingPitch();
                const std::string& segmentFirstSoundingPitch = segmentFirstNoteOn->getSoundingPitch();

                const Interval transposeInterval(patternFirstSoundingPitch, segmentFirstSoundingPitch);
                intervalName = transposeInterval.getName() + " " + transposeInterval.getDirection();
            }

            const std::vector<float> semitonesDiff =
                (intervalsSimilarityCallback == nullptr)
                    ? Helper::getSemitonesDifferenceBetweenMelodies(melodyPattern, segment)
                    : intervalsSimilarityCallback(melodyPattern, segment);

            const std::vector<float> durationDiff =
                (rhythmSimilarityCallback == nullptr)
                    ? Helper::getDurationDifferenceBetweenRhythms(melodyPattern, segment)
                    : rhythmSimilarityCallback(melodyPattern, segment);

            // Calcula as similaridades
            float totalIntervalSimilarity = -1.0f;
            float totalRhythmSimilarity = -1.0f;
            if (intervalsSimilarityCallback == nullptr) {
                totalIntervalSimilarity = Helper::calculateMelodyEuclideanSimilarity(semitonesDiff);
            } else {
                totalIntervalSimilarity = totalIntervalSimilarityCallback(semitonesDiff);
            }

            if (rhythmSimilarityCallback == nullptr) {
                totalRhythmSimilarity = Helper::calculateRhythmicEuclideanSimilarity(durationDiff);
            } else {
                totalRhythmSimilarity = totalRhythmSimilarityCallback(durationDiff);
            }

            // Verifica se as similaridades estão acima dos limites
            if (totalIntervalSimilarity < totalIntervalsSimilarityThreshold ||
                totalRhythmSimilarity < totalRhythmSimilarityThreshold) {
                continue;
            }
            // Calcula a similaridade total usando o callback personalizado
            float totalSimilarity = -1.0f;
            if (totalSimilarityCallback == nullptr) {
                totalSimilarity = (totalIntervalSimilarity + totalRhythmSimilarity) / 2.0f;
            } else {
                totalSimilarity =
                    totalSimilarityCallback(totalIntervalSimilarity, totalRhythmSimilarity);
            }

            std::vector<std::string> segmentPitchList(segment.size());
            for (int p = 0; p < (int)segment.size(); p++) {
                segmentPitchList[p] = segment[p].getWrittenPitch();
            }

            // Armazena o resultado
            MelodyPatternRow row(currentPartName,
                                 noteEvents[i].measureIdx,  // Número do compasso
                                 noteEvents[i].staveIdx,    // ID da clave
                                 noteEvents[i].keyName,         // Tonalidade do compasso
                                 intervalName, // Intervalo de transposição
                                 segmentPitchList, // Lista de pitchs do segmento
                                 semitonesDiff,  // Lista de diferenças intervalares em semitons
                                 durationDiff,   // Lista de similaridade rítmica
                                 totalIntervalSimilarity,  // Similaridade intervalar total
                                 totalRhythmSimilarity,    // Similaridade rítmica total
                                 totalSimilarity           // Similaridade total
            );

            resultTable.push_back(row);
        }
    }

    return resultTable;
}

bool Score::haveAnacrusisMeasure() const { return _haveAnacrusisMeasure; }

int Score::xPathCountNodes(const std::string& xPath) const {
    // Select all nodes from the given XPath:
    const pugi::xpath_node_set nodes = _doc.select_nodes(xPath.c_str());

    // Compute the number of nodes:
    return nodes.size();
}

void Score::setRepeat(int measureStart, int measureEnd) {
    // INPUT ARGUMENTS VALIDATION
    measureStart = (measureStart < 0) ? 0 : measureStart;
    measureEnd =
        (measureEnd < 0 || measureEnd > (_numMeasures - 1)) ? _numMeasures - 1 : measureEnd;

    // Error checking
    if (measureEnd == 0) {
        LOG_ERROR("'measureEnd' MUST BE greater than 0");
    }

    // For each part: set repeat barlines
    for (auto& part : _part) {
        part.getMeasure(measureStart).setRepeatStart();
        part.getMeasure(measureEnd).setRepeatEnd();
    }
}

void Score::forEachNote(
    std::function<void(Part* part, Measure* measure, int staveId, Note* note)> callback,
    int measureStart, int measureEnd, std::vector<std::string> partNames) {
    // PROFILE_FUNCTION();

    // Set the correct values of the measure end input argument
    measureEnd = (measureEnd < 0) ? getNumMeasures() : measureEnd;

    std::vector<int> selectedParts;

    if (partNames.empty()) {
        // selectedParts.resize(getNumParts());
        // // Fill the 'selectParts' vector with all part indexes sequence
        // std::iota(std::begin(selectedParts), std::end(selectedParts), 0);

        for (int i = 0; i < getNumParts(); i++) {
            selectedParts.push_back(i);
        }

    } else {
        for (const auto& partName : partNames) {
            int partIdx = 0;
            const bool isValid = getPartIndex(partName, &partIdx);
            if (!isValid) {
                LOG_ERROR("Invalid part name: " + partName);
                return;
            }
            selectedParts.push_back(partIdx);
        }
    }

    for (auto& p : selectedParts) {
        Part& currentPart = getPart(p);

        for (int m = measureStart; m < measureEnd; m++) {
            Measure& currentMeasure = currentPart.getMeasure(m);

            for (int s = 0; s < currentMeasure.getNumStaves(); s++) {
                const int numNotes = currentMeasure.getNumNotes(s);

                for (int n = 0; n < numNotes; n++) {
                    Note& currentNote = currentMeasure.getNote(n, s);

                    callback(&currentPart, &currentMeasure, s, &currentNote);
                }
            }
        }
    }
}

nlohmann::json Score::instrumentFragmentation(nlohmann::json config) {
    // ===== STEP 1: PARSE THE INPUT CONFIG JSON ===== //

    // ===== STEP 1.0: READ PART NAMES ===== //
    std::vector<std::string> partNames;

    // Type checking
    if (config.contains("partNames") && !config["partNames"].is_array()) {
        printPartNames();
        LOG_ERROR(
            "'partNames' is a optional config argument and MUST BE a strings "
            "array");
        return {};
    }

    // If not setted, set the default value = "all part names"
    if (!config.contains("partNames")) {
        partNames = getPartsNames();
    } else {
        for (const auto& partNameValue : config["partNames"]) {
            const std::string partName = partNameValue.get<std::string>();
            int idx = 0;
            bool isValid = getPartIndex(partName, &idx);

            if (!isValid) {
                LOG_ERROR("Invalid part name: " + partName);
                printPartNames();
                return {};
            }

            partNames.push_back(partName);
        }
    }

    // ===== STEP 1.1: READ MEASURE START ===== //
    int measureStart = 0;

    // If not setted, set the default value = 0
    if (!config.contains("measureStart")) {
        measureStart = 0;
        config["measureStart"] = measureStart;
        // LOG_WARN("Setting the 'measureStart' to the first measure: " <<
        // measureStart);
    }

    // Type checking
    if (config.contains("measureStart") && !config["measureStart"].is_number_integer()) {
        LOG_ERROR(
            "'measureStart' is a optional config argument and MUST BE a "
            "positive integer!");
        return {};
    }

    // Get measure start value
    measureStart = config["measureStart"].get<int>();

    // Error checking:
    if (measureStart < 0) {
        LOG_ERROR("The 'measureStart' value MUST BE a positive integer!");
        return {};
    }

    // ===== STEP 1.2: READ MEASURE END ===== //
    int measureEnd = 0;

    // If not setted, set the default value = All measures!
    if (!config.contains("measureEnd")) {
        measureEnd = getNumMeasures();
        config["measureEnd"] = measureEnd;
        // LOG_WARN("Setting the 'measureEnd' to the last measure: " +
        // std::to_string(measureEnd));
    }

    // Type checking:
    if (config.contains("measureEnd") && !config["measureEnd"].is_number_integer()) {
        LOG_ERROR(
            "'measureEnd' is a optional config argument and MUST BE a positive "
            "integer!");
        return {};
    }

    // Get the 'measureEnd' config value:
    measureEnd = config["measureEnd"].get<int>();

    // Error checking:
    if (measureEnd < 0) {
        LOG_ERROR("The 'measureEnd' value MUST BE greater than 0!");
        return {};
    }

    // Error checking:
    if (measureEnd > getNumMeasures()) {
        LOG_WARN("The 'measureEnd' value is greater then the music length!");
        LOG_WARN("Changing the 'measureEnd' value to: " << getNumMeasures());
        measureEnd = getNumMeasures();
    }

    // Error checking:
    if (measureStart > measureEnd) {
        LOG_ERROR("'measureEnd' value MUST BE greater than 'measureStart' value");
        return {};
    }

    // ===== STEP 2: FILL THE JSON OUTPUT WITH DATA ===== //
    // The output JSON to be filled inside the for loop below
    nlohmann::json out;

    const int instrumentCount = partNames.size();
    for (int i = 0; i < instrumentCount; i++) {
        //  // Selection of objects via XPath
        const std::string xPathRoot = "/score-partwise";  // Selects the Score
        const std::string xPathPart =
            "/part[" + std::to_string(i + 1) + "]";  // Selects the Instrument (or Voice)
        // Selects the Initial and Final Measures (that is, a Section) for
        // Analysis
        const std::string xPathMeasureSection =
            "/measure[" + std::to_string(measureStart) +
            " <= position() and position() <= " + std::to_string(measureEnd) + "]";
        const std::string xPathNote = "//note";              // Selects all notes in the Section
        const std::string xPathFilterNote = "[not(grace)]";  // This makes grace notes not be
                                                             // considered to time calculations
        const std::string xPath = xPathRoot + xPathPart + xPathMeasureSection + xPathNote +
                                  xPathFilterNote;  // Concatenation of the all partial paths to
                                                    // that one of interest
        // const pugi::xpath_node_set notes = _doc.select_nodes(xPath.c_str());
        // // score-partwise is the root of musicxml // .c_str() because it is C
        // coded

        const pugi::xpath_node_set notes =
            Helper::getNodeSet(_doc, xPath);  // this is a vector, see the documentation Pugi

        // const std::string xPathWork = "/score-partwise/work/work-title";
        // const pugi::xpath_node_set works = _doc.select_nodes(xPathWork.c_str());
        // const std::string workTitle = works[0].node().text().as_string();
        const std::string workTitle = getTitle();

        // const std::string xPathAuthor = "/score-partwise/identification/creator";
        // const pugi::xpath_node_set authors = _doc.select_nodes(xPathAuthor.c_str());
        // // const std::string author = authors[0].node().text().as_string();
        // const std::string author = "Adolfo";
        const std::string author = getComposerName();

        // ------Defintions of global variables to be used in the Algorithm

        const int maxNotes = notes.size();                // all indexes run on up to this
                                                          // number of notes in the Section
        std::vector<std::string> duration_vec(maxNotes);  // vector of durations; this is a vector
                                                          // of strings yet displaying  durations
        std::vector<std::string> pitch_vec(maxNotes);     // vector of pitches
        std::vector<int> duration_vec_value(maxNotes,
                                            0);  // vector of numerical value of durations after
                                                 // tranform: "string" to a positive integer (int)

        for (int n = 0; n < maxNotes; n++) {
            pugi::xml_node note = notes[n].node();  // note is a sintax sugar (temporary variable)
                                                    // of the i-th note

            // Define variables: atributes, values using the nodes "note"
            std::string pitch = note.child("pitch")
                                    .child("step")
                                    .text()
                                    .as_string();  // get the pitch of i-th note (a text as a
                                                   // string). A rest has value 0.
            std::string duration =
                note.child_value("duration");  // get the duration of i-th note as a number

            // // ------Loading Vectors with the variables defined as attributes
            // and values of the nodes (notes or measures, etc)

            duration_vec[n] = duration;  // load duration vector (as strings)
            pitch_vec[n] = pitch;        // load pitch vector
            duration_vec_value[n] =
                stoi(duration);  // stoi = transforms a number string to an integer

            const pugi::xpath_node_set x = note.select_nodes("chord");
            // LOG_DEBUG("ischord[" << i << "] " << x.size()); // ???????
        }

        // //------------- Here starts definitions of several auxiliary
        // functions to compute Activation Rate----------

        std::vector<int> duration_activations(maxNotes, 0);

        for (int i = 0; i < maxNotes; i++) {
            if (pitch_vec[i].size() != 0) {  // pitch has value 0 for rests (????)
                duration_activations[i] = duration_vec_value[i];
            } else {
                duration_activations[i] = 0;
            }
        }

        // this is the total duration of activation
        const int sum_duration_activations =
            std::accumulate(duration_activations.begin(), duration_activations.end(), 0);

        // this is the total duration of the entire segment without signs
        const int sum_total_duration =
            std::accumulate(duration_vec_value.begin(), duration_vec_value.end(), 0);

        // this is the rate of activation
        const float instrument_activation_rate =
            static_cast<float>(sum_duration_activations) / static_cast<float>(sum_total_duration);

        // Counting Number of Activations
        std::vector<int> get_sign(maxNotes,
                                  0);  // This vector has note = 1 and rest = -1
        for (int i = 0; i < maxNotes; i++) {
            if (pitch_vec[i].size() != 0) {
                get_sign[i] = 1;
            } else {
                get_sign[i] = -1;
            }
        }

        std::vector<int> activations_vec(maxNotes, 0);  // gives a vecto withh transitions (0-1)
                                                        // from no activations to activations
        for (int i = 1; i < maxNotes; i++) {
            if ((get_sign[i] - get_sign[i - 1]) > 0) {
                activations_vec[i] = 1;
            } else {
                activations_vec[i] = 0;
            }
        }

        if (get_sign[maxNotes - 1] == -1) {  // exception for the last element
            activations_vec[maxNotes - 1] = 1;
        }

        int activations_number = std::accumulate(activations_vec.begin(), activations_vec.end(), 0);

        //****************** End of the code to calculate Activation Rate
        //*************************

        //****************** Code to Plot Activation Function of an Instrument
        //***************************

        const std::string xPathDivisions =
            "/score-partwise/part[1]/measure[1]/attributes/divisions";  // The
                                                                        // division
                                                                        // information
                                                                        // is in
                                                                        // Measure
                                                                        // 1

        const std::string xPathBeatNumber =
            "/score-partwise/part[1]/measure[1]/attributes/time/beats";  // The
                                                                         // beat
                                                                         // Number
                                                                         // information
                                                                         // is
                                                                         // in
                                                                         // Measure
                                                                         // 1

        const std::string xPathBeatType =
            "/score-partwise/part[1]/measure[1]/attributes/time/"
            "beat-type";  // beat type not used

        const pugi::xpath_node_set divisions =
            _doc.select_nodes(xPathDivisions.c_str());  // coded inC

        const pugi::xpath_node_set beatNumber =
            _doc.select_nodes(xPathBeatNumber.c_str());  // score-partwise is the root of musicxml
                                                         // // .c_str() because it is coded in C

        const pugi::xpath_node_set beatType =
            _doc.select_nodes(xPathBeatType.c_str());  // coded in C

        if (beatNumber.size() < 1) {
            LOG_ERROR("beatNumber is empty");
            return nlohmann::json();
        }

        if (divisions.size() < 1) {
            LOG_ERROR("divisions is empty");
            return nlohmann::json();
        }

        const int beatNumber_value =
            std::stoi(beatNumber[0].node().text().as_string());  // number of beats per
                                                                 // measure
        const int divisions_value = std::stoi(divisions[0].node().text().as_string());
        const int beatType_value = std::stoi(beatType[0].node().text().as_string());

        std::vector<float> duration_in_beats(maxNotes,
                                             0);  // duration of notes in beats
        for (int i = 0; i < maxNotes; i++) {
            duration_in_beats[i] =
                static_cast<float>(duration_vec_value[i]) / static_cast<float>(divisions_value);
        }

        std::vector<float> accumulate_duration_vec(
            maxNotes, 0);  // vector of accumulated sum of durations (in beats)
        for (int i = 0; i < maxNotes; i++) {
            if (i == 0) {
                accumulate_duration_vec[0] = duration_in_beats[0];
            } else {
                accumulate_duration_vec[i] = accumulate_duration_vec[i - 1] + duration_in_beats[i];
            }
        }

        accumulate_duration_vec.insert(accumulate_duration_vec.begin(),
                                       0);  // needs a initial zero for our algorithm

        std::vector<int> activation_sign(maxNotes,
                                         0);  // This vector has rest = 0
        for (int i = 0; i < maxNotes; i++) {
            if (get_sign[i] < 0) {
                activation_sign[i] = 0;
            } else {
                activation_sign[i] = get_sign[i];
            }
        }

        std::vector<std::vector<std::tuple<float, float>>> lines(
            activation_sign.size());  // Set (vector) of activated lines segments;
                                      // structure lines[k] = [a(k), b(k)]
        std::vector<std::vector<std::tuple<float, float>>> lines_rests(
            activation_sign.size());  // Set (vector) of rests lines segments

        float durBeatType = 4.0f / static_cast<float>(beatType_value);
        float durMeasureBeats =
            static_cast<float>(beatNumber_value) * static_cast<float>(durBeatType);
        float initialSegment_beats =
            static_cast<float>(measureStart - 1) *
            static_cast<float>(durMeasureBeats);  // Initial Segment before the first
                                                  // measure considered

        for (int k = 0; k < static_cast<int>(activation_sign.size()); k++) {
            std::vector<std::tuple<float, float>> temp1(
                2);  // This loads the activated lines (only a collection of
                     // points; the lines are constructed in Python with
                     // LinesCollection plot)
            // if(activation_sign[k] != 0) {
            temp1[0] =
                std::make_tuple(accumulate_duration_vec[k] + initialSegment_beats,
                                (i + 1) * activation_sign[k]);  // tuple = (x-value of the initial
                                                                // point of the line in beats, y =
                                                                // height = activation of the i-th
                                                                // instrument)
            temp1[1] =
                std::make_tuple(accumulate_duration_vec[k + 1] + initialSegment_beats,
                                (i + 1) * activation_sign[k]);  // tuple = (x-value of the final
                                                                // point of the line in beats,
                                                                // y= height = activation)
            lines[k] = temp1;  // lines of activated notes segments are defined

            std::vector<std::tuple<float, float>> temp2(2);  // This loads the lines_rests
            temp2[0] = std::make_tuple(
                accumulate_duration_vec[k] + initialSegment_beats,
                (i + 1) * (1 - activation_sign[k]));  // activation_sign/level of the
                                                      // i-th instrument; activation
                                                      // of lines_rest = 1 -
                                                      // activation_sign of notes
            temp2[1] = std::make_tuple(accumulate_duration_vec[k + 1] + initialSegment_beats,
                                       (i + 1) * (1 - activation_sign[k]));
            lines_rests[k] = temp2;  // lines of rests segments are defined
        }

        std::vector<std::vector<std::tuple<float, float>>>
            lines2;  // this container is temporary: contains only the lines
                     // with positive numbers in the tuples ( extracted from
                     // lines)
        for (int k = 0; k < static_cast<int>(lines.size()); k++) {
            if (activation_sign[k])  // activation_sign acts as a boolean vector
                                     // and extract from lines only the elements
                                     // (a.b) with b positive (non zero)
                lines2.push_back(lines[k]);
        }

        lines = lines2;  // return to container "lines" now only with positive
                         // numbers in the tuples

        std::vector<std::vector<std::tuple<float, float>>>
            lines_rests2;  // this container is temporary: contains only the
                           // lines with positive numbers in the tuples (
                           // extracted from lines_rests)
        for (int k = 0; k < static_cast<int>(lines_rests.size()); k++) {
            if ((1 - activation_sign[k]))  // activation_sign acts as a boolean
                                           // vector and extract from
                                           // lines_rests only the elements
                                           // (a.b) with b positive (non zero)
                lines_rests2.push_back(lines_rests[k]);
        }

        lines_rests = lines_rests2;  // return to container "lines" now only
                                     // with positive numbers in the tuples

        // ----------Display lines vectors in a terminal just to check the code
        // LOG_DEBUG("Lines to plot in Python: Instrument:" << i+1);
        // LOG_DEBUG("lines size = " << lines.size());
        // LOG_DEBUG("lines_rests size = " << lines_rests.size());

        for (int k = 0; k < static_cast<int>(lines.size()); k++) {
            // float a1 = std::get<0>(lines[k][0]); //Initial beat of the
            // segment float b1 = std::get<1>(lines[k][0]);  // Activation Sign
            // of the segment ( = 1, 2, 3, ...) float c1 =
            // std::get<0>(lines[k][1]); // Final beat of the segment float d1 =
            // std::get<1>(lines[k][1]);  // Activation Sign of the segment ( =
            // 1)

            // LOG_DEBUG("lines[" << k << "] = " << a1 << "| " << b1  << " | "
            // << c1 << " | " << d1);
        }

        //        ------Ploting Rests Activation-------------

        for (int k = 0; k < static_cast<int>(lines_rests.size()); k++) {
            // float a2 = std::get<0>(lines_rests[k][0]); //Initial beat of the
            // segment float b2 = std::get<1>(lines_rests[k][0]);  // Activation
            // Sign of the segment ( = 1, 2, 3...) float c2 =
            // std::get<0>(lines_rests[k][1]); // Final beat of the segment
            // float d2 = std::get<1>(lines_rests[k][1]);  // Activation Sign of
            // the segment (=  1)

            // LOG_DEBUG("lines_rests[" << k << "] = " << a2 << "| " << b2  << "
            // | " << c2 << " | " << d2);
        }

        //--------JSON to store the output of the i-th instrument
        /* This allows parameters defined below by json to be read from a
         Pyhthon script through "pybind11" (after the compilation of the
         program" */

        nlohmann::json temp;
        temp["Activation Rate"] = instrument_activation_rate;
        temp["Number of Activations"] = activations_number;
        temp["lines"] = lines;
        temp["lines_rests"] = lines_rests;

        if (partNames[i].empty()) {             // Work title
            temp["partName"] = "no Part Name";  // return a "null" name if work
                                                // doesn't have a wortitle
        } else {
            temp["partName"] = partNames[i];
        }

        // meujson["color"] = std::make_tuple(1,0, 1,1);
        temp["measureStart"] = measureStart;
        temp["measureEnd"] = measureEnd;
        temp["beatNumber"] = beatNumber_value;
        temp["beatType"] = beatType_value;

        if (workTitle.empty() == true) {
            temp["work"] = "no Work Title";  // return a "null" name if work
                                             // doesn't have a work title
        } else {
            temp["work"] = workTitle;
        }

        if (author.empty() == true) {
            temp["author"] = "no Author";  // return a "null" name if work
                                           // doesn't have an Author
        } else {
            temp["author"] = author;
        }

        out["element"].push_back(temp);  // each interaction load the json out

    }  // this bracket closes a loop with instrument 'i" index
    return out;
}

std::vector<std::tuple<int, float, Key, Chord, bool>> Score::getChords(nlohmann::json config) {
    // ===== STEP 1: PARSE THE INPUT CONFIG JSON ===== //
    // ===== STEP 1.0: READ PART NAMES ===== //
    std::vector<std::string> partNames;
    // Type checking
    if (config.contains("partNames") && !config["partNames"].is_array()) {
        printPartNames();
        LOG_ERROR(
            "'partNames' is a optional config argument and MUST BE a strings "
            "array");
        return {};
    }

    // If not setted, set the default value = "all part names"
    if (!config.contains("partNames")) {
        partNames = getPartsNames();
    } else {
        for (const auto& partNameValue : config["partNames"]) {
            const std::string partName = partNameValue.get<std::string>();
            int idx = 0;
            bool isValid = getPartIndex(partName, &idx);

            if (!isValid) {
                LOG_ERROR("Invalid part name: " + partName);
                printPartNames();
                return {};
            }

            partNames.push_back(partName);
        }
    }

    // ===== STEP 1.1: READ MEASURE START ===== //
    int measureStart = 0;

    // If not setted, set the default value = 0
    if (!config.contains("measureStart")) {
        measureStart = 0;
        config["measureStart"] = measureStart;
        // LOG_WARN("Setting the 'measureStart' to the first measure: " <<
        // measureStart);
    }

    // Type checking
    if (config.contains("measureStart") && !config["measureStart"].is_number_integer()) {
        LOG_ERROR(
            "'measureStart' is a optional config argument and MUST BE a "
            "positive integer!");
        return {};
    }

    // Get measure start value
    measureStart = config["measureStart"].get<int>();

    // Error checking:
    if (measureStart < 0) {
        LOG_ERROR("The 'measureStart' value MUST BE a positive integer!");
        return {};
    }

    // ===== STEP 1.2: READ MEASURE END ===== //
    int measureEnd = 0;

    // If not setted, set the default value = All measures!
    if (!config.contains("measureEnd")) {
        measureEnd = getNumMeasures();
        config["measureEnd"] = measureEnd;
        // LOG_WARN("Setting the 'measureEnd' to the last measure: " +
        // std::to_string(measureEnd));
    }

    // Type checking:
    if (config.contains("measureEnd") && !config["measureEnd"].is_number_integer()) {
        LOG_ERROR(
            "'measureEnd' is a optional config argument and MUST BE a positive "
            "integer!");
        return {};
    }

    // Get the 'measureEnd' config value:
    measureEnd = config["measureEnd"].get<int>();

    // Error checking:
    if (measureEnd < 0) {
        LOG_ERROR("The 'measureEnd' value MUST BE greater than 0!");
        return {};
    }

    // Error checking:
    if (measureEnd > getNumMeasures()) {
        LOG_WARN("The 'measureEnd' value is greater then the music length!");
        LOG_WARN("Changing the 'measureEnd' value to: " << getNumMeasures());
        measureEnd = getNumMeasures();
    }

    // Error checking:
    if (measureStart > measureEnd) {
        LOG_ERROR("'measureEnd' value MUST BE greater than 'measureStart' value");
        return {};
    }

    // ===== STEP 1.3: READ MININIMUM CHORD STACKED NOTES ===== //
    int minStackedNotes = (!config.contains("minStack") || !config["minStack"].is_number_integer())
                              ? 2
                              : config["minStack"].get<int>();

    if (minStackedNotes < 1) {
        LOG_WARN("You set the 'minStack' to " << minStackedNotes << ", but the minimum value is 1");
        LOG_WARN("Setting the 'minStack' to the default: 2");
        minStackedNotes = 2;
    }

    // ===== STEP 1.4: READ MAXIMUM CHORD STACKED NOTES ===== //
    const int maxStackedNotes =
        (!config.contains("maxStack") || !config["maxStack"].is_number_integer())
            ? 1000
            : config["maxStack"].get<int>();

    // Error checking:
    if (minStackedNotes > maxStackedNotes) {
        LOG_ERROR("The 'maxStack' value MUST BE greater than 'minStack' value");
        return {};
    }

    // ===== STEP 1.8: READ 'INCLUDE UNPITCHED' ===== //
    const bool includeUnpitched =
        (!config.contains("includeUnpitched") || !config["includeUnpitched"].is_boolean())
            ? false
            : config["includeUnpitched"].get<bool>();

    // ===== STEP 1.9: READ 'INCLUDE DUPLICATES' ===== //
    const bool includeDuplicates =
        (!config.contains("includeDuplicates") || !config["includeDuplicates"].is_boolean())
            ? false
            : config["includeDuplicates"].get<bool>();

    // ===== STEP 2: CREATE A 'IN MEMORY' SQLITE DATABASE ===== //
    SQLite::Database db(":memory:",
                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_MEMORY);

    const std::string& sqlCreateTable =
        "create table events (partIdx integer, measureIdx integer, staveIdx integer, voiceIdx "
        "integer, noteIdx integer, pitch text, duration float, fractionDuration text, "
        "currentFractionDuration text, endFractionDuration text, currentFractionDurationFloat "
        "float, endFractionDurationFloat float,"
        "currentDuration float, endDuration float, currentMeasureFractionDuration text, "
        "endMeasureFractionDuration text, currentMeasureDuration float, "
        "endMeasureDuration float, currentTime float, endTime float, noteAddress intptr_t, "
        "measureAddress intptr_t, "
        "divisionPerQuarterNote integer);";

    db.exec(sqlCreateTable.c_str());

    std::string sqlInsertValues;
    const int partNamesSize = partNames.size();
    for (int partIdx = 0; partIdx < partNamesSize; partIdx++) {
        Part& currentPart = getPart(partNames[partIdx]);

        // Skip unpitched parts
        if (!includeUnpitched && !currentPart.isPitched()) {
            continue;
        }

        float currentDuration = 0.0f;
        float currentMeasureDuration = 0.0f;
        float previusDuration = 0.0f;
        float previusMeasureDuration = 0.0f;
        Fraction currentFractionDuration(0, 1);
        Fraction currentMeasureFractionDuration(0, 1);
        Fraction previusFractionDuration(0, 1);
        Fraction previusMeasureFractionDuration(0, 1);

        for (int measureIdx = measureStart; measureIdx < measureEnd; measureIdx++) {
            Measure& currentMeasure = currentPart.getMeasure(measureIdx);
            const float startMeasureDuration = static_cast<float>(measureIdx);
            currentMeasureDuration = startMeasureDuration;

            Fraction startMeasureFractionDuration(measureIdx, 1);
            currentMeasureFractionDuration = startMeasureFractionDuration;

            float measureBegginingDuration = 0.0f;
            Fraction measureBegginingFractionDuration(0, 1);

            const int divisionsPerQuarterNote = currentMeasure.getDivisionsPerQuarterNote();

            for (int staveIdx = 0; staveIdx < currentMeasure.getNumStaves(); staveIdx++) {
                const int numNotes = currentMeasure.getNumNotes(staveIdx);

                if (staveIdx == 0) {
                    measureBegginingDuration = currentDuration;
                    measureBegginingFractionDuration = currentFractionDuration;
                } else {
                    currentDuration = measureBegginingDuration;
                    currentFractionDuration = measureBegginingFractionDuration;
                }

                int currentVoice = 1;

                for (int noteIdx = 0; noteIdx < numNotes; noteIdx++) {
                    const Note& currentNote = currentMeasure.getNote(noteIdx, staveIdx);
                    const int voiceIdx = currentNote.getVoice();
                    const std::string& pitch = currentNote.getPitch();
                    const float duration = currentNote.getQuarterDuration();
                    const Fraction& fractionDuration =
                        currentNote.getDuration().getFractionDuration();
                    const std::string& fractionDurationAsString =
                        currentNote.getDuration().getFractionDurationAsString();

                    if (currentNote.isGraceNote()) {
                        continue;
                    }

                    if (currentNote.inChord()) {
                        currentDuration = previusDuration;
                        currentMeasureDuration = previusMeasureDuration;

                        currentFractionDuration = previusFractionDuration;
                        currentMeasureFractionDuration = previusMeasureFractionDuration;
                    }

                    if (currentNote.getVoice() != currentVoice) {
                        currentVoice = currentNote.getVoice();
                        currentDuration = measureBegginingDuration;
                        currentMeasureDuration = startMeasureDuration;

                        currentFractionDuration = measureBegginingFractionDuration;
                        currentMeasureFractionDuration = startMeasureFractionDuration;
                    }

                    const float endDuration = currentDuration + duration;
                    const float measureDurationDelta =
                        static_cast<float>(currentNote.getDurationTicks()) /
                        static_cast<float>(currentMeasure.getDurationTicks());
                    const float endMeasureDuration = currentMeasureDuration + measureDurationDelta;

                    const Fraction endFractionDuration = currentFractionDuration + fractionDuration;
                    const Fraction measureFractionDurationDelta =
                        currentNote.getDuration().getFractionDuration() /
                        currentMeasure.getFractionDuration();
                    const Fraction endMeasureFractionDuration =
                        currentMeasureFractionDuration + measureFractionDurationDelta;

                    sqlInsertValues +=
                        "\n(" + std::to_string(partIdx) + ", " + std::to_string(measureIdx) + ", " +
                        std::to_string(staveIdx) + ", " + std::to_string(voiceIdx) + ", " +
                        std::to_string(noteIdx) + ", '" + pitch + "', " + std::to_string(duration) +
                        ", " + fractionDurationAsString + ", " +
                        currentFractionDuration.toString() + ", " + endFractionDuration.toString() +
                        ", " + std::to_string(currentFractionDuration.getFloatValue()) + ", " +
                        std::to_string(endFractionDuration.getFloatValue()) + ", " +
                        std::to_string(currentDuration) + ", " + std::to_string(endDuration) +
                        ", " + currentMeasureFractionDuration.toString() + ", " +
                        endMeasureFractionDuration.toString() + ", " +
                        std::to_string(currentMeasureDuration) + ", " +
                        std::to_string(endMeasureDuration) + ", " +
                        std::to_string(currentMeasureFractionDuration.getFloatValue()) + ", " +
                        std::to_string(endMeasureFractionDuration.getFloatValue()) + ", " +
                        std::to_string((intptr_t)&currentNote) + ", " +
                        std::to_string((intptr_t)&currentMeasure) + ", " +
                        std::to_string(divisionsPerQuarterNote) + "),";

                    // Aux variables
                    previusDuration = currentDuration;
                    previusMeasureDuration = currentMeasureDuration;

                    previusFractionDuration = currentFractionDuration;
                    previusMeasureFractionDuration = currentMeasureFractionDuration;

                    currentDuration += duration;
                    currentMeasureDuration += measureDurationDelta;

                    currentFractionDuration += fractionDuration;
                    currentMeasureFractionDuration += measureFractionDurationDelta;
                }
            }
        }
    }

    // Substitui o último caractere ',' por ';'
    const std::string& replacement = ";";
    sqlInsertValues.back() = replacement[0];

    const std::string& sqlInsertCommand =
        "insert into events (partIdx, measureIdx, staveIdx, voiceIdx, noteIdx, "
        "pitch, duration, fractionDuration, currentFractionDuration, endFractionDuration, "
        "currentFractionDurationFloat, endFractionDurationFloat, "
        "currentDuration, endDuration, currentMeasureFractionDuration, endMeasureFractionDuration, "
        "currentMeasureDuration, endMeasureDuration, currentTime, endTime, noteAddress, "
        "measureAddress, "
        "divisionPerQuarterNote) "
        "VALUES ";

    const std::string& sqlCommand = sqlInsertCommand + sqlInsertValues;

    // std::cout << sqlCommand << std::endl;

    db.exec(sqlCommand.c_str());

    return getChordsPerEachNoteEvent(db, includeDuplicates);
}

std::vector<std::tuple<int, float, Key, Chord, bool>> Score::getChordsPerEachNoteEvent(
    SQLite::Database& db, const bool includeDuplicates) {
    // ===== STEP 0: CREATE INDEX TO SPEED UP QUERIES ===== //
    db.exec("CREATE INDEX currentTime_endTime_idx ON events (currentTime, endTime)");

    // ===== STEP 1: GET THE NUMBER OF UNIQUE START TIME EVENTS ===== //
    const int numUniqueEvents =
        db.execAndGet("SELECT COUNT (DISTINCT currentTime) from events").getInt();

    // ===== STEP 2: GET THE UNIQUE START TIME EVENTS ===== //
    std::vector<float> uniqueStartTime;
    uniqueStartTime.reserve(numUniqueEvents);
    SQLite::Statement query(db, "SELECT DISTINCT currentTime from events ORDER BY currentTime ASC");

    while (query.executeStep()) {
        uniqueStartTime.push_back(query.getColumn(0).getDouble());
    }

    // ===== STEP 3: FOR EACH UNIQUE START TIME - GET THE STACK CHORD ===== //
    std::vector<Score::ChordData> chordData(numUniqueEvents);
    int chordIdx = 0;
    std::vector<std::tuple<int, float, Key, Chord, bool>> stackedChords;
    stackedChords.reserve(numUniqueEvents);
    for (const float startTime : uniqueStartTime) {
        SQLite::Statement query(db,
                                "SELECT currentTime, noteAddress, measureAddress, "
                                "currentFractionDurationFloat, endFractionDurationFloat FROM "
                                "events WHERE currentTime <= ? and endTime > ?");
        // Bind query parameters
        query.bind(1, startTime);
        query.bind(2, startTime);

        const Measure* measurePtr = nullptr;
        auto& currentChordData = chordData.at(chordIdx);
        while (query.executeStep()) {
            const float measureStartTime = query.getColumn(0).getDouble();

            // Get the note pointer
            const intptr_t noteAddress = query.getColumn(1).getInt64();
            const Note* notePtr = reinterpret_cast<const Note*>(noteAddress);

            // Get the measure pointer
            const intptr_t measureAddress = query.getColumn(2).getInt64();
            measurePtr = reinterpret_cast<const Measure*>(measureAddress);

            const float beatStartTime = query.getColumn(3).getDouble();
            const float beatEndTime = query.getColumn(4).getDouble();

            // std::cout << "chordIdx: " << chordIdx << " | pitch: " << notePtr->getPitch()
            //           << " | startTime: " << startTime
            //           << " | measureStartTime: " << measureStartTime
            //           << " | beatStartTime: " << beatStartTime << " beatEndTime: " << beatEndTime
            //           << std::endl;

            if (!notePtr->isNoteOn()) {
                continue;
            }

            currentChordData.noteData.push_back(
                {measureStartTime, notePtr, beatStartTime, beatEndTime});

            if (beatEndTime > currentChordData.beatEndTimeHigherLimit) {
                currentChordData.beatEndTimeHigherLimit = beatEndTime;
            }

            if (beatStartTime < currentChordData.beatStartTimeLowerLimit) {
                currentChordData.beatStartTimeLowerLimit = beatStartTime;
            }

            currentChordData.isHomophonicChord &= isFloatEqual(startTime, measureStartTime, 0.005f);
            currentChordData.measurePtr = measurePtr;
        }

        currentChordData.chordQuarterDuration =
            currentChordData.beatEndTimeHigherLimit - currentChordData.beatStartTimeLowerLimit;

        if (currentChordData.noteData.empty()) {
            continue;
        }

        Chord chord;
        for (const auto& noteData : currentChordData.noteData) {
            chord.addNote(*noteData.notePtr);
        }

        const int measureDivisionPerQuarterNote = measurePtr->getDivisionsPerQuarterNote();
        chord.setDuration(currentChordData.chordQuarterDuration, measureDivisionPerQuarterNote);

        // Remove chord duplicate notes
        if (!includeDuplicates) {
            chord.removeDuplicateNotes();
        }

        chord.sortNotes();

        // Get the current measure Key
        const int measureIdx = measurePtr->getNumber();
        const Key& key = _part.at(0).getMeasure(measureIdx).getKey();

        stackedChords.push_back(
            {measureIdx + 1, startTime + 1, key, chord, currentChordData.isHomophonicChord});
        chordIdx++;
    }

    return stackedChords;
}
