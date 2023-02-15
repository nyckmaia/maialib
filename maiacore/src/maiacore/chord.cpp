#include "maiacore/chord.h"

#include <algorithm>  // std::rotate, std::count
#include <iostream>
#include <set>      // std::set
#include <utility>  // std::pair

#include "maiacore/constants.h"
#include "maiacore/helper.h"
#include "maiacore/interval.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

Chord::Chord() : _isStackedInThirds(false) {}

Chord::Chord(const std::vector<Note>& notes) : _isStackedInThirds(false) {
    for (const auto& n : notes) {
        addNote(n);
    }
}

Chord::Chord(const std::vector<std::string>& pitches) : _isStackedInThirds(false) {
    for (const auto& p : pitches) {
        addNote(Note(p));
    }
}

Chord::~Chord() {}

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

void Chord::setDurationTicks(const int durationTicks) {
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        _originalNotes[i].setDurationTicks(durationTicks);
        _openStack[i].setDurationTicks(durationTicks);
    }
}

void Chord::inversion(int inversionNumber) {
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
    const std::map<std::string, int> map{{MUSIC_XML::NOTE_TYPE::MAXIMA_DOT_DOT, 56000000},
                                         {MUSIC_XML::NOTE_TYPE::MAXIMA_DOT, 48000000},
                                         {MUSIC_XML::NOTE_TYPE::MAXIMA, 32000000},
                                         {MUSIC_XML::NOTE_TYPE::LONG_DOT_DOT, 28000000},
                                         {MUSIC_XML::NOTE_TYPE::LONG_DOT, 24000000},
                                         {MUSIC_XML::NOTE_TYPE::LONG, 16000000},
                                         {MUSIC_XML::NOTE_TYPE::BREVE_DOT_DOT, 14000000},
                                         {MUSIC_XML::NOTE_TYPE::BREVE_DOT, 12000000},
                                         {MUSIC_XML::NOTE_TYPE::BREVE, 8000000},
                                         {MUSIC_XML::NOTE_TYPE::WHOLE_DOT_DOT, 7000000},
                                         {MUSIC_XML::NOTE_TYPE::WHOLE_DOT, 6000000},
                                         {MUSIC_XML::NOTE_TYPE::WHOLE, 4000000},
                                         {MUSIC_XML::NOTE_TYPE::HALF_DOT_DOT, 3500000},
                                         {MUSIC_XML::NOTE_TYPE::HALF_DOT, 3000000},
                                         {MUSIC_XML::NOTE_TYPE::HALF, 2000000},
                                         {MUSIC_XML::NOTE_TYPE::QUARTER_DOT_DOT, 1750000},
                                         {MUSIC_XML::NOTE_TYPE::QUARTER_DOT, 1500000},
                                         {MUSIC_XML::NOTE_TYPE::QUARTER, 1000000},  // 1:1
                                         {MUSIC_XML::NOTE_TYPE::EIGHTH_DOT_DOT, 875000},
                                         {MUSIC_XML::NOTE_TYPE::EIGHTH_DOT, 750000},
                                         {MUSIC_XML::NOTE_TYPE::EIGHTH, 500000},
                                         {MUSIC_XML::NOTE_TYPE::N16TH_DOT_DOT, 437500},
                                         {MUSIC_XML::NOTE_TYPE::N16TH_DOT, 375000},
                                         {MUSIC_XML::NOTE_TYPE::N16TH, 250000},
                                         {MUSIC_XML::NOTE_TYPE::N32ND_DOT_DOT, 218750},
                                         {MUSIC_XML::NOTE_TYPE::N32ND_DOT, 187500},
                                         {MUSIC_XML::NOTE_TYPE::N32ND, 125000},
                                         {MUSIC_XML::NOTE_TYPE::N64TH_DOT_DOT, 109375},
                                         {MUSIC_XML::NOTE_TYPE::N64TH_DOT, 93750},
                                         {MUSIC_XML::NOTE_TYPE::N64TH, 62500},
                                         {MUSIC_XML::NOTE_TYPE::N128TH_DOT_DOT, 54688},
                                         {MUSIC_XML::NOTE_TYPE::N128TH_DOT, 46875},
                                         {MUSIC_XML::NOTE_TYPE::N128TH, 31250},
                                         {MUSIC_XML::NOTE_TYPE::N256TH_DOT_DOT, 27344},
                                         {MUSIC_XML::NOTE_TYPE::N256TH_DOT, 23438},
                                         {MUSIC_XML::NOTE_TYPE::N256TH, 15625},
                                         {MUSIC_XML::NOTE_TYPE::N512TH_DOT_DOT, 13672},
                                         {MUSIC_XML::NOTE_TYPE::N512TH_DOT, 11719},
                                         {MUSIC_XML::NOTE_TYPE::N512TH, 7813},
                                         {MUSIC_XML::NOTE_TYPE::N1024TH_DOT_DOT, 6836},
                                         {MUSIC_XML::NOTE_TYPE::N1024TH_DOT, 5859},
                                         {MUSIC_XML::NOTE_TYPE::N1024TH, 3906}};

    int minDuration = Helper::noteType2ticks(MUSIC_XML::NOTE_TYPE::MAXIMA_DOT_DOT, 1000000);
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
        LOG_WARN("The chord is empty!");
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
    _openStack.erase(std::unique(_openStack.begin(), _openStack.end(),
                                 [](const Note& a, const Note& b) {
                                     return a.getPitchClass() == b.getPitchClass();
                                 }),
                     _openStack.end());

    // Define the current 'chordSize'
    const int chordSize = _openStack.size();

    // Error checking:
    if (chordSize < 3) {
        LOG_WARN("The chord MUST BE at least 3 unique pitch classes!");
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
    for (auto& heap : validEnharmonicHeaps) {
        const std::vector<NoteDataHeap> heapInversions = computeAllHeapInversions(heap);
        std::vector<NoteDataHeap> tertianHeaps = filterTertianHeapsOnly(heapInversions);

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

        default:
            LOG_ERROR("Invalid chord size: " + std::to_string(chordSize));
            break;
    }

    return enharmonicHeaps;
}

std::vector<NoteDataHeap> Chord::removeHeapsWithDuplicatedPitchSteps(
    std::vector<NoteDataHeap>& heaps) const {
    const int heapsSize = heaps.size();
    std::vector<NoteDataHeap> validEnharmonicHeaps(heapsSize);

    int idx = 0;
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
        validEnharmonicHeaps[idx++] = heap;
    }

    validEnharmonicHeaps.resize(idx - 1);  // CHECAR ESSE IDX!!!

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

    std::vector<NoteDataHeap> tertianHeaps;

    for (const auto& heap : heaps) {
        bool isATertianHeap = true;
        for (int i = 0; i < heapSize - 1; i++) {
            const Note& currentNote = heap[i].note;
            const Note& nextNote = heap[i + 1].note;

            Interval interval(currentNote, nextNote);
            const int pitchStepInterval = interval.getPitchStepInterval();
            if ((pitchStepInterval != 3) && (pitchStepInterval != 5)) {
                isATertianHeap = false;
                break;
            }
        }

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
            LOG_ERROR("Invalid pitchStep");
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

std::vector<int> Chord::getMIDIIntervals(const bool firstNoteAsReference) const {
    const int numNotes = _originalNotes.size();

    if (numNotes <= 0) {
        LOG_ERROR("Chord is empty");
    }

    std::vector<int> midiIntervals(numNotes - 1);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        const int rootMidiNumber = _originalNotes[0].getMIDINumber();
        for (int i = 1; i < numNotes; i++) {
            midiIntervals[i - 1] = _originalNotes[i].getMIDINumber() - rootMidiNumber;
        }

        return midiIntervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numNotes - 1; i++) {
        midiIntervals[i] =
            _originalNotes[i + 1].getMIDINumber() - _originalNotes[i].getMIDINumber();
    }

    return midiIntervals;
}

std::vector<Interval> Chord::getIntervals(const bool firstNoteAsReference) const {
    const int numIntervals = size() - 1;

    if (numIntervals <= 0) {
        LOG_ERROR("Chord is empty");
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
        [](const Note& lh, const Note& rh) { return lh.getMIDINumber() <= rh.getMIDINumber(); });
}

bool Chord::haveMajorInterval(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajor(useEnharmony); });
}

