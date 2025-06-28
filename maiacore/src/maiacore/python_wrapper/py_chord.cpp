#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/chord.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

void ChordClass(const py::module& m) {
    m.doc() = "Chord class binding";

    // bindings to Chord class
    py::class_<Chord> cls(m, "Chord");
    cls.def(py::init<>(),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<Note>&, const RhythmFigure>(), 
                py::arg("notes"),
                py::arg("rhythmFigure") = RhythmFigure::QUARTER,
                py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<std::string>&, const RhythmFigure>(), 
                py::arg("pitches"),
                py::arg("rhythmFigure") = RhythmFigure::QUARTER,
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
    cls.def("setDuration", py::overload_cast<const Duration&>(&Chord::setDuration),
            py::arg("duration"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setDuration", py::overload_cast<const float, const int>(&Chord::setDuration),
            py::arg("quarterDuration"), py::arg("divisionsPerQuarterNote") = 256,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    //     cls.def("setDuration", py::overload_cast<const RhythmFigure, const
    //     int>(&Chord::setDuration),
    //             py::arg("rhythmFigure"), py::arg("divisionsPerQuarterNote") = 256);
    //     cls.def("setDuration",
    //             py::overload_cast<const float, const int, const int>(&Chord::setDuration),
    //             py::arg("durationValue"), py::arg("lowerTimeSignatureValue") = 4,
    //             py::arg("divisionsPerQuarterNote") = 256);
    //     cls.def("setDurationTicks", &Chord::setDurationTicks, py::arg("durationTicks"),
    //             py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("toInversion", &Chord::toInversion, py::arg("inversionNumber"),
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
                NoteDataHeap& heap = std::get<0>(heapData);
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

    cls.def("getCloseStackHarmonicComplexity", &Chord::getCloseStackHarmonicComplexity,
            py::arg("useEnharmony") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getHarmonicDensity",
            py::overload_cast<int, int>(&Chord::getHarmonicDensity, py::const_),
            py::arg("lowerBoundMIDI") = -1, py::arg("higherBoundMIDI") = -1);
    cls.def("getHarmonicDensity",
            py::overload_cast<const std::string&, const std::string&>(&Chord::getHarmonicDensity,
                                                                      py::const_),
            py::arg("lowerBoundPitch") = "", py::arg("higherBoundPitch") = "");

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
    cls.def("haveAnyOctaveDiminishedFifth", &Chord::haveAnyOctaveDiminishedFifth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctavePerfectFifth", &Chord::haveAnyOctavePerfectFifth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveAugmentedFifth", &Chord::haveAnyOctaveAugmentedFifth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMinorSixth", &Chord::haveAnyOctaveMinorSixth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMajorSixth", &Chord::haveAnyOctaveMajorSixth,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveDiminishedSeventh", &Chord::haveAnyOctaveDiminishedSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMinorSeventh", &Chord::haveAnyOctaveMinorSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveMajorSeventh", &Chord::haveAnyOctaveMajorSeventh,
            py::arg("useEnharmony") = false);
    cls.def("haveAnyOctaveDiminishedOctave", &Chord::haveAnyOctaveDiminishedOctave,
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

    cls.def("isSus", &Chord::isSus);
    cls.def("isMajorChord", &Chord::isMajorChord);
    cls.def("isMinorChord", &Chord::isMinorChord);
    cls.def("isAugmentedChord", &Chord::isAugmentedChord);
    cls.def("isDiminishedChord", &Chord::isDiminishedChord);
    cls.def("isHalfDiminishedChord", &Chord::isHalfDiminishedChord);
    cls.def("isWholeDiminishedChord", &Chord::isWholeDiminishedChord);
    cls.def("isDominantSeventhChord", &Chord::isDominantSeventhChord);
    cls.def("getQuality", &Chord::getQuality);

    cls.def("isSorted", &Chord::isSorted);
    cls.def("isTonal", &Chord::isTonal, py::arg("model") = nullptr);
    cls.def("isInRootPosition", &Chord::isInRootPosition);

    cls.def("getMidiIntervals", &Chord::getMidiIntervals, py::arg("firstNoteAsReference") = false);
    cls.def("getIntervals", &Chord::getIntervals, py::arg("firstNoteAsReference") = false);
    cls.def("getIntervalsFromOriginalSortedNotes", &Chord::getIntervalsFromOriginalSortedNotes);

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

    cls.def("toCents", &Chord::toCents);

    cls.def("getDegree", &Chord::getDegree, py::arg("key"), py::arg("enharmonyNotes") = false);
    cls.def("getRomanDegree", &Chord::getRomanDegree, py::arg("key"),
            py::arg("enharmonyNotes") = false);

    cls.def("getMeanFrequency", &Chord::getMeanFrequency);
    cls.def("getMeanOfExtremesFrequency", &Chord::getMeanOfExtremesFrequency);
    cls.def("getFrequencyStd", &Chord::getFrequencyStd);

    cls.def("getMeanMidiValue", &Chord::getMeanMidiValue);
    cls.def("getMeanOfExtremesMidiValue", &Chord::getMeanOfExtremesMidiValue);
    cls.def("getMidiValueStd", &Chord::getMidiValueStd);

    cls.def("getMeanPitch", &Chord::getMeanPitch, py::arg("accType") = "");
    cls.def("getMeanOfExtremesPitch", &Chord::getMeanOfExtremesPitch, py::arg("accType") = "");

    cls.def("getHarmonicSpectrum", &Chord::getHarmonicSpectrum, py::arg("numPartialsPerNote") = 6,
            py::arg("amplCallback") = nullptr);

    cls.def("getSetharesDissonance", &Chord::getSetharesDissonance,
            py::arg("numPartialsPerNote") = 6, py::arg("useMinModel") = true,
            py::arg("amplCallback") = nullptr, py::arg("dissCallback") = nullptr);

    cls.def(
        "getSetharesDyadsDataFrame",
        [](const Chord& chord, const int numPartialsPerNote, const bool useMinModel,
           const std::function<std::vector<float>(std::vector<float>)> amplCallback) {
            const SetharesDissonanceTable table = chord.getSetharesDyadsDissonanceValue(
                numPartialsPerNote, useMinModel, amplCallback);

            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"baseFreqIdx",
                                                "baseFreq",
                                                "basePitch",
                                                "basePitchCentsDeviation",
                                                "baseAmp",
                                                "targetFreqIdx",
                                                "targetFreq",
                                                "targetPitch",
                                                "targetPitchCentsDeviation",
                                                "targetAmp",
                                                "calcAmplitude",
                                                "freqRatio",
                                                "dissonance"};

            // Fill DataFrame with records and columns
            py::object df = FromRecords(table, "columns"_a = columns);

            // Base Frequency
            df.attr("baseFreqIdx") = df.attr("baseFreqIdx").attr("astype")("int16");
            df.attr("baseFreq") = df.attr("baseFreq").attr("astype")("float32");
            df.attr("basePitch") = df.attr("basePitch").attr("astype")("str");
            df.attr("basePitchCentsDeviation") =
                df.attr("basePitchCentsDeviation").attr("astype")("int16");
            df.attr("baseAmp") = df.attr("baseAmp").attr("astype")("float32");

            // Target Frequency
            df.attr("targetFreqIdx") = df.attr("targetFreqIdx").attr("astype")("int16");
            df.attr("targetFreq") = df.attr("targetFreq").attr("astype")("float32");
            df.attr("targetPitch") = df.attr("targetPitch").attr("astype")("str");
            df.attr("targetPitchCentsDeviation") =
                df.attr("targetPitchCentsDeviation").attr("astype")("int16");
            df.attr("targetAmp") = df.attr("targetAmp").attr("astype")("float32");

            // Final Results
            df.attr("freqRatio") = df.attr("freqRatio").attr("astype")("float32");
            df.attr("calcAmplitude") = df.attr("calcAmplitude").attr("astype")("float32");
            df.attr("dissonance") = df.attr("dissonance").attr("astype")("float32");

            return df;
        },
        py::arg("numPartialsPerNote") = 6, py::arg("useMinModel") = true,
        py::arg("amplCallback") = nullptr,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def(py::self == py::self);
    cls.def(py::self != py::self);
    cls.def(py::self + py::self);

    cls.def("__getitem__", [](const Chord& self, const size_t index) { return self[index]; });
    cls.def("__setitem__", [](Chord& self, const size_t index) { return self[index]; });

    // Default Python 'print' function:
    cls.def("__repr__", [](const Chord& chord) {
        const int chordSize = chord.size();

        if (chordSize == 0) {
            return std::string("<Chord []>");
        }

        std::string noteNames = "<Chord [";

        for (int i = 0; i < chordSize - 1; i++) {
            noteNames.append(chord[i].getSoundingPitch() + ", ");
        }

        // Add the last note without the semicomma in the end
        noteNames.append(chord[chordSize - 1].getSoundingPitch());

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

    // bindings to NoteDataHeap Data typedef
    py::class_<NoteData> clsNoteData(m, "NoteData");
    clsNoteData.def(py::init<>());
    clsNoteData.def(py::init<const Note&, const bool, const int>(), py::arg("note"),
                    py::arg("wasEnharmonized"), py::arg("enharmonicDiatonicDistance"));

    py::class_<NoteDataHeap> clsHeap(m, "NoteDataHeap");
    py::class_<HeapData> clsHeapData(m, "HeapData");
}
