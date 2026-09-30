#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/interval.h"
#include "maiacore/note.h"
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void IntervalClass(const py::module& m) {
    m.doc() = "Interval class binding";

    // bindings to Interval class
    py::class_<Interval> cls(m, "Interval");
    cls.def(py::init<const std::string&, const std::string&>(), py::arg("pitch_A") = "C4",
            py::arg("pitch_B") = "C4",
            R"pbdoc(
        Create the interval between two pitch strings.

        Interval analysis is defined only over twelve-tone equal temperament, so an interval
        with a quarter-tone note cannot be built: its name, quality and semitone count would
        all be wrong.

        Parameters
        ----------
        pitch_A : str, default "C4"
            The first pitch.
        pitch_B : str, default "C4"
            The second pitch.

        Raises
        ------
        RuntimeError
            If either pitch is a rest; if either pitch is a quarter tone -- the message names it
            and the remedy, ``Note.roundToSemitone``; or if a pitch string is invalid.

        Examples
        --------
        >>> ml.Interval("C4", "E4").getName()
        'M3'
    )pbdoc");

    // Overloaded constructor for rests
    cls.def(py::init<const Note&, const Note&>(), py::arg("note_A"), py::arg("note_B"),
            R"pbdoc(
        Create the interval between two notes.

        A note of a transposing instrument is related at concert pitch: by the pitch it sounds,
        spelled with its written letter moved by the diatonic transposing interval. A B-flat
        clarinet's written ``D4`` against a violin's ``C4`` is a perfect unison. ``getNotes``
        returns the notes as they were given.

        Parameters
        ----------
        note_A : Note
            The first note.
        note_B : Note
            The second note.

        Raises
        ------
        RuntimeError
            If either note is a rest; if either note is a quarter tone -- the message names it and
            the remedy, ``Note.roundToSemitone``; or if a note's transposing interval carries its
            sounding pitch below the lowest representable pitch, ``C1b-1`` (see
            ``Note.getSoundingPitch``).

        Examples
        --------
        >>> ml.Interval(ml.Note("C4"), ml.Note("G4")).getName()
        'P5'
        >>> clarinet = ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> ml.Interval(clarinet, ml.Note("C4")).getName()
        'P1'
    )pbdoc");

    cls.def("setNotes",
            py::overload_cast<const std::string&, const std::string&>(&Interval::setNotes),
            py::arg("pitch_A"), py::arg("pitch_B"),
            R"pbdoc(
        Replace both pitches of the interval.

        Parameters
        ----------
        pitch_A : str
            The first pitch.
        pitch_B : str
            The second pitch.

        Raises
        ------
        RuntimeError
            If either pitch is a rest; if either pitch is a quarter tone -- the message names it
            and the remedy, ``Note.roundToSemitone``; or if a pitch string is invalid.

        Examples
        --------
        >>> interval = ml.Interval()
        >>> interval.setNotes("C4", "Eb4")
        >>> interval.getName()
        'm3'
    )pbdoc");
    cls.def("setNotes", py::overload_cast<const Note&, const Note&>(&Interval::setNotes),
            py::arg("note_A"), py::arg("note_B"),
            R"pbdoc(
        Replace both notes of the interval.

        A note of a transposing instrument is related at concert pitch, as in the constructor.

        Parameters
        ----------
        note_A : Note
            The first note.
        note_B : Note
            The second note.

        Raises
        ------
        RuntimeError
            If either note is a rest; if either note is a quarter tone -- the message names it and
            the remedy, ``Note.roundToSemitone``; or if a note's transposing interval carries its
            sounding pitch below the lowest representable pitch, ``C1b-1`` (see
            ``Note.getSoundingPitch``).

        Examples
        --------
        >>> interval = ml.Interval()
        >>> interval.setNotes(ml.Note("C4"), ml.Note("A4"))
        >>> interval.getName()
        'M6'
    )pbdoc");

    cls.def("getName", &Interval::getName);

    cls.def("getNumSemitones", &Interval::getNumSemitones, py::arg("absoluteValue") = false);

    cls.def("getNumOctaves", &Interval::getNumOctaves, py::arg("absoluteValue") = false);

    cls.def("getDiatonicInterval", &Interval::getDiatonicInterval,
            py::arg("useSingleOctave") = true, py::arg("absoluteValue") = false);

    cls.def("getDiatonicSteps", &Interval::getDiatonicSteps, py::arg("useSingleOctave") = true,
            py::arg("absoluteValue") = false);
    cls.def("getPitchStepInterval", &Interval::getPitchStepInterval);
    cls.def("getNotes", &Interval::getNotes);
    cls.def("isAscendant", &Interval::isAscendant);
    cls.def("isDescendant", &Interval::isDescendant);
    cls.def("getDirection", &Interval::getDirection);

    cls.def("isSimple", &Interval::isSimple);
    cls.def("isCompound", &Interval::isCompound);

    cls.def("isTonal", &Interval::isTonal);

    cls.def("isMajor", &Interval::isMajor, py::arg("useEnharmony") = false);
    cls.def("isMinor", &Interval::isMinor, py::arg("useEnharmony") = false);
    cls.def("isPerfect", &Interval::isPerfect, py::arg("useEnharmony") = false);
    cls.def("isDiminished", &Interval::isDiminished, py::arg("useEnharmony") = false);
    cls.def("isAugmented", &Interval::isAugmented, py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 1 ===== //
    cls.def("isDiminishedUnisson", &Interval::isDiminishedUnisson, py::arg("useEnharmony") = false);
    cls.def("isPerfectUnisson", &Interval::isPerfectUnisson, py::arg("useEnharmony") = false);
    cls.def("isAugmentedUnisson", &Interval::isAugmentedUnisson, py::arg("useEnharmony") = false);
    cls.def("isMinorSecond", &Interval::isMinorSecond, py::arg("useEnharmony") = false);
    cls.def("isMajorSecond", &Interval::isMajorSecond, py::arg("useEnharmony") = false);
    cls.def("isMinorThird", &Interval::isMinorThird, py::arg("useEnharmony") = false);
    cls.def("isMajorThird", &Interval::isMajorThird, py::arg("useEnharmony") = false);
    cls.def("isPerfectFourth", &Interval::isPerfectFourth, py::arg("useEnharmony") = false);
    cls.def("isAugmentedFourth", &Interval::isAugmentedFourth, py::arg("useEnharmony") = false);
    cls.def("isDiminishedFifth", &Interval::isDiminishedFifth, py::arg("useEnharmony") = false);
    cls.def("isPerfectFifth", &Interval::isPerfectFifth, py::arg("useEnharmony") = false);
    cls.def("isAugmentedFifth", &Interval::isAugmentedFifth, py::arg("useEnharmony") = false);
    cls.def("isMinorSixth", &Interval::isMinorSixth, py::arg("useEnharmony") = false);
    cls.def("isMajorSixth", &Interval::isMajorSixth, py::arg("useEnharmony") = false);
    cls.def("isDiminishedSeventh", &Interval::isDiminishedSeventh, py::arg("useEnharmony") = false);
    cls.def("isMinorSeventh", &Interval::isMinorSeventh, py::arg("useEnharmony") = false);
    cls.def("isMajorSeventh", &Interval::isMajorSeventh, py::arg("useEnharmony") = false);
    cls.def("isDiminishedOctave", &Interval::isDiminishedOctave, py::arg("useEnharmony") = false);
    cls.def("isPerfectOctave", &Interval::isPerfectOctave, py::arg("useEnharmony") = false);
    cls.def("isAugmentedOctave", &Interval::isAugmentedOctave, py::arg("useEnharmony") = false);
    cls.def("isMinorNinth", &Interval::isMinorNinth, py::arg("useEnharmony") = false);
    cls.def("isMajorNinth", &Interval::isMajorNinth, py::arg("useEnharmony") = false);
    cls.def("isPerfectEleventh", &Interval::isPerfectEleventh, py::arg("useEnharmony") = false);
    cls.def("isSharpEleventh", &Interval::isSharpEleventh, py::arg("useEnharmony") = false);
    cls.def("isMinorThirdteenth", &Interval::isMinorThirdteenth, py::arg("useEnharmony") = false);
    cls.def("isMajorThirdteenth", &Interval::isMajorThirdteenth, py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 2 ===== //
    cls.def("isSecond", &Interval::isSecond, py::arg("useEnharmony") = false);
    cls.def("isThird", &Interval::isThird, py::arg("useEnharmony") = false);
    cls.def("isFourth", &Interval::isFourth, py::arg("useEnharmony") = false);
    cls.def("isFifth", &Interval::isFifth, py::arg("useEnharmony") = false);
    cls.def("isSixth", &Interval::isSixth, py::arg("useEnharmony") = false);
    cls.def("isSeventh", &Interval::isSeventh, py::arg("useEnharmony") = false);
    cls.def("isOctave", &Interval::isOctave, py::arg("useEnharmony") = false);
    cls.def("isNinth", &Interval::isNinth, py::arg("useEnharmony") = false);
    cls.def("isEleventh", &Interval::isEleventh, py::arg("useEnharmony") = false);
    cls.def("isThirdteenth", &Interval::isThirdteenth, py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 3 ===== //
    cls.def("isAnyOctaveMinorSecond", &Interval::isAnyOctaveMinorSecond,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMajorSecond", &Interval::isAnyOctaveMajorSecond,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMinorThird", &Interval::isAnyOctaveMinorThird,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMajorThird", &Interval::isAnyOctaveMajorThird,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctavePerfectFourth", &Interval::isAnyOctavePerfectFourth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveAugmentedFourth", &Interval::isAnyOctaveAugmentedFourth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveDiminishedFifth", &Interval::isAnyOctaveDiminishedFifth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctavePerfectFifth", &Interval::isAnyOctavePerfectFifth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveAugmentedFifth", &Interval::isAnyOctaveAugmentedFifth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMinorSixth", &Interval::isAnyOctaveMinorSixth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMajorSixth", &Interval::isAnyOctaveMajorSixth,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveDiminishedSeventh", &Interval::isAnyOctaveDiminishedSeventh,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMinorSeventh", &Interval::isAnyOctaveMinorSeventh,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveMajorSeventh", &Interval::isAnyOctaveMajorSeventh,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveDiminishedOctave", &Interval::isAnyOctaveDiminishedOctave,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctavePerfectOctave", &Interval::isAnyOctavePerfectOctave,
            py::arg("useEnharmony") = false);
    cls.def("isAnyOctaveAugmentedOctave", &Interval::isAnyOctaveAugmentedOctave,
            py::arg("useEnharmony") = false);

    // ===== ABSTRACTION 4 ===== //
    cls.def("isAnyOctaveSecond", &Interval::isAnyOctaveSecond);
    cls.def("isAnyOctaveThird", &Interval::isAnyOctaveThird);
    cls.def("isAnyOctaveFourth", &Interval::isAnyOctaveFourth);
    cls.def("isAnyOctaveFifth", &Interval::isAnyOctaveFifth);
    cls.def("isAnyOctaveSixth", &Interval::isAnyOctaveSixth);
    cls.def("isAnyOctaveSeventh", &Interval::isAnyOctaveSeventh);
    cls.def("isAnyOctaveOctave", &Interval::isAnyOctaveOctave);

    cls.def("toCents", &Interval::toCents, py::arg("freqA4") = 440.0f);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Interval& interval) {
        return "<Interval " + interval.getName() + " " + interval.getDirection() + ">";
    });

    cls.def("__hash__", [](const Interval& interval) {
        std::string temp;
        for (const auto& n : interval.getNotes()) {
            temp += n.getPitch();
        }

        return std::hash<std::string>{}(temp);
    });

    cls.def("__sizeof__", [](const Interval& interval) { return sizeof(interval); });

    cls.def(py::self < py::self);
}
