#include <gtest/gtest.h>

#include <sstream>

#include "maiacore/fraction.h"

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(FractionConstructor, DefaultConstructor) {
    Fraction frac;
    EXPECT_EQ(frac.toString(), "1/1");
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 1.0f);
}

TEST(FractionConstructor, ParameterizedConstructor) {
    Fraction frac(3, 4);
    EXPECT_EQ(frac.toString(), "3/4");
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 0.75f);
}

TEST(FractionConstructor, SimplificationOnConstruction) {
    Fraction frac(6, 8);
    EXPECT_EQ(frac.toString(), "3/4");  // Should simplify to 3/4
}

TEST(FractionConstructor, NegativeDenominatorNormalization) {
    Fraction frac(3, -4);
    EXPECT_EQ(frac.toString(), "-3/4");  // Denominator should become positive
}

TEST(FractionConstructor, BothNegativeSimplification) {
    Fraction frac(-6, -8);
    EXPECT_EQ(frac.toString(), "3/4");  // Both negatives cancel out
}

TEST(FractionConstructor, ZeroNumerator) {
    Fraction frac(0, 5);
    EXPECT_EQ(frac.toString(), "0/1");  // 0/5 simplifies to 0/1
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 0.0f);
}

TEST(FractionConstructor, NegativeNumerator) {
    Fraction frac(-3, 4);
    EXPECT_EQ(frac.toString(), "-3/4");
    EXPECT_FLOAT_EQ(frac.getFloatValue(), -0.75f);
}

// ============================================================================
// Arithmetic Addition Tests
// ============================================================================

TEST(FractionAddition, AddSameDenominator) {
    Fraction a(1, 4);
    Fraction b(2, 4);
    Fraction result = a + b;
    EXPECT_EQ(result.toString(), "3/4");
}

TEST(FractionAddition, AddDifferentDenominators) {
    Fraction a(1, 2);
    Fraction b(1, 3);
    Fraction result = a + b;
    EXPECT_EQ(result.toString(), "5/6");
}

TEST(FractionAddition, AddWithZero) {
    Fraction a(3, 4);
    Fraction b(0, 1);
    Fraction result = a + b;
    EXPECT_EQ(result.toString(), "3/4");
}

TEST(FractionAddition, AddNegativeNumbers) {
    Fraction a(-1, 2);
    Fraction b(1, 3);
    Fraction result = a + b;
    EXPECT_EQ(result.toString(), "-1/6");
}

TEST(FractionAddition, AddResultSimplification) {
    Fraction a(2, 6);
    Fraction b(2, 6);
    Fraction result = a + b;
    EXPECT_EQ(result.toString(), "2/3");  // (1/3 + 1/3) = 2/3
}

// ============================================================================
// Arithmetic Subtraction Tests
// ============================================================================

TEST(FractionSubtraction, SubtractSameDenominator) {
    Fraction a(3, 4);
    Fraction b(1, 4);
    Fraction result = a - b;
    EXPECT_EQ(result.toString(), "1/2");
}

TEST(FractionSubtraction, SubtractDifferentDenominators) {
    Fraction a(3, 4);
    Fraction b(1, 2);
    Fraction result = a - b;
    EXPECT_EQ(result.toString(), "1/4");
}

TEST(FractionSubtraction, SubtractResultingInZero) {
    Fraction a(2, 3);
    Fraction b(2, 3);
    Fraction result = a - b;
    EXPECT_EQ(result.toString(), "0/1");
}

TEST(FractionSubtraction, SubtractResultingInNegative) {
    Fraction a(1, 4);
    Fraction b(3, 4);
    Fraction result = a - b;
    EXPECT_EQ(result.toString(), "-1/2");
}

TEST(FractionSubtraction, SubtractNegativeNumbers) {
    Fraction a(1, 2);
    Fraction b(-1, 3);
    Fraction result = a - b;
    EXPECT_EQ(result.toString(), "5/6");
}

// ============================================================================
// Arithmetic Multiplication Tests
// ============================================================================

TEST(FractionMultiplication, MultiplySimple) {
    Fraction a(2, 3);
    Fraction b(3, 4);
    Fraction result = a * b;
    EXPECT_EQ(result.toString(), "1/2");  // (2*3)/(3*4) = 6/12 = 1/2
}

