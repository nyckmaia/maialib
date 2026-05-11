# maialib API cheatsheet

> Paste this into your AI assistant as context. Auto-generated from `.pyi` stubs by `scripts/build-cheatsheet.py`. Do not edit method signatures here — edit the C++ source or stubs.

## Import

```python
import maialib as ml
```

## Loading scores

```python
score = ml.Score('path/to/file.musicxml')
# or use a bundled sample:
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
```

## API surface

### `maiacore (C++ bindings)`

#### Class `Barline`
- `__init__() -> None`
- `clean() -> None`
- `getBarStyle() -> str`
- `getDirection() -> str`
- `getLocation() -> str`
- `setBarStyle(barStyle: str) -> None`
- `setDirection(direction: str) -> None`
- `setLocation(location: str) -> None`
- `setRepeatEnd() -> None`
- `setRepeatStart() -> None`
- `toXML(identSize: int = 2) -> str`

#### Class `Chord`
- `__init__() -> None`  *(overloaded)*
- `addNote(note: Note) -> None`  *(overloaded)*
- `clear() -> None`
- `getBassNote() -> Note`
- `getCloseChord(enharmonyNotes: bool = False) -> Chord`
- `getCloseStackChord(enharmonyNotes: bool = False) -> Chord`
- `getCloseStackHarmonicComplexity(useEnharmony: bool = False) -> float`
- `getCloseStackIntervals(firstNoteAsReference: bool = False) -> list[Interval]`
- `getDegree(key: Key, enharmonyNotes: bool = False) -> int`
- `getDuration() -> str`
- `getDurationTicks() -> int`
- `getFrequencyStd(freqA4: float = 440.0) -> float`
- `getHarmonicDensity(lowerBoundMIDI: int = -1, higherBoundMIDI: int = -1) -> float`  *(overloaded)*
- `getHarmonicSpectrum(numPartialsPerNote: int = 6, amplCallback: typing.Callable[[list[float]], list[float]] = None, partialsDecayExpRate: float = 0.8799999952316284) -> tuple[list[float], list[float]]`
- `getIntervals(firstNoteAsReference: bool = False) -> list[Interval]`
- `getIntervalsFromOriginalSortedNotes() -> list[Interval]`
- `getMeanFrequency(freqA4: float = 440.0) -> float`
- `getMeanMidiValue() -> int`
- `getMeanOfExtremesFrequency(freqA4: float = 440.0) -> float`
- `getMeanOfExtremesMidiValue() -> int`
- `getMeanOfExtremesPitch(accType: str = '') -> str`
- `getMeanPitch(accType: str = '') -> str`
- `getMidiIntervals(firstNoteAsReference: bool = False) -> list[int]`
- `getMidiValueStd() -> float`
- `getName() -> str`
- `getNote(noteIndex: int) -> Note`  *(overloaded)*
- `getNotes() -> list[Note]`
- `getOpenStackChord(enharmonyNotes: bool = False) -> Chord`
- `getOpenStackIntervals(firstNoteAsReference: bool = False) -> list[Interval]`
- `getOpenStackNotes() -> list[Note]`
- `getQuality() -> str`
- `getQuarterDuration() -> float`
- `getRomanDegree(key: Key, enharmonyNotes: bool = False) -> str`
- `getRoot() -> Note`
- `getSetharesDissonance(numPartialsPerNote: int = 6, useMinModel: bool = True, amplCallback: typing.Callable[[list[float]], list[float]] = None, partialsDecayExpRate: float = 0.8799999952316284, dissCallback: typing.Callable[[list[float]], float] = None) -> float`
- `getSetharesDyadsDataFrame(numPartialsPerNote: int = 6, useMinModel: bool = True, amplCallback: typing.Callable[[list[float]], list[float]] = None, partialsDecayExpRate: float = 0.8799999952316284) -> typing.Any`
- `getStackDataFrame(enharmonyNotes: bool = False) -> typing.Any`
- `haveAnyOctaveAugmentedFifth(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveAugmentedFourth(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveAugmentedOctave(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveDiminishedFifth(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveDiminishedOctave(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveDiminishedSeventh(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveFifth() -> bool`
- `haveAnyOctaveFourth() -> bool`
- `haveAnyOctaveMajorSecond(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMajorSeventh(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMajorSixth(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMajorThird(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMinorSecond(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMinorSeventh(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMinorSixth(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveMinorThird(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveOctave() -> bool`
- `haveAnyOctavePerfectFifth(useEnharmony: bool = False) -> bool`
- `haveAnyOctavePerfectFourth(useEnharmony: bool = False) -> bool`
- `haveAnyOctavePerfectOctave(useEnharmony: bool = False) -> bool`
- `haveAnyOctaveSecond() -> bool`
- `haveAnyOctaveSeventh() -> bool`
- `haveAnyOctaveSixth() -> bool`
- `haveAnyOctaveThird() -> bool`
- `haveAugmentedFifth(useEnharmony: bool = False) -> bool`
- `haveAugmentedFourth(useEnharmony: bool = False) -> bool`
- `haveAugmentedInterval(useEnharmony: bool = False) -> bool`
- `haveAugmentedOctave(useEnharmony: bool = False) -> bool`
- `haveAugmentedUnisson(useEnharmony: bool = False) -> bool`
- `haveDiminishedFifth(useEnharmony: bool = False) -> bool`
- `haveDiminishedInterval(useEnharmony: bool = False) -> bool`
- `haveDiminishedOctave(useEnharmony: bool = False) -> bool`
- `haveDiminishedSeventh(useEnharmony: bool = False) -> bool`
- `haveDiminishedUnisson(useEnharmony: bool = False) -> bool`
- `haveEleventh(useEnharmony: bool = False) -> bool`
- `haveFifth(useEnharmony: bool = False) -> bool`
- `haveFourth(useEnharmony: bool = False) -> bool`
- `haveMajorInterval(useEnharmony: bool = False) -> bool`
- `haveMajorNinth(useEnharmony: bool = False) -> bool`
- `haveMajorSecond(useEnharmony: bool = False) -> bool`
- `haveMajorSeventh(useEnharmony: bool = False) -> bool`
- `haveMajorSixth(useEnharmony: bool = False) -> bool`
- `haveMajorThird(useEnharmony: bool = False) -> bool`
- `haveMajorThirdteenth(useEnharmony: bool = False) -> bool`
- `haveMinorInterval(useEnharmony: bool = False) -> bool`
- `haveMinorNinth(useEnharmony: bool = False) -> bool`
- `haveMinorSecond(useEnharmony: bool = False) -> bool`
- `haveMinorSeventh(useEnharmony: bool = False) -> bool`
- `haveMinorSixth(useEnharmony: bool = False) -> bool`
- `haveMinorThird(useEnharmony: bool = False) -> bool`
- `haveMinorThirdteenth(useEnharmony: bool = False) -> bool`
- `haveNinth(useEnharmony: bool = False) -> bool`
- `haveOctave(useEnharmony: bool = False) -> bool`
- `havePerfectEleventh(useEnharmony: bool = False) -> bool`
- `havePerfectFifth(useEnharmony: bool = False) -> bool`
- `havePerfectFourth(useEnharmony: bool = False) -> bool`
- `havePerfectInterval(useEnharmony: bool = False) -> bool`
- `havePerfectOctave(useEnharmony: bool = False) -> bool`
- `havePerfectUnisson(useEnharmony: bool = False) -> bool`
- `haveSecond(useEnharmony: bool = False) -> bool`
- `haveSeventh(useEnharmony: bool = False) -> bool`
- `haveSharpEleventh(useEnharmony: bool = False) -> bool`
- `haveSixth(useEnharmony: bool = False) -> bool`
- `haveThird(useEnharmony: bool = False) -> bool`
- `haveThirdteenth(useEnharmony: bool = False) -> bool`
- `info() -> None`
- `insertNote(insertNote: Note, positionNote: int = 0) -> None`
- `isAugmentedChord() -> bool`
- `isDiminishedChord() -> bool`
- `isDominantSeventhChord() -> bool`
- `isHalfDiminishedChord() -> bool`
- `isInRootPosition() -> bool`
- `isMajorChord() -> bool`
- `isMinorChord() -> bool`
- `isSorted() -> bool`
- `isSus() -> bool`
- `isTonal(model: typing.Callable[[Chord], bool] = None) -> bool`
- `isWholeDiminishedChord() -> bool`
- `print() -> None`
- `printStack() -> None`
- `removeDuplicateNotes() -> None`
- `removeNote(noteIndex: int) -> None`
- `removeTopNote() -> None`
- `setDuration(duration: Duration) -> None`  *(overloaded)*
- `size() -> int`
- `sortNotes() -> None`
- `stackSize() -> int`
- `toCents() -> list[int]`
- `toInversion(inversionNumber: int) -> None`
- `transpose(semiTonesNumber: int) -> None`
- `transposeStackOnly(semiTonesNumber: int) -> None`

