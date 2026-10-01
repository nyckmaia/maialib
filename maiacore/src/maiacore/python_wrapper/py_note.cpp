#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <functional>
#include <string>
#include <unordered_map>

#include "../pitch-views.h"
#include "maiacore/note.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;

using maiacore::detail::concertPitch;

void NoteClass(const py::module& m) {
    m.doc() = "Note class binding";

    py::class_<Note> cls(m, "Note", R"pbdoc(
        A musical note: its pitch, duration, articulations and MusicXML attributes.

        Notes
        -----
        A note of a transposing instrument is written at one pitch and sounds at another, so a
        note answers about its pitch in three views:

        - Written, the pitch as written in the part: ``getWrittenPitch``,
          ``getWrittenOctave``, ``getWrittenPitchClass``, ``getWrittenPitchStep`` and
          ``getDiatonicWrittenPitchClass``. The unprefixed getters ``getPitch``, ``getOctave``,
          ``getPitchClass``, ``getPitchStep`` and ``getAlterSymbol`` are shortcuts for it; the
          enharmonic family (``getEnharmonicPitch``, ``getEnharmonicPitches``,
          ``getEnharmonicNote``, ``getEnharmonicNotes``, ``toEnharmonicPitch``),
          ``transpose``, the setters and ``toXML`` work on it.
        - Sounding, what the note sounds, in its simplest spelling: ``getSoundingPitch``,
          ``getSoundingOctave``, ``getSoundingPitchClass``, ``getSoundingPitchStep`` and
          ``getDiatonicSoundingPitchClass``. The written pitch is moved by the transposing
          interval -- its letter by ``transposeDiatonic``, its position by
          ``transposeChromatic`` -- and respelled with the smallest accidental any spelling of
          that position has; between a sharp and a flat equally close, the moved spelling's side
          is kept. The octave is that spelling's own (see ``getSoundingPitch``).
        - Acoustic, measures of what sounds: ``getMidiNumber``, ``getQuarterToneSteps``,
          ``getFrequency`` and ``getHarmonicSpectrum``.

        Without a transposing interval the written and the sounding pitch are the same pitch,
        spelled alike unless the written spelling has a simpler one: an untransposed ``Cb4``
        sounds ``B3``. ``Chord``, ``Interval`` and a score's chord and melody analyses relate a
        note of a transposing instrument at the pitch it sounds, spelled with its written letter
        moved by the diatonic transposing interval -- or, where that gives no spelling, by the
        fallback described on ``getSoundingPitch``, without the simplification: with
        ``transposeDiatonic=0`` and ``transposeChromatic=-2``, a written ``C4`` is a ``Bb3``
        there, not a ``Cbb4`` -- and ``==`` compares notes the same way.

        Examples
        --------
        >>> clarinet = ml.Note("Db4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> clarinet.getPitch(), clarinet.getSoundingPitch(), clarinet.getMidiNumber()
        ('Db4', 'B3', 59)
        >>> clarinet == ml.Note("Cb4")
        True
    )pbdoc");

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
            Pitch string such as ``"C4"``, ``"G#3"``, ``"Dbb-1"``, ``"Bx11"`` or ``"C1x4"``.
            Accidentals: ``bb``, ``b``, ``#``, ``x`` and the quarter tones ``3b``, ``1b``,
            ``1x``, ``3x``. Octaves: -1 to 11 (default 4). The MIDI number, rounded ties upward
            for a quarter tone, must be >= 0. An empty string or any string containing
            ``"rest"`` creates a rest.
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
            If the pitch string is invalid, or if the note is transposed and its sounding pitch
            cannot be spelled: one above ``"B11"`` (MIDI note 155) that the diatonic interval
            does not spell (see ``setTransposingInterval``).

        Examples
        --------
        >>> ml.Note("C1x4").getMidiNumber()
        61
        >>> ml.Note("C1x4").isQuarterTone()
        True
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
            If the MIDI number cannot be spelled with ``accType`` within octaves -1 to 11, or if
            the note is transposed and its sounding pitch cannot be spelled: one above ``"B11"``
            (MIDI note 155) that the diatonic interval does not spell (see
            ``setTransposingInterval``).
    )pbdoc");

    // ====== Methods SETTERS for class Note ===== //
    cls.def("setPitchClass", &Note::setPitchClass, py::arg("pitchClass"),
            R"pbdoc(
        Set the step and accidental of the written pitch, keeping its octave.

        A rest has no octave to keep, so it becomes a note in octave 4. The transposing interval
        is kept, and the MIDI number and every other getter follow the new pitch.

        On a transposing instrument the change is checked the way ``setPitch`` checks a whole
        pitch: if the written pitch it gives would sound where no spelling reaches -- above
        ``"B11"`` (MIDI note 155), unless the diatonic interval spells it (see
        ``setTransposingInterval``) -- it raises and the note is left unchanged. A sounding pitch
        below ``C1b-1`` is accepted, as ``setPitch`` accepts it.

        Parameters
        ----------
        pitchClass : str
            Pitch class without an octave, e.g. ``"Bb"`` or ``"D1x"``. Any string containing
            ``"rest"`` makes the note a rest.

        Raises
        ------
        RuntimeError
            If the pitch class is invalid, or the pitch it gives lies below MIDI note 0 (e.g.
            ``"Cb"`` on a ``C-1``), or if its sounding pitch cannot be spelled -- the error
            ``setPitch`` raises for that pitch. The note is then left unchanged.

        Examples
        --------
        >>> note = ml.Note("C4")
        >>> note.setPitchClass("Bb")
        >>> note.getPitch(), note.getMidiNumber()
        ('Bb4', 70)
        >>> rest = ml.Note("rest")
        >>> rest.setPitchClass("D1x")
        >>> rest.getPitch()
        'D1x4'
    )pbdoc");
    // The setters that can refuse a value print a warning through std::cout (LOG_WARN), so they
    // redirect it to Python's sys.stdout, where a notebook shows it and a test can capture it.
    cls.def("setOctave", &Note::setOctave, py::arg("octave"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the octave of the written pitch, keeping its step and accidental.

        Refused on a rest, whatever the value, and when the result would lie below MIDI note 0:
        in both cases a warning is printed to standard output and the note is left unchanged.
        ``getOctave()`` afterwards reports the same octave a note constructed with that pitch
        would.

        On a transposing instrument the change is checked the way ``setPitch`` checks a whole
        pitch: if the written pitch it gives would sound where no spelling reaches -- above
        ``"B11"`` (MIDI note 155), unless the diatonic interval spells it (see
        ``setTransposingInterval``) -- it raises and the note is left unchanged. A sounding pitch
        below ``C1b-1`` is accepted, as ``setPitch`` accepts it.

        Parameters
        ----------
        octave : int
            Octave from -1 to 11.

        Raises
        ------
        TypeError
            If ``octave`` is not an int -- e.g. None, which ``getOctave()`` returns for a rest.
        RuntimeError
            If the note is not a rest and ``octave`` lies outside -1 to 11, or if the sounding
            pitch the change gives cannot be spelled -- the error ``setPitch`` raises for that
            pitch. The note is then left unchanged.

        Examples
        --------
        >>> note = ml.Note("C1x4")
        >>> note.setOctave(5)
        >>> note.getPitch()
        'C1x5'
    )pbdoc");
    cls.def("setStep", &Note::setStep, py::arg("step"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the diatonic step of the written pitch, keeping the current accidental and octave.

        Delegates to the underlying pitch's step setter and inherits its policy: permissive on
        a rest, resurrecting it into a note with the octave defaulted to 4. On a note, a step
        that would move it below MIDI note 0 is refused: a warning is printed and the note is
        left unchanged.

        On a transposing instrument the change is checked the way ``setPitch`` checks a whole
        pitch: if the written pitch it gives would sound where no spelling reaches -- above
        ``"B11"`` (MIDI note 155), unless the diatonic interval spells it (see
        ``setTransposingInterval``) -- it raises and the note is left unchanged. A sounding pitch
        below ``C1b-1`` is accepted, as ``setPitch`` accepts it.

        Parameters
        ----------
        step : str
            Diatonic step ("A" to "G").

        Raises
        ------
        RuntimeError
            If step is not one of "A" to "G", or if the sounding pitch the change gives cannot
            be spelled -- the error ``setPitch`` raises for that pitch. The note is then left
            unchanged.
    )pbdoc");
    cls.def("setAlter", &Note::setAlter, py::arg("alter"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the accidental value (in semitones) of the written pitch.

        Delegates to the underlying pitch's alter setter and inherits its policy: refused on a
        rest, since a bare alter value carries no octave to resurrect one with, and when the
        result would lie below MIDI note 0 -- in both cases a warning is printed and the note is
        left unchanged. The value reaches the library as a 32-bit float, which must be exactly
        a multiple of 0.5: a value merely close to one, such as 0.99996, is rejected rather than
        rounded, but a Python float within float32 precision of one becomes that multiple on
        the way in and is accepted (``setAlter(0.9999999999)`` sets 1.0).

        On a transposing instrument the change is checked the way ``setPitch`` checks a whole
        pitch: if the written pitch it gives would sound where no spelling reaches -- above
        ``"B11"`` (MIDI note 155), unless the diatonic interval spells it (see
        ``setTransposingInterval``) -- it raises and the note is left unchanged. A sounding pitch
        below ``C1b-1`` is accepted, as ``setPitch`` accepts it.

        Parameters
        ----------
        alter : float
            Alter value; must be a multiple of 0.5 (a semitone or quarter-tone step), within
            [-2, 2].

        Raises
        ------
        RuntimeError
            If alter is NaN or infinite, is not exactly a multiple of 0.5, or is outside
            [-2, 2], or if the sounding pitch the change gives cannot be spelled -- the error
            ``setPitch`` raises for that pitch. The note is then left unchanged.
    )pbdoc");
    cls.def("setDuration", py::overload_cast<const Duration&>(&Note::setDuration),
            py::arg("duration"));
    cls.def("setDuration", py::overload_cast<const float, const int>(&Note::setDuration),
            py::arg("quarterDuration"), py::arg("divisionsPerQuarterNote") = 256,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the duration in quarter notes: 1.0 is a quarter note, 0.5 an eighth, 1.5 a dotted
        quarter.

        Parameters
        ----------
        quarterDuration : float
            Duration in quarter notes.
        divisionsPerQuarterNote : int, default 256
            Ticks per quarter note, used to convert the duration.

        Raises
        ------
        RuntimeError
            If the duration cannot be converted to a rhythm figure (e.g. 0 or a negative
            value); the note is then left unchanged.

        Examples
        --------
        >>> note = ml.Note("C4")
        >>> note.setDuration(0.5)
        >>> note.getType()
        'eighth'
    )pbdoc");

    //     cls.def("setDuration", py::overload_cast<const RhythmFigure, const
    //     int>(&Note::setDuration),
    //             py::arg("rhythmFigure"), py::arg("divisionsPerQuarterNote") = 256);
    //     cls.def("setDuration", py::overload_cast<const float, const int, const
    //     int>(&Note::setDuration),
    //             py::arg("durationValue"), py::arg("lowerTimeSignatureValue") = 4,
    //             py::arg("divisionsPerQuarterNote") = 256);

    //     cls.def("setDurationTicks", &Note::setDurationTicks, py::arg("durationTicks"));
    cls.def("setIsNoteOn", &Note::setIsNoteOn, py::arg("isNoteOn"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Make the note a rest, or keep it sounding.

        ``setIsNoteOn(False)`` makes the note a rest everywhere: its pitch is gone and every
        getter answers as for a rest (``getPitch()`` is ``"rest"``, ``getMidiNumber()`` -1,
        ``getOctave()`` None). Its transposing interval is kept. ``setIsNoteOn(True)`` leaves a
        sounding note as it is; a rest has no pitch it could sound, so it prints a warning and
        stays a rest -- give it a pitch with ``setPitch``, ``setPitchClass`` or ``setStep``.

        Parameters
        ----------
        isNoteOn : bool
            False makes the note a rest.

        Examples
        --------
        >>> note = ml.Note("C4")
        >>> note.setIsNoteOn(False)
        >>> note.getPitch(), note.getMidiNumber(), note.isNoteOff()
        ('rest', -1, True)
        >>> note.setIsNoteOn(True)  # doctest: +ELLIPSIS
        [WARN] Cannot turn a rest into a sounding note without a pitch; ignoring ...
        >>> note.isNoteOff()
        True
    )pbdoc");
    cls.def("setPitch", &Note::setPitch, py::arg("pitch"),
            R"pbdoc(
        Set the written pitch of the note.

        Replaces the step, accidental and octave. A transposing interval is kept, so the
        sounding pitch, ``getSoundingPitch()``, is this pitch moved by it. Setting a rest also
        clears the transposing interval and the in-chord and grace-note flags.

        Parameters
        ----------
        pitch : str
            Pitch string with the same rules as the pitch-string constructor (e.g. ``"Bb-1"``,
            ``"C10"``). An empty string or any string containing ``"rest"`` turns the note into
            a rest.

        Raises
        ------
        RuntimeError
            If the pitch string is invalid, or if its sounding pitch with the note's transposing
            interval cannot be spelled: one above ``"B11"`` (MIDI note 155) that the diatonic
            interval does not spell (see ``setTransposingInterval``). The note is then left
            unchanged. A sounding pitch below ``C1b-1`` is accepted, as
            ``setTransposingInterval`` accepts it.
    )pbdoc");
    cls.def("setIsInChord", &Note::setIsInChord, py::arg("inChord"));
    cls.def("setTransposingInterval", &Note::setTransposingInterval, py::arg("diatonicInterval"),
            py::arg("chromaticInterval"),
            R"pbdoc(
        Set the transposing interval: the note is written at its written pitch and sounds that
        pitch moved by the interval -- its letter by ``diatonicInterval`` letters, its position
        by ``chromaticInterval`` semitones (see ``getSoundingPitch``).

        Every sounding getter derives the sounding pitch from the written pitch and the interval
        when asked, so a second call replaces the interval rather than applying it again. The
        sounding pitch is also derived here, before the interval is stored, so one that cannot be
        spelled raises now, with the note unchanged. Every sounding pitch up to ``"B11"`` (MIDI
        note 155) can be spelled; above it only ``B1x11``, ``B#11``, ``B3x11`` and ``Bx11`` lie
        within octaves -1 to 11, and they are spelled only when the diatonic interval moves the
        written letter to the B of octave 11: a written ``A#11`` moved up a major second sounds
        ``B#11``, while with no diatonic interval it could not be spelled. On a rest this does
        nothing: a rest has no pitch to transpose.

        Parameters
        ----------
        diatonicInterval : int
            Diatonic steps from the written to the sounding pitch, e.g. -1 for a B-flat
            clarinet.
        chromaticInterval : int
            Semitones from the written to the sounding pitch, e.g. -2 for a B-flat clarinet.

        Raises
        ------
        RuntimeError
            If the sounding pitch cannot be spelled: one above ``"B11"`` that the diatonic
            interval does not spell. The note is then left unchanged. A sounding pitch below the
            lowest representable pitch, ``C1b-1``, is not raised here: the interval is stored,
            the note stays constructible and each sounding getter raises instead (see
            ``getSoundingPitch``).

        Examples
        --------
        >>> note = ml.Note("C4")
        >>> note.setTransposingInterval(-1, -2)
        >>> note.getPitch(), note.getSoundingPitch(), note.getMidiNumber()
        ('C4', 'Bb3', 58)
    )pbdoc");

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
            py::arg("normalType") = "eighth",
            R"pbdoc(
        Set the tuplet ratio: ``actualNotes`` notes in the time of ``normalNotes`` notes of
        ``normalType``. They are written to MusicXML when the note is a tuplet (``setIsTuplet``).

        Parameters
        ----------
        actualNotes : int
            Number of notes in the tuplet, e.g. 3 for a triplet.
        normalNotes : int
            Number of normal notes they take the time of, e.g. 2 for a triplet.
        normalType : str, default "eighth"
            Note type of the normal notes.

        Raises
        ------
        RuntimeError
            If ``normalType`` is not a note type; the note is then left unchanged.

        Examples
        --------
        >>> note = ml.Note("C4")
        >>> note.setIsTuplet(True)
        >>> note.setTupleValues(3, 2, "eighth")
        >>> "<actual-notes>3</actual-notes>" in note.toXML()
        True
    )pbdoc");
    cls.def("info", &Note::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Print the note's properties, one per line: whether it is sounding, its pitch
        (``getPitch()``, the written pitch), type, quarter duration, voice, staff, MIDI number
        (of what it sounds), stem, beams, tuplet, grace-note and in-chord flags, and transposing
        interval.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``, once it reaches the MIDI number; see
            ``getSoundingPitch``.
    )pbdoc");

    cls.def("setIsPitched", &Note::setIsPitched, py::arg("isPitched") = true);
    cls.def("isPitched", &Note::isPitched);

    cls.def("setUnpitchedIndex", &Note::setUnpitchedIndex);
    cls.def("getUnpitchedIndex", &Note::getUnpitchedIndex);

    // ===================== Method GETTERS for class Note ===== //
    cls.def("getWrittenPitchStep", &Note::getWrittenPitchStep,
            R"pbdoc(
        Return the diatonic step of the written pitch (as notated).

        Returns
        -------
        str
            ``"A"`` to ``"G"``, or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getWrittenPitchStep()
        'C'
        >>> ml.Note("rest").getWrittenPitchStep()
        'rest'
    )pbdoc");
    cls.def("getSoundingPitchStep", &Note::getSoundingPitchStep,
            R"pbdoc(
        Return the diatonic step of the sounding pitch (see ``getSoundingPitch``).

        Spelled as ``getSoundingPitch`` spells it, in its simplest spelling, with the same silent
        fallback for an interval that gives no letter.

        Returns
        -------
        str
            ``"A"`` to ``"G"``, or ``"rest"`` for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getSoundingPitchStep()
        'B'
        >>> ml.Note("E1b4").getSoundingPitchStep()
        'E'
    )pbdoc");
    cls.def("getPitchStep", &Note::getPitchStep,
            R"pbdoc(
        Return the diatonic step of the written pitch: a shortcut for ``getWrittenPitchStep``.

        The step as notated in the part, whatever the instrument sounds; the step it sounds is
        ``getSoundingPitchStep``. Never raises.

        Returns
        -------
        str
            ``"A"`` to ``"G"``, or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("E1b4").getPitchStep()
        'E'
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getPitchStep()
        'C'
        >>> ml.Note("rest").getPitchStep()
        'rest'
    )pbdoc");
    cls.def("getSoundingPitchClass", &Note::getSoundingPitchClass,
            R"pbdoc(
        Return the pitch class of the sounding pitch (see ``getSoundingPitch``): its step and
        accidental, without the octave.

        Spelled as ``getSoundingPitch`` spells it, in its simplest spelling, with the same silent
        fallback for an interval that gives no letter: an untransposed ``Cb4`` gives ``"B"``.

        Returns
        -------
        str
            Pitch class (e.g. ``"C1x"``, ``"Bb"``), or ``"rest"`` for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getSoundingPitchClass()
        'Bb'
        >>> ml.Note("C1x4").getSoundingPitchClass()
        'C1x'
    )pbdoc");
    cls.def("getSoundingPitch", &Note::getSoundingPitch,
            R"pbdoc(
        Return the sounding pitch: what the note sounds, in its simplest spelling.

        The written pitch moved by the transposing interval -- its letter by
        ``transposeDiatonic`` letters, carrying octaves across C, its exact position by
        ``transposeChromatic`` semitones -- and then respelled with the smallest accidental any
        spelling of that position has within octaves -1 to 11. Between a sharp and a flat equally
        close, the side of the moved spelling is kept. The octave is that spelling's own. A
        B-flat clarinet's written ``Db4`` sounds ``"B3"`` and its written ``Eb4`` ``"Db4"``; a
        quarter tone keeps its fraction (its written ``C1x4`` sounds ``"B1b3"``). Without a
        transposing interval this is the written pitch in its simplest spelling: an untransposed
        ``Cb4`` sounds ``"B3"`` and ``C3x4`` sounds ``"D1b4"``, while ``Db4`` stays ``"Db4"``.

        Fallback, silent (no warning is given): when the interval gives no letter --
        ``transposeDiatonic`` is 0 while ``transposeChromatic`` is not, as for a MusicXML
        ``<transpose>`` without ``<diatonic>`` -- or the letter it gives would need an accidental
        beyond a double sharp or flat, or an octave outside -1 to 11, the position is spelled from
        its semitone alone (rounded ties upward; a quarter tone stays in the accidental): as the
        white key there or the sharp of the white key below, or, when the instrument transposes
        down, as the flat of the white key above, if that key is white and the flat keeps the
        octave. With ``transposeDiatonic=0`` and ``transposeChromatic=-2``, a written ``C4`` is
        spelled ``Bb3``, a written ``F#4`` ``Fb4``, and a written ``C#4`` ``B3``, since ``Cb4``
        would leave octave 3. ``==`` and the analyses relate the note by that spelling; this
        method then simplifies it as above, so the written ``F#4`` sounds ``"E4"``. Without a
        diatonic interval a written ``D4`` sounds ``"D#4"`` a semitone up and ``"Db4"`` a semitone
        down.

        Returns
        -------
        str
            Sounding pitch string, or ``"rest"`` for a rest -- including a transposing
            instrument's note silenced with ``setIsNoteOn(False)``.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1`` (-0.5, MIDI 0) -- e.g. a B-flat clarinet's written
            ``"C#-1"``. Every sounding getter raises the same error for such a note; its written
            pitch is still available from ``getWrittenPitch()`` and ``getPitch()``. The message
            names the written pitch and the interval.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getSoundingPitch()
        'Bb3'
        >>> ml.Note("Db4", transposeDiatonic=-1, transposeChromatic=-2).getSoundingPitch()
        'B3'
        >>> ml.Note("C1x4", transposeDiatonic=-1, transposeChromatic=-2).getSoundingPitch()
        'B1b3'
        >>> ml.Note("Cb4").getSoundingPitch()
        'B3'
        >>> ml.Note("D4", transposeDiatonic=0, transposeChromatic=-1).getSoundingPitch()
        'Db4'
    )pbdoc");

    cls.def("getWrittenPitchClass", &Note::getWrittenPitchClass,
            R"pbdoc(
        Return the pitch class of the written pitch (as notated): its step and accidental,
        without the octave.

        Returns
        -------
        str
            Pitch class (e.g. ``"C1x"``, ``"Bb"``), or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getWrittenPitchClass()
        'C'
    )pbdoc");
    cls.def("getWrittenPitch", &Note::getWrittenPitch,
            R"pbdoc(
        Return the written pitch (as notated): the pitch the note was constructed or set with,
        whatever the instrument sounds.

        Returns
        -------
        str
            Pitch string (e.g. ``"C1x4"``), or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getWrittenPitch()
        'C4'
    )pbdoc");

    cls.def("getDiatonicWrittenPitchClass", &Note::getDiatonicWrittenPitchClass,
            R"pbdoc(
        Return the written pitch class without its accidental: the diatonic step.

        Returns
        -------
        str
            ``"A"`` to ``"G"``, or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("F1x4").getDiatonicWrittenPitchClass()
        'F'
        >>> ml.Note("rest").getDiatonicWrittenPitchClass()
        'rest'
    )pbdoc");
    cls.def("getDiatonicSoundingPitchClass", &Note::getDiatonicSoundingPitchClass,
            R"pbdoc(
        Return the sounding pitch class without its accidental: the diatonic step of
        ``getSoundingPitch``, spelled as it spells it, silent fallback included.

        Returns
        -------
        str
            ``"A"`` to ``"G"``, or ``"rest"`` for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> clarinet = ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> clarinet.getDiatonicSoundingPitchClass()
        'B'
    )pbdoc");

    cls.def("getSoundingOctave", &Note::getSoundingOctave,
            R"pbdoc(
        Return the octave of the sounding pitch (see ``getSoundingPitch``).

        The octave of the sounding spelling, silent fallback included: an untransposed ``Cb4``
        sounds ``B3``, octave 3, and a piccolo's written ``C4`` sounds ``C5``, octave 5.

        Returns
        -------
        int or None
            Sounding octave number, or ``None`` for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1`` (-0.5, MIDI 0) -- e.g. a B-flat clarinet's written
            ``"C#-1"``. Every sounding getter raises the same error for such a note; its written
            pitch is still available from ``getWrittenPitch()``.

        Examples
        --------
        >>> ml.Note("Cb4").getSoundingOctave()
        3
        >>> ml.Note("C4", transposeDiatonic=7, transposeChromatic=12).getSoundingOctave()
        5
    )pbdoc");
    cls.def("getWrittenOctave", &Note::getWrittenOctave,
            R"pbdoc(
        Return the written octave (as notated).

        Returns
        -------
        int or None
            Written octave number, or ``None`` for a rest.
    )pbdoc");

    cls.def("getPitchClass", &Note::getPitchClass,
            R"pbdoc(
        Return the pitch class of the written pitch: a shortcut for ``getWrittenPitchClass``.

        The step and accidental as notated in the part, without the octave, whatever the
        instrument sounds; the pitch class it sounds is ``getSoundingPitchClass``. Never raises.

        Returns
        -------
        str
            Pitch class (e.g. ``"C1x"``, ``"Bb"``), or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("C1x4").getPitchClass()
        'C1x'
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getPitchClass()
        'C'
    )pbdoc");
    cls.def("getOctave", &Note::getOctave,
            R"pbdoc(
        Return the octave of the written pitch: a shortcut for ``getWrittenOctave``.

        The octave as notated in the part, whatever the instrument sounds; the octave it sounds
        is ``getSoundingOctave``. Never raises.

        Returns
        -------
        int or None
            Octave number, or ``None`` for a rest.

        Examples
        --------
        >>> ml.Note("C4", transposeDiatonic=7, transposeChromatic=12).getOctave()
        4
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

    cls.def("isNoteOn", &Note::isNoteOn,
            R"pbdoc(
        Check whether the note sounds, i.e. is not a rest.

        A note is a rest exactly when its written pitch is one, so this is True for every note
        with a pitch -- including one whose transposing interval carries its sounding pitch below
        ``C1b-1``, which has no sounding pitch (see ``getSoundingPitch``).

        Returns
        -------
        bool
            False for a rest.

        Examples
        --------
        >>> ml.Note("C1x4").isNoteOn(), ml.Note("rest").isNoteOn()
        (True, False)
    )pbdoc");
    cls.def("isNoteOff", &Note::isNoteOff,
            R"pbdoc(
        Check whether the note is a rest: the negation of ``isNoteOn``.

        Returns
        -------
        bool
            True for a rest.

        Examples
        --------
        >>> ml.Note("rest").isNoteOff()
        True
    )pbdoc");
    cls.def("getPitch", &Note::getPitch,
            R"pbdoc(
        Return the written pitch: a shortcut for ``getWrittenPitch``.

        The pitch as notated in the part -- the one the note was constructed or set with --
        whatever the instrument sounds: a B-flat clarinet's written ``D4`` answers ``"D4"``.
        What it sounds is ``getSoundingPitch``, and ``getMidiNumber`` measures it. Never raises.

        Returns
        -------
        str
            Pitch string (e.g. ``"C1x4"``), or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Note("C1x4").getPitch()
        'C1x4'
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getPitch()
        'C4'
    )pbdoc");
    cls.def("getMidiNumber", &Note::getMidiNumber,
            R"pbdoc(
        Return the MIDI note number of the sounding pitch.

        The written pitch's MIDI number moved by the chromatic transposing interval. A quarter
        tone lies halfway between two MIDI numbers and is rounded, ties upward, to the one above:
        ``C1x4`` and ``D3b4`` (both 60.5) are 61. ``getQuarterToneSteps()`` gives the exact,
        unrounded position.

        Returns
        -------
        int
            MIDI note number, or -1 for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> ml.Note("C1x4").getMidiNumber()
        61
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getMidiNumber()
        58
    )pbdoc");

    cls.def("getQuarterToneSteps", &Note::getQuarterToneSteps,
            R"pbdoc(
        Return the exact, unrounded sounding pitch position in semitones.

        The written pitch's exact position plus the chromatic transposing interval, so a quarter
        tone keeps its half: ``C1x4`` is 60.5, where ``getMidiNumber()`` rounds it to 61. Every
        position is a multiple of 0.5, held exactly, and notes compare (``<``, ``>``, ...) by this
        value.

        Returns
        -------
        float
            The exact sounding position, a multiple of 0.5; -1.0 for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> ml.Note("C1x4").getQuarterToneSteps()
        60.5
        >>> ml.Note("C1x4", transposeDiatonic=-1, transposeChromatic=-2).getQuarterToneSteps()
        58.5
    )pbdoc");

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

    cls.def("getAlterSymbol", &Note::getAlterSymbol,
            R"pbdoc(
        Return the accidental symbol of the written pitch.

        The accidental as notated in the part, whatever the instrument sounds: a B-flat
        clarinet's written ``C4`` sounds ``Bb3``, but this returns ``""``; the sounding pitch's
        accidental is part of ``getSoundingPitchClass``. Never raises.

        Returns
        -------
        str
            One of ``"bb"``, ``"3b"``, ``"b"``, ``"1b"``, ``""`` (natural), ``"1x"``, ``"#"``,
            ``"3x"`` and ``"x"``; ``""`` for a rest.

        Examples
        --------
        >>> ml.Note("E1b4").getAlterSymbol()
        '1b'
        >>> ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2).getAlterSymbol()
        ''
    )pbdoc");

    cls.def("isQuarterTone", &Note::isQuarterTone,
            R"pbdoc(
        Check whether this note carries a quarter-tone accidental.

        Returns
        -------
        bool
            True if the note's alter value has a fractional part (e.g. ``Note("C1x4")``,
            ``Note("D3b4")``); False for the whole-tone accidentals and for a rest.
    )pbdoc");

    cls.def("roundToSemitone", &Note::roundToSemitone,
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
        Return an enharmonic spelling of the written pitch, ``getPitch()``.

        The written pitch is respelled whatever the instrument sounds, so a transposed note is
        respelled exactly as an untransposed note with the same written pitch. A semitone pitch
        is respelled among the spellings of the same MIDI number. White keys: a
        natural returns its flat-side spelling (``C4`` -> ``Dbb4``) and the sharp-side spelling
        as the alternative (``B#3``); other spellings return the natural and, as the
        alternative, the remaining spelling. Black keys: ``#`` and ``b`` swap
        (``C#4`` <-> ``Db4``) with the double accidental as the alternative; ``x``/``bb`` return
        the single accidental in the same direction and, as the alternative, the opposite one.

        A quarter tone is respelled on the 24-tone grid. Its partners are the white-key steps
        within 1.5 semitones of it, each spelled with the quarter-tone accidental that separates
        them (``1x`` +0.5, ``3x`` +1.5, ``1b`` -0.5, ``3b`` -1.5) in that step's own octave, so
        a quarter tone has one or two: ``C1x4`` (60.5) has ``D3b4`` and ``B3x3``, ``C3x4``
        (61.5) only ``D1b4``. The default is the partner with the smaller alter; on a tie, the
        one on the other side of the note's own accidental, as ``#`` and ``b`` swap
        (``C1x4`` -> ``D3b4``, ``E1b4`` -> ``D3x4``). The alternative is the other partner.

        Spellings outside octaves -1 to 11 do not exist: a missing alternative returns the
        default and a missing default returns the note's own pitch (``Bx11`` -> ``Bx11``,
        ``B1x11`` -> ``B1x11``).

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
        >>> ml.Note("C#4", transposeDiatonic=-1, transposeChromatic=-2).getEnharmonicPitch()
        'Db4'
        >>> ml.Note("C1x4").getEnharmonicPitch(), ml.Note("C1x4").getEnharmonicPitch(True)
        ('D3b4', 'B3x3')
        >>> ml.Note("C3x4").getEnharmonicPitch(), ml.Note("C3x4").getEnharmonicPitch(True)
        ('D1b4', 'D1b4')
    )pbdoc");
    cls.def("getEnharmonicPitches", &Note::getEnharmonicPitches,
            py::arg("includeCurrentPitch") = false,
            R"pbdoc(
        Return the default and alternative enharmonic spellings of the written pitch.

        Parameters
        ----------
        includeCurrentPitch : bool, default False
            Prepend the note's own pitch.

        Returns
        -------
        list of str
            ``[getEnharmonicPitch(False), getEnharmonicPitch(True)]``, preceded by
            ``getPitch()`` when ``includeCurrentPitch`` is True. Entries may repeat, e.g.
            ``["G#4", "Ab4", "Ab4"]``.

        Examples
        --------
        >>> ml.Note("C1x4").getEnharmonicPitches(True)
        ['C1x4', 'D3b4', 'B3x3']
        >>> ml.Note("C3x4").getEnharmonicPitches(True)
        ['C3x4', 'D1b4', 'D1b4']
    )pbdoc");

    cls.def("getEnharmonicNote", &Note::getEnharmonicNote,
            py::arg("alternativeEnharmonicPitch") = false,
            R"pbdoc(
        Return a new Note spelled with ``getEnharmonicPitch(alternativeEnharmonicPitch)``.

        The new note holds that respelling of the written pitch as its written pitch, with this
        note's transposing interval and the default rhythm figure, so it sounds exactly what this
        note sounds -- the same ``getMidiNumber()`` and ``getQuarterToneSteps()`` -- only written
        differently: a B-flat clarinet's written ``D4`` (sounding ``C4``) gives a written
        ``Ebb4``, also sounding ``C4``. A rest gives a rest.

        Parameters
        ----------
        alternativeEnharmonicPitch : bool, default False
            Use the alternative spelling instead of the default one.

        Returns
        -------
        Note
            Enharmonic note.

        Raises
        ------
        RuntimeError
            If the respelling cannot be spelled once moved by the transposing interval, the case
            ``toEnharmonicPitch`` raises for: a written ``A#11`` moved up a major second sounds
            ``B#11``, but its respelling ``Bb11`` would need the letter C of octave 12.

        Examples
        --------
        >>> ml.Note("E1b4").getEnharmonicNote().getPitch()
        'D3x4'
        >>> clarinet = ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> respelled = clarinet.getEnharmonicNote()
        >>> respelled.getPitch(), respelled.getSoundingPitch(), respelled.getMidiNumber()
        ('Ebb4', 'C4', 60)
    )pbdoc");
    cls.def("getEnharmonicNotes", &Note::getEnharmonicNotes, py::arg("includeCurrentPitch") = false,
            R"pbdoc(
        Return new Notes spelled with the strings ``getEnharmonicPitches`` returns.

        Each new note holds its respelling of the written pitch as its written pitch, with this
        note's transposing interval and the default rhythm figure, so every one sounds exactly
        what this note sounds (see ``getEnharmonicNote``).

        Parameters
        ----------
        includeCurrentPitch : bool, default False
            Prepend a note spelled with the current pitch.

        Returns
        -------
        list of Note
            Enharmonic notes (entries may repeat, see ``getEnharmonicPitches``).

        Raises
        ------
        RuntimeError
            If a respelling cannot be spelled once moved by the transposing interval (see
            ``getEnharmonicNote``).

        Examples
        --------
        >>> [note.getPitch() for note in ml.Note("E1b4").getEnharmonicNotes()]
        ['D3x4', 'F3b4']
        >>> clarinet = ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> [(n.getPitch(), n.getSoundingPitch()) for n in clarinet.getEnharmonicNotes()]
        [('Ebb4', 'C4'), ('Cx4', 'C4')]
    )pbdoc");

    cls.def("toEnharmonicPitch", &Note::toEnharmonicPitch,
            py::arg("alternativeEnharmonicPitch") = false,
            R"pbdoc(
        Respell the written pitch in place with
        ``getEnharmonicPitch(alternativeEnharmonicPitch)``.

        The respelling is set as the written pitch, through ``setPitch``, and the transposing
        interval is kept, so the note sounds exactly what it sounded before, only written
        differently: a B-flat clarinet's written ``C#4`` (sounding ``B3``) becomes a written
        ``Db4``, still sounding ``B3``. When this raises, the note is left unchanged.

        Parameters
        ----------
        alternativeEnharmonicPitch : bool, default False
            Use the alternative spelling instead of the default one.

        Raises
        ------
        RuntimeError
            If ``setPitch`` raises for the respelling: on a transposing instrument, when the
            respelling cannot be spelled once moved by the transposing interval (a written
            ``A#11`` moved up a major second sounds ``B#11``, but its respelling ``Bb11`` would
            need the letter C of octave 12).

        Examples
        --------
        >>> note = ml.Note("C1x4")
        >>> note.toEnharmonicPitch()
        >>> note.getPitch()
        'D3b4'
        >>> clarinet = ml.Note("C#4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> clarinet.toEnharmonicPitch()
        >>> clarinet.getPitch(), clarinet.getSoundingPitch(), clarinet.getMidiNumber()
        ('Db4', 'B3', 59)
    )pbdoc");
    cls.def("getScaleDegree", &Note::getScaleDegree, py::arg("key"),
            R"pbdoc(
        Return the scale degree, 1 to 7, of the written pitch's diatonic step in a key.

        Only the step counts, not the accidental: in C major ``C#4`` and ``C1x4`` are degree 1,
        like ``C4``. The step is the written one, ``getPitchStep()``: the keys read from a part's
        measures are that part's written key signatures, so a note of a transposing instrument is
        measured in the key its part is written in -- a B-flat clarinet's written ``D4`` is
        degree 2 in C major, although it sounds ``C4``.

        Parameters
        ----------
        key : Key
            The key; a minor key counts from its own tonic.

        Returns
        -------
        int
            The scale degree, or 0 for a rest.

        Examples
        --------
        >>> ml.Note("E4").getScaleDegree(ml.Key("C"))
        3
        >>> ml.Note("C1x4").getScaleDegree(ml.Key("C"))
        1
        >>> ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2).getScaleDegree(ml.Key("C"))
        2
    )pbdoc");
    cls.def("getFrequency", &Note::getFrequency, py::arg("freqA4") = 440.0f,
            R"pbdoc(
        Return the frequency of the sounding pitch in Hz, in twelve-tone equal temperament.

        Computed from the rounded MIDI number, ``getMidiNumber()``, so a quarter tone is rounded,
        ties upward, to the semitone above it: ``Note("A1x4")`` gets the frequency of ``A#4``,
        about 466.16 Hz. ``Pitch.getFrequency`` gives the exact frequency, about 452.89 Hz.

        Parameters
        ----------
        freqA4 : float, default 440.0
            Reference frequency of A4, in Hz.

        Returns
        -------
        float
            Frequency in Hz, or 0.0 for a rest.

        Raises
        ------
        RuntimeError
            If the note's transposing interval carries its sounding pitch below the lowest
            representable pitch, ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> round(ml.Note("A4").getFrequency(), 2)
        440.0
        >>> round(ml.Note("A1x4").getFrequency(), 2), round(ml.Pitch("A1x4").getFrequency(), 2)
        (466.16, 452.89)
    )pbdoc");
    cls.def("getHarmonicSpectrum", &Note::getHarmonicSpectrum, py::arg("numPartials") = 6,
            py::arg("amplCallback") = nullptr, py::arg("partialsDecayExpRate") = 0.88f,
            py::arg("freqA4") = 440.0f,
            R"pbdoc(
        Return the harmonic spectrum of the sounding pitch: its partials' frequencies and
        amplitudes.

        The fundamental is ``getFrequency(freqA4)``, so a quarter tone's spectrum is built on
        the semitone above it (see ``getFrequency``). Partial ``k``, from 1, has frequency
        ``k * fundamental`` and, without ``amplCallback``, amplitude
        ``partialsDecayExpRate ** (k - 1)``.

        Parameters
        ----------
        numPartials : int, default 6
            Number of partials, the fundamental included.
        amplCallback : callable, optional
            Takes the list of partial frequencies and returns one amplitude per partial.
        partialsDecayExpRate : float, default 0.88
            Ratio between the amplitudes of consecutive partials, without ``amplCallback``.
        freqA4 : float, default 440.0
            Reference frequency of A4, in Hz.

        Returns
        -------
        tuple of (list of float, list of float)
            The partials' frequencies and their amplitudes.

        Raises
        ------
        RuntimeError
            If ``numPartials`` is not positive, if ``amplCallback`` does not return one amplitude
            per partial, or if the note's transposing interval carries its sounding pitch below
            the lowest representable pitch, ``C1b-1`` (see ``getSoundingPitch``).

        Examples
        --------
        >>> frequencies, amplitudes = ml.Note("A4").getHarmonicSpectrum(3)
        >>> [round(f, 1) for f in frequencies], [round(a, 4) for a in amplitudes]
        ([440.0, 880.0, 1320.0], [1.0, 0.88, 0.7744])
    )pbdoc");

    cls.def("transpose", &Note::transpose, py::arg("semitones"),
            py::arg("accType") = MUSIC_XML::ACCIDENT::NONE,
            R"pbdoc(
        Transpose the written pitch by a number of semitones.

        Computed on exact pitch positions, so a quarter tone survives: ``C4`` up 0.5 is
        ``C1x4``, and ``C1x4`` up 2 is ``D1x4``. The written pitch moves once and the
        transposing interval is kept, so what the note sounds moves by the same number of
        semitones: a B-flat clarinet's written ``C4`` (sounding ``Bb3``) transposed by 2 is a
        written ``D4``, sounding ``C4``, and transposing by 0 changes nothing. The note is left
        unchanged when this raises.

        Parameters
        ----------
        semitones : float
            Number of semitones (negative values transpose down); must be finite and a multiple
            of 0.5, where 0.5 is one quarter tone.
        accType : str, default ""
            Preferred accidental type of the result: ``""``, ``"#"``, ``"b"``, ``"x"`` or
            ``"bb"``.

        Raises
        ------
        RuntimeError
            If ``semitones`` is not finite or not a multiple of 0.5; if the transposed written
            pitch would lie outside the representable range, ``"C1b-1"`` to ``"Bx11"`` -- the note
            never silently becomes a rest; if it cannot be spelled with ``accType`` within
            octaves -1 to 11; or, on a transposing instrument, if it would sound where it cannot
            be spelled (see ``setPitch``). The note is left unchanged when this raises.

        Examples
        --------
        >>> note = ml.Note("C4")
        >>> note.transpose(0.5)
        >>> note.getPitch()
        'C1x4'
        >>> clarinet = ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> clarinet.transpose(2)
        >>> clarinet.getPitch(), clarinet.getSoundingPitch()
        ('D4', 'C4')
    )pbdoc");

    cls.def("toXML", &Note::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Serialize the note as a MusicXML ``<note>`` element.

        The written pitch's accidental is written as ``<alter>`` with its real value: a whole
        number for a whole-tone accidental (``1``, ``-2``) and one decimal place for a quarter
        tone (``0.5``, ``-1.5``). A quarter tone also gets an ``<accidental>`` element, right
        after ``<type>``, carrying its Tartini name (``quarter-sharp``, ``three-quarters-flat``,
        ...), because no key signature can imply one; a whole-tone accidental gets none.

        Parameters
        ----------
        instrumentId : int, default 1
            Zero-based index of the part: a sounding note references the score instrument
            ``P<instrumentId + 1>-I<n>``.
        identSize : int, default 2
            Number of spaces per indentation level.

        Returns
        -------
        str
            The ``<note>`` element as XML text.

        Examples
        --------
        >>> xml = ml.Note("C1x4").toXML()
        >>> "<alter>0.5</alter>" in xml, "<accidental>quarter-sharp</accidental>" in xml
        (True, True)
    )pbdoc");

    // Default Python 'print' function:
    cls.def(
        "__repr__", [](const Note& note) { return "<Note " + note.getPitch() + ">"; },
        R"pbdoc(
        ``<Note P>``, where P is ``getPitch()``, the written pitch -- the pitch the note was
        constructed or set with, whatever the instrument sounds. Never raises.
    )pbdoc");

    // The hash of exactly what == compares, the concert spelling, so that equal notes hash equally
    // as Python requires.
    cls.def(
        "__hash__",
        [](const Note& note) { return std::hash<std::string>{}(concertPitch(note).getPitch()); },
        R"pbdoc(
        Hash of what ``==`` compares: the pitch, spelled as ``==`` spells it, so equal notes hash
        equally whatever their duration or other attributes -- a B-flat clarinet's written
        ``D4`` and ``Note("C4")`` land in the same set entry. Raises ``RuntimeError`` for a note
        whose sounding pitch lies below ``C1b-1``; see ``getSoundingPitch``.
    )pbdoc");

    cls.def("__sizeof__", [](const Note& note) { return sizeof(note); });

    // The ordering operators compare exact sounding positions, getQuarterToneSteps(); == and !=
    // compare spellings, a transposed note's at concert pitch. Every one of them raises for a note
    // whose sounding pitch lies below the lowest representable pitch.
    cls.def(py::self < py::self,
            R"pbdoc(
        Compare the exact sounding positions, ``getQuarterToneSteps()``: ``Note("E1b4") <
        Note("E4")`` is True, although both have the MIDI number 64. Raises ``RuntimeError`` for
        a note whose sounding pitch lies below ``C1b-1``; see ``getSoundingPitch``.
    )pbdoc");
    cls.def(py::self > py::self,
            R"pbdoc(
        Compare the exact sounding positions, ``getQuarterToneSteps()``; see ``__lt__``.
    )pbdoc");
    cls.def(py::self <= py::self,
            R"pbdoc(
        Compare the exact sounding positions, ``getQuarterToneSteps()``; see ``__lt__``.
    )pbdoc");
    cls.def(py::self >= py::self,
            R"pbdoc(
        Compare the exact sounding positions, ``getQuarterToneSteps()``; see ``__lt__``.
    )pbdoc");
    cls.def(py::self == py::self,
            R"pbdoc(
        Compare the pitches, spelled alike; duration and every other attribute are ignored.

        A note of a transposing instrument is compared at the pitch it sounds, spelled with its
        written letter moved by the diatonic transposing interval, as ``Chord`` and ``Interval``
        relate it: a B-flat clarinet's written ``D4`` equals ``Note("C4")``, and its written
        ``Db4`` equals ``Note("Cb4")``, not ``Note("B3")``. Where the diatonic interval gives no
        such spelling, the position is spelled by the fallback described on
        ``getSoundingPitch``, without the simplification: with ``transposeDiatonic=0`` and
        ``transposeChromatic=-2``, a written ``C4`` equals ``Note("Bb3")``, not
        ``Note("Cbb4")``, and a written ``F#4`` equals ``Note("Fb4")``, not ``Note("E4")``. An
        untransposed note is compared as written, so ``Note("C#4") == Note("Db4")`` and
        ``Note("E1b4") == Note("E4")`` are both False. Raises ``RuntimeError`` for a note whose
        sounding pitch lies below ``C1b-1``; see ``getSoundingPitch``.

        Examples
        --------
        >>> ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2) == ml.Note("C4")
        True
        >>> ml.Note("C#4") == ml.Note("Db4")
        False
    )pbdoc");
    cls.def(py::self != py::self,
            R"pbdoc(
        Negation of ``==``: True if the pitches differ, compared as ``==`` compares them.
    )pbdoc");
}
