#include <gtest/gtest.h>

#include "maiacore/constants.h"
#include "maiacore/duration.h"

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(DurationConstructor, FromQuarterDurationDefault) {
    Duration dur(1.0f);  // One quarter note
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
    EXPECT_EQ(dur.getTicks(), 256);  // Default divisions
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 256);
}

TEST(DurationConstructor, FromQuarterDurationCustomDivisions) {
    Duration dur(1.0f, 480);  // One quarter note, 480 divisions
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
    EXPECT_EQ(dur.getTicks(), 480);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 480);
}

TEST(DurationConstructor, FromQuarterDurationHalfNote) {
    Duration dur(2.0f);  // Half note
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 2.0f);
    EXPECT_EQ(dur.getTicks(), 512);  // 2 * 256
}

TEST(DurationConstructor, FromQuarterDurationEighthNote) {
    Duration dur(0.5f);  // Eighth note
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 0.5f);
    EXPECT_EQ(dur.getTicks(), 128);  // 0.5 * 256
}

TEST(DurationConstructor, FromTicksDefault) {
    Duration dur(256);  // 256 ticks = 1 quarter note
    EXPECT_EQ(dur.getTicks(), 256);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
}

TEST(DurationConstructor, FromTicksCustomDivisions) {
    Duration dur(480, 480);  // 480 ticks with 480 divisions = 1 quarter
    EXPECT_EQ(dur.getTicks(), 480);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 480);
}

TEST(DurationConstructor, FromTicksWithTuplet) {
    Duration dur(170, 256, 3, 2);  // Triplet eighth note
    EXPECT_EQ(dur.getTicks(), 170);
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 3);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 2);
}

TEST(DurationConstructor, FromTicksWholeNote) {
    Duration dur(1024);  // Whole note (4 quarters)
    EXPECT_EQ(dur.getTicks(), 1024);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 4.0f);
}

// ============================================================================
// Rhythm Figure Tests
// ============================================================================

TEST(DurationRhythmFigure, QuarterNote) {
    Duration dur(1.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::QUARTER);
    EXPECT_EQ(dur.getNoteType(), "quarter");
}

TEST(DurationRhythmFigure, HalfNote) {
    Duration dur(2.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::HALF);
    EXPECT_EQ(dur.getNoteType(), "half");
}

TEST(DurationRhythmFigure, WholeNote) {
    Duration dur(4.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::WHOLE);
    EXPECT_EQ(dur.getNoteType(), "whole");
}

TEST(DurationRhythmFigure, EighthNote) {
    Duration dur(0.5f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::EIGHTH);
    EXPECT_EQ(dur.getNoteType(), "eighth");
}

TEST(DurationRhythmFigure, SixteenthNote) {
    Duration dur(0.25f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::N16TH);
    EXPECT_EQ(dur.getNoteType(), "16th");
}

TEST(DurationRhythmFigure, ThirtySecondNote) {
    Duration dur(0.125f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::N32ND);
    EXPECT_EQ(dur.getNoteType(), "32nd");
}

TEST(DurationRhythmFigure, SixtyFourthNote) {
    Duration dur(0.0625f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::N64TH);
    EXPECT_EQ(dur.getNoteType(), "64th");
}

TEST(DurationRhythmFigure, BreveNote) {
    Duration dur(8.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::BREVE);
    EXPECT_EQ(dur.getNoteType(), "breve");
}

// ============================================================================
// Dotted Notes Tests
// ============================================================================

TEST(DurationDots, QuarterNoteNoDots) {
    Duration dur(1.0f);
    EXPECT_EQ(dur.getDots(), 0);
}

TEST(DurationDots, DottedQuarterNote) {
    Duration dur(1.5f);  // 1 + 0.5 = dotted quarter
    EXPECT_EQ(dur.getDots(), 1);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::QUARTER);
}

TEST(DurationDots, DoubleDottedQuarterNote) {
    Duration dur(1.75f);  // 1 + 0.5 + 0.25 = double dotted quarter
    EXPECT_EQ(dur.getDots(), 2);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::QUARTER);
}

