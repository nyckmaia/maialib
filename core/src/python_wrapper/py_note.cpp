#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <bitset>
#include <unordered_map>

#include "note.h"
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void NoteClass(const py::module& m) {
    m.doc() = "Note class binding";

    py::class_<Note> cls(m, "Note");

    cls.def(
        py::init<const std::string&, const Duration, bool, bool, const int, const int, const int>(),
        py::arg("pitch"), py::arg("duration") = Duration::QUARTER, py::arg("isNoteOn") = true,
        py::arg("inChord") = false, py::arg("transposeDiatonic") = 0,
        py::arg("transposeChromatic") = 0, py::arg("divisionsPerQuarterNote") = 256);

    cls.def(py::init<const int, const std::string&, const Duration, bool, bool, const int,
                     const int, const int>(),
            py::arg("midiNumber"), py::arg("accType") = "", py::arg("duration") = Duration::QUARTER,
            py::arg("isNoteOn") = true, py::arg("inChord") = false,
            py::arg("transposeDiatonic") = 0, py::arg("transposeChromatic") = 0,
            py::arg("divisionsPerQuarterNote") = 256);

    // ====== Methods SETTERS for class Note ===== //
    cls.def("setPitchClass", &Note::setPitchClass, py::arg("pitchClass"),
            "Set the note pitch class");
    cls.def("setOctave", &Note::setOctave, py::arg("octave"));

    cls.def("setDuration", py::overload_cast<const Duration, const int>(&Note::setDuration),
            py::arg("duration"), py::arg("divisionsPerQuarterNote") = 256);
    cls.def("setDuration", py::overload_cast<const float, const int, const int>(&Note::setDuration),
            py::arg("durationValue"), py::arg("lowerTimeSignatureValue") = 4,
            py::arg("divisionsPerQuarterNote") = 256);

    cls.def("setDurationTicks", &Note::setDurationTicks, py::arg("durationTicks"));
    cls.def("setIsNoteOn", &Note::setIsNoteOn, py::arg("isNoteOn"));
    cls.def("setPitch", &Note::setPitch, py::arg("pitch"));
    cls.def("setIsInChord", &Note::setIsInChord, py::arg("inChord"));
    cls.def("setTransposingInterval", &Note::setTransposingInterval, py::arg("diatonicInterval"),
            py::arg("chromaticInterval"));

    cls.def("setVoice", &Note::setVoice, py::arg("voice"));
    cls.def("setStaff", &Note::setStaff, py::arg("staff"));
    cls.def("setIsGraceNote", &Note::setIsGraceNote, py::arg("isGraceNote") = false);
    cls.def("setStem", &Note::setStem, py::arg("stem"));
    cls.def("removeDots", &Note::removeDots);
    cls.def("setSingleDot", &Note::setSingleDot);
    cls.def("setDoubleDot", &Note::setDoubleDot);
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

    cls.def("getEnharmonicPitch", &Note::getEnharmonicPitch,
            py::arg("alternativeEnhamonicPitch") = false);
    cls.def("getEnharmonicPitches", &Note::getEnharmonicPitches,
            py::arg("includeCurrentPitch") = false);

    cls.def("getEnharmonicNote", &Note::getEnharmonicNote,
            py::arg("alternativeEnhamonicPitch") = false);
    cls.def("getEnharmonicNotes", &Note::getEnharmonicNotes,
            py::arg("includeCurrentPitch") = false);

    cls.def("toEnharmonicPitch", &Note::toEnharmonicPitch,
            py::arg("alternativeEnhamonicPitch") = false);
    cls.def("getFrequency", &Note::getFrequency);

    cls.def("transpose", &Note::transpose, py::arg("semitones"),
            py::arg("accType") = MUSIC_XML::ACCIDENT::NONE);

    cls.def("toXML", &Note::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    // Default Python 'print' function:
    cls.def("__repr__", [](const Note& note) { return "<Note " + note.getPitch() + ">"; });

    cls.def("__hash__", [](const Note& note) {
        const std::string temp01 = note.getSoundingPitch() + note.getType();
        const int temp02 = note.isNoteOn() | (note.inChord() << 1) | (note.isGraceNote() << 2) |
                           (note.isTuplet() << 3) | (note.isPitched() << 4);

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
