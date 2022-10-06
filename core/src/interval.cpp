#include "interval.h"
#include "constants.h"
#include "utils.h"

#include <stdexcept>

Interval::Interval(const std::string& pitch_A, const std::string& pitch_B) :
    Interval(Note(pitch_A), Note(pitch_B))
{

}

Interval::Interval(const Note& note_A, const Note& note_B) :
    _numSemitones(0)
{
    // Error checking:
    if (!note_A.isNoteOn() || !note_B.isNoteOn()) {
        throw std::runtime_error("[maiacore] Cannot compute the interval between a note and a REST");
    }

    _note.resize(2);
    _note[0] = note_A;
    _note[1] = note_B;

    _numSemitones = _note[1].getMIDINumber() - _note[0].getMIDINumber();
}

void Interval::setNotes(const std::string& pitch_A, const std::string& pitch_B)
{
    setNotes(Note(pitch_A), Note(pitch_B));
}

void Interval::setNotes(const Note& note_A, const Note& note_B)
{
    // Error checking:
    if (!note_A.isNoteOn() || !note_B.isNoteOn()) {
        throw std::runtime_error("[maiacore] Cannot compute the interval between a note and a rest");
    }

    _note.clear();
    _note.resize(2);
    _note[0] = note_A;
    _note[1] = note_B;

    _numSemitones = _note[1].getMIDINumber() - _note[0].getMIDINumber();
}

int Interval::whiteKeyDistance() const
{
    const Note& firstNote = _note[0];
    const Note& secondNote = _note[1];

    const std::string firstNoteWhiteKey = firstNote.getPitchStep() + std::to_string(firstNote.getOctave());
    const std::string secondNoteWhiteKey = secondNote.getPitchStep() + std::to_string(secondNote.getOctave());
    
    const auto foundFirst = std::find(c_pianoWhiteKeys.begin(), c_pianoWhiteKeys.end(), firstNoteWhiteKey);
    const auto foundSecond = std::find(c_pianoWhiteKeys.begin(), c_pianoWhiteKeys.end(), secondNoteWhiteKey);
    
    return std::distance(foundFirst, foundSecond);
}

