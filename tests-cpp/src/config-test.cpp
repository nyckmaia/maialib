#include <gtest/gtest.h>

#include "maiacore/config.h"

// ============================================================================
// Initial State Tests
// ============================================================================

TEST(ConfigTuningSystem, DefaultIsEqualTemperament) {
    // Reset to default before checking
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);

    TuningSystem current = getTuningSystem();
    EXPECT_EQ(current, TuningSystem::EQUAL_TEMPERAMENT);
}

// ============================================================================
// Set and Get Tests for Each Tuning System
// ============================================================================

TEST(ConfigTuningSystem, SetEqualTemperament) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);
}

TEST(ConfigTuningSystem, SetJustIntonation) {
    setTuningSystem(TuningSystem::JUST_INTONATION);
    EXPECT_EQ(getTuningSystem(), TuningSystem::JUST_INTONATION);
}

TEST(ConfigTuningSystem, SetPythagoreanTuning) {
    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);
}

TEST(ConfigTuningSystem, SetMeantoneTemperament) {
    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);
}

TEST(ConfigTuningSystem, SetWellTemperament) {
    setTuningSystem(TuningSystem::WELL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::WELL_TEMPERAMENT);
}

// ============================================================================
// State Persistence Tests
// ============================================================================

TEST(ConfigTuningSystem, StatePersistsAcrossCalls) {
    setTuningSystem(TuningSystem::JUST_INTONATION);

    TuningSystem first = getTuningSystem();
    TuningSystem second = getTuningSystem();
    TuningSystem third = getTuningSystem();

    EXPECT_EQ(first, TuningSystem::JUST_INTONATION);
    EXPECT_EQ(second, TuningSystem::JUST_INTONATION);
    EXPECT_EQ(third, TuningSystem::JUST_INTONATION);
}

TEST(ConfigTuningSystem, StateChangePersists) {
    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);

    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);
}

// ============================================================================
// Multiple Changes Tests
// ============================================================================

TEST(ConfigTuningSystem, SequentialChanges) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);

    setTuningSystem(TuningSystem::JUST_INTONATION);
    EXPECT_EQ(getTuningSystem(), TuningSystem::JUST_INTONATION);

    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);

    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);

    setTuningSystem(TuningSystem::WELL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::WELL_TEMPERAMENT);
}

TEST(ConfigTuningSystem, SetSameValueMultipleTimes) {
    setTuningSystem(TuningSystem::JUST_INTONATION);
    setTuningSystem(TuningSystem::JUST_INTONATION);
    setTuningSystem(TuningSystem::JUST_INTONATION);

    EXPECT_EQ(getTuningSystem(), TuningSystem::JUST_INTONATION);
}

TEST(ConfigTuningSystem, AlternatingBetweenTwoSystems) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);

    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);

    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);

    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ConfigTuningSystem, ResetToDefault) {
    // Change to a non-default value
    setTuningSystem(TuningSystem::WELL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::WELL_TEMPERAMENT);

    // Reset to default
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);
}

TEST(ConfigTuningSystem, CycleThroughAllSystems) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    TuningSystem ts1 = getTuningSystem();

    setTuningSystem(TuningSystem::JUST_INTONATION);
    TuningSystem ts2 = getTuningSystem();

    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    TuningSystem ts3 = getTuningSystem();

    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    TuningSystem ts4 = getTuningSystem();

    setTuningSystem(TuningSystem::WELL_TEMPERAMENT);
    TuningSystem ts5 = getTuningSystem();

    // Verify all are different
    EXPECT_EQ(ts1, TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(ts2, TuningSystem::JUST_INTONATION);
    EXPECT_EQ(ts3, TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(ts4, TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_EQ(ts5, TuningSystem::WELL_TEMPERAMENT);

    EXPECT_NE(ts1, ts2);
    EXPECT_NE(ts2, ts3);
    EXPECT_NE(ts3, ts4);
    EXPECT_NE(ts4, ts5);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(ConfigTuningSystem, SetAndVerifyAllSystems) {
    // Test all tuning systems can be set and retrieved
    TuningSystem systems[] = {
        TuningSystem::EQUAL_TEMPERAMENT,
        TuningSystem::JUST_INTONATION,
        TuningSystem::PYTHAGOREAN_TUNING,
        TuningSystem::MEANTONE_TEMPERAMENT,
        TuningSystem::WELL_TEMPERAMENT
    };

    for (const auto& system : systems) {
        setTuningSystem(system);
        EXPECT_EQ(getTuningSystem(), system);
    }
}

TEST(ConfigTuningSystem, MultipleGetsSameResult) {
    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);

    // Multiple gets should return the same value
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);
    }
}
