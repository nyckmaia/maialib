#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <optional>

#include "maiacore/helper.h"
#include "maiacore/interval.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;

void HelperClass(const py::module& m) {
    m.doc() = "Helper class binding";

    // bindings to Interval class
    py::class_<Helper> cls(m, "Helper");

    //--------------------- //
    cls.def_static("getLibraryVersion", &Helper::getLibraryVersion,
                   R"pbdoc(
        Return the maiacore library version.

        Reads the version baked into the compiled library from the ``MAIALIB_VERSION_INFO``
        macro (set by CMake from the repo-root ``VERSION`` file). Returns ``"dev"`` when the
        macro is undefined.

        Returns
        -------
        str
            Library version string (e.g. ``"1.10.3"``), or ``"dev"`` if unavailable.

        Examples
        --------
        >>> ml.Helper.getLibraryVersion()
        '1.10.3'
    )pbdoc");
    //--------------------- //
    cls.def_static("freq2midiNote", &Helper::freq2midiNote, py::arg("freq"),
                   py::arg("modelo") = nullptr);
    //--------------------- //
    cls.def_static("midiNote2freq", &Helper::midiNote2freq, py::arg("midiNoteValue"),
                   py::arg("freqA4") = 440.0f);
    //--------------------- //
    cls.def_static("pitch2midiNote", &Helper::pitch2midiNote, py::arg("pitch"),
                   R"pbdoc(
        Convert a pitch string to a MIDI note number.

        A quarter tone lies halfway between two MIDI numbers and rounds to the upper one:
        ``"C1x4"`` and ``"D3b4"`` (both 60.5) give 61. Use ``Pitch.getQuarterToneSteps`` for
        the exact position.

        Parameters
        ----------
        pitch : str
            Pitch string such as ``"C4"``, ``"F#11"``, ``"Dbb-1"`` or ``"C1x4"``. Accidentals:
            ``bb``, ``b``, ``#``, ``x`` and the quarter tones ``3b``, ``1b``, ``1x``, ``3x``.
            Octaves: -1 to 11 (default 4). An empty string or any string containing ``"rest"``
            is a rest.

        Returns
        -------
        int
            MIDI note number (``"C-1"`` is 0, ``"C4"`` is 60), or -1 for a rest.

        Raises
        ------
        RuntimeError
            If the pitch string is invalid or below MIDI note 0 (e.g. ``"Cb-1"``).

        Examples
        --------
        >>> ml.Helper.pitch2midiNote("Bx11")
        157
        >>> ml.Helper.pitch2midiNote("D1b4")
        62
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("spelling2midiNote", &Helper::spelling2midiNote, py::arg("pitchStep"),
                   py::arg("alterValue"), py::arg("octave"),
                   R"pbdoc(
        Compute the MIDI note number from already-parsed pitch spelling components.

        Single implementation of ``12 * (octave + 1) + stepSemitone + alterValue``, also used
        internally by ``pitch2midiNote`` and ``splitPitch``. Intended for callers that already
        parsed a pitch string with ``splitPitch`` and want to avoid rebuilding and re-parsing a
        pitch string just to get its MIDI number. A quarter-tone ``alterValue`` leaves the sum
        halfway between two MIDI numbers, and it rounds to the upper one.

        Parameters
        ----------
        pitchStep : str
            Diatonic step, one of ``"A"`` to ``"G"`` (see ``splitPitch``).
        alterValue : float
            Accidental value in semitones, from -2.0 to 2.0 (e.g. -2.0 for ``"bb"``, 0.5 for
            ``"1x"``).
        octave : int
            Octave number, from -1 to 11.

        Returns
        -------
        int
            MIDI note number, rounded ties upward for a quarter tone.

        Raises
        ------
        RuntimeError
            If pitchStep is not a valid diatonic step, if alterValue is NaN, infinite or outside
            [-2, 2], or if octave is outside [-1, 11].

        Examples
        --------
        >>> ml.Helper.spelling2midiNote("B", 2.0, 11)
        157
        >>> ml.Helper.spelling2midiNote("D", -0.5, 4)
        62
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static(
        "splitPitch",
        [](const std::string& pitch) {
            std::string pitchClass;
            std::string pitchStep;
            std::string alterSymbol;
            std::optional<int> octave;
            float alterValue = 0.0f;
            Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);
            return std::make_tuple(pitchClass, pitchStep, octave, alterValue, alterSymbol);
        },
        py::arg("pitch"),
        R"pbdoc(
        Parse a pitch string into its components.

        Parameters
        ----------
        pitch : str
            Pitch string such as ``"C4"``, ``"F#11"``, ``"Dbb-1"``, ``"C1x4"`` or ``"Eb"``
            (default octave 4). Accidentals: ``bb``, ``b``, ``#``, ``x`` and the quarter tones
            ``3b``, ``1b``, ``1x``, ``3x``.

        Returns
        -------
        tuple of (str, str, int or None, float, str)
            ``(pitchClass, pitchStep, octave, alterValue, alterSymbol)``; ``alterValue`` is a
            multiple of 0.5 (0.5 for ``1x``). A rest has no octave and returns
            ``("rest", "rest", None, 0.0, "")``.

        Raises
        ------
        RuntimeError
            If the step, accidental or octave is invalid, or the pitch is below MIDI note 0.

        Examples
        --------
        >>> ml.Helper.splitPitch("Dbb-1")
        ('Dbb', 'D', -1, -2.0, 'bb')
        >>> ml.Helper.splitPitch("E1b4")
        ('E1b', 'E', 4, -0.5, '1b')
    )pbdoc",
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("midiNote2pitches", &Helper::midiNote2pitches, py::arg("midiNote"),
                   R"pbdoc(
        Return every spelling of a MIDI note number within octaves -1 to 11.

        Parameters
        ----------
        midiNote : int
            MIDI note number.

        Returns
        -------
        list of str
            Sorted, unique pitch strings using ``bb``, ``b``, natural, ``#`` or ``x``.

        Examples
        --------
        >>> ml.Helper.midiNote2pitches(0)
        ['C-1', 'Dbb-1']
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("midiNote2pitch", &Helper::midiNote2pitch, py::arg("midiNote"),
                   py::arg("accType") = std::string(),
                   R"pbdoc(
        Convert a MIDI note number to a pitch string.

        Parameters
        ----------
        midiNote : int
            MIDI note number. Negative values return ``"rest"``.
        accType : str, default ""
            ``""`` (natural for white keys, ``#`` for black keys), ``"#"``, ``"b"``, ``"x"`` or
            ``"bb"``.

        Returns
        -------
        str
            Pitch string within octaves -1 to 11.

        Raises
        ------
        RuntimeError
            If ``accType`` is unknown, the note cannot be written with it, or the octave falls
            outside -1 to 11.

        Examples
        --------
        >>> ml.Helper.midiNote2pitch(157, "x")
        'Bx11'
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("steps2pitch", &Helper::steps2pitch, py::arg("exactSteps"),
                   py::arg("accType") = std::string(),
                   R"pbdoc(
        Spell an exact pitch position, which may be a quarter tone, as a pitch string.

        The fractional counterpart of ``midiNote2pitch``. The position is split into the
        semitone it rounds to, ties upward, and the quarter tone left over; the semitone is
        spelled with ``accType`` and the quarter tone is folded into its accidental, so 60.5
        with ``accType=""`` gives ``"C1x4"``.

        The representable range runs from -0.5 (``"C1b-1"``, the lowest position that still
        rounds to MIDI note 0) to 157 (``"Bx11"``). A position below it -- one that rounds to a
        negative MIDI number, i.e. anything below -0.5 -- returns ``"rest"``, as
        ``midiNote2pitch`` does for a negative MIDI number.

        Parameters
        ----------
        exactSteps : float
            Exact pitch position in semitones (60.0 is ``"C4"``, 60.5 is ``"C1x4"``). Must be
            finite and a multiple of 0.5.
        accType : str, default ""
            Preferred accidental of the semitone the position rounds to: ``""`` (natural for a
            white key, ``#`` for a black key), ``"#"``, ``"b"``, ``"x"`` or ``"bb"``. A
            preference for the quarter tone: when it cannot absorb it (a ``"bb"`` spelling is
            already at the -2 limit), the default spelling is used and a warning is printed.

        Returns
        -------
        str
            Pitch string within octaves -1 to 11, or ``"rest"`` below MIDI note 0.

        Raises
        ------
        RuntimeError
            If ``exactSteps`` is not finite (the message names the value and the representable
            range), is not a multiple of 0.5 (names the value), or lies above the representable
            range (names the value and the range). Also if the semitone cannot be spelled with
            ``accType`` within octaves -1 to 11: 157 exists only as ``"Bx11"``, so it raises for
            any other ``accType``.

        Examples
        --------
        >>> ml.Helper.steps2pitch(60.5)
        'C1x4'
        >>> ml.Helper.steps2pitch(63.5)
        'E1b4'
        >>> ml.Helper.steps2pitch(63.5, "b")
        'F3b4'
        >>> ml.Helper.steps2pitch(-0.5)
        'C1b-1'
        >>> ml.Helper.steps2pitch(-1.0)
        'rest'
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("validateTransposeSemitones", &Helper::validateTransposeSemitones,
                   py::arg("semitones"),
                   R"pbdoc(
        Check that a transposition interval lies on the quarter-tone grid.

        The rule every transposition applies before it moves anything (``transposePitch``,
        ``Note.transpose``, ``Chord.transpose`` and ``Chord.transposeStackOnly``): the finest
        interval this library can spell is the quarter tone, so an interval must be a finite
        multiple of 0.5 semitones.

        Parameters
        ----------
        semitones : float
            Transposition interval in semitones.

        Raises
        ------
        RuntimeError
            If ``semitones`` is not finite (``inf``, ``-inf`` or ``nan``), or is finite but not a
            multiple of 0.5. Both messages name the value.

        Examples
        --------
        >>> ml.Helper.validateTransposeSemitones(-1.5)
        >>> ml.Helper.validateTransposeSemitones(0.3)
        Traceback (most recent call last):
            ...
        RuntimeError: [maiacore] A transposition must be a multiple of 0.5 ... '0.300000'...
    )pbdoc");
    //--------------------- //
    cls.def_static("alterName2symbol", &Helper::alterName2symbol, py::arg("alterName"),
                   R"pbdoc(
        Convert a MusicXML accidental name to this library's accidental symbol.

        Accepts the 14 names that denote an accidental this library can spell: the whole-tone
        ``"flat-flat"``, ``"flat"``, ``"natural"``, ``"sharp"``, ``"double-sharp"`` and
        ``"sharp-sharp"`` (a double sharp drawn as two sharps), and, for the quarter tones, both
        the Tartini names ``"quarter-flat"``, ``"three-quarters-flat"``, ``"quarter-sharp"``,
        ``"three-quarters-sharp"`` and the arrow names ``"flat-up"`` (``"1b"``), ``"flat-down"``
        (``"3b"``), ``"sharp-down"`` (``"1x"``) and ``"sharp-up"`` (``"3x"``).

        Parameters
        ----------
        alterName : str
            Accidental name, as in a MusicXML ``<accidental>`` element.

        Returns
        -------
        str
            One of ``"bb"``, ``"3b"``, ``"b"``, ``"1b"``, ``""``, ``"1x"``, ``"#"``, ``"3x"``,
            ``"x"``.

        Raises
        ------
        RuntimeError
            If the name is not one of the 14 accepted names (MusicXML defines others, e.g.
            ``"slash-flat"``, which this library cannot spell).

        Examples
        --------
        >>> ml.Helper.alterName2symbol("quarter-sharp")
        '1x'
        >>> ml.Helper.alterName2symbol("flat-down")
        '3b'
    )pbdoc");
    //--------------------- //
    cls.def_static("alterSymbol2Value", &Helper::alterSymbol2Value, py::arg("alterSymbol"),
                   R"pbdoc(
        Convert an accidental symbol to its value in semitones.

        Parameters
        ----------
        alterSymbol : str
            One of ``"bb"``, ``"3b"``, ``"b"``, ``"1b"``, ``""`` (natural), ``"1x"``, ``"#"``,
            ``"3x"``, ``"x"``.

        Returns
        -------
        float
            The value, a multiple of 0.5 from -2.0 to 2.0 (``"1x"`` is 0.5).

        Raises
        ------
        RuntimeError
            If the symbol is not one of the nine accepted symbols.

        Examples
        --------
        >>> ml.Helper.alterSymbol2Value("3b")
        -1.5
        >>> ml.Helper.alterSymbol2Value("#")
        1.0
    )pbdoc");
    //--------------------- //
    cls.def_static("alterValue2symbol", &Helper::alterValue2symbol, py::arg("alterValue"),
                   R"pbdoc(
        Convert an accidental value in semitones to its symbol.

        The value reaches the library as a 32-bit float, which must be exactly one of the nine
        accepted values: one merely close to them, such as 0.46, raises rather than being
        rounded, but a Python float within float32 precision of one becomes that value on the
        way in and is accepted. ``-0.0`` is the natural. The result does not depend on the
        process locale.

        Parameters
        ----------
        alterValue : float
            A multiple of 0.5 from -2.0 to 2.0.

        Returns
        -------
        str
            One of ``"bb"``, ``"3b"``, ``"b"``, ``"1b"``, ``""`` (natural), ``"1x"``, ``"#"``,
            ``"3x"``, ``"x"``.

        Raises
        ------
        RuntimeError
            If the value is not exactly one of the nine accepted values; the message names it.

        Examples
        --------
        >>> ml.Helper.alterValue2symbol(-1.5)
        '3b'
        >>> ml.Helper.alterValue2symbol(0.5)
        '1x'
    )pbdoc");
    //--------------------- //
    cls.def_static("alterValue2Name", &Helper::alterValue2Name, py::arg("alterValue"),
                   R"pbdoc(
        Convert an accidental value in semitones to its MusicXML accidental name.

        The quarter tones get their Tartini names (``"quarter-sharp"``, ``"three-quarters-flat"``,
        ...), the names ``Note.toXML`` writes. As in ``alterValue2symbol``, the value, a 32-bit
        float once it reaches the library, must be exactly one of the nine accepted values
        (``-0.0`` is the natural), and the result does not depend on the process locale.

        Parameters
        ----------
        alterValue : float
            A multiple of 0.5 from -2.0 to 2.0.

        Returns
        -------
        str
            One of ``"flat-flat"``, ``"three-quarters-flat"``, ``"flat"``, ``"quarter-flat"``,
            ``"natural"``, ``"quarter-sharp"``, ``"sharp"``, ``"three-quarters-sharp"``,
            ``"double-sharp"``.

        Raises
        ------
        RuntimeError
            If the value is not exactly one of the nine accepted values; the message names it.

        Examples
        --------
        >>> ml.Helper.alterValue2Name(0.5)
        'quarter-sharp'
        >>> ml.Helper.alterValue2Name(-1.5)
        'three-quarters-flat'
    )pbdoc");
    //--------------------- //
    cls.def_static(
        "notes2Intervals",
        py::overload_cast<const std::vector<std::string>&, const bool>(&Helper::notes2Intervals),
        py::arg("pitches"), py::arg("firstNoteAsReference") = false);
    cls.def_static(
        "notes2Intervals",
        py::overload_cast<const std::vector<Note>&, const bool>(&Helper::notes2Intervals),
        py::arg("notes"), py::arg("firstNoteAsReference") = false);
    //--------------------- //
    cls.def_static("midiNote2octave", &Helper::midiNote2octave, py::arg("midiNote"),
                   R"pbdoc(
        Convert a MIDI note number to its octave number.

        Parameters
        ----------
        midiNote : int
            MIDI note number.

        Returns
        -------
        int or None
            Octave number, or ``None`` if ``midiNote < 0`` (a rest has no octave).

        Examples
        --------
        >>> ml.Helper.midiNote2octave(60)
        4
        >>> ml.Helper.midiNote2octave(-1) is None
        True
    )pbdoc");
    //--------------------- //
    cls.def_static("noteType2ticks", &Helper::noteType2ticks, py::arg("noteType"),
                   py::arg("divisionsPerQuarterNote") = 256,
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def_static("ticks2noteType", &Helper::ticks2noteType, py::arg("durationTicks"),
                   py::arg("divisionsPerQuarterNote") = 256, py::arg("actualNotes") = 1,
                   py::arg("normalNotes") = 1,
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    //--------------------- //
    cls.def_static("isEnharmonic", &Helper::isEnharmonic, py::arg("pitch_A"), py::arg("pitch_B"),
                   R"pbdoc(
        Check whether two pitch strings are enharmonically equivalent.

        Parameters
        ----------
        pitch_A : str
            First pitch string.
        pitch_B : str
            Second pitch string.

        Returns
        -------
        bool
            True if both pitches denote the same exact pitch position (two rests are enharmonic).
            Compared exactly, so the two spellings of a quarter tone are enharmonic ("C1x4" and
            "D3b4"), while a quarter tone and the semitone it rounds to are not ("C1x4" and
            "C#4").

        Raises
        ------
        RuntimeError
            If a pitch string is invalid.

        Examples
        --------
        >>> ml.Helper.isEnharmonic("E#4", "F4")
        True
        >>> ml.Helper.isEnharmonic("C1x4", "C#4")
        False
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("transposePitch", &Helper::transposePitch, py::arg("pitch"),
                   py::arg("semitones"), py::arg("accType") = "#",
                   R"pbdoc(
        Transpose a pitch string by a number of semitones.

        Computed on exact pitch positions, so a quarter tone survives the transposition instead
        of being rounded away first. A real pitch never silently becomes a rest: a result outside
        the representable range, from -0.5 (``"C1b-1"``) to 157 (``"Bx11"``), raises.

        Parameters
        ----------
        pitch : str
            Input pitch string. A rest (an empty string or any string containing ``"rest"``)
            transposes to ``"rest"``.
        semitones : float
            Number of semitones (negative values transpose down). Must be finite and a multiple
            of 0.5: 0.5 is one quarter tone up. 0 returns ``pitch`` unchanged.
        accType : str, default "#"
            Preferred accidental type of the result's base semitone: ``""``, ``"#"``, ``"b"``,
            ``"x"`` or ``"bb"`` (see ``Helper.steps2pitch``, which spells the result).

        Returns
        -------
        str
            Transposed pitch string within octaves -1 to 11, or ``"rest"`` for a rest.

        Raises
        ------
        RuntimeError
            If ``semitones`` is not finite (``inf``, ``-inf`` or ``nan``) or is not a multiple of
            0.5; if ``pitch`` is invalid, whatever the interval; if a non-rest pitch transposes
            outside the representable range (the message names the pitch, the interval and the
            range); or if the result cannot be spelled with ``accType`` within octaves -1 to 11
            (e.g. MIDI 0 with ``"#"``, which would be ``"B#-2"``).

        Examples
        --------
        >>> ml.Helper.transposePitch("B11", 1)
        'B#11'
        >>> ml.Helper.transposePitch("C1x4", 2, "")
        'D1x4'
        >>> ml.Helper.transposePitch("C-1", -0.5, "")
        'C1b-1'
    )pbdoc",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("durationRatio", &Helper::durationRatio, py::arg("duration_A"),
                   py::arg("duration_B"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("rhythmFigure2noteType", &Helper::rhythmFigure2noteType, py::arg("rhythmFigure"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def_static("rhythmFigure2Ticks", &Helper::rhythmFigure2Ticks, py::arg("rhythmFigure"),
                   py::arg("divisionsPerQuarterNote") = 265);
    //--------------------- //
    cls.def_static("noteType2RhythmFigure", &Helper::noteType2RhythmFigure, py::arg("noteType"));
    //--------------------- //
    cls.def_static("pitch2freq", &Helper::pitch2freq, py::arg("pitch"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
                   R"pbdoc(
        Return the approximate equal-tempered frequency of a pitch, in Hz.

        Looked up in a table of octave-0 frequencies rounded to 0.01 Hz and scaled by octave, so
        ``"C4"`` gives 261.6 where the exact value is about 261.63. Only pitches with a whole-tone
        accidental, or none, have an entry: a quarter tone is refused rather than approximated.
        ``Pitch(pitch).getFrequency()`` computes the exact frequency of any pitch, a quarter tone
        included.

        Parameters
        ----------
        pitch : str
            Pitch string, e.g. ``"A4"``.

        Returns
        -------
        float
            Frequency in Hz.

        Raises
        ------
        RuntimeError
            If the pitch string is invalid, or is a rest or a quarter tone ("Pitch not found!").

        Examples
        --------
        >>> ml.Helper.pitch2freq("A4")
        440.0
        >>> round(ml.Pitch("C1x4").getFrequency(), 2)
        269.29
    )pbdoc");
    //--------------------- //
    cls.def_static("freq2pitch", &Helper::freq2pitch, py::arg("freq"), py::arg("accType") = "");
    //--------------------- //
    cls.def_static("pitchRatio", &Helper::pitchRatio, py::arg("pitch_A"), py::arg("pitch_B"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static(
        "noteSimilarity",
        [](std::string& pitchClass_A, int octave_A, const float duration_A,
           std::string& pitchClass_B, int octave_B, const float duration_B,
           const bool enableEnharmonic) {
            float durRatio = 0.0f;
            float pitchRatio = 0.0f;

            float averageRatio =
                Helper::noteSimilarity(pitchClass_A, octave_A, duration_A, pitchClass_B, octave_B,
                                       duration_B, durRatio, pitchRatio, enableEnharmonic);
            return std::make_tuple(pitchRatio, durRatio, averageRatio);
        },
        py::arg("pitchClass_A"), py::arg("octave_A"), py::arg("duration_A"),
        py::arg("pitchClass_B"), py::arg("octave_B"), py::arg("duration_B"),
        py::arg("enableEnharmonic") = false,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static(
        "getPercentiles",
        [](const py::object& pyTable, const std::vector<float>& desiredPercentiles) {
            nlohmann::json table = py::object(pyTable);
            return py::object(Helper::getPercentiles(table, desiredPercentiles));
        },
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("frequencies2cents", &Helper::frequencies2cents, py::arg("freq_A"),
                   py::arg("freq_B"));
    cls.def_static("freq2equalTemperament", &Helper::freq2equalTemperament, py::arg("freq"),
                   py::arg("referenceFreq") = 440.0f);

    cls.def_static("getSemitonesDifferenceBetweenMelodies",
                   &Helper::getSemitonesDifferenceBetweenMelodies, py::arg("referenceMelody"),
                   py::arg("otherMelody"),
                   R"pbdoc(
        Compare two melodies' contours, interval by interval.

        At each position the interval from one note to the next is taken in each melody, as the
        difference of the two notes' exact sounding positions (``Note.getQuarterToneSteps``), so
        a quarter tone counts as half a semitone; a pair that includes a rest counts as an
        interval of 0. Only intervals are compared, so a transposed copy of a melody gives all
        zeros. If the melodies differ in length, only the first
        ``min(len(referenceMelody), len(otherMelody))`` notes of each are compared.

        Parameters
        ----------
        referenceMelody : list of Note
            The reference melody.
        otherMelody : list of Note
            The melody compared with it.

        Returns
        -------
        list of float
            One value per pair of consecutive notes: the reference melody's interval minus the
            other melody's, in semitones.

        Raises
        ------
        RuntimeError
            If either melody has fewer than 2 notes, or holds a note whose transposing interval
            carries its sounding pitch below the lowest representable pitch, ``C1b-1`` (see
            ``Note.getSoundingPitch``).

        Examples
        --------
        >>> reference = [ml.Note(p) for p in ("C4", "E4", "G4")]
        >>> transposed = [ml.Note(p) for p in ("D4", "F#4", "A4")]
        >>> ml.Helper.getSemitonesDifferenceBetweenMelodies(reference, transposed)
        [0.0, 0.0]
        >>> neutralThird = [ml.Note(p) for p in ("C4", "E1b4", "G4")]
        >>> ml.Helper.getSemitonesDifferenceBetweenMelodies(reference, neutralThird)
        [0.5, -0.5]
    )pbdoc");

    cls.def_static("calculateMelodyEuclideanSimilarity",
                   py::overload_cast<const std::vector<Note>&, const std::vector<Note>&>(
                       &Helper::calculateMelodyEuclideanSimilarity),
                   py::arg("melodyPattern"), py::arg("otherMelody"),
                   R"pbdoc(
        Score how alike two melodies' contours are: ``1 / (1 + d)``, where ``d`` is the Euclidean
        norm of ``getSemitonesDifferenceBetweenMelodies(melodyPattern, otherMelody)``.

        1.0 means the same contour -- a transposed copy included -- and the score falls toward 0
        as the intervals diverge; a quarter tone counts as half a semitone.

        Parameters
        ----------
        melodyPattern : list of Note
            The reference melody.
        otherMelody : list of Note
            The melody compared with it.

        Returns
        -------
        float
            The similarity, in (0, 1].

        Raises
        ------
        RuntimeError
            As ``getSemitonesDifferenceBetweenMelodies`` raises.

        Examples
        --------
        >>> reference = [ml.Note(p) for p in ("C4", "E4", "G4")]
        >>> neutralThird = [ml.Note(p) for p in ("C4", "E1b4", "G4")]
        >>> round(ml.Helper.calculateMelodyEuclideanSimilarity(reference, neutralThird), 4)
        0.5858
    )pbdoc");

    cls.def_static(
        "calculateMelodyEuclideanSimilarity",
        py::overload_cast<const std::vector<float>&>(&Helper::calculateMelodyEuclideanSimilarity),
        py::arg("semitonesDifference"),
        R"pbdoc(
        Score precomputed interval differences: ``1 / (1 + d)``, where ``d`` is their Euclidean
        norm.

        Parameters
        ----------
        semitonesDifference : list of float
            Interval differences, as ``getSemitonesDifferenceBetweenMelodies`` returns them.

        Returns
        -------
        float
            The similarity, in (0, 1].

        Examples
        --------
        >>> round(ml.Helper.calculateMelodyEuclideanSimilarity([0.5, -0.5]), 4)
        0.5858
    )pbdoc");

    cls.def_static("getDurationDifferenceBetweenRhythms",
                   &Helper::getDurationDifferenceBetweenRhythms, py::arg("referenceRhythm"),
                   py::arg("otherRhythm"));

    cls.def_static("calculateRhythmicEuclideanSimilarity",
                   py::overload_cast<const std::vector<Note>&, const std::vector<Note>&>(
                       &Helper::calculateRhythmicEuclideanSimilarity),
                   py::arg("rhythmPattern"), py::arg("otherRhythm"));

    cls.def_static(
        "calculateRhythmicEuclideanSimilarity",
        py::overload_cast<const std::vector<float>&>(&Helper::calculateRhythmicEuclideanSimilarity),
        py::arg("durationDifferences"));
}
