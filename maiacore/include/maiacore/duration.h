#pragma once

#include <string>

#include "maiacore/constants.h"
#include "maiacore/fraction.h"

/**
 * @brief Represents the duration of a musical note, supporting multiple temporal representations.
 *
 * The Duration class provides methods for converting between quarter note values, ticks, rhythm figures,
 * tuplets, and fractional durations. It is designed for music analysis, computational musicology, and
 * MusicXML processing, allowing precise control over note durations in various formats.
 */
class Duration {
   public:
    /**
     * @brief Constructs a Duration from a quarter note value.
     * @param quarterDuration Duration in quarter notes (e.g., 1.0 for a quarter note, 0.5 for an eighth).
     * @param divisionsPerQuarterNote Number of ticks per quarter note (default: 256).
     */
    Duration(float quarterDuration, int divisionsPerQuarterNote = 256);

    /**
     * @brief Constructs a Duration from ticks, with optional tuplet information.
     * @param ticks Duration in ticks.
     * @param divisionsPerQuarterNote Number of ticks per quarter note (default: 256).
     * @param actualNotes Tuplet numerator (default: 1).
     * @param normalNotes Tuplet denominator (default: 1).
     */
    Duration(int ticks, int divisionsPerQuarterNote = 256, int actualNotes = 1,
             int normalNotes = 1);

    /**
     * @brief Sets the duration using a quarter note value.
     * @param quarterDuration Duration in quarter notes.
     * @param divisionsPerQuarterNote Number of ticks per quarter note (default: 256).
     */
    void setQuarterDuration(float quarterDuration, int divisionsPerQuarterNote = 256);

    /**
     * @brief Returns the rhythm figure (e.g., QUARTER, EIGHTH) for this duration.
     * @return RhythmFigure enum value.
     */
    RhythmFigure getRhythmFigure() const;

    /**
     * @brief Returns the MusicXML note type string for this duration (e.g., "quarter", "eighth").
     * @return Note type string.
     */
    const std::string getNoteType() const;

    /**
     * @brief Returns the number of dots (augmentation dots) for this duration.
     * @return Number of dots (0, 1, or 2).
     */
    int getDots() const;

    /**
     * @brief Returns the duration in ticks.
     * @return Duration in ticks.
     */
    int getTicks() const;

    /**
     * @brief Returns the number of ticks per quarter note.
     * @return Divisions per quarter note.
     */
    int getDivisionsPerQuarterNote() const;

    /**
     * @brief Returns the duration as a Fraction object.
     * @return Fraction representing the duration.
     */
    const Fraction& getFractionDuration() const;

    /**
     * @brief Returns the duration as a string in fractional notation (e.g., "1/4").
     * @return String representing the fractional duration.
     */
    std::string getFractionDurationAsString() const;

    /**
     * @brief Returns the duration in quarter notes as a float.
     * @return Duration in quarter notes.
     */
    float getQuarterDuration() const;

    /**
     * @brief Returns true if the duration represents a tuplet (e.g., triplet, quintuplet).
     * @return True if tuplet.
     */
    bool isTuplet() const;

    /**
     * @brief Sets the tuplet values for the duration.
     * @param actualNotes Tuplet numerator (number of notes played).
     * @param normalNotes Tuplet denominator (number of notes expected).
     * @param normalType Note type for the tuplet (e.g., "eighth").
     */
    void setTupleValues(const int actualNotes, const int normalNotes,
                        const std::string& normalType);

    /**
     * @brief Returns the tuplet numerator (actual notes).
     * @return Integer representing the actual notes in the tuplet.
     */
    int getTimeModificationActualNotes() const;

    /**
     * @brief Returns the tuplet denominator (normal notes).
     * @return Integer representing the normal notes in the tuplet.
     */
    int getTimeModificationNormalNotes() const;

    /**
     * @brief Returns the note type string for the tuplet's normal notes.
     * @return String representing the normal note type (e.g., "eighth").
     */
    std::string getTimeModificationNormalType() const;

   private:
    int _numDots; ///< Number of augmentation dots.
    int _ticks; ///< Duration in ticks.
    float _quarterDuration; ///< Duration in quarter notes.
    int _divisionsPerQuarterNote; ///< Number of ticks per quarter note.
    int _actualNotes; ///< Tuplet numerator.
    int _normalNotes; ///< Tuplet denominator.
    RhythmFigure _rhythmFigure; ///< Rhythm figure (e.g., QUARTER, EIGHTH).
    Fraction _fractionDuration; ///< Fractional representation of duration.
};