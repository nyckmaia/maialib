#include "import-report.h"

#include <gtest/gtest.h>

#include <cctype>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using maiacore::detail::importSummary;
using maiacore::detail::issueCatalogue;
using maiacore::detail::IssueCode;
using maiacore::detail::IssueLocation;
using maiacore::detail::makeIssue;
using maiacore::detail::validUtf8;

namespace {
const std::string kReplacement = "\xEF\xBF\xBD";  // U+FFFD in UTF-8
}  // namespace

// Valid UTF-8 passes unchanged: ASCII, and two-, three- and four-byte sequences at the edges of
// their ranges.
TEST(ImportReportUtf8, ValidTextIsUnchanged) {
    for (const std::string& text :
         {std::string("C4 <alter>"), std::string("can\xC3\xA7\xC3\xA3o"),
          std::string("\xE6\x97\xA5\xE6\x9C\xAC"), std::string("\xF0\x9D\x84\x9E"),
          std::string("\xED\x9F\xBF"), std::string("\xEE\x80\x80"), std::string("\xF4\x8F\xBF\xBF"),
          std::string("")}) {
        EXPECT_EQ(validUtf8(text), text);
    }
}

// Each byte that belongs to no well-formed sequence becomes one U+FFFD; the bytes around it stay.
TEST(ImportReportUtf8, EachInvalidByteBecomesAReplacementCharacter) {
    EXPECT_EQ(validUtf8("\xE9"), kReplacement);                     // Latin-1 e acute
    EXPECT_EQ(validUtf8("a\xE9z"), "a" + kReplacement + "z");       // in the middle
    EXPECT_EQ(validUtf8("\x80"), kReplacement);                     // stray continuation
    EXPECT_EQ(validUtf8("\xC3"), kReplacement);                     // truncated at the end
    EXPECT_EQ(validUtf8("\xC0\xAF"), kReplacement + kReplacement);  // overlong '/'
    EXPECT_EQ(validUtf8("\xE0\x80\xAF"), kReplacement + kReplacement + kReplacement);
    EXPECT_EQ(validUtf8("\xED\xA0\x80"), kReplacement + kReplacement + kReplacement);  // surrogate
    EXPECT_EQ(validUtf8("\xF4\x90\x80\x80"),
              kReplacement + kReplacement + kReplacement + kReplacement);  // above U+10FFFF
    EXPECT_EQ(validUtf8("\xE6\x97x"), kReplacement + kReplacement + "x");  // cut short
}

// Every code is UPPER_SNAKE, listed once, in sorted order, with the kind "corrected" or "dropped".
TEST(ImportReportCatalogue, CodesAreUpperSnakeSortedAndUnique) {
    std::string previous;
    for (const IssueCode& entry : issueCatalogue()) {
        const std::string code = entry.code;
        ASSERT_FALSE(code.empty());
        for (const char c : code) {
            EXPECT_TRUE(std::isupper(static_cast<unsigned char>(c)) || c == '_') << code;
        }
        EXPECT_LT(previous, code);
        previous = code;
        EXPECT_TRUE(std::string(entry.kind) == "corrected" || std::string(entry.kind) == "dropped")
            << code;
    }
}

// A record takes its kind from the catalogue and every text through validUtf8(); a code outside
// the catalogue is a programming error.
TEST(ImportReportRecord, ARecordTakesItsKindAndSanitisesItsText) {
    const ImportIssue issue = makeIssue("ALTER_OFF_GRID", IssueLocation{0, "Viol\xE9", "1\xE9", 3},
                                        "note/\xE9", "\xE9", "C4", "The <alter> value '\xE9' ...");
    EXPECT_EQ(issue.code, "ALTER_OFF_GRID");
    EXPECT_EQ(issue.kind, "corrected");
    EXPECT_EQ(issue.partIndex, 0);
    EXPECT_EQ(issue.partName, "Viol" + kReplacement);
    EXPECT_EQ(issue.measureNumber, "1" + kReplacement);
    EXPECT_EQ(issue.measureIndex, 3);
    EXPECT_EQ(issue.element, "note/" + kReplacement);
    EXPECT_EQ(issue.found, kReplacement);
    EXPECT_EQ(issue.used, "C4");
    EXPECT_EQ(issue.message, "The <alter> value '" + kReplacement + "' ...");

    EXPECT_THROW(makeIssue("NOT_A_CODE", IssueLocation{}, "", "", "", ""), std::logic_error);
}

// The summary counts the corrections and the distinct elements of the dropped records.
TEST(ImportReportSummary, TheSummaryCountsCorrectionsAndDroppedElementTypes) {
    ImportIssue corrected;
    corrected.kind = "corrected";
    ImportIssue lyric;
    lyric.kind = "dropped";
    lyric.element = "note/lyric";
    ImportIssue direction = lyric;
    direction.element = "direction";
    EXPECT_EQ(importSummary("a.xml", {corrected, corrected, lyric, direction, lyric}),
              "[maiacore] a.xml: 2 corrections, 2 element types not modelled (dropped on "
              "export); see Score.getImportIssues()");
    EXPECT_EQ(importSummary("b\xE9.xml", {corrected}),
              "[maiacore] b" + kReplacement +
                  ".xml: 1 corrections, 0 element types not modelled (dropped on export); see "
                  "Score.getImportIssues()");
}
