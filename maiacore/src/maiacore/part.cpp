#include "maiacore/part.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cherno/instrumentor.h"
#include "maiacore/chord.h"
#include "maiacore/clef.h"
#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/utils.h"
#include "pitch-views.h"

namespace {
// A pitched note's transposition as the MusicXML writer writes it: the diatonic interval the note
// is spelled with (the conventional one for the chromatic interval when the stored one is 0), the
// chromatic interval and the octave doubling. A stored 0 and the conventional interval it stands
// for are one transposition.
struct WrittenTransposition {
    std::int64_t diatonic = 0;
    std::int64_t chromatic = 0;
    OctaveDoubling doubling = OctaveDoubling::NONE;

    bool operator==(const WrittenTransposition& other) const {
        return diatonic == other.diatonic && chromatic == other.chromatic &&
               doubling == other.doubling;
    }
    bool operator!=(const WrittenTransposition& other) const { return !(*this == other); }
};

WrittenTransposition writtenTransposition(const Note& note) {
    WrittenTransposition transposition;
    transposition.diatonic = maiacore::detail::spelledDiatonicInterval(
        note.getTransposeDiatonic(), note.getTransposeChromatic());
    transposition.chromatic = note.getTransposeChromatic();
    transposition.doubling = note.getOctaveDoubling();
    return transposition;
}

// The transposition of the first pitched note of a staff of a measure, if it has one.
std::optional<WrittenTransposition> firstPitchedTransposition(const Measure& measure,
                                                              const int staff) {
    if (staff >= measure.getNumStaves()) {
        return std::nullopt;
    }
    for (int n = 0; n < measure.getNumNotes(staff); n++) {
        const Note& note = measure.getNote(n, staff);
        if (note.isNoteOn() && note.isPitched()) {
            return writtenTransposition(note);
        }
    }
    return std::nullopt;
}

// A <transpose> element: number="staff + 1", or no number when 'staff' is -1. The interval is
// unfolded: <octave-change> takes the chromatic interval's whole octaves, rounded toward zero,
// and <diatonic> and <chromatic> what remains of each; <octave-change> is written only when it is
// not 0, as MusicXML asks for intervals of less than an octave.
std::string transposeXML(const WrittenTransposition& transposition, const int staff,
                         const int identSize) {
    const std::int64_t octaves = transposition.chromatic / 12;
    const std::string inner = Helper::generateIdentation(5, identSize);
    std::string xml = Helper::generateIdentation(4, identSize) + "<transpose";
    if (staff >= 0) {
        xml.append(" number=\"" + std::to_string(staff + 1) + "\"");
    }
    xml.append(">\n");
    xml.append(inner + "<diatonic>" + std::to_string(transposition.diatonic - 7 * octaves) +
               "</diatonic>\n");
    xml.append(inner + "<chromatic>" + std::to_string(transposition.chromatic - 12 * octaves) +
               "</chromatic>\n");
    if (octaves != 0) {
        xml.append(inner + "<octave-change>" + std::to_string(octaves) + "</octave-change>\n");
    }
    if (transposition.doubling == OctaveDoubling::BELOW) {
        xml.append(inner + "<double/>\n");
    } else if (transposition.doubling == OctaveDoubling::ABOVE) {
        xml.append(inner + "<double above=\"yes\"/>\n");
    }
    xml.append(Helper::generateIdentation(4, identSize) + "</transpose>\n");
    return xml;
}

// Where a part's <transpose> elements go, derived from its notes, which hold the transpositions:
// the elements of each measure's <attributes>, and the <attributes> written before a note, keyed
// by (staff, note index).
struct TransposePlan {
    std::vector<std::string> measureStart;
    std::vector<std::map<std::pair<int, int>, std::string>> beforeNote;
};

// Each staff's transposition in force is that of its last pitched note; rests and unpitched notes
// change nothing. Measure 1 states each staff's first pitched note's transposition, which applies
// from the start of the part, when it is not (0, 0, NONE). A later change goes into the
// <attributes> at the start of its measure when the staff's first pitched note there brings it,
// and otherwise into an <attributes> written just before the first note of the chord that brings
// it. A <transpose> has no number when every staff has the same transposition at that point; a
// change in the middle of a measure of a part with more than one staff always has one, as the
// staves are written one after another. A chord whose notes differ cannot be written.
TransposePlan transposePlan(const Part& part, const int identSize) {
    const int numMeasures = part.getNumMeasures();
    TransposePlan plan;
    plan.measureStart.resize(numMeasures);
    plan.beforeNote.resize(numMeasures);

    int numStaves = part.getNumStaves();
    for (int m = 0; m < numMeasures; m++) {
        numStaves = std::max(numStaves, part.getMeasure(m).getNumStaves());
    }

    // The transposition in force on each staff, from the start: that of its first pitched note.
    std::vector<WrittenTransposition> current(numStaves);
    std::vector<bool> found(numStaves, false);
    for (int m = 0; m < numMeasures; m++) {
        for (int s = 0; s < numStaves; s++) {
            if (found[s]) {
                continue;
            }
            const std::optional<WrittenTransposition> first =
                firstPitchedTransposition(part.getMeasure(m), s);
            if (first) {
                current[s] = *first;
                found[s] = true;
            }
        }
    }

    for (int m = 0; m < numMeasures; m++) {
        const Measure& measure = part.getMeasure(m);

        // At the start of the measure: in measure 1, every staff that is transposed or doubled;
        // later, every staff whose first pitched note in the measure changes its transposition.
        std::vector<int> changed;
        for (int s = 0; s < numStaves; s++) {
            if (m == 0) {
                if (current[s] != WrittenTransposition{}) {
                    changed.push_back(s);
                }
                continue;
            }
            const std::optional<WrittenTransposition> first = firstPitchedTransposition(measure, s);
            if (first && *first != current[s]) {
                current[s] = *first;
                changed.push_back(s);
            }
        }
        if (!changed.empty()) {
            const WrittenTransposition shared = current[changed.front()];
            const bool everyStaffAlike =
                std::all_of(current.begin(), current.end(),
                            [&shared](const WrittenTransposition& t) { return t == shared; });
            if (everyStaffAlike) {
                plan.measureStart[m] = transposeXML(shared, -1, identSize);
            } else {
                for (const int s : changed) {
                    plan.measureStart[m] += transposeXML(current[s], s, identSize);
                }
            }
        }

        // Inside the measure: a pitched note whose transposition differs from that of the
        // previous pitched note of its staff, after the staff's first one there.
        for (int s = 0; s < measure.getNumStaves(); s++) {
            bool firstPitched = true;
            int chordStart = 0;
            std::optional<WrittenTransposition> chordTransposition;
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                const Note& note = measure.getNote(n, s);
                if (!note.inChord()) {
                    chordStart = n;
                    chordTransposition.reset();
                }
                if (!note.isNoteOn() || !note.isPitched()) {
                    continue;
                }
                const WrittenTransposition transposition = writtenTransposition(note);
                if (chordTransposition && *chordTransposition != transposition) {
                    LOG_ERROR("Part '" + part.getName() + "', measure " + std::to_string(m + 1) +
                              ", staff " + std::to_string(s + 1) +
                              ": the notes of a chord have different transposing intervals or "
                              "octave doublings, which one MusicXML <transpose> cannot express. "
                              "Give every note of the chord the same interval and doubling.");
                }
                chordTransposition = transposition;
                if (firstPitched) {
                    firstPitched = false;
                    continue;
                }
                if (transposition != current[s]) {
                    current[s] = transposition;
                    const int number = (numStaves > 1) ? s : -1;
                    plan.beforeNote[m][{s, chordStart}] =
                        Helper::generateIdentation(3, identSize) + "<attributes>\n" +
                        transposeXML(transposition, number, identSize) +
                        Helper::generateIdentation(3, identSize) + "</attributes>\n";
                }
            }
        }
    }
    return plan;
}
}  // namespace

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

