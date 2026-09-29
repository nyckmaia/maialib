# Maialib - Change Log

All notable changes to this project will be documented in this file.

## [Unreleased]

### Improve

- Add `Helper.getLibraryVersion()` (C++ and Python) so the compiled `maiacore` library itself can report the version from the root `VERSION` file, matching `maialib.__version__` and `maialib.maiacore.__version__`
- Accept every pitch spelling with `bb`, `b`, `#` and `x` accidentals in octaves -1 to 11 (e.g. `Cbb0`, `Bx9`, `C-1`, `Bx11`)
- Accept the four quarter-tone accidentals `1b`, `1x`, `3b` and `3x` in pitch strings (e.g. `C1x4`, `D3b4`), visible from Python through `Helper.pitch2midiNote()` and `Helper.splitPitch()`; `pitch2midiNote()` rounds the resulting quarter-tone MIDI number to the nearest integer, ties broken upward
- Compute MIDI numbers and enharmonic spellings arithmetically instead of hard-coded tables
- `Note(midiNumber)` accepts the whole spelled range: the old `midiNumber > 127` guard is replaced by the octave range check in `Helper.midiNote2pitch()`
- Add `Helper.splitPitch()` to Python and document the pitch-spelling bindings
- Add the `Pitch` class (C++ and Python): a single pitch held as a diatonic step, an accidental that may be a quarter tone, and an octave — or a rest, whose `getOctave()` is `None`. `Note` is now built on it. In Python the pitch string is its only constructor, `ml.Pitch("C1x4")` (default `"rest"`); a MIDI number or a frequency goes through the named factories `Pitch.fromMidi(midiNumber, accType="")` and `Pitch.fromFrequency(frequency, accType="", freqA4=440.0, enableQuarterToneRound=False)`, because as constructor overloads `ml.Pitch(110)` meant as 110 Hz would silently have become MIDI 110. `ml.Pitch(60)` and `Pitch.fromMidi(60.0)` raise `TypeError`, while `Pitch.fromFrequency(440)` accepts an int. The setters that refuse a value (`setStep`, `setAlter`, `setOctave` below MIDI 0 or on a rest) print a warning, which Python receives on `sys.stdout`
- Add `Helper.steps2pitch()`, `Helper.validateTransposeSemitones()`, `Helper.alterName2symbol()`, `Helper.alterSymbol2Value()`, `Helper.alterValue2symbol()` and `Helper.alterValue2Name()` to Python, documented
- Add `Chord.getStackedHeaps()` and `Chord.isDyad()` to Python — both existed only in C++ — and read-only `note`, `wasEnharmonized` and `enharmonicDiatonicDistance` properties on the `NoteData` objects `getStackedHeaps()` returns, which previously exposed no content at all
- Add `Note.setStep()` and `Note.setAlter()` (C++ and Python), which change one component of the written pitch. Like `Pitch`'s setters, `setStep()` on a rest creates a note in octave 4, while `setAlter()` on a rest, or one that would move the note below MIDI 0, warns and changes nothing
- Add frequency support to the `Pitch` class: `Pitch::getFrequency()`, `Pitch::setFrequency()`, and `Pitch(int midiNumber, accType)` / `Pitch(float frequency, accType, freqA4, enableQuarterToneRound)` constructors. `setFrequency()` always produces a real pitch for a positive, finite frequency — never a rest, never a throw — clamping to the lowest or highest representable pitch (with a warning) when the frequency falls outside the representable range, or when the requested accidental type cannot express the rounded result. `+Infinity` is treated as above the representable range and clamps the same way; `NaN` is a caller error and throws, since it satisfies neither "zero or negative" nor "positive"
- Add `Chord.roundQuarterTones()` (C++ and Python), the escape hatch for analysing a chord that contains quarter tones: it rounds every quarter-tone note to the nearest semitone in place (ties upward, so `E1b4` becomes `E4` and `E3b4` becomes `Eb4`), invalidates the cached stacked-in-thirds analysis so the next analysis call sees the rounded pitches, and returns the number of notes it changed (0 means the chord contained no quarter tone and is musically unchanged). The rounding is destructive: the original quarter-tone spelling is not recoverable from the chord afterwards. Also adds `Note.isQuarterTone()` and `Note.roundToSemitone()` (C++ and Python) — the per-note predicate and rounding that the new guards and `Chord.roundQuarterTones()` are built on, so every quarter-tone rounding follows the same ties-upward rule; `Note.roundToSemitone()` is also the remedy an `Interval` names when it rejects a quarter tone
- Transposition accepts quarter tones. `Note.transpose()`, `Chord.transpose()`, `Chord.transposeStackOnly()` and `Helper.transposePitch()` (C++ and Python) now take a `float` number of semitones instead of an `int`, so `Note.transpose(0.5)` moves a note up by one quarter tone (`C4` becomes `C1x4`). The interval must be a multiple of 0.5 — the finest interval this library can spell — and anything else raises `RuntimeError` naming the value, rather than being silently truncated to an `int`. **Transposing a chord by a quarter tone moves it off the semitone grid, so every harmonic-analysis method will then reject it** (see the quarter-tone rejection entry below); call `Chord.roundQuarterTones()` to analyse such a chord. Also adds `Helper.steps2pitch()` (C++ and Python): the fractional counterpart of `Helper.midiNote2pitch()`, and the one place a quarter tone is spelled from a numeric pitch position
- Add `Note.getQuarterToneSteps()` (C++ and Python): the exact, unrounded sounding pitch position in semitones, so `Note("C1x4").getQuarterToneSteps()` is 60.5 where `getMidiNumber()` gives 61. Notes order by it
- `Pitch.setAlter()` and `Note.setAlter()` accept only an alter exactly on the quarter-tone grid within [-2, 2]: NaN, infinity and near-grid values such as `0.99996` raise, and `-0.0` is stored as `0.0`
- `Pitch.fromFrequency()`, `Pitch.setFrequency()` and `Pitch.getFrequency()` raise when `freqA4` is not a finite number greater than 0
- `Score(path)` reads a MusicXML `<alter>` with a `.` decimal point whatever the C or C++ locale, and only when it is exactly a multiple of 0.5 from -2 to 2: a value near a quarter tone, such as `0.46`, is read as natural with a warning instead of being rounded to it. When a note's `<accidental>` and `<alter>` disagree, the `<accidental>` is used and a warning is printed. `sharp-sharp`, MusicXML's double sharp drawn as two sharps, is read as a double sharp, and `Helper.alterName2symbol()` accepts it
- `Helper.getSemitonesDifferenceBetweenMelodies()` measures quarter-tone intervals exactly, in halves of a semitone (a neutral third against a major third differs by 0.5), so melody pattern searches compare quarter-tone contours; `Score::findAnyMelodyPattern()` keeps two patterns a quarter tone apart as two patterns
- `Pitch.fromFrequency()` and `Pitch.setFrequency()` print a warning, naming the accidental type, when the requested `accType` cannot spell the rounded pitch and the default spelling is used instead, as documented; a double accidental that cannot absorb a quarter-tone remainder is reported the same way, not as a clamp
- The `Chord` methods that build an `Interval` from pairs of the chord's notes (`getIntervals()`, `getIntervalsFromOriginalSortedNotes()`, `haveMajorInterval()` and the other interval predicates) reject a quarter tone with a message that names `Chord.roundQuarterTones()`, a remedy a `Chord` caller can apply, instead of `Interval`'s `Note.roundToSemitone()`, which cannot reach the chord's own notes
- `Note.getEnharmonicPitch()`, `getEnharmonicPitches()`, `getEnharmonicNote()`, `getEnharmonicNotes()` and `toEnharmonicPitch()` (C++ and Python) respell a quarter tone instead of raising "Quarter-tone enharmonic spelling is not supported". Its partners are the white-key steps within 1.5 semitones of it, spelled with a quarter-tone accidental in their own octave, so it has one or two: `C1x4` respells as `D3b4` by default and `B3x3` as the alternative, `C3x4` only as `D1b4`. The default is the partner with the smaller accidental, a tie going to the side opposite the note's own accidental, as `C#4` becomes `Db4`. Semitone spellings are unchanged
- `Pitch` compares by spelling: `ml.Pitch("C4") == ml.Pitch("C4")` is `True` (it compared identity), while `Pitch("C#4") != Pitch("Db4")` — `Helper.isEnharmonic()` compares sounding positions — and a `Pitch` is hashable consistently with it. Add `Pitch(step, alter, octave)` in C++, exposed in Python as the factory `Pitch.fromComponents(step, alter, octave)`, validating every component; and `Pitch.setMidiNumber(midiNumber, accType="")` takes the accidental type `Pitch.fromMidi()` already did
- Document the Python bindings whose behaviour changed on the way to quarter-tone support: every `Note` getter that raises for a sounding pitch below `C1b-1` says so, `Note.getMidiNumber()` and `Note.getFrequency()` say they round a quarter tone, `Interval`'s constructors and `setNotes()` list their rest and below-`C1b-1` errors, and `Score.findMelodyPatternDataFrame()`, `ScoreCollection.findMelodyPatternDataFrame()`, `Helper.getSemitonesDifferenceBetweenMelodies()`, `Helper.calculateMelodyEuclideanSimilarity()` and `Helper.pitch2freq()` are documented. `Pitch.fromFrequency()` and `Pitch.setFrequency()` describe what happens at the ends of the range: with the default `accType` every frequency from MIDI 156 up becomes `B11`

