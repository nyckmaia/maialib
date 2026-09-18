#include "maiacore/note.h"

#include <ctype.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
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
    const auto stepIt =
        std::find(c_diatonicStepSemitones.begin(), c_diatonicStepSemitones.end(), pitchClassIdx);
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
    : _writtenPitch(MUSIC_XML::PITCH::REST),
      _inChord(false),
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

    // The rest case already returned above, so this cannot take Pitch::setPitch()'s own rest
    // branch: _writtenPitch ends up a sounding note.
    _writtenPitch.setPitch(pitch);
    _inChord = inChord;
    // _duration.divisionsPerQuarterNote = divisionsPerQuarterNote;
    // setDuration(duration, divisionsPerQuarterNote);

    // Store the transposing interval (isNoteOn() is true here, so this always applies).
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
    LOG_INFO("Is note on: " << std::boolalpha << isNoteOn());
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
    // Delegates to Pitch::setPitchClass() and inherits its policy: replaces step + alter,
    // keeping the current octave (or defaulting it to 4 if this note was a rest). The MIDI
    // number is no longer a stored field, so it is automatically correct on the next
    // getMidiNumber() call -- closing the stale-MIDI defect this used to carry (Step 1's test).
    _writtenPitch.setPitchClass(pitchClass);
}

void Note::setStep(const std::string& step) { _writtenPitch.setStep(step); }

void Note::setAlter(const float alter) { _writtenPitch.setAlter(alter); }

void Note::setIsPitched(const bool isPitched) { _isPitched = isPitched; }

bool Note::isPitched() const { return _isPitched; }

std::string Note::getPitchClass() const { return getSoundingPitchClass(); }

void Note::setOctave(const int octave) {
    // Delegates to Pitch::setOctave() and inherits its policy: refuses on a rest (LOG_WARN, no
    // mutation) instead of writing a fabricated octave onto one.
    _writtenPitch.setOctave(octave);
}

std::optional<int> Note::getOctave() const {
    // Fix round 2: NOT an alias for getSoundingOctave() any more (that method is now arithmetic
    // -- see its own comment). This is "the octave" (documented as the sounding octave) and,
    // pre-Task-6, its body was `return _soundingOctave;`, a field tracked by the same
    // sharp/flat/double-sharp/double-flat scale-lookup computeSoundingPitch() below reproduces
    // verbatim; d26aa67's getOctave() and getSoundingOctave() were never the same value for a
    // buggy transposition (measured: swept 1197 (pitch, transposeDiatonic, transposeChromatic)
    // combinations against d26aa67 -- getOctave() matched in every single one; only
    // getMidiNumber(), and the octave DIGIT embedded in getSoundingPitch()'s string, regressed).
    // Task 6b: the `-2` sentinel is gone -- a rest now answers an empty optional instead of a
    // fabricated numeric octave. computeSoundingPitch().getOctave() is already an empty optional
    // for a rest, so this is a pure propagation; the non-rest scale-lookup behaviour above
    // (including its known defect, owned by Task 10) is untouched.
    return computeSoundingPitch().getOctave();
}

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

void Note::setIsNoteOn(bool isNoteOn) {
    if (isNoteOn) {
        // Turning a rest into a sounding note needs a pitch, which this call does not carry.
        // Boundary condition, not a caller error (spec 4.2 precedent: Pitch("rest").setAlter()):
        // refuse with a warning and leave the rest exactly as it was; this must NOT throw.
        if (_writtenPitch.isRest()) {
            LOG_WARN(
                "Cannot turn a rest into a sounding note without a pitch; ignoring "
                "setIsNoteOn(true). Use setPitch()/setPitchClass()/setStep() instead.");
        }
        // Already sounding: no-op either way.
        return;
    }

    // isNoteOn(false): make this note a rest. Same outcome as the constructor's
    // isNoteOn=false branch; every getter (all of which now read _writtenPitch.isRest()) is
    // consistent the moment this line runs.
    _writtenPitch.setPitch(MUSIC_XML::PITCH::REST);
}

bool Note::isNoteOn() const { return !_writtenPitch.isRest(); }

bool Note::isNoteOff() const { return _writtenPitch.isRest(); }

std::string Note::getAlterSymbol() const { return computeSoundingPitch().getAlterSymbol(); }

void Note::setIsInChord(bool inChord) { _inChord = inChord; }

bool Note::isTransposed() const {
    return (_transposeDiatonic == 0 && _transposeChromatic == 0) ? false : true;
}

