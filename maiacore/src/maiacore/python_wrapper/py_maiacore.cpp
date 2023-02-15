#include <pybind11/pybind11.h>

#include "maiacore/config.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

void ScoreCollectionClass(const py::module &);
void ScoreClass(const py::module &);
void PartClass(const py::module &);
void ClefClass(const py::module &);
void MeasureClass(const py::module &);
void NoteClass(const py::module &);
void ChordClass(const py::module &);
void BarlineClass(const py::module &);
void IntervalClass(const py::module &);
void HelperClass(const py::module &);
void Constants(const py::module &);
void Config(py::module &);

PYBIND11_MODULE(maiacore, m) {
    m.doc() = "This is a Python binding of C++ Maia Library";

    Constants(m);
    NoteClass(m);
    ChordClass(m);
    ScoreClass(m);
    PartClass(m);
    ClefClass(m);
    BarlineClass(m);
    MeasureClass(m);
    IntervalClass(m);
    HelperClass(m);
    ScoreCollectionClass(m);
    Config(m);

#ifdef MAIALIB_VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(MAIALIB_VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

namespace py = pybind11;

void Config(py::module &m) {
    m.def("getTuningSystem", &getTuningSystem);
    m.def("setTuningSystem", &setTuningSystem, py::arg("tuningSystem"));
}