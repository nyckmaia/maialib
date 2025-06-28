#pragma once

#include <string>
#include <vector>

#include "maiacore/note.h"

/**
 * @brief Represents a musical interval between two notes, supporting tonal, diatonic, and chromatic analysis.
 *
 * The Interval class provides methods for constructing, analyzing, and classifying musical intervals.
 * It supports both pitch string and Note object input, and offers detailed intervallic queries for music analysis and computational musicology.
 */
class Interval {
   private:
    std::vector<Note> _note; ///< The two notes defining the interval.
    int _numSemitones;       ///< Number of semitones between the notes.

    /**
     * @brief Analyzes the interval and returns its name and tonal status.
     * @return Pair of interval name (string) and tonal status (bool).
     */
    std::pair<std::string, bool> analyse() const;

    /**
     * @brief Computes the distance between the two notes in terms of white keys (C-D-E-F-G-A-B).
     * @return Integer representing the white key distance.
     */
    int whiteKeyDistance() const;

   public:
    /**
     * @brief Constructs an Interval from two pitch strings.
     * @param pitch_A First pitch string (default: "C4").
     * @param pitch_B Second pitch string (default: "C4").
     */
    Interval(const std::string& pitch_A = "C4", const std::string& pitch_B = "C4");

    /**
     * @brief Constructs an Interval from two Note objects.
     * @param note_A First Note.
     * @param note_B Second Note.
     */
    Interval(const Note& note_A, const Note& note_B);

    /**
     * @brief Sets the notes of the interval using pitch strings.
     * @param pitch_A First pitch string.
     * @param pitch_B Second pitch string.
     */
    void setNotes(const std::string& pitch_A, const std::string& pitch_B);

    /**
     * @brief Sets the notes of the interval using Note objects.
     * @param note_A First Note.
     * @param note_B Second Note.
     */
    void setNotes(const Note& note_A, const Note& note_B);

    /**
     * @brief Returns the interval name (e.g., "M3", "P5", "m2").
     * @return String with the interval name.
     */
    std::string getName() const;

    /**
     * @brief Returns the number of semitones between the notes.
     * @param absoluteValue If true, returns absolute value.
     * @return Number of semitones.
     */
    int getNumSemitones(const bool absoluteValue = false) const;

    /**
     * @brief Returns the number of octaves between the notes.
     * @param absoluteValue If true, returns absolute value.
     * @return Number of octaves.
     */
    int getNumOctaves(const bool absoluteValue = false) const;

    /**
     * @brief Returns the diatonic interval number (e.g., 3 for a third).
     * @param useSingleOctave If true, computes within a single octave.
     * @param absoluteValue If true, returns absolute value.
     * @return Diatonic interval number.
     */
    int getDiatonicInterval(const bool useSingleOctave = true,
                            const bool absoluteValue = false) const;

    /**
     * @brief Returns the number of diatonic steps between the notes.
     * @param useSingleOctave If true, computes within a single octave.
     * @param absoluteValue If true, returns absolute value.
     * @return Number of diatonic steps.
     */
    int getDiatonicSteps(const bool useSingleOctave = true, const bool absoluteValue = false) const;

    /**
     * @brief Returns the pitch step interval (e.g., 1 for unison, 2 for second).
     * @return Pitch step interval as integer.
     */
    int getPitchStepInterval() const;

    /**
     * @brief Returns the two notes defining the interval.
     * @return Vector of Note objects.
     */
    std::vector<Note> getNotes() const;

    /**
     * @brief Returns true if the interval is ascending.
     * @return True if ascending.
     */
    bool isAscendant() const;

    /**
     * @brief Returns true if the interval is descending.
     * @return True if descending.
     */
    bool isDescendant() const;

    /**
     * @brief Returns the direction of the interval as a string ("asc", "desc", or empty for unison).
     * @return Direction string.
     */
    std::string getDirection() const;

    /**
     * @brief Returns true if the interval is simple (within an octave).
     * @return True if simple.
     */
    bool isSimple() const;

    /**
     * @brief Returns true if the interval is compound (greater than an octave).
     * @return True if compound.
     */
    bool isCompound() const;

    /**
     * @brief Returns true if the interval is tonal (major, minor, perfect, etc.).
     * @return True if tonal.
     */
    bool isTonal() const;

