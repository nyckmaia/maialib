#include "clef.h"

#ifdef PYBIND

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void ClefClass(const py::module& m) {
    m.doc() = "Clef class binding";

    // bindings to Clef class
    py::class_<Clef> cls(m, "Clef");
    cls.def(py::init<const std::string&, int>(), py::arg("sign") = "G", py::arg("line") = 2);

    cls.def("getLine", &Clef::getLine);
    cls.def("setLine", &Clef::setLine, py::arg("line"));

    cls.def("getSign", &Clef::getSign);
    cls.def("setSign", &Clef::setSign, py::arg("sign"));

    // Default Python 'print' function:
    cls.def("__repr__", [](const Clef& clef) { return "<Clef " + clef.getSign() + ">"; });

    cls.def("__hash__", [](const Clef& clef) {
        return std::hash<std::string>{}(clef.getSign() + std::to_string(clef.getLine()));
    });

    cls.def("__sizeof__", [](const Clef& clef) { return sizeof(clef); });
}
#endif
