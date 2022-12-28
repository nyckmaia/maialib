#pragma once
#include <algorithm>  // std::rotate std::count
#include <functional>
#include <iostream>
#include <set>      // std::set
#include <tuple>    // std::tuple
#include <utility>  // std::pair
#include <vector>

#include "constants.h"
#include "interval.h"
#include "note.h"

struct NoteData {
    Note note = Note("rest");
    bool wasEnharmonized = false;
    int enharmonicDiatonicDistance = 0;

    NoteData() : note(Note("rest")), wasEnharmonized(false), enharmonicDiatonicDistance(0) {}

    NoteData(const Note& _originalNotes, const bool _wasEnhar, const int _enharDiat)
        : note(_originalNotes),
          wasEnharmonized(_wasEnhar),
          enharmonicDiatonicDistance(_enharDiat){};

    friend bool operator<(const NoteData& lhs, const NoteData& rhs) {
        return lhs.note.getMIDINumber() < rhs.note.getMIDINumber();
    }
};

// NoteDataHeap Type [Vector of NotesData]
typedef std::vector<NoteData> NoteDataHeap;
// HeapData Type [NoteDataHeap, stackMatchValue]
typedef std::tuple<NoteDataHeap, float> HeapData;

bool operator<(const HeapData& a, const HeapData& b);

void printHeap(const NoteDataHeap& heap);

void sortHeapOctaves(NoteDataHeap* heap);

class Chord {
   private:
    // original chord notes. No enharmony applied
    std::vector<Note> _originalNotes;

    // enharmonic notes stacked in thirds in open position
    std::vector<Note> _openStack;

    // enharmonic notes stacked in thirds in closed position
    std::vector<Note> _closeStack;

    // all enharmonic stacks in open position
    std::vector<HeapData> _stackedHeaps;
    std::vector<Interval> _closeStackintervals;

    Note _bassNote;
    bool _isStackedInThirds;

    void computeIntervals();
    void stackInThirds(const bool enharmonyNotes = false);
    HeapData stackInThirdsTemplateMatch(const NoteDataHeap& heap) const;
    std::vector<NoteDataHeap> computeEnharmonicUnitsGroups() const;
    std::vector<NoteDataHeap> computeEnharmonicHeaps(const std::vector<NoteDataHeap>& heaps) const;
    std::vector<NoteDataHeap> removeHeapsWithDuplicatedPitchSteps(
        std::vector<NoteDataHeap>& heaps) const;
    std::vector<NoteDataHeap> computeAllHeapInversions(NoteDataHeap& heap) const;
    std::vector<NoteDataHeap> filterTertianHeapsOnly(const std::vector<NoteDataHeap>& heaps) const;
    std::vector<Note> computeBestOpenStackHeap(std::vector<HeapData>& stackedHeaps);
    void computeCloseStack(const std::vector<Note>& openStack);

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

    std::vector<HeapData> getStackedHeaps(const bool enharmonyNotes = false);

    std::string getDuration() const;
    float getQuarterDuration() const;
    int getDurationTicks() const;
    Note& getNote(int noteIndex);
    const Note& getNote(const int noteIndex) const;
    const Note& getRoot();
    std::string getName();
    const Note& getBassNote();
    const std::vector<Note>& getNotes() const;

