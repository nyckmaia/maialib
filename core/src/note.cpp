#include "note.h"
#include "helper.h"

Note::Note() : 
    Note("A4")
{
    
}

Note::Note(const std::string& pitch, const Duration duration, bool isNoteOn, bool inChord, int transposeDiatonic, int transposeChromatic, const int divisionsPerQuarterNote) :
    _writtenPitchClass(MUSIC_XML::PITCH::REST),
    _writtenOctave(0),
    _soundingPitchClass(MUSIC_XML::PITCH::REST),
    _soundingOctave(0),
    _isNoteOn(false),
    _inChord(false),
    _midiNumber(MUSIC_XML::MIDI::NUMBER::MIDI_REST),
    _transposeDiatonic(0),
    _transposeChromatic(0),
    _voice(1),
    _staff(0),
    _isGraceNote(false),
    _isTuplet(false),
    _isPitched(true),
    _unpitchedIndex(0)
{
    // Rest case: This is necessary to prevent: empty pitchClass + alterSymbol
    if (pitch.empty() || (pitch.find(MUSIC_XML::PITCH::REST) != std::string::npos)
        || isNoteOn == false) {
            setDuration(duration, divisionsPerQuarterNote);
            return; 
    }

    const size_t pitchSize = pitch.size();

    // Error checking:
    if(pitchSize > 4) {
        const std::string msg = "The pitch '" + pitch + "' have a invalid length: " + std::to_string(pitchSize);
        throw std::runtime_error(msg); 
    }

    const std::string diatonicPitch = pitch.substr(0, 1);
    bool foundPitch = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonicPitch) != c_C_diatonicScale.end();

    // Error checking:
    if(!foundPitch && !pitch.empty()) {
        const std::string msg = "Unknown diatonc pitch: " + diatonicPitch;
        throw std::runtime_error(msg);  
    }

    // Verify if the input data is a full pitch or just a pitchClass. Ex.: "A4" or "A"
    const bool isFullPitch = (isdigit(pitch.back())) ? true : false;

    const std::string pitchClass = (isFullPitch) ? pitch.substr(0, pitchSize - 1) : pitch;
    const int octave = (isFullPitch) ? (int)pitch.back() - 48 : 4;

    const size_t pitchClassSize = pitchClass.size();

    // Store the alter symbol: # / b / x / bb
    if (pitchClassSize > 1) {
        _alterSymbol = pitchClass.substr(1, pitchClassSize);
    }

    bool foundAlterSymbol = std::find(c_alterSymbol.begin(), c_alterSymbol.end(), _alterSymbol) != c_alterSymbol.end();
    if(!foundAlterSymbol && !_alterSymbol.empty()) { 
        const std::string msg = "Unknown alter symbol: " + _alterSymbol;
        throw std::runtime_error(msg);  
    }
    _writtenPitchClass = pitchClass;
    _writtenOctave = octave;
    _midiNumber = Helper::pitch2midiNote(pitchClass + std::to_string(octave));
    _inChord = inChord;
    _soundingPitchClass = pitchClass;
    _soundingOctave = octave;
    _transposeDiatonic = transposeDiatonic;
    _transposeChromatic = transposeChromatic;
    _isNoteOn = true;
    _duration.divisionsPerQuarterNote = divisionsPerQuarterNote;
    setDuration(duration);

    // Update the sounding Pitch/PitchClass and MIDI number
    setTransposingInterval(transposeDiatonic, transposeChromatic);
}

Note::Note(const int midiNumber, const std::string& accType, const Duration duration, bool isNoteOn, bool inChord, const int transposeDiatonic, const int transposeChromatic, const int divisionsPerQuarterNote) :
    Note(Helper::midiNote2pitch(midiNumber, accType), duration, isNoteOn, inChord, transposeDiatonic, transposeChromatic, divisionsPerQuarterNote)
{
   
}

Note::~Note()
{

}

