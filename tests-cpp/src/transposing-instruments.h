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

// The first line of the error for 'writtenPitch' on a (diatonic, chromatic) instrument whose
// sounding pitch lies at 'position' (as std::to_string() writes it), above B11, where no spelling
// within octaves -1..11 reaches it: only B1x11, B#11, B3x11 and Bx11 lie above B11, and only a
// diatonic interval that moves the written letter to the B of octave 11 spells them.
inline std::string aboveTheCeiling(const std::string& writtenPitch, const int diatonic,
                                   const int chromatic, const std::string& position) {
    return "[maiacore] The sounding pitch of the written pitch '" + writtenPitch +
           "' with transposeDiatonic=" + std::to_string(diatonic) +
           " and transposeChromatic=" + std::to_string(chromatic) + " is at position " + position +
           ", above B11 (MIDI note 155), and has no sounding spelling within octaves -1..11: "
           "above B11 only B1x11, B#11, B3x11 and Bx11 can be spelled, when the diatonic "
           "interval moves the written letter to the B of octave 11. A lower written pitch or a "
           "smaller transposing interval keeps the sounding pitch at or below B11.";
}
