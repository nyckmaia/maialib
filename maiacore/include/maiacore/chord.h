#pragma once
#include <algorithm>  // std::transform
#include <functional>
#include <numeric>  // std::accumulate
#include <tuple>    // std::tuple
#include <vector>

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
          enharmonicDiatonicDistance(_enharDiat) {};

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
 * @details The Chord class encapsulates a collection of musical notes, allowing for operations such
 * as stacking in thirds, transposing, and computing harmonic properties. It supports both open and
 * closed stack representations, as well as enharmonic transformations of notes.
 *
 * Notes of transposing instruments are analysed at concert pitch: at the pitch each one sounds,
 * spelled with its written letter moved by the diatonic transposing interval, which is inferred
 * from the chromatic one when it is 0; where that gives no spelling, the fallback spells the
 * position (see Note::getSoundingPitch()). A horn in F's written B4 is an E4, so with C4 and G4 it
 * makes a C major chord, and so does a (0, -7) instrument's. The notes the analysis returns -- the
 * root, the bass note, the open and close stacks, their heaps and the chords built from them --
 * are untransposed notes at those pitches; getNotes() returns the chord's own notes as they were
 * added.
 */
class Chord {
   private:
    /**
     * @brief Stores the original notes of the chord (no enharmonic transformation).
     */
    std::vector<Note> _originalNotes;

    /**
     * @brief Stores the notes stacked in thirds in open position (may include enharmonic notes).
     * @details stackInThirds() fills it with untransposed copies of the chord's notes at concert
     *          pitch, so the close stack, the bass note and the heaps hold untransposed notes too.
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
     * @brief Invalidate every cached/derived representation of the chord.
     *
     * Every mutator that changes '_originalNotes' (pitch content, membership or order) must call
     * this instead of resetting '_isStackedInThirds' by hand, so the cache is invalidated
     * consistently. Without it, stale '_closeStack'/'_stackedHeaps' entries computed for a
     * previous note set can survive a mutation and be read against the new (larger, smaller or
     * reordered) '_originalNotes'/'_openStack', which is an out-of-bounds read, not just a stale
     * answer. Does not touch '_openStack': it is kept in sync by each mutator directly (or fully
     * rebuilt by stackInThirds() itself), so clearing it here would only erase data the caller
     * may still read via a const method (e.g. printStack()) before the next stack computation.
     */
    void invalidateStackCache();

    /**
     * @brief Find the first note of the chord carrying a quarter-tone accidental.
     *
     * Shared by the analysis guard in stackInThirds(), which rejects such a chord, and by info(),
     * which degrades on one instead of throwing. Searches '_originalNotes' (the authoritative
     * note set, which stackInThirds() copies into '_openStack') in original order, so the note it
     * names in either message is stable and recognisable to the caller.
     *
     * @return Pointer to the first quarter-tone note, or nullptr if the chord has none. The
     *         pointer is owned by '_originalNotes' and is invalidated by any mutation of it.
     */
    const Note* findQuarterToneNote() const;

   public:
    /**
     * @brief Construct an empty Chord object.
     *
     * The chord will have no notes initially.
     */
    Chord();

    /**
     * @brief Construct a Chord from a vector of Note objects.
     * @details Each note is held untransposed, at concert pitch, with the given rhythm figure: a
     *          B-flat clarinet's written F#4 is held as an E4. Rests are skipped.
     * @param notes Vector of Note objects to initialize the chord.
     * @param rhythmFigure The rhythm figure to assign to each note (default: QUARTER).
     */
    explicit Chord(const std::vector<Note>& notes,
                   const RhythmFigure rhythmFigure = RhythmFigure::QUARTER);

    /**
     * @brief Construct a Chord from a vector of pitch strings.
     * @param pitches Vector of pitch strings (e.g., "C4", "E4", "G4").
     * @param rhythmFigure The rhythm figure to assign to each note (default: QUARTER).
     */
    explicit Chord(const std::vector<std::string>& pitches,
                   const RhythmFigure rhythmFigure = RhythmFigure::QUARTER);

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
     * @throws std::runtime_error if the chord is empty.
     */
    void removeTopNote();

    /**
     * @brief Insert a note at a specific position in the chord.
     * @param insertNote The note to insert.
     * @param positionNote The index at which to insert the note, in `0 .. size()` inclusive.
     *        `positionNote == size()` is valid and appends the note at the end, matching
     *        `std::vector::insert()`'s own valid range.
     * @throws std::runtime_error if `positionNote` is negative or greater than `size()`.
     */
    void insertNote(Note& insertNote, int positionNote = 0);

    /**
     * @brief Remove a note at a specific index from the chord.
     * @param noteIndex Index of the note to remove, in `0 .. size() - 1`. Unlike `insertNote()`,
     *        `noteIndex == size()` is out of range here: there is no note to remove there.
     * @throws std::runtime_error if `noteIndex` is negative or `>= size()` (e.g. on an empty
     *         chord).
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
     * @brief Round every quarter-tone note in the chord to the nearest semitone, ties upward.
     * @details The escape hatch for the harmonic-analysis rejection. `getName()`, `getQuality()`,
     *          `getRoot()`, `getBassNote()`, `getDegree()`, `stackSize()`, the `isXxx()` family,
     *          the `haveXxx()` predicates and every other method that stacks the chord in thirds
     *          throw on a chord containing a quarter tone, because all of them are defined over
     *          twelve-tone equal temperament. Calling this first rounds the quarter tones away
     *          (`Pitch::roundToSemitone()`'s ties-upward rule: `E1b4` -> `E4`, `E3b4` -> `Eb4`),
     *          so the analysis then runs on the rounded pitches.
     * @warning The rounding is destructive and in place: the original quarter-tone spelling is
     *          not recoverable from this Chord afterwards. Keep a copy if you need it.
     * @note Invalidates any cached stacked-in-thirds analysis, so the next analysis call sees the
     *       rounded pitches.
     * @return Number of notes whose pitch was rounded; 0 if the chord contained no quarter tone,
     *         in which case the chord is musically unchanged.
     */
    int roundQuarterTones();

