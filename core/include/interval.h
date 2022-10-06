#pragma once

#include <string>
#include <vector>
#include <algorithm>  // std::find

#include "note.h"

class Interval
{

private:
    std::vector<Note> _note;
    int _numSemitones;

    std::pair<std::string, bool> analyse() const;
    int whiteKeyDistance() const;

public:
    Interval(const std::string& pitch_A = "C4", const std::string& pitch_B = "C4");
    Interval(const Note& note_A, const Note& note_B);

    void setNotes(const std::string& pitch_A, const std::string& pitch_B);
    void setNotes(const Note& note_A, const Note& note_B);

    std::string getName() const;
    int getNumSemitones(const bool absoluteValue = false) const;
    int getNumOctaves(const bool absoluteValue = false) const;
    int getDiatonicInterval(const bool useSingleOctave = true, const bool absoluteValue = false) const;
    int getDiatonicSteps(const bool useSingleOctave = true, const bool absoluteValue = false) const;
    int getPitchStepInterval() const;
    std::vector<Note> getNotes() const;
    bool isAscendant() const;
    bool isDescendant() const;
    
    bool isTonal() const;

    bool isUnisson(const bool useEnharmony = false) const;
    
    // bool isSecond(const bool useEnharmony = false) const;
    // bool isMinorSecond(const bool useEnharmony = false) const;
    // bool isMajorSecond(const bool useEnharmony = false) const;
    // bool isThird(const bool useEnharmony = false) const;
    // bool isFourth(const bool useEnharmony = false) const;
    // bool isPerfectFourth(const bool useEnharmony = false) const;
    // bool isAugmentedFourth(const bool useEnharmony = false) const;
    // bool isFifth(const bool useEnharmony = false) const;
    // bool isSixth(const bool useEnharmony = false) const;
    // bool isMinorSixth(const bool useEnharmony = false) const;
    // bool isMajorSixth(const bool useEnharmony = false) const;
    // bool isSeventh(const bool useEnharmony = false) const;
    // bool isNinth(const bool useEnharmony = false) const;
    // bool isEleventh(const bool useEnharmony = false) const;
    // bool isThirdteenth(const bool useEnharmony = false) const;

    bool isMinorThird() const;
    bool isMajorThird() const;
    bool isDiminishedFifth() const;
    bool isPerfectFifth() const;
    bool isAugmentedFifth() const;
    bool isDiminishedSeventh() const;
    bool isMinorSeventh() const;
    bool isMajorSeventh() const;
    bool isMinorNinth() const;
    bool isMajorNinth() const;
    bool isPerfectEleventh() const;
    bool isSharpEleventh() const;
    bool isMinorThirdteenth() const;
    bool isMajorThirdteenth() const;

    bool isSimple() const;
    bool isCompound() const;

    bool operator < (const Interval& otherInterval) const {
        return getNumSemitones(true) < otherInterval.getNumSemitones(true);
    }
};
