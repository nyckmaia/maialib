#include "part.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void PartClass(py::module &m) {

    m.doc() = "Part class binding";

    // bindings to Part class
    py::class_<Part> cls(m, "Part");
    cls.def(py::init<const std::string&, const int, const bool, const int>(),
            py::arg("partName"),
            py::arg("numStaves") = 1,
            py::arg("isPitched") = true,
            py::arg("divisionsPerQuarterNote") = 256);

    cls.def("clear", &Part::clear);
    cls.def("getName", &Part::getName);
    cls.def("getShortName", &Part::getShortName);

    cls.def("addMidiUnpitched", &Part::addMidiUnpitched);
    cls.def("getMidiUnpitched", &Part::getMidiUnpitched);

    cls.def("addMeasure", &Part::addMeasure, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeMeasure", &Part::removeMeasure);

    cls.def("getMeasure", py::overload_cast<const int>(&Part::getMeasure),
            py::arg("measureId"),
            py::return_value_policy::reference_internal);
    cls.def("getMeasure", py::overload_cast<const int>(&Part::getMeasure, py::const_),
            py::arg("measureId"),
            py::return_value_policy::reference_internal);

    cls.def("getNumMeasures", &Part::getNumMeasures);

    cls.def("setNumStaves", &Part::setNumStaves);
    cls.def("addStaves", &Part::addStaves, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeStave", &Part::removeStave);
    cls.def("getNumStaves", &Part::getNumStaves);

    cls.def("getNumNotes", &Part::getNumNotes);
    cls.def("setShortName", &Part::setShortName);

    cls.def("info", &Part::info, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("setIsPitched", &Part::setIsPitched);
    cls.def("setStaffLines", &Part::setStaffLines);
    cls.def("isPitched", &Part::isPitched);
    cls.def("getStaffLines", &Part::getStaffLines);

    cls.def("append", py::overload_cast<const std::variant<Note, Chord>&, const int, const int>(&Part::append),
            py::arg("obj"),
            py::arg("position") = -1,
            py::arg("staveId") = 0);

    cls.def("append", py::overload_cast<const std::vector<std::variant<Note, Chord>>&, const int, const int>(&Part::append),
            py::arg("objs"),
            py::arg("position") = -1,
            py::arg("staveId") = 0);

    cls.def("toXML", &Part::toXML,
        py::arg("instrumentId") = 1,
        py::arg("identSize") = 2);
    cls.def("toJSON", &Part::toJSON);
}
#endif