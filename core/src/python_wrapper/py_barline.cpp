#include "part.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void BarlineClass(py::module &m) {

    m.doc() = "Barline class binding";

    // bindings to Barline class
    py::class_<Barline> cls(m, "Barline");
    cls.def(py::init<>());

    cls.def("clean", &Barline::clean);

    cls.def("getBarStyle", &Barline::getBarStyle);
    cls.def("setBarStyle", &Barline::setBarStyle);

    cls.def("getDirection", &Barline::getDirection);
    cls.def("setDirection", &Barline::setDirection);

    cls.def("getLocation", &Barline::getLocation);
    cls.def("setLocation", &Barline::setLocation);

    cls.def("setRepeatStart", &Barline::setRepeatStart);
    cls.def("setRepeatEnd", &Barline::setRepeatEnd);

    cls.def("toXML", &Barline::toXML);
}
#endif