    bool haveMajorInterval(const bool useEnharmony = false) const;
    bool haveMinorInterval(const bool useEnharmony = false) const;
    bool havePerfectInterval(const bool useEnharmony = false) const;
    bool haveDiminishedInterval(const bool useEnharmony = false) const;
    bool haveAugmentedInterval(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 1 ===== //
    bool haveDiminishedUnisson(const bool useEnharmony = false) const;
    bool havePerfectUnisson(const bool useEnharmony = false) const;
    bool haveAugmentedUnisson(const bool useEnharmony = false) const;
    bool haveMinorSecond(const bool useEnharmony = false) const;
    bool haveMajorSecond(const bool useEnharmony = false) const;
    bool haveMinorThird(const bool useEnharmony = false) const;
    bool haveMajorThird(const bool useEnharmony = false) const;
    bool havePerfectFourth(const bool useEnharmony = false) const;
    bool haveAugmentedFourth(const bool useEnharmony = false) const;
    bool haveDiminishedFifth(const bool useEnharmony = false) const;
    bool havePerfectFifth(const bool useEnharmony = false) const;
    bool haveAugmentedFifth(const bool useEnharmony = false) const;
    bool haveMinorSixth(const bool useEnharmony = false) const;
    bool haveMajorSixth(const bool useEnharmony = false) const;
    bool haveDiminishedSeventh(const bool useEnharmony = false) const;
    bool haveMinorSeventh(const bool useEnharmony = false) const;
    bool haveMajorSeventh(const bool useEnharmony = false) const;
    bool haveDiminishedOctave(const bool useEnharmony = false) const;
    bool havePerfectOctave(const bool useEnharmony = false) const;
    bool haveAugmentedOctave(const bool useEnharmony = false) const;
    bool haveMinorNinth(const bool useEnharmony = false) const;
    bool haveMajorNinth(const bool useEnharmony = false) const;
    bool havePerfectEleventh(const bool useEnharmony = false) const;
    bool haveSharpEleventh(const bool useEnharmony = false) const;
    bool haveMinorThirdteenth(const bool useEnharmony = false) const;
    bool haveMajorThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 2 ===== //
    bool haveSecond(const bool useEnharmony = false) const;
    bool haveThird(const bool useEnharmony = false) const;
    bool haveFourth(const bool useEnharmony = false) const;
    bool haveFifth(const bool useEnharmony = false) const;
    bool haveSixth(const bool useEnharmony = false) const;
    bool haveSeventh(const bool useEnharmony = false) const;
    bool haveOctave(const bool useEnharmony = false) const;
    bool haveNinth(const bool useEnharmony = false) const;
    bool haveEleventh(const bool useEnharmony = false) const;
    bool haveThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //
    bool haveAnyOctaveMinorSecond(const bool useEnharmony = false) const;
    bool haveAnyOctaveMajorSecond(const bool useEnharmony = false) const;
    bool haveAnyOctaveMinorThird(const bool useEnharmony = false) const;
    bool haveAnyOctaveMajorThird(const bool useEnharmony = false) const;
    bool haveAnyOctavePerfectFourth(const bool useEnharmony = false) const;
    bool haveAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;
    bool haveAnyOctaveDiminhavehedFifth(const bool useEnharmony = false) const;
    bool haveAnyOctavePerfectFifth(const bool useEnharmony = false) const;
    bool haveAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;
    bool haveAnyOctaveMinorSixth(const bool useEnharmony = false) const;
    bool haveAnyOctaveMajorSixth(const bool useEnharmony = false) const;
    bool haveAnyOctaveDiminhavehedSeventh(const bool useEnharmony = false) const;
    bool haveAnyOctaveMinorSeventh(const bool useEnharmony = false) const;
    bool haveAnyOctaveMajorSeventh(const bool useEnharmony = false) const;
    bool haveAnyOctaveDiminhavehedOctave(const bool useEnharmony = false) const;
    bool haveAnyOctavePerfectOctave(const bool useEnharmony = false) const;
    bool haveAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //
    bool haveAnyOctaveSecond() const;
    bool haveAnyOctaveThird() const;
    bool haveAnyOctaveFourth() const;
    bool haveAnyOctaveFifth() const;
    bool haveAnyOctaveSixth() const;
    bool haveAnyOctaveSeventh() const;
    bool haveAnyOctaveOctave() const;

    bool isMajorChord();
    bool isMinorChord();
    bool isAugmentedChord();
    bool isDiminishedChord();
    bool isDominantSeventhChord();
    bool isHalfDiminishedChord();

    bool isSorted() const;
    bool isTonal(std::function<bool(const Chord& chord)> model = nullptr);
    bool isInRootPosition();

    std::vector<int> getMIDIIntervals(const bool firstNoteAsReference = false) const;
    std::vector<Interval> getIntervals(const bool firstNoteAsReference = false) const;
    std::vector<Interval> getOpenStackIntervals(const bool firstNoteAsReference = false);
    std::vector<Interval> getCloseStackIntervals(const bool firstNoteAsReference = false);
    std::vector<Note> getOpenStackNotes();

    int size() const;
    int stackSize();

    void print() const;
    void printStack() const;
    void info();

    Chord getOpenStackChord(const bool enharmonyNotes = false);
    Chord getCloseStackChord(const bool enharmonyNotes = false);
    Chord getCloseChord(const bool enharmonyNotes = false);

    void sortNotes();

    const Note& operator[](size_t index) const { return _originalNotes.at(index); }

    Note& operator[](size_t index) { return _originalNotes.at(index); }

    bool operator==(const Chord& otherChord) const {
        size_t sizeA = this->size();
        size_t sizeB = otherChord.size();

        if (sizeA != sizeB) {
            return false;
        }

        for (size_t i = 0; i < sizeA; i++) {
            if (_originalNotes[i] != otherChord.getNote(i)) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const Chord& otherChord) const {
        size_t sizeA = this->size();
        size_t sizeB = otherChord.size();

        if (sizeA != sizeB) {
            return true;
        }

        for (size_t i = 0; i < sizeA; i++) {
            if (_originalNotes[i] != otherChord.getNote(i)) {
                return true;
            }
        }

        return false;
    }

    Chord operator+(const Chord& otherChord) const {
        Chord x = *this;

        const size_t sizeChord = otherChord.size();
        for (size_t i = 0; i < sizeChord; i++) {
            x.addNote(otherChord[i]);
        }

        return x;
    }

    void operator<<(const Note& note) { addNote(note); }

    Note operator>>(Note& note) {
        note = _originalNotes[_originalNotes.size() - 1];

        removeTopNote();

        return note;
    }
};
