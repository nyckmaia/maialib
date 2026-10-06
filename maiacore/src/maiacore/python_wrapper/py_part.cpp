#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/chord.h"
#include "maiacore/clef.h"
#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/part.h"
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void PartClass(const py::module& m) {
    m.doc() = "Part class binding";

    // bindings to Part class
    py::class_<Part> cls(m, "Part");
    cls.def(py::init<const std::string&, const int, const bool, const int>(), py::arg("partName"),
            py::arg("numStaves") = 1, py::arg("isPitched") = true,
            py::arg("divisionsPerQuarterNote") = 256);

    cls.def("clear", &Part::clear);

    cls.def("getPartIndex", &Part::getPartIndex);
    cls.def("setPartIndex", &Part::setPartIndex, py::arg("partIdx"));

    cls.def("getName", &Part::getName);
    cls.def("getShortName", &Part::getShortName);

    cls.def("addMidiUnpitched", &Part::addMidiUnpitched, py::arg("midiUnpitched"));
    cls.def("getMidiUnpitched", &Part::getMidiUnpitched);

    cls.def("addMeasure", &Part::addMeasure, py::arg("numMeasures"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeMeasure", &Part::removeMeasure, py::arg("measureStart"), py::arg("measureEnd"));

    cls.def("getMeasure", py::overload_cast<const int>(&Part::getMeasure), py::arg("measureId"),
            py::return_value_policy::reference_internal);
    cls.def("getMeasure", py::overload_cast<const int>(&Part::getMeasure, py::const_),
            py::arg("measureId"), py::return_value_policy::reference_internal);
    cls.def("getMeasures", &Part::getMeasures, py::return_value_policy::reference_internal);
    cls.def("getNumMeasures", &Part::getNumMeasures);

    cls.def("setNumStaves", &Part::setNumStaves, py::arg("numStaves"));
    cls.def("addStaves", &Part::addStaves, py::arg("numStaves") = 1,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removeStave", &Part::removeStave, py::arg("staveId"));
    cls.def("getNumStaves", &Part::getNumStaves);

    cls.def("getNumNotes", &Part::getNumNotes, py::arg("staveId") = -1);
    cls.def("getNumNotesOn", &Part::getNumNotesOn, py::arg("staveId") = -1);
    cls.def("getNumNotesOff", &Part::getNumNotesOff, py::arg("staveId") = -1);

    cls.def("setShortName", &Part::setShortName, py::arg("shortName"));

    cls.def("info", &Part::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("setIsPitched", &Part::setIsPitched, py::arg("isPitched") = true);
    cls.def("setTransposingInterval", &Part::setTransposingInterval, py::arg("diatonicInterval"),
            py::arg("chromaticInterval"), py::arg("measureStart") = 0, py::arg("measureEnd") = -1,
            py::arg("staff") = -1, py::arg("doubling") = OctaveDoubling::NONE,
            R"pbdoc(
        Set the transposing interval and the octave doubling of the pitched notes of a range of
        measures and a staff, in place.

        Every pitched note of the measures ``measureStart`` up to, not including, ``measureEnd``
        on the staff ``staff`` gets the interval (see ``Note.setTransposingInterval``) and the
        doubling (see ``Note.setOctaveDoubling``); rests and unpitched notes are left alone. The
        interval is the total one: an octave transposition is folded in as 7 letters and 12
        semitones per octave, so a B-flat bass clarinet is ``(-8, -14)``. The notes hold the
        transposition, and the MusicXML export writes its ``<transpose>`` elements from them.
        It changes the notes in place, as an edit through ``Measure.getNote()`` (a live
        reference) or ``Score.forEachNote`` does one note at a time. Every note is checked
        before any changes, so the call changes all of them or none. An export -> import keeps
        the pair only when it is the conventional one for its chromatic interval (or the
        diminished-fifth tritone); another pair is read back corrected (same sound,
        conventional spelling), with a ``TRANSPOSE_PAIR_CORRECTED`` record in the import report.

        Parameters
        ----------
        diatonicInterval : int
            Letters from the written to the sounding pitch, e.g. -1 for a B-flat clarinet.
        chromaticInterval : int
            Semitones from the written to the sounding pitch, e.g. -2 for a B-flat clarinet.
        measureStart : int, default 0
            Index of the first measure.
        measureEnd : int, default -1
            Index one past the last measure; -1 for the end of the part.
        staff : int, default -1
            Zero-based staff index; -1 for every staff.
        doubling : OctaveDoubling, default OctaveDoubling.NONE
            The octave doubling.

        Raises
        ------
        IndexError
            If the measures are not a range of the part's measures, or ``staff`` is neither -1
            nor one of its staves.
        RuntimeError
            If a note would have no sounding pitch with the interval (below ``C1b-1``, or above
            ``B11`` where its letter cannot spell it); the message names the first such note,
            and no note is changed.

        Examples
        --------
        >>> score = ml.Score(["Clarinet"], 2)
        >>> for m in range(2):
        ...     score.getPart(0).getMeasure(m).addNote(ml.Note("D4"))
        >>> score.getPart(0).setTransposingInterval(-1, -2, measureStart=1)
        >>> [score.getPart(0).getMeasure(m).getNote(0).getSoundingPitch() for m in range(2)]
        ['D4', 'C4']
    )pbdoc");
    cls.def("setStaffLines", &Part::setStaffLines, py::arg("staffLines") = 5);
    cls.def("isPitched", &Part::isPitched);
    cls.def("getStaffLines", &Part::getStaffLines);

    cls.def(
        "append",
        py::overload_cast<const std::variant<Note, Chord>&, const int, const int>(&Part::append),
        py::arg("obj"), py::arg("position") = -1, py::arg("staveId") = 0,
        R"pbdoc(
        Append a Note or Chord to the part at a given position and staff.

        If ``obj`` doesn't fit in the current measure's remaining space, it is split into a tied
        pair: the first part fills the rest of the current measure and the second part is placed
        at the start of the next measure.

        Parameters
        ----------
        obj : Note or Chord
            The note or chord to append.
        position : int, optional
            Position in the measure (default: -1, meaning append to the end).
        staveId : int, optional
            Stave index (default: 0).

        Raises
        ------
        RuntimeError
            If ``obj`` overflows the last measure of the part (there is no following measure to
            hold the tied remainder). The part is left unchanged when this happens -- no partial
            write occurs. Add another measure (``addMeasure()``) before appending an object that
            overflows the current last measure.
    )pbdoc");

    cls.def("append",
            py::overload_cast<const std::vector<std::variant<Note, Chord>>&, const int, const int>(
                &Part::append),
            py::arg("objs"), py::arg("position") = -1, py::arg("staveId") = 0,
            R"pbdoc(
        Append multiple Note/Chord objects to the part at a given position and staff.

        Calls ``append()`` once per element of ``objs``; see its docstring for the splitting and
        failure behavior applied to each element.

        Parameters
        ----------
        objs : list[Note or Chord]
            The notes/chords to append, in order.
        position : int, optional
            Position in the measure (default: -1, meaning append to the end).
        staveId : int, optional
            Stave index (default: 0).

        Raises
        ------
        RuntimeError
            If any element overflows the last measure of the part. Elements appended before the
            failing one remain in the part -- this is not an all-or-nothing operation across the
            list, only within each individual append.
    )pbdoc");

    cls.def("toXML", &Part::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2,
            R"pbdoc(
        Return the part's measures as MusicXML text.

        The transpositions are written from the notes, which hold them: for each staff, a
        ``<transpose>`` in measure 1 when its first pitched note is transposed or doubled, and
        one wherever a pitched note's interval or octave doubling differs from that of the
        staff's previous pitched note -- in the measure's ``<attributes>`` when the note is the
        staff's first pitched note there, otherwise in an ``<attributes>`` just before it. Rests
        and unpitched notes change nothing. A ``<transpose>`` has no ``number`` when every staff
        has the same transposition there; the interval is unfolded into ``<diatonic>``,
        ``<chromatic>`` and, from an octave on, ``<octave-change>``, and ``<double/>`` or
        ``<double above="yes"/>`` states the doubling. An export -> import keeps a note's
        interval pair only when it is the conventional one for its chromatic interval (or the
        diminished-fifth tritone); another pair is read back corrected (same sound,
        conventional spelling), with a ``TRANSPOSE_PAIR_CORRECTED`` record in the import report.

        Parameters
        ----------
        instrumentId : int, default 1
            Zero-based index of the part in its score.
        identSize : int, default 2
            Number of spaces per indentation level.

        Returns
        -------
        str
            The ``<measure>`` elements of the part.

        Raises
        ------
        RuntimeError
            If the notes of a chord have different transposing intervals or octave doublings,
            which one ``<transpose>`` cannot express; the message names the part, the measure
            and the staff.
    )pbdoc");
    cls.def("toJSON", &Part::toJSON);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Part& part) { return "<Part " + part.getName() + ">"; });

    cls.def("__hash__", [](const Part& part) { return std::hash<std::string>{}(part.toXML()); });

    cls.def("__sizeof__", [](const Part& part) { return sizeof(part); });
}
