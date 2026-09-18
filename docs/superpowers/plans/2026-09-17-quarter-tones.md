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

// Flat-side cases. Every test above uses a positive alter, where the two
// candidate rounding rules give identical answers; only these distinguish
// ties-upward from ties-away-from-zero. Added during execution after Task 4
// shipped that exact bug through a sharp-side-only suite.
TEST(Pitch, midiNumberRoundsHalfUpOnFlatSide) {
    EXPECT_EQ(Pitch("D1b4").getMidiNumber(), 62);   // 61.5 -> 62, not 61
    EXPECT_EQ(Pitch("D3b4").getMidiNumber(), 61);   // 60.5 -> 61, not 60
    EXPECT_EQ(Pitch("C1x-1").getMidiNumber(), 1);   // negative octave, sharp side
    EXPECT_FLOAT_EQ(Pitch("D1b4").getQuarterToneSteps(), 61.5f);
}
```

- [ ] **Step 2: Run them and confirm they fail to compile** (`pitch.h` does not exist)

- [ ] **Step 3: Write `pitch.h`**

Declare the class exactly as spec §4.2 lists it, `#include <optional>`, with Doxygen on every public member. `getOctave()`'s Doxygen states that an empty optional means a rest and that `isRest()` is the authoritative test.

- [ ] **Step 4: Write `pitch.cpp`**

`getMidiNumber()` computes the exact value in floating point and rounds **half upward**, returning `MIDI_REST` when `isRest()`:

```cpp
const float exact = 12.0f * (octave + 1) + c_diatonicStepSemitones[stepIdx] + _alter;
return static_cast<int>(std::floor(exact + 0.5f));
```

> **Corrected during execution.** This step originally specified `lround(alter)`. `std::lround` rounds half *away from zero*, so it disagrees with spec §4.5 (ties upward) on negative alters: `lround(-0.5)` is −1 where the spec requires 0. The two agree on positive alters, which is why a sharp-side-only test suite cannot tell them apart — Task 4 shipped exactly this bug past its implementer and halfway past its reviewer for that reason. `std::floor(x + 0.5f)` rounds half up for both signs and matches what `roundToSemitone()` already does below. Requires `#include <cmath>`. `getQuarterToneSteps()` is the same without rounding. `setAlter` throws via `LOG_ERROR` unless `alter * 2` is integral and `alter` is within `[-2, 2]`. `roundToSemitone()` sets `_alter = std::floor(_alter + 0.5f)`.

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

### Task 6b: A rest has no octave in `Note`

**Files:**
- Modify: `maiacore/include/maiacore/note.h` (declarations of `getOctave`, `getWrittenOctave`, `getSoundingOctave`; add `#include <optional>`)
- Modify: `maiacore/src/maiacore/note.cpp` (the three definitions)
- Modify: `maiacore/src/maiacore/python_wrapper/py_note.cpp` (bindings and numpydoc)
- Modify: every call site the compiler names (expect `chord.cpp`, `measure.cpp`, `score.cpp`)
- Modify: `CHANGELOG.md`
- Test: `tests-cpp/src/note-test.cpp`, `test/test_note.py`

**Interfaces:**
- Consumes: `Pitch::getOctave() -> std::optional<int>` (Task 2); `Note` holding one canonical `Pitch` (Task 6).
- Produces: `Note::getOctave()`, `Note::getWrittenOctave()`, `Note::getSoundingOctave()`, all `std::optional<int>`, empty for a rest.

**Why this is its own task.** Task 6 transplants the fields and leaves the three octave getters agreeing on the `-2` sentinel, so that the riskiest task in the plan changes no public signature. This task removes the sentinel from the public API behind its own review gate. Spec §4.4 and the paragraph following it require that a rest's octave carry no magic number — a sentinel is sound only outside the value's domain, and `0` and `-1` are both legitimate octaves. Spec §12.1 reconciled the two free functions and the MusicXML reader but never named `Note`, which is public and Python-bound; this task closes that gap.

- [ ] **Step 1: Write the failing test**

```cpp
TEST(Note, restHasNoOctaveAnywhere) {
    const Note rest("");
    EXPECT_FALSE(rest.getOctave().has_value());
    EXPECT_FALSE(rest.getWrittenOctave().has_value());
    EXPECT_FALSE(rest.getSoundingOctave().has_value());

    const Note note("C#4");
    EXPECT_EQ(note.getOctave().value(), 4);
    EXPECT_EQ(note.getWrittenOctave().value(), 4);
}
```

