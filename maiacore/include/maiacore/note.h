#pragma once

#include <functional>
#include <string>
#include <vector>

#include "maiacore/constants.h"
#include "maiacore/duration.h"
#include "maiacore/key.h"
#include "maiacore/time-signature.h"

class Note {
   private:
    std::string _writtenPitchClass;
    int _writtenOctave;

    std::string _soundingPitchClass;
    int _soundingOctave;

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
    Duration _duration;

    // timeModification_st _timeModification;

    std::pair<std::string, std::string> _slur;

    std::string _alterSymbol;
    std::vector<std::string> _tie;

    std::vector<std::string> _articulation;
    std::vector<std::string> _beam;

   public:
    Note();
    explicit Note(const std::string& pitch, const RhythmFigure rhythmFigure = RhythmFigure::QUARTER,
                  bool isNoteOn = true, bool inChord = false, const int transposeDiatonic = 0,
                  const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);
    explicit Note(const int midiNumber, const std::string& accType = "",
                  const RhythmFigure rhythmFigure = RhythmFigure::QUARTER, bool isNoteOn = true,
                  bool inChord = false, const int transposeDiatonic = 0,
                  const int transposeChromatic = 0, const int divisionsPerQuarterNote = 256);

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
     * @brief Set the Duration object
     *
     * @param duration
     */
    void setDuration(const Duration& duration);

    /**
     * @brief Set the Duration object
     *
     * @param quarterDuration
     * @param divisionsPerQuarterNote
     */
    void setDuration(const float quarterDuration, const int divisionsPerQuarterNote = 256);

    // /**
    //  * @brief Set the note duration
    //  *
    //  * @param rhythmFigure RhythmFigure as a Enum class element
    //  * @param divisionsPerQuarterNote Divisions per quarter note
    //  */
    // void setDuration(const RhythmFigure rhythmFigure, const int divisionsPerQuarterNote = 256);

    // /**
    //  * @brief Set the note duration
    //  *
    //  * @param durationValue Float-point duration between 0 and 1 using the relative lower time
    //  * signature value
    //  * @param lowerTimeSignatureValue Lower time signature value
    //  */
    // void setDuration(const float durationValue, const int lowerTimeSignatureValue = 4,
    //                  const int divisionsPerQuarterNote = 256);

    // /**
    //  * @brief Set the note duration ticks
    //  *
    //  * @param durationTicks Duration in ticks unit
    //  */
    // void setDurationTicks(int durationTicks);

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
    // void removeDots();
    // void setSingleDot();
    // void setDoubleDot();
    void setTieStart();
    void setTieStop();
    void setTieStopStart();
    void addTie(const std::string& tieType);
    void addSlur(const std::string& slurType, const std::string& slurOrientation);
    void addArticulation(const std::string& articulation);
    void addBeam(const std::string& beam);
    void setIsTuplet(const bool isTuplet = false);
    void setTupleValues(const int actualNotes, const int normalNotes,
                        const std::string& normalType = "eighth");
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
    int getOctave() const;

    std::string getPitchClass() const;

    std::string getWrittenPitchStep() const;
    std::string getSoundingPitchStep() const;
    std::string getPitchStep() const;

    std::string getType() const;
    std::string getLongType() const;
    std::string getShortType() const;
    int getDurationTicks() const;
    int getNumDots() const;
    bool isDotted() const;
    bool isDoubleDotted() const;
    int getDivisionsPerQuarterNote() const;
    const Duration& getDuration() const;
    float getQuarterDuration() const;

    bool isNoteOn() const;
    bool isNoteOff() const;
    std::string getPitch() const;
    int getMidiNumber() const;
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
    // Ele tambem pode usar 1 diatonic step com dobrado bemol ou dobrado
    // sustenido: C# -> Bx Notas com pitchClass sem acidentes como 'C' serão
    // enharmonizadas por default sempre diatonicamente pra cima: Dbb
    std::string getEnharmonicPitch(const bool alternativeEnhamonicPitch = false) const;
    std::vector<std::string> getEnharmonicPitches(const bool includeCurrentPitch = false) const;

    Note getEnharmonicNote(const bool alternativeEnhamonicPitch = false) const;
    std::vector<Note> getEnharmonicNotes(const bool includeCurrentPitch = false) const;

    /**
     * @brief Get the musical scale degree of the note
     *
     * @param key The key signature reference to calculate the musical scale
     * @return The scale degree as an integer
     */
    int getScaleDegree(const Key& key) const;

    /**
     * @brief
     *
     * @param alternativeEnhamonicPitch
     */
    void toEnharmonicPitch(const bool alternativeEnhamonicPitch = false);

    /**
     * @brief Get the Frequency object
     *
     * @param equalTemperament
     * @param freqA4
     * @return float
     */
    float getFrequency(const bool equalTemperament = true, const float freqA4 = 440.0f) const;

    /**
     * @brief Get the Harmonic Spectrum object
     *
     * @param numPartials
     * @param amplCallback
     * @return std::pair<std::vector<float>, std::vector<float>>
     */
    std::pair<std::vector<float>, std::vector<float>> getHarmonicSpectrum(
        const int numPartials = 6,
        const std::function<std::vector<float>(std::vector<float>)> amplCallback = nullptr) const;

    void transpose(const int semitones, const std::string& accType = MUSIC_XML::ACCIDENT::NONE);

    const std::string toXML(const size_t instrumentId = 1, const int identSize = 2) const;

    bool operator<(const Note& otherNote) const {
        return (_midiNumber < otherNote.getMidiNumber());
    }

    bool operator>(const Note& otherNote) const {
        return (_midiNumber > otherNote.getMidiNumber());
    }

    bool operator<=(const Note& otherNote) const {
        return (_midiNumber <= otherNote.getMidiNumber());
    }

    bool operator>=(const Note& otherNote) const {
        return (_midiNumber >= otherNote.getMidiNumber());
    }

    bool operator==(const Note& otherNote) const { return getPitch() == otherNote.getPitch(); }

    bool operator!=(const Note& otherNote) const { return getPitch() != otherNote.getPitch(); }
};
