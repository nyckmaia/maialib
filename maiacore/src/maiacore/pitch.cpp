#include "maiacore/pitch.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "maiacore/config.h"
#include "maiacore/helper.h"
#include "maiacore/log.h"

Pitch::Pitch(const std::string& pitch) : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setPitch(pitch);
}

Pitch::Pitch(int midiNumber, const std::string& accType)
    : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setPitch(Helper::midiNote2pitch(midiNumber, accType));
}

Pitch::Pitch(float frequency, const std::string& accType, float freqA4,
             bool enableQuarterToneRound)
    : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setFrequency(frequency, accType, freqA4, enableQuarterToneRound);
}

std::string Pitch::getPitch() const {
    if (isRest()) {
        return MUSIC_XML::PITCH::REST;
    }

    return getPitchClass() + std::to_string(_octave.value());
}

std::string Pitch::getPitchClass() const { return _step + getAlterSymbol(); }

std::string Pitch::getPitchStep() const { return _step; }

std::string Pitch::getAlterSymbol() const { return Helper::alterValue2symbol(_alter); }

float Pitch::getAlter() const { return _alter; }

std::optional<int> Pitch::getOctave() const { return _octave; }

int Pitch::getMidiNumber() const {
    if (isRest()) return MUSIC_XML::MIDI::NUMBER::MIDI_REST;
    return Helper::spelling2midiNote(_step, _alter, _octave.value());
}

float Pitch::computeQuarterToneSteps(const std::string& step, float alter, int octave) {
    const auto stepIt = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), step);
    if (stepIt == c_C_diatonicScale.end()) {
        LOG_ERROR("Unknown diatonic pitch step: " + step);
    }
    const auto stepIdx = static_cast<size_t>(std::distance(c_C_diatonicScale.begin(), stepIt));

    // Same base formula as Helper::spelling2midiNote(), without the ties-upward rounding: this
    // is the one place that value is intentionally computed unrounded (see getMidiNumber()).
    return 12.0f * (octave + 1) + c_diatonicStepSemitones[stepIdx] + alter;
}

float Pitch::getQuarterToneSteps() const {
    if (isRest()) {
        return static_cast<float>(MUSIC_XML::MIDI::NUMBER::MIDI_REST);
    }

    return computeQuarterToneSteps(_step, _alter, _octave.value());
}

bool Pitch::isRest() const { return _step == MUSIC_XML::PITCH::REST; }

float Pitch::getFrequency(float freqA4) const {
    if (isRest()) {
        return 0.0f;
    }

    if (getTuningSystem() != TuningSystem::EQUAL_TEMPERAMENT) {
        LOG_ERROR("Tuning system not implemented in SP2; only EQUAL_TEMPERAMENT is available");
    }

    // Uses the exact, unrounded quarter-tone step position (not getMidiNumber()) so a
    // quarter-tone alter is never rounded away here -- see THE TRAP note in task-3-brief.md
    // about Helper::freq2midiNote(), which is deliberately not used for this reason. Computed in
    // double (review round 1, I3) and narrowed to float only in the return: keeps this symmetric
    // with setFrequency()'s inverse, which needs the extra precision for its rounding to be
    // reliably tie-exact across platforms (see setFrequency()'s comment).
    const double frequency = static_cast<double>(freqA4) *
                              std::pow(2.0, (static_cast<double>(getQuarterToneSteps()) - 69.0) /
                                                12.0);
    return static_cast<float>(frequency);
}

void Pitch::setStep(const std::string& step) {
    const bool isValidStep =
        std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), step) !=
        c_C_diatonicScale.end();
    if (!isValidStep) {
        LOG_ERROR("Unknown diatonic pitch step: " + step);
    }

    if (isRest()) {
        // Permissive on a rest: resurrects this Pitch into a note, defaulting the octave to 4.
        // The rest invariant guarantees _alter == 0.0f already, so this can never breach the
        // MIDI >= 0 floor (the lowest reachable value here is octave 4, step C, alter 0).
        _step = step;
        _octave = 4;
        return;
    }

    if (Helper::spelling2midiNote(step, _alter, _octave.value()) < 0) {
        LOG_WARN("Pitch::setStep: step '" + step +
                 "' would move this pitch below MIDI note 0; ignoring");
        return;
    }

    _step = step;
}

