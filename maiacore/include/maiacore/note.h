#pragma once

#include <functional>
#include <string>
#include <vector>

#include "maiacore/constants.h"
#include "maiacore/duration.h"
#include "maiacore/key.h"
#include "maiacore/time-signature.h"

/**
 * @brief Represents a musical note, including pitch, duration, articulation, and MusicXML-related attributes.
 *
 * The Note class provides methods for manipulating and querying musical notes, including pitch and octave handling,
 * duration and rhythm, articulations, ties, beams, transposition, enharmonic equivalents, and MusicXML serialization.
 * Designed for music analysis, computational musicology, and MusicXML processing.
 */
class Note {
   private:
    std::string _writtenPitchClass; ///< Written pitch class (e.g., "C#", "Bb").
    int _writtenOctave; ///< Written octave number.

    std::string _soundingPitchClass; ///< Sounding pitch class (after transposition).
    int _soundingOctave; ///< Sounding octave number (after transposition).

    bool _isNoteOn; ///< True if this is a sounding note, false if rest.
    bool _inChord; ///< True if this note is part of a chord.
    int _midiNumber; ///< MIDI note number.
    int _transposeDiatonic; ///< Diatonic transposition interval.
    int _transposeChromatic; ///< Chromatic transposition interval.
    int _voice; ///< Voice number.
    int _staff; ///< Staff number.
    bool _isGraceNote; ///< True if this is a grace note.
    std::string _stem; ///< Stem direction ("up", "down", etc.).
    bool _isTuplet; ///< True if this note is part of a tuplet.
    bool _isPitched; ///< True if this note is pitched, false for unpitched.
    int _unpitchedIndex; ///< Index for unpitched percussion notes.
    Duration _duration; ///< Duration object for this note.

    std::pair<std::string, std::string> _slur; ///< Slur type and orientation.
    std::string _alterSymbol; ///< Accidental symbol (e.g., "#", "b").
    std::vector<std::string> _tie; ///< Tie types ("start", "stop").
    std::vector<std::string> _articulation; ///< Articulation marks.
    std::vector<std::string> _beam; ///< Beam types.

   public:
    /**
     * @brief Default constructor. Initializes a note as "A4" (MIDI 69).
     */
    Note();

