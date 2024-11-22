#include "maiacore/chord.h"

#include <algorithm>  // std::rotate, std::count
#include <iostream>
#include <map>
#include <set>      // std::set
#include <utility>  // std::pair

#include "maiacore/constants.h"
#include "maiacore/helper.h"
#include "maiacore/interval.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

// const int RESERVE_NUM_NOTES = 20;

Chord::Chord() : _isStackedInThirds(false) {}

Chord::Chord(const std::vector<Note>& notes) : _isStackedInThirds(false) {
    // reserveNotesMemory();
    for (const auto& n : notes) {
        addNote(n);
    }
}

Chord::Chord(const std::vector<std::string>& pitches) : _isStackedInThirds(false) {
    // reserveNotesMemory();
    for (const auto& p : pitches) {
        addNote(Note(p));
    }
}

Chord::~Chord() {}

// void Chord::reserveNotesMemory() {
//     _originalNotes.reserve(RESERVE_NUM_NOTES);
//     _openStack.reserve(RESERVE_NUM_NOTES);
//     _closeStack.reserve(RESERVE_NUM_NOTES);
//     _stackedHeaps.reserve(RESERVE_NUM_NOTES);
//     _closeStackintervals.reserve(RESERVE_NUM_NOTES);
// }

void Chord::clear() {
    _originalNotes.clear();
    _openStack.clear();
}

void Chord::info() {
    LOG_INFO("Name: " << getName());
    LOG_INFO("Size:" << size());

    const int chordSize = size();
    std::string noteNames = "[";

    for (int i = 0; i < chordSize - 1; i++) {
        noteNames.append(_originalNotes[i].getPitch() + ", ");
    }

    // Add the last note without the semicomma in the end
    noteNames.append(_originalNotes[chordSize - 1].getPitch());

    noteNames.append("]");

    LOG_INFO(noteNames);

    // ---------------- //
    const int chordStackSize = stackSize();

    LOG_INFO("=====> CHORD STACK <=====");
    LOG_INFO("Open Stack Size:" << chordStackSize);

    std::string stackNames = "[";

    for (int i = 0; i < chordStackSize - 1; i++) {
        stackNames.append(_openStack[i].getPitch() + ", ");
    }

    // Add the last note without the semicomma in the end
    stackNames.append(_openStack[chordStackSize - 1].getPitch());

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

    // Reset the chord stacked flag
    _isStackedInThirds = false;
}

void Chord::addNote(const std::string& pitch) { addNote(Note(pitch)); }

void Chord::removeTopNote() {
    _originalNotes.pop_back();

    // Reset the chord stacked flag
    _isStackedInThirds = false;
}

void Chord::insertNote(Note& note, int noteIndex) {
    note.setIsInChord(true);
    _originalNotes.insert(_originalNotes.begin() + noteIndex, note);

    _openStack.push_back(note);

    // Reset the chord stacked flag
    _isStackedInThirds = false;
}

void Chord::removeNote(int noteIndex) {
    _originalNotes.erase(_originalNotes.begin() + noteIndex);

    // Reset the chord stacked flag
    _isStackedInThirds = false;
}

void Chord::setDuration(const Duration& duration) {
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        _originalNotes[i].setDuration(duration);
        _openStack[i].setDuration(duration);
    }
}

void Chord::setDuration(const float quarterDuration, const int divisionsPerQuarterNote) {
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        _originalNotes[i].setDuration(quarterDuration, divisionsPerQuarterNote);
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
    for (int i = 0; i < inversionNumber; i++) {
        _originalNotes[0].transpose(12);  // isto apenas altera a nota uma oitava acima

        const Note& x = _originalNotes[0];  // pega a nota que foi alterada acima
        _originalNotes.push_back(x);
        _originalNotes.erase(_originalNotes.begin());
    }
}