TEST(FractionMultiplication, MultiplyByZero) {
    Fraction a(3, 4);
    Fraction b(0, 1);
    Fraction result = a * b;
    EXPECT_EQ(result.toString(), "0/1");
}

TEST(FractionMultiplication, MultiplyByOne) {
    Fraction a(3, 4);
    Fraction b(1, 1);
    Fraction result = a * b;
    EXPECT_EQ(result.toString(), "3/4");
}

TEST(FractionMultiplication, MultiplyNegativeNumbers) {
    Fraction a(-2, 3);
    Fraction b(3, 4);
    Fraction result = a * b;
    EXPECT_EQ(result.toString(), "-1/2");
}

TEST(FractionMultiplication, MultiplyTwoNegatives) {
    Fraction a(-2, 3);
    Fraction b(-3, 4);
    Fraction result = a * b;
    EXPECT_EQ(result.toString(), "1/2");  // Negatives cancel out
}

// ============================================================================
// Arithmetic Division Tests
// ============================================================================

TEST(FractionDivision, DivideSimple) {
    Fraction a(1, 2);
    Fraction b(1, 4);
    Fraction result = a / b;
    EXPECT_EQ(result.toString(), "2/1");  // (1/2) / (1/4) = 2
}

TEST(FractionDivision, DivideByOne) {
    Fraction a(3, 4);
    Fraction b(1, 1);
    Fraction result = a / b;
    EXPECT_EQ(result.toString(), "3/4");
}

TEST(FractionDivision, DivideNegativeNumbers) {
    Fraction a(-1, 2);
    Fraction b(1, 4);
    Fraction result = a / b;
    EXPECT_EQ(result.toString(), "-2/1");
}

TEST(FractionDivision, DivideTwoNegatives) {
    Fraction a(-1, 2);
    Fraction b(-1, 4);
    Fraction result = a / b;
    EXPECT_EQ(result.toString(), "2/1");  // Negatives cancel out
}

TEST(FractionDivision, DivideWithSimplification) {
    Fraction a(2, 3);
    Fraction b(4, 6);
    Fraction result = a / b;
    EXPECT_EQ(result.toString(), "1/1");  // (2/3) / (2/3) = 1
}

// ============================================================================
// Compound Assignment Tests (+=, -=, *=, /=)
// ============================================================================

TEST(FractionCompoundAssignment, AddAssign) {
    Fraction a(1, 2);
    Fraction b(1, 3);
    a += b;
    EXPECT_EQ(a.toString(), "5/6");
}

TEST(FractionCompoundAssignment, SubtractAssign) {
    Fraction a(3, 4);
    Fraction b(1, 4);
    a -= b;
    EXPECT_EQ(a.toString(), "1/2");
}

TEST(FractionCompoundAssignment, MultiplyAssign) {
    Fraction a(2, 3);
    Fraction b(3, 4);
    a *= b;
    EXPECT_EQ(a.toString(), "1/2");
}

TEST(FractionCompoundAssignment, DivideAssign) {
    Fraction a(1, 2);
    Fraction b(1, 4);
    a /= b;
    EXPECT_EQ(a.toString(), "2/1");
}

TEST(FractionCompoundAssignment, ChainedAddAssign) {
    Fraction a(1, 4);
    Fraction b(1, 4);
    Fraction c(1, 4);
    a += b;
    a += c;
    EXPECT_EQ(a.toString(), "3/4");
}

TEST(FractionCompoundAssignment, AddAssignWithNegative) {
    Fraction a(1, 2);
    Fraction b(-1, 3);
    a += b;
    EXPECT_EQ(a.toString(), "1/6");
}

// ============================================================================
// Comparison Operator Tests
// ============================================================================

TEST(FractionComparison, EqualityTrue) {
    Fraction a(1, 2);
    Fraction b(2, 4);  // Should simplify to 1/2
    EXPECT_TRUE(a == b);
}

TEST(FractionComparison, EqualityFalse) {
    Fraction a(1, 2);
    Fraction b(1, 3);
    EXPECT_FALSE(a == b);
}

