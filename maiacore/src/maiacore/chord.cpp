#include "maiacore/chord.h"

#include <algorithm>  // std::rotate, std::count
#include <cmath>      // std::sqrt
#include <iostream>
#include <map>
#include <optional>
#include <set>      // std::set
#include <utility>  // std::pair

#include "maiacore/constants.h"
#include "maiacore/duration.h"
#include "maiacore/helper.h"
#include "maiacore/interval.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"
#include "pitch-views.h"

using maiacore::detail::concertPitch;

namespace {
// An untransposed copy of 'note' at the pitch it sounds, spelled as the analyses relate it
// (concertPitch()), with the note's other attributes -- duration, voice, ties and so on --
// kept. An untransposed note, or a rest, is its own copy.
//
// The harmonic analysis stacks such copies, so every getter it reads on a stacked note -- and on
// every note it returns: the root, the bass note, the stacks and their heaps -- answers with the
// concert spelling, whatever instrument the chord's own note was written for.
Note concertCopy(const Note& note) {
    if (!note.isTransposed() || note.isNoteOff()) {
        return note;
    }

    // The interval is cleared first: setPitch() checks the new pitch with the note's interval.
    const Pitch concert = concertPitch(note);
    Note copy = note;
    copy.setTransposingInterval(0, 0);
    copy.setPitch(concert.getPitch());
    return copy;
}

// The pitch string a chord shows for 'note': its concert spelling, the one the analysis relates
// it by, so a chord is shown as it is analysed. An untransposed note is shown as written.
std::string concertName(const Note& note) { return concertPitch(note).getPitch(); }

// The note count divided by the span it occupies, in semitones, between two exact sounding
// positions (Note::getQuarterToneSteps()). Positions are multiples of 0.5, which float holds
// exactly, so the span is exact. The '+ 1' keeps the inclusive-semitone-slot convention of the
// numeric overload.
//
// Shared by both getHarmonicDensity() overloads: the string overload cannot delegate to the numeric
// one without rounding, because that one's parameters are ints.
float densityOverRange(const int numNotes, const float lowestSteps, const float highestSteps) {
    const float midiRange = (highestSteps - lowestSteps) + 1.0f;

    return static_cast<float>(numNotes) / midiRange;
}

// Rejects a quarter tone in a method whose RETURN TYPE cannot express one, in the style the
// analysis chokepoint in stackInThirds() established: the message names the offending note and the
// escape hatch, so a caller can act on it without consulting the documentation.
//
// These methods reach neither of the two analysis guards (Interval's and stackInThirds()'s): they
// never build an Interval and never stack the chord in thirds, they just do integer arithmetic on
// MIDI numbers. That is precisely why they need a guard of their own -- without it a quarter tone
// would be silently rounded to the semitone above, and the wrong answer would be
// indistinguishable from a right one (getMidiIntervals() would return [4, 3] for a triad with a
// neutral third, exactly as for a plain major triad).
//
// 'quantity' completes the sentence "Cannot compute <quantity> for a chord containing ...".
void rejectQuarterToneInMidiDomain(const Note* quarterToneNote, const std::string& quantity) {
    if (quarterToneNote == nullptr) {
        return;
    }

    LOG_ERROR("Cannot compute " + quantity + " for a chord containing the quarter tone " +
              quarterToneNote->getWrittenPitch() +
              ": this value is expressed in whole MIDI semitones, which cannot represent a quarter "
              "tone. Call Chord::roundQuarterTones() to round every quarter tone to the nearest "
              "semitone, then repeat the computation.");
}

// Rejects a quarter tone in the interval family: the methods that build an Interval from pairs of
// the chord's notes, getIntervals() and getIntervalsFromOriginalSortedNotes(), through which every
// have*() predicate measured between adjacent notes goes. Interval would reject the quarter tone
// too, but its message names Note::roundToSemitone(), a remedy a Chord caller cannot apply to the
// chord's own notes -- in Python, getNotes() returns copies -- so the chord names its own.
void rejectQuarterToneInIntervals(const Note* quarterToneNote) {
    if (quarterToneNote == nullptr) {
        return;
    }

    LOG_ERROR("Cannot compute the intervals of a chord containing the quarter tone " +
              quarterToneNote->getWrittenPitch() +
              ": interval analysis is defined only over twelve-tone equal temperament. Call "
              "Chord::roundQuarterTones() to round every quarter tone to the nearest semitone, "
              "then repeat the computation.");
}
}  // namespace

Chord::Chord() : _isStackedInThirds(false) {}

Chord::Chord(const std::vector<Note>& notes, const RhythmFigure rhythmFigure)
    : _isStackedInThirds(false) {
    // Each note is held untransposed at the pitch it sounds, spelled as the analyses relate it.
    for (const auto& n : notes) {
        const Note& note = Note(concertPitch(n).getPitch(), rhythmFigure);
        addNote(note);
    }
}

Chord::Chord(const std::vector<std::string>& pitches, const RhythmFigure rhythmFigure)
    : _isStackedInThirds(false) {
    for (const auto& p : pitches) {
        const Note& note = Note(p, rhythmFigure);
        addNote(note);
    }
}

Chord::~Chord() {}

void Chord::clear() {
    _originalNotes.clear();
    _openStack.clear();
    invalidateStackCache();
}

const Note* Chord::findQuarterToneNote() const {
    for (const auto& note : _originalNotes) {
        if (note.isQuarterTone()) {
            return &note;
        }
    }

    return nullptr;
}

int Chord::roundQuarterTones() {
    int numRoundedNotes = 0;

    for (auto& note : _originalNotes) {
        if (note.isQuarterTone()) {
            note.roundToSemitone();
            numRoundedNotes++;
        }
    }

    // '_originalNotes' and '_openStack' are bounded separately, for the same reason setDuration()
    // iterates them separately: stackInThirds() dedups '_openStack' down to one note per unique
    // pitch class, so it can be strictly smaller than '_originalNotes'. '_openStack' is rounded
    // here too rather than left alone because invalidateStackCache() deliberately does not clear
    // it (const readers such as printStack() may still read it before the next stack
    // computation), so skipping it would leave those readers showing the pre-round pitches.
    for (auto& note : _openStack) {
        if (note.isQuarterTone()) {
            note.roundToSemitone();
        }
    }

    // Unconditional, not 'if (numRoundedNotes > 0)': a note can acquire a quarter tone through
    // the mutable getNote()/operator[] references, which bypass every mutator and leave the cache
    // marked valid (see operator[]'s @warning in chord.h). Such a chord can reach here with a
    // stale cache, and -- if some other call already rounded it -- with nothing left to round, so
    // tying the invalidation to this call's own count could keep a stack computed for the
    // pre-round pitches. Invalidating regardless costs one recomputation and can never be wrong.
    invalidateStackCache();

    return numRoundedNotes;
}

void Chord::info() {
    // A chord containing a quarter tone cannot be analysed (see the guard in stackInThirds()),
    // but info() degrades instead of throwing: it is the diagnostic a caller reaches for
    // precisely when holding a chord they do not understand, so it must work on any chord that
    // can be built. Both of this method's routes into the analysis chokepoint -- getName() just
    // below and stackSize() further down -- are skipped, and everything that does not depend on
    // the stacked-in-thirds representation is still printed.
    const Note* quarterToneNote = findQuarterToneNote();

    if (quarterToneNote == nullptr) {
        LOG_INFO("Name: " << getName());
    } else {
        LOG_INFO("Name: <unavailable: the chord contains quarter tones>");
    }

    LOG_INFO("Size:" << size());

    const int chordSize = size();

    if (chordSize == 0) {
        LOG_ERROR("The chord is empty");
    }

    std::string noteNames = "[";

    for (int i = 0; i < chordSize - 1; i++) {
        noteNames.append(concertName(_originalNotes[i]) + ", ");
    }

    // Add the last note without the semicomma in the end
    noteNames.append(concertName(_originalNotes[chordSize - 1]));

    noteNames.append("]");

    LOG_INFO(noteNames);

    // The note list above is printed from '_originalNotes' and never touches the stack, so it is
    // still correct for a quarter-tone chord. Everything below it is the stacked-in-thirds
    // analysis, which is what the quarter tone makes unavailable: replace it with the reason and
    // the remedy rather than letting stackSize() throw out of a diagnostic method.
    if (quarterToneNote != nullptr) {
        LOG_INFO("=====> HARMONIC ANALYSIS UNAVAILABLE <=====");
        LOG_INFO("This chord contains the quarter tone "
                 << quarterToneNote->getWrittenPitch()
                 << ", which the harmonic analysis cannot represent. Call "
                    "Chord::roundQuarterTones() to round every quarter tone to the nearest "
                    "semitone, then call info() again.");
        return;
    }

    // ---------------- //
    const int chordStackSize = stackSize();

    LOG_INFO("=====> CHORD STACK <=====");
    LOG_INFO("Open Stack Size:" << chordStackSize);

    if (chordStackSize == 0) {
        LOG_ERROR("The chord stack is empty");
    }

    std::string stackNames = "[";

    for (int i = 0; i < chordStackSize - 1; i++) {
        stackNames.append(concertName(_openStack[i]) + ", ");
    }

    // Add the last note without the semicomma in the end
    stackNames.append(concertName(_openStack[chordStackSize - 1]));

    stackNames.append("]");

    LOG_INFO(stackNames);
}

void Chord::addNote(const Note& note) {
    // Skip rests
    if (note.isNoteOff()) {
        return;
    }

    // Add this note in the both chord versions
    _originalNotes.push_back(note);
    _openStack.push_back(note);

    if (size() == 1) {
        // The first note MUST be setted as inChord false
        _originalNotes[0].setIsInChord(false);
        _openStack[0].setIsInChord(false);
    } else {
        _originalNotes.back().setIsInChord(true);
        _openStack.back().setIsInChord(true);

        // const std::string& noteType = _originalNotes.begin()->getType();
        // _originalNotes.back().setType(noteType);
        // _stack.back().setType(noteType);
    }

    // Invalidate any previously-computed stacked-in-thirds representation
    invalidateStackCache();
}

void Chord::addNote(const std::string& pitch) { addNote(Note(pitch)); }

void Chord::removeTopNote() {
    if (_originalNotes.empty()) {
        LOG_ERROR("The chord is empty");
    }

    _originalNotes.pop_back();

    // Invalidate any previously-computed stacked-in-thirds representation
    invalidateStackCache();
}

void Chord::insertNote(Note& note, int noteIndex) {
    if (noteIndex < 0 || noteIndex > size()) {
        LOG_ERROR("Invalid note index: " + std::to_string(noteIndex));
    }

    note.setIsInChord(true);
    _originalNotes.insert(_originalNotes.begin() + noteIndex, note);

    _openStack.push_back(note);

    // Invalidate any previously-computed stacked-in-thirds representation
    invalidateStackCache();
}

void Chord::removeNote(int noteIndex) {
    if (noteIndex < 0 || noteIndex >= size()) {
        LOG_ERROR("Invalid note index: " + std::to_string(noteIndex));
    }

    _originalNotes.erase(_originalNotes.begin() + noteIndex);

    // Invalidate any previously-computed stacked-in-thirds representation
    invalidateStackCache();
}

