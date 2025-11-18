#include <gtest/gtest.h>

#include "maiacore/key.h"

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(KeyConstructor, DefaultConstructor) {
    Key key;
    EXPECT_EQ(key.getFifthCircle(), 0);  // C major / A minor
    EXPECT_EQ(key.isMajorMode(), 1);     // Major mode
    EXPECT_EQ(key.getName(), "C");
}

TEST(KeyConstructor, ConstructorWithFifthCircleAndMode) {
    Key key(1, true);  // G major
    EXPECT_EQ(key.getFifthCircle(), 1);
    EXPECT_EQ(key.isMajorMode(), 1);
    EXPECT_EQ(key.getName(), "G");
}

TEST(KeyConstructor, ConstructorMinorMode) {
    Key key(0, false);  // A minor
    EXPECT_EQ(key.getFifthCircle(), 0);
    EXPECT_EQ(key.isMajorMode(), 0);
    EXPECT_EQ(key.getName(), "Am");
}

TEST(KeyConstructor, ConstructorFromKeyNameMajor) {
    Key key("C");
    EXPECT_EQ(key.getFifthCircle(), 0);
    EXPECT_EQ(key.isMajorMode(), 1);
    EXPECT_EQ(key.getName(), "C");
}

TEST(KeyConstructor, ConstructorFromKeyNameMinor) {
    Key key("Am");
    EXPECT_EQ(key.getFifthCircle(), 0);
    EXPECT_EQ(key.isMajorMode(), 0);
    EXPECT_EQ(key.getName(), "Am");
}

TEST(KeyConstructor, ConstructorFromKeyNameGMajor) {
    Key key("G");
    EXPECT_EQ(key.getFifthCircle(), 1);
    EXPECT_EQ(key.isMajorMode(), 1);
}

TEST(KeyConstructor, ConstructorFromKeyNameFSharpMinor) {
    Key key("F#m");
    EXPECT_EQ(key.getFifthCircle(), 3);
    EXPECT_EQ(key.isMajorMode(), 0);
}

// ============================================================================
// Fifth Circle Tests - Sharp Keys
// ============================================================================

TEST(KeyFifthCircle, CMajorNoSharps) {
    Key key(0, true);
    EXPECT_EQ(key.getFifthCircle(), 0);
    EXPECT_EQ(key.getName(), "C");
}

TEST(KeyFifthCircle, GMajorOneSharp) {
    Key key(1, true);
    EXPECT_EQ(key.getFifthCircle(), 1);
    EXPECT_EQ(key.getName(), "G");
}

TEST(KeyFifthCircle, DMajorTwoSharps) {
    Key key(2, true);
    EXPECT_EQ(key.getFifthCircle(), 2);
    EXPECT_EQ(key.getName(), "D");
}

TEST(KeyFifthCircle, AMajorThreeSharps) {
    Key key(3, true);
    EXPECT_EQ(key.getFifthCircle(), 3);
    EXPECT_EQ(key.getName(), "A");
}

TEST(KeyFifthCircle, EMajorFourSharps) {
    Key key(4, true);
    EXPECT_EQ(key.getFifthCircle(), 4);
    EXPECT_EQ(key.getName(), "E");
}

TEST(KeyFifthCircle, BMajorFiveSharps) {
    Key key(5, true);
    EXPECT_EQ(key.getFifthCircle(), 5);
    EXPECT_EQ(key.getName(), "B");
}

TEST(KeyFifthCircle, FSharpMajorSixSharps) {
    Key key(6, true);
    EXPECT_EQ(key.getFifthCircle(), 6);
    EXPECT_EQ(key.getName(), "F#");
}

TEST(KeyFifthCircle, CSharpMajorSevenSharps) {
    Key key(7, true);
    EXPECT_EQ(key.getFifthCircle(), 7);
    EXPECT_EQ(key.getName(), "C#");
}

// ============================================================================
// Fifth Circle Tests - Flat Keys
// ============================================================================

TEST(KeyFifthCircle, FMajorOneFlat) {
    Key key(-1, true);
    EXPECT_EQ(key.getFifthCircle(), -1);
    EXPECT_EQ(key.getName(), "F");
}

TEST(KeyFifthCircle, BFlatMajorTwoFlats) {
    Key key(-2, true);
    EXPECT_EQ(key.getFifthCircle(), -2);
    EXPECT_EQ(key.getName(), "Bb");
}

TEST(KeyFifthCircle, EFlatMajorThreeFlats) {
    Key key(-3, true);
    EXPECT_EQ(key.getFifthCircle(), -3);
    EXPECT_EQ(key.getName(), "Eb");
}

