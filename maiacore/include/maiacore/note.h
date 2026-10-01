#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "maiacore/constants.h"
#include "maiacore/duration.h"
#include "maiacore/key.h"
#include "maiacore/pitch.h"
#include "maiacore/time-signature.h"

/// @cond IGNORE_DOXYGEN
class Note;

// The spelling the analyses relate a note by. Declared, with its documentation, in the private
// header pitch-views.h next to the sources; not part of the public API.
namespace maiacore::detail {
Pitch concertPitch(const Note& note);
}  // namespace maiacore::detail
/// @endcond

/**
 * @brief Represents a musical note, including pitch, duration, articulation, and MusicXML-related
 * attributes.
 *
 * The Note class provides methods for manipulating and querying musical notes, including pitch and
 * octave handling, duration and rhythm, articulations, ties, beams, transposition, enharmonic
 * equivalents, and MusicXML serialization. Designed for music analysis, computational musicology,
 * and MusicXML processing.
 *
 * @par Pitch views
 * A note of a transposing instrument is written at one pitch and sounds at another, so Note
 * answers about its pitch in three views:
 * - **Written**, the pitch as written in the part: getWrittenPitch(), getWrittenOctave(),
 *   getWrittenPitchClass(), getWrittenPitchStep() and getDiatonicWrittenPitchClass(). The
 *   unprefixed getters getPitch(), getOctave(), getPitchClass(), getPitchStep() and
 *   getAlterSymbol() are shortcuts for it; the enharmonic family (getEnharmonicPitch(),
 *   getEnharmonicPitches(), getEnharmonicNote(), getEnharmonicNotes(), toEnharmonicPitch()),
 *   transpose(), the setters and toXML() work on it.
 * - **Sounding**, what the note sounds, in its simplest spelling: getSoundingPitch(),
 *   getSoundingOctave(), getSoundingPitchClass(), getSoundingPitchStep() and
 *   getDiatonicSoundingPitchClass(). The written pitch is moved by the transposing interval -- its
 *   letter by the diatonic interval, its position by the chromatic one -- and respelled with the
 *   smallest accidental any spelling of that position has; between a sharp and a flat equally
 *   close, the moved spelling's side is kept. The octave is that spelling's own: an untransposed
 *   Cb4 sounds B3, a B-flat clarinet's written Db4 sounds B3 and its written Eb4 sounds Db4.
 * - **Acoustic**, measures of what sounds: getMidiNumber(), getQuarterToneSteps(), getFrequency()
 *   and getHarmonicSpectrum().
 *
 * Without a transposing interval the written and the sounding pitch are the same pitch, spelled
 * alike unless the written spelling has a simpler one (Cb, Fb, E#, B#, a double accidental, or a
 * quarter tone such as C3x4, which sounds D1b4).
 *
 * The analyses that relate notes -- Chord, Interval, and the chord extraction and melody search of
 * Score -- relate a note of a transposing instrument at the pitch it sounds, spelled with its
 * written letter moved by the diatonic transposing interval (a B-flat clarinet's written Db4 is a
 * Cb4 there, as in a C-flat chord), and an untransposed note exactly as it is written. operator==()
 * compares notes the same way.
 */
class Note {
   private:
    Pitch _writtenPitch;  ///< Written pitch. A rest is represented by _writtenPitch.isRest();
                          ///< there is no separate "is this a rest" flag (see isNoteOn()).

    bool _inChord;            ///< True if this note is part of a chord.
    int _transposeDiatonic;   ///< Diatonic transposition interval.
    int _transposeChromatic;  ///< Chromatic transposition interval.
    int _voice;               ///< Voice number.
    int _staff;               ///< Staff number.
    bool _isGraceNote;        ///< True if this is a grace note.
    std::string _stem;        ///< Stem direction ("up", "down", etc.).
    bool _isTuplet;           ///< True if this note is part of a tuplet.
    bool _isPitched;          ///< True if this note is pitched, false for unpitched.
    int _unpitchedIndex;      ///< Index for unpitched percussion notes.
    Duration _duration;       ///< Duration object for this note.

    std::pair<std::string, std::string> _slur;  ///< Slur type and orientation.
    std::vector<std::string> _tie;              ///< Tie types ("start", "stop").
    std::vector<std::string> _articulation;     ///< Articulation marks.
    std::vector<std::string> _beam;             ///< Beam types.

    /**
     * @brief Computes the sounding Pitch: the simplest spelling
     *        (maiacore::detail::simplestSpelling()) of the concert Pitch, computeConcertPitch().
     * @details A rest returns a rest. Nothing is cached: every Sounding getter
     *          (getSoundingPitch(), getSoundingOctave(), getSoundingPitchClass(), ...) calls this
     *          on demand. setTransposingInterval(), setPitch() and the partial setters derive the
     *          concert spelling for the new interval or written pitch before storing it, so a
     *          sounding pitch that cannot be spelled throws there, with the note unchanged, rather
     *          than from a later getter call -- except one below the lowest representable pitch,
     *          which leaves the note constructible.
     * @return The sounding Pitch.
     * @throws std::runtime_error If this note's sounding pitch lies below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    Pitch computeSoundingPitch() const;

    /**
     * @brief Computes the concert Pitch: the written pitch moved by the transposing interval, with
     *        the letter the diatonic interval reaches (maiacore::detail::concertSpelling()).
     * @details A rest, or an untransposed note, returns _writtenPitch itself. Nothing is cached.
     *          The analyses that relate pitches read it through
     *          maiacore::detail::concertPitch().
     * @return The concert Pitch.
     * @throws std::runtime_error If this note's sounding pitch lies below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    Pitch computeConcertPitch() const;

    /// @cond IGNORE_DOXYGEN
    friend Pitch maiacore::detail::concertPitch(const Note& note);
    /// @endcond

   public:
    /**
     * @brief Default constructor. Initializes a note as "A4" (MIDI 69).
     */
    Note();

