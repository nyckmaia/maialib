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
    bool isSimple() const;
    bool isCompound() const;
    
    bool isTonal() const;

    bool isMajor(const bool useEnharmony = false) const;
    bool isMinor(const bool useEnharmony = false) const;
    bool isPerfect(const bool useEnharmony = false) const;
    bool isDiminished(const bool useEnharmony = false) const;
    bool isAugmented(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 1 ===== //
    bool isDiminishedUnisson(const bool useEnharmony = false) const;
    bool isPerfectUnisson(const bool useEnharmony = false) const;
    bool isAugmentedUnisson(const bool useEnharmony = false) const;
    bool isMinorSecond(const bool useEnharmony = false) const;
    bool isMajorSecond(const bool useEnharmony = false) const;
    bool isMinorThird(const bool useEnharmony = false) const;
    bool isMajorThird(const bool useEnharmony = false) const;
    bool isPerfectFourth(const bool useEnharmony = false) const;
    bool isAugmentedFourth(const bool useEnharmony = false) const;
    bool isDiminishedFifth(const bool useEnharmony = false) const;
    bool isPerfectFifth(const bool useEnharmony = false) const;
    bool isAugmentedFifth(const bool useEnharmony = false) const;
    bool isMinorSixth(const bool useEnharmony = false) const;
    bool isMajorSixth(const bool useEnharmony = false) const;
    bool isDiminishedSeventh(const bool useEnharmony = false) const;
    bool isMinorSeventh(const bool useEnharmony = false) const;
    bool isMajorSeventh(const bool useEnharmony = false) const;
    bool isDiminishedOctave(const bool useEnharmony = false) const;
    bool isPerfectOctave(const bool useEnharmony = false) const;
    bool isAugmentedOctave(const bool useEnharmony = false) const;
    bool isMinorNinth(const bool useEnharmony = false) const;
    bool isMajorNinth(const bool useEnharmony = false) const;
    bool isPerfectEleventh(const bool useEnharmony = false) const;
    bool isSharpEleventh(const bool useEnharmony = false) const;
    bool isMinorThirdteenth(const bool useEnharmony = false) const;
    bool isMajorThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 2 ===== //
    bool isSecond(const bool useEnharmony = false) const;
    bool isThird(const bool useEnharmony = false) const;
    bool isFourth(const bool useEnharmony = false) const;
    bool isFifth(const bool useEnharmony = false) const;
    bool isSixth(const bool useEnharmony = false) const;
    bool isSeventh(const bool useEnharmony = false) const;
    bool isOctave(const bool useEnharmony = false) const;
    bool isNinth(const bool useEnharmony = false) const;
    bool isEleventh(const bool useEnharmony = false) const;
    bool isThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //
    bool isAnyOctaveMinorSecond(const bool useEnharmony = false) const;
    bool isAnyOctaveMajorSecond(const bool useEnharmony = false) const;
    bool isAnyOctaveMinorThird(const bool useEnharmony = false) const;
    bool isAnyOctaveMajorThird(const bool useEnharmony = false) const;
    bool isAnyOctavePerfectFourth(const bool useEnharmony = false) const;
    bool isAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;
    bool isAnyOctaveDiminishedFifth(const bool useEnharmony = false) const;
    bool isAnyOctavePerfectFifth(const bool useEnharmony = false) const;
    bool isAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;
    bool isAnyOctaveMinorSixth(const bool useEnharmony = false) const;
    bool isAnyOctaveMajorSixth(const bool useEnharmony = false) const;
    bool isAnyOctaveDiminishedSeventh(const bool useEnharmony = false) const;
    bool isAnyOctaveMinorSeventh(const bool useEnharmony = false) const;
    bool isAnyOctaveMajorSeventh(const bool useEnharmony = false) const;
    bool isAnyOctaveDiminishedOctave(const bool useEnharmony = false) const;
    bool isAnyOctavePerfectOctave(const bool useEnharmony = false) const;
    bool isAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //
    bool isAnyOctaveSecond() const;
    bool isAnyOctaveThird() const;
    bool isAnyOctaveFourth() const;
    bool isAnyOctaveFifth() const;
    bool isAnyOctaveSixth() const;
    bool isAnyOctaveSeventh() const;
    bool isAnyOctaveOctave() const;

    bool operator < (const Interval& otherInterval) const {
        return getNumSemitones(true) < otherInterval.getNumSemitones(true);
    }
};
