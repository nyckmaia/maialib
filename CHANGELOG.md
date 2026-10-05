# Maialib - Change Log

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

- **Quarter-tone support.** A pitch can carry a quarter-tone accidental — `1x` (+0.5 semitone), `3x` (+1.5), `1b` (-0.5) or `3b` (-1.5) — as well as `bb`, `b`, `#` and `x`, in any octave from -1 to 11: `C1x4` lies halfway between `C4` and `C#4`. A pitch string used to accept only the four whole-tone accidentals, so `Note("C1x4")` and `Helper.pitch2midiNote("C1x4")` raised
  - **`Pitch`**, a new class (C++ and Python), holds one pitch: a diatonic step, an accidental that is exactly a multiple of 0.5 from -2 to 2, and an octave — or a rest, whose `getOctave()` is `None`. `getMidiNumber()` rounds a quarter tone to the nearest semitone, ties upward (`C1x4` is 61, `D1b4` is 62), `getQuarterToneSteps()` is the exact position (60.5), and `getFrequency()` is exact (`A1x4` is 452.89 Hz). In Python the pitch string is the only constructor, `ml.Pitch("C1x4")` (default `"rest"`); a MIDI number, a frequency or the three components go through the factories `Pitch.fromMidi(midiNumber, accType="")`, `Pitch.fromFrequency(frequency, accType="", freqA4=440.0, enableQuarterToneRound=False)` and `Pitch.fromComponents(step, alter, octave)`, so that `ml.Pitch(110)` — 110 Hz or MIDI 110? — raises `TypeError` instead of guessing. A positive frequency always gives a pitch, rounded to the nearest semitone or, with `enableQuarterToneRound`, the nearest quarter tone (ties upward), and moved into the representable range with a warning when it lies outside (below MIDI 0 it becomes `C-1`; with the default `accType` every frequency from MIDI 156 up becomes `B11`); a frequency of 0 or less is a rest, and `NaN`, or a `freqA4` that is not a finite number greater than 0, raises. When the requested `accType` cannot spell the result, the default spelling is used and a warning says so. `setMidiNumber(midiNumber, accType="")` takes the same accidental type as `fromMidi()`. Two pitches are equal when they are spelled the same (`Pitch("C#4") != Pitch("Db4")`, although `Helper.isEnharmonic()` finds them enharmonic), and `Pitch` is hashable consistently with that. `setAlter()` accepts only a value exactly on the quarter-tone grid (`0.99996` raises; `-0.0` is stored as `0.0`); a setter that refuses a value on a rest, or one that would move the pitch below MIDI 0, prints a warning, which Python receives on `sys.stdout`. Frequencies are equal-tempered: in C++, `getFrequency()` and `setFrequency()` raise when `setTuningSystem()` has selected another tuning system
  - **`Note`** is built on `Pitch`. It accepts quarter-tone pitches and adds `isQuarterTone()`, `roundToSemitone()` (ties upward: `C1x4` becomes `C#4`, `D1b4` becomes `D4`), `getQuarterToneSteps()` — the exact sounding position, by which notes are ordered — and `setStep()` and `setAlter()`, which change one component of the written pitch. `getMidiNumber()` and `getFrequency()` round a quarter tone to the nearest semitone, ties upward (`Note("A1x4").getFrequency()` is 466.16 Hz); `Pitch.getFrequency()` gives the exact value. On a transposing instrument a quarter tone sounds as one: a B-flat clarinet's written `C1x4` sounds `B1b3`. The enharmonic family (`getEnharmonicPitch()`, `getEnharmonicPitches()`, `getEnharmonicNote()`, `getEnharmonicNotes()`, `toEnharmonicPitch()`) respells a quarter tone from each white key within 1.5 semitones of it, in that key's own octave, so a quarter tone has one or two partners: `C1x4` is `D3b4` by default and `B3x3` as the alternative, and `C3x4` is only `D1b4`. The default is the partner with the smaller accidental, a tie going to the side opposite the note's own accidental, as `C#4` becomes `Db4`
  - **Transposition** by quarter tones: `Note.transpose()`, `Chord.transpose()`, `Chord.transposeStackOnly()` and `Helper.transposePitch()` take a `float` number of semitones, which must be a multiple of 0.5 (anything else raises `RuntimeError` naming the value), and compute on exact positions: `Note("C4").transpose(0.5)` gives `C1x4`, and `C1x4` transposed by 2 gives `D1x4`. `Helper.steps2pitch()` spells an exact position, and `Helper.validateTransposeSemitones()` checks an interval
  - **MusicXML.** `Score(path)` reads a quarter tone from `<accidental>` — the Tartini names `quarter-sharp`, `three-quarters-sharp`, `quarter-flat` and `three-quarters-flat`, and the arrow glyphs `sharp-down`, `sharp-up`, `flat-up` and `flat-down` — or from a decimal `<alter>` that is exactly a multiple of 0.5 from -2 to 2, read with a `.` decimal point whatever the C or C++ locale (a near value such as `0.46` reads as natural, with a warning). `sharp-sharp`, the double sharp drawn as two sharps, is read as a double sharp. `Note.toXML()` writes a quarter tone's `<alter>` with one decimal place (`0.5`, `-1.5`) and its Tartini `<accidental>`, which no key signature can imply; a whole-tone accidental is written as before, as `<alter>` alone. What this changes for files that already held quarter-tone accidentals is under Fix
  - **Analysis.** A method whose return type can express a quarter tone computes it: `Chord.toCents()` (`["C4", "E1b4", "G4"]` gives `[350, 350]`), `Chord.isSorted()`, `Chord.sortNotes()` and `Note`'s ordering operators (exact positions), both `Chord.getHarmonicDensity()` overloads, `Chord.getMidiValueStd()`, `Helper.isEnharmonic()` (`C1x4` and `D3b4` are enharmonic, `C1x4` and `C#4` are not) and `Helper.getSemitonesDifferenceBetweenMelodies()` (a neutral third against a major third differs by 0.5). A method whose return type cannot raises `RuntimeError`, naming the offending note and a remedy the caller can apply. The harmonic analysis (`getName()`, `getQuality()`, `getRoot()`, `getBassNote()`, `getDegree()`, `stackSize()`, `getStackedHeaps()`, the stack getters, the `is*` and `have*` predicates), the `Chord` methods that build intervals (`getIntervals()`, `getIntervalsFromOriginalSortedNotes()`) and the MIDI-integer methods (`getMidiIntervals()`, `getMeanMidiValue()`, `getMeanOfExtremesMidiValue()`, `getMeanPitch()`, `getMeanOfExtremesPitch()`) name `Chord.roundQuarterTones()`, which rounds a chord's quarter tones in place, ties upward, and returns how many notes it changed; `Interval` names `Note.roundToSemitone()`; and the melody-pattern search (`Score.findMelodyPatternDataFrame()`) raises for a pattern, or a segment of the score, that starts on a quarter tone, naming the note and, for a segment, its part, measure and stave. `Chord.info()` still prints a quarter-tone chord's notes, without the analysis. `maialib.plotScorePitchEnvelope()` uses the mean methods, so it raises for a score with quarter tones until they are rounded
  - `Helper.alterName2symbol()`, `Helper.alterSymbol2Value()`, `Helper.alterValue2symbol()` and `Helper.alterValue2Name()` are bound to Python, as is the new `Helper.spelling2midiNote()`, which gives the MIDI number of a step, alter and octave and rejects an alter that is not a finite number from -2 to 2 or an octave outside -1 to 11