### Fix

- `Note.setOctave()`, `Note.setStep()`, `Note.setAlter()` and `Note.setIsNoteOn()` print the warning for a value they refuse on Python's `sys.stdout`, where a notebook shows it and `contextlib.redirect_stdout()` captures it, as `Pitch`'s setters do; it went to the C++ standard output
- **Breaking:** `Score.findMelodyPatternDataFrame()` with a list of patterns — and the C++ `Score::findMelodyPattern()` list overload it and `ScoreCollection::findMelodyPattern()` use — searched only as many patterns as the machine has hardware threads and returned the rest empty, and when a pattern's search failed it printed the error and returned that pattern's results empty. Every pattern is now searched, and a failing search raises its error, as the single-pattern overload does
- **Breaking:** A note whose sounding pitch falls below the lowest representable pitch, `C1b-1` — a transposing instrument's note written in octave -1, such as a B-flat clarinet's written `C#-1` — stays constructible, and `isNoteOn()` is still `True`, but every sounding getter (`getPitch()`, `getSoundingPitch()`, `getMidiNumber()`, `getOctave()`, `getSoundingOctave()`, `getPitchClass()`, `getPitchStep()`, `getAlterSymbol()`, `getFrequency()`, `getQuarterToneSteps()`, …) raises one error naming the written pitch and the interval, instead of answering a rest's values (`-1`, `None`, `"rest"`, `""`, `0.0`). An `Interval` with such a note is refused with the same error when it is built, instead of raising a bare `bad optional access`. The written getters are unchanged
- **Breaking:** `Helper::alterValue2symbol()` and `Helper::alterValue2Name()` accept only an alter that is exactly a multiple of 0.5 from -2 to 2 and raise, naming the value, for any other — a value within 0.05 of one used to be rounded to it — and no longer depend on the global C++ locale, under a comma-decimal one of which they raised for every value. `-0.0` is the natural
- **Breaking:** `Helper.spelling2midiNote()` raises when `alterValue` is NaN, infinite or outside [-2, 2], or `octave` is outside [-1, 11] — the components of a pitch this library can spell — instead of converting an out-of-range sum to `int`, which is undefined behaviour
- `maialib.__version__` and `maialib.maiacore.__version__` no longer carry literal quote characters; the value still comes from the root `VERSION` file
- `Helper.pitch2midiNote("Db10")` returned 132 instead of 133
- `Note.setPitch()` kept the previous accidental symbol and rejected multi-digit octaves
- `Helper.isEnharmonic()` did not detect E#/F, B#/C and Cb/B (this also affected `Helper.noteSimilarity()`)
- `Interval` direction and diatonic interval were wrong for notes outside C0–C10
- `Helper.midiNote2pitch()` could return spellings outside the supported octaves
- `Note`'s setters could leave a note contradicting itself, because each getter read its own stored copy of the pitch. `Note` now holds one canonical `Pitch` and derives every getter from it. Fixed with it: `setPitchClass()` kept the previous MIDI number (`Note("C4").setPitchClass("Bb")` reported 60, now 70); `setTransposingInterval()` applied the chromatic interval to the MIDI number again on every call (called twice on a written `C4` with -2 it reported 56, now 58); `setIsNoteOn(False)` only flipped a flag, so `getPitchClass()`, `getMidiNumber()` and `getPitch()` kept reporting the old pitch — it now makes the note a rest everywhere; and `setIsNoteOn(True)` on a rest marked a note sounding with no pitch (`getPitch()` gave `"rest0"`) — it now warns and leaves the rest unchanged, since a flag cannot supply a pitch
- A rest's `getPitchStep()`, `getWrittenPitchStep()`, `getSoundingPitchStep()`, `getDiatonicWrittenPitchClass()` and `getDiatonicSoundingPitchClass()` returned `"r"`, the first character of `"rest"`; they now return `"rest"`. Also, `Note.setPitchClass()` on a rest now creates a note in octave 4 (`Note("rest").setPitchClass("Bb")` is `Bb4`) where it used to leave the rest unchanged, matching `setPitch()` and `setStep()`
- `Note.getPitch()` and `Note.getSoundingPitch()` returned a transposing instrument's stale sounding pitch (e.g. `"Bb3"`) for a note silenced with `Note.setIsNoteOn(False)`, because silencing a note deliberately keeps its transposing interval; both now return `"rest"`, matching the untransposed case and `Note.setPitch("rest")`
- `Score(path)` (MusicXML read) discarded quarter-tone accidentals: the note-construction path never read `<accidental>` at all, and its `<alter>` fallback only matched the four integer semitone strings, so a decimal alter like `0.5` was silently read as natural. Now reads `<accidental>` first (where quarter tones live), then the decimal `<alter>` value, else natural — the same precedence MuseScore's own historical export (glyph with no matching `<alter>`) requires
- Loading MusicXML with `Score(path)` now prints a warning when it cannot represent a note's accidental, instead of silently reading the note as natural. An `<alter>` value this library cannot spell (e.g. a triple sharp, `3`, or an eighth tone, `0.25`) still leaves the note natural — off by that amount — but now says so; an `<accidental>` name it cannot spell (MusicXML defines roughly 40; `Helper.alterName2symbol()` knows 13) warns and falls back to `<alter>`, then to natural. Neither aborts the load
- `Note.toXML()` (MusicXML write) discarded quarter-tone accidentals on export: the written pitch's `<alter>` value was truncated to an integer before being written (`<alter>0</alter>` for any quarter tone), in both the pitched and unpitched (percussion) branches. `<alter>` now writes the pitch's real value for every accidental, quarter tone or not: an integer with no decimal part for whole-tone accidentals (`1`, `-2`), one decimal place for quarter tones (`0.5`, `-1.5`) — never `std::to_string(float)`'s six decimals. `Note.toXML()` also now emits an `<accidental>` element (immediately after `<type>`, using the Tartini names `Helper.alterValue2Name()` maps quarter tones to) specifically for quarter tones — a key signature can never imply one, so the glyph must always be written explicitly. A whole-tone accidental (`#`, `b`, `x`, `bb`) still emits only `<alter>`, no `<accidental>`, since a key signature can already imply it (e.g. an F# in D major needs no glyph); a score containing no quarter tones therefore re-exports with the same `<alter>`/`<accidental>` shape it had before this fix. Because `Measure.__hash__` (Python) hashes `Measure.toXML()`'s output string, a `Measure` containing a quarter-tone note now hashes differently than before this fix — expected for anyone keeping a `Measure` in a `set` or using one as a `dict` key, not a regression
- Harmonic analysis silently produced confident wrong answers for quarter tones instead of rejecting them. Every answer the stacked-in-thirds analysis computes — the thirds, the fifths, the chord quality, the scale degrees — is defined over twelve-tone equal temperament, so a quarter tone never made the analysis fail; it made it answer wrongly, which in an analysis library is worse than an error. `Chord` now rejects at the single chokepoint every analysis method funnels through, so `getName()`, `getQuality()`, `getRoot()`, `getBassNote()`, `getDegree()`, `getRomanDegree()`, `stackSize()`, `getStackedHeaps()`, the open/close stack getters, the interval getters, the `is*` family and the `have*` predicates all raise `RuntimeError` on a chord containing a quarter tone. `Interval` rejects a quarter tone at construction (both constructors and both `setNotes()` overloads), which additionally covers the `Chord.have*` overloads that build intervals from the original notes instead of from the stack. Both messages name the offending note and the remedy — `Chord.roundQuarterTones()` for a chord, `Note.roundToSemitone()` for a single note — so a caller can act on the error without consulting the documentation. Note that `Chord.getName()` *raises* here although it only warns and returns an empty string for a non-tonal chord: that inconsistency is deliberate, because a quarter tone is not an atonal chord but input the analyser cannot represent at all, and returning empty would hide the loss silently. Plain accessors and mutators (`size()`, `getNote()`, `getNotes()`, `getDuration()`, `setDuration()`, `addNote()`, `removeNote()`, `removeTopNote()`, `print()`, `printStack()`, `clear()`) never touch the stacked-in-thirds representation and keep working unchanged on a quarter-tone chord. `Chord.info()` also keeps working, degrading rather than raising: it still prints the size and the full note list, replaces the name with a note that the harmonic analysis is unavailable, skips the stack section and names `roundQuarterTones()` — so a user can always inspect a chord they do not yet understand, which is exactly when `info()` gets called
- A third family of `Chord` methods answered quarter tones wrongly without ever reaching either of the guards above, because it works in the MIDI integer domain and never builds an `Interval` or stacks the chord in thirds: `Chord.getMidiIntervals()` returned `[4, 3]` for `["C4", "E1b4", "G4"]`, a triad with a neutral third — byte for byte what a plain C major triad returns. Each such method is now settled by one question, whether its **return type can express a quarter tone**. Those that cannot now raise `RuntimeError`, naming the offending note and `Chord.roundQuarterTones()` exactly as the analysis chokepoint does: `getMidiIntervals()`, `getMeanMidiValue()` and `getMeanOfExtremesMidiValue()`, plus `getMeanPitch()` and `getMeanOfExtremesPitch()`, which spell the integer mean and so inherit the rejection. (A pitch string can spell a quarter tone, but the value being spelled is the mean of N notes — a multiple of 1/(2N), generally not a multiple of 0.5 — so there is no exact spelling to return.) Those that **can** express one now compute the true value instead: `isSorted()` compares exact positions, so `["E1b4", "E4"]` is correctly sorted where it previously reported `False` (both notes rounded to 64); `getHarmonicDensity()` takes its auto-detected extremes from exact positions, so `["C1x4", "G4"]` spans 6.5 semitones rather than 6; and `getMidiValueStd()` likewise uses exact positions. Note that `maialib.plotScorePitchEnvelope()` draws on the mean family, so plotting a score that contains quarter tones now raises instead of drawing a silently skewed envelope — call `Chord.roundQuarterTones()` on the affected chords to plot it anyway
- **Breaking:** `Chord.toCents()` no longer raises on a chord containing a quarter tone — it was the one method rejecting a quarter tone it can represent perfectly well. Cents are the only unit in this library that expresses a quarter tone exactly (50 cents to the quarter tone, 350 to the neutral third), so `["C4", "E1b4", "G4"]` now returns `[350, 350]` instead of raising. It is now computed as integer arithmetic on exact step positions rather than from the two notes' frequencies, because `Note.getFrequency()` derives the frequency from the *rounded* MIDI number, which reported a neutral third as 400 cents. A side effect for chords with no quarter tone: intervals are now exact multiples of 100, where the frequency route truncated rather than rounded — a C major triad now reads `[400, 300]` instead of `[400, 299]`
- **Breaking:** `Chord.getMidiValueStd()` returned the standard deviation of a list padded with a leading run of zeros — it sized its vector to the note count and then appended the real values after them — so a C major triad reported `31.8978` (the deviation of `{0, 0, 0, 60, 64, 67}`) instead of `2.8674`. The value was wrong for every chord, quarter tone or not; it now reports the real standard deviation of the chord's pitches, and an empty chord returns `0.0` instead of `NaN`. `Chord.getFrequencyStd()` had the identical padding defect and is fixed the same way — see the next entry
- **Breaking:** `Chord.getFrequencyStd()` had the same zero-padding defect as `getMidiValueStd()`, so its value too was wrong for every chord, quarter tone or not: it sized its vector to the note count and then appended the real values after it, averaging a leading run of zeros. A C major triad reported `168.14` — the standard deviation of `{0, 0, 0, 261.63, 329.63, 392.00}` — instead of the true `53.24`; an empty chord returned `NaN` and now returns `0.0`. This is **not** the frequency-rounding issue: `Note.getFrequency()` still derives its value from the rounded MIDI number, so a quarter tone still contributes the frequency of the semitone above it, and repairing that would not have repaired this. The padding is arithmetic, independent of where the frequencies come from
- **Breaking:** `Note`'s ordering operators (`<`, `>`, `<=`, `>=`, and therefore Python's `<`, `>`, `<=`, `>=` on `Note`) compared the *rounded* MIDI number, so `Note("E1b4") < Note("E4")` was `False` although 63.5 is genuinely below 64. They now compare exact pitch positions. The visible consequence is that `Chord.sortNotes()` actually sorts a chord containing a quarter tone: sorting `["E4", "E1b4"]` previously left it untouched — `std::sort` saw two equal notes — after which `Chord.isSorted()` reported `False`, a chord that `sortNotes()` could not make sorted. Any chord or comparison without a quarter tone is completely unaffected, since exact and rounded positions coincide there. `Note.__eq__`/`__ne__` compare pitch strings and were already correct
- `Chord.getHarmonicDensity(lowerBoundPitch, higherBoundPitch)` rounded a quarter-tone bound, because it converted the pitch strings with `Helper.pitch2midiNote()` before delegating to the numeric overload: `getHarmonicDensity("C1x4", "G4")` reported `2/7` where the auto-detected numeric path already reported the true `2/7.5`. Both overloads now share one exact-range computation
- A transposing instrument's sounding pitch was computed by a lookup through four hard-coded twelve-entry scales (sharp, flat, double-sharp, double-flat) indexed by written pitch class, which carried three defects at once and could not represent a quarter tone at all. It is now derived arithmetically from the written pitch and the chromatic interval, and the scales are gone. Fixed by the rewrite: (1) **wrong pitch class** — a written pitch class found only in the sharp scale still indexed the flat array, at its end, so a B-flat clarinet's written `C#4` sounded `Bb3` and a horn in F's written `F#4` sounded `F3`, where both sound MIDI 59, which is `B3`; (2) **missing octave change at exactly ±12 semitones** — the octave-wrap check did not fire for a full-octave transposition, so a piccolo's written `C4` reported octave 4 instead of 5, and `Note.getOctave()` disagreed with `Note.getSoundingOctave()` for every transposing instrument; (3) **quarter tones threw** `Unknown note type` — a pitch class such as `C1x` is in none of the four scales, and the message named neither the pitch class nor the interval. A quarter-tone pitch on a transposing instrument now simply sounds: written `C1x4` on a B-flat clarinet sounds `B1b3`. `Note.getMidiNumber()` is unaffected — it has been arithmetic since the sounding-pitch rework, which is why it stayed correct while the spellings around it did not
- `Helper.isEnharmonic()` compared the *rounded* MIDI numbers of the two pitches, so a quarter tone and the semitone it rounds to were reported as the same pitch: `isEnharmonic("C1x4", "C#4")` was `True`, although 60.5 and 61 are different pitches. It now compares exact pitch positions, so that pair is `False` while the two spellings of one quarter tone (`C1x4` and `D3b4`) are `True`. Semitone-only comparisons are unchanged
- `Chord.transpose()` and `Chord.transposeStackOnly()` each carried their own copy of the transposition instead of calling `Helper.transposePitch()`, so transposing a `Chord` and transposing a `Note` diverged; all three now share one implementation. The visible effect is that a chord containing a quarter tone keeps it: transposing `["C1x4", "E4", "G4"]` up two semitones gave `["D4", "F#4", "A4"]`, silently rounding the first note, and now gives `["D1x4", "F#4", "A4"]`
- **Breaking:** A pitch transposed below the lowest representable pitch silently became a rest — on `main` too, not only on quarter tones: `Helper.transposePitch("C4", -61)` returned `"rest"`, so `Note.transpose()` and `Chord.transpose()` silently deleted a note transposed too low (`Chord(["C4", "C-1"]).transpose(-1)` gave `["B3", "rest"]`). Transposing a real pitch outside the representable range, from `C1b-1` (-0.5, the lowest pitch this library holds) to `Bx11` (157), now raises `RuntimeError` naming the pitch, the interval and the range; transposing a rest still returns `"rest"`. Three neighbouring defects, also present on `main`, are fixed with it: `Chord.transpose()` and `Chord.transposeStackOnly()` stored each note as they went, so a note that raised part-way through left the chord half-transposed (`["C4", "B11"]` transposed by 2 was left as `["D4", "B11"]`) — every note is now transposed before any is stored, so a rejection leaves the chord unchanged; `Helper.transposePitch("", 2)` returned `"C#-1"`, because only the exact string `"rest"` was recognised as a rest — every rest spelling now transposes to `"rest"`; and an interval of 0 returned an invalid pitch string unchanged — the pitch is now parsed first, so `transposePitch("H4", 0)` raises like `transposePitch("H4", 1)` always did

