#include "measure.h"

Measure::Measure(const int numStaves, const int divisionsPerQuarterNote) :
    _number(0),
    _timeSignatureUpper(4),
    _timeSignatureLower(4),
    _fifthCicle(0),
    _keySignature("C"),
    _metronomeFigure("quarter"),
    _metronomeValue(0),
    _isMajorKeyMode(true),
    _isKeySignatureChanged(false),
    _isTimeSignatureChanged(false),
    _isClefChanged(false),
    _isMetronomeChanged(false),
    _isDivisionsPerQuarterNoteChanged(false),
    _numStaves(numStaves),
    _divisionsPerQuarterNote(divisionsPerQuarterNote)
{
    _note.resize(numStaves); // Create Staves
    _clef.resize(numStaves);

    _barlineLeft.setLocation("left");
    _barlineRight.setLocation("right");
}

Measure::~Measure()
{

}

void Measure::info() const
{
    std::cout << "Number: " << _number << std::endl;
    std::cout << "Time Signature: " << _timeSignatureUpper << "/" << _timeSignatureLower << std::endl;
    std::cout << "Fifth Cicle: " << _fifthCicle << std::endl;
    std::cout << "Metronome Mark: " << _metronomeFigure << " - " << _metronomeValue << std::endl;
}

void Measure::setNumber(const int MeasureNumber)
{
    _number = MeasureNumber;
}

void Measure::setKeySignature(const int fifthCicle, const bool isMajorMode)
{
    const std::map<int, std::string> majorKey {
      std::make_pair(0, ""),
      std::make_pair(1, "G"),
      std::make_pair(2, "D"),
      std::make_pair(3, "A"),
      std::make_pair(4, "E"),
      std::make_pair(5, "B"),
      std::make_pair(6, "F#"),
      std::make_pair(7, "C#"),
      std::make_pair(8, "G#"),
      std::make_pair(9, "D#"),
      std::make_pair(10, "A#"),
      std::make_pair(11, "E#"),
      std::make_pair(12, "B#"),
      std::make_pair(-1, "F"),
      std::make_pair(-2, "Bb"),
      std::make_pair(-3, "Eb"),
      std::make_pair(-4, "Ab"),
      std::make_pair(-5, "Db"),
      std::make_pair(-6, "Gb")
    };

    const std::map<int, std::string> minorKey {
      std::make_pair(0, "Am"),
      std::make_pair(1, "Em"),
      std::make_pair(2, "Bm"),
      std::make_pair(3, "F#m"),
      std::make_pair(4, "C#m"),
      std::make_pair(5, "G#m"),
      std::make_pair(6, "D#m"),
      std::make_pair(7, "A#m"),
      std::make_pair(8, "E#m"),
      std::make_pair(9, "B#m"),
      std::make_pair(10, "Fxm"),
      std::make_pair(11, "Cxm"),
      std::make_pair(12, "Gxm"),
      std::make_pair(-1, "Dm"),
      std::make_pair(-2, "Gm"),
      std::make_pair(-3, "Cm"),
      std::make_pair(-4, "Fm"),
      std::make_pair(-5, "Bbm"),
      std::make_pair(-6, "Ebm")
    };

    _keySignature = (isMajorMode) ? majorKey.at(fifthCicle) : minorKey.at(fifthCicle);
    _fifthCicle = fifthCicle;
    _isMajorKeyMode = isMajorMode;

    _isKeySignatureChanged = true;
}

void Measure::setTimeSignature(const int timeSignatureUpper, const int timeSignatureLower)
{
    _isTimeSignatureChanged = true;
    _timeSignatureUpper = timeSignatureUpper;
    _timeSignatureLower = timeSignatureLower;
}

void Measure::setMetronome(const int bpm, const std::string& rhythmFigure)
{
    _metronomeValue = bpm;
    _metronomeFigure = rhythmFigure;
    setIsMetronomeChanged(true);
}

void Measure::setKeyMode(const bool isMajorKeyMode)
{
    _isMajorKeyMode = isMajorKeyMode;
}

void Measure::setIsKeySignatureChanged(bool isKeySignatureChanged)
{
    _isKeySignatureChanged = isKeySignatureChanged;
}

