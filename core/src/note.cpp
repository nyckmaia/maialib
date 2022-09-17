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

std::string Note::getEnharmonicPitch(const bool alternativeEnhamonicPitch) const
{
    if (isNoteOff()) { return MUSIC_XML::PITCH::REST; }

    std::string enharmonicPitch;

    switch (hash(getPitch().c_str()))
    {
        // Octave -1
        case hash("Cbb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#-2" : "Bb-2";  break;
        case hash("Cb-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax-2" : "B-2";   break;
        case hash("C-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#-2" : "Dbb-1"; break;
        case hash("C#-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx-2" : "Db-1";  break;
        case hash("Cx-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb-1" : "D-1";  break;
        case hash("Dbb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#-2" : "C-1";   break;
        case hash("Db-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx-2" : "C#-1";  break;
        case hash("D-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx-1" : "Ebb-1"; break;
        case hash("D#-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb-1" : "Eb-1"; break;
        case hash("Dx-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb-1" : "E-1";   break;
        case hash("Ebb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx-1" : "D-1";   break;
        case hash("Eb-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb-1" : "D#-1"; break;
        case hash("E-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx-1" : "Fb-1";  break;
        case hash("E#-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb-1" : "F-1";  break;
        case hash("Ex-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb-1" : "F#-1";  break;
        case hash("Fbb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#-1" : "Eb-1";  break;
        case hash("Fb-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx-1" : "E-1";   break;
        case hash("F-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#-1" : "Gbb-1"; break;
        case hash("F#-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex-1" : "Gb-1";  break;
        case hash("Fx-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb-1" : "G-1";  break;
        case hash("Gbb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#-1" : "F-1";   break;
        case hash("Gb-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex-1" : "F#-1";  break;
        case hash("G-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx-1" : "Abb-1"; break;
        case hash("G#-1"):  enharmonicPitch = "Ab-1"; break;
        case hash("Gx-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb-1" : "A-1";  break;
        case hash("Abb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx-1" : "G-1";   break;
        case hash("Ab-1"):  enharmonicPitch = "G#-1"; break;
        case hash("A-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx-1" : "Bbb-1"; break;
        case hash("A#-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb0" : "Bb-1"; break;
        case hash("Ax-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb0" : "B-1";   break;
        case hash("Bbb-1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx-1" : "A-1";   break;
        case hash("Bb-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb0" : "A#-1"; break;
        case hash("B-1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax-1" : "Cb0";  break;
        case hash("B#-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb0" : "C0";  break;
        case hash("Bx-1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db0" : "C#0";  break;

        // Octave 0
        case hash("Cbb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#-1" : "Bb-1";  break;
        case hash("Cb0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax-1" : "B-1";   break;
        case hash("C0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#-1" : "Dbb0"; break;
        case hash("C#0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx-1" : "Db0";  break;
        case hash("Cx0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb0" : "D0";  break;
        case hash("Dbb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#-1" : "C0";   break;
        case hash("Db0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx-1" : "C#0";  break;
        case hash("D0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx0" : "Ebb0"; break;
        case hash("D#0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb0" : "Eb0"; break;
        case hash("Dx0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb0" : "E0";   break;
        case hash("Ebb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx0" : "D0";   break;
        case hash("Eb0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb0" : "D#0"; break;
        case hash("E0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx0" : "Fb0";  break;
        case hash("E#0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb0" : "F0";  break;
        case hash("Ex0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb0" : "F#0";  break;
        case hash("Fbb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#0" : "Eb0";  break;
        case hash("Fb0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx0" : "E0";   break;
        case hash("F0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#0" : "Gbb0"; break;
        case hash("F#0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex0" : "Gb0";  break;
        case hash("Fx0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb0" : "G0";  break;
        case hash("Gbb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#0" : "F0";   break;
        case hash("Gb0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex0" : "F#0";  break;
        case hash("G0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx0" : "Abb0"; break;
        case hash("G#0"):  enharmonicPitch = "Ab0"; break;
        case hash("Gx0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb0" : "A0";  break;
        case hash("Abb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx0" : "G0";   break;
        case hash("Ab0"):  enharmonicPitch = "G#0"; break;
        case hash("A0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx0" : "Bbb0"; break;
        case hash("A#0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb1" : "Bb0"; break;
        case hash("Ax0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb1" : "B0";   break;
        case hash("Bbb0"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx0" : "A0";   break;
        case hash("Bb0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb1" : "A#0"; break;
        case hash("B0"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax0" : "Cb1";  break;
        case hash("B#0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb1" : "C1";  break;
        case hash("Bx0"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db1" : "C#1";  break;

        // Octave 1
        case hash("Cbb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#0" : "Bb0";  break;
        case hash("Cb1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax0" : "B0";   break;
        case hash("C1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#0" : "Dbb1"; break;
        case hash("C#1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx0" : "Db1";  break;
        case hash("Cx1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb1" : "D1";  break;
        case hash("Dbb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#0" : "C1";   break;
        case hash("Db1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx0" : "C#1";  break;
        case hash("D1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx1" : "Ebb1"; break;
        case hash("D#1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb1" : "Eb1"; break;
        case hash("Dx1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb1" : "E1";   break;
        case hash("Ebb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx1" : "D1";   break;
        case hash("Eb1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb1" : "D#1"; break;
        case hash("E1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx1" : "Fb1";  break;
        case hash("E#1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb1" : "F1";  break;
        case hash("Ex1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb1" : "F#1";  break;
        case hash("Fbb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#1" : "Eb1";  break;
        case hash("Fb1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx1" : "E1";   break;
        case hash("F1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#1" : "Gbb1"; break;
        case hash("F#1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex1" : "Gb1";  break;
        case hash("Fx1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb1" : "G1";  break;
        case hash("Gbb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#1" : "F1";   break;
        case hash("Gb1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex1" : "F#1";  break;
        case hash("G1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx1" : "Abb1"; break;
        case hash("G#1"):  enharmonicPitch = "Ab1"; break;
        case hash("Gx1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb1" : "A1";  break;
        case hash("Abb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx1" : "G1";   break;
        case hash("Ab1"):  enharmonicPitch = "G#1"; break;
        case hash("A1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx1" : "Bbb1"; break;
        case hash("A#1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb2" : "Bb1"; break;
        case hash("Ax1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb2" : "B1";   break;
        case hash("Bbb1"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx1" : "A1";   break;
        case hash("Bb1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb2" : "A#1"; break;
        case hash("B1"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax1" : "Cb2";  break;
        case hash("B#1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb2" : "C2";  break;
        case hash("Bx1"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db2" : "C#2";  break;

        // Octave 2
        case hash("Cbb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#1" : "Bb1";  break;
        case hash("Cb2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax1" : "B1";   break;
        case hash("C2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#1" : "Dbb2"; break;
        case hash("C#2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx1" : "Db2";  break;
        case hash("Cx2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb2" : "D2";  break;
        case hash("Dbb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#1" : "C2";   break;
        case hash("Db2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx1" : "C#2";  break;
        case hash("D2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx2" : "Ebb2"; break;
        case hash("D#2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb2" : "Eb2"; break;
        case hash("Dx2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb2" : "E2";   break;
        case hash("Ebb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx2" : "D2";   break;
        case hash("Eb2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb2" : "D#2"; break;
        case hash("E2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx2" : "Fb2";  break;
        case hash("E#2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb2" : "F2";  break;
        case hash("Ex2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb2" : "F#2";  break;
        case hash("Fbb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#2" : "Eb2";  break;
        case hash("Fb2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx2" : "E2";   break;
        case hash("F2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#2" : "Gbb2"; break;
        case hash("F#2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex2" : "Gb2";  break;
        case hash("Fx2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb2" : "G2";  break;
        case hash("Gbb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#2" : "F2";   break;
        case hash("Gb2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex2" : "F#2";  break;
        case hash("G2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx2" : "Abb2"; break;
        case hash("G#2"):  enharmonicPitch = "Ab2"; break;
        case hash("Gx2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb2" : "A2";  break;
        case hash("Abb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx2" : "G2";   break;
        case hash("Ab2"):  enharmonicPitch = "G#2"; break;
        case hash("A2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx2" : "Bbb2"; break;
        case hash("A#2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb3" : "Bb2"; break;
        case hash("Ax2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb3" : "B2";   break;
        case hash("Bbb2"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx2" : "A2";   break;
        case hash("Bb2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb3" : "A#2"; break;
        case hash("B2"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax2" : "Cb3";  break;
        case hash("B#2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb3" : "C3";  break;
        case hash("Bx2"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db3" : "C#3";  break;

        // Octave 3
        case hash("Cbb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#2" : "Bb2";  break;
        case hash("Cb3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax2" : "B2";   break;
        case hash("C3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#2" : "Dbb3"; break;
        case hash("C#3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx2" : "Db3";  break;
        case hash("Cx3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb3" : "D3";  break;
        case hash("Dbb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#2" : "C3";   break;
        case hash("Db3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx2" : "C#3";  break;
        case hash("D3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx3" : "Ebb3"; break;
        case hash("D#3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb3" : "Eb3"; break;
        case hash("Dx3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb3" : "E3";   break;
        case hash("Ebb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx3" : "D3";   break;
        case hash("Eb3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb3" : "D#3"; break;
        case hash("E3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx3" : "Fb3";  break;
        case hash("E#3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb3" : "F3";  break;
        case hash("Ex3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb3" : "F#3";  break;
        case hash("Fbb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#3" : "Eb3";  break;
        case hash("Fb3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx3" : "E3";   break;
        case hash("F3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#3" : "Gbb3"; break;
        case hash("F#3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex3" : "Gb3";  break;
        case hash("Fx3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb3" : "G3";  break;
        case hash("Gbb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#3" : "F3";   break;
        case hash("Gb3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex3" : "F#3";  break;
        case hash("G3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx3" : "Abb3"; break;
        case hash("G#3"):  enharmonicPitch = "Ab3"; break;
        case hash("Gx3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb3" : "A3";  break;
        case hash("Abb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx3" : "G3";   break;
        case hash("Ab3"):  enharmonicPitch = "G#3"; break;
        case hash("A3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx3" : "Bbb3"; break;
        case hash("A#3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb4" : "Bb3"; break;
        case hash("Ax3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb4" : "B3";   break;
        case hash("Bbb3"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx3" : "A3";   break;
        case hash("Bb3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb4" : "A#3"; break;
        case hash("B3"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax3" : "Cb4";  break;
        case hash("B#3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb4" : "C4";  break;
        case hash("Bx3"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db4" : "C#4";  break;

        // Octave 4
        case hash("Cbb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#3" : "Bb3";  break;
        case hash("Cb4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax3" : "B3";   break;
        case hash("C4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#3" : "Dbb4"; break;
        case hash("C#4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx3" : "Db4";  break;
        case hash("Cx4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb4" : "D4";  break;
        case hash("Dbb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#3" : "C4";   break;
        case hash("Db4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx3" : "C#4";  break;
        case hash("D4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx4" : "Ebb4"; break;
        case hash("D#4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb4" : "Eb4"; break;
        case hash("Dx4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb4" : "E4";   break;
        case hash("Ebb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx4" : "D4";   break;
        case hash("Eb4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb4" : "D#4"; break;
        case hash("E4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx4" : "Fb4";  break;
        case hash("E#4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb4" : "F4";  break;
        case hash("Ex4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb4" : "F#4";  break;
        case hash("Fbb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#4" : "Eb4";  break;
        case hash("Fb4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx4" : "E4";   break;
        case hash("F4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#4" : "Gbb4"; break;
        case hash("F#4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex4" : "Gb4";  break;
        case hash("Fx4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb4" : "G4";  break;
        case hash("Gbb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#4" : "F4";   break;
        case hash("Gb4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex4" : "F#4";  break;
        case hash("G4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx4" : "Abb4"; break;
        case hash("G#4"):  enharmonicPitch = "Ab4"; break;
        case hash("Gx4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb4" : "A4";  break;
        case hash("Abb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx4" : "G4";   break;
        case hash("Ab4"):  enharmonicPitch = "G#4"; break;
        case hash("A4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx4" : "Bbb4"; break;
        case hash("A#4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb5" : "Bb4"; break;
        case hash("Ax4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb5" : "B4";   break;
        case hash("Bbb4"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx4" : "A4";   break;
        case hash("Bb4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb5" : "A#4"; break;
        case hash("B4"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax4" : "Cb5";  break;
        case hash("B#4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb5" : "C5";  break;
        case hash("Bx4"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db5" : "C#5";  break;

        // Octave 5
        case hash("Cbb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#4" : "Bb4";  break;
        case hash("Cb5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax4" : "B4";   break;
        case hash("C5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#4" : "Dbb5"; break;
        case hash("C#5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx4" : "Db5";  break;
        case hash("Cx5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb5" : "D5";  break;
        case hash("Dbb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#4" : "C5";   break;
        case hash("Db5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx4" : "C#5";  break;
        case hash("D5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx5" : "Ebb5"; break;
        case hash("D#5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb5" : "Eb5"; break;
        case hash("Dx5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb5" : "E5";   break;
        case hash("Ebb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx5" : "D5";   break;
        case hash("Eb5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb5" : "D#5"; break;
        case hash("E5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx5" : "Fb5";  break;
        case hash("E#5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb5" : "F5";  break;
        case hash("Ex5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb5" : "F#5";  break;
        case hash("Fbb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#5" : "Eb5";  break;
        case hash("Fb5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx5" : "E5";   break;
        case hash("F5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#5" : "Gbb5"; break;
        case hash("F#5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex5" : "Gb5";  break;
        case hash("Fx5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb5" : "G5";  break;
        case hash("Gbb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#5" : "F5";   break;
        case hash("Gb5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex5" : "F#5";  break;
        case hash("G5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx5" : "Abb5"; break;
        case hash("G#5"):  enharmonicPitch = "Ab5"; break;
        case hash("Gx5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb5" : "A5";  break;
        case hash("Abb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx5" : "G5";   break;
        case hash("Ab5"):  enharmonicPitch = "G#5"; break;
        case hash("A5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx5" : "Bbb5"; break;
        case hash("A#5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb6" : "Bb5"; break;
        case hash("Ax5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb6" : "B5";   break;
        case hash("Bbb5"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx5" : "A5";   break;
        case hash("Bb5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb6" : "A#5"; break;
        case hash("B5"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax5" : "Cb6";  break;
        case hash("B#5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb6" : "C6";  break;
        case hash("Bx5"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db6" : "C#6";  break;

        // Octave 6
        case hash("Cbb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#5" : "Bb5";  break;
        case hash("Cb6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax5" : "B5";   break;
        case hash("C6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#5" : "Dbb6"; break;
        case hash("C#6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx5" : "Db6";  break;
        case hash("Cx6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb6" : "D6";  break;
        case hash("Dbb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#5" : "C6";   break;
        case hash("Db6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx5" : "C#6";  break;
        case hash("D6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx6" : "Ebb6"; break;
        case hash("D#6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb6" : "Eb6"; break;
        case hash("Dx6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb6" : "E6";   break;
        case hash("Ebb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx6" : "D6";   break;
        case hash("Eb6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb6" : "D#6"; break;
        case hash("E6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx6" : "Fb6";  break;
        case hash("E#6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb6" : "F6";  break;
        case hash("Ex6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb6" : "F#6";  break;
        case hash("Fbb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#6" : "Eb6";  break;
        case hash("Fb6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx6" : "E6";   break;
        case hash("F6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#6" : "Gbb6"; break;
        case hash("F#6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex6" : "Gb6";  break;
        case hash("Fx6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb6" : "G6";  break;
        case hash("Gbb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#6" : "F6";   break;
        case hash("Gb6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex6" : "F#6";  break;
        case hash("G6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx6" : "Abb6"; break;
        case hash("G#6"):  enharmonicPitch = "Ab6"; break;
        case hash("Gx6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb6" : "A6";  break;
        case hash("Abb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx6" : "G6";   break;
        case hash("Ab6"):  enharmonicPitch = "G#6"; break;
        case hash("A6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx6" : "Bbb6"; break;
        case hash("A#6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb7" : "Bb6"; break;
        case hash("Ax6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb7" : "B6";   break;
        case hash("Bbb6"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx6" : "A6";   break;
        case hash("Bb6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb7" : "A#6"; break;
        case hash("B6"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax6" : "Cb7";  break;
        case hash("B#6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb7" : "C7";  break;
        case hash("Bx6"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db7" : "C#7";  break;

        // Octave 7
        case hash("Cbb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#6" : "Bb6";  break;
        case hash("Cb7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax6" : "B6";   break;
        case hash("C7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#6" : "Dbb7"; break;
        case hash("C#7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx6" : "Db7";  break;
        case hash("Cx7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb7" : "D7";  break;
        case hash("Dbb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#6" : "C7";   break;
        case hash("Db7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx6" : "C#7";  break;
        case hash("D7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx7" : "Ebb7"; break;
        case hash("D#7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb7" : "Eb7"; break;
        case hash("Dx7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb7" : "E7";   break;
        case hash("Ebb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx7" : "D7";   break;
        case hash("Eb7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb7" : "D#7"; break;
        case hash("E7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx7" : "Fb7";  break;
        case hash("E#7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb7" : "F7";  break;
        case hash("Ex7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb7" : "F#7";  break;
        case hash("Fbb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#7" : "Eb7";  break;
        case hash("Fb7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx7" : "E7";   break;
        case hash("F7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#7" : "Gbb7"; break;
        case hash("F#7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex7" : "Gb7";  break;
        case hash("Fx7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb7" : "G7";  break;
        case hash("Gbb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#7" : "F7";   break;
        case hash("Gb7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex7" : "F#7";  break;
        case hash("G7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx7" : "Abb7"; break;
        case hash("G#7"):  enharmonicPitch = "Ab7"; break;
        case hash("Gx7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb7" : "A7";  break;
        case hash("Abb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx7" : "G7";   break;
        case hash("Ab7"):  enharmonicPitch = "G#7"; break;
        case hash("A7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx7" : "Bbb7"; break;
        case hash("A#7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb8" : "Bb7"; break;
        case hash("Ax7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb8" : "B7";   break;
        case hash("Bbb7"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx7" : "A7";   break;
        case hash("Bb7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb8" : "A#7"; break;
        case hash("B7"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax7" : "Cb8";  break;
        case hash("B#7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb8" : "C8";  break;
        case hash("Bx7"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db8" : "C#8";  break;

        // Octave 8
        case hash("Cbb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#7" : "Bb7";  break;
        case hash("Cb8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax7" : "B7";   break;
        case hash("C8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#7" : "Dbb8"; break;
        case hash("C#8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx7" : "Db8";  break;
        case hash("Cx8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb8" : "D8";  break;
        case hash("Dbb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#7" : "C8";   break;
        case hash("Db8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx7" : "C#8";  break;
        case hash("D8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx8" : "Ebb8"; break;
        case hash("D#8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb8" : "Eb8"; break;
        case hash("Dx8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb8" : "E8";   break;
        case hash("Ebb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx8" : "D8";   break;
        case hash("Eb8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb8" : "D#8"; break;
        case hash("E8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx8" : "Fb8";  break;
        case hash("E#8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb8" : "F8";  break;
        case hash("Ex8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb8" : "F#8";  break;
        case hash("Fbb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#8" : "Eb8";  break;
        case hash("Fb8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx8" : "E8";   break;
        case hash("F8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#8" : "Gbb8"; break;
        case hash("F#8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex8" : "Gb8";  break;
        case hash("Fx8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb8" : "G8";  break;
        case hash("Gbb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#8" : "F8";   break;
        case hash("Gb8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex8" : "F#8";  break;
        case hash("G8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx8" : "Abb8"; break;
        case hash("G#8"):  enharmonicPitch = "Ab8"; break;
        case hash("Gx8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb8" : "A8";  break;
        case hash("Abb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx8" : "G8";   break;
        case hash("Ab8"):  enharmonicPitch = "G#8"; break;
        case hash("A8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx8" : "Bbb8"; break;
        case hash("A#8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb9" : "Bb8"; break;
        case hash("Ax8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb9" : "B8";   break;
        case hash("Bbb8"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx8" : "A8";   break;
        case hash("Bb8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb9" : "A#8"; break;
        case hash("B8"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax8" : "Cb9";  break;
        case hash("B#8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb9" : "C9";  break;
        case hash("Bx8"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db9" : "C#9";  break;

        // Octave 9
        case hash("Cbb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "A#8" : "Bb8";  break;
        case hash("Cb9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax8" : "B8";   break;
        case hash("C9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "B#8" : "Dbb9"; break;
        case hash("C#9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx8" : "Db9";  break;
        case hash("Cx9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ebb9" : "D9";  break;
        case hash("Dbb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "B#8" : "C9";   break;
        case hash("Db9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bx8" : "C#9";  break;
        case hash("D9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx9" : "Ebb9"; break;
        case hash("D#9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb9" : "Eb9"; break;
        case hash("Dx9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fb9" : "E9";   break;
        case hash("Ebb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Cx9" : "D9";   break;
        case hash("Eb9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Fbb9" : "D#9"; break;
        case hash("E9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx9" : "Fb9";  break;
        case hash("E#9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gbb9" : "F9";  break;
        case hash("Ex9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Gb9" : "F#9";  break;
        case hash("Fbb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "D#9" : "Eb9";  break;
        case hash("Fb9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dx9" : "E9";   break;
        case hash("F9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "E#9" : "Gbb9"; break;
        case hash("F#9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex9" : "Gb9";  break;
        case hash("Fx9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Abb9" : "G9";  break;
        case hash("Gbb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "E#9" : "F9";   break;
        case hash("Gb9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Ex9" : "F#9";  break;
        case hash("G9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx9" : "Abb9"; break;
        case hash("G#9"):  enharmonicPitch = "Ab9"; break;
        case hash("Gx9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Bbb9" : "A9";  break;
        case hash("Abb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Fx9" : "G9";   break;
        case hash("Ab9"):  enharmonicPitch = "G#9"; break;
        case hash("A9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx9" : "Bbb9"; break;
        case hash("A#9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb10" : "Bb9"; break;
        case hash("Ax9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cb10" : "B9";   break;
        case hash("Bbb9"): enharmonicPitch = (alternativeEnhamonicPitch) ? "Gx9" : "A9";   break;
        case hash("Bb9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Cbb10" : "A#9"; break;
        case hash("B9"):   enharmonicPitch = (alternativeEnhamonicPitch) ? "Ax9" : "Cb10";  break;
        case hash("B#9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Dbb10" : "C10";  break;
        case hash("Bx9"):  enharmonicPitch = (alternativeEnhamonicPitch) ? "Db10" : "C#10";  break;

        default: throw std::runtime_error("Unknown note pitchClass"); break;
    }

    return enharmonicPitch;
}

std::vector<std::string> Note::getEnharmonicPitches(const bool includeCurrentPitch) const
{
    if (includeCurrentPitch) {
        return std::vector<std::string>({getPitch(), getEnharmonicPitch(false), getEnharmonicPitch(true)});
    }

    return std::vector<std::string>({getEnharmonicPitch(false), getEnharmonicPitch(true)});
}

void Note::toEnharmonicPitch(const bool alternativeEnhamonicPitch)
{
    setPitch(getEnharmonicPitch(alternativeEnhamonicPitch));
}

Note Note::getEnharmonicNote(const bool alternativeEnhamonicPitch) const
{
    return Note(getEnharmonicPitch(alternativeEnhamonicPitch));
}

std::vector<Note> Note::getEnharmonicNotes(const bool includeCurrentPitch) const
{
    if (includeCurrentPitch) {
        return {Note(getPitch()), Note(getEnharmonicPitch(false)), Note(getEnharmonicPitch(true))}; 
    }

    return {Note(getEnharmonicPitch(false)), Note(getEnharmonicPitch(true))};
}

float Note::getFrequency() const
{
    return Helper::pitch2freq(getSoundingPitch());
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
    return Helper::midiNote2octave(_midiNumber);
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
            xml.append(Helper::generateIdentation(4, identSize) + "<type>" + getShortType() + "</type>\n");
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
