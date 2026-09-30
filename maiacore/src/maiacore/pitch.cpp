#include "maiacore/pitch.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "maiacore/config.h"
#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

// The project's CI runs no C++ or Python tests (wheels.yml only builds the wheels), so this
// compile-time guard is the only automated check for this hazard: Pitch::setFrequency() below
// depends on std::isnan() and std::isfinite() to keep a non-finite frequency from reaching a
// static_cast<int>(std::floor(...)) that is undefined behaviour for it. A fast-math build flag
// (clang/GCC -ffast-math, or MSVC /fp:fast) permits the compiler to assume no NaN or infinity
// value ever occurs, and is free to delete those checks outright -- silently, with no warning,
// because from the compiler's declared-legal standpoint under that flag they are dead code.
// __FAST_MATH__ and __FINITE_MATH_ONLY__ cover clang and GCC (the latter is always defined by
// those compilers, 0 or 1, hence the value check rather than a bare defined()); _M_FP_FAST covers
// MSVC, relevant for the wheels this project ships. If this fires, remove the fast-math flag for
// this translation unit -- do not silence this #error instead.
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__) || \
    defined(_M_FP_FAST)
#error \
    "pitch.cpp was compiled with a fast-math flag (-ffast-math, -ffinite-math-only, or \
/fp:fast). Pitch::setFrequency() relies on std::isnan()/std::isfinite() to keep a non-finite \
frequency out of undefined behaviour; fast-math lets the compiler assume NaN and infinity \
never occur and may delete those checks. Remove the fast-math flag for this file."
#endif

namespace {
// The reference frequency of A4 must be a positive, finite number of Hz. Zero or a negative value
// has no equal-tempered scale to measure against, and a non-finite one makes every pitch position
// non-finite, which the conversion to a MIDI number cannot survive. Checked first, before any
// arithmetic, whatever the frequency.
void validateFreqA4(const float freqA4) {
    if (!std::isfinite(freqA4) || !(freqA4 > 0.0f)) {
        LOG_ERROR(
            "The reference frequency freqA4 must be a finite number of Hz greater than 0, "
            "but '" +
            std::to_string(freqA4) + "' is not");
    }
}

// An alter is malformed unless it is finite, exactly on the quarter-tone grid and within the
// [-2, 2] a double accidental reaches.
void validateAlter(const float alter) {
    if (!std::isfinite(alter)) {
        LOG_ERROR("Alter value must be a finite number of semitones, but '" +
                  std::to_string(alter) + "' is not");
    }
    if (!isOnQuarterToneGrid(alter)) {
        LOG_ERROR("Alter value must be a multiple of 0.5 (a semitone or quarter-tone step): " +
                  std::to_string(alter));
    }
    if (alter < -2.0f || alter > 2.0f) {
        LOG_ERROR("Alter value out of range [-2, 2]: " + std::to_string(alter));
    }
}
}  // namespace

Pitch::Pitch(const std::string& pitch) : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setPitch(pitch);
}

Pitch::Pitch(int midiNumber, const std::string& accType)
    : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setPitch(Helper::midiNote2pitch(midiNumber, accType));
}

Pitch::Pitch(float frequency, const std::string& accType, float freqA4, bool enableQuarterToneRound)
    : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setFrequency(frequency, accType, freqA4, enableQuarterToneRound);
}

Pitch::Pitch(const std::string& step, float alter, int octave)
    : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    if (std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), step) ==
        c_C_diatonicScale.end()) {
        LOG_ERROR("Unknown diatonic pitch step: " + step);
    }
    validateAlter(alter);
    if (octave < c_minPitchOctave || octave > c_maxPitchOctave) {
        LOG_ERROR("Invalid octave value: " + std::to_string(octave));
    }

    // Stored canonically, as setAlter() stores it: -0.0 becomes +0.0.
    const float canonicalAlter = alter + 0.0f;
    if (Helper::spelling2midiNote(step, canonicalAlter, octave) < 0) {
        LOG_ERROR("The pitch '" + step + Helper::alterValue2symbol(canonicalAlter) +
                  std::to_string(octave) +
                  "' is below MIDI note 0; the lowest representable pitch is C1b-1");
    }

    _step = step;
    _alter = canonicalAlter;
    _octave = octave;
}