void Measure::setIsTimeSignatureChanged(bool isTimeSignatureChanged)
{
    _isTimeSignatureChanged = isTimeSignatureChanged;
}

void Measure::setIsClefChanged(bool isClefChanged)
{
    _isClefChanged = isClefChanged;
}

void Measure::setIsMetronomeChanged(bool isMetronomeMarkChanged)
{
    _isMetronomeChanged = isMetronomeMarkChanged;
}

void Measure::setIsDivisionsPerQuarterNoteChanged(bool isDivisionsPerQuarterNoteChanged)
{
    _isDivisionsPerQuarterNoteChanged = isDivisionsPerQuarterNoteChanged;
}

void Measure::setNumStaves(const int numStaves)
{
    _numStaves = numStaves;

    _note.resize(numStaves);
    _clef.resize(numStaves);
}

int Measure::getNumStaves() const
{
    return _numStaves;
}

void Measure::clear()
{
    _note.clear();
    _metronomeValue = 0;
    _metronomeFigure = {};
}

bool Measure::empty() const
{
    bool isEmpty = false;
    for (auto& stave : _note) {
        isEmpty |= stave.empty();
    }

    return isEmpty;
}

void Measure::addNote(const Note& note, const int staveId, int position)
{
    auto& stave = _note[staveId];

    if (position < 0) {
        position = static_cast<int>(stave.size()); // append to the end of the vector
    }

    stave.insert(stave.begin() + position, note);
}

void Measure::addNote(const std::vector<Note>& noteVec, const int staveId, int position)
{
    for (auto& note : noteVec) {
        addNote(note, staveId, position);
    }
}

void Measure::addNote(const std::string& pitch, const int staveId, int position)
{
    Note note(pitch);
    addNote(note, staveId, position);
}

void Measure::addNote(const std::vector<std::string>& pitchClassVec, const int staveId, int position)
{
    for (auto& pitch : pitchClassVec) {
        addNote(pitch, staveId, position);
    }
}

void Measure::removeNote(const int noteId, const int staveId)
{
    try {
        auto& stave = _note[staveId];
        stave.erase(stave.begin(), stave.begin() + noteId);
    } catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << '\n';
    }
}

bool Measure::timeSignatureChanged() const
{
    return _isTimeSignatureChanged;
}

bool Measure::keySignatureChanged() const
{
    return _isKeySignatureChanged;
}

int Measure::getNumber() const
{
    return _number;
}

const Clef& Measure::getClef(const int clefId) const
{
    return _clef[clefId];
}

Clef& Measure::getClef(const int clefId)
{
    return _clef[clefId];
}

const Barline& Measure::getBarlineLeft() const
{
    return _barlineLeft;
}

Barline& Measure::getBarlineLeft()
{
    return _barlineLeft;
}

const Barline& Measure::getBarlineRight() const
{
    return _barlineRight;
}

Barline& Measure::getBarlineRight()
{
    return _barlineRight;
}

void Measure::setRepeatStart()
{
    _barlineLeft.setRepeatStart();
}

void Measure::setRepeatEnd()
{
    _barlineRight.setRepeatEnd();
}

void Measure::removeRepeatStart()
{
    _barlineLeft.clean();
}

void Measure::removeRepeatEnd()
{
    _barlineRight.clean();
}

bool Measure::clefChanged() const
{
    return _isClefChanged;
}

bool Measure::metronomeChanged() const
{
    return _isMetronomeChanged;
}

bool Measure::divisionsPerQuarterNoteChanged() const
{
    return _isDivisionsPerQuarterNoteChanged;
}

bool Measure::isMajorKeyMode() const
{
    return _isMajorKeyMode;
}

const Note& Measure::getNote(const int noteId, const int staveId) const
{
    const auto& stave = _note[staveId];

    if (noteId > (static_cast<int>(stave.size() - 1))) { throw std::out_of_range("Out of Range error"); }

    return stave[noteId];
}

Note& Measure::getNote(const int noteId, const int staveId)
{
    auto& stave = _note[staveId];

    if (noteId > (static_cast<int>(stave.size() - 1))) { throw std::out_of_range("Out of Range error"); }

    return stave[noteId];
}

