#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "maiacore/score_collection.h"

namespace py = pybind11;
using namespace pybind11::literals;

void ScoreCollectionClass(const py::module& m) {
    m.doc() = "ScoreCollection class binding";

    // bindings to ScoreCollection class
    py::class_<ScoreCollection> cls(m, "ScoreCollection");
    cls.def(py::init<const std::string&>(), py::arg("directoryPath") = std::string());

    cls.def(py::init<const std::vector<std::string>&>(),
            py::arg("directoriesPaths") = std::vector<std::string>());

    cls.def("getDirectoriesPaths", &ScoreCollection::getDirectoriesPaths);
    cls.def("setDirectoriesPaths", &ScoreCollection::setDirectoriesPaths,
            py::arg("directoriesPaths"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("addDirectory", &ScoreCollection::addDirectory, py::arg("directoryPath"));

    cls.def("addScore", py::overload_cast<const Score&>(&ScoreCollection::addScore),
            py::arg("score"));
    cls.def("addScore", py::overload_cast<const std::string&>(&ScoreCollection::addScore),
            py::arg("filePath"));
    cls.def("addScore",
            py::overload_cast<const std::vector<std::string>&>(&ScoreCollection::addScore),
            py::arg("filePaths"));

    cls.def("clear", &ScoreCollection::clear);
    cls.def("getNumDirectories", &ScoreCollection::getNumDirectories);
    cls.def("getNumScores", &ScoreCollection::getNumScores);

    cls.def("getScores", py::overload_cast<>(&ScoreCollection::getScores),
            py::return_value_policy::reference_internal);
    cls.def("getScores", py::overload_cast<>(&ScoreCollection::getScores, py::const_),
            py::return_value_policy::reference_internal);

    cls.def("isEmpty", &ScoreCollection::isEmpty);
    cls.def("merge", &ScoreCollection::merge, py::arg("other"));
    cls.def("removeScore", &ScoreCollection::removeScore, py::arg("scoreIdx"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("findMelodyPatternDataFrame",
            [](const ScoreCollection& collection, const std::vector<Note>& melodyPattern,
               float totalIntervalsSimilarityThreshold, float totalRhythmSimilarityThreshold,
               const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
               const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
               const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
               const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
               const std::function<float(float, float)>& totalSimilarityCallback) {
                
                auto results = collection.findMelodyPattern(melodyPattern, totalIntervalsSimilarityThreshold,
                                                            totalRhythmSimilarityThreshold,
                                                            intervalsSimilarityCallback, rhythmSimilarityCallback,
                                                            totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                                                            totalSimilarityCallback);

                // Converte para uma lista de dicionários para compatibilidade com Pandas
                std::vector<py::dict> records;
                for (const auto& row : results) {
                    records.push_back(py::dict("filename"_a = std::get<0>(row),
                                           "composerName"_a = std::get<1>(row),
                                           "scoreTitle"_a = std::get<2>(row),
                                           "partName"_a = std::get<3>(row),
                                           "measureId"_a = std::get<4>(row),
                                           "staveId"_a = std::get<5>(row),
                                           "writtenClefKey"_a = std::get<6>(row),
                                           "transposeInterval"_a = std::get<7>(row),
                                           "segmentWrittenPitch"_a = std::get<8>(row),
                                           "semitonesDiff"_a = std::get<9>(row),
                                           "rhythmDiff"_a = std::get<10>(row),
                                           "totalIntervalSimilarity"_a = std::get<11>(row),
                                           "totalRhythmSimilarity"_a = std::get<12>(row),
                                           "totalSimilarity"_a = std::get<13>(row)));
                }

                py::object pandas = py::module_::import("pandas");
                auto df = pandas.attr("DataFrame")(records);

                // Resetando o índice e ordenando pelo título da partitura
                df.attr("sort_values")("by"_a = "scoreTitle", "inplace"_a = true);
                df.attr("reset_index")("drop"_a = true, "inplace"_a = true);

                return df;
            },
            py::arg("melodyPattern"),
            py::arg("totalIntervalsSimilarityThreshold") = 0.5f,
            py::arg("totalRhythmSimilarityThreshold") = 0.5f,
            py::arg("intervalsSimilarityCallback") = nullptr,
            py::arg("rhythmSimilarityCallback") = nullptr,
            py::arg("totalIntervalSimilarityCallback") = nullptr,
            py::arg("totalRhythmSimilarityCallback") = nullptr,
            py::arg("totalSimilarityCallback") = nullptr);

    // Wrapper para a segunda versão de findMelodyPatternDataFrame, que aceita múltiplos padrões
    cls.def("findMelodyPatternDataFrame",
            [](const ScoreCollection& collection, const std::vector<std::vector<Note>>& melodyPatterns,
               float totalIntervalsSimilarityThreshold, float totalRhythmSimilarityThreshold,
               const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
               const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
               const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
               const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
               const std::function<float(float, float)>& totalSimilarityCallback) {

                auto allResults = collection.findMelodyPattern(melodyPatterns, totalIntervalsSimilarityThreshold,
                                                               totalRhythmSimilarityThreshold,
                                                               intervalsSimilarityCallback, rhythmSimilarityCallback,
                                                               totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                                                               totalSimilarityCallback);

                // Converte para uma lista de dicionários para compatibilidade com Pandas
                py::object pandas = py::module_::import("pandas");
                std::vector<py::object> dataframes;
                
                for (const auto& table : allResults) {
                    std::vector<py::dict> records;
                    for (const auto& row : table) {
                        records.push_back(py::dict(
                                            "patternIdx"_a = std::get<0>(row),
                                            "filename"_a = std::get<1>(row),
                                            "composerName"_a = std::get<2>(row),
                                            "scoreTitle"_a = std::get<3>(row),
                                            "partName"_a = std::get<4>(row),
                                            "measureId"_a = std::get<5>(row),
                                            "staveId"_a = std::get<6>(row),
                                            "writtenClefKey"_a = std::get<7>(row),
                                            "transposeInterval"_a = std::get<8>(row),
                                            "segmentWrittenPitch"_a = std::get<9>(row),
                                            "semitonesDiff"_a = std::get<10>(row),
                                            "rhythmDiff"_a = std::get<11>(row),
                                            "totalIntervalSimilarity"_a = std::get<12>(row),
                                            "totalRhythmSimilarity"_a = std::get<13>(row),
                                            "totalSimilarity"_a = std::get<14>(row)));
                    }
                    dataframes.push_back(pandas.attr("DataFrame")(records));
                }
                
                // Concatena, reseta o índice e ordena pelo título da partitura
                auto final_df = pandas.attr("concat")(dataframes, "ignore_index"_a = true);
                
                final_df.attr("sort_values")("by"_a = "scoreTitle", "inplace"_a = true);
                final_df.attr("reset_index")("drop"_a = true, "inplace"_a = true);

                return final_df;
            },
            py::arg("melodyPatterns"),
            py::arg("totalIntervalsSimilarityThreshold") = 0.5f,
            py::arg("totalRhythmSimilarityThreshold") = 0.5f,
            py::arg("intervalsSimilarityCallback") = nullptr,
            py::arg("rhythmSimilarityCallback") = nullptr,
            py::arg("totalIntervalSimilarityCallback") = nullptr,
            py::arg("totalRhythmSimilarityCallback") = nullptr,
            py::arg("totalSimilarityCallback") = nullptr);

    // Default Python 'print' function:
    cls.def("__repr__", [](const ScoreCollection& scoreCollection) {
        return "<ScoreCollection - " + std::to_string(scoreCollection.getNumScores()) + " scores>";
    });

    cls.def("__hash__", [](const ScoreCollection& scoreCollection) {
        std::string temp;
        for (const auto& dir : scoreCollection.getDirectoriesPaths()) {
            temp += dir;
        }
        return std::hash<std::string>{}(temp);
    });

    cls.def("__sizeof__",
            [](const ScoreCollection& scoreCollection) { return sizeof(scoreCollection); });

    cls.def(py::self + py::self);
}
