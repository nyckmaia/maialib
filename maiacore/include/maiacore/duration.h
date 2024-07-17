#pragma once

#include <string>

#include "maiacore/constants.h"
#include "maiacore/fraction.h"

class Duration {
   public:
    Duration(float quarterDuration, int divisionsPerQuarterNote = 256);
    // Duration(RhythmFigure rhythmFigure, int dots = 0);
    Duration(int ticks, int divisionsPerQuarterNote = 256, int actualNotes = 1,
             int normalNotes = 1);
    // Duration(const Fraction& fractionDuration, int divisionsPerQuarterNote = 256);

    void setQuarterDuration(float quarterDuration, int divisionsPerQuarterNote = 256);
    RhythmFigure getRhythmFigure() const;
    const std::string getNoteType() const;
    int getDots() const;
    int getTicks() const;
    int getDivisionsPerQuarterNote() const;
    const Fraction& getFractionDuration() const;
    std::string getFractionDurationAsString() const;
    float getQuarterDuration() const;
    bool isTuplet() const;
    void setTupleValues(const int actualNotes, const int normalNotes,
                        const std::string& normalType);
    int getTimeModificationActualNotes() const;
    int getTimeModificationNormalNotes() const;
    std::string getTimeModificationNormalType() const;

   private:
    int _numDots;
    int _ticks;
    float _quarterDuration;
    int _divisionsPerQuarterNote;
    int _actualNotes;
    int _normalNotes;
    RhythmFigure _rhythmFigure;
    Fraction _fractionDuration;
};