void Pitch::setAlter(float alter) {
    const float doubled = alter * 2.0f;
    const float roundedDoubled = std::round(doubled);
    if (std::fabs(doubled - roundedDoubled) > 1e-4f) {
        LOG_ERROR(
            "Alter value must be a multiple of 0.5 (a semitone or quarter-tone step): " +
            std::to_string(alter));
    }
    if (alter < -2.0f || alter > 2.0f) {
        LOG_ERROR("Alter value out of range [-2, 2]: " + std::to_string(alter));
    }

    if (isRest()) {
        LOG_WARN("Pitch::setAlter: cannot set the alter of a rest; ignoring");
        return;
    }

    if (Helper::spelling2midiNote(_step, alter, _octave.value()) < 0) {
        LOG_WARN("Pitch::setAlter: alter " + std::to_string(alter) +
                 " would move this pitch below MIDI note 0; ignoring");
        return;
    }

    _alter = alter;
}

void Pitch::setOctave(int octave) {
    if (octave < c_minPitchOctave || octave > c_maxPitchOctave) {
        LOG_ERROR("Invalid octave value: " + std::to_string(octave));
    }

    if (isRest()) {
        LOG_WARN("Pitch::setOctave: cannot set the octave of a rest; ignoring");
        return;
    }

    if (Helper::spelling2midiNote(_step, _alter, octave) < 0) {
        LOG_WARN("Pitch::setOctave: octave " + std::to_string(octave) +
                 " would move this pitch below MIDI note 0; ignoring");
        return;
    }

    _octave = octave;
}

void Pitch::setPitch(const std::string& pitch) {
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

    _step = pitchStep;
    _alter = alterValue;
    _octave = (pitchStep == MUSIC_XML::PITCH::REST) ? std::nullopt : std::optional<int>(octave);
}

void Pitch::setPitchClass(const std::string& pitchClass) {
    const std::string octaveSuffix = _octave.has_value() ? std::to_string(_octave.value()) : "";
    setPitch(pitchClass + octaveSuffix);
}

void Pitch::setMidiNumber(int midiNumber) { setPitch(Helper::midiNote2pitch(midiNumber)); }

