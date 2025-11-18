#include <gtest/gtest.h>

#include "maiacore/utils.h"
#include <cmath>
#include <limits>

// ============================================================================
// hash() Function Tests
// ============================================================================

TEST(UtilsHash, HashEmptyString) {
    unsigned int result = hash("");
    EXPECT_EQ(result, 5381);  // Base hash value
}

TEST(UtilsHash, HashSingleChar) {
    unsigned int h_a = hash("a");
    unsigned int h_b = hash("b");

    EXPECT_NE(h_a, h_b);  // Different characters should hash differently
    EXPECT_NE(h_a, 5381);  // Should not equal base value
}

TEST(UtilsHash, HashMultipleChars) {
    unsigned int result = hash("hello");
    EXPECT_NE(result, 5381);  // Should not equal base value
}

TEST(UtilsHash, HashSameStringConsistent) {
    unsigned int h1 = hash("test");
    unsigned int h2 = hash("test");

    EXPECT_EQ(h1, h2);  // Same input should always produce same hash
}

TEST(UtilsHash, HashDifferentStrings) {
    unsigned int h_abc = hash("abc");
    unsigned int h_abd = hash("abd");
    unsigned int h_xyz = hash("xyz");

    EXPECT_NE(h_abc, h_abd);
    EXPECT_NE(h_abc, h_xyz);
    EXPECT_NE(h_abd, h_xyz);
}

TEST(UtilsHash, HashCaseSensitive) {
    unsigned int h_lower = hash("hello");
    unsigned int h_upper = hash("HELLO");
    unsigned int h_mixed = hash("Hello");

    EXPECT_NE(h_lower, h_upper);
    EXPECT_NE(h_lower, h_mixed);
    EXPECT_NE(h_upper, h_mixed);
}

TEST(UtilsHash, HashLongString) {
    const char* long_str = "This is a very long string to test the hash function with multiple characters";
    unsigned int result = hash(long_str);

    EXPECT_NE(result, 5381);  // Should not equal base value
}

TEST(UtilsHash, HashSpecialCharacters) {
    unsigned int h1 = hash("!@#$%");
    unsigned int h2 = hash("^&*()");

    EXPECT_NE(h1, h2);
    EXPECT_NE(h1, 5381);
}

TEST(UtilsHash, HashNumbers) {
    unsigned int h_123 = hash("123");
    unsigned int h_456 = hash("456");

    EXPECT_NE(h_123, h_456);
}

TEST(UtilsHash, HashWhitespace) {
    unsigned int h_space = hash(" ");
    unsigned int h_tab = hash("\t");
    unsigned int h_newline = hash("\n");

    EXPECT_NE(h_space, h_tab);
    EXPECT_NE(h_space, h_newline);
    EXPECT_NE(h_tab, h_newline);
}

// ============================================================================
// factorial() Function Tests
// ============================================================================

TEST(UtilsFactorial, FactorialZero) {
    EXPECT_EQ(factorial(0), 1);  // 0! = 1
}

TEST(UtilsFactorial, FactorialOne) {
    EXPECT_EQ(factorial(1), 1);  // 1! = 1
}

TEST(UtilsFactorial, FactorialTwo) {
    EXPECT_EQ(factorial(2), 2);  // 2! = 2
}

TEST(UtilsFactorial, FactorialThree) {
    EXPECT_EQ(factorial(3), 6);  // 3! = 6
}

TEST(UtilsFactorial, FactorialFour) {
    EXPECT_EQ(factorial(4), 24);  // 4! = 24
}

TEST(UtilsFactorial, FactorialFive) {
    EXPECT_EQ(factorial(5), 120);  // 5! = 120
}

TEST(UtilsFactorial, FactorialSix) {
    EXPECT_EQ(factorial(6), 720);  // 6! = 720
}

TEST(UtilsFactorial, FactorialSeven) {
    EXPECT_EQ(factorial(7), 5040);  // 7! = 5040
}

TEST(UtilsFactorial, FactorialTen) {
    EXPECT_EQ(factorial(10), 3628800);  // 10! = 3628800
}

TEST(UtilsFactorial, FactorialTwelve) {
    EXPECT_EQ(factorial(12), 479001600);  // 12! = 479001600
}

// ============================================================================
// isFloatEqual() Function Tests
// ============================================================================

TEST(UtilsIsFloatEqual, ExactlyEqual) {
    EXPECT_TRUE(isFloatEqual(1.0f, 1.0f));
    EXPECT_TRUE(isFloatEqual(0.0f, 0.0f));
    EXPECT_TRUE(isFloatEqual(-1.0f, -1.0f));
}

TEST(UtilsIsFloatEqual, VeryCloseWithinDefaultEpsilon) {
    EXPECT_TRUE(isFloatEqual(1.0f, 1.001f));  // Within 0.005
    EXPECT_TRUE(isFloatEqual(1.0f, 1.004f));  // Within 0.005
    EXPECT_TRUE(isFloatEqual(1.0f, 0.996f));  // Within 0.005
}