TEST(KeyFifthCircle, AFlatMajorFourFlats) {
    Key key(-4, true);
    EXPECT_EQ(key.getFifthCircle(), -4);
    EXPECT_EQ(key.getName(), "Ab");
}

TEST(KeyFifthCircle, DFlatMajorFiveFlats) {
    Key key(-5, true);
    EXPECT_EQ(key.getFifthCircle(), -5);
    EXPECT_EQ(key.getName(), "Db");
}

TEST(KeyFifthCircle, GFlatMajorSixFlats) {
    Key key(-6, true);
    EXPECT_EQ(key.getFifthCircle(), -6);
    EXPECT_EQ(key.getName(), "Gb");
}

// ============================================================================
// Fifth Circle Tests - Minor Keys
// ============================================================================

TEST(KeyFifthCircleMinor, AMinorNoSharps) {
    Key key(0, false);
    EXPECT_EQ(key.getFifthCircle(), 0);
    EXPECT_EQ(key.getName(), "Am");
}

TEST(KeyFifthCircleMinor, EMinorOneSharp) {
    Key key(1, false);
    EXPECT_EQ(key.getFifthCircle(), 1);
    EXPECT_EQ(key.getName(), "Em");
}

TEST(KeyFifthCircleMinor, BMinorTwoSharps) {
    Key key(2, false);
    EXPECT_EQ(key.getFifthCircle(), 2);
    EXPECT_EQ(key.getName(), "Bm");
}

TEST(KeyFifthCircleMinor, FSharpMinorThreeSharps) {
    Key key(3, false);
    EXPECT_EQ(key.getFifthCircle(), 3);
    EXPECT_EQ(key.getName(), "F#m");
}

TEST(KeyFifthCircleMinor, CSharpMinorFourSharps) {
    Key key(4, false);
    EXPECT_EQ(key.getFifthCircle(), 4);
    EXPECT_EQ(key.getName(), "C#m");
}

TEST(KeyFifthCircleMinor, GSharpMinorFiveSharps) {
    Key key(5, false);
    EXPECT_EQ(key.getFifthCircle(), 5);
    EXPECT_EQ(key.getName(), "G#m");
}

TEST(KeyFifthCircleMinor, DSharpMinorSixSharps) {
    Key key(6, false);
    EXPECT_EQ(key.getFifthCircle(), 6);
    EXPECT_EQ(key.getName(), "D#m");
}

TEST(KeyFifthCircleMinor, DMinorOneFlat) {
    Key key(-1, false);
    EXPECT_EQ(key.getFifthCircle(), -1);
    EXPECT_EQ(key.getName(), "Dm");
}

TEST(KeyFifthCircleMinor, GMinorTwoFlats) {
    Key key(-2, false);
    EXPECT_EQ(key.getFifthCircle(), -2);
    EXPECT_EQ(key.getName(), "Gm");
}

TEST(KeyFifthCircleMinor, CMinorThreeFlats) {
    Key key(-3, false);
    EXPECT_EQ(key.getFifthCircle(), -3);
    EXPECT_EQ(key.getName(), "Cm");
}

TEST(KeyFifthCircleMinor, FMinorFourFlats) {
    Key key(-4, false);
    EXPECT_EQ(key.getFifthCircle(), -4);
    EXPECT_EQ(key.getName(), "Fm");
}

TEST(KeyFifthCircleMinor, BFlatMinorFiveFlats) {
    Key key(-5, false);
    EXPECT_EQ(key.getFifthCircle(), -5);
    EXPECT_EQ(key.getName(), "Bbm");
}

// ============================================================================
// Set Fifth Circle Tests
// ============================================================================

TEST(KeySetFifthCircle, ChangeFromCToG) {
    Key key;
    EXPECT_EQ(key.getName(), "C");

    key.setFifthCircle(1);
    EXPECT_EQ(key.getFifthCircle(), 1);
    EXPECT_EQ(key.getName(), "G");
}

TEST(KeySetFifthCircle, ChangeFromCToF) {
    Key key;
    key.setFifthCircle(-1);
    EXPECT_EQ(key.getFifthCircle(), -1);
    EXPECT_EQ(key.getName(), "F");
}

TEST(KeySetFifthCircle, ChangeThroughMultipleKeys) {
    Key key;

    key.setFifthCircle(2);
    EXPECT_EQ(key.getName(), "D");

    key.setFifthCircle(-3);
    EXPECT_EQ(key.getName(), "Eb");

    key.setFifthCircle(0);
    EXPECT_EQ(key.getName(), "C");
}

// ============================================================================
// Mode Tests
// ============================================================================

TEST(KeyMode, SetMajorMode) {
    Key key(0, false);  // Start with A minor
    EXPECT_EQ(key.isMajorMode(), 0);

    key.setIsMajorMode(true);
    EXPECT_EQ(key.isMajorMode(), 1);
    EXPECT_EQ(key.getName(), "C");  // Same fifth circle, now major
}

