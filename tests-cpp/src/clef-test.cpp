#include <gtest/gtest.h>

#include "maiacore/clef.h"

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(ClefConstructor, DefaultConstructor) {
    Clef clef;
    EXPECT_EQ(clef.getSign(), ClefSign::G);
    EXPECT_EQ(clef.getLine(), 2);
    EXPECT_FALSE(clef.isClefChanged());
}

TEST(ClefConstructor, GClefDefaultLine) {
    Clef clef(ClefSign::G);
    EXPECT_EQ(clef.getSign(), ClefSign::G);
    EXPECT_EQ(clef.getLine(), 2);
}

TEST(ClefConstructor, FClefDefaultLine) {
    Clef clef(ClefSign::F);
    EXPECT_EQ(clef.getSign(), ClefSign::F);
    EXPECT_EQ(clef.getLine(), 4);
}

TEST(ClefConstructor, CClefDefaultLine) {
    Clef clef(ClefSign::C);
    EXPECT_EQ(clef.getSign(), ClefSign::C);
    EXPECT_EQ(clef.getLine(), 3);
}

TEST(ClefConstructor, PercussionClefDefaultLine) {
    Clef clef(ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getSign(), ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getLine(), 2);
}

TEST(ClefConstructor, CClefCustomLine) {
    // Constructor parameter ignored - always uses default
    Clef clef(ClefSign::C, 4);
    EXPECT_EQ(clef.getSign(), ClefSign::C);
    EXPECT_EQ(clef.getLine(), 3);  // Default C clef line
}

TEST(ClefConstructor, PercussionClefCustomLine) {
    // Constructor parameter ignored - always uses default
    Clef clef(ClefSign::PERCUSSION, 3);
    EXPECT_EQ(clef.getSign(), ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getLine(), 2);  // Default percussion line
}

TEST(ClefConstructor, InitialChangedStatus) {
    Clef clef(ClefSign::G);
    EXPECT_FALSE(clef.isClefChanged());
}

// ============================================================================
// Get/Set Sign Tests
// ============================================================================

TEST(ClefSign, GetSignG) {
    Clef clef(ClefSign::G);
    EXPECT_EQ(clef.getSign(), ClefSign::G);
}

TEST(ClefSign, GetSignF) {
    Clef clef(ClefSign::F);
    EXPECT_EQ(clef.getSign(), ClefSign::F);
}

TEST(ClefSign, GetSignC) {
    Clef clef(ClefSign::C);
    EXPECT_EQ(clef.getSign(), ClefSign::C);
}

TEST(ClefSign, GetSignPercussion) {
    Clef clef(ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getSign(), ClefSign::PERCUSSION);
}

TEST(ClefSign, SetSignToF) {
    Clef clef(ClefSign::G);
    clef.setSign(ClefSign::F);
    EXPECT_EQ(clef.getSign(), ClefSign::F);
    EXPECT_EQ(clef.getLine(), 4);  // Auto-updates to F default
}

TEST(ClefSign, SetSignToC) {
    Clef clef(ClefSign::G);
    clef.setSign(ClefSign::C);
    EXPECT_EQ(clef.getSign(), ClefSign::C);
    EXPECT_EQ(clef.getLine(), 3);  // Auto-updates to C default
}

TEST(ClefSign, SetSignToG) {
    Clef clef(ClefSign::F);
    clef.setSign(ClefSign::G);
    EXPECT_EQ(clef.getSign(), ClefSign::G);
    EXPECT_EQ(clef.getLine(), 2);  // Auto-updates to G default
}

TEST(ClefSign, SetSignToPercussion) {
    Clef clef(ClefSign::G);
    clef.setSign(ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getSign(), ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getLine(), 2);  // Auto-updates to percussion default
}

TEST(ClefSign, SetSignTriggersChanged) {
    Clef clef(ClefSign::G);
    EXPECT_FALSE(clef.isClefChanged());

    clef.setSign(ClefSign::F);
    EXPECT_TRUE(clef.isClefChanged());
}

// ============================================================================
// Get/Set Line Tests
// ============================================================================

