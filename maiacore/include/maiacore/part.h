#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "maiacore/constants.h"

class Chord;
class Measure;
class Note;

/**
 * @brief Represents a musical part (instrument or voice) in a score, containing measures, staves,
 * and notes.
 *
 * The Part class provides methods for managing measures, staves, notes, and chords, as well as for
 * querying and modifying part-level metadata. It supports MusicXML serialization and is designed
 * for music analysis and computational musicology.
 */
class Part {
   private:
    int _partIndex;                   ///< Index of the part in the score.
    int _numStaves;                   ///< Number of staves in the part.
    int _divisionsPerQuarterNote;     ///< Divisions per quarter note for rhythmic precision.
    bool _isPitched;                  ///< True if the part is pitched (not percussion).
    int _staffLines;                  ///< Number of staff lines (default: 5).
    std::string _partName;            ///< Full name of the part.
    std::string _shortName;           ///< Short name/abbreviation of the part.
    std::vector<Measure> _measure;    ///< Vector of measures in the part.
    std::vector<int> _midiUnpitched;  ///< MIDI numbers for unpitched percussion instruments.

    /**
     * @brief Appends a single note to the part at a given position and staff.
     * @param note Note to append.
     * @param position Position in the measure (-1 for auto).
     * @param staveId Staff index (default: 0).
     */
    void appendNote(const Note& note, const int position = -1, const int staveId = 0);

    /**
     * @brief Appends multiple notes to the part at a given position and staff.
     * @param notes Vector of notes to append.
     * @param position Position in the measure (-1 for auto).
     * @param staveId Staff index (default: 0).
     */
    void appendNotes(const std::vector<Note>& notes, const int position = -1,
                     const int staveId = 0);

    /**
     * @brief Appends a chord to the part at a given position and staff.
     * @param chord Chord to append.
     * @param position Position in the measure (-1 for auto).
     * @param staveId Staff index (default: 0).
     */
    void appendChord(const Chord& chord, const int position = -1, const int staveId = 0);

    /**
     * @brief Appends multiple chords to the part at a given position and staff.
     * @param chords Vector of chords to append.
     * @param position Position in the measure (-1 for auto).
     * @param staveId Staff index (default: 0).
     */
    void appendChords(const std::vector<Chord>& chords, const int position = -1,
                      const int staveId = 0);

   public:
    /**
     * @brief Constructs a Part with a given name, number of staves, pitch type, and rhythmic
     * division.
     * @param partName Name of the part (instrument or voice).
     * @param numStaves Number of staves (default: 1).
     * @param isPitched True if pitched (default: true).
     * @param divisionsPerQuarterNote Divisions per quarter note (default: 256).
     */
    Part(const std::string& partName, const int numStaves = 1, const bool isPitched = true,
         const int divisionsPerQuarterNote = 256);

    /**
     * @brief Destructor.
     */
    ~Part();

    /**
     * @brief Removes all measures and resets the part.
     */
    void clear();

    /**
     * @brief Returns the index of the part in the score.
     * @return Part index.
     */
    int getPartIndex() const;

    /**
     * @brief Sets the index of the part in the score.
     * @param partIdx New part index.
     */
    void setPartIndex(int partIdx);

    /**
     * @brief Returns the full name of the part.
     * @return Part name string.
     */
    const std::string& getName() const;

    /**
     * @brief Returns the short name (abbreviation) of the part.
     * @return Short name string.
     */
    const std::string& getShortName() const;

    /**
     * @brief Adds a MIDI number for an unpitched percussion instrument.
     * @param midiUnpitched MIDI number to add.
     */
    void addMidiUnpitched(const int midiUnpitched);

    /**
     * @brief Appends a Note or Chord (variant) to the part at a given position and staff.
     * @param obj Note or Chord to append.
     * @param position Position in the measure (-1 for auto).
     * @param staveId Staff index (default: 0).
     */
    void append(const std::variant<Note, Chord>& obj, const int position = -1,
                const int staveId = 0);

    /**
     * @brief Appends multiple Note or Chord variants to the part at a given position and staff.
     * @param objs Vector of Note or Chord variants.
     * @param position Position in the measure (-1 for auto).
     * @param staveId Staff index (default: 0).
     */
    void append(const std::vector<std::variant<Note, Chord>>& objs, const int position = -1,
                const int staveId = 0);

    /**
     * @brief Adds a specified number of measures to the part.
     * @param numMeasures Number of measures to add.
     */
    void addMeasure(const int numMeasures);

    /**
     * @brief Removes a range of measures from the part.
     * @param measureStart Index of the first measure to remove.
     * @param measureEnd Index of the last measure to remove.
     */
    void removeMeasure(const int measureStart, const int measureEnd);

    /**
     * @brief Returns a reference to a measure by its index.
     * @param measureId Measure index.
     * @return Reference to the Measure object.
     */
    Measure& getMeasure(const int measureId);

    /**
     * @brief Returns a const reference to a measure by its index.
     * @param measureId Measure index.
     * @return Const reference to the Measure object.
     */
    const Measure& getMeasure(const int measureId) const;

    /**
     * @brief Returns a vector of all measures in the part.
     * @return Vector of Measure objects.
     */
    const std::vector<Measure> getMeasures() const;

    /**
     * @brief Returns the number of measures in the part.
     * @return Number of measures.
     */
    int getNumMeasures() const;

    /**
     * @brief Sets the number of staves for the part.
     * @param numStaves New number of staves.
     */
    void setNumStaves(const int numStaves);

    /**
     * @brief Sets whether the part is pitched (true) or unpitched (false).
     * @param isPitched True if pitched.
     */
    void setIsPitched(const bool isPitched = true);