void Note::info() const
{
    std::cout << "Is note on: " << std::boolalpha << _isNoteOn << std::endl;
    std::cout << "Pitch: " << getPitch() << std::endl;
    std::cout << "Note Type: " << _duration.noteType << std::endl;
    std::cout << "Quarter Duration: " << getQuarterDuration() << std::endl;
    std::cout << "Voice: " << _voice << std::endl;
    std::cout << "Staff: " << _staff << std::endl;
    std::cout << "MIDI Number: " << getMIDINumber() << std::endl;
    std::cout << "Stem: " << _stem << std::endl;
    std::cout << "Beams: " << _beam.size() << std::endl;
    std::cout << "Is Tuplet: " << std::boolalpha << _isTuplet << std::endl;
    std::cout << "Is Grace Note: " << std::boolalpha << _isGraceNote << std::endl;
    std::cout << "In Chord: " << std::boolalpha << _inChord << std::endl;
    std::cout << "Transpose Diatonic: " << _transposeDiatonic << std::endl;
    std::cout << "Transpose Chromatic: " << _transposeChromatic << std::endl;
}

void Note::setUnpitchedIndex(const int unpitchedIndex)
{
    _unpitchedIndex = unpitchedIndex;
}

int Note::getUnpitchedIndex() const
{
    return _unpitchedIndex;
}

void Note::setPitchClass(const std::string& pitchClass)
{
    _writtenPitchClass = pitchClass;
    _soundingPitchClass = _writtenPitchClass;

    // Store the alter symbol: # / b
    if (pitchClass.size() > 1) {
        _alterSymbol = pitchClass.substr(1, pitchClass.size());
    }

    // Update sounding Pitch Class
    setTransposingInterval(_transposeDiatonic, _transposeChromatic);
}

void Note::setIsPitched(const bool isPitched)
{
    _isPitched = isPitched;
}

bool Note::isPitched() const
{
    return _isPitched;
}

std::string Note::getPitchClass() const
{
    return _writtenPitchClass;
}

void Note::setOctave(const int octave)
{
    _writtenOctave = octave;

    const int writtenMIDINumber = Helper::pitch2midiNote(getWrittenPitch());
    _midiNumber = writtenMIDINumber + _transposeChromatic;

    const std::string soundingPitch = Helper::midiNote2pitch(_midiNumber);

    std::string pitchAcc, step, alterSymbol;
    float alterValue = 0.0f;
    int oct = 0;

    Helper::splitPitch(soundingPitch, pitchAcc, step, oct, alterValue, alterSymbol);

    _soundingOctave = oct;
}

int Note::getOctave() const
{
    return _soundingOctave;
}

int Note::getTransposeDiatonic() const
{
    return _transposeDiatonic;
}

int Note::getTransposeChromatic() const
{
    return _transposeChromatic;
}

void Note::setDuration(const Duration duration, const int divisionsPerQuarterNote)
{
    _duration.duration = duration;
    _duration.divisionsPerQuarterNote = divisionsPerQuarterNote;
    _duration.noteType = Helper::duration2noteType(duration);
    _duration.ticks = Helper::noteType2ticks(_duration.noteType, _duration.divisionsPerQuarterNote);
    const auto typeDotsPair = Helper::ticks2noteType(_duration.ticks, _duration.divisionsPerQuarterNote);
    _duration.numDots = typeDotsPair.second;
}

void Note::setDurationTicks(int durationTicks)
{
    _duration.ticks = durationTicks;
    const auto typeDotsPair = Helper::ticks2noteType(_duration.ticks, _duration.divisionsPerQuarterNote);
    _duration.noteType = typeDotsPair.first;
    _duration.numDots = typeDotsPair.second;
    _duration.duration = Helper::noteType2duration(_duration.noteType);
}

void Note::setIsGraceNote(const bool isGraceNote)
{
    _isGraceNote = isGraceNote;
}

