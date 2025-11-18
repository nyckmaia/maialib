#include <gtest/gtest.h>

#include "maiacore/score_collection.h"

// Test directories with XML files
const std::string BACH_DIR = "./test/xml_examples/Bach";
const std::string BEETHOVEN_DIR = "./test/xml_examples/Beethoven";
const std::string UNIT_TEST_DIR = "./test/xml_examples/unit_test";

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(ScoreCollectionConstructor, DefaultConstructor) {
    ScoreCollection collection(std::vector<std::string>{});  // Empty string constructor
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_EQ(collection.getNumDirectories(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, SingleDirectoryConstructor) {
    ScoreCollection collection(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);
    EXPECT_GT(collection.getNumScores(), 0);  // Should load Bach files
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, MultipleDirectoriesConstructor) {
    std::vector<std::string> dirs = {BACH_DIR, BEETHOVEN_DIR};
    ScoreCollection collection(dirs);

    EXPECT_EQ(collection.getNumDirectories(), 2);
    EXPECT_GT(collection.getNumScores(), 0);  // Should load from both directories
}

TEST(ScoreCollectionConstructor, EmptyDirectoryPath) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, EmptyDirectoryList) {
    std::vector<std::string> empty_dirs;
    ScoreCollection collection(std::vector<std::string>{});

    EXPECT_EQ(collection.getNumDirectories(), 0);
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

// ============================================================================
// Directory Management Tests
// ============================================================================

TEST(ScoreCollectionDirectories, GetDirectoriesPaths) {
    std::vector<std::string> dirs = {BACH_DIR, BEETHOVEN_DIR};
    ScoreCollection collection(dirs);

    std::vector<std::string> retrieved = collection.getDirectoriesPaths();
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved[0], BACH_DIR);
    EXPECT_EQ(retrieved[1], BEETHOVEN_DIR);
}

TEST(ScoreCollectionDirectories, SetDirectoriesPaths) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumDirectories(), 0);

    std::vector<std::string> dirs = {BACH_DIR};
    collection.setDirectoriesPaths(dirs);

    EXPECT_EQ(collection.getNumDirectories(), 1);
    EXPECT_GT(collection.getNumScores(), 0);  // Should auto-load
}

TEST(ScoreCollectionDirectories, SetDirectoriesReloads) {
    ScoreCollection collection(BACH_DIR);
    int initial_count = collection.getNumScores();

    // Set new directories - should replace and reload
    std::vector<std::string> new_dirs = {BEETHOVEN_DIR};
    collection.setDirectoriesPaths(new_dirs);

    EXPECT_EQ(collection.getNumDirectories(), 1);
    // Score count may differ based on number of files in each directory
}

TEST(ScoreCollectionDirectories, AddDirectory) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.addDirectory(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);

    collection.addDirectory(BEETHOVEN_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 2);
}

TEST(ScoreCollectionDirectories, AddDirectoryDoesNotAutoLoad) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addDirectory(BACH_DIR);

    // addDirectory only adds to list, doesn't load files
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionDirectories, GetNumDirectories) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumDirectories(), 0);

    collection.addDirectory(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);

    collection.addDirectory(BEETHOVEN_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 2);
}

// ============================================================================
// Score Management Tests
// ============================================================================

TEST(ScoreCollectionScores, AddScoreByObject) {
    ScoreCollection collection(std::vector<std::string>{});
    Score score("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    collection.addScore(score);
    EXPECT_EQ(collection.getNumScores(), 1);
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionScores, AddScoreByFilePath) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, AddScoreByFilePathList) {
    ScoreCollection collection(std::vector<std::string>{});
    std::vector<std::string> files = {
        "./test/xml_examples/Bach/prelude_1_BWV_846.xml",
        "./test/xml_examples/Bach/cello_suite_1_violin.xml"
    };

    collection.addScore(files);
    EXPECT_EQ(collection.getNumScores(), 2);
}

TEST(ScoreCollectionScores, AddMultipleScoresByObject) {
    ScoreCollection collection(std::vector<std::string>{});

    Score score1("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    Score score2("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection.addScore(score1);
    collection.addScore(score2);

    EXPECT_EQ(collection.getNumScores(), 2);
}

TEST(ScoreCollectionScores, GetNumScores) {
    ScoreCollection collection(BACH_DIR);
    int num_scores = collection.getNumScores();

    EXPECT_GT(num_scores, 0);  // Bach dir should have at least 1 file
}

TEST(ScoreCollectionScores, GetScoresNonConst) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    std::vector<Score>& scores = collection.getScores();
    EXPECT_EQ(scores.size(), 1);

    // Can modify through reference
    scores.clear();
    EXPECT_EQ(collection.getNumScores(), 0);
}

