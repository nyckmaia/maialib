#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/clef.h"
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void ClefClass(const py::module& m) {
    py::enum_<ClefSign>(m, "ClefSign")
        .value("G", ClefSign::G)
        .value("F", ClefSign::F)
        .value("C", ClefSign::C)
        .value("P", ClefSign::PERCUSSION);

    m.doc() = "Clef class binding";

    // bindings to Clef class
    py::class_<Clef> cls(m, "Clef");
    cls.def(py::init<const ClefSign, int>(), py::arg("sign") = ClefSign::G, py::arg("line") = -1);

    cls.def("getLine", &Clef::getLine);
    cls.def("setLine", &Clef::setLine, py::arg("line"));

    cls.def("getSign", &Clef::getSign);
    cls.def("setSign", &Clef::setSign, py::arg("sign"));

    cls.def("toXML", &Clef::toXML, py::arg("clefNumber") = -1, py::arg("identSize") = 2);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Clef& clef) { return "<Clef " + clef.getClefSignStr() + ">"; });

    cls.def("__hash__", [](const Clef& clef) {
        return std::hash<std::string>{}(clef.getClefSignStr() + std::to_string(clef.getLine()));
    });

    cls.def("__sizeof__", [](const Clef& clef) { return sizeof(clef); });
}