#### Class `Clef`
- `__init__(sign: ClefSign = ..., line: int = -1) -> None`
- `getLine() -> int`
- `getSign() -> ClefSign`
- `setLine(line: int) -> None`
- `setSign(sign: ClefSign) -> None`
- `toXML(clefNumber: int = -1, identSize: int = 2) -> str`

#### Class `ClefSign`
- `__init__(value: int) -> None`
- `name() -> str`
- `value() -> int`

#### Class `Helper`
- `calculateMelodyEuclideanSimilarity(melodyPattern: list[Note], otherMelody: list[Note]) -> float`  *(overloaded)*
- `calculateRhythmicEuclideanSimilarity(rhythmPattern: list[Note], otherRhythm: list[Note]) -> float`  *(overloaded)*
- `durationRatio(duration_A: float, duration_B: float) -> float`
- `freq2equalTemperament(freq: float, referenceFreq: float = 440.0) -> float`
- `freq2midiNote(freq: float, modelo: typing.Callable[[float], int] = None) -> tuple[int, int]`
- `freq2pitch(freq: float, accType: str = '') -> tuple[str, int]`
- `frequencies2cents(freq_A: float, freq_B: float) -> int`
- `getDurationDifferenceBetweenRhythms(referenceRhythm: list[Note], otherRhythm: list[Note]) -> list[float]`
- `getPercentiles(arg0: typing.Any, arg1: list[float]) -> typing.Any`
- `getSemitonesDifferenceBetweenMelodies(referenceMelody: list[Note], otherMelody: list[Note]) -> list[float]`
- `isEnharmonic(arg0: str, arg1: str) -> bool`
- `midiNote2freq(midiNoteValue: int, freqA4: float = 440.0) -> float`
- `midiNote2octave(midiNote: int) -> int`
- `midiNote2pitch(midiNote: int, accType: str = '') -> str`
- `midiNote2pitches(midiNote: int) -> list[str]`
- `noteSimilarity(pitchClass_A: str, octave_A: int, duration_A: float, pitchClass_B: str, octave_B: int, duration_B: float, enableEnharmonic: bool = False) -> tuple[float, float, float]`
- `noteType2RhythmFigure(noteType: str) -> RhythmFigure`
- `noteType2ticks(noteType: str, divisionsPerQuarterNote: int = 256) -> int`
- `notes2Intervals(pitches: list[str], firstNoteAsReference: bool = False) -> list[Interval]`  *(overloaded)*
- `pitch2freq(pitch: str) -> float`
- `pitch2midiNote(pitch: str) -> int`
- `pitch2number(pitch: str) -> float`  *(overloaded)*
- `pitchRatio(pitch_A: str, pitch_B: str) -> float`
- `rhythmFigure2Ticks(rhythmFigure: RhythmFigure, divisionsPerQuarterNote: int = 265) -> int`
- `rhythmFigure2noteType(rhythmFigure: RhythmFigure) -> str`
- `ticks2noteType(durationTicks: int, divisionsPerQuarterNote: int = 256, actualNotes: int = 1, normalNotes: int = 1) -> tuple[str, int]`
- `transposePitch(pitch: str, semitones: int, accType: str = '#') -> str`

