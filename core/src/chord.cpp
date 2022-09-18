#include "chord.h"
#include "helper.h"

Chord::Chord() :
    _isStackedInThirds(false),
    _haveMinorThird(false),
    _haveMajorThird(false),
    _haveDiminishedFifth(false),
    _havePerfectFifth(false),
    _haveAugmentedFifth(false),
    _haveDiminishedSeventh(false),
    _haveMinorSeventh(false),
    _haveMajorSeventh(false),
    _haveMinorNinth(false),
    _haveMajorNinth(false),
    _havePerfectEleventh(false),
    _haveSharpEleventh(false),
    _haveMinorThirdteenth(false),
    _haveMajorThirdteenth(false)
{

}

Chord::Chord(const std::vector<Note>& notes) :
    _isStackedInThirds(false),
    _haveMinorThird(false),
    _haveMajorThird(false),
    _haveDiminishedFifth(false),
    _havePerfectFifth(false),
    _haveAugmentedFifth(false),
    _haveDiminishedSeventh(false),
    _haveMinorSeventh(false),
    _haveMajorSeventh(false),
    _haveMinorNinth(false),
    _haveMajorNinth(false),
    _havePerfectEleventh(false),
    _haveSharpEleventh(false),
    _haveMinorThirdteenth(false),
    _haveMajorThirdteenth(false)
{
    for (const auto& n : notes) {
        addNote(n);
    }
}

Chord::Chord(const std::vector<std::string>& pitches) :
    _isStackedInThirds(false),
    _haveMinorThird(false),
    _haveMajorThird(false),
    _haveDiminishedFifth(false),
    _havePerfectFifth(false),
    _haveAugmentedFifth(false),
    _haveDiminishedSeventh(false),
    _haveMinorSeventh(false),
    _haveMajorSeventh(false),
    _haveMinorNinth(false),
    _haveMajorNinth(false),
    _havePerfectEleventh(false),
    _haveSharpEleventh(false),
    _haveMinorThirdteenth(false),
    _haveMajorThirdteenth(false)
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
    _note.clear();
    _stack.clear();
}

void Chord::info()
{
    std::cout << "=====> CHORD <=====" << std::endl;
    std::cout << "Name: " << getName() << std::endl;
    std::cout << "Size:" << size() << std::endl;


    const size_t chordSize = size();
    std::string noteNames = "[";

    for (size_t i = 0; i < chordSize-1; i++) {
        noteNames.append(_note[i].getPitch() + ", ");
    }

    // Add the last note without the semicomma in the end
    noteNames.append(_note[chordSize-1].getPitch());

    noteNames.append("]");

    std::cout << noteNames << std::endl;

    // ---------------- //
    const size_t chordStackSize = stackSize();

    std::cout << "=====> CHORD STACK <=====" << std::endl;
    std::cout << "Stack Size:" << chordStackSize << std::endl;


    std::string stackNames = "[";

    for (size_t i = 0; i < chordStackSize-1; i++) {
        stackNames.append(_stack[i].getPitch() + ", ");
    }

    // Add the last note without the semicomma in the end
    stackNames.append(_stack[chordStackSize-1].getPitch());

    stackNames.append("]");

    std::cout << stackNames << std::endl;

}

