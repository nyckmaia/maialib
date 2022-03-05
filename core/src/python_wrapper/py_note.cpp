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

    cls.def(py::init<const std::string&, const Duration, bool, bool, const int, const int, const int>(),
            py::arg("pitch"),
            py::arg("duration") = Duration::QUARTER,
            py::arg("isNoteOn") = true,
            py::arg("inChord") = false,
            py::arg("transposeDiatonic") = 0,
            py::arg("transposeChromatic") = 0,
            py::arg("divisionsPerQuarterNote") = 256);
    
    cls.def(py::init<const int, const std::string&, const Duration, bool, bool, const int, const int, const int>(),
            py::arg("midiNumber"),
            py::arg("accType") = "",
            py::arg("duration") = Duration::QUARTER,
            py::arg("isNoteOn") = true,
            py::arg("inChord") = false,
            py::arg("transposeDiatonic") = 0,
            py::arg("transposeChromatic") = 0,
            py::arg("divisionsPerQuarterNote") = 256);

    // ====== Methods SETTERS for class Note ===== //
    cls.def("setPitchClass", &Note::setPitchClass);
    cls.def("setOctave", &Note::setOctave);
    cls.def("setDurationTicks", &Note::setDurationTicks);
    cls.def("setIsNoteOn", &Note::setIsNoteOn);
    cls.def("setPitch", &Note::setPitch);
    cls.def("setIsInChord", &Note::setIsInChord);
    cls.def("setTransposingInterval", &Note::setTransposingInterval);
    cls.def("transpose", &Note::transpose,
        py::arg("semitones"),
        py::arg("accType") = MUSIC_XML::ACCIDENT::NONE);
    
    cls.def("setVoice", &Note::setVoice);
    cls.def("setStaff", &Note::setStaff);
    cls.def("setIsGraceNote", &Note::setIsGraceNote,
        py::arg("isGraceNote") = false);
    cls.def("setStem", &Note::setStem);
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

    cls.def("getPitchClass", &Note::getPitchClass);
    cls.def("getOctave", &Note::getOctave);
    
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
    cls.def("getMIDINumber", &Note::getMIDINumber);

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

    cls.def("inChord", &Note::inChord);
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