#### Class `Interval`
- `__init__(pitch_A: str = 'C4', pitch_B: str = 'C4') -> None`  *(overloaded)*
- `getDiatonicInterval(useSingleOctave: bool = True, absoluteValue: bool = False) -> int`
- `getDiatonicSteps(useSingleOctave: bool = True, absoluteValue: bool = False) -> int`
- `getDirection() -> str`
- `getName() -> str`
- `getNotes() -> list[Note]`
- `getNumOctaves(absoluteValue: bool = False) -> int`
- `getNumSemitones(absoluteValue: bool = False) -> int`
- `getPitchStepInterval() -> int`
- `isAnyOctaveAugmentedFifth(useEnharmony: bool = False) -> bool`
- `isAnyOctaveAugmentedFourth(useEnharmony: bool = False) -> bool`
- `isAnyOctaveAugmentedOctave(useEnharmony: bool = False) -> bool`
- `isAnyOctaveDiminishedFifth(useEnharmony: bool = False) -> bool`
- `isAnyOctaveDiminishedOctave(useEnharmony: bool = False) -> bool`
- `isAnyOctaveDiminishedSeventh(useEnharmony: bool = False) -> bool`
- `isAnyOctaveFifth() -> bool`
- `isAnyOctaveFourth() -> bool`
- `isAnyOctaveMajorSecond(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMajorSeventh(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMajorSixth(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMajorThird(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMinorSecond(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMinorSeventh(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMinorSixth(useEnharmony: bool = False) -> bool`
- `isAnyOctaveMinorThird(useEnharmony: bool = False) -> bool`
- `isAnyOctaveOctave() -> bool`
- `isAnyOctavePerfectFifth(useEnharmony: bool = False) -> bool`
- `isAnyOctavePerfectFourth(useEnharmony: bool = False) -> bool`
- `isAnyOctavePerfectOctave(useEnharmony: bool = False) -> bool`
- `isAnyOctaveSecond() -> bool`
- `isAnyOctaveSeventh() -> bool`
- `isAnyOctaveSixth() -> bool`
- `isAnyOctaveThird() -> bool`
- `isAscendant() -> bool`
- `isAugmented(useEnharmony: bool = False) -> bool`
- `isAugmentedFifth(useEnharmony: bool = False) -> bool`
- `isAugmentedFourth(useEnharmony: bool = False) -> bool`
- `isAugmentedOctave(useEnharmony: bool = False) -> bool`
- `isAugmentedUnisson(useEnharmony: bool = False) -> bool`
- `isCompound() -> bool`
- `isDescendant() -> bool`
- `isDiminished(useEnharmony: bool = False) -> bool`
- `isDiminishedFifth(useEnharmony: bool = False) -> bool`
- `isDiminishedOctave(useEnharmony: bool = False) -> bool`
- `isDiminishedSeventh(useEnharmony: bool = False) -> bool`
- `isDiminishedUnisson(useEnharmony: bool = False) -> bool`
- `isEleventh(useEnharmony: bool = False) -> bool`
- `isFifth(useEnharmony: bool = False) -> bool`
- `isFourth(useEnharmony: bool = False) -> bool`
- `isMajor(useEnharmony: bool = False) -> bool`
- `isMajorNinth(useEnharmony: bool = False) -> bool`
- `isMajorSecond(useEnharmony: bool = False) -> bool`
- `isMajorSeventh(useEnharmony: bool = False) -> bool`
- `isMajorSixth(useEnharmony: bool = False) -> bool`
- `isMajorThird(useEnharmony: bool = False) -> bool`
- `isMajorThirdteenth(useEnharmony: bool = False) -> bool`
- `isMinor(useEnharmony: bool = False) -> bool`
- `isMinorNinth(useEnharmony: bool = False) -> bool`
- `isMinorSecond(useEnharmony: bool = False) -> bool`
- `isMinorSeventh(useEnharmony: bool = False) -> bool`
- `isMinorSixth(useEnharmony: bool = False) -> bool`
- `isMinorThird(useEnharmony: bool = False) -> bool`
- `isMinorThirdteenth(useEnharmony: bool = False) -> bool`
- `isNinth(useEnharmony: bool = False) -> bool`
- `isOctave(useEnharmony: bool = False) -> bool`
- `isPerfect(useEnharmony: bool = False) -> bool`
- `isPerfectEleventh(useEnharmony: bool = False) -> bool`
- `isPerfectFifth(useEnharmony: bool = False) -> bool`
- `isPerfectFourth(useEnharmony: bool = False) -> bool`
- `isPerfectOctave(useEnharmony: bool = False) -> bool`
- `isPerfectUnisson(useEnharmony: bool = False) -> bool`
- `isSecond(useEnharmony: bool = False) -> bool`
- `isSeventh(useEnharmony: bool = False) -> bool`
- `isSharpEleventh(useEnharmony: bool = False) -> bool`
- `isSimple() -> bool`
- `isSixth(useEnharmony: bool = False) -> bool`
- `isThird(useEnharmony: bool = False) -> bool`
- `isThirdteenth(useEnharmony: bool = False) -> bool`
- `isTonal() -> bool`
- `setNotes(pitch_A: str, pitch_B: str) -> None`  *(overloaded)*
- `toCents(freqA4: float = 440.0) -> int`

