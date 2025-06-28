#pragma once
#include <functional>
#include <numeric>  // std::accumulate
#include <tuple>    // std::tuple
#include <vector>
#include <algorithm> // std::transform

#include "maiacore/interval.h"
#include "maiacore/key.h"
#include "maiacore/note.h"

/// @cond IGNORE_DOXYGEN
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
        return lhs.note.getMidiNumber() < rhs.note.getMidiNumber();
    }
};

/// @endcond

// NoteDataHeap Type [Vector of NotesData]
typedef std::vector<NoteData> NoteDataHeap;
// HeapData Type [NoteDataHeap, stackMatchValue]
typedef std::tuple<NoteDataHeap, float> HeapData;

bool operator<(const HeapData& a, const HeapData& b);

void printHeap(const NoteDataHeap& heap);

void sortHeapOctaves(NoteDataHeap* heap);

/**
 * @brief Sethares Dissonance Table Row type
 * 00) Base Frequency Idx
 * 01) Base Frequency
 * 02) Base Frequency Pitch
 * 03) Base Frequency Pitch (deviation in cents)
 * 04) Base Freqyency Amplitude
 * 05) Target Frequency Idx
 * 06) Target Frequency
 * 07) Target Frequency Pitch
 * 08) Target Frequency Pitch (deviation in cents)
 * 09) Target Freqyency Amplitude
 * 10) Sethares Calculated Amplitude
 * 11) Dyad Frequency Ratio
 * 12) Sethares Dyad frequencies Dissonance
 */
typedef std::tuple<int, float, std::string, int, float, int, float, std::string, int, float, float,
                   float, float>
    SetharesDissonanceTableRow;
typedef std::vector<SetharesDissonanceTableRow> SetharesDissonanceTable;

/**
 * @brief Represents a musical chord
 * @details The Chord class encapsulates a collection of musical notes, allowing for operations such as stacking
 * in thirds, transposing, and computing harmonic properties. It supports both open and closed stack representations,
 * as well as enharmonic transformations of notes.
 */
class Chord {
   private:
    /**
     * @brief Stores the original notes of the chord (no enharmonic transformation).
     */
    std::vector<Note> _originalNotes;

    /**
     * @brief Stores the notes stacked in thirds in open position (may include enharmonic notes).
     */
    std::vector<Note> _openStack;

    /**
     * @brief Stores the notes stacked in thirds in closed position (may include enharmonic notes).
     */
    std::vector<Note> _closeStack;

    /**
     * @brief Stores all possible enharmonic stacks in open position.
     */
    std::vector<HeapData> _stackedHeaps;

    /**
     * @brief Stores the intervals between notes in the closed stack.
     */
    std::vector<Interval> _closeStackintervals;

    /**
     * @brief Stores the bass note (lowest note) of the chord.
     */
    Note _bassNote;

    /**
     * @brief Indicates if the chord has already been stacked in thirds.
     */
    bool _isStackedInThirds;

    /**
     * @brief Computes and stores the intervals for the closed stack.
     */
    void computeIntervals();

    /**
     * @brief Stacks the chord notes in thirds, optionally using enharmonic equivalents.
     * @param enharmonyNotes If true, considers enharmonic notes for stacking.
     */
    void stackInThirds(const bool enharmonyNotes = false);

    /**
     * @brief Computes a template match value for a given heap of notes stacked in thirds.
     * @param heap The heap of NoteData to evaluate.
     * @return HeapData containing the heap and its match value.
     */
    HeapData stackInThirdsTemplateMatch(const NoteDataHeap& heap) const;

    /**
     * @brief Computes all possible enharmonic unit groups for the open stack notes.
     * @return Vector of NoteDataHeap, each representing a group of enharmonic variants for a note.
     */
    std::vector<NoteDataHeap> computeEnharmonicUnitsGroups() const;

    /**
     * @brief Computes all possible enharmonic heaps (combinations) from unit groups.
     * @param heaps The unit groups for each note.
     * @return Vector of NoteDataHeap, each representing a possible heap.
     */
    std::vector<NoteDataHeap> computeEnharmonicHeaps(const std::vector<NoteDataHeap>& heaps) const;

    /**
     * @brief Removes heaps that contain duplicated pitch steps.
     * @param heaps The input heaps to filter.
     * @return Vector of valid NoteDataHeap without duplicated pitch steps.
     */
    std::vector<NoteDataHeap> removeHeapsWithDuplicatedPitchSteps(
        std::vector<NoteDataHeap>& heaps) const;

