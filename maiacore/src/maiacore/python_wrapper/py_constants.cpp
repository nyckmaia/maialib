#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/constants.h"

namespace py = pybind11;

void Constants(const py::module &m) {
    py::enum_<Duration>(m, "Duration")
        .value("MAXIMA_DOT_DOT", Duration::MAXIMA_DOT_DOT)
        .value("MAXIMA_DOT", Duration::MAXIMA_DOT)
        .value("MAXIMA", Duration::MAXIMA)
        .value("LONG_DOT_DOT", Duration::LONG_DOT_DOT)
        .value("LONG_DOT", Duration::LONG_DOT)
        .value("LONG", Duration::LONG)
        .value("BREVE_DOT_DOT", Duration::BREVE_DOT_DOT)
        .value("BREVE_DOT", Duration::BREVE_DOT)
        .value("BREVE", Duration::BREVE)
        .value("WHOLE_DOT_DOT", Duration::WHOLE_DOT_DOT)
        .value("WHOLE_DOT", Duration::WHOLE_DOT)
        .value("WHOLE", Duration::WHOLE)
        .value("HALF_DOT_DOT", Duration::HALF_DOT_DOT)
        .value("HALF_DOT", Duration::HALF_DOT)
        .value("HALF", Duration::HALF)
        .value("QUARTER_DOT_DOT", Duration::QUARTER_DOT_DOT)
        .value("QUARTER_DOT", Duration::QUARTER_DOT)
        .value("QUARTER", Duration::QUARTER)
        .value("EIGHTH_DOT_DOT", Duration::EIGHTH_DOT_DOT)
        .value("EIGHTH_DOT", Duration::EIGHTH_DOT)
        .value("EIGHTH", Duration::EIGHTH)
        .value("N16TH_DOT_DOT", Duration::N16TH_DOT_DOT)
        .value("N16TH_DOT", Duration::N16TH_DOT)
        .value("N16TH", Duration::N16TH)
        .value("N32ND_DOT_DOT", Duration::N32ND_DOT_DOT)
        .value("N32ND_DOT", Duration::N32ND_DOT)
        .value("N32ND", Duration::N32ND)
        .value("N64TH_DOT_DOT", Duration::N64TH_DOT_DOT)
        .value("N64TH_DOT", Duration::N64TH_DOT)
        .value("N64TH", Duration::N64TH)
        .value("N128TH_DOT_DOT", Duration::N128TH_DOT_DOT)
        .value("N128TH_DOT", Duration::N128TH_DOT)
        .value("N128TH", Duration::N128TH)
        .value("N256TH_DOT_DOT", Duration::N256TH_DOT_DOT)
        .value("N256TH_DOT", Duration::N256TH_DOT)
        .value("N256TH", Duration::N256TH)
        .value("N512TH_DOT_DOT", Duration::N512TH_DOT_DOT)
        .value("N512TH_DOT", Duration::N512TH_DOT)
        .value("N512TH", Duration::N512TH)
        .value("N1024TH_DOT_DOT", Duration::N1024TH_DOT_DOT)
        .value("N1024TH_DOT", Duration::N1024TH_DOT)
        .value("N1024TH", Duration::N1024TH);
}
