#include "import-report.h"

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace maiacore::detail {

namespace {
// The closed element list: the elements the model holds, by path. Outside a measure the path
// starts at the root element's children; a part's measure starts a path of its own, from the
// measure's children. "<path>/*" holds every child of <path>, whatever its name. The children of
// a held element are checked in turn; an element that is not held is dropped with everything in
// it.
const std::set<std::string>& heldOutsideMeasures() {
    static const std::set<std::string> held = {
        "work",
        "work/work-title",
        "identification",
        "identification/creator",
        "part-list",
        "part-list/score-part",
        "part-list/score-part/part-name",
        "part-list/score-part/midi-instrument",
        "part-list/score-part/midi-instrument/midi-unpitched",
        "part",
    };
    return held;
}

const std::set<std::string>& heldInMeasures() {
    static const std::set<std::string> held = {
        "attributes",
        "attributes/divisions",
        "attributes/key",
        "attributes/key/fifths",
        "attributes/key/mode",
        "attributes/time",
        "attributes/time/beats",
        "attributes/time/beat-type",
        "attributes/staves",
        "attributes/clef",
        "attributes/clef/sign",
        "attributes/clef/line",
        "attributes/staff-details",
        "attributes/staff-details/staff-lines",
        "attributes/transpose",
        "attributes/transpose/diatonic",
        "attributes/transpose/chromatic",
        "attributes/transpose/octave-change",
        "attributes/transpose/double",
        "barline",
        "barline/bar-style",
        "barline/repeat",
        "backup",
        "backup/duration",
        "forward",
        "forward/duration",
        "forward/voice",
        "forward/staff",
        "note",
        "note/pitch",
        "note/pitch/step",
        "note/pitch/alter",
        "note/pitch/octave",
        "note/unpitched",
        "note/unpitched/display-step",
        "note/unpitched/display-octave",
        "note/rest",
        "note/chord",
        "note/grace",
        "note/duration",
        "note/voice",
        "note/type",
        "note/dot",
        "note/accidental",
        "note/stem",
        "note/staff",
        "note/time-modification",
        "note/time-modification/actual-notes",
        "note/time-modification/normal-notes",
        "note/time-modification/normal-type",
        "note/instrument",
        "note/beam",
        "note/tie",
        "note/notations",
        "note/notations/articulations",
        "note/notations/articulations/*",
        "note/notations/slur",
        "note/notations/tied",
    };
    return held;
}

// Elements inside a measure that the reader drops with a record of their own, so that the
// closed-list pass neither counts them nor looks into them.
const std::set<std::string>& reportedInMeasures() {
    static const std::set<std::string> reported = {"attributes/for-part"};
    return reported;
}
}  // namespace

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
        {"ELEMENT_NOT_MODELLED", "dropped"},
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

std::vector<ImportIssue> droppedElements(const pugi::xml_document& document) {
    // An element whose children are still to be checked: its path, and whether the path starts
    // at a measure.
    struct Pending {
        pugi::xml_node element;
        std::string path;
        bool inMeasure;
    };
    std::map<std::string, int> counts;
    std::vector<Pending> pending = {{document.document_element(), "", false}};
    while (!pending.empty()) {
        const Pending current = pending.back();
        pending.pop_back();
        const std::set<std::string>& held =
            current.inMeasure ? heldInMeasures() : heldOutsideMeasures();
        const bool everyChildHeld = held.count(current.path + "/*") > 0;
        for (const pugi::xml_node child : current.element.children()) {
            if (child.type() != pugi::node_element) {
                continue;
            }
            const std::string path =
                current.path.empty() ? child.name() : current.path + "/" + child.name();
            if (!current.inMeasure && path == "part/measure") {
                pending.push_back({child, "", true});
            } else if (current.inMeasure && reportedInMeasures().count(path) > 0) {
                continue;
            } else if (everyChildHeld || held.count(path) > 0) {
                pending.push_back({child, path, current.inMeasure});
            } else {
                counts[path]++;
            }
        }
    }

    std::vector<ImportIssue> issues;
    for (const auto& entry : counts) {
        const std::string count = std::to_string(entry.second);
        issues.push_back(makeIssue("ELEMENT_NOT_MODELLED", IssueLocation{}, entry.first, count, "",
                                   "'" + entry.first +
                                       "' is not held by the model and is dropped on export (" +
                                       count + " in the file)."));
    }
    return issues;
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