TEST(DurationDots, DottedHalfNote) {
    Duration dur(3.0f);  // 2 + 1 = dotted half
    EXPECT_EQ(dur.getDots(), 1);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::HALF);
}

TEST(DurationDots, DoubleDottedHalfNote) {
    Duration dur(3.5f);  // 2 + 1 + 0.5 = double dotted half
    EXPECT_EQ(dur.getDots(), 2);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::HALF);
}

TEST(DurationDots, DottedEighthNote) {
    Duration dur(0.75f);  // 0.5 + 0.25 = dotted eighth
    EXPECT_EQ(dur.getDots(), 1);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::EIGHTH);
}

TEST(DurationDots, DoubleDottedEighthNote) {
    Duration dur(0.875f);  // 0.5 + 0.25 + 0.125 = double dotted eighth
    EXPECT_EQ(dur.getDots(), 2);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::EIGHTH);
}

TEST(DurationDots, DottedWholeNote) {
    Duration dur(6.0f);  // 4 + 2 = dotted whole
    EXPECT_EQ(dur.getDots(), 1);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::WHOLE);
}

TEST(DurationDots, DottedSixteenthNote) {
    Duration dur(0.375f);  // 0.25 + 0.125 = dotted 16th
    EXPECT_EQ(dur.getDots(), 1);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::N16TH);
}

// ============================================================================
// Ticks and Divisions Tests
// ============================================================================

TEST(DurationTicks, QuarterNoteDefaultDivisions) {
    Duration dur(1.0f, 256);
    EXPECT_EQ(dur.getTicks(), 256);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 256);
}

TEST(DurationTicks, QuarterNote480Divisions) {
    Duration dur(1.0f, 480);
    EXPECT_EQ(dur.getTicks(), 480);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 480);
}

TEST(DurationTicks, QuarterNote768Divisions) {
    Duration dur(1.0f, 768);
    EXPECT_EQ(dur.getTicks(), 768);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 768);
}

TEST(DurationTicks, HalfNoteVarious) {
    Duration dur1(2.0f, 256);
    EXPECT_EQ(dur1.getTicks(), 512);

    Duration dur2(2.0f, 480);
    EXPECT_EQ(dur2.getTicks(), 960);
}

TEST(DurationTicks, EighthNoteVarious) {
    Duration dur1(0.5f, 256);
    EXPECT_EQ(dur1.getTicks(), 128);

    Duration dur2(0.5f, 480);
    EXPECT_EQ(dur2.getTicks(), 240);
}

TEST(DurationTicks, FromTicksPreservesValue) {
    Duration dur(384, 256);  // Some arbitrary tick value
    EXPECT_EQ(dur.getTicks(), 384);
}

// ============================================================================
// Fraction Duration Tests
// ============================================================================

TEST(DurationFraction, QuarterNoteFraction) {
    Duration dur(1.0f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "1/1");  // Represents quarter notes
}

TEST(DurationFraction, HalfNoteFraction) {
    Duration dur(2.0f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "2/1");  // 2 quarter notes
}

TEST(DurationFraction, WholeNoteFraction) {
    Duration dur(4.0f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "4/1");  // 4 quarter notes
}

TEST(DurationFraction, EighthNoteFraction) {
    Duration dur(0.5f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "1/2");  // 1/2 quarter note
}

TEST(DurationFraction, SixteenthNoteFraction) {
    Duration dur(0.25f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "1/4");  // 1/4 quarter note
}

TEST(DurationFraction, DottedQuarterFraction) {
    Duration dur(1.5f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "3/2");  // 3/2 quarter notes
}

TEST(DurationFraction, DottedHalfFraction) {
    Duration dur(3.0f);
    EXPECT_EQ(dur.getFractionDurationAsString(), "3/1");  // 3 quarter notes
}

TEST(DurationFraction, FractionObjectReference) {
    Duration dur(1.0f);
    const Fraction& frac = dur.getFractionDuration();
    EXPECT_EQ(frac.toString(), "1/1");  // 1 quarter note
}

// ============================================================================
// Quarter Duration Tests
// ============================================================================

