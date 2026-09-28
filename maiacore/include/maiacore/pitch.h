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
 * rounding. `Note` holds its written pitch as a Pitch; Pitch itself knows nothing of `Note`.
 *
 * @invariant `isRest() ⟺ _step == "rest" ⟺ !_octave.has_value() ⟹ _alter == 0.0f`, across every
 *            reachable state. A rest is represented internally by an empty octave (see
 *            getOctave()) and reports isRest() == true; isRest() is the authoritative test.
 *
 * @invariant setStep() is permissive on a rest: it resurrects the object into a note, defaulting
 *            the octave to 4 (call setOctave() afterward for a different one). setAlter() and
 *            setOctave() instead refuse on a rest, since neither carries enough information to
 *            resurrect one on its own; setPitch(), setPitchClass() and setMidiNumber() remain the
 *            general way to turn a rest into a note or a note into a rest.
 *
 * @invariant Every non-rest state satisfies `getMidiNumber() >= 0`. A field-level setter whose
 *            argument is individually well-formed but would move an already-constructed Pitch
 *            below MIDI note 0 treats that as a boundary condition, not a caller error: it logs a
 *            warning and leaves the object unchanged, rather than throwing. Malformed input (an
 *            unknown step, a non-finite, non-multiple-of-0.5 or out-of-range alter, an
 *            out-of-range octave) remains a caller error and still throws std::runtime_error.
 */
class Pitch {
   private:
    std::string _step;           ///< Diatonic step ("A".."G"), or "rest" for a rest.
    float _alter;                ///< Accidental value in semitones; may be a quarter-tone fraction
                                  ///< (a multiple of 0.5) within [-2, 2].
    std::optional<int> _octave;  ///< Octave number, or an empty optional for a rest.

    /**
     * @brief Computes the exact, unrounded pitch position in quarter-tone steps for an arbitrary
     *        (step, alter, octave) triple, without reading or mutating this object's state.
     * @details Single implementation, within this class, of
     *          `12 * (octave + 1) + diatonicStepSemitones + alter`, so getQuarterToneSteps()
     *          does not carry its own inline copy of that formula. getMidiNumber() does not use
     *          it: it delegates to Helper::spelling2midiNote(), which evaluates the same sum and
     *          rounds it with roundTiesUpward() (utils.h), the library's single implementation of
     *          the ties-upward rule.
     * @param step Diatonic step ("A".."G").
     * @param alter Accidental value in semitones.
     * @param octave Octave number.
     * @return Quarter-tone steps, unrounded.
     * @throws std::runtime_error If step is not one of "A".."G".
     */
    static float computeQuarterToneSteps(const std::string& step, float alter, int octave);

   public:
    /**
     * @brief Constructs a Pitch from a pitch string.
     * @param pitch Pitch string (e.g., "C4", "C1x4", "Dbb-1"). An empty string or any string
     *        containing "rest" constructs a rest (default: "rest").
     * @throws std::runtime_error If the pitch string is invalid (see Helper::splitPitch()).
     */
    explicit Pitch(const std::string& pitch = "rest");

    /**
     * @brief Constructs a Pitch from a MIDI note number, with an explicit accidental type.
     * @details Delegates to Helper::midiNote2pitch(), so the requested accType must be a valid
     *          spelling for that specific MIDI note (e.g. "#" is rejected for a MIDI note that is
     *          a natural, white-key pitch). A negative midiNumber constructs a rest, matching
     *          setMidiNumber().
     * @param midiNumber MIDI note number (negative values construct a rest).
     * @param accType Accidental type: "" (natural for white keys, "#" for black keys, the
     *        default), "#", "b", "x" or "bb".
     * @throws std::runtime_error If midiNumber cannot be spelled using accType, or if the
     *         resulting octave falls outside -1..11.
     */
    explicit Pitch(int midiNumber, const std::string& accType = {});

    /**
     * @brief Constructs a Pitch from a frequency in Hz.
     * @details Delegates to setFrequency(); see its documentation for the rounding, spelling and
     *          rest rules applied.
     * @param frequency Frequency in Hz. A value <= 0 constructs a rest.
     * @param accType Preferred accidental type for the base semitone spelling: "" (natural for
     *        white keys, "#" for black keys, the default), "#", "b", "x" or "bb". Falls back to
     *        the default spelling, with a warning (LOG_WARN), when the rounded pitch's base
     *        semitone cannot use the requested accidental type.
     * @param freqA4 Reference frequency for A4, in Hz (default: 440.0).
     * @param enableQuarterToneRound When false (default), rounds to the nearest semitone; when
     *        true, rounds to the nearest quarter tone. Ties round upward.
     * @throws std::runtime_error If freqA4 is not a finite number greater than 0, if accType is
     *         not one of the five accepted values, if the active tuning system is not
     *         TuningSystem::EQUAL_TEMPERAMENT, or if frequency is NaN.
     */
    explicit Pitch(float frequency, const std::string& accType = {}, float freqA4 = 440.0f,
                   bool enableQuarterToneRound = false);