const Note& Measure::getNoteOn(const int noteOnId, const int staveId) const
{
    auto& stave = _note[staveId];

    const int numNotes = getNumNotes(staveId);

    if (noteOnId < 0 || noteOnId > (numNotes - 1)) { throw std::out_of_range("Out of Range error"); }

    int noteCount = 0;
    for (int n = 0; n < numNotes; n++) {
        const Note& currentNote = stave[n];

        if (!currentNote.isNoteOn()) { continue; }

        if (noteOnId != noteCount) {
            noteCount++;
            continue;
        }
        
        return currentNote;
    }

    return stave[0]; // This return statement is only to remove the warning "returning reference to temporary"
}

Note& Measure::getNoteOn(const int noteOnId, const int staveId)
{
    auto& stave = _note[staveId];

    const int numNotes = getNumNotes(staveId);

    if (noteOnId < 0 || noteOnId > (numNotes - 1)) { throw std::out_of_range("Out of Range error"); }

    int noteCount = 0;
    Note* currentNote = nullptr;
    for (int n = 0; n < numNotes; n++) {
        currentNote = &stave[n];

        if (!currentNote->isNoteOn()) { continue; } 
        
        if (noteOnId != noteCount) {
            noteCount++;
            continue;
        }

        break;
    }

    return *currentNote;
}

const Note& Measure::getNoteOff(const int noteOffId, const int staveId) const
{
    auto& stave = _note[staveId];

    const int numNotes = getNumNotes(staveId);

    if (noteOffId < 0 || noteOffId > (numNotes - 1)) { throw std::out_of_range("Out of Range error"); }

    int noteCount = 0;
    for (int n = 0; n < numNotes; n++) {
        const Note& currentNote = stave[n];

        if (!currentNote.isNoteOff()) { continue; }

        if (noteOffId != noteCount) {
            noteCount++;
            continue;
        }
        
        return currentNote;
    }

    return stave[0]; // This return statement is only to remove the warning "returning reference to temporary"
}

Note& Measure::getNoteOff(const int noteOffId, const int staveId)
{
    auto& stave = _note[staveId];

    const int numNotes = getNumNotes(staveId);

    if (noteOffId < 0 || noteOffId > (numNotes - 1)) { throw std::out_of_range("Out of Range error"); }

    int noteCount = 0;
    Note* currentNote = nullptr;
    for (int n = 0; n < numNotes; n++) {
        currentNote = &stave[n];

        if (!currentNote->isNoteOff()) { continue; } 
        
        if (noteOffId != noteCount) {
            noteCount++;
            continue;
        }

        break;
    }

    return *currentNote;
}

int Measure::getNumNotesOn() const
{
    int numNotes = 0;

    for (const auto& stave : _note) {
        for (const auto& note : stave) {
            if (note.isNoteOn()) {
                numNotes++;
            }
        }
    }

    return numNotes;
}

int Measure::getNumNotesOn(const int staveId) const
{
    const int staveSize = static_cast<int>(_note.size());

    if (staveId > staveSize - 1) { throw std::out_of_range("Out of Range error"); }

    const auto& stave = _note[staveId];

    int numNotes = 0;
    for (const auto& note : stave) {
        if (note.isNoteOn()) {
            numNotes++;
        }
    }

    return numNotes;
}

int Measure::getNumNotesOff() const
{
    int numRests = 0;

    for (const auto& stave : _note) {
        for (const auto& note : stave) {
            if (note.isNoteOff()) {
                numRests++;
            }
        }
    }

    return numRests;
}

int Measure::getNumNotesOff(const int staveId) const
{
    const int staveSize = static_cast<int>(_note.size());

    if (staveId > staveSize - 1) { throw std::out_of_range("Out of Range error"); }

    const auto& stave = _note[staveId];

    int numRests = 0;
    for (const auto& note : stave) {
        if (note.isNoteOff()) {
            numRests++;
        }
    }

    return numRests;
}

int Measure::getNumNotes() const
{
    int numNotes = 0;

    for (const auto& stave : _note) {
        numNotes += static_cast<int>(stave.size());
    }

    return numNotes;
}