std::pair<std::string, bool> Interval::analyse() const
{
    // Get basic data to compute the interval name
    const int diatonicInterval = getDiatonicInterval(true, true);
    const int numSemitones = getNumSemitones(true);

    // Use 2 switchs to combine 'diatonicSteps' with 'numSemitones'
    switch (diatonicInterval) {
        // ----- UNISSON OR OCTAVE ----- //
        case 1:
            switch (numSemitones) {
                case -120: return std::make_pair<std::string, bool>("-10 oct", true);
                case -108: return std::make_pair<std::string, bool>("-9 oct", true);
                case -96:  return std::make_pair<std::string, bool>("-8 oct", true);
                case -84:  return std::make_pair<std::string, bool>("-7 oct", true);
                case -72:  return std::make_pair<std::string, bool>("-6 oct", true);
                case -60:  return std::make_pair<std::string, bool>("-5 oct", true);
                case -48:  return std::make_pair<std::string, bool>("-4 oct", true);
                case -36:  return std::make_pair<std::string, bool>("-3 oct", true);
                case -24:  return std::make_pair<std::string, bool>("-2 oct", true);
                case -12:  return std::make_pair<std::string, bool>("-1 oct", true);
                case 0:    return std::make_pair<std::string, bool>("unisson", true);
                case 12:   return std::make_pair<std::string, bool>("1 oct", true);
                case 24:   return std::make_pair<std::string, bool>("2 oct", true);
                case 36:   return std::make_pair<std::string, bool>("3 oct", true);
                case 48:   return std::make_pair<std::string, bool>("4 oct", true);
                case 60:   return std::make_pair<std::string, bool>("5 oct", true);
                case 72:   return std::make_pair<std::string, bool>("6 oct", true);
                case 84:   return std::make_pair<std::string, bool>("7 oct", true);
                case 96:   return std::make_pair<std::string, bool>("8 oct", true);
                case 108:  return std::make_pair<std::string, bool>("9 oct", true);
                case 120:  return std::make_pair<std::string, bool>("10 oct", true);
            }
            break;

        // ----- SECOND ----- //
        case 2:
            switch (numSemitones % 12) {
                case -3: return std::make_pair<std::string, bool>("+++d2", false);
                case -2: return std::make_pair<std::string, bool>("++d2", false);
                case -1: return std::make_pair<std::string, bool>("+d2", false);
                case 0:  return std::make_pair<std::string, bool>("d2", false);
                case 1:  return std::make_pair<std::string, bool>("m2", true);
                case 2:  return std::make_pair<std::string, bool>("M2", true);
                case 3:  return std::make_pair<std::string, bool>("A2", false);
                case 4:  return std::make_pair<std::string, bool>("+A2", false);
                case 5:  return std::make_pair<std::string, bool>("++A2", false);
                case 6:  return std::make_pair<std::string, bool>("+++A2", false);
            }
            break;

        // ----- THRID ----- //
        case 3:
            switch (numSemitones % 12) {
                case -1: return std::make_pair<std::string, bool>("+++d3", false);
                case 0: return std::make_pair<std::string, bool>("++d3", false);
                case 1: return std::make_pair<std::string, bool>("+d3", false);
                case 2: return std::make_pair<std::string, bool>("d3", false);
                case 3: return std::make_pair<std::string, bool>("m3", true);
                case 4: return std::make_pair<std::string, bool>("M3", true);
                case 5: return std::make_pair<std::string, bool>("A3", false);
                case 6: return std::make_pair<std::string, bool>("+A3", false);
                case 7: return std::make_pair<std::string, bool>("++A3", false);
                case 8: return std::make_pair<std::string, bool>("+++A3", false);
            }
            break;

        // ----- FOURTH ----- //
        case 4:
            switch (numSemitones % 12) {
                case 1: return std::make_pair<std::string, bool>("+++d4", false);
                case 2: return std::make_pair<std::string, bool>("++d4", false);
                case 3: return std::make_pair<std::string, bool>("+d4", false);
                case 4: return std::make_pair<std::string, bool>("d4", false);
                case 5: return std::make_pair<std::string, bool>("P4", true);
                case 6: return std::make_pair<std::string, bool>("A4", true);
                case 7: return std::make_pair<std::string, bool>("+A4", false);
                case 8: return std::make_pair<std::string, bool>("++A4", false);
                case 9: return std::make_pair<std::string, bool>("+++A4", false);
            }

            break;

        // ----- FIFTH ----- //
        case 5:
            switch (numSemitones % 12) {
                case 3:  return std::make_pair<std::string, bool>("+++d5", false);
                case 4:  return std::make_pair<std::string, bool>("++d5", false);
                case 5:  return std::make_pair<std::string, bool>("+d5", false);
                case 6:  return std::make_pair<std::string, bool>("d5", true);
                case 7:  return std::make_pair<std::string, bool>("P5", true);
                case 8:  return std::make_pair<std::string, bool>("A5", true);
                case 9:  return std::make_pair<std::string, bool>("+A5", false);
                case 10: return std::make_pair<std::string, bool>("++A5", false);
                case 11: return std::make_pair<std::string, bool>("+++A5", false);
            }

            break;

        // ----- SIXTH ----- //
        case 6:
            switch (numSemitones % 12) {
                case 4:  return std::make_pair<std::string, bool>("+++d6", false);
                case 5:  return std::make_pair<std::string, bool>("++d6", false);
                case 6:  return std::make_pair<std::string, bool>("+d6", false);
                case 7:  return std::make_pair<std::string, bool>("d6", false);
                case 8:  return std::make_pair<std::string, bool>("m6", true);
                case 9:  return std::make_pair<std::string, bool>("M6", true);
                case 10: return std::make_pair<std::string, bool>("A6", false);
                case 11: return std::make_pair<std::string, bool>("+A6", false);
                case 12: return std::make_pair<std::string, bool>("++A6", false);
                case 13: return std::make_pair<std::string, bool>("+++A6", false);
            }

            break;

        // ----- SEVENTH ----- //
        case 7:
            switch (numSemitones % 12) {
                case 6:  return std::make_pair<std::string, bool>("+++d7", false);
                case 7:  return std::make_pair<std::string, bool>("++d7", false);
                case 8:  return std::make_pair<std::string, bool>("+d7", false);
                case 9:  return std::make_pair<std::string, bool>("d7", true);
                case 10: return std::make_pair<std::string, bool>("m7", true);
                case 11: return std::make_pair<std::string, bool>("M7", true);
                case 0:  return std::make_pair<std::string, bool>("A7", false); // Special case: Using mod 12 in switch statement
                case 13: return std::make_pair<std::string, bool>("+A7", false);
                case 14: return std::make_pair<std::string, bool>("++A7", false);
                case 15: return std::make_pair<std::string, bool>("+++A7", false);
            }

            break;

        default:
            throw std::runtime_error("[maiacore] Unknown 'diatonicInterval' value: " + std::to_string(diatonicInterval));
    }

    // Only to avoid the 'no-return' compiler warning
    return std::make_pair<std::string, bool>("", false);
}

std::string Interval::getName() const
{
    return analyse().first;
}

int Interval::getNumSemitones(const bool absoluteValue) const
{
    return (absoluteValue) ? abs(_numSemitones) : _numSemitones;
}

int Interval::getNumOctaves(const bool absoluteValue) const
{
    const int diff = _note[1].getOctave() - _note[0].getOctave();

    return (absoluteValue) ? abs(diff) : diff;
}