void Note::removeDots()
{
    _duration.numDots = 0;
    _duration.noteType = _duration.noteType.substr(0, _duration.noteType.find('-'));
    _duration.ticks = Helper::noteType2ticks(_duration.noteType, _duration.divisionsPerQuarterNote);
    _duration.duration = Helper::noteType2duration(_duration.noteType);
}

void Note::setSingleDot()
{
    _duration.numDots = 1;
    _duration.noteType = _duration.noteType.substr(0, _duration.noteType.find('-'));
    _duration.noteType.append("-dot");
    _duration.ticks = Helper::noteType2ticks(_duration.noteType, _duration.divisionsPerQuarterNote);
    _duration.duration = Helper::noteType2duration(_duration.noteType);
}

void Note::setDoubleDot()
{
    _duration.numDots = 2;
    _duration.noteType = _duration.noteType.substr(0, _duration.noteType.find('-'));
    _duration.noteType.append("-dot-dot");
    _duration.ticks = Helper::noteType2ticks(_duration.noteType, _duration.divisionsPerQuarterNote);
    _duration.duration = Helper::noteType2duration(_duration.noteType);
}

std::string Note::getType() const
{
    return getLongType();
}

std::string Note::getLongType() const
{
    return _duration.noteType;
}

std::string Note::getShortType() const
{
    return _duration.noteType.substr(0, _duration.noteType.find('-'));
}

int Note::getDurationTicks() const
{
    return _duration.ticks;
}

int Note::getNumDots() const
{
    return _duration.numDots;
}

bool Note::isDotted() const
{
    return (_duration.numDots <= 0) ? false : true;
}

bool Note::isDoubleDotted() const
{
    return (_duration.numDots == 2) ? true : false;
}

int Note::getDivisionsPerQuarterNote() const
{
    return _duration.divisionsPerQuarterNote;
}

float Note::getDuration() const
{
    return getQuarterDuration();
}

float Note::getQuarterDuration() const
{
    return static_cast<float>(_duration.ticks) / static_cast<float>(_duration.divisionsPerQuarterNote);
}

bool Note::isGraceNote() const
{
    return _isGraceNote;
}

void Note::setIsNoteOn(bool isNoteOn)
{
    _isNoteOn = isNoteOn;
}

bool Note::isNoteOn() const
{
    return _isNoteOn;
}

bool Note::isNoteOff() const
{
    return !_isNoteOn;
}

std::string Note::getAlterSymbol() const
{
    return _alterSymbol;
}

void Note::setIsInChord(bool inChord)
{
    _inChord = inChord;
}

bool Note::isTransposed() const
{
    return (_transposeDiatonic == 0 && _transposeChromatic == 0) ? false : true;
}

void Note::transpose(const int semitones, const std::string& accType)
{
    const std::string newPitch = Helper::transposePitch(getPitch(), semitones, accType);

    setPitch(newPitch);
}


