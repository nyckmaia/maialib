#include "chord.h"
#include "helper.h"

Chord::Chord() :
    _isStackedInThirds(false)
{

}

Chord::Chord(const std::vector<Note>& notes) :
    _isStackedInThirds(false)
{
    for (const auto& n : notes) {
        addNote(n);
    }
}

Chord::Chord(const std::vector<std::string>& pitches) :
    _isStackedInThirds(false)
{
    for (const auto& p : pitches) {
        addNote(Note(p));
    }
}

Chord::~Chord()
{

}

void Chord::clear()
{
    _originalNotes.clear();
    _openStack.clear();
}

void Chord::info()
{
    std::cout << "=====> CHORD <=====" << std::endl;
    std::cout << "Name: " << getName() << std::endl;
    std::cout << "Size:" << size() << std::endl;

    const int chordSize = size();
    std::string noteNames = "[";

    for (int i = 0; i < chordSize-1; i++) {
        noteNames.append(_originalNotes[i].getPitch() + ", ");
    }

    // Add the last note without the semicomma in the end
    noteNames.append(_originalNotes[chordSize-1].getPitch());

    noteNames.append("]");

    std::cout << noteNames << std::endl;

    // ---------------- //
    const int chordStackSize = stackSize();

    std::cout << "=====> CHORD STACK <=====" << std::endl;
    std::cout << "Open Stack Size:" << chordStackSize << std::endl;

    std::string stackNames = "[";

    for (int i = 0; i < chordStackSize-1; i++) {
        stackNames.append(_openStack[i].getPitch() + ", ");
    }

    // Add the last note without the semicomma in the end
    stackNames.append(_openStack[chordStackSize-1].getPitch());

    stackNames.append("]");

    std::cout << stackNames << std::endl;

}

void Chord::addNote(const Note& note)
{
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

void Chord::addNote(const std::string& pitch)
{
    addNote(Note(pitch));
}

void Chord::removeTopNote()
{
    _originalNotes.pop_back();

    // Reset the chord stacked flag
    _isStackedInThirds = false;
}

void Chord::insertNote(Note& note, int noteIndex)
{
    note.setIsInChord(true);
    _originalNotes.insert(_originalNotes.begin() + noteIndex, note);

    _openStack.push_back(note);

    // Reset the chord stacked flag
     _isStackedInThirds = false;
}

void Chord::removeNote(int noteIndex)
{
    _originalNotes.erase(_originalNotes.begin() + noteIndex);

    // Reset the chord stacked flag
     _isStackedInThirds = false;
}

void Chord::setDurationTicks(const int durationTicks)
{
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        _originalNotes[i].setDurationTicks(durationTicks);
        _openStack[i].setDurationTicks(durationTicks);
    }
}

void Chord::inversion(int inversionNumber)
{
    for (int i = 0; i < inversionNumber; i++) {
        _originalNotes[0].transpose(12);  // isto apenas altera a nota uma oitava acima

        const Note& x = _originalNotes[0];  // pega a nota que foi alterada acima
        _originalNotes.push_back(x);
        _originalNotes.erase(_originalNotes.begin());
    }
}

