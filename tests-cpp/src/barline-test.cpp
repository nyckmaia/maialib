#include <gtest/gtest.h>

#include "maiacore/barline.h"

// ============================================================================
// Constructor Tests
// ============================================================================

TEST(BarlineConstructor, DefaultConstructor) {
    Barline barline;

    EXPECT_EQ(barline.getBarStyle(), "");
    EXPECT_EQ(barline.getDirection(), "");
    EXPECT_EQ(barline.getLocation(), "");
}

TEST(BarlineConstructor, InitialState) {
    Barline barline;

    // All properties should be empty strings initially
    EXPECT_TRUE(barline.getBarStyle().empty());
    EXPECT_TRUE(barline.getDirection().empty());
    EXPECT_TRUE(barline.getLocation().empty());
}

// ============================================================================
// BarStyle Tests
// ============================================================================

TEST(BarlineBarStyle, SetAndGetBarStyle) {
    Barline barline;

    barline.setBarStyle("light-heavy");
    EXPECT_EQ(barline.getBarStyle(), "light-heavy");
}

TEST(BarlineBarStyle, SetHeavyLight) {
    Barline barline;

    barline.setBarStyle("heavy-light");
    EXPECT_EQ(barline.getBarStyle(), "heavy-light");
}

TEST(BarlineBarStyle, SetRegular) {
    Barline barline;

    barline.setBarStyle("regular");
    EXPECT_EQ(barline.getBarStyle(), "regular");
}

TEST(BarlineBarStyle, SetDotted) {
    Barline barline;

    barline.setBarStyle("dotted");
    EXPECT_EQ(barline.getBarStyle(), "dotted");
}

TEST(BarlineBarStyle, SetDashed) {
    Barline barline;

    barline.setBarStyle("dashed");
    EXPECT_EQ(barline.getBarStyle(), "dashed");
}

TEST(BarlineBarStyle, SetHeavy) {
    Barline barline;

    barline.setBarStyle("heavy");
    EXPECT_EQ(barline.getBarStyle(), "heavy");
}

TEST(BarlineBarStyle, SetLightLight) {
    Barline barline;

    barline.setBarStyle("light-light");
    EXPECT_EQ(barline.getBarStyle(), "light-light");
}

TEST(BarlineBarStyle, SetEmptyString) {
    Barline barline;
    barline.setBarStyle("heavy");

    barline.setBarStyle("");
    EXPECT_EQ(barline.getBarStyle(), "");
}

// ============================================================================
// Direction Tests
// ============================================================================

TEST(BarlineDirection, SetAndGetDirection) {
    Barline barline;

    barline.setDirection("forward");
    EXPECT_EQ(barline.getDirection(), "forward");
}

TEST(BarlineDirection, SetForward) {
    Barline barline;

    barline.setDirection("forward");
    EXPECT_EQ(barline.getDirection(), "forward");
}

TEST(BarlineDirection, SetBackward) {
    Barline barline;

    barline.setDirection("backward");
    EXPECT_EQ(barline.getDirection(), "backward");
}

TEST(BarlineDirection, SetEmptyString) {
    Barline barline;
    barline.setDirection("forward");

    barline.setDirection("");
    EXPECT_EQ(barline.getDirection(), "");
}

// ============================================================================
// Location Tests
// ============================================================================

TEST(BarlineLocation, SetAndGetLocation) {
    Barline barline;

    barline.setLocation("right");
    EXPECT_EQ(barline.getLocation(), "right");
}

TEST(BarlineLocation, SetLeft) {
    Barline barline;

    barline.setLocation("left");
    EXPECT_EQ(barline.getLocation(), "left");
}

TEST(BarlineLocation, SetRight) {
    Barline barline;

    barline.setLocation("right");
    EXPECT_EQ(barline.getLocation(), "right");
}

TEST(BarlineLocation, SetMiddle) {
    Barline barline;

    barline.setLocation("middle");
    EXPECT_EQ(barline.getLocation(), "middle");
}

TEST(BarlineLocation, SetEmptyString) {
    Barline barline;
    barline.setLocation("left");

    barline.setLocation("");
    EXPECT_EQ(barline.getLocation(), "");
}

// ============================================================================
// Repeat Start Tests
// ============================================================================