void Note::setPitch(const std::string& pitch){
    // Rest case: This is necessary to prevent: empty pitchClass + alterSymbol
    if (pitch.empty() || (pitch.find(MUSIC_XML::PITCH::REST) != std::string::npos)) {
        _writtenPitchClass = MUSIC_XML::PITCH::REST;
        _writtenOctave = 0;
        _soundingPitchClass = MUSIC_XML::PITCH::REST;
        _soundingOctave = 0;
        _isNoteOn = false;
        _inChord = false;
        _midiNumber = MUSIC_XML::MIDI::NUMBER::MIDI_REST;
        _transposeDiatonic = 0;
        _transposeChromatic = 0;
        _isGraceNote = false;
        return;
    }

    const size_t pitchSize = pitch.size();

    // Error checking:
    if(pitchSize > 4) {
        const std::string msg = "Invalid pitchClass length: " + std::to_string(pitchSize);
        throw std::runtime_error(msg);
    }

    const std::string diatonicPitch = pitch.substr(0, 1);
    bool foundPitch = std::find(c_C_diatonicScale.begin(), c_C_diatonicScale.end(), diatonicPitch) != c_C_diatonicScale.end();

    // Error checking:
    if(!foundPitch && !pitch.empty()) {
        const std::string msg = "Unknown diatonc pitch: " + diatonicPitch;
        throw std::runtime_error(msg);
    }

    // Verify if the input data is a full pitch or just a pitchClass. Ex.: "A4" or "A"
    const bool isFullPitch = (isdigit(pitch.back())) ? true : false;

    const std::string pitchClass = (isFullPitch) ? pitch.substr(0, pitchSize - 1) : pitch;
    const int octave = (isFullPitch) ? (int)pitch.back() - 48 : 4;

    const size_t pitchClassSize = pitchClass.size();

    // Store the alter symbol: # / b / x / bb
    if (pitchClassSize > 1) {
        _alterSymbol = pitchClass.substr(1, pitchClassSize);
    }

    bool foundAlterSymbol = std::find(c_alterSymbol.begin(), c_alterSymbol.end(), _alterSymbol) != c_alterSymbol.end();
    if(!foundAlterSymbol && !_alterSymbol.empty()) {
        const std::string msg = "Unknown alter symbol: " + _alterSymbol;
        throw std::runtime_error(msg);
    }
    _writtenPitchClass = pitchClass;
    _writtenOctave = octave;
    _midiNumber = Helper::pitch2midiNote(pitchClass + std::to_string(octave));
//    _inChord = inChord;
    _soundingPitchClass = pitchClass;
    _soundingOctave = octave;
//    _transposeDiatonic = transposeDiatonic;
//    _transposeChromatic = transposeChromatic;
    _isNoteOn = true;

    // Update the sounding Pitch/PitchClass and MIDI number
    setTransposingInterval(_transposeDiatonic, _transposeChromatic);
}