TEST(ClefLine, GetLineG) {
    Clef clef(ClefSign::G);
    EXPECT_EQ(clef.getLine(), 2);
}

TEST(ClefLine, GetLineF) {
    Clef clef(ClefSign::F);
    EXPECT_EQ(clef.getLine(), 4);
}

TEST(ClefLine, GetLineC) {
    Clef clef(ClefSign::C);
    EXPECT_EQ(clef.getLine(), 3);
}

TEST(ClefLine, GetLinePercussion) {
    Clef clef(ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getLine(), 2);
}

TEST(ClefLine, SetLineForCClef) {
    Clef clef(ClefSign::C);
    clef.setLine(4);  // Tenor clef
    EXPECT_EQ(clef.getLine(), 4);
}

TEST(ClefLine, SetLineForPercussionClef) {
    Clef clef(ClefSign::PERCUSSION);
    clef.setLine(3);
    EXPECT_EQ(clef.getLine(), 3);
}

TEST(ClefLine, SetLineIgnoredForGClef) {
    Clef clef(ClefSign::G);
    clef.setLine(5);  // Should be ignored
    EXPECT_EQ(clef.getLine(), 2);  // Unchanged
}

TEST(ClefLine, SetLineIgnoredForFClef) {
    Clef clef(ClefSign::F);
    clef.setLine(3);  // Should be ignored
    EXPECT_EQ(clef.getLine(), 4);  // Unchanged
}

TEST(ClefLine, SetLineTriggersChangedForC) {
    Clef clef(ClefSign::C);
    EXPECT_FALSE(clef.isClefChanged());

    clef.setLine(1);  // Soprano clef
    EXPECT_TRUE(clef.isClefChanged());
}

TEST(ClefLine, SetLineTriggersChangedForPercussion) {
    Clef clef(ClefSign::PERCUSSION);
    EXPECT_FALSE(clef.isClefChanged());

    clef.setLine(3);
    EXPECT_TRUE(clef.isClefChanged());
}

TEST(ClefLine, SetLineDoesNotTriggerChangedForG) {
    Clef clef(ClefSign::G);
    EXPECT_FALSE(clef.isClefChanged());

    clef.setLine(5);  // Ignored
    EXPECT_FALSE(clef.isClefChanged());  // Still false
}

TEST(ClefLine, SetLineDoesNotTriggerChangedForF) {
    Clef clef(ClefSign::F);
    EXPECT_FALSE(clef.isClefChanged());

    clef.setLine(3);  // Ignored
    EXPECT_FALSE(clef.isClefChanged());  // Still false
}

// ============================================================================
// Clef Changed Status Tests
// ============================================================================

TEST(ClefChanged, InitialStatusFalse) {
    Clef clef(ClefSign::G);
    EXPECT_FALSE(clef.isClefChanged());
}

TEST(ClefChanged, SetSignMakesTrue) {
    Clef clef(ClefSign::G);
    clef.setSign(ClefSign::F);
    EXPECT_TRUE(clef.isClefChanged());
}

TEST(ClefChanged, SetLineMakesTrueForC) {
    Clef clef(ClefSign::C);
    clef.setLine(4);
    EXPECT_TRUE(clef.isClefChanged());
}

TEST(ClefChanged, SetLineDoesNotAffectGClef) {
    Clef clef(ClefSign::G);
    clef.setLine(5);  // Ignored
    EXPECT_FALSE(clef.isClefChanged());
}

TEST(ClefChanged, MultipleSignChanges) {
    Clef clef(ClefSign::G);

    clef.setSign(ClefSign::F);
    EXPECT_TRUE(clef.isClefChanged());

    clef.setSign(ClefSign::C);
    EXPECT_TRUE(clef.isClefChanged());
}

// ============================================================================
// String Conversion Tests
// ============================================================================

TEST(ClefString, GetClefSignStrG) {
    Clef clef(ClefSign::G);
    EXPECT_EQ(clef.getClefSignStr(), "G");
}

TEST(ClefString, GetClefSignStrF) {
    Clef clef(ClefSign::F);
    EXPECT_EQ(clef.getClefSignStr(), "F");
}