void Chord::transpose(const int semitonesNumber)
{
    if (semitonesNumber == 0) { return; }

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

void Chord::transposeStackOnly(const int semitonesNumber)
{
    if (semitonesNumber == 0) { return; }

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

void Chord::removeDuplicateNotes()
{
    sortNotes();
    _originalNotes.erase(std::unique(_originalNotes.begin(), _originalNotes.end() ), _originalNotes.end());
}

std::vector<HeapData> Chord::getStackedHeaps(const bool enharmonyNotes)
{
    if (!_isStackedInThirds) { stackInThirds(enharmonyNotes); }

    return _stackedHeaps;
}

std::string Chord::getDuration() const
{
    const std::map<std::string, int> map {
        { MUSIC_XML::NOTE_TYPE::MAXIMA_DOT_DOT, 56000000 },
        { MUSIC_XML::NOTE_TYPE::MAXIMA_DOT, 48000000 },
        { MUSIC_XML::NOTE_TYPE::MAXIMA, 32000000 },
        { MUSIC_XML::NOTE_TYPE::LONG_DOT_DOT, 28000000 },
        { MUSIC_XML::NOTE_TYPE::LONG_DOT, 24000000 },
        { MUSIC_XML::NOTE_TYPE::LONG, 16000000 },
        { MUSIC_XML::NOTE_TYPE::BREVE_DOT_DOT, 14000000 },
        { MUSIC_XML::NOTE_TYPE::BREVE_DOT, 12000000 },
        { MUSIC_XML::NOTE_TYPE::BREVE, 8000000 },
        { MUSIC_XML::NOTE_TYPE::WHOLE_DOT_DOT, 7000000 },
        { MUSIC_XML::NOTE_TYPE::WHOLE_DOT, 6000000 },
        { MUSIC_XML::NOTE_TYPE::WHOLE, 4000000 },
        { MUSIC_XML::NOTE_TYPE::HALF_DOT_DOT, 3500000 },
        { MUSIC_XML::NOTE_TYPE::HALF_DOT, 3000000 },
        { MUSIC_XML::NOTE_TYPE::HALF, 2000000 },
        { MUSIC_XML::NOTE_TYPE::QUARTER_DOT_DOT, 1750000 },
        { MUSIC_XML::NOTE_TYPE::QUARTER_DOT, 1500000},
        { MUSIC_XML::NOTE_TYPE::QUARTER, 1000000 }, // 1:1
        { MUSIC_XML::NOTE_TYPE::EIGHTH_DOT_DOT, 875000 },
        { MUSIC_XML::NOTE_TYPE::EIGHTH_DOT, 750000 },
        { MUSIC_XML::NOTE_TYPE::EIGHTH, 500000 },
        { MUSIC_XML::NOTE_TYPE::N16TH_DOT_DOT, 437500 },
        { MUSIC_XML::NOTE_TYPE::N16TH_DOT, 375000 },
        { MUSIC_XML::NOTE_TYPE::N16TH, 250000 },
        { MUSIC_XML::NOTE_TYPE::N32ND_DOT_DOT, 218750 },
        { MUSIC_XML::NOTE_TYPE::N32ND_DOT, 187500 },
        { MUSIC_XML::NOTE_TYPE::N32ND, 125000 },
        { MUSIC_XML::NOTE_TYPE::N64TH_DOT_DOT, 109375 },
        { MUSIC_XML::NOTE_TYPE::N64TH_DOT, 93750 },
        { MUSIC_XML::NOTE_TYPE::N64TH, 62500 },
        { MUSIC_XML::NOTE_TYPE::N128TH_DOT_DOT, 54688 },
        { MUSIC_XML::NOTE_TYPE::N128TH_DOT, 46875 },
        { MUSIC_XML::NOTE_TYPE::N128TH, 31250 },
        { MUSIC_XML::NOTE_TYPE::N256TH_DOT_DOT, 27344 },
        { MUSIC_XML::NOTE_TYPE::N256TH_DOT, 23438 },
        { MUSIC_XML::NOTE_TYPE::N256TH, 15625 },
        { MUSIC_XML::NOTE_TYPE::N512TH_DOT_DOT, 13672 },
        { MUSIC_XML::NOTE_TYPE::N512TH_DOT, 11719 },
        { MUSIC_XML::NOTE_TYPE::N512TH, 7813 },
        { MUSIC_XML::NOTE_TYPE::N1024TH_DOT_DOT, 6836 },
        { MUSIC_XML::NOTE_TYPE::N1024TH_DOT, 5859 },
        { MUSIC_XML::NOTE_TYPE::N1024TH, 3906 }
    };

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
          break; // to stop searching
       }
    }

    return keyNote;
}

float Chord::getQuarterDuration() const
{
    std::string duration = getDuration();
    const int ticks = Helper::noteType2ticks(duration, 256);

    return static_cast<float>(ticks) / 256.0f;
}

int Chord::size() const
{
    return static_cast<int>(_originalNotes.size());
}

int Chord::getDurationTicks() const
{
    // Error check
    if (_originalNotes.empty()) { return 0; }

    // For each internal note, get the minimum duration ticks value
    int minValue = _originalNotes[0].getDurationTicks();
    for (const auto& note : _originalNotes) {
        if (note.getDurationTicks() < minValue) {
            minValue = note.getDurationTicks();
        }
    }

    return minValue;
}

Note& Chord::getNote(int noteIndex)
{
   return _originalNotes[noteIndex];
}

const Note& Chord::getNote(const int noteIndex) const
{
   return _originalNotes[noteIndex];
}

