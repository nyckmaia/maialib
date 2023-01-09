#pragma once
#include <ctype.h>

#include <string>
#include <vector>

#include "maiacore/barline.h"
#include "maiacore/clef.h"
#include "maiacore/constants.h"

class Note;
class Barline;

class Measure {
   private:
    int _number;
    int _timeSignatureUpper;
    int _timeSignatureLower;
    int _fifthCicle;
    std::string _keySignature;
    std::string _metronomeFigure;
    int _metronomeValue;
    bool _isMajorKeyMode;
    bool _isKeySignatureChanged;
    bool _isTimeSignatureChanged;
    bool _isClefChanged;
    bool _isMetronomeChanged;
    bool _isDivisionsPerQuarterNoteChanged;
    int _numStaves;
    int _divisionsPerQuarterNote;

    std::vector<std::vector<Note>> _note;
    std::vector<Clef> _clef;
    Barline _barlineLeft;
    Barline _barlineRight;

   public:
    /**
     * @brief Construct a new Measure object
     *
     * @param numStaves
     * @param divisionsPerQuarterNote
     */
    Measure(const int numStaves = 1, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Destroy the Measure object
     *
     */
    ~Measure();

    /**
     * @brief
     *
     */
    void clear();

    /**
     * @brief Set the Number object
     *
     * @param measureNumber
     */
    void setNumber(const int measureNumber);

    /**
     * @brief Set the Key Signature object
     *
     * @param fifthCicle
     * @param isMajorMode
     */
    void setKeySignature(const int fifthCicle, const bool isMajorMode = true);

    /**
     * @brief Set the Time Signature object
     *
     * @param upper
     * @param lower
     */
    void setTimeSignature(const int upper, const int lower);

    /**
     * @brief Set the Metronome object
     *
     * @param bpm
     * @param duration
     */
    void setMetronome(const int bpm, const Duration duration = Duration::QUARTER);

    /**
     * @brief Set the Key Mode object
     *
     * @param isMajorKeyMode
     */
    void setKeyMode(const bool isMajorKeyMode);

    /**
     * @brief Set the Is Key Signature Changed object
     *
     * @param isKeySignatureChanged
     */
    void setIsKeySignatureChanged(bool isKeySignatureChanged = false);

    /**
     * @brief Set the Is Time Signature Changed object
     *
     * @param isTimeSignatureChanged
     */
    void setIsTimeSignatureChanged(bool isTimeSignatureChanged = false);

    /**
     * @brief Set the Is Clef Changed object
     *
     * @param isClefChanged
     */
    void setIsClefChanged(bool isClefChanged = false);

    /**
     * @brief Set the Is Metronome Changed object
     *
     * @param isMetronomeChanged
     */
    void setIsMetronomeChanged(bool isMetronomeChanged = false);

    /**
     * @brief Set the Is Divisions Per Quarter Note Changed object
     *
     * @param isDivisionsPerQuarterNoteChanged
     */
    void setIsDivisionsPerQuarterNoteChanged(bool isDivisionsPerQuarterNoteChanged = false);

    /**
     * @brief Set the Num Staves object
     *
     * @param numStaves
     */
    void setNumStaves(const int numStaves);

    /**
     * @brief Set the Divisions Per Quarter Note object
     *
     * @param divisionsPerQuarterNote
     */
    void setDivisionsPerQuarterNote(const int divisionsPerQuarterNote);

    /**
     * @brief
     *
     * @param note
     * @param staveId
     * @param position
     */
    void addNote(const Note& note, const int staveId = 0, int position = -1);

    /**
     * @brief
     *
     * @param noteVec
     * @param staveId
     * @param position
     */
    void addNote(const std::vector<Note>& noteVec, const int staveId = 0, int position = -1);

    /**
     * @brief
     *
     * @param pitchClass
     * @param staveId
     * @param position
     */
    void addNote(const std::string& pitchClass, const int staveId = 0, int position = -1);

    /**
     * @brief
     *
     * @param pitchClassVec
     * @param staveId
     * @param position
     */
    void addNote(const std::vector<std::string>& pitchClassVec, const int staveId = 0,
                 int position = -1);

    /**
     * @brief
     *
     * @param noteId
     * @param staveId
     */
    void removeNote(const int noteId, const int staveId = 0);

    /**
     * @brief Get the Number object
     *
     * @return int
     */
    int getNumber() const;

    /**
     * @brief Get the Clef object
     *
     * @param clefId
     * @return const Clef&
     */
    const Clef& getClef(const int clefId = 0) const;

    /**
     * @brief Get the Clef object
     *
     * @param clefId
     * @return Clef&
     */
    Clef& getClef(const int clefId = 0);

    /**
     * @brief Get the Num Staves object
     *
     * @return int
     */
    int getNumStaves() const;

    /**
     * @brief Get the Barline Left object
     *
     * @return const Barline&
     */
    const Barline& getBarlineLeft() const;

    /**
     * @brief Get the Barline Left object
     *
     * @return Barline&
     */
    Barline& getBarlineLeft();

    /**
     * @brief Get the Barline Right object
     *
     * @return const Barline&
     */
    const Barline& getBarlineRight() const;

    /**
     * @brief Get the Barline Right object
     *
     * @return Barline&
     */
    Barline& getBarlineRight();

    /**
     * @brief Set the Repeat Start object
     *
     */
    void setRepeatStart();

    /**
     * @brief Set the Repeat End object
     *
     */
    void setRepeatEnd();

    /**
     * @brief
     *
     */
    void removeRepeatStart();

    /**
     * @brief
     *
     */
    void removeRepeatEnd();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool clefChanged() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool timeSignatureChanged() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool keySignatureChanged() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool metronomeChanged() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool divisionsPerQuarterNoteChanged() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isMajorKeyMode() const;

    /**
     * @brief Get the Note object
     *
     * @param noteId
     * @param staveId
     * @return const Note&
     */
    const Note& getNote(const int noteId, const int staveId = 0) const;

    /**
     * @brief Get the Note object
     *
     * @param noteId
     * @param staveId
     * @return Note&
     */
    Note& getNote(const int noteId, const int staveId = 0);

    /**
     * @brief Get the Note On object
     *
     * @param noteOnId
     * @param staveId
     * @return const Note&
     */
    const Note& getNoteOn(const int noteOnId, const int staveId = 0) const;

    /**
     * @brief Get the Note On object
     *
     * @param noteOnId
     * @param staveId
     * @return Note&
     */
    Note& getNoteOn(const int noteOnId, const int staveId = 0);

    /**
     * @brief Get the Note Off object
     *
     * @param noteOffId
     * @param staveId
     * @return const Note&
     */
    const Note& getNoteOff(const int noteOffId, const int staveId = 0) const;

    /**
     * @brief Get the Note Off object
     *
     * @param noteOffId
     * @param staveId
     * @return Note&
     */
    Note& getNoteOff(const int noteOffId, const int staveId = 0);

    /**
     * @brief Get the Num Notes On object
     *
     * @param staveId
     * @return int
     */
    int getNumNotesOn(const int staveId) const;

    /**
     * @brief Get the Num Notes On object
     *
     * @return int
     */
    int getNumNotesOn() const;

    /**
     * @brief Get the Num Notes Off object
     *
     * @param staveId
     * @return int
     */
    int getNumNotesOff(const int staveId) const;

    /**
     * @brief Get the Num Notes Off object
     *
     * @return int
     */
    int getNumNotesOff() const;

    /**
     * @brief Get the Num Notes object
     *
     * @param staveId
     * @return int
     */
    int getNumNotes(const int staveId) const;

    /**
     * @brief Get the Num Notes object
     *
     * @return int
     */
    int getNumNotes() const;

    /**
     * @brief Get the Divisions Per Quarter Note object
     *
     * @return int
     */
    int getDivisionsPerQuarterNote() const;

    /**
     * @brief Get the Fifth Cicle object
     *
     * @return int
     */
    int getFifthCicle() const;

    /**
     * @brief Get the Key Signature object
     *
     * @return const std::string
     */
    const std::string getKeySignature() const;

    /**
     * @brief Get the Time Signature object
     *
     * @return std::pair<int, int>
     */
    std::pair<int, int> getTimeSignature() const;

    /**
     * @brief Get the Metronome object
     *
     * @return std::pair<std::string, int>
     */
    std::pair<std::string, int> getMetronome() const;

    /**
     * @brief
     *
     */
    void info() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool empty() const;

    /**
     * @brief Get the Empty Duration Ticks object
     *
     * @return int
     */
    int getEmptyDurationTicks() const;

    /**
     * @brief
     *
     * @param instrumentId
     * @param identSize
     * @return const std::string
     */
    const std::string toXML(const int instrumentId = 1, const int identSize = 2) const;

    /**
     * @brief
     *
     * @return const std::string
     */
    const std::string toJSON() const;
};