int Measure::getNumNotes(const int staveId) const
{
    const int staveSize = static_cast<int>(_note.size());

    if (staveId > staveSize - 1) { throw std::out_of_range("Out of Range error"); }

    const auto& stave = _note[staveId];

    int numNotes = static_cast<int>(stave.size());

    return numNotes;
}

int Measure::getFifthCicle() const
{
    return _fifthCicle;
}

const std::string Measure::getKeySignature() const
{
    return _keySignature;
}

std::pair<int,int> Measure::getTimeSignature() const
{
    return std::make_pair(_timeSignatureUpper, _timeSignatureLower);
}

std::pair<std::string, int> Measure::getMetronome() const
{
    return {_metronomeFigure, _metronomeValue};
}

const std::string Measure::toXML(const int instrumentId, const int identSize) const
{
    std::string xml;

    int backup = 0;

    const auto& firstStave = _note[0];

    for (const auto& note : firstStave) {
        if (!note.inChord() && note.getVoice() == 1) {
            backup += note.getDurationTicks();
        }
    }

    if (!_barlineLeft.getBarStyle().empty()) {
        xml.append(_barlineLeft.toXML(identSize));
    }

    if (empty()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<note>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<rest measure=\"yes\" />\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<duration>" + std::to_string(256 * 4) + "</duration>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<voice>1</voice>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<staff>1</staff>\n");
        xml.append(Helper::generateIdentation(4, identSize) + "</note>\n");

        xml.append(Helper::generateIdentation(4, identSize) + "<backup>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<duration>" + std::to_string(256 * 4) + "</duration>\n");
        xml.append(Helper::generateIdentation(4, identSize) + "</backup>\n");

        if (!_barlineRight.getBarStyle().empty()) {
            xml.append(_barlineRight.toXML(identSize));
        }

        return xml;
    }

    int sumDurationVoice1 = 0;
    bool haveAnyNoteOn = false;

    std::vector<std::vector<int>> voiceDuration(5);
    for (const auto& stave : _note) {
        for (const auto& note : stave) {
            voiceDuration[note.getVoice()].push_back(note.getDurationTicks());
            haveAnyNoteOn |= note.isNoteOn();
        }
    }

    sumDurationVoice1 = std::accumulate(voiceDuration[1].begin(), voiceDuration[1].end(), 0);

    for (int s = 0; s < _numStaves; s++) {
        const int numNotes = static_cast<int>(_note[s].size());
        for (int n = 0; n < numNotes; n++) {

                if (s == 0 && haveAnyNoteOn && _note[s][n].getVoice() != _note[s][n-1].getVoice() ) {
                    xml.append(Helper::generateIdentation(3, identSize) + "<backup>\n");
                    xml.append(Helper::generateIdentation(4, identSize) + "<duration>" + std::to_string(sumDurationVoice1) + "</duration>\n");
                    xml.append(Helper::generateIdentation(3, identSize) + "</backup>\n");
                }

            xml.append(_note[s][n].toXML(instrumentId, identSize));
        }
    }

    if (!_barlineRight.getBarStyle().empty()) {
        xml.append(_barlineRight.toXML(identSize));
    }

    return xml;
}

const std::string Measure::toJSON() const
{
    return std::string();
}

int Measure::getEmptyDurationTicks() const
{
    int sumDuration = 0;

    for (const auto& stave : _note) {
        for (const auto& note : stave) {
            if (note.inChord()) { continue; }
            sumDuration += note.getDurationTicks();
        }
    }

    const int divisionsPerUnit = [&]() {
        switch (_timeSignatureLower) {
            case 2: return 512; break;
            case 4: return 256; break;
            case 8: return 128; break;
            case 16: return 64; break;
            case 32: return 32; break;
        default:
            std::cerr << "[ERROR]: Invalid timeSignatureLower!" << std::endl;
            return 256;
            break;
        }
    }();

    const int maxDurationTicks = _timeSignatureUpper * divisionsPerUnit;

    const int diff  = maxDurationTicks - sumDuration;

    return diff;
}

void Measure::setDivisionsPerQuarterNote(const int divisionsPerQuarterNote)
{
    _divisionsPerQuarterNote = divisionsPerQuarterNote;
}

int Measure::getDivisionsPerQuarterNote() const
{
    return _divisionsPerQuarterNote;
}