TEST(DurationQuarter, GetQuarterDurationVariousNotes) {
    EXPECT_FLOAT_EQ(Duration(1.0f).getQuarterDuration(), 1.0f);
    EXPECT_FLOAT_EQ(Duration(2.0f).getQuarterDuration(), 2.0f);
    EXPECT_FLOAT_EQ(Duration(0.5f).getQuarterDuration(), 0.5f);
    EXPECT_FLOAT_EQ(Duration(0.25f).getQuarterDuration(), 0.25f);
    EXPECT_FLOAT_EQ(Duration(4.0f).getQuarterDuration(), 4.0f);
}

TEST(DurationQuarter, SetQuarterDuration) {
    Duration dur(1.0f);
    dur.setQuarterDuration(2.0f);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 2.0f);
    EXPECT_EQ(dur.getTicks(), 512);
}

TEST(DurationQuarter, SetQuarterDurationWithCustomDivisions) {
    Duration dur(1.0f, 480);
    dur.setQuarterDuration(2.0f, 480);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 2.0f);
    EXPECT_EQ(dur.getTicks(), 960);
}

TEST(DurationQuarter, SetQuarterDurationChangesRhythmFigure) {
    Duration dur(1.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::QUARTER);

    dur.setQuarterDuration(2.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::HALF);
}

TEST(DurationQuarter, SetQuarterDurationChangesDots) {
    Duration dur(1.0f);
    EXPECT_EQ(dur.getDots(), 0);

    dur.setQuarterDuration(1.5f);
    EXPECT_EQ(dur.getDots(), 1);
}

// ============================================================================
// Tuplet Tests
// ============================================================================

TEST(DurationTuplet, NotTupletByDefault) {
    Duration dur(1.0f);
    EXPECT_FALSE(dur.isTuplet());
}

TEST(DurationTuplet, TripletConstructor) {
    Duration dur(170, 256, 3, 2);  // Triplet eighth note
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 3);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 2);
}

TEST(DurationTuplet, QuintupletConstructor) {
    Duration dur(102, 256, 5, 4);  // Quintuplet 16th note
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 5);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 4);
}

TEST(DurationTuplet, SetTupleValues) {
    Duration dur(1.0f);
    EXPECT_FALSE(dur.isTuplet());

    dur.setTupleValues(3, 2, "eighth");
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 3);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 2);
    EXPECT_EQ(dur.getTimeModificationNormalType(), "eighth");
}

TEST(DurationTuplet, SetTupleValuesQuarter) {
    Duration dur(2.0f);
    dur.setTupleValues(3, 2, "quarter");
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationNormalType(), "quarter");
}

TEST(DurationTuplet, SetTupleValuesSixteenth) {
    Duration dur(0.25f);
    dur.setTupleValues(5, 4, "16th");
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 5);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 4);
    EXPECT_EQ(dur.getTimeModificationNormalType(), "16th");
}

TEST(DurationTuplet, TripletQuarterNotes) {
    Duration dur(170, 256, 3, 2);
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 3);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 2);
}

TEST(DurationTuplet, SeptupletConstructor) {
    Duration dur(73, 256, 7, 4);  // Septuplet 16th note
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationActualNotes(), 7);
    EXPECT_EQ(dur.getTimeModificationNormalNotes(), 4);
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

TEST(DurationEdgeCases, VeryShortDuration) {
    Duration dur(0.03125f);  // 128th note
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::N128TH);
    EXPECT_EQ(dur.getNoteType(), "128th");
}

TEST(DurationEdgeCases, VeryLongDuration) {
    Duration dur(8.0f);  // Breve
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::BREVE);
}

TEST(DurationEdgeCases, LargeDivisionsPerQuarter) {
    Duration dur(1.0f, 1920);  // Very high resolution
    EXPECT_EQ(dur.getTicks(), 1920);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 1920);
}

TEST(DurationEdgeCases, SmallDivisionsPerQuarter) {
    Duration dur(1.0f, 64);
    EXPECT_EQ(dur.getTicks(), 64);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 64);
}

