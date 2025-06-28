#pragma once
#include <ctype.h>

#include <string>
#include <vector>

#include "maiacore/barline.h"
#include "maiacore/clef.h"
#include "maiacore/constants.h"
#include "maiacore/key.h"
#include "maiacore/time-signature.h"

class Note;
class Barline;
class Fraction;

/**
 * @brief Represents a musical measure (bar) within a score, containing notes, staves, key/time signatures, and barlines.
 *
 * The Measure class provides methods for managing notes, staves, key and time signatures, metronome marks, and barlines.
 * It supports MusicXML serialization and is designed for music analysis, computational musicology, and MusicXML processing.
 */
class Measure {
   private:
    int _number; ///< Measure number (index in the score).
    Key _key; ///< Key signature for the measure.
    std::string _metronomeFigure; ///< Metronome rhythm figure (e.g., "quarter").
    int _metronomeValue; ///< Metronome value (BPM).
    bool _isKeySignatureChanged; ///< True if key signature changes in this measure.
    bool _isTimeSignatureChanged; ///< True if time signature changes in this measure.
    bool _isMetronomeChanged; ///< True if metronome mark changes in this measure.
    bool _isDivisionsPerQuarterNoteChanged; ///< True if rhythmic division changes in this measure.
    int _numStaves; ///< Number of staves in the measure.
    int _divisionsPerQuarterNote; ///< Divisions per quarter note for rhythmic precision.
    TimeSignature _timeSignature; ///< Time signature for the measure.

    std::vector<std::vector<Note>> _note; ///< Notes for each staff in the measure.
    std::vector<Clef> _clef; ///< Clefs for each staff.
    Barline _barlineLeft; ///< Left barline.
    Barline _barlineRight; ///< Right barline.

