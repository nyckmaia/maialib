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

        Parameters
        ----------
        pitch : str
            Pitch string such as ``"C4"``, ``"F#11"`` or ``"Dbb-1"``. Accidentals: ``bb``, ``b``,
            ``#``, ``x``. Octaves: -1 to 11 (default 4). An empty string or any string containing
            ``"rest"`` is a rest.

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
        pitch string just to get its MIDI number.

        Parameters
        ----------
        pitchStep : str
            Diatonic step, one of ``"A"`` to ``"G"`` (see ``splitPitch``).
        alterValue : float
            Accidental value in semitones (e.g. -2.0 for ``"bb"``).
        octave : int
            Octave number.

        Returns
        -------
        int
            MIDI note number.

        Raises
        ------
        RuntimeError
            If pitchStep is not a valid diatonic step.

        Examples
        --------
        >>> ml.Helper.spelling2midiNote("B", 2.0, 11)
        157
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
            Pitch string such as ``"C4"``, ``"F#11"``, ``"Dbb-1"`` or ``"Eb"`` (default octave 4).

        Returns
        -------
        tuple of (str, str, int or None, float, str)
            ``(pitchClass, pitchStep, octave, alterValue, alterSymbol)``. A rest has no octave
            and returns ``("rest", "rest", None, 0.0, "")``.

        Raises
        ------
        RuntimeError
            If the step, accidental or octave is invalid, or the pitch is below MIDI note 0.

        Examples
        --------
        >>> ml.Helper.splitPitch("Dbb-1")
        ('Dbb', 'D', -1, -2.0, 'bb')
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
            ``"x"`` or ``"bb"``.

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
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
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
                   py::arg("otherMelody"));

    cls.def_static("calculateMelodyEuclideanSimilarity",
                   py::overload_cast<const std::vector<Note>&, const std::vector<Note>&>(
                       &Helper::calculateMelodyEuclideanSimilarity),
                   py::arg("melodyPattern"), py::arg("otherMelody"));

    cls.def_static(
        "calculateMelodyEuclideanSimilarity",
        py::overload_cast<const std::vector<float>&>(&Helper::calculateMelodyEuclideanSimilarity),
        py::arg("semitonesDifference"));

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
