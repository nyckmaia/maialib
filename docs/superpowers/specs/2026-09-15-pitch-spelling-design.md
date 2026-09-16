# Design Spec: Pitch Spelling Foundation (SP1)

- **Date:** 2026-09-15
- **Branch:** `feature/pitch-spelling` (from `main`)
- **Status:** Design approved section by section; this document awaits final review.
- **Roadmap:** SP1 of 4 — SP1 pitch spelling → SP2 quarter tones → SP3 tuning-aware frequency →
  SP4 `Interval::getFraction` (spec on hold at `feature/interval-get-fraction`, commit `2d372b3`).

---

## 1. Summary

Replace the hand-written pitch tables of `maiacore` with arithmetic built on a single pitch-string
parser. A pitch spelling (step, accidental, octave) becomes the single source for MIDI numbers,
reverse spelling and enharmonic spellings across octaves -1..11. This closes the gaps in the current
tables (e.g. `Cbb0`, `Cb0`, `Bx9`, most of octaves 10–11), fixes several parsing bugs, removes dead
code, and prepares the representation (`alterValue` as `float`) for quarter tones in SP2.

## 2. Goals and Non-Goals

### Goals

- One parser (`Helper::splitPitch`) used by every pitch-string consumer.
- Every spelling with accidentals `bb`, `b`, natural, `#`, `x` in octaves -1..11 whose MIDI number
  is >= 0 is accepted (453 spellings).
- MIDI numbers, MIDI-to-pitch spelling and enharmonic spellings are computed, not tabulated.
- Existing behavior is preserved and pinned by characterization tests, except for the documented
  fixes in §9.
- Removal of dead or superseded code: `Helper::pitch2number`, `Helper::number2pitch`,
  `MUSIC_XML::MIDI::NUMBER::MIDI_000..MIDI_132`, `c_pianoWhiteKeys`.

### Non-Goals

- Quarter tones (`1x`, `3x`, `1b`, `3b`) — SP2.
- MusicXML `<alter>` / `<accidental>` reading and writing — SP2.
- Tuning systems and frequency — SP3.
- Restricting `<octave>` output to MusicXML's 0..9 range.

## 3. Background (Current State)

| Fact | Location |
|---|---|
| `Helper::pitch2midiNote` is a 351-case string-hash switch; any string containing `-` returns -1 (rest); unknown strings throw "Unknown pitch". | `helper.cpp:207-1065` |
| Switch gaps inside octaves -1..11 with MIDI >= 0: 102 spellings (octave -1: 33, 0: 2, 9: 1, 10: 31, 11: 35). | verified by script |
| Switch bug: `Db10` returns 132 (same as `C10`); correct value is 133. | `helper.cpp:1053-1054` |
| `Note::getEnharmonicPitch` is a 385-case switch (octaves -1..9); some outputs are unparseable (`Bb-2`, `Cbb10`, `C#10`). | `note.cpp:321-1512` |
| The enharmonic switch follows one consistent rule (§6.4): 0 mismatches over 385 cases. | verified by script |
| Pitch strings are parsed in four divergent places: `Note` constructor (`note.cpp:40-92`), `Note::setPitch` (`note.cpp:1642-1676`, last-digit octave only), `Helper::splitPitch` (`helper.cpp:2189-2259`), `Helper::pitch2number` (`helper.cpp:2530-2599`, last-digit octave only). All but `pitch2number` reject strings longer than 4 characters. | — |
| `Note::setPitch` does not clear `_alterSymbol` when the new pitch has no accidental. | `note.cpp:1668-1670` |
| `Helper::isEnharmonic` compares `pitch2number` values, which treat E–F and B–C as whole steps: `isEnharmonic("E#4", "F4") == false`. It is called by `Helper::noteSimilarity`. | `helper.cpp:2517-2528`, `:1988` |
| `Helper::pitch2number` has no caller other than `isEnharmonic`; it is bound twice in Python. `Helper::number2pitch` has no caller and no binding. | `py_helper.cpp:58, :114`; `helper.cpp:2414` |
| `MIDI_000..MIDI_132` are referenced by the `pitch2midiNote` switch and by the `midiNote2octave.midiValues` test (72 uses in `tests-cpp/src/helpers-test.cpp:263-344`, to be rewritten as integer literals); none are bound to Python. | `constants.h:240-372` |
| `Interval::whiteKeyDistance` looks notes up in `c_pianoWhiteKeys` (`C0..C10`); a note outside that array silently yields a wrong distance, affecting `isAscendant`, `isDescendant`, `getDiatonicInterval`. It is the array's only use. | `interval.cpp:46-62`, `constants.h:106` |
| `Chord::computeEnharmonicUnitsGroups` builds three variants per note via `getEnharmonicNote(false/true)`; it requires parseable outputs. | `chord.cpp:445-464` |
| MIDI convention: octave = MIDI / 12 − 1 (C4 = 60, C0 = 12, C-1 = 0). | `helper.cpp:1096-1104` |
| The only exception test in note/interval/chord suites is `Note("C#123")`. No test covers `pitch2midiNote`, `isEnharmonic`, `pitch2number` or octave -1/10/11 spellings. | `note-test.cpp:55` |