void Chord::setDuration(const Duration& duration) {
    // '_originalNotes' and '_openStack' are bounded separately: stackInThirds() dedups
    // '_openStack' down to one note per unique pitch class, so it can be strictly smaller than
    // '_originalNotes' (e.g. after Chord({"C4", "C5"}).getName()). Indexing both by
    // '_originalNotes.size()' would write past the end of '_openStack' once that happens.
    const int chordSize = static_cast<int>(_originalNotes.size());
    for (int i = 0; i < chordSize; i++) {
        _originalNotes[i].setDuration(duration);
    }

    const int openStackSize = static_cast<int>(_openStack.size());
    for (int i = 0; i < openStackSize; i++) {
        _openStack[i].setDuration(duration);
    }
}

void Chord::setDuration(const float quarterDuration, const int divisionsPerQuarterNote) {
    // See the comment in the Duration& overload above: '_openStack' can be smaller than
    // '_originalNotes' after stacking, so the two are bounded separately.
    const int chordSize = static_cast<int>(_originalNotes.size());
    for (int i = 0; i < chordSize; i++) {
        _originalNotes[i].setDuration(quarterDuration, divisionsPerQuarterNote);
    }

    const int openStackSize = static_cast<int>(_openStack.size());
    for (int i = 0; i < openStackSize; i++) {
        _openStack[i].setDuration(quarterDuration, divisionsPerQuarterNote);
    }
}

// void Chord::setDurationTicks(const int durationTicks) {
//     const int chordSize = _originalNotes.size();

//     for (int i = 0; i < chordSize; i++) {
//         _originalNotes[i].setDurationTicks(durationTicks);
//         _openStack[i].setDurationTicks(durationTicks);
//     }
// }

// void Chord::setDuration(const RhythmFigure rhythmFigure, const int divisionsPerQuarterNote) {
//     const int chordSize = _originalNotes.size();

//     for (int i = 0; i < chordSize; i++) {
//         _originalNotes[i].setDuration(duration, divisionsPerQuarterNote);
//         _openStack[i].setDuration(duration, divisionsPerQuarterNote);
//     }
// }

// void Chord::setDuration(const float durationValue, const int lowerTimeSignatureValue,
//                         const int divisionsPerQuarterNote) {
//     const int chordSize = _originalNotes.size();

//     for (int i = 0; i < chordSize; i++) {
//         _originalNotes[i].setDuration(durationValue, lowerTimeSignatureValue,
//                                       divisionsPerQuarterNote);
//         _openStack[i].setDuration(durationValue, lowerTimeSignatureValue,
//         divisionsPerQuarterNote);
//     }
// }

void Chord::toInversion(int inversionNumber) {
    if (_originalNotes.empty()) {
        LOG_ERROR("The chord is empty");
    }

    for (int i = 0; i < inversionNumber; i++) {
        _originalNotes[0].transpose(12);  // isto apenas altera a nota uma oitava acima

        const Note& x = _originalNotes[0];  // pega a nota que foi alterada acima
        _originalNotes.push_back(x);
        _originalNotes.erase(_originalNotes.begin());
    }

    // The pitch content and order of '_originalNotes' both changed (when inversionNumber > 0):
    // any previously-computed stacked-in-thirds representation no longer matches.
    if (inversionNumber > 0) {
        invalidateStackCache();
    }
}

namespace {
// The accidental type Chord::transpose() and Chord::transposeStackOnly() prefer when spelling a
// transposed note: the note's own accidental, so a chord written with flats stays written with
// flats.
//
// A quarter-tone accidental ("1x", "1b", "3x", "3b") is not one of the five spellings
// Helper::midiNote2pitch() accepts, so those fall back to the default spelling of the base
// semitone. The quarter tone itself is never lost by that fallback: Helper::steps2pitch()
// re-applies it to whichever base spelling comes back.
std::string preferredAccType(const std::string& pitch) {
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    std::optional<int> octave;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

    const bool isSpellableAccType = alterSymbol == MUSIC_XML::ACCIDENT::NONE ||
                                    alterSymbol == MUSIC_XML::ACCIDENT::SHARP ||
                                    alterSymbol == MUSIC_XML::ACCIDENT::FLAT ||
                                    alterSymbol == MUSIC_XML::ACCIDENT::DOUBLE_SHARP ||
                                    alterSymbol == MUSIC_XML::ACCIDENT::DOUBLE_FLAT;

    return isSpellableAccType ? alterSymbol : MUSIC_XML::ACCIDENT::NONE;
}

// Transposes a COPY of 'notes' and returns it, so a rejection part-way through -- a note that
// transposes outside the representable range raises -- leaves the caller's notes untouched.
// Transposing in place would leave a chord half-transposed when a later note raised:
// {"C4", "B11"} transposed by 2 would be left as {"D4", "B11"}.
std::vector<Note> transposedCopy(const std::vector<Note>& notes, const float semitonesNumber) {
    std::vector<Note> transposed = notes;
    for (auto& note : transposed) {
        const std::string pitch = note.getWrittenPitch();
        note.setPitch(Helper::transposePitch(pitch, semitonesNumber, preferredAccType(pitch)));
    }
    return transposed;
}
}  // namespace

void Chord::transpose(const float semitonesNumber) {
    // Validated before the zero check, so an off-the-grid interval is rejected even for an empty
    // chord, where the loop below would never run to reject it.
    Helper::validateTransposeSemitones(semitonesNumber);

    if (semitonesNumber == 0.0f) {
        return;
    }

    // Transposes through Helper::transposePitch(), the single implementation that
    // Note::transpose() uses too, so transposing a Chord and transposing a Note cannot diverge.
    // It computes on exact pitch positions, so a quarter tone is not rounded away before the
    // interval is applied.
    //
    // Both halves are computed before either is committed, so a rejection leaves the whole chord
    // -- original notes and stack alike -- exactly as it was.
    std::vector<Note> transposedNotes = transposedCopy(_originalNotes, semitonesNumber);
    std::vector<Note> transposedStack = transposedCopy(_openStack, semitonesNumber);

    _originalNotes = std::move(transposedNotes);
    _openStack = std::move(transposedStack);

    // The pitch content of '_originalNotes' changed: any previously-computed stacked-in-thirds
    // representation (computed from the pre-transpose pitches) no longer matches.
    invalidateStackCache();
}

void Chord::transposeStackOnly(const float semitonesNumber) {
    Helper::validateTransposeSemitones(semitonesNumber);

    if (semitonesNumber == 0.0f) {
        return;
    }

    // Transpose the stack version, through the same single implementation transpose() uses, and
    // with the same all-or-nothing commit.
    _openStack = transposedCopy(_openStack, semitonesNumber);
}

void Chord::removeDuplicateNotes() {
    sortNotes();  // also invalidates the stack cache; the erase() below only shrinks further

    // Two notes are duplicates when they sound the same pitch, spelled as the analyses relate it:
    // a B-flat clarinet's written D4 duplicates a violin's C4.
    const auto sameConcertPitch = [](const Note& a, const Note& b) {
        return concertPitch(a) == concertPitch(b);
    };
    _originalNotes.erase(
        std::unique(_originalNotes.begin(), _originalNotes.end(), sameConcertPitch),
        _originalNotes.end());
}

std::vector<HeapData> Chord::getStackedHeaps(const bool enharmonyNotes) {
    if (!_isStackedInThirds) {
        stackInThirds(enharmonyNotes);
    }

    return _stackedHeaps;
}

std::string Chord::getDuration() const {
    const std::map<std::string, int> map{
        {MUSIC_XML::NOTE_TYPE::MAXIMA, 32000000}, {MUSIC_XML::NOTE_TYPE::LONG, 16000000},
        {MUSIC_XML::NOTE_TYPE::BREVE, 8000000},   {MUSIC_XML::NOTE_TYPE::WHOLE, 4000000},
        {MUSIC_XML::NOTE_TYPE::HALF, 2000000},    {MUSIC_XML::NOTE_TYPE::QUARTER, 1000000},  // 1:1
        {MUSIC_XML::NOTE_TYPE::EIGHTH, 500000},   {MUSIC_XML::NOTE_TYPE::N16TH, 250000},
        {MUSIC_XML::NOTE_TYPE::N32ND, 125000},    {MUSIC_XML::NOTE_TYPE::N64TH, 62500},
        {MUSIC_XML::NOTE_TYPE::N128TH, 31250},    {MUSIC_XML::NOTE_TYPE::N256TH, 15625},
        {MUSIC_XML::NOTE_TYPE::N512TH, 7813},     {MUSIC_XML::NOTE_TYPE::N1024TH, 3906}};

    int minDuration = Helper::noteType2ticks(MUSIC_XML::NOTE_TYPE::MAXIMA, 1000000);
    for (const auto& note : _originalNotes) {
        const auto noteType = note.getType();

        const int durationMap = map.at(noteType);

        if (durationMap < minDuration) {
            minDuration = durationMap;
        }
    }

    std::string keyNote;
    for (const auto& el : map) {
        if (el.second == minDuration) {
            keyNote = el.first;
            break;  // to stop searching
        }
    }

    return keyNote;
}

float Chord::getQuarterDuration() const {
    std::string duration = getDuration();
    const int ticks = Helper::noteType2ticks(duration, 256);

    return static_cast<float>(ticks) / 256.0f;
}

int Chord::size() const { return static_cast<int>(_originalNotes.size()); }

int Chord::getDurationTicks() const {
    // Error check
    if (_originalNotes.empty()) {
        return 0;
    }

    // For each internal note, get the minimum duration ticks value
    int minValue = _originalNotes[0].getDurationTicks();
    for (const auto& note : _originalNotes) {
        if (note.getDurationTicks() < minValue) {
            minValue = note.getDurationTicks();
        }
    }

    return minValue;
}

Note& Chord::getNote(int noteIndex) {
    if (noteIndex < 0 || noteIndex >= size()) {
        LOG_ERROR("Invalid note index: " + std::to_string(noteIndex));
    }

    return _originalNotes[noteIndex];
}

const Note& Chord::getNote(const int noteIndex) const {
    if (noteIndex < 0 || noteIndex >= size()) {
        LOG_ERROR("Invalid note index: " + std::to_string(noteIndex));
    }

    return _originalNotes[noteIndex];
}

void Chord::print() const {
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        LOG_INFO("note[" << i << "] = " << concertName(_originalNotes[i]));
    }
}

void Chord::printStack() const {
    const int stackSize = _openStack.size();

    for (int i = 0; i < stackSize; i++) {
        LOG_INFO("openStack[" << i << "] = " << concertName(_openStack[i]));
    }
}