### Removed

- **Breaking:** `Helper.pitch2number()` (C++ and Python) and `Helper::number2pitch()` (C++) — use `Helper.pitch2midiNote()` for the numeric value and `Helper.isEnharmonic()` for the comparison it was used for
- Unused C++ constants `MUSIC_XML::MIDI::NUMBER::MIDI_000` … `MIDI_132` and `c_pianoWhiteKeys`
- **Breaking:** Renamed the misspelled `alternativeEnhamonicPitch` keyword argument to `alternativeEnharmonicPitch` on `Note.getEnharmonicPitch()`, `Note.getEnharmonicNote()` and `Note.toEnharmonicPitch()` (C++ and Python) — callers passing it by keyword must update the spelling
- **Breaking:** A rest has no octave: `Helper.splitPitch()` returns `None` (not `0`) for the octave of a rest, e.g. `ml.Helper.splitPitch("rest")` is now `("rest", "rest", None, 0.0, "")`; `Helper.midiNote2octave()` returns `None` (not `-2`) for a negative MIDI number. In C++, `Helper::splitPitch()`'s `octave` output parameter is now `std::optional<int>&` and `Helper::midiNote2octave()` now returns `std::optional<int>`, empty instead of `-2` for `MIDI_REST`
- **Breaking:** `Note.getAlterSymbol()` now returns the sounding pitch's accidental symbol; previously it returned the written pitch's. Affects transposing instruments only — a written pitch's accidental symbol is unchanged for a non-transposing (or untransposed) `Note`
- **Breaking:** `Note.setOctave()` no longer recomputes the sounding octave arithmetically, so `Note.getOctave()` after a `setOctave()` call now reports the same octave it reports on construction. Affects transposing instruments only: for a written `C#4` on a B-flat clarinet, calling `setOctave(4)` — the note's own current octave, a no-op value — previously changed `getOctave()` from 4 to 3; it now leaves it unchanged, at 3, the octave of the sounding `B3`. The previous behaviour was history-dependent (constructing a note and then re-setting its own octave reported two different octaves for an unchanged note), and the 4 it reported on construction came from the transpose scale-lookup defect that the sounding-pitch entry under Fix removes. `Note.getSoundingOctave()`, `Note.getMidiNumber()` and `Note.getPitch()` are computed arithmetically and are unaffected
- **Breaking:** A rest has no octave, on `Note` too: `Note.getOctave()`, `Note.getWrittenOctave()` and `Note.getSoundingOctave()` return `int | None`, `None` for a rest instead of the numeric sentinel `-2` (which was itself unsound: `0` and `-1` are both legitimate octaves). Every non-rest input is unaffected — the same `int` value as before, now wrapped in an engaged optional. In C++, all three now return `std::optional<int>`. Also fixes `Pitch::setOctave()` (and, through it, `Note::setOctave()`) to warn and refuse — never throw — when called on a rest with an out-of-range octave; previously the range check ran before the rest check, so a rest given, e.g., the old `-2` sentinel back would throw instead of being refused like every other rest mutation attempt. `Note.getSoundingOctave()` can also be `None` for a *sounding* (non-rest) note: an ordinary, constructible transposed note whose sounding pitch falls below the representable minimum C-1/MIDI 0 (e.g. a B-flat clarinet's written `C#-1`) has no sounding octave either — `isNoteOff()` alone does not cover this case, only `has_value()`/`is None` does. `Note.getSoundingPitch()`/`Note.getPitch()` now fail with a diagnosable `RuntimeError` naming that condition for such a note, instead of an unexplained internal error (or, pre-Task-6b, a malformed pitch string built from the same `-2` sentinel)
- **Breaking:** Removed `Helper::semitonesBetweenPitches()` from the public C++ API. It subtracted the two pitches' *rounded* MIDI numbers and returned an `int`, so it was silently wrong for quarter tones (`C1x4` to `C#4` measured 0 semitones), and it had no caller anywhere in the repository. It was never bound to Python, so the Python package is unaffected; `Pitch::getQuarterToneSteps()` gives the exact difference
- **Breaking:** Removed `Score::getNote()` (three overloads) and `Score::getNoteNodeData()` from the public C++ API. Both were dead code: `Score::getNote()`'s only callers were its own shorter overloads delegating to the longest one, and `Score::getNoteNodeData()` had no callers at all. Also removes the unimplemented `Helper::getNoteNodeData()` declaration, which had no definition anywhere in the repository. None of the three was ever bound to Python, so the Python package is unaffected

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