#### Class `Key`
- `__init__(fifthCircle: int = 0, isMajorMode: bool = True) -> None`  *(overloaded)*
- `getFifthCircle() -> int`
- `getName() -> str`
- `getRelativeKeyName() -> str`
- `isMajorMode() -> int`
- `setFifthCircle(fifthCircle: int) -> None`
- `setIsMajorMode(isMajorMode: bool) -> None`

#### Class `Measure`
- `__init__(numStaves: int = 1, divisionsPerQuarterNote: int = 256) -> None`
- `addNote(note: Note, staveId: int = 0, position: int = -1) -> None`  *(overloaded)*
- `clear() -> None`
- `divisionsPerQuarterNoteChanged() -> bool`
- `getBarlineLeft() -> Barline`  *(overloaded)*
- `getBarlineRight() -> Barline`  *(overloaded)*
- `getClef(clefId: int = 0) -> Clef`  *(overloaded)*
- `getDivisionsPerQuarterNote() -> int`
- `getDurationTicks() -> int`
- `getEmptyDurationTicks() -> int`
- `getFifthCircle() -> int`
- `getFilledDurationTicks(staveId: int = 0) -> int`
- `getFilledQuarterDuration(staveId: int = 0) -> float`
- `getFractionDuration() -> Fraction`
- `getFreeDurationTicks(staveId: int = 0) -> int`
- `getFreeQuarterDuration(staveId: int = 0) -> float`
- `getKey() -> Key`
- `getKeyName() -> str`
- `getKeySignature() -> str`
- `getNote(noteId: int, staveId: int = 0) -> Note`  *(overloaded)*
- `getNoteOff(noteOffId: int, staveId: int = 0) -> Note`  *(overloaded)*
- `getNoteOn(noteOnId: int, staveId: int = 0) -> Note`  *(overloaded)*
- `getNumNotes() -> int`  *(overloaded)*
- `getNumNotesOff() -> int`  *(overloaded)*
- `getNumNotesOn() -> int`  *(overloaded)*
- `getNumStaves() -> int`
- `getNumber() -> int`
- `getQuarterDuration() -> float`
- `getTimeMetronome() -> tuple[str, int]`
- `getTimeSignature() -> TimeSignature`
- `info() -> None`
- `isClefChanged() -> bool`
- `isEmpty() -> bool`
- `isMajorKeyMode() -> bool`
- `keySignatureChanged() -> bool`
- `metronomeChanged() -> bool`
- `removeNote(noteId: int, staveId: int = 0) -> None`
- `removeRepeatEnd() -> None`
- `removeRepeatStart() -> None`
- `setDivisionsPerQuarterNote(arg0: int) -> None`
- `setIsDivisionsPerQuarterNoteChanged(isDivisionsPerQuarterNoteChanged: bool = False) -> None`
- `setIsKeySignatureChanged(isKeySignatureChanged: bool = False) -> None`
- `setIsMetronomeChanged(isMetronomeChanged: bool = False) -> None`
- `setIsTimeSignatureChanged(isTimeSignatureChanged: bool = False) -> None`
- `setKey(fifthCircle: int, isMajorMode: bool = True) -> None`
- `setKeyMode(isMajorMode: bool) -> None`
- `setKeySignature(fifthCircle: int, isMajorMode: bool = True) -> None`
- `setMetronome(bpm: int, rhythmFigure: RhythmFigure = ...) -> None`
- `setNumStaves(numStaves: int) -> None`
- `setNumber(measureNumber: int) -> None`
- `setRepeatEnd() -> None`
- `setRepeatStart() -> None`
- `setTimeSignature(upper: int, lower: int) -> None`
- `timeSignatureChanged() -> bool`
- `toJSON() -> str`
- `toXML(instrumentId: int = 1, identSize: int = 2) -> str`