void Chord::stackInThirds(const bool enharmonyNotes) {
    ignore(enharmonyNotes);

    // ===== STEP 0: INPUT VALIDATION ===== //
    // Error checking:
    if (_originalNotes.empty()) {
        // LOG_WARN("The chord is empty!");
        return;
    }

    // Single chokepoint for this class's whole harmonic-analysis surface. Every analysis method
    // opens with `if (!_isStackedInThirds) { stackInThirds(...); }`, so rejecting here covers
    // getName(), getQuality(), getRoot(), getBassNote(), getDegree(), getStackedHeaps(),
    // stackSize(), the isXxx() family, the 26 haveXxx() predicates and the interval getters at
    // once, without a guard in any of them.
    //
    // It is an outright rejection rather than a best effort because all of those answers -- the
    // thirds, the fifths, the chord quality, the scale degrees -- are defined over twelve-tone
    // equal temperament. Given a quarter tone the analysis does not fail: it returns a confident
    // wrong answer, which in an analysis library is worse than an error. The message names
    // roundQuarterTones(), the escape hatch for callers who want the analysis anyway.
    //
    // Deliberately NOT covered by this guard: info(), which degrades instead of throwing, and
    // every plain accessor and mutator (size(), getNote(), getNotes(), getDuration(),
    // setDuration(), addNote(), removeNote(), ...), none of which touches the stack -- they must
    // keep working on a quarter-tone chord.
    const Note* quarterToneNote = findQuarterToneNote();
    if (quarterToneNote != nullptr) {
        LOG_ERROR("Cannot analyse a chord containing the quarter tone " +
                  quarterToneNote->getWrittenPitch() +
                  ": harmonic analysis is defined only over twelve-tone equal temperament. Call "
                  "Chord::roundQuarterTones() to round every quarter tone to the nearest "
                  "semitone, then repeat the analysis.");
    }

    // ===== STEP 1: COMPUTE THE OPEN STACK ===== //
    // The stack holds an untransposed copy of each note at the pitch it sounds (concertCopy()).
    _openStack.clear();
    _openStack.reserve(_originalNotes.size());
    for (const Note& note : _originalNotes) {
        _openStack.push_back(concertCopy(note));
    }

    // '_stackedHeaps' is a member, populated by push_back() further down (STEP 5). It must start
    // empty on every run: without this, a chord re-stacked after a mutation (addNote, removeNote,
    // ...) keeps appending onto the previous run's heaps, so computeBestOpenStackHeap() can select
    // a heap sized for the OLD note count -- an out-of-bounds read once _closeStack/_openStack no
    // longer agree with the current chord size. Every mutator also clears this via
    // invalidateStackCache() before stackInThirds() is ever re-entered; this is the belt-and-braces
    // half, so stackInThirds() is correct on its own even if some future mutator forgets to.
    _stackedHeaps.clear();

    // ===== STEP 1.1: GET THE BASS NOTE ===== //
    // Sort chord notes using the MIDI note value
    std::sort(_openStack.begin(), _openStack.end());

    // Get the bass note of the chord
    _bassNote = _openStack[0];

    // ===== STEP 1.2: FILTER UNIQUE PITCH CLASS NOTES ===== //
    // Remove duplacated notes (pitchClass)
    std::vector<int> myHashs;
    myHashs.reserve(_openStack.size());

    for (const auto& note : _openStack) {
        int noteHash = hash(note.getPitchClass().c_str());
        myHashs.push_back(noteHash);
    }

    std::sort(myHashs.begin(), myHashs.end());
    myHashs.erase(std::unique(myHashs.begin(), myHashs.end()), myHashs.end());

    const int uniquePitchClasses = myHashs.size();

    std::vector<Note> uniqueNotes;
    uniqueNotes.reserve(uniquePitchClasses);

    for (const auto& note : _openStack) {
        const int noteHash = hash(note.getPitchClass().c_str());
        if (std::find(myHashs.begin(), myHashs.end(), noteHash) != myHashs.end()) {
            uniqueNotes.push_back(note);
            myHashs.erase(std::remove(myHashs.begin(), myHashs.end(), noteHash), myHashs.end());
        }
    }

    // ===== STEP 1.3: OVERRIDE THE 'OPEN STACK' VECTOR WITH UNIQUE NOTE PITCH CLASSES ===== //
    _openStack = uniqueNotes;

    // Define the current 'chordSize'
    const int chordSize = _openStack.size();

    // ===== ERROR CHECKING ===== //
    // Empty chord
    if (chordSize == 0) {
        _isStackedInThirds = true;
        _closeStack.clear();
        return;
    }
    // Single note chord
    if (chordSize == 1) {
        _isStackedInThirds = true;
        _closeStack.clear();
        _closeStack.push_back(_openStack[0]);
        return;
    }

    // ===== STEP 2: COMPUTE ENHARMONIC UNIT GROUPS ===== //
    std::vector<NoteDataHeap> enharmonicUnitGroups = computeEnharmonicUnitsGroups();

    // ===== STEP 3: COMPUTE ALL ENHARMONIC HEAPS (VALID AND INVALID HEAPS)
    std::vector<NoteDataHeap> allEnharmonicHeaps = computeEnharmonicHeaps(enharmonicUnitGroups);

    // ===== STEP 4: FILTER HEAPS THAT DO NOT CONTAIN INTERNAL DUPLICATED PITCH
    // STEPS ===== //
    std::vector<NoteDataHeap> validEnharmonicHeaps =
        removeHeapsWithDuplicatedPitchSteps(allEnharmonicHeaps);
    // ===== STEP 5: COMPUTE THE STACK IN THIRDS TEMPLATE MATCH ===== //
    const int possibleNumberOfHeapInvertions = 4;
    _stackedHeaps.reserve(validEnharmonicHeaps.size() * possibleNumberOfHeapInvertions);
    for (auto& heap : validEnharmonicHeaps) {
        const std::vector<NoteDataHeap> heapInversions = computeAllHeapInversions(heap);
        // std::cout << "  C2: heapInversions: " << heapInversions.size() << std::endl;
        std::vector<NoteDataHeap> tertianHeaps = filterTertianHeapsOnly(heapInversions);
        // std::cout << "  C3: tertianHeaps: " << tertianHeaps.size() << std::endl;
        for (auto& heapInversion : tertianHeaps) {
            // Check if the first heap interval is a musical third
            const auto& firstNote = heapInversion[0].note;
            const auto& secondNote = heapInversion[1].note;
            const Interval firstInterval(firstNote, secondNote);

            // Skip chords without the tonal major/minor third
            if (firstInterval.getPitchStepInterval() != 3) {
                continue;
            }

            const auto& heapData = stackInThirdsTemplateMatch(heapInversion);
            _stackedHeaps.push_back(heapData);
        }
    }

    // ===== STEP 6: SORT HEAPS BY STACK IN THIRDS MATCHING VALUE ===== //
    std::sort(_stackedHeaps.begin(), _stackedHeaps.end(), std::greater<>());

    // ===== STEP 7: COMPUTE THE CLOSE STACK VERSION ===== //
    const auto bestStackedHeapNotes = computeBestOpenStackHeap(_stackedHeaps);
    computeCloseStack(bestStackedHeapNotes);

    // ===== STEP 8: SET INTERNAL FLAG AND COMPUTE INTERVALS ===== //
    _isStackedInThirds = true;
    _closeStackintervals = Helper::notes2Intervals(_closeStack, true);
}

std::vector<NoteDataHeap> Chord::computeEnharmonicUnitsGroups() const {
    const int chordSize = _openStack.size();
    std::vector<NoteDataHeap> enharUnitGroups(chordSize);
    const int numUnitGroups = enharUnitGroups.size();

    const int numOfEnharmonics = 3;

    for (int i = 0; i < numUnitGroups; i++) {
        NoteDataHeap unitGroup(numOfEnharmonics);

        Note originalNote = _openStack[i];  // must be a copy
        unitGroup[0] = NoteData(originalNote, false, 0);
        unitGroup[1] = NoteData(originalNote.getEnharmonicNote(false), true, 1);
        unitGroup[2] = NoteData(originalNote.getEnharmonicNote(true), true, 2);

        enharUnitGroups[i] = unitGroup;
    }

    return enharUnitGroups;
}

