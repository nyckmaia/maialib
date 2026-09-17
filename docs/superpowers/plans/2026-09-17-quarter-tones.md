# Quarter Tone Support (SP2) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make quarter tones first-class in maiacore — parseable, representable, transposable, sounding and round-trippable through MusicXML — by introducing a `Pitch` class that owns every representation of a pitch coherently.

**Architecture:** A new `Pitch` value class stores only `(step, alter, octave)` and computes every other representation on demand, with no cache. It is built and verified standalone first; only then does `Note` replace its parallel written/sounding fields with one canonical written `Pitch` plus the transposing interval. Harmonic analysis rejects quarter tones at two chokepoints rather than across 181 public methods.

**Tech Stack:** C++17, pybind11, GoogleTest, pugixml, Python 3.12 for the binding tests.

**Spec:** `docs/superpowers/specs/2026-09-17-quarter-tones-design.md` — read it alongside this plan.

## Global Constraints

- Branch `feature/quarter-tones`, base `main` @ `43a2473`. Never commit on `main`.
- **Never stage `.gitignore`** — it carries an unrelated local modification by the user.
- Internal documentation (Doxygen, docstrings, Markdown) is in **technical English**.
- Every public maiacore method gets a pybind11 wrapper with a numpydoc docstring.
- Octave range is unchanged: `c_minPitchOctave = -1`, `c_maxPitchOctave = 11`. `C-1` = MIDI 0 is the system minimum and SP1's "MIDI number is always >= 0" rule stands. **Do not widen the range.**
- Accidental symbols, canonical in both directions: `bb 3b b 1b 1x # 3x x`.
- Ties round **upward**, everywhere rounding occurs.
- Analysis rejects quarter tones; it never answers from a rounded value.
- `export VCToolsInstallDir='C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'` in every build command — single quotes, the value ends in a backslash.
- `rm -f build/Windows/cpp-tests/cpp-tests.exe` before re-running the C++ suite after any library edit; `tests-cpp` links `maiacore.lib` via `-l` and ignores its mtime.
- `make cpp-tests` / `make py-tests` **always exit 0** — read the output. And never read an exit code through a pipe: `cmd | tail` reports `tail`'s status. Capture `$?` directly.
- **Run the C++ suite at least 3 times** per task. This project has shipped undefined behaviour that a single green run hid for its entire history.
- Baselines entering this work: **C++ 832/832, Python 264/264.**
- Python work needs a brand-new clean Python 3.12 venv, `make clean` before `make dev`, and import checks run from **outside** the repo root (the repo's own `maialib/` directory shadows the installed package).
- `CMakeLists.txt` collects sources with `file(GLOB ...)` (`:110-112`, `:146-147`), so new `.cpp` files need **no** CMake edit — but globs evaluate at configure time, so a new file requires a fresh configure. `make clean` then rebuild, or delete `build/Windows/cpp-tests/CMakeCache.txt`.

---

## File Structure

**Created:**
- `maiacore/include/maiacore/pitch.h` — the `Pitch` class declaration. One responsibility: represent a single pitch and convert between its representations.
- `maiacore/src/maiacore/pitch.cpp` — its implementation.
- `maiacore/src/maiacore/python_wrapper/py_pitch.cpp` — `void PitchClass(const py::module&)`.
- `tests-cpp/src/pitch-test.cpp` — C++ tests for `Pitch`.
- `tests-cpp/src/quarter-tone-characterization-data.h` — generated table pinning pre-change behaviour.
- `test/test_pitch.py` — Python tests for `Pitch`.
- `test/xml_examples/unit_test/quarter_tone_tartini.xml`, `quarter_tone_arrow.xml`, `quarter_tone_accidental_only.xml` — MusicXML fixtures.

**Modified:** `constants.h` (accidental table), `helper.h`/`helper.cpp` (whitelist, truncation, `optional` octave, transposition), `note.h`/`note.cpp` (composition, transposition), `chord.h`/`chord.cpp` (rejection guard, `roundQuarterTones`), `interval.cpp` (rejection guard), `score.h`/`score.cpp` (MusicXML read), `py_helper.cpp`, `py_note.cpp`, `py_chord.cpp`, `py_maiacore.cpp` (bindings).

---

### Task 1: Characterisation table

**Files:**
- Create: `tests-cpp/src/quarter-tone-characterization-data.h`
- Test: `tests-cpp/src/pitch-test.cpp` (created here, extended later)

**Interfaces:**
- Produces: `struct CharEntry { const char* pitch; int midiNumber; const char* enharmonicDefault; };` and `kSemitoneCharTable[]`, consumed by Task 4's regression check.

- [ ] **Step 1: Write a generator that prints the current behaviour**

Create a temporary `tests-cpp/src/gen-char.cpp` with a `TEST(GenChar, dump)` that loops every step in `{C,D,E,F,G,A,B}`, every symbol in `{"bb","b","","#","x"}` and octaves `-1..11`, calling `Helper::pitch2midiNote(p)` and `Note(p).getEnharmonicPitch(false)` inside a `try`, printing one `{"C#4", 61, "Db4"},` line per pitch that does not throw.

- [ ] **Step 2: Run it and capture the output**

```bash
export VCToolsInstallDir='C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'
rm -f build/Windows/cpp-tests/cpp-tests.exe
make build-cpp-tests
./build/Windows/cpp-tests/cpp-tests.exe --gtest_filter=GenChar.dump > char.txt; echo $?
```

- [ ] **Step 3: Write the header from that output**

Wrap the captured lines in the struct above. Add a header comment: "Generated once from pre-SP2 behaviour. Quarter tones are pure addition; none of these answers may change."

- [ ] **Step 4: Delete the generator, add the regression test**

```cpp
TEST(Characterization, semitoneBehaviourIsUnchanged) {
    for (const auto& e : kSemitoneCharTable) {
        EXPECT_EQ(Helper::pitch2midiNote(e.pitch), e.midiNumber) << e.pitch;
        EXPECT_EQ(Note(e.pitch).getEnharmonicPitch(false), e.enharmonicDefault) << e.pitch;
    }
}
```

- [ ] **Step 5: Run the suite 3 times, true exit code each**

Expected: 832 + 1 tests, all passing.

- [ ] **Step 6: Commit**

```bash
git add tests-cpp/src/quarter-tone-characterization-data.h tests-cpp/src/pitch-test.cpp
git commit -m "test: pin pre-SP2 semitone behaviour with a characterization table"
```

---

### Task 2: `Pitch` standalone — canonical state and string representations

**Files:**
- Create: `maiacore/include/maiacore/pitch.h`, `maiacore/src/maiacore/pitch.cpp`
- Test: `tests-cpp/src/pitch-test.cpp`

**Interfaces:**
- Consumes: `Helper::splitPitch`, `Helper::alterSymbol2Value`, `Helper::alterValue2symbol`, `c_diatonicStepSemitones`.
- Produces: `class Pitch` with `getPitch/getPitchClass/getPitchStep/getAlterSymbol/getAlter/getOctave/getMidiNumber/getQuarterToneSteps/isRest`, setters `setStep/setAlter/setOctave/setPitch/setPitchClass/setMidiNumber`, and `roundToSemitone()`. `getOctave()` returns `std::optional<int>`.

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(Pitch, quarterToneStringRoundTrip) {
    Pitch p("C1x4");
    EXPECT_EQ(p.getPitchStep(), "C");
    EXPECT_FLOAT_EQ(p.getAlter(), 0.5f);
    EXPECT_EQ(p.getOctave().value(), 4);
    EXPECT_EQ(p.getPitch(), "C1x4");
    EXPECT_EQ(p.getMidiNumber(), 61);          // ties round up
    EXPECT_FLOAT_EQ(p.getQuarterToneSteps(), 60.5f);
}

TEST(Pitch, restHasNoOctave) {
    Pitch r("rest");
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());
    EXPECT_EQ(r.getMidiNumber(), MUSIC_XML::MIDI::NUMBER::MIDI_REST);
    EXPECT_EQ(r.getPitch(), "rest");
}