std::string Note::getEnharmonicPitch(const bool alternativeEnharmonicPitch) const {
    if (isNoteOff()) {
        return MUSIC_XML::PITCH::REST;
    }

    // Fix round 3 (N1): deliberately NOT computeSoundingPitch().getMidiNumber() -- that is the
    // buggy scale-lookup-tracked octave (the same one getOctave() intentionally still
    // reproduces), and routing this method's own MIDI number through it reintroduced exactly
    // the coupling fix round 2 removed from getMidiNumber() itself, one call further out: HEAD
    // contradicted itself (getPitch() and getEnharmonicPitch() disagreeing on the octave of the
    // same note; measured for the Piccolo case: getPitch()=="C5" vs
    // getEnharmonicPitch(false)=="Dbb4"). getPitch() itself is correct again since fix round 2
    // (its octave digit is arithmetic; only the pitch CLASS is the pre-existing, still-broken
    // spelling), so re-deriving through it here -- exactly as d26aa67 did -- restores agreement
    // without touching the scale lookup or the class it produces.
    const std::string pitch = getPitch();
    std::string pitchClass;
    std::string pitchStep;
    std::string alterSymbol;
    std::optional<int> octave;
    float alterValue = 0.0f;
    Helper::splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol);

    const int midiNumber = Helper::pitch2midiNote(pitch);
    // spellMidiNumber() below only enumerates the five integer-semitone accidentals
    // ("bb","b","","#","x"); a quarter-tone alter has no spelling in that vocabulary.
    if (alterValue != std::floor(alterValue)) {
        LOG_ERROR("Quarter-tone enharmonic spelling is not supported for pitch: " + pitch);
    }
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

    return alternativeEnharmonicPitch ? alternativePitch : defaultPitch;
}

std::vector<std::string> Note::getEnharmonicPitches(const bool includeCurrentPitch) const {
    if (includeCurrentPitch) {
        return std::vector<std::string>(
            {getPitch(), getEnharmonicPitch(false), getEnharmonicPitch(true)});
    }

    return std::vector<std::string>({getEnharmonicPitch(false), getEnharmonicPitch(true)});
}

void Note::toEnharmonicPitch(const bool alternativeEnharmonicPitch) {
    setPitch(getEnharmonicPitch(alternativeEnharmonicPitch));
}

Note Note::getEnharmonicNote(const bool alternativeEnharmonicPitch) const {
    return Note(getEnharmonicPitch(alternativeEnharmonicPitch));
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
    return Helper::midiNote2freq(getMidiNumber(), freqA4);
}

std::pair<std::vector<float>, std::vector<float>> Note::getHarmonicSpectrum(
    const int numPartials, const std::function<std::vector<float>(std::vector<float>)> amplCallback,
    const float partialsDecayExpRate, const float freqA4) const {
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
        _writtenPitch.setPitch(MUSIC_XML::PITCH::REST);
        _inChord = false;
        _transposeDiatonic = 0;
        _transposeChromatic = 0;
        _isGraceNote = false;
        return;
    }

    // The rest case already returned above, so this cannot take Pitch::setPitch()'s own rest
    // branch; see the constructor's identical comment.
    _writtenPitch.setPitch(pitch);

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

    // Eagerly derive (and discard) the sounding pitch so an unspellable written pitch class
    // under a nonzero transpose throws here, matching this method's historical timing, rather
    // than from a later getter call. Nothing is cached: getSoundingPitch() and every other
    // "sounding" getter recompute this on demand via computeSoundingPitch().
    computeSoundingPitch();
}

