#include "interval.h"

#include <stdexcept>

#include "constants.h"
#include "utils.h"

Interval::Interval(const std::string& pitch_A, const std::string& pitch_B)
    : Interval(Note(pitch_A), Note(pitch_B)) {}

Interval::Interval(const Note& note_A, const Note& note_B) : _numSemitones(0) {
    // Error checking:
    if (!note_A.isNoteOn() || !note_B.isNoteOn()) {
        LOG_ERROR("Cannot compute the interval between a note and a REST");
    }

    _note.resize(2);
    _note[0] = note_A;
    _note[1] = note_B;

    _numSemitones = _note[1].getMIDINumber() - _note[0].getMIDINumber();
}

void Interval::setNotes(const std::string& pitch_A, const std::string& pitch_B) {
    setNotes(Note(pitch_A), Note(pitch_B));
}

void Interval::setNotes(const Note& note_A, const Note& note_B) {
    // Error checking:
    if (!note_A.isNoteOn() || !note_B.isNoteOn()) {
        LOG_ERROR("Cannot compute the interval between a note and a rest");
    }

    _note.clear();
    _note.resize(2);
    _note[0] = note_A;
    _note[1] = note_B;

    _numSemitones = _note[1].getMIDINumber() - _note[0].getMIDINumber();
}

int Interval::whiteKeyDistance() const {
    const Note& firstNote = _note[0];
    const Note& secondNote = _note[1];

    const std::string firstNoteWhiteKey =
        firstNote.getPitchStep() + std::to_string(firstNote.getOctave());
    const std::string secondNoteWhiteKey =
        secondNote.getPitchStep() + std::to_string(secondNote.getOctave());

    const auto foundFirst =
        std::find(c_pianoWhiteKeys.begin(), c_pianoWhiteKeys.end(), firstNoteWhiteKey);
    const auto foundSecond =
        std::find(c_pianoWhiteKeys.begin(), c_pianoWhiteKeys.end(), secondNoteWhiteKey);

    return std::distance(foundFirst, foundSecond);
}

