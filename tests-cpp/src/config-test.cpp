#include "maiacore/config.h"

#include <gtest/gtest.h>

// REVIEW ROUND 1 (task-3-review.md I4) — this suite has no fixture and no TearDown, and 10 of
// its 14 tests end on a non-default tuning system; the global tuning system is process-wide
// state (config.cpp), and tests-cpp/CMakeLists.txt links this file immediately before
// pitch-test.cpp, so whatever this suite leaves behind was silently inherited there. The fixture
// below resets EQUAL_TEMPERAMENT after every test in this suite, regardless of execution or
// --gtest_shuffle order, so no later test file (this one, or a future one -- Task 6 makes
// Note::getFrequency() read this same global) can observe a state this suite changed.
class ConfigTuningSystem : public ::testing::Test {
   protected:
    void TearDown() override { setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT); }
};

// ============================================================================
// Initial State Tests
// ============================================================================

// REVIEW ROUND 1 (task-3-review.md I4) — this used to call setTuningSystem(EQUAL_TEMPERAMENT)
// immediately before asserting it, so it could not detect a wrong default; it always passed
// regardless of what config.cpp actually initialises _tuningSystem to. The fixture's TearDown
// above now guarantees every test in this suite starts from EQUAL_TEMPERAMENT (reset by
// whichever test ran before it, or by config.cpp's own static initialiser if this is the first),
// so asserting the current value directly, with no reset of its own, is a meaningful check again.
TEST_F(ConfigTuningSystem, DefaultIsEqualTemperament) {
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);
}

// ============================================================================
// Set and Get Tests for Each Tuning System
// ============================================================================

TEST_F(ConfigTuningSystem, SetEqualTemperament) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);
}

TEST_F(ConfigTuningSystem, SetJustIntonation) {
    setTuningSystem(TuningSystem::JUST_INTONATION);
    EXPECT_EQ(getTuningSystem(), TuningSystem::JUST_INTONATION);
}

TEST_F(ConfigTuningSystem, SetPythagoreanTuning) {
    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);
}

TEST_F(ConfigTuningSystem, SetMeantoneTemperament) {
    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);
}

TEST_F(ConfigTuningSystem, SetWellTemperament) {
    setTuningSystem(TuningSystem::WELL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::WELL_TEMPERAMENT);
}

// ============================================================================
// State Persistence Tests
// ============================================================================

TEST_F(ConfigTuningSystem, StatePersistsAcrossCalls) {
    setTuningSystem(TuningSystem::JUST_INTONATION);

    TuningSystem first = getTuningSystem();
    TuningSystem second = getTuningSystem();
    TuningSystem third = getTuningSystem();

    EXPECT_EQ(first, TuningSystem::JUST_INTONATION);
    EXPECT_EQ(second, TuningSystem::JUST_INTONATION);
    EXPECT_EQ(third, TuningSystem::JUST_INTONATION);
}

TEST_F(ConfigTuningSystem, StateChangePersists) {
    setTuningSystem(TuningSystem::PYTHAGOREAN_TUNING);
    EXPECT_EQ(getTuningSystem(), TuningSystem::PYTHAGOREAN_TUNING);

    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);
}

// ============================================================================
// Multiple Changes Tests
// ============================================================================

TEST_F(ConfigTuningSystem, SequentialChanges) {
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

TEST_F(ConfigTuningSystem, SetSameValueMultipleTimes) {
    setTuningSystem(TuningSystem::JUST_INTONATION);
    setTuningSystem(TuningSystem::JUST_INTONATION);
    setTuningSystem(TuningSystem::JUST_INTONATION);

    EXPECT_EQ(getTuningSystem(), TuningSystem::JUST_INTONATION);
}

TEST_F(ConfigTuningSystem, AlternatingBetweenTwoSystems) {
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

TEST_F(ConfigTuningSystem, ResetToDefault) {
    // Change to a non-default value
    setTuningSystem(TuningSystem::WELL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::WELL_TEMPERAMENT);

    // Reset to default
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(getTuningSystem(), TuningSystem::EQUAL_TEMPERAMENT);
}

TEST_F(ConfigTuningSystem, CycleThroughAllSystems) {
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

TEST_F(ConfigTuningSystem, SetAndVerifyAllSystems) {
    // Test all tuning systems can be set and retrieved
    TuningSystem systems[] = {TuningSystem::EQUAL_TEMPERAMENT, TuningSystem::JUST_INTONATION,
                              TuningSystem::PYTHAGOREAN_TUNING, TuningSystem::MEANTONE_TEMPERAMENT,
                              TuningSystem::WELL_TEMPERAMENT};

    for (const auto& system : systems) {
        setTuningSystem(system);
        EXPECT_EQ(getTuningSystem(), system);
    }
}

TEST_F(ConfigTuningSystem, MultipleGetsSameResult) {
    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);

    // Multiple gets should return the same value
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(getTuningSystem(), TuningSystem::MEANTONE_TEMPERAMENT);
    }
}