bool Chord::haveMinorInterval(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinor(useEnharmony); });
}

bool Chord::havePerfectInterval(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isPerfect(useEnharmony); });
}

bool Chord::haveDiminishedInterval(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isDiminished(useEnharmony); });
}

bool Chord::haveAugmentedInterval(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isAugmented(useEnharmony); });
}

// ===== ABSTRACTION 1 ===== //

bool Chord::haveDiminishedUnisson(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isDiminishedUnisson(useEnharmony);
                       });
}

bool Chord::havePerfectUnisson(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isPerfectUnisson(useEnharmony);
                       });
}

bool Chord::haveAugmentedUnisson(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAugmentedUnisson(useEnharmony);
                       });
}

bool Chord::haveMinorSecond(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinorSecond(useEnharmony); });
}

bool Chord::haveMajorSecond(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajorSecond(useEnharmony); });
}

bool Chord::haveMinorThird(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinorThird(useEnharmony); });
}

bool Chord::haveMajorThird(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajorThird(useEnharmony); });
}

bool Chord::havePerfectFourth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isPerfectFourth(useEnharmony);
                       });
}

bool Chord::haveAugmentedFourth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAugmentedFourth(useEnharmony);
                       });
}

bool Chord::haveDiminishedFifth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isDiminishedFifth(useEnharmony);
                       });
}

