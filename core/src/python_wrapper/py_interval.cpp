#include "interval.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void IntervalClass(py::module &m) {

    m.doc() = "Interval class binding";

    // bindings to Interval class
    py::class_<Interval> cls(m, "Interval");
    cls.def(py::init<const std::string&, const std::string&>(),
            py::arg("pitch_A") = "C4",
            py::arg("pitch_B") = "C4");

    // Overloaded constructor for rests
    cls.def(py::init<const Note&, const Note&>(),
            py::arg("note_A"),
            py::arg("note_B"));

    cls.def("setNotes", py::overload_cast<const std::string&, const std::string&>(&Interval::setNotes),
        py::arg("pitch_A"),
        py::arg("pitch_B"));
    cls.def("setNotes", py::overload_cast<const Note&, const Note&>(&Interval::setNotes),
        py::arg("note_A"),
        py::arg("note_B"));

    cls.def("getName", &Interval::getName);

    cls.def("getNumSemitones", &Interval::getNumSemitones,
        py::arg("absoluteValue") = false);

    cls.def("getNumOctaves", &Interval::getNumOctaves,
        py::arg("absoluteValue") = false);

    cls.def("getDiatonicInterval", &Interval::getDiatonicInterval,
        py::arg("useSingleOctave") = true,
        py::arg("absoluteValue") = false);

    cls.def("getDiatonicSteps", &Interval::getDiatonicSteps,
        py::arg("useSingleOctave") = true,
        py::arg("absoluteValue") = false);
    cls.def("getPitchStepInterval", &Interval::getPitchStepInterval);
    cls.def("getNotes", &Interval::getNotes);
    cls.def("isAscendant", &Interval::isAscendant);
    cls.def("isDescendant", &Interval::isDescendant);
    cls.def("isTonal", &Interval::isTonal);

    cls.def("isMinorThird", &Interval::isMinorThird);
    cls.def("isMajorThird", &Interval::isMajorThird);
    cls.def("isDiminishedFifth", &Interval::isDiminishedFifth);
    cls.def("isPerfectFifth", &Interval::isPerfectFifth);
    cls.def("isAugmentedFifth", &Interval::isAugmentedFifth);
    cls.def("isDiminishedSeventh", &Interval::isDiminishedSeventh);
    cls.def("isMinorSeventh", &Interval::isMinorSeventh);
    cls.def("isMajorSeventh", &Interval::isMajorSeventh);
    cls.def("isMinorNinth", &Interval::isMinorNinth);
    cls.def("isMajorNinth", &Interval::isMajorNinth);
    cls.def("isPerfectEleventh", &Interval::isPerfectEleventh);
    cls.def("isSharpEleventh", &Interval::isSharpEleventh);
    cls.def("isMinorThirdteenth", &Interval::isMinorThirdteenth);
    cls.def("isMajorThirdteenth", &Interval::isMajorThirdteenth);

    cls.def("isSimple", &Interval::isSimple);
    cls.def("isCompound", &Interval::isCompound);

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

    cls.def("__sizeof__", [](const Interval& interval) {
        return sizeof(interval);
    });

    cls.def(py::self < py::self);
}
#endif