int Interval::getDiatonicInterval(const bool useSingleOctave, const bool absoluteValue) const
{
    const int diatonicSteps = getDiatonicSteps(useSingleOctave, absoluteValue);
    if (diatonicSteps == 0) { return 1; }

    const int diatonicInterval = diatonicSteps + 1;

    if (isAscendant()) { return diatonicInterval; }

    return (absoluteValue) ? abs(diatonicInterval) : diatonicSteps - 1;
}

int Interval::getDiatonicSteps(const bool useSingleOctave, const bool absoluteValue) const
{
    const int diatonicScaleSize = (absoluteValue) ? c_C_diatonicScale.size() : static_cast<int>(c_C_diatonicScale.size()) * -1;

    // Get the diatonic values of each interval note
    const std::string& diatonic_A = _note[0].getPitchStep();
    const std::string& diatonic_B = _note[1].getPitchStep();

    // Get the iterator of each diatonic note
    const auto& itA = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonic_A);
    const auto& itB = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonic_B);

    // Compute the diatonic distance
    const int diatonicDistance = std::distance(itA, itB) + (getNumOctaves() * c_C_diatonicScale.size());

    const int x = (absoluteValue) ? abs(diatonicDistance) : diatonicDistance;
    
    return (useSingleOctave) ? x % diatonicScaleSize : x;
}

int Interval::getPitchStepInterval() const
{
    // Get the diatonic values of each interval note
    const std::string& diatonic_A = _note[0].getPitchStep();
    const std::string& diatonic_B = _note[1].getPitchStep();

    // Select the chord stack using the 'possible root' note
    const std::array<std::string, 7>* diatonicScale = nullptr;

    switch (hash(diatonic_A.c_str())) {
        case hash("C"): diatonicScale = &c_C_diatonicScale; break;
        case hash("D"): diatonicScale = &c_D_diatonicScale; break;
        case hash("E"): diatonicScale = &c_E_diatonicScale; break;
        case hash("F"): diatonicScale = &c_F_diatonicScale; break;
        case hash("G"): diatonicScale = &c_G_diatonicScale; break;
        case hash("A"): diatonicScale = &c_A_diatonicScale; break;
        case hash("B"): diatonicScale = &c_B_diatonicScale; break;
        default: throw std::runtime_error("[maiacore] Invalid diatonic pitch step"); break;
    }

    // Compute the distance between the interval first and second pitch step
    const int noteDistance = std::distance(diatonicScale->begin(), std::find(diatonicScale->begin(), diatonicScale->end(), diatonic_B));
    
    return noteDistance + 1;
}


std::vector<Note> Interval::getNotes() const
{
    return _note;
}

bool Interval::isAscendant() const
{
    return (whiteKeyDistance() > 0) ? true : false;
}

bool Interval::isDescendant() const
{
    return (whiteKeyDistance() < 0) ? true : false;
}

bool Interval::isUnisson(const bool useEnharmony) const
{
    return (useEnharmony) ? _note[0].getMIDINumber() == _note[1].getMIDINumber() : 
        _note[0].getPitch() == _note[1].getPitch();
}

bool Interval::isTonal() const
{
    return analyse().second;
}

bool Interval::isMinorThird() const
{
    return (getName() == "m3") ? true : false;
}

bool Interval::isMajorThird() const
{
    return (getName() == "M3") ? true : false;
}

bool Interval::isDiminishedFifth() const
{
    return (getName() == "d5") ? true : false;
}

bool Interval::isPerfectFifth() const
{
    return (getName() == "P5") ? true : false;
}

bool Interval::isAugmentedFifth() const
{
    return (getName() == "A5") ? true : false;
}

bool Interval::isDiminishedSeventh() const
{
    return (getName() == "d7") ? true : false;
}

bool Interval::isMinorSeventh() const
{
    return (getName() == "m7") ? true : false;
}

bool Interval::isMajorSeventh() const
{
    return (getName() == "M7") ? true : false;
}

bool Interval::isMinorNinth() const
{
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones(true);

    if (diatonicInterval == 2 && (numSemitones % 13) == 0) { return true; }
    
    return false;
}

bool Interval::isMajorNinth() const
{
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones(true);

    if (diatonicInterval == 2 && (numSemitones % 14) == 0) { return true; }
    
    return false;
}

bool Interval::isPerfectEleventh() const
{
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones(true);

    if (diatonicInterval == 4 && (numSemitones % 17) == 0) { return true; }
    
    return false;
}

bool Interval::isSharpEleventh() const
{
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones(true);

    if (diatonicInterval == 4 && (numSemitones % 18) == 0) { return true; }
    
    return false;
}

bool Interval::isMinorThirdteenth() const
{
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones(true);

    if (diatonicInterval == 6 && (numSemitones % 20) == 0) { return true; }
    
    return false;
}

bool Interval::isMajorThirdteenth() const
{
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones(true);

    if (diatonicInterval == 6 && (numSemitones % 21) == 0) { return true; }
    
    return false;
}

bool Interval::isSimple() const
{
    return (_numSemitones <= 12) ? true : false; 
}

bool Interval::isCompound() const
{
    return !isSimple();
}