void Chord::addNote(const Note& note)
{
    // Add this note in the both chord versions
    _note.push_back(note);
    _stack.push_back(note);

    if (size() == 1) {
        // The first note MUST be setted as inChord false
        _note[0].setIsInChord(false);
        _stack[0].setIsInChord(false);
    } else {
        _note.back().setIsInChord(true);
        _stack.back().setIsInChord(true);

        // const std::string& noteType = _note.begin()->getType();
        // _note.back().setType(noteType);
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
    _note.pop_back();

    // Reset the chord stacked flag
    _isStackedInThirds = false;
}

void Chord::insertNote(Note& note, int noteIndex)
{
    note.setIsInChord(true);
    _note.insert(_note.begin() + noteIndex, note);

    _stack.push_back(note);

    // Reset the chord stacked flag
     _isStackedInThirds = false;
}

void Chord::removeNote(int noteIndex)
{
    _note.erase(_note.begin() + noteIndex);

    // Reset the chord stacked flag
     _isStackedInThirds = false;
}

void Chord::setDurationTicks(const int durationTicks)
{
    const int chordSize = _note.size();

    for (int i = 0; i < chordSize; i++) {
        _note[i].setDurationTicks(durationTicks);
        _stack[i].setDurationTicks(durationTicks);
    }
}

void Chord::inversion(int inversionNumber)
{
    for (int i = 0; i < inversionNumber; i++) {
        _note[0].transpose(12);  // isto apenas altera a nota uma oitava acima

        const Note& x = _note[0];  // pega a nota que foi alterada acima
        _note.push_back(x);
        _note.erase(_note.begin());
    }
}

void Chord::transpose(const int semitonesNumber)
{
    if (semitonesNumber == 0) { return; }

    std::string pitchClass, pitchStep, alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;

    // Transpose the original chord
    for (size_t i = 0; i < _note.size(); i++) {
        const std::string pitch = _note[i].getWrittenPitch();

        int midinumber = Helper::pitch2midiNote(pitch) + semitonesNumber;

        Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

        const std::string newPitch = Helper::midiNote2pitch(midinumber, alterSymbol);

        _note[i].setPitch(newPitch);
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
    for (size_t i = 0; i < _stack.size(); i++) {
        const std::string pitch = _stack[i].getWrittenPitch();

        int midinumber = Helper::pitch2midiNote(pitch) + semitonesNumber;

        Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

        const std::string newPitch = Helper::midiNote2pitch(midinumber, alterSymbol);

        _stack[i].setPitch(newPitch);
    }
}

void Chord::removeDuplicateNotes()
{
    sortNotes();
    _note.erase(std::unique(_note.begin(), _note.end() ), _note.end());
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
    for (const auto& note : _note) {
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

size_t Chord::size() const
{
    return _note.size();
}

int Chord::getDurationTicks() const
{
    // Error check
    if (_note.empty()) { return 0; }

    // For each internal note, get the minimum duration ticks value
    int minValue = _note[0].getDurationTicks();
    for (const auto& note : _note) {
        if (note.getDurationTicks() < minValue) {
            minValue = note.getDurationTicks();
        }
    }

    return minValue;
}

Note& Chord::getNote(int noteIndex)
{
   return _note[noteIndex];
}

const Note& Chord::getNote(const int noteIndex) const
{
   return _note[noteIndex];
}

void Chord::print() const
{
    const int chordSize = _note.size();

    for (int i = 0; i < chordSize; i++) {
        std::cout << "note[" << i << "] = " << _note[i].getPitch() << std::endl;
    }
}

void Chord::printStack() const
{
    const int stackSize = _stack.size();

    for (int i = 0; i < stackSize; i++) {
        std::cout << "stack[" << i << "] = " << _stack[i].getPitch() << std::endl;
    }
}

void Chord::stackInThirds(const bool enharmonyNotes)
{
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
    if (_note.empty()) {
        std::cerr << "[WARN]: The chord is empty!" << std::endl;
        return;
    }

    // Remove rests
    _note.erase(
        std::remove_if(_note.begin(), _note.end(), [&](const Note& note) {
            return note.isNoteOff();
        }), _note.end());

    // ===== STEP 1: GET THE BASS NOTE ===== //

    // Copy the orginal chord to a stack vector
    _stack.clear();
    _stack = _note;

    // Sort chord notes using the MIDI note value
    std::sort(_stack.begin(), _stack.end());

    // Get the bass note of the chord
    _bassNote = _stack[0];

    // ===== STEP 3: FILTER UNIQUE PITCH CLASS NOTES ===== //
    // Remove duplacated notes (pitchClass)
    _stack.erase(std::unique(_stack.begin(), _stack.end(),
                 [](const Note& a, const Note& b) {
        return a.getPitchClass() == b.getPitchClass();
    }), _stack.end());

    // Update the 'chordSize' variable
    const int chordSize = _stack.size();
    // std::cout << "chordSize: " << chordSize << std::endl;

    // Error checking:
    if (chordSize < 3) {
        std::cout << "[WARN]: The chord MUST BE at least 3 unique pitch classes!" << std::endl;
        return;
    }

    // ===== STEP 4: COMPUTE ENHARMONIC UNIT GROUPS ===== //
    std::vector<Heap> enharmonicUnitGroups = computeEnharmonicUnitsGroups();

    // ===== STEP 5: COMPUTE ENHARMONIC HEAPS ===== //
    std::vector<Heap> enharmonicHeaps = computeEnharmonicHeaps(enharmonicUnitGroups);
    // std::cout << "enharmonicHeaps: " << enharmonicHeaps.size() << std::endl;

    // ===== STEP 6: COMPUTE THE STACK IN THIRDS TEMPLATE MATCH ===== //
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

    // ===== STEP 7: SORT HEAPS BY STACK IN THIRDS MATCHING VALUE ===== //
    std::sort(_stackedHeaps.begin(), _stackedHeaps.end(), std::greater<>());

    // std::cout << "===== PRINT ALL HEAPS =====" << std::endl;
    int idx = 0;
    for (const auto& heapData : _stackedHeaps) {
        Heap heap = std::get<0>(heapData);
        float heapCountPoints = std::get<1>(heapData);

        std::cout << "Heap[" << idx << "] ";
        for (const auto& el : heap) {
            std::cout << el.note.getPitch() << " ";
        }
        std::cout << "| matchValue: " << heapCountPoints << std::endl;

        idx++;
    }

    // // ===== STEP 3: TRANSPOSE ALL NOTES TO OCTAVE 4 REFERENCE ===== //
    // // Create a temp note to be the 'octave 4' root reference
    // Note rootOct4 = stack.at(0);
    // rootOct4.setOctave(4);

    // // Compute the distance between this new root and the current root
    // const int rootOct4Distance = rootOct4.getMIDINumber() - stack.at(0).getMIDINumber();

    // // Move the entire chord to the 'octave 4' root reference
    // transposeStackOnly(rootOct4Distance);

    // ===== STEP 4: SET INTERNAL FLAG AND COMPUTE INTERVALS ===== //
    // Set a internal control flag
    _isStackedInThirds = true;

    // computeIntervals();
}

// void Chord::stackInThirds(const bool enharmonyNotes)
// {
//     ignore(enharmonyNotes);

//     // ===== STEP 0: INPUT VALIDATION===== //
//     // Error checking:
//     if (_note.empty()) {
//         std::cerr << "[ERROR]: The chord is empty!" << std::endl;
//         return;
//     }

//     // Remove rests
//     _note.erase(
//         std::remove_if(_note.begin(), _note.end(), [&](const Note& note) {
//             return note.isNoteOn() == false;
//         }), _note.end());

//     // Copy the orginal chord to a stack vector
//     _stack.clear();
//     _stack = _note;

//     // Sort chord notes using the MIDI note value
//     std::sort(_stack.begin(), _stack.end());

//     // Get the bass note of the chord
//     _bassNote = _stack[0];

//     // Remove same PitchClass => Get a 'piano reduction' chord version
//     _stack.erase(std::unique(_stack.begin(), _stack.end(),
//                  [](const Note& a, const Note& b) {
//         return a.getPitchClass() == b.getPitchClass();
//     }), _stack.end());

//     // Update the 'chordSize' variable
//     const size_t chordSize = _stack.size();
//     std::cout << "chordSize: " << chordSize << std::endl;

//     // Error checking:
//     if (chordSize < 3) {
//         // std::cerr << "[ERROR]: The chord size MUST BE at least 3 unique pitchClasses!" << std::endl;
//         return;
//     }

//     // ===== STEP 1: FOR EACH NOTE: GENERATE A SPECIFIC TEMP/TRY CHORD STACK (DIATONIC) ===== //
//     std::vector<int> stackSum(chordSize, 0);
//     std::vector<std::map<int, Note>> possibleStack(chordSize);

//     // For each 'possible root' note
//     for (size_t r = 0; r < chordSize; r++) {
//         // Alias to the 'possible root' note
//         const auto& possibleRoot = _stack[r];
        
//         std::cout << "-----> Possible Root: " << possibleRoot.getPitch() << std::endl;

//         const std::string rootDiatonicPitchClass = possibleRoot.getDiatonicSoundingPitchClass();

//         // Error checking:
//         if (rootDiatonicPitchClass.empty()) { throw std::runtime_error("A note diatonic pitchClass is empty!"); }

//         // Select the chord stack using the possible root note
//         const char* diatonicStack = nullptr;
//         switch (hash(rootDiatonicPitchClass.c_str())) {
//             case hash("C"): diatonicStack = c_C_diatonicChordStack.data(); break;
//             case hash("D"): diatonicStack = c_D_diatonicChordStack.data(); break;
//             case hash("E"): diatonicStack = c_E_diatonicChordStack.data(); break;
//             case hash("F"): diatonicStack = c_F_diatonicChordStack.data(); break;
//             case hash("G"): diatonicStack = c_G_diatonicChordStack.data(); break;
//             case hash("A"): diatonicStack = c_A_diatonicChordStack.data(); break;
//             case hash("B"): diatonicStack = c_B_diatonicChordStack.data(); break;
//             default: throw std::runtime_error("Invalid diatonic pitchClass"); break;
//         }

//         // For each note in the chord
//         for (size_t n = 0; n < chordSize; n++) {
//             auto& note = _stack[n];
//             const std::string diatonicPitchClass = note.getDiatonicSoundingPitchClass();

//             std::cout << "n[" << n << "]: Note=" << note.getPitch() << std::endl; 

//             // Check if the 'possible root' and the current 'note' are diatonic different
//             if ((possibleRoot.getPitch() != note.getPitch()) && (rootDiatonicPitchClass == diatonicPitchClass)) {
//                 // Change the 'current note' pitch to an enharmonic equivalent
//                 std::cout << "Enharmonizando: " << note.getPitch() << std::endl;
//                 note.toEnharmonicPitch();
//             }

//             // const char toFind = *diatonicPitchClass.c_str();
//             const char toFind = *note.getDiatonicSoundingPitchClass().c_str();

//             // Compute the distance between the possible root and this note
//             const int noteDistance = std::distance(&diatonicStack[0], std::find(&diatonicStack[0], &diatonicStack[6], toFind));

//             // Store the note 'n' inside the possible stack 'r'
//             possibleStack[r].insert({noteDistance, note});

//             // Compute the note weigth. MAY BE REMOVE IN THE FUTURE!!
//             const int noteWeigth = noteDistance;
//             stackSum[r] += noteWeigth;            
//         }
//     }

//     // Select the better computed chord stack
//     const int betterStackIdx = std::min_element(stackSum.begin(), stackSum.end()) - stackSum.begin();

//     // Just an alias to the final stack
//     const auto& stack = possibleStack[betterStackIdx];

//     std::cout << "===== possibleStack =====" << betterStackIdx << std::endl;
//     for (const auto& ps : possibleStack) {
//         for (const auto& n : ps) {
//             std::cout << n.first << " | " << n.second.getPitch() << std::endl;
//         }
//         std::cout << "------" << std::endl;
//     }

//     // ===== STEP 2: STORE STACKED NOTES IN ACENDENT ORDER ===== //
//     std::vector<Note> orderedStack;
//     for (auto it = stack.begin(); it != stack.end(); it++) { orderedStack.push_back(it->second); }

//     _stack.clear();
//     _stack = orderedStack;

//     std::cout << "AFTER CLEAR" << std::endl;
//     for (const auto& n : _stack) {
//         std::cout << n.getPitch() << std::endl;
//     }

    

//     // ===== STEP 3: TRANSPOSE ALL NOTES TO OCTAVE 4 REFERENCE ===== //
//     // Create a temp note to be the 'octave 4' root reference
//     Note rootOct4 = stack.at(0);
//     rootOct4.setOctave(4);

//     // Compute the distance between this new root and the current root
//     const int rootOct4Distance = rootOct4.getMIDINumber() - stack.at(0).getMIDINumber();

//     // Move the entire chord to the 'octave 4' root reference
//     transposeStackOnly(rootOct4Distance);

//     // ===== STEP 4: SET INTERNAL FLAG AND COMPUTE INTERVALS ===== //
//     // Set a internal control flag
//     _isStackedInThirds = true;

//     std::cout << "BEFORE COMPUTE INTERVALS" << std::endl;
//     for (const auto& n : _stack) {
//         std::cout << n.getPitch() << std::endl;
//     }

//     computeIntervals();
// }

std::vector<Heap> Chord::computeEnharmonicUnitsGroups() const
{
    const int chordSize = _stack.size();
    std::vector<Heap> enharUnitGroups(chordSize);
    const int numUnitGroups = enharUnitGroups.size();

    const int numOfEnharmonics = 3;

    for (int i = 0; i < numUnitGroups; i++) {
        Heap unitGroup(numOfEnharmonics);

        Note originalNote = _stack[i]; // must be a copy
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
    const NoteData& rootNoteData = heap[0];
    const Note& rootNote = rootNoteData.note;
    const int stackIntervals[] = {1, 3, 5, 7, 2, 4, 6};
    Interval interval(rootNote, rootNote); // temp interval object
    
    // Iterate over each heap interval (rootNote as base reference)
    for (int i = 0; i < heapSize; i++) {
        interval.setNotes(rootNote, heap[i].note);
        // std::cout << "interval.getPitchStepInterval()[i] = " << interval.getPitchStepInterval() << std::endl;
        
        if (interval.getPitchStepInterval() == stackIntervals[i]) {
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
        // std::cout << "heapCountPoints[" << i << "] " << *heapCountPoints << std::endl;
        // Exit condition
        if (heapSize == (i+1)) { 
            *heapCountPoints /= maxHeapPointsValue;
            break;
        }
    }

    return heapData;  
}

void Chord::computeIntervals()
{
    const size_t chordSize = _stack.size();

    // ===== STEP 1: GET RIGHT POSITION STACKED INTERVALS ====== //
    _midiInterval.clear();
    _midiInterval.resize(chordSize - 1, 0);

    // Get the root note
    const Note& rootNote = _stack[0];

    // Get the number of _stack intervals
    const size_t intervalSize = chordSize - 1;

    // Compute interval for each _stack note
    for (size_t i = 0; i < intervalSize; i++) {

        Note& previusNote = (i == 0) ? _stack[0] : _stack[i];
        Note& nextNote = _stack[i+1];

        const size_t currentOct = previusNote.getOctave();

        // To get only ascendent intervals, the 'nextNote' MUST BE ALWAYS higher then the 'previusNote'
        // So we check this condition and change the 'nextNote' octave if is necessary
        if ((nextNote.getMIDINumber() % 12) > (previusNote.getMIDINumber() % 12)) {
            nextNote.setOctave(currentOct);
        } else {
            nextNote.setOctave(currentOct + 1);
        }

        // Compute the absolute semitones interval between the root and the 'nextNote'
        _midiInterval[i] = nextNote.getMIDINumber() - rootNote.getMIDINumber();
    }

    // ===== STEP 2: GET CHORD INTERVALS ====== //

    // Verify the existence of each _stack interval
    // Third's
    _haveMinorThird = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_THIRD) != _midiInterval.end();
    _haveMajorThird = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_THIRD) != _midiInterval.end();

    // Fifth's
    _haveDiminishedFifth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::DIMINISHED_FIFTH) != _midiInterval.end();
    _havePerfectFifth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::PERFECT_FIFTH) != _midiInterval.end();
    _haveAugmentedFifth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_SIXTH) != _midiInterval.end();

    // Seventh's
    _haveDiminishedSeventh = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_SIXTH) != _midiInterval.end();
    _haveMinorSeventh = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_SEVENTH) != _midiInterval.end();
    _haveMajorSeventh = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_SEVENTH) != _midiInterval.end();

    // Ninth's
    _haveMinorNinth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_NINTH) != _midiInterval.end();
    _haveMajorNinth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_NINTH) != _midiInterval.end();

    // Eleventh's
    _havePerfectEleventh = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::PERFECT_ELEVENTH) != _midiInterval.end();

    const bool haveSharpElevenFlatTwelve = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::SHARP_ELEVEN) != _midiInterval.end();

    if (haveSharpElevenFlatTwelve) {
        const auto it = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::SHARP_ELEVEN);
        const int index = it - _midiInterval.begin();
        const Note& note = _stack[index];

        // Distinguish between 'diminished fifth' and 'sharp eleven'
        if (note.getAlterSymbol() == MUSIC_XML::ACCIDENT::SHARP) {
            _haveSharpEleventh = true;
        } else {
            _haveDiminishedFifth = true;
        }
    }

    // Thirdteenth's
    _haveMinorThirdteenth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MINOR_THIRTEENTH) != _midiInterval.end();
    _haveMajorThirdteenth = std::find(_midiInterval.begin(), _midiInterval.end(), MUSIC_XML::MIDI::INTERVAL::MAJOR_THIRTEENTH) != _midiInterval.end();
}

