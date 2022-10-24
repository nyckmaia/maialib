#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "part.h"
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void BarlineClass(const py::module& m) {
    m.doc() = "Barline class binding";

    // bindings to Barline class
    py::class_<Barline> cls(m, "Barline");
    cls.def(py::init<>());

    cls.def("clean", &Barline::clean);

    cls.def("getBarStyle", &Barline::getBarStyle);
    cls.def("setBarStyle", &Barline::setBarStyle, py::arg("barStyle"));

    cls.def("getDirection", &Barline::getDirection);
    cls.def("setDirection", &Barline::setDirection, py::arg("direction"));

    cls.def("getLocation", &Barline::getLocation);
    cls.def("setLocation", &Barline::setLocation, py::arg("location"));

    cls.def("setRepeatStart", &Barline::setRepeatStart);
    cls.def("setRepeatEnd", &Barline::setRepeatEnd);

    cls.def("toXML", &Barline::toXML, py::arg("identSize") = 2);

    // Default Python 'print' function:
    cls.def("__repr__",
            [](const Barline& barline) { return "<Barline " + barline.getBarStyle() + ">"; });

    cls.def("__hash__", [](const Barline& barline) {
        return std::hash<std::string>{}(barline.getBarStyle() + barline.getDirection() +
                                        barline.getLocation());
    });

    cls.def("__sizeof__", [](const Barline& barline) { return sizeof(barline); });
}