    /**
     * @brief Returns the highest MIDI number this class can spell.
     * @details A pure function -- it reads no instance state and has no side effects. "B" double-
     *          sharp at the top octave (c_maxPitchOctave); e.g. "Bx11". Factored out of
     *          clampToRepresentableMidi() (review round 4, task-3-review.md F1) so a caller that
     *          only needs the ceiling *value* -- setFrequency()'s +infinity handling is the only
     *          one today -- does not have to exercise the clamp *branch* to get it. Before this
     *          split, the +infinity path obtained the ceiling via
     *          `clampToRepresentableMidi(INT_MAX)`, which meant a single regression in that one
     *          comparison could make both the ceiling clamp itself AND the +infinity path hang on
     *          an unbounded walk, on every platform -- not the AArch64-specific concern round 2's
     *          N2 guarded against, but a strictly worse, platform-independent version of it.
     * @return The highest representable MIDI number (157 as of this writing: "Bx11").
     */
    static int maxRepresentableMidi();

    /**
     * @brief Clamps an arbitrary MIDI number to the range this class can represent.
     * @details A pure function -- it reads no instance state and has no side effects -- used by
     *          setFrequency() for both ends of its range clamp (review round 3, task-3-review.md
     *          item 2): extracted out of that method so the clamp bound (maxRepresentableMidi())
     *          can be tested directly and deterministically, without going through
     *          setFrequency()'s frequency-to-steps pipeline or timing anything.
     * @param midi MIDI number to clamp, of any magnitude (including values well outside any
     *        audible frequency's range).
     * @return 0 (C-1, the lowest representable pitch) if midi is negative; maxRepresentableMidi()
     *         if midi exceeds it; midi unchanged otherwise.
     */
    static int clampToRepresentableMidi(int midi);

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
     * @details Delegates to Helper::spelling2midiNote(), which rounds with roundTiesUpward()
     *          (utils.h), the library's single implementation of the ties-upward rule.
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
     * @brief Returns this Pitch's frequency in Hz, under the currently active tuning system.
     * @details A rest has no frequency and returns 0.0f. For a non-rest Pitch, only
     *          TuningSystem::EQUAL_TEMPERAMENT (see config.h) is currently implemented; it
     *          returns `freqA4 * 2^((getQuarterToneSteps() - 69) / 12)`, using the exact,
     *          unrounded quarter-tone step position so a quarter-tone alter is never rounded away
     *          in the process (contrast Helper::freq2midiNote(), which rounds to an integer MIDI
     *          number first and is therefore never used here).
     * @param freqA4 Reference frequency for A4, in Hz (default: 440.0).
     * @return Frequency in Hz, or 0.0f for a rest.
     * @throws std::runtime_error If freqA4 is not a finite number greater than 0 (checked for a
     *         rest too), or if the active tuning system is not TuningSystem::EQUAL_TEMPERAMENT.
     */
    float getFrequency(float freqA4 = 440.0f) const;

    /**
     * @brief Sets the diatonic step.
     * @details Permissive on a rest: resurrects this Pitch into a note, defaulting the octave to
     *          4 (call setOctave() afterward to use a different one). On a non-rest Pitch, a step
     *          that would move it below MIDI note 0 is a boundary condition, not a caller error:
     *          it is refused with a warning (LOG_WARN) and this Pitch is left unchanged, rather
     *          than throwing.
     * @param step Diatonic step ("A".."G").
     * @throws std::runtime_error If step is not one of "A".."G".
     */
    void setStep(const std::string& step);

    /**
     * @brief Sets the accidental value in semitones.
     * @details Refuses (warns via LOG_WARN, no-op) when called on a rest, and refuses the same
     *          way when the resulting pitch would fall below MIDI note 0 — both are boundary
     *          conditions, not caller errors, so this Pitch is left unchanged rather than an
     *          exception being thrown. A malformed alter (NaN or infinite, not exactly a
     *          multiple of 0.5, or outside [-2, 2]) remains a caller error and still throws.
     *          Membership of the grid is exact (isOnQuarterToneGrid(), utils.h): a value near a
     *          multiple of 0.5, such as 0.99996, is rejected rather than rounded. The value is
     *          stored canonically: -0.0 is stored as +0.0.
     * @param alter Alter value; must be a multiple of 0.5 (a semitone or quarter-tone step),
     *        within [-2, 2].
     * @throws std::runtime_error If alter is NaN or infinite, is not a multiple of 0.5, or is
     *         outside [-2, 2].
     */
    void setAlter(float alter);