- **Pitch views of a note, and diatonic transposition.** A note of a transposing instrument is written at one pitch and sounds at another, and `Note` answers about its pitch in three views, described in its class documentation (the Doxygen "Pitch views" section and the `Note` class docstring)
  - **Written**, the pitch as written in the part: `getWrittenPitch()`, `getWrittenOctave()`, `getWrittenPitchClass()`, `getWrittenPitchStep()` and `getDiatonicWrittenPitchClass()`. The unprefixed getters `getPitch()`, `getOctave()`, `getPitchClass()`, `getPitchStep()` and `getAlterSymbol()` are shortcuts for it, and the enharmonic family, `transpose()`, the setters and `toXML()` work on it
  - **Sounding**, what the note sounds, in its simplest spelling: `getSoundingPitch()`, `getSoundingOctave()`, `getSoundingPitchClass()`, `getSoundingPitchStep()` and `getDiatonicSoundingPitchClass()`. Of the spellings of the position, the simplest has the smallest accidental; between a sharp and a flat equally close, the side of the spelling it simplifies is kept; the octave is that spelling's own. An untransposed `Cb4` sounds `B3` (octave 3) and `C3x4` sounds `D1b4`, while `Db4`, `C#4` and `E1x4` stay as they are. The plots of `maialib` (piano roll, parts activity, pitch envelope, Sethares dissonance) label each note with it
  - **Acoustic**, measures of what sounds: `getMidiNumber()`, `getQuarterToneSteps()`, `getFrequency()` and `getHarmonicSpectrum()`
  - **Diatonic transposition.** A transposed note sounds its written pitch moved by both parts of its transposing interval: the letter by `transposeDiatonic` (MusicXML `<diatonic>`), carrying octaves across C, and the exact position by `transposeChromatic`. A B-flat clarinet's (-1, -2) written `F#4` sounds `E4`, its written `Db4` sounds `B3` and its written `C1x4` sounds `B1b3`; a horn in F's (-4, -7) written `B4` sounds `E4`; a piccolo's (7, 12) written `Bb4` sounds `Bb5`. A transposing interval given only in semitones — `transposeDiatonic` 0 with a non-zero `transposeChromatic`, as for a `<transpose>` without `<diatonic>` or a `Note` built with only `transposeChromatic` — moves the letter by the diatonic interval conventionally written for those semitones: 7 letters for each whole octave, plus a second for 1 or 2 semitones, a third for 3 or 4, a fourth for 5 or 6 (the tritone as an augmented fourth), a fifth for 7, a sixth for 8 or 9 and a seventh for 10 or 11, so (0, -2) sounds as a B-flat clarinet's (-1, -2) does (a written `F#4` sounds `E4`) and (0, -7) as a horn in F's (-4, -7); `getTransposeDiatonic()` still returns 0, and a non-zero `transposeDiatonic` is used as given. Where the diatonic interval gives no spelling — an accidental beyond a double sharp or flat, or an octave outside -1 to 11 — the position is spelled from its semitone alone: as the white key there or the sharp of the white key below, or, when the instrument sounds lower than written, as the flat of the white key above if that key is white and the flat keeps the octave (a B-flat clarinet's written `Cbb4` is spelled `Ab3`, as the letter B would need three flats), and then simplified. The inferred interval and this fallback are silent and documented on `getSoundingPitch()`. Above `B11` the diatonic interval spells `B1x11`, `B#11`, `B3x11` and `Bx11` (a written `A#11` a major second up sounds `B#11`); at that very top a respelling can have no spelling once transposed, so `toEnharmonicPitch()`, `getEnharmonicNote()` and `getEnharmonicNotes()` raise for it (the written `A#11`'s respelling `Bb11` would need `C12`)
  - **Analyses at concert pitch.** `Chord`, `Interval`, `Score.getChords()` and the melody-pattern search relate a transposed note at concert pitch: its written letter moved by the diatonic interval (inferred, as above, when only the chromatic one is given), with the accidental of the exact position, not simplified — or, where the diatonic interval gives no spelling, the fallback above before its simplification. A B-flat clarinet's written `D4` and a violin's `C4` are a unison, a horn in F's written `B4` with `C4` and `G4` is a C major chord, and a B-flat clarinet's written `Db4` is a `Cb4` there; a written `F#4` with only `transposeChromatic=-2` is an `E4`, a unison with `E4`. Untransposed notes are related as written. `Note` has no getter for the concert spelling; the notes the chord analysis returns are untransposed notes at concert pitch, as the stacking spells them (see Fix)
  - **Known limitation:** the chord analysis distinguishes pitch classes by spelling, so a chord that mixes a transposing part written in a sharp key with parts in flat keys can be left unnamed: Dvořák's horns in E and clarinets in A sound `E#4`, `G#4` and `C#5` against the strings' `F`, `Ab` and `Db`
- **MusicXML `<transpose>`.** `Score(path)` reads a `<transpose>` in every measure and per staff (`number`), with `<octave-change>` and `<double>`, and the export writes `<transpose>` elements derived from the notes, so a re-imported score keeps its transpositions
  - Each note holds its transposition: the reader stamps every pitched note with the `<transpose>` in force for its staff, from the notes written after it in its measure up to the next `<transpose>` for that staff or for every staff. `<octave-change>` is folded into the note's interval, 7 letters and 12 semitones per octave (a B-flat bass clarinet's `-1`, `-2`, `-1` is `(-8, -14)`)
  - `OctaveDoubling` (`NONE`, `BELOW`, `ABOVE`; C++ and Python) with `Note.setOctaveDoubling()` and `Note.getOctaveDoubling()` holds a `<double/>` or `<double above="yes"/>`. `Score.getChords()` adds, for each doubled note, an untransposed note one octave below or above its concert pitch to each chord it sounds in (a doubled octave outside the representable range is left out with one warning per note), and so do `getChordsDataFrame()`, the chord qualities, the Sethares dissonance and the chord plots built on it; `plotPianoRoll()` draws the doubled octave too; the melody-pattern search ignores it. `==` and the hash ignore the doubling; a rest refuses it with a warning, and `setPitch("rest")` clears it
  - `Part.setTransposingInterval(diatonicInterval, chromaticInterval, measureStart=0, measureEnd=-1, staff=-1, doubling=OctaveDoubling.NONE)` stamps the pitched notes of a range of measures and a staff, all or none: a note that would have no sounding pitch raises `RuntimeError` naming it, and no note changes. In Python it changes a score's transpositions in place, which an edit through `Measure.getNote()`, a copy, cannot
  - The export writes, for each staff, a `<transpose>` in measure 1 when its first pitched note is transposed or doubled, and one wherever the interval or the doubling changes: in the measure's `<attributes>` when the staff's first pitched note there brings the change, otherwise in an `<attributes>` written just before the note; without `number` when every staff has the same transposition there. The interval is unfolded into `<diatonic>`, `<chromatic>` and, from an octave on, `<octave-change>`; a stored diatonic interval of 0 is written as the conventional one. A chord whose notes have different intervals or doublings cannot be written: the export raises `RuntimeError` naming the part, the measure and the staff
  - The reader corrects or ignores a `<transpose>` with one warning per correction, which starts with a code: `[transpose-chromatic-not-integer]` and `[transpose-octave-change-not-integer]`, a value that is not a whole number, ignored; `[transpose-pair-corrected]`, a `<diatonic>` that does not match `<chromatic>`, replaced by the conventional diatonic interval (a tritone accepts the augmented fourth and the diminished fifth); `[transpose-out-of-range]`, an interval with which a note of its scope would have no sounding pitch, ignored for its whole scope. An ignored `<transpose>` leaves the previous transposition in force, and a note that cannot sound with that one either is read untransposed. `<for-part>` is not modelled: `[for-part-not-modelled]` reports it and it is dropped
- Add `Helper.getLibraryVersion()` (C++ and Python) so the compiled `maiacore` library itself can report the version from the root `VERSION` file, matching `maialib.__version__` and `maialib.maiacore.__version__`
- Add `Helper.splitPitch()` to Python
- Add `Chord.getStackedHeaps()` and `Chord.isDyad()` to Python — both existed only in C++ — and the read-only properties `note`, `wasEnharmonized` and `enharmonicDiatonicDistance` to `NoteData`, the objects `getStackedHeaps()` returns, which were bound with none

### Improve

- Accept every pitch spelling with `bb`, `b`, `#` and `x` accidentals in octaves -1 to 11: `Note` rejected `Cbb0`, `Bx9`, `G#10`, `Bx11` and every pitch in octave -1. MIDI numbers and enharmonic spellings are computed arithmetically instead of from hard-coded tables, and `Interval` works across the whole range
- `Note(midiNumber)` accepts every MIDI number that can be spelled within octaves -1 to 11 (up to 155 with the default accidental type, 157 with `"x"`), where it rejected everything above 127
- Document the Python bindings of `Note`, `Pitch`, `Interval`, `Helper`, `Score` and `ScoreCollection` that round or raise for quarter tones, or for a note sounding below `C1b-1`, with examples
- Document in Python, with examples, `Score.getChords()` and `Score.getChordsDataFrame()` and the `Chord` getters that return analysed notes: `getRoot()`, `getBassNote()`, `getOpenStackNotes()`, `getOpenStackChord()`, `getCloseStackChord()` and `getCloseChord()`
- **Breaking:** Declare minimum versions for the runtime dependencies: `plotly>=6.1.1`, `kaleido>=0.2.1` and `nbformat>=4.2.0`, and `numpy>=1.23.2`, which `maialib.maiapy` imports directly and which was not declared at all. An environment pinned to an older `plotly` no longer resolves

