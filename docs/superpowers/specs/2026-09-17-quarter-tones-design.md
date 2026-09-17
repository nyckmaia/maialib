# Quarter Tone Support (SP2) — Design

**Status:** approved 2026-09-17
**Predecessor:** SP1 pitch spelling (`2026-09-15-pitch-spelling-design.md`), merged
**Successor:** SP3 tuning-aware frequency

## 1. Goal

Make quarter tones first-class in maiacore: parseable, representable, transposable, sounding, and round-trippable through MusicXML. Introduce a `Pitch` class that owns every representation of a pitch coherently, and remove the three truncation points and two broken MusicXML read paths that currently discard quarter-tone information.

## 2. Scope

In scope: the `Pitch` class; `Note` composing it; quarter-tone parsing and spelling; MusicXML read and write; enharmonic equivalence; transposition; an explicit rejection contract for harmonic analysis; Python bindings for everything new or changed.

Out of scope, with reasons:

- **Tuning systems other than 12-TET.** `Pitch::getFrequency()` reads the global `TuningSystem` and the hook is designed here, but only equal temperament is implemented. SP3 adds just intonation, Pythagorean, meantone and Werckmeister, and settles the reference-tonic question that only non-equal tunings raise (§11).
- **Naming quarter-tone intervals** (neutral seconds, thirds, sixths, sevenths) and extending chord analysis to them. That is a sub-project in its own right, larger than the whole of SP2. Analysis instead rejects quarter tones explicitly (§8).
- **The `_originalNotes` / `_openStack` dual representation in `Chord`.** Known defective and tracked separately; not touched here.

## 3. Background: this is a half-built implementation, not a greenfield feature

Reading the code before designing produced the finding that shapes everything below: **the value layer already speaks quarter tones.**

- `Helper::alterSymbol2Value` (`helper.cpp:291`) already maps `"1x"`→+0.5, `"3x"`→+1.5, `"1b"`→−0.5, `"3b"`→−1.5.
- `Helper::alterValue2Name` (`:320`) and `Helper::alterName2symbol` (`:398`) are exact inverses over nine values and already cover all four quarter tones.
- `score.cpp:1431-1437` holds commented-out quarter-tone write code.

Three things block it:

1. **`c_alterSymbol`** (`constants.h:103`) is `{"bb","b","#","x"}`. It is the whitelist `Helper::splitPitch` validates against (`helper.cpp:1418`), so `"C1x4"` is rejected at parse time by the `LOG_ERROR` at `:1419`.
2. **Two `static_cast<int>(alterValue)` truncations**, at `helper.cpp:249` and `note.cpp:315`. Both carry `// SP2:` comments left deliberately by SP1 pointing at this work.
3. **`<alter>` is written as an int**, at `note.cpp:766` and `:784`.

And there are **two independent MusicXML read paths, broken differently**:

- `score.cpp:646-661` — the main note loader switches on the raw `<alter>` string with only the cases `"-2"`, `"-1"`, `"1"`, `"2"`. A decimal such as `"0.5"` matches nothing, the accidental symbol stays empty, and **the note silently becomes natural**. This path ignores `<accidental>` entirely.
- `score.cpp:1644-1651` — `Score::getNoteNodeData` prefers `<accidental>` (correct) but falls back to `atoi("0.5")` = 0, and declares `alterValue` as `int&` (`score.h:438`), a type that cannot hold 0.5.

**The parser itself needs no new logic.** `Helper::splitPitch` (`helper.cpp:1374-1433`) locates the octave by walking backwards over trailing digits (`:1396-1399`) and takes everything between the step and that point as the accidental (`:1415`). Traced against real inputs: `"C1x4"` stops the backward scan at `x`, yielding octave `"4"` and symbol `"1x"`; `"C3b"` has no trailing digit, so the octave defaults to 4 with symbol `"3b"`; `"C1"` consumes `1` as the octave with an empty symbol — unambiguous, because quarter-tone symbols are always two characters and never a bare digit; `"C1x-1"` works through the sign handling at `:1401`. **Widening `c_alterSymbol` from four entries to eight is the entire parsing change.**

## 4. The `Pitch` class

### 4.1 State

`Pitch` stores exactly three values and nothing else:

```cpp
std::string _step;    // "C".."B"
float       _alter;   // multiple of 0.5, in [-2, +2]
int         _octave;
```

Everything else — MIDI number, pitch class, pitch string, frequency — is **computed on demand**. There is **no cache**.

