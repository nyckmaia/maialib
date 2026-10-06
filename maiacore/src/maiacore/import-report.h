#pragma once

#include <string>
#include <vector>

#include "maiacore/import-issue.h"

namespace maiacore::detail {

// 'text' as valid UTF-8 (RFC 3629): every byte that does not belong to a well-formed sequence --
// a stray continuation byte, a truncated or overlong sequence, a surrogate, a code point above
// U+10FFFF -- is replaced by U+FFFD. Text taken from a file passes through it before it reaches
// a record, a message or the console: Python decodes all three as UTF-8.
std::string validUtf8(const std::string& text);

// Where a record of the import report is: a part, and a measure of it, or neither.
struct IssueLocation {
    int partIndex = -1;
    std::string partName;
    std::string measureNumber;  // as the file writes it
    int measureIndex = -1;
};

// A code of the import report and the kind of every record that carries it.
struct IssueCode {
    const char* code;
    const char* kind;  // "corrected" or "dropped"
};

// Every code the reader records, sorted by code.
const std::vector<IssueCode>& issueCatalogue();

// A record with the catalogue's kind for 'code'; every text is passed through validUtf8().
// Throws std::logic_error for a code that is not in the catalogue.
ImportIssue makeIssue(const std::string& code, const IssueLocation& location,
                      const std::string& element, const std::string& found, const std::string& used,
                      const std::string& message);

// The line printed after a load whose report is not empty, without its line break:
// "[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on export); see
// Score.getImportIssues()", where n counts the "corrected" records and m the distinct elements of
// the "dropped" ones.
std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues);

}  // namespace maiacore::detail
