#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "chord.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

void ChordClass(const py::module& m) {
    m.doc() = "Chord class binding";

    // bindings to Chord class
    py::class_<Chord> cls(m, "Chord");
    cls.def(py::init<>(),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<Note>&>(), py::arg("notes"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<std::string>&>(), py::arg("pitches"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &Chord::clear);

    cls.def("addNote", py::overload_cast<const Note&>(&Chord::addNote), py::arg("note"));
    cls.def("addNote", py::overload_cast<const std::string&>(&Chord::addNote), py::arg("pitch"));

    cls.def("removeTopNote", &Chord::removeTopNote,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("insertNote", &Chord::insertNote, py::arg("insertNote"), py::arg("positionNote") = 0,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeNote", &Chord::removeNote, py::arg("noteIndex"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setDurationTicks", &Chord::setDurationTicks, py::arg("durationTicks"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("inversion", &Chord::inversion, py::arg("inversionNumber"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("transpose", &Chord::transpose, py::arg("semiTonesNumber"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("transposeStackOnly", &Chord::transposeStackOnly, py::arg("semiTonesNumber"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("removeDuplicateNotes", &Chord::removeDuplicateNotes);

    cls.def(
        "getStackDataFrame",
        [](Chord& chord, const bool enharmonyNotes) {
            std::vector<HeapData> heapsData = chord.getStackedHeaps(enharmonyNotes);

            using enharDist = std::pair<std::string, int>;  // [notePitchClass, diatonicDistance]
            using dfRow = std::tuple<std::vector<Note>, std::vector<std::string>, int,
                                     std::vector<enharDist>, int, std::vector<std::string>, float>;
            std::vector<dfRow> output(heapsData.size());

            int idx = 0;
            for (auto& heapData : heapsData) {
                Heap& heap = std::get<0>(heapData);
                const float value = std::get<1>(heapData);

                const int heapSize = heap.size();
                std::vector<Note> notes(heapSize);
                std::vector<std::string> pitchClasses(heapSize);
                std::vector<enharDist> notesEnharDist(heapSize);

                // ===== COMPUTE ROW VALUES ===== //

                // Original non-sorted Note Objects
                int noteIdx = 0;
                for (const auto& noteData : heap) {
                    notes[noteIdx] = noteData.note;
                    noteIdx++;
                }

                sortHeapOctaves(&heap);

                noteIdx = 0;
                int numEnhamonicNotes = 0;
                for (const auto& noteData : heap) {
                    const std::string& notePitchClass = noteData.note.getPitchClass();

                    pitchClasses[noteIdx] = notePitchClass;
                    notesEnharDist[noteIdx] =
                        std::make_pair(notePitchClass, noteData.enharmonicDiatonicDistance);

                    if (noteData.wasEnharmonized) {
                        numEnhamonicNotes++;
                    }

                    noteIdx++;
                }

                // sortHeapOctaves(&heap);

                // Compute heap num of non tonal intervals
                const int numIntervals = heapSize - 1;
                std::vector<std::string> heapIntervals(numIntervals);
                int numNonTonalIntervals = 0;
                for (int i = 0; i < numIntervals; i++) {
                    const auto& currentNote = heap.at(i).note;
                    const auto& nextNote = heap.at(i + 1).note;

                    Interval interval(currentNote.getPitch(), nextNote.getPitch());
                    if (!interval.isTonal()) {
                        numNonTonalIntervals++;
                    }

                    heapIntervals[i] = interval.getName();
                }

                // Store data in the row
                output[idx++] =
                    std::make_tuple(notes, pitchClasses, numEnhamonicNotes, notesEnharDist,
                                    numNonTonalIntervals, heapIntervals, value);
            }

            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"notes",
                                                "pitchClassStack",
                                                "numEnharNotes",
                                                "diatonicDistance",
                                                "numNonTonalIntervals",
                                                "intervals",
                                                "matchValue"};

            // Fill DataFrame with records and columns
            py::object df = FromRecords(output, "columns"_a = columns);

            df.attr("numEnharNotes") = df.attr("numEnharNotes").attr("astype")("int16");
            df.attr("numNonTonalIntervals") =
                df.attr("numNonTonalIntervals").attr("astype")("int16");
            df.attr("matchValue") = df.attr("matchValue").attr("astype")("float32");

            return df;
        },
        py::arg("enharmonyNotes") = false);

    cls.def("getDuration", &Chord::getDuration);

    cls.def("getDurationTicks", &Chord::getDurationTicks);
    cls.def("getNote", py::overload_cast<int>(&Chord::getNote), py::arg("noteIndex"));
    cls.def("getNote", py::overload_cast<int>(&Chord::getNote), py::arg("noteIndex"),
            py::return_value_policy::reference_internal);

    cls.def("getRoot", &Chord::getRoot,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getName", &Chord::getName,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getBassNote", &Chord::getBassNote);
    cls.def("getNotes", &Chord::getNotes);

    cls.def("haveMajorInterval", &Chord::haveMajorInterval, py::arg("useEnharmony") = false);
    cls.def("haveMinorInterval", &Chord::haveMinorInterval, py::arg("useEnharmony") = false);
    cls.def("havePerfectInterval", &Chord::havePerfectInterval, py::arg("useEnharmony") = false);
    cls.def("haveDiminishedInterval", &Chord::haveDiminishedInterval,
            py::arg("useEnharmony") = false);
    cls.def("haveAugmentedInterval", &Chord::haveAugmentedInterval,
            py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 1 ===== //
    cls.def("haveDiminishedUnisson", &Chord::haveDiminishedUnisson,
            py::arg("useEnharmony") = false);
    cls.def("havePerfectUnisson", &Chord::havePerfectUnisson, py::arg("useEnharmony") = false);
    cls.def("haveAugmentedUnisson", &Chord::haveAugmentedUnisson, py::arg("useEnharmony") = false);
    cls.def("haveMinorSecond", &Chord::haveMinorSecond, py::arg("useEnharmony") = false);
    cls.def("haveMajorSecond", &Chord::haveMajorSecond, py::arg("useEnharmony") = false);
    cls.def("haveMinorThird", &Chord::haveMinorThird, py::arg("useEnharmony") = false);
    cls.def("haveMajorThird", &Chord::haveMajorThird, py::arg("useEnharmony") = false);
    cls.def("havePerfectFourth", &Chord::havePerfectFourth, py::arg("useEnharmony") = false);
    cls.def("haveAugmentedFourth", &Chord::haveAugmentedFourth, py::arg("useEnharmony") = false);
    cls.def("haveDiminishedFifth", &Chord::haveDiminishedFifth, py::arg("useEnharmony") = false);
    cls.def("havePerfectFifth", &Chord::havePerfectFifth, py::arg("useEnharmony") = false);
    cls.def("haveAugmentedFifth", &Chord::haveAugmentedFifth, py::arg("useEnharmony") = false);
    cls.def("haveMinorSixth", &Chord::haveMinorSixth, py::arg("useEnharmony") = false);
    cls.def("haveMajorSixth", &Chord::haveMajorSixth, py::arg("useEnharmony") = false);
    cls.def("haveDiminishedSeventh", &Chord::haveDiminishedSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveMinorSeventh", &Chord::haveMinorSeventh, py::arg("useEnharmony") = false);
    cls.def("haveMajorSeventh", &Chord::haveMajorSeventh, py::arg("useEnharmony") = false);
    cls.def("haveDiminishedOctave", &Chord::haveDiminishedOctave, py::arg("useEnharmony") = false);
    cls.def("havePerfectOctave", &Chord::havePerfectOctave, py::arg("useEnharmony") = false);
    cls.def("haveAugmentedOctave", &Chord::haveAugmentedOctave, py::arg("useEnharmony") = false);
    cls.def("haveMinorNinth", &Chord::haveMinorNinth, py::arg("useEnharmony") = false);
    cls.def("haveMajorNinth", &Chord::haveMajorNinth, py::arg("useEnharmony") = false);
    cls.def("havePerfectEleventh", &Chord::havePerfectEleventh, py::arg("useEnharmony") = false);
    cls.def("haveSharpEleventh", &Chord::haveSharpEleventh, py::arg("useEnharmony") = false);
    cls.def("haveMinorThirdteenth", &Chord::haveMinorThirdteenth, py::arg("useEnharmony") = false);
    cls.def("haveMajorThirdteenth", &Chord::haveMajorThirdteenth, py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 2 ===== //
    cls.def("haveSecond", &Chord::haveSecond, py::arg("useEnharmony") = false);
    cls.def("haveThird", &Chord::haveThird, py::arg("useEnharmony") = false);
    cls.def("haveFourth", &Chord::haveFourth, py::arg("useEnharmony") = false);
    cls.def("haveFifth", &Chord::haveFifth, py::arg("useEnharmony") = false);
    cls.def("haveSixth", &Chord::haveSixth, py::arg("useEnharmony") = false);
    cls.def("haveSeventh", &Chord::haveSeventh, py::arg("useEnharmony") = false);
    cls.def("haveOctave", &Chord::haveOctave, py::arg("useEnharmony") = false);
    cls.def("haveNinth", &Chord::haveNinth, py::arg("useEnharmony") = false);
    cls.def("haveEleventh", &Chord::haveEleventh, py::arg("useEnharmony") = false);
    cls.def("haveThirdteenth", &Chord::haveThirdteenth, py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 3 ===== //
    cls.def("haveAnyOctaveMinorSecond", &Chord::haveAnyOctaveMinorSecond,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMajorSecond", &Chord::haveAnyOctaveMajorSecond,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMinorThird", &Chord::haveAnyOctaveMinorThird,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMajorThird", &Chord::haveAnyOctaveMajorThird,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctavePerfectFourth", &Chord::haveAnyOctavePerfectFourth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveAugmentedFourth", &Chord::haveAnyOctaveAugmentedFourth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveDiminhavehedFifth", &Chord::haveAnyOctaveDiminhavehedFifth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctavePerfectFifth", &Chord::haveAnyOctavePerfectFifth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveAugmentedFifth", &Chord::haveAnyOctaveAugmentedFifth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMinorSixth", &Chord::haveAnyOctaveMinorSixth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMajorSixth", &Chord::haveAnyOctaveMajorSixth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveDiminhavehedSeventh", &Chord::haveAnyOctaveDiminhavehedSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMinorSeventh", &Chord::haveAnyOctaveMinorSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMajorSeventh", &Chord::haveAnyOctaveMajorSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveDiminhavehedOctave", &Chord::haveAnyOctaveDiminhavehedOctave,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctavePerfectOctave", &Chord::haveAnyOctavePerfectOctave,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveAugmentedOctave", &Chord::haveAnyOctaveAugmentedOctave,
            py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 4 ===== //
    cls.def("haveAnyOctaveSecond", &Chord::haveAnyOctaveSecond);
    cls.def("haveAnyOctaveThird", &Chord::haveAnyOctaveThird);
    cls.def("haveAnyOctaveFourth", &Chord::haveAnyOctaveFourth);
    cls.def("haveAnyOctaveFifth", &Chord::haveAnyOctaveFifth);
    cls.def("haveAnyOctaveSixth", &Chord::haveAnyOctaveSixth);
    cls.def("haveAnyOctaveSeventh", &Chord::haveAnyOctaveSeventh);
    cls.def("haveAnyOctaveOctave", &Chord::haveAnyOctaveOctave);

    cls.def("isMajorChord", &Chord::isMajorChord);
    cls.def("isMinorChord", &Chord::isMinorChord);
    cls.def("isAugmentedChord", &Chord::isAugmentedChord);
    cls.def("isDiminishedChord", &Chord::isDiminishedChord);
    cls.def("isDominantSeventhChord", &Chord::isDominantSeventhChord);
    cls.def("isHalfDiminishedChord", &Chord::isHalfDiminishedChord);

    cls.def("isSorted", &Chord::isSorted);
    cls.def("isTonal", &Chord::isTonal, py::arg("model") = nullptr);
    cls.def("isInRootPosition", &Chord::isInRootPosition);

    cls.def("getMIDIIntervals", &Chord::getMIDIIntervals, py::arg("firstNoteAsReference") = false);
    cls.def("getIntervals", &Chord::getIntervals, py::arg("firstNoteAsReference") = false);
    cls.def("getOpenStackIntervals", &Chord::getOpenStackIntervals,
            py::arg("firstNoteAsReference") = false);
    cls.def("getCloseStackIntervals", &Chord::getCloseStackIntervals,
            py::arg("firstNoteAsReference") = false);
    cls.def("getQuarterDuration", &Chord::getQuarterDuration);

    cls.def("size", &Chord::size);
    cls.def("stackSize", &Chord::stackSize);
    cls.def("info", &Chord::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("print", &Chord::print,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("printStack", &Chord::printStack,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getOpenStackChord", &Chord::getOpenStackChord, py::arg("enharmonyNotes") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getCloseStackChord", &Chord::getCloseStackChord, py::arg("enharmonyNotes") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getCloseChord", &Chord::getCloseChord, py::arg("enharmonyNotes") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getOpenStackNotes", &Chord::getOpenStackNotes);

    cls.def("sortNotes", &Chord::sortNotes);

    cls.def(py::self == py::self);
    cls.def(py::self != py::self);
    cls.def(py::self + py::self);

    cls.def("__getitem__", [](const Chord& self, const size_t index) { return self[index]; });
    cls.def("__setitem__", [](Chord& self, const size_t index) { return self[index]; });

    // Default Python 'print' function:
    cls.def("__repr__", [](const Chord& chord) {
        const int chordSize = chord.size();
        std::string noteNames = "<Chord [";

        for (int i = 0; i < chordSize - 1; i++) {
            noteNames.append(chord[i].getPitch() + ", ");
        }

        // Add the last note without the semicomma in the end
        noteNames.append(chord[chordSize - 1].getPitch());

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

    cls.def("__sizeof__", [](const Chord& chord) { return sizeof(chord); });

    // bindings to Heap Data typedef
    py::class_<NoteData> clsNoteData(m, "NoteData");
    clsNoteData.def(py::init<>());
    clsNoteData.def(py::init<const Note&, const bool, const int>(), py::arg("note"),
                    py::arg("wasEnharmonized"), py::arg("enharmonicDiatonicDistance"));

    py::class_<Heap> clsHeap(m, "Heap");
    py::class_<HeapData> clsHeapData(m, "HeapData");
}
