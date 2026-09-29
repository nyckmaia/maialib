#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/pitch.h"

namespace py = pybind11;

void PitchClass(const py::module& m) {
    m.doc() = "Pitch class binding";

    py::class_<Pitch> cls(m, "Pitch", R"pbdoc(
        A single musical pitch: a diatonic step, an accidental and an octave.

        The accidental may be a quarter tone (``1b``, ``1x``, ``3b``, ``3x``), so a ``Pitch``
        holds any position on the quarter-tone grid from ``"C1b-1"`` to ``"Bx11"``. A rest is a
        ``Pitch`` too: ``isRest()`` is True and ``getOctave()`` is None.

        Create one from a pitch string with the constructor, or from a number with a named
        factory: ``Pitch.fromMidi`` for a MIDI note number and ``Pitch.fromFrequency`` for a
        frequency in Hz. There is deliberately no numeric constructor: ``Pitch(110)`` could mean
        MIDI 110 or 110 Hz, so it raises ``TypeError`` instead of guessing.

        Examples
        --------
        >>> p = ml.Pitch("C1x4")
        >>> p.getMidiNumber(), p.getQuarterToneSteps()
        (61, 60.5)
        >>> ml.Pitch.fromMidi(110).getPitch(), ml.Pitch.fromFrequency(110).getPitch()
        ('D8', 'A2')
    )pbdoc");

    // The string constructor is the ONLY __init__. The int (MIDI) and float (frequency) C++
    // constructors are exposed as the named factories below instead: bound as __init__
    // overloads, pybind11 would choose between them purely by int-versus-float, so
    // ml.Pitch(110) meant as 110 Hz would silently become MIDI 110 (a D8).
    cls.def(py::init<const std::string&>(), py::arg("pitch") = "rest",
            R"pbdoc(
        Create a pitch from a pitch string.

        Parameters
        ----------
        pitch : str, default "rest"
            A step ``A`` to ``G``, an optional accidental (``bb``, ``3b``, ``b``, ``1b``,
            ``1x``, ``#``, ``3x``, ``x``) and an optional octave from -1 to 11 (default 4), e.g.
            ``"C4"``, ``"C1x4"``, ``"Dbb-1"``. An empty string or any string containing
            ``"rest"`` creates a rest.

        Raises
        ------
        TypeError
            If ``pitch`` is not a string. A number is rejected too: use ``Pitch.fromMidi`` for
            a MIDI note number and ``Pitch.fromFrequency`` for a frequency.
        RuntimeError
            If the pitch string is invalid, or the pitch lies below MIDI note 0 (e.g.
            ``"Cb-1"``).

        Examples
        --------
        >>> ml.Pitch("D3b4").getAlter()
        -1.5
        >>> ml.Pitch().isRest()
        True
    )pbdoc");

    cls.def_static(
        "fromMidi",
        [](const int midiNumber, const std::string& accType) {
            return Pitch(midiNumber, accType);
        },
        py::arg("midiNumber"), py::arg("accType") = "",
        R"pbdoc(
        Create a pitch from a MIDI note number.

        Parameters
        ----------
        midiNumber : int
            MIDI note number, from 0 (``"C-1"``) upward. A negative number creates a rest.
        accType : str, default ""
            Accidental of the spelling: ``""`` (natural for a white key, ``#`` for a black key),
            ``"#"``, ``"b"``, ``"x"`` or ``"bb"``.

        Returns
        -------
        Pitch
            The new pitch.

        Raises
        ------
        TypeError
            If ``midiNumber`` is not an int. A float such as ``60.0`` is rejected rather than
            truncated: use ``Pitch.fromFrequency`` for a frequency.
        RuntimeError
            If ``accType`` is unknown or cannot spell this MIDI number (e.g. ``"b"`` for MIDI
            60, which has no flat spelling), or the spelling falls outside octaves -1 to 11.

        Examples
        --------
        >>> ml.Pitch.fromMidi(61, "b").getPitch()
        'Db4'
        >>> ml.Pitch.fromMidi(-1).isRest()
        True
    )pbdoc");

    cls.def_static(
        "fromFrequency",
        [](const float frequency, const std::string& accType, const float freqA4,
           const bool enableQuarterToneRound) {
            return Pitch(frequency, accType, freqA4, enableQuarterToneRound);
        },
        py::arg("frequency"), py::arg("accType") = "", py::arg("freqA4") = 440.0f,
        py::arg("enableQuarterToneRound") = false,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
        R"pbdoc(
        Create a pitch from a frequency in Hz.

        The frequency is rounded to the nearest semitone -- or to the nearest quarter tone,
        with ``enableQuarterToneRound`` -- ties upward, and spelled with ``accType``. A positive
        frequency always yields a pitch, with a warning whenever it is not the rounded one: a
        pitch below MIDI note 0 becomes MIDI note 0 (``"C-1"``, or ``"Dbb-1"`` with ``"bb"``),
        and a pitch neither ``accType`` nor the default spelling can spell within octave 11 is
        moved down, by whole semitones and without its quarter tone, until one of them can. So
        with the default ``accType`` everything from MIDI 156 up becomes ``"B11"`` (155),
        although ``"B#11"`` (156), ``"B3x11"`` (156.5) and ``"Bx11"`` (157) exist: ``"#"``
        reaches the first two and ``"x"`` the last. Zero or a negative frequency creates a rest.

        Parameters
        ----------
        frequency : float
            Frequency in Hz. A Python int is accepted: ``fromFrequency(440)``.
        accType : str, default ""
            Preferred accidental of the spelling: ``""``, ``"#"``, ``"b"``, ``"x"`` or ``"bb"``.
            A preference, not a demand: when it cannot spell the rounded pitch, the default
            spelling is used and a warning is printed.
        freqA4 : float, default 440.0
            Reference frequency of A4, in Hz: a finite number greater than 0.
        enableQuarterToneRound : bool, default False
            Round to the nearest quarter tone instead of the nearest semitone.

        Returns
        -------
        Pitch
            The new pitch.

        Raises
        ------
        RuntimeError
            If ``freqA4`` is not a finite number greater than 0 (checked first, whatever the
            frequency), if ``frequency`` is NaN, if ``accType`` is not one of the five accepted
            values, or if the active tuning system is not equal temperament (the only one
            implemented; it cannot currently be changed from Python).

        Examples
        --------
        >>> ml.Pitch.fromFrequency(440).getPitch()
        'A4'
        >>> ml.Pitch.fromFrequency(449.0, "#", 440.0, True).getPitch()  # doctest: +ELLIPSIS
        [WARN] Pitch::setFrequency: the accidental type '#' cannot spell ...; using A1x4 instead
        'A1x4'
        >>> ml.Pitch.fromFrequency(0).isRest()
        True
    )pbdoc");

    cls.def_static(
        "fromComponents",
        [](const std::string& step, const float alter, const int octave) {
            return Pitch(step, alter, octave);
        },
        py::arg("step"), py::arg("alter"), py::arg("octave"),
        R"pbdoc(
        Create a pitch from its three components: step, accidental and octave.

        The accidental is a number of semitones, so a quarter tone is given directly:
        ``Pitch.fromComponents("C", 0.5, 4)`` is ``"C1x4"``. It must be exactly a multiple of 0.5:
        one merely close to it, such as 0.99996, is rejected rather than rounded. ``-0.0`` is
        stored as ``0.0``.

        Parameters
        ----------
        step : str
            ``"A"`` to ``"G"``.
        alter : float
            A multiple of 0.5 from -2.0 to 2.0, e.g. 0.5 for a quarter-tone sharp or -1.5 for
            three quarter tones flat.
        octave : int
            Octave from -1 to 11.

        Returns
        -------
        Pitch
            The new pitch.

        Raises
        ------
        TypeError
            If ``octave`` is not an int (a float such as ``4.0`` is rejected, not truncated).
        RuntimeError
            If ``step`` is not one of ``"A"`` to ``"G"``; if ``alter`` is NaN or infinite, is not
            exactly a multiple of 0.5, or lies outside [-2, 2]; if ``octave`` lies outside -1 to
            11; or if the pitch lies below MIDI note 0 (e.g. ``("C", -1, -1)``, ``"Cb-1"``). The
            conditions are checked in that order.

        Examples
        --------
        >>> ml.Pitch.fromComponents("C", 0.5, 4).getPitch()
        'C1x4'
        >>> ml.Pitch.fromComponents("D", -1.5, 4) == ml.Pitch("D3b4")
        True
    )pbdoc");

    cls.def_static("maxRepresentableMidi", &Pitch::maxRepresentableMidi,
                   R"pbdoc(
        Return the highest MIDI note number a pitch can have.

        Returns
        -------
        int
            157, the MIDI number of ``"Bx11"`` (B double-sharp in octave 11).

        Examples
        --------
        >>> ml.Pitch.maxRepresentableMidi()
        157
    )pbdoc");

    cls.def_static("clampToRepresentableMidi", &Pitch::clampToRepresentableMidi, py::arg("midi"),
                   R"pbdoc(
        Clamp a MIDI note number to the representable range, 0 to ``maxRepresentableMidi()``.

        Parameters
        ----------
        midi : int
            MIDI note number, of any magnitude a 32-bit signed integer holds.

        Returns
        -------
        int
            0 for a negative number, ``maxRepresentableMidi()`` for a number above it, and
            ``midi`` itself otherwise.

        Raises
        ------
        TypeError
            If ``midi`` is not an int, or does not fit in a 32-bit signed integer.

        Examples
        --------
        >>> ml.Pitch.clampToRepresentableMidi(1000)
        157
        >>> ml.Pitch.clampToRepresentableMidi(-5)
        0
    )pbdoc");

    // ===== GETTERS ===== //
    cls.def("getPitch", &Pitch::getPitch,
            R"pbdoc(
        Return the pitch string: the pitch class followed by the octave.

        Returns
        -------
        str
            Pitch string (e.g. ``"C1x4"``), or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Pitch("C1x4").getPitch()
        'C1x4'
        >>> ml.Pitch("Eb").getPitch()
        'Eb4'
    )pbdoc");

    cls.def("getPitchClass", &Pitch::getPitchClass,
            R"pbdoc(
        Return the pitch class: the step followed by the accidental symbol, without the octave.

        Returns
        -------
        str
            Pitch class (e.g. ``"C1x"``, ``"Bb"``), or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Pitch("C1x4").getPitchClass()
        'C1x'
    )pbdoc");

    cls.def("getPitchStep", &Pitch::getPitchStep,
            R"pbdoc(
        Return the diatonic step.

        Returns
        -------
        str
            ``"A"`` to ``"G"``, or ``"rest"`` for a rest.

        Examples
        --------
        >>> ml.Pitch("D3b4").getPitchStep()
        'D'
    )pbdoc");

    cls.def("getAlterSymbol", &Pitch::getAlterSymbol,
            R"pbdoc(
        Return the accidental symbol.

        Returns
        -------
        str
            One of ``"bb"``, ``"3b"``, ``"b"``, ``"1b"``, ``""`` (natural), ``"1x"``, ``"#"``,
            ``"3x"`` and ``"x"``; ``""`` for a rest.

        Examples
        --------
        >>> ml.Pitch("D3b4").getAlterSymbol()
        '3b'
    )pbdoc");

    cls.def("getAlter", &Pitch::getAlter,
            R"pbdoc(
        Return the accidental as a number of semitones.

        Returns
        -------
        float
            A multiple of 0.5 from -2.0 to 2.0 (0.5 is a quarter-tone sharp); 0.0 for a rest.

        Examples
        --------
        >>> ml.Pitch("C1x4").getAlter()
        0.5
    )pbdoc");

    cls.def("getOctave", &Pitch::getOctave,
            R"pbdoc(
        Return the octave number.

        Returns
        -------
        int or None
            Octave from -1 to 11, or None for a rest: a rest has no octave. ``isRest()`` is the
            test for a rest.

        Examples
        --------
        >>> ml.Pitch("C1x4").getOctave()
        4
        >>> ml.Pitch("rest").getOctave() is None
        True
    )pbdoc");

    cls.def("getMidiNumber", &Pitch::getMidiNumber,
            R"pbdoc(
        Return the MIDI note number, rounding a quarter tone to the nearest semitone, ties upward.

        A quarter tone lies exactly halfway between two MIDI numbers, so it always rounds up:
        ``C1x4`` and ``D3b4`` (both 60.5) are 61, and ``D1b4`` (61.5) is 62. Use
        ``getQuarterToneSteps()`` for the exact, unrounded position.

        Returns
        -------
        int
            MIDI note number, or -1 for a rest.

        Examples
        --------
        >>> ml.Pitch("C1x4").getMidiNumber()
        61
        >>> ml.Pitch("D1b4").getMidiNumber()
        62
    )pbdoc");

    cls.def("getQuarterToneSteps", &Pitch::getQuarterToneSteps,
            R"pbdoc(
        Return the exact, unrounded pitch position in semitones.

        Computed as ``12 * (octave + 1) + stepSemitones + alter``, so a quarter tone keeps its
        half: ``C1x4`` is 60.5, where ``getMidiNumber()`` rounds it to 61.

        Returns
        -------
        float
            The exact position, a multiple of 0.5; -1.0 for a rest.

        Examples
        --------
        >>> ml.Pitch("C1x4").getQuarterToneSteps()
        60.5
    )pbdoc");

    cls.def("getFrequency", &Pitch::getFrequency, py::arg("freqA4") = 440.0f,
            R"pbdoc(
        Return the frequency in Hz, in twelve-tone equal temperament.

        Computed from the exact position, ``freqA4 * 2 ** ((getQuarterToneSteps() - 69) / 12)``,
        so a quarter tone is never rounded away.

        Parameters
        ----------
        freqA4 : float, default 440.0
            Reference frequency of A4, in Hz: a finite number greater than 0.

        Returns
        -------
        float
            Frequency in Hz, or 0.0 for a rest.

        Raises
        ------
        RuntimeError
            If ``freqA4`` is not a finite number greater than 0 (for a rest too), or if the
            active tuning system is not equal temperament, the only one implemented. The tuning
            system cannot currently be changed from Python.

        Examples
        --------
        >>> round(ml.Pitch("A4").getFrequency(), 2)
        440.0
        >>> round(ml.Pitch("A1x4").getFrequency(), 2)
        452.89
    )pbdoc");

    cls.def("isRest", &Pitch::isRest,
            R"pbdoc(
        Check whether this pitch is a rest.

        Returns
        -------
        bool
            True for a rest.

        Examples
        --------
        >>> ml.Pitch("rest").isRest()
        True
        >>> ml.Pitch("").isRest()
        True
    )pbdoc");

    // ===== SETTERS ===== //
    // The setters that can refuse a value print a warning through std::cout (LOG_WARN), so they
    // redirect it to Python's sys.stdout, where a notebook shows it and a test can capture it.
    cls.def("setStep", &Pitch::setStep, py::arg("step"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the diatonic step, keeping the accidental and the octave.

        On a rest this creates a note: the rest becomes that step in octave 4 (call
        ``setOctave`` afterwards for another octave). On a note, a step that would move it below
        MIDI note 0 is refused: a warning is printed and the pitch is left unchanged.

        Parameters
        ----------
        step : str
            ``"A"`` to ``"G"``.

        Raises
        ------
        RuntimeError
            If ``step`` is not one of ``"A"`` to ``"G"``.

        Examples
        --------
        >>> p = ml.Pitch("C1x4")
        >>> p.setStep("D")
        >>> p.getPitch()
        'D1x4'
    )pbdoc");

    cls.def("setAlter", &Pitch::setAlter, py::arg("alter"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the accidental, as a number of semitones.

        Refused on a rest, and when the result would lie below MIDI note 0: in both cases a
        warning is printed and the pitch is left unchanged. A malformed value is an error. The
        value must be exactly a multiple of 0.5: one merely close to it, such as 0.99996, is
        rejected rather than rounded. ``-0.0`` is stored as ``0.0``.

        Parameters
        ----------
        alter : float
            A multiple of 0.5 from -2.0 to 2.0, e.g. 0.5 for a quarter-tone sharp or -1.5 for
            three quarter tones flat.

        Raises
        ------
        RuntimeError
            If ``alter`` is NaN or infinite, is not exactly a multiple of 0.5, or lies outside
            [-2, 2].

        Examples
        --------
        >>> p = ml.Pitch("C4")
        >>> p.setAlter(0.5)
        >>> p.getPitch()
        'C1x4'
    )pbdoc");

    cls.def("setOctave", &Pitch::setOctave, py::arg("octave"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the octave, keeping the step and the accidental.

        Refused on a rest, whatever the value, and when the result would lie below MIDI note 0:
        in both cases a warning is printed and the pitch is left unchanged.

        Parameters
        ----------
        octave : int
            Octave from -1 to 11.

        Raises
        ------
        TypeError
            If ``octave`` is not an int -- e.g. None, which ``getOctave()`` returns for a rest.
        RuntimeError
            If this pitch is not a rest and ``octave`` lies outside -1 to 11.

        Examples
        --------
        >>> p = ml.Pitch("C1x4")
        >>> p.setOctave(5)
        >>> p.getPitch()
        'C1x5'
    )pbdoc");

    cls.def("setPitch", &Pitch::setPitch, py::arg("pitch"),
            R"pbdoc(
        Replace the whole pitch from a pitch string.

        Parameters
        ----------
        pitch : str
            Pitch string, with the constructor's rules. An empty string or any string
            containing ``"rest"`` makes this pitch a rest.

        Raises
        ------
        RuntimeError
            If the pitch string is invalid, or the pitch lies below MIDI note 0.

        Examples
        --------
        >>> p = ml.Pitch()
        >>> p.setPitch("E1b4")
        >>> p.getQuarterToneSteps()
        63.5
    )pbdoc");

    cls.def("setPitchClass", &Pitch::setPitchClass, py::arg("pitchClass"),
            R"pbdoc(
        Replace the step and the accidental from a pitch class, keeping the octave.

        A rest has no octave to keep, so it takes octave 4.

        Parameters
        ----------
        pitchClass : str
            Pitch class without an octave (e.g. ``"D1x"``, ``"Bb"``). Any string containing
            ``"rest"`` makes this pitch a rest.

        Raises
        ------
        RuntimeError
            If the pitch class is invalid, or the result lies below MIDI note 0.

        Examples
        --------
        >>> p = ml.Pitch("C5")
        >>> p.setPitchClass("D1x")
        >>> p.getPitch()
        'D1x5'
    )pbdoc");

    cls.def("setMidiNumber", &Pitch::setMidiNumber, py::arg("midiNumber"), py::arg("accType") = "",
            R"pbdoc(
        Replace the whole pitch from a MIDI note number, spelled with ``accType``.

        The spelling rules are ``Pitch.fromMidi``'s. When this raises, the pitch is left
        unchanged.

        Parameters
        ----------
        midiNumber : int
            MIDI note number. A negative number makes this pitch a rest.
        accType : str, default ""
            Accidental of the spelling: ``""`` (natural for a white key, ``#`` for a black key),
            ``"#"``, ``"b"``, ``"x"`` or ``"bb"``.

        Raises
        ------
        TypeError
            If ``midiNumber`` is not an int.
        RuntimeError
            If ``accType`` is unknown or cannot spell this MIDI number (e.g. ``"b"`` for MIDI
            60, which has no flat spelling), or the spelling falls outside octaves -1 to 11
            (with the default ``accType``, anything above 155, ``"B11"``; ``"x"`` reaches 157).

        Examples
        --------
        >>> p = ml.Pitch()
        >>> p.setMidiNumber(61)
        >>> p.getPitch()
        'C#4'
        >>> p.setMidiNumber(61, "b")
        >>> p.getPitch()
        'Db4'
    )pbdoc");

    cls.def("setFrequency", &Pitch::setFrequency, py::arg("frequency"), py::arg("accType") = "",
            py::arg("freqA4") = 440.0f, py::arg("enableQuarterToneRound") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Replace the whole pitch from a frequency in Hz.

        The rules are ``Pitch.fromFrequency``'s: rounded to the nearest semitone, or quarter
        tone with ``enableQuarterToneRound``, ties upward, and spelled with ``accType``; a pitch
        below MIDI note 0, or one that neither ``accType`` nor the default spelling can spell
        within octave 11, is moved to one they can, with a warning (with the default
        ``accType``, everything from MIDI 156 up becomes ``"B11"``); zero or a negative
        frequency makes this pitch a rest.

        Parameters
        ----------
        frequency : float
            Frequency in Hz.
        accType : str, default ""
            Preferred accidental of the spelling (see ``Pitch.fromFrequency``).
        freqA4 : float, default 440.0
            Reference frequency of A4, in Hz: a finite number greater than 0.
        enableQuarterToneRound : bool, default False
            Round to the nearest quarter tone instead of the nearest semitone.

        Raises
        ------
        RuntimeError
            If ``freqA4`` is not a finite number greater than 0 (checked first, whatever the
            frequency), if ``frequency`` is NaN, if ``accType`` is not one of the five accepted
            values, or if the active tuning system is not equal temperament (it cannot currently
            be changed from Python).

        Examples
        --------
        >>> p = ml.Pitch()
        >>> p.setFrequency(466.16)
        >>> p.getPitch()
        'A#4'
    )pbdoc");

    cls.def("roundToSemitone", &Pitch::roundToSemitone,
            R"pbdoc(
        Round a quarter-tone accidental to the nearest semitone, ties upward.

        ``C1x4`` becomes ``C#4``, ``D1b4`` becomes ``D4`` and ``D3b4`` becomes ``Db4``. A
        whole-tone accidental is unchanged, and so is a rest.

        Examples
        --------
        >>> p = ml.Pitch("D1b4")
        >>> p.roundToSemitone()
        >>> p.getPitch()
        'D4'
    )pbdoc");

    cls.def("__repr__", [](const Pitch& pitch) { return "<Pitch " + pitch.getPitch() + ">"; });

    cls.def(py::self == py::self,
            R"pbdoc(
        Compare two pitches by spelling: step, accidental and octave.

        Spelling equality, not enharmonic equality: ``Pitch("C#4") != Pitch("Db4")`` although
        both are MIDI 61 (``Helper.isEnharmonic`` compares sounding positions). Two rests are
        equal. Anything that is not a ``Pitch`` compares unequal.

        Examples
        --------
        >>> ml.Pitch("C1x4") == ml.Pitch.fromComponents("C", 0.5, 4)
        True
        >>> ml.Pitch("C#4") == ml.Pitch("Db4")
        False
    )pbdoc");
    cls.def(py::self != py::self,
            R"pbdoc(
        Negation of ``==``: True if the pitches differ in step, accidental or octave.
    )pbdoc");

    // Defining __eq__ makes pybind11 set __hash__ to None, which would make Pitch unhashable. The
    // hash is the pitch string's: equal pitches have equal spellings, so equal hashes.
    cls.def(
        "__hash__", [](const Pitch& pitch) { return std::hash<std::string>{}(pitch.getPitch()); },
        R"pbdoc(
        Hash consistent with ``==``: equal pitches hash equally, so a ``Pitch`` can be a set
        member or a dict key.
    )pbdoc");
}