void Chord::transpose(const int semitonesNumber) {
    if (semitonesNumber == 0) {
        return;
    }

    std::string pitchClass, pitchStep, alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;

    // Transpose the original chord
    const int chordSize = _originalNotes.size();
    for (int i = 0; i < chordSize; i++) {
        const std::string pitch = _originalNotes[i].getWrittenPitch();

        int midinumber = Helper::pitch2midiNote(pitch) + semitonesNumber;

        Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

        const std::string newPitch = Helper::midiNote2pitch(midinumber, alterSymbol);

        _originalNotes[i].setPitch(newPitch);
    }

    transposeStackOnly(semitonesNumber);
}

void Chord::transposeStackOnly(const int semitonesNumber) {
    if (semitonesNumber == 0) {
        return;
    }

    std::string pitchClass, pitchStep, alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;

    // Transpose the stack version
    const int openStackSize = _openStack.size();
    for (int i = 0; i < openStackSize; i++) {
        const std::string pitch = _openStack[i].getWrittenPitch();

        int midinumber = Helper::pitch2midiNote(pitch) + semitonesNumber;

        Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

        const std::string newPitch = Helper::midiNote2pitch(midinumber, alterSymbol);

        _openStack[i].setPitch(newPitch);
    }
}

void Chord::removeDuplicateNotes() {
    sortNotes();
    _originalNotes.erase(std::unique(_originalNotes.begin(), _originalNotes.end()),
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

Note& Chord::getNote(int noteIndex) { return _originalNotes[noteIndex]; }

const Note& Chord::getNote(const int noteIndex) const { return _originalNotes[noteIndex]; }

void Chord::print() const {
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        LOG_INFO("note[" << i << "] = " << _originalNotes[i].getPitch());
    }
}

void Chord::printStack() const {
    const int stackSize = _openStack.size();

    for (int i = 0; i < stackSize; i++) {
        LOG_INFO("openStack[" << i << "] = " << _openStack[i].getPitch());
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

    // ===== STEP 1: COMPUTE THE OPEN STACK ===== //
    // Copy the orginal chord to a stack vector
    _openStack.clear();
    _openStack = _originalNotes;

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

    // Error checking:
    if (chordSize < 2) {
        // LOG_WARN("The chord MUST BE at least 2 unique pitch classes!");
        return;
    }

    // ===== STEP 2: COMPUTE ENHARMONIC UNIT GROUPS ===== //
    std::vector<NoteDataHeap> enharmonicUnitGroups = computeEnharmonicUnitsGroups();

    // ===== STEP 3: COMPUTE ALL ENHARMONIC HEAPS (VALID AND INVALID HEAPS)
    // ===== //
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

            // sortHeapOctaves(&heapInversion);
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
    const float bestStackMatchValue = std::get<1>(stackedHeaps[0]);
    const int heapSize = std::get<0>(stackedHeaps[0]).size();

    int swapHeapDataIdx = 0;
    bool foundHeapMatch = false;

    std::vector<std::string> originalNotesPitchClass(heapSize);
    for (int i = 0; i < heapSize; i++) {
        originalNotesPitchClass[i] = _originalNotes[i].getPitchClass();
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

std::vector<Interval> Chord::getIntervalsFromOriginalSortedNotes() const {
    std::vector<Note> sortedNotes = _originalNotes;
    std::sort(sortedNotes.begin(), sortedNotes.end());

    const int sortedNotesSize = sortedNotes.size();
    const int numIntervals = sortedNotesSize - 1;

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

bool Chord::isMajorChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMajorThird() && !haveAugmentedFifth()) ? true : false;
}

bool Chord::isMinorChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMinorThird() && !haveDiminishedFifth()) ? true : false;
}

bool Chord::isAugmentedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMajorThird() && haveAugmentedFifth()) ? true : false;
}

bool Chord::isDiminishedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMinorThird() && haveDiminishedFifth() && haveMinorSeventh()) ? true : false;
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

bool Chord::isHalfDiminishedChord() {
    if (!_isStackedInThirds) {
        stackInThirds();
    }

    return (haveMinorThird() && haveDiminishedFifth()) ? true : false;
}

