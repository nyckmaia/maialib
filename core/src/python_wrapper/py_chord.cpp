#include "chord.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/operators.h>
#include <pybind11/functional.h>

namespace py = pybind11;

void ChordClass(py::module &m) {

    m.doc() = "Chord class binding";

    // bindings to Chord class
    py::class_<Chord> cls(m, "Chord");
    cls.def(py::init<>(), py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<Note>&>(), py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<std::string>&>(), py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &Chord::clear);

    cls.def("addNote", py::overload_cast<const Note&>(&Chord::addNote),
            py::arg("note"));
    cls.def("addNote", py::overload_cast<const std::string&>(&Chord::addNote),
            py::arg("pitch"));

    cls.def("removeTopNote", &Chord::removeTopNote, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("insertNote", &Chord::insertNote, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeNote", &Chord::removeNote, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setDurationTicks", &Chord::setDurationTicks, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("inversion", &Chord::inversion, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("transpose", &Chord::transpose, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("removeDuplicateNotes", &Chord::removeDuplicateNotes);
    cls.def("getDuration", &Chord::getDuration);

    cls.def("getDurationTicks", &Chord::getDurationTicks);
    cls.def("getNote", py::overload_cast<size_t>(&Chord::getNote),
            py::arg("noteIndex"));
    cls.def("getNote", py::overload_cast<size_t>(&Chord::getNote),
            py::arg("noteIndex"),
            py::return_value_policy::reference_internal);

    cls.def("getRoot", &Chord::getRoot, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getName", &Chord::getName,
            py::arg("useEnharmonicName") = true,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBassNote", &Chord::getBassNote);

    cls.def("haveMinorThird", &Chord::haveMinorThird);
    cls.def("haveMajorThird", &Chord::haveMajorThird);
    cls.def("haveDiminishedFifth", &Chord::haveDiminishedFifth);
    cls.def("havePerfectFifth", &Chord::havePerfectFifth);
    cls.def("haveAugmentedFifth", &Chord::haveAugmentedFifth);
    cls.def("haveDiminishedSeventh", &Chord::haveDiminishedSeventh);
    cls.def("haveMinorSeventh", &Chord::haveMinorSeventh);
    cls.def("haveMajorSeventh", &Chord::haveMajorSeventh);
    cls.def("haveMinorNinth", &Chord::haveMinorNinth);
    cls.def("haveMajorNinth", &Chord::haveMajorNinth);
    cls.def("havePerfectEleventh", &Chord::havePerfectEleventh);
    cls.def("haveSharpEleventh", &Chord::haveSharpEleventh);
    cls.def("haveMinorThirdteenth", &Chord::haveMinorThirdteenth);
    cls.def("haveMajorThirdteenth", &Chord::haveMajorThirdteenth);

    cls.def("isMajorChord", &Chord::isMajorChord);
    cls.def("isMinorChord", &Chord::isMinorChord);
    cls.def("isAugmentedChord", &Chord::isAugmentedChord);
    cls.def("isDiminishedChord", &Chord::isDiminishedChord);
    cls.def("isDominantSeventhChord", &Chord::isDominantSeventhChord);
    cls.def("isHalfDiminishedChord", &Chord::isHalfDiminishedChord);

    cls.def("isTonal", &Chord::isTonal,
        py::arg("model") = nullptr);

    cls.def("getMIDIIntervals", &Chord::getMIDIIntervals);
    cls.def("getIntervalNames", &Chord::getIntervalNames);
    cls.def("getQuarterDuration", &Chord::getQuarterDuration);
    
    cls.def("size", &Chord::size);
    cls.def("stackSize", &Chord::stackSize);
    cls.def("info", &Chord::info, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("print", &Chord::print, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("printStack", &Chord::printStack, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getStackedChord", &Chord::getStackedChord);

    cls.def("sortNotes", &Chord::sortNotes);

    cls.def(py::self == py::self);
    cls.def(py::self != py::self);
    cls.def(py::self + py::self);

    cls.def("__getitem__", [](Chord& self, const size_t index) { return self[index]; });
    cls.def("__setitem__", [](Chord& self, const size_t index) { return self[index]; });

    // Default Python 'print' function:
    cls.def("__repr__", [](const Chord& chord) {
        const size_t chordSize = chord.size();
        std::string noteNames = "[";

        for (size_t i = 0; i < chordSize-1; i++) {
            noteNames.append(chord[i].getPitch() + ", ");
        }

        // Add the last note without the semicomma in the end
        noteNames.append(chord[chordSize-1].getPitch());

        noteNames.append("]");

        return noteNames;
    });
}
#endif