    /**
     * @brief Invert the chord by moving the lowest note up by one octave, repeated inversionNumber
     * times.
     * @param inversionNumber Number of inversions to perform.
     */
    void toInversion(int inversionNumber);

    /**
     * @brief Transpose all notes in the chord by a number of semitones.
     * @details Each note keeps its own accidental as the preferred spelling of the result, and
     *          transposition is computed on exact pitch positions, so a quarter-tone chord
     *          transposes without losing its quarter tones.
     * @param semiTonesNumber Number of semitones to transpose; must be a multiple of 0.5.
     * @warning Transposing BY a quarter tone (any interval that is not a whole number of
     *          semitones) moves the chord off the semitone grid, so every harmonic-analysis
     *          method — getName(), getQuality(), the stacked-in-thirds family and the have*
     *          predicates — will then reject the chord, because all of them are defined over
     *          twelve-tone equal temperament. Call roundQuarterTones() first to analyse it.
     * @throws std::runtime_error If semiTonesNumber is not finite or not a multiple of 0.5, or if
     *         a transposed note falls outside the representable range or cannot be spelled within
     *         octaves -1..11 (see Helper::transposePitch(); a note never silently becomes a rest).
     *         Every note is transposed before any is stored, so the chord is left unchanged when
     *         this throws.
     */
    void transpose(const float semiTonesNumber);

    /**
     * @brief Transpose only the stacked (open) version of the chord by a number of semitones.
     * @param semiTonesNumber Number of semitones to transpose; must be a multiple of 0.5.
     * @warning Same off-the-grid caveat as transpose(); see its documentation.
     * @throws std::runtime_error In the same cases as transpose(), and with the same guarantee:
     *         the stack is left unchanged when this throws.
     */
    void transposeStackOnly(const float semiTonesNumber);

    /**
     * @brief Remove duplicate notes (by pitch) from the chord.
     * @details Sorts the notes (see sortNotes()), then removes each note spelled, at concert
     *          pitch, exactly as the note before it: a B-flat clarinet's written D4 duplicates a
     *          violin's C4, while C#4 and Db4 are both kept.
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
     * @details Calculates harmonic complexity by analyzing the intervallic structure of the chord
     *          in its closed-stack (root position) configuration. The complexity metric quantifies
     *          the degree of dissonance and intervallic tension based on:
     *          - Interval quality distribution (consonant vs. dissonant intervals)
     *          - Number of distinct interval classes present
     *          - Presence of augmented/diminished intervals (chromaticism indicator)
     *
     *          Returns a normalized value where:
     *          - 0.0 represents minimal complexity (e.g., perfect unisons, octaves, simple triads)
     *          - 1.0 represents maximal complexity (highly chromatic cluster chords, tone clusters)
     *
     *          When useEnharmony=true, enharmonic equivalents (e.g., C# ≡ Db) are treated as
     *          identical pitch classes, reducing perceived complexity in chromatic contexts.
     *
     *          This metric is useful for:
     *          - Quantifying harmonic tension in tonal and post-tonal music
     *          - Analyzing chord progressions for complexity contours
     *          - Identifying moments of harmonic density in scores
     *          - Comparative analysis of tertian vs. quartal/quintal harmony
     *
     * @note The closed-stack representation ensures transposition and inversion invariance
     *       for complexity calculations, enabling direct comparison across different voicings.
     */
    float getCloseStackHarmonicComplexity(const bool useEnharmony = false);

    /**
     * @brief Compute the harmonic density of the chord in a MIDI range.
     * @param lowerBoundMIDI Lowest MIDI note (default: -1, auto-detect).
     * @param higherBoundMIDI Highest MIDI note (default: -1, auto-detect).
     * @return Density as float.
     * @details Calculates the spatial distribution of notes within a specified MIDI pitch range,
     *          quantifying how densely packed the harmonic content is across the frequency
     * spectrum. The density metric reflects:
     *          - Average interval spacing between adjacent notes in ascending pitch order
     *          - Registral compression (cluster chords) vs. expansion (open voicings)
     *          - Spectral distribution of harmonic energy
     *
     *          When bounds are set to -1 (default), the function automatically detects the range
     *          using the chord's lowest and highest notes. Custom bounds enable:
     *          - Fixed-range density comparison across different chords
     *          - Orchestral register analysis (e.g., bass register density C1-C3)
     *          - Texture analysis within specific frequency bands
     *
     *          Higher density values indicate:
     *          - Close-voiced harmonies, tone clusters, chromatic saturation
     *          - Potential for increased psychoacoustic roughness (critical band interactions)
     *
     *          Lower density values indicate:
     *          - Open voicings, wide spacing, arpeggiated textures
     *          - Greater spectral clarity and reduced masking effects
     *
     *          Useful for analyzing orchestration, voice-leading efficiency, and harmonic texture.
     *
     *          When the range is auto-detected, the extremes are taken from exact pitch positions,
     *          so a quarter-tone extreme widens the range by half a semitone rather than being
     *          rounded: Chord{"C1x4", "G4"} spans 6.5 semitones, not 6. A float expresses that
     *          exactly, so this method never rejects a quarter-tone chord.
     * @note Calling this overload with NO arguments is ambiguous in C++, because both
     *       getHarmonicDensity() overloads have every parameter defaulted; pass both bounds
     *       explicitly (e.g. `getHarmonicDensity(-1, -1)`). Python is unaffected.
     */
    float getHarmonicDensity(int lowerBoundMIDI = -1, int higherBoundMIDI = -1) const;