TEST(Pitch, setAlterRejectsNonMultipleOfHalf) {
    Pitch p("C4");
    EXPECT_THROW(p.setAlter(0.3f), std::runtime_error);
}

TEST(Pitch, roundToSemitoneTiesUp) {
    Pitch p("C1x4");
    p.roundToSemitone();
    EXPECT_EQ(p.getPitch(), "C#4");
}
```

- [ ] **Step 2: Run them and confirm they fail to compile** (`pitch.h` does not exist)

- [ ] **Step 3: Write `pitch.h`**

Declare the class exactly as spec §4.2 lists it, `#include <optional>`, with Doxygen on every public member. `getOctave()`'s Doxygen states that an empty optional means a rest and that `isRest()` is the authoritative test.

- [ ] **Step 4: Write `pitch.cpp`**

`getMidiNumber()` is `12 * (octave + 1) + c_diatonicStepSemitones[stepIdx] + lround(alter)`, returning `MIDI_REST` when `isRest()`. `getQuarterToneSteps()` is the same without rounding. `setAlter` throws via `LOG_ERROR` unless `alter * 2` is integral and `alter` is within `[-2, 2]`. `roundToSemitone()` sets `_alter = std::floor(_alter + 0.5f)`.

- [ ] **Step 5: Reconfigure (the GLOB must pick up the new file), rebuild, run 3 times**

