#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/time-signature.h"
namespace py = pybind11;

void TimeSignatureClass(const py::module& m) {
    m.doc() = "TimeSignature class binding";

    // bindings to TimeSignature class
    py::class_<TimeSignature> cls(m, "TimeSignature");
    cls.def(py::init<const int, const int>(), py::arg("timeUpper") = 4, py::arg("timeLower") = 4);

    cls.def("getUpperValue", &TimeSignature::getUpperValue);
    cls.def("getLowerValue", &TimeSignature::getLowerValue);
    cls.def("setUpperValue", &TimeSignature::setUpperValue, py::arg("timeUpper"));
    cls.def("setLowerValue", &TimeSignature::setLowerValue, py::arg("timeLower"));
    cls.def("getMetric", &TimeSignature::getMetric);
}