TEST(BarlineRepeat, SetRepeatStart) {
    Barline barline;

    barline.setRepeatStart();

    EXPECT_EQ(barline.getBarStyle(), "heavy-light");
    EXPECT_EQ(barline.getDirection(), "forward");
}

TEST(BarlineRepeat, SetRepeatStartOverridesBarStyle) {
    Barline barline;
    barline.setBarStyle("light-heavy");
    barline.setDirection("backward");

    barline.setRepeatStart();

    EXPECT_EQ(barline.getBarStyle(), "heavy-light");
    EXPECT_EQ(barline.getDirection(), "forward");
}

TEST(BarlineRepeat, SetRepeatStartKeepsLocation) {
    Barline barline;
    barline.setLocation("left");

    barline.setRepeatStart();

    EXPECT_EQ(barline.getLocation(), "left");  // Location unchanged
    EXPECT_EQ(barline.getBarStyle(), "heavy-light");
}

// ============================================================================
// Repeat End Tests
// ============================================================================

TEST(BarlineRepeat, SetRepeatEnd) {
    Barline barline;

    barline.setRepeatEnd();

    EXPECT_EQ(barline.getBarStyle(), "light-heavy");
    EXPECT_EQ(barline.getDirection(), "backward");
}

TEST(BarlineRepeat, SetRepeatEndOverridesBarStyle) {
    Barline barline;
    barline.setBarStyle("heavy-light");
    barline.setDirection("forward");

    barline.setRepeatEnd();

    EXPECT_EQ(barline.getBarStyle(), "light-heavy");
    EXPECT_EQ(barline.getDirection(), "backward");
}

TEST(BarlineRepeat, SetRepeatEndKeepsLocation) {
    Barline barline;
    barline.setLocation("right");

    barline.setRepeatEnd();

    EXPECT_EQ(barline.getLocation(), "right");  // Location unchanged
    EXPECT_EQ(barline.getBarStyle(), "light-heavy");
}

// ============================================================================
// Clean Tests
// ============================================================================

TEST(BarlineClean, CleanResetsAllProperties) {
    Barline barline;
    barline.setBarStyle("light-heavy");
    barline.setDirection("backward");
    barline.setLocation("right");

    barline.clean();

    EXPECT_TRUE(barline.getBarStyle().empty());
    EXPECT_TRUE(barline.getDirection().empty());
    EXPECT_TRUE(barline.getLocation().empty());
}

TEST(BarlineClean, CleanOnEmptyBarline) {
    Barline barline;

    barline.clean();  // Should not crash

    EXPECT_TRUE(barline.getBarStyle().empty());
    EXPECT_TRUE(barline.getDirection().empty());
    EXPECT_TRUE(barline.getLocation().empty());
}

// ============================================================================
// XML Serialization Tests
// ============================================================================

TEST(BarlineXML, ToXMLWithBarStyleOnly) {
    Barline barline;
    barline.setBarStyle("regular");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<barline>") != std::string::npos);
    EXPECT_TRUE(xml.find("<bar-style>regular</bar-style>") != std::string::npos);
    EXPECT_TRUE(xml.find("</barline>") != std::string::npos);
    EXPECT_TRUE(xml.find("location=") == std::string::npos);  // No location
    EXPECT_TRUE(xml.find("<repeat") == std::string::npos);  // No repeat
}

TEST(BarlineXML, ToXMLWithLocation) {
    Barline barline;
    barline.setBarStyle("light-heavy");
    barline.setLocation("right");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<barline location=\"right\">") != std::string::npos);
    EXPECT_TRUE(xml.find("<bar-style>light-heavy</bar-style>") != std::string::npos);
}

TEST(BarlineXML, ToXMLWithDirection) {
    Barline barline;
    barline.setBarStyle("light-heavy");
    barline.setDirection("backward");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<bar-style>light-heavy</bar-style>") != std::string::npos);
    EXPECT_TRUE(xml.find("<repeat direction=\"backward\"/>") != std::string::npos);
}

TEST(BarlineXML, ToXMLRepeatStart) {
    Barline barline;
    barline.setRepeatStart();
    barline.setLocation("left");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<barline location=\"left\">") != std::string::npos);
    EXPECT_TRUE(xml.find("<bar-style>heavy-light</bar-style>") != std::string::npos);
    EXPECT_TRUE(xml.find("<repeat direction=\"forward\"/>") != std::string::npos);
}