std::vector<NoteDataHeap> Chord::computeEnharmonicHeaps(
    const std::vector<NoteDataHeap>& heaps) const {
    const int chordSize = heaps.size();

    const int numEnharmonicHeapVariants = std::pow(3, chordSize);

    std::vector<NoteDataHeap> enharmonicHeaps(numEnharmonicHeapVariants);
    int idx = 0;
    const int numOfEnharmonics = 3;

    switch (chordSize) {
        case 2:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2]}};
                }
            }
            break;

        case 3:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    for (int i3 = 0; i3 < numOfEnharmonics; i3++) {
                        enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2], heaps[2][i3]}};
                    }
                }
            }
            break;

        case 4:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    for (int i3 = 0; i3 < numOfEnharmonics; i3++) {
                        for (int i4 = 0; i4 < numOfEnharmonics; i4++) {
                            enharmonicHeaps[idx++] = {
                                {heaps[0][i1], heaps[1][i2], heaps[2][i3], heaps[3][i4]}};
                        }
                    }
                }
            }
            break;

        case 5:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    for (int i3 = 0; i3 < numOfEnharmonics; i3++) {
                        for (int i4 = 0; i4 < numOfEnharmonics; i4++) {
                            for (int i5 = 0; i5 < numOfEnharmonics; i5++) {
                                enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2], heaps[2][i3],
                                                           heaps[3][i4], heaps[4][i5]}};
                            }
                        }
                    }
                }
            }
            break;

        case 6:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    for (int i3 = 0; i3 < numOfEnharmonics; i3++) {
                        for (int i4 = 0; i4 < numOfEnharmonics; i4++) {
                            for (int i5 = 0; i5 < numOfEnharmonics; i5++) {
                                for (int i6 = 0; i6 < numOfEnharmonics; i6++) {
                                    enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2],
                                                               heaps[2][i3], heaps[3][i4],
                                                               heaps[4][i5], heaps[5][i6]}};
                                }
                            }
                        }
                    }
                }
            }
            break;

        case 7:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    for (int i3 = 0; i3 < numOfEnharmonics; i3++) {
                        for (int i4 = 0; i4 < numOfEnharmonics; i4++) {
                            for (int i5 = 0; i5 < numOfEnharmonics; i5++) {
                                for (int i6 = 0; i6 < numOfEnharmonics; i6++) {
                                    for (int i7 = 0; i7 < numOfEnharmonics; i7++) {
                                        enharmonicHeaps[idx++] = {
                                            {heaps[0][i1], heaps[1][i2], heaps[2][i3], heaps[3][i4],
                                             heaps[4][i5], heaps[5][i6], heaps[6][i7]}};
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 8:
            for (int i1 = 0; i1 < numOfEnharmonics; i1++) {
                for (int i2 = 0; i2 < numOfEnharmonics; i2++) {
                    for (int i3 = 0; i3 < numOfEnharmonics; i3++) {
                        for (int i4 = 0; i4 < numOfEnharmonics; i4++) {
                            for (int i5 = 0; i5 < numOfEnharmonics; i5++) {
                                for (int i6 = 0; i6 < numOfEnharmonics; i6++) {
                                    for (int i7 = 0; i7 < numOfEnharmonics; i7++) {
                                        for (int i8 = 0; i8 < numOfEnharmonics; i8++) {
                                            enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2],
                                                                       heaps[2][i3], heaps[3][i4],
                                                                       heaps[4][i5], heaps[5][i6],
                                                                       heaps[6][i7], heaps[7][i8]}};
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;

        default:
            LOG_ERROR("Invalid chord size: " + std::to_string(chordSize));
            break;
    }

    return enharmonicHeaps;
}

std::vector<NoteDataHeap> Chord::removeHeapsWithDuplicatedPitchSteps(
    std::vector<NoteDataHeap>& heaps) const {
    const int heapsSize = heaps.size();
    // std::vector<NoteDataHeap> validEnharmonicHeaps(heapsSize);
    std::vector<NoteDataHeap> validEnharmonicHeaps;
    validEnharmonicHeaps.reserve(heapsSize);

    // int idx = 0;
    for (auto& heap : heaps) {
        // Sort heap notes alphabetically
        std::sort(heap.begin(), heap.end(), [](const NoteData& a, const NoteData& b) {
            return a.note.getPitchStep() < b.note.getPitchStep();
        });

        // Check if all heap notes are pitch step unique
        auto x = std::unique(heap.begin(), heap.end(), [](const NoteData& a, const NoteData& b) {
            return a.note.getPitchStep() == b.note.getPitchStep();
        });

        // Skip the heaps that contains the same pitch step notes
        if (x != heap.end()) {
            continue;
        }

        // Add current heap to the output vector
        // validEnharmonicHeaps[idx++] = heap;
        validEnharmonicHeaps.push_back(heap);
        // idx++;
    }

    // validEnharmonicHeaps.resize(idx - 1);

    return validEnharmonicHeaps;
}

std::vector<NoteDataHeap> Chord::computeAllHeapInversions(NoteDataHeap& heap) const {
    std::vector<NoteDataHeap> heapInversions(factorial(heap.size()));
    int i = 0;

    std::sort(heap.begin(), heap.end());
    do {
        heapInversions[i++] = heap;
    } while (std::next_permutation(heap.begin(), heap.end()));

    return heapInversions;
}

std::vector<NoteDataHeap> Chord::filterTertianHeapsOnly(
    const std::vector<NoteDataHeap>& heaps) const {
    if (heaps.empty()) {
        LOG_ERROR("Heaps vector is empty");
    }

    const int heapSize = heaps[0].size();
    // std::cout << " | heapSize: " << heapSize << std::endl;
    std::vector<NoteDataHeap> tertianHeaps;
    tertianHeaps.reserve(heapSize);
    // int k = 0;
    for (const auto& heap : heaps) {
        bool isATertianHeap = true;

        if (heap.empty()) {
            continue;
        }

        for (int i = 0; i < heapSize - 1; i++) {
            const Note& currentNote = heap[i].note;
            const Note& nextNote = heap[i + 1].note;
            // std::cout << "heap[" << k << "][" << i << "/" << heapSize - 1
            //           << "]: " << currentNote.getPitch() << " " << nextNote.getPitch() <<
            //           std::endl;

            Interval interval(currentNote, nextNote);
            // std::cout << "    intervalName: " << interval.getName() << std::endl;
            const int pitchStepInterval = interval.getPitchStepInterval();
            // std::cout << "    pitchStepInterval: " << pitchStepInterval << std::endl;

            if ((pitchStepInterval != 3) && (pitchStepInterval != 5)) {
                isATertianHeap = false;
                break;
            }
        }
        // k++;

        if (isATertianHeap) {
            tertianHeaps.push_back(heap);
        }
    }

    return tertianHeaps;
}

std::vector<Note> Chord::computeBestOpenStackHeap(std::vector<HeapData>& stackedHeaps) {
    // 'stackedHeaps' (built in stackInThirds()'s STEP 5) can be empty. Each note has at most 3
    // enharmonic spellings (itself + 2 alternates, see Note::getEnharmonicPitch), drawn from only
    // 7 possible pitch letters (A-G), and removeHeapsWithDuplicatedPitchSteps() rejects any
    // respelling where two notes land on the same letter -- so this is reachable whenever no
    // respelling can give every note a distinct letter *and* have the resulting heap satisfy the
    // stacked-in-thirds interval pattern (STEP 5 only keeps a heap whose first interval, after
    // respelling, is an exact third).
    //
    // This is NOT limited to large chords: pitch classes are de-duplicated by spelling string
    // (see hash(note.getPitchClass()) a few dozen lines up), not by enharmonic-equivalent pitch,
    // so e.g. C, C#, Db and B# are four distinct pitch classes that all draw their letters from
    // just {B, C, D}. A 3-note chord as ordinary as {C4, C#4, Db4} already reaches this guard --
    // there is no simple "N distinct pitch classes" threshold; whether a valid heap exists depends
    // on the specific notes involved, not just how many there are. (A chord with more than 8
    // distinct pitch classes does throw earlier and unconditionally, in computeEnharmonicHeaps()'s
    // "Invalid chord size" case -- that part of the size argument holds -- but plenty of chords
    // well under 8 notes reach this exact guard too.)
    if (stackedHeaps.empty()) {
        LOG_ERROR("Unable to find a valid stacked-in-thirds heap for this chord");
    }

    const float bestStackMatchValue = std::get<1>(stackedHeaps[0]);
    const int heapSize = std::get<0>(stackedHeaps[0]).size();

    int swapHeapDataIdx = 0;
    bool foundHeapMatch = false;

    // The heaps are spelled from the stack's concert copies, so the chord's own notes are compared
    // with them by their concert spellings too.
    std::vector<std::string> originalNotesPitchClass(heapSize);
    for (int i = 0; i < heapSize; i++) {
        originalNotesPitchClass[i] = concertPitch(_originalNotes[i]).getPitchClass();
    }

    for (const auto& heapData : stackedHeaps) {
        const float currentMatchValue = std::get<1>(heapData);
        if (!isFloatEqual(bestStackMatchValue, currentMatchValue)) {
            break;  // Exit loop
        }

        const auto& heap = std::get<0>(heapData);
        int pitchClassMatchSum = 0;
        for (int i = 0; i < heapSize; i++) {
            const Note& note = heap[i].note;

            if (!std::count(originalNotesPitchClass.begin(), originalNotesPitchClass.end(),
                            note.getPitchClass())) {
                break;  // Exit inner loop
            }

            pitchClassMatchSum++;
        }

        if (pitchClassMatchSum == heapSize) {
            foundHeapMatch = true;
            // this heap is the beast match value and also
            // matchs with the original user informed notes
            break;
        }

        swapHeapDataIdx++;
    }

    if (foundHeapMatch) {
        std::swap(stackedHeaps[0], stackedHeaps[swapHeapDataIdx]);
    }

    const HeapData& bestStackedHeap = stackedHeaps[0];
    const NoteDataHeap& heap = std::get<0>(bestStackedHeap);
    std::vector<Note> bestStackedHeapNotes(heapSize);
    for (int i = 0; i < heapSize; i++) {
        bestStackedHeapNotes[i] = heap[i].note;
    }

    return bestStackedHeapNotes;
}

void Chord::computeCloseStack(const std::vector<Note>& openStack) {
    _closeStack = openStack;
    const Note& rootNote = _closeStack[0];

    const std::map<char, int>* closeStackOctavesMap = nullptr;

    switch (hash(rootNote.getPitchStep().c_str())) {
        case hash("C"):
            closeStackOctavesMap = &c_C_closeStackOctavesMap;
            break;
        case hash("D"):
            closeStackOctavesMap = &c_D_closeStackOctavesMap;
            break;
        case hash("E"):
            closeStackOctavesMap = &c_E_closeStackOctavesMap;
            break;
        case hash("F"):
            closeStackOctavesMap = &c_F_closeStackOctavesMap;
            break;
        case hash("G"):
            closeStackOctavesMap = &c_G_closeStackOctavesMap;
            break;
        case hash("A"):
            closeStackOctavesMap = &c_A_closeStackOctavesMap;
            break;
        case hash("B"):
            closeStackOctavesMap = &c_B_closeStackOctavesMap;
            break;
        default:
            LOG_ERROR("Invalid pitchStep: " + rootNote.getPitchStep());
            break;
    }

    for (auto& note : _closeStack) {
        const int oct = closeStackOctavesMap->at(*note.getPitchStep().c_str());
        note.setOctave(oct);
    }
}

void Chord::invalidateStackCache() {
    _isStackedInThirds = false;
    _closeStack.clear();
    _stackedHeaps.clear();
    _closeStackintervals.clear();
    _bassNote = Note();
}

std::vector<Interval> Chord::getIntervalsFromOriginalSortedNotes() const {
    std::vector<Note> sortedNotes = _originalNotes;
    std::sort(sortedNotes.begin(), sortedNotes.end());

    const int sortedNotesSize = sortedNotes.size();
    const int numIntervals = sortedNotesSize - 1;

    // A chord with fewer than two notes builds no interval, so a quarter tone in it is not
    // rejected.
    if (numIntervals > 0) {
        rejectQuarterToneInIntervals(findQuarterToneNote());
    }

    std::vector<Interval> intervals;
    intervals.reserve(numIntervals);

    for (int i = 0; i < numIntervals; i++) {
        intervals.push_back({sortedNotes[i], sortedNotes[i + 1]});
    }

    return intervals;
}

HeapData Chord::stackInThirdsTemplateMatch(const NoteDataHeap& heap) const {
    const int heapSize = heap.size();

    // Compute the max heap count points value
    int maxHeapPointsValue = 0;
    for (int i = 0; i < heapSize; i++) {
        maxHeapPointsValue += std::pow(2, (7 - i));
    }

    // LOG_DEBUG("maxHeapPointsValue: " << maxHeapPointsValue);

    HeapData heapData;
    std::get<0>(heapData) = heap;
    float* heapCountPoints = &std::get<1>(heapData);
    *heapCountPoints = 0.0f;
    const NoteData& rootNoteData = heap[0];
    const Note& rootNote = rootNoteData.note;

    const std::map<char, int>* stackPositionWeightMap = nullptr;

    switch (hash(rootNote.getPitchStep().c_str())) {
        case hash("C"):
            stackPositionWeightMap = &c_C_stackPositionWeightMap;
            break;
        case hash("D"):
            stackPositionWeightMap = &c_D_stackPositionWeightMap;
            break;
        case hash("E"):
            stackPositionWeightMap = &c_E_stackPositionWeightMap;
            break;
        case hash("F"):
            stackPositionWeightMap = &c_F_stackPositionWeightMap;
            break;
        case hash("G"):
            stackPositionWeightMap = &c_G_stackPositionWeightMap;
            break;
        case hash("A"):
            stackPositionWeightMap = &c_A_stackPositionWeightMap;
            break;
        case hash("B"):
            stackPositionWeightMap = &c_B_stackPositionWeightMap;
            break;
        default:
            LOG_ERROR("Invalid pitchStep: " + rootNote.getPitchStep());
            break;
    }

    // Iterate over each heap note
    for (int i = 0; i < heapSize; i++) {
        const std::string notePitchStep = heap[i].note.getPitchStep();
        const char notePitchStepChar = *notePitchStep.c_str();
        const float stackPositionWeight = stackPositionWeightMap->at(notePitchStepChar);

        float enharmonicNoteWeight = 0.0f;
        switch (heap[i].enharmonicDiatonicDistance) {
            case 0:
                enharmonicNoteWeight = 1.00f;
                break;
            case 1:
                enharmonicNoteWeight = 0.50f;
                break;
            case 2:
                enharmonicNoteWeight = 0.25f;
                break;
            default:
                LOG_ERROR("Invalid enharmonic diatonic distance");
                break;
        }

        // LOG_DEBUG("pitchStep: " << notePitchStep << " | stackPositionWeight:
        // " << stackPositionWeight << " | enharmonicNoteWeight: " <<
        // enharmonicNoteWeight);
        *heapCountPoints += stackPositionWeight * enharmonicNoteWeight;
    }

    // Normalize heap Count Points
    *heapCountPoints /= maxHeapPointsValue;

    return heapData;
}

void Chord::computeIntervals() {
    const std::vector<Interval> intervals = Helper::notes2Intervals(_closeStack);
    ignore(intervals);
}

std::vector<int> Chord::getMidiIntervals(const bool firstNoteAsReference) const {
    const int numNotes = _originalNotes.size();

    if (numNotes <= 0) {
        return {};
    }

    // A vector<int> of semitone counts cannot express the 3.5 semitones of a neutral third, so
    // this rejects rather than answering 3 or 4.
    rejectQuarterToneInMidiDomain(findQuarterToneNote(), "the MIDI intervals");

    std::vector<int> midiIntervals(numNotes - 1);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        const int rootMidiNumber = _originalNotes[0].getMidiNumber();
        for (int i = 1; i < numNotes; i++) {
            midiIntervals[i - 1] = _originalNotes[i].getMidiNumber() - rootMidiNumber;
        }

        return midiIntervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numNotes - 1; i++) {
        midiIntervals[i] =
            _originalNotes[i + 1].getMidiNumber() - _originalNotes[i].getMidiNumber();
    }

    return midiIntervals;
}

std::vector<Interval> Chord::getIntervals(const bool firstNoteAsReference) const {
    const int numIntervals = size() - 1;

    if (numIntervals <= 0) {
        return {};
    }

    rejectQuarterToneInIntervals(findQuarterToneNote());

    std::vector<Interval> intervals(numIntervals);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(_originalNotes[0], _originalNotes[i + 1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(_originalNotes[i], _originalNotes[i + 1]);
    }

    return intervals;
}

std::vector<Interval> Chord::getOpenStackIntervals(const bool firstNoteAsReference) {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    const int numIntervals = stackSize() - 1;

    if (numIntervals <= 0) {
        return {};
    }

    std::vector<Interval> intervals(numIntervals);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(_openStack[0], _openStack[i + 1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(_openStack[i], _openStack[i + 1]);
    }

    return intervals;
}
std::vector<Interval> Chord::getCloseStackIntervals(const bool fromRoot) {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    const int numIntervals = stackSize() - 1;

    if (numIntervals <= 0) {
        return {};
    }

    std::vector<Interval> intervals(numIntervals);

    // ===== GET INTERVALS FROM ROOT ===== //
    if (fromRoot) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(_closeStack[0], _closeStack[i + 1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(_closeStack[i], _closeStack[i + 1]);
    }

    return intervals;
}

std::vector<Note> Chord::getOpenStackNotes() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return _openStack;
}

int Chord::stackSize() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return static_cast<int>(_openStack.size());
}

const Note& Chord::getRoot() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return _closeStack.at(0);
}

std::string Chord::getName() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    // ===== STEP 1: CHECK IF THE CHORD IS TONAL OR NOT ====== //
    if (!isTonal()) {
        LOG_WARN("Unable to get a tonal name of a non-tonal chord");
        return {};
    }

    // Error checking
    if (!haveMinorThird() && !haveMajorThird()) {
        LOG_WARN("Unable to give a tonal chord name without a major/minor third");
        return {};
    }

    // ===== STEP 2: SET THE CHORD BASIC CLASSIFICATIION ====== //

    std::string basicClassification;

    // Set the note 'basicClassification' using the third, fifth and seventh
    // intervals
    if (haveMinorThird()) {
        if (haveDiminishedFifth()) {
            if (!haveDiminishedSeventh() && !haveMinorSeventh() && !haveMajorSeventh()) {
                basicClassification = "m(b5)";
            } else if (haveDiminishedSeventh()) {
                basicClassification = "º";
            } else if (haveMinorSeventh()) {
                basicClassification = "m7(b5)";
            } else if (haveMajorSeventh()) {
                basicClassification = "m7M(b5)";
            }
        } else if (havePerfectFifth() || (!haveDiminishedFifth() && !haveAugmentedFifth())) {
            if (!haveDiminishedSeventh() && !haveMinorSeventh() && !haveMajorSeventh()) {
                basicClassification = "m";
            } else if (haveDiminishedSeventh()) {
                basicClassification = "dim7";
            } else if (haveMinorSeventh()) {
                basicClassification = "m7";
            } else if (haveMajorSeventh()) {
                basicClassification = "m7M";
            }
        }
    } else {
        if (haveDiminishedFifth()) {
            if (!haveDiminishedSeventh() && !haveMinorSeventh() && !haveMajorSeventh()) {
                basicClassification = "(b5)";
            } else if (haveDiminishedSeventh()) {
                basicClassification = "dim7(b5)";
            } else if (haveMinorSeventh()) {
                basicClassification = "7(b5)";
            } else if (haveMajorSeventh()) {
                basicClassification = "7M(b5)";
            }
        } else if (havePerfectFifth() || (!haveDiminishedFifth() && !haveAugmentedFifth())) {
            if (!haveDiminishedSeventh() && !haveMinorSeventh() && !haveMajorSeventh()) {
                basicClassification = "";
            } else if (haveDiminishedSeventh()) {
                basicClassification = "dim7";
            } else if (haveMinorSeventh()) {
                basicClassification = "7";
            } else if (haveMajorSeventh()) {
                basicClassification = "7M";
            }
        } else if (haveAugmentedFifth()) {
            if (!haveDiminishedSeventh() && !haveMinorSeventh() && !haveMajorSeventh()) {
                basicClassification = "aug";
            } else if (haveDiminishedSeventh()) {
                basicClassification = "aug(dim7)";
            } else if (haveMinorSeventh()) {
                basicClassification = "aug(7)";
            } else if (haveMajorSeventh()) {
                basicClassification = "aug(7M)";
            }
        }
    }

    // ===== STEP 3: ADD CHORD EXTENSIONS ====== //
    std::string ninth;

    // If exists, add ninth
    if (haveMinorNinth()) {
        ninth = "9b";
    } else if (haveMajorNinth()) {
        ninth = "9";
    }

    std::string eleventh;

    // If exists, add eleventh
    if (havePerfectEleventh()) {
        eleventh = "(11)";
    } else if (haveSharpEleventh()) {
        eleventh = "(#11)";
    }

    std::string thirdteenth;

    // If exists, add thirdteenth
    if (haveMinorThirdteenth()) {
        thirdteenth = "13b";
    } else if (haveMajorThirdteenth()) {
        thirdteenth = "13";
    }

    // ===== STEP 4: ADD BASS NOTE ====== //
    std::string bassNoteStr;
    if (_closeStack[0].getPitchClass() != _bassNote.getPitchClass() &&
        !_bassNote.getPitchClass().empty()) {
        bassNoteStr.append("/" + _bassNote.getPitchClass());
    }

    // ===== STEP 5: SET CHORD NAME ====== //
    // Concatenate all _stack extensions
    const std::string chordName = _closeStack[0].getPitchClass() + basicClassification + ninth +
                                  eleventh + thirdteenth + bassNoteStr;

    return chordName;
}

bool Chord::isDyad() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (stackSize() == 2) ? true : false;
}

bool Chord::isSus() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    if (stackSize() < 3) {
        return false;
    }

    // ===== EXEMPLE ===== //
    // stackChord = [C4, Ebb4, G4]
    // Can be interpreted as => [G4, C5, D5] = Gsus4
    const Note root(_closeStack[2].getMidiNumber());
    const Note fourth(_closeStack[0].getMidiNumber() + 12);
    const Note fifth(_closeStack[1].getMidiNumber() + 12);

    const Interval firstInterval(root, fourth);
    const Interval secondInterval(root, fifth);

    const bool havePerfectFourth = firstInterval.isPerfectFourth(true);
    const bool havePerfectFifth = secondInterval.isPerfectFifth(true);

    return (havePerfectFourth && havePerfectFifth) ? true : false;
}

bool Chord::isMajorChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (stackSize() >= 3 && haveMajorThird() && !haveAugmentedFifth()) ? true : false;
}

bool Chord::isMinorChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (stackSize() >= 3 && haveMinorThird() && !haveDiminishedFifth()) ? true : false;
}

bool Chord::isAugmentedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (stackSize() >= 3 && haveMajorThird() && haveAugmentedFifth()) ? true : false;
}

bool Chord::isDiminishedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (stackSize() >= 3 && haveMinorThird() && haveDiminishedFifth() && !haveMinorSeventh() &&
            !haveDiminishedSeventh())
               ? true
               : false;
}

