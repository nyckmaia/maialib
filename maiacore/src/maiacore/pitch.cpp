#include "maiacore/pitch.h"

#include <algorithm>
#include <cmath>
#include <limits>
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

int Pitch::clampToRepresentableMidi(int midi) {
    // The highest quarter-tone step position this class can ever hold: B, double-sharp, at the
    // top octave.
    const int maxMidi = static_cast<int>(computeQuarterToneSteps("B", 2.0f, c_maxPitchOctave));
    if (midi < 0) {
        return 0;
    }
    if (midi > maxMidi) {
        return maxMidi;
    }
    return midi;
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
    // with setFrequency()'s inverse, and removes any cross-platform libm imprecision this
    // specific computation could otherwise introduce.
    //
    // REVIEW ROUND 2 (task-3-review.md N5) -- this comment previously claimed double precision
    // makes setFrequency()'s rounding "reliably tie-exact across platforms". That was wrong, and
    // setFrequency()'s own comment (on the granularity/roundedSteps computation) now says why in
    // detail: no frequency-based computation can be tie-exact regardless of precision, because a
    // tie requires an irrational ratio no finite float representation can hit exactly. This
    // method's double-precision computation is worth keeping on its own merits -- see above --
    // but it does not, and cannot, deliver that stronger guarantee.
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

    int baseMidi = 0;
    float residual = 0.0f;
    bool clamped = false;

    // REVIEW ROUND 2 (task-3-review.md N2) — a non-finite frequency (+infinity or NaN; -infinity
    // is already caught by the `frequency <= 0.0f` rest check above) must never reach
    // static_cast<int>(std::floor(...)) below: that cast is undefined behaviour for non-finite
    // input. Measured (not assumed) against the real binary: on x86-64/MSVC it silently returns
    // INT_MIN, which the floor-clamp then quietly turns into "C-1" -- the wrong end of the range,
    // masked rather than crashing. On AArch64 (Apple Silicon; this library targets macOS --
    // CLAUDE.md) the same cast saturates to INT_MAX instead, and the ceiling clamp below would
    // need on the order of 2^31 iterations to walk down from it: a hang, not merely a wrong
    // answer.
    //
    // REVIEW ROUND 3 (task-3-review.md ruling 1) — +infinity and NaN are NOT the same case, and
    // round 2 conflated them. +infinity genuinely lies above the representable range, so it
    // clamps to the ceiling and warns, same as any other out-of-range positive frequency. NaN
    // satisfies neither half of spec section 4.3's dichotomy ("<= 0" or "positive") -- it is
    // unordered under IEEE 754, so every comparison against it, including the frequency <= 0.0f
    // check above, is false -- and fabricating a pitch from it would hand a caller a valid "B11"
    // and a warning buried in the log for what is actually a caller error (e.g. an FFT result
    // divided by zero). NaN is therefore on the same side of the line as a malformed accType or
    // an unimplemented tuning system: it throws via LOG_ERROR.
    //
    // @warning Both branches below depend on -ffast-math (or an equivalent fast-math build flag)
    // never being enabled for this translation unit: fast-math permits the compiler to assume no
    // NaN or infinity value ever occurs and to remove std::isnan()/std::isfinite() checks
    // outright. Absent from CMakeLists.txt and setup.py as of this writing; must stay absent.
    if (std::isnan(frequency)) {
        LOG_ERROR("Frequency must not be NaN");
    }
    if (!std::isfinite(frequency)) {
        // Only +infinity reaches here: NaN threw above, -infinity is already a rest, and every
        // other non-finite IEEE 754 value is one of those two.
        baseMidi = clampToRepresentableMidi(std::numeric_limits<int>::max());
        clamped = true;
    } else {
        // Exact inverse of getFrequency()'s formula, in quarter-tone-step space. Computed in
        // double (review round 1, I3) to remove the cross-platform libm risk a float32
        // computation would carry here.
        const double steps =
            12.0 * std::log2(static_cast<double>(frequency) / static_cast<double>(freqA4)) +
            69.0;

        // Ties round upward (spec section 4.5): std::floor(x + 0.5), scaled to the rounding
        // granularity, never std::round()/std::lround(). Those round half away from zero and
        // would disagree with this rule for a negative step position (e.g. a frequency below
        // C-1).
        //
        // REVIEW ROUND 1 (task-3-review.md I3) -- this specific line cannot be exercised by a
        // frequency-based unit test at a genuine mathematical tie, and that is not a test-writing
        // gap: `frequency` is float32 by this method's public signature, and no non-integer power
        // of two (the ratio a tie in `steps` requires, since a tie needs frequency / freqA4 ==
        // 2^(k/12) for k ending in .5 or .25/.75) is exactly representable in finite binary
        // floating point -- it always involves an irrational factor (e.g. sqrt(2) for a
        // semitone-granularity tie). Consequently the *recovered* `steps` for any frequency a
        // test can pass in lands fractionally off an intended tie, in an unpredictable direction,
        // which floor(x + 0.5) and std::round(x) agree on identically (they only diverge exactly
        // AT a tie). This was verified empirically during round 1's fix, not assumed: a frequency
        // engineered for the quarter-tone tie at steps == -0.25 passed under both this line and a
        // std::round() revert. The rounding rule itself remains covered where it CAN be tested
        // exactly -- directly against alter/step values with no transcendental round-trip -- by
        // Pitch::roundToSemitoneTiesUpFlatSide and Pitch::midiNumberRoundsHalfUpOnFlatSide
        // (pitch-test.cpp), which this line's formula matches exactly in shape. What
        // frequency-based tests in this file DO verify at this boundary: that it resolves to a
        // real pitch, never a silent rest, at and around the C-1 floor (see
        // fromFrequencyAtFloorBoundaryIsNotARest, fromFrequencyClampsBelowFloorInsteadOfSilentRest
        // and fromFrequencyRecoversExactQuarterToneAtFloor in pitch-test.cpp).
        const double granularity = enableQuarterToneRound ? 0.5 : 1.0;
        const double roundedSteps = granularity * std::floor(steps / granularity + 0.5);

        // Split into an integer MIDI number plus a residual of 0 or 0.5. Safe to cast now: the
        // non-finite case above never reaches here, and roundedSteps is otherwise always finite.
        baseMidi = static_cast<int>(std::floor(roundedSteps));
        residual = static_cast<float>(roundedSteps - baseMidi);

        // REVIEW ROUND 1 (task-3-review.md C1) — a negative baseMidi is checked directly, rather
        // than relying on Helper::midiNote2pitch() to throw for it: it does not. It returns the
        // *string* "rest" for a negative MIDI number, which previously flowed straight through
        // splitPitch()'s substring rest-detection to a silent rest for a valid positive
        // frequency, indistinguishable from the one rest case spec section 4.3 actually sanctions
        // (freq <= 0).
        //
        // When the residual is +0.5, the exact same pitch position also has a valid spelling one
        // semitone up with a flat-side residual instead (e.g. roundedSteps == -0.5 is
        // unrepresentable as baseMidi -1 with a +0.5 residual, but is exactly "C1b-1" as
        // baseMidi 0 with a -0.5 residual -- and Pitch("C1b-1") is constructible:
        // Helper::spelling2midiNote() rounds its -0.5 exact position up to MIDI 0). That is not a
        // clamp -- it is the correct, lossless representation of an exact position -- so it is
        // tried before any clamping.
        if (baseMidi < 0 && residual == 0.5f && baseMidi + 1 >= 0) {
            baseMidi += 1;
            residual -= 1.0f;
        }

        // Genuinely outside the representable range at either end. The controller's ruling: a
        // positive frequency never produces a rest and never throws for being out of range --
        // clamp to the nearest representable pitch (C-1 below, or the highest MIDI number this
        // class can spell above) and warn instead.
        //
        // REVIEW ROUND 3 (task-3-review.md item 2) — clampToRepresentableMidi() is extracted out
        // as a pure function (review round 2, N3's fix was this same logic inlined here) so both
        // ends of the clamp -- including the ceiling jump that keeps the walk-down loop below at
        // O(1) rather than O(baseMidi), the fix N3 required -- are testable directly and
        // deterministically, without going through this method's frequency-to-steps pipeline or
        // timing anything.
        const int clampedMidi = clampToRepresentableMidi(baseMidi);
        if (clampedMidi != baseMidi) {
            baseMidi = clampedMidi;
            residual = 0.0f;
            clamped = true;
        }
    }

    // Spell baseMidi via the existing funnel, walking downward at most a few semitones if
    // neither the requested accType nor the default spelling fits (reachable only right at the
    // top of the octave range, where an octave transition pushes a specific accType's spelling
    // past 11 -- e.g. baseMidi 156/157 need "#"/"x" specifically, and overflow to octave 12 with
    // the default spelling). accType is a preference, not a demand: this never throws for it not
    // applying, and (thanks to the clampToRepresentableMidi() clamp above) never needs more than
    // a handful of steps to find a spelling that fits. Guaranteed to terminate: the default
    // spelling is valid for every MIDI number at or below the top of octave 11.
    // REVIEW ROUND 3 (task-3-review.md O5) — the "try preferredAccType, then fall back to the
    // default spelling" pair used to be written out twice (once before the loop, once inside
    // it); factored into trySpellPreferredThenDefault() so there is exactly one copy.
    auto trySpellPreferredThenDefault = [&](const std::string& preferredAccType) -> std::string {
        std::string pitch = trySpell(baseMidi, preferredAccType);
        if (pitch.empty()) {
            pitch = trySpell(baseMidi, {});
        }
        return pitch;
    };
    auto spellWithClamp = [&](const std::string& preferredAccType) -> std::string {
        std::string pitch = trySpellPreferredThenDefault(preferredAccType);
        while (pitch.empty() && baseMidi > 0) {
            --baseMidi;
            residual = 0.0f;
            clamped = true;
            pitch = trySpellPreferredThenDefault(preferredAccType);
        }
        return pitch;
    };

    std::string basePitch = spellWithClamp(accType);

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
    //
    // REVIEW ROUND 2 (task-3-review.md N1) — this fallback used to call
    // Helper::midiNote2pitch(baseMidi, {}) directly instead of through spellWithClamp(). At
    // baseMidi values only reachable with a specific non-default accType (e.g. 157, "Bx11" --
    // this exact reproducer), the default spelling ALSO fails, that direct call returned "", and
    // Helper::splitPitch("") took its empty-string rest branch: the C1 defect this method exists
    // to fix, reopened at the other end by this exact fallback. Routing through spellWithClamp()
    // here closes it: if the default spelling of the current baseMidi does not fit either, this
    // now walks the same clamp-and-retry ceiling logic used above rather than ever handing
    // splitPitch() an empty string.
    try {
        alterSymbol = Helper::alterValue2symbol(finalAlter);
    } catch (const std::runtime_error&) {
        basePitch = spellWithClamp({});
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
