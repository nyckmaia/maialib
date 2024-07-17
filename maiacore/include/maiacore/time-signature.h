#pragma once

#include <string>

enum class METRIC { SIMPLE, COMPOUND, COMPLEX };

class TimeSignature {
   public:
    TimeSignature(const int timeUpper = 4, const int timeLower = 4);

    int getUpperValue() const;
    int getLowerValue() const;

    void setUpperValue(int upperValue);
    void setLowerValue(int lowerValue);

    METRIC getMetric() const;
    const std::string getMetricAsString() const;

   private:
    int _timeUpper;
    int _timeLower;
};