bool Pitch::operator==(const Pitch& other) const {
    // The canonical triple: a rest is "rest", 0 and no octave, so two rests are equal. Every
    // stored alter is exactly on the grid, so == compares it exactly.
    return _step == other._step && _alter == other._alter && _octave == other._octave;
}

bool Pitch::operator!=(const Pitch& other) const { return !(*this == other); }

int Pitch::maxRepresentableMidi() {
    // The highest quarter-tone step position this class can ever hold: B, double-sharp, at the
    // top octave.
    return static_cast<int>(computeQuarterToneSteps("B", 2.0f, c_maxPitchOctave));
}

int Pitch::clampToRepresentableMidi(int midi) {
    const int maxMidi = maxRepresentableMidi();
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
    validateFreqA4(freqA4);

    if (isRest()) {
        return 0.0f;
    }

    if (getTuningSystem() != TuningSystem::EQUAL_TEMPERAMENT) {
        LOG_ERROR("Tuning system is not implemented; only EQUAL_TEMPERAMENT is available");
    }

    // Uses the exact, unrounded quarter-tone step position (not getMidiNumber()) so a
    // quarter-tone alter is never rounded away here. Helper::freq2midiNote() is not used for the
    // same reason: it rounds to an integer MIDI number first. Computed in double and narrowed to
    // float only in the return: this keeps it symmetric with setFrequency()'s inverse, and removes
    // any cross-platform libm imprecision a float32 computation could introduce.
    //
    // Double precision does not make setFrequency()'s rounding tie-exact, though: no frequency
    // lands exactly on a tie, whatever the precision, because a tie needs an irrational ratio no
    // finite float can hold (see setFrequency()).
    const double frequency =
        static_cast<double>(freqA4) *
        std::pow(2.0, (static_cast<double>(getQuarterToneSteps()) - 69.0) / 12.0);
    return static_cast<float>(frequency);
}

void Pitch::setStep(const std::string& step) {
    const bool isValidStep = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), step) !=
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
    validateAlter(alter);

    // Stored canonically: adding +0.0f turns -0.0 into +0.0 and leaves every other value on the
    // grid unchanged, so a natural always reads back as the one value +0.0.
    const float canonicalAlter = alter + 0.0f;

    if (isRest()) {
        LOG_WARN("Pitch::setAlter: cannot set the alter of a rest; ignoring");
        return;
    }

    if (Helper::spelling2midiNote(_step, canonicalAlter, _octave.value()) < 0) {
        LOG_WARN("Pitch::setAlter: alter " + std::to_string(canonicalAlter) +
                 " would move this pitch below MIDI note 0; ignoring");
        return;
    }

    _alter = canonicalAlter;
}

