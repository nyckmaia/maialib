#include "maiacore/pitch.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "maiacore/helper.h"
#include "maiacore/log.h"

Pitch::Pitch(const std::string& pitch) : _step("rest"), _alter(0.0f), _octave(std::nullopt) {
    setPitch(pitch);
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

float Pitch::getQuarterToneSteps() const {
    if (isRest()) {
        return static_cast<float>(MUSIC_XML::MIDI::NUMBER::MIDI_REST);
    }

    const auto stepIt = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), _step);
    if (stepIt == c_C_diatonicScale.end()) {
        LOG_ERROR("Unknown diatonic pitch step: " + _step);
    }
    const auto stepIdx = static_cast<size_t>(std::distance(c_C_diatonicScale.begin(), stepIt));

    // Same base formula as Helper::spelling2midiNote(), without the ties-upward rounding: this
    // is the one place that value is intentionally computed unrounded (see getMidiNumber()).
    return 12.0f * (_octave.value() + 1) + c_diatonicStepSemitones[stepIdx] + _alter;
}

bool Pitch::isRest() const { return _step == MUSIC_XML::PITCH::REST; }

void Pitch::setStep(const std::string& step) {
    const bool isValidStep =
        std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), step) !=
        c_C_diatonicScale.end();
    if (!isValidStep) {
        LOG_ERROR("Unknown diatonic pitch step: " + step);
    }

    _step = step;
    if (!_octave.has_value()) {
        _octave = 4;
    }
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

    _alter = alter;
}

void Pitch::setOctave(int octave) {
    if (isRest()) {
        LOG_ERROR("Cannot set the octave of a rest");
    }
    if (octave < c_minPitchOctave || octave > c_maxPitchOctave) {
        LOG_ERROR("Invalid octave value: " + std::to_string(octave));
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

void Pitch::roundToSemitone() { _alter = std::floor(_alter + 0.5f); }