Rationale, and a correction of an earlier draft: caching was considered and rejected after asking what a frequency actually costs. In 12-TET it is `freqA4 * 2^((midi-69)/12)`, one `pow()` call. The other tuning systems are *cheaper*, not dearer — a twelve-entry ratio table and a multiply for just, Pythagorean and meantone; a fixed cents table for Werckmeister III. No search, no allocation, no iteration. A cache would have bought nothing measurable while costing a key struct, a `mutable` member and a body of invalidation reasoning — the exact shape of bug this project spent 2026-09-16 removing from `Chord`. If profiling ever justifies memoisation, it can be added as a private detail without changing a signature.

### 4.2 Interface

Constructors, setters and getters exist for **every** representation, because a caller should be able to build a `Pitch` from whatever they have. The invariant that keeps this safe: **a setter per representation is not a stored field per representation.** The non-canonical setters convert *into* `(step, alter, octave)`.

```cpp
Pitch(const std::string& step, float alter, int octave);
explicit Pitch(const std::string& pitch);                    // "C1x4"
Pitch(int midiNumber, const std::string& accType = "");      // 61 -> C#4 or Db4
Pitch(float frequency, const std::string& accType = "",
      float freqA4 = 440.0f, bool enableQuarterToneRound = false);

std::string getPitch() const;           // "C1x4"
std::string getPitchClass() const;      // "C1x"
std::string getPitchStep() const;       // "C"
std::string getAlterSymbol() const;     // "1x"
float       getAlter() const;           // 0.5
int         getOctave() const;
int         getMidiNumber() const;      // nearest integer
float       getQuarterToneSteps() const;// exact, unrounded
float       getFrequency(float freqA4 = 440.0f) const;
bool        isRest() const;

void setStep(const std::string&);
void setAlter(float);                   // validates multiple of 0.5
void setOctave(int);
void setPitch(const std::string&);
void setPitchClass(const std::string&);
void setMidiNumber(int, const std::string& accType = "");
void setFrequency(float, const std::string& accType = "",
                  float freqA4 = 440.0f, bool enableQuarterToneRound = false);
```

To be precise about what the last two setters do: they **convert and discard**. `setMidiNumber` resolves a MIDI number to `(step, alter, octave)` and stores only that; it does not keep the integer. `setFrequency` does the same with a frequency. Neither MIDI nor frequency is ever stored as authoritative state — they are views on the canonical triple, and letting either become the source of truth would recreate the two-sources-of-truth problem this class exists to avoid.

### 4.3 Inverse conversions carry explicit policies

Neither MIDI nor frequency maps back to a spelling bijectively.

**From MIDI:** 61 is `C#4` or `Db4`. Resolved by an accidental preference, exactly as the existing `Note(midiNumber, accType)` constructor already does. No new concept.

**From frequency:**

- It **always** yields a note; it never rejects.
- Default (`enableQuarterToneRound == false`): rounds to the nearest **semitone**.
- With `enableQuarterToneRound == true`: rounds to the nearest **quarter tone**.
- A frequency of **0 or negative produces a rest**.

### 4.4 Rest semantics

Because `setFrequency(0)` produces a rest, `Pitch` must be able to *be* a rest, and a rest has no step, alter or octave. The class invariant is therefore "either a rest, or three valid values", and every getter has a defined answer:

| Getter | Ordinary note | Rest |
|---|---|---|
| `isRest()` | `false` | `true` |
| `getPitch()` / `getPitchClass()` / `getPitchStep()` | `"C1x4"` / `"C1x"` / `"C"` | `"rest"` |
| `getAlterSymbol()` / `getAlter()` | `"1x"` / `0.5` | `""` / `0.0` |
| `getOctave()` | `4` | `-1` |
| `getMidiNumber()` | `61` | `MIDI_REST` (−1) |
| `getFrequency()` | `277.18` | `0.0` |

**The rest octave is standardised at −1 across the whole library.** `MUSIC_XML::OCTAVE::ALL` is already `-1` (`constants.h:175`) and the XML reader already assigns it (`score.cpp:1659`), so no new constant is needed. But the library currently gives **three different answers** to the same question, and all three collapse to −1:

| Site | Current value |
|---|---|
| `Helper::splitPitch` (`helper.cpp:1380`) | `0` |
| MusicXML reader (`score.cpp:1659`) | `-1` |
| `Helper::midiNote2octave(MIDI_REST)` | `-2` |