std::pair<std::string, bool> Interval::analyse() const {
    // Get basic data to compute the interval name
    const int diatonicInterval = getDiatonicInterval(true, true);
    const int numSemitones = getNumSemitones(true);

    // Use 2 switchs to combine 'diatonicSteps' with 'numSemitones'
    switch (diatonicInterval) {
        // ----- UNISSON OR OCTAVE ----- //
        case 1:
            switch (numSemitones) {
                case -120:
                    return {"-10 oct", true};
                case -108:
                    return {"-9 oct", true};
                case -96:
                    return {"-8 oct", true};
                case -84:
                    return {"-7 oct", true};
                case -72:
                    return {"-6 oct", true};
                case -60:
                    return {"-5 oct", true};
                case -48:
                    return {"-4 oct", true};
                case -36:
                    return {"-3 oct", true};
                case -24:
                    return {"-2 oct", true};
                case -12:
                    return {"-1 oct", true};
                case 0:
                    return {"unisson", true};
                case 12:
                    return {"1 oct", true};
                case 24:
                    return {"2 oct", true};
                case 36:
                    return {"3 oct", true};
                case 48:
                    return {"4 oct", true};
                case 60:
                    return {"5 oct", true};
                case 72:
                    return {"6 oct", true};
                case 84:
                    return {"7 oct", true};
                case 96:
                    return {"8 oct", true};
                case 108:
                    return {"9 oct", true};
                case 120:
                    return {"10 oct", true};
            }
            break;

        // ----- SECOND ----- //
        case 2:
            switch (numSemitones % 12) {
                case -3:
                    return {"+++d2", false};
                case -2:
                    return {"++d2", false};
                case -1:
                    return {"+d2", false};
                case 0:
                    return {"d2", false};
                case 1:
                    return {"m2", true};
                case 2:
                    return {"M2", true};
                case 3:
                    return {"A2", false};
                case 4:
                    return {"+A2", false};
                case 5:
                    return {"++A2", false};
                case 6:
                    return {"+++A2", false};
            }
            break;

        // ----- THRID ----- //
        case 3:
            switch (numSemitones % 12) {
                case -1:
                    return {"+++d3", false};
                case 0:
                    return {"++d3", false};
                case 1:
                    return {"+d3", false};
                case 2:
                    return {"d3", false};
                case 3:
                    return {"m3", true};
                case 4:
                    return {"M3", true};
                case 5:
                    return {"A3", false};
                case 6:
                    return {"+A3", false};
                case 7:
                    return {"++A3", false};
                case 8:
                    return {"+++A3", false};
            }
            break;

        // ----- FOURTH ----- //
        case 4:
            switch (numSemitones % 12) {
                case 1:
                    return {"+++d4", false};
                case 2:
                    return {"++d4", false};
                case 3:
                    return {"+d4", false};
                case 4:
                    return {"d4", false};
                case 5:
                    return {"P4", true};
                case 6:
                    return {"A4", true};
                case 7:
                    return {"+A4", false};
                case 8:
                    return {"++A4", false};
                case 9:
                    return {"+++A4", false};
            }

            break;

        // ----- FIFTH ----- //
        case 5:
            switch (numSemitones % 12) {
                case 3:
                    return {"+++d5", false};
                case 4:
                    return {"++d5", false};
                case 5:
                    return {"+d5", false};
                case 6:
                    return {"d5", true};
                case 7:
                    return {"P5", true};
                case 8:
                    return {"A5", true};
                case 9:
                    return {"+A5", false};
                case 10:
                    return {"++A5", false};
                case 11:
                    return {"+++A5", false};
            }

            break;

        // ----- SIXTH ----- //
        case 6:
            switch (numSemitones % 12) {
                case 4:
                    return {"+++d6", false};
                case 5:
                    return {"++d6", false};
                case 6:
                    return {"+d6", false};
                case 7:
                    return {"d6", false};
                case 8:
                    return {"m6", true};
                case 9:
                    return {"M6", true};
                case 10:
                    return {"A6", false};
                case 11:
                    return {"+A6", false};
                case 12:
                    return {"++A6", false};
                case 13:
                    return {"+++A6", false};
            }

            break;

        // ----- SEVENTH ----- //
        case 7:
            switch (numSemitones % 12) {
                case 6:
                    return {"+++d7", false};
                case 7:
                    return {"++d7", false};
                case 8:
                    return {"+d7", false};
                case 9:
                    return {"d7", true};
                case 10:
                    return {"m7", true};
                case 11:
                    return {"M7", true};
                case 0:
                    return {"A7", false};  // Special case: Using mod 12
                                           // in switch statement
                case 13:
                    return {"+A7", false};
                case 14:
                    return {"++A7", false};
                case 15:
                    return {"+++A7", false};
            }

            break;

        default:
            LOG_ERROR("Unknown 'diatonicInterval' value: " + std::to_string(diatonicInterval));
    }

    // Only to avoid the 'no-return' compiler warning
    return {"", false};
}

std::string Interval::getName() const { return analyse().first; }

int Interval::getNumSemitones(const bool absoluteValue) const {
    return (absoluteValue) ? abs(_numSemitones) : _numSemitones;
}

int Interval::getNumOctaves(const bool absoluteValue) const {
    const int diff = _note[1].getOctave() - _note[0].getOctave();

    return (absoluteValue) ? abs(diff) : diff;
}

int Interval::getDiatonicInterval(const bool useSingleOctave, const bool absoluteValue) const {
    const int diatonicSteps = getDiatonicSteps(useSingleOctave, absoluteValue);
    if (diatonicSteps == 0) {
        return 1;
    }

    const int diatonicInterval = diatonicSteps + 1;

    if (isAscendant()) {
        return diatonicInterval;
    }

    return (absoluteValue) ? abs(diatonicInterval) : diatonicSteps - 1;
}