- [ ] **Step 2: Run to verify it fails**

Run: `make cpp-tests`
Expected: a compile error — `has_value()` called on `int`.

- [ ] **Step 3: Widen the three declarations**

In `note.h`, add `#include <optional>` and change all three to return `std::optional<int>`, updating each Doxygen block to state that an empty optional means a rest and that `isNoteOff()` is the authoritative test.

- [ ] **Step 4: Propagate the optional in `note.cpp`**

Task 6 left each of these ending in `.value_or(-2)`. Delete that call and return the optional unchanged, so the absence travels instead of a number:

```cpp
std::optional<int> Note::getWrittenOctave() const { return _writtenPitch.getOctave(); }
```

Apply the same removal to `getSoundingOctave()` and `getOctave()`. **No `value_or` may remain in any of the three.**

- [ ] **Step 5: Rebuild and fix every call site the compiler names**

Run: `make cpp-tests`. At each site, decide explicitly whether a rest belongs there. Where the surrounding code already rejects rests, `.value()` is correct; where it does not, handle the empty case rather than reaching for a default.

- [ ] **Step 6: Update the bindings**

In `py_note.cpp`, update the numpydoc `Returns` section of each of the three getters to `int | None`, stating that `None` means a rest. `<pybind11/stl.h>` is already included, so the conversion needs no extra code.

- [ ] **Step 7: Python test**

```python
def test_rest_has_no_octave(self):
    rest = ml.Note("", isNoteOn=False)
    self.assertIsNone(rest.getOctave())
    self.assertIsNone(rest.getWrittenOctave())
    self.assertIsNone(rest.getSoundingOctave())
    self.assertEqual(ml.Note("C#4").getOctave(), 4)
```

- [ ] **Step 8: Deliberate expectation changes**

Any existing test asserting `-2` or `0` as a rest's octave on a `Note` is now stale and must be updated to assert absence — not "repaired" by reintroducing a sentinel. Record each one you changed in the commit body.

- [ ] **Step 9: Verify**

Run `make cpp-tests` and `make py-tests`. Then mutation-test the new tests: restore `.value_or(-2)` in one getter, rebuild, confirm `restHasNoOctaveAnywhere` fails, revert, confirm it passes. Only the experiment counts.

- [ ] **Step 10: CHANGELOG and commit**

Add a breaking-change entry under `[Unreleased]` stating that `Note`'s three octave getters return `int | None`, with `None` for a rest.

