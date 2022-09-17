#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <ctype.h>
#include <cctype>
#include <string>

#include "constants.h"

struct timeModification_st {
    int actualNotes = 0;
    int normalNotes = 0;
    std::string normalType = "eighth";
};

typedef struct duration_st {
    Duration duration;
    int ticks = 0;
    std::string noteType;
    int divisionsPerQuarterNote = 0;
    int numDots = 0;
} DurationType;

class Note
{

private:
    std::string _writtenPitchClass;
    int _writtenOctave;

    std::string _soundingPitchClass;
    int _soundingOctave;

    DurationType _duration;
    bool _isNoteOn;
    bool _inChord;
    int _midiNumber;
    int _transposeDiatonic;
    int _transposeChromatic;
    int _voice;
    int _staff;
    bool _isGraceNote;
    std::string _stem;
    bool _isTuplet;
    bool _isPitched;
    int _unpitchedIndex;

    timeModification_st _timeModification;

    std::pair<std::string, std::string> _slur;

    std::string _alterSymbol;
    std::vector<std::string> _tie;

    std::vector<std::string> _articulation;
    std::vector<std::string> _beam;

public:
    Note();
    explicit Note(const std::string& pitch, const Duration duration = Duration::QUARTER, bool isNoteOn = true, bool inChord = false, const int transposeDiatonic = 0, const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);
    explicit Note(const int midiNumber, const std::string& accType = "", const Duration duration = Duration::QUARTER, bool isNoteOn = true, bool inChord = false, const int transposeDiatonic = 0, const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);

    ~Note();

    // ===== SETTERS ===== //
    /**
     * @brief Set the note pitch class
     * 
     * Examples: "C", "D", "E"
     * 
     * @param pitchClass Pitch class name
     */
    void setPitchClass(const std::string& pitchClass);

    /**
     * @brief Set the note octave
     * 
     * @param octave Octave number
     */
    void setOctave(int octave);

    /**
     * @brief Set the note duration
     * 
     * @param duration Duration as a Enum class element
     * @param divisionsPerQuarterNote Divisions per quarter note
     */
    void setDuration(const Duration duration, const int divisionsPerQuarterNote = 256);

    /**
     * @brief Set the note duration ticks
     * 
     * @param durationTicks Duration in ticks unit
     */
    void setDurationTicks(int durationTicks);

    /**
     * @brief Set the note ON or OFF
     * 
     * Note ON = Sounding Note \n
     * Note OFF = Rest Note (Silente)
     * 
     * @param isNoteOn 
     */
    void setIsNoteOn(bool isNoteOn);

    /**
     * @brief Set the note pitch name
     * 
     * Examples: "C4", "C#4", "Cx4", "Cb4", "Cbb4"
     * 
     * @param pitch Pitch name
     */
    void setPitch(const std::string& pitch);

    void setIsInChord(bool inChord);
    void setTransposingInterval(const int diatonicInterval, const int chromaticInterval);
    void setVoice(const int voice);
    void setStaff(const int staff);
    void setIsGraceNote(const bool isGraceNote = false);
    void setStem(const std::string& stem);
    void removeDots();
    void setSingleDot();
    void setDoubleDot();
    void setTieStart();
    void setTieStop();
    void setTieStopStart();
    void addTie(const std::string& tieType);
    void addSlur(const std::string& slurType, const std::string& slurOrientation);
    void addArticulation(const std::string& articulation);
    void addBeam(const std::string& beam);
    void setIsTuplet(const bool isTuplet = false);
    void setTupleValues(const int actualNotes, const int normalNotes, const std::string& normalType = "eighth");
    void setIsPitched(const bool isPitched = true);
    void setUnpitchedIndex(const int unpitchedIndex);

    void info() const;

    // ===== GETTERS ===== //
    const std::string getSoundingPitchClass() const;
    const std::string getSoundingPitch() const;

    const std::string getWrittenPitchClass() const;
    const std::string getWrittenPitch() const;

    const std::string getDiatonicWrittenPitchClass() const;
    const std::string getDiatonicSoundingPitchClass() const;

    int getSoundingOctave() const;
    int getWrittenOctave() const;

    std::string getPitchClass() const;
    std::string getPitchStep() const;
    int getOctave() const;

    std::string getType() const;
    std::string getLongType() const;
    std::string getShortType() const;
    int getDurationTicks() const;
    int getNumDots() const;
    bool isDotted() const;
    bool isDoubleDotted() const;
    int getDivisionsPerQuarterNote() const;
    float getDuration() const;
    float getQuarterDuration() const;
    
    bool isNoteOn() const;
    bool isNoteOff() const;
    std::string getPitch() const;
    int getMIDINumber() const;
    int getVoice() const;
    int getStaff() const;
    std::string getStem() const;
    
    std::vector<std::string> getTie() const;
    void removeTies();
    std::pair<std::string, std::string> getSlur() const;
    std::vector<std::string> getArticulation() const;
    std::vector<std::string> getBeam() const;
    bool isTuplet() const;
    bool isPitched() const;
    int getUnpitchedIndex() const;

    std::string getAlterSymbol() const;

    bool inChord() const;
    int getTransposeDiatonic() const;
    int getTransposeChromatic() const;
    bool isTransposed() const;
    bool isGraceNote() const;

    // Colocar na documentacao que caso use alternativeEnhamonicPitch
    // Ele vai tentar skipTwoDiatonicSteps exemplo D# -> Fbb OU
    // Ele tambem pode usar 1 diatonic step com dobrado bemol ou dobrado sustenido: C# -> Bx
    // Notas com pitchClass sem acidentes como 'C' serão enharmonizadas por default sempre diatonicamente pra cima: Dbb
    std::string getEnharmonicPitch(const bool alternativeEnhamonicPitch = false) const;
    std::vector<std::string> getEnharmonicPitches(const bool includeCurrentPitch = false) const;

    Note getEnharmonicNote(const bool alternativeEnhamonicPitch = false) const;
    std::vector<Note> getEnharmonicNotes(const bool includeCurrentPitch = false) const;

    // Colocar praticamente a mesma documentaçao do método 'getEnharmonicPitch()'
    void toEnharmonicPitch(const bool alternativeEnhamonicPitch = false);
    float getFrequency() const;

    void transpose(const int semitones, const std::string& accType = MUSIC_XML::ACCIDENT::NONE);

    const std::string toXML(const size_t instrumentId = 1, const int identSize = 2) const;

    bool operator < (const Note& otherNote) const {
        return (_midiNumber < otherNote.getMIDINumber());
    }

    bool operator > (const Note& otherNote) const {
        return (_midiNumber > otherNote.getMIDINumber());
    }

    bool operator <= (const Note& otherNote) const {
        return (_midiNumber <= otherNote.getMIDINumber());
    }

    bool operator >= (const Note& otherNote) const {
        return (_midiNumber >= otherNote.getMIDINumber());
    }

    bool operator == (const Note& otherNote) const {
        return getPitch() == otherNote.getPitch();
    }

    bool operator != (const Note& otherNote) const {
        return getPitch() != otherNote.getPitch();
    }
};