int Interval::getDiatonicSteps(const bool useSingleOctave, const bool absoluteValue) const {
    const int diatonicScaleSize = (absoluteValue) ? c_C_diatonicScale.size()
                                                  : static_cast<int>(c_C_diatonicScale.size()) * -1;

    // Get the diatonic values of each interval note
    const std::string& diatonic_A = _note[0].getPitchStep();
    const std::string& diatonic_B = _note[1].getPitchStep();

    // Get the iterator of each diatonic note
    const auto& itA = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonic_A);
    const auto& itB = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonic_B);

    // Compute the diatonic distance
    const int diatonicDistance =
        std::distance(itA, itB) + (getNumOctaves() * c_C_diatonicScale.size());

    const int x = (absoluteValue) ? abs(diatonicDistance) : diatonicDistance;

    return (useSingleOctave) ? x % diatonicScaleSize : x;
}

int Interval::getPitchStepInterval() const {
    // Get the diatonic values of each interval note
    const std::string& diatonic_A = _note[0].getPitchStep();
    const std::string& diatonic_B = _note[1].getPitchStep();

    // Select the chord stack using the 'possible root' note
    const std::array<std::string, 7>* diatonicScale = nullptr;

    switch (hash(diatonic_A.c_str())) {
        case hash("C"):
            diatonicScale = &c_C_diatonicScale;
            break;
        case hash("D"):
            diatonicScale = &c_D_diatonicScale;
            break;
        case hash("E"):
            diatonicScale = &c_E_diatonicScale;
            break;
        case hash("F"):
            diatonicScale = &c_F_diatonicScale;
            break;
        case hash("G"):
            diatonicScale = &c_G_diatonicScale;
            break;
        case hash("A"):
            diatonicScale = &c_A_diatonicScale;
            break;
        case hash("B"):
            diatonicScale = &c_B_diatonicScale;
            break;
        default:
            LOG_ERROR("Invalid diatonic pitch step");
            break;
    }

    // Compute the distance between the interval first and second pitch step
    const int noteDistance =
        std::distance(diatonicScale->begin(),
                      std::find(diatonicScale->begin(), diatonicScale->end(), diatonic_B));

    return noteDistance + 1;
}

std::vector<Note> Interval::getNotes() const { return _note; }

bool Interval::isAscendant() const { return (whiteKeyDistance() > 0) ? true : false; }

bool Interval::isDescendant() const { return (whiteKeyDistance() < 0) ? true : false; }

bool Interval::isSimple() const { return (_numSemitones <= 12) ? true : false; }

bool Interval::isCompound() const { return !isSimple(); }

bool Interval::isTonal() const { return analyse().second; }

bool Interval::isMajor(const bool useEnharmony) const {
    return isAnyOctaveMajorSecond(useEnharmony) || isAnyOctaveMajorThird(useEnharmony) ||
           isAnyOctaveMajorSixth(useEnharmony) || isAnyOctaveMajorSeventh(useEnharmony);
}

bool Interval::isMinor(const bool useEnharmony) const {
    return isAnyOctaveMinorSecond(useEnharmony) || isAnyOctaveMinorThird(useEnharmony) ||
           isAnyOctaveMinorSixth(useEnharmony) || isAnyOctaveMinorSeventh(useEnharmony);
}

bool Interval::isPerfect(const bool useEnharmony) const {
    return isAnyOctavePerfectFourth(useEnharmony) || isAnyOctavePerfectFifth(useEnharmony) ||
           isAnyOctavePerfectOctave(useEnharmony);
}

bool Interval::isDiminished(const bool useEnharmony) const {
    return isAnyOctaveDiminishedFifth(useEnharmony) || isAnyOctaveDiminishedSeventh(useEnharmony) ||
           isAnyOctaveDiminishedOctave(useEnharmony);
}

bool Interval::isAugmented(const bool useEnharmony) const {
    return isAnyOctaveAugmentedFourth(useEnharmony) || isAnyOctaveAugmentedFifth(useEnharmony) ||
           isAnyOctaveAugmentedOctave(useEnharmony);
}