void Chord::print() const
{
    const int chordSize = _originalNotes.size();

    for (int i = 0; i < chordSize; i++) {
        std::cout << "note[" << i << "] = " << _originalNotes[i].getPitch() << std::endl;
    }
}

void Chord::printStack() const
{
    const int stackSize = _openStack.size();

    for (int i = 0; i < stackSize; i++) {
        std::cout << "openStack[" << i << "] = " << _openStack[i].getPitch() << std::endl;
    }
}

void Chord::stackInThirds(const bool enharmonyNotes)
{
    // REVER ESSA SEQUENCIA! MUDOU!
    // a) Calcular o numero de possibilidades enharmonicas = (tamanho do acorde)^3
    // b) Gerar uma strtutura contendo:
    //    1) Os 3 vetores contendo a versão original + 2 versões enarmonicas do acorde
    //    2) Cada elemento associdado com um int que diz se ele foi enharmonizado ou nao (0, 1, 2)
    //       2.1) Note note
    //       2.2) booleano wasEnharmonized
    //       2.3) int enharmonicDiatonicDistance 
    // c) Criar 3 for's, um dentro do outro. Cada for é para um dos nomes enarmonicos
    // d) Dentro do for mais interno:
    //    1) verifica se possui pitchClass iguais.
    //      Se sim, exclui a possibilidade do vetor de empilhamentos possiveis
    //    2) Salva a possibilidade de empilhamento atual armazenando:
    // f) Para cada possibilidade de empilhamento (for):
    //    1) Rotacionar os pitches de modo que eles fiquem empilhados em terças
    //    2) Compara com os gabaritos de empilhamento em terças
    //    3) Na comparação: idxPossibilidade, Somatória do pontos
    //       Pensar como contabilizar os "descontos" por ter tido alguma nota enharmonizada
    // g) Selecionar o empilhamento que deu mais pontos 
    // h) Coloca ela em root position?
    // i) Faz a tranposição para a oitava 4
    // j) Seta a flag interna isStackedInThirds e computeIntervals()
    ignore(enharmonyNotes);

    // ===== STEP 0: INPUT VALIDATION ===== //
    // Error checking:
    if (_originalNotes.empty()) {
        std::cerr << "[WARN]: The chord is empty!" << std::endl;
        return;
    }

    // Remove rests
    _originalNotes.erase(
        std::remove_if(_originalNotes.begin(), _originalNotes.end(), [&](const Note& note) {
            return note.isNoteOff();
        }), _originalNotes.end());

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
    }), _openStack.end());

    // Define the current 'chordSize'
    const int chordSize = _openStack.size();

    // Error checking:
    if (chordSize < 3) {
        std::cout << "[WARN]: The chord MUST BE at least 3 unique pitch classes!" << std::endl;
        return;
    }

    // ===== STEP 2: COMPUTE ENHARMONIC UNIT GROUPS ===== //
    std::vector<Heap> enharmonicUnitGroups = computeEnharmonicUnitsGroups();

    // ===== STEP 3: COMPUTE ENHARMONIC HEAPS ===== //
    std::vector<Heap> enharmonicHeaps = computeEnharmonicHeaps(enharmonicUnitGroups);

    // ===== STEP 4: COMPUTE THE STACK IN THIRDS TEMPLATE MATCH ===== //
    for (auto& heap : enharmonicHeaps) {
        const std::vector<Heap>& heapInversions = computeAllHeapInversions(heap);
        const std::vector<Heap>& tertianHeaps = filterTertianHeapsOnly(heapInversions);
        for (const auto& heapInversion :  tertianHeaps) {
            // Check if the first heap interval is a musical third
            const auto& firstNote = heapInversion[0].note;
            const auto& secondNote = heapInversion[1].note;
            const Interval firstInterval(firstNote, secondNote);

            // Skip chords without the tonal major/minor third
            if (firstInterval.getPitchStepInterval() != 3) { continue; }

            const auto& heapData = stackInThirdsTemplateMatch(heapInversion);
            _stackedHeaps.push_back(heapData);
        }
    }

    // ===== STEP 5: SORT HEAPS BY STACK IN THIRDS MATCHING VALUE ===== //
    std::sort(_stackedHeaps.begin(), _stackedHeaps.end(), std::greater<>());

    // ===== STEP 6: COMPUTE THE CLOSE STACK VERSION ===== //
    const auto& bestStackedHeap = _stackedHeaps[0];
    const auto& heap = std::get<0>(bestStackedHeap);
    std::vector<Note> bestStackedHeapNotes(chordSize);
    for (int i = 0; i < chordSize; i++) {
        bestStackedHeapNotes[i] = heap[i].note;
    }

    computeCloseStack(bestStackedHeapNotes);

    // ===== STEP 7: SET INTERNAL FLAG AND COMPUTE INTERVALS ===== //
    // Set a internal control flag
    _isStackedInThirds = true;

    _closeStackintervals = Helper::notes2Intervals(_closeStack, true);
}

