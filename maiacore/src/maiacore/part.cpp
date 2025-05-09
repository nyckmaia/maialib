#include "maiacore/part.h"

#include "cherno/instrumentor.h"
#include "maiacore/chord.h"
#include "maiacore/clef.h"
#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/utils.h"

Part::Part(const std::string& partName, const int numStaves, const bool isPitched,
           const int divisionsPerQuarterNote)
    : _partIndex(-1),
      _numStaves(numStaves),
      _divisionsPerQuarterNote(divisionsPerQuarterNote),
      _isPitched(isPitched),
      _staffLines(5) {
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

Part::~Part() {}

void Part::clear() { _measure.clear(); }

int Part::getPartIndex() const { return _partIndex; }

void Part::setPartIndex(int partIdx) { _partIndex = partIdx; }

void Part::info() const {
    LOG_INFO("Part Name: " << _partName);
    LOG_INFO("Short Name: " << _shortName);
    LOG_INFO("Number of Staves: " << _numStaves);
}

const std::string& Part::getName() const { return _partName; }

const std::string& Part::getShortName() const { return _shortName; }

bool Part::isPitched() const { return _isPitched; }

void Part::setStaffLines(const int staffLines) {
    PROFILE_FUNCTION();

    _staffLines = staffLines;
}

int Part::getStaffLines() const { return _staffLines; }

void Part::setIsPitched(const bool isPitched) {
    PROFILE_FUNCTION();

    _isPitched = isPitched;

    for (auto& clef : _measure.at(0).getClefs()) {
        clef.setSign(ClefSign::PERCUSSION);
    }

    for (auto& m : _measure) {
        const int numStaves = m.getNumStaves();
        for (int s = 0; s < numStaves; s++) {
            for (int n = 0; n < m.getNumNotes(s); n++) {
                m.getNote(n, s).setIsPitched(isPitched);
            }
        }
    }
}

void Part::addMeasure(const int numMeasures) {
    const int currentSize = _measure.size();
    const int newSize = currentSize + numMeasures;

    _measure.resize(newSize);

    for (int m = currentSize; m < newSize; m++) {
        _measure[m].setNumStaves(_numStaves);
        _measure[m].setDivisionsPerQuarterNote(_divisionsPerQuarterNote);
    }
}

void Part::removeMeasure(const int measureStart, const int measureEnd) {
    _measure.erase(_measure.begin() + measureStart, _measure.begin() + measureEnd);
}

Measure& Part::getMeasure(const int measureId) { return _measure.at(measureId); }

const Measure& Part::getMeasure(const int measureId) const {
    PROFILE_FUNCTION();
    return _measure.at(measureId);
}

const std::vector<Measure> Part::getMeasures() const { return _measure; }

int Part::getNumMeasures() const { return _measure.size(); }

void Part::setNumStaves(const int numStaves) {
    PROFILE_FUNCTION();

    _numStaves = numStaves;

    for (auto& m : _measure) {
        m.setNumStaves(numStaves);
    }

    // Set the default ClefSign for multiple stave instruments (like piano)
    _measure.at(0).getClef(0).setSign(ClefSign::G);
    for (int s = 1; s < numStaves; s++) {
        _measure.at(0).getClef(s).setSign(ClefSign::F);
    }
}

void Part::addStaves(const int numStaves) { _numStaves += numStaves; }

void Part::removeStave(const int staveId) {
    ignore(staveId);
    //    for (auto& measure : _measure) {
    //        // Check for each note if is with staveId
    //        // if yes, remove from vector
    //    }
}

int Part::getNumStaves() const { return _numStaves; }

void Part::addMidiUnpitched(const int midiUnpitched) {
    PROFILE_FUNCTION();

    _midiUnpitched.push_back(midiUnpitched);
}

std::vector<int> Part::getMidiUnpitched() const { return _midiUnpitched; }

int Part::getNumNotes(const int staveId) {
    int numNotes = 0;

    const int numMeasures = getNumMeasures();

    for (int i = 0; i < numMeasures; i++) {
        numNotes += (staveId < 0) ? _measure[i].getNumNotes() : _measure[i].getNumNotes(staveId);
    }

    return numNotes;
}

int Part::getNumNotesOn(const int staveId) {
    int numNotes = 0;

    const int numMeasures = getNumMeasures();

    for (int i = 0; i < numMeasures; i++) {
        numNotes +=
            (staveId < 0) ? _measure[i].getNumNotesOn() : _measure[i].getNumNotesOn(staveId);
    }

    return numNotes;
}

int Part::getNumNotesOff(const int staveId) {
    int numNotes = 0;

    const int numMeasures = getNumMeasures();

    for (int i = 0; i < numMeasures; i++) {
        numNotes +=
            (staveId < 0) ? _measure[i].getNumNotesOff() : _measure[i].getNumNotesOff(staveId);
    }

    return numNotes;
}

void Part::setShortName(const std::string& shortName) { _shortName = shortName; }

const std::string Part::toXML(const int instrumentId, const int identSize) const {
    std::string xml;

    const int numMeasures = getNumMeasures();

    for (int m = 0; m < numMeasures; m++) {
        xml.append(Helper::Helper::generateIdentation(2, identSize) + "<!--============== Part: P" +
                   std::to_string(instrumentId + 1) + ", Measure: " + std::to_string(m + 1) +
                   " ==============-->\n");
        xml.append(Helper::Helper::generateIdentation(2, identSize) + "<measure number=\"" +
                   std::to_string(m + 1) + "\">\n");

        // ===== PRINT TAG ===== //
        // xml.append(Helper::generateIdentation(3, identSize) + "<print
        // new-page=\"yes\">\n"); xml.append(Helper::generateIdentation(4,
        // identSize) + "<system-layout>\n");
        // xml.append(Helper::generateIdentation(5, identSize) +
        // "<system-margins>\n"); xml.append(Helper::generateIdentation(6,
        // identSize) + "<left-margin>88</left-margin>\n");
        // xml.append(Helper::generateIdentation(6, identSize) +
        // "<right-margin>0</right-margin>\n");
        // xml.append(Helper::generateIdentation(5, identSize) +
        // "</system-margins>\n"); xml.append(Helper::generateIdentation(5,
        // identSize) + "<top-system-distance>218</top-system-distance>\n");
        // xml.append(Helper::generateIdentation(4, identSize) +
        // "</system-layout>\n"); xml.append(Helper::generateIdentation(3,
        // identSize) + "</print>\n");

        // ===== ATTRIBUTES TAG ===== //
        bool attributeChanged = m == 0 || _measure[m].keySignatureChanged() ||
                                _measure[m].timeSignatureChanged() ||
                                _measure[m].divisionsPerQuarterNoteChanged();
        const auto& measureClefs = _measure[m].getClefs();
        for (const auto& clef : measureClefs) {
            attributeChanged |= clef.isClefChanged();
        }

        if (attributeChanged) {
            xml.append(Helper::generateIdentation(3, identSize) + "<attributes>\n");
        }

        if (_measure[m].divisionsPerQuarterNoteChanged()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<divisions>" +
                       std::to_string(_measure[m].getDivisionsPerQuarterNote()) + "</divisions>\n");
        }

        if (_measure[m].keySignatureChanged()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<key>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<fifths>" +
                       std::to_string(_measure[m].getFifthCircle()) + "</fifths>\n");

            const std::string keyMode = (_measure[m].isMajorKeyMode()) ? "major" : "minor";
            xml.append(Helper::generateIdentation(5, identSize) + "<mode>" + keyMode + "</mode>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</key>\n");
        }

        if (_measure[m].timeSignatureChanged()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<time>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<beats>" +
                       std::to_string(_measure[m].getTimeSignature().getUpperValue()) +
                       "</beats>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<beat-type>" +
                       std::to_string(_measure[m].getTimeSignature().getLowerValue()) +
                       "</beat-type>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</time>\n");
        }

        if (m == 0 && _numStaves > 1) {
            xml.append(Helper::generateIdentation(4, identSize) + "<staves>" +
                       std::to_string(_numStaves) + "</staves>\n");
        }

        if (m == 0 || _measure[m].isClefChanged()) {
            if (_numStaves == 1) {
                xml.append(_measure[m].getClef().toXML(-1, identSize));
            } else {
                for (int clefIdx = 0; clefIdx < _numStaves; clefIdx++) {
                    const auto& currentClef = _measure[m].getClef(clefIdx);
                    xml.append(currentClef.toXML(clefIdx, identSize));
                }
            }
        }

        if (m == 0 && _staffLines != 5) {
            xml.append(Helper::generateIdentation(4, identSize) + "<staff-details>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "<staff-lines>" +
                       std::to_string(_staffLines) + "</staff-lines>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</staff-details>\n");
        }

        if (attributeChanged) {
            xml.append(Helper::generateIdentation(3, identSize) + "</attributes>\n");
        }

        // ===== METRONOME MARK ===== //
        bool metronomeMarkChanged = _measure[m].metronomeChanged();
        if (metronomeMarkChanged) {
            xml.append(Helper::generateIdentation(3, identSize) +
                       "<direction placement=\"above\">\n");
            xml.append(Helper::generateIdentation(4, identSize) + "<direction-type>\n");
            xml.append(Helper::generateIdentation(5, identSize) +
                       "<metronome parentheses=\"no\">\n");
            xml.append(Helper::generateIdentation(6, identSize) + "<beat-unit>" +
                       _measure[m].getMetronome().first + "</beat-unit>\n");
            xml.append(Helper::generateIdentation(6, identSize) + "<per-minute>" +
                       std::to_string(_measure[m].getMetronome().second) + "</per-minute>\n");
            xml.append(Helper::generateIdentation(5, identSize) + "</metronome>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "</direction-type>\n");
            xml.append(Helper::generateIdentation(4, identSize) + "<sound tempo=\"" +
                       std::to_string(_measure[m].getMetronome().second) + "\"/>\n");
            xml.append(Helper::generateIdentation(3, identSize) + "</direction>\n");
        }

        xml.append(_measure[m].toXML(instrumentId, identSize));

        if ((m == numMeasures - 1) && _measure[m].getBarlineRight().getBarStyle().empty()) {
            xml.append(Helper::generateIdentation(2, identSize) + "<barline location=\"right\">\n");
            xml.append(Helper::generateIdentation(3, identSize) +
                       "<bar-style>light-heavy</bar-style>\n");
            xml.append(Helper::generateIdentation(2, identSize) + "</barline>\n");
        }

        xml.append(Helper::generateIdentation(2, identSize) + "</measure>\n");
    }

    return xml;
}