**Documented ambiguity:** −1 is simultaneously the rest sentinel and a legitimate octave, since SP1 fixed the range at −1..11 with `C-1` = MIDI 0. `getOctave() == -1` therefore cannot distinguish a rest from a real note in octave −1. The Doxygen on both `getOctave()` and `isRest()` must state that **`isRest()` is the authoritative test**. This is a pre-existing property of the sentinel choice, not introduced here.

### 4.5 Rounding

Ties round **upward**, pinned explicitly rather than left to the compiler's rounding mode. This applies to `setFrequency` and to `getMidiNumber()` when the alter is exactly ±0.5, so `C1x4` reports MIDI 61.

Consequently `C1x4` and `C#4` report the same MIDI number. This does **not** contradict §9: enharmonic equivalence compares exact pitch, never the rounded MIDI number. `getMidiNumber()` is a documented lossy view; callers needing the exact value use `getQuarterToneSteps()`.

## 5. `Note` composes one `Pitch`

`Note` currently stores **two** pitches in parallel — written (`_writtenPitchClass`, `_writtenOctave`) and sounding (`_soundingPitchClass`, `_soundingOctave`) — plus `_midiNumber` and `_alterSymbol`, because of transposing instruments (`_transposeDiatonic`, `_transposeChromatic`).

Read from `note.cpp`: written is canonical and sounding is derived — but derived **eagerly and then stored**. The constructor assigns the written fields, copies them into the sounding fields, and only then calls `setTransposingInterval()` to correct them (`:74-87`). `setPitchClass()` repeats the same dance (`:121-132`); `setOctave()` recomputes from `getWrittenPitch()` (`:140-143`).

That is the "store every representation and have each setter recompute them all" pattern, and it is why `Note::setPitchClass` is on this project's known-defect list — it keeps the old accidental and skips the MIDI update. Any setter that forgets the `setTransposingInterval()` call leaves the sounding pitch stale.

**Design:** `Note` holds one canonical **written** `Pitch` plus the transposing interval. The sounding pitch becomes `getSoundingPitch()`, derived on demand. The parallel fields are deleted. This removes the entire "did you remember to call it?" class of bug rather than guarding against it, and fixes the known `setPitchClass` defect as a side effect.

Public `Note` accessors keep their current signatures and forward to the contained `Pitch`, so existing C++ and Python callers are unaffected. Fields that stay on `Note`: `_inChord`, `_voice`, `_staff`, `_isGraceNote`, `_stem`, `_isTuplet`, `_isNoteOn`, `_isPitched`, `_unpitchedIndex`, and the duration family.

## 6. Accidental vocabulary

`c_alterSymbol` (`constants.h:103`) widens from four entries to eight, ordered by pitch:

```cpp
{"bb", "3b", "b", "1b", "1x", "#", "3x", "x"}
```

These four quarter-tone symbols are canonical on both input and output. Aliases may be added later without breaking anything; none are added now.

The two `static_cast<int>(alterValue)` truncations (`helper.cpp:249`, `note.cpp:315`) are deleted. `alterValue` widens from `int&` to `float&` at `helper.h:385` and `score.h:438`; the `float&` at `helper.h:320` is already correct. These are public signature changes, so the pybind11 wrappers follow.

## 7. MusicXML

### 7.1 Read: one path, explicit precedence

The two existing read paths are replaced by a single implementation with this precedence:

1. `<accidental>` present → translate by name, accepting **both** accidental families (13 names, §7.3).
2. Otherwise `<alter>` present → parse as a **decimal**, not an integer.
3. Otherwise → natural.

