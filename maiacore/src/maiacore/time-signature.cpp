#include "maiacore/time-signature.h"

TimeSignature::TimeSignature(int timeUpper, int timeLower)
    : _timeUpper(timeUpper), _timeLower(timeLower) {}

int TimeSignature::getUpperValue() const { return _timeUpper; }

int TimeSignature::getLowerValue() const { return _timeLower; }

void TimeSignature::setUpperValue(int upperValue) { _timeUpper = upperValue; }

void TimeSignature::setLowerValue(int lowerValue) { _timeLower = lowerValue; }

METRIC TimeSignature::getMetric() const {
    // Check simple metrics
    if ((_timeUpper == 2 || _timeUpper == 3 || _timeUpper == 4) && _timeLower != 8) {
        return METRIC::SIMPLE;
    }

    // Check compound metrics
    if (_timeUpper % 3 == 0 && _timeLower == 8) {
        return METRIC::COMPOUND;
    }

    // Other cases
    return METRIC::COMPLEX;
}

const std::string TimeSignature::getMetricAsString() const {
    switch (getMetric()) {
        case METRIC::SIMPLE:
            return "SIMPLE";
        case METRIC::COMPOUND:
            return "COMPOUND";
        case METRIC::COMPLEX:
            return "COMPLEX";
    }

    return {};
}