bool Chord::isHalfDiminishedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMinorThird() && haveDiminishedFifth() && haveMinorSeventh()) ? true : false;
}

bool Chord::isWholeDiminishedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMinorThird() && haveDiminishedFifth() && haveDiminishedSeventh()) ? true : false;
}

bool Chord::isDominantSeventhChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMajorThird() && !haveDiminishedFifth() && !haveAugmentedFifth() &&
            haveMinorSeventh())
               ? true
               : false;
}

std::string Chord::getQuality() {
    if (!isTonal()) {
        return "non-tonal";
    }

    const int orginalChordSize = size();
    const int stackSizeSize = stackSize();
    if (orginalChordSize == 0) {
        return {};
    } else if (stackSizeSize == 1 && orginalChordSize == 1) {
        return "single-note";
    } else if (stackSizeSize == 1 && orginalChordSize > 1) {
        return "dyad-octave";
    } else if (isDyad()) {
        return "dyad";
    } else if (isMajorChord()) {
        return "major";
    } else if (isMinorChord()) {
        return "minor";
    } else if (isDiminishedChord()) {
        return "diminished";
    } else if (isHalfDiminishedChord()) {
        return "half-diminished";
    } else if (isWholeDiminishedChord()) {
        return "whole-diminished";
    } else if (isAugmentedChord()) {
        return "augmented";
    } else if (isSus()) {
        return "sus";
    } else {
        return "indeterminate";
    }

    return {};
}