## 4. Pitch String Grammar and Range

```
pitch      := step accidental? octave?
step       := "A" | "B" | "C" | "D" | "E" | "F" | "G"
accidental := "bb" | "b" | "#" | "x"
octave     := "-"? digit+            (integer value must be in [-1, 11])
```

- Missing octave → octave 4 (unchanged behavior).
- Empty string or any string containing `"rest"` → rest (unchanged behavior).
- The fixed 4-character limit is removed; the grammar bounds the length.
- The resulting MIDI number must be >= 0 (`Cb-1` and `Cbb-1` are rejected because -1 is
  `MIDI_REST`).
- Error messages (all via `LOG_ERROR`, i.e. `std::runtime_error`):
  - invalid step → `Unknown diatonic pitch: <step>` (typo "diatonc" fixed)
  - invalid accidental → `Unknown alter symbol: <symbol>` (unchanged)
  - octave out of range → `Invalid octave value: <octave>` (unchanged)
  - MIDI below 0 → `The pitch '<pitch>' is below MIDI note 0`

## 5. Public API and Behavior

No existing signature changes. One binding is added (`splitPitch`), two functions are removed.

### 5.1 `Helper`

| Function | Behavior |
|---|---|
| `splitPitch(pitch, pitchClass, pitchStep, octave, alterValue, alterSymbol)` | The single parser (§4). Outputs unchanged in meaning; rest outputs unchanged (`"rest"`, `"rest"`, 0, 0, `""`). |
| `pitch2midiNote(pitch)` | Rest (empty / contains `"rest"`) → `-1` (previously `"rest"` threw). Otherwise `splitPitch` + formula (§6.2). |
| `midiNote2pitch(midiNote, accType)` | `midiNote < 0` → `"rest"` (unchanged). `accType` validation unchanged. Spelling computed (§6.3). Throws if no step fits the accidental (unchanged message) or if the octave falls outside -1..11. |
| `midiNote2pitches(midiNote)` | Unchanged logic; inherits the new range. |
| `transposePitch(pitch, semitones, accType)` | Unchanged logic; inherits the new range. |
| `isEnharmonic(pitch_A, pitch_B)` | `pitch2midiNote(pitch_A) == pitch2midiNote(pitch_B)`. |
| `pitch2number`, `number2pitch` | **Removed.** |

### 5.2 `Note`

| Member | Behavior |
|---|---|
| `Note(const std::string& pitch, ...)` | Uses `splitPitch`; accepts the full range. |
| `Note(int midiNumber, accType, ...)` | Unchanged delegation to `midiNote2pitch`; `Note(5)` now yields `F-1`. |
| `setPitch(pitch)` | Uses `splitPitch`; always assigns `_alterSymbol` (bug fix); multi-digit and negative octaves work. |
| `getEnharmonicPitch(alternative)` | Computed rule (§6.4) with range fallback: a missing alternative falls back to the default; a missing default falls back to the note's own pitch. Never returns an out-of-range spelling. Rest → `"rest"` (unchanged). |
| `getEnharmonicPitches`, `getEnharmonicNote(s)`, `toEnharmonicPitch` | Unchanged composition. Lists may still contain duplicates (e.g. `G#4` → `["G#4", "Ab4", "Ab4"]`), because chord analysis expects three variants per note. |

