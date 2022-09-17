#pragma once

#include <string>
#include <vector>

#include "note.h"

class Interval
{

private:
    std::vector<Note> _note;
    int _numSemitones;

    std::pair<std::string, bool> analyse() const;

public:
    Interval(const std::string& pitch_A, const std::string& pitch_B);
    Interval(const Note& note_A, const Note& note_B);

    void setNotes(const std::string& pitch_A, const std::string& pitch_B);
    void setNotes(const Note& note_A, const Note& note_B);

    std::string getName() const;
    int getValue(const bool absoluteValue = false) const;
    int getNumSemitones(const bool absoluteValue = false) const;
    int getNumOctaves(const bool absoluteValue = false) const;
    int getDiatonicInterval(const bool absoluteValue = false) const;
    int getDiatonicSteps(const bool useSingleOctave = true, const bool absoluteValue = false) const;
    int getPitchStepInterval() const;
    std::vector<Note> getNotes() const;
    bool isAscendant() const;
    bool isDescendant() const;
    bool isTonal() const;
};