```bash
make clean && make build-cpp-tests
./build/Windows/cpp-tests/cpp-tests.exe > r.txt 2>&1; echo $?
```

- [ ] **Step 6: Commit**

```bash
git add maiacore/include/maiacore/pitch.h maiacore/src/maiacore/pitch.cpp tests-cpp/src/pitch-test.cpp
git commit -m "feat: Pitch class with canonical (step, alter, octave) state"
```

---

### Task 3: `Pitch` — frequency and the MIDI/frequency constructors

**Files:** Modify `pitch.h`, `pitch.cpp`; Test `tests-cpp/src/pitch-test.cpp`

**Interfaces:**
- Produces: `Pitch(int midiNumber, const std::string& accType)`, `Pitch(float frequency, const std::string& accType, float freqA4, bool enableQuarterToneRound)`, `getFrequency(float freqA4 = 440.0f)`, `setFrequency(...)`.

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(Pitch, frequencyOfA4) {
    EXPECT_NEAR(Pitch("A4").getFrequency(), 440.0f, 0.01f);
}

TEST(Pitch, fromFrequencyRoundsToSemitoneByDefault) {
    Pitch p(444.0f);                       // between A4 and A1x4
    EXPECT_EQ(p.getPitch(), "A4");
}

TEST(Pitch, fromFrequencyRoundsToQuarterToneWhenEnabled) {
    Pitch p(449.0f, "#", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "A1x4");
}

TEST(Pitch, nonPositiveFrequencyIsARest) {
    EXPECT_TRUE(Pitch(0.0f).isRest());
    EXPECT_TRUE(Pitch(-5.0f).isRest());
}
```

- [ ] **Step 2: Run and confirm failure**

- [ ] **Step 3: Implement**

`getFrequency` returns `0.0f` for a rest; otherwise reads `getTuningSystem()` and, for `EQUAL_TEMPERAMENT`, returns `freqA4 * std::pow(2.0f, (getQuarterToneSteps() - 69.0f) / 12.0f)`. Any other enum value throws `LOG_ERROR("Tuning system not implemented in SP2; only EQUAL_TEMPERAMENT is available")`. `setFrequency` maps frequency to quarter-tone steps, rounds to the nearest 1.0 or 0.5 depending on the flag with ties upward, then spells via `Helper::midiNote2pitch`-style logic.

- [ ] **Step 4: Run 3 times; Step 5: Commit**

```bash
git commit -m "feat: Pitch frequency, plus MIDI and frequency constructors"
```

---

### Task 4: Widen the accidental vocabulary

**Files:** Modify `maiacore/include/maiacore/constants.h:103`, `maiacore/src/maiacore/helper.cpp:249`; Test `tests-cpp/src/helpers-test.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST(splitPitch, acceptsQuarterTones) {
    std::string pc, step, sym; int oct; float alter;
    Helper::splitPitch("C1x4", pc, step, oct, alter, sym);
    EXPECT_EQ(sym, "1x");
    EXPECT_FLOAT_EQ(alter, 0.5f);
    EXPECT_EQ(oct, 4);
}
```

> **Forward dependency:** Task 5 changes `splitPitch`'s octave out-parameter from `int&` to `std::optional<int>&`. This test is written against the current signature and **Task 5 updates it**, along with every call site Tasks 2 and 4 create. Do not try to anticipate the new signature here; Task 5 owns that migration in one place.

- [ ] **Step 2: Run — expect a throw from `helper.cpp:1419` ("Unknown alter symbol: 1x")**

- [ ] **Step 3: Widen the table**

```cpp
const std::array<std::string, 8> c_alterSymbol = {"bb", "3b", "b", "1b", "1x", "#", "3x", "x"};
```

- [ ] **Step 4: Delete the truncation at `helper.cpp:249`** — remove `static_cast<int>` and the two `// SP2:` comment lines, returning the quarter-tone-aware value.

