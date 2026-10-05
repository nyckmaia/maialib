#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/score.h"

namespace maiacore_python {

namespace py = pybind11;

// The pandas DataFrame of a melody search, built column by column. Every column has a fixed
// dtype, so a search without a match gives an empty DataFrame with every column, and with the
// dtypes of a search with matches: text columns `str`, integer columns `int64`, real columns
// `float64` and list columns `object`.
class MelodyDataFrame {
   public:
    enum class Kind { Text, Integer, Real, List };

    // 'leading' names the columns that come before the match's own (patternIdx, fileName, ...),
    // with their kinds.
    explicit MelodyDataFrame(const std::vector<std::pair<std::string, Kind>>& leading)
        : _numLeading(leading.size()) {
        for (const auto& column : leading) {
            _columns.push_back({column.first, column.second, py::list()});
        }
        const std::vector<std::pair<std::string, Kind>> matchColumns = {
            {"partName", Kind::Text},           {"measure", Kind::Integer},
            {"staff", Kind::Integer},           {"voice", Kind::Integer},
            {"writtenKey", Kind::Text},         {"concertKey", Kind::Text},
            {"transposeInterval", Kind::Text},  {"transposeSemitones", Kind::Real},
            {"writtenPitches", Kind::List},     {"soundingPitches", Kind::List},
            {"semitonesDiff", Kind::List},      {"rhythmDiff", Kind::List},
            {"intervalSimilarity", Kind::Real}, {"rhythmSimilarity", Kind::Real},
            {"totalSimilarity", Kind::Real}};
        for (const auto& column : matchColumns) {
            _columns.push_back({column.first, column.second, py::list()});
        }
    }

    // Appends a row: 'leading' holds the values of the leading columns, in their order.
    void appendRow(const py::list& leading, const Score::MelodyPatternRow& match) {
        if (leading.size() != _numLeading) {
            throw std::logic_error("MelodyDataFrame: a row needs one value per leading column");
        }
        size_t c = 0;
        for (const py::handle value : leading) {
            _columns[c++].values.append(value);
        }
        const py::object values[] = {py::cast(match.partName),
                                     py::cast(match.measure),
                                     py::cast(match.staff),
                                     py::cast(match.voice),
                                     py::cast(match.writtenKey),
                                     py::cast(match.concertKey),
                                     py::cast(match.transposeInterval),
                                     py::cast(match.transposeSemitones),
                                     py::cast(match.writtenPitches),
                                     py::cast(match.soundingPitches),
                                     py::cast(match.semitonesDiff),
                                     py::cast(match.rhythmDiff),
                                     py::cast(match.intervalSimilarity),
                                     py::cast(match.rhythmSimilarity),
                                     py::cast(match.totalSimilarity)};
        for (const py::object& value : values) {
            _columns[c++].values.append(value);
        }
    }

    // The DataFrame, with the rows in the order they were appended.
    py::object build() const {
        const py::module_ pandas = py::module_::import("pandas");
        const py::object text = py::module_::import("builtins").attr("str");
        py::dict data;
        for (const Column& column : _columns) {
            py::object dtype;
            switch (column.kind) {
                case Kind::Text:
                    dtype = text;
                    break;
                case Kind::Integer:
                    dtype = py::str("int64");
                    break;
                case Kind::Real:
                    dtype = py::str("float64");
                    break;
                case Kind::List:
                    dtype = py::str("object");
                    break;
            }
            data[py::str(column.name)] =
                pandas.attr("Series")(column.values, py::arg("dtype") = dtype);
        }
        return pandas.attr("DataFrame")(data);
    }

   private:
    struct Column {
        std::string name;
        Kind kind;
        py::list values;
    };
    size_t _numLeading;
    std::vector<Column> _columns;
};

}  // namespace maiacore_python