std::vector<Heap> Chord::computeEnharmonicUnitsGroups() const
{
    const int chordSize = _openStack.size();
    std::vector<Heap> enharUnitGroups(chordSize);
    const int numUnitGroups = enharUnitGroups.size();

    const int numOfEnharmonics = 3;

    for (int i = 0; i < numUnitGroups; i++) {
        Heap unitGroup(numOfEnharmonics);

        Note originalNote = _openStack[i]; // must be a copy
        unitGroup[0] = NoteData(originalNote, false, 0);
        unitGroup[1] = NoteData(originalNote.getEnharmonicNote(false), true, 1);
        unitGroup[2] = NoteData(originalNote.getEnharmonicNote(true), true, 2);

        enharUnitGroups[i] = unitGroup;
    }

    return enharUnitGroups;
}

std::vector<Heap> Chord::computeEnharmonicHeaps(const std::vector<Heap>& heaps) const
{
    const int chordSize = heaps.size();
    const int numEnharmonicHeapVariants = std::pow(3, chordSize);

    std::vector<Heap> enharmonicHeaps(numEnharmonicHeapVariants);
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
                            enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2], 
                                                       heaps[2][i3], heaps[3][i4]}};
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
                                enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2], 
                                                           heaps[2][i3], heaps[3][i4],
                                                           heaps[4][i5]}};
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
                                        enharmonicHeaps[idx++] = {{heaps[0][i1], heaps[1][i2], 
                                                                   heaps[2][i3], heaps[3][i4],
                                                                   heaps[4][i5], heaps[5][i6],
                                                                   heaps[6][i7]}};
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
    
        default: throw std::runtime_error("Invalid chord size: " + std::to_string(chordSize)); break;
    }
    
    return enharmonicHeaps;
}


std::vector<Heap> Chord::computeAllHeapInversions(Heap& heap) const
{
    std::vector<Heap> heapInversions(factorial(heap.size()));
    int i = 0;

    do {
        heapInversions[i++] = heap;
    } while(std::next_permutation(heap.begin(), heap.end()));

    return heapInversions;
}

std::vector<Heap> Chord::filterTertianHeapsOnly(const std::vector<Heap>& heaps) const
{
    if (heaps.empty()) { throw std::runtime_error("heaps vector is empty"); }
    
    const int heapSize = heaps[0].size();
    
    std::vector<Heap> tertianHeaps;

    for (const auto& heap : heaps) {
        bool isATertianHeap = true;
        for (int i = 0; i < heapSize-1; i++) {
            const Note& currentNote = heap[i].note;
            const Note& nextNote = heap[i+1].note;

            Interval interval(currentNote, nextNote);
            const int pitchStepInterval = interval.getPitchStepInterval();
            if ((pitchStepInterval != 3) && (pitchStepInterval != 5)) {
                isATertianHeap = false;
                // talvez adicionar um break? quebra ambos loops?
            }
        }

        if (isATertianHeap) {
            tertianHeaps.push_back(heap);
        }
    }

    return tertianHeaps;
}

void Chord::computeCloseStack(const std::vector<Note>& openStack)
{
    _closeStack = openStack;
    const Note& rootNote = _closeStack[0];

    const std::map<char, int>* closeStackOctavesMap = nullptr;

    switch (hash(rootNote.getPitchStep().c_str())) {
        case hash("C"): closeStackOctavesMap = &c_C_closeStackOctavesMap; break;
        case hash("D"): closeStackOctavesMap = &c_D_closeStackOctavesMap; break;
        case hash("E"): closeStackOctavesMap = &c_E_closeStackOctavesMap; break;
        case hash("F"): closeStackOctavesMap = &c_F_closeStackOctavesMap; break;
        case hash("G"): closeStackOctavesMap = &c_G_closeStackOctavesMap; break;
        case hash("A"): closeStackOctavesMap = &c_A_closeStackOctavesMap; break;
        case hash("B"): closeStackOctavesMap = &c_B_closeStackOctavesMap; break;
        default: 
            throw std::runtime_error("[maiacore] Invalid pitchStep: " + rootNote.getPitchStep());
            break;
    }

    for (auto& note : _closeStack) {
        const char* pitchStep = note.getPitchStep().c_str();
        const int oct = closeStackOctavesMap->at(*pitchStep);
        note.setOctave(oct);
    }
}