bool Chord::isTonal(std::function<bool(const Chord& chord)> model) {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    if (model != nullptr) {
        return model(*this);
    }

    // Special Case
    if (isSus()) {
        return true;
    }

    for (int i = 0; i < stackSize() - 1; i++) {
        const Interval interval(_closeStack[i], _closeStack[i + 1]);

        if (!interval.isTonal()) {
            return false;
        }
    }

    return true;
}

bool Chord::isInRootPosition() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    // Both containers are checked directly because both are read directly below: '_closeStack[0]'
    // on the next line of this guard's caller, and 'tempNotes[0]' (a copy of '_originalNotes') a
    // few lines down. Every _originalNotes mutator (clear(), removeNote(), ...) calls
    // invalidateStackCache(), which keeps the two in sync -- clearing '_closeStack' and resetting
    // '_isStackedInThirds' together -- so neither clause should be independently reachable
    // through the public mutator API. Checking both explicitly, rather than checking
    // one and relying on the other to match it, keeps this function correct on its own even if
    // that invariant is ever violated elsewhere, instead of depending on a property this function
    // has no way to verify.
    if (_closeStack.empty() || _originalNotes.empty()) {
        return false;
    }

    std::vector<Note> tempNotes = _originalNotes;
    std::sort(tempNotes.begin(), tempNotes.end());

    // The root is a concert copy, so the lowest of the chord's own notes is read by its concert
    // spelling too.
    return _closeStack[0].getPitchClass() == concertPitch(tempNotes[0]).getPitchClass();
}

bool Chord::isSorted() const {
    // Ordering is a predicate, and a bool expresses the true answer for a quarter-tone chord
    // exactly, so this computes rather than rejects.
    //
    // The exactness lives in Note's comparison operators, not in a lambda here, so this method
    // and Chord::sortNotes() share one source of truth for pitch order and agree by
    // construction. With an exact lambda here and the rounded order in sortNotes(), sorting
    // {"E4", "E1b4"} would leave the pair untouched -- std::sort would see two equal notes -- and
    // this method would then call the result unsorted: a chord sortNotes() could not make sorted.
    //
    // With the '<=', a chord holding the same pitch twice reports unsorted. That is a defect
    // about equal pitches rather than about quarter tones, and changing it would change the
    // answer for semitone chords too.
    return std::is_sorted(_originalNotes.begin(), _originalNotes.end(),
                          [](const Note& lh, const Note& rh) { return lh <= rh; });
}

float Chord::getCloseStackHarmonicComplexity(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return 0.0f;
    }

    float sum = 0.0f;
    const int numCloseIntervals = closeStackSize - 1;
    const auto& root = _closeStack.at(0);
    for (int i = 0; i < numCloseIntervals; i++) {
        const auto& nextNote = _closeStack.at(i + 1);
        const Interval interval(root.getPitch(), nextNote.getPitch());

        if (interval.isThird(useEnharmony)) {
            sum += HAVE_THIRD_VALUE;
        } else if (interval.isFifth(useEnharmony)) {
            sum += HAVE_FIFTH_VALUE;
        } else if (interval.isSeventh(useEnharmony)) {
            sum += HAVE_SEVENTH_VALUE;
        } else if (interval.isNinth(useEnharmony)) {
            sum += HAVE_NINTH_VALUE;
        } else if (interval.isEleventh(useEnharmony)) {
            sum += HAVE_ELEVENTH_VALUE;
        } else if (interval.isThirdteenth(useEnharmony)) {
            sum += HAVE_THIRDTEENTH_VALUE;
        } else {
            LOG_ERROR(
                "Invalid close chord interval[" + std::to_string(i) + "]: [" + root.getPitch() +
                ", " + nextNote.getPitch() + "] = " + interval.getName() +
                "\nuseEnharmony: " + std::to_string(useEnharmony) + " | getDiatonicInterval: " +
                std::to_string(interval.getDiatonicInterval(false, true)) +
                " | getDiatonicSteps: " + std::to_string(interval.getDiatonicSteps(false, true)));
        }
    }

    const float maxHarmonicComplexity =
        std::accumulate(c_haveComplexityValues.begin(), c_haveComplexityValues.end(), 0);

    // Normalize output
    sum += 1.0f;
    sum /= (maxHarmonicComplexity + 1);
    const std::string sumStr = Helper::formatFloat(sum, 4);

    // Formated string back to float type
    sum = std::stof(sumStr);

    return sum;
}

float Chord::getHarmonicDensity(int lowerBoundMIDI, int higherBoundMIDI) const {
    // ===== INPUT VALIDATION ===== //
    if ((lowerBoundMIDI == -1 && higherBoundMIDI != -1) ||
        (lowerBoundMIDI != -1 && higherBoundMIDI == -1)) {
        LOG_ERROR("You need set both values: 'lowerBoundMIDI' and 'higherBoundMIDI'");
    }

    // Case 01: No parameters are passed by the user: Use default values
    if (lowerBoundMIDI == -1 && higherBoundMIDI == -1) {
        // Returns a float, which expresses a range that is a whole number of semitones plus a
        // quarter tone exactly, so this computes rather than rejects. The extremes come from the
        // exact positions: the rounded MIDI numbers can pick the wrong extreme, and
        // Chord{"C1x4", "G4"} spans 6.5 semitones, not 7. For a chord with no quarter tone every
        // value below is a whole number. An empty chord raises std::out_of_range from .at(0).
        const int numNotes = static_cast<int>(_originalNotes.size());
        float lowestSteps = _originalNotes.at(0).getQuarterToneSteps();
        float highestSteps = lowestSteps;

        for (const auto& note : _originalNotes) {
            const float steps = note.getQuarterToneSteps();
            lowestSteps = std::min(lowestSteps, steps);
            highestSteps = std::max(highestSteps, steps);
        }

        return densityOverRange(numNotes, lowestSteps, highestSteps);
    }

    // Case 02: User defined values of 'higherBoundMIDI' and 'lowerBoundMIDI'
    const int numNotes = _originalNotes.size();
    const int midiRange = (higherBoundMIDI - lowerBoundMIDI) + 1;
    const float density = static_cast<float>(numNotes) / static_cast<float>(midiRange);

    return density;
}

float Chord::getHarmonicDensity(const std::string& lowerBoundPitch,
                                const std::string& higherBoundPitch) const {
    // ===== INPUT VALIDATION ===== //
    if (lowerBoundPitch.empty() || lowerBoundPitch == MUSIC_XML::PITCH::REST) {
        LOG_ERROR("'lowerBoundPitch' cannot be empty or be 'rest'");
    }

    if (higherBoundPitch.empty() || higherBoundPitch == MUSIC_XML::PITCH::REST) {
        LOG_ERROR("'higherBoundPitch' cannot be empty or be 'rest'");
    }

    // The bounds are exact rather than rounded, so a quarter-tone bound keeps its half step:
    // "C1x4" to "G4" spans 6.5 semitones, not the 6 that Helper::pitch2midiNote() would give.
    // Delegating to the numeric overload cannot preserve the half step, because its parameters
    // are ints, so the density is computed here from the same shared helper it uses.
    const float lowerBoundSteps = Note(lowerBoundPitch).getQuarterToneSteps();
    const float higherBoundSteps = Note(higherBoundPitch).getQuarterToneSteps();

    return densityOverRange(static_cast<int>(_originalNotes.size()), lowerBoundSteps,
                            higherBoundSteps);
}

bool Chord::haveMajorInterval(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();

    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajor(useEnharmony); });
}

bool Chord::haveMinorInterval(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinor(useEnharmony); });
}

bool Chord::havePerfectInterval(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isPerfect(useEnharmony); });
}

bool Chord::haveDiminishedInterval(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isDiminished(useEnharmony); });
}

bool Chord::haveAugmentedInterval(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isAugmented(useEnharmony); });
}

// ===== ABSTRACTION 1 ===== //
bool Chord::haveDiminishedUnisson(const bool useEnharmony) const {
    // auto sortedNotes = _originalNotes;
    // std::sort(sortedNotes.begin(), sortedNotes.end());

    const auto intervals = getIntervalsFromOriginalSortedNotes();

    const auto root = intervals.at(0).getNotes().at(0);
    const auto nextNote = intervals.at(0).getNotes().at(1);
    const auto interval = Interval(nextNote, root);

    return interval.isDiminishedUnisson(useEnharmony);
}

bool Chord::havePerfectUnisson(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return intervals.at(0).isPerfectUnisson(useEnharmony);
}

bool Chord::haveAugmentedUnisson(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return intervals.at(0).isAugmentedUnisson(useEnharmony);
}

bool Chord::haveMinorSecond(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 2;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMinorSecond(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMajorSecond(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 2;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMajorSecond(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMinorThird(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 3;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMinorThird(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMajorThird(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 3;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMajorThird(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::havePerfectFourth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 4;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isPerfectFourth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveAugmentedFourth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 4;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isAugmentedFourth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveDiminishedFifth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 5;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isDiminishedFifth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::havePerfectFifth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 5;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isPerfectFifth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveAugmentedFifth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 5;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isAugmentedFifth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMinorSixth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 6;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMinorSixth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMajorSixth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 6;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMajorSixth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveDiminishedSeventh(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 7;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isDiminishedSeventh(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMinorSeventh(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 7;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMinorSeventh(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMajorSeventh(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 7;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMajorSeventh(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveDiminishedOctave(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 8;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isDiminishedOctave(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::havePerfectOctave(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 8;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isPerfectOctave(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveAugmentedOctave(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 8;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isAugmentedOctave(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMinorNinth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 9;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMinorNinth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMajorNinth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 9;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMajorNinth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::havePerfectEleventh(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 11;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isPerfectEleventh(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveSharpEleventh(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 11;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isSharpEleventh(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMinorThirdteenth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 13;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMinorThirdteenth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

bool Chord::haveMajorThirdteenth(const bool useEnharmony) {
    if (!_isStackedInThirds) {
        stackInThirds(useEnharmony);
    }

    const int closeStackSize = _closeStack.size();
    if (closeStackSize < 2) {
        return false;
    }

    const auto& root = _closeStack.at(0);
    const int intervalScaleDegree = 13;
    const int maxIndex =
        (closeStackSize < intervalScaleDegree) ? closeStackSize : intervalScaleDegree - 1;
    for (int i = 1; i < maxIndex; i++) {
        const auto& nextNote = _closeStack.at(i);
        Interval interval(root, nextNote);
        if (interval.isMajorThirdteenth(useEnharmony)) {
            return true;
        }
    }

    return false;
}

// ===== ABSTRACTION 2 ===== //
bool Chord::haveSecond(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isSecond(useEnharmony); });
}

bool Chord::haveThird(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isThird(useEnharmony); });
}

bool Chord::haveFourth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isFourth(useEnharmony); });
}

bool Chord::haveFifth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isFifth(useEnharmony); });
}

bool Chord::haveSixth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isSixth(useEnharmony); });
}

bool Chord::haveSeventh(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isSeventh(useEnharmony); });
}

bool Chord::haveOctave(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isOctave(useEnharmony); });
}

bool Chord::haveNinth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isNinth(useEnharmony); });
}

bool Chord::haveEleventh(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isEleventh(useEnharmony); });
}

bool Chord::haveThirdteenth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(
        intervals.begin(), intervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isThirdteenth(useEnharmony); });
}