- [ ] **Step 5: Run 3 times — the Task 1 characterisation table must still pass unchanged**

- [ ] **Step 6: Commit**

```bash
git commit -m "feat: accept quarter-tone accidentals in pitch strings"
```

---

### Task 5: `optional` octave across the free functions

**Files:** Modify `helper.h:320`, `helper.h` (`midiNote2octave`), `helper.cpp:281-289`, `helper.cpp:1380`, `score.h:438`, `py_helper.cpp`; Test `tests-cpp/src/helpers-test.cpp:261`, `test/test_helpers.py:409`

> **DELIBERATE EXPECTATION CHANGES — DO NOT "REPAIR" THE CODE TO MATCH THE OLD TESTS.**
> `tests-cpp/src/helpers-test.cpp:261` asserts `midiNote2octave(MIDI_REST) == -2`; it becomes an empty optional.
> `test/test_helpers.py:409` asserts `splitPitch("rest")` yields octave `0`; it becomes `None`.

- [ ] **Step 1: Update both existing tests to the new expectations**

```cpp
EXPECT_FALSE(Helper::midiNote2octave(MUSIC_XML::MIDI::NUMBER::MIDI_REST).has_value());
```

```python
self.assertEqual(ml.Helper.splitPitch(pitch="rest"), ("rest", "rest", None, 0.0, ""))
```

- [ ] **Step 2: Run — expect failure**

- [ ] **Step 3: Change the signatures**

`Helper::midiNote2octave` returns `std::optional<int>`, returning `std::nullopt` when `midiNote < 0`. `Helper::splitPitch`'s `int& octave` becomes `std::optional<int>&`, left empty in the rest branch at `helper.cpp:1380`. `Score::getNoteNodeData`'s `int& alterValue` becomes `float&` in the same pass (spec §6).

- [ ] **Step 3b: Update every call site the earlier tasks created**

This task owns the whole migration, so nothing else is left compiling against the old signature. Fix at minimum: `Pitch`'s string constructor (Task 2), `TEST(splitPitch, acceptsQuarterTones)` (Task 4), and every in-tree caller — `grep -rn "splitPitch\|midiNote2octave" maiacore/ tests-cpp/ --include=*.cpp --include=*.h` and work the list to zero.

- [ ] **Step 4: Update the pybind11 wrappers** in `py_helper.cpp`; `<pybind11/stl.h>` is already included, so the optional surfaces as `None`.

- [ ] **Step 5: Run C++ 3 times; then a full Python cycle** (fresh venv, `make clean`, `make dev`, `make py-tests`, import check from outside the repo).

- [ ] **Step 6: Commit**

```bash
git commit -m "refactor: a rest has no octave anywhere in the public API"
```

---

### Task 6: `Note` composes `Pitch`

**Files:** Modify `note.h:23-44` (fields), `note.cpp:74-87`, `:121-132`, `:140-157`, `:291`, `:743`, `:896`; Test `tests-cpp/src/note-test.cpp`, `test/test_note.py`

**Interfaces:**
- Consumes: the whole `Pitch` surface from Tasks 2-3.
- Produces: `Note` with a single `Pitch _writtenPitch` plus `_transposeDiatonic`/`_transposeChromatic`; `getSoundingPitch()` derived on demand.

This is the riskiest task in the plan. `Note` is used by `Chord`, `Measure`, `Part`, `Score`, MusicXML I/O and the bindings.

- [ ] **Step 1: Write the failing test that pins the bug this fixes**

```cpp
TEST(Note, setPitchClassUpdatesAccidentalAndMidi) {
    Note n("C4");
    n.setPitchClass("Eb");
    EXPECT_EQ(n.getAlterSymbol(), "b");
    EXPECT_EQ(n.getMidiNumber(), 63);
}
```