    /**
     * @brief Sets the transposing interval and the octave doubling of the pitched notes of a
     *        range of measures and a staff.
     * @details Stamps every pitched note of the measures [measureStart, measureEnd) on the staff
     *          'staff' with Note::setTransposingInterval() and Note::setOctaveDoubling(); rests
     *          and unpitched notes are left alone. The interval is the total one: an octave
     *          transposition is folded in as 7 letters and 12 semitones per octave (a B-flat bass
     *          clarinet is (-8, -14)). The notes hold the transposition -- no Part, Measure or
     *          Score state records it -- and the MusicXML writer derives its `<transpose>`
     *          elements from them. Every note is checked before any changes, so the call changes
     *          all of them or none. An export -> import keeps the pair only when it is the
     *          conventional one for its chromatic interval (or the diminished-fifth tritone);
     *          another pair is read back corrected (same sound, conventional spelling), with a
     *          TRANSPOSE_PAIR_CORRECTED record in the import report (Score::getImportIssues()).
     * @param diatonicInterval Letters from the written to the sounding pitch (-1 for a B-flat
     *        clarinet).
     * @param chromaticInterval Semitones from the written to the sounding pitch (-2 for a
     *        B-flat clarinet).
     * @param measureStart Index of the first measure (default 0).
     * @param measureEnd Index one past the last measure; -1 (default) for the end of the part.
     * @param staff 0-based staff index; -1 (default) for every staff.
     * @param doubling The octave doubling (default OctaveDoubling::NONE).
     * @throws std::out_of_range If the measures are not a range of the part's measures, or the
     *         staff is neither -1 nor one of the part's staves.
     * @throws std::runtime_error If a note would have no sounding pitch with the interval: below
     *         C1b-1, or above B11 where its letter cannot spell it. The message names the first
     *         such note (measure index, staff index, written pitch); no note is changed.
     */
    void setTransposingInterval(const int diatonicInterval, const int chromaticInterval,
                                const int measureStart = 0, const int measureEnd = -1,
                                const int staff = -1,
                                const OctaveDoubling doubling = OctaveDoubling::NONE);

    /**
     * @brief Sets the number of staff lines for the part.
     * @param staffLines Number of staff lines (default: 5).
     */
    void setStaffLines(const int staffLines = 5);

    /**
     * @brief Adds additional staves to the part.
     * @param numStaves Number of staves to add (default: 1).
     */
    void addStaves(const int numStaves = 1);

    /**
     * @brief Removes a staff from the part by its index.
     * @param staveId Staff index to remove.
     */
    void removeStave(const int staveId);

    /**
     * @brief Returns the number of staves in the part.
     * @return Number of staves.
     */
    int getNumStaves() const;

    /**
     * @brief Returns the MIDI numbers for unpitched percussion instruments.
     * @return Vector of MIDI numbers.
     */
    std::vector<int> getMidiUnpitched() const;

    /**
     * @brief Returns true if the part is pitched (not percussion).
     * @return True if pitched.
     */
    bool isPitched() const;

    /**
     * @brief Returns the number of staff lines.
     * @return Number of staff lines.
     */
    int getStaffLines() const;

    /**
     * @brief Returns the number of notes in the part (optionally for a specific staff).
     * @param staveId Staff index (-1 for all staves).
     * @return Number of notes.
     */
    int getNumNotes(const int staveId = -1);

    /**
     * @brief Returns the number of sounding notes (note on) in the part (optionally for a specific
     * staff).
     * @param staveId Staff index (-1 for all staves).
     * @return Number of sounding notes.
     */
    int getNumNotesOn(const int staveId = -1);

    /**
     * @brief Returns the number of rest notes (note off) in the part (optionally for a specific
     * staff).
     * @param staveId Staff index (-1 for all staves).
     * @return Number of rest notes.
     */
    int getNumNotesOff(const int staveId = -1);

    /**
     * @brief Sets the short name (abbreviation) for the part.
     * @param shortName Short name string.
     */
    void setShortName(const std::string& shortName);

    /**
     * @brief Prints summary information about the part to the log.
     */
    void info() const;

    /**
     * @brief Serializes the part to MusicXML format.
     * @details The transpositions are written from the notes, which hold them: for each staff, a
     *          `<transpose>` in measure 1 when its first pitched note is transposed or doubled
     *          (it applies from the start of the part), and one wherever a pitched note's interval
     *          or doubling differs from that of the staff's previous pitched note -- in the
     *          measure's `<attributes>` when the note is the staff's first pitched note there,
     *          otherwise in an `<attributes>` written just before the first note of its chord.
     *          Rests and unpitched notes change nothing. A `<transpose>` has no `number` when
     *          every staff has the same transposition at that point; a change in the middle of a
     *          measure of a part with more than one staff always has one. The interval is
     *          unfolded into `<diatonic>`, `<chromatic>` and, from an octave on,
     *          `<octave-change>` (its whole octaves, rounded toward zero), with the diatonic
     *          interval the note is spelled with -- the conventional one when the stored one is 0
     *          -- and `<double/>` or `<double above="yes"/>` states the doubling. The elements go
     *          after `<clef>` and `<staff-details>`; an `<attributes>` is opened for them when
     *          nothing else needs one.
     * @param instrumentId Instrument index (default: 1).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the part.
     * @throws std::runtime_error If the notes of a chord have different transposing intervals or
     *         octave doublings, which one `<transpose>` cannot express; the message names the
     *         part, the measure and the staff.
     */
    const std::string toXML(const int instrumentId = 1, const int identSize = 2) const;

    /**
     * @brief Serializes the part to JSON format.
     * @return JSON string for the part.
     */
    std::string toJSON() const;
};
