#include "note.h"

#ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void NoteClass(py::module &m) {

    m.doc() = "Note class binding";

    py::class_<Note> cls(m, "Note");

    cls.def(py::init<std::string, size_t, bool, bool, int, int, int>(),
            py::arg("pitch"),
            py::arg("durationTicks") = 256,
            py::arg("isNoteOn") = true,
            py::arg("inChord") = false,
            py::arg("transposeDiatonic") = 0,
            py::arg("transposeChromatic") = 0,
            py::arg("divisionsPerQuarterNote") = 256);

    // Overloaded constructor for rests
    cls.def(py::init<size_t, int>(), 
        py::arg("durationTicks") = 256,
        py::arg("divisionsPerQuarterNote") = 256);

    // ====== Methods SETTERS for class Note ===== //
    cls.def("setPitchClass", &Note::setPitchClass);
    cls.def("setOctave", &Note::setOctave);
    cls.def("setDurationTicks", &Note::setDurationTicks);
    cls.def("setIsNoteOn", &Note::setIsNoteOn);
    cls.def("setPitch", &Note::setPitch);
    cls.def("setIsInChord", &Note::setIsInChord);
    cls.def("setTransposingInterval", &Note::setTransposingInterval);
    

    cls.def("setVoice", &Note::setVoice);
    cls.def("setStaff", &Note::setStaff);
    cls.def("setIsGraceNote", &Note::setIsGraceNote,
        py::arg("isGraceNote") = false);
    cls.def("setType", &Note::setType);
    cls.def("setStem", &Note::setStem);
    cls.def("setDivisionsPerQuarterNote", &Note::setDivisionsPerQuarterNote,
        py::arg("divisionsPerQuarterNote") = 256);
    cls.def("removeDots", &Note::removeDots);
    cls.def("setSingleDot", &Note::setSingleDot);
    cls.def("setDoubleDot", &Note::setDoubleDot);
    cls.def("setTieStart", &Note::setTieStart);
    cls.def("setTieStop", &Note::setTieStop);
    cls.def("setTieStopStart", &Note::setTieStopStart);
    cls.def("addTie", &Note::addTie);
    cls.def("addSlur", &Note::addSlur);
    cls.def("addArticulation", &Note::addArticulation);
    cls.def("addBeam", &Note::addBeam);
    cls.def("info", &Note::info, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());


    cls.def("setIsPitched", &Note::setIsPitched);
    cls.def("isPitched", &Note::isPitched);

    cls.def("setUnpitchedIndex", &Note::setUnpitchedIndex);
    cls.def("getUnpitchedIndex", &Note::getUnpitchedIndex);

    cls.def("transposition", &Note::transposition);

    // ===================== Method GETTERS for class Note ===== //
    cls.def("getPitchStep", &Note::getPitchStep);
    cls.def("getSoundingPitchClass", &Note::getSoundingPitchClass);
    cls.def("getSoundingPitch", &Note::getSoundingPitch);

    cls.def("getWrittenPitchClass", &Note::getWrittenPitchClass);
    cls.def("getWrittenPitch", &Note::getWrittenPitch);

    cls.def("getDiatonicWrittenPitchClass", &Note::getDiatonicWrittenPitchClass);
    cls.def("getDiatonicSoundingPitchClass", &Note::getDiatonicSoundingPitchClass);

    cls.def("getSoundingOctave", &Note::getSoundingOctave);
    cls.def("getWrittenOctave", &Note::getWrittenOctave);

    cls.def("getPitchClass", &Note::getPitchClass, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getOctave", &Note::getOctave, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getDurationTicks", &Note::getDurationTicks, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("isNoteOn", &Note::isNoteOn, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("isNoteOff", &Note::isNoteOff, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getPitch", &Note::getPitch, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getMIDINumber", &Note::getMIDINumber);

    cls.def("getVoice", &Note::getVoice);
    cls.def("getType", &Note::getType);
    cls.def("getStem", &Note::getStem);
    cls.def("getDivisionsPerQuarterNote", &Note::getDivisionsPerQuarterNote);
    cls.def("getNumDots", &Note::getNumDots);
    cls.def("getTie", &Note::getTie);
    cls.def("removeTies", &Note::removeTies);
    cls.def("getSlur", &Note::getSlur);
    cls.def("getArticulation", &Note::getArticulation);
    cls.def("getBeam", &Note::getBeam);

    cls.def("getAlterSymbol", &Note::getAlterSymbol);

    cls.def("inChord", &Note::inChord, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("getTransposeDiatonic", &Note::getTransposeDiatonic);
    cls.def("getTransposeChromatic", &Note::getTransposeChromatic);
    cls.def("isTransposed", &Note::isTransposed);
    cls.def("isGraceNote", &Note::isGraceNote);

    cls.def("toXML", &Note::toXML, 
        py::arg("instrumentId") = 1,
        py::arg("identSize") = 2,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());    

    // Default Python 'print' function:
    cls.def("__repr__", [](const Note& note) { return note.getPitch(); });

    cls.def(py::self < py::self);
    cls.def(py::self > py::self);
    cls.def(py::self <= py::self);
    cls.def(py::self >= py::self);
    cls.def(py::self == py::self);
    cls.def(py::self != py::self);
}
#endif