### 5.3 `Interval` (private)

`whiteKeyDistance()` computes `diatonicIndex(B) − diatonicIndex(A)` with
`diatonicIndex = stepIndex(C=0 … B=6) + 7 × octave`, using the same accessors as today
(`getPitchStep()`, `getOctave()`). For notes in `C0..C10` the result is identical to the current
array lookup.

## 6. Algorithms

### 6.1 Parsing (`splitPitch`)

1. Rest check (empty or contains `"rest"`).
2. First character must be a step (`A`–`G`).
3. Octave token: trailing digits, including one immediately preceding `-`. If absent, octave = 4.
   Parse as integer; require `-1 <= octave <= 11`.
4. Accidental token: characters between the step and the octave token; must be empty or one of
   `c_alterSymbol` (`bb`, `b`, `#`, `x`).
5. `alterValue = Helper::alterSymbol2Value(alterSymbol)`; `pitchClass = step + alterSymbol`.
6. Require `12 × (octave + 1) + stepSemitone(step) + alterValue >= 0`.

### 6.2 Pitch → MIDI

```
stepSemitone = {C: 0, D: 2, E: 4, F: 5, G: 7, A: 9, B: 11}
midi = 12 × (octave + 1) + stepSemitone[step] + alterValue
```

Verified against the existing switch: 350 of 351 cases match; the mismatch is the `Db10` bug.

### 6.3 MIDI → Pitch

For `accType` empty, the accidental is natural for white keys and `#` for black keys (unchanged
default). Otherwise `alter` comes from `accType`. The step is the unique `s` with
`(midi − stepSemitone[s] − alter) mod 12 == 0`; if none exists, throw (unchanged message).
`octave = (midi − stepSemitone[s] − alter) / 12 − 1`; if outside -1..11, throw.

### 6.4 Enharmonic Rule

Candidates are all spellings (`alter ∈ {-2, -1, 0, 1, 2}`) of the same MIDI number, excluding the
note's own spelling.

**White key** (a natural spelling exists):

| Note spelling | Default | Alternative |
|---|---|---|
| natural | the flat-side spelling (`alter < 0`, next step up) | the sharp-side spelling (`alter > 0`) |
| flat-side (`b` / `bb`) | natural | sharp-side |
| sharp-side (`#` / `x`) | natural | flat-side |

**Black key** (no natural spelling):

| Note spelling | Default | Alternative |
|---|---|---|
| `#` | `b` spelling | double-accidental spelling (if any, else the default) |
| `b` | `#` spelling | double-accidental spelling (if any, else the default) |
| `x` | `#` spelling | `b` spelling |
| `bb` | `b` spelling | `#` spelling |

Examples: `C4` → `Dbb4` / `B#3`; `Cx4` → `D4` / `Ebb4`; `C#4` → `Db4` / `Bx3`; `Fbb4` → `Eb4` /
`D#4`; `G#4` → `Ab4` / `Ab4`.

**Range fallback:** a candidate outside octaves -1..11 does not exist. If the alternative is
missing, return the default; if the default is missing, return the note's own pitch (e.g. `Bx11`
→ `Bx11`).

Verified against the existing switch: 0 mismatches over 385 cases.

## 7. Removals

| Item | Reason |
|---|---|
| `pitch2midiNote` switch body (351 cases) | Replaced by §6.2 |
| `getEnharmonicPitch` switch body (385 cases) | Replaced by §6.4 |
| `Helper::pitch2number` + both Python bindings | Only caller (`isEnharmonic`) no longer uses it; not enharmonic-safe |
| `Helper::number2pitch` | No callers, no binding |
| `MUSIC_XML::MIDI::NUMBER::MIDI_000..MIDI_132` (`MIDI_REST` stays) | Only used by the removed switch and by `midiNote2octave.midiValues`, which is rewritten with integer literals |
| `c_pianoWhiteKeys` | Only used by the replaced `whiteKeyDistance` |

## 8. Python Bindings