void Pitch::setOctave(int octave) {
    // The rest check runs FIRST, ahead of the range check below: an operation that is
    // inapplicable to a rest is inapplicable whatever the argument, so a rest handed an
    // out-of-range octave warns rather than throws (pinned by pitch-test.cpp's
    // setOctaveOutOfRangeOnRestIsRefusedAndWarnsNotThrows and note-test.cpp's
    // SetOctaveOutOfRangeOnRestWarnsAndDoesNotThrow).
    if (isRest()) {
        LOG_WARN("Pitch::setOctave: cannot set the octave of a rest; ignoring");
        return;
    }

    if (octave < c_minPitchOctave || octave > c_maxPitchOctave) {
        LOG_ERROR("Invalid octave value: " + std::to_string(octave));
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
    std::optional<int> octave;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

    _step = pitchStep;
    _alter = alterValue;
    // splitPitch() leaves 'octave' empty for a rest and populated otherwise, so it maps directly
    // onto Pitch's own invariant; no separate rest check is needed here.
    _octave = octave;
}

void Pitch::setPitchClass(const std::string& pitchClass) {
    const std::string octaveSuffix = _octave.has_value() ? std::to_string(_octave.value()) : "";
    setPitch(pitchClass + octaveSuffix);
}

void Pitch::setMidiNumber(int midiNumber, const std::string& accType) {
    setPitch(Helper::midiNote2pitch(midiNumber, accType));
}

void Pitch::setFrequency(float frequency, const std::string& accType, float freqA4,
                         bool enableQuarterToneRound) {
    validateFreqA4(freqA4);

    // A frequency <= 0 is a whole-state replacement into a rest (spec section 4.4.1), not a
    // caller error: this never throws for that reason alone.
    //
    // This must stay spelled as `<= 0.0f`, not rewritten to the logically-tempting
    // `!(frequency > 0.0f)`. They are NOT equivalent for NaN: every comparison against NaN is
    // false under IEEE 754, so `frequency <= 0.0f` is false for NaN (falls through, as intended,
    // to the NaN throw below) while `!(frequency > 0.0f)` would be true for NaN
    // (`frequency > 0.0f` is false, negated to true) and route NaN into this rest branch instead:
    // a silent rest for what is a caller error. The NaN throw below is only reachable because this
    // comparison is spelled this way.
    if (frequency <= 0.0f) {
        setPitch(MUSIC_XML::PITCH::REST);
        return;
    }

    // Mirrors getFrequency()'s guard. A tuning system other than EQUAL_TEMPERAMENT is a caller
    // error (throws), not a boundary condition: the setter must not silently apply the 12-TET
    // inverse while the getter refuses to use it.
    if (getTuningSystem() != TuningSystem::EQUAL_TEMPERAMENT) {
        LOG_ERROR("Tuning system is not implemented; only EQUAL_TEMPERAMENT is available");
    }

    // accType is validated up front, against the same five values Helper::midiNote2pitch()
    // itself accepts. A malformed accType is a caller error, as it is for Pitch(int, accType),
    // which throws for it through midiNote2pitch()'s own check, rather than something the
    // accType-applicability fallback below should absorb along with unrelated failures.
    if (!accType.empty() && accType != MUSIC_XML::ACCIDENT::SHARP &&
        accType != MUSIC_XML::ACCIDENT::FLAT && accType != MUSIC_XML::ACCIDENT::DOUBLE_SHARP &&
        accType != MUSIC_XML::ACCIDENT::DOUBLE_FLAT) {
        LOG_ERROR("Unknown accident type: " + accType);
    }

    // A helper that spells a MIDI number without throwing, so the fallbacks below can probe
    // spellability instead of relying on an exception that -- per Helper::midiNote2pitch()'s own
    // negative-MIDI special case -- does not always come (see the negative-baseMidi comment
    // below).
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

    // A non-finite frequency (+infinity or NaN; -infinity is already caught by the
    // `frequency <= 0.0f` rest check above) must never reach static_cast<int>(std::floor(...))
    // below: that cast is undefined behaviour for non-finite input. In practice x86-64/MSVC
    // returns INT_MIN, which the floor clamp would quietly turn into "C-1" -- the wrong end of
    // the range, masked rather than crashing -- and AArch64 (the macOS wheels) saturates to
    // INT_MAX instead.
    //
    // +infinity and NaN are NOT the same case. +infinity genuinely lies above the representable
    // range, so it clamps to the ceiling and warns, same as any other out-of-range positive
    // frequency. NaN satisfies neither half of spec section 4.3's dichotomy ("<= 0" or
    // "positive") -- it is unordered under IEEE 754, so every comparison against it, including
    // the frequency <= 0.0f check above, is false -- and fabricating a pitch from it would hand a
    // caller a valid "B11" and a warning buried in the log for what is actually a caller error
    // (e.g. an FFT result divided by zero). NaN is therefore on the same side of the line as a
    // malformed accType or an unimplemented tuning system: it throws via LOG_ERROR.
    //
    // @warning Both branches below depend on -ffast-math (or an equivalent fast-math build flag)
    // never being enabled for this translation unit: fast-math permits the compiler to assume no
    // NaN or infinity value ever occurs and to remove std::isnan()/std::isfinite() checks
    // outright. The #error at the top of this file refuses such a build.
    if (std::isnan(frequency)) {
        LOG_ERROR("Frequency must not be NaN");
    }
    if (!std::isfinite(frequency)) {
        // Only +infinity reaches here: NaN threw above, -infinity is already a rest, and every
        // other non-finite IEEE 754 value is one of those two.
        //
        // Reads the ceiling value directly from maxRepresentableMidi() rather than exercising
        // clampToRepresentableMidi()'s clamp branch with a sentinel such as INT_MAX: a single
        // comparison should not simultaneously gate this path, the finite ceiling clamp below,
        // and the walk-down loop's starting point, so an error in the clamp branch cannot also
        // break this path.
        baseMidi = maxRepresentableMidi();
        clamped = true;
    } else {
        // Exact inverse of getFrequency()'s formula, in quarter-tone-step space. Computed in
        // double to remove the cross-platform libm risk a float32 computation would carry here.
        const double steps =
            12.0 * std::log2(static_cast<double>(frequency) / static_cast<double>(freqA4)) + 69.0;

        // Ties round upward (spec section 4.5), at the rounding granularity, through
        // roundTiesUpward() -- never std::round()/std::lround(), which round half away from zero
        // and would disagree with this rule for a negative step position (a frequency below C-1).
        //
        // No frequency lands exactly on a tie: a tie needs frequency / freqA4 to equal 2^(k/12)
        // for a k ending in .5 (or .25/.75), an irrational ratio that no binary float holds, so
        // the recovered position always sits slightly to one side of it, where roundTiesUpward()
        // and std::round() agree. The rule is therefore pinned where it can be hit exactly, on
        // alters and step positions (pitch-test.cpp's roundToSemitoneTiesUpFlatSide and
        // midiNumberRoundsHalfUpOnFlatSide); the frequency tests pin that a frequency at or near
        // the C-1 floor resolves to a real pitch, never a silent rest.
        const double granularity = enableQuarterToneRound ? 0.5 : 1.0;
        const double roundedSteps = granularity * roundTiesUpward(steps / granularity);

        // Split into an integer MIDI number plus a residual of 0 or 0.5. Safe to cast now: the
        // non-finite case above never reaches here, and roundedSteps is otherwise always finite.
        baseMidi = static_cast<int>(std::floor(roundedSteps));
        residual = static_cast<float>(roundedSteps - baseMidi);

        // A negative baseMidi is checked directly, rather than relying on
        // Helper::midiNote2pitch() to throw for it: it does not. It returns the *string* "rest"
        // for a negative MIDI number, which would flow straight through splitPitch()'s substring
        // rest-detection to a silent rest for a valid positive frequency, indistinguishable from
        // the one rest case spec section 4.3 actually sanctions (freq <= 0).
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

        // Genuinely outside the representable range at either end. A positive frequency never
        // produces a rest and never throws for being out of range: it is clamped to MIDI note 0
        // (C-1) below, or to the highest MIDI number this class can spell above, with a warning.
        //
        // clampToRepresentableMidi() is a pure function so that both ends of the clamp --
        // including the ceiling jump that keeps the walk-down loop below at O(1) rather than
        // O(baseMidi) -- are testable directly and deterministically, without going through this
        // method's frequency-to-steps pipeline.
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
    //
    // accTypeIgnored records whether the spelling finally used had to fall back from a requested
    // accType to the default one, which is reported below. Each attempt overwrites it, so it
    // describes the last, used, spelling. When no accType was requested the fallback is skipped:
    // {} and "" spell identically.
    bool accTypeIgnored = false;
    auto trySpellPreferredThenDefault = [&](const std::string& preferredAccType) -> std::string {
        std::string pitch = trySpell(baseMidi, preferredAccType);
        accTypeIgnored = pitch.empty() && !preferredAccType.empty();
        if (accTypeIgnored) {
            pitch = trySpell(baseMidi, {});
        }
        return pitch;
    };

    // kMaxWalkDownSteps bounds the walk-down independently of clampToRepresentableMidi()'s and
    // maxRepresentableMidi()'s own correctness: it is a small constant, not derived from either
    // of them or from anything else this loop already depends on, so an error in the clamp above
    // cannot also make this loop run away. The walk never takes more than two steps -- the
    // default spelling fits again at MIDI 155 (B11), two below the ceiling of 157 -- so this is
    // generous headroom, not a tuned value. If the loop were ever exhausted without a fitting
    // spelling (unreachable, but this must hold even if that changes), it falls back to the
    // always-valid natural spelling of MIDI 0 rather than ever handing splitPitch() an empty
    // string.
    constexpr int kMaxWalkDownSteps = 16;
    auto spellWithClamp = [&](const std::string& preferredAccType) -> std::string {
        std::string pitch = trySpellPreferredThenDefault(preferredAccType);
        int steps = 0;
        while (pitch.empty() && baseMidi > 0 && steps < kMaxWalkDownSteps) {
            --baseMidi;
            ++steps;
            residual = 0.0f;
            clamped = true;
            pitch = trySpellPreferredThenDefault(preferredAccType);
        }
        if (pitch.empty()) {
            baseMidi = 0;
            residual = 0.0f;
            clamped = true;
            pitch = trySpell(0, {});
        }
        return pitch;
    };

    std::string basePitch = spellWithClamp(accType);

    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    std::optional<int> octave;
    float baseAlter = 0.0f;
    Helper::splitPitch(basePitch, pitchClass, pitchStep, octave, baseAlter, alterSymbol);

    float finalAlter = baseAlter + residual;

    // accType is a preference, not a demand. A base spelling with its own double accidental
    // (accType "x" => alter +2.0, "bb" => -2.0) combined with a quarter-tone residual can leave a
    // value Helper::alterValue2symbol() cannot express (+-2.5, outside this class's [-2, 2]
    // invariant). Then the base semitone is spelled the default way instead -- its alter is
    // always 0 or +1, since Helper::midiNote2pitch()'s default branch never produces a double
    // accidental, so with a 0 or 0.5 residual it always lands in range. That is the same pitch,
    // spelled without the requested accType, and reported as such. The default spelling goes
    // through spellWithClamp(), so a baseMidi only a specific accType can spell (157 is only
    // "Bx11") walks down to a spellable pitch instead of handing splitPitch() an empty string.
    try {
        alterSymbol = Helper::alterValue2symbol(finalAlter);
    } catch (const std::runtime_error&) {
        basePitch = spellWithClamp({});
        accTypeIgnored = true;
        Helper::splitPitch(basePitch, pitchClass, pitchStep, octave, baseAlter, alterSymbol);
        // spellWithClamp() never hands splitPitch() an empty string (see the walk-down comments
        // above), so 'octave' is always populated here and below.
        finalAlter = baseAlter + residual;
        alterSymbol = Helper::alterValue2symbol(finalAlter);
    }

    const std::string spelled = pitchStep + alterSymbol + std::to_string(octave.value());
    if (accTypeIgnored) {
        LOG_WARN("Pitch::setFrequency: the accidental type '" + accType + "' cannot spell the " +
                 "pitch " + std::to_string(frequency) + " Hz rounds to; using " + spelled +
                 " instead");
    }

    if (clamped) {
        LOG_WARN("Pitch::setFrequency: " + std::to_string(frequency) +
                 " Hz could not be represented exactly as requested; using " + spelled +
                 " instead");
    }

    setPitch(spelled);
}

void Pitch::roundToSemitone() { _alter = roundTiesUpward(_alter); }
