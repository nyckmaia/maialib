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

/**
 * @brief Represents a musical note, including pitch, duration, articulation, and MusicXML-related
 * attributes.
 *
 * The Note class provides methods for manipulating and querying musical notes, including pitch and
 * octave handling, duration and rhythm, articulations, ties, beams, transposition, enharmonic
 * equivalents, and MusicXML serialization. Designed for music analysis, computational musicology,
 * and MusicXML processing.
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
     * @brief Computes the sounding Pitch: the written pitch transposed by
     *        _transposeDiatonic/_transposeChromatic.
     * @details A rest, or an untransposed note, returns _writtenPitch itself: nothing is cached,
     *          every "sounding" getter (getSoundingPitch(), getOctave(), getAlterSymbol(),
     *          getPitchClass(), ...) calls this on demand. setTransposingInterval() and
     *          setPitch() derive it for the new interval or written pitch before storing it, so a
     *          sounding pitch that cannot be spelled throws there, with the note unchanged, rather
     *          than from a later getter call -- except one below the lowest representable pitch,
     *          which leaves the note constructible.
     * @return The sounding Pitch.
     * @throws std::runtime_error If this note is transposed and its sounding pitch lies below the
     *         lowest representable pitch, C1b-1 (see getMidiNumber()), or above B11 (MIDI note
     *         155), the highest sounding pitch that can be spelled within octaves -1..11.
     */
    Pitch computeSoundingPitch() const;

   public:
    /**
     * @brief Default constructor. Initializes a note as "A4" (MIDI 69).
     */
    Note();

    /**
     * @brief Constructs a Note from a pitch string and rhythm figure.
     * @param pitch Pitch string (e.g., "C4", "G#3", "Dbb-1", "Bx11"). Accidentals: "bb", "b",
     *        "#", "x"; octaves -1..11 (a missing octave defaults to 4); the MIDI number must be
     *        >= 0. An empty string or a string containing "rest" creates a rest.
     * @param rhythmFigure Rhythm figure (default: QUARTER).
     * @param isNoteOn True if sounding note, false for rest.
     * @param inChord True if part of a chord.
     * @param transposeDiatonic Diatonic transposition interval.
     * @param transposeChromatic Chromatic transposition interval.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     * @throws std::runtime_error If the pitch string is invalid (see Helper::splitPitch()), or if
     *         the note is transposed and its sounding pitch lies above B11 (MIDI note 155), the
     *         highest sounding pitch that can be spelled within octaves -1..11 (see
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
     *         octaves -1..11, or if the note is transposed and its sounding pitch lies above B11
     *         (MIDI note 155), the highest sounding pitch that can be spelled within octaves
     *         -1..11 (see setTransposingInterval()).
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
     *          pitch: a sounding pitch above B11 (MIDI note 155), the highest sounding pitch that
     *          can be spelled within octaves -1..11, throws with the note unchanged. A sounding
     *          pitch below the lowest representable pitch, C1b-1, is accepted, as setPitch()
     *          accepts it.
     * @param pitchClass The pitch class string.
     * @throws std::runtime_error If the pitch class is invalid or the pitch lies below MIDI note 0
     *         (see Helper::splitPitch()), or if the sounding pitch the change gives lies above B11
     *         (the same error setPitch() raises for that pitch). The note is then left unchanged.
     */
    void setPitchClass(const std::string& pitchClass);

    /**
     * @brief Sets the octave of the written pitch, keeping its step and accidental.
     * @details Delegates to Pitch::setOctave() and inherits its policy: refuses on a rest, and
     *          when the pitch would fall below MIDI note 0 (LOG_WARN, no change).
     *
     *          The change is made on a copy of the written pitch, whose sounding pitch is checked
     *          with the transposing interval before it is stored, as setPitch() checks a whole
     *          pitch: a sounding pitch above B11 (MIDI note 155), the highest sounding pitch that
     *          can be spelled within octaves -1..11, throws with the note unchanged. A sounding
     *          pitch below the lowest representable pitch, C1b-1, is accepted, as setPitch()
     *          accepts it.
     * @param octave Octave number, within [-1, 11].
     * @throws std::runtime_error If octave is outside [-1, 11] on a note, or if the sounding pitch
     *         the change gives lies above B11 (the same error setPitch() raises for that pitch).
     *         The note is then left unchanged.
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
     *          pitch: a sounding pitch above B11 (MIDI note 155), the highest sounding pitch that
     *          can be spelled within octaves -1..11, throws with the note unchanged. A sounding
     *          pitch below the lowest representable pitch, C1b-1, is accepted, as setPitch()
     *          accepts it.
     * @param step Diatonic step ("A".."G").
     * @throws std::runtime_error If step is not one of "A".."G", or if the sounding pitch the
     *         change gives lies above B11 (the same error setPitch() raises for that pitch). The
     *         note is then left unchanged.
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
     *          pitch: a sounding pitch above B11 (MIDI note 155), the highest sounding pitch that
     *          can be spelled within octaves -1..11, throws with the note unchanged. A sounding
     *          pitch below the lowest representable pitch, C1b-1, is accepted, as setPitch()
     *          accepts it.
     * @param alter Alter value; must be a multiple of 0.5 (a semitone or quarter-tone step),
     *        within [-2, 2].
     * @throws std::runtime_error If alter is NaN or infinite, is not a multiple of 0.5, or is
     *         outside [-2, 2], or if the sounding pitch the change gives lies above B11 (the same
     *         error setPitch() raises for that pitch). The note is then left unchanged.
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
     *         its sounding pitch with the current transposing interval lies above B11 (MIDI note
     *         155), the highest sounding pitch that can be spelled within octaves -1..11. The note
     *         is then left unchanged. A sounding pitch below the lowest representable pitch, C1b-1,
     *         is accepted, as setTransposingInterval() accepts it.
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
     *          interval; every sounding getter derives the sounding pitch on demand, so a second
     *          call replaces the interval rather than adding to it. The sounding pitch is derived
     *          here too, before the interval is stored, so one that cannot be spelled throws now,
     *          with the note unchanged, rather than from a later getter call. A sounding pitch
     *          below the lowest representable pitch, C1b-1, is accepted: the note stays
     *          constructible, and each sounding getter reports the condition when asked. A rest
     *          ignores the call.
     * @param diatonicInterval Diatonic interval.
     * @param chromaticInterval Chromatic interval.
     * @throws std::runtime_error If the sounding pitch with this interval lies above B11 (MIDI note
     *         155), the highest sounding pitch that can be spelled within octaves -1..11; the note
     *         is then left unchanged.
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
     * @brief Returns the sounding pitch class (after transposition).
     * @return Sounding pitch class string, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    const std::string getSoundingPitchClass() const;

    /**
     * @brief Returns the full sounding pitch (after transposition).
     * @return Sounding pitch string, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    const std::string getSoundingPitch() const;

    /**
     * @brief Returns the written pitch class (as notated).
     * @return Written pitch class string.
     */
    const std::string getWrittenPitchClass() const;

    /**
     * @brief Returns the full written pitch (as notated).
     * @return Written pitch string.
     */
    const std::string getWrittenPitch() const;

    /**
     * @brief Returns the diatonic written pitch class (e.g., "C", "D").
     * @return Diatonic written pitch class.
     */
    const std::string getDiatonicWrittenPitchClass() const;

    /**
     * @brief Returns the diatonic sounding pitch class (e.g., "C", "D").
     * @return Diatonic sounding pitch class, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    const std::string getDiatonicSoundingPitchClass() const;

    /**
     * @brief Returns the sounding octave (after transposition).
     * @details Arithmetic, from getMidiNumber(). An empty optional means this note is a rest,
     *          which has no octave; isNoteOff() is the authoritative test.
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
     * @brief Returns the octave (sounding).
     * @details The octave of the sounding spelling, which always agrees with
     *          getSoundingOctave(). An empty optional means this note is a rest, which has no
     *          octave; isNoteOff() is the authoritative test.
     * @return Octave number, or an empty optional for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::optional<int> getOctave() const;

    /**
     * @brief Returns the pitch class (sounding).
     * @return Pitch class string, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::string getPitchClass() const;

    /**
     * @brief Returns the written pitch step (e.g., "C", "D").
     * @return Written pitch step.
     */
    std::string getWrittenPitchStep() const;

    /**
     * @brief Returns the sounding pitch step (e.g., "C", "D").
     * @return Sounding pitch step, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::string getSoundingPitchStep() const;

    /**
     * @brief Returns the pitch step (sounding).
     * @return Pitch step string, or "rest" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
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
     * @brief Returns the full pitch string (sounding).
     * @return Pitch string.
     * @throws std::runtime_error See getSoundingPitch(), which this delegates to.
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
     *          has no sounding spelling, octave, MIDI number or frequency, so this method and
     *          every other sounding getter throw the same error for it rather than answer a rest's
     *          values. Its written pitch stays available through the written getters.
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
     * @brief Returns the accidental symbol of the sounding pitch (e.g., "#", "b").
     * @return Accidental symbol string, or "" for a rest.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
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
     * @brief Returns an enharmonic spelling of the sounding pitch, getPitch().
     * @details A semitone pitch is respelled among the other spellings of the same MIDI number,
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
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::string getEnharmonicPitch(const bool alternativeEnharmonicPitch = false) const;

    /**
     * @brief Returns the default and alternative enharmonic spellings of the sounding pitch.
     * @details {getEnharmonicPitch(false), getEnharmonicPitch(true)}, preceded by getPitch() if
     *          includeCurrentPitch is true, so the vector may contain duplicates (e.g., "G#4" ->
     *          {"G#4", "Ab4", "Ab4"}, "C3x4" -> {"C3x4", "D1b4", "D1b4"}).
     * @param includeCurrentPitch If true, includes the current pitch.
     * @return Vector of enharmonic pitch strings.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::vector<std::string> getEnharmonicPitches(const bool includeCurrentPitch = false) const;

    /**
     * @brief Returns a new Note spelled with getEnharmonicPitch(alternativeEnharmonicPitch).
     * @details The new Note holds that spelling as its written pitch, with no transposing
     *          interval and the default rhythm figure.
     * @param alternativeEnharmonicPitch If true, uses the alternative enharmonic.
     * @return Enharmonic Note object.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    Note getEnharmonicNote(const bool alternativeEnharmonicPitch = false) const;

    /**
     * @brief Returns new Notes spelled with the strings getEnharmonicPitches() returns.
     * @details Each new Note holds its spelling as its written pitch, with no transposing
     *          interval and the default rhythm figure; entries may repeat.
     * @param includeCurrentPitch If true, includes a Note spelled with the current pitch.
     * @return Vector of enharmonic Note objects.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()).
     */
    std::vector<Note> getEnharmonicNotes(const bool includeCurrentPitch = false) const;

    /**
     * @brief Returns the scale degree of the note in a given key.
     * @param key Key object for reference.
     * @return Scale degree as integer.
     */
    int getScaleDegree(const Key& key) const;

    /**
     * @brief Respells the note with getEnharmonicPitch(alternativeEnharmonicPitch), through
     *        setPitch().
     * @details setPitch() sets the written pitch. On a transposing instrument the respelling is
     *          of the sounding pitch, so the note then sounds that respelling moved by the
     *          transposing interval again: a B-flat clarinet's written C#4 (sounding B3) becomes
     *          a written Cb4, sounding A3.
     * @param alternativeEnharmonicPitch If true, uses alternative enharmonic.
     * @throws std::runtime_error If this note's sounding pitch falls below the lowest
     *         representable pitch, C1b-1 (see getMidiNumber()), or if setPitch() throws for the
     *         respelling -- on a transposing instrument, when the respelling moved by the
     *         transposing interval cannot be spelled. The note is then left unchanged.
     */
    void toEnharmonicPitch(const bool alternativeEnharmonicPitch = false);

    /**
     * @brief Returns the frequency of the note in Hz.
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
     * @brief Transposes the note by a number of semitones and optional accidental type.
     * @details Transposes on exact pitch positions, so a quarter tone survives: transposing
     *          "C4" by 0.5 gives "C1x4", and transposing "C1x4" by 2 gives "D1x4".
     * @param semitones Number of semitones; must be a multiple of 0.5 (e.g. 0.5 for one quarter
     *        tone up, -2 for a whole tone down).
     * @param accType Accidental type (e.g., "#", "b").
     * @throws std::runtime_error If semitones is not finite or not a multiple of 0.5; if the
     *         transposed pitch falls outside the representable range or cannot be spelled within
     *         octaves -1..11 (see Helper::transposePitch()); or, on a transposing instrument, if
     *         setPitch() throws for the result, which it stores as the written pitch. The note is
     *         left unchanged when this throws: a note transposed too low never silently becomes a
     *         rest.
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
     * @brief Equality operator for comparing notes by pitch.
     */
    bool operator==(const Note& otherNote) const;

    /**
     * @brief Inequality operator for comparing notes by pitch.
     */
    bool operator!=(const Note& otherNote) const;
};