std::vector<int> Chord::getMIDIIntervals()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return _midiInterval;
}

std::vector<Interval> Chord::getIntervals(const bool fromRoot) const
{
    const int numIntervals = size() - 1;
    std::vector<Interval> intervals;

    // ===== GET INTERVALS FROM ROOT ===== //
    if (fromRoot) {
        for (int i = 0; i < numIntervals; i++) {
            intervals.emplace_back(_note[0], _note[i+1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals.emplace_back(_note[i], _note[i+1]);
    }

    return intervals;
}

std::vector<Interval> Chord::getStackIntervals(const bool fromRoot)
{
    if (!_isStackedInThirds) { stackInThirds(); }

    const int numIntervals = stackSize() - 1;
    std::vector<Interval> intervals;

    // ===== GET INTERVALS FROM ROOT ===== //
    if (fromRoot) {
        for (int i = 0; i < numIntervals; i++) {
            intervals.emplace_back(_stack[0], _stack[i+1]);
        }

        return intervals;
    }

    // ===== GET INTERVALS FROM EACH NOTE PAIR ===== //
    for (int i = 0; i < numIntervals; i++) {
        intervals.emplace_back(_stack[i], _stack[i+1]);
    }

    return intervals;
}

std::vector<Note> Chord::getStackedNotes() const
{
    return _stack;
}

std::string Chord::enharmonicName()
{
    // ===== STEP 1: CHECK IF THE CHORD IS TONAL OR NOT ====== //
    if (!isTonal()) {
        std::cout << "[WARNING] Unable to get a tonal name of a non-tonal chord" << std::endl;
        return {}; 
    }

    // Error checking
    if (!_haveMinorThird && !_haveMajorThird) {
        // std::cerr << "[ERROR]: Unable to classify this chord!" << std::endl;
        return {};
    }

    // ===== STEP 2: SET THE CHORD BASIC CLASSIFICATIION ====== //

    std::string basicClassification;

    // Set the note 'basicClassification' using the third, fifth and seventh intervals
    if (_haveMinorThird) {
        if (_haveDiminishedFifth) {
            if (!_haveDiminishedSeventh && !_haveMinorSeventh && !_haveMajorSeventh) {
                basicClassification = "m(b5)";
            } else if (_haveDiminishedSeventh) {
                basicClassification = "º";
            } else if (_haveMinorSeventh) {
                basicClassification = "m7(b5)";
            } else if (_haveMajorSeventh) {
                basicClassification = "m7M(b5)";
            }
        } else if (_havePerfectFifth || (!_haveDiminishedFifth && !_haveAugmentedFifth)) {
            if (!_haveDiminishedSeventh && !_haveMinorSeventh && !_haveMajorSeventh) {
                basicClassification = "m";
            } else if (_haveDiminishedSeventh) {
                basicClassification = "dim7";
            } else if (_haveMinorSeventh) {
                basicClassification = "m7";
            } else if (_haveMajorSeventh) {
                basicClassification = "m7M";
            }
        }
    } else {
        if (_haveDiminishedFifth) {
            if (!_haveDiminishedSeventh && !_haveMinorSeventh && !_haveMajorSeventh) {
                basicClassification = "(b5)";
            } else if (_haveDiminishedSeventh) {
                basicClassification = "dim7(b5)";
            } else if (_haveMinorSeventh) {
                basicClassification = "7(b5)";
            } else if (_haveMajorSeventh) {
                basicClassification = "7M(b5)";
            }
        } else if (_havePerfectFifth || (!_haveDiminishedFifth && !_haveAugmentedFifth)) {
            if (!_haveDiminishedSeventh && !_haveMinorSeventh && !_haveMajorSeventh) {
                basicClassification = "";
            } else if (_haveDiminishedSeventh) {
                basicClassification = "dim7";
            } else if (_haveMinorSeventh) {
                basicClassification = "7";
            } else if (_haveMajorSeventh) {
                basicClassification = "7M";
            }
        } else if (_haveAugmentedFifth) {
            if (!_haveDiminishedSeventh && !_haveMinorSeventh && !_haveMajorSeventh) {
                basicClassification = "aug";
            } else if (_haveDiminishedSeventh) {
                basicClassification = "aug(dim7)";
            } else if (_haveMinorSeventh) {
                basicClassification = "aug(7)";
            } else if (_haveMajorSeventh) {
                basicClassification = "aug(7M)";
            }
        }
    }

    // ===== STEP 3: ADD CHORD EXTENSIONS ====== //
    std::string ninth;

    // If exists, add ninth
    if (_haveMinorNinth) {
        ninth = "9b";
    } else if (_haveMajorNinth) {
        ninth = "9";
    }

    std::string eleventh;

    // If exists, add eleventh
    if (_havePerfectEleventh) {
        eleventh = "(11)";
    } else if (_haveSharpEleventh) {
        eleventh = "(#11)";
    }

    std::string thirdteenth;

    // If exists, add thirdteenth
    if (_haveMinorThirdteenth) {
        thirdteenth = "13b";
    } else if (_haveMajorThirdteenth) {
        thirdteenth = "13";
    }

    // ===== STEP 4: ADD BASS NOTE ====== //
    std::string bassNoteStr;
    if (_stack[0].getPitchClass() != _bassNote.getPitchClass() && !_bassNote.getPitchClass().empty()) {
        bassNoteStr.append("/" + _bassNote.getPitchClass());
    }

    // ===== STEP 5: VERIFY THE ROOT ENHARMONIC PITCH ====== //
    // Check if there is a third between the root and the chord third
    const Interval interval(_stack[0], _stack[1]);
    const int diatonicIntervalRootThird = interval.getDiatonicInterval(true);

    std::string enharmonicRootPitchClass = _stack[0].getPitchClass();
    if (diatonicIntervalRootThird != 3) {
        const std::string rootAlterSymbol = _stack[0].getAlterSymbol();

        const int rootMidiNumber = Helper::pitch2midiNote(_stack[0].getPitch());

        if (rootAlterSymbol == MUSIC_XML::ACCIDENT::SHARP) {
            enharmonicRootPitchClass = Helper::midiNote2pitch(rootMidiNumber, MUSIC_XML::ACCIDENT::FLAT);
        } else if (rootAlterSymbol == MUSIC_XML::ACCIDENT::FLAT) {
            enharmonicRootPitchClass = Helper::midiNote2pitch(rootMidiNumber, MUSIC_XML::ACCIDENT::SHARP);
        }
    }

    // ===== STEP 5: SET CHORD NAME ====== //
    // Concatenate all _stack extensions
    const std::string chordName = enharmonicRootPitchClass + basicClassification + ninth + eleventh + thirdteenth + bassNoteStr;

    return chordName;
}

size_t Chord::stackSize() const
{
    return _stack.size();
}

const Note& Chord::getRoot()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return _stack.at(0);
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

    return (_haveMajorThird &&  !_haveAugmentedFifth) ? true : false;
}

bool Chord::isMinorChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (_haveMinorThird && !_haveDiminishedFifth) ? true : false;
}

bool Chord::isAugmentedChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (_haveMajorThird && _haveAugmentedFifth) ? true : false;
}