TEST(FractionComparison, GreaterThanTrue) {
    Fraction a(3, 4);
    Fraction b(1, 2);
    EXPECT_TRUE(a > b);
}

TEST(FractionComparison, GreaterThanFalse) {
    Fraction a(1, 2);
    Fraction b(3, 4);
    EXPECT_FALSE(a > b);
}

TEST(FractionComparison, GreaterThanOrEqualTrue) {
    Fraction a(3, 4);
    Fraction b(3, 4);
    EXPECT_TRUE(a >= b);
}

TEST(FractionComparison, GreaterThanOrEqualWithGreater) {
    Fraction a(3, 4);
    Fraction b(1, 2);
    EXPECT_TRUE(a >= b);
}

TEST(FractionComparison, LessThanOrEqualTrue) {
    Fraction a(1, 2);
    Fraction b(3, 4);
    EXPECT_TRUE(a <= b);
}

TEST(FractionComparison, LessThanOrEqualWithEqual) {
    Fraction a(1, 2);
    Fraction b(2, 4);
    EXPECT_TRUE(a <= b);
}

TEST(FractionComparison, CompareNegativeNumbers) {
    Fraction a(-1, 2);
    Fraction b(-1, 3);
    EXPECT_TRUE(b > a);  // -1/3 > -1/2 (less negative)
}

TEST(FractionComparison, CompareZeroWithNegative) {
    Fraction a(0, 1);
    Fraction b(-1, 2);
    EXPECT_TRUE(a > b);
}

// ============================================================================
// String Conversion Tests
// ============================================================================

TEST(FractionString, ToStringSimple) {
    Fraction frac(3, 4);
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionString, ToStringWithNegative) {
    Fraction frac(-3, 4);
    EXPECT_EQ(frac.toString(), "-3/4");
}

TEST(FractionString, ToStringSimplified) {
    Fraction frac(6, 8);
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionString, StringToFractionSimple) {
    Fraction frac = Fraction::stringToFraction("3/4");
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionString, StringToFractionWithSimplification) {
    Fraction frac = Fraction::stringToFraction("6/8");
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionString, StringToFractionNegative) {
    Fraction frac = Fraction::stringToFraction("-3/4");
    EXPECT_EQ(frac.toString(), "-3/4");
}

TEST(FractionString, StringToFractionZero) {
    Fraction frac = Fraction::stringToFraction("0/5");
    EXPECT_EQ(frac.toString(), "0/1");
}

// ============================================================================
// Float Value Tests
// ============================================================================

TEST(FractionFloat, GetFloatValueSimple) {
    Fraction frac(1, 2);
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 0.5f);
}

TEST(FractionFloat, GetFloatValueThreeQuarters) {
    Fraction frac(3, 4);
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 0.75f);
}

TEST(FractionFloat, GetFloatValueNegative) {
    Fraction frac(-1, 2);
    EXPECT_FLOAT_EQ(frac.getFloatValue(), -0.5f);
}

TEST(FractionFloat, GetFloatValueZero) {
    Fraction frac(0, 1);
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 0.0f);
}

TEST(FractionFloat, GetFloatValueGreaterThanOne) {
    Fraction frac(5, 4);
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 1.25f);
}

// ============================================================================
// SetValues Tests
// ============================================================================

TEST(FractionSetValues, SetValuesSimple) {
    Fraction frac;
    frac.setValues(3, 4);
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionSetValues, SetValuesWithSimplification) {
    Fraction frac;
    frac.setValues(6, 8);
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionSetValues, SetValuesNegative) {
    Fraction frac;
    frac.setValues(-3, 4);
    EXPECT_EQ(frac.toString(), "-3/4");
}

TEST(FractionSetValues, SetValuesNegativeDenominator) {
    Fraction frac;
    frac.setValues(3, -4);
    EXPECT_EQ(frac.toString(), "-3/4");
}

TEST(FractionSetValues, SetValuesZeroNumerator) {
    Fraction frac(1, 2);
    frac.setValues(0, 5);
    EXPECT_EQ(frac.toString(), "0/1");
}

// ============================================================================
// Explicit Simplification Tests
// ============================================================================

