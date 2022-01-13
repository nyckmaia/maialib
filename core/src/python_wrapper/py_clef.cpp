#include "clef.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void ClefClass(py::module &m) {

    m.doc() = "Clef class binding";

    // bindings to Clef class
    py::class_<Clef> cls(m, "Clef");
    cls.def(py::init<const std::string&, int>(),
            py::arg("sign") = "G",
            py::arg("line") = 2);

    cls.def("getLine", &Clef::getLine);
    cls.def("setLine", &Clef::setLine);

    cls.def("getSign", &Clef::getSign);
    cls.def("setSign", &Clef::setSign);
}
#endif