bool Interval::isDiminishedUnisson(const bool useEnharmony) const {
    return (useEnharmony) ? _numSemitones == -1
                          : _numSemitones == -1 && getDiatonicInterval(false, true) == 1;
}

bool Interval::isPerfectUnisson(const bool useEnharmony) const {
    return (useEnharmony) ? _numSemitones == 0
                          : _numSemitones == 0 && getDiatonicInterval(false, true) == 1;
}

bool Interval::isAugmentedUnisson(const bool useEnharmony) const {
    return (useEnharmony) ? _numSemitones == 1
                          : _numSemitones == 1 && getDiatonicInterval(false, true) == 1;
}

bool Interval::isMinorSecond(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 1
                          : absNumSemitones == 1 && getDiatonicInterval(false, true) == 2;
}

bool Interval::isMajorSecond(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 2
                          : absNumSemitones == 2 && getDiatonicInterval(false, true) == 2;
}

bool Interval::isMinorThird(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 3
                          : absNumSemitones == 3 && getDiatonicInterval(false, true) == 3;
}

bool Interval::isMajorThird(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 4
                          : absNumSemitones == 4 && getDiatonicInterval(false, true) == 3;
}

bool Interval::isPerfectFourth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 5
                          : absNumSemitones == 5 && getDiatonicInterval(false, true) == 4;
}

bool Interval::isAugmentedFourth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 6
                          : absNumSemitones == 6 && getDiatonicInterval(false, true) == 4;
}

bool Interval::isDiminishedFifth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 6
                          : absNumSemitones == 6 && getDiatonicInterval(false, true) == 5;
}

bool Interval::isPerfectFifth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 7
                          : absNumSemitones == 7 && getDiatonicInterval(false, true) == 5;
}

bool Interval::isAugmentedFifth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 8
                          : absNumSemitones == 8 && getDiatonicInterval(false, true) == 5;
}

bool Interval::isMinorSixth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 8
                          : absNumSemitones == 8 && getDiatonicInterval(false, true) == 6;
}

bool Interval::isMajorSixth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 9
                          : absNumSemitones == 9 && getDiatonicInterval(false, true) == 6;
}

bool Interval::isDiminishedSeventh(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 9
                          : absNumSemitones == 9 && getDiatonicInterval(false, true) == 7;
}

bool Interval::isMinorSeventh(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 10
                          : absNumSemitones == 10 && getDiatonicInterval(false, true) == 7;
}

bool Interval::isMajorSeventh(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 11
                          : absNumSemitones == 11 && getDiatonicInterval(false, true) == 7;
}

bool Interval::isPerfectOctave(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 12
                          : absNumSemitones == 12 && getDiatonicInterval(true, true) == 1;
}

bool Interval::isDiminishedOctave(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 11
                          : absNumSemitones == 11 && getDiatonicInterval(true, true) == 1;
}

bool Interval::isAugmentedOctave(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 13
                          : absNumSemitones == 13 && getDiatonicInterval(true, true) == 1;
}

bool Interval::isMinorNinth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 13
                          : absNumSemitones == 13 && getDiatonicInterval(true, true) == 2;
}

bool Interval::isMajorNinth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 14
                          : absNumSemitones == 14 && getDiatonicInterval(true, true) == 2;
}

bool Interval::isPerfectEleventh(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 17
                          : absNumSemitones == 17 && getDiatonicInterval(true, true) == 4;
}

bool Interval::isSharpEleventh(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 18
                          : absNumSemitones == 18 && getDiatonicInterval(true, true) == 4;
}

bool Interval::isMinorThirdteenth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 20
                          : absNumSemitones == 20 && getDiatonicInterval(true, true) == 6;
}

bool Interval::isMajorThirdteenth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 21
                          : absNumSemitones == 21 && getDiatonicInterval(true, true) == 6;
}

bool Interval::isSecond(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 1 || absNumSemitones == 2
                          : getDiatonicInterval(false, true) == 2;
}

bool Interval::isThird(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 3 || absNumSemitones == 4
                          : getDiatonicInterval(false, true) == 3;
}

