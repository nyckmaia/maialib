/**
 * @file config.h
 * @brief Global configuration for maiacore tuning system selection.
 *
 * Provides runtime configuration of pitch-to-frequency conversion models for historical
 * performance practice research, microtonal analysis, and period-appropriate acoustical studies.
 * The tuning system affects all frequency-dependent operations throughout the library.
 *
 * Default configuration: Equal Temperament (12-TET, modern standard tuning).
 */

/**
 * @brief Enumeration of supported tuning systems for pitch-to-frequency conversion.
 * @details Different tuning systems alter the frequency ratios between pitches, directly impacting
 *          psychoacoustic dissonance calculations, spectral content analysis, and harmonic perception
 *          studies. Selection of tuning system is critical for:
 *          - Historical musicology (Renaissance, Baroque, Classical period works)
 *          - Microtonal composition and analysis
 *          - Cross-cultural music research (non-Western tuning practices)
 *          - Acoustical modeling of period instruments
 *
 *          The active tuning system affects Note::getFrequency() and all derived spectral operations.
 */
enum class TuningSystem {
    /**
     * @brief 12-tone equal temperament (12-TET), modern standard tuning.
     * @details Each semitone is exactly 2^(1/12) ≈ 1.05946 frequency ratio.
     *          All intervals are equally tempered, enabling free modulation to all keys
     *          with identical interval quality. Standard for contemporary Western music
     *          since the late 19th century. Pitch reference: A4 = 440 Hz (configurable).
     */
    EQUAL_TEMPERAMENT,

    /**
     * @brief Just intonation using pure integer frequency ratios.
     * @details Intervals are derived from simple integer ratios (3:2 perfect fifth, 5:4 major third,
     *          4:3 perfect fourth, etc.), producing maximum consonance for tonal harmony but limiting
     *          modulation due to comma accumulation. Historically used in vocal music, a cappella
     *          ensembles, and theoretical tuning studies. Particularly relevant for Renaissance
     *          polyphony and harmonic overtone analysis.
     */
    JUST_INTONATION,

    /**
     * @brief Pythagorean tuning based on stacked perfect fifths (3:2 ratio).
     * @details Generates the diatonic scale by iterating pure 3:2 fifths, producing bright major thirds
     *          (81:64 ≈ 408 cents, sharper than equal temperament's 400 cents) and narrow minor thirds.
     *          Favors melodic purity and linear counterpoint over harmonic consonance. Dominant tuning
     *          system in Medieval and early Renaissance music. The Pythagorean comma (≈24 cents) limits
     *          enharmonic equivalence.
     */
    PYTHAGOREAN_TUNING,

    /**
     * @brief Quarter-comma meantone temperament, standard Renaissance/Baroque tuning.
     * @details Tempers perfect fifths narrow by 1/4 syntonic comma to achieve pure major thirds (5:4 ratio).
     *          Produces excellent triadic consonance in commonly used keys (up to 2-3 sharps/flats) but
     *          increasingly dissonant intervals ("wolf tones") in remote keys. Widely used from 1500-1700
     *          for keyboard instruments, fretted strings, and fixed-pitch ensembles. Essential for authentic
     *          performance practice of Baroque repertoire.
     */
    MEANTONE_TEMPERAMENT,

    /**
     * @brief Well temperament (Werckmeister, Kirnberger, Vallotti variants).
     * @details Irregular temperament system with variable key colors—different keys possess distinct
     *          harmonic characters due to asymmetric interval distribution. Allows modulation to all
     *          24 major/minor keys while preserving tonal affect differences. Widely used in Baroque
     *          and early Classical periods (1680-1810). Bach's "Well-Tempered Clavier" exploits these
     *          tonal color variations. Implementation typically follows Werckmeister III tuning.
     */
    WELL_TEMPERAMENT
};

/**
 * @brief Returns the currently active tuning system for pitch-frequency calculations.
 * @return Current TuningSystem enumeration value.
 * @details The tuning system is a global configuration parameter that persists throughout
 *          program execution unless explicitly changed. Default value is EQUAL_TEMPERAMENT.
 */
TuningSystem getTuningSystem();

/**
 * @brief Sets the global tuning system for all subsequent pitch-to-frequency conversions.
 * @param tuningSystem Desired TuningSystem enumeration value to activate.
 * @details This function modifies global state affecting all frequency-dependent operations,
 *          including:
 *          - Note::getFrequency() and spectral analysis methods
 *          - Harmonic spectrum generation (Note::getHarmonicSpectrum())
 *          - Psychoacoustic dissonance calculations (Sethares model)
 *          - Frequency-domain analysis in chord and interval classes
 *
 * @warning Changes affect the entire library globally. Call this function before performing
 *          analysis if non-equal-temperament tuning is required. For comparative analysis
 *          across multiple tuning systems, reset to the desired system before each calculation.
 *
 * @note Tuning system selection does not affect MIDI pitch numbers, pitch class representations,
 *       or symbolic music operations—only continuous frequency calculations.
 *
 * @example
 * @code
 * // Configure for Baroque analysis with meantone temperament
 * setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
 * Note note("A4");
 * float freq = note.getFrequency(); // Returns A4 frequency in meantone tuning
 *
 * // Compare dissonance across tuning systems
 * setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
 * float dissonance_12tet = chord.getSetharesDyadsDissonanceValue();
 * setTuningSystem(TuningSystem::JUST_INTONATION);
 * float dissonance_just = chord.getSetharesDyadsDissonanceValue();
 * @endcode
 */
void setTuningSystem(TuningSystem tuningSystem);