    /**
     * @brief Sets the octave number.
     * @details Refuses (warns via LOG_WARN, no-op) when called on a rest, and refuses the same
     *          way when the resulting pitch would fall below MIDI note 0 — both are boundary
     *          conditions, not caller errors, so this Pitch is left unchanged rather than an
     *          exception being thrown. An out-of-range octave remains a caller error and still
     *          throws.
     * @param octave Octave number, within [-1, 11].
     * @throws std::runtime_error If octave is outside [-1, 11].
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
     * @brief Replaces this Pitch's full state from a frequency in Hz.
     * @details A frequency <= 0 makes this Pitch a rest (spec section 4.4.1's whole-state
     *          replacement, not a caller error, so this never throws for that reason). For a
     *          positive frequency, the exact quarter-tone step position is computed as the
     *          inverse of getFrequency()'s formula -- `12 * log2(frequency / freqA4) + 69` -- then
     *          rounded to the nearest semitone (enableQuarterToneRound == false, the default) or
     *          the nearest quarter tone (enableQuarterToneRound == true). Ties round upward (spec
     *          section 4.5), via `std::floor(x + 0.5)` scaled to the rounding granularity, never
     *          std::round()/std::lround(): those round half away from zero and would disagree
     *          with the ties-upward rule for a negative step position.
     *
     *          The rounded position is split into an integer MIDI number and a residual of 0 or
     *          0.5, spelled through Helper::midiNote2pitch() and Helper::alterValue2symbol(), and
     *          handed to setPitch() -- no new spelling logic is introduced here. This method
     *          *always* produces a note for a positive frequency; it never rejects one (spec
     *          section 4.3):
     *          - `accType` is a preference for the base semitone's spelling, not a demand: when
     *            it does not apply to that specific chromatic degree, or when the base spelling
     *            it produces cannot combine with the quarter-tone residual into a representable
     *            alter, this falls back to the default spelling and logs a warning (LOG_WARN).
     *          - A frequency below the lowest representable pitch (C-1), or above the highest,
     *            is clamped to that extreme and logs a warning (LOG_WARN) -- never a silent rest
     *            (indistinguishable from the one sanctioned rest case, frequency <= 0) and never
     *            a throw.
     *          - +infinity genuinely lies above the representable range, so it is treated the
     *            same as a finite frequency above the highest representable pitch: clamped to
     *            the ceiling and logged with LOG_WARN, never passed to a cast that would be
     *            undefined behaviour for it. -infinity is already a rest, being <= 0.
     *          - NaN satisfies neither half of spec section 4.3's dichotomy ("<= 0" or
     *            "positive"), being unordered under IEEE 754: every comparison against it is
     *            false. It is therefore a caller error, not a boundary condition -- the same
     *            side of the line as a malformed accType or an unimplemented tuning system --
     *            and throws via LOG_ERROR rather than fabricating a pitch from the absence of a
     *            value.
     *
     *          @warning This distinction, and the undefined-behaviour cast it guards against for
     *          +infinity, depend on `-ffast-math` (or an equivalent fast-math build flag) never
     *          being enabled for this translation unit: fast-math permits the compiler to assume
     *          no NaN or infinity value ever occurs and to remove the std::isfinite()/std::isnan()
     *          checks this behaviour relies on. Absent from CMakeLists.txt and setup.py as of
     *          this writing; must stay absent, or be re-verified against this method's non-finite
     *          handling if ever introduced.
     * @param frequency Frequency in Hz. A value <= 0 makes this Pitch a rest.
     * @param accType Preferred accidental type for the base semitone spelling: "" (natural for
     *        white keys, "#" for black keys, the default), "#", "b", "x" or "bb".
     * @param freqA4 Reference frequency for A4, in Hz (default: 440.0).
     * @param enableQuarterToneRound When false (default), rounds to the nearest semitone; when
     *        true, rounds to the nearest quarter tone.
     * @throws std::runtime_error If freqA4 is not a finite number greater than 0 (checked
     *         first, whatever the frequency), if accType is not one of the five accepted values,
     *         if the active tuning system is not TuningSystem::EQUAL_TEMPERAMENT, or if
     *         frequency is NaN.
     */
    void setFrequency(float frequency, const std::string& accType = {}, float freqA4 = 440.0f,
                       bool enableQuarterToneRound = false);

    /**
     * @brief Rounds a quarter-tone alter to the nearest semitone, ties upward.
     * @details Sets alter to `roundTiesUpward(alter)` (utils.h), the same ties-upward rule
     *          getMidiNumber() applies.
     */
    void roundToSemitone();
};