   public:
    /**
     * @brief Constructs a Measure with a given number of staves and rhythmic division.
     * @param numStaves Number of staves (default: 1).
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     */
    Measure(const int numStaves = 1, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Destructor.
     */
    ~Measure();

    /**
     * @brief Sets the key signature for the measure.
     * @param fifthCircle Number of accidentals in the circle of fifths.
     * @param isMajorMode True for major, false for minor.
     */
    void setKey(int fifthCircle, bool isMajorMode = true);

    /**
     * @brief Returns the Key object for the measure.
     * @return Key object.
     */
    Key getKey() const;

    /**
     * @brief Returns the key name (e.g., "C major", "G minor").
     * @return Key name string.
     */
    std::string getKeyName() const;

    /**
     * @brief Removes all notes and resets measure state.
     */
    void clear();

    /**
     * @brief Sets the measure number.
     * @param measureNumber New measure number.
     */
    void setNumber(const int measureNumber);

    /**
     * @brief Sets the key signature for the measure.
     * @param fifthCircle Number of accidentals.
     * @param isMajorMode True for major, false for minor.
     */
    void setKeySignature(const int fifthCircle, const bool isMajorMode = true);

    /**
     * @brief Sets the time signature for the measure.
     * @param upper Numerator.
     * @param lower Denominator.
     */
    void setTimeSignature(const int upper, const int lower);

    /**
     * @brief Sets the metronome mark (BPM and rhythm figure) for the measure.
     * @param bpm Beats per minute.
     * @param duration Rhythm figure (default: QUARTER).
     */
    void setMetronome(const int bpm, const RhythmFigure duration = RhythmFigure::QUARTER);

    /**
     * @brief Sets the key mode (major or minor) for the measure.
     * @param isMajorKeyMode True for major, false for minor.
     */
    void setKeyMode(const bool isMajorKeyMode);

    /**
     * @brief Sets whether the key signature changes in this measure.
     * @param isKeySignatureChanged True if changed.
     */
    void setIsKeySignatureChanged(bool isKeySignatureChanged = false);

    /**
     * @brief Sets whether the time signature changes in this measure.
     * @param isTimeSignatureChanged True if changed.
     */
    void setIsTimeSignatureChanged(bool isTimeSignatureChanged = false);

    // void setIsClefChanged(bool isClefChanged = false);

    /**
     * @brief Sets whether the metronome mark changes in this measure.
     * @param isMetronomeChanged True if changed.
     */
    void setIsMetronomeChanged(bool isMetronomeChanged = false);

    /**
     * @brief Sets whether the divisions per quarter note changes in this measure.
     * @param isDivisionsPerQuarterNoteChanged True if changed.
     */
    void setIsDivisionsPerQuarterNoteChanged(bool isDivisionsPerQuarterNoteChanged = false);

    /**
     * @brief Sets the number of staves in the measure.
     * @param numStaves New number of staves.
     */
    void setNumStaves(const int numStaves);

    /**
     * @brief Sets the divisions per quarter note for the measure.
     * @param divisionsPerQuarterNote New divisions per quarter note.
     */
    void setDivisionsPerQuarterNote(const int divisionsPerQuarterNote);

    /**
     * @brief Adds a note to a specific staff and position in the measure.
     * @param note Note to add.
     * @param staveId Staff index (default: 0).
     * @param position Position in the staff (-1 for append).
     */
    void addNote(const Note& note, const int staveId = 0, int position = -1);

    /**
     * @brief Adds multiple notes to a specific staff and position in the measure.
     * @param noteVec Vector of notes to add.
     * @param staveId Staff index (default: 0).
     * @param position Position in the staff (-1 for append).
     */
    void addNote(const std::vector<Note>& noteVec, const int staveId = 0, int position = -1);

    /**
     * @brief Adds a note by pitch string to a specific staff and position.
     * @param pitchClass Pitch string (e.g., "C4").
     * @param staveId Staff index (default: 0).
     * @param position Position in the staff (-1 for append).
     */
    void addNote(const std::string& pitchClass, const int staveId = 0, int position = -1);

    /**
     * @brief Adds multiple notes by pitch string to a specific staff and position.
     * @param pitchClassVec Vector of pitch strings.
     * @param staveId Staff index (default: 0).
     * @param position Position in the staff (-1 for append).
     */
    void addNote(const std::vector<std::string>& pitchClassVec, const int staveId = 0,
                 int position = -1);

    /**
     * @brief Removes a note from a specific staff and position.
     * @param noteId Note index.
     * @param staveId Staff index (default: 0).
     */
    void removeNote(const int noteId, const int staveId = 0);

    /**
     * @brief Returns the measure number.
     * @return Measure number.
     */
    int getNumber() const;

    /**
     * @brief Returns the total duration of the measure in quarter notes.
     * @return Duration in quarter notes.
     */
    float getQuarterDuration() const;

    /**
     * @brief Returns the filled duration (sum of note durations) in quarter notes for a staff.
     * @param staveId Staff index (default: 0).
     * @return Filled duration in quarter notes.
     */
    float getFilledQuarterDuration(const int staveId = 0) const;

    /**
     * @brief Returns the free (remaining) duration in quarter notes for a staff.
     * @param staveId Staff index (default: 0).
     * @return Free duration in quarter notes.
     */
    float getFreeQuarterDuration(const int staveId = 0) const;

    /**
     * @brief Returns the duration of the measure as a Fraction object.
     * @return Fraction representing the duration.
     */
    Fraction getFractionDuration() const;

    /**
     * @brief Returns the total duration of the measure in ticks.
     * @return Duration in ticks.
     */
    int getDurationTicks() const;

    /**
     * @brief Returns the filled duration (sum of note durations) in ticks for a staff.
     * @param staveId Staff index (default: 0).
     * @return Filled duration in ticks.
     */
    int getFilledDurationTicks(const int staveId = 0) const;

    /**
     * @brief Returns the free (remaining) duration in ticks for a staff.
     * @param staveId Staff index (default: 0).
     * @return Free duration in ticks.
     */
    int getFreeDurationTicks(const int staveId = 0) const;

    /**
     * @brief Returns the clefs for all staves in the measure.
     * @return Const reference to vector of Clef objects.
     */
    const std::vector<Clef>& getClefs() const;

    /**
     * @brief Returns the clefs for all staves in the measure (modifiable).
     * @return Reference to vector of Clef objects.
     */
    std::vector<Clef>& getClefs();

    /**
     * @brief Returns the clef for a specific staff.
     * @param clefId Clef index (default: 0).
     * @return Const reference to Clef object.
     */
    const Clef& getClef(const int clefId = 0) const;

    /**
     * @brief Returns the clef for a specific staff (modifiable).
     * @param clefId Clef index (default: 0).
     * @return Reference to Clef object.
     */
    Clef& getClef(const int clefId = 0);

    /**
     * @brief Returns the number of staves in the measure.
     * @return Number of staves.
     */
    int getNumStaves() const;

    /**
     * @brief Returns the left barline object.
     * @return Const reference to Barline.
     */
    const Barline& getBarlineLeft() const;

    /**
     * @brief Returns the left barline object (modifiable).
     * @return Reference to Barline.
     */
    Barline& getBarlineLeft();

    /**
     * @brief Returns the right barline object.
     * @return Const reference to Barline.
     */
    const Barline& getBarlineRight() const;

    /**
     * @brief Returns the right barline object (modifiable).
     * @return Reference to Barline.
     */
    Barline& getBarlineRight();

    /**
     * @brief Sets the left barline as a repeat start.
     */
    void setRepeatStart();

    /**
     * @brief Sets the right barline as a repeat end.
     */
    void setRepeatEnd();

    /**
     * @brief Removes the repeat start from the left barline.
     */
    void removeRepeatStart();

    /**
     * @brief Removes the repeat end from the right barline.
     */
    void removeRepeatEnd();

    /**
     * @brief Returns true if any clef changes in this measure.
     * @return True if clef changed.
     */
    bool isClefChanged() const;

    /**
     * @brief Returns true if the time signature changes in this measure.
     * @return True if time signature changed.
     */
    bool timeSignatureChanged() const;

    /**
     * @brief Returns true if the key signature changes in this measure.
     * @return True if key signature changed.
     */
    bool keySignatureChanged() const;

    /**
     * @brief Returns true if the metronome mark changes in this measure.
     * @return True if metronome changed.
     */
    bool metronomeChanged() const;

    /**
     * @brief Returns true if the divisions per quarter note changes in this measure.
     * @return True if divisions changed.
     */
    bool divisionsPerQuarterNoteChanged() const;

    /**
     * @brief Returns true if the key mode is major.
     * @return True if major mode.
     */
    bool isMajorKeyMode() const;

    /**
     * @brief Returns a const reference to a note in a specific staff and position.
     * @param noteId Note index.
     * @param staveId Staff index (default: 0).
     * @return Const reference to Note.
     */
    const Note& getNote(const int noteId, const int staveId = 0) const;

    /**
     * @brief Returns a reference to a note in a specific staff and position.
     * @param noteId Note index.
     * @param staveId Staff index (default: 0).
     * @return Reference to Note.
     */
    Note& getNote(const int noteId, const int staveId = 0);

    /**
     * @brief Returns a const reference to a sounding note (note on) by index and staff.
     * @param noteOnId Note on index.
     * @param staveId Staff index (default: 0).
     * @return Const reference to Note.
     */
    const Note& getNoteOn(const int noteOnId, const int staveId = 0) const;

    /**
     * @brief Returns a reference to a sounding note (note on) by index and staff.
     * @param noteOnId Note on index.
     * @param staveId Staff index (default: 0).
     * @return Reference to Note.
     */
    Note& getNoteOn(const int noteOnId, const int staveId = 0);

    /**
     * @brief Returns a const reference to a rest note (note off) by index and staff.
     * @param noteOffId Note off index.
     * @param staveId Staff index (default: 0).
     * @return Const reference to Note.
     */
    const Note& getNoteOff(const int noteOffId, const int staveId = 0) const;

    /**
     * @brief Returns a reference to a rest note (note off) by index and staff.
     * @param noteOffId Note off index.
     * @param staveId Staff index (default: 0).
     * @return Reference to Note.
     */
    Note& getNoteOff(const int noteOffId, const int staveId = 0);

    /**
     * @brief Returns the number of sounding notes (note on) in the measure (all staves).
     * @return Number of sounding notes.
     */
    int getNumNotesOn() const;

    /**
     * @brief Returns the number of sounding notes (note on) in a specific staff.
     * @param staveId Staff index.
     * @return Number of sounding notes.
     */
    int getNumNotesOn(const int staveId) const;

    /**
     * @brief Returns the number of rest notes (note off) in the measure (all staves).
     * @return Number of rest notes.
     */
    int getNumNotesOff() const;

    /**
     * @brief Returns the number of rest notes (note off) in a specific staff.
     * @param staveId Staff index.
     * @return Number of rest notes.
     */
    int getNumNotesOff(const int staveId) const;

    /**
     * @brief Returns the total number of notes in the measure (all staves).
     * @return Number of notes.
     */
    int getNumNotes() const;

    /**
     * @brief Returns the number of notes in a specific staff.
     * @param staveId Staff index.
     * @return Number of notes in the specified staff.
     */
    int getNumNotes(const int staveId) const;

    /**
     * @brief Returns the divisions per quarter note for the measure.
     * @return Divisions per quarter note (integer).
     */
    int getDivisionsPerQuarterNote() const;

    /**
     * @brief Returns the number of accidentals in the circle of fifths for the key signature.
     * @return Integer representing the fifth circle value.
     */
    int getFifthCircle() const;

    /**
     * @brief Returns the key signature as a string (e.g., "C", "Gm").
     * @return Key signature string.
     */
    const std::string getKeySignature() const;

    /**
     * @brief Returns the TimeSignature object for the measure.
     * @return Const reference to the TimeSignature.
     */
    const TimeSignature& getTimeSignature() const;

    /**
     * @brief Returns the metronome mark as a pair (figure, value).
     * @return Pair of metronome figure (string) and value (BPM).
     */
    std::pair<std::string, int> getMetronome() const;

    /**
     * @brief Prints summary information about the measure to the log.
     * @details Includes measure number, key, time signature, and note count.
     */
    void info() const;

    /**
     * @brief Returns true if the measure contains no notes in any staff.
     * @return True if the measure is empty.
     */
    bool isEmpty() const;

    /**
     * @brief Returns the empty (unfilled) duration in ticks for the measure.
     * @return Number of ticks representing the empty duration.
     */
    int getEmptyDurationTicks() const;

    /**
     * @brief Serializes the measure to MusicXML format.
     * @param instrumentId Instrument index (default: 1).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the measure.
     */
    const std::string toXML(const int instrumentId = 1, const int identSize = 2) const;

    /**
     * @brief Serializes the measure to JSON format.
     * @return JSON string for the measure.
     */
    const std::string toJSON() const;
};
