#include "score_collection.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

namespace py = pybind11;

void ScoreCollectionClass(py::module &m) {

    m.doc() = "ScoreCollection class binding";

    // bindings to ScoreCollection class
    py::class_<ScoreCollection> cls(m, "ScoreCollection");
    cls.def(py::init<const std::string&, const std::string&>(), 
        py::arg("collectionName"), 
        py::arg("directoryPath") = std::string(),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getName", &ScoreCollection::getName);
    cls.def("getScores", &ScoreCollection::getScores, py::return_value_policy::reference_internal);
    cls.def("getNumScores", &ScoreCollection::getNumScores);
    cls.def("getDirectoryPath", &ScoreCollection::getDirectoryPath);

    cls.def("setName", &ScoreCollection::setName, py::arg("collectionName"));
    cls.def("setDirectoryPath", &ScoreCollection::setDirectoryPath, py::arg("directoryPath"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    
    cls.def("loadCollectionFiles", &ScoreCollection::loadCollectionFiles,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &ScoreCollection::clear);
    cls.def("isEmpty", &ScoreCollection::isEmpty);
    cls.def("addScore", &ScoreCollection::addScore, py::arg("score"));
    cls.def("removeScore", &ScoreCollection::removeScore, py::arg("scoreIdx"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    // Default Python 'print' function:
    cls.def("__repr__", [](const ScoreCollection& scoreCollection) {
        return "<ScoreCollection '" + scoreCollection.getName() + "'>";
    });

    cls.def("__hash__", [](const ScoreCollection& scoreCollection) {
        return std::hash<std::string>{}(scoreCollection.getName() + scoreCollection.getDirectoryPath());
    });

    cls.def("__sizeof__", [](const ScoreCollection& scoreCollection) {
        return sizeof(scoreCollection);
    });
}
#endif
