#include "import-report.h"

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace maiacore::detail {

std::string validUtf8(const std::string& text) {
    static const char kReplacement[] = "\xEF\xBF\xBD";  // U+FFFD
    std::string valid;
    valid.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        // The length of the sequence 'lead' starts, and the range its second byte must be in.
        size_t length = 0;
        unsigned char low = 0x80;
        unsigned char high = 0xBF;
        if (lead < 0x80) {
            length = 1;
        } else if (lead >= 0xC2 && lead <= 0xDF) {
            length = 2;
        } else if (lead == 0xE0) {
            length = 3;
            low = 0xA0;  // no overlong form
        } else if (lead == 0xED) {
            length = 3;
            high = 0x9F;  // no surrogate
        } else if (lead >= 0xE1 && lead <= 0xEF) {
            length = 3;
        } else if (lead == 0xF0) {
            length = 4;
            low = 0x90;  // no overlong form
        } else if (lead >= 0xF1 && lead <= 0xF3) {
            length = 4;
        } else if (lead == 0xF4) {
            length = 4;
            high = 0x8F;  // nothing above U+10FFFF
        }
        bool wellFormed = length > 0 && i + length <= text.size();
        for (size_t k = 1; wellFormed && k < length; k++) {
            const unsigned char byte = static_cast<unsigned char>(text[i + k]);
            wellFormed = (k == 1) ? (byte >= low && byte <= high) : (byte >= 0x80 && byte <= 0xBF);
        }
        if (wellFormed) {
            valid.append(text, i, length);
            i += length;
        } else {
            valid += kReplacement;
            i++;
        }
    }
    return valid;
}

const std::vector<IssueCode>& issueCatalogue() {
    static const std::vector<IssueCode> catalogue = {
        {"ACCIDENTAL_ALTER_MISMATCH", "corrected"},
        {"ACCIDENTAL_NAME_UNKNOWN", "corrected"},
        {"ALTER_OFF_GRID", "corrected"},
        {"DIVISIONS_MISSING", "corrected"},
        {"FOR_PART_NOT_MODELLED", "dropped"},
        {"PART_NAME_DUPLICATE", "corrected"},
        {"STAFF_CLAMPED", "corrected"},
        {"TRANSPOSE_CHROMATIC_NOT_INTEGER", "corrected"},
        {"TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER", "corrected"},
        {"TRANSPOSE_OUT_OF_RANGE", "corrected"},
        {"TRANSPOSE_PAIR_CORRECTED", "corrected"},
        {"TUPLET_CLAMPED", "corrected"},
        {"VOICE_NOT_POSITIVE", "corrected"},
    };
    return catalogue;
}

ImportIssue makeIssue(const std::string& code, const IssueLocation& location,
                      const std::string& element, const std::string& found, const std::string& used,
                      const std::string& message) {
    for (const IssueCode& entry : issueCatalogue()) {
        if (code == entry.code) {
            ImportIssue issue;
            issue.code = code;
            issue.kind = entry.kind;
            issue.partIndex = location.partIndex;
            issue.partName = validUtf8(location.partName);
            issue.measureNumber = validUtf8(location.measureNumber);
            issue.measureIndex = location.measureIndex;
            issue.element = validUtf8(element);
            issue.found = validUtf8(found);
            issue.used = validUtf8(used);
            issue.message = validUtf8(message);
            return issue;
        }
    }
    throw std::logic_error("makeIssue: '" + code + "' is not in the import-report catalogue");
}

std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues) {
    int corrections = 0;
    std::set<std::string> dropped;
    for (const ImportIssue& issue : issues) {
        if (issue.kind == "corrected") {
            corrections++;
        } else {
            dropped.insert(issue.element);
        }
    }
    return "[maiacore] " + validUtf8(fileName) + ": " + std::to_string(corrections) +
           " corrections, " + std::to_string(dropped.size()) +
           " element types not modelled (dropped on export); see Score.getImportIssues()";
}

}  // namespace maiacore::detail