void Part::setTransposingInterval(const int diatonicInterval, const int chromaticInterval,
                                  const int measureStart, const int measureEnd, const int staff,
                                  const OctaveDoubling doubling) {
    const int numMeasures = getNumMeasures();
    const int end = (measureEnd == -1) ? numMeasures : measureEnd;
    if (measureStart < 0 || end < measureStart || end > numMeasures) {
        throw std::out_of_range("Part::setTransposingInterval: the measures [" +
                                std::to_string(measureStart) + ", " + std::to_string(measureEnd) +
                                ") are not a range of the part's " + std::to_string(numMeasures) +
                                " measures");
    }
    if (staff < -1 || staff >= _numStaves) {
        throw std::out_of_range("Part::setTransposingInterval: staff " + std::to_string(staff) +
                                " is neither -1 nor one of the part's " +
                                std::to_string(_numStaves) + " staves");
    }

    // Every pitched note of the range is checked before any is changed, so a note that cannot
    // sound with the interval leaves the part as it was.
    std::vector<Note*> notes;
    for (int m = measureStart; m < end; m++) {
        Measure& measure = _measure[m];
        for (int s = 0; s < measure.getNumStaves(); s++) {
            if (staff != -1 && s != staff) {
                continue;
            }
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                Note& note = measure.getNote(n, s);
                if (!note.isNoteOn() || !note.isPitched()) {
                    continue;
                }
                if (!maiacore::detail::soundsWithinRange(note, diatonicInterval,
                                                         chromaticInterval)) {
                    LOG_ERROR("Part::setTransposingInterval: with the transposing interval (" +
                              std::to_string(diatonicInterval) + ", " +
                              std::to_string(chromaticInterval) + "), the written " +
                              note.getWrittenPitch() + " at measure index " + std::to_string(m) +
                              ", staff index " + std::to_string(s) +
                              " would sound below the lowest representable pitch, C1b-1, or "
                              "above B11 (MIDI note 155) where its letter cannot spell it; no "
                              "note was changed.");
                }
                notes.push_back(&note);
            }
        }
    }

    for (Note* note : notes) {
        note->setTransposingInterval(diatonicInterval, chromaticInterval);
        note->setOctaveDoubling(doubling);
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
    // +1 to make measureEnd inclusive (as per documentation)
    _measure.erase(_measure.begin() + measureStart, _measure.begin() + measureEnd + 1);
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
    const TransposePlan transposes = transposePlan(*this, identSize);

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
        attributeChanged |= !transposes.measureStart[m].empty();

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

        // After clef and staff-details, as the MusicXML content model of <attributes> requires.
        xml.append(transposes.measureStart[m]);

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

        xml.append(_measure[m].toXML(instrumentId, identSize, transposes.beforeNote[m]));

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
            // The remainder would need to be tied into the next measure. Check that it exists
            // *before* writing anything, so a failure here leaves the Part untouched instead of
            // holding an orphan tie-start note with no tie-stop partner. A blank measure is not
            // appended automatically: that is a feature decision (what time signature/key/clef
            // would it inherit, should getNumMeasures() silently change under the caller), and
            // Part has no visibility into Score::addMeasure() to do it properly anyway.
            if (m == numMeasures - 1) {
                LOG_ERROR(
                    "Unable to append the note: it doesn't fit in the last measure and there "
                    "is no following measure to hold the tied remainder. Add another measure "
                    "before appending a note that overflows the last one.");
            }

            Note first = note;
            first.setDuration({emptySpace, divisionsPerQuarterNote});
            first.setTieStart();

            Note second = note;
            second.setDuration({abs(diff), divisionsPerQuarterNote});
            second.setTieStop();

            _measure[m].addNote(first, staveId, position);
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
            // See the matching comment in appendNote() for why there is no auto-append here.
            // Checked before any write lands, so a failure leaves the Part untouched instead of
            // holding orphan tie-start notes with no tie-stop partners.
            if (m == numMeasures - 1) {
                LOG_ERROR(
                    "Unable to append the chord: it doesn't fit in the last measure and there "
                    "is no following measure to hold the tied remainder. Add another measure "
                    "before appending a chord that overflows the last one.");
            }

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