TEST(UtilsIsFloatEqual, BeyondDefaultEpsilon) {
    EXPECT_FALSE(isFloatEqual(1.0f, 1.01f));  // Beyond 0.005
    EXPECT_FALSE(isFloatEqual(1.0f, 0.99f));  // Beyond 0.005
}

TEST(UtilsIsFloatEqual, CustomEpsilonSmall) {
    float epsilon = 0.001f;
    EXPECT_TRUE(isFloatEqual(1.0f, 1.0005f, epsilon));
    EXPECT_FALSE(isFloatEqual(1.0f, 1.002f, epsilon));
}

TEST(UtilsIsFloatEqual, CustomEpsilonLarge) {
    float epsilon = 0.1f;
    EXPECT_TRUE(isFloatEqual(1.0f, 1.05f, epsilon));
    EXPECT_TRUE(isFloatEqual(1.0f, 0.95f, epsilon));
    EXPECT_FALSE(isFloatEqual(1.0f, 1.15f, epsilon));
}

TEST(UtilsIsFloatEqual, NegativeNumbers) {
    EXPECT_TRUE(isFloatEqual(-1.0f, -1.001f));
    EXPECT_TRUE(isFloatEqual(-5.5f, -5.502f));
    EXPECT_FALSE(isFloatEqual(-1.0f, -1.01f));
}

TEST(UtilsIsFloatEqual, ZeroComparison) {
    EXPECT_TRUE(isFloatEqual(0.0f, 0.001f));
    EXPECT_TRUE(isFloatEqual(0.001f, 0.0f));
    EXPECT_FALSE(isFloatEqual(0.0f, 0.01f));
}

TEST(UtilsIsFloatEqual, VerySmallNumbers) {
    EXPECT_TRUE(isFloatEqual(0.0001f, 0.0002f, 0.001f));
    EXPECT_FALSE(isFloatEqual(0.0001f, 0.002f, 0.001f));
}

TEST(UtilsIsFloatEqual, VeryLargeNumbers) {
    EXPECT_TRUE(isFloatEqual(1000.0f, 1000.004f));
    EXPECT_FALSE(isFloatEqual(1000.0f, 1000.01f));
}

TEST(UtilsIsFloatEqual, PositiveAndNegative) {
    EXPECT_FALSE(isFloatEqual(1.0f, -1.0f));
    EXPECT_FALSE(isFloatEqual(0.001f, -0.001f, 0.001f));
}

TEST(UtilsIsFloatEqual, EdgeCaseEpsilonZero) {
    // With epsilon = 0, the comparison is abs(A-B) < 0, which is always false
    // This is the actual behavior of the implementation
    EXPECT_FALSE(isFloatEqual(1.0f, 1.0f, 0.0f));
    EXPECT_FALSE(isFloatEqual(1.0f, 1.0f + std::numeric_limits<float>::epsilon(), 0.0f));
}

TEST(UtilsIsFloatEqual, BoundaryValues) {
    // Test at epsilon boundary
    float epsilon = 0.005f;
    EXPECT_TRUE(isFloatEqual(1.0f, 1.0049f, epsilon));   // Just within
    EXPECT_FALSE(isFloatEqual(1.0f, 1.0051f, epsilon));  // Just beyond
}

// ============================================================================
// ignore() Template Function Tests
// ============================================================================

TEST(UtilsIgnore, IgnoreInt) {
    int value = 42;
    ignore(value);  // Should compile and do nothing
    EXPECT_EQ(value, 42);  // Value unchanged
}

TEST(UtilsIgnore, IgnoreFloat) {
    float value = 3.14f;
    ignore(value);
    EXPECT_FLOAT_EQ(value, 3.14f);
}

TEST(UtilsIgnore, IgnoreString) {
    std::string str = "test";
    ignore(str);
    EXPECT_EQ(str, "test");
}

TEST(UtilsIgnore, IgnoreTemporary) {
    ignore(123);  // Should handle temporary values
    ignore(std::string("temp"));
    // Test just verifies compilation
    SUCCEED();
}

// ============================================================================
// Integration / Cross-Function Tests
// ============================================================================

TEST(UtilsIntegration, HashAndCompareResults) {
    unsigned int h1 = hash("value1");
    unsigned int h2 = hash("value2");
    unsigned int h3 = hash("value1");

    EXPECT_EQ(h1, h3);  // Same input = same hash
    EXPECT_NE(h1, h2);  // Different input = different hash (usually)
}

TEST(UtilsIntegration, FactorialSequence) {
    // Verify factorial sequence property: n! = n * (n-1)!
    for (int n = 2; n <= 10; ++n) {
        EXPECT_EQ(factorial(n), n * factorial(n - 1));
    }
}

TEST(UtilsIntegration, FloatEqualityTransitivity) {
    // If A ≈ B and B ≈ C (with same epsilon), check transitivity
    float a = 1.0f;
    float b = 1.002f;
    float c = 1.004f;
    float epsilon = 0.005f;

    EXPECT_TRUE(isFloatEqual(a, b, epsilon));
    EXPECT_TRUE(isFloatEqual(b, c, epsilon));
    // Note: Due to epsilon nature, A might not equal C
}
