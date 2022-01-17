#include "part.h"

//Part::Part(const std::string& partName, const int numMeasures, const int numStaves, const int divisionsPerQuarterNote) :
//    _numStaves(numStaves),
//    _divisionsPerQuarterNote(divisionsPerQuarterNote)
//{
//    const int partNameSize = partName.size();

//    _partName = partName;

//    if (partNameSize > 5) {
//        _shortName = partName.substr(0, 4) + ".";
//    } else {
//        _shortName = partName.substr(0, partNameSize) + ".";
//    }

//    _measure.resize(numMeasures);

//    for (auto& m : _measure) {
//        m.setNumStaves(numStaves);
//        m.setDivisionsPerQuarterNote(divisionsPerQuarterNote);

//        m.setKeyMode("major");
//        m.setTimeSignature(4, 4);
//        m.setKeySignature(0);
//    }

//    _measure[0].setKeyMode("major");
//    _measure[0].setTimeSignature(4, 4);
//    _measure[0].setKeySignature(0);
//}

Part::Part(const std::string& partName, const int numStaves, const bool isPitched, const int divisionsPerQuarterNote) :
    _numStaves(numStaves),
    _divisionsPerQuarterNote(divisionsPerQuarterNote),
    _isPitched(isPitched),
    _staffLines(5)
{
    const int partNameSize = partName.size();

    _partName = partName;

    if (partNameSize > 5) {
        _shortName = partName.substr(0, 4) + ".";
    } else {
        _shortName = partName.substr(0, partNameSize) + ".";
    }

//    for (auto& m : _measure) {
//        m.setNumStaves(numStaves);
//        m.setDivisionsPerQuarterNote(divisionsPerQuarterNote);

//        m.setKeyMode("major");
//        m.setTimeSignature(4, 4);
//        m.setKeySignature(0);
//    }
}

Part::~Part()
{

}

void Part::clear()
{
    _measure.clear();
}

void Part::info() const
{
    std::cout << "Part Name: " << _partName << std::endl;
    std::cout << "Short Name: " << _shortName << std::endl;
    std::cout << "Number of Staves: " << _numStaves << std::endl;
}

const std::string& Part::getName() const
{
    return _partName;
}

const std::string& Part::getShortName() const
{
    return _shortName;
}

bool Part::isPitched() const
{
    return _isPitched;;
}

void Part::setStaffLines(const int staffLines)
{
    _staffLines = staffLines;
}

int Part::getStaffLines() const
{
    return _staffLines;
}

void Part::setIsPitched(const bool isPitched)
{
    _isPitched = isPitched;

    for (auto& m : _measure) {
        const int numStaves = m.getNumStaves();
        for (int s = 0; s < numStaves; s++) {
            for (int n = 0; n < m.getNumNotes(s); n++) {
                m.getNote(n, s).setIsPitched(isPitched);
            }
        }
    }
}

void Part::addMeasure(const int numMeasures)
{
    const int currentSize = _measure.size();
    const int newSize = currentSize + numMeasures;

    _measure.resize(newSize);

    for (int m = currentSize; m < newSize; m++) {
        _measure[m].setNumStaves(_numStaves);
        _measure[m].setDivisionsPerQuarterNote(_divisionsPerQuarterNote);
    }
}

void Part::removeMeasure(const int measureStart, const int measureEnd)
{
    _measure.erase(_measure.begin() + measureStart, _measure.begin() + measureEnd);
}

Measure& Part::getMeasure(const int measureId)
{
    return _measure.at(measureId);
}

const Measure& Part::getMeasure(const int measureId) const
{
    return _measure.at(measureId);
}

int Part::getNumMeasures() const
{
    return _measure.size();
}

void Part::setNumStaves(const int numStaves)
{
    _numStaves = numStaves;

    for (auto& m : _measure) {
        m.setNumStaves(numStaves);
    }
}

void Part::addStaves(const int numStaves)
{
    _numStaves += numStaves;
}

void Part::removeStave(const int staveId)
{
    ignore(staveId);
//    for (auto& measure : _measure) {
//        // Check for each note if is with staveId
//        // if yes, remove from vector
//    }
}

int Part::getNumStaves() const
{
    return _numStaves;
}

void Part::addMidiUnpitched(const int midiUnpitched)
{
    _midiUnpitched.push_back(midiUnpitched);
}

std::vector<int> Part::getMidiUnpitched() const
{
    return _midiUnpitched;
}

int Part::getNumNotes(const int staveId)
{
    int numNotes = 0;

    const int numMeasures = getNumMeasures();

    for (int i = 0; i < numMeasures; i++) {
        numNotes += (staveId < 0) ? _measure[i].getNumNotes() : _measure[i].getNumNotes(staveId);
    }

    return numNotes;
}

int Part::getNumNotesOn(const int staveId)
{
    int numNotes = 0;

    const int numMeasures = getNumMeasures();

    for (int i = 0; i < numMeasures; i++) {
        numNotes += (staveId < 0) ? _measure[i].getNumNotesOn() : _measure[i].getNumNotesOn(staveId);
    }
    
    return numNotes;
}