#### Class `Note`
- `__init__(pitch: str, rhythmFigure: RhythmFigure = ..., isNoteOn: bool = True, inChord: bool = False, transposeDiatonic: int = 0, transposeChromatic: int = 0, divisionsPerQuarterNote: int = 256) -> None`  *(overloaded)*
- `addArticulation(articulation: str) -> None`
- `addBeam(beam: str) -> None`
- `addSlur(slurType: str, slurOrientation: str) -> None`
- `addTie(tieType: str) -> None`
- `getAlterSymbol() -> str`
- `getArticulation() -> list[str]`
- `getBeam() -> list[str]`
- `getDiatonicSoundingPitchClass() -> str`
- `getDiatonicWrittenPitchClass() -> str`
- `getDivisionsPerQuarterNote() -> int`
- `getDuration() -> Duration`
- `getDurationTicks() -> int`
- `getEnharmonicNote(alternativeEnhamonicPitch: bool = False) -> Note`
- `getEnharmonicNotes(includeCurrentPitch: bool = False) -> list[Note]`
- `getEnharmonicPitch(alternativeEnhamonicPitch: bool = False) -> str`
- `getEnharmonicPitches(includeCurrentPitch: bool = False) -> list[str]`
- `getFrequency(freqA4: float = 440.0) -> float`
- `getHarmonicSpectrum(numPartials: int = 6, amplCallback: typing.Callable[[list[float]], list[float]] = None, partialsDecayExpRate: float = 0.8799999952316284, freqA4: float = 440.0) -> tuple[list[float], list[float]]`
- `getLongType() -> str`
- `getMidiNumber() -> int`
- `getNumDots() -> int`  *(overloaded)*
- `getOctave() -> int`
- `getPitch() -> str`
- `getPitchClass() -> str`
- `getPitchStep() -> str`
- `getQuarterDuration() -> float`
- `getScaleDegree(key: Key) -> int`
- `getShortType() -> str`
- `getSlur() -> tuple[str, str]`
- `getSoundingOctave() -> int`
- `getSoundingPitch() -> str`
- `getSoundingPitchClass() -> str`
- `getSoundingPitchStep() -> str`
- `getStaff() -> int`
- `getStem() -> str`
- `getTie() -> list[str]`
- `getTransposeChromatic() -> int`
- `getTransposeDiatonic() -> int`
- `getType() -> str`  *(overloaded)*
- `getUnpitchedIndex() -> int`
- `getVoice() -> int`
- `getWrittenOctave() -> int`
- `getWrittenPitch() -> str`
- `getWrittenPitchClass() -> str`
- `getWrittenPitchStep() -> str`
- `inChord() -> bool`
- `info() -> None`
- `isDotted() -> bool`
- `isDoubleDotted() -> bool`
- `isGraceNote() -> bool`
- `isNoteOff() -> bool`
- `isNoteOn() -> bool`
- `isPitched() -> bool`
- `isTransposed() -> bool`
- `removeTies() -> None`
- `setDuration(duration: Duration) -> None`  *(overloaded)*
- `setIsGraceNote(isGraceNote: bool = False) -> None`
- `setIsInChord(inChord: bool) -> None`
- `setIsNoteOn(isNoteOn: bool) -> None`
- `setIsPitched(isPitched: bool = True) -> None`
- `setIsTuplet(isTuplet: bool = False) -> None`
- `setOctave(octave: int) -> None`
- `setPitch(pitch: str) -> None`
- `setPitchClass(pitchClass: str) -> None`
- `setStaff(staff: int) -> None`
- `setStem(stem: str) -> None`
- `setTieStart() -> None`
- `setTieStop() -> None`
- `setTieStopStart() -> None`
- `setTransposingInterval(diatonicInterval: int, chromaticInterval: int) -> None`
- `setTupleValues(actualNotes: int, normalNotes: int, normalType: str = 'eighth') -> None`
- `setUnpitchedIndex(arg0: int) -> None`
- `setVoice(voice: int) -> None`
- `toEnharmonicPitch(alternativeEnhamonicPitch: bool = False) -> None`
- `toXML(instrumentId: int = 1, identSize: int = 2) -> str`
- `transpose(semitones: int, accType: str = '') -> None`