TEST(DurationEdgeCases, TicksFromConstructor) {
    Duration dur(100, 256);  // Arbitrary tick count
    EXPECT_EQ(dur.getTicks(), 100);
}

TEST(DurationEdgeCases, VerySmallTicks) {
    Duration dur(1, 256);  // Smallest non-zero duration
    EXPECT_EQ(dur.getTicks(), 1);
    EXPECT_GT(dur.getQuarterDuration(), 0.0f);
}

// ============================================================================
// Conversion Tests
// ============================================================================

TEST(DurationConversion, TicksToQuarter256) {
    Duration dur(256);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
}

TEST(DurationConversion, TicksToQuarter480) {
    Duration dur(480, 480);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
}

TEST(DurationConversion, QuarterToTicks256) {
    Duration dur(1.0f, 256);
    EXPECT_EQ(dur.getTicks(), 256);
}

TEST(DurationConversion, QuarterToTicks480) {
    Duration dur(1.0f, 480);
    EXPECT_EQ(dur.getTicks(), 480);
}

TEST(DurationConversion, HalfToTicks) {
    Duration dur1(2.0f, 256);
    EXPECT_EQ(dur1.getTicks(), 512);

    Duration dur2(2.0f, 480);
    EXPECT_EQ(dur2.getTicks(), 960);
}

TEST(DurationConversion, EighthToTicks) {
    Duration dur1(0.5f, 256);
    EXPECT_EQ(dur1.getTicks(), 128);

    Duration dur2(0.5f, 480);
    EXPECT_EQ(dur2.getTicks(), 240);
}

// ============================================================================
// Complex Rhythm Tests
// ============================================================================

TEST(DurationComplexRhythm, DottedQuarterPlusDottedEighth) {
    Duration dotted_quarter(1.5f);
    Duration dotted_eighth(0.75f);

    EXPECT_EQ(dotted_quarter.getDots(), 1);
    EXPECT_EQ(dotted_eighth.getDots(), 1);

    float total = dotted_quarter.getQuarterDuration() + dotted_eighth.getQuarterDuration();
    EXPECT_FLOAT_EQ(total, 2.25f);
}

TEST(DurationComplexRhythm, TripletEighthNotes) {
    Duration triplet(170, 256, 3, 2);
    EXPECT_TRUE(triplet.isTuplet());

    // Three triplet eighths should equal one quarter
    float three_triplets = triplet.getQuarterDuration() * 3;
    EXPECT_NEAR(three_triplets, 2.0f, 0.1f);  // Approximately 2 quarters
}

TEST(DurationComplexRhythm, MixedDurations) {
    Duration whole(4.0f);
    Duration half(2.0f);
    Duration quarter(1.0f);
    Duration eighth(0.5f);

    float total = whole.getQuarterDuration() + half.getQuarterDuration() +
                  quarter.getQuarterDuration() + eighth.getQuarterDuration();
    EXPECT_FLOAT_EQ(total, 7.5f);
}

TEST(DurationComplexRhythm, AllBasicDurations) {
    std::vector<float> durations = {
        8.0f,   // Breve
        4.0f,   // Whole
        2.0f,   // Half
        1.0f,   // Quarter
        0.5f,   // Eighth
        0.25f,  // 16th
        0.125f, // 32nd
        0.0625f // 64th
    };

    for (float dur_val : durations) {
        Duration dur(dur_val);
        EXPECT_FLOAT_EQ(dur.getQuarterDuration(), dur_val);
    }
}

// ============================================================================
// Note Type String Tests
// ============================================================================

TEST(DurationNoteType, AllBasicNoteTypes) {
    EXPECT_EQ(Duration(8.0f).getNoteType(), "breve");
    EXPECT_EQ(Duration(4.0f).getNoteType(), "whole");
    EXPECT_EQ(Duration(2.0f).getNoteType(), "half");
    EXPECT_EQ(Duration(1.0f).getNoteType(), "quarter");
    EXPECT_EQ(Duration(0.5f).getNoteType(), "eighth");
    EXPECT_EQ(Duration(0.25f).getNoteType(), "16th");
    EXPECT_EQ(Duration(0.125f).getNoteType(), "32nd");
    EXPECT_EQ(Duration(0.0625f).getNoteType(), "64th");
}