    /**
     * @brief Returns true if the interval is major.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major.
     */
    bool isMajor(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is minor.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor.
     */
    bool isMinor(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is perfect.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if perfect.
     */
    bool isPerfect(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is diminished.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if diminished.
     */
    bool isDiminished(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is augmented.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if augmented.
     */
    bool isAugmented(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 1 ===== //

    /**
     * @brief Returns true if the interval is a diminished unison.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if diminished unison.
     */
    bool isDiminishedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect unison.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if perfect unison.
     */
    bool isPerfectUnisson(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented unison.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if augmented unison.
     */
    bool isAugmentedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor second.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor second.
     */
    bool isMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major second.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major second.
     */
    bool isMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor third.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor third.
     */
    bool isMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major third.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major third.
     */
    bool isMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect fourth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if perfect fourth.
     */
    bool isPerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented fourth (tritone).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if augmented fourth.
     */
    bool isAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a diminished fifth (tritone).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if diminished fifth.
     */
    bool isDiminishedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect fifth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if perfect fifth.
     */
    bool isPerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented fifth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if augmented fifth.
     */
    bool isAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor sixth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor sixth.
     */
    bool isMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major sixth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major sixth.
     */
    bool isMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a diminished seventh.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if diminished seventh.
     */
    bool isDiminishedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor seventh.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor seventh.
     */
    bool isMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major seventh.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major seventh.
     */
    bool isMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a diminished octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if diminished octave.
     */
    bool isDiminishedOctave(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if perfect octave.
     */
    bool isPerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if augmented octave.
     */
    bool isAugmentedOctave(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor ninth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor ninth.
     */
    bool isMinorNinth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major ninth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major ninth.
     */
    bool isMajorNinth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect eleventh.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if perfect eleventh.
     */
    bool isPerfectEleventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a sharp eleventh.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if sharp eleventh.
     */
    bool isSharpEleventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor thirteenth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if minor thirteenth.
     */
    bool isMinorThirdteenth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major thirteenth.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if major thirteenth.
     */
    bool isMajorThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 2 ===== //

    /**
     * @brief Returns true if the interval is a generic second (major or minor).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if second.
     */
    bool isSecond(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a generic third (major or minor).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if third.
     */
    bool isThird(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a generic fourth (perfect, augmented, or diminished).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if fourth.
     */
    bool isFourth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a generic fifth (perfect, augmented, or diminished).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if fifth.
     */
    bool isFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a generic sixth (major or minor).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if sixth.
     */
    bool isSixth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a generic seventh (major, minor, or diminished).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if seventh.
     */
    bool isSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an octave (perfect, augmented, or diminished).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if octave.
     */
    bool isOctave(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a ninth (major or minor).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if ninth.
     */
    bool isNinth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an eleventh (perfect or sharp).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if eleventh.
     */
    bool isEleventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a thirteenth (major or minor).
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if thirteenth.
     */
    bool isThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //

    /**
     * @brief Returns true if the interval is a minor second in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave minor second.
     */
    bool isAnyOctaveMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major second in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave major second.
     */
    bool isAnyOctaveMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor third in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave minor third.
     */
    bool isAnyOctaveMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major third in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave major third.
     */
    bool isAnyOctaveMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect fourth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave perfect fourth.
     */
    bool isAnyOctavePerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented fourth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave augmented fourth.
     */
    bool isAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a diminished fifth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave diminished fifth.
     */
    bool isAnyOctaveDiminishedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect fifth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave perfect fifth.
     */
    bool isAnyOctavePerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented fifth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave augmented fifth.
     */
    bool isAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor sixth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave minor sixth.
     */
    bool isAnyOctaveMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major sixth in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave major sixth.
     */
    bool isAnyOctaveMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a diminished seventh in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave diminished seventh.
     */
    bool isAnyOctaveDiminishedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a minor seventh in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave minor seventh.
     */
    bool isAnyOctaveMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a major seventh in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave major seventh.
     */
    bool isAnyOctaveMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a diminished octave in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave diminished octave.
     */
    bool isAnyOctaveDiminishedOctave(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is a perfect octave in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave perfect octave.
     */
    bool isAnyOctavePerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief Returns true if the interval is an augmented octave in any octave.
     * @param useEnharmony If true, considers enharmonic equivalence.
     * @return True if any-octave augmented octave.
     */
    bool isAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //

    /**
     * @brief Returns true if the interval is a generic second (any octave).
     * @return True if any-octave second.
     */
    bool isAnyOctaveSecond() const;

    /**
     * @brief Returns true if the interval is a generic third (any octave).
     * @return True if any-octave third.
     */
    bool isAnyOctaveThird() const;

    /**
     * @brief Returns true if the interval is a generic fourth (any octave).
     * @return True if any-octave fourth.
     */
    bool isAnyOctaveFourth() const;

    /**
     * @brief Returns true if the interval is a generic fifth (any octave).
     * @return True if any-octave fifth.
     */
    bool isAnyOctaveFifth() const;

    /**
     * @brief Returns true if the interval is a generic sixth (any octave).
     * @return True if any-octave sixth.
     */
    bool isAnyOctaveSixth() const;

    /**
     * @brief Returns true if the interval is a generic seventh (any octave).
     * @return True if any-octave seventh.
     */
    bool isAnyOctaveSeventh() const;

    /**
     * @brief Returns true if the interval is a generic octave (any octave).
     * @return True if any-octave octave.
     */
    bool isAnyOctaveOctave() const;

    /**
     * @brief Returns the interval size in cents (for microtonal/tuning analysis).
     * @return Interval size in cents.
     */
    int toCents() const;

    /**
     * @brief Less-than operator for comparing intervals by number of semitones.
     * @param otherInterval Interval to compare.
     * @return True if this interval is smaller.
     */
    bool operator<(const Interval& otherInterval) const {
        return getNumSemitones(true) < otherInterval.getNumSemitones(true);
    }
};
