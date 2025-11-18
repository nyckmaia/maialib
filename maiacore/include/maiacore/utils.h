#pragma once

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
 *          from rounding errors (e.g., frequency calculations, rhythmic quantization, spectral analysis)
 *          should be treated as equivalent. The default epsilon of 0.005 accommodates typical
 *          floating-point precision errors in quarter-note duration and frequency ratio computations.
 * @note The comparison is symmetric: isFloatEqual(A, B) == isFloatEqual(B, A).
 *       However, transitivity is not guaranteed: if isFloatEqual(A, B) and isFloatEqual(B, C),
 *       it does not necessarily follow that isFloatEqual(A, C).
 */
inline bool isFloatEqual(float A, float B, float epsilon = 0.005f) {
    return (std::fabs(A - B) < epsilon);
}