TEST(FractionSimplification, SimplifyFractionAlreadySimplified) {
    Fraction frac(3, 4);
    frac.simplifyFraction();
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionSimplification, SimplifyFractionLargeGCD) {
    Fraction frac(12, 16);
    EXPECT_EQ(frac.toString(), "3/4");  // Automatically simplified on construction
}

TEST(FractionSimplification, SimplifyFractionAfterSetValues) {
    Fraction frac;
    frac.setValues(10, 15);
    EXPECT_EQ(frac.toString(), "2/3");  // setValues includes simplification
}

// ============================================================================
// Stream Operator Tests
// ============================================================================

TEST(FractionStream, OutputStreamOperator) {
    Fraction frac(3, 4);
    std::ostringstream oss;
    oss << frac;
    EXPECT_EQ(oss.str(), "3/4");
}

TEST(FractionStream, OutputStreamNegative) {
    Fraction frac(-3, 4);
    std::ostringstream oss;
    oss << frac;
    EXPECT_EQ(oss.str(), "-3/4");
}

TEST(FractionStream, InputStreamOperator) {
    Fraction frac;
    std::istringstream iss("3/4");
    iss >> frac;
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionStream, InputStreamWithSimplification) {
    Fraction frac;
    std::istringstream iss("6/8");
    iss >> frac;
    EXPECT_EQ(frac.toString(), "3/4");
}

TEST(FractionStream, InputStreamNegative) {
    Fraction frac;
    std::istringstream iss("-3/4");
    iss >> frac;
    EXPECT_EQ(frac.toString(), "-3/4");
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

TEST(FractionEdgeCases, LargeNumeratorAndDenominator) {
    Fraction frac(1000, 2000);
    EXPECT_EQ(frac.toString(), "1/2");
}

TEST(FractionEdgeCases, VeryLargeSimplifiedFraction) {
    Fraction frac(999999, 1000000);
    EXPECT_EQ(frac.toString(), "999999/1000000");  // No common factors
}

TEST(FractionEdgeCases, WholeNumber) {
    Fraction frac(4, 2);
    EXPECT_EQ(frac.toString(), "2/1");
}

TEST(FractionEdgeCases, ImproperFraction) {
    Fraction frac(7, 4);
    EXPECT_EQ(frac.toString(), "7/4");
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 1.75f);
}

TEST(FractionEdgeCases, NegativeImproperFraction) {
    Fraction frac(-7, 4);
    EXPECT_EQ(frac.toString(), "-7/4");
    EXPECT_FLOAT_EQ(frac.getFloatValue(), -1.75f);
}

TEST(FractionEdgeCases, AdditionResultingInWholeNumber) {
    Fraction a(1, 2);
    Fraction b(1, 2);
    Fraction result = a + b;
    EXPECT_EQ(result.toString(), "1/1");
}

TEST(FractionEdgeCases, DivisionByFractionLessThanOne) {
    Fraction a(1, 2);
    Fraction b(1, 4);
    Fraction result = a / b;
    EXPECT_EQ(result.toString(), "2/1");
}

// ============================================================================
// Complex Arithmetic Scenarios
// ============================================================================

TEST(FractionComplexArithmetic, MultipleOperationsChained) {
    Fraction a(1, 2);
    Fraction b(1, 3);
    Fraction c(1, 4);
    Fraction result = a + b + c;
    EXPECT_EQ(result.toString(), "13/12");
}

TEST(FractionComplexArithmetic, MixedOperations) {
    Fraction a(1, 2);
    Fraction b(1, 3);
    Fraction c(2, 1);
    Fraction result = (a + b) * c;
    EXPECT_EQ(result.toString(), "5/3");
}

TEST(FractionComplexArithmetic, DivisionAndMultiplication) {
    Fraction a(1, 2);
    Fraction b(1, 4);
    Fraction c(1, 8);
    Fraction result = (a / b) * c;
    EXPECT_EQ(result.toString(), "1/4");
}

TEST(FractionComplexArithmetic, SubtractionResultingInNegative) {
    Fraction a(1, 4);
    Fraction b(1, 2);
    Fraction c(1, 4);
    Fraction result = a - b - c;
    EXPECT_EQ(result.toString(), "-1/2");
}