// ===== ABSTRACTION 3 ===== //
bool Chord::haveAnyOctaveMinorSecond(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorSecond(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorSecond(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorSecond(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMinorThird(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorThird(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorThird(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorThird(useEnharmony);
                       });
}

bool Chord::haveAnyOctavePerfectFourth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctavePerfectFourth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveAugmentedFourth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveAugmentedFourth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveDiminishedFifth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveDiminishedFifth(useEnharmony);
                       });
}

bool Chord::haveAnyOctavePerfectFifth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctavePerfectFifth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveAugmentedFifth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveAugmentedFifth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMinorSixth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorSixth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorSixth(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorSixth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveDiminishedSeventh(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveDiminishedSeventh(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMinorSeventh(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorSeventh(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorSeventh(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorSeventh(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveDiminishedOctave(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveDiminishedOctave(useEnharmony);
                       });
}

bool Chord::haveAnyOctavePerfectOctave(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctavePerfectOctave(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveAugmentedOctave(const bool useEnharmony) const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveAugmentedOctave(useEnharmony);
                       });
}

// ===== ABSTRACTION 4 ===== //
bool Chord::haveAnyOctaveSecond() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveSecond(); });
}

bool Chord::haveAnyOctaveThird() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveThird(); });
}

bool Chord::haveAnyOctaveFourth() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveFourth(); });
}

bool Chord::haveAnyOctaveFifth() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveFifth(); });
}

bool Chord::haveAnyOctaveSixth() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveSixth(); });
}

bool Chord::haveAnyOctaveSeventh() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveSeventh(); });
}

bool Chord::haveAnyOctaveOctave() const {
    const auto intervals = getIntervalsFromOriginalSortedNotes();
    return std::any_of(intervals.begin(), intervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveOctave(); });
}

const Note& Chord::getBassNote() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return _bassNote;
}

const std::vector<Note>& Chord::getNotes() const { return _originalNotes; }

Chord Chord::getOpenStackChord(const bool enharmonyNotes) {
    if (!_isStackedInThirds) {
        stackInThirds(enharmonyNotes);
    }

    return Chord(_openStack);
}

Chord Chord::getCloseStackChord(const bool enharmonyNotes) {
    if (!_isStackedInThirds) {
        stackInThirds(enharmonyNotes);
    }

    return Chord(_closeStack);
}

Chord Chord::getCloseChord(const bool enharmonyNotes) {
    if (!_isStackedInThirds) {
        stackInThirds(enharmonyNotes);
    }

    Chord closeChord(_closeStack);

    const Note& rootNote = getRoot();
    // Chord::addNote() skips rests, so every note reachable through a Chord is a real pitch and
    // getWrittenOctave() is always engaged here.
    const int rootNoteOctave = rootNote.getWrittenOctave().value();

    // The root and the close chord's notes are concert copies, so the chord's own notes are looked
    // up, and their octaves compared, by their concert spellings too.
    const std::vector<Note> originalNotes = getNotes();

    const auto rootNoteWithOriginalOct =
        std::find_if(originalNotes.begin(), originalNotes.end(), [rootNote](const Note& note) {
            return concertPitch(note).getPitchClass() == rootNote.getPitchClass();
        });

    const int closeChordSize = closeChord.size();

    for (int i = 1; i < closeChordSize; i++) {
        Note& closeNote = closeChord[i];

        const Interval interval(rootNote, closeNote);

        if (interval.isSimple()) {
            continue;
        }

        const auto extendedNoteWithOriginalOct =
            std::find_if(originalNotes.begin(), originalNotes.end(), [closeNote](const Note& note) {
                return concertPitch(note).getPitchClass() == closeNote.getPitchClass();
            });

        if (concertPitch(*extendedNoteWithOriginalOct).getOctave().value() ==
            concertPitch(*rootNoteWithOriginalOct).getOctave().value()) {
            closeNote.setOctave(rootNoteOctave);
        }
    }

    closeChord.sortNotes();
    return closeChord;
}

void Chord::sortNotes() {
    std::sort(_originalNotes.begin(), _originalNotes.end());

    // Reordering '_originalNotes' invalidates any previously-computed stacked-in-thirds
    // representation: internal stacking code (e.g. computeBestOpenStackHeap()) relies on
    // '_originalNotes' and the open/close stacks corresponding by position.
    invalidateStackCache();
}

std::vector<int> Chord::toCents() const {
    const int numNotes = static_cast<int>(_originalNotes.size());

    if (numNotes <= 0) {
        return {};
    }

    const int numIntervals = numNotes - 1;
    std::vector<int> centsVec(numIntervals, 0);

    // Cents are the one unit in this library that expresses a quarter tone exactly -- 50 cents to
    // the quarter tone, 350 to the neutral third -- so this computes rather than rejects.
    //
    // Twelve-tone equal temperament puts exactly 100 cents in a semitone, so the value comes
    // straight from the exact sounding positions: no frequency, no logarithm, no dependence on
    // freqA4. Positions are multiples of 0.5, which float holds exactly, so the difference times
    // 100 is an exact whole number of cents. A frequency route would instead inherit
    // Note::getFrequency()'s rounded MIDI number (a neutral third would read 400) and
    // Helper::frequencies2cents()'s truncation (a C major triad would read [400, 299]).
    for (int i = 0; i < numIntervals; i++) {
        const float stepsDiff =
            _originalNotes[i + 1].getQuarterToneSteps() - _originalNotes[i].getQuarterToneSteps();
        centsVec[i] = static_cast<int>(stepsDiff * 100.0f);
    }

    return centsVec;
}

int Chord::getDegree(const Key& key, bool enharmonyNotes) {
    if (!_isStackedInThirds) {
        stackInThirds(enharmonyNotes);
    }

    const bool isMajorMode = key.isMajorMode();
    const std::string majorKeyName = (isMajorMode) ? key.getName() : key.getRelativeKeyName();
    const std::string keyPitchStep = majorKeyName.substr(0, 1);
    const std::array<std::string, 7>* diatonicScale = nullptr;
    switch (hash(keyPitchStep.c_str())) {
        case hash("C"):
            diatonicScale = (isMajorMode) ? &c_C_diatonicScale : &c_A_diatonicScale;
            break;
        case hash("D"):
            diatonicScale = (isMajorMode) ? &c_D_diatonicScale : &c_B_diatonicScale;
            break;
        case hash("E"):
            diatonicScale = (isMajorMode) ? &c_E_diatonicScale : &c_C_diatonicScale;
            break;
        case hash("F"):
            diatonicScale = (isMajorMode) ? &c_F_diatonicScale : &c_D_diatonicScale;
            break;
        case hash("G"):
            diatonicScale = (isMajorMode) ? &c_G_diatonicScale : &c_E_diatonicScale;
            break;
        case hash("A"):
            diatonicScale = (isMajorMode) ? &c_A_diatonicScale : &c_F_diatonicScale;
            break;
        case hash("B"):
            diatonicScale = (isMajorMode) ? &c_B_diatonicScale : &c_G_diatonicScale;
            break;
        default:
            LOG_ERROR("Invalid pitchStep: " + keyPitchStep);
            break;
    }

    // Skip invalid chord size
    if (_closeStack.empty()) {
        return 0;
    }

    const std::string rootPitchStep = _closeStack.at(0).getPitchStep();

    const auto it = std::find(diatonicScale->begin(), diatonicScale->end(), rootPitchStep);
    const int index = std::distance(diatonicScale->begin(), it);
    const int degree = index + 1;

    return degree;
}

std::string Chord::getRomanDegree(const Key& key, bool enharmonyNotes) {
    const int degree = getDegree(key, enharmonyNotes);

    if (degree == 0) {
        return {};
    }

    const int index = degree - 1;

    // OVER SIMPLIFIED!! TO IMPROVE!!
    return c_harmonyKeyDegrees.at(index);
}

float Chord::getMeanFrequency(const float freqA4) const {
    float sum = 0.0f;
    for (const auto& note : _originalNotes) {
        sum += note.getFrequency(freqA4);
    }

    const int mean = sum / _originalNotes.size();
    return mean;
}

float Chord::getMeanOfExtremesFrequency(const float freqA4) const {
    if (_originalNotes.size() == 0) {
        return 0.0f;
    }

    auto copyNotes = _originalNotes;
    std::sort(copyNotes.begin(), copyNotes.end());
    int lowFreq = copyNotes.at(0).getFrequency(freqA4);
    int highFreq = copyNotes.at(copyNotes.size() - 1).getFrequency(freqA4);

    const float mean = (static_cast<float>(lowFreq) + static_cast<float>(highFreq)) / 2.0f;
    return mean;
}

float Chord::getFrequencyStd(const float freqA4) const {
    // The population standard deviation of the notes' frequencies, accumulated over the notes
    // themselves, so the sample holds exactly one value per note: a vector sized to the note
    // count and then push_back()ed into would also hold a leading run of zeros.
    //
    // Note::getFrequency() derives each frequency from the rounded MIDI number, so a quarter tone
    // contributes the frequency of the semitone above it.
    //
    // An empty chord returns 0.0f rather than dividing by zero.
    const size_t numNotes = _originalNotes.size();

    if (numNotes == 0) {
        return 0.0f;
    }

    float sum = 0.0f;
    for (const auto& note : _originalNotes) {
        sum += note.getFrequency(freqA4);
    }

    const float mean = sum / static_cast<float>(numNotes);

    float squaredDeviationSum = 0.0f;
    for (const auto& note : _originalNotes) {
        const float deviation = note.getFrequency(freqA4) - mean;
        squaredDeviationSum += deviation * deviation;
    }

    return std::sqrt(squaredDeviationSum / static_cast<float>(numNotes));
}

int Chord::getMeanMidiValue() const {
    if (_originalNotes.size() == 0) {
        return 0;
    }

    // An int cannot express the 63.5 that {C4, E1b4, G4} averages to, so this rejects rather than
    // answering 63 -- the same value a plain C major triad gives.
    rejectQuarterToneInMidiDomain(findQuarterToneNote(), "the mean MIDI value");

    int sum = 0;
    for (const auto& note : _originalNotes) {
        sum += note.getMidiNumber();
    }

    const int mean = sum / static_cast<int>(_originalNotes.size());
    return mean;
}

int Chord::getMeanOfExtremesMidiValue() const {
    if (_originalNotes.size() == 0) {
        return MUSIC_XML::MIDI::NUMBER::MIDI_REST;
    }

    // An int cannot express the mean of two extremes when either of them is a quarter tone, so
    // this rejects. It rejects for any quarter tone anywhere in the chord, not only one that
    // happens to be an extreme, because the extremes themselves are selected by an ordering that
    // the rounding can get wrong.
    rejectQuarterToneInMidiDomain(findQuarterToneNote(), "the mean of extremes MIDI value");

    auto copyNotes = _originalNotes;
    std::sort(copyNotes.begin(), copyNotes.end());
    int lowMIDI = copyNotes.at(0).getMidiNumber();
    int highMIDI = copyNotes.at(copyNotes.size() - 1).getMidiNumber();

    const int mean = (lowMIDI + highMIDI) / 2;
    return mean;
}