TEST(ClefString, GetClefSignStrC) {
    Clef clef(ClefSign::C);
    EXPECT_EQ(clef.getClefSignStr(), "C");
}

TEST(ClefString, GetClefSignStrPercussion) {
    Clef clef(ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getClefSignStr(), "percussion");
}

TEST(ClefString, ClefSignStr2ClefSignG) {
    ClefSign sign = Clef::clefSignStr2ClefSign("G");
    EXPECT_EQ(sign, ClefSign::G);
}

TEST(ClefString, ClefSignStr2ClefSignF) {
    ClefSign sign = Clef::clefSignStr2ClefSign("F");
    EXPECT_EQ(sign, ClefSign::F);
}

TEST(ClefString, ClefSignStr2ClefSignC) {
    ClefSign sign = Clef::clefSignStr2ClefSign("C");
    EXPECT_EQ(sign, ClefSign::C);
}

TEST(ClefString, ClefSignStr2ClefSignPercussion) {
    ClefSign sign = Clef::clefSignStr2ClefSign("percussion");
    EXPECT_EQ(sign, ClefSign::PERCUSSION);
}

TEST(ClefString, RoundTripG) {
    Clef clef(ClefSign::G);
    std::string str = clef.getClefSignStr();
    ClefSign sign = Clef::clefSignStr2ClefSign(str);
    EXPECT_EQ(sign, ClefSign::G);
}

TEST(ClefString, RoundTripF) {
    Clef clef(ClefSign::F);
    std::string str = clef.getClefSignStr();
    ClefSign sign = Clef::clefSignStr2ClefSign(str);
    EXPECT_EQ(sign, ClefSign::F);
}

TEST(ClefString, RoundTripC) {
    Clef clef(ClefSign::C);
    std::string str = clef.getClefSignStr();
    ClefSign sign = Clef::clefSignStr2ClefSign(str);
    EXPECT_EQ(sign, ClefSign::C);
}

TEST(ClefString, RoundTripPercussion) {
    Clef clef(ClefSign::PERCUSSION);
    std::string str = clef.getClefSignStr();
    ClefSign sign = Clef::clefSignStr2ClefSign(str);
    EXPECT_EQ(sign, ClefSign::PERCUSSION);
}

// ============================================================================
// XML Serialization Tests
// ============================================================================

TEST(ClefXML, GClefToXMLBasic) {
    Clef clef(ClefSign::G);
    std::string xml = clef.toXML();

    EXPECT_TRUE(xml.find("<clef>") != std::string::npos);
    EXPECT_TRUE(xml.find("<sign>G</sign>") != std::string::npos);
    EXPECT_TRUE(xml.find("<line>2</line>") != std::string::npos);
    EXPECT_TRUE(xml.find("</clef>") != std::string::npos);
}

TEST(ClefXML, FClefToXMLBasic) {
    Clef clef(ClefSign::F);
    std::string xml = clef.toXML();

    EXPECT_TRUE(xml.find("<sign>F</sign>") != std::string::npos);
    EXPECT_TRUE(xml.find("<line>4</line>") != std::string::npos);
}

TEST(ClefXML, CClefToXMLBasic) {
    Clef clef(ClefSign::C);
    std::string xml = clef.toXML();

    EXPECT_TRUE(xml.find("<sign>C</sign>") != std::string::npos);
    EXPECT_TRUE(xml.find("<line>3</line>") != std::string::npos);
}

TEST(ClefXML, PercussionClefToXMLBasic) {
    Clef clef(ClefSign::PERCUSSION);
    std::string xml = clef.toXML();

    EXPECT_TRUE(xml.find("<sign>percussion</sign>") != std::string::npos);
    EXPECT_TRUE(xml.find("<line>2</line>") != std::string::npos);
}

TEST(ClefXML, ToXMLWithClefNumber) {
    Clef clef(ClefSign::G);
    std::string xml = clef.toXML(0);  // First clef

    EXPECT_TRUE(xml.find("<clef number=\"1\">") != std::string::npos);
    EXPECT_TRUE(xml.find("<sign>G</sign>") != std::string::npos);
}

