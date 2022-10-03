#include "chord.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/operators.h>
#include <pybind11/functional.h>

namespace py = pybind11;
using namespace pybind11::literals;

void ChordClass(py::module &m) {

    m.doc() = "Chord class binding";

    // bindings to Chord class
    py::class_<Chord> cls(m, "Chord");
    cls.def(py::init<>(), py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<Note>&>(), 
        py::arg("notes"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<std::string>&>(), 
        py::arg("pitches"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &Chord::clear);

    cls.def("addNote", py::overload_cast<const Note&>(&Chord::addNote),
            py::arg("note"));
    cls.def("addNote", py::overload_cast<const std::string&>(&Chord::addNote),
            py::arg("pitch"));

    cls.def("removeTopNote", &Chord::removeTopNote, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("insertNote", &Chord::insertNote, 
        py::arg("insertNote"),
        py::arg("positionNote") = 0,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeNote", &Chord::removeNote, 
        py::arg("noteIndex"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setDurationTicks", &Chord::setDurationTicks, 
        py::arg("durationTicks"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("inversion", &Chord::inversion, 
        py::arg("inversionNumber"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("transpose", &Chord::transpose, 
        py::arg("semiTonesNumber"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("transposeStackOnly", &Chord::transposeStackOnly, 
        py::arg("semiTonesNumber"),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("removeDuplicateNotes", &Chord::removeDuplicateNotes);
 
    cls.def("getStackedHeaps", [](Chord& chord, const bool enharmonyNotes)
        {
            const std::vector<HeapData>& heapsData = chord.getStackedHeaps(enharmonyNotes);
            
            using enharDist = std::pair<std::string, int>; // [notePitch, diatonicDistance]
            using dfRow = std::tuple<std::vector<std::string>, int, std::vector<enharDist>, int, std::vector<std::string>, float>;
            std::vector<dfRow> output(heapsData.size());

            int idx = 0;
            for (const auto& heapData : heapsData) {
                const Heap& heap = std::get<0>(heapData);
                const float value = std::get<1>(heapData);

                const int heapSize = heap.size();
                std::vector<std::string> pitches(heapSize);
                std::vector<enharDist> notesEnharDist(heapSize);

                // Compute heap num of non tonal intervals
                const int numIntervals = heapSize-1;
                std::vector<std::string> heapIntervals(numIntervals);
                int numNonTonalIntervals = 0;
                for (int i = 0; i < numIntervals; i++) {
                    const auto& currentNote = heap[i].note;
                    const auto& nextNote = heap[i+1].note;

                    Interval interval(currentNote, nextNote);
                    if (!interval.isTonal()) { numNonTonalIntervals++; }
                    heapIntervals[i] = interval.getName();
                }

                // Compute other row variables
                int noteIdx = 0;
                int numEnhamonicNotes = 0;
                for (const auto& noteData : heap) {
                    const std::string& notePitch = noteData.note.getPitch();
                    pitches[noteIdx] = notePitch;
                    if (noteData.wasEnharmonized) {
                        numEnhamonicNotes++;
                    }

                    notesEnharDist[noteIdx] = std::make_pair(notePitch, noteData.enharmonicDiatonicDistance);

                    noteIdx++;
                }

                // Store data in the row
                output[idx++] = std::make_tuple(pitches, numEnhamonicNotes, notesEnharDist, numNonTonalIntervals, heapIntervals, value);
            }

            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"heapPitches", "numEnharNotes", "diatonicDistance", "numNonTonalIntervals", "heapIntervals", "matchValue"};
            
            // Fill DataFrame with records and columns
            py::object df = FromRecords(output, "columns"_a = columns);

            return df;
        }, 
        py::arg("enharmonyNotes") = false);

    cls.def("getDuration", &Chord::getDuration);

    cls.def("getDurationTicks", &Chord::getDurationTicks);
    cls.def("getNote", py::overload_cast<int>(&Chord::getNote),
            py::arg("noteIndex"));
    cls.def("getNote", py::overload_cast<int>(&Chord::getNote),
            py::arg("noteIndex"),
            py::return_value_policy::reference_internal);

    cls.def("getRoot", &Chord::getRoot, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getName", &Chord::getName,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBassNote", &Chord::getBassNote);
    cls.def("getNotes", &Chord::getNotes);

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

    cls.def("getMIDIIntervals", &Chord::getMIDIIntervals,
        py::arg("firstNoteAsReference") = false);
    cls.def("getIntervals", &Chord::getIntervals,
        py::arg("firstNoteAsReference") = false);
    cls.def("getOpenStackIntervals", &Chord::getOpenStackIntervals,
        py::arg("firstNoteAsReference") = false);
    cls.def("getCloseStackIntervals", &Chord::getCloseStackIntervals,
        py::arg("firstNoteAsReference") = false);
    cls.def("getQuarterDuration", &Chord::getQuarterDuration);
    
    cls.def("size", &Chord::size);
    cls.def("stackSize", &Chord::stackSize);
    cls.def("info", &Chord::info, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("print", &Chord::print, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("printStack", &Chord::printStack, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getOpenStackChord", &Chord::getOpenStackChord,
        py::arg("enharmonyNotes") = false,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getCloseStackChord", &Chord::getCloseStackChord,
        py::arg("enharmonyNotes") = false,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getOpenStackNotes", &Chord::getOpenStackNotes);
        

    cls.def("sortNotes", &Chord::sortNotes);

    cls.def(py::self == py::self);
    cls.def(py::self != py::self);
    cls.def(py::self + py::self);

    cls.def("__getitem__", [](Chord& self, const size_t index) { return self[index]; });
    cls.def("__setitem__", [](Chord& self, const size_t index) { return self[index]; });

    // Default Python 'print' function:
    cls.def("__repr__", [](const Chord& chord) {
        const int chordSize = chord.size();
        std::string noteNames = "<Chord [";

        for (int i = 0; i < chordSize-1; i++) {
            noteNames.append(chord[i].getPitch() + ", ");
        }

        // Add the last note without the semicomma in the end
        noteNames.append(chord[chordSize-1].getPitch());

        noteNames.append("]>");

        return noteNames;
    });

    cls.def("__hash__", [](const Chord& chord) {
        std::string temp;

        for (const auto& n : chord.getNotes()) {
            temp += n.getPitch() + n.getLongType();
        }

        return std::hash<std::string>{}(temp);
    });

    cls.def("__sizeof__", [](const Chord& chord) {
        return sizeof(chord);
    });

    // bindings to Heap Data typedef
    py::class_<NoteData> clsNoteData(m, "NoteData");
    clsNoteData.def(py::init<>());
    clsNoteData.def(py::init<const Note&, const bool, const int>(),
            py::arg("note"),
            py::arg("wasEnharmonized"),
            py::arg("enharmonicDiatonicDistance"));

    py::class_<Heap> clsHeap(m, "Heap");
    py::class_<HeapData> clsHeapData(m, "HeapData");
}
#endif
