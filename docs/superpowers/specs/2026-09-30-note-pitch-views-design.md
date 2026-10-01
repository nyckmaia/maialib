# Note pitch views and diatonic transposition (roadmap step 1a) — Design

**Status:** decisions approved by the user on 2026-09-30 (answers recorded below).
**Roadmap:** step 1a of `C:/Users/nyck/.claude/plans/wobbly-coalescing-quill.md`. Step 1b (`<transpose>` read/write, `octave-change`, concert key) builds on this.

## 1. Problem

Measured on `main` @ `a287fcf`:

- `Note` has three getter families: `Written*`, `Sounding*`, and unprefixed getters. Today the unprefixed pitch getters are aliases of `Sounding*` (`getPitch()` has returned the sounding pitch since v1.x).
- The sounding spelling ignores `transposeDiatonic`: `soundingPitchOf` (`note.cpp`, anonymous namespace) spells the position reached by the chromatic interval with sharps going up and flats going down. A B-flat clarinet's written `F#4` sounds `"Fb4"`, `G#4` → `"Gb4"`, `D#4` → `"Db4"`; a horn in F's written `B4` → `"Fb4"`; up a major second `Db4` → `"D#4"`; a piccolo's written `Bb4` → `"A#5"`.
- `getSoundingPitch()` glues a spelled pitch class to an arithmetic octave, which breaks for spellings that cross an octave (Cb, B#); `getOctave()` (spelled) and `getSoundingOctave()` (arithmetic) disagree for them.
- `Note::transpose` reads the sounding pitch and writes it as the written pitch, so on a transposing instrument the interval is applied twice (`transpose(0)` moves the note); `toEnharmonicPitch` stores a sounding respelling as the written pitch, moving the sounding pitch.

## 2. Decisions (user, 2026-09-30)

**D1 — three pitch views.**
- **Written:** the pitch as written in the part — `getWrittenPitch`, `getWrittenOctave`, `getWrittenPitchClass`, `getWrittenPitchStep`, `getDiatonicWrittenPitchClass`.
- **Unprefixed pitch-spelling getters are shortcuts for Written:** `getPitch` = `getWrittenPitch`, `getOctave` = `getWrittenOctave`, `getPitchClass` = `getWrittenPitchClass`, `getPitchStep` = `getWrittenPitchStep`; `getAlterSymbol` is the written pitch's accidental; the enharmonic family (`getEnharmonicPitch(es)`, `getEnharmonicNote(s)`, `toEnharmonicPitch`) works on the written pitch.
- **Sounding:** what sounds, in its simplest spelling (D3), with the octave of that spelling — `getSoundingPitch`, `getSoundingOctave`, `getSoundingPitchClass`, `getSoundingPitchStep`, `getDiatonicSoundingPitchClass`.

**D2 — acoustic getters stay sounding (concert), as today:** `getMidiNumber`, `getFrequency`, `getQuarterToneSteps`, `getHarmonicSpectrum`. A B-flat clarinet's written `D4` has MIDI 60 and C4's frequency.

**D3 — simplest spelling.** Among the spellings of the same exact position, take the one with the smallest `|alter|`; on a tie, keep the direction (sharp side / flat side) of the spelling being simplified. Semitones: `Db4`→`Db4`, `C#4`→`C#4`, `Cb4`→`B3`, `E#4`→`F4`, `B#3`→`C4`, `Fb4`→`E4`, `Ebb4`→`D4`, `Fx4`→`G4`, `Bbb4`→`A4`. Quarter tones: `C1x4`→`C1x4` (0.5 beats `D3b4`'s 1.5), `C3x4`→`D1b4`, `E1x4`→`E1x4` (tie with `F1b4`, keeps the sharp side), `B1b3`→`B1b3`. The octave is that spelling's own octave (`Cb4` sounds `B3`, octave 3).

**D4 — concert spelling (internal view).** The written pitch moved by the transposing interval, with the letter taken from the diatonic interval (written step + `transposeDiatonic`, carrying octaves) and the alter from the exact position (written position + `transposeChromatic`). For an untransposed note it is the written pitch. **Fallback, silent and documented:** when `transposeDiatonic == 0` while `transposeChromatic != 0` (e.g. a MusicXML file without `<diatonic>`), or when the resulting alter is not one of the nine representable accidentals, or the octave falls outside −1..11, the current chromatic rule spells the position. The speller must not throw in normal operation (the current one uses try/catch on every downward transposition).
- The **Sounding** view is D3 applied to the concert spelling: B-flat clarinet written `Db4` → concert `Cb4` → sounding `B3`; written `Eb4` → concert `Db4` → sounding `Db4` (tie with `C#4`, keeps the flat side); written `C1x4` → concert `B1b3` → sounding `B1b3`.

**D5 — analyses use the concert spelling (D4), not the simplified Sounding spelling.** Everything that relates pitches — `Chord` analyses, `Interval`, chord extraction from a `Score`, melody-pattern search — works on the concert spelling. For untransposed notes this is exactly today's written spelling, so no interval or chord name changes (`Interval(C4, Cb5)` stays a diminished octave; `Chord(Ab4, Cb5, Eb5)` stays A-flat minor); for transposing parts it is the correct concert pitch. The concert view stays **internal**: no Python binding and no documented public API (C++ access for `Chord`/`Interval`/`Score` through the narrowest mechanism the code allows, e.g. a `detail` function or friendship). Python-side displays (maiapy plot labels, dissonance labels) have no concert view and use the public Sounding view.

**D6 — mutators work on the written pitch.** `transpose()` transposes the written pitch once (`transpose(0)` changes nothing); `toEnharmonicPitch()` respells the written pitch, so the sounding position never moves. Existing strong exception guarantees stay.

**D7 — the fallback of D4 is silent** (no warning), documented in the Doxygen and numpydoc of the Sounding getters.

## 3. Examples (after this step)

| Case | `getPitch()` = `getWrittenPitch()` | concert (internal) | `getSoundingPitch()` / octave | `getMidiNumber()` |
|---|---|---|---|---|
| Untransposed `Cb4` | `Cb4` | `Cb4` | `B3` / 3 | 59 |
| B♭ clarinet (d=−1, c=−2) written `F#4` | `F#4` | `E4` | `E4` / 4 | 64 |
| B♭ clarinet written `Db4` | `Db4` | `Cb4` | `B3` / 3 | 59 |
| B♭ clarinet written `C1x4` | `C1x4` | `B1b3` | `B1b3` / 3 | 59 (58.5 rounds up) |
| Horn in F (d=−4, c=−7) written `B4` | `B4` | `E4` | `E4` / 4 | 64 |
| Piccolo (d=7, c=12) written `Bb4` | `Bb4` | `Bb5` | `Bb5` / 5 | 82 |
| No `<diatonic>` (d=0, c=−2) written `C4` | `C4` | fallback `Bb3` | `Bb3` / 3 | 58 |
| Rest | unchanged | — | unchanged | −1 |

## 4. Internal consumers

Every internal use of an unprefixed pitch getter must be reviewed and switched deliberately, because its meaning changes for transposed notes: `maiacore/src/maiacore/chord.cpp` (~40 uses), `interval.cpp` (~8), `score.cpp` (~2), the pybind11 bindings (e.g. `__repr__`), and maiapy (`plots.py` ×3, `sethares_dissonance.py` ×1). Rule: pitch relationships → concert view (D5); Python display → Sounding; the part itself (MusicXML output, anything that must round-trip the written part) → Written. The per-site decision list is part of the implementation report.

## 5. Tests and measurement

- **Measure before changing:** a table of every getter's value for every written spelling (semitone and quarter-tone accidentals, octaves 3–5) × transposing intervals (−1,−2) (−2,−3) (−4,−7) (−5,−9) (1,2) (2,3) (7,12) (−7,−12) (−8,−14) (0,−2) (0,0), before and after; and the chord and interval names produced from the repository's fixtures and samples before and after. For untransposed notes the names must be identical; every difference for transposing parts is listed.
- The characterisation tables (`quarter-tone-characterization-data.h`, `pitch-spelling-legacy-data.h`) stay byte-identical and green (they pin untransposed notes).
- Every changed test expectation is listed (old → new, with the decision that justifies it); reviewers reject unlisted changes. Every new test is proven to fail under a targeted mutation.

## 6. Breaking changes

- `getPitch`, `getOctave`, `getPitchClass`, `getPitchStep`, `getAlterSymbol` and the enharmonic family describe the written pitch (they described the sounding pitch) — different only for transposed notes. The notes `getEnharmonicNote(s)` return keep the transposing interval: their `getPitch()` is the respelled written pitch, and they sound like the note.
- `getSounding*` return the simplest spelling and its octave (`Cb4` → `B3`, octave 3) — different for untransposed notes spelled with Cb, Fb, E#, B#, a double accidental or a three-quarter-tone accidental (`C3x4` → `D1b4`), and for transposed notes. `getSoundingOctave` is the spelling's octave, not the MIDI number's: `B1x3` → 3 (was 4); `B1x11`, `B#11`, `B3x11`, `Bx11` → 11 (was 12).
- `transpose()` moves the written pitch once; `toEnharmonicPitch()` respells the written pitch.
- Analyses of transposing parts use correct concert spellings (chord and interval names may change there). The notes the chord analysis returns (root, bass note, stacks) are untransposed notes at concert pitch; `Chord(notes)` holds its notes untransposed at their concert spelling; `removeDuplicateNotes()` pairs notes by concert spelling.
- `Note` `==`/`!=` compare the concert spelling (a B♭ clarinet's written `D4` equals a `C4`); the `Note` and `Chord` hashes hash exactly what `==` compares; `Note.__repr__`/`info()` show the written pitch, and `__repr__` no longer raises for a note sounding below `C1b-1`; `Chord`'s representations show the notes at concert spelling (unchanged for untransposed chords); `getScaleDegree()` reads the written step.
- Edges of the range: `toEnharmonicPitch()` and `getEnharmonicNote(s)` raise at the very top when the respelling cannot be spelled once transposed (`A#11` on a (1, 2) instrument: `Bb11` would need `C12`); a `Chord` display raises for a note sounding below `C1b-1`.

## 7. Out of scope

`<transpose>` reading/writing, `octave-change`, `double`, the concert key in `getChords` (step 1b); a Python-visible concert view.

## 8. Verification

`make cpp-tests`, `make py-tests` (brand-new venv, `make dev`), `make validate`, `make msvc-gate`; `make linux-gate` when WSL has `cmake`.
