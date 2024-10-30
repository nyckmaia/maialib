#pragma once

#include <string>
#include <vector>

#include "maiacore/note.h"

class Interval {
   private:
    std::vector<Note> _note;
    int _numSemitones;

    std::pair<std::string, bool> analyse() const;
    int whiteKeyDistance() const;

   public:
    /**
     * @brief Construct a new Interval object
     *
     * @param pitch_A
     * @param pitch_B
     */
    Interval(const std::string& pitch_A = "C4", const std::string& pitch_B = "C4");

    /**
     * @brief Construct a new Interval object
     *
     * @param note_A
     * @param note_B
     */
    Interval(const Note& note_A, const Note& note_B);

    /**
     * @brief Set the Notes object
     *
     * @param pitch_A
     * @param pitch_B
     */
    void setNotes(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Set the Notes object
     *
     * @param note_A
     * @param note_B
     */
    void setNotes(const Note& note_A, const Note& note_B);

    /**
     * @brief Get the Name object
     *
     * @return std::string
     */
    std::string getName() const;

    /**
     * @brief Get the Num Semitones object
     *
     * @param absoluteValue
     * @return int
     */
    int getNumSemitones(const bool absoluteValue = false) const;

    /**
     * @brief Get the Num Octaves object
     *
     * @param absoluteValue
     * @return int
     */
    int getNumOctaves(const bool absoluteValue = false) const;

    /**
     * @brief Get the Diatonic Interval object
     *
     * @param useSingleOctave
     * @param absoluteValue
     * @return int
     */
    int getDiatonicInterval(const bool useSingleOctave = true,
                            const bool absoluteValue = false) const;
    /**
     * @brief Get the Diatonic Steps object
     *
     * @param useSingleOctave
     * @param absoluteValue
     * @return int
     */
    int getDiatonicSteps(const bool useSingleOctave = true, const bool absoluteValue = false) const;

    /**
     * @brief Get the Pitch Step Interval object
     *
     * @return int
     */
    int getPitchStepInterval() const;

    /**
     * @brief Get the Notes object
     *
     * @return std::vector<Note>
     */
    std::vector<Note> getNotes() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAscendant() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isDescendant() const;

    /**
     * @brief Get the Direction object
     * 
     * @return std::string 
     */
    std::string getDirection() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isSimple() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isCompound() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isTonal() const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajor(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinor(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isPerfect(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isDiminished(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAugmented(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 1 ===== //
    bool isDiminishedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isPerfectUnisson(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAugmentedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isPerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isDiminishedFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isPerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isDiminishedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isDiminishedOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isPerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAugmentedOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinorNinth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajorNinth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isPerfectEleventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isSharpEleventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMinorThirdteenth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isMajorThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 2 ===== //

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isNinth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isEleventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctavePerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveDiminishedFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctavePerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveDiminishedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveDiminishedOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctavePerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool isAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveSecond() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveThird() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveFourth() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveFifth() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveSixth() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveSeventh() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyOctaveOctave() const;

    /**
     * @brief
     *
     * @return int
     */
    int toCents() const;

    /**
     * @brief
     *
     * @param otherInterval
     * @return true
     * @return false
     */
    bool operator<(const Interval& otherInterval) const {
        return getNumSemitones(true) < otherInterval.getNumSemitones(true);
    }
};