#### Class `NoteData`
- `__init__() -> None`  *(overloaded)*

#### Class `Part`
- `__init__(partName: str, numStaves: int = 1, isPitched: bool = True, divisionsPerQuarterNote: int = 256) -> None`
- `addMeasure(numMeasures: int) -> None`
- `addMidiUnpitched(midiUnpitched: int) -> None`
- `addStaves(numStaves: int = 1) -> None`
- `append(obj: Note | Chord, position: int = -1, staveId: int = 0) -> None`  *(overloaded)*
- `clear() -> None`
- `getMeasure(measureId: int) -> Measure`  *(overloaded)*
- `getMeasures() -> list[Measure]`
- `getMidiUnpitched() -> list[int]`
- `getName() -> str`
- `getNumMeasures() -> int`
- `getNumNotes(staveId: int = -1) -> int`
- `getNumNotesOff(staveId: int = -1) -> int`
- `getNumNotesOn(staveId: int = -1) -> int`
- `getNumStaves() -> int`
- `getPartIndex() -> int`
- `getShortName() -> str`
- `getStaffLines() -> int`
- `info() -> None`
- `isPitched() -> bool`
- `removeMeasure(measureStart: int, measureEnd: int) -> None`
- `removeStave(staveId: int) -> None`
- `setIsPitched(isPitched: bool = True) -> None`
- `setNumStaves(numStaves: int) -> None`
- `setPartIndex(partIdx: int) -> None`
- `setShortName(shortName: str) -> None`
- `setStaffLines(staffLines: int = 5) -> None`
- `toJSON() -> str`
- `toXML(instrumentId: int = 1, identSize: int = 2) -> str`

