#include "maiacore/duration.h"

#include "maiacore/helper.h"
#include "maiacore/log.h"

Duration::Duration(float quarterDuration, int divisionsPerQuarterNote)
    : _numDots(0),
      _ticks(0),
      _quarterDuration(quarterDuration),
      _divisionsPerQuarterNote(divisionsPerQuarterNote),
      _actualNotes(1),
      _normalNotes(1),
      _rhythmFigure(RhythmFigure::QUARTER) {
    _ticks = static_cast<int>(quarterDuration * static_cast<float>(divisionsPerQuarterNote));
    const auto& pair =
        Helper::ticks2rhythmFigure(_ticks, divisionsPerQuarterNote, _actualNotes, _normalNotes);

    _rhythmFigure = pair.first;
    _numDots = pair.second;

    _fractionDuration.setValues(_ticks, _divisionsPerQuarterNote);
}

Duration::Duration(int ticks, int divisionsPerQuarterNote, int actualNotes, int normalNotes)
    : _numDots(0),
      _ticks(ticks),
      _quarterDuration(0.0f),
      _divisionsPerQuarterNote(divisionsPerQuarterNote),
      _actualNotes(actualNotes),
      _normalNotes(normalNotes) {
    const auto& pair =
        Helper::ticks2rhythmFigure(ticks, divisionsPerQuarterNote, actualNotes, normalNotes);

    _rhythmFigure = pair.first;
    _numDots = pair.second;

    _quarterDuration = (float)ticks / (float)divisionsPerQuarterNote;
    const bool isTuplet = actualNotes != normalNotes;
    const bool isPerfectMultipleTuplet = (divisionsPerQuarterNote % actualNotes == 0);

    if (!isTuplet || isPerfectMultipleTuplet) {
        _fractionDuration.setValues(_ticks, _divisionsPerQuarterNote);
        return;
    }

    const int rawTicks = Helper::rhythmFigure2Ticks(_rhythmFigure, _divisionsPerQuarterNote);
    const int numerador = (rawTicks * _normalNotes) / _divisionsPerQuarterNote;
    _fractionDuration.setValues(numerador, _actualNotes);
}

void Duration::setQuarterDuration(float quarterDuration, int divisionsPerQuarterNote) {
    _ticks = static_cast<int>(quarterDuration * static_cast<float>(divisionsPerQuarterNote));
    const auto& pair =
        Helper::ticks2rhythmFigure(_ticks, divisionsPerQuarterNote, _actualNotes, _normalNotes);

    _rhythmFigure = pair.first;
    _numDots = pair.second;

    _fractionDuration.setValues(_ticks, _divisionsPerQuarterNote);
}

RhythmFigure Duration::getRhythmFigure() const { return _rhythmFigure; }

const std::string Duration::getNoteType() const { return Helper::toString(_rhythmFigure); }

int Duration::getDots() const { return _numDots; }

int Duration::getTicks() const { return _ticks; }

int Duration::getDivisionsPerQuarterNote() const { return _divisionsPerQuarterNote; }

const Fraction& Duration::getFractionDuration() const { return _fractionDuration; }

std::string Duration::getFractionDurationAsString() const { return _fractionDuration.toString(); }

float Duration::getQuarterDuration() const {
    return static_cast<float>(_ticks) / static_cast<float>(_divisionsPerQuarterNote);
}

bool Duration::isTuplet() const { return _actualNotes != _normalNotes; }

void Duration::setTupleValues(const int actualNotes, const int normalNotes,
                              const std::string& normalType) {
    _actualNotes = actualNotes;
    _normalNotes = normalNotes;
    _rhythmFigure = Helper::noteType2RhythmFigure(normalType);
}

int Duration::getTimeModificationActualNotes() const { return _actualNotes; }
int Duration::getTimeModificationNormalNotes() const { return _normalNotes; }
std::string Duration::getTimeModificationNormalType() const {
    return Helper::rhythmFigure2noteType(_rhythmFigure);
}