    /**
     * @brief Computes all possible inversions (permutations) of a heap.
     * @param heap The heap to invert.
     * @return Vector of NoteDataHeap, each representing a permutation.
     */
    std::vector<NoteDataHeap> computeAllHeapInversions(NoteDataHeap& heap) const;

    /**
     * @brief Filters heaps to retain only those that are tertian (built in thirds).
     * @param heaps The input heaps.
     * @return Vector of tertian NoteDataHeap.
     */
    std::vector<NoteDataHeap> filterTertianHeapsOnly(const std::vector<NoteDataHeap>& heaps) const;

    /**
     * @brief Selects the best open stack heap based on match value and pitch class correspondence.
     * @param stackedHeaps Vector of HeapData to evaluate.
     * @return Vector of Note representing the best heap.
     */
    std::vector<Note> computeBestOpenStackHeap(std::vector<HeapData>& stackedHeaps);

    /**
     * @brief Computes the closed stack version from the best open stack heap.
     * @param openStack The best open stack heap.
     */
    void computeCloseStack(const std::vector<Note>& openStack);

    /**
     * @brief Computes the standard deviation of a vector of values.
     * @tparam T Numeric type.
     * @param v The vector of values.
     * @return Standard deviation as float.
     */
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
     * @brief Construct an empty Chord object.
     *
     * The chord will have no notes initially.
     */
    Chord();

    /**
     * @brief Construct a Chord from a vector of Note objects.
     * @param notes Vector of Note objects to initialize the chord.
     * @param rhythmFigure The rhythm figure to assign to each note (default: QUARTER).
     */
    explicit Chord(const std::vector<Note>& notes, const RhythmFigure rhythmFigure = RhythmFigure::QUARTER);

    /**
     * @brief Construct a Chord from a vector of pitch strings.
     * @param pitches Vector of pitch strings (e.g., "C4", "E4", "G4").
     * @param rhythmFigure The rhythm figure to assign to each note (default: QUARTER).
     */
    explicit Chord(const std::vector<std::string>& pitches, const RhythmFigure rhythmFigure = RhythmFigure::QUARTER);

    /**
     * @brief Destroy the Chord object and release resources.
     */
    ~Chord();

    /**
     * @brief Remove all notes from the chord, resetting its state.
     */
    void clear();

    /**
     * @brief Add a Note object to the chord.
     * @param note The Note to add. Rests are ignored.
     */
    void addNote(const Note& note);

    /**
     * @brief Add a note to the chord by pitch string.
     * @param pitch The pitch string (e.g., "C4").
     */
    void addNote(const std::string& pitch);

    /**
     * @brief Remove the top (last) note from the chord.
     */
    void removeTopNote();

    /**
     * @brief Insert a note at a specific position in the chord.
     * @param insertNote The note to insert.
     * @param positionNote The index at which to insert the note.
     */
    void insertNote(Note& insertNote, int positionNote = 0);

    /**
     * @brief Remove a note at a specific index from the chord.
     * @param noteIndex Index of the note to remove.
     */
    void removeNote(int noteIndex);

    /**
     * @brief Set the duration for all notes in the chord.
     * @param duration The Duration object to assign.
     */
    void setDuration(const Duration& duration);