- [ ] **Step 2: Run — expect failure** (the known `setPitchClass` defect: stale accidental, no MIDI update).

- [ ] **Step 3: Replace the fields**

Delete `_writtenPitchClass`, `_writtenOctave`, `_soundingPitchClass`, `_soundingOctave`, `_midiNumber`, `_alterSymbol`. Add `Pitch _writtenPitch;`.

**This is also where the second truncation dies.** Spec §6 names two `static_cast<int>(alterValue)` sites: `helper.cpp:249` (removed in Task 4) and **`note.cpp:315`**, inside the enharmonic spelling logic, which carries the same `// SP2:` marker. Rewriting these fields removes it — verify with `grep -n "SP2:" maiacore/src/maiacore/` returning nothing when this task is done. `getSoundingPitch()` computes from `_writtenPitch` plus the transposing interval; the public getters — `getOctave()` (`:157`), `getPitch()` (`:743`), `getMidiNumber()` (`:896`), `getAlterSymbol()` (`:291`), `getPitchClass()` (`:138`) — forward to the **sounding** pitch, preserving today's semantics. Their signatures do not change.

- [ ] **Step 4: Run the full C++ suite 3 times and the Python suite once.** Every pre-existing test must pass; the characterisation table is the tripwire.

- [ ] **Step 5: Commit**

```bash
git commit -m "refactor: Note holds one canonical written Pitch"
```

---

### Task 7: MusicXML read

**Files:** Modify `score.cpp:646-661`, `:1644-1651`, `helper.cpp:398-431` (`alterName2symbol`); Create the three fixtures; Test `tests-cpp/src/score-test.cpp`

- [ ] **Step 1: Author the fixtures** — `quarter_tone_tartini.xml` (`<alter>0.5</alter>` + `<accidental>quarter-sharp</accidental>`), `quarter_tone_arrow.xml` (`sharp-down`), `quarter_tone_accidental_only.xml` (accidental, no `<alter>` — the MuseScore case).

- [ ] **Step 2: Write the failing tests** — load each fixture, assert the first note's pitch is `C1x4`.

- [ ] **Step 3: Run — expect the note to read as natural `C4`.**

- [ ] **Step 4: Implement** — one shared reader with precedence `<accidental>` → decimal `<alter>` → natural; delete the four-case switch and the `atoi`; grow `alterName2symbol` from 9 names to 13 by adding `quarter-sharp`, `quarter-flat`, `three-quarters-sharp`, `three-quarters-flat`.

- [ ] **Step 5: Run 3 times; Step 6: Commit**

```bash
git commit -m "fix: read quarter tones from MusicXML, preferring <accidental>"
```

---

### Task 8: MusicXML write

**Files:** Modify `note.cpp:766-769`, `:784-787`, `helper.cpp:320-354` (`alterValue2Name`), `score.cpp:1431-1437` (delete the commented block); Test `tests-cpp/src/note-test.cpp`

- [ ] **Step 1: Write the failing round-trip test** — build `Note("C1x4")`, call `toXML`, assert the output contains `<alter>0.5</alter>` and `<accidental>quarter-sharp</accidental>`, and that `Note("C4").toXML()` still contains `<alter>` written as an integer with no decimal part.

- [ ] **Step 2: Run — expect `<alter>0</alter>` and no accidental element.**

- [ ] **Step 3: Implement** — repoint `alterValue2Name`'s four quarter-tone outputs to the Tartini names; emit both elements; format `<alter>` with no decimals when integral and one decimal otherwise.

- [ ] **Step 4: Run 3 times; Step 5: Commit**

```bash
git commit -m "feat: write quarter tones to MusicXML as alter plus Tartini accidental"
```

---

### Task 9: Analysis rejection and the escape hatch

**Files:** Modify `interval.cpp:15`, `:32`, `chord.cpp` (`stackInThirds`), `chord.h`; Test `tests-cpp/src/chord-test.cpp`, `tests-cpp/src/interval-test.cpp`, `test/test_chord.py`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(Chord, analysisRejectsQuarterTones) {
    Chord c(std::vector<std::string>{"C4", "E1b4", "G4"});
    EXPECT_THROW(c.getName(), std::runtime_error);
}

