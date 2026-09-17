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
    // about Helper::freq2midiNote(), which is deliberately not used for this reason.
    return freqA4 * std::pow(2.0f, (getQuarterToneSteps() - 69.0f) / 12.0f);
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

    // Exact inverse of getFrequency()'s formula, in quarter-tone-step space.
    const float steps = 12.0f * std::log2(frequency / freqA4) + 69.0f;

    // Ties round upward (spec section 4.5): std::floor(x + 0.5f), scaled to the rounding
    // granularity, never std::round()/std::lround(). Those round half away from zero and would
    // disagree with this rule for a negative step position (e.g. a frequency below C-1).
    const float granularity = enableQuarterToneRound ? 0.5f : 1.0f;
    const float roundedSteps = granularity * std::floor(steps / granularity + 0.5f);

    // Split into an integer MIDI number plus a residual of 0 or 0.5.
    const int baseMidi = static_cast<int>(std::floor(roundedSteps));
    const float residual = roundedSteps - static_cast<float>(baseMidi);

    // Spell the base MIDI number via the existing funnel. accType is only a preference for the
    // base semitone: when it does not apply to this specific chromatic degree (e.g. "#" was
    // requested but the rounded base MIDI is a natural, white-key note), fall back to the
    // default spelling rather than treating that as a caller error.
    std::string basePitch;
    try {
        basePitch = Helper::midiNote2pitch(baseMidi, accType);
    } catch (const std::runtime_error&) {
        basePitch = Helper::midiNote2pitch(baseMidi, {});
    }

    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    int octave = 0;
    float baseAlter = 0.0f;
    Helper::splitPitch(basePitch, pitchClass, pitchStep, octave, baseAlter, alterSymbol);

    const float finalAlter = baseAlter + residual;
    setPitch(pitchStep + Helper::alterValue2symbol(finalAlter) + std::to_string(octave));
}

void Pitch::roundToSemitone() { _alter = std::floor(_alter + 0.5f); }
