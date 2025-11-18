#pragma once

#include <string>

/**
 * @brief Enumeration for metric types in time signatures.
 * SIMPLE: numerator ∈ {2, 3, 4} (binary subdivision);
 * COMPOUND: numerator is multiple of 3 and > 3 (ternary subdivision);
 * IRREGULAR: asymmetric meters (e.g., 5/4, 7/8, 11/8).
 */
enum class METRIC { SIMPLE, COMPOUND, IRREGULAR };

/**
 * @brief Represents a musical time signature (meter) for a measure.
 *
 * The TimeSignature class provides methods for setting and querying the upper and lower values of the time signature,
 * as well as determining the metric type (simple, compound, or complex). It is designed for music analysis,
 * computational musicology, and MusicXML processing.
 */
class TimeSignature {
   public:
    /**
     * @brief Constructs a TimeSignature with specified upper and lower values.
     * @param timeUpper Numerator of the time signature (default: 4).
     * @param timeLower Denominator of the time signature (default: 4).
     */
    TimeSignature(const int timeUpper = 4, const int timeLower = 4);

    /**
     * @brief Returns the upper value (numerator) of the time signature.
     * @return Integer representing the numerator.
     */
    int getUpperValue() const;

    /**
     * @brief Returns the lower value (denominator) of the time signature.
     * @return Integer representing the denominator.
     */
    int getLowerValue() const;

    /**
     * @brief Sets the upper value (numerator) of the time signature.
     * @param upperValue New numerator value.
     */
    void setUpperValue(int upperValue);

    /**
     * @brief Sets the lower value (denominator) of the time signature.
     * @param lowerValue New denominator value.
     */
    void setLowerValue(int lowerValue);

    /**
     * @brief Returns the metric type of the time signature (simple, compound, or irregular).
     * @details Simple: 2/4, 3/4, 4/4, 2/8, 3/8, 4/8, etc.;
     *          Compound: 6/8, 9/8, 12/8, 6/4, 9/4, 15/16, etc.;
     *          Irregular: 5/4, 7/8, 11/8, 13/8, etc.
     * @return METRIC enum value.
     */
    METRIC getMetric() const;

    /**
     * @brief Returns the metric type as a string ("SIMPLE", "COMPOUND", "IRREGULAR").
     * @return String representation of the metric type.
     */
    const std::string getMetricAsString() const;

   private:
    int _timeUpper;  ///< Numerator of the time signature.
    int _timeLower;  ///< Denominator of the time signature.
};