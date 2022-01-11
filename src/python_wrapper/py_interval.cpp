#include "interval.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void IntervalClass(py::module &m) {

    m.doc() = "Interval class binding";

    // bindings to Interval class
    py::class_<Interval> cls(m, "Interval");
    cls.def(py::init<const std::string&, const std::string&>(),
            py::arg("pitch_A"),
            py::arg("pitch_B"));

    // Overloaded constructor for rests
    cls.def(py::init<const Note&, const Note&>(),
            py::arg("note_A"),
            py::arg("note_B"));

    cls.def("getName", &Interval::getName);
    cls.def("getNumSemitones", &Interval::getNumSemitones,
            py::arg("absoluteValue") = false);

    cls.def("getNumOctaves", &Interval::getNumOctaves,
            py::arg("absoluteValue") = false);

    cls.def("getDiatonicInterval", &Interval::getDiatonicInterval,
            py::arg("absoluteValue") = false);

    cls.def("getDiatonicSteps", &Interval::getDiatonicSteps,
            py::arg("absoluteValue") = false);

    cls.def("getNotes", &Interval::getNotes);
    cls.def("isAscendant", &Interval::isAscendant);
    cls.def("isDescendant", &Interval::isDescendant);
    cls.def("isTonal", &Interval::isTonal);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Interval& interval) { return interval.getName(); });

}
#endif
