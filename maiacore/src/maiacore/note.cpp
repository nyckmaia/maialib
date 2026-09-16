#include "maiacore/note.h"

#include <ctype.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

namespace {
// Returns the pitch string that spells 'midiNumber' with 'alter' semitones (-2..2), or an
// empty string if no diatonic step fits or the octave falls outside the supported range
std::string spellMidiNumber(const int midiNumber, const int alter) {
    static const std::array<std::string, 5> alterSymbols = {"bb", "b", "", "#", "x"};

    const int base = midiNumber - alter;
    const int pitchClassIdx = ((base % 12) + 12) % 12;
    const auto stepIt = std::find(c_diatonicStepSemitones.begin(), c_diatonicStepSemitones.end(),
                                  pitchClassIdx);
    if (stepIt == c_diatonicStepSemitones.end()) {
        return {};
    }

    const int octave = (base - pitchClassIdx) / 12 - 1;
    if (octave < c_minPitchOctave || octave > c_maxPitchOctave) {
        return {};
    }

    const auto stepIdx =
        static_cast<size_t>(std::distance(c_diatonicStepSemitones.begin(), stepIt));
    return c_C_diatonicScale[stepIdx] + alterSymbols[static_cast<size_t>(alter + 2)] +
           std::to_string(octave);
}
}  // namespace

Note::Note() : Note("A4") {}

Note::Note(const std::string& pitch, const RhythmFigure rhythmFigure, bool isNoteOn, bool inChord,
           int transposeDiatonic, int transposeChromatic, const int divisionsPerQuarterNote)
    : _writtenPitchClass(MUSIC_XML::PITCH::REST),
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
      _unpitchedIndex(0),
      _duration(Helper::rhythmFigure2Ticks(rhythmFigure, divisionsPerQuarterNote),
                divisionsPerQuarterNote) {
    // Rest case: This is necessary to prevent: empty pitchClass + alterSymbol
    if (pitch.empty() || (pitch.find(MUSIC_XML::PITCH::REST) != std::string::npos) ||
        isNoteOn == false) {
        // setDuration(duration, divisionsPerQuarterNote);
        return;
    }

    std::string pitchClass;
    std::string pitchStep;
    int octave = 0;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, _alterSymbol);

    _writtenPitchClass = pitchClass;
    _writtenOctave = octave;
    _midiNumber = Helper::pitch2midiNote(pitchClass + std::to_string(octave));
    _inChord = inChord;
    _soundingPitchClass = pitchClass;
    _soundingOctave = octave;
    _transposeDiatonic = transposeDiatonic;
    _transposeChromatic = transposeChromatic;
    _isNoteOn = true;
    // _duration.divisionsPerQuarterNote = divisionsPerQuarterNote;
    // setDuration(duration, divisionsPerQuarterNote);

    // Update the sounding Pitch/PitchClass and MIDI number
    setTransposingInterval(transposeDiatonic, transposeChromatic);
}

Note::Note(const int midiNumber, const std::string& accType, const RhythmFigure duration,
           bool isNoteOn, bool inChord, const int transposeDiatonic, const int transposeChromatic,
           const int divisionsPerQuarterNote)
    : Note(Helper::midiNote2pitch(midiNumber, accType), duration, isNoteOn, inChord,
           transposeDiatonic, transposeChromatic, divisionsPerQuarterNote) {
    // Range validation (octaves -1..11) is already enforced by Helper::midiNote2pitch().
}

Note::~Note() {}

void Note::info() const {
    LOG_INFO("Is note on: " << std::boolalpha << _isNoteOn);
    LOG_INFO("Pitch: " << getPitch());
    LOG_INFO("Note Type: " << _duration.getNoteType());
    LOG_INFO("Quarter Duration: " << getQuarterDuration());
    LOG_INFO("Voice: " << _voice);
    LOG_INFO("Staff: " << _staff);
    LOG_INFO("MIDI Number: " << getMidiNumber());
    LOG_INFO("Stem: " << _stem);
    LOG_INFO("Beams: " << _beam.size());
    LOG_INFO("Is Tuplet: " << std::boolalpha << _isTuplet);
    LOG_INFO("Is Grace Note: " << std::boolalpha << _isGraceNote);
    LOG_INFO("In Chord: " << std::boolalpha << _inChord);
    LOG_INFO("Transpose Diatonic: " << _transposeDiatonic);
    LOG_INFO("Transpose Chromatic: " << _transposeChromatic);
}

