#pragma once

#include <cmath>
#include <iostream>
#include <numeric>

/**
 * @file utils.h
 * @brief Utility functions and templates for internal maiacore operations.
 *
 * Provides compile-time and runtime utilities including hashing for switch-case optimization,
 * mathematical functions, and floating-point comparison with epsilon tolerance for music
 * analysis applications.
 */

/**
 * @brief Suppresses unused variable warnings by accepting and discarding a forwarding reference.
 * @tparam T Type of the unused variable.
 * @param Unnamed forwarding reference to ignore.
 * @details Utility template for suppressing compiler warnings when function parameters
 *          are intentionally unused in certain code paths or conditional compilation scenarios.
 */
template <typename T>
void ignore(T &&) {}

/**
 * @brief Compile-time string hash function using the djb2 algorithm.
 * @param s Null-terminated C-string to hash.
 * @param off Offset for recursive character processing (default: 0, internal use).
 * @return Unsigned integer hash value computed at compile time.
 * @details Implements the djb2 hash algorithm recursively for constexpr evaluation.
 *          Used primarily for optimizing switch-case statements with string literals
 *          in MusicXML parsing and enumeration conversions. The hash formula is:
 *          hash * 33 XOR current_character, starting with seed value 5381.
 */
constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}

/**
 * @brief Compile-time factorial calculation using recursive template evaluation.
 * @param n Non-negative integer input (n >= 0).
 * @return Factorial of n (n! = n × (n-1) × ... × 2 × 1), with 0! = 1.
 * @details Recursively computes factorial at compile time. Used for combinatorial
 *          calculations in harmonic analysis and voice permutation computations.
 * @warning Only suitable for small values of n due to integer overflow constraints.
 *          For n > 12, results exceed 32-bit integer capacity.
 */
constexpr int factorial(const int n) { return (n == 0) || (n == 1) ? 1 : n * factorial(n - 1); }

/**
 * @brief Floating-point equality comparison with epsilon tolerance for approximate matching.
 * @param A First value to compare.
 * @param B Second value to compare.
 * @param epsilon Tolerance threshold for considering values equal (default: 0.005).
 * @return True if the absolute difference |A - B| is strictly less than epsilon.
 * @details Uses absolute difference comparison suitable for music analysis where small deviations
 *          from rounding errors (e.g., frequency calculations, rhythmic quantization, spectral
 * analysis) should be treated as equivalent. The default epsilon of 0.005 accommodates typical
 *          floating-point precision errors in quarter-note duration and frequency ratio
 * computations.
 * @note The comparison is symmetric: isFloatEqual(A, B) == isFloatEqual(B, A).
 *       However, transitivity is not guaranteed: if isFloatEqual(A, B) and isFloatEqual(B, C),
 *       it does not necessarily follow that isFloatEqual(A, C).
 */
inline bool isFloatEqual(float A, float B, float epsilon = 0.005f) {
    return (std::fabs(A - B) < epsilon);
}

/**
 * @brief Rounds a value to the nearest integer, ties upward (toward +infinity).
 * @tparam T A floating-point type.
 * @param value Value to round.
 * @return `std::floor(value + 0.5)`, as T.
 * @details The library's single implementation of the ties-upward rule for pitch positions and
 *          alters: a quarter tone always rounds to the semitone above it, on the flat side as on
 *          the sharp side ("C1x4", 60.5, rounds to 61; "D1b4", 61.5, to 62). std::round() and
 *          std::lround() round half away from zero instead, and disagree with this rule for every
 *          negative tie: -0.5 rounds to 0 here and to -1 there.
 */
template <typename T>
T roundTiesUpward(const T value) {
    return std::floor(value + static_cast<T>(0.5));
}

/**
 * @brief Tests whether a value is a whole multiple of 0.5 -- a point of the quarter-tone grid.
 * @tparam T A floating-point type.
 * @param value A pitch position, alter or interval, in semitones.
 * @return True if value is finite and exactly a multiple of 0.5.
 * @details The library's single definition of "on the grid". Every pitch position, alter and
 *          transposition this library can spell is a multiple of 0.5 semitones: there is no pitch
 *          between "C1x4" and "C#4". The test is exact, with no tolerance, so a value near the
 *          grid (0.46, or 0.99996) is off it rather than snapped to it. Doubling turns the test
 *          into an integer test, and doubling is exact in binary floating point. A non-finite
 *          value is never on the grid.
 */
template <typename T>
bool isOnQuarterToneGrid(const T value) {
    const T twice = value * static_cast<T>(2);
    return std::isfinite(value) && twice == std::floor(twice);
}

/**
 * @brief Tests whether a value is a quarter tone: on the grid, but between two semitones.
 * @tparam T A floating-point type.
 * @param value A pitch position, alter or interval, in semitones.
 * @return True if value is an odd multiple of 0.5 (0.5, -1.5, 60.5, ...).
 * @details False for a whole number of semitones and for any value off the grid (see
 *          isOnQuarterToneGrid()).
 */
template <typename T>
bool isQuarterToneValue(const T value) {
    return isOnQuarterToneGrid(value) && value != std::floor(value);
}