    /**
     * @brief Set the duration for all notes in the chord using quarter note value.
     * @param quarterDuration Duration in quarter notes.
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     */
    void setDuration(const float quarterDuration, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Invert the chord by moving the lowest note up by one octave, repeated inversionNumber times.
     * @param inversionNumber Number of inversions to perform.
     */
    void toInversion(int inversionNumber);

    /**
     * @brief Transpose all notes in the chord by a number of semitones.
     * @param semiTonesNumber Number of semitones to transpose.
     */
    void transpose(const int semiTonesNumber);

    /**
     * @brief Transpose only the stacked (open) version of the chord by a number of semitones.
     * @param semiTonesNumber Number of semitones to transpose.
     */
    void transposeStackOnly(const int semiTonesNumber);

    /**
     * @brief Remove duplicate notes (by pitch) from the chord.
     */
    void removeDuplicateNotes();

    /**
     * @brief Get all possible stacked heaps (enharmonic variants) for the chord.
     * @param enharmonyNotes If true, considers enharmonic equivalents.
     * @return Vector of HeapData representing all possible stacks.
     */
    std::vector<HeapData> getStackedHeaps(const bool enharmonyNotes = false);

    /**
     * @brief Get the shortest duration type among all notes in the chord.
     * @return String representing the duration type (e.g., "quarter").
     */
    std::string getDuration() const;

    /**
     * @brief Get the shortest duration in quarter notes among all notes.
     * @return Duration in quarter notes as float.
     */
    float getQuarterDuration() const;

    /**
     * @brief Get the minimum duration in ticks among all notes.
     * @return Duration in ticks as int.
     */
    int getDurationTicks() const;

    /**
     * @brief Get a reference to a note at a given index.
     * @param noteIndex Index of the note.
     * @return Reference to the Note.
     */
    Note& getNote(int noteIndex);

    /**
     * @brief Get a const reference to a note at a given index.
     * @param noteIndex Index of the note.
     * @return Const reference to the Note.
     */
    const Note& getNote(const int noteIndex) const;

    /**
     * @brief Get the root note of the chord (after stacking in thirds).
     * @return Const reference to the root Note.
     */
    const Note& getRoot();

    /**
     * @brief Get the chord name using tonal analysis (e.g., "Cm7", "G7").
     * @return String with the chord name, or empty if indeterminate.
     */
    std::string getName();

    /**
     * @brief Get the bass note of the chord (lowest note).
     * @return Const reference to the bass Note.
     */
    const Note& getBassNote();

    /**
     * @brief Get all notes in the chord (original order).
     * @return Const reference to vector of Notes.
     */
    const std::vector<Note>& getNotes() const;

    /**
     * @brief Compute the harmonic complexity of the chord in closed position.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return Normalized complexity value (0.0 to 1.0).
     */
    float getCloseStackHarmonicComplexity(const bool useEnharmony = false);

    /**
     * @brief Compute the harmonic density of the chord in a MIDI range.
     * @param lowerBoundMIDI Lowest MIDI note (default: -1, auto-detect).
     * @param higherBoundMIDI Highest MIDI note (default: -1, auto-detect).
     * @return Density as float.
     */
    float getHarmonicDensity(int lowerBoundMIDI = -1, int higherBoundMIDI = -1) const;

    /**
     * @brief Compute the harmonic density of the chord in a pitch range.
     * @param lowerBoundPitch Lowest pitch string (e.g., "C4").
     * @param higherBoundPitch Highest pitch string (e.g., "G5").
     * @return Density as float.
     */
    float getHarmonicDensity(const std::string& lowerBoundPitch = {},
                             const std::string& higherBoundPitch = {}) const;

    /**
     * @brief Check if the chord contains at least one major interval.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major interval exists.
     */
    bool haveMajorInterval(const bool useEnharmony = false) const;

    /**
     * @brief Check if the chord contains at least one minor interval.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor interval exists.
     */
    bool haveMinorInterval(const bool useEnharmony = false) const;

    /**
     * @brief Check if the chord contains at least one perfect interval.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect interval exists.
     */
    bool havePerfectInterval(const bool useEnharmony = false) const;

    /**
     * @brief Check if the chord contains at least one diminished interval.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished interval exists.
     */
    bool haveDiminishedInterval(const bool useEnharmony = false) const;

    /**
     * @brief Check if the chord contains at least one augmented interval.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented interval exists.
     */
    bool haveAugmentedInterval(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 1 ===== //
    /**
     * @brief Checks if the chord contains a diminished unison interval (e.g., C and Cb).
     * @details Useful for identifying chromatic clusters or microtonal chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished unison is present between any two notes.
     */
    bool haveDiminishedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect unison interval (e.g., two notes with the same pitch).
     * @details Indicates the presence of doubled notes in the chord.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect unison is present.
     */
    bool havePerfectUnisson(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented unison interval (e.g., C and C#).
     * @details Useful for detecting chromatic clusters or extended harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented unison is present.
     */
    bool haveAugmentedUnisson(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor second interval (e.g., C and Db).
     * @details Identifies the presence of the smallest diatonic interval, often associated with dissonance.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor second is present.
     */
    bool haveMinorSecond(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major second interval (e.g., C and D).
     * @details Useful for analyzing stepwise motion or clusters.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major second is present.
     */
    bool haveMajorSecond(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a minor third interval (e.g., C and Eb).
     * @details Essential for identifying minor triads and seventh chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor third is present.
     */
    bool haveMinorThird(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major third interval (e.g., C and E).
     * @details Essential for identifying major triads and seventh chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major third is present.
     */
    bool haveMajorThird(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a perfect fourth interval (e.g., C and F).
     * @details Useful for quartal harmonies and sus chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect fourth is present.
     */
    bool havePerfectFourth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains an augmented fourth interval (tritone, e.g., C and F#).
     * @details The tritone is a key interval in dominant and diminished harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented fourth is present.
     */
    bool haveAugmentedFourth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a diminished fifth interval (tritone, e.g., C and Gb).
     * @details The tritone is a key interval in dominant and diminished harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished fifth is present.
     */
    bool haveDiminishedFifth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a perfect fifth interval (e.g., C and G).
     * @details The perfect fifth is fundamental to tonal harmony and chord structure.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect fifth is present.
     */
    bool havePerfectFifth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains an augmented fifth interval (e.g., C and G#).
     * @details Indicates the presence of augmented chords or altered dominants.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented fifth is present.
     */
    bool haveAugmentedFifth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a minor sixth interval (e.g., C and Ab).
     * @details Useful for identifying extended harmonies and voice leading.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor sixth is present.
     */
    bool haveMinorSixth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major sixth interval (e.g., C and A).
     * @details Useful for identifying extended harmonies and voice leading.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major sixth is present.
     */
    bool haveMajorSixth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a diminished seventh interval (e.g., C and Bbb).
     * @details Characteristic of diminished seventh chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished seventh is present.
     */
    bool haveDiminishedSeventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a minor seventh interval (e.g., C and Bb).
     * @details Essential for minor seventh and dominant seventh chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor seventh is present.
     */
    bool haveMinorSeventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major seventh interval (e.g., C and B).
     * @details Essential for major seventh chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major seventh is present.
     */
    bool haveMajorSeventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a diminished octave interval (e.g., C and Cb an octave apart).
     * @details Rare, but may occur in complex or microtonal harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished octave is present.
     */
    bool haveDiminishedOctave(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a perfect octave interval (e.g., C4 and C5).
     * @details Indicates the presence of doubled notes in different octaves.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect octave is present.
     */
    bool havePerfectOctave(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains an augmented octave interval (e.g., C and C# an octave apart).
     * @details Rare, but may occur in complex or microtonal harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented octave is present.
     */
    bool haveAugmentedOctave(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a minor ninth interval (e.g., C and Db an octave apart).
     * @details Useful for identifying extended harmonies (e.g., 9th chords).
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor ninth is present.
     */
    bool haveMinorNinth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major ninth interval (e.g., C and D an octave apart).
     * @details Useful for identifying extended harmonies (e.g., 9th chords).
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major ninth is present.
     */
    bool haveMajorNinth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a perfect eleventh interval (e.g., C and F two octaves apart).
     * @details Useful for identifying 11th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect eleventh is present.
     */
    bool havePerfectEleventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a sharp eleventh interval (e.g., C and F# two octaves apart).
     * @details Useful for identifying altered dominants and Lydian harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a sharp eleventh is present.
     */
    bool haveSharpEleventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a minor thirteenth interval (e.g., C and Ab two octaves plus a sixth apart).
     * @details Useful for identifying 13th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor thirteenth is present.
     */
    bool haveMinorThirdteenth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major thirteenth interval (e.g., C and A two octaves plus a sixth apart).
     * @details Useful for identifying 13th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major thirteenth is present.
     */
    bool haveMajorThirdteenth(const bool useEnharmony = false);

    // ===== ABSTRACTION 2 ===== //

    /**
     * @brief Checks if the chord contains any second interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a second (major or minor) between any two notes in the chord, regardless of octave.
     * This is useful for identifying stepwise relationships within the chord's structure.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a second interval is present, false otherwise.
     */
    bool haveSecond(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any third interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a third (major or minor) between any two notes in the chord, regardless of octave.
     * This is useful for identifying tertian (third-based) relationships within the chord.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a third interval is present, false otherwise.
     */
    bool haveThird(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any fourth interval (perfect, augmented, or diminished) between its notes.
     * @details Returns true if there is at least one interval of a fourth (of any quality) between any two notes in the chord, regardless of octave.
     * Useful for identifying quartal harmonies or sus chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a fourth interval is present, false otherwise.
     */
    bool haveFourth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any fifth interval (perfect, augmented, or diminished) between its notes.
     * @details Returns true if there is at least one interval of a fifth (of any quality) between any two notes in the chord, regardless of octave.
     * Useful for identifying quintal harmonies or power chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a fifth interval is present, false otherwise.
     */
    bool haveFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any sixth interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a sixth (major or minor) between any two notes in the chord, regardless of octave.
     * Useful for identifying extended tertian harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a sixth interval is present, false otherwise.
     */
    bool haveSixth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any seventh interval (major, minor, or diminished) between its notes.
     * @details Returns true if there is at least one interval of a seventh (of any quality) between any two notes in the chord, regardless of octave.
     * Useful for identifying seventh chords and complex harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a seventh interval is present, false otherwise.
     */
    bool haveSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any octave interval (perfect, augmented, or diminished) between its notes.
     * @details Returns true if there is at least one interval of an octave (of any quality) between any two notes in the chord.
     * Useful for identifying doubled notes or octave relationships.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an octave interval is present, false otherwise.
     */
    bool haveOctave(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any ninth interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a ninth (major or minor) between any two notes in the chord.
     * Useful for identifying extended harmonies such as 9th chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a ninth interval is present, false otherwise.
     */
    bool haveNinth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any eleventh interval (perfect or sharp) between its notes.
     * @details Returns true if there is at least one interval of an eleventh (perfect or sharp) between any two notes in the chord.
     * Useful for identifying 11th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an eleventh interval is present, false otherwise.
     */
    bool haveEleventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any thirteenth interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a thirteenth (major or minor) between any two notes in the chord.
     * Useful for identifying 13th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a thirteenth interval is present, false otherwise.
     */
    bool haveThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //

    /**
     * @brief Checks if the chord contains a minor second interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a minor second interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting clusters or dissonant relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor second (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major second interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a major second interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting stepwise motion or clusters in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major second (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor third interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a minor third interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting tertian relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor third (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major third interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a major third interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting tertian relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major third (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect fourth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a perfect fourth interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting quartal harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect fourth (any octave) is present, false otherwise.
     */
    bool haveAnyOctavePerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented fourth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms an augmented fourth (tritone) interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting tritones in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented fourth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a diminished fifth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a diminished fifth (tritone) interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting tritones in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished fifth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveDiminishedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect fifth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a perfect fifth interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting quintal harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect fifth (any octave) is present, false otherwise.
     */
    bool haveAnyOctavePerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented fifth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms an augmented fifth interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting altered harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented fifth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor sixth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a minor sixth interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting extended tertian harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor sixth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major sixth interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a major sixth interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting extended tertian harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major sixth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a diminished seventh interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a diminished seventh interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting diminished harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished seventh (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveDiminishedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor seventh interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a minor seventh interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting seventh chords in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor seventh (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major seventh interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a major seventh interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting major seventh chords in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major seventh (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a diminished octave interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a diminished octave interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting rare or microtonal relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished octave (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveDiminishedOctave(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect octave interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms a perfect octave interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting doubled notes in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect octave (any octave) is present, false otherwise.
     */
    bool haveAnyOctavePerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented octave interval between any two notes, ignoring octave differences.
     * @details Returns true if any pair of notes forms an augmented octave interval, regardless of their octave placement.
     * This method abstracts away octave information and considers only pitch class relationships.
     * Useful for detecting rare or microtonal relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented octave (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //

    /**
     * @brief Checks if the chord contains a generic second interval (major or minor, any octave).
     * @details Useful for identifying stepwise relationships between any two notes, regardless of octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any second interval is present.
     */
    bool haveAnyOctaveSecond() const;

    /**
     * @brief Checks if the chord contains a generic third interval (major or minor, any octave).
     * @details Useful for identifying tertian relationships between any two notes, regardless of octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any third interval is present.
     */
    bool haveAnyOctaveThird() const;

    /**
     * @brief Checks if the chord contains a generic fourth interval (perfect, augmented, or diminished, any octave).
     * @details Useful for identifying quartal relationships between any two notes, regardless of octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any fourth interval is present.
     */
    bool haveAnyOctaveFourth() const;

    /**
     * @brief Checks if the chord contains a generic fifth interval (perfect, augmented, or diminished, any octave).
     * @details Useful for identifying quintal relationships between any two notes, regardless of octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any fifth interval is present.
     */
    bool haveAnyOctaveFifth() const;

    /**
     * @brief Checks if the chord contains a generic sixth interval (major or minor, any octave).
     * @details Useful for identifying extended tertian or added-sixth harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any sixth interval is present.
     */
    bool haveAnyOctaveSixth() const;

    /**
     * @brief Checks if the chord contains a generic seventh interval (major, minor, or diminished, any octave).
     * @details Useful for identifying seventh chords or complex harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any seventh interval is present.
     */
    bool haveAnyOctaveSeventh() const;

    /**
     * @brief Checks if the chord contains a generic octave interval (perfect, augmented, or diminished, any octave).
     * @details Useful for identifying doubled notes or octave relationships.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any octave interval is present.
     */
    bool haveAnyOctaveOctave() const;

    /**
     * @brief Determines if the chord is a dyad (contains exactly two distinct notes).
     * @details Dyads are the simplest harmonic structures, often analyzed as intervals.
     * @return True if the chord is a dyad.
     */
    bool isDyad();

    /**
     * @brief Determines if the chord is a suspended chord (sus2 or sus4).
     * @details Suspended chords replace the third with a second or fourth, creating a characteristic open sound.
     * @return True if the chord is a sus chord.
     */
    bool isSus();

    /**
     * @brief Determines if the chord is a major triad.
     * @details Major triads contain a root, major third, and perfect fifth.
     * @return True if the chord is a major triad.
     */
    bool isMajorChord();

    /**
     * @brief Determines if the chord is a minor triad.
     * @details Minor triads contain a root, minor third, and perfect fifth.
     * @return True if the chord is a minor triad.
     */
    bool isMinorChord();

    /**
     * @brief Determines if the chord is an augmented triad.
     * @details Augmented triads contain a root, major third, and augmented fifth.
     * @return True if the chord is an augmented triad.
     */
    bool isAugmentedChord();

    /**
     * @brief Determines if the chord is a diminished triad.
     * @details Diminished triads contain a root, minor third, and diminished fifth.
     * @return True if the chord is a diminished triad.
     */
    bool isDiminishedChord();

    /**
     * @brief Determines if the chord is a half-diminished seventh chord.
     * @details Half-diminished chords contain a root, minor third, diminished fifth, and minor seventh.
     * @return True if the chord is half-diminished.
     */
    bool isHalfDiminishedChord();

    /**
     * @brief Determines if the chord is a fully diminished seventh chord.
     * @details Fully diminished chords contain a root, minor third, diminished fifth, and diminished seventh.
     * @return True if the chord is fully diminished.
     */
    bool isWholeDiminishedChord();

    /**
     * @brief Determines if the chord is a dominant seventh chord.
     * @details Dominant sevenths contain a root, major third, perfect fifth, and minor seventh.
     * @return True if the chord is a dominant seventh.
     */
    bool isDominantSeventhChord();

    /**
     * @brief Returns a string describing the chord quality (e.g., "major", "minor", "diminished", "augmented", "sus", etc).
     * @details The quality is determined by the chord's interval structure and is useful for harmonic analysis.
     * @return String representing the chord quality.
     */
    std::string getQuality();

    /**
     * @brief Checks if the notes in the chord are sorted in ascending order by MIDI number.
     * @details Useful for ensuring consistent interval calculations and for algorithms that require sorted input.
     * @return True if the notes are sorted from lowest to highest pitch.
     */
    bool isSorted() const;

    /**
     * @brief Determines if the chord is tonal according to a given model or default rules.
     * @details By default, checks if all intervals in the closed stack are tonal (major, minor, perfect, etc).
     *          If a custom model is provided, it is used to evaluate the chord's tonality.
     * @param model Optional: a function that takes a Chord and returns true if it is tonal.
     * @return True if the chord is considered tonal.
     */
    bool isTonal(std::function<bool(const Chord& chord)> model = nullptr);

    /**
     * @brief Checks if the chord is in root position (lowest note is the root).
     * @details Compares the pitch class of the lowest note with the root of the closed stack.
     * @return True if the chord is in root position.
     */
    bool isInRootPosition();

    /**
     * @brief Returns the intervals between notes in the chord as MIDI semitone values.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to each subsequent note.
     *          Otherwise, intervals are calculated between each pair of adjacent notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of intervals in semitones.
     */
    std::vector<int> getMidiIntervals(const bool firstNoteAsReference = false) const;

    /**
     * @brief Returns the intervals between notes in the chord as Interval objects.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to each subsequent note.
     *          Otherwise, intervals are calculated between each pair of adjacent notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of Interval objects.
     */
    std::vector<Interval> getIntervals(const bool firstNoteAsReference = false) const;

    /**
     * @brief Returns the intervals between the sorted original notes as Interval objects.
     * @details Useful for intervallic analysis independent of note order.
     * @return Vector of Interval objects between sorted notes.
     */
    std::vector<Interval> getIntervalsFromOriginalSortedNotes() const;

    /**
     * @brief Returns the intervals between notes in the open stack as Interval objects.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to each subsequent note.
     *          Otherwise, intervals are calculated between each pair of adjacent notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of Interval objects.
     */
    std::vector<Interval> getOpenStackIntervals(const bool firstNoteAsReference = false);

    /**
     * @brief Returns the intervals between notes in the closed stack as Interval objects.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to each subsequent note.
     *          Otherwise, intervals are calculated between each pair of adjacent notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of Interval objects.
     */
    std::vector<Interval> getCloseStackIntervals(const bool firstNoteAsReference = false);

    /**
     * @brief Returns the notes of the chord in open stack (stacked in thirds) order.
     * @details Triggers stacking if not already performed.
     * @return Vector of Note objects in open stack order.
     */
    std::vector<Note> getOpenStackNotes();

    /**
     * @brief Returns the number of notes in the chord.
     * @details Equivalent to the size of the original notes vector.
     * @return Number of notes in the chord.
     */
    int size() const;

    /**
     * @brief Returns the number of notes in the open stack (after stacking in thirds).
     * @details Triggers stacking if not already performed.
     * @return Number of notes in the open stack.
     */
    int stackSize();

    /**
     * @brief Prints the pitches of all notes in the chord to the log.
     * @details Useful for debugging and inspection.
     */
    void print() const;

    /**
     * @brief Prints the pitches of all notes in the open stack to the log.
     * @details Useful for debugging and inspection of the stacked chord.
     */
    void printStack() const;

    /**
     * @brief Prints detailed information about the chord, including name, size, notes, and stack.
     * @details Useful for analysis and debugging.
     */
    void info();

    /**
     * @brief Returns a Chord object representing the open stack (stacked in thirds).
     * @details Optionally considers enharmonic equivalents for stacking.
     * @param enharmonyNotes If true, considers enharmonic equivalents.
     * @return Chord object in open stack form.
     */
    Chord getOpenStackChord(const bool enharmonyNotes = false);

    /**
     * @brief Returns a Chord object representing the closed stack (stacked in thirds, closed position).
     * @details Optionally considers enharmonic equivalents for stacking.
     * @param enharmonyNotes If true, considers enharmonic equivalents.
     * @return Chord object in closed stack form.
     */
    Chord getCloseStackChord(const bool enharmonyNotes = false);

    /**
     * @brief Returns a Chord object representing the closed chord, with octaves adjusted to match the original.
     * @details Optionally considers enharmonic equivalents for stacking.
     * @param enharmonyNotes If true, considers enharmonic equivalents.
     * @return Chord object in closed form with original octaves.
     */
    Chord getCloseChord(const bool enharmonyNotes = false);

    /**
     * @brief Sorts the notes in the chord in ascending order by MIDI number.
     * @details Useful for ensuring consistent interval calculations and for algorithms that require sorted input.
     */
    void sortNotes();

    /**
     * @brief Returns a vector with the intervals (in cents) between each pair of adjacent notes in the chord.
     * @details Useful for microtonal and tuning analysis, as well as for comparing intervallic content.
     * @return Vector of integer values representing the interval in cents between each note pair.
     */
    std::vector<int> toCents() const;

    /**
     * @brief Returns the scale degree of the chord's root in a given key.
     * @details The degree is calculated relative to the provided Key object, considering enharmonic spelling if requested.
     * @param key The Key object representing the tonal center.
     * @param enharmonyNotes If true, considers enharmonic equivalents for the root.
     * @return Integer representing the scale degree (1 = tonic, 2 = supertonic, etc).
     */
    int getDegree(const Key& key, bool enharmonyNotes = false);

    /**
     * @brief Returns the Roman numeral degree of the chord's root in a given key.
     * @details The degree is calculated relative to the provided Key object, considering enharmonic spelling if requested.
     * @param key The Key object representing the tonal center.
     * @param enharmonyNotes If true, considers enharmonic equivalents for the root.
     * @return String with the Roman numeral (e.g., "I", "ii", "V").
     */
    std::string getRomanDegree(const Key& key, bool enharmonyNotes = false);

    /**
     * @brief Calculates the arithmetic mean of the frequencies of all notes in the chord.
     * @details Useful for spectral centroid and timbral analysis.
     * @return Mean frequency as a float.
     */
    float getMeanFrequency() const;

    /**
     * @brief Calculates the mean frequency between the lowest and highest notes in the chord.
     * @details Useful for summarizing the chord's spectral span.
     * @return Mean of extremes frequency as a float.
     */
    float getMeanOfExtremesFrequency() const;

    /**
     * @brief Calculates the standard deviation of the frequencies of all notes in the chord.
     * @details Useful for measuring the spectral spread or compactness of the chord.
     * @return Frequency standard deviation as a float.
     */
    float getFrequencyStd() const;

    /**
     * @brief Calculates the arithmetic mean of the MIDI numbers of all notes in the chord.
     * @details Useful for pitch center analysis.
     * @return Mean MIDI value as an integer.
     */
    int getMeanMidiValue() const;

    /**
     * @brief Calculates the mean MIDI value between the lowest and highest notes in the chord.
     * @details Useful for summarizing the pitch range of the chord.
     * @return Mean of extremes MIDI value as an integer.
     */
    int getMeanOfExtremesMidiValue() const;

    /**
     * @brief Calculates the standard deviation of the MIDI numbers of all notes in the chord.
     * @details Useful for measuring the pitch spread or compactness of the chord.
     * @return MIDI value standard deviation as a float.
     */
    float getMidiValueStd() const;

    /**
     * @brief Returns the pitch name corresponding to the mean MIDI value of the chord.
     * @details The accidental type can be specified (e.g., sharp, flat, natural).
     * @param accType Optional: specify accidental type for pitch spelling.
     * @return String with the pitch name (e.g., "C4", "F#3").
     */
    std::string getMeanPitch(const std::string& accType = {}) const;

    /**
     * @brief Returns the pitch name corresponding to the mean of the lowest and highest MIDI values in the chord.
     * @details The accidental type can be specified (e.g., sharp, flat, natural).
     * @param accType Optional: specify accidental type for pitch spelling.
     * @return String with the pitch name (e.g., "C4", "F#3").
     */
    std::string getMeanOfExtremesPitch(const std::string& accType = {}) const;

    /**
     * @brief Computes the combined harmonic spectrum of the chord by summing the spectra of all notes.
     * @details Useful for timbral and psychoacoustic analysis.
     * @param numPartialsPerNote Number of partials to include for each note.
     * @param amplCallback Optional: function to modify amplitude vector.
     * @return Pair of vectors: frequencies and corresponding amplitudes.
     */
    std::pair<std::vector<float>, std::vector<float>> getHarmonicSpectrum(
        const int numPartialsPerNote = 6,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr) const;

    /**
     * @brief Calculates the Sethares dissonance value for all dyads in the chord.
     * @details Uses the Sethares model for sensory dissonance, considering all pairs of partials.
     * @param numPartials Number of partials per note.
     * @param useMinModel If true, uses the minimum amplitude model; otherwise, uses the product.
     * @param amplCallback Optional: function to modify amplitude vector.
     * @return Table with detailed dissonance information for each dyad.
     */
    SetharesDissonanceTable getSetharesDyadsDissonanceValue(
        const int numPartials = 6, const bool useMinModel = true,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr) const;

    /**
     * @brief Calculates the total Sethares dissonance for the chord.
     * @details Aggregates the dissonance values for all dyads, optionally using a custom aggregation function.
     * @param numPartialsPerNote Number of partials per note.
     * @param useMinModel If true, uses the minimum amplitude model; otherwise, uses the product.
     * @param amplCallback Optional: function to modify amplitude vector.
     * @param dissCallback Optional: function to aggregate dissonance values (e.g., mean, max).
     * @return Total dissonance as a float.
     */
    float getSetharesDissonance(
        const int numPartialsPerNote = 6, const bool useMinModel = true,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr,
        const std::function<float(std::vector<float>)> dissCallback = nullptr) const;

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
     * @brief Returns an iterator to the beginning of the original notes vector.
     * @return Iterator to the first Note in the chord.
     */
    std::vector<Note>::iterator begin() { return _originalNotes.begin(); }

    /**
     * @brief Returns an iterator to the end of the original notes vector.
     * @return Iterator to one past the last Note in the chord.
     */
    std::vector<Note>::iterator end() { return _originalNotes.end(); }

    /**
     * @brief Output stream operator for Chord.
     * Prints the chord as a list of pitches.
     * @param os Output stream.
     * @param chord Chord to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Chord& chord);
};