bool Interval::isFourth(const bool useEnharmony) const {
    const int absNumSemitones = abs(_numSemitones);
    return (useEnharmony) ? absNumSemitones == 5 || absNumSemitones == 6
                          : getDiatonicInterval(false, true) == 4;
}

bool Interval::isFifth(const bool useEnharmony) const {
    return (useEnharmony)
               ? isDiminishedFifth(true) || isPerfectFifth(true) || isAugmentedFifth(true)
               : getDiatonicInterval(false, true) == 5;
}

bool Interval::isSixth(const bool useEnharmony) const {
    return (useEnharmony) ? isMinorSixth(true) || isMajorSixth(true)
                          : getDiatonicInterval(false, true) == 6;
}

bool Interval::isSeventh(const bool useEnharmony) const {
    return (useEnharmony)
               ? isDiminishedSeventh(true) || isMinorSeventh(true) || isMajorSeventh(true)
               : getDiatonicInterval(false, true) == 7;
}

bool Interval::isOctave(const bool useEnharmony) const {
    return (useEnharmony)
               ? isDiminishedOctave(true) || isPerfectOctave(true) || isAugmentedOctave(true)
               : getDiatonicInterval(false, true) == 8;
}

bool Interval::isNinth(const bool useEnharmony) const {
    return (useEnharmony) ? isMinorNinth(true) || isMajorNinth(true)
                          : getDiatonicInterval(false, true) == 9;
}

bool Interval::isEleventh(const bool useEnharmony) const {
    return (useEnharmony) ? isPerfectEleventh(true) || isSharpEleventh(true)
                          : getDiatonicInterval(false, true) == 11;
}

bool Interval::isThirdteenth(const bool useEnharmony) const {
    return (useEnharmony) ? isMinorThirdteenth(true) || isMajorThirdteenth(true)
                          : getDiatonicInterval(false, true) == 13;
}

bool Interval::isAnyOctaveMinorSecond(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 1;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 2;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMajorSecond(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 2;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 2;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMinorThird(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 3;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 3;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMajorThird(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 4;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 3;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctavePerfectFourth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 5;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 4;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveAugmentedFourth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 6;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 4;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveDiminishedFifth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 6;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 5;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctavePerfectFifth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 7;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 5;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveAugmentedFifth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 8;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 5;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMinorSixth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 8;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 6;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMajorSixth(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 9;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 6;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveDiminishedSeventh(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 9;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 7;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMinorSeventh(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 10;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 7;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveMajorSeventh(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 11;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 7;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveDiminishedOctave(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 11;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 1;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctavePerfectOctave(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 0;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 1;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveAugmentedOctave(const bool useEnharmony) const {
    const int CloseOctaveSemitonesDistance = 1;
    const bool isAscMinorSecond = (_numSemitones - CloseOctaveSemitonesDistance) % 12 == 0;
    const bool isDescMinorSecond = (_numSemitones + CloseOctaveSemitonesDistance) % 12 == 0;

    if (useEnharmony) {
        return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
    }

    const bool diatonicIntervalCheck = getDiatonicInterval(true, true) == 1;
    if (!diatonicIntervalCheck) {
        return false;
    }

    return (_numSemitones > 0) ? isAscMinorSecond : isDescMinorSecond;
}

bool Interval::isAnyOctaveSecond() const { return getDiatonicInterval(true, true) == 2; }

bool Interval::isAnyOctaveThird() const { return getDiatonicInterval(true, true) == 3; }

bool Interval::isAnyOctaveFourth() const { return getDiatonicInterval(true, true) == 4; }

bool Interval::isAnyOctaveFifth() const { return getDiatonicInterval(true, true) == 5; }

bool Interval::isAnyOctaveSixth() const { return getDiatonicInterval(true, true) == 6; }

bool Interval::isAnyOctaveSeventh() const { return getDiatonicInterval(true, true) == 7; }

bool Interval::isAnyOctaveOctave() const { return getDiatonicInterval(true, true) == 1; }