TEST(ClefXML, ToXMLWithClefNumber2) {
    Clef clef(ClefSign::F);
    std::string xml = clef.toXML(1);  // Second clef

    EXPECT_TRUE(xml.find("<clef number=\"2\">") != std::string::npos);
}

TEST(ClefXML, ToXMLWithoutClefNumber) {
    Clef clef(ClefSign::G);
    std::string xml = clef.toXML(-1);  // Default

    EXPECT_TRUE(xml.find("<clef>") != std::string::npos);
    EXPECT_TRUE(xml.find("number") == std::string::npos);
}

TEST(ClefXML, CClefCustomLineToXML) {
    Clef clef(ClefSign::C);
    clef.setLine(4);  // Tenor clef
    std::string xml = clef.toXML();

    EXPECT_TRUE(xml.find("<sign>C</sign>") != std::string::npos);
    EXPECT_TRUE(xml.find("<line>4</line>") != std::string::npos);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(ClefIntegration, ConstructAndModifySign) {
    Clef clef(ClefSign::G);
    EXPECT_EQ(clef.getSign(), ClefSign::G);
    EXPECT_EQ(clef.getLine(), 2);
    EXPECT_FALSE(clef.isClefChanged());

    clef.setSign(ClefSign::F);
    EXPECT_EQ(clef.getSign(), ClefSign::F);
    EXPECT_EQ(clef.getLine(), 4);  // Auto-updated
    EXPECT_TRUE(clef.isClefChanged());
}

TEST(ClefIntegration, ModifyBetweenMultipleSigns) {
    Clef clef(ClefSign::G);

    clef.setSign(ClefSign::F);
    EXPECT_EQ(clef.getLine(), 4);

    clef.setSign(ClefSign::C);
    EXPECT_EQ(clef.getLine(), 3);

    clef.setSign(ClefSign::PERCUSSION);
    EXPECT_EQ(clef.getLine(), 2);

    EXPECT_EQ(clef.getSign(), ClefSign::PERCUSSION);
}

TEST(ClefIntegration, CClefVariants) {
    // Alto clef (C on line 3)
    Clef alto(ClefSign::C);
    EXPECT_EQ(alto.getLine(), 3);

    // Tenor clef (C on line 4)
    Clef tenor(ClefSign::C);
    tenor.setLine(4);
    EXPECT_EQ(tenor.getLine(), 4);
    EXPECT_EQ(tenor.getSign(), ClefSign::C);

    // Soprano clef (C on line 1)
    Clef soprano(ClefSign::C);
    soprano.setLine(1);
    EXPECT_EQ(soprano.getLine(), 1);
    EXPECT_EQ(soprano.getSign(), ClefSign::C);
}

TEST(ClefIntegration, LineRestrictionBehavior) {
    Clef gClef(ClefSign::G);
    Clef fClef(ClefSign::F);
    Clef cClef(ClefSign::C);

    // G and F clefs ignore setLine
    gClef.setLine(5);
    fClef.setLine(2);
    EXPECT_EQ(gClef.getLine(), 2);  // Unchanged
    EXPECT_EQ(fClef.getLine(), 4);  // Unchanged

    // C clef accepts setLine
    cClef.setLine(4);
    EXPECT_EQ(cClef.getLine(), 4);  // Changed
}

TEST(ClefIntegration, XMLRoundTripConsistency) {
    Clef original(ClefSign::C);
    original.setLine(4);  // Tenor clef

    std::string xml = original.toXML();

    EXPECT_TRUE(xml.find("<sign>C</sign>") != std::string::npos);
    EXPECT_TRUE(xml.find("<line>4</line>") != std::string::npos);

    // Verify all clefs produce valid XML
    Clef gClef(ClefSign::G);
    Clef fClef(ClefSign::F);
    Clef percClef(ClefSign::PERCUSSION);

    EXPECT_FALSE(gClef.toXML().empty());
    EXPECT_FALSE(fClef.toXML().empty());
    EXPECT_FALSE(percClef.toXML().empty());
}
