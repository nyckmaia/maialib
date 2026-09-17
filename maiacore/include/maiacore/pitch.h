#pragma once

#include <optional>
#include <string>

/**
 * @brief Represents a single musical pitch as canonical (step, alter, octave) state, independent
 * of any Note or score context.
 *
 * The Pitch class stores a pitch using its diatonic step ("A".."G"), an accidental value in
 * semitones that may carry a quarter-tone fraction (a multiple of 0.5, within [-2, 2]), and an
 * octave number. It provides pitch-string round-tripping, MIDI conversion and quarter-tone-aware
 * rounding. This class is the foundation `Note` will be rebuilt on in a later task; it is
 * deliberately standalone and does not interact with `Note`. A rest is represented internally by
 * an empty octave (see getOctave()) and reports isRest() == true.
 */
class Pitch {
   private:
    std::string _step;           ///< Diatonic step ("A".."G"), or "rest" for a rest.
    float _alter;                ///< Accidental value in semitones; may be a quarter-tone fraction
                                  ///< (a multiple of 0.5) within [-2, 2].
    std::optional<int> _octave;  ///< Octave number, or an empty optional for a rest.

   public:
    /**
     * @brief Constructs a Pitch from a pitch string.
     * @param pitch Pitch string (e.g., "C4", "C1x4", "Dbb-1"). An empty string or any string
     *        containing "rest" constructs a rest (default: "rest").
     * @throws std::runtime_error If the pitch string is invalid (see Helper::splitPitch()).
     */
    explicit Pitch(const std::string& pitch = "rest");

    /**
     * @brief Returns the full pitch string (pitch class followed by octave).
     * @return Pitch string (e.g., "C1x4"), or "rest" for a rest.
     */
    std::string getPitch() const;

    /**
     * @brief Returns the pitch class: diatonic step followed by accidental symbol, without octave.
     * @return Pitch class string (e.g., "C1x", "Bb"), or "rest" for a rest.
     */
    std::string getPitchClass() const;

    /**
     * @brief Returns the diatonic step.
     * @return Step string ("A".."G"), or "rest" for a rest.
     */
    std::string getPitchStep() const;

    /**
     * @brief Returns the accidental symbol.
     * @return Accidental symbol (e.g., "1x", "#", "bb", ""), or "" for a rest.
     */
    std::string getAlterSymbol() const;

    /**
     * @brief Returns the accidental value in semitones.
     * @return Alter value (e.g., 0.5f for a quarter-tone sharp), or 0.0f for a rest.
     */
    float getAlter() const;

    /**
     * @brief Returns the octave number.
     * @details An empty optional means this Pitch is a rest. isRest() is the authoritative test
     *          for whether this Pitch is a rest; do not infer that solely from this value being
     *          empty.
     * @return Octave number, or an empty optional for a rest.
     */
    std::optional<int> getOctave() const;

    /**
     * @brief Returns the MIDI note number, rounding a quarter-tone alter half upward to the
     *        nearest semitone.
     * @details Delegates to Helper::spelling2midiNote(), the single implementation of the
     *          ties-upward rounding rule, so it is never re-implemented here.
     * @return MIDI note number, or MUSIC_XML::MIDI::NUMBER::MIDI_REST for a rest.
     */
    int getMidiNumber() const;

    /**
     * @brief Returns the exact, unrounded pitch position in quarter-tone steps.
     * @details Computed as `12 * (octave + 1) + diatonicStepSemitones + alter`, without rounding,
     *          so a quarter-tone alter (e.g. 0.5f) is preserved in the fractional part.
     * @return Quarter-tone steps, or MUSIC_XML::MIDI::NUMBER::MIDI_REST (as a float) for a rest.
     */
    float getQuarterToneSteps() const;

    /**
     * @brief Checks whether this Pitch is a rest.
     * @return True if this Pitch is a rest.
     */
    bool isRest() const;

    /**
     * @brief Sets the diatonic step.
     * @details If this Pitch was a rest, its octave is initialized to the default octave (4).
     * @param step Diatonic step ("A".."G").
     * @throws std::runtime_error If step is not one of "A".."G".
     */
    void setStep(const std::string& step);

    /**
     * @brief Sets the accidental value in semitones.
     * @param alter Alter value; must be a multiple of 0.5 (a semitone or quarter-tone step),
     *        within [-2, 2].
     * @throws std::runtime_error If alter is not a multiple of 0.5, or is outside [-2, 2].
     */
    void setAlter(float alter);

    /**
     * @brief Sets the octave number.
     * @param octave Octave number, within [-1, 11].
     * @throws std::runtime_error If called on a rest, or if octave is outside [-1, 11].
     */
    void setOctave(int octave);

    /**
     * @brief Replaces this Pitch's full state by re-parsing a pitch string.
     * @param pitch Pitch string (e.g., "C4", "C1x4", "Dbb-1"). An empty string or any string
     *        containing "rest" makes this Pitch a rest.
     * @throws std::runtime_error If the pitch string is invalid (see Helper::splitPitch()).
     */
    void setPitch(const std::string& pitch);

    /**
     * @brief Replaces this Pitch's step and alter from a pitch class string, keeping the current
     *        octave (or defaulting it to 4 if this Pitch was a rest).
     * @param pitchClass Pitch class string (e.g., "C1x", "Bb"). Any string containing "rest" makes
     *        this Pitch a rest.
     * @throws std::runtime_error If the pitch class string is invalid (see Helper::splitPitch()).
     */
    void setPitchClass(const std::string& pitchClass);

    /**
     * @brief Replaces this Pitch's full state from a MIDI note number.
     * @details Delegates to Helper::midiNote2pitch(), spelled with the natural/sharp default
     *          accidental type.
     * @param midiNumber MIDI note number (negative values make this Pitch a rest).
     * @throws std::runtime_error If midiNumber cannot be spelled within octaves -1..11.
     */
    void setMidiNumber(int midiNumber);

    /**
     * @brief Rounds a quarter-tone alter to the nearest semitone, ties upward.
     * @details Sets alter to `std::floor(alter + 0.5f)`, matching the ties-upward rounding rule
     *          used by getMidiNumber().
     */
    void roundToSemitone();
};