int Part::getNumNotesOff(const int staveId)
{
    int numNotes = 0;

    const int numMeasures = getNumMeasures();

    for (int i = 0; i < numMeasures; i++) {
        numNotes += (staveId < 0) ? _measure[i].getNumNotesOff() : _measure[i].getNumNotesOff(staveId);
    }
    
    return numNotes;
}

void Part::setShortName(const std::string& shortName)
{
    _shortName = shortName;
}

const std::string Part::toXML(const int instrumentId, const int identSize) const
{
    std::string xml;

    const int numMeasures = getNumMeasures();

    for (int m = 0; m < numMeasures; m++) {
        xml.append(Helper::Helper::generateIdentation(2, identSize) + "<!--============== Part: P" + std::to_string(instrumentId+1) + ", Measure: " + std::to_string(m+1) + " ==============-->\n");
        xml.append(Helper::Helper::generateIdentation(2, identSize) + "<measure number=\"" + std::to_string(m+1) + "\">\n");

        // ===== PRINT TAG ===== //
        // xml.append(Helper::generateIdentation(3, identSize) + "<print new-page=\"yes\">\n");
        // xml.append(Helper::generateIdentation(4, identSize) + "<system-layout>\n");
        // xml.append(Helper::generateIdentation(5, identSize) + "<system-margins>\n");
        // xml.append(Helper::generateIdentation(6, identSize) + "<left-margin>88</left-margin>\n");
        // xml.append(Helper::generateIdentation(6, identSize) + "<right-margin>0</right-margin>\n");
        // xml.append(Helper::generateIdentation(5, identSize) + "</system-margins>\n");
        // xml.append(Helper::generateIdentation(5, identSize) + "<top-system-distance>218</top-system-distance>\n");
        // xml.append(Helper::generateIdentation(4, identSize) + "</system-layout>\n");
        // xml.append(Helper::generateIdentation(3, identSize) + "</print>\n");

        // ===== ATTRIBUTES TAG ===== //
        bool attributeChanged = m == 0 || _measure[m].keySignatureChanged() || _measure[m].timeSignatureChanged() || _measure[m].clefChanged();

        if (attributeChanged) { xml.append(Helper::generateIdentation(3, identSize) + "<attributes>\n"); }

        if (m == 0) {
            if (_measure[m].empty()) {
                xml.append(Helper::generateIdentation(4, identSize) + "<divisions>" + std::to_string(256) + "</divisions>\n");
            } else {
                xml.append(Helper::generateIdentation(4, identSize) + "<divisions>" + std::to_string(_measure[m].getNote(0, 0).getDivisionsPerQuarterNote()) + "</divisions>\n");
            }
        }

        if (_measure[m].keySignatureChanged()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<key>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<fifths>" + std::to_string(_measure[m].getFifthCicle()) + "</fifths>\n");

            const std::string keyMode = (_measure[m].isMajorKeyMode()) ? "major" : "minor";
            xml.append(Helper::generateIdentation(5, identSize) + "<mode>" + keyMode + "</mode>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</key>\n");
        }

        if (_measure[m].timeSignatureChanged()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<time>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<beats>" + std::to_string(_measure[m].getTimeSignature().first) + "</beats>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<beat-type>" + std::to_string(_measure[m].getTimeSignature().second) + "</beat-type>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</time>\n");
        }

        if (m == 0 && _numStaves > 1) {
            xml.append(Helper::generateIdentation(4, identSize) + "<staves>" + std::to_string(_numStaves) + "</staves>\n");
        }

        if (m == 0 || _measure[m].clefChanged()) {
            for (int c = 0; c < _measure[m].getNumStaves(); c++) {
                xml.append(Helper::generateIdentation(4, identSize) + "<clef number=\"" + std::to_string(c+1) + "\">\n");
                xml.append(Helper::generateIdentation(5, identSize) + "<sign>" + _measure[m].getClef(c).getSign()+ "</sign>\n");
                xml.append(Helper::generateIdentation(5, identSize) + "<line>" + std::to_string(_measure[m].getClef(c).getLine()) + "</line>\n");
                xml.append(Helper::generateIdentation(4, identSize) + "</clef>\n");
            }
        }

        if (m == 0 && _staffLines != 5) {
            xml.append(Helper::generateIdentation(4, identSize) + "<staff-details>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<staff-lines>" + std::to_string(_staffLines) + "</staff-lines>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</staff-details>\n");
        }

        if (attributeChanged) { xml.append(Helper::generateIdentation(3, identSize) + "</attributes>\n"); }

        // ===== METRONOME MARK ===== //
        bool metronomeMarkChanged = _measure[m].metronomeChanged();
        if (metronomeMarkChanged) { 
            xml.append(Helper::generateIdentation(3, identSize) + "<direction placement=\"above\">\n"); 
            xml.append(Helper::generateIdentation(4, identSize) + "<direction-type>\n"); 
            xml.append(Helper::generateIdentation(5, identSize) + "<metronome parentheses=\"no\">\n");
            xml.append(Helper::generateIdentation(6, identSize) + "<beat-unit>" +  _measure[m].getMetronome().first + "</beat-unit>\n");
            xml.append(Helper::generateIdentation(6, identSize) + "<per-minute>" + std::to_string(_measure[m].getMetronome().second) + "</per-minute>\n");        
            xml.append(Helper::generateIdentation(5, identSize) + "</metronome>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</direction-type>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "<sound tempo=\"" + std::to_string(_measure[m].getMetronome().second) + "\"/>\n");   
            xml.append(Helper::generateIdentation(3, identSize) + "</direction>\n");
        }

        xml.append(_measure[m].toXML(instrumentId, identSize));

        if ((m == numMeasures - 1) && _measure[m].getBarlineRight().getBarStyle().empty()) {
            xml.append(Helper::generateIdentation(2, identSize) + "<barline location=\"right\">\n");
            xml.append(Helper::generateIdentation(3, identSize) + "<bar-style>light-heavy</bar-style>\n");
            xml.append(Helper::generateIdentation(2, identSize) + "</barline>\n");
        }

        xml.append(Helper::generateIdentation(2, identSize) + "</measure>\n");
    }

    return xml;
}