float Chord::getMidiValueStd() const {
    // Returns a float, which expresses the spread of a quarter-tone chord exactly, so this
    // computes rather than rejects: it reads each note's exact position, getQuarterToneSteps(),
    // not the rounded getMidiNumber(). The sample holds exactly one value per note (see
    // getFrequencyStd()), and the running sum is a float: an int accumulator, such as
    // std::accumulate() seeded with the literal 0, would truncate the .5 of every quarter tone.
    //
    // An empty chord returns 0.0f rather than dividing by zero.
    const size_t numNotes = _originalNotes.size();

    if (numNotes == 0) {
        return 0.0f;
    }

    float sum = 0.0f;
    for (const auto& note : _originalNotes) {
        sum += note.getQuarterToneSteps();
    }

    const float mean = sum / static_cast<float>(numNotes);

    float squaredDeviationSum = 0.0f;
    for (const auto& note : _originalNotes) {
        const float deviation = note.getQuarterToneSteps() - mean;
        squaredDeviationSum += deviation * deviation;
    }

    return std::sqrt(squaredDeviationSum / static_cast<float>(numNotes));
}

std::string Chord::getMeanPitch(const std::string& accType) const {
    const int meanMIDI = getMeanMidiValue();

    return Helper::midiNote2pitch(meanMIDI, accType);
}

std::string Chord::getMeanOfExtremesPitch(const std::string& accType) const {
    const int meanMIDI = getMeanOfExtremesMidiValue();

    return Helper::midiNote2pitch(meanMIDI, accType);
}

std::pair<std::vector<float>, std::vector<float>> Chord::getHarmonicSpectrum(
    const int numPartialsPerNote,
    const std::function<std::vector<float>(std::vector<float>)> amplCallback,
    const float partialsDecayExpRate) const {
    if (numPartialsPerNote <= 0) {
        LOG_ERROR("The 'numPartialsPerNote' must be a positive value");
    }

    std::map<float, float> freqAmplMap;

    for (const auto& note : _originalNotes) {
        const auto freqsAmplsPair =
            note.getHarmonicSpectrum(numPartialsPerNote, amplCallback, partialsDecayExpRate);

        for (size_t i = 0; i < freqsAmplsPair.first.size(); ++i) {
            auto freq = freqsAmplsPair.first[i];
            auto ampl = freqsAmplsPair.second[i];

            freqAmplMap[freq] += ampl;
        }
    }

    std::vector<float> combinedFrequencies;
    std::vector<float> combinedAmplitudes;

    // Extract keys and values from the map to the vectors
    std::transform(freqAmplMap.begin(), freqAmplMap.end(), std::back_inserter(combinedFrequencies),
                   [](const std::pair<float, float>& pair) { return pair.first; });

    std::transform(freqAmplMap.begin(), freqAmplMap.end(), std::back_inserter(combinedAmplitudes),
                   [](const std::pair<float, float>& pair) { return pair.second; });

    return {combinedFrequencies, combinedAmplitudes};
}

SetharesDissonanceTable Chord::getSetharesDyadsDissonanceValue(
    const int numPartialsPerNote, const bool useMinModel,
    const std::function<std::vector<float>(std::vector<float>)> amplCallback,
    const float partialsDecayExpRate) const {
    /*
    Given a list of partials in fvec, with amplitudes in amp, this routine
    calculates the dissonance by summing the roughness of every sine pair
    based on a model of Plomp-Levelt's roughness curve.

    The older model (model='product') was based on the product of the two
    amplitudes, but the newer model (model='min') is based on the minimum
    of the two amplitudes, since this matches the beat frequency amplitude.
    */

    const auto& freqAmplPair =
        getHarmonicSpectrum(numPartialsPerNote, amplCallback, partialsDecayExpRate);

    const std::vector<float>& fvec = freqAmplPair.first;
    const std::vector<float>& amp = freqAmplPair.second;

    // Ensure input vectors are of the same size
    if (fvec.size() != amp.size()) {
        LOG_ERROR("The 'frequency' and 'amplitude' vectors must have the same size.");
    }

    // Constants
    const float Dstar = 0.24f;  //  Point of maximum dissonance
    const float S1 = 0.0207f;
    const float S2 = 18.96f;
    const float C1 = 5.0f;
    const float C2 = -5.0f;
    const float A1 = -3.51f;  // Plomp-Levelt roughness curve
    const float A2 = -5.75f;

    // Sort indices based on fvec
    std::vector<size_t> sort_idx(fvec.size());
    std::iota(sort_idx.begin(), sort_idx.end(), 0);  // Fill with 0, 1, ... , fvec.size()-1
    std::sort(sort_idx.begin(), sort_idx.end(),
              [&fvec](size_t i, size_t j) { return fvec[i] < fvec[j]; });

    // Sort fvec and amp using the sorted indices
    std::vector<float> fr_sorted(fvec.size());
    std::vector<float> am_sorted(amp.size());
    for (size_t i = 0; i < sort_idx.size(); ++i) {
        fr_sorted[i] = fvec[sort_idx[i]];
        am_sorted[i] = amp[sort_idx[i]];
    }

    const int freqSortedSize = fr_sorted.size();

    std::vector<SetharesDissonanceTableRow> table;
    const int tableSize = freqSortedSize * freqSortedSize;
    table.reserve(tableSize);
    for (int freqBaseIdx = 0; freqBaseIdx < freqSortedSize; ++freqBaseIdx) {
        for (int freqTargetIdx = freqBaseIdx + 1; freqTargetIdx < freqSortedSize; ++freqTargetIdx) {
            float Fmin = fr_sorted[freqBaseIdx];
            float S = Dstar / (S1 * Fmin + S2);
            float Fdif = fr_sorted[freqTargetIdx] - Fmin;

            // Select model: 'min' or 'product'
            float a = (useMinModel) ? std::min(am_sorted[freqBaseIdx], am_sorted[freqTargetIdx])
                                    : am_sorted[freqBaseIdx] * am_sorted[freqTargetIdx];

            float SFdif = S * Fdif;

            const float diss = a * (C1 * std::exp(A1 * SFdif) + C2 * std::exp(A2 * SFdif));

            // Compute the pitch and cents deviation of both 'base' and 'target' frequencies
            const auto basePitchCentsPair =
                Helper::freq2pitch(fr_sorted[freqBaseIdx], MUSIC_XML::ACCIDENT::NONE);
            const auto targetPitchCentsPair =
                Helper::freq2pitch(fr_sorted[freqTargetIdx], MUSIC_XML::ACCIDENT::NONE);

            // Compute the dyad frequency ratio
            const float freqRatio = fr_sorted[freqTargetIdx] / fr_sorted[freqBaseIdx];

            // Store all data in a table row
            table.push_back({freqBaseIdx, fr_sorted[freqBaseIdx], basePitchCentsPair.first,
                             basePitchCentsPair.second, am_sorted[freqBaseIdx], freqTargetIdx,
                             fr_sorted[freqTargetIdx], targetPitchCentsPair.first,
                             targetPitchCentsPair.second, am_sorted[freqTargetIdx], a, freqRatio,
                             diss});
        }
    }

    return table;
}

float Chord::getSetharesDissonance(
    const int numPartialsPerNote, const bool useMinModel,
    const std::function<std::vector<float>(std::vector<float>)> amplCallback,
    const float partialsDecayExpRate,
    const std::function<float(std::vector<float>)> dissCallback) const {
    const SetharesDissonanceTable table = getSetharesDyadsDissonanceValue(
        numPartialsPerNote, useMinModel, amplCallback, partialsDecayExpRate);

    const int tableSize = table.size();
    const int dissColIdx = 12;
    if (dissCallback == nullptr) {
        float totalDissonance = 0.0f;
        for (int row = 0; row < tableSize; row++) {
            totalDissonance += std::get<dissColIdx>(table[row]);
        }

        return totalDissonance;
    }

    std::vector<float> dissonanceVec(tableSize, 0.0f);
    for (int row = 0; row < tableSize; row++) {
        dissonanceVec[row] = std::get<dissColIdx>(table[row]);
    }

    return dissCallback(dissonanceVec);
}

void printHeap(const NoteDataHeap& heap) {
    for (const auto& noteData : heap) {
        std::cout << "printHeap: " << noteData.note.getPitch() << " ";
    }
    std::cout << std::endl;
}

void sortHeapOctaves(NoteDataHeap* heap) {
    // Sort heap octaves from octave '3'
    heap->at(0).note.setOctave(3);

    const int heapSize = heap->size();

    for (int i = 0; i < heapSize - 1; i++) {
        auto& currentNote = heap->at(i).note;
        auto& nextNote = heap->at(i + 1).note;

        const int currentNoteFirstOct = currentNote.getMidiNumber() % 12;
        const int nextNoteFirstOct = nextNote.getMidiNumber() % 12;

        // Every note reachable here came through Chord::addNote(), which skips rests, so
        // getWrittenOctave() is always engaged.
        if (nextNoteFirstOct > currentNoteFirstOct) {
            // Special cases:
            if (nextNote.getPitchClass() == "Cb" || nextNote.getPitchClass() == "Cbb") {
                nextNote.setOctave(currentNote.getWrittenOctave().value() + 1);
            } else if (currentNote.getPitchClass() == "B#" || currentNote.getPitchClass() == "Bx") {
                nextNote.setOctave(currentNote.getWrittenOctave().value() + 1);
            } else {
                nextNote.setOctave(currentNote.getWrittenOctave().value());
            }

            continue;
        }

        if (nextNoteFirstOct < currentNoteFirstOct) {
            // Special cases:
            if (nextNote.getPitchClass() == "B#" || nextNote.getPitchClass() == "Bx") {
                nextNote.setOctave(currentNote.getWrittenOctave().value());
            } else {
                nextNote.setOctave(currentNote.getWrittenOctave().value() + 1);
            }
        }
    }
}

bool operator<(const HeapData& a, const HeapData& b) { return std::get<1>(a) < std::get<1>(b); }

std::ostream& operator<<(std::ostream& os, const Chord& chord) {
    const int chordSize = chord.size();

    // A stream operator should not throw where it can answer: gtest calls operator<< to format
    // values in failure messages, so a throwing operator<< can turn a clean test failure into a
    // process abort. getNote(-1) on an empty chord throws, so the empty case is handled here
    // explicitly, matching __repr__'s "[]" for an empty chord. A note sounding below C1b-1 has no
    // spelling to show, so it still throws, as the declaration documents.
    if (chordSize == 0) {
        os << "[]";
        return os;
    }

    os << "[";
    for (int i = 0; i < chordSize - 1; i++) {
        os << concertName(chord.getNote(i)) << ",";
    }
    os << concertName(chord.getNote(chordSize - 1)) + "]";
    return os;
}