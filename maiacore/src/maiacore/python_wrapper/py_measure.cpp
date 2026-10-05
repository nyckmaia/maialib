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
            R"pbdoc(
        Insert a copy of a note into a staff of the measure.

        Parameters
        ----------
        note : Note
            The note.
        staveId : int, optional
            Staff index (default: 0).
        position : int, optional
            Index the note takes on the staff, from 0 to ``getNumNotes(staveId)``; -1 (the
            default), or any negative value, appends it.

        Raises
        ------
        IndexError
            If ``staveId`` is not a staff of the measure, or ``position`` is past the end of the
            staff; the measure is unchanged.
    )pbdoc");
    cls.def("addNote",
            py::overload_cast<const std::vector<Note>&, const int, int>(&Measure::addNote),
            py::arg("noteVec"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert copies of notes into a staff of the measure, in list order.

        Parameters
        ----------
        noteVec : list of Note
            The notes; the first takes index ``position``, the next ``position + 1``, and so on.
        staveId : int, optional
            Staff index (default: 0).
        position : int, optional
            Index the first note takes, from 0 to ``getNumNotes(staveId)``; -1 (the default), or
            any negative value, appends them.

        Raises
        ------
        IndexError
            If ``staveId`` is not a staff of the measure, or ``position`` is past the end of the
            staff; the measure is unchanged.

        Examples
        --------
        >>> measure = ml.Measure()
        >>> measure.addNote("C4")
        >>> measure.addNote([ml.Note("A4"), ml.Note("B4")], 0, 0)
        >>> [measure.getNote(i).getPitch() for i in range(measure.getNumNotes())]
        ['A4', 'B4', 'C4']
    )pbdoc");
    cls.def("addNote", py::overload_cast<const std::string&, const int, int>(&Measure::addNote),
            py::arg("pitchClass"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert a new note of the given pitch into a staff of the measure, as
        ``addNote(Note(pitchClass), staveId, position)`` does.
    )pbdoc");
    cls.def("addNote",
            py::overload_cast<const std::vector<std::string>&, const int, int>(&Measure::addNote),
            py::arg("pitchClassVec"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert new notes of the given pitches into a staff of the measure, in list order, as
        ``addNote([Note(p) for p in pitchClassVec], staveId, position)`` does.
    )pbdoc");

    cls.def("removeNote", &Measure::removeNote, py::arg("noteId"), py::arg("staveId") = 0,
            R"pbdoc(
        Remove the note at an index of a staff.

        Parameters
        ----------
        noteId : int
            Index of the note on the staff, from 0 to ``getNumNotes(staveId) - 1``.
        staveId : int, optional
            Staff index (default: 0).

        Raises
        ------
        IndexError
            If ``staveId`` or ``noteId`` is outside the measure; the measure is unchanged.

        Examples
        --------
        >>> measure = ml.Measure()
        >>> measure.addNote(["C4", "D4", "E4"])
        >>> measure.removeNote(1)
        >>> [measure.getNote(i).getPitch() for i in range(measure.getNumNotes())]
        ['C4', 'E4']
    )pbdoc");

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
            py::arg("noteId"), py::arg("staveId") = 0, py::return_value_policy::reference_internal,
            R"pbdoc(
        Get the note at a given index on a given stave, as a live reference.

        An edit made through the returned note reaches the measure and its score. The reference
        is valid until the measure gains or loses notes (``addNote``, ``removeNote``, ``clear``,
        or an edit of its part or score that rebuilds the measure); fetch the note again after
        that. ``Chord.getNote`` and ``Part.getMeasures`` return copies.

        Parameters
        ----------
        noteId : int
            Index of the note within the stave, in ``0 .. getNumNotes(staveId) - 1``.
        staveId : int, optional
            Stave index (default: 0).

        Returns
        -------
        Note
            The note at ``noteId`` on ``staveId``.

        Raises
        ------
        IndexError
            If ``staveId`` or ``noteId`` is negative or out of range (e.g. on an empty stave).

        Examples
        --------
        >>> score = ml.Score(["Flute"], 1)
        >>> score.getPart(0).getMeasure(0).addNote("C4")
        >>> score.getPart(0).getMeasure(0).getNote(0).setPitch("D4")
        >>> score.getPart(0).getMeasure(0).getNote(0).getPitch()
        'D4'
    )pbdoc");

    cls.def("getNoteOn", py::overload_cast<const int, const int>(&Measure::getNoteOn),
            py::arg("noteOnId"), py::arg("staveId") = 0,
            py::return_value_policy::reference_internal,
            R"pbdoc(
        Get the sounding note (not a rest) at a given index among the stave's sounding notes, as
        a live reference.

        An edit made through the returned note reaches the measure and its score. The reference
        is valid until the measure gains or loses notes; fetch the note again after that.

        Parameters
        ----------
        noteOnId : int
            Index among the sounding notes of the stave, in ``0 .. getNumNotesOn(staveId) - 1``.
        staveId : int, optional
            Stave index (default: 0).

        Raises
        ------
        IndexError
            If ``staveId`` is not a stave of the measure, or ``noteOnId`` is negative or not
            below the stave's number of sounding notes.
    )pbdoc");

    cls.def("getNoteOff", py::overload_cast<const int, const int>(&Measure::getNoteOff),
            py::arg("noteOffId"), py::arg("staveId") = 0,
            py::return_value_policy::reference_internal,
            R"pbdoc(
        Get the rest at a given index among the stave's rests, as a live reference.

        An edit made through the returned note reaches the measure and its score. The reference
        is valid until the measure gains or loses notes; fetch the note again after that.

        Parameters
        ----------
        noteOffId : int
            Index among the rests of the stave, in ``0 .. getNumNotesOff(staveId) - 1``.
        staveId : int, optional
            Stave index (default: 0).

        Raises
        ------
        IndexError
            If ``staveId`` is not a stave of the measure, or ``noteOffId`` is negative or not
            below the stave's number of rests.
    )pbdoc");

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

    cls.def("toXML", py::overload_cast<const int, const int>(&Measure::toXML, py::const_),
            py::arg("instrumentId") = 1, py::arg("identSize") = 2);
    cls.def("toJSON", &Measure::toJSON);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Measure& measure) {
        return "<Measure " + std::to_string(measure.getNumber()) + ">";
    });

    cls.def("__hash__",
            [](const Measure& measure) { return std::hash<std::string>{}(measure.toXML()); });

    cls.def("__sizeof__", [](const Measure& measure) { return sizeof(measure); });
}