- `py_helper.cpp`:
  - add `Helper.splitPitch(pitch) -> tuple[str, str, int, float, str]` (`pitchClass`, `pitchStep`,
    `octave`, `alterValue`, `alterSymbol`) via a lambda;
  - add `py::arg("pitch_A")`, `py::arg("pitch_B")` to `isEnharmonic`;
  - remove both `pitch2number` bindings.
- numpydoc docstrings (technical English, `R"pbdoc(...)pbdoc"`) for every binding whose behavior
  changes: `pitch2midiNote`, `midiNote2pitch`, `midiNote2pitches`, `transposePitch`,
  `isEnharmonic`, `splitPitch`; in `py_note.cpp`: the pitch-string and MIDI constructors,
  `setPitch`, `getEnharmonicPitch`, `getEnharmonicPitches`, `getEnharmonicNote`,
  `getEnharmonicNotes`, `toEnharmonicPitch`.

## 9. Behavior Changes

| Input | Before | After |
|---|---|---|
| `Note("Cbb0")`, `Note("Cb0")`, `Note("Bx9")` | throws | MIDI 10, 11, 133 |
| Spellings in octaves -1, 10, 11 (e.g. `Note("C-1")`, `Note("E10")`, `Note("Bx11")`) | throws (except `C10`, `B#9`, `Db10`) | accepted (MIDI 0, 136, 157) |
| `Note(5)` | throws | `F-1` |
| `Note(157, "x")` | throws (`Note(int)` rejected MIDI > 127) | `Bx11`; the removed guard is replaced by `midiNote2pitch`'s octave range check |
| `pitch2midiNote("Db10")` | 132 | 133 |
| `pitch2midiNote("C-1")` | -1 (treated as rest) | 0 |
| `pitch2midiNote("rest")` | throws | -1 |
| `Note("Cb-1")` | throws (unknown alter symbol) | throws (below MIDI 0) |
| `setPitch("C10")` | throws | works |
| `setPitch("D4")` on a former `C#4` | `getAlterSymbol() == "#"` | `""` |
| `isEnharmonic("E#4", "F4")`, `("B#3", "C4")`, `("Cb4", "B3")` | `false` | `true` (also affects `noteSimilarity`) |
| `Interval` with a note outside `C0..C10` | wrong direction / diatonic distance | correct |
| `midiNote2pitch` producing octave 12 | returned an unparseable spelling | throws |
| `getEnharmonicPitch` near the range edges | could return `…-2` / octave-10+ spellings that throw | falls back per §6.4 |
| `Helper.pitch2number` (Python) | available | **removed (breaking)** |

## 10. Testing

### 10.1 Characterization (before any removal)

1. A one-off extraction script (not committed) reads the current switches and generates
   `tests-cpp/src/pitch-spelling-legacy-data.h` with:
   - `kLegacyMidiTable`: 351 `{pitch, midi}` pairs, `Db10` stored as 133 with a comment;
   - `kLegacyEnharmonicTable`: 385 `{pitch, default, alternative}` triples.
2. The same commit fixes the `Db10` line in the legacy switch, so the new tests pass against the
   **current** implementation.
3. Tests (header included by existing, CMake-registered sources):
   - `PitchSpellingLegacy.MidiTableMatches` in `helpers-test.cpp`;
   - `PitchSpellingLegacy.EnharmonicTableMatches` in `note-test.cpp` — entries whose input is
     below MIDI 0 (`Cbb-1`, `Cb-1`) are asserted to throw; entries whose legacy outputs fall
     outside octaves -1..11 are asserted against the §6.4 fallback instead of the legacy value.
4. The implementation is then replaced; these tests stay unchanged and must keep passing.

### 10.2 Full-Range Properties (C++)

For all 453 valid spellings:
- `Note(p).getMidiNumber() == Helper::pitch2midiNote(p)`;
- `Helper::midiNote2pitch(pitch2midiNote(p), accidentalOf(p)) == p`;
- every `getEnharmonicPitch(false/true)` output is accepted by `Note`, has the same MIDI number, and
  stays in range;
- `Helper::isEnharmonic(p, q)` is true for every enharmonic pair.

### 10.3 Edge Cases (C++)

