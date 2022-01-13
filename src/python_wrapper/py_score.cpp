#include "score.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/functional.h>
namespace py = pybind11;

void ScoreClass(py::module &m) {

    m.doc() = "Score class binding";

    // bindings to Score class
    py::class_<Score> cls(m, "Score");
    cls.def(py::init<const std::initializer_list<std::string>&, const int, const int>(),
            py::arg("partsName"),
            py::arg("numMeasures") = 20,
            py::arg("divisionsPerQuarterNote") = 256,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def(py::init<const std::vector<std::string>&, const int, const int>(),
            py::arg("partsName"),
            py::arg("numMeasures") = 20,
            py::arg("divisionsPerQuarterNote") = 256,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def(py::init<const std::string&>(), py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &Score::clear);
    cls.def("loadXMLFile", &Score::loadXMLFile);
    cls.def("addPart", &Score::addPart,
        py::arg("partName"),
        py::arg("numStaves") = 1,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removePart", &Score::removePart);
    cls.def("addMeasure", &Score::addMeasure);
    cls.def("removeMeasure", &Score::removeMeasure);
    cls.def("getPart", &Score::getPart, py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getNumParts", &Score::getNumParts);
    cls.def("getNumMeasures", &Score::getNumMeasures);
    cls.def("getNumNotes", &Score::getNumNotes);
    cls.def("setTitle", &Score::setTitle);
    cls.def("setComposerName", &Score::setComposerName);

    cls.def("setKeySignature", py::overload_cast<const int, const bool, const int>(&Score::setKeySignature),
        py::arg("fifthCicle"),
        py::arg("isMajorMode") = true,
        py::arg("measureId") = 0);

    cls.def("setKeySignature", py::overload_cast<const std::string&, const int>(&Score::setKeySignature),
        py::arg("key"),
        py::arg("measureId") = 0);

    cls.def("setTimeSignature", &Score::setTimeSignature,
        py::arg("timeUpper"),
        py::arg("timeLower"),
        py::arg("measureId") = -1);

    cls.def("setMetronomeMark", &Score::setMetronomeMark,
        py::arg("bpm"),
        py::arg("rhythmFigure") = std::string(),
        py::arg("measureStart") = 0);

    cls.def("toXML", &Score::toXML);
    cls.def("toJSON", &Score::toJSON);
    cls.def("toFile", &Score::toFile,
            py::arg("fileName"),
            py::arg("identSize") = 2);
    cls.def("info", &Score::info, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    // ===== OLD MUSICXML Class methods ===== //
    cls.def("isValid", &Score::isValid, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("xPathCountNodes", &Score::xPathCountNodes, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getPartsName", &Score::getPartsName, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getPartName", &Score::getPartName, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("selectNotes", &Score::selectNotes, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("countNotes", &Score::countNotes, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("findPattern", &Score::findPattern, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("instrumentFragmentation", &Score::instrumentFragmentation, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getPianoRoll", &Score::getPianoRoll, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getChords", &Score::getChords,
        py::arg("config") = nlohmann::json(),
        py::arg("sameAttackNotes") = true,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("forEachNote", &Score::forEachNote,
        py::arg("callback"),
        py::arg("measureStart") = 0,
        py::arg("measureEnd") = -1,
        py::arg("partNames") = std::vector<std::string>(),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
}
#endif