HeapData Chord::stackInThirdsTemplateMatch(const Heap& heap) const
{
    // std::cout << "===== PRINTING HEAPS INSIDE stackInThirdsTemplateMatch =====" << std::endl;
    // for (const auto& noteData : heap) {
    //     std::cout << noteData.note.getPitch() << " ";
    // }
    // std::cout << std::endl;

    const int heapSize = heap.size();

    // Compute the max heap count points value
    int maxHeapPointsValue = 0;
    for (int i = 0; i < heapSize; i++) {
        maxHeapPointsValue += std::pow(2, heapSize-i);
    }

    // std::cout << "maxHeapPointsValue: " << maxHeapPointsValue << std::endl;

    HeapData heapData;
    std::get<0>(heapData) = heap;
    float* heapCountPoints = &std::get<1>(heapData);
    *heapCountPoints = 0.0f;
    const NoteData& rootNoteData = heap[0];
    const Note& rootNote = rootNoteData.note;
    Interval interval(rootNote, rootNote); // temp interval object
    
    // Iterate over each heap interval (rootNote as base reference)
    for (int i = 0; i < heapSize; i++) {
        interval.setNotes(rootNote, heap[i].note);
        
        float enharmonicNoteWeight = 0.0f;
        switch (heap[i].enharmonicDiatonicDistance) {
            case 0: enharmonicNoteWeight = 1.0f; break;
            case 1: enharmonicNoteWeight = 0.8f; break;
            case 2: enharmonicNoteWeight = 0.6f; break;
            default: throw std::runtime_error("Invalid enharmonic diatonic distance"); break;
        }
        const float stackPositionWeight = std::pow(2, heapSize-i);
        *heapCountPoints += stackPositionWeight * enharmonicNoteWeight;        
    }

    // Normalize heap Count Points
    *heapCountPoints /= maxHeapPointsValue;

    return heapData;  
}

void Chord::computeIntervals()
{
    const std::vector<Interval> intervals = Helper::notes2Intervals(_closeStack);
    ignore(intervals);
}

// void Chord::computeIntervals()
// {
//     const int chordSize = _openStack.size();

//     // ===== STEP 1: GET RIGHT POSITION STACKED INTERVALS ====== //
//     _midiInterval.clear();
//     _midiInterval.resize(chordSize - 1, 0);

//     // Get the root note
//     const Note& rootNote = _openStack[0];

//     // Get the number of _openStack intervals
//     const int intervalSize = chordSize - 1;

//     // Compute interval for each _openStack note
//     for (int i = 0; i < intervalSize; i++) {

//         Note& previusNote = (i == 0) ? _openStack[0] : _openStack[i];
//         Note& nextNote = _openStack[i+1];

//         const int currentOct = previusNote.getOctave();

//         // To get only ascendent intervals, the 'nextNote' MUST BE ALWAYS higher then the 'previusNote'
//         // So we check this condition and change the 'nextNote' octave if is necessary
//         if ((nextNote.getMIDINumber() % 12) > (previusNote.getMIDINumber() % 12)) {
//             nextNote.setOctave(currentOct);
//         } else {
//             nextNote.setOctave(currentOct + 1);
//         }

//         // Compute the absolute semitones interval between the root and the 'nextNote'
//         _midiInterval[i] = nextNote.getMIDINumber() - rootNote.getMIDINumber();
//     }

//     // ===== STEP 2: GET CHORD INTERVALS ====== //

//     // Verify the existence of each _stack interval
//     // Third's
//     haveMinorThird() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_THIRD) != _midiInterval.end();
//     haveMajorThird() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_THIRD) != _midiInterval.end();

//     // Fifth's
//     haveDiminishedFifth() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::DIMINISHED_FIFTH) != _midiInterval.end();
//     havePerfectFifth() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::PERFECT_FIFTH) != _midiInterval.end();
//     haveAugmentedFifth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_SIXTH) != _midiInterval.end();

