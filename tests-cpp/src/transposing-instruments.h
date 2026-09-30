#pragma once

#include <string>

#include "maiacore/constants.h"
#include "maiacore/note.h"

// Notes written in the parts of transposing instruments, for the tests of every class that relates
// them.

// A note written 'written' in the part of an instrument that sounds 'transposeDiatonic' letters and
// 'transposeChromatic' semitones away from what it reads.
inline Note transposingNote(const std::string& written, const int transposeDiatonic,
                            const int transposeChromatic,
                            const RhythmFigure rhythmFigure = RhythmFigure::QUARTER) {
    return Note(written, rhythmFigure, /*isNoteOn=*/true, /*inChord=*/false, transposeDiatonic,
                transposeChromatic);
}

// A B-flat clarinet sounds a major second below what it reads.
inline Note bFlatClarinet(const std::string& written,
                          const RhythmFigure rhythmFigure = RhythmFigure::QUARTER) {
    return transposingNote(written, -1, -2, rhythmFigure);
}

// A horn in F sounds a perfect fifth below what it reads.
inline Note hornInF(const std::string& written,
                    const RhythmFigure rhythmFigure = RhythmFigure::QUARTER) {
    return transposingNote(written, -4, -7, rhythmFigure);
}

// A horn in E sounds a minor sixth below what it reads.
inline Note hornInE(const std::string& written,
                    const RhythmFigure rhythmFigure = RhythmFigure::QUARTER) {
    return transposingNote(written, -5, -8, rhythmFigure);
}

// A bass clarinet in B-flat sounds a major ninth below what it reads.
inline Note bassClarinet(const std::string& written,
                         const RhythmFigure rhythmFigure = RhythmFigure::QUARTER) {
    return transposingNote(written, -8, -14, rhythmFigure);
}

// A piccolo sounds an octave above what it reads.
inline Note piccolo(const std::string& written,
                    const RhythmFigure rhythmFigure = RhythmFigure::QUARTER) {
    return transposingNote(written, 7, 12, rhythmFigure);
}
