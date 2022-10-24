#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "score_collection.h"

namespace py = pybind11;

void ScoreCollectionClass(const py::module& m) {
    m.doc() = "ScoreCollection class binding";

    // bindings to ScoreCollection class
    py::class_<ScoreCollection> cls(m, "ScoreCollection");
    cls.def(py::init<const std::string&, const std::string&>(), py::arg("collectionName"),
            py::arg("directoryPath") = std::string());

    cls.def(py::init<const std::string&, const std::vector<std::string>&>(),
            py::arg("collectionName"), py::arg("directoriesPaths") = std::vector<std::string>());

    cls.def("getDirectoriesPaths", &ScoreCollection::getDirectoriesPaths);
    cls.def("setDirectoriesPaths", &ScoreCollection::setDirectoriesPaths,
            py::arg("directoriesPaths"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getName", &ScoreCollection::getName);
    cls.def("setName", &ScoreCollection::setName, py::arg("collectionName"));

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
    cls.def("merge", &ScoreCollection::merge, py::arg("other"),
            py::arg("mergedCollectionName") = std::string());
    cls.def("removeScore", &ScoreCollection::removeScore, py::arg("scoreIdx"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    // Default Python 'print' function:
    cls.def("__repr__", [](const ScoreCollection& scoreCollection) {
        return "<ScoreCollection '" + scoreCollection.getName() + "'>";
    });

    cls.def("__hash__", [](const ScoreCollection& scoreCollection) {
        std::string temp;
        for (const auto& dir : scoreCollection.getDirectoriesPaths()) {
            temp += dir;
        }
        return std::hash<std::string>{}(scoreCollection.getName() + temp);
    });

    cls.def("__sizeof__",
            [](const ScoreCollection& scoreCollection) { return sizeof(scoreCollection); });

    cls.def(py::self + py::self);
}
