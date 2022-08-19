#ifdef PYBIND
#include <pybind11/pybind11.h>

namespace py = pybind11;

// void ScoreCollectionClass(py::module &);
void ScoreClass(py::module &);
void PartClass(py::module &);
void ClefClass(py::module &);
void MeasureClass(py::module &);
void NoteClass(py::module &);
void ChordClass(py::module &);
void BarlineClass(py::module &);
void IntervalClass(py::module &);
void HelperClass(py::module &);
void Constants(py::module &);

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
    // ScoreCollectionClass(m);    
}
#endif