//     // Seventh's
//     haveDiminishedSeventh() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_SIXTH) != _midiInterval.end();
//     haveMinorSeventh() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_SEVENTH) != _midiInterval.end();
//     haveMajorSeventh() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_SEVENTH) != _midiInterval.end();

//     // Ninth's
//     haveMinorNinth() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_NINTH) != _midiInterval.end();
//     haveMajorNinth() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_NINTH) != _midiInterval.end();

//     // Eleventh's
//     havePerfectEleventh() = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::PERFECT_ELEVENTH) != _midiInterval.end();

//     const bool haveSharpElevenFlatTwelve = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::SHARP_ELEVEN) != _midiInterval.end();

//     if (haveSharpElevenFlatTwelve) {
//         const auto it = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::SHARP_ELEVEN);
//         const int index = it - _midiInterval.begin();
//         const Note& note = _openStack[index];

//         // Distinguish between 'diminished fifth' and 'sharp eleven'
//         if (note.getAlterSymbol() == MUSIC_XML::ACCIDENT::SHARP) {
//             haveSharpEleventh() = true;
//         } else {
//             haveDiminishedFifth() = true;
//         }
//     }

//     // Thirdteenth's
//     haveMinorThird()teenth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_THIRTEENTH) != _midiInterval.end();
//     haveMajorThird()teenth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_THIRTEENTH) != _midiInterval.end();
// }

std::vector<int> Chord::getMIDIIntervals(const bool firstNoteAsReference) const
{
    const int numNotes = _originalNotes.size();

    if (numNotes <= 0) {
        throw std::runtime_error("[maiacore] Chord is empty");
    }

    std::vector<int> midiIntervals(numNotes - 1);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        const int rootMidiNumber = _originalNotes[0].getMIDINumber();
        for (int i = 1; i < numNotes; i++) {
            midiIntervals[i-1] = _originalNotes[i].getMIDINumber() - rootMidiNumber;
        }

        return midiIntervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numNotes - 1; i++) {
        midiIntervals[i] = _originalNotes[i+1].getMIDINumber() - _originalNotes[i].getMIDINumber();
    }

    return midiIntervals;
}

std::vector<Interval> Chord::getIntervals(const bool firstNoteAsReference) const
{
    const int numIntervals = size() - 1;

    if (numIntervals <= 0) {
        throw std::runtime_error("[maiacore] Chord is empty");
    }

    std::vector<Interval> intervals(numIntervals);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(_originalNotes[0], _originalNotes[i+1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(_originalNotes[i], _originalNotes[i+1]);
    }

    return intervals;
}

std::vector<Interval> Chord::getOpenStackIntervals(const bool firstNoteAsReference)
{
    if (!_isStackedInThirds) { stackInThirds(); }

    const int numIntervals = stackSize() - 1;
    std::vector<Interval> intervals(numIntervals);

    // ===== GET INTERVALS USING THE FIRST NOTE AS REFERENCE ===== //
    if (firstNoteAsReference) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(_openStack[0], _openStack[i+1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(_openStack[i], _openStack[i+1]);
    }

    return intervals;
}
std::vector<Interval> Chord::getCloseStackIntervals(const bool fromRoot)
{
    if (!_isStackedInThirds) { stackInThirds(); }

    const int numIntervals = stackSize() - 1;
    std::vector<Interval> intervals(numIntervals);

    // ===== GET INTERVALS FROM ROOT ===== //
    if (fromRoot) {
        for (int i = 0; i < numIntervals; i++) {
            intervals[i] = Interval(_closeStack[0], _closeStack[i+1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals[i] = Interval(_closeStack[i], _closeStack[i+1]);
    }

    return intervals;
}

std::vector<Note> Chord::getOpenStackNotes() const
{
    return _openStack;
}

std::string Chord::enharmonicName()
{
    // ===== STEP 1: CHECK IF THE CHORD IS TONAL OR NOT ====== //
    if (!isTonal()) {
        std::cout << "[WARNING] Unable to get a tonal name of a non-tonal chord" << std::endl;
        return {}; 
    }

    // Error checking
    if (!haveMinorThird() && !haveMajorThird()) {
        // std::cerr << "[ERROR]: Unable to classify this chord!" << std::endl;
        return {};
    }

    // ===== STEP 2: SET THE CHORD BASIC CLASSIFICATIION ====== //

    std::string basicClassification;

    // Set the note 'basicClassification' using the third, fifth and seventh intervals
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
    if (_closeStack[0].getPitchClass() != _bassNote.getPitchClass() && !_bassNote.getPitchClass().empty()) {
        bassNoteStr.append("/" + _bassNote.getPitchClass());
    }

    // ===== STEP 5: SET CHORD NAME ====== //
    // Concatenate all _stack extensions
    const std::string chordName = _closeStack[0].getPitchClass() + basicClassification + ninth + eleventh + thirdteenth + bassNoteStr;

    return chordName;
}

int Chord::stackSize() const
{
    return static_cast<int>(_openStack.size());
}

const Note& Chord::getRoot()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return _openStack.at(0);
}

std::string Chord::nonEnharmonicName() const
{
    return "not implemented yet";
}

std::string Chord::getName(const bool useEnharmonicName)
{
    return (useEnharmonicName) ? enharmonicName() : nonEnharmonicName();
}

bool Chord::isMajorChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (haveMajorThird() &&  !haveAugmentedFifth()) ? true : false;
}

bool Chord::isMinorChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (haveMinorThird() && !haveDiminishedFifth()) ? true : false;
}

bool Chord::isAugmentedChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (haveMajorThird() && haveAugmentedFifth()) ? true : false;
}

bool Chord::isDiminishedChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (haveMinorThird() && haveDiminishedFifth() && haveMinorSeventh()) ? true : false;
}