    /**
     * @brief Constructs a Note from a pitch string and rhythm figure.
     * @param pitch Pitch string (e.g., "C4", "G#3", "Dbb-1", "Bx11", "C1x4"). Accidentals: "bb",
     *        "b", "#", "x" and the quarter tones "3b", "1b", "1x", "3x"; octaves -1..11 (a missing
     *        octave defaults to 4); the MIDI number, rounded ties upward for a quarter tone, must
     *        be >= 0. An empty string or a string containing "rest" creates a rest.
     * @param rhythmFigure Rhythm figure (default: QUARTER).
     * @param isNoteOn True if sounding note, false for rest.
     * @param inChord True if part of a chord.
     * @param transposeDiatonic Diatonic transposition interval.
     * @param transposeChromatic Chromatic transposition interval.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     * @throws std::runtime_error If the pitch string is invalid (see Helper::splitPitch()), or if
     *         the note is transposed and its sounding pitch cannot be spelled: one above B11
     *         (MIDI note 155) that the diatonic interval does not spell (see
     *         setTransposingInterval()).
     */
    explicit Note(const std::string& pitch, const RhythmFigure rhythmFigure = RhythmFigure::QUARTER,
                  bool isNoteOn = true, bool inChord = false, const int transposeDiatonic = 0,
                  const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Constructs a Note from a MIDI number, accidental type, and rhythm figure.
     * @param midiNumber MIDI note number, spelled within octaves -1..11 (e.g., 5 -> "F-1").
     * @param accType Accidental type: "", "#", "b", "x" or "bb" (see Helper::midiNote2pitch()).
     * @param rhythmFigure Rhythm figure (default: QUARTER).
     * @param isNoteOn True if sounding note, false for rest.
     * @param inChord True if part of a chord.
     * @param transposeDiatonic Diatonic transposition interval.
     * @param transposeChromatic Chromatic transposition interval.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     * @throws std::runtime_error If the MIDI number cannot be spelled with accType within
     *         octaves -1..11, or if the note is transposed and its sounding pitch cannot be
     *         spelled: one above B11 (MIDI note 155) that the diatonic interval does not spell (see
     *         setTransposingInterval()).
     */
    explicit Note(const int midiNumber, const std::string& accType = "",
                  const RhythmFigure rhythmFigure = RhythmFigure::QUARTER, bool isNoteOn = true,
                  bool inChord = false, const int transposeDiatonic = 0,
                  const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Destructor.
     */
    ~Note();

    // ===== SETTERS ===== //

    /**
     * @brief Sets the pitch class (e.g., "C", "D#", "Bb") of the written pitch, keeping its
     *        octave.
     * @details Delegates to Pitch::setPitchClass() and inherits its policy: on a rest, which has
     *          no octave to keep, the octave defaults to 4.
     *
     *          The change is made on a copy of the written pitch, whose sounding pitch is checked
     *          with the transposing interval before it is stored, as setPitch() checks a whole
     *          pitch: a sounding pitch that cannot be spelled, one above B11 (MIDI note 155) that
     *          the diatonic interval does not spell (see setTransposingInterval()), throws with
     *          the note unchanged. A sounding pitch below the lowest representable pitch, C1b-1,
     *          is accepted, as setPitch() accepts it.
     * @param pitchClass The pitch class string.
     * @throws std::runtime_error If the pitch class is invalid or the pitch lies below MIDI note 0
     *         (see Helper::splitPitch()), or if the sounding pitch the change gives cannot be
     *         spelled (the same error setPitch() raises for that pitch). The note is then left
     *         unchanged.
     */
    void setPitchClass(const std::string& pitchClass);

    /**
     * @brief Sets the octave of the written pitch, keeping its step and accidental.
     * @details Delegates to Pitch::setOctave() and inherits its policy: refuses on a rest, and
     *          when the pitch would fall below MIDI note 0 (LOG_WARN, no change).
     *
     *          The change is made on a copy of the written pitch, whose sounding pitch is checked
     *          with the transposing interval before it is stored, as setPitch() checks a whole
     *          pitch: a sounding pitch that cannot be spelled, one above B11 (MIDI note 155) that
     *          the diatonic interval does not spell (see setTransposingInterval()), throws with
     *          the note unchanged. A sounding pitch below the lowest representable pitch, C1b-1,
     *          is accepted, as setPitch() accepts it.
     * @param octave Octave number, within [-1, 11].
     * @throws std::runtime_error If octave is outside [-1, 11] on a note, or if the sounding pitch
     *         the change gives cannot be spelled (the same error setPitch() raises for that
     *         pitch). The note is then left unchanged.
     */
    void setOctave(int octave);

    /**
     * @brief Sets the diatonic step of the written pitch, keeping the current accidental and
     *        octave.
     * @details Delegates to Pitch::setStep() and inherits its policy: permissive on a rest,
     *          resurrecting it into a note with the octave defaulted to 4, and refusing a step
     *          that would move the pitch below MIDI note 0 (LOG_WARN, no change).
     *
     *          The change is made on a copy of the written pitch, whose sounding pitch is checked
     *          with the transposing interval before it is stored, as setPitch() checks a whole
     *          pitch: a sounding pitch that cannot be spelled, one above B11 (MIDI note 155) that
     *          the diatonic interval does not spell (see setTransposingInterval()), throws with
     *          the note unchanged. A sounding pitch below the lowest representable pitch, C1b-1,
     *          is accepted, as setPitch() accepts it.
     * @param step Diatonic step ("A".."G").
     * @throws std::runtime_error If step is not one of "A".."G", or if the sounding pitch the
     *         change gives cannot be spelled (the same error setPitch() raises for that pitch).
     *         The note is then left unchanged.
     */
    void setStep(const std::string& step);

    /**
     * @brief Sets the accidental value (in semitones) of the written pitch.
     * @details Delegates to Pitch::setAlter() and inherits its policy: refuses on a rest, since
     *          a bare alter value carries no octave to resurrect one with, and when the pitch would
     *          fall below MIDI note 0 (LOG_WARN, no change). The alter must be exactly on the grid:
     *          a value near a multiple of 0.5, such as 0.99996, is rejected rather than rounded.
     *
     *          The change is made on a copy of the written pitch, whose sounding pitch is checked
     *          with the transposing interval before it is stored, as setPitch() checks a whole
     *          pitch: a sounding pitch that cannot be spelled, one above B11 (MIDI note 155) that
     *          the diatonic interval does not spell (see setTransposingInterval()), throws with
     *          the note unchanged. A sounding pitch below the lowest representable pitch, C1b-1,
     *          is accepted, as setPitch() accepts it.
     * @param alter Alter value; must be a multiple of 0.5 (a semitone or quarter-tone step),
     *        within [-2, 2].
     * @throws std::runtime_error If alter is NaN or infinite, is not a multiple of 0.5, or is
     *         outside [-2, 2], or if the sounding pitch the change gives cannot be spelled (the
     *         same error setPitch() raises for that pitch). The note is then left unchanged.
     */
    void setAlter(float alter);

    /**
     * @brief Rounds a quarter-tone accidental to the nearest semitone, ties upward.
     * @details Delegates to Pitch::roundToSemitone(), which applies roundTiesUpward()
     *          (utils.h), the library's single implementation of the ties-upward rule:
     *          `C1x4` -> `C#4`, `D1b4` -> `D4`, `D3b4` -> `Db4`. A note that already
     *          carries a whole-tone accidental (or none) is left unchanged, so this is safe to
     *          call unconditionally. Acts on the written pitch; see isQuarterTone().
     */
    void roundToSemitone();

    /**
     * @brief Sets the duration for the note.
     * @param duration Duration object.
     */
    void setDuration(const Duration& duration);

    /**
     * @brief Sets the duration for the note using quarter note value.
     * @param quarterDuration Duration in quarter notes.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     * @throws std::runtime_error If the duration cannot be converted to a rhythm figure (e.g. 0 or
     *         a negative value); the note is then left unchanged.
     */
    void setDuration(const float quarterDuration, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Sets the duration for the note using tick values.
     * @param durationTicks Duration in ticks.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     */
    void setDuration(const int durationTicks, const int divisionsPerQuarterNote);

    /**
     * @brief Sets whether the note is sounding (note on) or a rest (note off).
     * @param isNoteOn True for sounding note, false for rest.
     */
    void setIsNoteOn(bool isNoteOn);

    /**
     * @brief Sets the pitch (e.g., "C4", "G#3", "Bb-1", "C10") for the note.
     * @details Replaces the pitch class, octave, accidental symbol and MIDI number. Accepts the
     *          same spellings as the pitch-string constructor. The transposing interval is kept,
     *          so the note then sounds this pitch moved by it; setting a rest also clears the
     *          interval and the in-chord and grace-note flags.
     * @param pitch Pitch string. An empty string or a string containing "rest" turns the note
     *        into a rest.
     * @throws std::runtime_error If the pitch string is invalid (see Helper::splitPitch()), or if
     *         its sounding pitch with the current transposing interval cannot be spelled: one
     *         above B11 (MIDI note 155) that the diatonic interval does not spell (see
     *         setTransposingInterval()). The note is then left unchanged. A sounding pitch below
     *         the lowest representable pitch, C1b-1, is accepted, as setTransposingInterval()
     *         accepts it.
     */
    void setPitch(const std::string& pitch);

    /**
     * @brief Sets whether the note is part of a chord.
     * @param inChord True if in chord.
     */
    void setIsInChord(bool inChord);

    /**
     * @brief Sets the transposing interval for the note.
     * @details The note is written at its written pitch and sounds that pitch moved by the
     *          interval: its letter by diatonicInterval letters, its position by chromaticInterval
     *          semitones (see getSoundingPitch()). Every sounding getter derives the sounding
     *          pitch on demand, so a second call replaces the interval rather than adding to it.
     *          The sounding pitch is derived here too, before the interval is stored, so one that
     *          cannot be spelled throws now, with the note unchanged, rather than from a later
     *          getter call. Every sounding pitch up to B11 (MIDI note 155) can be spelled; above
     *          it only B1x11, B#11, B3x11 and Bx11 lie within octaves -1..11, and they are spelled
     *          only when the diatonic interval moves the written letter to the B of octave 11 (a
     *          written A#11 moved up a major second sounds B#11; with no diatonic interval it could
     *          not be spelled). A sounding pitch below the lowest representable pitch, C1b-1, is
     *          accepted: the note stays constructible, and each sounding getter reports the
     *          condition when asked. A rest ignores the call.
     * @param diatonicInterval Diatonic interval: letters from the written to the sounding pitch
     *        (-1 for a B-flat clarinet).
     * @param chromaticInterval Chromatic interval: semitones from the written to the sounding pitch
     *        (-2 for a B-flat clarinet).
     * @throws std::runtime_error If the sounding pitch with this interval cannot be spelled: one
     *         above B11 that the diatonic interval does not spell. The note is then left unchanged.
     */
    void setTransposingInterval(const int diatonicInterval, const int chromaticInterval);

    /**
     * @brief Sets the voice number for the note.
     * @param voice Voice number.
     */
    void setVoice(const int voice);

    /**
     * @brief Sets the staff number for the note.
     * @param staff Staff number.
     */
    void setStaff(const int staff);

    /**
     * @brief Sets whether the note is a grace note.
     * @param isGraceNote True if grace note.
     */
    void setIsGraceNote(const bool isGraceNote = false);

    /**
     * @brief Sets the stem direction for the note.
     * @param stem Stem direction ("up", "down", etc.).
     */
    void setStem(const std::string& stem);

    /**
     * @brief Sets the note as part of a tuplet.
     * @param isTuplet True if part of a tuplet.
     */
    void setIsTuplet(const bool isTuplet = false);

    /**
     * @brief Sets the tuplet values for the note.
     * @param actualNotes Number of actual notes in tuplet.
     * @param normalNotes Number of normal notes in tuplet.
     * @param normalType Note type for normal notes (default: "eighth").
     * @throws std::runtime_error If normalType is not a note type; the note is then left
     *         unchanged.
     */
    void setTupleValues(const int actualNotes, const int normalNotes,
                        const std::string& normalType = "eighth");

    /**
     * @brief Sets whether the note is pitched (true) or unpitched (false).
     * @param isPitched True if pitched.
     */
    void setIsPitched(const bool isPitched = true);

    /**
     * @brief Sets the unpitched index for percussion notes.
     * @param unpitchedIndex Index for unpitched note.
     */
    void setUnpitchedIndex(const int unpitchedIndex);

    /**
     * @brief Sets the note as tied at the start.
     */
    void setTieStart();

    /**
     * @brief Sets the note as tied at the stop.
     */
    void setTieStop();

    /**
     * @brief Sets the note as tied at both start and stop.
     */
    void setTieStopStart();

    /**
     * @brief Adds a tie type to the note.
     * @param tieType Tie type ("start", "stop").
     */
    void addTie(const std::string& tieType);

    /**
     * @brief Adds a slur to the note.
     * @param slurType Slur type ("start", "stop").
     * @param slurOrientation Slur orientation ("above", "below").
     */
    void addSlur(const std::string& slurType, const std::string& slurOrientation);

    /**
     * @brief Adds an articulation mark to the note.
     * @param articulation Articulation string.
     */
    void addArticulation(const std::string& articulation);

    /**
     * @brief Adds a beam type to the note.
     * @param beam Beam type string.
     */
    void addBeam(const std::string& beam);

    // ===== GETTERS ===== //

    /**
     * @brief Returns the pitch class of the sounding pitch (see getSoundingPitch()): its step and
     *        accidental, without the octave.
     * @details Spelled as getSoundingPitch() spells it, silent fallback included: a B-flat
     *          clarinet's written Db4 gives "B", an untransposed Cb4 "B".
     * @return Sounding pitch class string, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    const std::string getSoundingPitchClass() const;

    /**
     * @brief Returns the sounding pitch: what the note sounds, in its simplest spelling.
     * @details The written pitch moved by the transposing interval -- its letter by
     *          getTransposeDiatonic() letters, carrying octaves across C, its exact position by
     *          getTransposeChromatic() semitones -- and then respelled with the smallest accidental
     *          any spelling of that position has within octaves -1..11. Between a sharp and a flat
     *          equally close, the side of the moved spelling is kept. The octave is that
     *          spelling's own. A B-flat clarinet's written Db4 sounds B3 and its written Eb4 Db4;
     *          a quarter tone keeps its fraction (a written C1x4 sounds B1b3). Without a
     *          transposing interval this is the written pitch in its simplest spelling: an
     *          untransposed Cb4 sounds B3, C3x4 sounds D1b4, Db4 stays Db4.
     *
     *          Fallback, silent (no warning is given): when the interval gives no letter --
     *          getTransposeDiatonic() is 0 while getTransposeChromatic() is not, as for a MusicXML
     *          `<transpose>` without `<diatonic>` -- or the letter it gives would need an
     *          accidental beyond a double sharp or flat, or an octave outside -1..11, the position
     *          is spelled from the semitones alone and then simplified as above: a black key takes
     *          a sharp when the instrument transposes up and a flat when it transposes down. A
     *          written D4 sounds D#4 a semitone up and Db4 a semitone down without a diatonic
     *          interval.
     * @return Sounding pitch string, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    const std::string getSoundingPitch() const;

    /**
     * @brief Returns the pitch class of the written pitch (as notated).
     * @return Written pitch class string, or "rest" for a rest.
     */
    const std::string getWrittenPitchClass() const;

    /**
     * @brief Returns the full written pitch (as notated).
     * @return Written pitch string, or "rest" for a rest.
     */
    const std::string getWrittenPitch() const;

    /**
     * @brief Returns the diatonic step of the written pitch (e.g., "C", "D").
     * @return Diatonic written pitch class, or "rest" for a rest.
     */
    const std::string getDiatonicWrittenPitchClass() const;

    /**
     * @brief Returns the diatonic step of the sounding pitch (e.g., "C", "D").
     * @details The step of getSoundingPitch(), spelled as it spells it, silent fallback included.
     * @return Diatonic sounding pitch class, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    const std::string getDiatonicSoundingPitchClass() const;

    /**
     * @brief Returns the octave of the sounding pitch (see getSoundingPitch()).
     * @details The octave of the sounding spelling, silent fallback included: an untransposed Cb4
     *          sounds B3, octave 3, and a piccolo's written C4 sounds C5, octave 5. An empty
     *          optional means this note is a rest, which has no octave; isNoteOff() is the
     *          authoritative test.
     * @return Sounding octave number, or an empty optional for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::optional<int> getSoundingOctave() const;

    /**
     * @brief Returns the written octave (as notated).
     * @details An empty optional means this note is a rest, which has no octave.
     *          isNoteOff() is the authoritative test. This reads the written pitch directly, so
     *          it never throws.
     * @return Written octave number, or an empty optional for a rest.
     */
    std::optional<int> getWrittenOctave() const;

    /**
     * @brief Returns the octave of the written pitch: a shortcut for getWrittenOctave().
     * @details The octave as notated in the part, whatever the instrument sounds; the octave it
     *          sounds is getSoundingOctave(). An empty optional means this note is a rest, which
     *          has no octave; isNoteOff() is the authoritative test. Never throws.
     * @return Octave number, or an empty optional for a rest.
     */
    std::optional<int> getOctave() const;

    /**
     * @brief Returns the pitch class of the written pitch: a shortcut for getWrittenPitchClass().
     * @details The step and accidental as notated in the part; the pitch class it sounds is
     *          getSoundingPitchClass(). Never throws.
     * @return Pitch class string, or "rest" for a rest.
     */
    std::string getPitchClass() const;

    /**
     * @brief Returns the diatonic step of the written pitch (e.g., "C", "D").
     * @return Written pitch step, or "rest" for a rest.
     */
    std::string getWrittenPitchStep() const;

    /**
     * @brief Returns the diatonic step of the sounding pitch (e.g., "C", "D").
     * @details The step of getSoundingPitch(), spelled as it spells it, silent fallback included.
     * @return Sounding pitch step, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::string getSoundingPitchStep() const;

    /**
     * @brief Returns the diatonic step of the written pitch: a shortcut for getWrittenPitchStep().
     * @details The step as notated in the part; the step it sounds is getSoundingPitchStep().
     *          Never throws.
     * @return Pitch step string, or "rest" for a rest.
     */
    std::string getPitchStep() const;

    /**
     * @brief Returns the note type as a string (e.g., "quarter", "eighth-dot").
     * @return Note type string.
     */
    std::string getType() const;

    /**
     * @brief Returns the long note type string (e.g., "quarter-dot").
     * @return Long note type string.
     */
    std::string getLongType() const;

    /**
     * @brief Returns the short note type string (e.g., "quarter").
     * @return Short note type string.
     */
    std::string getShortType() const;

    /**
     * @brief Returns the duration in ticks.
     * @return Duration in ticks.
     */
    int getDurationTicks() const;

    /**
     * @brief Returns the number of dots for the note.
     * @return Number of dots.
     */
    int getNumDots() const;

    /**
     * @brief Returns true if the note is dotted.
     * @return True if dotted.
     */
    bool isDotted() const;

    /**
     * @brief Returns true if the note is double-dotted.
     * @return True if double-dotted.
     */
    bool isDoubleDotted() const;

    /**
     * @brief Returns the divisions per quarter note for the note.
     * @return Divisions per quarter note.
     */
    int getDivisionsPerQuarterNote() const;

    /**
     * @brief Returns the Duration object for the note.
     * @return Duration object.
     */
    const Duration& getDuration() const;

    /**
     * @brief Returns the duration in quarter notes as a float.
     * @return Quarter duration.
     */
    float getQuarterDuration() const;

    /**
     * @brief Returns true if the note is a grace note.
     * @return True if grace note.
     */
    bool isGraceNote() const;

    /**
     * @brief Returns true if the note is a sounding note (note on).
     * @return True if note on.
     */
    bool isNoteOn() const;

    /**
     * @brief Returns true if the note is a rest (note off).
     * @return True if note off.
     */
    bool isNoteOff() const;

    /**
     * @brief Returns true if this note carries a quarter-tone accidental.
     * @details True when the alter value has a fractional part (e.g. 0.5 for `C1x4`, -1.5 for
     *          `D3b4`), false for the whole-tone accidentals and for a rest (whose alter is 0).
     *          Reads the *written* pitch deliberately: transposition is an integer number of
     *          semitones, so a written quarter tone is always a sounding quarter tone and vice
     *          versa, and the written pitch can never throw the way getSoundingPitch() can for a
     *          note transposed below the representable minimum.
     * @return True if the note's accidental is a quarter tone.
     */
    bool isQuarterTone() const;

    /**
     * @brief Returns the written pitch: a shortcut for getWrittenPitch().
     * @details The pitch as notated in the part -- the one the note was constructed or set with --
     *          whatever the instrument sounds: a B-flat clarinet's written D4 answers "D4". What it
     *          sounds is getSoundingPitch(), and getMidiNumber() measures it. Never throws.
     * @return Pitch string, or "rest" for a rest.
     */
    std::string getPitch() const;

    /**
     * @brief Returns the MIDI note number (sounding).
     * @details The written MIDI number moved by the chromatic transposing interval. A quarter
     *          tone rounds, ties upward, to the semitone above it (see getQuarterToneSteps() for
     *          the exact position).
     *
     *          A transposing instrument can sound below the lowest pitch this library represents,
     *          C1b-1 (-0.5, which rounds to MIDI note 0): e.g. a written "C#-1" on a B-flat
     *          clarinet sounds at -1. Such a note is constructible and isNoteOn() is true, but it
     *          has no sounding spelling, octave, MIDI number or frequency, so this method, the
     *          other acoustic getters and the Sounding getters throw the same error for it rather
     *          than answer a rest's values. Its written pitch stays available through the written
     *          getters and the unprefixed ones.
     * @return MIDI note number, or -1 (MUSIC_XML::MIDI::NUMBER::MIDI_REST) for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below C1b-1.
     */
    int getMidiNumber() const;

    /**
     * @brief Returns the exact, unrounded sounding pitch position, in semitones.
     * @details The written pitch's exact position (Pitch::getQuarterToneSteps()) plus the
     *          chromatic transposing interval, so a quarter tone keeps its half: a written `C1x4`
     *          is 60.5 where getMidiNumber() rounds it to 61, and on a B-flat clarinet
     *          (transposeChromatic -2) it sounds at 58.5. Every position is a multiple of 0.5,
     *          which a float holds exactly, so positions compare and subtract exactly. The
     *          ordering operators compare notes by this value.
     * @return The exact sounding position, or -1.0 (MUSIC_XML::MIDI::NUMBER::MIDI_REST) for a
     *         rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    float getQuarterToneSteps() const;

    /**
     * @brief Returns the voice number.
     * @return Voice number.
     */
    int getVoice() const;

    /**
     * @brief Returns the staff number.
     * @return Staff number.
     */
    int getStaff() const;

    /**
     * @brief Returns the stem direction.
     * @return Stem string.
     */
    std::string getStem() const;

    /**
     * @brief Returns the tie types for the note.
     * @return Vector of tie strings.
     */
    std::vector<std::string> getTie() const;

    /**
     * @brief Removes all ties from the note.
     */
    void removeTies();

    /**
     * @brief Returns the slur information (type and orientation).
     * @return Pair of slur type and orientation.
     */
    std::pair<std::string, std::string> getSlur() const;

    /**
     * @brief Returns the articulation marks for the note.
     * @return Vector of articulation strings.
     */
    std::vector<std::string> getArticulation() const;

    /**
     * @brief Returns the beam types for the note.
     * @return Vector of beam strings.
     */
    std::vector<std::string> getBeam() const;

    /**
     * @brief Returns true if the note is part of a tuplet.
     * @return True if tuplet.
     */
    bool isTuplet() const;

    /**
     * @brief Returns true if the note is pitched.
     * @return True if pitched.
     */
    bool isPitched() const;

    /**
     * @brief Returns the unpitched index for percussion notes.
     * @return Unpitched index.
     */
    int getUnpitchedIndex() const;

    /**
     * @brief Returns the accidental symbol of the written pitch (e.g., "#", "b").
     * @details The accidental as notated in the part: a B-flat clarinet's written C4 answers ""
     *          although it sounds Bb3 (see getSoundingPitchClass()). Never throws.
     * @return Accidental symbol string, or "" for a natural and for a rest.
     */
    std::string getAlterSymbol() const;

    /**
     * @brief Returns true if the note is part of a chord.
     * @return True if in chord.
     */
    bool inChord() const;

    /**
     * @brief Returns the diatonic transposition interval.
     * @return Diatonic interval.
     */
    int getTransposeDiatonic() const;

    /**
     * @brief Returns the chromatic transposition interval.
     * @return Chromatic interval.
     */
    int getTransposeChromatic() const;

    /**
     * @brief Returns true if the note is transposed.
     * @return True if transposed.
     */
    bool isTransposed() const;

    /**
     * @brief Returns an enharmonic spelling of the written pitch, getPitch().
     * @details The written pitch is respelled whatever the instrument sounds, so a transposed note
     *          is respelled exactly as an untransposed note with the same written pitch.
     *
     *          A semitone pitch is respelled among the other spellings of the same MIDI number,
     *          with the accidentals "bb", "b", natural, "#" and "x", within octaves -1..11:
     *          - White keys: a natural returns its flat-side spelling by default (C4 -> Dbb4) and
     *            its sharp-side spelling as the alternative (B#3); a flat-side or sharp-side
     *            spelling returns the natural by default and the remaining spelling as the
     *            alternative.
     *          - Black keys: "#" and "b" swap (C#4 <-> Db4) and the double accidental is the
     *            alternative; "x" and "bb" return the single accidental in the same direction
     *            by default and the opposite single accidental as the alternative.
     *
     *          A quarter tone is respelled on the 24-tone grid. Its partners are the white-key
     *          steps within 1.5 semitones of it, each spelled with the quarter-tone accidental
     *          that separates them ("1x" +0.5, "3x" +1.5, "1b" -0.5, "3b" -1.5) in that step's
     *          own octave, within octaves -1..11, so a quarter tone has one or two: C1x4 (60.5)
     *          has D3b4 and B3x3, C3x4 (61.5) only D1b4. The default is the partner with the
     *          smaller alter; on a tie, the one on the other side of the note's own accidental,
     *          as "#" and "b" swap (C1x4 -> D3b4, E1b4 -> D3x4). The alternative is the other
     *          partner.
     *
     *          Range fallback, for both: a missing alternative returns the default, and a missing
     *          default returns the note's own pitch (e.g., "Bx11" -> "Bx11", "B1x11" -> "B1x11").
     * @param alternativeEnharmonicPitch If true, returns the alternative enharmonic.
     * @return Enharmonic pitch string, or "rest" for a rest.
     */
    std::string getEnharmonicPitch(const bool alternativeEnharmonicPitch = false) const;

    /**
     * @brief Returns the default and alternative enharmonic spellings of the written pitch.
     * @details {getEnharmonicPitch(false), getEnharmonicPitch(true)}, preceded by getPitch() if
     *          includeCurrentPitch is true, so the vector may contain duplicates (e.g., "G#4" ->
     *          {"G#4", "Ab4", "Ab4"}, "C3x4" -> {"C3x4", "D1b4", "D1b4"}).
     * @param includeCurrentPitch If true, includes the current pitch.
     * @return Vector of enharmonic pitch strings.
     */
    std::vector<std::string> getEnharmonicPitches(const bool includeCurrentPitch = false) const;

    /**
     * @brief Returns a new Note spelled with getEnharmonicPitch(alternativeEnharmonicPitch).
     * @details The new Note holds that respelling of the written pitch as its written pitch, with
     *          this note's transposing interval and the default rhythm figure, so it sounds exactly
     *          what this note sounds -- the same getMidiNumber() and getQuarterToneSteps() -- only
     *          written differently: a B-flat clarinet's written D4 (sounding C4) gives a written
     *          Ebb4, also sounding C4. A rest gives a rest.
     * @param alternativeEnharmonicPitch If true, uses the alternative enharmonic.
     * @return Enharmonic Note object.
     * @throws std::runtime_error If the respelling cannot be spelled once moved by the transposing
     *         interval, the case toEnharmonicPitch() raises for: a written A#11 moved up a major
     *         second sounds B#11, but its respelling Bb11 would need the letter C of octave 12.
     */
    Note getEnharmonicNote(const bool alternativeEnharmonicPitch = false) const;

    /**
     * @brief Returns new Notes spelled with the strings getEnharmonicPitches() returns.
     * @details Each new Note holds its respelling of the written pitch as its written pitch, with
     *          this note's transposing interval and the default rhythm figure, so every one sounds
     *          exactly what this note sounds (see getEnharmonicNote()); entries may repeat.
     * @param includeCurrentPitch If true, includes a Note spelled with the current pitch.
     * @return Vector of enharmonic Note objects.
     * @throws std::runtime_error If a respelling cannot be spelled once moved by the transposing
     *         interval (see getEnharmonicNote()).
     */
    std::vector<Note> getEnharmonicNotes(const bool includeCurrentPitch = false) const;

    /**
     * @brief Returns the scale degree of the note's written step in a given key.
     * @details Reads getPitchStep(), the written step, and only the step: in C major C#4 and
     *          C1x4 are degree 1, like C4. The keys read from a part's measures are that part's
     *          written key signatures, so a note of a transposing instrument is measured in the
     *          key its part is written in: a B-flat clarinet's written D4 is degree 2 in C major,
     *          although it sounds C4.
     * @param key Key object for reference; a minor key counts from its own tonic.
     * @return Scale degree, 1 to 7, or 0 for a rest.
     */
    int getScaleDegree(const Key& key) const;

    /**
     * @brief Respells the written pitch with getEnharmonicPitch(alternativeEnharmonicPitch),
     *        through setPitch().
     * @details The transposing interval is kept, so the note sounds exactly what it sounded
     *          before, only written differently: a B-flat clarinet's written C#4 (sounding B3)
     *          becomes a written Db4, still sounding B3.
     * @param alternativeEnharmonicPitch If true, uses alternative enharmonic.
     * @throws std::runtime_error If setPitch() throws for the respelling: on a transposing
     *         instrument, when the respelling cannot be spelled once moved by the transposing
     *         interval (a written A#11 moved up a major second sounds B#11, but its respelling
     *         Bb11 would need the letter C of octave 12). The note is then left unchanged.
     */
    void toEnharmonicPitch(const bool alternativeEnharmonicPitch = false);

    /**
     * @brief Returns the frequency of the note's sounding pitch in Hz, in twelve-tone equal
     * temperament.
     * @details Computed from the rounded MIDI number, getMidiNumber(), so a quarter tone is
     *          rounded to the nearest semitone, ties upward -- the semitone above it: a note
     *          "A1x4" gets the frequency of A#4, about 466.16 Hz. Pitch::getFrequency() gives the
     *          exact frequency, about 452.89 Hz.
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Frequency in Hz, or 0.0 for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    float getFrequency(const float freqA4 = 440.0f) const;

    /**
     * @brief Returns the harmonic spectrum of the note (partials and amplitudes).
     * @param numPartials Number of partials.
     * @param amplCallback Optional amplitude callback.
     * @param partialsDecayExpRate Optional Partials decay exponential rate (default: 0.88).
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Pair of vectors: frequencies and amplitudes.
     */
    std::pair<std::vector<float>, std::vector<float>> getHarmonicSpectrum(
        const int numPartials = 6,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr,
        const float partialsDecayExpRate = 0.88f, const float freqA4 = 440.0f) const;

    /**
     * @brief Transposes the written pitch by a number of semitones and optional accidental type.
     * @details Transposes on exact pitch positions, so a quarter tone survives: transposing
     *          "C4" by 0.5 gives "C1x4", and transposing "C1x4" by 2 gives "D1x4". The written
     *          pitch moves once and the transposing interval is kept, so what the note sounds moves
     *          by the same number of semitones: a B-flat clarinet's written C4 (sounding Bb3)
     *          transposed by 2 is a written D4, sounding C4, and transposing by 0 changes nothing.
     * @param semitones Number of semitones; must be a multiple of 0.5 (e.g. 0.5 for one quarter
     *        tone up, -2 for a whole tone down).
     * @param accType Accidental type (e.g., "#", "b").
     * @throws std::runtime_error If semitones is not finite or not a multiple of 0.5; if the
     *         transposed written pitch falls outside the representable range or cannot be spelled
     *         within octaves -1..11 (see Helper::transposePitch()); or, on a transposing
     *         instrument, if setPitch() throws for it: its sounding pitch cannot be spelled. The
     *         note is left unchanged when this throws: a note transposed too low never silently
     *         becomes a rest.
     */
    void transpose(const float semitones, const std::string& accType = MUSIC_XML::ACCIDENT::NONE);

    /**
     * @brief Serializes the note to MusicXML format.
     * @param instrumentId Instrument index (default: 1).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the note.
     */
    const std::string toXML(const size_t instrumentId = 1, const int identSize = 2) const;

    /**
     * @brief Prints detailed information about the note to the log.
     * @details One property per line: whether it is sounding, its pitch -- getPitch(), the written
     *          pitch -- its type, quarter duration, voice, staff, MIDI number (what it sounds),
     *          stem, beams, tuplet, grace-note and in-chord flags, and transposing interval.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1, once it reaches the MIDI number (see getMidiNumber()).
     */
    void info() const;

    // ===== OPERATORS ===== //

    /**
     * @brief Less-than operator for comparing notes by exact pitch position.
     * @details Compares getQuarterToneSteps(), the exact, unrounded sounding position, so a
     *          quarter tone orders correctly instead of being collapsed onto the semitone above
     *          it: `Note("E1b4") < Note("E4")` is true, because 63.5 really is below 64. Identical
     *          to comparing getMidiNumber() for any note without a quarter tone. This is the
     *          single source of truth for pitch order: Chord::sortNotes() and Chord::isSorted()
     *          both resolve through it.
     */
    bool operator<(const Note& otherNote) const;

    /**
     * @brief Greater-than operator for comparing notes by exact pitch position.
     * @details See operator<() for the exactness guarantee.
     */
    bool operator>(const Note& otherNote) const;

    /**
     * @brief Less-than-or-equal operator for comparing notes by exact pitch position.
     * @details See operator<() for the exactness guarantee.
     */
    bool operator<=(const Note& otherNote) const;

    /**
     * @brief Greater-than-or-equal operator for comparing notes by exact pitch position.
     * @details See operator<() for the exactness guarantee.
     */
    bool operator>=(const Note& otherNote) const;

    /**
     * @brief Equality operator: true when both notes are the same pitch, spelled alike.
     * @details A note of a transposing instrument is compared at the pitch it sounds, spelled with
     *          its written letter moved by the diatonic transposing interval, as Chord and Interval
     *          relate it: a B-flat clarinet's written D4 equals a C4, and its written Db4 equals a
     *          Cb4, not a B3. An untransposed note is compared as written, so C#4 and Db4 differ,
     *          as do E1b4 and E4. Duration, voice and every other attribute are ignored. Two equal
     *          notes lie at the same exact position, so neither is ordered before the other
     *          (operator<()).
     * @throws std::runtime_error If either note's sounding pitch lies below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    bool operator==(const Note& otherNote) const;

    /**
     * @brief Inequality operator: the negation of operator==().
     * @throws std::runtime_error If either note's sounding pitch lies below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    bool operator!=(const Note& otherNote) const;
};