void Note::setTransposingInterval(const int diatonicInterval, const int chromaticInterval)
{
    // Error checking for rest
    if (!isNoteOn()) { return; }

    // Set internal Note members
    _transposeDiatonic = diatonicInterval;
    _transposeChromatic = chromaticInterval;

    // Update internal MIDI number member
    _midiNumber += _transposeChromatic;

    // ===== TRANSPOSE PITCH (STRING) ===== //

    // Check if this is a transposing instrument
    if (!isTransposed()) { return; }

    // Create musical scales
    const std::array<std::string, 12> sharpScale = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const std::array<std::string, 12> flatScale = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    const std::array<std::string, 12> doubleSharpScale = {"C#", "Cx", "D#", "Dx", "E#", "F#", "Fx", "G#", "Gx", "A#", "Ax", "B#"};
    const std::array<std::string, 12> doubleFlatScale = {"Cb", "Dbb", "Db", "Ebb", "Eb", "Fb", "Gbb", "Gb", "Abb", "Ab", "Bbb", "Bb"};

    // Try to find the written PitchClass in both scales
    const auto sharpNote = find(sharpScale.begin(), sharpScale.end(), _writtenPitchClass);
    const auto flatNote = find(flatScale.begin(), flatScale.end(), _writtenPitchClass);
    const auto doubleShapNote = find(doubleSharpScale.begin(), doubleSharpScale.end(), _writtenPitchClass);
    const auto doubleFlatNote = find(doubleFlatScale.begin(), doubleFlatScale.end(), _writtenPitchClass);

    // Check if the current note is a double flat/sharp note
    const bool isInsideSharpScale = sharpNote != sharpScale.end();
    const bool isInsideFlatScale = flatNote != flatScale.end();
    const bool isInsideDoubleSharpScale = doubleShapNote != doubleSharpScale.end();
    const bool isInsideDoubleFlatScale = doubleFlatNote != doubleFlatScale.end();

    // Get the transposition direction
    const bool upDirection = (_transposeChromatic > 0) ? true : false;

    // Define control variables
    bool useFlatScale = false;
    bool useSharpScale = false;
    bool useDoubleFlatScale = false;
    bool useDoubleSharpScale = false;

    // Index control variable
    int writtenPitchClassIdx = 0;

    // Choose the between use flat or sharp scale
    if (isInsideSharpScale && upDirection) {
        useSharpScale = true;

        // Get the index of the written pitch
        writtenPitchClassIdx = sharpNote - sharpScale.begin();

    } else if ((isInsideSharpScale && !upDirection) || isInsideFlatScale) {
        useFlatScale = true;
        // Get the index of the written pitch
        writtenPitchClassIdx = flatNote - flatScale.begin();

    } else if (isInsideDoubleSharpScale) {
        useDoubleSharpScale = true;

        // Get the index of the written pitch
        writtenPitchClassIdx = doubleShapNote - doubleSharpScale.begin();

    } else if (isInsideDoubleFlatScale) {
        useDoubleFlatScale = true;

        // Get the index of the written pitch
        writtenPitchClassIdx = doubleFlatNote - doubleFlatScale.begin();
    } else {
        std::cerr << "[ERROR] Unknown note type" << std::endl;
    }

    // Compute the temp 'fake' index (can be out of array bounds)
    const int tempIdx = writtenPitchClassIdx + _transposeChromatic;

    // Scale index to be setted in the future
    int soundingPitchClassIdx = 0;

    // Get the single octave chromatic scale index for sounding pitchClass
    if (tempIdx >= 0 && tempIdx <= 12) { // Transpose inside one octave
        soundingPitchClassIdx = tempIdx % 12;
    } else if (tempIdx > 0 && tempIdx > 12) { // Transpose outside one octave
        soundingPitchClassIdx = tempIdx % 12;
        _soundingOctave++;
    } else { // Transpose down octave
        soundingPitchClassIdx = tempIdx + 12;
        _soundingOctave--;
    }

    // Get the correct sounding pitch
    if (useSharpScale) {
        _soundingPitchClass = sharpScale[soundingPitchClassIdx];
    } else if (useFlatScale) {
        _soundingPitchClass = flatScale[soundingPitchClassIdx];
    } else if (useDoubleSharpScale) {
        _soundingPitchClass = doubleSharpScale[soundingPitchClassIdx];
    } else if (useDoubleFlatScale) {
        _soundingPitchClass = doubleFlatScale[soundingPitchClassIdx];
    } else {
        std::cerr << "[ERROR] Unknown note type in Note::setTransposingInterval()" << std::endl;
    }
}

void Note::setVoice(const int voice)
{
    _voice = voice;
}

void Note::setStaff(const int staff)
{
    _staff = staff;
}

void Note::setStem(const std::string& stem)
{
    _stem = stem;
}

void Note::setIsTuplet(const bool isTuplet)
{
    _isTuplet = isTuplet;
}

void Note::setTupleValues(const int actualNotes, const int normalNotes, const std::string& normalType)
{
    _timeModification.actualNotes = actualNotes;
    _timeModification.normalNotes = normalNotes;
    _timeModification.normalType = normalType;
}

bool Note::isTuplet() const
{
    return _isTuplet;
}

std::string Note::getStem() const
{
    return _stem;
}

void Note::setTieStart()
{
    _tie.clear();
    _tie.push_back("start");
}

void Note::setTieStop()
{
    _tie.clear();
    _tie.push_back("stop");
}

void Note::setTieStopStart()
{
    _tie.clear();
    _tie.push_back("start");
    _tie.push_back("stop");
}

void Note::addTie(const std::string& tieType)
{
    _tie.push_back(tieType);
}

void Note::removeTies()
{
    _tie.clear();
}

std::string Note::getPitchStep() const
{
    return _writtenPitchClass.substr(0, 1);
}

int Note::getVoice() const
{
    return _voice;
}

int Note::getStaff() const
{
    return _staff;
}

std::vector<std::string> Note::getTie() const
{
    return _tie;
}

std::pair<std::string, std::string> Note::getSlur() const
{
    return _slur;
}

void Note::addSlur(const std::string& slurType, const std::string& slurOrientation)
{
    _slur.first = slurType;
    _slur.second = slurOrientation;
}

