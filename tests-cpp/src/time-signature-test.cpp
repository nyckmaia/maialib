#include <gtest/gtest.h>

#include "maiacore/time-signature.h"

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(TimeSignatureConstructor, DefaultConstructor) {
    TimeSignature ts;
    EXPECT_EQ(ts.getUpperValue(), 4);
    EXPECT_EQ(ts.getLowerValue(), 4);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
}

TEST(TimeSignatureConstructor, CommonTime4_4) {
    TimeSignature ts(4, 4);
    EXPECT_EQ(ts.getUpperValue(), 4);
    EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(TimeSignatureConstructor, ThreeFour) {
    TimeSignature ts(3, 4);
    EXPECT_EQ(ts.getUpperValue(), 3);
    EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(TimeSignatureConstructor, TwoFour) {
    TimeSignature ts(2, 4);
    EXPECT_EQ(ts.getUpperValue(), 2);
    EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(TimeSignatureConstructor, SixEight) {
    TimeSignature ts(6, 8);
    EXPECT_EQ(ts.getUpperValue(), 6);
    EXPECT_EQ(ts.getLowerValue(), 8);
}

TEST(TimeSignatureConstructor, TwoTwo) {
    TimeSignature ts(2, 2);
    EXPECT_EQ(ts.getUpperValue(), 2);
    EXPECT_EQ(ts.getLowerValue(), 2);
}

// ============================================================================
// Get Upper/Lower Value Tests
// ============================================================================

TEST(TimeSignatureGetters, GetUpperValue) {
    TimeSignature ts(5, 4);
    EXPECT_EQ(ts.getUpperValue(), 5);
}

TEST(TimeSignatureGetters, GetLowerValue) {
    TimeSignature ts(3, 8);
    EXPECT_EQ(ts.getLowerValue(), 8);
}

TEST(TimeSignatureGetters, GetBothValues) {
    TimeSignature ts(7, 8);
    EXPECT_EQ(ts.getUpperValue(), 7);
    EXPECT_EQ(ts.getLowerValue(), 8);
}

TEST(TimeSignatureGetters, VariousTimeSignatures) {
    std::vector<std::pair<int, int>> signatures = {
        {2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 8}, {9, 8}, {12, 8}, {3, 8}, {7, 8}
    };

    for (const auto& sig : signatures) {
        TimeSignature ts(sig.first, sig.second);
        EXPECT_EQ(ts.getUpperValue(), sig.first);
        EXPECT_EQ(ts.getLowerValue(), sig.second);
    }
}

// ============================================================================
// Set Upper/Lower Value Tests
// ============================================================================

TEST(TimeSignatureSetters, SetUpperValue) {
    TimeSignature ts;
    EXPECT_EQ(ts.getUpperValue(), 4);

    ts.setUpperValue(3);
    EXPECT_EQ(ts.getUpperValue(), 3);
    EXPECT_EQ(ts.getLowerValue(), 4);  // Lower unchanged
}

TEST(TimeSignatureSetters, SetLowerValue) {
    TimeSignature ts;
    EXPECT_EQ(ts.getLowerValue(), 4);

    ts.setLowerValue(8);
    EXPECT_EQ(ts.getLowerValue(), 8);
    EXPECT_EQ(ts.getUpperValue(), 4);  // Upper unchanged
}

TEST(TimeSignatureSetters, SetBothValues) {
    TimeSignature ts;

    ts.setUpperValue(6);
    ts.setLowerValue(8);

    EXPECT_EQ(ts.getUpperValue(), 6);
    EXPECT_EQ(ts.getLowerValue(), 8);
}

TEST(TimeSignatureSetters, ChangeMultipleTimes) {
    TimeSignature ts(4, 4);

    ts.setUpperValue(3);
    EXPECT_EQ(ts.getUpperValue(), 3);

    ts.setUpperValue(5);
    EXPECT_EQ(ts.getUpperValue(), 5);

    ts.setLowerValue(8);
    EXPECT_EQ(ts.getLowerValue(), 8);
}

// ============================================================================
// Simple Metric Tests
// ============================================================================

TEST(TimeSignatureMetric, Simple2_4) {
    TimeSignature ts(2, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

TEST(TimeSignatureMetric, Simple3_4) {
    TimeSignature ts(3, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

TEST(TimeSignatureMetric, Simple4_4) {
    TimeSignature ts(4, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

TEST(TimeSignatureMetric, Simple2_2) {
    TimeSignature ts(2, 2);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
}

TEST(TimeSignatureMetric, Simple3_8) {
    TimeSignature ts(3, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

TEST(TimeSignatureMetric, Simple4_8) {
    TimeSignature ts(4, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

TEST(TimeSignatureMetric, Simple2_8) {
    TimeSignature ts(2, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

// ============================================================================
// Compound Metric Tests
// ============================================================================

TEST(TimeSignatureMetric, Compound6_8) {
    TimeSignature ts(6, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound9_8) {
    TimeSignature ts(9, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound12_8) {
    TimeSignature ts(12, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound6_4) {
    TimeSignature ts(6, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound9_4) {
    TimeSignature ts(9, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound12_4) {
    TimeSignature ts(12, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound6_16) {
    TimeSignature ts(6, 16);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetric, Compound15_8) {
    TimeSignature ts(15, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

// ============================================================================
// Irregular Metric Tests
// ============================================================================

TEST(TimeSignatureMetric, Irregular5_4) {
    TimeSignature ts(5, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
    EXPECT_EQ(ts.getMetricAsString(), "IRREGULAR");
}

TEST(TimeSignatureMetric, Irregular7_8) {
    TimeSignature ts(7, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
    EXPECT_EQ(ts.getMetricAsString(), "IRREGULAR");
}

TEST(TimeSignatureMetric, Irregular5_8) {
    TimeSignature ts(5, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}

TEST(TimeSignatureMetric, Irregular7_4) {
    TimeSignature ts(7, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}

TEST(TimeSignatureMetric, Irregular11_8) {
    TimeSignature ts(11, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}

TEST(TimeSignatureMetric, Irregular13_8) {
    TimeSignature ts(13, 8);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}

// ============================================================================
// Metric String Tests
// ============================================================================

TEST(TimeSignatureMetricString, SimpleMetricString) {
    TimeSignature ts(4, 4);
    EXPECT_EQ(ts.getMetricAsString(), "SIMPLE");
}

TEST(TimeSignatureMetricString, CompoundMetricString) {
    TimeSignature ts(6, 8);
    EXPECT_EQ(ts.getMetricAsString(), "COMPOUND");
}

TEST(TimeSignatureMetricString, IrregularMetricString) {
    TimeSignature ts(5, 4);
    EXPECT_EQ(ts.getMetricAsString(), "IRREGULAR");
}

// ============================================================================
// Edge Cases and Uncommon Time Signatures
// ============================================================================

TEST(TimeSignatureEdgeCases, UncommonUpperValue) {
    TimeSignature ts(1, 4);
    EXPECT_EQ(ts.getUpperValue(), 1);
    EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(TimeSignatureEdgeCases, LargeDenominator) {
    TimeSignature ts(3, 16);
    EXPECT_EQ(ts.getUpperValue(), 3);
    EXPECT_EQ(ts.getLowerValue(), 16);
}

TEST(TimeSignatureEdgeCases, VeryLargeDenominator) {
    TimeSignature ts(2, 32);
    EXPECT_EQ(ts.getUpperValue(), 2);
    EXPECT_EQ(ts.getLowerValue(), 32);
}

TEST(TimeSignatureEdgeCases, LargeNumerator) {
    TimeSignature ts(16, 4);
    EXPECT_EQ(ts.getUpperValue(), 16);
    EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(TimeSignatureEdgeCases, VeryIrregularSignature) {
    TimeSignature ts(17, 16);
    EXPECT_EQ(ts.getUpperValue(), 17);
    EXPECT_EQ(ts.getLowerValue(), 16);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}

// ============================================================================
// Metric Classification Boundaries
// ============================================================================

TEST(TimeSignatureMetricBoundaries, ThreeIsAlwaysSimple) {
    TimeSignature ts3_4(3, 4);
    TimeSignature ts3_8(3, 8);
    TimeSignature ts3_2(3, 2);
    TimeSignature ts3_16(3, 16);

    EXPECT_EQ(ts3_4.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts3_8.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts3_2.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(ts3_16.getMetric(), METRIC::SIMPLE);
}

TEST(TimeSignatureMetricBoundaries, SixIsAlwaysCompound) {
    TimeSignature ts6_8(6, 8);
    TimeSignature ts6_4(6, 4);
    TimeSignature ts6_16(6, 16);
    TimeSignature ts6_2(6, 2);

    EXPECT_EQ(ts6_8.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts6_4.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts6_16.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(ts6_2.getMetric(), METRIC::COMPOUND);
}

TEST(TimeSignatureMetricBoundaries, FiveIsAlwaysIrregular) {
    TimeSignature ts5_4(5, 4);
    TimeSignature ts5_8(5, 8);
    TimeSignature ts5_16(5, 16);

    EXPECT_EQ(ts5_4.getMetric(), METRIC::IRREGULAR);
    EXPECT_EQ(ts5_8.getMetric(), METRIC::IRREGULAR);
    EXPECT_EQ(ts5_16.getMetric(), METRIC::IRREGULAR);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(TimeSignatureIntegration, ConstructModifyCheck) {
    TimeSignature ts;

    // Initial state (4/4)
    EXPECT_EQ(ts.getUpperValue(), 4);
    EXPECT_EQ(ts.getLowerValue(), 4);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);

    // Change to 3/4
    ts.setUpperValue(3);
    EXPECT_EQ(ts.getUpperValue(), 3);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);

    // Change to 6/8
    ts.setUpperValue(6);
    ts.setLowerValue(8);
    EXPECT_EQ(ts.getUpperValue(), 6);
    EXPECT_EQ(ts.getLowerValue(), 8);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);

    // Change to 5/4
    ts.setUpperValue(5);
    ts.setLowerValue(4);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}

TEST(TimeSignatureIntegration, AllStandardSignatures) {
    // Test with known reliable classifications
    std::vector<std::tuple<int, int, METRIC>> standard_sigs = {
        {2, 4, METRIC::SIMPLE},    {3, 4, METRIC::SIMPLE},    {4, 4, METRIC::SIMPLE},
        {2, 2, METRIC::SIMPLE},    {6, 8, METRIC::COMPOUND},
        {9, 8, METRIC::COMPOUND},  {12, 8, METRIC::COMPOUND}, {5, 4, METRIC::IRREGULAR},
        {7, 8, METRIC::IRREGULAR}
    };

    for (const auto& sig : standard_sigs) {
        TimeSignature ts(std::get<0>(sig), std::get<1>(sig));
        EXPECT_EQ(ts.getUpperValue(), std::get<0>(sig));
        EXPECT_EQ(ts.getLowerValue(), std::get<1>(sig));
        EXPECT_EQ(ts.getMetric(), std::get<2>(sig));
    }
}

TEST(TimeSignatureIntegration, MetricStringConsistency) {
    TimeSignature simple(4, 4);
    TimeSignature compound(6, 8);
    TimeSignature irregular(5, 4);

    EXPECT_EQ(simple.getMetric(), METRIC::SIMPLE);
    EXPECT_EQ(simple.getMetricAsString(), "SIMPLE");

    EXPECT_EQ(compound.getMetric(), METRIC::COMPOUND);
    EXPECT_EQ(compound.getMetricAsString(), "COMPOUND");

    EXPECT_EQ(irregular.getMetric(), METRIC::IRREGULAR);
    EXPECT_EQ(irregular.getMetricAsString(), "IRREGULAR");
}

TEST(TimeSignatureIntegration, ModifyAndCheckMetricChange) {
    TimeSignature ts(4, 4);
    EXPECT_EQ(ts.getMetric(), METRIC::SIMPLE);

    ts.setUpperValue(6);
    ts.setLowerValue(8);
    EXPECT_EQ(ts.getMetric(), METRIC::COMPOUND);

    ts.setUpperValue(5);
    EXPECT_EQ(ts.getMetric(), METRIC::IRREGULAR);
}
