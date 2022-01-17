#include "measure.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void MeasureClass(py::module &m) {

    m.doc() = "Measure class binding";

    // bindings to Measure class
    py::class_<Measure> cls(m, "Measure");
    cls.def(py::init<const int>(),
        py::arg("numStaves") = 1);

    cls.def("clear", &Measure::clear);

    cls.def("setKeySignature", &Measure::setKeySignature);
    cls.def("setTimeSignature", &Measure::setTimeSignature);
    cls.def("setMetronome", &Measure::setMetronome);

    cls.def("setKeyMode", &Measure::setKeyMode,
        py::arg("keyMode") = "major");
    cls.def("setIsKeySignatureChanged", &Measure::setIsKeySignatureChanged,
        py::arg("isKeySignatureChanged") = false);
    cls.def("setIsTimeSignatureChanged", &Measure::setIsTimeSignatureChanged,
        py::arg("isTimeSignatureChanged") = false);
    cls.def("setIsClefChanged", &Measure::setIsClefChanged,
        py::arg("isClefChanged") = false);
    cls.def("setIsMetronomeChanged", &Measure::setIsMetronomeChanged,
        py::arg("isMetronomeChanged") = false);
    cls.def("setNumStaves", &Measure::setNumStaves);
    
    cls.def("addNote", py::overload_cast<const Note&, const int, int>(&Measure::addNote),
        py::arg("note"), 
        py::arg("staveId") = 0,
        py::arg("position") = -1, "single notes");
    cls.def("addNote", py::overload_cast<const std::vector<Note>&, const int, int>(&Measure::addNote),
        py::arg("noteVec"), 
        py::arg("staveId") = 0,
        py::arg("position") = -1, "multiple notes");
    cls.def("addNote", py::overload_cast<const std::string&, const int, int>(&Measure::addNote),
        py::arg("pitchClass"), 
        py::arg("staveId") = 0,
        py::arg("position") = -1, "call default note constructor");
    cls.def("addNote", py::overload_cast<const std::vector<std::string>&, const int, int>(&Measure::addNote),
        py::arg("pitchClassVec"), 
        py::arg("staveId") = 0,
        py::arg("position") = -1, "call multiple default note constructor");

    cls.def("removeNote", &Measure::removeNote,
        py::arg("noteId"), 
        py::arg("staveId") = 0);

    cls.def("getClef", py::overload_cast<const int>(&Measure::getClef), 
        py::arg("clefId") = 0, 
        py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getClef", py::overload_cast<const int>(&Measure::getClef, py::const_), 
        py::arg("clefId") = 0,
        py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getBarlineLeft", py::overload_cast<>(&Measure::getBarlineLeft),
        py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBarlineLeft", py::overload_cast<>(&Measure::getBarlineLeft, py::const_),
        py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getBarlineRight", py::overload_cast<>(&Measure::getBarlineRight),
        py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBarlineRight", py::overload_cast<>(&Measure::getBarlineRight, py::const_),
        py::return_value_policy::reference_internal, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("setRepeatStart", &Measure::setRepeatStart);
    cls.def("setRepeatEnd", &Measure::setRepeatEnd);

    cls.def("removeRepeatStart", &Measure::removeRepeatStart);
    cls.def("removeRepeatEnd", &Measure::removeRepeatEnd);

    cls.def("getNumStaves", &Measure::getNumStaves);

    cls.def("clefChanged", &Measure::clefChanged);
    cls.def("timeSignatureChanged", &Measure::timeSignatureChanged);
    cls.def("keySignatureChanged", &Measure::keySignatureChanged);
    cls.def("metronomeChanged", &Measure::metronomeChanged);
    cls.def("isMajorKeyMode", &Measure::isMajorKeyMode);

    cls.def("getNote", py::overload_cast<const int, const int>(&Measure::getNote), 
        py::arg("noteId"),
        py::arg("staveId") = 0);
    cls.def("getNote", py::overload_cast<const int, const int>(&Measure::getNote, py::const_), 
        py::arg("noteId"),
        py::arg("staveId") = 0,
        py::return_value_policy::reference_internal);

    cls.def("getNoteOn", py::overload_cast<const int, const int>(&Measure::getNoteOn), 
        py::arg("noteOnId"),
        py::arg("staveId") = 0);
    cls.def("getNoteOn", py::overload_cast<const int, const int>(&Measure::getNoteOn, py::const_), 
        py::arg("noteOnId"),
        py::arg("staveId") = 0,
        py::return_value_policy::reference_internal);

    cls.def("getNoteOff", py::overload_cast<const int, const int>(&Measure::getNoteOff), 
        py::arg("noteOffId"),
        py::arg("staveId") = 0);
    cls.def("getNoteOff", py::overload_cast<const int, const int>(&Measure::getNoteOff, py::const_), 
        py::arg("noteOffId"),
        py::arg("staveId") = 0,
        py::return_value_policy::reference_internal);
    
    cls.def("getNumNotesOn", py::overload_cast<>(&Measure::getNumNotesOn, py::const_));
    cls.def("getNumNotesOn", py::overload_cast<const int>(&Measure::getNumNotesOn, py::const_),
        py::arg("staveId") = 0);

    cls.def("getNumNotesOff", py::overload_cast<>(&Measure::getNumNotesOff, py::const_));
    cls.def("getNumNotesOff", py::overload_cast<const int>(&Measure::getNumNotesOff, py::const_),
        py::arg("staveId") = 0);

    cls.def("getNumNotes", py::overload_cast<>(&Measure::getNumNotes, py::const_));
    cls.def("getNumNotes", py::overload_cast<const int>(&Measure::getNumNotes, py::const_),
        py::arg("staveId") = 0);
    
    cls.def("getFifthCicle", &Measure::getFifthCicle);
    cls.def("getKeySignature", &Measure::getKeySignature);
    cls.def("getTimeSignature", &Measure::getTimeSignature);
    cls.def("getTimeMetronome", &Measure::getMetronome);

    cls.def("info", &Measure::info, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("empty", &Measure::empty);
    cls.def("getEmptyDurationTicks", &Measure::getEmptyDurationTicks);
    cls.def("setDivisionsPerQuarterNote", &Measure::setDivisionsPerQuarterNote);
    cls.def("getDivisionsPerQuarterNote", &Measure::getDivisionsPerQuarterNote);

    cls.def("toXML", &Measure::toXML,
        py::arg("instrumentId") = 1,
        py::arg("identSize") = 2);
    cls.def("toJSON", &Measure::toJSON);
}

#endif