bool Chord::isDominantSeventhChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (haveMajorThird() && !haveDiminishedFifth() && !haveAugmentedFifth() && haveMinorSeventh()) ? true : false;
}

bool Chord::isHalfDiminishedChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (haveMinorThird() && haveDiminishedFifth() ) ? true : false;
}

bool Chord::isTonal(std::function<bool(const Chord& chord)> model)
{
    if (!_isStackedInThirds) { stackInThirds(); }

    if (model != nullptr) { return model(*this); }

    for (int i = 0; i < stackSize() - 1; i++) {
        const Interval interval(_openStack[i], _openStack[i+1]);

        if (!interval.isTonal()) { return false; }
    }

    return true;
}

bool Chord::haveMinorThird()
{   
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMinorThird();
    });
}

bool Chord::haveMajorThird()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMajorThird();
    });
}

bool Chord::haveDiminishedFifth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isDiminishedFifth();
    });
}

bool Chord::havePerfectFifth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isPerfectFifth();
    });
}

bool Chord::haveAugmentedFifth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isAugmentedFifth();
    });
}

bool Chord::haveDiminishedSeventh()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isDiminishedSeventh();
    });
}

bool Chord::haveMinorSeventh()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMinorSeventh();
    });
}

bool Chord::haveMajorSeventh()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMajorSeventh();
    });
}

bool Chord::haveMinorNinth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMinorNinth();
    });
}

bool Chord::haveMajorNinth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMajorNinth();
    });
}

bool Chord::havePerfectEleventh()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isPerfectEleventh();
    });
}

bool Chord::haveSharpEleventh()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isSharpEleventh();
    });
}

bool Chord::haveMinorThirdteenth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMinorThirdteenth();
    });
}

bool Chord::haveMajorThirdteenth()
{
    return std::any_of(_closeStackintervals.begin(), _closeStackintervals.end(), 
        [](const Interval& interval) {
        return interval.isMajorThirdteenth();
    });
}

const Note& Chord::getBassNote()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return _bassNote;
}

const std::vector<Note>& Chord::getNotes() const
{
    return _originalNotes;
}

Chord Chord::getOpenStackChord(const bool enharmonyNotes)
{
    if (!_isStackedInThirds) { stackInThirds(enharmonyNotes); }

    return Chord(_openStack);
}

Chord Chord::getCloseStackChord(const bool enharmonyNotes)
{
    if (!_isStackedInThirds) { stackInThirds(enharmonyNotes); }
    
    return Chord(_closeStack);
}

void Chord::sortNotes()
{
    std::sort(_originalNotes.begin(), _originalNotes.end());
}

void printHeap(const Heap& heap) {
    for (const auto& noteData : heap) {
        std::cout << "printHeap: " << noteData.note.getPitch() << " ";
    }
    std::cout << std::endl;
}

bool operator<(const HeapData& a, const HeapData& b){
    return std::get<1>(a) < std::get<1>(b);
}
