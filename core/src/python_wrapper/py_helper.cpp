#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "helper.h"
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;

void HelperClass(const py::module& m) {
    m.doc() = "Helper class binding";

    // bindings to Interval class
    py::class_<Helper> cls(m, "Helper");

    //--------------------- //
    cls.def_static("freq2midiNote", &Helper::freq2midiNote, py::arg("freq"),
                   py::arg("modelo") = nullptr);
    //--------------------- //
    cls.def_static("midiNote2freq", &Helper::midiNote2freq, py::arg("midiNoteValue"));
    //--------------------- //
    cls.def_static("pitch2midiNote", &Helper::pitch2midiNote, py::arg("pitch"));
    //--------------------- //
    cls.def_static("midiNote2pitches", &Helper::midiNote2pitches, py::arg("midiNote"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("midiNote2pitch", &Helper::midiNote2pitch, py::arg("midiNote"),
                   py::arg("accType") = std::string(),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("notes2Intervals", &Helper::notes2Intervals, py::arg("notes"),
                   py::arg("firstNoteAsReference") = false);
    //--------------------- //
    cls.def_static("midiNote2octave", &Helper::midiNote2octave, py::arg("midiNote"));
    //--------------------- //
    cls.def_static("noteType2ticks", &Helper::noteType2ticks, py::arg("noteType"),
                   py::arg("divisionsPerQuarterNote") = 256,
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def_static("ticks2noteType", &Helper::ticks2noteType, py::arg("ticks"),
                   py::arg("divisionsPerQuarterNote") = 256,
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def_static("pitch2number", &Helper::pitch2number, py::arg("pitch"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("number2pitch", &Helper::number2pitch, py::arg("number"),
                   py::arg("accType") = "#",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("isEnharmonic", &Helper::isEnharmonic,
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("transposePitch", &Helper::transposePitch, py::arg("pitch"),
                   py::arg("semitones"), py::arg("accType") = "#",
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("durationRatio", &Helper::durationRatio, py::arg("duration_A"),
                   py::arg("duration_B"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("duration2noteType", &Helper::duration2noteType, py::arg("duration"));
    //--------------------- //
    cls.def_static("noteType2duration", &Helper::noteType2duration, py::arg("noteType"));
    //--------------------- //
    cls.def_static("pitch2freq", &Helper::pitch2freq, py::arg("pitch"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //--------------------- //
    cls.def_static("freq2pitch", &Helper::freq2pitch, py::arg("freq"), py::arg("accType") = "#");
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
    cls.def_static("pitch2number", &Helper::pitch2number, py::arg("pitch"),
                   py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
}