TEST(Chord, roundQuarterTonesEnablesAnalysis) {
    Chord c(std::vector<std::string>{"C4", "E1b4", "G4"});
    EXPECT_EQ(c.roundQuarterTones(), 1);
    EXPECT_EQ(c.getName(), "C");
}

TEST(Interval, rejectsQuarterTones) {
    EXPECT_THROW(Interval("C4", "E1b4"), std::runtime_error);
}
```

- [ ] **Step 2: Run — expect wrong answers rather than throws.**

- [ ] **Step 3: Implement the guards** beside the existing rest guards at `interval.cpp:17-19` and `:34-36`, and at the top of `stackInThirds()`. The message names the offending note.

- [ ] **Step 4: Add `Chord::roundQuarterTones()`** — iterates notes, calls `Pitch::roundToSemitone()`, **calls `invalidateStackCache()`** (`chord.h:185`), returns the count changed.

- [ ] **Step 5: Write the coverage test** — walk every public analysis method of `Chord` and `Interval` with a quarter-tone input and assert each throws.

- [ ] **Step 6: Run 3 times; Step 7: Commit**

```bash
git commit -m "feat: reject quarter tones in analysis, with roundQuarterTones() as the escape hatch"
```

---

### Task 10: Exact enharmonics and fractional transposition

**Files:** Modify `helper.cpp` (`isEnharmonic`), `note.cpp:299` (`getEnharmonicPitch`), `note.h:593`, `chord.h:301`, `:307`, `helper.h:289`; Test `tests-cpp/src/helpers-test.cpp`, `note-test.cpp`, `chord-test.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(isEnharmonic, comparesExactPitch) {
    EXPECT_TRUE(Helper::isEnharmonic("C1x4", "D3b4"));
    EXPECT_FALSE(Helper::isEnharmonic("C1x4", "C#4"));
}

TEST(Note, transposeByQuarterTone) {
    Note n("C4");
    n.transpose(0.5f);
    EXPECT_EQ(n.getPitch(), "C1x4");
}

TEST(Note, transposeRejectsNonMultipleOfHalf) {
    Note n("C4");
    EXPECT_THROW(n.transpose(0.3f), std::runtime_error);
}
```

- [ ] **Step 2: Run and confirm failure.**

- [ ] **Step 3: Implement** — `isEnharmonic` compares `getQuarterToneSteps()`; the four transposition entry points widen to `float` with a multiple-of-0.5 validation. Document on `Chord::transpose` that transposing off the semitone grid throws, because it re-stacks.

- [ ] **Step 4: Run 3 times; Step 5: Commit**

```bash
git commit -m "feat: exact enharmonic comparison and fractional transposition"
```

---

### Task 11: Bindings, documentation and final verification

**Files:** Create `py_pitch.cpp`; Modify `py_maiacore.cpp:10-22` and `:29`, `py_note.cpp`, `py_chord.cpp`, `py_helper.cpp`; Create `test/test_pitch.py`; Modify `CHANGELOG.md`

- [ ] **Step 1: Write `py_pitch.cpp`** — `void PitchClass(const py::module& m)`, following `py_note.cpp`'s header block exactly (the five pybind11 includes, then `#include "maiacore/pitch.h"`). Every method gets a numpydoc docstring with Parameters, Returns, Raises and Examples.

- [ ] **Step 2: Register it** — add `void PitchClass(const py::module &);` to the declaration list at `py_maiacore.cpp:10-22` and `PitchClass(m);` to the module body beside `NoteClass(m);`.

- [ ] **Step 3: Write `test/test_pitch.py`** — mirror the C++ tests, including `self.assertIsNone(ml.Pitch("rest").getOctave())`.

- [ ] **Step 4: Update the docstrings** of every changed binding, and add a `CHANGELOG.md` entry under `## [Unreleased]`.

- [ ] **Step 5: Full verification** — `make clean`, fresh venv, `make dev`, `make py-tests`, import check from outside the repo, and the C++ suite **5 times** with true exit codes.

- [ ] **Step 6: Regenerate the AI docs** — `make dev` rewrites `AI_API_CHEATSHEET.md` and `llms-full.txt` from the stubs; commit them if they changed.

- [ ] **Step 7: Commit**

```bash
git commit -m "feat: Python bindings and documentation for quarter tone support"
```
