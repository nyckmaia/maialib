#pragma once

#include "maiacore/pitch.h"

class Note;

// The pitch spellings behind Note's pitch views. Internal to maiacore: this header lives next to
// the sources, is not among the public headers, and nothing here is bound to Python. Everything
// here is defined in note.cpp, next to the chromatic spelling rule and the white-key speller the
// spellings share with Note.
namespace detail {

/**
 * @brief The concert spelling of a written pitch on an instrument that transposes by
 *        (transposeDiatonic, transposeChromatic).
 * @details The written pitch moved by the transposing interval. The letter is the written step
 *          moved by transposeDiatonic letters, carrying whole octaves across C; the alter is
 *          whatever separates that letter's natural from the exact position transposeChromatic
 *          semitones away, so a quarter tone keeps its fraction. A B-flat clarinet (-1, -2)
 *          writes F#4 for E4, Db4 for Cb4 and C1x4 for B1b3; a horn in F (-4, -7) writes B4 for
 *          E4; a piccolo (7, 12) writes Bb4 for Bb5.
 *
 *          Fallback: when transposeDiatonic is 0 while transposeChromatic is not (a MusicXML
 *          `<transpose>` without `<diatonic>`), when that alter would pass a double accidental,
 *          or when that octave would leave -1..11, the position is spelled by the chromatic rule
 *          instead: the natural or sharp spelling when transposeChromatic is positive; otherwise
 *          the flat spelling when one exists in the same octave, and the natural or sharp one
 *          when not (C4 with (0, -2) gives Bb3, Db4 with (0, -2) gives B3, C4 with (0, 2) gives
 *          D4).
 * @param written The written pitch. A rest is returned unchanged.
 * @param transposeDiatonic Letters from the written to the sounding pitch (-1 for a B-flat
 *        clarinet).
 * @param transposeChromatic Semitones from the written to the sounding pitch (-2 for a B-flat
 *        clarinet).
 * @return The written pitch itself when both intervals are 0; otherwise its concert spelling.
 * @throws std::runtime_error If the position lies below the lowest representable pitch, C1b-1
 *         (-0.5), or if it needs the fallback and lies above B11 (MIDI note 155). A spelling the
 *         diatonic interval reaches is always returned, up to Bx11.
 */
Pitch concertSpelling(const Pitch& written, int transposeDiatonic, int transposeChromatic);

/**
 * @brief The simplest spelling of a pitch: its exact position, spelled with the smallest alter.
 * @details Among the spellings of the pitch's exact position within octaves -1..11, the one with
 *          the smallest |alter|. When a sharp-side and a flat-side spelling are equally close,
 *          the one on the side of the pitch's own accidental. The octave is that spelling's own:
 *          Cb4 gives B3 and B#3 gives C4; Db4 stays Db4 and Bx3 gives C#4; C3x4 gives D1b4 and
 *          E1x4 stays E1x4.
 * @param pitch The pitch to respell. A rest is returned unchanged.
 * @return The simplest spelling of the same exact position.
 */
Pitch simplestSpelling(const Pitch& pitch);

/**
 * @brief The concert spelling of a note: concertSpelling() of its written pitch and its
 *        transposing interval.
 * @details The spelling every analysis that relates pitches reads -- Chord, Interval, and the
 *          chord extraction and melody-pattern search of Score -- so that a note of a
 *          transposing instrument is related by the pitch it sounds, spelled with its written
 *          letter moved by the diatonic interval: a B-flat clarinet's written D4 is C4, a unison
 *          with a violin's C4. For an untransposed note it is the written pitch itself, so the
 *          analyses relate untransposed notes exactly as they are written; for a rest, a rest.
 *          Note declares this function its friend, so it reads the written pitch and the
 *          interval without copying them through the public getters.
 * @param note The note.
 * @return Its concert spelling.
 * @throws std::runtime_error If the note's sounding pitch lies below the lowest representable
 *         pitch, C1b-1 (see Note::getMidiNumber()).
 */
Pitch concertPitch(const Note& note);

}  // namespace detail