bool Chord::isDiminishedChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (_haveMinorThird && _haveDiminishedFifth && _haveMinorSeventh) ? true : false;
}

bool Chord::isDominantSeventhChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (_haveMajorThird && !_haveDiminishedFifth && !_haveAugmentedFifth && _haveMinorSeventh) ? true : false;
}

bool Chord::isHalfDiminishedChord()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return (_haveMinorThird && _haveDiminishedFifth ) ? true : false;
}

bool Chord::isTonal(std::function<bool(const Chord& chord)> model)
{
    if (!_isStackedInThirds) { stackInThirds(); }

    if (model != nullptr) { return model(*this); }

    for (size_t i = 0; i < stackSize() - 1; i++) {
        const Interval interval(_stack[i], _stack[i+1]);

        if (!interval.isTonal()) { return false; }
    }

    return true;
}

bool Chord::haveMinorThird()
{
    return _haveMinorThird;
}

bool Chord::haveMajorThird()
{
    return _haveMajorThird;
}

bool Chord::haveDiminishedFifth()
{
    return _haveDiminishedFifth;
}

bool Chord::havePerfectFifth()
{
    return _havePerfectFifth;
}

bool Chord::haveAugmentedFifth()
{
    return _haveAugmentedFifth;
}

bool Chord::haveDiminishedSeventh()
{
    return _haveDiminishedSeventh;
}