#### Class `RhythmFigure`
- `__init__(value: int) -> None`
- `name() -> str`
- `value() -> int`

#### Class `Score`
- `__init__(partsName: list[str], numMeasures: int = 20) -> None`  *(overloaded)*
- `addMeasure(numMeasures: int) -> None`
- `addPart(partName: str, numStaves: int = 1) -> None`
- `clear() -> None`
- `findMelodyPatternDataFrame(melodyPattern: list[Note], totalIntervalsSimilarityThreshold: float = 0.5, totalRhythmSimilarityThreshold: float = 0.5, intervalsSimilarityCallback: typing.Callable[[list[Note], list[Note]], list[float]] = None, rhythmSimilarityCallback: typing.Callable[[list[Note], list[Note]], list[float]] = None, totalIntervalSimilarityCallback: typing.Callable[[list[float]], float] = None, totalRhythmSimilarityCallback: typing.Callable[[list[float]], float] = None, totalSimilarityCallback: typing.Callable[[float, float], float] = None) -> typing.Any`  *(overloaded)*
- `forEachNote(callback: typing.Callable[[Part, Measure, int, Note], None], measureStart: int = 0, measureEnd: int = -1, partNames: list[str] = []) -> None`
- `getChords(config: json = None) -> list[tuple[int, float, Key, Chord, bool]]`
- `getChordsDataFrame(config: json = None) -> typing.Any`
- `getComposerName() -> str`
- `getFileName() -> str`
- `getFilePath() -> str`
- `getNumMeasures() -> int`
- `getNumNotes() -> int`
- `getNumParts() -> int`
- `getPart(partId: int) -> Part`  *(overloaded)*
- `getPartIndex(partName: str, index: int) -> bool`
- `getPartName(partId: int) -> str`
- `getPartsNames() -> list[str]`
- `getTitle() -> str`
- `haveAnacrusisMeasure() -> bool`
- `info() -> None`
- `isValid() -> bool`
- `removeMeasure(measureStart: int, measureEnd: int) -> None`
- `removePart(partId: int) -> None`
- `setComposerName(composerName: str) -> None`
- `setKeySignature(fifthCicle: int, isMajorMode: bool = True, measureId: int = 0) -> None`  *(overloaded)*
- `setMetronomeMark(bpm: int, rhythmFigure: RhythmFigure = ..., measureStart: int = 0) -> None`
- `setRepeat(measureStart: int, measureEnd: int = -1) -> None`
- `setTimeSignature(timeUpper: int, timeLower: int, measureId: int = -1) -> None`
- `setTitle(scoreTitle: str) -> None`
- `toDataFrame() -> typing.Any`
- `toFile(fileName: str, compressedXML: bool = False, identSize: int = 2) -> None`
- `toJSON() -> str`
- `toXML(identSize: int = 2) -> str`
- `xPathCountNodes(xPath: str) -> int`

