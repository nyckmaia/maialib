#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <bitset>
#include <unordered_map>

#include "maiacore/note.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;

void NoteClass(const py::module& m) {
    m.doc() = "Note class binding";

    py::class_<Note> cls(m, "Note");

    cls.def(py::init<const std::string&, const RhythmFigure, bool, bool, const int, const int,
                     const int>(),
            py::arg("pitch"), py::arg("rhythmFigure") = RhythmFigure::QUARTER,
            py::arg("isNoteOn") = true, py::arg("inChord") = false,
            py::arg("transposeDiatonic") = 0, py::arg("transposeChromatic") = 0,
            py::arg("divisionsPerQuarterNote") = 256,
            R"pbdoc(
        Create a note from a pitch string.

        Parameters
        ----------
        pitch : str
            Pitch string such as ``"C4"``, ``"G#3"``, ``"Dbb-1"`` or ``"Bx11"``. Accidentals:
            ``bb``, ``b``, ``#``, ``x``. Octaves: -1 to 11 (default 4). The MIDI number must be
            >= 0. An empty string or any string containing ``"rest"`` creates a rest.
        rhythmFigure : RhythmFigure, default RhythmFigure.QUARTER
            Rhythm figure.
        isNoteOn : bool, default True
            False creates a rest.
        inChord : bool, default False
            Whether the note belongs to a chord.
        transposeDiatonic : int, default 0
            Diatonic transposition interval (transposing instruments).
        transposeChromatic : int, default 0
            Chromatic transposition interval (transposing instruments).
        divisionsPerQuarterNote : int, default 256
            Divisions per quarter note.

        Raises
        ------
        RuntimeError
            If the pitch string is invalid.
    )pbdoc");

    cls.def(py::init<const int, const std::string&, const RhythmFigure, bool, bool, const int,
                     const int, const int>(),
            py::arg("midiNumber"), py::arg("accType") = "",
            py::arg("rhythmFigure") = RhythmFigure::QUARTER, py::arg("isNoteOn") = true,
            py::arg("inChord") = false, py::arg("transposeDiatonic") = 0,
            py::arg("transposeChromatic") = 0, py::arg("divisionsPerQuarterNote") = 256,
            R"pbdoc(
        Create a note from a MIDI note number.

        Parameters
        ----------
        midiNumber : int
            MIDI note number, spelled within octaves -1 to 11 (e.g. 5 -> ``"F-1"``). -1 creates
            a rest.
        accType : str, default ""
            ``""``, ``"#"``, ``"b"``, ``"x"`` or ``"bb"`` (see ``Helper.midiNote2pitch``).
        rhythmFigure : RhythmFigure, default RhythmFigure.QUARTER
            Rhythm figure.
        isNoteOn : bool, default True
            False creates a rest.
        inChord : bool, default False
            Whether the note belongs to a chord.
        transposeDiatonic : int, default 0
            Diatonic transposition interval.
        transposeChromatic : int, default 0
            Chromatic transposition interval.
        divisionsPerQuarterNote : int, default 256
            Divisions per quarter note.

        Raises
        ------
        RuntimeError
            If the MIDI number cannot be spelled with ``accType`` within octaves -1 to 11.
    )pbdoc");

    // ====== Methods SETTERS for class Note ===== //
    cls.def("setPitchClass", &Note::setPitchClass, py::arg("pitchClass"),
            "Set the note pitch class");
    cls.def("setOctave", &Note::setOctave, py::arg("octave"));
    cls.def("setStep", &Note::setStep, py::arg("step"),
            R"pbdoc(
        Set the diatonic step of the written pitch, keeping the current accidental and octave.

        Delegates to the underlying pitch's step setter and inherits its policy: permissive on
        a rest, resurrecting it into a note with the octave defaulted to 4.

        Parameters
        ----------
        step : str
            Diatonic step ("A" to "G").

        Raises
        ------
        RuntimeError
            If step is not one of "A" to "G".
    )pbdoc");
    cls.def("setAlter", &Note::setAlter, py::arg("alter"),
            R"pbdoc(
        Set the accidental value (in semitones) of the written pitch.

        Delegates to the underlying pitch's alter setter and inherits its policy: refuses on a
        rest (logs a warning, no mutation) since a bare alter value carries no octave to
        resurrect one with.

        Parameters
        ----------
        alter : float
            Alter value; must be a multiple of 0.5 (a semitone or quarter-tone step), within
            [-2, 2].

        Raises
        ------
        RuntimeError
            If alter is not a multiple of 0.5, or is outside [-2, 2].
    )pbdoc");
    cls.def("setDuration", py::overload_cast<const Duration&>(&Note::setDuration),
            py::arg("duration"));
    cls.def("setDuration", py::overload_cast<const float, const int>(&Note::setDuration),
            py::arg("quarterDuration"), py::arg("divisionsPerQuarterNote") = 256,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    //     cls.def("setDuration", py::overload_cast<const RhythmFigure, const
    //     int>(&Note::setDuration),
    //             py::arg("rhythmFigure"), py::arg("divisionsPerQuarterNote") = 256);
    //     cls.def("setDuration", py::overload_cast<const float, const int, const
    //     int>(&Note::setDuration),
    //             py::arg("durationValue"), py::arg("lowerTimeSignatureValue") = 4,
    //             py::arg("divisionsPerQuarterNote") = 256);

    //     cls.def("setDurationTicks", &Note::setDurationTicks, py::arg("durationTicks"));
    cls.def("setIsNoteOn", &Note::setIsNoteOn, py::arg("isNoteOn"));
    cls.def("setPitch", &Note::setPitch, py::arg("pitch"),
            R"pbdoc(
        Set the pitch of the note.

        Replaces the pitch class, octave, accidental symbol and MIDI number.

        Parameters
        ----------
        pitch : str
            Pitch string with the same rules as the pitch-string constructor (e.g. ``"Bb-1"``,
            ``"C10"``). An empty string or any string containing ``"rest"`` turns the note into
            a rest.

        Raises
        ------
        RuntimeError
            If the pitch string is invalid.
    )pbdoc");
    cls.def("setIsInChord", &Note::setIsInChord, py::arg("inChord"));
    cls.def("setTransposingInterval", &Note::setTransposingInterval, py::arg("diatonicInterval"),
            py::arg("chromaticInterval"));

    cls.def("setVoice", &Note::setVoice, py::arg("voice"));
    cls.def("setStaff", &Note::setStaff, py::arg("staff"));
    cls.def("setIsGraceNote", &Note::setIsGraceNote, py::arg("isGraceNote") = false);
    cls.def("setStem", &Note::setStem, py::arg("stem"));
    //     cls.def("removeDots", &Note::removeDots);
    //     cls.def("setSingleDot", &Note::setSingleDot);
    //     cls.def("setDoubleDot", &Note::setDoubleDot);
    cls.def("setTieStart", &Note::setTieStart);
    cls.def("setTieStop", &Note::setTieStop);
    cls.def("setTieStopStart", &Note::setTieStopStart);
    cls.def("addTie", &Note::addTie, py::arg("tieType"));
    cls.def("addSlur", &Note::addSlur, py::arg("slurType"), py::arg("slurOrientation"));
    cls.def("addArticulation", &Note::addArticulation, py::arg("articulation"));
    cls.def("addBeam", &Note::addBeam, py::arg("beam"));
    cls.def("setIsTuplet", &Note::setIsTuplet, py::arg("isTuplet") = false);
    cls.def("setTupleValues", &Note::setTupleValues, py::arg("actualNotes"), py::arg("normalNotes"),
            py::arg("normalType") = "eighth");
    cls.def("info", &Note::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("setIsPitched", &Note::setIsPitched, py::arg("isPitched") = true);
    cls.def("isPitched", &Note::isPitched);

    cls.def("setUnpitchedIndex", &Note::setUnpitchedIndex);
    cls.def("getUnpitchedIndex", &Note::getUnpitchedIndex);

    // ===================== Method GETTERS for class Note ===== //
    cls.def("getWrittenPitchStep", &Note::getWrittenPitchStep);
    cls.def("getSoundingPitchStep", &Note::getSoundingPitchStep);
    cls.def("getPitchStep", &Note::getPitchStep);
    cls.def("getSoundingPitchClass", &Note::getSoundingPitchClass);
    cls.def("getSoundingPitch", &Note::getSoundingPitch);

    cls.def("getWrittenPitchClass", &Note::getWrittenPitchClass);
    cls.def("getWrittenPitch", &Note::getWrittenPitch);

    cls.def("getDiatonicWrittenPitchClass", &Note::getDiatonicWrittenPitchClass);
    cls.def("getDiatonicSoundingPitchClass", &Note::getDiatonicSoundingPitchClass);

    cls.def("getSoundingOctave", &Note::getSoundingOctave,
            R"pbdoc(
        Return the sounding octave (after transposition).

        Arithmetic (written MIDI + transposeChromatic), so this is ``None`` in two cases: the
        note is a rest, or the note is sounding but transposition carries its sounding pitch
        below the representable minimum C-1 / MIDI 0. ``isNoteOff()`` alone does not cover the
        second case.

        Returns
        -------
        int or None
            Sounding octave number, or ``None`` if this note is a rest or its sounding pitch
            falls below MIDI 0.
    )pbdoc");
    cls.def("getWrittenOctave", &Note::getWrittenOctave,
            R"pbdoc(
        Return the written octave (as notated).

        Returns
        -------
        int or None
            Written octave number, or ``None`` for a rest.
    )pbdoc");

    cls.def("getPitchClass", &Note::getPitchClass);
    cls.def("getOctave", &Note::getOctave,
            R"pbdoc(
        Return the octave (sounding).

        Returns
        -------
        int or None
            Octave number, or ``None`` for a rest.
    )pbdoc");

    cls.def("getType", &Note::getType);
    cls.def("getLongType", &Note::getLongType);
    cls.def("getShortType", &Note::getShortType);
    cls.def("getDurationTicks", &Note::getDurationTicks);
    cls.def("getNumDots", &Note::getNumDots);
    cls.def("isDotted", &Note::isDotted);
    cls.def("isDoubleDotted", &Note::isDoubleDotted);
    cls.def("getDivisionsPerQuarterNote", &Note::getDivisionsPerQuarterNote);
    cls.def("getDuration", &Note::getDuration);
    cls.def("getQuarterDuration", &Note::getQuarterDuration);

    cls.def("isNoteOn", &Note::isNoteOn);
    cls.def("isNoteOff", &Note::isNoteOff);
    cls.def("getPitch", &Note::getPitch);
    cls.def("getMidiNumber", &Note::getMidiNumber);

    cls.def("getVoice", &Note::getVoice);
    cls.def("getStaff", &Note::getStaff);
    cls.def("getType", &Note::getType);
    cls.def("getStem", &Note::getStem);
    cls.def("getNumDots", &Note::getNumDots);
    cls.def("getTie", &Note::getTie);
    cls.def("removeTies", &Note::removeTies);
    cls.def("getSlur", &Note::getSlur);
    cls.def("getArticulation", &Note::getArticulation);
    cls.def("getBeam", &Note::getBeam);

    cls.def("getAlterSymbol", &Note::getAlterSymbol);

    cls.def("isQuarterTone", &Note::isQuarterTone,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Check whether this note carries a quarter-tone accidental.

        Returns
        -------
        bool
            True if the note's alter value has a fractional part (e.g. ``Note("C1x4")``,
            ``Note("D3b4")``); False for the whole-tone accidentals and for a rest.
    )pbdoc");

    cls.def("roundToSemitone", &Note::roundToSemitone,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Round a quarter-tone accidental to the nearest semitone, ties upward.

        ``C1x4`` becomes ``C#4``, ``D1b4`` becomes ``D4`` and ``D3b4`` becomes ``Db4``. A note
        that already carries a whole-tone accidental (or none) is unchanged, so this is safe to
        call unconditionally.

        Use it to make a quarter-tone note acceptable to ``Interval``, which rejects quarter
        tones at construction. The rounding is destructive: the original quarter-tone spelling
        is not recoverable afterwards.
    )pbdoc");

    cls.def("inChord", &Note::inChord);
    cls.def("getTransposeDiatonic", &Note::getTransposeDiatonic);
    cls.def("getTransposeChromatic", &Note::getTransposeChromatic);
    cls.def("isTransposed", &Note::isTransposed);
    cls.def("isGraceNote", &Note::isGraceNote);

    cls.def("getEnharmonicPitch", &Note::getEnharmonicPitch,
            py::arg("alternativeEnharmonicPitch") = false,
            R"pbdoc(
        Return an enharmonic spelling of the note.

        White keys: a natural returns its flat-side spelling (``C4`` -> ``Dbb4``) and the
        sharp-side spelling as the alternative (``B#3``); other spellings return the natural and,
        as the alternative, the remaining spelling. Black keys: ``#`` and ``b`` swap
        (``C#4`` <-> ``Db4``) with the double accidental as the alternative; ``x``/``bb`` return
        the single accidental in the same direction and, as the alternative, the opposite one.
        Spellings outside octaves -1 to 11 do not exist: a missing alternative returns the
        default and a missing default returns the note's own pitch.

        Parameters
        ----------
        alternativeEnharmonicPitch : bool, default False
            Return the alternative spelling instead of the default one.

        Returns
        -------
        str
            Enharmonic pitch string, or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("C#4").getEnharmonicPitch()
        'Db4'
    )pbdoc");
    cls.def("getEnharmonicPitches", &Note::getEnharmonicPitches,
            py::arg("includeCurrentPitch") = false,
            R"pbdoc(
        Return the default and alternative enharmonic spellings.

        Parameters
        ----------
        includeCurrentPitch : bool, default False
            Prepend the note's own pitch.

        Returns
        -------
        list of str
            ``[default, alternative]`` (optionally preceded by the current pitch). Entries may
            repeat, e.g. ``["G#4", "Ab4", "Ab4"]``.
    )pbdoc");

    cls.def("getEnharmonicNote", &Note::getEnharmonicNote,
            py::arg("alternativeEnharmonicPitch") = false,
            R"pbdoc(
        Return a new Note with an enharmonic spelling (see ``getEnharmonicPitch``).

        Parameters
        ----------
        alternativeEnharmonicPitch : bool, default False
            Use the alternative spelling instead of the default one.

        Returns
        -------
        Note
            Enharmonic note.
    )pbdoc");
    cls.def("getEnharmonicNotes", &Note::getEnharmonicNotes, py::arg("includeCurrentPitch") = false,
            R"pbdoc(
        Return Notes for the default and alternative enharmonic spellings.

        Parameters
        ----------
        includeCurrentPitch : bool, default False
            Prepend a copy of the current note.

        Returns
        -------
        list of Note
            Enharmonic notes (entries may repeat, see ``getEnharmonicPitches``).
    )pbdoc");

    cls.def("toEnharmonicPitch", &Note::toEnharmonicPitch,
            py::arg("alternativeEnharmonicPitch") = false,
            R"pbdoc(
        Respell the note in place with an enharmonic spelling (see ``getEnharmonicPitch``).

        Parameters
        ----------
        alternativeEnharmonicPitch : bool, default False
            Use the alternative spelling instead of the default one.
    )pbdoc");
    cls.def("getScaleDegree", &Note::getScaleDegree, py::arg("key"));
    cls.def("getFrequency", &Note::getFrequency, py::arg("freqA4") = 440.0f);
    cls.def("getHarmonicSpectrum", &Note::getHarmonicSpectrum, py::arg("numPartials") = 6,
            py::arg("amplCallback") = nullptr, py::arg("partialsDecayExpRate") = 0.88f,
            py::arg("freqA4") = 440.0f);

    cls.def("transpose", &Note::transpose, py::arg("semitones"),
            py::arg("accType") = MUSIC_XML::ACCIDENT::NONE);

    cls.def("toXML", &Note::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    // Default Python 'print' function:
    cls.def("__repr__", [](const Note& note) { return "<Note " + note.getPitch() + ">"; });

    cls.def("__hash__", [](const Note& note) {
        const std::string temp01 = note.getSoundingPitch() + note.getType();
        const int temp02 = (int)note.isNoteOn() | ((int)note.inChord() << 1) |
                           ((int)note.isGraceNote() << 2) | ((int)note.isTuplet() << 3) |
                           ((int)note.isPitched() << 4);

        const std::string temp02Str = std::bitset<8>(temp02).to_string();

        return std::hash<std::string>{}(temp01 + temp02Str);
    });

    cls.def("__sizeof__", [](const Note& note) { return sizeof(note); });

    cls.def(py::self < py::self);
    cls.def(py::self > py::self);
    cls.def(py::self <= py::self);
    cls.def(py::self >= py::self);
    cls.def(py::self == py::self);
    cls.def(py::self != py::self);
}