std::string Part::toJSON() const { return std::string(); }

void Part::appendNote(const Note& note, const int position, const int staveId) {
    const int noteDuration = note.getDurationTicks();
    const int numMeasures = getNumMeasures();

    for (int m = 0; m < numMeasures; m++) {
        auto& measure = _measure[m];
        const int emptySpace = measure.getEmptyDurationTicks();
        const int divisionsPerQuarterNote = measure.getDivisionsPerQuarterNote();

        // This measure 'm' doesn't have any empty space.
        // Go to the next measure
        if (emptySpace == 0) {
            continue;
        }

        // Compute the diference
        const int diff = emptySpace - noteDuration;

        // CASE 01: split the note and use ties
        if (diff < 0) {
            Note first = note;
            first.setDuration({emptySpace, divisionsPerQuarterNote});
            first.setTieStart();

            Note second = note;
            second.setDuration({abs(diff), divisionsPerQuarterNote});
            second.setTieStop();

            _measure[m].addNote(first, staveId, position);

            // se tiver um compasso livre:
            // Se for o ultimo compasso...
            if (m == numMeasures - 1) {
                // Add a blank measure at the end
                // Score::addMeasure() -> tem q herdar public, e os metodos
                // tem q ser protected
                // https://stackoverflow.com/questions/357307/how-to-call-a-parent-class-function-from-derived-class-function
            }

            _measure[m + 1].addNote(second, staveId, position);

            return;
        }

        // CASE 02: just add the note
        measure.addNote(note, staveId, position);
        return;
    }
}