```bash
git add maiacore/include/maiacore/note.h maiacore/src/maiacore/note.cpp \
        maiacore/src/maiacore/python_wrapper/py_note.cpp \
        tests-cpp/src/note-test.cpp test/test_note.py CHANGELOG.md
git commit -m "refactor: a rest has no octave in Note either"
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


### Task 7b: Delete the dead Score readers

**Files:**
- Modify: `maiacore/include/maiacore/score.h` (remove three `getNote` declarations and the `getNoteNodeData` declaration)
- Modify: `maiacore/src/maiacore/score.cpp` (remove three `getNote` definitions and the `getNoteNodeData` definition)
- Modify: `maiacore/include/maiacore/helper.h` (remove the `getNoteNodeData` declaration, which has no definition anywhere)
- Modify: `CHANGELOG.md`
- Test: no new tests; the compiler and linker are the proof, and both existing suites must hold

**Interfaces:**
- Consumes: nothing.
- Produces: nothing. This task only removes.

**Why this is its own task.** These are public C++ API removals. They deserve an isolated commit, a CHANGELOG breaking-change entry and their own review gate rather than being buried inside a MusicXML-read diff — the same reasoning that gave the octave migration its own Task 6b. The removal also closes two findings from Task 7's review for free (M2: `Score::getNote` still held an untouched copy of the four-case `-2/-1/1/2` switch the spec condemned; M3: the rest `octave` out-parameter left unwritten in both dead readers), and it finally satisfies spec §7.1's "one read path", which Task 7 could not achieve while a duplicate reader still existed.

**What is NOT being removed — read this before touching anything.** There are three different `getNote` methods in this project. `Chord::getNote` and `Measure::getNote` are heavily used internal helpers **and** are bound to Python — they appear in `part.cpp`, `chord.cpp`, `chord.h`, `score.cpp`, `py_score.cpp`, `py_measure.cpp`, `py_chord.cpp` and `maialib/maiapy/plots.py`. **Only `Score::getNote` is dead.** Deleting either of the others would break the library and the Python package.

- [ ] **Step 1: Verify the targets are dead yourself — do not trust this brief**

Run these and read the output before deleting anything:

```bash
grep -rn "getNoteNodeData" --include=*.cpp --include=*.h --include=*.py . | grep -v "^./build/"
grep -rn "getNote(" --include=*.cpp --include=*.h --include=*.py . | grep -v "^./build/"
```

Expected: `getNoteNodeData` appears only as two declarations (`helper.h`, `score.h`) and one definition (`score.cpp`), with zero calls. Every live `getNote(` call is on a `Chord` or a `Measure` object, never on a `Score`. `Score::getNote`'s only callers are its own short overloads delegating to the long one.

If what you find disagrees with that, **stop and report** rather than deleting.

- [ ] **Step 2: Delete the three `Score::getNote` overloads**

Remove the three declarations from `score.h` and the three definitions from `score.cpp`, including their Doxygen blocks. Two of the definitions exist only to delegate to the third, so all three go together.

- [ ] **Step 3: Delete `Score::getNoteNodeData`**

Remove the declaration from `score.h` and the definition from `score.cpp`, with its Doxygen block.

- [ ] **Step 4: Delete `Helper::getNoteNodeData`**

Remove the declaration from `helper.h`. It has no definition anywhere in the repository, so any caller would already have been a link error — which is itself the proof that none exists.

- [ ] **Step 5: Build — the toolchain is the test**

Run: `make cpp-tests`
Expected: compiles and links cleanly. A link error here would mean a caller existed after all; if that happens, stop and report rather than restoring blindly.

Delete `cpp-tests.exe` before rebuilding — `tests-cpp/CMakeLists.txt` links `maiacore` by bare name and Make does not track it, so a stale binary would hide a real failure. Set the MSVC environment (VC 14.40.33807 + Windows SDK 10.0.22621.0) from PowerShell, never from Git Bash. Read the log tail; the wrapper and `make cpp-tests` both exit 0 even on failure.

- [ ] **Step 6: Run both suites**

Run: `make cpp-tests` and `make py-tests`
Expected: C++ **906/906** and Python **282/282**, unchanged. Nothing should move — if a test fails, the deleted code was not dead and you must stop and report.

- [ ] **Step 7: CHANGELOG**

Add a breaking-change bullet under `[Unreleased]`, following the file's existing conventions, recording that `Score::getNote` (three overloads) and `Score::getNoteNodeData` are removed from the public C++ API, along with the unimplemented `Helper::getNoteNodeData` declaration. Note that none of them was ever bound to Python, so the Python package is unaffected. Do not edit the Task 5, 6, 6b or 7 entries.

- [ ] **Step 8: Commit**

```bash
git add maiacore/include/maiacore/score.h maiacore/src/maiacore/score.cpp \
        maiacore/include/maiacore/helper.h CHANGELOG.md
git commit -m "refactor!: remove the dead Score readers and the unimplemented Helper declaration"
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


### Task 9b: The MIDI-domain family the two chokepoints miss

**Files:**
- Modify: `maiacore/src/maiacore/chord.cpp` (the methods classified below)
- Modify: `maiacore/src/maiacore/python_wrapper/py_chord.cpp` (docstrings for any changed contract)
- Modify: `CHANGELOG.md`
- Test: `tests-cpp/src/chord-test.cpp`, `test/test_chord.py`

**Interfaces:**
- Consumes: the `Chord` and `Interval` guards from Task 9; `Pitch::roundToSemitone()`; `Chord::roundQuarterTones()`.
- Produces: no new API. This task decides, per method, between rejecting and computing correctly.

**Why this task exists.** Task 9 rejected quarter tones at two chokepoints — `Interval`'s constructor/`setNotes` and `Chord::stackInThirds()` — and its review proved those two cover the stacked-in-thirds and interval-quality surface completely. But a third family bypasses both, because it works in the MIDI integer domain and never builds an `Interval` or a stack. Measured on a `Chord{"C4", "E1b4", "G4"}` — a triad with a neutral third:

- `getMidiIntervals()` returns **`[4, 3]`**, identical to a plain C major triad, while its sibling `getIntervals()` correctly throws.
- `getMeanMidiValue()` returns `63`; `getMeanPitch()` returns `"D#4"`.
- `getHarmonicDensity()`, `isSorted()` and the `getMeanOfExtremes*`/`*Std` family answer likewise.

A confident wrong answer is exactly what Task 9 exists to prevent, and this is the largest remaining instance of it.

**The classification rule — and why the previous one failed.** Task 9's brief said a method counts as analysis if it *reaches a guard*. That rule is circular: it defines coverage by what is already covered, and by construction can never find a gap like this one. Replace it with a rule about representability:

> **Can the method's return type express a quarter tone?**
> If **no**, the only honest answer is to reject.
> If **yes**, rejecting destroys working functionality — compute the correct value instead.

- [ ] **Step 1: Classify every method yourself before changing anything**

Enumerate every public `Chord` method whose result derives from pitch and which reaches neither guard. For each, record the return type and which side of the rule it falls on. **Report the table to the controller before implementing.** My preliminary classification follows; treat it as a hypothesis to verify, not as instruction:

*Cannot represent — expected to reject:*
`std::vector<int> getMidiIntervals()`, `int getMeanMidiValue()`, `int getMeanOfExtremesMidiValue()`.

*Can represent — expected to compute correctly:*
`std::string getMeanPitch()` and `getMeanOfExtremesPitch()` (the string `"C1x4"` exists), `float getMidiValueStd()`, `bool isSorted()` (ordering is computable from a fractional alter).

*Frequency-based, therefore SP3 and NOT yours:*
`getMeanFrequency()`, `getMeanOfExtremesFrequency()`, `getFrequencyStd()`, `getSetharesDissonance()`. Leave them alone; their correct fix is for `Note::getFrequency()` to stop routing through a rounded MIDI number, which is SP3 tuning work.

*Undecided, measure and recommend:* `float getHarmonicDensity()` — it returns a float but takes MIDI bounds as `int`.

- [ ] **Step 2: `toCents()` is the opposite bug — it rejects and should not**

`std::vector<int> toCents()` is currently covered by the `Interval` guard, so a quarter tone makes it throw. **Cents are the one unit in this codebase that expresses a quarter tone exactly**: a quarter tone is 50 cents, a three-quarter tone 150. Integer cents lose nothing.

Measure what it does today, then make it compute the correct value rather than reject. Pin it with a test asserting a neutral third reads 350 cents, not a throw and not 300 or 400.

- [ ] **Step 3: Implement the rejections**

For the methods that cannot represent a quarter tone, reject in the style Task 9 established: the message names the offending note and points at `Chord::roundQuarterTones()`. `LOG_ERROR` throws; `LOG_WARN` only prints.

- [ ] **Step 4: Implement the corrections**

For the methods that can represent one, compute the true value. Do not round, and do not reach for `getMidiNumber()`, which rounds at `helper.cpp:255` — that rounding is the root cause of this whole family and fixing `getFrequency()` alone would not reach these methods, since they never touch frequency.

- [ ] **Step 5: Pin the two Task 9 methods that are covered but untested**

`getIntervals()` and — before your change — `toCents()` are covered by the `Interval` guard but pinned by no test. Add pins so neither coverage can be removed unnoticed.

- [ ] **Step 6: Tests**

Every rejecting method throws with a message naming the note, and works after `roundQuarterTones()`. Every computing method returns the correct value for a neutral third. **Do not use a bare `EXPECT_THROW`** — Task 9 proved it does not discriminate here, because a pre-existing guard in `Note::getEnharmonicPitch()` throws anyway; assert the message, as `EXPECT_REJECTED_NAMING` does.

Prove every test discriminates individually.

- [ ] **Step 7: Run both suites, then commit**

C++ **932/932** and Python **296/296** are the baselines. After rebuilding, assert **both** a fresh mtime on `cpp-tests.exe` **and** a `Linking CXX executable` line — `make` prints `Built target cpp-tests` without relinking when only a `maiacore` source changed, and two of Task 9's mutation rounds were invalidated by exactly that.

```bash
git commit -m "fix: reject or correct quarter tones in the MIDI-domain analysis family"
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