bool Chord::haveMinorSeventh()
{
    return _haveMinorSeventh;
}

bool Chord::haveMajorSeventh()
{
    return _haveMajorSeventh;
}

bool Chord::haveMinorNinth()
{
    return _haveMinorNinth;
}

bool Chord::haveMajorNinth()
{
    return _haveMajorNinth;
}

bool Chord::havePerfectEleventh()
{
    return _havePerfectEleventh;
}

bool Chord::haveSharpEleventh()
{
    return _haveSharpEleventh;
}

bool Chord::haveMinorThirdteenth()
{
    return _haveMinorThirdteenth;
}

bool Chord::haveMajorThirdteenth()
{
    return _haveMajorThirdteenth;
}

const Note& Chord::getBassNote()
{
    if (!_isStackedInThirds) { stackInThirds(); }

    return _bassNote;
}

const std::vector<Note>& Chord::getNotes() const
{
    return _note;
}

Chord Chord::getStackedChord(const bool enharmonyNotes)
{
    if (!_isStackedInThirds) { stackInThirds(enharmonyNotes); }
    
    const HeapData& bestHeapData = _stackedHeaps[0];
    const Heap& bestHeap = std::get<0>(bestHeapData);

    Chord stackChord;
    for (const auto& noteData : bestHeap) {
        stackChord.addNote(noteData.note);
    }

    return stackChord;
}

void Chord::sortNotes()
{
    std::sort(_note.begin(), _note.end());
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
