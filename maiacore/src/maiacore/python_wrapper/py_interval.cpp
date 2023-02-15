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
            py::arg("pitch_B") = "C4");

    // Overloaded constructor for rests
    cls.def(py::init<const Note&, const Note&>(), py::arg("note_A"), py::arg("note_B"));

    cls.def("setNotes",
            py::overload_cast<const std::string&, const std::string&>(&Interval::setNotes),
            py::arg("pitch_A"), py::arg("pitch_B"));
    cls.def("setNotes", py::overload_cast<const Note&, const Note&>(&Interval::setNotes),
            py::arg("note_A"), py::arg("note_B"));

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

    cls.def("toCents", &Interval::toCents);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Interval& interval) {
        const std::string direction = (interval.isAscendant()) ? "asc" : "desc";
        return "<Interval " + interval.getName() + " " + direction + ">";
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