void Note::setUnpitchedIndex(const int unpitchedIndex) { _unpitchedIndex = unpitchedIndex; }

int Note::getUnpitchedIndex() const { return _unpitchedIndex; }

void Note::setPitchClass(const std::string& pitchClass) {
    _writtenPitchClass = pitchClass;
    _soundingPitchClass = _writtenPitchClass;

    // Store the alter symbol: # / b
    if (pitchClass.size() > 1) {
        _alterSymbol = pitchClass.substr(1, pitchClass.size());
    }

    // Update sounding Pitch Class
    setTransposingInterval(_transposeDiatonic, _transposeChromatic);
}

void Note::setIsPitched(const bool isPitched) { _isPitched = isPitched; }

bool Note::isPitched() const { return _isPitched; }

std::string Note::getPitchClass() const { return getSoundingPitchClass(); }

void Note::setOctave(const int octave) {
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

int Note::getOctave() const { return _soundingOctave; }

int Note::getTransposeDiatonic() const { return _transposeDiatonic; }

int Note::getTransposeChromatic() const { return _transposeChromatic; }

void Note::setDuration(const Duration& duration) { _duration = duration; }

void Note::setDuration(const float quarterDuration, const int divisionsPerQuarterNote) {
    _duration.setQuarterDuration(quarterDuration, divisionsPerQuarterNote);
}

void Note::setDuration(const int durationTicks, const int divisionsPerQuarterNote) {
    _duration = Duration(durationTicks, divisionsPerQuarterNote);
}

// void Note::setDuration(const RhythmFigure rhythmFigure, const int divisionsPerQuarterNote) {
//     _duration.duration = rhythmFigure;
//     _duration.divisionsPerQuarterNote = divisionsPerQuarterNote;
//     _duration.noteType = Helper::rhythmFigure2noteType(rhythmFigure);
//     _duration.ticks = Helper::noteType2ticks(_duration.noteType,
//     _duration.divisionsPerQuarterNote); const auto typeDotsPair =
//         Helper::ticks2noteType(_duration.ticks, _duration.divisionsPerQuarterNote);
//     _duration.numDots = typeDotsPair.second;
// }

// void Note::setDuration(const float durationValue, const int lowerTimeSignatureValue,
//                        const int divisionsPerQuarterNote) {
//     if (c_TimeSignatureLowerValueMap.find(lowerTimeSignatureValue) ==
//         c_TimeSignatureLowerValueMap.end()) {
//         LOG_ERROR("Unable to use the lower time signature value: " +
//                   std::to_string(lowerTimeSignatureValue));
//     }

//     const RhythmFigure lowTimeSigDur = c_TimeSignatureLowerValueMap.at(lowerTimeSignatureValue);

//     const std::string lowTimeSigNoteType = Helper::rhythmFigure2noteType(lowTimeSigDur);
//     const int lowTicks = Helper::noteType2ticks(lowTimeSigNoteType, divisionsPerQuarterNote);
//     const int durTick = static_cast<int>(durationValue * lowTicks);

//     // ===== CHECK CONVERT UPPER AND LOWER LIMITS ===== //
//     // Upper limit
//     const std::string maximaNoteType = Helper::rhythmFigure2noteType(RhythmFigure::MAXIMA);
//     const int maximaTicks = Helper::noteType2ticks(maximaNoteType, divisionsPerQuarterNote);

//     // Lower limit
//     const std::string n1024NoteType = Helper::rhythmFigure2noteType(RhythmFigure::N1024TH);
//     const int n1024Ticks = Helper::noteType2ticks(n1024NoteType, divisionsPerQuarterNote);

//     // Check upper and lower limits
//     if (durTick > maximaTicks || durTick < n1024Ticks) {
//         LOG_ERROR("The '" + std::to_string(durationValue) +
//                   "' duration value extrapolates the range of values that can be associated with
//                   a " "rhythmic figure using the time signature lower value '" +
//                   std::to_string(lowerTimeSignatureValue) + "'");
//     }

//     // ===== SET THE DURATION INTERVAL VALUES ===== //
//     _duration.ticks = durTick;

//     const auto durNoteTypePair = Helper::ticks2noteType(_duration.ticks,
//     divisionsPerQuarterNote); _duration.noteType = durNoteTypePair.first; _duration.numDots =
//     durNoteTypePair.second; _duration.duration =
//     Helper::noteType2RhythmFigure(_duration.noteType); _duration.divisionsPerQuarterNote =
//     divisionsPerQuarterNote;
// }

// void Note::setDurationTicks(int durationTicks) {
//     _duration.ticks = durationTicks;
//     const auto typeDotsPair =
//         Helper::ticks2noteType(_duration.ticks, _duration.divisionsPerQuarterNote);
//     _duration.noteType = typeDotsPair.first;
//     _duration.numDots = typeDotsPair.second;
//     _duration.duration = Helper::noteType2RhythmFigure(_duration.noteType);
// }

void Note::setIsGraceNote(const bool isGraceNote) { _isGraceNote = isGraceNote; }

// void Note::removeDots() {
//     _duration.numDots = 0;
//     _duration.noteType = _duration.noteType.substr(0, _duration.noteType.find('-'));
//     _duration.ticks = Helper::noteType2ticks(_duration.noteType,
//     _duration.divisionsPerQuarterNote); _duration.duration =
//     Helper::noteType2RhythmFigure(_duration.noteType);
// }

// void Note::setSingleDot() {
//     _duration.numDots = 1;
//     _duration.noteType = _duration.noteType.substr(0, _duration.noteType.find('-'));
//     _duration.noteType.append("-dot");
//     _duration.ticks = Helper::noteType2ticks(_duration.noteType,
//     _duration.divisionsPerQuarterNote); _duration.duration =
//     Helper::noteType2RhythmFigure(_duration.noteType);
// }

// void Note::setDoubleDot() {
//     _duration.numDots = 2;
//     _duration.noteType = _duration.noteType.substr(0, _duration.noteType.find('-'));
//     _duration.noteType.append("-dot-dot");
//     _duration.ticks = Helper::noteType2ticks(_duration.noteType,
//     _duration.divisionsPerQuarterNote); _duration.duration =
//     Helper::noteType2RhythmFigure(_duration.noteType);
// }

std::string Note::getType() const { return getLongType(); }

std::string Note::getLongType() const { return _duration.getNoteType(); }

std::string Note::getShortType() const {
    return _duration.getNoteType().substr(0, _duration.getNoteType().find('-'));
}

int Note::getDurationTicks() const { return _duration.getTicks(); }

int Note::getNumDots() const { return _duration.getDots(); }

bool Note::isDotted() const { return (_duration.getDots() <= 0) ? false : true; }

bool Note::isDoubleDotted() const { return (_duration.getDots() == 2) ? true : false; }

int Note::getDivisionsPerQuarterNote() const { return _duration.getDivisionsPerQuarterNote(); }

const Duration& Note::getDuration() const { return _duration; }

float Note::getQuarterDuration() const { return _duration.getQuarterDuration(); }

bool Note::isGraceNote() const { return _isGraceNote; }

void Note::setIsNoteOn(bool isNoteOn) { _isNoteOn = isNoteOn; }

bool Note::isNoteOn() const { return _isNoteOn; }

bool Note::isNoteOff() const { return !_isNoteOn; }

std::string Note::getAlterSymbol() const { return _alterSymbol; }

void Note::setIsInChord(bool inChord) { _inChord = inChord; }

bool Note::isTransposed() const {
    return (_transposeDiatonic == 0 && _transposeChromatic == 0) ? false : true;
}

std::string Note::getEnharmonicPitch(const bool alternativeEnhamonicPitch) const {
    if (isNoteOff()) {
        return MUSIC_XML::PITCH::REST;
    }

    const std::string pitch = getPitch();
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    int octave = 0;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

    const int midiNumber = Helper::pitch2midiNote(pitch);
    const int ownAlter = static_cast<int>(alterValue);

    // Other spellings of the same MIDI number, indexed by 'alter + 2' (empty if unavailable)
    std::array<std::string, 5> spellings;
    for (int alter = -2; alter <= 2; alter++) {
        if (alter != ownAlter) {
            spellings[static_cast<size_t>(alter + 2)] = spellMidiNumber(midiNumber, alter);
        }
    }
    const auto spelling = [&spellings](const int alter) {
        return spellings[static_cast<size_t>(alter + 2)];
    };
    const auto firstSpelling = [&spelling](const int alterA, const int alterB) {
        return spelling(alterA).empty() ? spelling(alterB) : spelling(alterA);
    };

    std::string defaultPitch;
    std::string alternativePitch;

    const bool isWhiteKey =
        std::find(c_diatonicStepSemitones.begin(), c_diatonicStepSemitones.end(),
                  midiNumber % 12) != c_diatonicStepSemitones.end();
    if (isWhiteKey) {
        // Natural <-> flat-side spelling; the sharp-side spelling is the alternative
        const std::string flatSide = firstSpelling(-1, -2);
        const std::string sharpSide = firstSpelling(1, 2);
        if (ownAlter == 0) {
            defaultPitch = flatSide;
            alternativePitch = sharpSide;
        } else if (ownAlter < 0) {
            defaultPitch = spelling(0);
            alternativePitch = sharpSide;
        } else {
            defaultPitch = spelling(0);
            alternativePitch = flatSide;
        }
    } else {
        // Sharp <-> flat; double accidentals return the single accidental in the same direction
        const std::string doubleAccidental = firstSpelling(-2, 2);
        switch (ownAlter) {
            case 1:
                defaultPitch = spelling(-1);
                alternativePitch = doubleAccidental;
                break;
            case -1:
                defaultPitch = spelling(1);
                alternativePitch = doubleAccidental;
                break;
            case 2:
                defaultPitch = spelling(1);
                alternativePitch = spelling(-1);
                break;
            default:  // -2
                defaultPitch = spelling(-1);
                alternativePitch = spelling(1);
                break;
        }
    }

    // Range fallback: a missing default returns the own pitch; a missing alternative the default
    if (defaultPitch.empty()) {
        defaultPitch = pitch;
    }
    if (alternativePitch.empty()) {
        alternativePitch = defaultPitch;
    }

    return alternativeEnhamonicPitch ? alternativePitch : defaultPitch;
}

std::vector<std::string> Note::getEnharmonicPitches(const bool includeCurrentPitch) const {
    if (includeCurrentPitch) {
        return std::vector<std::string>(
            {getPitch(), getEnharmonicPitch(false), getEnharmonicPitch(true)});
    }

    return std::vector<std::string>({getEnharmonicPitch(false), getEnharmonicPitch(true)});
}

void Note::toEnharmonicPitch(const bool alternativeEnhamonicPitch) {
    setPitch(getEnharmonicPitch(alternativeEnhamonicPitch));
}

Note Note::getEnharmonicNote(const bool alternativeEnhamonicPitch) const {
    return Note(getEnharmonicPitch(alternativeEnhamonicPitch));
}

std::vector<Note> Note::getEnharmonicNotes(const bool includeCurrentPitch) const {
    if (includeCurrentPitch) {
        return {Note(getPitch()), Note(getEnharmonicPitch(false)), Note(getEnharmonicPitch(true))};
    }

    return {Note(getEnharmonicPitch(false)), Note(getEnharmonicPitch(true))};
}

int Note::getScaleDegree(const Key& key) const {
    // Skip rests
    if (isNoteOff()) {
        return 0;
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

    const std::string rootPitchStep = getPitchStep();

    const auto it = std::find(diatonicScale->begin(), diatonicScale->end(), rootPitchStep);
    const int index = std::distance(diatonicScale->begin(), it);
    const int degree = index + 1;

    return degree;
}

float Note::getFrequency(const float freqA4) const {
    return Helper::midiNote2freq(_midiNumber, freqA4);
}

std::pair<std::vector<float>, std::vector<float>> Note::getHarmonicSpectrum(
    const int numPartials,
    const std::function<std::vector<float>(std::vector<float>)> amplCallback,
    const float partialsDecayExpRate,
    const float freqA4) const {
    if (numPartials <= 0) {
        LOG_ERROR("The 'numPartials' must be a positive value");
    }

    std::vector<float> freqs(numPartials, 0);
    for (int i = 0; i < numPartials; i++) {
        freqs[i] = getFrequency(freqA4) * (i + 1);
    }

    std::vector<float> ampls(numPartials, 0);
    if (amplCallback == nullptr) {
        for (int i = 0; i < numPartials; i++) {
            ampls[i] = std::pow(partialsDecayExpRate, i);
        }
    } else {
        ampls = amplCallback(freqs);
    }

    if (freqs.size() != ampls.size()) {
        LOG_ERROR(
            "The output vector of 'amplCallback' function must have the size of 'numPartials'=" +
            std::to_string(numPartials));
    }

    return {freqs, ampls};
}

void Note::transpose(const int semitones, const std::string& accType) {
    const std::string newPitch = Helper::transposePitch(getPitch(), semitones, accType);
    setPitch(newPitch);
}

void Note::setPitch(const std::string& pitch) {
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

    std::string pitchClass;
    std::string pitchStep;
    int octave = 0;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, _alterSymbol);

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

void Note::setTransposingInterval(const int diatonicInterval, const int chromaticInterval) {
    // Error checking for rest
    if (!isNoteOn()) {
        return;
    }

    // Set internal Note members
    _transposeDiatonic = diatonicInterval;
    _transposeChromatic = chromaticInterval;

    // Update internal MIDI number member
    _midiNumber += _transposeChromatic;

    // ===== TRANSPOSE PITCH (STRING) ===== //

    // Check if this is a transposing instrument
    if (!isTransposed()) {
        return;
    }

    // Create musical scales
    const std::array<std::string, 12> sharpScale = {"C",  "C#", "D",  "D#", "E",  "F",
                                                    "F#", "G",  "G#", "A",  "A#", "B"};
    const std::array<std::string, 12> flatScale = {"C",  "Db", "D",  "Eb", "E",  "F",
                                                   "Gb", "G",  "Ab", "A",  "Bb", "B"};
    const std::array<std::string, 12> doubleSharpScale = {"C#", "Cx", "D#", "Dx", "E#", "F#",
                                                          "Fx", "G#", "Gx", "A#", "Ax", "B#"};
    const std::array<std::string, 12> doubleFlatScale = {"Cb",  "Dbb", "Db",  "Ebb", "Eb",  "Fb",
                                                         "Gbb", "Gb",  "Abb", "Ab",  "Bbb", "Bb"};

    // Try to find the written PitchClass in both scales
    const auto sharpNote = find(sharpScale.begin(), sharpScale.end(), _writtenPitchClass);
    const auto flatNote = find(flatScale.begin(), flatScale.end(), _writtenPitchClass);
    const auto doubleShapNote =
        find(doubleSharpScale.begin(), doubleSharpScale.end(), _writtenPitchClass);
    const auto doubleFlatNote =
        find(doubleFlatScale.begin(), doubleFlatScale.end(), _writtenPitchClass);

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
        LOG_ERROR("Unknown note type");
    }

    // Compute the temp 'fake' index (can be out of array bounds)
    const int tempIdx = writtenPitchClassIdx + _transposeChromatic;

    // Scale index to be setted in the future
    int soundingPitchClassIdx = 0;

    // Get the single octave chromatic scale index for sounding pitchClass
    if (tempIdx >= 0 && tempIdx <= 12) {  // Transpose inside one octave
        soundingPitchClassIdx = tempIdx % 12;
    } else if (tempIdx > 12) {  // Transpose outside one octave
        soundingPitchClassIdx = tempIdx % 12;
        _soundingOctave++;
    } else {  // Transpose down octave
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
        LOG_ERROR("Unknown note type");
    }
}

void Note::setVoice(const int voice) { _voice = voice; }

void Note::setStaff(const int staff) { _staff = staff; }

void Note::setStem(const std::string& stem) { _stem = stem; }

void Note::setIsTuplet(const bool isTuplet) { _isTuplet = isTuplet; }

void Note::setTupleValues(const int actualNotes, const int normalNotes,
                          const std::string& normalType) {
    _duration.setTupleValues(actualNotes, normalNotes, normalType);
}

bool Note::isTuplet() const { return _isTuplet; }

std::string Note::getStem() const { return _stem; }

void Note::setTieStart() {
    _tie.clear();
    _tie.push_back("start");
}

void Note::setTieStop() {
    _tie.clear();
    _tie.push_back("stop");
}

void Note::setTieStopStart() {
    _tie.clear();
    _tie.push_back("start");
    _tie.push_back("stop");
}

void Note::addTie(const std::string& tieType) { _tie.push_back(tieType); }

void Note::removeTies() { _tie.clear(); }

std::string Note::getWrittenPitchStep() const { return _writtenPitchClass.substr(0, 1); }

std::string Note::getSoundingPitchStep() const { return _soundingPitchClass.substr(0, 1); }

std::string Note::getPitchStep() const { return getSoundingPitchStep(); }

int Note::getVoice() const { return _voice; }

int Note::getStaff() const { return _staff; }

std::vector<std::string> Note::getTie() const { return _tie; }

std::pair<std::string, std::string> Note::getSlur() const { return _slur; }

void Note::addSlur(const std::string& slurType, const std::string& slurOrientation) {
    _slur.first = slurType;
    _slur.second = slurOrientation;
}

void Note::addArticulation(const std::string& articulation) {
    _articulation.push_back(articulation);
}

void Note::addBeam(const std::string& beam) { _beam.push_back(beam); }

std::vector<std::string> Note::getBeam() const { return _beam; }

std::vector<std::string> Note::getArticulation() const { return _articulation; }

const std::string Note::getSoundingPitchClass() const { return _soundingPitchClass; }

const std::string Note::getSoundingPitch() const {
    // Check transposing instrument
    if (!isTransposed()) {
        return getWrittenPitch();
    }

    return getSoundingPitchClass() + std::to_string(getSoundingOctave());
}

const std::string Note::getDiatonicWrittenPitchClass() const {
    return (_isNoteOn) ? getWrittenPitchClass().substr(0, 1) : MUSIC_XML::PITCH::REST;
}

const std::string Note::getDiatonicSoundingPitchClass() const {
    return (_isNoteOn) ? getSoundingPitchClass().substr(0, 1) : MUSIC_XML::PITCH::REST;
}

int Note::getSoundingOctave() const { return Helper::midiNote2octave(_midiNumber); }

const std::string Note::getWrittenPitchClass() const {
    return (_isNoteOn) ? _writtenPitchClass : MUSIC_XML::PITCH::REST;
}

const std::string Note::getWrittenPitch() const {
    return (_isNoteOn) ? _writtenPitchClass + std::to_string(_writtenOctave)
                       : MUSIC_XML::PITCH::REST;
}

int Note::getWrittenOctave() const { return _writtenOctave; }

std::string Note::getPitch() const { return getSoundingPitch(); }

bool Note::inChord() const { return _inChord; }

const std::string Note::toXML(const size_t instrumentId, const int identSize) const {
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
                Helper::generateIdentation(5, identSize) + "<step>" + _writtenPitchClass[0] +
                "</step>\n";

            if (_writtenPitchClass.size() > 1) {
                float alterValue = Helper::alterSymbol2Value(_alterSymbol);
                int x = static_cast<int>(alterValue);
                pitch.append(Helper::generateIdentation(5, identSize) + "<alter>" +
                             std::to_string(x) + "</alter>\n");
            }

            pitch.append(Helper::generateIdentation(5, identSize) + "<octave>" +
                         std::to_string(_writtenOctave) + "</octave>\n" +
                         Helper::generateIdentation(4, identSize) + "</pitch>\n");

            xml.append(pitch);
        } else {  // Unpitched notes
            std::string unpitched =
                std::string(Helper::generateIdentation(4, identSize) + "<unpitched>\n") +
                Helper::generateIdentation(5, identSize) + "<display-step>" +
                _writtenPitchClass[0] + "</display-step>\n";

            if (_writtenPitchClass.size() > 1) {
                float alterValue = Helper::alterSymbol2Value(_alterSymbol);
                int x = static_cast<int>(alterValue);
                unpitched.append(Helper::generateIdentation(5, identSize) + "<alter>" +
                                 std::to_string(x) + "</alter>\n");
            }

            unpitched.append(Helper::generateIdentation(5, identSize) + "<display-octave>" +
                             std::to_string(_writtenOctave) + "</display-octave>\n" +
                             Helper::generateIdentation(4, identSize) + "</unpitched>\n");

            xml.append(unpitched);
        }
    } else {
        xml.append(Helper::generateIdentation(4, identSize) + "<rest/>\n");
    }

    if (!isGraceNote()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<duration>" +
                   std::to_string(getDurationTicks()) + "</duration>\n");

        for (const auto& tie : _tie) {
            xml.append(Helper::generateIdentation(4, identSize) + "<tie type=\"" + tie + "\" />\n");
        }
    }

    if (_isNoteOn) {
        if (_unpitchedIndex == 0) {
            xml.append(Helper::generateIdentation(4, identSize) + "<instrument id=\"P" +
                       std::to_string(instrumentId + 1) + "-I" + std::to_string(1) + "\" />\n");
        } else {
            xml.append(Helper::generateIdentation(4, identSize) + "<instrument id=\"P" +
                       std::to_string(instrumentId + 1) + "-I" + std::to_string(_unpitchedIndex) +
                       "\" />\n");
        }
    }

    xml.append(Helper::generateIdentation(4, identSize) + "<voice>" + std::to_string(_voice) +
               "</voice>\n");

    if (_isGraceNote) {
        xml.append(Helper::generateIdentation(4, identSize) + "<type>16th</type>\n");
    } else {
        if (!_duration.getNoteType().empty()) {
            xml.append(Helper::generateIdentation(4, identSize) + "<type>" + getShortType() +
                       "</type>\n");
        } else {
            // xml.append(Helper::generateIdentation(4, identSize) + "<type>" +
            // Helper::ticks2noteType(_durationTicks, _divisionsPerQuarterNote)
            // + "</type>\n");
        }
    }

    if (_isTuplet) {
        xml.append(Helper::generateIdentation(4, identSize) + "<time-modification>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<actual-notes>" +
                   std::to_string(_duration.getTimeModificationActualNotes()) +
                   "</actual-notes>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<normal-notes>" +
                   std::to_string(_duration.getTimeModificationNormalNotes()) +
                   "</normal-notes>\n");
        xml.append(Helper::generateIdentation(5, identSize) + "<normal-type>" +
                   _duration.getTimeModificationNormalType() + "</normal-type>\n");
        xml.append(Helper::generateIdentation(4, identSize) + "</time-modification>\n");
    }

    for (int d = 0; d < _duration.getDots(); d++) {
        xml.append(Helper::generateIdentation(4, identSize) + "<dot />\n");
    }

    if (!_stem.empty()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<stem>" + _stem + "</stem>\n");
    }

    xml.append(Helper::generateIdentation(4, identSize) + "<staff>" + std::to_string(_staff + 1) +
               "</staff>\n");

    for (size_t b = 0; b < _beam.size(); b++) {
        xml.append(Helper::generateIdentation(4, identSize) + "<beam number=\"" +
                   std::to_string(b + 1) + "\">" + _beam[b] + "</beam>\n");
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
            xml.append(Helper::generateIdentation(5, identSize) + "<tied type=\"" + tie +
                       "\" />\n");
        }

        if (!_slur.first.empty()) {
            xml.append(Helper::generateIdentation(5, identSize) + "<slur type=\"" + _slur.first +
                       "\" orientation=\"" + _slur.second + "\" />\n");
        }

        xml.append(Helper::generateIdentation(4, identSize) + "</notations>\n");
    }

    xml.append(Helper::generateIdentation(3, identSize) + "</note>\n");

    return xml;
}

int Note::getMidiNumber() const { return _midiNumber; }


bool Note::operator<(const Note& otherNote) const {
        return (_midiNumber < otherNote.getMidiNumber());
    }

bool Note::operator>(const Note& otherNote) const {
    return (_midiNumber > otherNote.getMidiNumber());
}

bool Note::operator<=(const Note& otherNote) const {
    return (_midiNumber <= otherNote.getMidiNumber());
}

bool Note::operator>=(const Note& otherNote) const {
    return (_midiNumber >= otherNote.getMidiNumber());
}

bool Note::operator==(const Note& otherNote) const { return getPitch() == otherNote.getPitch(); }

bool Note::operator!=(const Note& otherNote) const { return getPitch() != otherNote.getPitch(); }