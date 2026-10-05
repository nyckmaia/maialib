#pragma once

#include <cstdint>

#include "maiacore/pitch.h"

class Note;

// The pitch spellings behind Note's pitch views. Internal to maiacore: this header lives next to
// the sources, is not among the public headers, and nothing here is bound to Python. Everything
// here is defined in note.cpp, next to the chromatic spelling rule and the white-key speller the
// spellings share with Note.
namespace maiacore::detail {

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
 *          Inferred diatonic interval: when transposeDiatonic is 0 while transposeChromatic is
 *          not (a Note given only a chromatic interval; the MusicXML reader stores the
 *          conventional interval itself), the letter is moved by the diatonic interval
 *          conventionally written for those semitones: 7 letters for each whole octave plus, for
 *          the semitones left over, 1 for 1 or 2, 2 for 3 or 4, 3 for 5 or 6 (the tritone as an
 *          augmented fourth), 4 for 7, 5 for 8 or 9 and 6 for 10 or 11, in the direction of
 *          transposeChromatic. With (0, -2) F#4 gives E4 and C4 gives Bb3, as with a B-flat
 *          clarinet's (-1, -2), and (0, -7) moves the letter as a horn in F's (-4, -7) does. A
 *          non-zero transposeDiatonic is used as given, even when it disagrees with
 *          transposeChromatic.
 *
 *          Fallback: when the alter the letter needs would pass a double accidental, or when the
 *          letter's octave would leave -1..11, the position is spelled by the chromatic rule
 *          instead: the natural or sharp spelling when transposeChromatic is positive; otherwise
 *          the flat spelling when one exists in the same octave, and the natural or sharp one
 *          when not. A B-flat clarinet's Cbb4 gives Ab3, as Bbbb3 is not representable; Fx4 with
 *          (1, 3) gives A#4, as G would need three sharps; Bb11 with (0, 1) or (1, 1) gives B11,
 *          as Cb12 would leave octave 11.
 * @param written The written pitch. A rest is returned unchanged.
 * @param transposeDiatonic Letters from the written to the sounding pitch (-1 for a B-flat
 *        clarinet); 0 with a non-zero transposeChromatic stands for the conventional diatonic
 *        interval of those semitones.
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

/**
 * @brief The diatonic interval the speller moves the letter by: transposeDiatonic, or, when it is
 *        0 while transposeChromatic is not, the diatonic interval conventionally written for
 *        those semitones (see concertSpelling()).
 * @details concertSpelling() and soundsWithinRange() spell with it, so that every user of a stored
 *          transposing interval that needs its letters takes them from here and a stored 0 and
 *          the conventional interval it stands for are one transposition.
 * @param transposeDiatonic The stored diatonic interval.
 * @param transposeChromatic The stored chromatic interval.
 * @return The diatonic interval concertSpelling() uses.
 */
std::int64_t spelledDiatonicInterval(int transposeDiatonic, int transposeChromatic);

/**
 * @brief Whether a note would have a sounding pitch with the transposing interval
 *        (transposeDiatonic, transposeChromatic) in place of its own.
 * @details True exactly when concertSpelling() of the note's written pitch with that interval
 *          returns a spelling: false when the sounding position lies below the lowest
 *          representable pitch, C1b-1 (-0.5), or above B11 (MIDI note 155) where the diatonic
 *          interval gives no spelling (a written A#11 moved up a major second sounds B#11, a
 *          written B11 moved up a minor second has no spelling). A rest, and an untransposed
 *          note, always have one. It never throws, so a caller can check many notes before
 *          changing any: the MusicXML reader checks a <transpose>'s whole scope with it, and
 *          Part::setTransposingInterval() every note of its range.
 * @param note The note; only its written pitch is read.
 * @param transposeDiatonic Letters from the written to the sounding pitch; 0 with a non-zero
 *        transposeChromatic stands for the conventional diatonic interval of those semitones.
 * @param transposeChromatic Semitones from the written to the sounding pitch.
 * @return True if the note sounds a spellable pitch with the interval.
 */
bool soundsWithinRange(const Note& note, int transposeDiatonic, int transposeChromatic);

/**
 * @brief The diatonic interval conventionally written for a transposing interval of
 *        transposeChromatic semitones.
 * @details Seven letters for each whole octave, plus the letters of the simple interval left
 *          over -- 1 for 1 or 2 semitones (a second), 2 for 3 or 4 (a third), 3 for 5 or 6 (a
 *          fourth, the tritone being an augmented fourth), 4 for 7 (a fifth), 5 for 8 or 9 (a
 *          sixth) and 6 for 10 or 11 (a seventh) -- in the direction of the chromatic interval:
 *          -2 gives -1 (a B-flat clarinet), -9 gives -5 (an E-flat alto saxophone), 12 gives 7 (a
 *          piccolo). concertSpelling() moves the letter by it when the diatonic interval is 0,
 *          and the MusicXML reader stores it for a `<transpose>` without `<diatonic>`.
 * @param transposeChromatic Semitones from the written to the sounding pitch.
 * @return The number of letters, with the sign of transposeChromatic; it always fits an int.
 */
std::int64_t conventionalDiatonicInterval(int transposeChromatic);

}  // namespace maiacore::detail