void Part::appendNotes(const std::vector<Note>& notes, const int position, const int staveId) {
    for (const auto& note : notes) {
        appendNote(note, position, staveId);
    }
}

void Part::appendChord(const Chord& chord, const int position, const int staveId) {
    const int chordDuration = chord[0].getDurationTicks();
    const int numMeasures = getNumMeasures();

    for (int m = 0; m < numMeasures; m++) {
        auto& measure = _measure[m];
        const int emptySpace = measure.getEmptyDurationTicks();
        const int divisionsPerQuarterNote = measure.getDivisionsPerQuarterNote();

        // This measure 'm' doesn't have any empty space.
        // Go to the next measure
        if (emptySpace == 0) {
            continue;
        }

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
                first[n].setDuration({emptySpace, divisionsPerQuarterNote});
                first[n].setTieStart();

                _measure[m].addNote(first[n], staveId, position);
            }

            for (int n = 0; n < chordSize; n++) {
                second[n].setDuration({abs(diff), divisionsPerQuarterNote});
                second[n].setTieStop();

                _measure[m + 1].addNote(second[n], staveId, position);
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

void Part::appendChords(const std::vector<Chord>& chords, const int position, const int staveId) {
    for (const auto& chord : chords) {
        appendChord(chord, position, staveId);
    }
}

void Part::append(const std::variant<Note, Chord>& obj, const int position, const int staveId) {
    const int type = obj.index();

    switch (type) {
        case 0:
            appendNote(*std::get_if<Note>(&obj), position, staveId);
            break;  // Note
        case 1:
            appendChord(*std::get_if<Chord>(&obj), position, staveId);
            break;  // Chord
        default:
            LOG_ERROR("The object must be a Note or a Chord");
            break;
    }
}

void Part::append(const std::vector<std::variant<Note, Chord>>& objs, const int position,
                  const int staveId) {
    for (const auto& obj : objs) {
        append(obj, position, staveId);
    }
}