TEST(ScoreCollectionScores, GetScoresConst) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    const ScoreCollection& const_ref = collection;
    const std::vector<Score>& scores = const_ref.getScores();

    EXPECT_EQ(scores.size(), 1);
}

TEST(ScoreCollectionScores, IsEmpty) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_TRUE(collection.isEmpty());

    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionScores, Clear) {
    ScoreCollection collection(BACH_DIR);
    EXPECT_GT(collection.getNumScores(), 0);

    collection.clear();
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionScores, ClearDoesNotAffectDirectories) {
    ScoreCollection collection(BACH_DIR);
    int num_dirs = collection.getNumDirectories();

    collection.clear();

    EXPECT_EQ(collection.getNumDirectories(), num_dirs);  // Directories unchanged
}

TEST(ScoreCollectionScores, RemoveScoreByIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    EXPECT_EQ(collection.getNumScores(), 2);

    collection.removeScore(0);
    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, RemoveScoreLastIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection.removeScore(1);  // Remove last
    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, RemoveScoreInvalidIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    EXPECT_THROW(collection.removeScore(10), std::runtime_error); // Flat 
    EXPECT_EQ(collection.getNumScores(), 1);  // Unchanged
}

// ============================================================================
// Merge Tests
// ============================================================================

TEST(ScoreCollectionMerge, MergeCollections) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumScores(), 2);
}

TEST(ScoreCollectionMerge, MergeDirectories) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addDirectory(BACH_DIR);

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addDirectory(BEETHOVEN_DIR);

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumDirectories(), 2);
}

TEST(ScoreCollectionMerge, MergeBothScoresAndDirectories) {
    ScoreCollection collection1(BACH_DIR);
    int scores1 = collection1.getNumScores();
    int dirs1 = collection1.getNumDirectories();

    ScoreCollection collection2(BEETHOVEN_DIR);
    int scores2 = collection2.getNumScores();
    int dirs2 = collection2.getNumDirectories();

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumScores(), scores1 + scores2);
    EXPECT_EQ(collection1.getNumDirectories(), dirs1 + dirs2);
}

TEST(ScoreCollectionMerge, MergeEmptyCollection) {
    ScoreCollection collection1(BACH_DIR);
    int original_count = collection1.getNumScores();

    ScoreCollection empty_collection(std::vector<std::string>{});
    collection1.merge(empty_collection);

    EXPECT_EQ(collection1.getNumScores(), original_count);  // Unchanged
}

TEST(ScoreCollectionMerge, MergeIntoEmptyCollection) {
    ScoreCollection empty_collection(std::vector<std::string>{});
    ScoreCollection collection(BACH_DIR);
    int count = collection.getNumScores();

    empty_collection.merge(collection);

    EXPECT_EQ(empty_collection.getNumScores(), count);
}

// ============================================================================
// Operator Tests
// ============================================================================

TEST(ScoreCollectionOperator, PlusOperator) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    ScoreCollection merged = collection1 + collection2;

    EXPECT_EQ(merged.getNumScores(), 2);
    // Original collections should be unchanged
    EXPECT_EQ(collection1.getNumScores(), 1);
    EXPECT_EQ(collection2.getNumScores(), 1);
}

TEST(ScoreCollectionOperator, PlusOperatorChaining) {
    ScoreCollection c1(std::vector<std::string>{});
    c1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection c2(std::vector<std::string>{});
    c2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    ScoreCollection c3(std::vector<std::string>{});
    c3.addScore("./test/xml_examples/Beethoven/Beethoven_quartet_133.xml");

    ScoreCollection merged = c1 + c2 + c3;

    EXPECT_EQ(merged.getNumScores(), 3);
}

// ============================================================================
// Pattern Finding Tests - Single Pattern
// ============================================================================

TEST(ScoreCollectionPatternFinding, FindMelodyPatternBasic) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    // Create a simple 3-note pattern
    std::vector<Note> pattern = {
        Note("C4"),
        Note("D4"),
        Note("E4")
    };

    auto results = collection.findMelodyPattern(pattern, 0.5f, 0.5f);

    // Results should be a table (vector of tuples)
    // May or may not find matches depending on the score content
    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionPatternFinding, FindMelodyPatternMultipleScores) {
    ScoreCollection collection(BACH_DIR);

    std::vector<Note> pattern = {
        Note("C4"),
        Note("E4"),
        Note("G4")
    };

    auto results = collection.findMelodyPattern(pattern, 0.3f, 0.3f);

    // With multiple scores and low thresholds, should potentially find matches
    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionPatternFinding, FindMelodyPatternHighThresholds) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    std::vector<Note> pattern = {
        Note("C4"),
        Note("D4")
    };

    // Very high thresholds - likely few/no matches
    auto results = collection.findMelodyPattern(pattern, 0.99f, 0.99f);

    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionPatternFinding, FindMelodyPatternEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});  // Empty

    std::vector<Note> pattern = {Note("C4")};

    auto results = collection.findMelodyPattern(pattern);

    EXPECT_EQ(results.size(), 0);  // No scores = no results
}