- Accepted: `Cbb0`=10, `Cb0`=11, `Bx9`=133, `Db10`=133, `C-1`=0, `Bx11`=157, `C04`=60, `"rest"`=-1.
- Rejected: `Cb-1`, `Cbb-1`, `C12`, `C#123`, `H4`, `C#4x`, `C-`, `C1x4`.
- `setPitch`: `C#4` → `D4` clears the accidental; `setPitch("C10")` works.
- `isEnharmonic`: `E#4/F4`, `B#3/C4`, `Cb4/B3` true; `C4/D4` false; `rest/rest` true.
- `midiNote2pitch(158)` throws; `Note(5).getPitch() == "F-1"`; `Note(157, "x").getPitch() == "Bx11"`.
- `getEnharmonicPitch`: `Bx11` → `Bx11`; `C-1` alternative falls back to `Dbb-1`.
- `Interval`: `C-1→D-1` ascending; `B10→C11` diatonic steps 1 (non-single-octave); `C-1→C11`
  distance 84.

### 10.4 Python (unittest)

- `test/test_helpers.py`: `pitch2midiNote` edge cases; `isEnharmonic` fixes; `splitPitch` tuple;
  `hasattr(ml.Helper, "pitch2number")` is false.
- `test/test_note.py`: `Cbb0`, `Bx9`, `C-1`, `Note(5)`; `setPitch` accidental reset; enharmonic
  range fallback.

All existing C++ and Python tests must pass unchanged.

## 11. Documentation and Release

- **Doxygen** (technical English): `helper.h` (grammar, range, rest handling, errors for
  `splitPitch`, `pitch2midiNote`, `midiNote2pitch`, `isEnharmonic`); `note.h` (accepted range for
  constructors and `setPitch`, enharmonic rule and range fallback); `interval.cpp` comment for
  `whiteKeyDistance`.
- **pybind11 docstrings** per §8.
- **`CHANGELOG.md`** — new `## [Unreleased]` section:
  - Improve: full pitch-spelling range (octaves -1..11, double accidentals everywhere); computed
    MIDI and enharmonic spelling; `Helper.splitPitch` in Python.
  - Fix: `Db10` MIDI number; `Note.setPitch` accidental reset and multi-digit octaves;
    `Helper.isEnharmonic` for E–F and B–C; `Interval` direction/distance outside `C0..C10`.
  - Removed (breaking): `Helper.pitch2number`.
- **AI docs** (`AI_API_CHEATSHEET.md`, `llms-full.txt`, tracked) are regenerated by `make dev`;
  commit the resulting diff. Python stubs (`*.pyi`, `stubs/`) are git-ignored and not committed.
- No `VERSION` bump; no Doxygen HTML regeneration.

## 12. Verification Procedure

Always in a brand-new, clean Python virtual environment (never the global interpreter, never a
reused venv), created from the same interpreter the module is built against:

```bash
python -m venv <fresh-venv-dir>            # outside the repository
source <fresh-venv-dir>/Scripts/activate   # Git Bash on Windows
# install the build/dev tooling required by scripts/ (exact list in the implementation plan)
make clean        # REQUIRED: drops build/ (stale CMakeCache may pin another Python), dist/, stubs
make dev          # uninstall + module-release + install (+ stubs, AI docs)
python -c "import maialib; print(maialib.__version__)"   # sanity check inside the venv
make cpp-tests    # GoogleTest
make py-tests     # unittest (requires the installed module)
make validate     # cpplint + cppcheck (+ ruff)
```

`make module-clean` is **not** an alternative to `make clean`: `scripts/make-clean.py` only
implements the `all` and `dist` options, so `module`, `static` and `shared` are silent no-ops
(pre-existing bug, out of scope for SP1).

## 13. Files Touched