    /**
     * @brief Compute the harmonic density of the chord in a pitch range.
     * @details The bounds are taken at their exact pitch positions, so a quarter-tone bound is not
     *          rounded: "C1x4" is 60.5, giving a span of 6.5 semitones to "G4", matching the
     *          numeric overload's auto-detected range rather than the 6 that rounding produces.
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
     * @brief Checks if the chord contains a perfect unison interval (e.g., two notes with the same
     * pitch).
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
     * @details Identifies the presence of the smallest diatonic interval, often associated with
     * dissonance.
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
     * @brief Checks if the chord contains a diminished octave interval (e.g., C and Cb an octave
     * apart).
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
     * @brief Checks if the chord contains an augmented octave interval (e.g., C and C# an octave
     * apart).
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
     * @brief Checks if the chord contains a perfect eleventh interval (e.g., C and F two octaves
     * apart).
     * @details Useful for identifying 11th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect eleventh is present.
     */
    bool havePerfectEleventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a sharp eleventh interval (e.g., C and F# two octaves
     * apart).
     * @details Useful for identifying altered dominants and Lydian harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a sharp eleventh is present.
     */
    bool haveSharpEleventh(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a minor thirteenth interval (e.g., C and Ab two octaves
     * plus a sixth apart).
     * @details Useful for identifying 13th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor thirteenth is present.
     */
    bool haveMinorThirdteenth(const bool useEnharmony = false);

    /**
     * @brief Checks if the chord contains a major thirteenth interval (e.g., C and A two octaves
     * plus a sixth apart).
     * @details Useful for identifying 13th chords and complex extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major thirteenth is present.
     */
    bool haveMajorThirdteenth(const bool useEnharmony = false);

    // ===== ABSTRACTION 2 ===== //

    /**
     * @brief Checks if the chord contains any second interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a second (major or minor) between
     * any two notes in the chord, regardless of octave. This is useful for identifying stepwise
     * relationships within the chord's structure.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a second interval is present, false otherwise.
     */
    bool haveSecond(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any third interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a third (major or minor) between
     * any two notes in the chord, regardless of octave. This is useful for identifying tertian
     * (third-based) relationships within the chord.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a third interval is present, false otherwise.
     */
    bool haveThird(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any fourth interval (perfect, augmented, or diminished)
     * between its notes.
     * @details Returns true if there is at least one interval of a fourth (of any quality) between
     * any two notes in the chord, regardless of octave. Useful for identifying quartal harmonies or
     * sus chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a fourth interval is present, false otherwise.
     */
    bool haveFourth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any fifth interval (perfect, augmented, or diminished)
     * between its notes.
     * @details Returns true if there is at least one interval of a fifth (of any quality) between
     * any two notes in the chord, regardless of octave. Useful for identifying quintal harmonies or
     * power chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a fifth interval is present, false otherwise.
     */
    bool haveFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any sixth interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a sixth (major or minor) between
     * any two notes in the chord, regardless of octave. Useful for identifying extended tertian
     * harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a sixth interval is present, false otherwise.
     */
    bool haveSixth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any seventh interval (major, minor, or diminished)
     * between its notes.
     * @details Returns true if there is at least one interval of a seventh (of any quality) between
     * any two notes in the chord, regardless of octave. Useful for identifying seventh chords and
     * complex harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a seventh interval is present, false otherwise.
     */
    bool haveSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any octave interval (perfect, augmented, or diminished)
     * between its notes.
     * @details Returns true if there is at least one interval of an octave (of any quality) between
     * any two notes in the chord. Useful for identifying doubled notes or octave relationships.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an octave interval is present, false otherwise.
     */
    bool haveOctave(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any ninth interval (major or minor) between its notes.
     * @details Returns true if there is at least one interval of a ninth (major or minor) between
     * any two notes in the chord. Useful for identifying extended harmonies such as 9th chords.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a ninth interval is present, false otherwise.
     */
    bool haveNinth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any eleventh interval (perfect or sharp) between its
     * notes.
     * @details Returns true if there is at least one interval of an eleventh (perfect or sharp)
     * between any two notes in the chord. Useful for identifying 11th chords and complex
     * extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an eleventh interval is present, false otherwise.
     */
    bool haveEleventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains any thirteenth interval (major or minor) between its
     * notes.
     * @details Returns true if there is at least one interval of a thirteenth (major or minor)
     * between any two notes in the chord. Useful for identifying 13th chords and complex
     * extensions.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a thirteenth interval is present, false otherwise.
     */
    bool haveThirdteenth(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 3 ===== //

    /**
     * @brief Checks if the chord contains a minor second interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a minor second interval, regardless of their
     * octave placement. This method abstracts away octave information and considers only pitch
     * class relationships. Useful for detecting clusters or dissonant relationships in any
     * register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor second (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major second interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a major second interval, regardless of their
     * octave placement. This method abstracts away octave information and considers only pitch
     * class relationships. Useful for detecting stepwise motion or clusters in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major second (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorSecond(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor third interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a minor third interval, regardless of their
     * octave placement. This method abstracts away octave information and considers only pitch
     * class relationships. Useful for detecting tertian relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor third (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorThird(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major third interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a major third interval, regardless of their
     * octave placement. This method abstracts away octave information and considers only pitch
     * class relationships. Useful for detecting tertian relationships in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major third (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorThird(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect fourth interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a perfect fourth interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting quartal harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect fourth (any octave) is present, false otherwise.
     */
    bool haveAnyOctavePerfectFourth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented fourth interval between any two notes,
     * ignoring octave differences.
     * @details Returns true if any pair of notes forms an augmented fourth (tritone) interval,
     * regardless of their octave placement. This method abstracts away octave information and
     * considers only pitch class relationships. Useful for detecting tritones in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented fourth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveAugmentedFourth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a diminished fifth interval between any two notes,
     * ignoring octave differences.
     * @details Returns true if any pair of notes forms a diminished fifth (tritone) interval,
     * regardless of their octave placement. This method abstracts away octave information and
     * considers only pitch class relationships. Useful for detecting tritones in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished fifth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveDiminishedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect fifth interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a perfect fifth interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting quintal harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect fifth (any octave) is present, false otherwise.
     */
    bool haveAnyOctavePerfectFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented fifth interval between any two notes,
     * ignoring octave differences.
     * @details Returns true if any pair of notes forms an augmented fifth interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting altered harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented fifth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveAugmentedFifth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor sixth interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a minor sixth interval, regardless of their
     * octave placement. This method abstracts away octave information and considers only pitch
     * class relationships. Useful for detecting extended tertian harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor sixth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major sixth interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a major sixth interval, regardless of their
     * octave placement. This method abstracts away octave information and considers only pitch
     * class relationships. Useful for detecting extended tertian harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major sixth (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorSixth(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a diminished seventh interval between any two notes,
     * ignoring octave differences.
     * @details Returns true if any pair of notes forms a diminished seventh interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting diminished harmonies in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished seventh (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveDiminishedSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a minor seventh interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a minor seventh interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting seventh chords in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a minor seventh (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMinorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a major seventh interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a major seventh interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting major seventh chords in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a major seventh (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveMajorSeventh(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a diminished octave interval between any two notes,
     * ignoring octave differences.
     * @details Returns true if any pair of notes forms a diminished octave interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting rare or microtonal relationships in any
     * register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a diminished octave (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveDiminishedOctave(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains a perfect octave interval between any two notes, ignoring
     * octave differences.
     * @details Returns true if any pair of notes forms a perfect octave interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting doubled notes in any register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if a perfect octave (any octave) is present, false otherwise.
     */
    bool haveAnyOctavePerfectOctave(const bool useEnharmony = false) const;

    /**
     * @brief Checks if the chord contains an augmented octave interval between any two notes,
     * ignoring octave differences.
     * @details Returns true if any pair of notes forms an augmented octave interval, regardless of
     * their octave placement. This method abstracts away octave information and considers only
     * pitch class relationships. Useful for detecting rare or microtonal relationships in any
     * register.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if an augmented octave (any octave) is present, false otherwise.
     */
    bool haveAnyOctaveAugmentedOctave(const bool useEnharmony = false) const;

    // ===== ABSTRACTION 4 ===== //

    /**
     * @brief Checks if the chord contains a generic second interval (major or minor, any octave).
     * @details Useful for identifying stepwise relationships between any two notes, regardless of
     * octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any second interval is present.
     */
    bool haveAnyOctaveSecond() const;

    /**
     * @brief Checks if the chord contains a generic third interval (major or minor, any octave).
     * @details Useful for identifying tertian relationships between any two notes, regardless of
     * octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any third interval is present.
     */
    bool haveAnyOctaveThird() const;

    /**
     * @brief Checks if the chord contains a generic fourth interval (perfect, augmented, or
     * diminished, any octave).
     * @details Useful for identifying quartal relationships between any two notes, regardless of
     * octave.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any fourth interval is present.
     */
    bool haveAnyOctaveFourth() const;

    /**
     * @brief Checks if the chord contains a generic fifth interval (perfect, augmented, or
     * diminished, any octave).
     * @details Useful for identifying quintal relationships between any two notes, regardless of
     * octave.
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
     * @brief Checks if the chord contains a generic seventh interval (major, minor, or diminished,
     * any octave).
     * @details Useful for identifying seventh chords or complex harmonies.
     * @param useEnharmony If true, considers enharmonic equivalents.
     * @return True if any seventh interval is present.
     */
    bool haveAnyOctaveSeventh() const;

    /**
     * @brief Checks if the chord contains a generic octave interval (perfect, augmented, or
     * diminished, any octave).
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
     * @details Suspended chords replace the third with a second or fourth, creating a
     * characteristic open sound.
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
     * @details Half-diminished chords contain a root, minor third, diminished fifth, and minor
     * seventh.
     * @return True if the chord is half-diminished.
     */
    bool isHalfDiminishedChord();

    /**
     * @brief Determines if the chord is a fully diminished seventh chord.
     * @details Fully diminished chords contain a root, minor third, diminished fifth, and
     * diminished seventh.
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
     * @brief Returns a string describing the chord quality (e.g., "major", "minor", "diminished",
     * "augmented", "sus", etc).
     * @details The quality is determined by the chord's interval structure and is useful for
     * harmonic analysis.
     * @return String representing the chord quality.
     */
    std::string getQuality();

    /**
     * @brief Checks if the notes in the chord are sorted in ascending order by pitch.
     * @details Useful for ensuring consistent interval calculations and for algorithms that require
     * sorted input.
     *
     *          Compares exact pitch positions, so a quarter tone orders correctly instead of being
     *          rounded onto the semitone above it: Chord{"E1b4", "E4"} is sorted, because 63.5
     *          precedes 64. Ordering is a predicate, so the bool returned expresses the true
     *          answer for a quarter-tone chord exactly and this method never rejects one.
     * @return True if the notes are sorted from lowest to highest pitch.
     */
    bool isSorted() const;

    /**
     * @brief Determines if the chord is tonal according to a given model or default rules.
     * @param model Optional: a function that takes a Chord and returns true if it is tonal.
     * @return True if the chord is considered tonal.
     * @details Evaluates whether the chord conforms to tonal harmonic principles based on its
     *          intervallic content in closed-stack configuration. Two evaluation modes:
     *
     *          **Default Model (model = nullptr)**:
     *          Analyzes all intervals in the closed stack and verifies they belong to the tonal
     *          interval inventory:
     *          - Consonances: perfect unison/octave (P1/P8), perfect fifth (P5), perfect fourth
     * (P4), major/minor thirds (M3/m3), major/minor sixths (M6/m6)
     *          - Diatonic dissonances: major/minor seconds (M2/m2), major/minor sevenths (M7/m7)
     *
     *          Returns false if ANY interval is:
     *          - Augmented or diminished (except diminished fifth in dominant seventh contexts)
     *          - Microtonal or non-12TET intervals
     *          - Extended jazz intervals beyond the 13th
     *
     *          **Custom Model**:
     *          Allows user-defined tonality criteria via callback function. Useful for:
     *          - Period-specific tonality rules (Renaissance vs. Romantic harmony)
     *          - Genre-specific harmonic syntax (jazz altered dominants, modal harmony)
     *          - Experimental tonal systems (extended tonality, pandiatonicism)
     *
     *          Applications:
     *          - Identifying tonal vs. atonal/post-tonal harmonic regions in scores
     *          - Filtering tonal subsets from chromatic harmonic progressions
     *          - Analyzing functional harmonic syntax vs. non-functional chromaticism
     *          - Historical style analysis (tonal practice across musical periods)
     *
     * @note This function operates on the closed-stack representation, ensuring consistent
     *       tonality evaluation regardless of chord voicing or register.
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
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to
     * each subsequent note. Otherwise, intervals are calculated between each pair of adjacent
     * notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of intervals in semitones.
     * @throws std::runtime_error If the chord contains a quarter tone. A vector of whole semitone
     *         counts cannot express the 3.5 semitones of a neutral third, and answering 3 or 4
     *         would be indistinguishable from a chord that really holds a minor or major third.
     *         Call roundQuarterTones() first, or use toCents(), which expresses a quarter tone
     *         exactly.
     */
    std::vector<int> getMidiIntervals(const bool firstNoteAsReference = false) const;

    /**
     * @brief Returns the intervals between notes in the chord as Interval objects.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to
     * each subsequent note. Otherwise, intervals are calculated between each pair of adjacent
     * notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of Interval objects.
     * @throws std::runtime_error If the chord has at least two notes and one is a quarter tone,
     *         which interval analysis cannot represent; the message names the note and
     *         roundQuarterTones().
     */
    std::vector<Interval> getIntervals(const bool firstNoteAsReference = false) const;

    /**
     * @brief Returns the intervals between the sorted original notes as Interval objects.
     * @details Useful for intervallic analysis independent of note order. The have*() interval
     *          predicates measured between adjacent notes are computed from these.
     * @return Vector of Interval objects between sorted notes.
     * @throws std::runtime_error If the chord has at least two notes and one is a quarter tone,
     *         which interval analysis cannot represent; the message names the note and
     *         roundQuarterTones().
     */
    std::vector<Interval> getIntervalsFromOriginalSortedNotes() const;

    /**
     * @brief Returns the intervals between notes in the open stack as Interval objects.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to
     * each subsequent note. Otherwise, intervals are calculated between each pair of adjacent
     * notes.
     * @param firstNoteAsReference If true, use the first note as the reference for all intervals.
     * @return Vector of Interval objects.
     */
    std::vector<Interval> getOpenStackIntervals(const bool firstNoteAsReference = false);

    /**
     * @brief Returns the intervals between notes in the closed stack as Interval objects.
     * @details If firstNoteAsReference is true, intervals are calculated from the first note to
     * each subsequent note. Otherwise, intervals are calculated between each pair of adjacent
     * notes.
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
     * @details Each note is shown at concert pitch, as the analysis relates it: a note of a
     *          transposing instrument at the pitch it sounds, spelled with its written letter moved
     *          by the diatonic transposing interval, inferred from the chromatic one when it is 0
     *          (a B-flat clarinet's written Db5 is shown Cb5, and so is a (0, -2) instrument's) --
     *          or, where that gives no spelling, by the fallback described on
     *          Note::getSoundingPitch(), without the simplification (a B-flat clarinet's written
     *          Cbb4 is shown Ab3) -- and an untransposed note as written. Useful for debugging and
     *          inspection.
     * @throws std::runtime_error If a note's sounding pitch lies below the lowest representable
     *         pitch, C1b-1 (see Note::getMidiNumber()).
     */
    void print() const;

    /**
     * @brief Prints the pitches of all notes in the open stack to the log.
     * @details The stack is the open stack of the last analysis (see getOpenStackNotes()) or, for
     *          a chord never analysed, the notes as they were added. A note added or removed since
     *          is not stacked until an analysis method runs again: addNote() and insertNote()
     *          append their note to the end of the stack, and a note removed with removeNote(),
     *          removeTopNote() or removeDuplicateNotes() stays in it. transpose(),
     *          transposeStackOnly(), roundQuarterTones() and clear() change the stack itself, so it
     *          shows their result at once. Each note is shown at concert pitch, as print() shows
     *          it. Useful for debugging and inspection of the stacked chord.
     * @throws std::runtime_error If a note of the stack sounds below the lowest representable
     *         pitch, C1b-1 (see Note::getMidiNumber()).
     */
    void printStack() const;

    /**
     * @brief Prints detailed information about the chord, including name, size, notes, and stack.
     * @details The notes and the stack are shown at concert pitch, as print() shows them. Useful
     *          for analysis and debugging.
     *
     *          On a chord containing a quarter tone this degrades rather than throwing, unlike
     *          every analysis method: it still prints the size and the note list, replaces the
     *          name with a note that the harmonic analysis is unavailable, skips the stack
     *          section, and names roundQuarterTones() as the way to enable the analysis. This is
     *          deliberate -- info() is the diagnostic a caller reaches for precisely when holding
     *          a chord they do not understand, so it must work on any chord that can be built.
     * @throws std::runtime_error If the chord (or, for an analysable chord, its stacked-in-thirds
     *         representation) is empty. Quarter tones are never the cause.
     * @throws std::runtime_error If a note's sounding pitch lies below the lowest representable
     *         pitch, C1b-1 (see Note::getMidiNumber()).
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
     * @brief Returns a Chord object representing the closed stack (stacked in thirds, closed
     * position).
     * @details Optionally considers enharmonic equivalents for stacking.
     * @param enharmonyNotes If true, considers enharmonic equivalents.
     * @return Chord object in closed stack form.
     */
    Chord getCloseStackChord(const bool enharmonyNotes = false);

    /**
     * @brief Returns a Chord object representing the closed chord, with octaves adjusted to match
     * the original.
     * @details Optionally considers enharmonic equivalents for stacking.
     * @param enharmonyNotes If true, considers enharmonic equivalents.
     * @return Chord object in closed form with original octaves.
     */
    Chord getCloseChord(const bool enharmonyNotes = false);

    /**
     * @brief Sorts the notes in the chord in ascending order by MIDI number.
     * @details Useful for ensuring consistent interval calculations and for algorithms that require
     * sorted input.
     */
    void sortNotes();

    /**
     * @brief Returns a vector with the intervals (in cents) between each pair of adjacent notes in
     * the chord.
     * @details Useful for microtonal and tuning analysis, as well as for comparing intervallic
     * content.
     *
     *          Cents are the one unit in this library that expresses a quarter tone exactly -- 50
     *          cents to the quarter tone, 350 to the neutral third -- so this method accepts a
     *          quarter-tone chord and computes the true value, unlike the MIDI-semitone methods,
     *          which reject one. Computed from the notes' exact sounding positions,
     *          Note::getQuarterToneSteps() (100 cents to the semitone in twelve-tone equal
     *          temperament), not from their frequencies, so it does not depend on freqA4. Every
     *          position is a multiple of 0.5, which a float holds exactly, so each difference
     *          times 100 is an exact whole number of cents, and converting it to int loses
     *          nothing.
     * @return Vector of integer values representing the interval in cents between each note pair.
     */
    std::vector<int> toCents() const;

    /**
     * @brief Returns the scale degree of the chord's root in a given key.
     * @details The degree is calculated relative to the provided Key object, considering enharmonic
     * spelling if requested.
     * @param key The Key object representing the tonal center.
     * @param enharmonyNotes If true, considers enharmonic equivalents for the root.
     * @return Integer representing the scale degree (1 = tonic, 2 = supertonic, etc).
     */
    int getDegree(const Key& key, bool enharmonyNotes = false);

    /**
     * @brief Returns the Roman numeral degree of the chord's root in a given key.
     * @details The degree is calculated relative to the provided Key object, considering enharmonic
     * spelling if requested.
     * @param key The Key object representing the tonal center.
     * @param enharmonyNotes If true, considers enharmonic equivalents for the root.
     * @return String with the Roman numeral (e.g., "I", "ii", "V").
     */
    std::string getRomanDegree(const Key& key, bool enharmonyNotes = false);

    /**
     * @brief Calculates the arithmetic mean of the frequencies of all notes in the chord.
     * @details Useful for spectral centroid and timbral analysis.
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Mean frequency as a float.
     */
    float getMeanFrequency(const float freqA4 = 440.0f) const;

    /**
     * @brief Calculates the mean frequency between the lowest and highest notes in the chord.
     * @details Useful for summarizing the chord's spectral span.
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Mean of extremes frequency as a float.
     */
    float getMeanOfExtremesFrequency(const float freqA4 = 440.0f) const;

    /**
     * @brief Calculates the standard deviation of the frequencies of all notes in the chord.
     * @details Useful for measuring the spectral spread or compactness of the chord. An empty chord
     *          returns 0.0f.
     * @note The frequencies are derived from the rounded MIDI number by Note::getFrequency(), so
     *       a quarter tone contributes the frequency of the semitone above it.
     * @param freqA4 Reference frequency for A4 (default: 440.0 Hz).
     * @return Frequency standard deviation as a float.
     */
    float getFrequencyStd(const float freqA4 = 440.0f) const;

    /**
     * @brief Calculates the arithmetic mean of the MIDI numbers of all notes in the chord.
     * @details Useful for pitch center analysis.
     * @return Mean MIDI value as an integer.
     * @throws std::runtime_error If the chord contains a quarter tone: an int cannot express the
     *         63.5 that {C4, E1b4, G4} averages to, and answering 63 would give the same value as
     *         a plain C major triad. Call roundQuarterTones() first.
     */
    int getMeanMidiValue() const;

    /**
     * @brief Calculates the mean MIDI value between the lowest and highest notes in the chord.
     * @details Useful for summarizing the pitch range of the chord.
     * @return Mean of extremes MIDI value as an integer.
     * @throws std::runtime_error If the chord contains a quarter tone, anywhere in it and not only
     *         at an extreme -- the extremes themselves are selected by an ordering that the
     *         rounding can get wrong. Call roundQuarterTones() first.
     */
    int getMeanOfExtremesMidiValue() const;

    /**
     * @brief Calculates the standard deviation of the MIDI numbers of all notes in the chord.
     * @details Useful for measuring the pitch spread or compactness of the chord.
     *
     *          Uses exact pitch positions, so a quarter tone contributes its true value; a float
     *          expresses that spread exactly and this method never rejects a quarter-tone chord.
     *          An empty chord returns 0.0f.
     * @return MIDI value standard deviation as a float.
     */
    float getMidiValueStd() const;

    /**
     * @brief Returns the pitch name corresponding to the mean MIDI value of the chord.
     * @details The accidental type can be specified (e.g., sharp, flat, natural).
     * @param accType Optional: specify accidental type for pitch spelling.
     * @return String with the pitch name (e.g., "C4", "F#3").
     * @throws std::runtime_error If the chord contains a quarter tone, inherited from
     *         getMeanMidiValue(), whose mean this method spells. The mean of a quarter-tone
     *         chord's exact pitch positions is generally not on the quarter-tone grid, so there is
     *         no exact spelling to return. Call roundQuarterTones() first.
     */
    std::string getMeanPitch(const std::string& accType = {}) const;

    /**
     * @brief Returns the pitch name corresponding to the mean of the lowest and highest MIDI values
     * in the chord.
     * @details The accidental type can be specified (e.g., sharp, flat, natural).
     * @param accType Optional: specify accidental type for pitch spelling.
     * @return String with the pitch name (e.g., "C4", "F#3").
     * @throws std::runtime_error If the chord contains a quarter tone, inherited from
     *         getMeanOfExtremesMidiValue(), which this method spells. Call roundQuarterTones()
     *         first.
     */
    std::string getMeanOfExtremesPitch(const std::string& accType = {}) const;

    /**
     * @brief Computes the combined harmonic spectrum of the chord by summing the spectra of all
     * notes.
     * @details Useful for timbral and psychoacoustic analysis.
     * @param numPartialsPerNote Number of partials to include for each note.
     * @param amplCallback Optional: function to modify amplitude vector.
     * @param partialsDecayExpRate Optional Partials decay exponential rate (default: 0.88).
     * @return Pair of vectors: frequencies and corresponding amplitudes.
     */
    std::pair<std::vector<float>, std::vector<float>> getHarmonicSpectrum(
        const int numPartialsPerNote = 6,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr,
        const float partialsDecayExpRate = 0.88f) const;

    /**
     * @brief Calculates the Sethares dissonance value for all dyads in the chord.
     * @param numPartials Number of partials per note.
     * @param useMinModel If true, uses the minimum amplitude model; otherwise, uses the product.
     * @param amplCallback Optional: function to modify amplitude vector.
     * @param partialsDecayExpRate Optional Partials decay exponential rate (default: 0.88).
     * @return Table with detailed dissonance information for each dyad.
     * @details Computes psychoacoustic sensory dissonance using the Sethares spectral dissonance
     * model, which quantifies roughness perception arising from critical band interactions between
     *          harmonic partials. The model calculates dissonance for all unique dyads (note pairs)
     *          in the chord by analyzing beating patterns and frequency proximity effects.
     *
     *          **Dissonance Calculation Process**:
     *          1. Generate harmonic spectra for each note (fundamental + overtones)
     *          2. For each dyad, compute pairwise partial interactions
     *          3. Apply critical band masking curves (Plomp-Levelt dissonance function)
     *          4. Weight contributions by partial amplitudes
     *          5. Aggregate dissonance values across all dyads
     *
     *          **Parameters**:
     *          - `numPartials`: Higher values increase spectral accuracy but computational cost.
     *            Typical range: 6-10 for realistic timbral modeling.
     *          - `useMinModel`: Amplitude weighting strategy
     *            - true: min(amp1, amp2) - assumes weaker partial dominates perception
     *            - false: amp1 × amp2 - assumes multiplicative interaction
     *          - `amplCallback`: Custom spectral envelope shaping (e.g., formant filtering,
     *            instrument-specific harmonic rolloff). Input/output: vector of partial amplitudes.
     *          - `partialsDecayExpRate`: Exponential decay rate for overtone amplitudes.
     *            Default 0.88 models typical harmonic decay (~-1.5 dB per partial).
     *
     *          **Return Value**:
     *          SetharesDissonanceTable containing per-dyad dissonance values, enabling:
     *          - Identification of most/least dissonant intervals in the chord
     *          - Voice-leading optimization (minimize dissonance motion)
     *          - Timbral dissonance profiling across different tuning systems
     *
     *          **Applications**:
     *          - Microtonal harmony analysis (optimal tuning selection)
     *          - Orchestration timbre studies (spectral clash identification)
     *          - Historical tuning system comparisons (just intonation vs. equal temperament)
     *          - Consonance/dissonance trajectory analysis in harmonic progressions
     *
     * @note This model reflects sensory (psychoacoustic) dissonance, NOT tonal/functional
     * dissonance. A dominant seventh chord (functionally dissonant in tonal theory) may have low
     * Sethares dissonance due to its harmonic series alignment.
     *
     * @warning Computational complexity is O(n² × p²) where n = number of notes, p = numPartials.
     *          Use sparingly in real-time applications for large chords.
     */
    SetharesDissonanceTable getSetharesDyadsDissonanceValue(
        const int numPartials = 6, const bool useMinModel = true,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr,
        const float partialsDecayExpRate = 0.88f) const;

    /**
     * @brief Calculates the total Sethares dissonance for the chord.
     * @details Aggregates the dissonance values for all dyads, optionally using a custom
     * aggregation function.
     * @param numPartialsPerNote Number of partials per note.
     * @param useMinModel If true, uses the minimum amplitude model; otherwise, uses the product.
     * @param amplCallback Optional: function to modify amplitude vector.
     * @param partialsDecayExpRate Optional Partials decay exponential rate (default: 0.88).
     * @param dissCallback Optional: function to aggregate dissonance values (e.g., mean, max).
     * @return Total dissonance as a float.
     */
    float getSetharesDissonance(
        const int numPartialsPerNote = 6, const bool useMinModel = true,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr,
        const float partialsDecayExpRate = 0.88f,
        const std::function<float(std::vector<float>)> dissCallback = nullptr) const;

    /**
     * @brief Array subscript operator for read-only access to notes in original (unsorted) order.
     * @param index Zero-based index of the note to retrieve (0 ≤ index < size()).
     * @return Const reference to the Note at the specified index position.
     * @throws std::out_of_range if index >= size().
     * @details Provides direct access to notes in their original input order, preserving
     *          the vertical arrangement as specified in MusicXML or programmatic construction.
     *          Does not reflect sorted or closed-stack ordering used in harmonic analysis.
     *          Useful for voice-leading analysis, pitch-space operations, and preserving
     *          registral relationships.
     */
    const Note& operator[](size_t index) const { return _originalNotes.at(index); }

    /**
     * @brief Array subscript operator for mutable access to notes in original (unsorted) order.
     * @param index Zero-based index of the note to retrieve (0 ≤ index < size()).
     * @return Mutable reference to the Note at the specified index position.
     * @throws std::out_of_range if index >= size().
     * @details Allows modification of notes while preserving their positional order.
     * @warning Changes made through the returned reference do NOT invalidate any previously
     *          computed harmonic analysis. Chord tracks a separate cache of stacked-in-thirds
     *          results (getName(), isTonal(), isInRootPosition(), getCloseStackIntervals(), the
     *          have*() family, ...) that every mutator method (addNote(), removeNote(),
     *          transpose(), clear(), ...) invalidates on your behalf — but a Note obtained
     *          through this operator is mutated directly, bypassing every mutator, so the cache
     *          is NOT recomputed on demand and will keep returning results for the notes as they
     *          were before this edit. Call a mutator (or reconstruct the Chord) after editing a
     *          Note this way if you need the cache to reflect the change.
     */
    Note& operator[](size_t index) { return _originalNotes.at(index); }

    /**
     * @brief Equality operator comparing chords by pitch-space note ordering.
     * @param otherChord Chord to compare against.
     * @return True if both chords contain identical notes in the same original order.
     * @details Performs pitch-wise equality comparison using the original (unsorted) note sequence.
     *          Two chords are considered equal if and only if:
     *          1. They contain the same number of notes (cardinality equality)
     *          2. Each corresponding note pair is equal (Note::operator==): the same pitch,
     *             spelled alike, a note of a transposing instrument taken at the pitch it sounds,
     *             spelled with its written letter moved by the diatonic transposing interval,
     *             inferred from the chromatic one when it is 0, so a chord holding a B-flat
     *             clarinet's or a (0, -2) instrument's written D4 equals one holding a C4 in its
     *             place -- or, where that gives no spelling, by the fallback described on
     *             Note::getSoundingPitch(), without the simplification (a B-flat clarinet's
     *             written Cbb4 is an Ab3)
     *
     *          This is a strict pitch-space comparison that preserves registral and voice-leading
     *          relationships. It does NOT compare:
     *          - Closed-stack or root-position equivalence
     *          - Pitch-class set equivalence (enharmonic equivalence)
     *          - Inversional or transpositional equivalence
     *
     *          For harmonic function equivalence analysis (e.g., comparing C-E-G vs. E-G-C as
     *          both being C major triads), use getPitchClassSet() or getChordName() instead.
     * @note Enharmonic notes (e.g., C# vs. Db) are NOT considered equal by this operator,
     *       as Note::operator== compares spellings.
     * @throws std::runtime_error If a note's sounding pitch lies below the lowest representable
     *         pitch, C1b-1 (see Note::getMidiNumber()).
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
     * @brief Inequality operator comparing chords by pitch-space note ordering.
     * @param otherChord Chord to compare against.
     * @return True if chords differ in size or contain non-identical notes at any position.
     * @details Logical negation of operator==. Returns true if chords are not pitch-wise identical
     *          in their original note ordering. Inequality holds if:
     *          1. Chord cardinalities differ (different number of notes), OR
     *          2. Any corresponding note pair differs (Note::operator!=, see operator==())
     *
     *          This is a strict pitch-space inequality preserving registral distinctions.
     *          Does NOT account for:
     *          - Harmonic function equivalence (e.g., inversions of the same chord)
     *          - Enharmonic equivalence (e.g., C# vs. Db)
     *          - Pitch-class set equivalence under transposition/inversion
     *
     *          For functional harmonic analysis, use chord quality/root comparison methods instead.
     * @note Two chords may be harmonically equivalent but still return true for inequality
     *       if their voicings differ (e.g., C4-E4-G4 vs. E4-G4-C5).
     * @throws std::runtime_error If a note's sounding pitch lies below the lowest representable
     *         pitch, C1b-1 (see Note::getMidiNumber()).
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
     * @brief Chord concatenation operator merging note collections in pitch-space order.
     * @param otherChord Chord whose notes will be appended to the current chord.
     * @return New Chord containing all notes from both chords in concatenated order.
     * @details Creates a composite chord by appending all notes from otherChord to a copy
     *          of the current chord's note sequence. The resulting chord preserves:
     *          - Original note ordering from the left operand (this chord)
     *          - Original note ordering from the right operand (appended notes)
     *          - All registral and voice-leading relationships
     *
     *          This operation is useful for:
     *          - Polychord construction (e.g., C major triad + F# major triad → bitonality)
     *          - Voice accumulation in orchestration analysis
     *          - Combining melodic and harmonic layers
     *          - Building complex harmonic structures from simpler tertian components
     *
     *          The operation does NOT:
     *          - Remove duplicate notes (allows octave doubling, unison reinforcement)
     *          - Sort notes by pitch (preserves original vertical ordering)
     *          - Merge enharmonic equivalents
     *
     * @note The resulting chord may contain duplicate pitch classes if both operands share
     *       common notes. Use Chord::removeDuplicates() if unique pitch-class collection
     *       is required for set-theoretic analysis.
     *
     * @example
     * @code
     * Chord triadC({"C4", "E4", "G4"});      // C major triad
     * Chord triadF({"F4", "A4", "C5"});      // F major triad
     * Chord poly = triadC + triadF;          // Polychord: C4-E4-G4-F4-A4-C5
     * @endcode
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
     * Prints the chord as a list of pitches, each at concert pitch, as print() shows it.
     * @param os Output stream.
     * @param chord Chord to print.
     * @return Reference to the output stream.
     * @throws std::runtime_error If a note's sounding pitch lies below the lowest representable
     *         pitch, C1b-1 (see Note::getMidiNumber()).
     */
    friend std::ostream& operator<<(std::ostream& os, const Chord& chord);
};