#include "interval.h"

Interval::Interval(const std::string& pitch_A, const std::string& pitch_B) :
    Interval(Note(pitch_A), Note(pitch_B))
{

}

Interval::Interval(const Note& note_A, const Note& note_B) :
    _numSemitones(0)
{
    // Error checking:
    if (!note_A.isNoteOn() || !note_B.isNoteOn()) {
        throw std::runtime_error("Cannot compute the interval between a note and a REST");
    }

    _note.push_back(note_A);
    _note.push_back(note_B);

    _numSemitones = _note[1].getMIDINumber() - _note[0].getMIDINumber();
}

std::pair<std::string, bool> Interval::analyse() const
{
    // Get basic data to compute the interval name
    const int diatonicInterval = getDiatonicInterval(true);
    const int numSemitones = getNumSemitones();

    // Use 2 switchs to combine 'diatonicSteps' with 'numSemitones'
    switch (diatonicInterval) {
        // ----- UNISSON OR OCTAVE ----- //
        case 0:
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
        case 1:
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
        case 2:
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
        case 3:
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
        case 4:
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
        case 5:
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
        case 6:
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
            throw std::runtime_error("Unknown 'diatonicInterval' value: " + std::to_string(diatonicInterval));
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

int Interval::getDiatonicInterval(const bool absoluteValue) const
{
    return getDiatonicSteps(absoluteValue) % c_C_diatonicScale.size();
}

int Interval::getDiatonicSteps(const bool absoluteValue) const
{
    // Get the diatonic values of each interval note
    const std::string diatonic_A = _note[0].getPitch().substr(0, 1);
    const std::string diatonic_B = _note[1].getPitch().substr(0, 1);

    // Get the iterator of each diatonic note
    const auto& itA = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonic_A);
    const auto& itB = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonic_B);

    // Compute the diatonic distance
    const int diatonicDistance = std::distance(itA, itB) + (getNumOctaves() * c_C_diatonicScale.size());

    return (absoluteValue) ? abs(diatonicDistance) : diatonicDistance;
}

std::vector<Note> Interval::getNotes() const
{
    return _note;
}

bool Interval::isAscendant() const
{
    return (_numSemitones <= 0) ? false : true;
}

bool Interval::isDescendant() const
{
    return (_numSemitones >= 0) ? false : true;
}

bool Interval::isTonal() const
{
    return analyse().second;
}