std::string Chord::getQuality() {
    switch (size()) {
        case 0:
        case 1:
            return {};
        case 2:
            return "dyad";
    }

    if (isMajorChord()) {
        return "major";
    } else if (isMinorChord()) {
        return "minor";
    } else if (isDiminishedChord()) {
        return "diminished";
    } else if (isHalfDiminishedChord()) {
        return "half-diminished";
    } else if (isAugmentedChord()) {
        return "augmented";
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

    for (int i = 0; i < stackSize() - 1; i++) {
        const Interval interval(_openStack[i], _openStack[i + 1]);

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

    std::vector<Note> tempNotes = _originalNotes;
    std::sort(tempNotes.begin(), tempNotes.end());

    return _closeStack[0].getPitchClass() == tempNotes[0].getPitchClass();
}

bool Chord::isSorted() const {
    return std::is_sorted(
        _originalNotes.begin(), _originalNotes.end(),
        [](const Note& lh, const Note& rh) { return lh.getMidiNumber() <= rh.getMidiNumber(); });
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
        std::vector<Note> sortedNotes = _originalNotes;
        std::sort(sortedNotes.begin(), sortedNotes.end());

        const int numNotes = sortedNotes.size();
        const int lowestMIDI = sortedNotes.at(0).getMidiNumber();
        const int highestMIDI = sortedNotes.at(numNotes - 1).getMidiNumber();

        const int midiRange = (highestMIDI - lowestMIDI) + 1;
        const float density = static_cast<float>(numNotes) / static_cast<float>(midiRange);

        return density;
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

    // Get MIDI values from the user input pitches
    const int lowerBoundMIDI = Helper::pitch2midiNote(lowerBoundPitch);
    const int higherBoundMIDI = Helper::pitch2midiNote(higherBoundPitch);

    // Compute density
    return getHarmonicDensity(lowerBoundMIDI, higherBoundMIDI);
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

bool Chord::haveAnyOctaveDiminhavehedFifth(const bool useEnharmony) const {
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

bool Chord::haveAnyOctaveDiminhavehedSeventh(const bool useEnharmony) const {
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

bool Chord::haveAnyOctaveDiminhavehedOctave(const bool useEnharmony) const {
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
    const int rootNoteOctave = rootNote.getWrittenOctave();

    const std::vector<Note> originalNotes = getNotes();

    const auto rootNoteWithOriginalOct = std::find_if(
        originalNotes.begin(), originalNotes.end(),
        [rootNote](const Note& note) { return note.getPitchClass() == rootNote.getPitchClass(); });

    const int closeChordSize = closeChord.size();

    for (int i = 1; i < closeChordSize; i++) {
        Note& closeNote = closeChord[i];

        const Interval interval(rootNote, closeNote);

        if (interval.isSimple()) {
            continue;
        }

        const auto extendedNoteWithOriginalOct =
            std::find_if(originalNotes.begin(), originalNotes.end(), [closeNote](const Note& note) {
                return note.getPitchClass() == closeNote.getPitchClass();
            });

        if (extendedNoteWithOriginalOct->getOctave() == rootNoteWithOriginalOct->getOctave()) {
            closeNote.setOctave(rootNoteOctave);
        }
    }

    closeChord.sortNotes();
    return closeChord;
}

void Chord::sortNotes() { std::sort(_originalNotes.begin(), _originalNotes.end()); }

std::vector<int> Chord::toCents() const {
    const int numIntervals = _originalNotes.size() - 1;
    std::vector<int> centsVec(numIntervals, 0);
    for (int i = 0; i < numIntervals; i++) {
        const Interval interval(_originalNotes[i], _originalNotes[i + 1]);
        centsVec[i] = interval.toCents();
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

float Chord::getMeanFrequency() const {
    float sum = 0.0f;
    for (const auto& note : _originalNotes) {
        sum += note.getFrequency();
    }

    const int mean = sum / _originalNotes.size();
    return mean;
}

float Chord::getMeanOfExtremesFrequency() const {
    if (_originalNotes.size() == 0) {
        return 0.0f;
    }

    auto copyNotes = _originalNotes;
    std::sort(copyNotes.begin(), copyNotes.end());
    int lowFreq = copyNotes.at(0).getFrequency();
    int highFreq = copyNotes.at(copyNotes.size() - 1).getFrequency();

    const float mean = (static_cast<float>(lowFreq) + static_cast<float>(highFreq)) / 2.0f;
    return mean;
}

float Chord::getFrequencyStd() const {
    std::vector<float> freqVec(_originalNotes.size(), 0.0f);

    for (const auto& note : _originalNotes) {
        freqVec.push_back(note.getFrequency());
    }

    return computeStandardDeviation(freqVec);
}

int Chord::getMeanMidiValue() const {
    if (_originalNotes.size() == 0) {
        return 0;
    }

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

    auto copyNotes = _originalNotes;
    std::sort(copyNotes.begin(), copyNotes.end());
    int lowMIDI = copyNotes.at(0).getMidiNumber();
    int highMIDI = copyNotes.at(copyNotes.size() - 1).getMidiNumber();

    const int mean = (lowMIDI + highMIDI) / 2;
    return mean;
}

float Chord::getMidiValueStd() const {
    std::vector<int> midiVec(_originalNotes.size(), 0);

    for (const auto& note : _originalNotes) {
        midiVec.push_back(note.getMidiNumber());
    }

    return computeStandardDeviation(midiVec);
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
    const std::function<std::vector<float>(std::vector<float>)> amplCallback) const {
    if (numPartialsPerNote <= 0) {
        LOG_ERROR("The 'numPartialsPerNote' must be a positive value");
    }

    std::map<float, float> freqAmplMap;

    for (const auto& note : _originalNotes) {
        const auto freqsAmplsPair = note.getHarmonicSpectrum(numPartialsPerNote, amplCallback);

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
    const std::function<std::vector<float>(std::vector<float>)> amplCallback) const {
    /*
    Given a list of partials in fvec, with amplitudes in amp, this routine
    calculates the dissonance by summing the roughness of every sine pair
    based on a model of Plomp-Levelt's roughness curve.

    The older model (model='product') was based on the product of the two
    amplitudes, but the newer model (model='min') is based on the minimum
    of the two amplitudes, since this matches the beat frequency amplitude.
    */

    const auto& freqAmplPair = getHarmonicSpectrum(numPartialsPerNote, amplCallback);

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

        // std::cout << "[i=" << i << "] freq=" << fr_sorted[i] << " | amp=" << am_sorted[i]
        //           << std::endl;
    }

    // float D = 0.0f;

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
    const std::function<float(std::vector<float>)> dissCallback) const {
    const SetharesDissonanceTable table =
        getSetharesDyadsDissonanceValue(numPartialsPerNote, useMinModel, amplCallback);

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

        if (nextNoteFirstOct > currentNoteFirstOct) {
            // Special cases:
            if (nextNote.getPitchClass() == "Cb" || nextNote.getPitchClass() == "Cbb") {
                nextNote.setOctave(currentNote.getWrittenOctave() + 1);
            } else if (currentNote.getPitchClass() == "B#" || currentNote.getPitchClass() == "Bx") {
                nextNote.setOctave(currentNote.getWrittenOctave() + 1);
            } else {
                nextNote.setOctave(currentNote.getWrittenOctave());
            }

            continue;
        }

        if (nextNoteFirstOct < currentNoteFirstOct) {
            // Special cases:
            if (nextNote.getPitchClass() == "B#" || nextNote.getPitchClass() == "Bx") {
                nextNote.setOctave(currentNote.getWrittenOctave());
            } else {
                nextNote.setOctave(currentNote.getWrittenOctave() + 1);
            }
        }
    }
}

bool operator<(const HeapData& a, const HeapData& b) { return std::get<1>(a) < std::get<1>(b); }

std::ostream& operator<<(std::ostream& os, const Chord& chord) {
    const int chordSize = chord.size();
    os << "[";
    for (int i = 0; i < chordSize - 1; i++) {
        os << chord.getNote(i).getPitch() << ",";
    }
    os << chord.getNote(chordSize - 1).getPitch() + "]";
    return os;
}