void Pitch::setFrequency(float frequency, const std::string& accType, float freqA4,
                          bool enableQuarterToneRound) {
    // A frequency <= 0 is a whole-state replacement into a rest (spec section 4.4.1), not a
    // caller error: this never throws for that reason alone.
    if (frequency <= 0.0f) {
        setPitch(MUSIC_XML::PITCH::REST);
        return;
    }

    // REVIEW ROUND 1 (task-3-review.md I1) — mirrors getFrequency()'s guard. A tuning system
    // other than EQUAL_TEMPERAMENT is a caller error (throws), not a boundary condition: the
    // setter must not silently apply the 12-TET inverse while the getter refuses to use it.
    if (getTuningSystem() != TuningSystem::EQUAL_TEMPERAMENT) {
        LOG_ERROR("Tuning system not implemented in SP2; only EQUAL_TEMPERAMENT is available");
    }

    // REVIEW ROUND 1 (task-3-review.md I2) — validate accType up front, against the same five
    // values Helper::midiNote2pitch() itself accepts. A malformed accType is a caller error (this
    // now matches Pitch(int, accType), which already throws for it via midiNote2pitch()'s own
    // check) rather than something the accType-applicability fallback below should absorb along
    // with unrelated failures.
    if (!accType.empty() && accType != MUSIC_XML::ACCIDENT::SHARP &&
        accType != MUSIC_XML::ACCIDENT::FLAT && accType != MUSIC_XML::ACCIDENT::DOUBLE_SHARP &&
        accType != MUSIC_XML::ACCIDENT::DOUBLE_FLAT) {
        LOG_ERROR("Unknown accident type: " + accType);
    }

    // Exact inverse of getFrequency()'s formula, in quarter-tone-step space. Computed in double
    // (review round 1, I3): float32 leaves an exact tie in `steps` with zero margin against a
    // single ULP of cross-platform libm disagreement in log2f/powf (this library targets MSVC,
    // GCC and Apple's libm -- CLAUDE.md), which is enough to flip which side of the tie the
    // ties-upward rule resolves to. Double buys roughly nine more decimal digits of headroom,
    // and the result is narrowed to float only where the rest of this class already lives
    // (`residual`, below).
    const double steps =
        12.0 * std::log2(static_cast<double>(frequency) / static_cast<double>(freqA4)) + 69.0;

    // Ties round upward (spec section 4.5): std::floor(x + 0.5), scaled to the rounding
    // granularity, never std::round()/std::lround(). Those round half away from zero and would
    // disagree with this rule for a negative step position (e.g. a frequency below C-1).
    //
    // REVIEW ROUND 1 (task-3-review.md I3) -- this specific line cannot be exercised by a
    // frequency-based unit test at a genuine mathematical tie, and that is not a test-writing
    // gap: `frequency` is float32 by this method's public signature, and no non-integer power of
    // two (the ratio a tie in `steps` requires, since a tie needs frequency / freqA4 == 2^(k/12)
    // for k ending in .5 or .25/.75) is exactly representable in finite binary floating point --
    // it always involves an irrational factor (e.g. sqrt(2) for a semitone-granularity tie).
    // Consequently the *recovered* `steps` for any frequency a test can pass in lands fractionally
    // off an intended tie, in an unpredictable direction, which floor(x + 0.5) and std::round(x)
    // agree on identically (they only diverge exactly AT a tie). This was verified empirically
    // during round 1's fix, not assumed: a frequency engineered for the quarter-tone tie at
    // steps == -0.25 passed under both this line and a std::round() revert. The rounding rule
    // itself remains covered where it CAN be tested exactly -- directly against alter/step values
    // with no transcendental round-trip -- by Pitch::roundToSemitoneTiesUpFlatSide and
    // Pitch::midiNumberRoundsHalfUpOnFlatSide (pitch-test.cpp), which this line's formula matches
    // exactly in shape. What frequency-based tests in this file DO verify at this boundary: that
    // it resolves to a real pitch, never a silent rest, at and around the C-1 floor (see
    // fromFrequencyRoundsHalfUpOnFlatSide, fromFrequencyClampsBelowFloorInsteadOfSilentRest and
    // fromFrequencyRecoversExactQuarterToneAtFloor in pitch-test.cpp).
    const double granularity = enableQuarterToneRound ? 0.5 : 1.0;
    const double roundedSteps = granularity * std::floor(steps / granularity + 0.5);

    // Split into an integer MIDI number plus a residual of 0 or 0.5.
    int baseMidi = static_cast<int>(std::floor(roundedSteps));
    float residual = static_cast<float>(roundedSteps - baseMidi);

    // A helper that spells a MIDI number without throwing, so the fallbacks below can probe
    // spellability instead of relying on an exception that -- per Helper::midiNote2pitch()'s own
    // negative-MIDI special case -- does not always come (see the C1 comment just below).
    auto trySpell = [](int midi, const std::string& type) -> std::string {
        try {
            return Helper::midiNote2pitch(midi, type);
        } catch (const std::runtime_error&) {
            return {};
        }
    };

    bool clamped = false;

    // REVIEW ROUND 1 (task-3-review.md C1) — a negative baseMidi is checked directly, rather
    // than relying on Helper::midiNote2pitch() to throw for it: it does not. It returns the
    // *string* "rest" for a negative MIDI number, which previously flowed straight through
    // splitPitch()'s substring rest-detection to a silent rest for a valid positive frequency,
    // indistinguishable from the one rest case spec section 4.3 actually sanctions (freq <= 0).
    //
    // When the residual is +0.5, the exact same pitch position also has a valid spelling one
    // semitone up with a flat-side residual instead (e.g. roundedSteps == -0.5 is unrepresentable
    // as baseMidi -1 with a +0.5 residual, but is exactly "C1b-1" as baseMidi 0 with a -0.5
    // residual -- and Pitch("C1b-1") is constructible: Helper::spelling2midiNote() rounds its
    // -0.5 exact position up to MIDI 0). That is not a clamp -- it is the correct, lossless
    // representation of an exact position -- so it is tried before any clamping.
    if (baseMidi < 0 && residual == 0.5f && baseMidi + 1 >= 0) {
        baseMidi += 1;
        residual -= 1.0f;
    }
    if (baseMidi < 0) {
        // Genuinely below the representable floor. The controller's ruling: a positive
        // frequency never produces a rest and never throws for being out of range -- clamp to
        // the lowest representable pitch (C-1) and warn instead.
        baseMidi = 0;
        residual = 0.0f;
        clamped = true;
    }

    // Spell the base MIDI number via the existing funnel. accType is only a preference for the
    // base semitone: when it does not apply to this specific chromatic degree (e.g. "#" was
    // requested but the rounded base MIDI is a natural, white-key note), fall back to the
    // default spelling rather than treating that as a caller error.
    std::string basePitch = trySpell(baseMidi, accType);
    if (basePitch.empty()) {
        basePitch = trySpell(baseMidi, {});
    }

    // REVIEW ROUND 1 (task-3-review.md M6, the ruling's ceiling mirror of C1) — reachable only
    // for a handful of MIDI numbers right at the top of the octave range, where the octave
    // transition a specific accType applies pushes the spelling past 11 for every accType tried
    // above (e.g. baseMidi 156/157 need "#"/"x" specifically, and overflow to octave 12 with the
    // default spelling). Clamp downward one semitone at a time, retrying the same accType-then-
    // default spelling at each step, until one succeeds, and warn. Guaranteed to terminate: the
    // default spelling is valid for every MIDI number at or below the top of octave 11.
    while (basePitch.empty() && baseMidi > 0) {
        --baseMidi;
        residual = 0.0f;
        clamped = true;
        basePitch = trySpell(baseMidi, accType);
        if (basePitch.empty()) {
            basePitch = trySpell(baseMidi, {});
        }
    }

    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    int octave = 0;
    float baseAlter = 0.0f;
    Helper::splitPitch(basePitch, pitchClass, pitchStep, octave, baseAlter, alterSymbol);

    float finalAlter = baseAlter + residual;

    // REVIEW ROUND 1 (task-3-review.md C2) — accType is a preference, not a demand. A base
    // spelling with its own accidental (e.g. accType "x" => alter +2.0) combined with a +0.5
    // quarter-tone residual can leave a value Helper::alterValue2symbol() cannot express (+-2.5,
    // outside this class's own [-2, 2] invariant), which previously escaped as an uncaught throw.
    // When that happens, fall back to the default spelling for the base semitone -- whose alter
    // is always 0 or +1 (Helper::midiNote2pitch()'s default branch never produces a double
    // accidental), so combined with a 0 or 0.5 residual it always lands in range -- and warn.
    try {
        alterSymbol = Helper::alterValue2symbol(finalAlter);
    } catch (const std::runtime_error&) {
        basePitch = trySpell(baseMidi, {});
        Helper::splitPitch(basePitch, pitchClass, pitchStep, octave, baseAlter, alterSymbol);
        finalAlter = baseAlter + residual;
        alterSymbol = Helper::alterValue2symbol(finalAlter);
        clamped = true;
    }

    if (clamped) {
        LOG_WARN("Pitch::setFrequency: " + std::to_string(frequency) +
                 " Hz could not be represented exactly as requested; using " + pitchStep +
                 alterSymbol + std::to_string(octave) + " instead");
    }

    setPitch(pitchStep + alterSymbol + std::to_string(octave));
}

void Pitch::roundToSemitone() { _alter = std::floor(_alter + 0.5f); }