    /**
     * @brief Constructs a Note from a pitch string and rhythm figure.
     * @param pitch Pitch string (e.g., "C4", "G#3").
     * @param rhythmFigure Rhythm figure (default: QUARTER).
     * @param isNoteOn True if sounding note, false for rest.
     * @param inChord True if part of a chord.
     * @param transposeDiatonic Diatonic transposition interval.
     * @param transposeChromatic Chromatic transposition interval.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     */
    explicit Note(const std::string& pitch, const RhythmFigure rhythmFigure = RhythmFigure::QUARTER,
                  bool isNoteOn = true, bool inChord = false, const int transposeDiatonic = 0,
                  const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Constructs a Note from a MIDI number, accidental type, and rhythm figure.
     * @param midiNumber MIDI note number.
     * @param accType Accidental type (e.g., "#", "b").
     * @param rhythmFigure Rhythm figure (default: QUARTER).
     * @param isNoteOn True if sounding note, false for rest.
     * @param inChord True if part of a chord.
     * @param transposeDiatonic Diatonic transposition interval.
     * @param transposeChromatic Chromatic transposition interval.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
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
     * @brief Sets the pitch class (e.g., "C", "D#", "Bb") for the note.
     * @param pitchClass The pitch class string.
     */
    void setPitchClass(const std::string& pitchClass);

    /**
     * @brief Sets the octave for the note.
     * @param octave Octave number.
     */
    void setOctave(int octave);

    /**
     * @brief Sets the duration for the note.
     * @param duration Duration object.
     */
    void setDuration(const Duration& duration);

    /**
     * @brief Sets the duration for the note using quarter note value.
     * @param quarterDuration Duration in quarter notes.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     */
    void setDuration(const float quarterDuration, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Sets whether the note is sounding (note on) or a rest (note off).
     * @param isNoteOn True for sounding note, false for rest.
     */
    void setIsNoteOn(bool isNoteOn);

    /**
     * @brief Sets the pitch (e.g., "C4", "G#3") for the note.
     * @param pitch Pitch string.
     */
    void setPitch(const std::string& pitch);

    /**
     * @brief Sets whether the note is part of a chord.
     * @param inChord True if in chord.
     */
    void setIsInChord(bool inChord);

    /**
     * @brief Sets the transposing interval for the note.
     * @param diatonicInterval Diatonic interval.
     * @param chromaticInterval Chromatic interval.
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
     * @return Sounding pitch class string.
     */
    const std::string getSoundingPitchClass() const;

    /**
     * @brief Returns the full sounding pitch (after transposition).
     * @return Sounding pitch string.
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
     * @return Diatonic sounding pitch class.
     */
    const std::string getDiatonicSoundingPitchClass() const;

    /**
     * @brief Returns the sounding octave (after transposition).
     * @return Sounding octave number.
     */
    int getSoundingOctave() const;

    /**
     * @brief Returns the written octave (as notated).
     * @return Written octave number.
     */
    int getWrittenOctave() const;

    /**
     * @brief Returns the octave (sounding).
     * @return Octave number.
     */
    int getOctave() const;

    /**
     * @brief Returns the pitch class (sounding).
     * @return Pitch class string.
     */
    std::string getPitchClass() const;

    /**
     * @brief Returns the written pitch step (e.g., "C", "D").
     * @return Written pitch step.
     */
    std::string getWrittenPitchStep() const;

    /**
     * @brief Returns the sounding pitch step (e.g., "C", "D").
     * @return Sounding pitch step.
     */
    std::string getSoundingPitchStep() const;

    /**
     * @brief Returns the pitch step (sounding).
     * @return Pitch step string.
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
     * @brief Returns the full pitch string (sounding).
     * @return Pitch string.
     */
    std::string getPitch() const;

    /**
     * @brief Returns the MIDI note number.
     * @return MIDI note number.
     */
    int getMidiNumber() const;

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
     * @brief Returns the accidental symbol for the note (e.g., "#", "b").
     * @return Accidental symbol string.
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
     * @brief Returns an enharmonic equivalent pitch string.
     * @param alternativeEnhamonicPitch If true, returns an alternative enharmonic.
     * @return Enharmonic pitch string.
     */
    std::string getEnharmonicPitch(const bool alternativeEnhamonicPitch = false) const;

    /**
     * @brief Returns all enharmonic pitch strings for the note.
     * @param includeCurrentPitch If true, includes the current pitch.
     * @return Vector of enharmonic pitch strings.
     */
    std::vector<std::string> getEnharmonicPitches(const bool includeCurrentPitch = false) const;

    /**
     * @brief Returns an enharmonic equivalent Note object.
     * @param alternativeEnhamonicPitch If true, returns an alternative enharmonic.
     * @return Enharmonic Note object.
     */
    Note getEnharmonicNote(const bool alternativeEnhamonicPitch = false) const;

    /**
     * @brief Returns all enharmonic Note objects for the note.
     * @param includeCurrentPitch If true, includes the current note.
     * @return Vector of enharmonic Note objects.
     */
    std::vector<Note> getEnharmonicNotes(const bool includeCurrentPitch = false) const;

    /**
     * @brief Returns the scale degree of the note in a given key.
     * @param key Key object for reference.
     * @return Scale degree as integer.
     */
    int getScaleDegree(const Key& key) const;

    /**
     * @brief Converts the note to its enharmonic equivalent.
     * @param alternativeEnhamonicPitch If true, uses alternative enharmonic.
     */
    void toEnharmonicPitch(const bool alternativeEnhamonicPitch = false);

    /**
     * @brief Returns the frequency of the note in Hz.
     * @param equalTemperament If true, uses equal temperament.
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Frequency in Hz.
     */
    float getFrequency(const bool equalTemperament = true, const float freqA4 = 440.0f) const;

    /**
     * @brief Returns the harmonic spectrum of the note (partials and amplitudes).
     * @param numPartials Number of partials.
     * @param amplCallback Optional amplitude callback.
     * @return Pair of vectors: frequencies and amplitudes.
     */
    std::pair<std::vector<float>, std::vector<float>> getHarmonicSpectrum(
        const int numPartials = 6,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr) const;

    /**
     * @brief Transposes the note by a number of semitones and optional accidental type.
     * @param semitones Number of semitones.
     * @param accType Accidental type (e.g., "#", "b").
     */
    void transpose(const int semitones, const std::string& accType = MUSIC_XML::ACCIDENT::NONE);

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
     * @brief Less-than operator for comparing notes by MIDI number.
     */
    bool operator<(const Note& otherNote) const;

    /**
     * @brief Greater-than operator for comparing notes by MIDI number.
     */
    bool operator>(const Note& otherNote) const;

    /**
     * @brief Less-than-or-equal operator for comparing notes by MIDI number.
     */
    bool operator<=(const Note& otherNote) const;

    /**
     * @brief Greater-than-or-equal operator for comparing notes by MIDI number.
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
