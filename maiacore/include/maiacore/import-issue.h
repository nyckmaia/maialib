#pragma once

#include <string>
#include <tuple>

/**
 * @brief One record of the import report of a score loaded from a MusicXML file: a value the
 *        reader corrected, or an element it dropped.
 * @details Score::getImportIssues() returns the records of the load, in the order the reader made
 *          them. A record never changes after the load: edits and exports of the score leave the
 *          report as it is.
 */
struct ImportIssue {
    /// Stable identifier in UPPER_SNAKE case, such as "ALTER_OFF_GRID"; a code is never renamed.
    std::string code;
    /// "corrected" when a value read was replaced or filled, "dropped" when an element was
    /// removed.
    std::string kind;
    int partIndex = -1;    ///< 0-based index of the part; -1 when the record is about no one part.
    std::string partName;  ///< The part's name in the score; empty when partIndex is -1.
    /// The measure's number attribute as the file writes it; empty when the record is about no
    /// one measure.
    std::string measureNumber;
    int measureIndex = -1;  ///< 0-based index of the measure; -1 when measureNumber is empty.
    /// The element's path: from its measure for an element inside a measure
    /// ("note/pitch/alter"), from the score's root element otherwise
    /// ("part-list/score-part/part-name").
    std::string element;
    /// The value read; empty when the element is absent. For a "dropped" element of the closed
    /// element list, the number of such elements in the file.
    std::string found;
    /// The value stored in the score; empty when nothing of the element is stored.
    std::string used;
    std::string message;  ///< What happened, in English.

    /**
     * @brief Field-by-field equality.
     */
    bool operator==(const ImportIssue& other) const {
        return std::tie(code, kind, partIndex, partName, measureNumber, measureIndex, element,
                        found, used, message) == std::tie(other.code, other.kind, other.partIndex,
                                                          other.partName, other.measureNumber,
                                                          other.measureIndex, other.element,
                                                          other.found, other.used, other.message);
    }
};
