#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/config.h"
#include "maiacore/constants.h"

namespace py = pybind11;

void Constants(const py::module &m) {
    py::enum_<RhythmFigure>(m, "RhythmFigure")
        .value("MAXIMA", RhythmFigure::MAXIMA)
        .value("LONG", RhythmFigure::LONG)
        .value("BREVE", RhythmFigure::BREVE)
        .value("WHOLE", RhythmFigure::WHOLE)
        .value("HALF", RhythmFigure::HALF)
        .value("QUARTER", RhythmFigure::QUARTER)
        .value("EIGHTH", RhythmFigure::EIGHTH)
        .value("N16TH", RhythmFigure::N16TH)
        .value("N32ND", RhythmFigure::N32ND)
        .value("N64TH", RhythmFigure::N64TH)
        .value("N128TH", RhythmFigure::N128TH)
        .value("N256TH", RhythmFigure::N256TH)
        .value("N512TH", RhythmFigure::N512TH)
        .value("N1024TH", RhythmFigure::N1024TH);

    py::enum_<OctaveDoubling>(m, "OctaveDoubling", R"pbdoc(
        Whether a note's part is doubled one octave from what the note sounds, as a MusicXML
        ``<double>`` inside ``<transpose>`` states it: mixed cello and bass parts are doubled
        below, mixed flute and piccolo parts above. See ``Note.getOctaveDoubling``.
    )pbdoc")
        .value("NONE", OctaveDoubling::NONE, "Not doubled.")
        .value("BELOW", OctaveDoubling::BELOW, "Doubled one octave below: ``<double/>``.")
        .value("ABOVE", OctaveDoubling::ABOVE,
               "Doubled one octave above: ``<double above=\"yes\"/>``.");

    // py::enum_<TuningSystem>(m, "TuningSystem")
    //     .value("EQUAL_TEMPERAMENT", TuningSystem::EQUAL_TEMPERAMENT)
    //     .value("JUST_INTONATION", TuningSystem::JUST_INTONATION)
    //     .value("PYTHAGOREAN_TUNING", TuningSystem::PYTHAGOREAN_TUNING)
    //     .value("MEANTONE_TEMPERAMENT", TuningSystem::MEANTONE_TEMPERAMENT)
    //     .value("WELL_TEMPERAMENT", TuningSystem::WELL_TEMPERAMENT);
}
