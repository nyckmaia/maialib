#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <ctype.h>
#include <cctype>
#include <string>

#include "helper.h"

struct timeModification_st {
    int actualNotes = 0;
    int normalNotes = 0;
    std::string normalType = "eighth";
};

class Note
{

private:
    std::string _writtenPitchClass;
    int _writtenOctave;

    std::string _soundingPitchClass;
    int _soundingOctave;

    int _durationTicks;
    bool _isNoteOn;
    bool _inChord;
    int _midiNumber;
    int _transposeDiatonic;
    int _transposeChromatic;
    int _voice;
    int _staff;
    bool _isGraceNote;
    std::string _type;
    std::string _stem;
    int _numDots;
    bool _isTuplet;
    bool _isPitched;
    int _unpitchedIndex;

    timeModification_st _timeModification;

    std::pair<std::string, std::string> _slur;

    std::string _alterSymbol;
    std::vector<std::string> _tie;

    std::vector<std::string> _articulation;
    std::vector<std::string> _beam;

    int _divisionsPerQuarterNote;

public:

    /**
     * @brief Construct a new Note object
     * 
     * @param pitch Note Pitch
     * @param durationTicks Duration in Ticks
     * @param isNoteOn If is note = true | If is rest = false 
     * @param inChord Is this note inside of a chord?
     * @param transposeDiatonic Number of diatonic semitones of the transpose instrument
     * @param transposeChromatic Number of chromatic semitones of the transpose instrument
     * @param divisionsPerQuarterNote Number of divisions per quarter note (MusicXML protocol specification)
     */
    Note(const std::string& pitch, const size_t durationTicks = 256, bool isNoteOn = true, bool inChord = false, int transposeDiatonic = 0, int transposeChromatic = 0, int divisionsPerQuarterNote = 256);
    
    /**
     * @brief Construct a new Rest (Note object)
     * This is an alias to construct a rest
     * 
     * @param durationTicks Duration in Ticks
     * @param divisionsPerQuarterNote Number of divisions per quarter note (MusicXML protocol specification)
     */
    Note(const size_t durationTicks = 256, int divisionsPerQuarterNote = 256); // For rest

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
    void setType(const std::string& type);
    void setStem(const std::string& steam);
    void setDivisionsPerQuarterNote(const int divisionsPerQuarterNote = 256);
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

    void transposition(int semitonesNumber);

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
    int getDurationTicks() const;
    bool isNoteOn() const;
    bool isNoteOff() const;
    std::string getPitch() const;
    int getMIDINumber() const;
    int getVoice() const;
    int getStaff() const;
    std::string getType() const;
    std::string getStem() const;
    int getDivisionsPerQuarterNote() const;
    int getNumDots() const;
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


