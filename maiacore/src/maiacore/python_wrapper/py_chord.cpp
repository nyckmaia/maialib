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
    cls.def(py::init<const std::vector<Note>&, const RhythmFigure>(), py::arg("notes"),
            py::arg("rhythmFigure") = RhythmFigure::QUARTER,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def(py::init<const std::vector<std::string>&, const RhythmFigure>(), py::arg("pitches"),
            py::arg("rhythmFigure") = RhythmFigure::QUARTER,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("clear", &Chord::clear);

    cls.def("addNote", py::overload_cast<const Note&>(&Chord::addNote), py::arg("note"));
    cls.def("addNote", py::overload_cast<const std::string&>(&Chord::addNote), py::arg("pitch"));

    cls.def("removeTopNote", &Chord::removeTopNote,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Remove the last note (in original order) from the chord.

        Raises
        ------
        RuntimeError
            If the chord is empty.
    )pbdoc");
    cls.def("insertNote", &Chord::insertNote, py::arg("insertNote"), py::arg("positionNote") = 0,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Insert a note at a given position (original order).

        Parameters
        ----------
        insertNote : Note
            The note to insert.
        positionNote : int, optional
            Index to insert at, in ``0 .. size()`` (``size()`` appends at the end; default: 0).

        Raises
        ------
        RuntimeError
            If ``positionNote`` is negative or greater than ``size()``.
    )pbdoc");
    cls.def("removeNote", &Chord::removeNote, py::arg("noteIndex"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Remove the note at a given index (original order).

        Parameters
        ----------
        noteIndex : int
            Index of the note to remove, in ``0 .. size() - 1``.

        Raises
        ------
        RuntimeError
            If ``noteIndex`` is negative or out of range (e.g. on an empty chord).
    )pbdoc");
    cls.def("setDuration", py::overload_cast<const Duration&>(&Chord::setDuration),
            py::arg("duration"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setDuration", py::overload_cast<const float, const int>(&Chord::setDuration),
            py::arg("quarterDuration"), py::arg("divisionsPerQuarterNote") = 256,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("roundQuarterTones", &Chord::roundQuarterTones,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Round every quarter-tone note in the chord to the nearest semitone, ties upward.

        This is the escape hatch for the harmonic-analysis rejection: ``getName``,
        ``getQuality``, ``getRoot``, ``getBassNote``, ``getDegree``, ``stackSize``, the ``is*``
        family, the ``have*`` predicates and every other method that stacks the chord in thirds
        raise ``RuntimeError`` on a chord containing a quarter tone, because all of them are
        defined over twelve-tone equal temperament. Call this first and the analysis runs on the
        rounded pitches::

            chord = ml.Chord(["C4", "E1b4", "G4"])
            chord.roundQuarterTones()   # 1
            chord.getName()             # "C"

        ``E1b4`` becomes ``E4`` and ``E3b4`` becomes ``Eb4``. The rounding is destructive and in
        place: the original quarter-tone spelling is not recoverable from this chord afterwards.

        Returns
        -------
        int
            Number of notes whose pitch was rounded; 0 if the chord contained no quarter tone,
            in which case the chord is musically unchanged.
    )pbdoc");

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
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Invert the chord by moving the lowest note up an octave, repeated ``inversionNumber``
        times.

        Parameters
        ----------
        inversionNumber : int
            Number of inversions to perform.

        Raises
        ------
        RuntimeError
            If the chord is empty.
    )pbdoc");
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
    cls.def("getNote", py::overload_cast<int>(&Chord::getNote), py::arg("noteIndex"),
            R"pbdoc(
        Get the note at a given index.

        Parameters
        ----------
        noteIndex : int
            Index of the note, in ``0 .. size() - 1``.

        Returns
        -------
        Note
            The note at ``noteIndex``.

        Raises
        ------
        RuntimeError
            If ``noteIndex`` is negative or out of range (e.g. on an empty chord).
    )pbdoc");
    // Do not add a second `getNote` registration with `return_value_policy::reference_internal`
    // (one existed here and was dead code: pybind11 always resolved to the copy-returning
    // registration above, since both had the identical C++ signature `int -> Note&` and
    // overloads are tried in registration order). A reference-returning binding would hand
    // Python a live `Note&` into `_originalNotes`; mutating it (e.g. `chord.getNote(0).setPitch(
    // "F#4")`) would desync the chord's cached stacked-in-thirds analysis without going through
    // any mutator, silently reopening the staleness class invalidateStackCache() closes. Keep
    // `getNote` copy-returning.

    cls.def("getRoot", &Chord::getRoot,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getName", &Chord::getName,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Get the chord's tonal name (e.g. ``"Cm7"``, ``"G7"``), computing the stacked-in-thirds
        representation first if needed.

        Returns
        -------
        str
            The chord name, or an empty string if the chord isn't tonal.

        Raises
        ------
        RuntimeError
            If no enharmonic respelling of this chord's notes can produce a valid
            stacked-in-thirds representation. This is not limited to large chords: pitch classes
            are distinguished by spelling (``"C"``, ``"C#"`` and ``"Db"`` are three different
            pitch classes), so even a 3-note chord like ``["C4", "C#4", "Db4"]`` can raise this.
            This applies to every method that triggers the stacked-in-thirds computation (e.g.
            ``isDyad``, ``stackSize``, ``isInRootPosition``, ``getOpenStackIntervals``,
            ``getCloseStackIntervals``, the ``have*`` family), not only ``getName``.
    )pbdoc");
    cls.def("getBassNote", &Chord::getBassNote);
    cls.def("getNotes", &Chord::getNotes);

    cls.def("getCloseStackHarmonicComplexity", &Chord::getCloseStackHarmonicComplexity,
            py::arg("useEnharmony") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getHarmonicDensity",
            py::overload_cast<int, int>(&Chord::getHarmonicDensity, py::const_),
            py::arg("lowerBoundMIDI") = -1, py::arg("higherBoundMIDI") = -1,
            R"pbdoc(
        Get the harmonic density of the chord within a MIDI pitch range.

        When the bounds are left at -1 the range is auto-detected from the chord's own extremes,
        taken from exact pitch positions: a quarter-tone extreme widens the range by half a
        semitone rather than being rounded, so ``["C1x4", "G4"]`` spans 6.5 semitones, not 6. A
        float expresses that exactly, so this method never raises on a quarter-tone chord.

        Returns
        -------
        float
            The density: the note count divided by the range it spans.
    )pbdoc");
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

    cls.def("isSorted", &Chord::isSorted,
            R"pbdoc(
        Check whether the chord's notes are in ascending pitch order.

        Compares exact pitch positions, so a quarter tone orders correctly instead of being
        rounded onto the semitone above it: ``Chord(["E1b4", "E4"])`` is sorted, because 63.5
        precedes 64. Ordering is a predicate, so the bool returned expresses the true answer for a
        quarter-tone chord exactly and this method never raises on one.

        Returns
        -------
        bool
            ``True`` if the notes run from lowest to highest.
    )pbdoc");
    cls.def("isTonal", &Chord::isTonal, py::arg("model") = nullptr);
    cls.def("isInRootPosition", &Chord::isInRootPosition,
            R"pbdoc(
        Check whether the chord's close-stacked root matches its lowest sounding note.

        Returns
        -------
        bool
            ``True`` if the chord is in root position, ``False`` otherwise -- including for an
            empty chord, which cannot be in root position.

        Raises
        ------
        RuntimeError
            If no enharmonic respelling of this chord's notes can produce a valid
            stacked-in-thirds representation (see ``getName``'s docstring for when this applies --
            it is not limited to large chords).
    )pbdoc");

    cls.def("getMidiIntervals", &Chord::getMidiIntervals, py::arg("firstNoteAsReference") = false,
            R"pbdoc(
        Get the intervals between the chord's notes, in whole MIDI semitones.

        Returns
        -------
        list[int]
            One value per note pair, or an empty list for an empty chord.

        Raises
        ------
        RuntimeError
            If the chord contains a quarter tone. A list of whole semitone counts cannot express
            the 3.5 semitones of a neutral third, and answering 3 or 4 would be indistinguishable
            from a chord that really holds a minor or major third. Call ``roundQuarterTones``
            first, or use ``toCents``, which expresses a quarter tone exactly.
    )pbdoc");
    cls.def("getIntervals", &Chord::getIntervals, py::arg("firstNoteAsReference") = false);
    cls.def("getIntervalsFromOriginalSortedNotes", &Chord::getIntervalsFromOriginalSortedNotes);

    cls.def("getOpenStackIntervals", &Chord::getOpenStackIntervals,
            py::arg("firstNoteAsReference") = false,
            R"pbdoc(
        Get the intervals between consecutive notes of the open (stacked-in-thirds) chord.

        Returns
        -------
        list[Interval]
            One interval per adjacent pair in the open stack, or an empty list if the chord has
            fewer than 2 notes in its open stack (including an empty chord).

        Raises
        ------
        RuntimeError
            If no enharmonic respelling of this chord's notes can produce a valid
            stacked-in-thirds representation (see ``getName``'s docstring for when this applies --
            it is not limited to large chords).
    )pbdoc");
    cls.def("getCloseStackIntervals", &Chord::getCloseStackIntervals,
            py::arg("firstNoteAsReference") = false,
            R"pbdoc(
        Get the intervals between consecutive notes of the close (stacked-in-thirds) chord.

        Returns
        -------
        list[Interval]
            One interval per adjacent pair in the close stack, or an empty list if the chord has
            fewer than 2 notes in its close stack (including an empty chord).

        Raises
        ------
        RuntimeError
            If no enharmonic respelling of this chord's notes can produce a valid
            stacked-in-thirds representation (see ``getName``'s docstring for when this applies --
            it is not limited to large chords).
    )pbdoc");
    cls.def("getQuarterDuration", &Chord::getQuarterDuration);

    cls.def("size", &Chord::size);
    cls.def("stackSize", &Chord::stackSize);
    cls.def("info", &Chord::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Print the chord's name, size, note list and stacked-in-thirds data to stdout.

        On a chord containing a quarter tone this degrades instead of raising, unlike every
        analysis method: it still prints the size and the note list, replaces the name with a
        note that the harmonic analysis is unavailable, skips the stack section, and names
        ``roundQuarterTones`` as the way to enable the analysis. ``info`` works on any chord
        that can be built.

        Raises
        ------
        RuntimeError
            If the chord (or, for an analysable chord, its stacked-in-thirds representation) is
            empty. Quarter tones are never the cause.
    )pbdoc");

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

    cls.def("toCents", &Chord::toCents,
            R"pbdoc(
        Get the interval, in cents, between each pair of consecutive notes.

        Returns
        -------
        Cents are the one unit in this library that expresses a quarter tone exactly -- 50 cents to
        the quarter tone, 350 to the neutral third -- so this method accepts a quarter-tone chord
        and computes the true value, unlike the MIDI-semitone methods, which reject one.

        Computed as integer arithmetic on exact step positions (100 cents to the semitone in
        twelve-tone equal temperament) rather than from the notes' frequencies, so the result
        carries no floating-point error and does not depend on ``freqA4``.

        Returns
        -------
        list[int]
            One value per adjacent note pair, or an empty list if the chord has fewer than 2
            notes (including an empty chord).
    )pbdoc");

    cls.def("getDegree", &Chord::getDegree, py::arg("key"), py::arg("enharmonyNotes") = false);
    cls.def("getRomanDegree", &Chord::getRomanDegree, py::arg("key"),
            py::arg("enharmonyNotes") = false);

    cls.def("getMeanFrequency", &Chord::getMeanFrequency, py::arg("freqA4") = 440.0f);
    cls.def("getMeanOfExtremesFrequency", &Chord::getMeanOfExtremesFrequency,
            py::arg("freqA4") = 440.0f);
    cls.def("getFrequencyStd", &Chord::getFrequencyStd, py::arg("freqA4") = 440.0f);

    cls.def("getMeanMidiValue", &Chord::getMeanMidiValue,
            R"pbdoc(
        Get the arithmetic mean of the chord's MIDI numbers.

        Returns
        -------
        int
            The mean MIDI value, or 0 for an empty chord.

        Raises
        ------
        RuntimeError
            If the chord contains a quarter tone: an int cannot express the 63.5 that
            ``["C4", "E1b4", "G4"]`` averages to, and the 63 it used to return is the same value a
            plain C major triad gives. Call ``roundQuarterTones`` first.
    )pbdoc");
    cls.def("getMeanOfExtremesMidiValue", &Chord::getMeanOfExtremesMidiValue,
            R"pbdoc(
        Get the mean MIDI value of the chord's lowest and highest notes.

        Returns
        -------
        int
            The mean of the extremes.

        Raises
        ------
        RuntimeError
            If the chord contains a quarter tone -- anywhere in it, not only at an extreme, since
            the extremes themselves are selected by an ordering the rounding can get wrong. Call
            ``roundQuarterTones`` first.
    )pbdoc");
    cls.def("getMidiValueStd", &Chord::getMidiValueStd,
            R"pbdoc(
        Get the standard deviation of the chord's pitch positions.

        Uses exact pitch positions, so a quarter tone contributes its true value; a float expresses
        that spread exactly and this method never raises on a quarter-tone chord.

        Returns
        -------
        float
            The standard deviation, or ``0.0`` for an empty chord.
    )pbdoc");

    cls.def("getMeanPitch", &Chord::getMeanPitch, py::arg("accType") = "",
            R"pbdoc(
        Get the pitch name of the chord's mean MIDI value.

        Returns
        -------
        str
            The pitch name (e.g. ``"C4"``, ``"F#3"``).

        Raises
        ------
        RuntimeError
            If the chord contains a quarter tone, inherited from ``getMeanMidiValue``, which this
            method spells. A pitch string CAN spell a quarter tone (``"D3x4"``), but the value
            being spelled is the mean of N notes -- a multiple of 1/N, generally not a multiple of
            0.5 -- so there is no exact spelling to return. Call ``roundQuarterTones`` first.
    )pbdoc");
    cls.def("getMeanOfExtremesPitch", &Chord::getMeanOfExtremesPitch, py::arg("accType") = "",
            R"pbdoc(
        Get the pitch name of the chord's mean-of-extremes MIDI value.

        Returns
        -------
        str
            The pitch name (e.g. ``"C4"``, ``"F#3"``).

        Raises
        ------
        RuntimeError
            If the chord contains a quarter tone, inherited from ``getMeanOfExtremesMidiValue``,
            which this method spells. Call ``roundQuarterTones`` first.
    )pbdoc");

    cls.def("getHarmonicSpectrum", &Chord::getHarmonicSpectrum, py::arg("numPartialsPerNote") = 6,
            py::arg("amplCallback") = nullptr, py::arg("partialsDecayExpRate") = 0.88f);

    cls.def("getSetharesDissonance", &Chord::getSetharesDissonance,
            py::arg("numPartialsPerNote") = 6, py::arg("useMinModel") = true,
            py::arg("amplCallback") = nullptr, py::arg("partialsDecayExpRate") = 0.88f,
            py::arg("dissCallback") = nullptr);

    cls.def(
        "getSetharesDyadsDataFrame",
        [](const Chord& chord, const int numPartialsPerNote, const bool useMinModel,
           const std::function<std::vector<float>(std::vector<float>)> amplCallback,
           const float partialsDecayExpRate) {
            const SetharesDissonanceTable table = chord.getSetharesDyadsDissonanceValue(
                numPartialsPerNote, useMinModel, amplCallback, partialsDecayExpRate);

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
        py::arg("amplCallback") = nullptr, py::arg("partialsDecayExpRate") = 0.88f,
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