TEST(KeyMode, SetMinorMode) {
    Key key(0, true);  // Start with C major
    EXPECT_EQ(key.isMajorMode(), 1);

    key.setIsMajorMode(false);
    EXPECT_EQ(key.isMajorMode(), 0);
    EXPECT_EQ(key.getName(), "Am");  // Same fifth circle, now minor
}

TEST(KeyMode, ToggleModeGKey) {
    Key key(1, true);  // G major
    EXPECT_EQ(key.getName(), "G");

    key.setIsMajorMode(false);
    EXPECT_EQ(key.getName(), "Em");  // E minor

    key.setIsMajorMode(true);
    EXPECT_EQ(key.getName(), "G");  // Back to G major
}

TEST(KeyMode, MajorModeReturnsOne) {
    Key major_key(0, true);
    EXPECT_EQ(major_key.isMajorMode(), 1);
}

TEST(KeyMode, MinorModeReturnsZero) {
    Key minor_key(0, false);
    EXPECT_EQ(minor_key.isMajorMode(), 0);
}

// ============================================================================
// Relative Key Tests
// ============================================================================

TEST(KeyRelative, CMajorRelativeIsAMinor) {
    Key key("C");
    EXPECT_EQ(key.getRelativeKeyName(), "Am");
}

TEST(KeyRelative, AMinorRelativeIsCMajor) {
    Key key("Am");
    EXPECT_EQ(key.getRelativeKeyName(), "C");
}

TEST(KeyRelative, GMajorRelativeIsEMinor) {
    Key key("G");
    EXPECT_EQ(key.getRelativeKeyName(), "Em");
}

TEST(KeyRelative, EMinorRelativeIsGMajor) {
    Key key("Em");
    EXPECT_EQ(key.getRelativeKeyName(), "G");
}

TEST(KeyRelative, DMajorRelativeIsBMinor) {
    Key key("D");
    EXPECT_EQ(key.getRelativeKeyName(), "Bm");
}

TEST(KeyRelative, FMajorRelativeIsDMinor) {
    Key key("F");
    EXPECT_EQ(key.getRelativeKeyName(), "Dm");
}

TEST(KeyRelative, BFlatMajorRelativeIsGMinor) {
    Key key("Bb");
    EXPECT_EQ(key.getRelativeKeyName(), "Gm");
}

TEST(KeyRelative, FSharpMajorRelativeIsDSharpMinor) {
    Key key("F#");
    EXPECT_EQ(key.getRelativeKeyName(), "D#m");
}

TEST(KeyRelative, AllMajorKeysHaveRelativeMinor) {
    std::vector<std::pair<std::string, std::string>> major_minor_pairs = {
        {"C", "Am"},    {"G", "Em"},    {"D", "Bm"},   {"A", "F#m"},
        {"E", "C#m"},   {"B", "G#m"},   {"F#", "D#m"}, {"F", "Dm"},
        {"Bb", "Gm"},   {"Eb", "Cm"},   {"Ab", "Fm"},  {"Db", "Bbm"}
    };

    for (const auto& pair : major_minor_pairs) {
        Key major_key(pair.first);
        EXPECT_EQ(major_key.getRelativeKeyName(), pair.second);
    }
}

TEST(KeyRelative, AllMinorKeysHaveRelativeMajor) {
    std::vector<std::pair<std::string, std::string>> minor_major_pairs = {
        {"Am", "C"},    {"Em", "G"},    {"Bm", "D"},   {"F#m", "A"},
        {"C#m", "E"},   {"G#m", "B"},   {"D#m", "F#"}, {"Dm", "F"},
        {"Gm", "Bb"},   {"Cm", "Eb"},   {"Fm", "Ab"},  {"Bbm", "Db"}
    };

    for (const auto& pair : minor_major_pairs) {
        Key minor_key(pair.first);
        EXPECT_EQ(minor_key.getRelativeKeyName(), pair.second);
    }
}

// ============================================================================
// Key Name String Parsing Tests
// ============================================================================

TEST(KeyNameParsing, AllMajorKeysParsedCorrectly) {
    std::vector<std::string> major_keys = {
        "C", "G", "D", "A", "E", "B", "F#", "C#",
        "F", "Bb", "Eb", "Ab", "Db", "Gb"
    };

    for (const auto& key_name : major_keys) {
        Key key(key_name);
        EXPECT_EQ(key.getName(), key_name);
        EXPECT_EQ(key.isMajorMode(), 1);
    }
}

