#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/measure.h"
#include "maiacore/score.h"
#include "nlohmann/json.hpp"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

void ScoreClass(const py::module& m) {
    m.doc() = "Score class binding";

    // bindings to Score class
    py::class_<Score> cls(m, "Score");

    cls.def(py::init<const std::vector<std::string>&, const int>(), py::arg("partsName"),
            py::arg("numMeasures") = 20,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def(py::init<const std::string&>(), py::arg("filePath"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &Score::clear);
    cls.def("addPart", &Score::addPart, py::arg("partName"), py::arg("numStaves") = 1,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removePart", &Score::removePart, py::arg("partId"));
    cls.def("addMeasure", &Score::addMeasure, py::arg("numMeasures"));
    cls.def("removeMeasure", &Score::removeMeasure, py::arg("measureStart"), py::arg("measureEnd"));

    cls.def("getPart", py::overload_cast<const int>(&Score::getPart), py::arg("partId"),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getPart", py::overload_cast<const std::string&>(&Score::getPart), py::arg("partName"),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getNumParts", &Score::getNumParts);
    cls.def("getNumMeasures", &Score::getNumMeasures);
    cls.def("getNumNotes", &Score::getNumNotes);
    cls.def("getPartsNames", &Score::getPartsNames);
    cls.def("getTitle", &Score::getTitle);

    cls.def("getFilePath", &Score::getFilePath);
    cls.def("getFileName", &Score::getFileName);
    cls.def("setTitle", &Score::setTitle, py::arg("scoreTitle"));

    cls.def("getComposerName", &Score::getComposerName);

    cls.def("setComposerName", &Score::setComposerName, py::arg("composerName"));

    cls.def("setKeySignature",
            py::overload_cast<const int, const bool, const int>(&Score::setKeySignature),
            py::arg("fifthCicle"), py::arg("isMajorMode") = true, py::arg("measureId") = 0);

    cls.def("setKeySignature",
            py::overload_cast<const std::string&, const int>(&Score::setKeySignature),
            py::arg("key"), py::arg("measureId") = 0);

    cls.def("setTimeSignature", &Score::setTimeSignature, py::arg("timeUpper"),
            py::arg("timeLower"), py::arg("measureId") = -1);

    cls.def("setMetronomeMark", &Score::setMetronomeMark, py::arg("bpm"),
            py::arg("rhythmFigure") = RhythmFigure::QUARTER, py::arg("measureStart") = 0);

    cls.def("haveAnacrusisMeasure", &Score::haveAnacrusisMeasure);
    cls.def("toXML", &Score::toXML, py::arg("identSize") = 2);
    cls.def("toJSON", &Score::toJSON);
    cls.def("toFile", &Score::toFile, py::arg("fileName"), py::arg("compressedXML") = false,
            py::arg("identSize") = 2);
    cls.def("info", &Score::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("isValid", &Score::isValid,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("xPathCountNodes", &Score::xPathCountNodes, py::arg("xPath"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getPartName", &Score::getPartName, py::arg("partId"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getPartIndex", &Score::getPartIndex, py::arg("partName"), py::arg("index"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    //     cls.def("selectNotes", &Score::selectNotes, py::arg("config"),
    //             py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //     cls.def("countNotes", &Score::countNotes, py::arg("config"),
    //             py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setRepeat", &Score::setRepeat, py::arg("measureStart"), py::arg("measureEnd") = -1);

    cls.def(
        "findMelodyPatternDataFrame",
        [](Score& score, const std::vector<Note>& melodyPattern,
           const float totalIntervalsSimilarityThreshold,
           const float totalRhythmSimilarityThreshold,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {
            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"partName",
                                                "measureId",
                                                "staveId",
                                                "writtenClefKey",
                                                "transposeInterval",
                                                "segmentWrittenPitch",
                                                "semitonesDiff",
                                                "rhythmDiff",
                                                "totalIntervalSimilarity",
                                                "totalRhythmSimilarity",
                                                "totalSimilarity"};

            // Fill DataFrame with records and columns
            py::object df = FromRecords(
                score.findMelodyPattern(melodyPattern, totalIntervalsSimilarityThreshold,
                                        totalRhythmSimilarityThreshold, intervalsSimilarityCallback,
                                        rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                                        totalRhythmSimilarityCallback, totalSimilarityCallback),
                "columns"_a = columns);

            return df;
        },
        py::arg("melodyPattern"), py::arg("totalIntervalsSimilarityThreshold") = 0.5f,
        py::arg("totalRhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    // Overload para multiplos padrões em paralelo
    cls.def(
        "findMelodyPatternDataFrame",
        [](Score& score, const std::vector<std::vector<Note>>& melodyPatterns,
           float totalIntervalsSimilarityThreshold, float totalRhythmSimilarityThreshold,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)> intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)> rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {

            const auto& results = score.findMelodyPattern(melodyPatterns, totalIntervalsSimilarityThreshold,
                                        totalRhythmSimilarityThreshold, intervalsSimilarityCallback,
                                        rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                                        totalRhythmSimilarityCallback, totalSimilarityCallback);

            // Converte os resultados para DataFrames no contexto principal (com o GIL adquirido)
            py::gil_scoped_acquire acquire;
            py::object Pandas = py::module_::import("pandas");
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");
            std::vector<py::object> dataframes;
            
            // Definindo as colunas do DataFrame
            std::vector<std::string> columns = {"partName",
                                                "measureId",
                                                "staveId",
                                                "writtenClefKey",
                                                "transposeInterval",
                                                "segmentWrittenPitch",
                                                "semitonesDiff",
                                                "rhythmDiff",
                                                "totalIntervalSimilarity",
                                                "totalRhythmSimilarity",
                                                "totalSimilarity"};

            for (size_t idx = 0; idx < results.size(); ++idx) {
                py::object df = FromRecords(results[idx], "columns"_a = columns);
                int num_columns = df.attr("shape").cast<py::tuple>()[1].cast<int>();
                df.attr("insert")(num_columns, "patternIdx", idx);
                dataframes.push_back(df);
            }

            // Concatena todos os DataFrames processados com sucesso
            if (!dataframes.empty()) {
                // std::cout << "Concatenando DataFrames..." << std::endl;
                py::object result_df = Pandas.attr("concat")(dataframes, "ignore_index"_a = true);
                result_df.attr("sort_values")("by"_a = "measureId", "ascending"_a = true, "inplace"_a = true);
                return result_df;
            } else {
                throw std::runtime_error("Nenhum DataFrame foi concatenado devido a erro de memória ou outro problema.");
            }
        },
        py::arg("melodyPatterns"),
        py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr
    );

//     cls.def(
//     "findAnyMelodyPatternDataFrame",
//     [](Score& score, const int patternNumNotes,
//        float totalIntervalsSimilarityThreshold, float totalRhythmSimilarityThreshold,
//        const std::function<std::vector<float>(const std::vector<Note>&,
//                                               const std::vector<Note>&)> intervalsSimilarityCallback,
//        const std::function<std::vector<float>(const std::vector<Note>&,
//                                               const std::vector<Note>&)> rhythmSimilarityCallback,
//        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
//        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
//        const std::function<float(float, float)> totalSimilarityCallback) {

//         const auto& results = score.findAnyMelodyPattern(patternNumNotes, totalIntervalsSimilarityThreshold,
//                                     totalRhythmSimilarityThreshold, intervalsSimilarityCallback,
//                                     rhythmSimilarityCallback, totalIntervalSimilarityCallback,
//                                     totalRhythmSimilarityCallback, totalSimilarityCallback);

//         // Converte os resultados para DataFrames no contexto principal (com o GIL adquirido)
//         py::gil_scoped_acquire acquire;
//         py::object Pandas = py::module_::import("pandas");
//         py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");
//         std::vector<py::object> dataframes;

//         // Definindo as colunas do DataFrame
//         std::vector<std::string> columns = {"partName",
//                                             "measureId",
//                                             "staveId",
//                                             "writtenClefKey",
//                                             "transposeInterval",
//                                             "segmentWrittenPitch",
//                                             "semitonesDiff",
//                                             "rhythmDiff",
//                                             "totalIntervalSimilarity",
//                                             "totalRhythmSimilarity",
//                                             "totalSimilarity"};

//         for (size_t idx = 0; idx < results.size(); ++idx) {
//             py::object df = FromRecords(results[idx], "columns"_a = columns);
            
//             // Verifica se o DataFrame possui dados antes de adicioná-lo
//             if (df.attr("empty").cast<bool>()) {
//                 continue;  // Pula DataFrames vazios
//             }

//             // Adiciona coluna "patternIdx" para identificar o índice do padrão
//             int num_columns = df.attr("shape").cast<py::tuple>()[1].cast<int>();
//             df.attr("insert")(num_columns, "patternIdx", idx);

//             dataframes.push_back(df);
//         }

//         // Concatena todos os DataFrames processados com sucesso
//         if (!dataframes.empty()) {
//             py::object result_df = Pandas.attr("concat")(dataframes, "ignore_index"_a = true);

//             // Ordena o DataFrame pelo campo "measureId" antes de aplicar filtros
//             result_df.attr("sort_values")("by"_a = "measureId", "ascending"_a = true, "inplace"_a = true);

//             // Filtra as linhas onde "segmentWrittenPitch" contém apenas "rest"
//             py::object filtered_df = result_df.attr("loc")[
//                 result_df.attr("segmentWrittenPitch").attr("apply")(
//                     py::cpp_function([](const py::object& pitchList) {
//                         auto list = pitchList.cast<std::vector<std::string>>();
//                         return std::any_of(list.begin(), list.end(), [](const std::string& s) { return s != "rest"; });
//                     })
//                 )
//             ];

//             // Reseta o índice do DataFrame final após o filtro
//             filtered_df = filtered_df.attr("reset_index")("drop"_a = true);

//             return filtered_df;
//         } else {
//             throw std::runtime_error("Nenhum DataFrame foi concatenado devido a erro de memória ou outro problema.");
//         }
//     },
//     py::arg("patternNumNotes") = 5,
//     py::arg("intervalSimilarityThreshold") = 1.0f,
//     py::arg("rhythmSimilarityThreshold") = 1.0f,
//     py::arg("intervalsSimilarityCallback") = nullptr,
//     py::arg("rhythmSimilarityCallback") = nullptr,
//     py::arg("totalIntervalSimilarityCallback") = nullptr,
//     py::arg("totalRhythmSimilarityCallback") = nullptr,
//     py::arg("totalSimilarityCallback") = nullptr
// );


    // cls.def("instrumentFragmentation", &Score::instrumentFragmentation,
    //         py::arg("config") = nlohmann::json(),
    //         py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getChords", &Score::getChords, py::arg("config") = nlohmann::json(),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(
        "getChordsDataFrame",
        [](Score& score, nlohmann::json config) {
            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"measure", "floatMeasure", "key", "chord",
                                                "isHomophonic"};

            // Fill DataFrame with records and columns
            py::object df = FromRecords(score.getChords(config), "columns"_a = columns);

            return df;
        },
        py::arg("config") = nlohmann::json(),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("forEachNote", &Score::forEachNote, py::arg("callback"), py::arg("measureStart") = 0,
            py::arg("measureEnd") = -1, py::arg("partNames") = std::vector<std::string>(),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("toDataFrame", [](Score& score) {
        // Import Pandas module
        py::object Pandas = py::module_::import("pandas");

        // Get method 'from_records' from 'DataFrame()' object
        py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

        // Get basic score data
        const int numParts = score.getNumParts();
        const int numMeasures = score.getNumMeasures();
        const int numNotes = score.getNumNotes();

        // Create a temp store object: [PartObj][MeasureObj][NoteObj]
        typedef std::tuple<Part*, Measure*, Note*> DataFrameRow;
        std::vector<DataFrameRow> df_records(numNotes);

        // For each note inside the score
        int rowNumber = 0;
        for (int p = 0; p < numParts; p++) {
            Part& currentPart = score.getPart(p);

            for (int m = 0; m < numMeasures; m++) {
                Measure& currentMeasure = currentPart.getMeasure(m);

                for (int s = 0; s < currentMeasure.getNumStaves(); s++) {
                    const int numNotes = currentMeasure.getNumNotes(s);

                    for (int n = 0; n < numNotes; n++) {
                        Note& currentNote = currentMeasure.getNote(n, s);

                        df_records[rowNumber] =
                            DataFrameRow(&currentPart, &currentMeasure, &currentNote);

                        rowNumber++;
                    }
                }
            }
        }

        // Set DataFrame columns name
        std::vector<std::string> columns = {"Part", "Measure", "Note"};

        // Fill DataFrame with records and columns
        py::object df = FromRecords(df_records, "columns"_a = columns);

        return df;
    });

    // Default Python 'print' function:
    cls.def("__repr__", [](const Score& score) {
        const std::string title = (score.getTitle().empty()) ? "No Title" : score.getTitle();
        return "<Score '" + title + "'>";
    });

    cls.def("__hash__", [](const Score& score) { return std::hash<std::string>{}(score.toXML()); });

    cls.def("__sizeof__", [](const Score& score) { return sizeof(score); });
}
