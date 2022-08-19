#pragma once
#include <iostream>
#include <ctype.h>

#include <vector>

#include "note.h"
#include "clef.h"
#include "barline.h"
#include "helper.h"

class Measure
{

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
    Measure(const int numStaves = 1, const int divisionsPerQuarterNote = 256);
    ~Measure();

    void clear();

    void setNumber(const int measureNumber);
    void setKeySignature(const int fifthCicle, const bool isMajorMode = true); // overload with measure
    void setTimeSignature(const int upper, const int lower); // overload with measure
    void setMetronome(const int bpm, const Duration duration = Duration::QUARTER); // overload with measure

    void setKeyMode(const bool isMajorKeyMode);
    void setIsKeySignatureChanged(bool isKeySignatureChanged = false);
    void setIsTimeSignatureChanged(bool isTimeSignatureChanged = false);
    void setIsClefChanged(bool isClefChanged = false);
    void setIsMetronomeChanged(bool isMetronomeChanged = false);
    void setIsDivisionsPerQuarterNoteChanged(bool isDivisionsPerQuarterNoteChanged = false);
    void setNumStaves(const int numStaves);
    void setDivisionsPerQuarterNote(const int divisionsPerQuarterNote);

    void addNote(const Note& note, const int staveId = 0, int position = -1);
    void addNote(const std::vector<Note>& noteVec, const int staveId = 0, int position = -1);
    void addNote(const std::string& pitchClass, const int staveId = 0, int position = -1);
    void addNote(const std::vector<std::string>& pitchClassVec, const int staveId = 0, int position = -1);

    void removeNote(const int noteId, const int staveId = 0);

    int getNumber() const;
    const Clef& getClef(const int clefId = 0) const;
    Clef& getClef(const int clefId = 0);
    int getNumStaves() const;

    const Barline& getBarlineLeft() const;
    Barline& getBarlineLeft();

    const Barline& getBarlineRight() const;
    Barline& getBarlineRight();

    void setRepeatStart();
    void setRepeatEnd();

    void removeRepeatStart();
    void removeRepeatEnd();

    bool clefChanged() const;
    bool timeSignatureChanged() const;
    bool keySignatureChanged() const;
    bool metronomeChanged() const;
    bool divisionsPerQuarterNoteChanged() const;
    bool isMajorKeyMode() const;

    const Note& getNote(const int noteId, const int staveId = 0) const;
    Note& getNote(const int noteId, const int staveId = 0);

    const Note& getNoteOn(const int noteOnId, const int staveId = 0) const;
    Note& getNoteOn(const int noteOnId, const int staveId = 0);

    const Note& getNoteOff(const int noteOffId, const int staveId = 0) const;
    Note& getNoteOff(const int noteOffId, const int staveId = 0);

    int getNumNotesOn(const int staveId) const;
    int getNumNotesOn() const;

    int getNumNotesOff(const int staveId) const;
    int getNumNotesOff() const;

    int getNumNotes(const int staveId) const;
    int getNumNotes() const;

    int getDivisionsPerQuarterNote() const;

    int getFifthCicle() const;
    const std::string getKeySignature() const;
    std::pair<int, int> getTimeSignature() const;
    std::pair<std::string, int> getMetronome() const;

    void info() const;
    bool empty() const;
    int getEmptyDurationTicks() const;

    const std::string toXML(const int instrumentId = 1, const int identSize = 2) const;
    const std::string toJSON() const;
};