TEST(BarlineXML, ToXMLRepeatEnd) {
    Barline barline;
    barline.setRepeatEnd();
    barline.setLocation("right");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<barline location=\"right\">") != std::string::npos);
    EXPECT_TRUE(xml.find("<bar-style>light-heavy</bar-style>") != std::string::npos);
    EXPECT_TRUE(xml.find("<repeat direction=\"backward\"/>") != std::string::npos);
}

TEST(BarlineXML, ToXMLNoLocation) {
    Barline barline;
    barline.setBarStyle("heavy");
    barline.setDirection("forward");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<barline>") != std::string::npos);
    EXPECT_TRUE(xml.find("location=") == std::string::npos);
}

TEST(BarlineXML, ToXMLCustomIndentSize) {
    Barline barline;
    barline.setBarStyle("regular");

    std::string xml2 = barline.toXML(2);
    std::string xml4 = barline.toXML(4);

    // Different indent sizes should produce different XML
    EXPECT_NE(xml2, xml4);
    EXPECT_TRUE(xml2.find("<barline>") != std::string::npos);
    EXPECT_TRUE(xml4.find("<barline>") != std::string::npos);
}

TEST(BarlineXML, ToXMLAllPropertiesSet) {
    Barline barline;
    barline.setBarStyle("light-heavy");
    barline.setDirection("backward");
    barline.setLocation("right");

    std::string xml = barline.toXML();

    EXPECT_TRUE(xml.find("<barline location=\"right\">") != std::string::npos);
    EXPECT_TRUE(xml.find("<bar-style>light-heavy</bar-style>") != std::string::npos);
    EXPECT_TRUE(xml.find("<repeat direction=\"backward\"/>") != std::string::npos);
    EXPECT_TRUE(xml.find("</barline>") != std::string::npos);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(BarlineIntegration, CreateRepeatStartAndModify) {
    Barline barline;

    barline.setRepeatStart();
    EXPECT_EQ(barline.getBarStyle(), "heavy-light");
    EXPECT_EQ(barline.getDirection(), "forward");

    // Change to repeat end
    barline.setRepeatEnd();
    EXPECT_EQ(barline.getBarStyle(), "light-heavy");
    EXPECT_EQ(barline.getDirection(), "backward");
}

TEST(BarlineIntegration, SetAllPropertiesAndClean) {
    Barline barline;

    barline.setBarStyle("light-heavy");
    barline.setDirection("backward");
    barline.setLocation("right");

    EXPECT_FALSE(barline.getBarStyle().empty());
    EXPECT_FALSE(barline.getDirection().empty());
    EXPECT_FALSE(barline.getLocation().empty());

    barline.clean();

    EXPECT_TRUE(barline.getBarStyle().empty());
    EXPECT_TRUE(barline.getDirection().empty());
    EXPECT_TRUE(barline.getLocation().empty());
}

TEST(BarlineIntegration, XMLSerializationRoundTrip) {
    Barline barline;
    barline.setBarStyle("light-heavy");
    barline.setDirection("backward");
    barline.setLocation("right");

    std::string xml = barline.toXML();

    // Verify all properties are in XML
    EXPECT_TRUE(xml.find("light-heavy") != std::string::npos);
    EXPECT_TRUE(xml.find("backward") != std::string::npos);
    EXPECT_TRUE(xml.find("right") != std::string::npos);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(BarlineEdgeCases, MultipleCleanCalls) {
    Barline barline;
    barline.setBarStyle("heavy");

    barline.clean();
    barline.clean();  // Clean twice
    barline.clean();  // Clean thrice

    EXPECT_TRUE(barline.getBarStyle().empty());
}

TEST(BarlineEdgeCases, SetSameValueMultipleTimes) {
    Barline barline;

    barline.setBarStyle("regular");
    barline.setBarStyle("regular");
    barline.setBarStyle("regular");

    EXPECT_EQ(barline.getBarStyle(), "regular");
}

TEST(BarlineEdgeCases, AlternatingRepeatStartEnd) {
    Barline barline;

    barline.setRepeatStart();
    EXPECT_EQ(barline.getBarStyle(), "heavy-light");

    barline.setRepeatEnd();
    EXPECT_EQ(barline.getBarStyle(), "light-heavy");

    barline.setRepeatStart();
    EXPECT_EQ(barline.getBarStyle(), "heavy-light");
}
