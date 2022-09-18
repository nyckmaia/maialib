#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <utility> // std::pair
#include <tuple> // std::tuple
#include <algorithm> // std::rotate

#include "note.h"
#include "interval.h"
#include "constants.h"

struct NoteData 
{
    Note note = Note("rest");
    bool wasEnharmonized = false;
    int enharmonicDiatonicDistance = 0;

    NoteData() : 
        note(Note("rest")),
        wasEnharmonized(false),
        enharmonicDiatonicDistance(0)
    {
    }

    NoteData(const Note& _note, const bool _wasEnhar, const int _enharDiat) :
        note(_note),
        wasEnharmonized(_wasEnhar),
        enharmonicDiatonicDistance(_enharDiat)
    {
    };

    friend bool operator<(const NoteData& lhs, const NoteData& rhs) { 
        return lhs.note.getMIDINumber() < rhs.note.getMIDINumber(); 
    }
};

// Heap Type [Vector of NotesData]
typedef std::vector<NoteData> Heap;
// HeapData Type [Heap, stackMatchValue]
typedef std::tuple<Heap, float> HeapData;

void printHeap(const Heap& heap);

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
    void stackInThirds(const bool enharmonyNotes = false);
    HeapData stackInThirdsTemplateMatch(const Heap& heap) const;
    std::vector<Heap> computeEnharmonicUnitsGroups() const;
    std::vector<Heap> computeEnharmonicHeaps(const std::vector<Heap>& heaps) const;
    std::vector<Heap> computeAllHeapInversions(Heap& heap) const;
    std::vector<Heap> filterTertianHeapsOnly(const std::vector<Heap>& heaps) const;

public:

    Chord();
    explicit Chord(const std::vector<Note>& notes);
    explicit Chord(const std::vector<std::string>& pitches);

    ~Chord();

    void clear();
    void addNote(const Note& note);
    void addNote(const std::string& pitch);
    void removeTopNote();
    void insertNote(Note& insertNote, int positionNote = 0);
    void removeNote(int noteIndex);
    void setDurationTicks(const int durationTicks);
    void inversion(int inversionNumber);
    void transpose(const int semiTonesNumber);
    void transposeStackOnly(const int semiTonesNumber);
    void removeDuplicateNotes();

    std::string getDuration() const;
    float getQuarterDuration() const;
    int getDurationTicks() const;
    Note& getNote(int noteIndex);
    const Note& getNote(const int noteIndex) const;
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
    std::vector<Note> getStackedNotes() const;
    std::vector<Interval> getStackIntervals(const bool fromRoot = false);

    size_t size() const;
    size_t stackSize() const;

    void print() const;
    void printStack() const;
    void info();

    Chord getStackedChord(const bool enharmonyNotes = false);

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

    void operator << (const Note& note) {
        addNote(note);
    }

    Note operator >> (Note& note) {
        note = _note[_note.size()-1];

        removeTopNote();

        return note;
    }
};