| File | Change |
|---|---|
| `maiacore/include/maiacore/helper.h` | remove `pitch2number`, `number2pitch`; Doxygen updates |
| `maiacore/src/maiacore/helper.cpp` | parser, `pitch2midiNote`, `midiNote2pitch`, `isEnharmonic`; remove switch and dead functions |
| `maiacore/include/maiacore/constants.h` | remove `MIDI_000..MIDI_132`, `c_pianoWhiteKeys` |
| `maiacore/include/maiacore/note.h` | Doxygen updates |
| `maiacore/src/maiacore/note.cpp` | constructor and `setPitch` via `splitPitch`; computed `getEnharmonicPitch` |
| `maiacore/src/maiacore/interval.cpp` | arithmetic `whiteKeyDistance` |
| `maiacore/src/maiacore/python_wrapper/py_helper.cpp` | `splitPitch` binding, `isEnharmonic` args, remove `pitch2number`, docstrings |
| `maiacore/src/maiacore/python_wrapper/py_note.cpp` | docstrings |
| `tests-cpp/src/pitch-spelling-legacy-data.h` | **new** (generated once) |
| `tests-cpp/src/helpers-test.cpp`, `note-test.cpp`, `interval-test.cpp` | tests per §10 |
| `test/test_helpers.py`, `test/test_note.py` | tests per §10.4 |
| `CHANGELOG.md` | `[Unreleased]` entry |
| `AI_API_CHEATSHEET.md`, `llms-full.txt` | regenerated |

## 14. Decision Log

| # | Decision | Rationale |
|---|---|---|
| D1 | Split the roadmap into SP1–SP4, in that order, each with its own spec/plan/implementation | The four requests touch different subsystems; SP1 is the base for all others |
| D2 | Range: octaves -1..11, MIDI >= 0 | Every MIDI 0..127 becomes a `Note`; keeps octaves 10–11 (existing `C10` test) |
| D3 | Arithmetic implementation + characterization tests | User first chose "complete the switches"; after verification (0/385 enharmonic mismatches, 350/351 MIDI with one bug) and the scope conflict, the user confirmed arithmetic |
| D4 | `splitPitch` is the single parser | Removes four divergent parsers |
| D5 | Bind `splitPitch` to Python as a tuple | User rule: every public maiacore method has a pybind11 wrapper |
| D6 | `pitch2midiNote("rest")` returns -1 | Symmetric with `midiNote2pitch(-1) == "rest"` |
| D7 | Remove `pitch2number` and `number2pitch` | User rule: remove unused code; no callers after `isEnharmonic` uses MIDI |
| D8 | Remove `MIDI_000..MIDI_132` and `c_pianoWhiteKeys` | Unused after the change (the one test using the MIDI constants is rewritten with integer literals); not exposed to Python |
| D9 | Enharmonic range fallback (alternative → default → self) | `Chord` analysis needs parseable variants |
| D10 | Keep duplicate entries in enharmonic lists | Preserves `Chord` three-variant expectation |
| D11 | Arithmetic `whiteKeyDistance` | The array silently breaks outside `C0..C10` |
| D12 | Fix "diatonc" typo in error message | No test depends on the text |
| D13 | Separate branch per sub-project | User choice |
| D14 | Verify only in a brand-new clean venv via `make dev` | User rule |

## 15. Risks and Items to Verify During Implementation

- **Breaking Python API:** `Helper.pitch2number` removal — called out in the changelog.
- **MusicXML output:** notes in octaves -1, 10, 11 export an `<octave>` outside MusicXML's 0..9
  (already true for `C10` today); unchanged by this spec.
- **Written vs sounding pitch:** `getEnharmonicPitch` and `whiteKeyDistance` must keep using the
  same accessors they use today (`getPitch()`, `getPitchStep()`, `getOctave()`).
- **Legacy enharmonic data at the range edges:** the characterization test must route those entries
  to the fallback assertions (§10.1).
- **`make dev` side effects:** it rewrites the tracked AI docs; review the diff before committing.
- **Performance:** per-`Note` parsing replaces a hash switch; confirm score-loading tests show no
  noticeable slowdown.
- **Stale CMake cache / wrong interpreter:** `build/Windows/module/CMakeCache.txt` currently pins
  `PYTHON_EXECUTABLE` to the global Python 3.12 (existing `maiacore.cp312-win_amd64.pyd`, built
  2025-11-24), while the default `python` on PATH is 3.14. `make dev` does not clean the build
  directory and `scripts/make-dist.py` copies the first `.pyd` it finds, so a fresh venv could
  receive a module built for another interpreter. The module build directory must be cleaned
  before `make dev` so CMake detects the venv interpreter (§12).
- **Toolchain paths:** `scripts/make-module.py` uses `clang++` and the hard-coded
  `C:/msys64/clang64/bin/mingw32-make.exe` (verified present on this machine).