Pitch Note::computeSoundingPitch() const {
    if (_writtenPitch.isRest()) {
        return _writtenPitch;
    }

    // ===== TRANSPOSE PITCH (STRING) ===== //

    // Check if this is a transposing instrument
    if (!isTransposed()) {
        return _writtenPitch;
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

    const std::string writtenPitchClass = _writtenPitch.getPitchClass();

    // Try to find the written PitchClass in both scales
    const auto sharpNote = find(sharpScale.begin(), sharpScale.end(), writtenPitchClass);
    const auto flatNote = find(flatScale.begin(), flatScale.end(), writtenPitchClass);
    const auto doubleShapNote =
        find(doubleSharpScale.begin(), doubleSharpScale.end(), writtenPitchClass);
    const auto doubleFlatNote =
        find(doubleFlatScale.begin(), doubleFlatScale.end(), writtenPitchClass);

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

    // Not a rest on this path (guarded above), so the written octave is always populated.
    int soundingOctave = _writtenPitch.getOctave().value();

    // Get the single octave chromatic scale index for sounding pitchClass
    if (tempIdx >= 0 && tempIdx <= 12) {  // Transpose inside one octave
        soundingPitchClassIdx = tempIdx % 12;
    } else if (tempIdx > 12) {  // Transpose outside one octave
        soundingPitchClassIdx = tempIdx % 12;
        soundingOctave++;
    } else {  // Transpose down octave
        soundingPitchClassIdx = tempIdx + 12;
        soundingOctave--;
    }

    // Get the correct sounding pitch class
    std::string soundingPitchClass;
    if (useSharpScale) {
        soundingPitchClass = sharpScale[soundingPitchClassIdx];
    } else if (useFlatScale) {
        soundingPitchClass = flatScale[soundingPitchClassIdx];
    } else if (useDoubleSharpScale) {
        soundingPitchClass = doubleSharpScale[soundingPitchClassIdx];
    } else if (useDoubleFlatScale) {
        soundingPitchClass = doubleFlatScale[soundingPitchClassIdx];
    } else {
        LOG_ERROR("Unknown note type");
    }

    return Pitch(soundingPitchClass + std::to_string(soundingOctave));
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

std::string Note::getWrittenPitchStep() const { return _writtenPitch.getPitchStep(); }

std::string Note::getSoundingPitchStep() const { return computeSoundingPitch().getPitchStep(); }

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

const std::string Note::getSoundingPitchClass() const {
    return computeSoundingPitch().getPitchClass();
}

const std::string Note::getSoundingPitch() const {
    // Fix round 5 (F1): a rest has no sounding pitch, so answer the rest sentinel BEFORE the
    // transposition branch below can concatenate anything. setIsNoteOn(false) turns a note into
    // a rest but deliberately does NOT clear its transposing intervals, so isTransposed() stays
    // true; without this guard that combination fell through to the concatenation below and
    // produced "rest" + "-2" == the malformed string "rest-2", which is not a valid pitch and
    // which no Note constructor would accept back. Measured against a d26aa67 worktree before
    // fixing: the baseline produced no malformed pitch string anywhere (0 occurrences across an
    // 864-cell probe, against 16 at HEAD), so this was a Task 6 regression, not pre-existing.
    // The guard is placed here, not at the concatenation site, so every route into this method
    // is covered at once.
    if (_writtenPitch.isRest()) {
        return MUSIC_XML::PITCH::REST;
    }

    // Fix round 2: restored to the pre-Task-6 body exactly (was
    // `computeSoundingPitch().getPitch()`, which glued the pitch CLASS to the SAME buggy
    // scale-lookup octave getOctave() intentionally still reproduces -- see that method's
    // comment). The pitch class here is still the pre-existing, unfixed lookup (Task 10's to
    // fix, not this round's); only the octave digit is arithmetic again, matching d26aa67.
    if (!isTransposed()) {
        return getWrittenPitch();
    }

    // Fix round 1 (Task 6b): NOT rest-guarded alone. getSoundingOctave() is arithmetic
    // (written MIDI + transposeChromatic) and is empty whenever that sum is negative, which
    // happens for an ordinary, constructible, non-rest transposed note whose sounding pitch
    // falls below the system minimum C-1 (MIDI 0) -- e.g. a B-flat clarinet's written "C#-1"
    // (transposeDiatonic=-1, transposeChromatic=-2) sounds MIDI -1. Calling .value() on that
    // unconditionally let a raw std::bad_optional_access escape this public getter (measured:
    // 162/4434 constructible non-rest transposed notes swept by the reviewer; corroborated
    // independently here). getSoundingPitchClass() does NOT fail alongside it: it derives its
    // answer from computeSoundingPitch()'s separate, pre-existing scale-lookup defect (Task
    // 10's to fix, untouched here), which can land back in-range by coincidence of its own
    // (unrelated) bug and so "succeeds" with a pitch class even when the arithmetic sounding
    // MIDI is unrepresentable. The two were never meant to agree (see getOctave()'s comment);
    // concatenating them when the arithmetic side is empty was never sound, sentinel or not.
    // Do NOT resurrect the pre-6b `-2` sentinel here (e.g. via value_or(-2)) -- octave -2 does
    // not exist in this library and round 5 of Task 6 was spent removing exactly that kind of
    // malformed pitch string. Fail loudly and diagnosably instead.
    const std::optional<int> soundingOctave = getSoundingOctave();
    if (!soundingOctave.has_value()) {
        LOG_ERROR(
            "Note::getSoundingPitch: this note's sounding pitch falls below the representable "
            "minimum C-1 (MIDI 0), so it has no sounding octave or sounding pitch string. "
            "Written pitch: '" +
            getWrittenPitch() + "', transposeDiatonic=" + std::to_string(_transposeDiatonic) +
            ", transposeChromatic=" + std::to_string(_transposeChromatic) +
            ", sounding MIDI=" + std::to_string(getMidiNumber()));
    }
    return getSoundingPitchClass() + std::to_string(soundingOctave.value());
}

const std::string Note::getDiatonicWrittenPitchClass() const {
    if (_writtenPitch.isRest()) {
        return MUSIC_XML::PITCH::REST;
    }
    return getWrittenPitchClass().substr(0, 1);
}

const std::string Note::getDiatonicSoundingPitchClass() const {
    if (_writtenPitch.isRest()) {
        return MUSIC_XML::PITCH::REST;
    }
    return getSoundingPitchClass().substr(0, 1);
}

std::optional<int> Note::getSoundingOctave() const {
    // Fix round 2: arithmetic, derived from the (now again arithmetic) getMidiNumber() -- the
    // pre-Task-6 body was `Helper::midiNote2octave(_midiNumber).value_or(-2)`; _midiNumber was
    // itself always arithmetic, so this is that same formula through the new single source of
    // truth. Deliberately NOT computeSoundingPitch().getOctave(): that tracks octave through the
    // same pre-existing, unfixed scale-lookup defect getOctave() below still (correctly, by
    // design) reproduces, and this method must not inherit it.
    // Task 6b: Helper::midiNote2octave() already returns an empty optional for MIDI_REST, so the
    // `-2` sentinel that used to replace it here is simply gone; nothing else changes.
    return Helper::midiNote2octave(getMidiNumber());
}

const std::string Note::getWrittenPitchClass() const { return _writtenPitch.getPitchClass(); }

const std::string Note::getWrittenPitch() const { return _writtenPitch.getPitch(); }

// Task 6b: propagates _writtenPitch's own optional instead of collapsing a rest to the `-2`
// sentinel.
std::optional<int> Note::getWrittenOctave() const { return _writtenPitch.getOctave(); }

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

    if (isNoteOn()) {
        if (_isPitched) {
            std::string pitch =
                std::string(Helper::generateIdentation(4, identSize) + "<pitch>\n") +
                Helper::generateIdentation(5, identSize) + "<step>" +
                _writtenPitch.getPitchStep() + "</step>\n";

            if (!_writtenPitch.getAlterSymbol().empty()) {
                const float alterValue = _writtenPitch.getAlter();
                const int x = static_cast<int>(alterValue);
                pitch.append(Helper::generateIdentation(5, identSize) + "<alter>" +
                             std::to_string(x) + "</alter>\n");
            }

            pitch.append(Helper::generateIdentation(5, identSize) + "<octave>" +
                         std::to_string(_writtenPitch.getOctave().value_or(0)) + "</octave>\n" +
                         Helper::generateIdentation(4, identSize) + "</pitch>\n");

            xml.append(pitch);
        } else {  // Unpitched notes
            std::string unpitched =
                std::string(Helper::generateIdentation(4, identSize) + "<unpitched>\n") +
                Helper::generateIdentation(5, identSize) + "<display-step>" +
                _writtenPitch.getPitchStep() + "</display-step>\n";

            if (!_writtenPitch.getAlterSymbol().empty()) {
                const float alterValue = _writtenPitch.getAlter();
                const int x = static_cast<int>(alterValue);
                unpitched.append(Helper::generateIdentation(5, identSize) + "<alter>" +
                                 std::to_string(x) + "</alter>\n");
            }

            unpitched.append(Helper::generateIdentation(5, identSize) + "<display-octave>" +
                             std::to_string(_writtenPitch.getOctave().value_or(0)) +
                             "</display-octave>\n" + Helper::generateIdentation(4, identSize) +
                             "</unpitched>\n");

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

    if (isNoteOn()) {
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

int Note::getMidiNumber() const {
    // Fix round 2: arithmetic, decoupled from computeSoundingPitch()'s spelling lookup on
    // purpose. That lookup (untouched, pre-existing) can pick the wrong pitch class for a
    // transposed note; routing MIDI through the resulting (mis-spelled) Pitch string used to
    // let that spelling defect corrupt the numeric answer too -- a Critical regression found in
    // review. Written MIDI + the chromatic transpose interval is correct regardless of spelling
    // and matches this method's pre-Task-6 behaviour exactly.
    if (_writtenPitch.isRest()) {
        return MUSIC_XML::MIDI::NUMBER::MIDI_REST;
    }
    return _writtenPitch.getMidiNumber() + _transposeChromatic;
}

bool Note::operator<(const Note& otherNote) const {
    return (getMidiNumber() < otherNote.getMidiNumber());
}

bool Note::operator>(const Note& otherNote) const {
    return (getMidiNumber() > otherNote.getMidiNumber());
}

bool Note::operator<=(const Note& otherNote) const {
    return (getMidiNumber() <= otherNote.getMidiNumber());
}

bool Note::operator>=(const Note& otherNote) const {
    return (getMidiNumber() >= otherNote.getMidiNumber());
}

bool Note::operator==(const Note& otherNote) const { return getPitch() == otherNote.getPitch(); }

bool Note::operator!=(const Note& otherNote) const { return getPitch() != otherNote.getPitch(); }