void Note::addArticulation(const std::string& articulation)
{
    _articulation.push_back(articulation);
}

void Note::addBeam(const std::string& beam)
{
    _beam.push_back(beam);
}

std::vector<std::string> Note::getBeam() const
{
    return _beam;
}

std::vector<std::string> Note::getArticulation() const
{
    return _articulation;
}

const std::string Note::getSoundingPitchClass() const
{
    return _soundingPitchClass;
}

const std::string Note::getSoundingPitch() const
{
    // Check transposing instrument
    if (!isTransposed()) { return getWrittenPitch(); }

    return getSoundingPitchClass() + std::to_string(getSoundingOctave());
}

const std::string Note::getDiatonicWrittenPitchClass() const
{
    return (_isNoteOn) ? getWrittenPitchClass().substr(0, 1) : MUSIC_XML::PITCH::REST;
}

const std::string Note::getDiatonicSoundingPitchClass() const
{
     return (_isNoteOn) ? getSoundingPitchClass().substr(0, 1) : MUSIC_XML::PITCH::REST;
}

int Note::getSoundingOctave() const
{
    // Check transposing instrument
    if (!isTransposed()) { return getWrittenOctave(); }

    // Get the sounding pitch
    const std::string soundingPitch = Helper::midiNote2pitch(_midiNumber);

    // Split the sounding pitch to get the octave
    std::string pitch, step, pitchAcc, alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;

    Helper::splitPitch(soundingPitch, pitchAcc, step, octave, alterValue, alterSymbol);

    return octave;
}

const std::string Note::getWrittenPitchClass() const
{
    return (_isNoteOn) ? _writtenPitchClass : MUSIC_XML::PITCH::REST;
}

const std::string Note::getWrittenPitch() const
{
    return (_isNoteOn) ? _writtenPitchClass + std::to_string(_writtenOctave) : MUSIC_XML::PITCH::REST;
}

int Note::getWrittenOctave() const
{
    return _writtenOctave;
}

std::string Note::getPitch() const
{
    return getWrittenPitch();
}

bool Note::inChord() const
{
    return _inChord;
}