This ordering is not cosmetic. MuseScore has historically exported the quarter-tone accidental glyph **without** a matching `<alter>` ([MuseScore #4474](https://musescore.org/en/node/4474), [#167031](https://musescore.org/en/node/167031)), so a reader that trusts `<alter>` loses quarter tones from real files.

What dies: the four-case string switch (`score.cpp:646-661`), the `atoi` fallback (`:1649`), the `int&` signatures, and the commented-out block at `:1431-1437`.

### 7.2 Write

Emit a decimal `<alter>` (the sounding value) **and** an `<accidental>` element (the notation), replacing the `static_cast<int>` at `note.cpp:766` and `:784`. The writer currently emits no `<accidental>` at all, so this is an addition.

**Number formatting must be pinned:** a semitone's `<alter>` continues to be written as `1` or `-2`, never `1.0` or `1.000000`. Integer values print without a decimal part; quarter tones print with one decimal place. Without this rule, the first re-export of any existing score produces an enormous false diff.

### 7.3 Accidental families

Verified against the specification ([W3C MusicXML 4.0, accidental-value](https://www.w3.org/2021/06/musicxml40/musicxml-reference/data-types/accidental-value/)) rather than assumed: MusicXML defines **two legitimate quarter-tone families**.

- `quarter-sharp`, `quarter-flat`, `three-quarters-sharp`, `three-quarters-flat` — Tartini-style accidentals, the conventional glyphs.
- `sharp-down`, `sharp-up`, `flat-down`, `flat-up` — quarter-tone accidentals drawn with arrows.

maialib currently emits the **arrow** family, which is valid MusicXML but a notational choice nobody made deliberately. **maialib will write Tartini and read both:**

| `<accidental>` accepted | symbol | alter |
|---|---|---|
| `quarter-sharp` *or* `sharp-down` | `1x` | +0.5 |
| `three-quarters-sharp` *or* `sharp-up` | `3x` | +1.5 |
| `quarter-flat` *or* `flat-up` | `1b` | −0.5 |
| `three-quarters-flat` *or* `flat-down` | `3b` | −1.5 |

`Helper::alterValue2Name` changes its four quarter-tone outputs to the Tartini names; `Helper::alterName2symbol` grows from 9 accepted names to 13.

**Blast radius of that output change: none today.** Excluding generated Doxygen HTML, the live callers are `alterSymbol2Value` → `helper.cpp:1423`, `note.cpp:766`, `:784`; `alterName2symbol` → `score.cpp:1646` only; `alterValue2symbol` → `score.cpp:1650` only; and **`alterValue2Name` → no live caller at all**. The one function whose output changes is currently dead code; its first live caller will be the `<accidental>` emission added here. None of the four is currently bound to Python; the changed ones gain documented wrappers as part of this work.

## 8. Harmonic analysis rejects quarter tones

Intervals, stacking in thirds and chord naming continue to operate in semitones and **throw a clear error** when they encounter a quarter tone, rather than answering from a rounded value. A confidently wrong answer is the worst failure mode for an analysis library. Creating, parsing, transposing, writing and sounding quarter-tone notes all continue to work.

The public analysis surface is 136 declarations in `chord.h` and 45+ in `interval.h`. Guarding each would be unmaintainable and impossible to prove complete. Tracing the implementations shows it funnels through **two** sites:

1. **`Interval`'s two state-populating entry points** — the `(Note, Note)` constructor (`interval.cpp:15`) and `setNotes(const Note&, const Note&)` (`:32`). The string overloads delegate to them (`:12-13`, `:28-30`). Both already carry a rest guard using `LOG_ERROR`, so the quarter-tone guard is that guard's sibling, not a new mechanism. Both compute `_numSemitones` by **integer MIDI subtraction**, which is exactly where a quarter tone would be silently rounded away.
2. **`Chord::stackInThirds()`** — every `have<Quality><Degree>` method opens with `if (!_isStackedInThirds) stackInThirds(useEnharmony);` before reading `_closeStack`, so all of them inherit, as do `getName()` and `isTonal()`.

`Chord::getIntervalsFromOriginalSortedNotes()` (`chord.cpp:877`) — the funnel for the `have<Degree>` family and for `haveMajorInterval`/`haveMinorInterval` — **constructs `Interval` objects**, so it inherits site 1's rejection transitively and needs no guard of its own.

Error messages name the offending note, not merely the condition.

## 9. Enharmonic equivalence uses exact pitch

`Helper::isEnharmonic` (`helper.h:302`) is currently documented as true when both pitches have the same MIDI number. It changes to compare **exact** pitch. `Note::getEnharmonicPitch`, `getEnharmonicPitches`, `getEnharmonicNote` and `getEnharmonicNotes` (`note.h:530-553`) extend to quarter tones.

```
C1x4 = C(0) + 0.5 = 0.5
D3b4 = D(2) - 1.5 = 0.5   -> enharmonic with each other
C#4  = 1.0                -> NOT enharmonic with C1x4
```

This does not contradict §8. Analysis is rejected because the theory has a genuine gap — there is no agreed chord-name for a neutral third. Enharmonic equivalence is merely "same sounding pitch, different spelling", which generalises to 24 divisions with no gap at all.

A quarter-tone spelling has **exactly one** enharmonic partner in the 24-tone grid, whereas semitone spellings may have two — which is what the `alternativeEnharmonicPitch` parameter exists for. When no alternative exists, SP1's established range-fallback rule applies: return the default spelling.

## 10. Transposition

`Chord::transpose` (`chord.h:301`), `Chord::transposeStackOnly` (`:307`) and `Helper::transposePitch` (`helper.h:289`) widen from `int` to a fractional step, **validated as a multiple of 0.5**. `transpose(2)` behaves exactly as today; `transpose(0.5)` moves one quarter tone; `transpose(0.3)` throws rather than rounding. Existing integer call sites are unaffected in both C++ and Python.

**Documented consequence:** `Chord::transpose` re-stacks the chord, and §8 makes stacking reject quarter tones, so transposing a *chord* off the semitone grid throws. This is the rejection contract being consistent, not an accident — but a caller reading `transpose(float)` will reasonably assume 0.5 works for chords. It must be stated in the method's own documentation, not discovered through a bug report.

## 11. Frequency and the SP2/SP3 boundary

`Pitch::getFrequency(float freqA4 = 440.0f)` reads the global `TuningSystem` (`config.h`) at call time. The signature matches the established project convention — `Note::getFrequency(const float freqA4 = 440.0f)` at `note.h:573`, and the same default across `Helper`, `Chord` and `Interval`.

**There is no reference-tonic parameter.** Non-equal tunings define ratios relative to a tonic — in just intonation, E4 as the major third of C is 5/4 × C, while E4 as the tonic of E major is a different absolute frequency — so the question has no answer without a key. But 12-TET is symmetric, only A4 matters, and SP2 implements only 12-TET. Adding the parameter now would be speculative API resolving SP3's tension prematurely. SP3 decides how a tonic is supplied, and can do so without changing this signature.

## 12. Testing

**Characterisation tables first, before any change.** Generate and pin the current output of `splitPitch`, `pitch2midiNote`, `getEnharmonicPitch` and `isEnharmonic` for every existing semitone spelling. Quarter tones are pure addition, so none of these answers may change. This is the technique that gave SP1 zero regressions.

**Rejection-coverage test.** Walk every public analysis method in `Chord` and `Interval` with a quarter-tone input and assert that each throws. With 181 methods, this is the only honest evidence that the two guard sites cover everything; reasoning about coverage is not evidence.

**MusicXML round-trip**, with three fixtures under `test/xml_examples/unit_test/`: one written by maialib itself; one third-party file using the arrow family; and one carrying `<accidental>` with **no** `<alter>`, the MuseScore case.

**Two deliberate expectation changes**, to be flagged prominently in the implementation plan so that no implementer "repairs" the code to match a stale test:

- `test/test_helpers.py:409` — `splitPitch("rest")` asserts octave `0`, becomes `-1`.
- `tests-cpp/src/helpers-test.cpp:261` — `midiNote2octave(MIDI_REST)` asserts `-2`, becomes `-1`.

**Mutation testing as the acceptance criterion for any test whose purpose is to pin new behaviour:** delete the line the test protects, rebuild, confirm the test fails, restore, confirm it passes. Reasoning that a test discriminates failed three separate times during the 2026-09-16 work; only the experiment counts.

Baselines to hold: C++ 832/832 and Python 264/264, plus whatever this work adds.

## 13. Implementation order

1. Characterisation tables.
2. **`Pitch` as a standalone class**, complete and exhaustively tested, touching `Note` not at all.
3. `Note` composes it; the parallel written/sounding fields are deleted.
4. Vocabulary: `c_alterSymbol` widened to eight entries.
5. Truncations deleted; `int&` → `float&`.
6. Rest octave standardised at −1 across all three sites.
7. MusicXML: unified read, then write.
8. Rejection guards at the two sites, plus the 181-method coverage test.
9. Exact enharmonic comparison.
10. Fractional transposition.
11. Python bindings and documentation throughout.

Steps 2 and 3 are deliberately separate. Step 3 is the riskiest change in the sub-project — `Note` is used by `Chord`, `Measure`, `Part`, `Score`, MusicXML I/O and the bindings — and splitting it from step 2 means that when it begins, the foundation is already verified in isolation. If something breaks during step 3, the cause is the transplant, not the pitch logic.

This is a larger sub-project than SP1: eleven steps, one broad refactor, and a substantial increase in test count.