bool Chord::havePerfectFifth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isPerfectFifth(useEnharmony); });
}

bool Chord::haveAugmentedFifth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAugmentedFifth(useEnharmony);
                       });
}

bool Chord::haveMinorSixth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinorSixth(useEnharmony); });
}

bool Chord::haveMajorSixth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajorSixth(useEnharmony); });
}

bool Chord::haveDiminishedSeventh(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isDiminishedSeventh(useEnharmony);
                       });
}

bool Chord::haveMinorSeventh(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinorSeventh(useEnharmony); });
}

bool Chord::haveMajorSeventh(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajorSeventh(useEnharmony); });
}

bool Chord::haveDiminishedOctave(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isDiminishedOctave(useEnharmony);
                       });
}

bool Chord::havePerfectOctave(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isPerfectOctave(useEnharmony);
                       });
}

bool Chord::haveAugmentedOctave(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAugmentedOctave(useEnharmony);
                       });
}

bool Chord::haveMinorNinth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMinorNinth(useEnharmony); });
}

bool Chord::haveMajorNinth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isMajorNinth(useEnharmony); });
}

bool Chord::havePerfectEleventh(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isPerfectEleventh(useEnharmony);
                       });
}

bool Chord::haveSharpEleventh(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isSharpEleventh(useEnharmony);
                       });
}

bool Chord::haveMinorThirdteenth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isMinorThirdteenth(useEnharmony);
                       });
}

bool Chord::haveMajorThirdteenth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isMajorThirdteenth(useEnharmony);
                       });
}

// ===== ABSTRACTION 2 ===== //
bool Chord::haveSecond(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isSecond(useEnharmony); });
}

bool Chord::haveThird(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isThird(useEnharmony); });
}

bool Chord::haveFourth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isFourth(useEnharmony); });
}

bool Chord::haveFifth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isFifth(useEnharmony); });
}

bool Chord::haveSixth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isSixth(useEnharmony); });
}

bool Chord::haveSeventh(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isSeventh(useEnharmony); });
}

bool Chord::haveOctave(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isOctave(useEnharmony); });
}

bool Chord::haveNinth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isNinth(useEnharmony); });
}

bool Chord::haveEleventh(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isEleventh(useEnharmony); });
}

bool Chord::haveThirdteenth(const bool useEnharmony) const {
    return std::any_of(
        _closeStackintervals.begin(), _closeStackintervals.end(),
        [useEnharmony](const Interval& interval) { return interval.isThirdteenth(useEnharmony); });
}

// ===== ABSTRACTION 3 ===== //
bool Chord::haveAnyOctaveMinorSecond(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorSecond(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorSecond(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorSecond(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMinorThird(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorThird(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorThird(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorThird(useEnharmony);
                       });
}

bool Chord::haveAnyOctavePerfectFourth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctavePerfectFourth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveAugmentedFourth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveAugmentedFourth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveDiminhavehedFifth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveDiminishedFifth(useEnharmony);
                       });
}

bool Chord::haveAnyOctavePerfectFifth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctavePerfectFifth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveAugmentedFifth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveAugmentedFifth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMinorSixth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorSixth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorSixth(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorSixth(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveDiminhavehedSeventh(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveDiminishedSeventh(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMinorSeventh(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMinorSeventh(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveMajorSeventh(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveMajorSeventh(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveDiminhavehedOctave(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveDiminishedOctave(useEnharmony);
                       });
}

bool Chord::haveAnyOctavePerfectOctave(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctavePerfectOctave(useEnharmony);
                       });
}

bool Chord::haveAnyOctaveAugmentedOctave(const bool useEnharmony) const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [useEnharmony](const Interval& interval) {
                           return interval.isAnyOctaveAugmentedOctave(useEnharmony);
                       });
}

// ===== ABSTRACTION 4 ===== //
bool Chord::haveAnyOctaveSecond() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveSecond(); });
}

bool Chord::haveAnyOctaveThird() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveThird(); });
}

bool Chord::haveAnyOctaveFourth() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveFourth(); });
}

bool Chord::haveAnyOctaveFifth() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveFifth(); });
}

bool Chord::haveAnyOctaveSixth() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveSixth(); });
}

bool Chord::haveAnyOctaveSeventh() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
                       [](const Interval& interval) { return interval.isAnyOctaveSeventh(); });
}

bool Chord::haveAnyOctaveOctave() const {
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(),
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

        const int currentNoteFirstOct = currentNote.getMIDINumber() % 12;
        const int nextNoteFirstOct = nextNote.getMIDINumber() % 12;

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