const std::string Note::toXML(const size_t instrumentId, const int identSize) const
{
    std::string xml = Helper::generateIdentation(3, identSize) + "<note>\n";

    if (isGraceNote()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<grace />\n");
    }

    if (_inChord) {
        xml.append(Helper::generateIdentation(4, identSize) + "<chord />\n");
    }

    if (_isNoteOn) {

        if (_isPitched) {
            std::string pitch =
                std::string(Helper::generateIdentation(4, identSize) + "<pitch>\n") +
                    Helper::generateIdentation(5, identSize) + "<step>" + _writtenPitchClass[0] + "</step>\n";

                if (_writtenPitchClass.size() > 1) {
                    float alterValue = Helper::alterSymbol2Value(_alterSymbol);
                    int x = static_cast<int>(alterValue);
                    pitch.append(Helper::generateIdentation(5, identSize) + "<alter>" + std::to_string(x) + "</alter>\n");
                }

                pitch.append(Helper::generateIdentation(5, identSize) + "<octave>" + std::to_string(_writtenOctave) + "</octave>\n" +
                             Helper::generateIdentation(4, identSize) + "</pitch>\n");

            xml.append(pitch);
        } else { // Unpitched notes
            std::string unpitched =
                std::string(Helper::generateIdentation(4, identSize) + "<unpitched>\n") +
                    Helper::generateIdentation(5, identSize) + "<display-step>" + _writtenPitchClass[0] + "</display-step>\n";

                if (_writtenPitchClass.size() > 1) {
                    float alterValue = Helper::alterSymbol2Value(_alterSymbol);
                    int x = static_cast<int>(alterValue);
                    unpitched.append(Helper::generateIdentation(5, identSize) + "<alter>" + std::to_string(x) + "</alter>\n");
                }

                unpitched.append(Helper::generateIdentation(5, identSize) + "<display-octave>" + std::to_string(_writtenOctave) + "</display-octave>\n" +
                             Helper::generateIdentation(4, identSize) + "</unpitched>\n");

            xml.append(unpitched);
        }
    } else {
        xml.append(Helper::generateIdentation(4, identSize) + "<rest/>\n");
    }

    if (!isGraceNote()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<duration>" + std::to_string(getDurationTicks()) + "</duration>\n");

        for (const auto& tie : _tie) {
            xml.append(Helper::generateIdentation(4, identSize) + "<tie type=\"" + tie + "\" />\n");
        }
    }

    if (_isNoteOn) {
        if (_unpitchedIndex == 0) {
            xml.append(Helper::generateIdentation(4, identSize) + "<instrument id=\"P" + std::to_string(instrumentId+1) + "-I" + std::to_string(1) + "\" />\n");
        } else {
            xml.append(Helper::generateIdentation(4, identSize) + "<instrument id=\"P" + std::to_string(instrumentId+1) + "-I" + std::to_string(_unpitchedIndex) + "\" />\n");
        }
    }

    xml.append(Helper::generateIdentation(4, identSize) + "<voice>" + std::to_string(_voice) + "</voice>\n");

    if (_isGraceNote) {
        xml.append(Helper::generateIdentation(4, identSize) + "<type>16th</type>\n");
    } else {
        if (!_duration.noteType.empty()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<type>" + _duration.noteType + "</type>\n");
        } else {
            // xml.append(Helper::generateIdentation(4, identSize) + "<type>" + Helper::ticks2noteType(_durationTicks, _divisionsPerQuarterNote) + "</type>\n");
        }
    }

    if (_isTuplet) {
        xml.append(Helper::generateIdentation(4, identSize) + "<time-modification>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<actual-notes>" + std::to_string(_timeModification.actualNotes) + "</actual-notes>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<normal-notes>" + std::to_string(_timeModification.normalNotes) + "</normal-notes>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<normal-type>" + _timeModification.normalType + "</normal-type>\n");
        xml.append(Helper::generateIdentation(4, identSize) + "</time-modification>\n");
    }

    for (int d = 0; d < _duration.numDots; d++) {
        xml.append(Helper::generateIdentation(4, identSize) + "<dot />\n");
    }

    if (!_stem.empty()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<stem>" + _stem + "</stem>\n");
    }

    xml.append(Helper::generateIdentation(4, identSize) + "<staff>" + std::to_string(_staff + 1) + "</staff>\n");

    for (size_t b = 0; b <  _beam.size(); b++) {
        xml.append(Helper::generateIdentation(4, identSize) + "<beam number=\"" + std::to_string(b+1) + "\">" + _beam[b] + "</beam>\n");
    }

    bool haveNotationTag = (!_tie.empty() || !_slur.first.empty()) ? true : false;

    if (haveNotationTag) {
        xml.append(Helper::generateIdentation(4, identSize) + "<notations>\n");

        if (!_articulation.empty()) {
            xml.append(Helper::generateIdentation(5, identSize) + "<articulations>\n");
            for (const auto& articulation : _articulation) {
                xml.append(Helper::generateIdentation(6, identSize) + "<" + articulation + " />\n");
            }
            xml.append(Helper::generateIdentation(5, identSize) + "</articulations>\n");
        }

        for (const auto& tie : _tie) {
            xml.append(Helper::generateIdentation(5, identSize) + "<tied type=\"" + tie + "\" />\n");
        }

        if (!_slur.first.empty()) {
            xml.append(Helper::generateIdentation(5, identSize) + "<slur type=\"" + _slur.first + "\" orientation=\"" + _slur.second + "\" />\n");
        }

        xml.append(Helper::generateIdentation(4, identSize) + "</notations>\n");
    }

    xml.append(Helper::generateIdentation(3, identSize) + "</note>\n");

    return xml;
}

int Note::getMIDINumber() const
{
    return _midiNumber;
}