std::string Part::toJSON() const
{
    return std::string();
}

void Part::appendNote(const Note& note, const int position, const int staveId)
{
    const int noteDuration = note.getDurationTicks();
    const int numMeasures = getNumMeasures();

    for (int m = 0; m < numMeasures; m++) {
        auto& measure = _measure[m];
        const int emptySpace = measure.getEmptyDurationTicks();

        // This measure 'm' doesn't have any empty space.
        // Go to the next measure
        if (emptySpace == 0) { continue; }

        // Compute the diference
        const int diff = emptySpace - noteDuration;

        // CASE 01: split the note and use ties
        if (diff < 0) {
            Note first = note;
            first.setDurationTicks(emptySpace);
            first.setTieStart();

            Note second = note;
            second.setDurationTicks(abs(diff));
            second.setTieStop();

            _measure[m].addNote(first, staveId, position);

            // se tiver um compasso livre:
            // Se for o ultimo compasso...
            if (m == numMeasures - 1) {
                // Add a blank measure at the end
                //Score::addMeasure() -> tem q herdar public, e os metodos
                // tem q ser protected
                // https://stackoverflow.com/questions/357307/how-to-call-a-parent-class-function-from-derived-class-function
            }

            _measure[m+1].addNote(second, staveId, position);

            return;
        }

        // CASE 02: just add the note
        measure.addNote(note, staveId, position);
        return;
    }
}

void Part::appendNotes(const std::vector<Note>& notes, const int position, const int staveId)
{
    for (const auto& note : notes) {
        appendNote(note, position, staveId);
    }
}

void Part::appendChord(const Chord& chord, const int position, const int staveId)
{
    const int chordDuration = chord[0].getDurationTicks();
    const int numMeasures = getNumMeasures();

    for (int m = 0; m < numMeasures; m++) {
        auto& measure = _measure[m];
        const int emptySpace = measure.getEmptyDurationTicks();

        // This measure 'm' doesn't have any empty space.
        // Go to the next measure
        if (emptySpace == 0) { continue; }

        // Compute the diference
        const int diff = emptySpace - chordDuration;

        const int chordSize = chord.size();

        // CASE 01: split the chord notes and use ties
        if (diff < 0) {
            std::vector<Note> first(chordSize);
            std::vector<Note> second(chordSize);

            for (int n = 0; n < chordSize; n++) {
                first[n] = chord[n];
                second[n] = chord[n];
            }

            for (int n = 0; n < chordSize; n++) {
                first[n].setDurationTicks(emptySpace);
                first[n].setTieStart();

                _measure[m].addNote(first[n], staveId, position);
            }

            for (int n = 0; n < chordSize; n++) {
                second[n].setDurationTicks(abs(diff));
                second[n].setTieStop();

                _measure[m+1].addNote(second[n], staveId, position);
            }

            return;
        }

        // CASE 02: just add the chord notes
        for (int n = 0; n < chordSize; n++) {
            measure.addNote(chord[n], staveId, position);
        }

        return;
    }
}

void Part::appendChords(const std::vector<Chord>& chords, const int position, const int staveId)
{
    for (const auto& chord : chords) {
        appendChord(chord, position, staveId);
    }
}

void Part::append(const std::variant<Note, Chord>& obj, const int position, const int staveId)
{
    const int type = obj.index();

    switch (type) {
    case 0: appendNote(std::get<Note>(obj), position, staveId); break; // Note
    case 1: appendChord(std::get<Chord>(obj), position, staveId); break; // Chord
    default: std::cerr << "[ERROR]: The object must be a Note or a Chord" << std::endl; break;
    }
}

void Part::append(const std::vector<std::variant<Note, Chord>>& objs, const int position, const int staveId)
{
    for (const auto& obj : objs) {
        append(obj, position, staveId);
    }
}