### Fix

- `Note.setPitch()` rejected a pitch with a two-digit octave, such as `"C10"`
- Methods that indexed past the end of a container check it first. On an empty or too-short chord, `Chord.getNote()`, `info()`, `toInversion()`, `removeTopNote()`, `insertNote()` and `removeNote()` raise `RuntimeError` — they indexed out of range, which crashed the interpreter or returned garbage (`Chord(["C4"]).getNote(5)` gave a note whose pitch was `"0"`) — and `isInRootPosition()`, `toCents()`, `getOpenStackIntervals()` and `getCloseStackIntervals()` answer `False` or an empty list, where they crashed or raised `ValueError`. `Measure.getNote()` raises `IndexError` for a stave index outside the measure, where it crashed, and `getNoteOn()`, `getNoteOff()` and the `getNumNotes*()` methods check a negative stave index too. `Part.append()` of a note or chord that does not fit in the last measure raises `RuntimeError` and leaves the part unchanged, where it crashed. `Measure.toXML()` no longer reads before a stave's first note, which crashed at random, and the C++ `Score::instrumentFragmentation()` no longer reads an empty vector for a section with no notes. A `Chord` analysed again after `addNote()` or `removeNote()` answers for its new notes, where it raised (e.g. `MemoryError`)
- `Chord.transpose()` and `Chord.transposeStackOnly()` stored each note as they went, so a note that raised left the chord half-transposed (`Chord(["C4", "E4", "G4"]).transpose(-61)` raised on `E4` after turning `C4` into a rest); every note is now transposed before any is stored, and a raise leaves the chord unchanged
- `Note.setDuration()` stored the new tick count, and `Note.setTupleValues()` the new note counts, before raising for a value they reject (`setDuration(0.0)` left the note with 0 ticks). A `Note` mutator that raises now leaves the note unchanged
- `Note.setOctave()` on a rest raised `RuntimeError` ("Unknown pitch: rest"); it now prints a warning and leaves the rest unchanged, as `setAlter()` does
- `Score.findMelodyPatternDataFrame()` and `ScoreCollection.findMelodyPatternDataFrame()` with a list of patterns hung forever when given a Python callback, because the worker threads need the GIL to call it while the calling thread held it; the search now releases the GIL while it runs, and several Python threads can search the same score at once. They also no longer print a progress line ("Processando padrão de melodia: N") for each pattern from those threads, which garbled one another
- `Score.findMelodyPatternDataFrame()` with a single pattern, and the C++ single-pattern `Score::findMelodyPattern()`, raised `ValueError` (`std::length_error` in C++) for a pattern longer than the first-voice melodies of all the parts together (chords and other voices are not searched) but not longer than the whole score, and `ScoreCollection.findMelodyPatternDataFrame()` with a single pattern passed that error on. `Score` now finds no match and returns an empty result; `ScoreCollection` raises its documented no-match error, `KeyError: 'scoreTitle'`. The overloads that take a list of patterns never raised this error
- `maialib.__version__` is defined — `from .maiacore import *` skipped it — and `maialib.maiacore.__version__` no longer carries literal quote characters (`'"1.10.3"'`) when the module is built with `make`; both are the version in the root `VERSION` file
- Loading MusicXML with `Score(path)` warns when it cannot represent a note's accidental: an `<alter>` other than the nine multiples of 0.5 from -2 to 2 (a triple sharp, `3`; an eighth tone, `0.25`), which is still read as natural, or an `<accidental>` name it does not know, for which the `<alter>` is used. Neither aborts the load
- **Breaking:** `Helper.isEnharmonic()` did not detect E#/F, B#/C and Cb/B: `isEnharmonic("E#4", "F4")` was `False`. `Helper.noteSimilarity(..., enableEnharmonic=True)` did not treat such a pair as one pitch either
- **Breaking:** `Helper.pitch2midiNote()` answered -1 for every pitch in octave -1 (`"C-1"` is 0, `"B-1"` is 11) and 132 for `"Db10"` (133); a pitch below MIDI 0, such as `"Cb-1"`, now raises
- **Breaking:** `Helper.midiNote2pitch()` and `Helper.transposePitch()` could answer a spelling in octave 12, outside the supported range (`midiNote2pitch(156)` was `"C12"`, `transposePitch("G9", 30, "")` was `"C#12"`); they now raise, naming the MIDI number and the octave range
- **Breaking:** `Note.setPitch()` kept the previous accidental symbol: `Note("C#4").setPitch("D4")` left `getAlterSymbol()` at `"#"`
- **Breaking:** `Note`'s setters could leave a note contradicting itself, because each getter read its own copy of the pitch; `Note` now holds one `Pitch` and derives every getter from it. `setPitchClass()` kept the previous MIDI number (`Note("C4").setPitchClass("Bb")` reported 60, now 70) and left a rest a rest (it now creates a note in octave 4, `Bb4`); `setTransposingInterval()` applied the chromatic interval again on every call (a written `C4` given -2 twice reported 56, now 58); `setIsNoteOn(False)` left `getPitchClass()` and `getMidiNumber()` reporting the old pitch (`"C#"`, 61), and a transposed note's `getPitch()` its old sounding pitch (`"Bb3"`) — the note is now a rest everywhere; and `setIsNoteOn(True)` on a rest made a note with no pitch (`getPitch()` gave `"rest0"`) — it now prints a warning and leaves the rest unchanged
- **Breaking:** A rest has no octave: `Note.getOctave()` and `Note.getWrittenOctave()` answered 0 for a rest, `Note.getSoundingOctave()` -2, and `Helper.midiNote2octave()` -2 for a negative MIDI number; they return `None` (in C++, an empty `std::optional<int>`, and `Helper::splitPitch()`'s octave parameter is a `std::optional<int>&`). `Note.getPitchStep()`, `getWrittenPitchStep()` and `getSoundingPitchStep()` answered `"r"` for a rest; they answer `"rest"`
- **Breaking:** On a transposing instrument, the sounding pitch was looked up in tables by written pitch class, which gave wrong pitches and spellings for ordinary notes: a B-flat clarinet's written `C#4` sounded `Bb3` and its written `F#4` `Bb4`, and a horn in F's written `F#4` sounded `F3`, where they sound MIDI 59, 64 and 59; and a transposed note's `getOctave()` could change when `setOctave()` re-set its own octave. The sounding pitch is now the written pitch moved by both parts of the transposing interval (see Added): `getSoundingPitch()` gives `B3`, `E4` and `B3`. `getMidiNumber()` is unchanged. Migration: read what a note sounds from the Sounding getters, as the next entries describe
- **Breaking:** `getPitch()`, `getOctave()`, `getPitchClass()`, `getPitchStep()` and `getAlterSymbol()` describe the written pitch, and the enharmonic family — `getEnharmonicPitch()`, `getEnharmonicPitches()`, `getEnharmonicNote()`, `getEnharmonicNotes()` and `toEnharmonicPitch()` — respells it; they described the sounding pitch (`getAlterSymbol()` keeps the written accidental it gave in v1.10). Only transposed notes change: a B-flat clarinet's written `D4` answers `getPitch()` `"D4"`, where it answered `"C4"`. Migration: use `getSoundingPitch()`, `getSoundingOctave()`, `getSoundingPitchClass()` or `getSoundingPitchStep()` where the sounding pitch was meant
- **Breaking:** `getEnharmonicNote()` and `getEnharmonicNotes()` return notes that keep the transposing interval: each is written with a respelling of the written pitch and sounds what the note sounds, so a B-flat clarinet's written `D4` gives clarinet notes written `Ebb4` and `Cx4`, sounding `C4`. They returned untransposed notes at a respelling of the sounding pitch. Migration: `getPitch()` of a returned note is the written respelling; for an untransposed note at a respelling of what sounds, use `Note(note.getSoundingPitch()).getEnharmonicNote()`
- **Breaking:** `getSoundingPitch()`, `getSoundingPitchClass()`, `getSoundingPitchStep()` and `getDiatonicSoundingPitchClass()` give the simplest spelling (see Added), also for an untransposed note, which answered its written spelling: `Cb4` sounds `B3`, `E#4` sounds `F4`, `Ebb4` sounds `D4` and `Fx4` sounds `G4`. This changes untransposed notes spelled with `Cb`, `Fb`, `E#`, `B#` or a double accidental, and transposed notes. `getSoundingOctave()` is that spelling's own octave: `B1x3` gives 3, though its MIDI number 60 lies in octave 4. Migration: use `getWrittenPitch()` (or `getPitch()`) for the spelling as written, and `getMidiNumber()` or `getQuarterToneSteps()` for the position
- **Breaking:** `transpose()` moves the written pitch once. On a transposing instrument it transposed the sounding pitch and stored the result as the written pitch, so the interval was applied twice and `transpose(0)` moved the note: a B-flat clarinet's written `C4` became a written `Bb3`, sounding `Ab3`. `toEnharmonicPitch()` respells the written pitch, so what the note sounds no longer moves: a B-flat clarinet's written `C#4` becomes `Db4`, still sounding MIDI 59; it respelled the sounding pitch and stored it as the written one, so the same note became a written `A#3` sounding MIDI 56. Migration: drop any compensation for the doubled interval, since `transpose(n)` now moves what a transposed note sounds by exactly `n` semitones; `toEnharmonicPitch()` keeps what the note sounds, and a respelling of what it sounds is `Note(note.getSoundingPitch()).getEnharmonicPitch()`
- **Breaking:** The analyses of transposing parts relate the notes at concert pitch (see Added), so chord and interval names can change there: `Interval`, every `Chord` analysis, `Score.getChords()` and the melody-pattern search's `transposeInterval`. They related the looked-up sounding spelling, which could name a wrong pitch: an `Interval` from a `C4` to a B-flat clarinet's written `F#4` raised "Unable to compute the interval [C4, Bb4]" and is now a major third, and the chord of that `F#4` with `C4` and `G4` had no name and is now C major; the same holds for a written `F#4` with only `transposeChromatic=-2`, which v1.10 read exactly as the clarinet's. When the diatonic interval — given, or inferred from the chromatic one — spells both notes, an interval between two notes of one part is now the interval between their written pitches. Untransposed notes are related as written, as in v1.10. Migration: none for scores without transposing parts; re-run the analyses of scores with transposing parts
- **Breaking:** The notes the chord analysis returns — `getRoot()`, `getBassNote()`, `getOpenStackNotes()`, the open and close stack chords, `getCloseChord()` and the heaps of `getStackedHeaps()` — are untransposed notes at concert pitch, also for a note of a transposing instrument; `getNotes()` still returns the notes as they were added. `Chord(notes)` holds its notes untransposed at their concert spelling, and `removeDuplicateNotes()`, which `Score.getChords()` applies unless `includeDuplicates` is set, pairs notes by concert spelling: a B-flat clarinet's written `F#4` duplicates an `E4`, and its written `Db4` no longer duplicates a `B3`. Migration: read the parts' own notes with `getNotes()`, and compare a note the analysis returns with `==`, which compares at concert pitch
- **Breaking:** `Note`'s `==` and `!=` compare the concert spelling, where they compared `getPitch()` strings: a B-flat clarinet's written `Db4` equals `Note("Cb4")`, where it equalled `Note("B3")`, and its written `F#4` equals `Note("E4")`. `Note.__hash__` and `Chord.__hash__` hash exactly what `==` compares; they also hashed what `==` ignores (the duration type and, for a note, five flags), so equal notes or chords could hash differently. `Note.__repr__` and `Note.info()` show the written pitch, where they showed the sounding one. `Chord`'s `__repr__`, `info()`, `print()`, `printStack()` and C++ `operator<<` show each note at its concert spelling, as the analysis relates it: a B-flat clarinet's written `Db5` is shown `Cb5`, where it was `B4`; an untransposed chord is shown as before. `getScaleDegree()` reads the written step, in the written key of the part: a B-flat clarinet's written `D4` is degree 2 of C major, where it was 1. Migration: compare `getWrittenPitch()` strings for identity as written and `getSoundingPitch()` strings for the simplest spelling of what sounds; for the degree of what sounds, call `getScaleDegree()` on `Note(note.getSoundingPitch())` with the key that sounds
- **Breaking:** A transposed note whose sounding pitch lies above `B11` (MIDI note 155) and that its diatonic interval does not spell is rejected (above `B11` only `B1x11`, `B#11`, `B3x11` and `Bx11` can be spelled, when the diatonic interval moves the written letter to the B of octave 11): the constructor, `setTransposingInterval()`, `setPitch()`, `setPitchClass()`, `setStep()`, `setOctave()`, `setAlter()`, `transpose()` and `toEnharmonicPitch()` raise `RuntimeError` naming the written pitch, the interval and the sounding position, and leave the note unchanged. Such a note was built and reported a pitch in octave 12 or above (`Note("C10", transposeDiatonic=1, transposeChromatic=30).getPitch()` was `"F#12"`). Migration: keep a transposed note's sounding pitch at or below `B11`, or catch `RuntimeError`
- **Breaking:** A note whose sounding pitch falls below the lowest representable pitch, `C1b-1` (-0.5, MIDI note 0) — such as a written `C0` with `transposeChromatic=-13` — is still constructible, and `isNoteOn()` is `True`, but every sounding or acoustic getter (`getSoundingPitch()`, `getSoundingOctave()`, `getMidiNumber()`, `getFrequency()`, …), `Note.info()`, every comparison or analysis of the note, and a `Chord`'s display (`__repr__`, `info()`, `print()`, `printStack()`, `operator<<`) raise one error naming the written pitch and the interval, where they answered values such as `-1` and `"B#-2"`. An `Interval` with such a note is refused when it is built. The written getters, and the unprefixed getters that are shortcuts for them, answer with the written pitch. Migration: keep a transposed note's sounding pitch at or above `C1b-1`, or catch `RuntimeError`
- **Breaking:** Transposing a pitch to below MIDI 0 gave a rest: `Helper.transposePitch("C4", -61)` returned `"rest"`, so `Note.transpose()` and `Chord.transpose()` turned such a note into a rest. Transposing a pitch outside the representable range, `C1b-1` (-0.5) to `Bx11` (157), now raises `RuntimeError` naming the pitch, the interval and the range; transposing a rest still gives `"rest"`, and an empty string is a rest too. `transposePitch()` also parses its input first, so an invalid pitch raises even for an interval of 0 (`transposePitch("H4", 0)` returned `"H4"`)
- **Breaking:** `Chord.toCents()` measured each interval from the two notes' frequencies and truncated it: a C major triad read `[400, 299]` and `["C4", "G4", "C5"]` read `[700, 499]`. It now computes on exact positions, so semitone intervals are exact multiples of 100 (`[400, 300]`, `[700, 500]`)
- **Breaking:** `Chord.getMidiValueStd()` and `Chord.getFrequencyStd()` computed the standard deviation of a list padded with one leading zero per note, so a C major triad reported `31.8978` and `168.144` instead of `2.8674` and `53.24`; an empty chord returned `NaN` and now returns `0.0`. `getFrequencyStd()` takes each note's frequency from `Note.getFrequency()`, which rounds a quarter tone
- **Breaking:** A `Chord` analysed and then emptied with `clear()` kept its analysis: `getName()` still gave the name of the chord before the clear; it now analyses the empty chord (`""`)
- **Breaking:** `Score.findMelodyPatternDataFrame()` with a list of patterns — and `ScoreCollection.findMelodyPatternDataFrame()` and the C++ `Score::findMelodyPattern()` list overload — searched only as many patterns as the machine has hardware threads, returning the rest empty, and returned a pattern empty when its search failed, printing the error to the C++ standard error. Every pattern is now searched, and a failing search raises its error, as the single-pattern search does
- **Breaking:** `Score(path)` ignored `<accidental>` and read only an `<alter>` of -2, -1, 1 or 2, so a quarter-tone accidental, or a decimal `<alter>` such as `0.5`, loaded as natural. Those notes now load as quarter tones (see Added), so the harmonic analysis, interval and melody-pattern methods raise for such a score until its quarter tones are rounded (e.g. `Note.roundToSemitone()` on every note through `Score.forEachNote()`). An `<accidental>` also wins over a disagreeing `<alter>`, with a warning (`<alter>1</alter>` with `<accidental>natural</accidental>` loaded `C#4`, now `C4`), and is read without one (`<accidental>sharp</accidental>` alone loaded `C4`, now `C#4`)
- **Breaking:** In C++, `Helper::alterValue2Name()` gives the Tartini names for quarter tones (`0.5` is `"quarter-sharp"`, where it was the arrow name `"sharp-down"`; likewise `"three-quarters-sharp"`, `"quarter-flat"` and `"three-quarters-flat"`), and `Helper::alterValue2symbol()` and `Helper::alterValue2Name()` accept only an alter exactly on the quarter-tone grid from -2 to 2, raising, with the value, for any other: a value within 0.05 of one, such as `0.46`, was rounded to it. Neither depends on the global C++ locale any more (under a comma-decimal one they raised for every value), and `-0.0` is the natural
- **Breaking:** `Score(path)` read `<transpose>` in a part's first measure only and ignored `<octave-change>`, `<double>` and `number`, taking the first `<diatonic>` and the first `<chromatic>` of that measure even from different elements: octave-transposing parts — contrabasses, contrabassoons, piccolos, celestas, bass clarinets — sounded an octave from what the file says, and a change of transposition was ignored (W3C 72c kept its E-flat clarinet in measure 2). Sounding pitches, `Score.getChords()` and everything built on it change for such files: in the Beethoven 5 sample 118 of 1,462 chord names and 721 bass notes change, and the Dvořák and Mahler 8 samples, W3C 41c and `test_getchords_poly.musicxml` change too
- **Breaking:** `Score.getChords()` and `Score.getChordsDataFrame()` report the concert key of each chord's measure: the most frequent written key among the pitched parts that are untransposed there or transposed by whole octaves (a tie goes to the first part), or, when every pitched part transposes, the first pitched part's written key moved by its interval and brought into -6..11 fifths. They reported the first part's written key, which disagreed with the concert-pitch chords whenever that part transposes (W3C 72a reported D major for a C major chord)
- **Breaking:** A `<transpose>` whose `<diatonic>` and `<chromatic>` disagree is corrected: the Dvořák sample's trumpets in E, `(3, 4)`, are read as `(2, 4)`, so a written `F#4` sounds `A#4` (it sounded `Bb4`). A `<transpose>` without `<diatonic>` stores the conventional diatonic interval, where it stored 0
- **Breaking:** A `<transpose>` that would push a note out of the representable range no longer aborts the load with `RuntimeError`; it is ignored with a warning
- **Breaking:** Exports write `<transpose>`, so `Score.__hash__` and `Part.__hash__`, which hash the export, change for scores with transpositions; an export used to lose every transposition, so a re-imported score sounded its written pitches

### Removed

- **Breaking:** `Helper.pitch2number()` (C++ and Python) and `Helper::number2pitch()` (C++) — use `Helper.pitch2midiNote()` for the numeric value and `Helper.isEnharmonic()` for the comparison it was used for
- **Breaking:** The C++ constants `MUSIC_XML::MIDI::NUMBER::MIDI_000` … `MIDI_132` and `c_pianoWhiteKeys`, which nothing used
- **Breaking:** Renamed the misspelled `alternativeEnhamonicPitch` keyword argument to `alternativeEnharmonicPitch` on `Note.getEnharmonicPitch()`, `Note.getEnharmonicNote()` and `Note.toEnharmonicPitch()` (C++ and Python) — callers passing it by keyword must update the spelling
- **Breaking:** `Helper::semitonesBetweenPitches()` (C++), which had no caller; `Pitch::getQuarterToneSteps()` gives exact differences, quarter tones included. It was never bound to Python
- **Breaking:** `Score::getNote()` (three overloads) and `Score::getNoteNodeData()` (C++), which had no callers, together with the `Helper::getNoteNodeData()` declaration, which had no definition. None was bound to Python

### Build and tooling

- Build and test commands exit with a non-zero code when a step fails. The scripts behind `make` ignored their commands' exit codes, so `make cpp-tests`, `make py-tests` and `make tests` exited 0 with failing tests, and a failed compilation looked like a success. `make tests` stops at the first failing suite
- Every script runs its Python tools (pip, the stub generators, the tests) with the interpreter it runs under, the Makefile's `PYTHON`, and `make dev` builds the module for that interpreter: CMake kept the interpreter of the first configure, even one from a deleted virtual environment. `make dist`, and so `make install`, package that interpreter's module (`maiacore` plus its extension suffix): they took the first module in the build directory, which after a switch to another Python version could be the one built for the previous version
- The C++ tests are built by the root CMake project and link the `maiacore` target, so every library change relinks them; a library-only change used to leave a stale test binary. The library and the tests use one cpptrace, v0.8.2 from FetchContent; the vendored v0.6.2 headers are removed
- `make clean` also removes the read-only files that FetchContent leaves in the build directories; `make static-clean`, `make shared-clean` and `make module-clean` remove their build directories, where they did nothing; `make cpp-tests-clean` is new; and `make cmake`, which ran a script that does not exist, is removed
- The scripts behind `make clean`, `make dist` and `make uninstall` delete only inside the repository: run directly from another directory, they deleted that directory's `build/` and `dist/`. A symbolic link or a junction among the paths they delete is removed as a link, and what it points to is left unchanged. An entry that cannot be removed no longer stops the removal of the rest, and every such entry is listed
- `requirements-dev.txt` exact-pins the runtime dependencies, the build and development tools (`setuptools`, `wheel`, `pybind11-stubgen`, `mypy`, `cpplint`, `cppcheck`, `ruff`) and the test-only `lxml`, so that `make tests` runs in a reproducible environment instead of picking up new upstream releases; `setup.py` keeps version floors only. It needs Python 3.12 or newer, which its pinned `numpy` requires
- `make validate` fails on cpplint and cppcheck findings that are not in `scripts/validate-baseline.json`, where it always passed; `make validate-update-baseline` accepts the current findings. cpplint does not check line length (`maiacore/CPPLINT.cfg` filters `whitespace/*`): clang-format applies the 100-column limit
- `make validate` runs cpplint and cppcheck with the Makefile's `PYTHON`, and `requirements-dev.txt` pins `cppcheck==1.5.1` (Cppcheck 2.17.1), whose findings the baseline holds; another cppcheck version can report other findings. cppcheck analyses the sources for the `unix64` platform on every operating system, so the baseline is the same everywhere. `make dev`, `make static` and `make shared` no longer run SQLiteCpp's own cppcheck pass, which any cppcheck on PATH turned on
- `make msvc-gate` (Windows) builds maiacore and the C++ tests with Visual Studio 2022 in Debug and Release and runs them, then builds the package with `pip install .`, as CI does, and runs the Python tests. `make linux-gate` exports the committed `HEAD` to a temporary directory on Linux (natively, or in WSL on Windows) and runs `make cpp-tests` there with GCC, then `make dev` and `make py-tests` in a fresh virtual environment; it lists the apt packages that are missing instead of installing them
- The Windows shared library built by `make shared` exports its symbols; it exported none, so no program could link against it
- MSVC Debug builds keep CMake's own Debug flags (`/Zi /Ob0 /Od /RTC1`): they were given the GCC and Clang flags `-g -O0`, which cl ignores with warning D9002
- Test fixtures named `test_*.xml` in any other directory are no longer ignored by Git: the rule meant for the files the test suites write into the repository root and `test/` matched them in every directory, so `git status` did not list a new fixture
- `make coverage` stops with a message on any operating system other than Linux, the only one whose Debug build records coverage: on Windows it printed a note and exited 0, and on macOS it ran lcov without coverage data
- The C++ tests are valid C++17: two lambdas captured structured bindings, a C++20 feature that Clang before 16 rejects
- The wheel workflow publishes to PyPI only for a published GitHub release; every push to `main` and every manual run tried to publish as well
- MusicXML test infrastructure (`test/musicxml/`, described in its README): the official MusicXML 4.0 XSD and the W3C MusicXML test suite, vendored unmodified with their licences; offline validation against the schema plus the rules it cannot express; a canonical JSON dump of a maialib `Score`, pinned by a golden file; a strict ledger of how every corpus file loads, exports, validates and round-trips, checked by `make py-tests` (the files of 10 MB or more, and the external corpus that `make corpus-fetch` downloads, which has its own ledger, only by `make corpus`); `make corpus`, `make corpus-update-ledger`, `make corpus-fetch` (OpenScore, CC0) and `make fuzz`/`make fuzz-minimize` (seeded mutation fuzzing). A corpus file whose path is not ASCII is loaded from an ASCII-named copy, because maialib cannot open such a path on Windows. `requirements-dev.txt` pins `lxml` as a test-only dependency, which `make msvc-gate` installs; the maialib package does not use it
- The C++ load/export round-trip test writes its file to the temporary directory, not to the repository root
- `make corpus` also runs the export/import round trip of `<transpose>` (`test/test_musicxml_transpose.py`) with `MAIALIB_SLOW_TESTS=1`, which adds the slow corpus files that `make py-tests` skips; the MusicXML model dump records each note's octave doubling
- **Breaking:** `tests-cpp` can no longer be configured as a standalone CMake project; configure the root project with `-DSTATIC_LIB=ON -DMAIACORE_BUILD_TESTS=ON`, as `make cpp-tests` does
- **Breaking:** `make shared`, `make shared-debug` and `make shared-release` build a shared library into `build/<OS>/shared/<Debug|Release>/`. `make shared-debug` built a static library into `build/<OS>/static/`, and the other two stopped with "No rule to make target"
- **Breaking:** `make validate` fails when cpplint or cppcheck reports a finding that is not in the baseline, or cannot run, which includes cppcheck not being installed for the Makefile's `PYTHON` (`pip install -r requirements-dev.txt`): a cppcheck on PATH is not used
- **Breaking:** The Makefile ignores an environment variable named `PYTHON`; pass the interpreter on the command line instead, e.g. `make "PYTHON=py -3.12"`

---

## [v1.10.0] - 2025-11-18

### Improve

- Add a complete C++ unit-test coverage (maiacore library)
- Improve C++ classes documentation
- Improve Python tutorials
- Improve README.md

### Fix

- Minor bug fixes

---

## [v1.9.6] - 2025-11-06

### Improve

- Add support to Python 3.13 and 3.14

---

## [v1.9.5] - 2025-08-31

### Improve

- Note and Chord class: Add partialsDecayExpRate optional parameter to control the partials decay exponential rate (default: 0.88).
- Improve Doxygen documentation

### Fix

- Fix Beethoven 5th Symphony notes trill

---

## [v1.9.3] - 2025-07-02

### Fix

- Rename Python wrapper ScoreCollection.findMelodyPattern to .findMelodyPatternDataFrame
- Update Doxygen documentation

---

## [v1.9.2] - 2025-06-28

### Improve

- Multiple classes doxygen documentation

### Fix

- Interval::_Diminished_ word spelling is correct now

---

## [v1.9.1] - 2025-06-11

### New features

### Improve

- maiapy.plotChordsNumberOfNotes
- maiapy.plotPianoRoll
- maiapy.plotScorePitchEnvelope
- maiapy.plotSetharesDissonanceCurve
- notes2Intervals add overload de pitch string

### Fix

---

## [v1.9.0] - 2025-03-29

### New features

- Add new chord classification: 'sus' for "suspended chord" (sus4 and sus2)

### Improve

- Split 'diminished' chord classification into:
  - diminished (triad or tetrad)
  - half-diminished (tetrad)
  - whole-diminished (tetrad)

### Fix

- Fix special dyads interval classifications. Exemple: [A4, Cbb5] as +d3

---

## [v1.8.2] - 2025-02-26

### Fixed

- Fix plot functions that were using .getMIDINumber() to use the new API .getMidiNumber()
- Fix Pandas dependecy minimum version to v2.0.0

---

## [v1.8.1] - 2024-11-22

### API Changes

- Note Class:
  - .getMIDINumber -> .getMidiNumber
- Chord Class:
  - .inversion -> .toInversion
  - .getMIDIIntervals -> .getMidiIntervals

---

## [v1.8.0] - 2024-11-08

### New Features

- ScoreCollection::findMelodyPattern method

---

## [v1.7.1] - 2024-11-03

### API Changes

### New Features

- Score::findMelodyPatternDataFrame() overload method support find multiple patterns at the same time in parallel

### Improved

- Update Pybind11 to v2.13.6
- C++ compiler from std=17 to std=20

### Fixed

- Fix Python 3.8 incompatible internal libraries

---

## [v1.7.0] - 2024-10-31

### API Changes

### New Features

- Score Class:
  - .findMelodyPattern()
  - .findMelodyPatternDataFrame()
- Helper Class
  - .getSemitonesDifferenceBetweenMelodies()
  - .getDurationDifferenceBetweenRhythms()
  - .calculateMelodyEuclideanSimilarity()
  - .calculateRhythmicEuclideanSimilarity()
- Part::getMeasures()
- Interval::getDiretion()

### Improved

### Fixed

---

## [v1.6.1] - 2024-08-16

### Fixed

- Replace the deprecatted 'pkg_recourses' package to 'importlib.resources'
- Fix break lines and doubles spaces on the score parts name

---

## [v1.6.0] - 2024-07-17

### API Changes

- Note and Chord classes:
  - .setDurationTicks(const int) replaced by .setDuration(const Duration&)
- MUSICXML::NOTE_TYPE remove all dotted rhythmFigure constants
- Replace all old mentions of 'duration' by 'RhythmFigure'
- Note class:
  - Remove: .removeDots(), .setSingleDot(), setDoubleDot()

### New Features

- New class Duration
- New class Fraction to store any possible musical note duration
- New class TimeSignature
- Add Helper::ticks2rhythmFigure()
- Add Cpptrace library to get the stackTrace when a exception occors inside the maiacore library

### Improved

- Improve CMakeLists.txt reading
- Add VERSION file
- Helper::ticks2noteType() now supports tuplets
- Note::getDuration() now supports tuplets
- Note class now have a Duration private member
- Add "kaleido" and "nbformat" as install requirements

### Fixed

- Score::getChords fixed chord stack with tuplets

---

## [v1.5.0] - 2024-06-23

### API Changes

- Score::getLoadedFilePath() to .getFilePath()

### New Features

- Score Class:
  - Add .getFileName()
- Chord Class:
  - Add .getQuality()
- Note Class:
  - Add .getWrittenPitchClass()
  - Add .getSoundingPitchClass()

### Improved

- Score::getChords() and getChordsDataFrame() returns struct with a new boolean field called 'isHomophonic'
- Interval::analyse() add 'diminished unison' and 'augmented unison' invervals

### Fixed

- Note::getPitchClass() now points to Note::getSoundingPitchClass()
- Note::getPitchStep() now points to Note::getSoundingPitchStep()

---

## [v1.4.8] - 2024-06-01

### Improved

- Remove ScoreCollection::get/setName
- Add Score::getLoadedFilePath

---

## [v1.4.7] - 2024-MM-DD

### Improved

- Improve Github Actions wheels.yml file
- Add Score::haveAnacrusisMeasure()

### Fixed

- Auto fix XML scores that contais multiple part with the same name

---

## [v1.4.6] - 2024-05-30

### Improved

- Update Pybind11 to v2.12.0
- Update pybind11-stub-gen to v2.5.1
- Update pybind11_json to v0.2.14
- Update nlomann::json to v3.11.3

### Fixed

- Note::getScaleDegree() now works ok
- Fix initial score clefs bug
- Score::toFile() show the output XML file absolute path in the LOG

---

## [v1.4.4] - 2023-11-14

### Fixed

- www.maialib.com DNS now point to the official maialib documentation
- Fix all maialib examples located inside the `python-tutorial` folder

---

## [v1.4.3] - 2023-09-20

### API Changes

- Chord::getSetharesPartialsDissonance() replaced by .getSetharesDyadsDissonance()
  - Wrapper: getSetharesPartialsDataFrame replaced by getSetharesDyadsDataFrame
    getSetharesDyadsDataFrame
  - Rename and add new columns

### New Features

- maiapy
  - .plotChordDyadsSetharesDissonanceHeatmap()

### Fixed

- ml.plotSetharesDissonanceCurve

---

## [v1.4.2] - 2023-09-04

### API Changes

- Python Wrapper: Chord.getSetharesDissonanceValue() replaced by .getSetharesDissonance()

### New Features

- Chord::getSetharesDyadsDissonance()
  - Python Wrapper called: .getSetharesDyadsDataFrame()

### Improved

- maiapy
  - 'plotScoreSetharesDissonance':
    - Add 'numPartials', 'useMin', 'ampCallback' arguments
    - Add optional 'plotType' input argument: 'scatter' or 'line'
    - Add optional 'lineShape' input argument: 'linear' or 'spline'
    - Add line label for the 'mean value': Dissonance Mean

### Fixed

---

## [v1.4.1] - 2023-08-30

### Improved

- maiacore
  - Improve error message to help users to fix corrupted XML files
- maiapy
  - plotScoreSetharesDissonance:
    - Add new input argument 'numPoints' to create a interpolated curve
    - Add a horizontal dashed line to show the 'dissonance mean' value

### Fixed

- maiapy
  - Fix plotScoreSetharesDissonance display title

---

## [v1.4.0] - 2023-08-26

### New Features

- Note Class

  - .getHarmonicSpectrum()

- Chord Class

  - .getHarmonicSpectrum()
  - .getSetharesDissonanceValue()

- maiapy
  - Add ml.plotSetharesDissonanceCurve()
  - Add ml.plotScoreSetharesDissonance()

### Improved

- Note::getPitch() now returns the 'soundingPitch' and not the 'writtenPitch'
- Note::getFrequency() now have 2 optional input arguments:
  - bool equalTemperament = true
  - float freqA4 = 440.0f

### Fixed

- Score class
  - Disable (comment) the methods: .countNotes() and .findPattern(). Future work.

---

## [v1.3.0] - 2023-07-18

### New Features

- Key class: Add overload constructor that accepts a string as a key signature
- Note class: Add .getScaleDegree(const Key&)

### Improved

### Fixed

---

## [v1.2.0] - 2023-05-20

### New Features

- Chord::getCloseStackHarmonicComplexity()
- Chord::getHarmonicDensity()

### Improved

### Fixed

---

## [v1.1.0] - 2023-05-06

### New Features

- Chord Class

  - .getMeanFrequency()
  - .getMeanMidiValue()
  - .getMeanPitch()
  - .getMeanOfExtremesFrequency()
  - .getMeanOfExtremesMidiValue()
  - .getMeanOfExtremesPitch()
  - .getFrequencyStd()
  - .getMidiValueStd()
  - .getDegree()
  - .getRomanDegree()

- New 'Key' Class

- Maiapy functions
  - .plotScorePitchEnvelope()
  - .plotChordsNumberOfNotes()

### Improved

- Score::getChordsDataFrame() includes a new column 'key'
- Python wrapper: Chord::_repr_() shows a list of soundingPitch notes
- Chord class now have "for range" C++ iterators
- Upgrade SqLiteCpp to v3.2.1

### Fixed

- Score::getChords() and .getChordsDataFrame() now works good
- Score::info() shows the partName list correctly
- Maialib plots that were broken, now works good
- Beethoven 5th XML metadata: fix to Cm key

---

## [v0.1.3] - 2023-04-05

### New Features

- Maiacore
  - `Measure` Class:
    - .getQuarterDuration, .getFilledQuarterDuration, .getFreeQuarterDuration
    - .getDurationTicks, .getFilledDurationTicks, .getFilledDurationTicks
  - `Helper::int duration2Ticks()`

### Improved

- Maiapy:
  - `setScoreEditorApp()` add special documentation for Mac OSX

### Fixed

- Score change clef in export XML file
- Part::setIsPitched() now works correctly

---

## [v0.1.2] - 2023-04-04

### New Features

- `Chord::getIntervalsFromOriginalSortedNotes()`

### Improved

### Fixed

- All `Chord` methods that starts with `.have` + Interval name

---

## [v0.1.1] - 2023-04-03

### New Features

- Maiacore:
  - Add: `Part::get/setPartIndex(int)`
- Maiapy:
  - Add: `setScoreEditorApp()` and `openScore()` functions

### Improved

- To build maiacore now is necessary v3.26
- Update GoogleTests to v1.13.0
- Split `Score::getChords()` and `Score::getChordsDataFrame()` in two methods

### Fixed

- Maiacore:
  - `Score::getPart(std::string&)` method
  - `Score::getPartsNames()` now is the standard way to get the all the parts names
  - `Measure::isEmpty()`

---

## [v0.0.19] - YYYY-MM-DD

### API Changes

- Nothing change

- Add `Duration` enum class Python wrapper
- Continuous Integration & Continuous Delivery through Github Actions to Pypi
- Add Microsoft Visual Studio Compiler (MSVC) compatibility
- Uses Clang++ as a default compiler on Windows when execute `make`
- `Helper Class`:
  - Add .frequencies2cents() method
  - Add .freq2equalTemperament() method
- `Interval Class`: Add .toCents() method
- `Chord Class`: Add .toCents() methods
- Add `Score::getComposerName()` method

### Improved

- `Score::info()` added a `partName` list
- `Score::instrumentFragmentation()` replace `partNumber` to `partNames`
- Add overload `Note::setDuration(const float durationValue, const int lowerTimeSignatureValue = 4)`
- Install using `pyproject.toml` file
- Better folder organization
- Better headers declaration organization inside the `*.cpp` files

### Fixed

- `Interval::isSimple()` and `Interval::isCompound()`
- `Score::instrumentFragmentation`: Call `getComposerName()` and `getTitle()`

---

## [v0.0.17] - 2022-10-22

### API Changes

- `Chord::getStackedHeaps()` -> `Chord::getStackDataFrame()`

### New Features

- `Interval Class`: 50 new classification methods
- Replace old log using `std::cout` by the new C++ macros for log data and exceptions:
  - `LOG_DEBUG()`
  - `LOG_INFO()`
  - `LOG_WARN()`
  - `LOG_ERROR()`
- `Chord Class`
  - 50 new `.haveXInterval()` methods
  - `.getCloseChord()`
  - `.isInRootPosition()`
- Add `make validate`: Includes `CppCheck` and `CppLint`

### Improved

- `Chord Class`
  - `.getStackDataFrame()` and `.getStackHeaps()`: Improve sorted heaps algorithm and DataFrame data types
- `Interval Class`:
  - `.isAscendant()`
  - `.isDescendant()`

### Fixed

- `Interval Class`:
  - `.getDiatonicInterval()`
- `Chord::getRoot()`

---

## [v0.0.16] - 2022-10-03

### API Changes

- `Chord::getStackedChord()` -> `Chord::getOpenStackChord()`

### New Features

- `Note Class`:
  - `.getFrequency()`
  - `.getEnharmonicPitch()`
  - `.toEnharmonicPitch()`
  - `.getEnharmonicPitches()`
  - `.getEnharmonicNote()`
  - `.getEnharmonicNotes()`
- `Interval Class`:
  - `.setNotes()`
  - `.getPitchStepInterval()`
  - `.isMinorThird()`
  - `.isMajorThird()`
  - `.isDiminishedFifth()`
  - `.isPerfectFifth()`
  - `.isAugmentedFifth()`
  - `.isDiminishedSeventh()`
  - `.isMinorSeventh()`
  - `.isMajorSeventh()`
  - `.isMinorNinth()`
  - `.isMajorNinth()`
  - `.isPerfectEleventh()`
  - `.isSharpEleventh()`
  - `.isMinorThirdteenth()`
  - `.isMajorThirdteenth()`
  - `.isSimple()`
  - `.isCompound()`
  - Oveload operator `<`
- `Helper Class`:
  - `.midiNote2octave()`
  - `.notes2intervals()`
- `Chord Class`:
  - `.getOpenStackChord()`
  - `.getCloseStackChord()`
  - `.getOpenStackedNotes()`
  - `.getStackedHeaps()`
  - `.getOpenStackIntervals()`
  - `.getCloseStackIntervals()`
  - `.isSorted()`
- New Unit Tests:
  - `Note::getEnharmonicPitch`
  - `Note::getEnharmonicPitches()`
  - `Note::getEnharmonicNotes()`

### Improved

- `Chord::stackInThirds()` now can use enharmony to compute the stacked chord
- `Interval::getDiatonicSteps()` added optional argument `useSingleOctave`
- `Chord::getMIDIIntervals()` added optional argument `firstNoteAsReference`
- Now every maiacore C++ exception message starts with `[maiacore]` prefix

### Fixed

- `Note::Note()` Prevent invalid user type octaves and out of range MIDI numbers

---

## [v0.0.15] - 2022-09-07

### New Features

- Add maialib Python stubs

### Improved

- `ScoreCollection` class:
  - Add overloaded constructor: vector of directories paths
  - Add `.merge(ScoreCollection& other)` method
  - Add `+` operator overload to `.merge(ScoreCollection& other)` method
  - Add `.getNumDirectories()` method
  - Add `.addDirectory()` method
  - Add `.setDirectoriesPaths()` method
  - Add oveloaded methods:
    - `.addScore(const std::string& filePath)`
    - `.addScore(const std::vector<std::string>& filePaths)`
- Replace `python3` to `python` calls
- Update `Pybind11` to v2.10.0

### Fixed

- C++ & Python unit tests are now running without any errors:
  - `make cpp-tests`
  - `make py-tests`
  - `make tests`

---

## [v0.0.14] - 2022-07-06

### New Features

- Add New `ScoreCollection` class
- Add to all maialib classes:
  - Add Python `__hash__` method
  - Add Python `__sizeof__` method
- `Chord` Class: Add `getNotes()` method

### Improved

- Update Pybind11 to v2.9.2
- Better maialib objects representation inside the Python environment
- `Score::forEachNote()` callback contains more input parameters

### Fixed

---

## [v0.0.13] - 2022-03-05

### New Features

### Improved

- `Chord` class:
  - Add optional input argument: `.getIntervals(bool fromRoot = false)`
  - Add optional input argument: `.getStackIntervals(bool fromRoot = false)`
- Trello: Cards and status updated

### Fixed

- Fix `Score::setRepeat()` default arguments
- Fix `Note::transpose()` now works as expected

---

## [v0.0.12] - 2022-03-04

### New Features

- Add `Note::getDuration()` as an alias to `Note::getQuarterDuration()`
- Add `Duration` enum class
- Add `std::vector<std::string> Helper::midiNotes2pitches(int midiNote)`
- Add `void Score::setRepeat(int measureStart, int measureEnd)`
- `Chord` class:
  - Add `.getIntervals()`
  - Add `.getStackIntervals()`

### Improved

- `Note` constructor now receives a `Duration` enum class as second parameter
- `Helper::midiNotes2pitch()` if the user try to do something impossible, now this method returns an error

### Fixed

- Replace `Chord::getIntervalNames()` by `Chord::getIntervals()`

---

## [v0.0.11] - 2022-02-16

### New Features

- `Note` class
  - Add constructor overload to receive MIDI Numbers
  - Add `.transpose()`
- Add external `Cherno Instrumentor` profiler class. Load the output `profile.json` file inside `chrome://tracing/`

### Improved

### Fixed

- `Helper::midiNote2pitch()`

---

## [v0.0.10] - 2022-02-14

### New Features

- Add Makefile target: `make validate`
  - Run `cppcheck` static analyzer (optional dev dependency)
  - Run `cpplint` (optional dev dependency)
- Add `float Chord::getQuarterDuration()`
- `Note` class:
  - Add `float getQuarterDuration()`
  - Add `std::string getType()`
  - Add `std::string getLongType()`
  - Add `std::string getShortType()`
  - Add `int getNumDots()`
  - Add `bool isDotted()`
  - Add `bool isDoubleDotted()`

### Improved

- `Note` class: abstract the `ticks` integer values and replace it to `rhythm figures` as strings
- `Chord::getDuration()` now returns a `rhythm figure` as string
- `Helper::noteType2ticks()`
- `Helper::ticks2NoteType()`

### Fixed

---

## [v0.0.9] - 2022-02-11

### New Features

- Add `Chord::removeDuplicateNotes()`

### Improved

- `Score::getChords()`
  - Add optional boolean config input argument: `includeUnpitched`
  - Add optional boolean config input argument: ``includeDuplicates`
  - Replace optional default values:
    - `minStack`: minimum value from `2` to `1`
    - `maxDuration`: maximum value from `quarter` to `maxima`
- Update Doxygen documentation
- Update Trello status and cards

### Fixed

- Fix `Score::getChords()` input arguments: `minDuration` and `maxDuration`

---

## [v0.0.8] - 2022-02-10

### New Features

- Add unit test to `Helper::ticks2noteType()`
- Add `Chord::sortNotes()`
- `Measure` Class
  - Add `.divisionsPerQuarterNoteChanged()`
  - Add `.setIsDivisionsPerQuarterNoteChanged()`

### Improved

- Performance: Add optimization flags to the build system
- Performance: `Score::getChords()` add database indexes to speed up queries
- Performance: `Helper::ticks2noteType()` add GCC extension: `case range`
- Better code reading on `Score::loadXML()`
- Move external utility functions to a new file called: `utils.h`

### Fixed

- Fix `Score::getChords()` to support multiple `divisionsPerQuarterNote`
- Fix `cpp-tests` build
- Move `divisionsPerQuarterNote` from `Score` to `Measure` class
- Remove `divisionsPerQuarterNote` from `Note` class

---

## [v0.0.7] - 2022-02-08

### New Features

- Rewrite: `Score::getChords()`
  - Mode `Continuos`
  - Mode `Same Attack`
- Add `Chord::getDurationTicks()` method
- Add external dependency: `SQLiteCpp`
- Add `BuildCache` as a _optional_ development tool

### Improved

- `VS Code`: Enable `pretty-printing` for `gdb`

### Fixed

- `Helper::noteType2ticks()` fix uppercase strings

---

## [v0.0.6] - 2022-01-24

### New Features

- Add `Measure::getNumber()` and `Measure::setNumber()`

### Improved

- `Score::toDataFrame()`
  - Now return more data and columns for index and objects
  - Better performace
- Python default print for: `Part` and `Measure` objects

### Fixed

---

## [v0.0.5] - 2022-01-22

### New Features

- Add `Note::getStaff()`
- Add `Score::getPart(const std::string& partName)` overload
- Add `Score::getPartNames()`
- Add `Score::toDataFrame()`

### Improved

- `make dev` first uninstall the current installed Python module before start the new installation
- Better `README.md`
- Added a new test score: `test_multiple_instruments4.musicxml`

### Fixed

- Python wrapper: `Helper::ticks2noteType()` and `Helper::noteType2ticks`
- tests-cpp: `CMakeLists.txt` to include `maiacore` library

---

## [v0.0.4] - 2022-01-17

### New Features

- Part Class:
  - `.getNumNotesOn()`
  - `.getNumNotesOff()`
- Measure Class:
  - `.getNumNotesOn()`
  - `.getNumNotesOff()`
  - `.getNoteOff()`
- Note Class:
  - `.isNoteOff()`
- New `dist` folder structure
- Added `maiapy` to this project

### Improved

- Rename the compiled C++ output module from `maialib` to `maiacore`
- Parallel build: dynamically count the max number of threads (`make jobs`)
- Rename `Measure::getNumElements()` to `.getNumNotes()`
- Rename `Measure::getNote()` to `.getNoteOn()`

### Fixed

---

## [v0.0.3] - 2022-01-13

### New Features

### Improved

- Folder structure: Create a `core` folder separate the C++ side
- Better `README.md`
- `Makefile`: Added the new `make dev` - build and install the Python module
- Update `pybin11_json.hpp` to `v0.2.12`

### Fixed

- `Score::Score`: Added a `std::vector` constructor overload
- `Makefile`: Renamed `make test` to `make tests`

---

## [v0.0.2] - 2022-01-10

### New Features

- New build system using `make` commands
- Parallel build: `make -j8`
- Created a `test` folder for store Python unit tests
- Maialib cross-platform installer: `make install`
- Created VS Code Tasks to `build`, `install` and `debug` code

### Improved

- Add full support to enharmonic pitch values to `midiNote2pitch()` function
- Update `pybind11` library to `v2.9.0`
- Update `nlohmann JSON` library from `v3.9.1` to `v3.10.5`
- Better folder tree structure
- Move pure maialib functions to `Helper` class static members
- Move old `orq_functions.cpp` content to `Score.cpp`
- Remove unnecessary files
- Update doxygen file
- Add `make doc` to the `Makefile` options
- Moved old `mxml_files` folder to `tests/xml_examples` folder

### Fixed

- `Score::forEachNote()` now works as expected
- `midiNote2pitch()` python wrapper default argument

---

## [v0.0.1] - 2021-08-14

### New Features

- A optional _callback function_ to the `Chord::isTonal()` method
- `Score::forEachNote()` method

### Improved

- Update `nlohmann::json` library to the v3.9.1
- Replace all `size_t` variables/arguments to `int` to improve performance
- `Score` constructor now can receive a list of parts as a `std::initializer_list`
- `Score::getChords()` include default arguments

### Fixed

- `Score::setMetronomeMark()` now works as expected
- `Note` constructor works for rest
- `tutorial/create_note_chords.ipynb` correct variable names
- `Measure::getNote()` now return only notes and skip all rests
- `Measure::getElement()` now return notes and rests

---
