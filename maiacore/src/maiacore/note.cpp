#include "maiacore/note.h"

#include <ctype.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <optional>
#include <string>

#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

namespace {
// True when a transposing instrument's sounding pitch falls below the lowest representable pitch,
// "C1b-1" (-0.5): its sounding MIDI number, the written one moved by the whole-semitone chromatic
// interval, is negative, which is the same "rounds, ties upward, to a negative MIDI number" rule
// every pitch in this library is held to.
bool isSoundingPitchBelowFloor(const Pitch& writtenPitch, const int transposeChromatic) {
    return !writtenPitch.isRest() && writtenPitch.getMidiNumber() + transposeChromatic < 0;
}

// The one error every sounding getter raises for such a note. The note is real -- isNoteOn() is
// true and its written pitch is intact -- but it has no sounding spelling, octave, MIDI number or
// frequency, and answering a rest's values for them would pass it off as a rest.
[[noreturn]] void throwSoundingPitchBelowFloor(const Pitch& writtenPitch,
                                               const int transposeDiatonic,
                                               const int transposeChromatic) {
    LOG_ERROR("The sounding pitch of the written pitch '" + writtenPitch.getPitch() +
              "' with transposeDiatonic=" + std::to_string(transposeDiatonic) +
              " and transposeChromatic=" + std::to_string(transposeChromatic) + " is at position " +
              std::to_string(writtenPitch.getQuarterToneSteps() +
                             static_cast<float>(transposeChromatic)) +
              ", below the lowest representable pitch C1b-1 (-0.5, MIDI note 0), so it has no "
              "sounding spelling, octave, MIDI number or frequency. The written pitch is still "
              "available from getWrittenPitch(); a transposing interval that keeps the sounding "
              "pitch at or above C1b-1 makes it spellable.");
}

// Formats a pitch alter value for the MusicXML <alter> element: a whole-tone accidental with no
// decimal part ("1", "-2"), a quarter tone with exactly one decimal place ("0.5", "-1.5"). The
// text is looked up by accidental symbol rather than formatted through a stream, so it never
// depends on the global C++ locale (a comma-decimal one would write "0,5"), and
// Helper::alterValue2symbol() throws, naming the value, for anything that is not one of the nine
// alters this library can spell.
std::string formatAlterValue(const float alterValue) {
    switch (hash(Helper::alterValue2symbol(alterValue).c_str())) {
        case hash("bb"):
            return "-2";
        case hash("3b"):
            return "-1.5";
        case hash("b"):
            return "-1";
        case hash("1b"):
            return "-0.5";
        case hash(""):
            return "0";
        case hash("1x"):
            return "0.5";
        case hash("#"):
            return "1";
        case hash("3x"):
            return "1.5";
        case hash("x"):
            return "2";
        default:
            break;
    }

    return {};  // unreachable: alterValue2symbol() answers one of the nine symbols above
}

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

void Note::roundToSemitone() { _writtenPitch.roundToSemitone(); }

void Note::setIsPitched(const bool isPitched) { _isPitched = isPitched; }

bool Note::isPitched() const { return _isPitched; }

std::string Note::getPitchClass() const { return getSoundingPitchClass(); }

void Note::setOctave(const int octave) {
    // Delegates to Pitch::setOctave() and inherits its policy: refuses on a rest (LOG_WARN, no
    // mutation) instead of writing a fabricated octave onto one.
    _writtenPitch.setOctave(octave);
}

std::optional<int> Note::getOctave() const {
    // The octave of the sounding spelling: empty for a rest, whose Pitch has no octave. A sounding
    // pitch below the lowest representable pitch throws from computeSoundingPitch().
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

bool Note::isQuarterTone() const { return isQuarterToneValue(_writtenPitch.getAlter()); }

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
    if (isQuarterToneValue(alterValue)) {
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

void Note::transpose(const float semitones, const std::string& accType) {
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

    // Eagerly derive (and discard) the sounding pitch, so one that cannot be spelled throws here
    // rather than from a later getter call. The exception is a sounding pitch below the lowest
    // representable pitch: such a note stays constructible, and each sounding getter reports the
    // condition when asked. Nothing is cached: every "sounding" getter recomputes the sounding
    // pitch on demand via computeSoundingPitch().
    if (!isSoundingPitchBelowFloor(_writtenPitch, _transposeChromatic)) {
        computeSoundingPitch();
    }
}

Pitch Note::computeSoundingPitch() const {
    if (_writtenPitch.isRest()) {
        return _writtenPitch;
    }

    // Not a transposing instrument: the written pitch IS the sounding pitch.
    if (!isTransposed()) {
        return _writtenPitch;
    }

    // Task 10: the sounding pitch is derived arithmetically from the written pitch and the
    // chromatic interval, exactly as getMidiNumber() has done since Task 6.
    //
    // This replaced four twelve-entry arrays of semitone spellings (sharp, flat, double-sharp and
    // double-flat), indexed by written pitch class. They were DELETED rather than repaired: with
    // twelve entries per octave there is no index for a quarter tone, so the structure could not
    // represent "C1x4" at all -- such a pitch class matched none of the four scales and fell
    // through to LOG_ERROR("Unknown note type"), a message naming neither the pitch class nor the
    // interval. Both of those throws went with the arrays; there is no longer a spelling this
    // method can fail to look up.
    //
    // The index arithmetic carried two further defects, both fixed by deleting it:
    //   1. A pitch class found only in the SHARP scale (e.g. "C#") still indexed the FLAT array,
    //      at end() -- an index of 12 on a 12-element array, which only stayed in bounds by
    //      accident -- so a B-flat clarinet's written C#4 sounded "Bb" instead of "B".
    //   2. The octave-wrap check `tempIdx >= 0 && tempIdx <= 12` also took the no-wrap branch at
    //      exactly 12, so a piccolo's written C4 (chromatic +12) sounded in octave 4, not 5.
    // A sounding pitch below the lowest representable pitch has no spelling. It is reported here,
    // rather than answered with Helper::steps2pitch()'s rest sentinel, so that no sounding getter
    // passes such a note off as a rest.
    if (isSoundingPitchBelowFloor(_writtenPitch, _transposeChromatic)) {
        throwSoundingPitchBelowFloor(_writtenPitch, _transposeDiatonic, _transposeChromatic);
    }

    const float soundingSteps =
        _writtenPitch.getQuarterToneSteps() + static_cast<float>(_transposeChromatic);

    // Accidental preference: flats going down, the default (natural/sharp-side) spelling going up.
    // That is the one sound convention the deleted lookup had, and it is what keeps a B-flat
    // clarinet's written C4 sounding "Bb3" rather than "A#3".
    const Pitch defaultSpelling(Helper::steps2pitch(soundingSteps, MUSIC_XML::ACCIDENT::NONE));

    if (_transposeChromatic > 0) {
        return defaultSpelling;
    }

    // Going down, the flat spelling is preferred -- but only when it agrees with the default about
    // the OCTAVE. getSoundingPitch() composes its string from this method's pitch CLASS and
    // getSoundingOctave()'s arithmetic octave, so a spelling that crosses the octave boundary
    // ("Cb4" rather than "B3" for MIDI 59) would compose "Cb3": a different note entirely, a full
    // octave off. Dropping the preference in that case is what keeps the two halves consistent.
    try {
        const Pitch flatSpelling(Helper::steps2pitch(soundingSteps, MUSIC_XML::ACCIDENT::FLAT));
        if (flatSpelling.getOctave() == defaultSpelling.getOctave()) {
            return flatSpelling;
        }
    } catch (const std::runtime_error& error) {
        // This MIDI note has no flat spelling at all (e.g. MIDI 60, a natural "C"). The default
        // spelling always exists, so the preference is simply dropped.
        ignore(error);
    }

    return defaultSpelling;
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
    // A rest has no sounding pitch: answer the rest sentinel before the transposition branch
    // below can compose anything. setIsNoteOn(false) turns a note into a rest but keeps its
    // transposing interval, so isTransposed() can be true for a rest.
    if (_writtenPitch.isRest()) {
        return MUSIC_XML::PITCH::REST;
    }

    if (!isTransposed()) {
        return getWrittenPitch();
    }

    // The pitch class and the octave come from the same sounding position: computeSoundingPitch()
    // only keeps a flat spelling whose octave agrees with getSoundingOctave(). A sounding pitch
    // below the lowest representable pitch throws from both, with the same error, so the
    // octave is always engaged here -- never a sentinel glued onto the pitch class.
    return getSoundingPitchClass() + std::to_string(getSoundingOctave().value());
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
    // Arithmetic, from the sounding MIDI number: empty for a rest (MIDI_REST), and a sounding
    // pitch below the lowest representable pitch throws from getMidiNumber().
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
                pitch.append(Helper::generateIdentation(5, identSize) + "<alter>" +
                             formatAlterValue(alterValue) + "</alter>\n");
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
                unpitched.append(Helper::generateIdentation(5, identSize) + "<alter>" +
                                 formatAlterValue(alterValue) + "</alter>\n");
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

    // <accidental> is reserved for accidentals a key signature cannot already imply -- i.e.
    // quarter tones, whose fractional alter has no key-signature analogue. A semitone
    // accidental (#, b, x, bb) may be implied by the key signature alone (e.g. an F# in D
    // major carries only <alter>1</alter>, no glyph), so emitting <accidental> for every
    // non-empty alter symbol would draw a redundant accidental on every such note. <alter> is
    // written for every accidental; <accidental> only for a quarter tone.
    //
    // Positioned right after <type> and before <time-modification>. Per the MusicXML schema
    // the note element sequence is `type?, dot*, accidental?, time-modification?, stem?`;
    // <dot> and <time-modification> are emitted in the wrong relative order below, so this is
    // the closest schema-correct position available without reordering the existing elements.
    if (isQuarterTone()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<accidental>" +
                   Helper::alterValue2Name(_writtenPitch.getAlter()) + "</accidental>\n");
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
    // Arithmetic, independent of any spelling: the written MIDI number moved by the chromatic
    // interval, a whole number of semitones.
    if (_writtenPitch.isRest()) {
        return MUSIC_XML::MIDI::NUMBER::MIDI_REST;
    }
    if (isSoundingPitchBelowFloor(_writtenPitch, _transposeChromatic)) {
        throwSoundingPitchBelowFloor(_writtenPitch, _transposeDiatonic, _transposeChromatic);
    }
    return _writtenPitch.getMidiNumber() + _transposeChromatic;
}

float Note::getQuarterToneSteps() const {
    if (_writtenPitch.isRest()) {
        return static_cast<float>(MUSIC_XML::MIDI::NUMBER::MIDI_REST);
    }
    if (isSoundingPitchBelowFloor(_writtenPitch, _transposeChromatic)) {
        throwSoundingPitchBelowFloor(_writtenPitch, _transposeDiatonic, _transposeChromatic);
    }

    // The transposing interval is a whole number of semitones, so the sounding position is the
    // written one moved by it, exactly.
    return _writtenPitch.getQuarterToneSteps() + static_cast<float>(_transposeChromatic);
}

// The four ordering operators compare exact sounding positions, getQuarterToneSteps(), rather
// than the rounded getMidiNumber(), which puts a quarter tone level with the semitone above it
// ("E1b4" and "E4" both round to 64). Every position is a multiple of 0.5, which float holds
// exactly, so these comparisons are exact; for notes without a quarter tone they order exactly
// as getMidiNumber() does.
//
// This is the single source of truth for pitch order in the library: Chord::sortNotes(),
// Chord::isSorted() and every internal std::sort over Notes go through it, as do Python's
// Note comparisons, which are bound directly to these operators.
//
// operator==/!= compare pitch strings instead, which distinguish "E1b4" from "E4" too.
bool Note::operator<(const Note& otherNote) const {
    return getQuarterToneSteps() < otherNote.getQuarterToneSteps();
}

bool Note::operator>(const Note& otherNote) const {
    return getQuarterToneSteps() > otherNote.getQuarterToneSteps();
}

bool Note::operator<=(const Note& otherNote) const {
    return getQuarterToneSteps() <= otherNote.getQuarterToneSteps();
}

bool Note::operator>=(const Note& otherNote) const {
    return getQuarterToneSteps() >= otherNote.getQuarterToneSteps();
}

bool Note::operator==(const Note& otherNote) const { return getPitch() == otherNote.getPitch(); }

bool Note::operator!=(const Note& otherNote) const { return getPitch() != otherNote.getPitch(); }