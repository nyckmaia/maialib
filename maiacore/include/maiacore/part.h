#pragma once

#include <iostream>
#include <variant>
#include <vector>

class Chord;
class Measure;
class Note;

class Part {
   private:
    std::vector<Measure> _measure;
    std::string _partName;
    std::string _shortName;
    size_t _numStaves;
    int _divisionsPerQuarterNote;
    bool _isPitched;
    int _staffLines;
    std::vector<int> _midiUnpitched;

    void appendNote(const Note& note, const int position = -1, const int staveId = 0);
    void appendNotes(const std::vector<Note>& notes, const int position = -1,
                     const int staveId = 0);

    void appendChord(const Chord& chord, const int position = -1, const int staveId = 0);
    void appendChords(const std::vector<Chord>& chords, const int position = -1,
                      const int staveId = 0);

   public:
    Part(const std::string& partName, const int numStaves = 1, const bool isPitched = true,
         const int divisionsPerQuarterNote = 256);

    ~Part();

    void clear();

    const std::string& getName() const;
    const std::string& getShortName() const;

    void addMidiUnpitched(const int midiUnpitched);

    void append(const std::variant<Note, Chord>& obj, const int position = -1,
                const int staveId = 0);
    void append(const std::vector<std::variant<Note, Chord>>& objs, const int position = -1,
                const int staveId = 0);

    void addMeasure(const int numMeasures);
    void removeMeasure(const int measureStart, const int measureEnd);
    Measure& getMeasure(const int measureId);
    const Measure& getMeasure(const int measureId) const;
    int getNumMeasures() const;

    void setNumStaves(const int numStaves);
    void setIsPitched(const bool isPitched = true);
    void setStaffLines(const int staffLines = 5);

    void addStaves(const int numStaves = 1);
    void removeStave(const int staveId);
    int getNumStaves() const;
    std::vector<int> getMidiUnpitched() const;

    bool isPitched() const;
    int getStaffLines() const;

    int getNumNotes(const int staveId = -1);
    int getNumNotesOn(const int staveId = -1);
    int getNumNotesOff(const int staveId = -1);

    void setShortName(const std::string& shortName);

    void info() const;

    const std::string toXML(const int instrumentId = 1, const int identSize = 2) const;
    std::string toJSON() const;
};
