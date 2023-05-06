#pragma once
#include <functional>
#include <numeric>  // std::accumulate
#include <tuple>    // std::tuple
#include <vector>

#include "maiacore/interval.h"
#include "maiacore/key.h"
#include "maiacore/note.h"

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

    template <typename T>
    float computeStandardDeviation(const std::vector<T>& v) const {
        const float sum = std::accumulate(v.begin(), v.end(), 0);
        const float mean = sum / v.size();

        std::vector<float> diff(v.size());
        std::transform(v.begin(), v.end(), diff.begin(), [mean](double x) { return x - mean; });
        const float sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        const float stdev = std::sqrt(sq_sum / v.size());

        return stdev;
    }

   public:
    /**
     * @brief Construct a new Chord object
     *
     */
    Chord();

    /**
     * @brief Construct a new Chord object
     *
     * @param notes
     */
    explicit Chord(const std::vector<Note>& notes);

    /**
     * @brief Construct a new Chord object
     *
     * @param pitches
     */
    explicit Chord(const std::vector<std::string>& pitches);

    /**
     * @brief Destroy the Chord object
     *
     */
    ~Chord();

    /**
     * @brief
     *
     */
    void clear();

    /**
     * @brief
     *
     * @param note
     */
    void addNote(const Note& note);

    /**
     * @brief
     *
     * @param pitch
     */
    void addNote(const std::string& pitch);

    /**
     * @brief
     *
     */
    void removeTopNote();

    /**
     * @brief
     *
     * @param insertNote
     * @param positionNote
     */
    void insertNote(Note& insertNote, int positionNote = 0);

    /**
     * @brief
     *
     * @param noteIndex
     */
    void removeNote(int noteIndex);

    /**
     * @brief Set the Duration Ticks object
     *
     * @param durationTicks
     */
    void setDurationTicks(const int durationTicks);

    /**
     * @brief
     *
     * @param inversionNumber
     */
    void inversion(int inversionNumber);

    /**
     * @brief
     *
     * @param semiTonesNumber
     */
    void transpose(const int semiTonesNumber);

    /**
     * @brief
     *
     * @param semiTonesNumber
     */
    void transposeStackOnly(const int semiTonesNumber);

    /**
     * @brief
     *
     */
    void removeDuplicateNotes();

    /**
     * @brief Get the Stacked Heaps object
     *
     * @param enharmonyNotes
     * @return std::vector<HeapData>
     */
    std::vector<HeapData> getStackedHeaps(const bool enharmonyNotes = false);

    /**
     * @brief Get the Duration object
     *
     * @return std::string
     */
    std::string getDuration() const;

    /**
     * @brief Get the Quarter Duration object
     *
     * @return float
     */
    float getQuarterDuration() const;

    /**
     * @brief Get the Duration Ticks object
     *
     * @return int
     */
    int getDurationTicks() const;

    /**
     * @brief Get the Note object
     *
     * @param noteIndex
     * @return Note&
     */
    Note& getNote(int noteIndex);

    /**
     * @brief Get the Note object
     *
     * @param noteIndex
     * @return const Note&
     */
    const Note& getNote(const int noteIndex) const;

    /**
     * @brief Get the Root object
     *
     * @return const Note&
     */
    const Note& getRoot();

    /**
     * @brief Get the Name object
     *
     * @return std::string
     */
    std::string getName();

    /**
     * @brief Get the Bass Note object
     *
     * @return const Note&
     */
    const Note& getBassNote();

    /**
     * @brief Get the Notes object
     *
     * @return const std::vector<Note>&
     */
    const std::vector<Note>& getNotes() const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorInterval(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorInterval(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool havePerfectInterval(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveDiminishedInterval(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAugmentedInterval(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 1 ===== //
    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveDiminishedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool havePerfectUnisson(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAugmentedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorSecond(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorSecond(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorThird(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorThird(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool havePerfectFourth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAugmentedFourth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveDiminishedFifth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool havePerfectFifth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAugmentedFifth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorSixth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorSixth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveDiminishedSeventh(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorSeventh(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorSeventh(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveDiminishedOctave(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool havePerfectOctave(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAugmentedOctave(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorNinth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorNinth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool havePerfectEleventh(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveSharpEleventh(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMinorThirdteenth(const bool useEnharmony = false);

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveMajorThirdteenth(const bool useEnharmony = false);

    // ===== ABSTRACTION 2 ===== //

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveNinth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveEleventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctavePerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveDiminhavehedFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctavePerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveDiminhavehedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveDiminhavehedOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctavePerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief
     *
     * @param useEnharmony
     * @return true
     * @return false
     */
    bool haveAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveSecond() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveThird() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveFourth() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveFifth() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveSixth() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveSeventh() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool haveAnyOctaveOctave() const;

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isMajorChord();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isMinorChord();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAugmentedChord();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isDiminishedChord();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isDominantSeventhChord();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isHalfDiminishedChord();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isSorted() const;

    /**
     * @brief
     *
     * @param model
     * @return true
     * @return false
     */
    bool isTonal(std::function<bool(const Chord& chord)> model = nullptr);

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isInRootPosition();

    /**
     * @brief
     *
     * @param firstNoteAsReference
     * @return std::vector<int>
     */
    std::vector<int> getMIDIIntervals(const bool firstNoteAsReference = false) const;

    /**
     * @brief Get the Intervals object
     *
     * @param firstNoteAsReference
     * @return std::vector<Interval>
     */
    std::vector<Interval> getIntervals(const bool firstNoteAsReference = false) const;

    /**
     * @brief Get the Intervals From Original Sorted Notes object
     *
     * @return std::vector<Interval>
     */
    std::vector<Interval> getIntervalsFromOriginalSortedNotes() const;

    /**
     * @brief Get the Open Stack Intervals object
     *
     * @param firstNoteAsReference
     * @return std::vector<Interval>
     */
    std::vector<Interval> getOpenStackIntervals(const bool firstNoteAsReference = false);

    /**
     * @brief Get the Close Stack Intervals object
     *
     * @param firstNoteAsReference
     * @return std::vector<Interval>
     */
    std::vector<Interval> getCloseStackIntervals(const bool firstNoteAsReference = false);

    /**
     * @brief Get the Open Stack Notes object
     *
     * @return std::vector<Note>
     */
    std::vector<Note> getOpenStackNotes();

    /**
     * @brief
     *
     * @return int
     */
    int size() const;

    /**
     * @brief
     *
     * @return int
     */
    int stackSize();

    /**
     * @brief
     *
     */
    void print() const;

    /**
     * @brief
     *
     */
    void printStack() const;

    /**
     * @brief
     *
     */
    void info();

    /**
     * @brief Get the Open Stack Chord object
     *
     * @param enharmonyNotes
     * @return Chord
     */
    Chord getOpenStackChord(const bool enharmonyNotes = false);

    /**
     * @brief Get the Close Stack Chord object
     *
     * @param enharmonyNotes
     * @return Chord
     */
    Chord getCloseStackChord(const bool enharmonyNotes = false);

    /**
     * @brief Get the Close Chord object
     *
     * @param enharmonyNotes
     * @return Chord
     */
    Chord getCloseChord(const bool enharmonyNotes = false);

    /**
     * @brief
     *
     */
    void sortNotes();

    /**
     * @brief
     *
     * @return std::vector<int>
     */
    std::vector<int> toCents() const;

    /**
     * @brief Get the Degree object
     *
     * @param key
     * @param enharmonyNotes
     * @return int
     */
    int getDegree(const Key& key, bool enharmonyNotes = false);

    /**
     * @brief Get the Roman Degree object
     *
     * @param key
     * @param enharmonyNotes
     * @return std::string
     */
    std::string getRomanDegree(const Key& key, bool enharmonyNotes = false);

    /**
     * @brief Get the Mean Frequency object
     *
     * @return float
     */
    float getMeanFrequency() const;

    /**
     * @brief Get the Mean Of Extremes Frequency object
     *
     * @return float
     */
    float getMeanOfExtremesFrequency() const;

    /**
     * @brief Get the Frequency Std object
     *
     * @return float
     */
    float getFrequencyStd() const;

    /**
     * @brief Get the Mean Midi Value object
     *
     * @return int
     */
    int getMeanMidiValue() const;

    /**
     * @brief Get the Mean Of Extremes Midi Values object
     *
     * @return int
     */
    int getMeanOfExtremesMidiValue() const;

    /**
     * @brief Get the Midi Value Std object
     *
     * @return float
     */
    float getMidiValueStd() const;

    /**
     * @brief Get the Mean Pitch object
     *
     * @param accType
     * @return std::string
     */
    std::string getMeanPitch(const std::string& accType = {}) const;

    /**
     * @brief Get the Mean Of Extremes Pitch object
     *
     * @param accType
     * @return std::string
     */
    std::string getMeanOfExtremesPitch(const std::string& accType = {}) const;

    /**
     * @brief
     *
     * @param index
     * @return const Note&
     */
    const Note& operator[](size_t index) const { return _originalNotes.at(index); }

    /**
     * @brief
     *
     * @param index
     * @return Note&
     */
    Note& operator[](size_t index) { return _originalNotes.at(index); }

    /**
     * @brief
     *
     * @param otherChord
     * @return true
     * @return false
     */
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

    /**
     * @brief
     *
     * @param otherChord
     * @return true
     * @return false
     */
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

    /**
     * @brief
     *
     * @param otherChord
     * @return Chord
     */
    Chord operator+(const Chord& otherChord) const {
        Chord x = *this;

        const size_t sizeChord = otherChord.size();
        for (size_t i = 0; i < sizeChord; i++) {
            x.addNote(otherChord[i]);
        }

        return x;
    }

    /**
     * @brief
     *
     * @param note
     */
    void operator<<(const Note& note) { addNote(note); }

    /**
     * @brief
     *
     * @param note
     * @return Note
     */
    Note operator>>(Note& note) {
        note = _originalNotes[_originalNotes.size() - 1];

        removeTopNote();

        return note;
    }

    std::vector<Note>::iterator begin() { return _originalNotes.begin(); }
    std::vector<Note>::iterator end() { return _originalNotes.end(); }
};