TEST(DurationNoteType, DottedNotesTypeUnchanged) {
    Duration dotted_quarter(1.5f);
    EXPECT_EQ(dotted_quarter.getNoteType(), "quarter");
    EXPECT_EQ(dotted_quarter.getDots(), 1);
}

TEST(DurationNoteType, DoubleDottedNotesTypeUnchanged) {
    Duration double_dotted_half(3.5f);
    EXPECT_EQ(double_dotted_half.getNoteType(), "half");
    EXPECT_EQ(double_dotted_half.getDots(), 2);
}

// ============================================================================
// Divisions Per Quarter Note Tests
// ============================================================================

TEST(DurationDivisions, StandardDivisions) {
    std::vector<int> standard_divisions = {64, 128, 256, 480, 768, 960, 1920};

    for (int div : standard_divisions) {
        Duration dur(1.0f, div);
        EXPECT_EQ(dur.getDivisionsPerQuarterNote(), div);
        EXPECT_EQ(dur.getTicks(), div);
    }
}

TEST(DurationDivisions, DivisionsAffectTickCount) {
    Duration dur1(2.0f, 256);
    Duration dur2(2.0f, 480);

    EXPECT_EQ(dur1.getTicks(), 512);  // 2 * 256
    EXPECT_EQ(dur2.getTicks(), 960);  // 2 * 480

    // Both represent the same quarter duration
    EXPECT_FLOAT_EQ(dur1.getQuarterDuration(), dur2.getQuarterDuration());
}

TEST(DurationDivisions, SetQuarterUpdatesTicksOnly) {
    Duration dur(1.0f, 256);
    EXPECT_EQ(dur.getTicks(), 256);
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 256);

    // setQuarterDuration updates ticks based on new divisions, but doesn't change stored divisions
    dur.setQuarterDuration(1.0f, 480);
    EXPECT_EQ(dur.getTicks(), 480);
    // Divisions per quarter note remains at original value
    EXPECT_EQ(dur.getDivisionsPerQuarterNote(), 256);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(DurationIntegration, CreateModifyCheck) {
    Duration dur(1.0f, 256);

    // Initial state
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::QUARTER);
    EXPECT_EQ(dur.getTicks(), 256);
    EXPECT_FLOAT_EQ(dur.getQuarterDuration(), 1.0f);
    EXPECT_EQ(dur.getDots(), 0);
    EXPECT_FALSE(dur.isTuplet());

    // Modify to dotted half
    dur.setQuarterDuration(3.0f);
    EXPECT_EQ(dur.getRhythmFigure(), RhythmFigure::HALF);
    EXPECT_EQ(dur.getDots(), 1);
    EXPECT_EQ(dur.getTicks(), 768);

    // Add tuplet
    dur.setTupleValues(3, 2, "half");
    EXPECT_TRUE(dur.isTuplet());
    EXPECT_EQ(dur.getTimeModificationNormalType(), "half");
}

TEST(DurationIntegration, FractionConsistency) {
    Duration dur(1.5f);  // Dotted quarter

    const Fraction& frac = dur.getFractionDuration();
    std::string frac_str = dur.getFractionDurationAsString();

    EXPECT_EQ(frac.toString(), frac_str);
    EXPECT_EQ(frac_str, "3/2");  // 3/2 quarter notes
    EXPECT_FLOAT_EQ(frac.getFloatValue(), 1.5f);  // 1.5 quarter notes
}

TEST(DurationIntegration, TicksAndQuarterConsistency) {
    std::vector<std::pair<float, int>> test_cases = {
        {1.0f, 256},
        {2.0f, 512},
        {0.5f, 128},
        {4.0f, 1024},
        {0.25f, 64}
    };

    for (const auto& test : test_cases) {
        Duration dur(test.first, 256);
        EXPECT_EQ(dur.getTicks(), test.second);
        EXPECT_FLOAT_EQ(dur.getQuarterDuration(), test.first);
    }
}