TEST(KeyNameParsing, AllMinorKeysParsedCorrectly) {
    std::vector<std::string> minor_keys = {
        "Am", "Em", "Bm", "F#m", "C#m", "G#m", "D#m",
        "Dm", "Gm", "Cm", "Fm", "Bbm"
    };

    for (const auto& key_name : minor_keys) {
        Key key(key_name);
        EXPECT_EQ(key.getName(), key_name);
        EXPECT_EQ(key.isMajorMode(), 0);
    }
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

TEST(KeyEdgeCases, ExtremeSharps) {
    Key key(8, true);  // G# major
    EXPECT_EQ(key.getFifthCircle(), 8);
    EXPECT_EQ(key.getName(), "G#");
}

TEST(KeyEdgeCases, VeryHighFifthCircle) {
    Key key(11, true);  // E# major
    EXPECT_EQ(key.getFifthCircle(), 11);
    EXPECT_EQ(key.getName(), "E#");
}

TEST(KeyEdgeCases, SwitchBetweenMajorAndMinor) {
    Key key(3, true);  // A major
    EXPECT_EQ(key.getName(), "A");
    EXPECT_EQ(key.getRelativeKeyName(), "F#m");

    key.setIsMajorMode(false);  // F# minor
    EXPECT_EQ(key.getName(), "F#m");
    EXPECT_EQ(key.getRelativeKeyName(), "A");
}

TEST(KeyEdgeCases, EnharmonicEquivalentFSharpGFlat) {
    Key f_sharp(6, true);   // F# major (6 sharps)
    Key g_flat(-6, true);   // Gb major (6 flats)

    EXPECT_EQ(f_sharp.getName(), "F#");
    EXPECT_EQ(g_flat.getName(), "Gb");
    EXPECT_NE(f_sharp.getFifthCircle(), g_flat.getFifthCircle());
}

TEST(KeyEdgeCases, CreateModifyCheck) {
    Key key;

    // Start with C major
    EXPECT_EQ(key.getFifthCircle(), 0);
    EXPECT_EQ(key.isMajorMode(), 1);
    EXPECT_EQ(key.getName(), "C");

    // Change to G major
    key.setFifthCircle(1);
    EXPECT_EQ(key.getName(), "G");

    // Change to E minor
    key.setFifthCircle(1);
    key.setIsMajorMode(false);
    EXPECT_EQ(key.getName(), "Em");

    // Change to Bb major
    key.setFifthCircle(-2);
    key.setIsMajorMode(true);
    EXPECT_EQ(key.getName(), "Bb");
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(KeyIntegration, ConstructFromStringAndGetFifthCircle) {
    Key key("D");
    EXPECT_EQ(key.getFifthCircle(), 2);
    EXPECT_EQ(key.isMajorMode(), 1);

    Key key2("Fm");
    EXPECT_EQ(key2.getFifthCircle(), -4);
    EXPECT_EQ(key2.isMajorMode(), 0);
}

TEST(KeyIntegration, SetFifthCircleAndCheckName) {
    Key key;
    key.setFifthCircle(3);
    EXPECT_EQ(key.getName(), "A");

    key.setIsMajorMode(false);
    EXPECT_EQ(key.getName(), "F#m");
}

TEST(KeyIntegration, RelativeKeyRoundTrip) {
    Key c_major("C");
    std::string relative = c_major.getRelativeKeyName();
    EXPECT_EQ(relative, "Am");

    Key a_minor(relative);
    EXPECT_EQ(a_minor.getRelativeKeyName(), "C");
}

TEST(KeyIntegration, AllCircleOfFifthsMajor) {
    for (int i = -6; i <= 11; ++i) {
        Key key(i, true);
        EXPECT_EQ(key.getFifthCircle(), i);
        EXPECT_EQ(key.isMajorMode(), 1);
        EXPECT_FALSE(key.getName().empty());
    }
}

TEST(KeyIntegration, AllCircleOfFifthsMinor) {
    for (int i = -6; i <= 6; ++i) {
        Key key(i, false);
        EXPECT_EQ(key.getFifthCircle(), i);
        EXPECT_EQ(key.isMajorMode(), 0);
        EXPECT_FALSE(key.getName().empty());
    }
}

TEST(KeyIntegration, FifthCircleConsistency) {
    std::vector<std::pair<int, std::string>> fifth_to_major = {
        {-6, "Gb"}, {-5, "Db"}, {-4, "Ab"}, {-3, "Eb"}, {-2, "Bb"}, {-1, "F"},
        {0, "C"},   {1, "G"},   {2, "D"},   {3, "A"},   {4, "E"},   {5, "B"}, {6, "F#"}
    };

    for (const auto& pair : fifth_to_major) {
        Key key(pair.first, true);
        EXPECT_EQ(key.getName(), pair.second);
        EXPECT_EQ(key.getFifthCircle(), pair.first);
    }
}