#### Class `ScoreCollection`
- `__init__(directoryPath: str = '') -> None`  *(overloaded)*
- `addDirectory(directoryPath: str) -> None`
- `addScore(score: Score) -> None`  *(overloaded)*
- `clear() -> None`
- `findMelodyPatternDataFrame(melodyPattern: list[Note], totalIntervalsSimilarityThreshold: float = 0.5, totalRhythmSimilarityThreshold: float = 0.5, intervalsSimilarityCallback: typing.Callable[[list[Note], list[Note]], list[float]] = None, rhythmSimilarityCallback: typing.Callable[[list[Note], list[Note]], list[float]] = None, totalIntervalSimilarityCallback: typing.Callable[[list[float]], float] = None, totalRhythmSimilarityCallback: typing.Callable[[list[float]], float] = None, totalSimilarityCallback: typing.Callable[[float, float], float] = None) -> typing.Any`  *(overloaded)*
- `getDirectoriesPaths() -> list[str]`
- `getNumDirectories() -> int`
- `getNumScores() -> int`
- `getScores() -> list[Score]`  *(overloaded)*
- `isEmpty() -> bool`
- `merge(other: ScoreCollection) -> None`
- `removeScore(scoreIdx: int) -> None`
- `setDirectoriesPaths(directoriesPaths: list[str]) -> None`

#### Class `TimeSignature`
- `__init__(timeUpper: int = 4, timeLower: int = 4) -> None`
- `getLowerValue() -> int`
- `getMetric() -> METRIC`
- `getUpperValue() -> int`
- `setLowerValue(timeLower: int) -> None`
- `setUpperValue(timeUpper: int) -> None`

### `other`

**Module-level functions:**
- `setScoreEditorApp(executableFullPath: str) -> None`
- `getScoreEditorApp() -> str`
- `openScore(score: mc.Score) -> None`
- `getSampleScorePath(sampleEnum: SampleScore) -> str`
- `getXmlSamplesDirPath() -> str`

### `plots`

**Module-level functions:**
- `plotPartsActivity(score: mc.Score) -> tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]`
- `plotPianoRoll(score: mc.Score) -> tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]`
- `plotScorePitchEnvelope(score: mc.Score) -> tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]`
- `plotChordsNumberOfNotes(score: mc.Score) -> tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]`

### `sethares_dissonance`

**Module-level functions:**
- `plotSetharesDissonanceCurve(fundamentalFreq: float = 440, numPartials: int = 6, ratioLowLimit: float = 1.0, ratioHighLimit: float = 2.3, ratioStepIncrement: float = 0.001, amplCallback: Callable[[list[float]], list[float]] | None = None, partialsDecayExpRate: float = 0.88) -> tuple[go.Figure, pd.DataFrame]`
- `plotScoreSetharesDissonance(score: mc.Score, plotType: str = 'line', lineShape: str = 'linear', numPartialsPerNote: int = 6, useMinModel: bool = True, partialsDecayExpRate: float = 0.88, amplCallback: Callable[[list[float]], list[float]] | None = None, dissCallback: Callable[[list[float]], float] | None = None) -> tuple[go.Figure, pd.DataFrame]`
- `plotChordDyadsSetharesDissonanceHeatmap(chord: mc.Chord, numPartialsPerNote: int = 6, useMinModel: bool = True, amplCallback: Callable[[list[float]], list[float]] | None = None, partialsDecayExpRate: float = 0.88, dissonanceThreshold: float = 0.1, dissonanceDecimalPoint: int = 2, showValues: bool = False, valuesDecimalPlaces: int = 2) -> tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]`

<!-- BEGIN MANUAL -->
## Quick recipes

```python
# Count chord qualities in Beethoven's 5th
import maialib as ml
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Beethoven_Symphony_5th))
df = score.getChordsDataFrame()
print(df['quality'].value_counts())
```

```python
# Find a melodic pattern (C-E-G) across all parts
import maialib as ml
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
matches = score.findMelodyPatternDataFrame(["C4", "E4", "G4"])
print(matches)
```

## Gotchas

- **MusicXML only** — `.xml`, `.mxl`, `.musicxml`. Not `.mid`.
- **camelCase methods** — `getPitchClass`, never `get_pitch_class`.
- **`Score(path)` takes `str`** — wrap `pathlib.Path` with `str()`.
- **Samples ship bundled** — use `ml.SampleScore` + `ml.getSampleScorePath()`.
- **No async** — everything is synchronous.
<!-- END MANUAL -->

