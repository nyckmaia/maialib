#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/clef.h"
#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;

void MeasureClass(const py::module& m) {
    m.doc() = "Measure class binding";

    // bindings to Measure class
    py::class_<Measure> cls(m, "Measure");
    cls.def(py::init<const int, const int>(), py::arg("numStaves") = 1,
            py::arg("divisionsPerQuarterNote") = 256);

    cls.def("setKey", &Measure::setKey, py::arg("fifthCircle"), py::arg("isMajorMode") = true);
    cls.def("getKey", &Measure::getKey);
    cls.def("getKeyName", &Measure::getKeyName);

    cls.def("clear", &Measure::clear);

    cls.def("setNumber", &Measure::setNumber, py::arg("measureNumber"));
    cls.def("setKeySignature", &Measure::setKeySignature, py::arg("fifthCircle"),
            py::arg("isMajorMode") = true);
    cls.def("setTimeSignature", &Measure::setTimeSignature, py::arg("upper"), py::arg("lower"));
    cls.def("setMetronome", &Measure::setMetronome, py::arg("bpm"),
            py::arg("rhythmFigure") = RhythmFigure::QUARTER);

    cls.def("setKeyMode", &Measure::setKeyMode, py::arg("isMajorMode"));
    cls.def("setIsKeySignatureChanged", &Measure::setIsKeySignatureChanged,
            py::arg("isKeySignatureChanged") = false);
    cls.def("setIsTimeSignatureChanged", &Measure::setIsTimeSignatureChanged,
            py::arg("isTimeSignatureChanged") = false);
    //     cls.def("setIsClefChanged", &Measure::setIsClefChanged, py::arg("isClefChanged") =
    //     false);
    cls.def("setIsMetronomeChanged", &Measure::setIsMetronomeChanged,
            py::arg("isMetronomeChanged") = false);
    cls.def("setNumStaves", &Measure::setNumStaves, py::arg("numStaves"));
    cls.def("setIsDivisionsPerQuarterNoteChanged", &Measure::setIsDivisionsPerQuarterNoteChanged,
            py::arg("isDivisionsPerQuarterNoteChanged") = false);

    cls.def("addNote", py::overload_cast<const Note&, const int, int>(&Measure::addNote),
            py::arg("note"), py::arg("staveId") = 0, py::arg("position") = -1,
            "Add a single Note object");
    cls.def("addNote",
            py::overload_cast<const std::vector<Note>&, const int, int>(&Measure::addNote),
            py::arg("noteVec"), py::arg("staveId") = 0, py::arg("position") = -1,
            "Add a Note object vector");
    cls.def("addNote", py::overload_cast<const std::string&, const int, int>(&Measure::addNote),
            py::arg("pitchClass"), py::arg("staveId") = 0, py::arg("position") = -1,
            "Create a new Note Obj and add it");
    cls.def("addNote",
            py::overload_cast<const std::vector<std::string>&, const int, int>(&Measure::addNote),
            py::arg("pitchClassVec"), py::arg("staveId") = 0, py::arg("position") = -1,
            "Create a new Note vector and add it");

    cls.def("removeNote", &Measure::removeNote, py::arg("noteId"), py::arg("staveId") = 0);

    cls.def("getClef", py::overload_cast<const int>(&Measure::getClef), py::arg("clefId") = 0,
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getClef", py::overload_cast<const int>(&Measure::getClef, py::const_),
            py::arg("clefId") = 0, py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getBarlineLeft", py::overload_cast<>(&Measure::getBarlineLeft),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBarlineLeft", py::overload_cast<>(&Measure::getBarlineLeft, py::const_),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getBarlineRight", py::overload_cast<>(&Measure::getBarlineRight),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBarlineRight", py::overload_cast<>(&Measure::getBarlineRight, py::const_),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("setRepeatStart", &Measure::setRepeatStart);
    cls.def("setRepeatEnd", &Measure::setRepeatEnd);

    cls.def("removeRepeatStart", &Measure::removeRepeatStart);
    cls.def("removeRepeatEnd", &Measure::removeRepeatEnd);

    cls.def("getNumStaves", &Measure::getNumStaves);
    cls.def("getNumber", &Measure::getNumber);

    cls.def("getQuarterDuration", &Measure::getQuarterDuration);
    cls.def("getFilledQuarterDuration", &Measure::getFilledQuarterDuration, py::arg("staveId") = 0);
    cls.def("getFreeQuarterDuration", &Measure::getFreeQuarterDuration, py::arg("staveId") = 0);
    cls.def("getFractionDuration", &Measure::getFractionDuration);
    cls.def("getDurationTicks", &Measure::getDurationTicks);
    cls.def("getFilledDurationTicks", &Measure::getFilledDurationTicks, py::arg("staveId") = 0);
    cls.def("getFreeDurationTicks", &Measure::getFreeDurationTicks, py::arg("staveId") = 0);

    cls.def("isClefChanged", &Measure::isClefChanged);
    cls.def("timeSignatureChanged", &Measure::timeSignatureChanged);
    cls.def("keySignatureChanged", &Measure::keySignatureChanged);
    cls.def("metronomeChanged", &Measure::metronomeChanged);
    cls.def("divisionsPerQuarterNoteChanged", &Measure::divisionsPerQuarterNoteChanged);
    cls.def("isMajorKeyMode", &Measure::isMajorKeyMode);

    cls.def("getNote", py::overload_cast<const int, const int>(&Measure::getNote),
            py::arg("noteId"), py::arg("staveId") = 0);
    cls.def("getNote", py::overload_cast<const int, const int>(&Measure::getNote, py::const_),
            py::arg("noteId"), py::arg("staveId") = 0, py::return_value_policy::reference_internal);

    cls.def("getNoteOn", py::overload_cast<const int, const int>(&Measure::getNoteOn),
            py::arg("noteOnId"), py::arg("staveId") = 0);
    cls.def("getNoteOn", py::overload_cast<const int, const int>(&Measure::getNoteOn, py::const_),
            py::arg("noteOnId"), py::arg("staveId") = 0,
            py::return_value_policy::reference_internal);

    cls.def("getNoteOff", py::overload_cast<const int, const int>(&Measure::getNoteOff),
            py::arg("noteOffId"), py::arg("staveId") = 0);
    cls.def("getNoteOff", py::overload_cast<const int, const int>(&Measure::getNoteOff, py::const_),
            py::arg("noteOffId"), py::arg("staveId") = 0,
            py::return_value_policy::reference_internal);

    cls.def("getNumNotesOn", py::overload_cast<>(&Measure::getNumNotesOn, py::const_));
    cls.def("getNumNotesOn", py::overload_cast<const int>(&Measure::getNumNotesOn, py::const_),
            py::arg("staveId"));

    cls.def("getNumNotesOff", py::overload_cast<>(&Measure::getNumNotesOff, py::const_));
    cls.def("getNumNotesOff", py::overload_cast<const int>(&Measure::getNumNotesOff, py::const_),
            py::arg("staveId"));

    cls.def("getNumNotes", py::overload_cast<>(&Measure::getNumNotes, py::const_));
    cls.def("getNumNotes", py::overload_cast<const int>(&Measure::getNumNotes, py::const_),
            py::arg("staveId"));

    cls.def("getFifthCircle", &Measure::getFifthCircle);
    cls.def("getKeySignature", &Measure::getKeySignature);
    cls.def("getTimeSignature", &Measure::getTimeSignature);
    cls.def("getTimeMetronome", &Measure::getMetronome);

    cls.def("info", &Measure::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("isEmpty", &Measure::isEmpty);
    cls.def("getEmptyDurationTicks", &Measure::getEmptyDurationTicks);
    cls.def("setDivisionsPerQuarterNote", &Measure::setDivisionsPerQuarterNote);
    cls.def("getDivisionsPerQuarterNote", &Measure::getDivisionsPerQuarterNote);

    cls.def("toXML", &Measure::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2);
    cls.def("toJSON", &Measure::toJSON);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Measure& measure) {
        return "<Measure " + std::to_string(measure.getNumber()) + ">";
    });

    cls.def("__hash__",
            [](const Measure& measure) { return std::hash<std::string>{}(measure.toXML()); });

    cls.def("__sizeof__", [](const Measure& measure) { return sizeof(measure); });
}
