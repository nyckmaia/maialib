#pragma once
#include <iostream>
#include <vector>
#include <functional>

#include "note.h"
#include "interval.h"
#include "constants.h"

class Chord
{

private:
    std::vector<Note> _note;
    std::vector<Note> _stack;

    Note _bassNote;
    bool _isStackedInThirds;
    std::vector<int> _midiInterval;

    bool _haveMinorThird;
    bool _haveMajorThird;
    bool _haveDiminishedFifth;
    bool _havePerfectFifth;
    bool _haveAugmentedFifth;
    bool _haveDiminishedSeventh;
    bool _haveMinorSeventh;
    bool _haveMajorSeventh;
    bool _haveMinorNinth;
    bool _haveMajorNinth;
    bool _havePerfectEleventh;
    bool _haveSharpEleventh;
    bool _haveMinorThirdteenth;
    bool _haveMajorThirdteenth;

    void computeIntervals();
    std::string enharmonicName();
    std::string nonEnharmonicName() const;
    void stackInThirds();
    
public:

    Chord();
    explicit Chord(const std::vector<Note>& notes);
    explicit Chord(const std::vector<std::string>& pitches);

    ~Chord();

    void clear();
    void addNote(const Note& note);
    void addNote(const std::string& pitch);
    void removeTopNote();
    void insertNote(Note& insertNote, size_t positionNote = 0);
    void removeNote(size_t noteIndex);
    void setDurationTicks(const int durationTicks);
    void inversion(size_t inversionNumber);
    void transpose(const int semiTonesNumber);
    void transposeStackOnly(const int semiTonesNumber);
    void removeDuplicateNotes();

    std::string getDuration() const;
    float getQuarterDuration() const;
    int getDurationTicks() const;
    Note& getNote(size_t noteIndex);
    const Note& getNote(const size_t noteIndex) const;
    const Note& getRoot();
    std::string getName(const bool useEnharmonicName = true);
    const Note& getBassNote();
    const std::vector<Note>& getNotes() const;

    bool haveMinorThird();
    bool haveMajorThird();
    bool haveDiminishedFifth();
    bool havePerfectFifth();
    bool haveAugmentedFifth();
    bool haveDiminishedSeventh();
    bool haveMinorSeventh();
    bool haveMajorSeventh();
    bool haveMinorNinth();
    bool haveMajorNinth();
    bool havePerfectEleventh();
    bool haveSharpEleventh();
    bool haveMinorThirdteenth();
    bool haveMajorThirdteenth();

    bool isMajorChord();
    bool isMinorChord();
    bool isAugmentedChord();
    bool isDiminishedChord();
    bool isDominantSeventhChord();
    bool isHalfDiminishedChord();

    bool isTonal(std::function<bool(const Chord& chord)> model = nullptr);

    std::vector<int> getMIDIIntervals();
    std::vector<Interval> getIntervals(const bool fromRoot = false) const;
    std::vector<Interval> getStackIntervals(const bool fromRoot = false);

    size_t size() const;
    size_t stackSize() const;

    void print() const;
    void printStack() const;
    void info();

    Chord getStackedChord() const;

    void sortNotes();

    const Note& operator[](size_t index) const {
        return _note.at(index);
    }

    Note& operator[](size_t index) {
        return _note.at(index);
    }

    bool operator == (const Chord& otherChord) const {
        size_t sizeA = this->size();
        size_t sizeB = otherChord.size();

        if (sizeA != sizeB) { return false; }

        for (size_t i = 0; i < sizeA; i++) {
            if (_note[i] != otherChord.getNote(i)) {
                return false;
            }
        }

        return true;
    }

    bool operator != (const Chord& otherChord) const {
        size_t sizeA = this->size();
        size_t sizeB = otherChord.size();

        if (sizeA != sizeB) { return true; }

        for (size_t i = 0; i < sizeA; i++) {
            if (_note[i] != otherChord.getNote(i)) {
                return true;
            }
        }

        return false;
    }

    Chord operator + (const Chord& otherChord) const {
        Chord x = *this;

        const size_t sizeChord = otherChord.size();
        for (size_t i = 0; i < sizeChord; i++) {
            x.addNote(otherChord[i]);
        }

        return x;
    }

    void operator << (Note& note) {
        addNote(note);
    }

    Note operator >> (Note& note) {
        note = _note[_note.size()-1];

        removeTopNote();

        return note;
    }
};
