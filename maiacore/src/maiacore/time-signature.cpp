#include "maiacore/time-signature.h"

TimeSignature::TimeSignature(int timeUpper, int timeLower)
    : _timeUpper(timeUpper), _timeLower(timeLower) {}

int TimeSignature::getUpperValue() const { return _timeUpper; }

int TimeSignature::getLowerValue() const { return _timeLower; }

void TimeSignature::setUpperValue(int upperValue) { _timeUpper = upperValue; }

void TimeSignature::setLowerValue(int lowerValue) { _timeLower = lowerValue; }

METRIC TimeSignature::getMetric() const {
    // SIMPLE: numerator ∈ {2, 3, 4} (binary subdivision)
    if (_timeUpper == 2 || _timeUpper == 3 || _timeUpper == 4) {
        return METRIC::SIMPLE;
    }

    // COMPOUND: numerator is multiple of 3 and > 3 (ternary subdivision)
    if (_timeUpper > 3 && _timeUpper % 3 == 0) {
        return METRIC::COMPOUND;
    }

    // IRREGULAR: asymmetric meters (everything else)
    return METRIC::IRREGULAR;
}

const std::string TimeSignature::getMetricAsString() const {
    switch (getMetric()) {
        case METRIC::SIMPLE:
            return "SIMPLE";
        case METRIC::COMPOUND:
            return "COMPOUND";
        case METRIC::IRREGULAR:
            return "IRREGULAR";
    }

    return {};
}