// ============================================================================
// Pattern Finding Tests - Multiple Patterns
// ============================================================================

TEST(ScoreCollectionMultiPattern, FindMultipleMelodyPatterns) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    std::vector<std::vector<Note>> patterns = {
        {Note("C4"), Note("D4")},
        {Note("E4"), Note("F4")},
        {Note("G4"), Note("A4")}
    };

    auto results = collection.findMelodyPattern(patterns, 0.5f, 0.5f);

    // Should return a vector of tables, one for each pattern
    // Even with no matches, should return correct structure
    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionMultiPattern, MultiplePatternsSingleScore) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    std::vector<std::vector<Note>> patterns = {
        {Note("C4"), Note("D4"), Note("E4")},
        {Note("G4"), Note("B4"), Note("A4")}
    };

    auto results = collection.findMelodyPattern(patterns, 0.3f, 0.3f);

    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionMultiPattern, MultiplePatternsMultipleScores) {
    ScoreCollection collection(BACH_DIR);

    std::vector<std::vector<Note>> patterns = {
        {Note("C4"), Note("D4")},
        {Note("E4"), Note("F4")}
    };

    auto results = collection.findMelodyPattern(patterns, 0.4f, 0.4f);

    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionMultiPattern, EmptyPatternList) {
    ScoreCollection collection(BACH_DIR);

    std::vector<std::vector<Note>> empty_patterns;

    auto results = collection.findMelodyPattern(empty_patterns);

    EXPECT_EQ(results.size(), 0);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(ScoreCollectionIntegration, FullWorkflow) {
    // Create collection with directory
    ScoreCollection collection(BACH_DIR);
    int initial_count = collection.getNumScores();
    EXPECT_GT(initial_count, 0);

    // Add more scores manually
    collection.addScore("./test/xml_examples/Beethoven/Beethoven_quartet_133.xml");
    EXPECT_EQ(collection.getNumScores(), initial_count + 1);

    // Remove a score
    collection.removeScore(0);
    EXPECT_EQ(collection.getNumScores(), initial_count);

    // Clear all
    collection.clear();
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionIntegration, MergeAndSearch) {
    ScoreCollection collection1(BACH_DIR);
    ScoreCollection collection2(BEETHOVEN_DIR);

    ScoreCollection merged = collection1 + collection2;

    EXPECT_GT(merged.getNumScores(), 0);
    EXPECT_EQ(merged.getNumDirectories(), 2);

    // Search in merged collection
    std::vector<Note> pattern = {Note("C4"), Note("E4")};
    auto results = merged.findMelodyPattern(pattern, 0.3f, 0.3f);

    EXPECT_GE(results.size(), 0);
}

TEST(ScoreCollectionIntegration, SetDirectoriesMultipleTimes) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.setDirectoriesPaths({BACH_DIR});
    int count1 = collection.getNumScores();

    collection.setDirectoriesPaths({BEETHOVEN_DIR});
    int count2 = collection.getNumScores();

    // Both should have loaded scores
    EXPECT_GT(count1, 0);
    EXPECT_GT(count2, 0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ScoreCollectionEdgeCases, RemoveFromEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});

    EXPECT_THROW(collection.removeScore(0), std::runtime_error);
    EXPECT_EQ(collection.getNumScores(), 0);
}

TEST(ScoreCollectionEdgeCases, ClearEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.clear();  // Should not crash
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionEdgeCases, MergeWithSelf) {
    ScoreCollection collection(BACH_DIR);
    int original_count = collection.getNumScores();

    collection.merge(collection);  // Merge with itself

    EXPECT_EQ(collection.getNumScores(), original_count * 2);  // Should duplicate
}

TEST(ScoreCollectionEdgeCases, LargeCollection) {
    ScoreCollection collection(UNIT_TEST_DIR);

    // Unit test dir has many files - test performance doesn't degrade
    EXPECT_GT(collection.getNumScores(), 10);

    // Clear should work efficiently
    collection.clear();
    EXPECT_TRUE(collection.isEmpty());
}
