# Note Pitch Views and Diatonic Transposition (roadmap step 1a) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give `Note` three well-defined pitch views — Written (unprefixed getters are shortcuts for it), Sounding (simplest spelling of what sounds) and an internal concert spelling used by every analysis — with transposing instruments spelled by their diatonic interval.

**Architecture:** Two pure spelling functions (concert speller from written pitch + interval; simplest-spelling reducer) feed `Note`'s getters. Analyses switch to the concert view first (no change for untransposed notes), then the public getters change meaning, then documentation and the CHANGELOG record the Breaking changes.

**Tech Stack:** C++17 (maiacore), pybind11 bindings with numpydoc docstrings, GoogleTest, Python unittest, maiapy (Plotly).

**Spec:** `docs/superpowers/specs/2026-09-30-note-pitch-views-design.md` — the binding authority (decisions D1–D7, the examples table in §3, the internal-consumer rule in §4, the measurement rules in §5).

## Global Constraints
- Characterisation tables `tests-cpp/src/quarter-tone-characterization-data.h` (blob af945ba2) and `tests-cpp/src/pitch-spelling-legacy-data.h` (blob 442de319) are never edited and stay green.
- Every changed test expectation is listed in the task report as old → new with the spec decision that justifies it; every new test is proven to fail under a targeted mutation of the code it protects.
- Every public maiacore method keeps a pybind11 binding with an accurate numpydoc docstring; Doxygen and numpydoc are updated in the same commit as the behaviour.
- The concert view is not bound to Python and not documented as public API (spec D5).
- Docs, comments, docstrings, CHANGELOG in technical English; comments explain the code, never its development history.
- Never stage the user's uncommitted `.gitignore` line `musescore/*`; stage files by name.
- Commit messages end with:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Baselines: C++ **1050/1050**, Python **472/472**.
- Test workflow: `make dev` only inside a brand-new `py -3.12` venv outside the repository with `requirements-dev.txt`; `make cpp-tests`, `make py-tests`, `make validate`, `make msvc-gate` exit non-zero on failure (read exit codes directly). For clang builds set, from PowerShell, `$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'` with `INCLUDE`/`LIB` unset.

---

### Task 0: Measure the current behaviour

**Files:** none in the repository; artefacts in the SDD workspace.

- [ ] **Step 1:** Write a measurement program (Python against a module built from `main`, or a scratch C++ test) that records, for every written spelling with each of the nine accidentals (`bb 3b b 1b '' 1x # 3x x`) on every step, octaves 3–5, × intervals (−1,−2) (−2,−3) (−4,−7) (−5,−9) (1,2) (2,3) (7,12) (−7,−12) (−8,−14) (0,−2) (0,0): `getPitch`, `getOctave`, `getPitchClass`, `getPitchStep`, `getAlterSymbol`, `getWrittenPitch`, `getWrittenOctave`, `getSoundingPitch`, `getSoundingOctave`, `getSoundingPitchClass`, `getSoundingPitchStep`, `getDiatonicSoundingPitchClass`, `getMidiNumber`, `getQuarterToneSteps`, `getEnharmonicPitch(False/True)`, or the exception text. Save as CSV (`before-note-views.csv`).
- [ ] **Step 2:** Record analysis output for the repository's XML fixtures and the 7 samples that load: `Score.getChords()` chord names (and the notes' pitches), and for every consecutive pair of notes in each part the `Interval` name; plus the melody-pattern search results used by the tests. Save as `before-analyses.csv`.
- [ ] **Step 3:** Note which fixtures contain transposing parts (`<transpose>`), so Task 4's diff can be read against them.

### Task 1: The two spelling functions

**Files:** `maiacore/src/maiacore/note.cpp` (or a new internal source/header pair next to it, e.g. `pitch-views.h/.cpp` in a `detail` namespace — choose the narrowest), `tests-cpp/src/note-test.cpp` (or a new `tests-cpp/src/pitch-views-test.cpp` registered in `tests-cpp/CMakeLists.txt`).

**Interfaces (produces):**
- `Pitch concertSpelling(const Pitch& written, int transposeDiatonic, int transposeChromatic)` — spec D4: letter = written step + diatonic (octave carry), alter = exact target position − natural position of that letter/octave; returns `written` when both intervals are 0; falls back to the current chromatic rule when `transposeDiatonic == 0 && transposeChromatic != 0`, when the alter is not one of the nine grid values in [−2, 2], or when the octave leaves −1..11. Never throws for a representable result; the existing below-floor / above-ceiling rejections keep their current messages.
- `Pitch simplestSpelling(const Pitch& pitch)` — spec D3: among the spellings of the same exact position choose the smallest `|alter|`; on a tie keep the side (sharp/flat) of `pitch`'s accidental; returns rests unchanged.

- [ ] **Step 1: Failing tests** for every example in spec §2 (D3 and D4 lists) and §3's concert column, the fallback cases (d=0,c≠0; an alter beyond ±2 such as a written `x` moved by an augmented interval; octave overflow), and quarter tones.
- [ ] **Step 2:** Implement; reuse `Pitch(step, alter, octave)`, `c_diatonicStepSemitones`, `Helper::steps2pitch`, the grid helpers in `utils.h`, and `spellFromWhiteKey` where it fits.
- [ ] **Step 3:** Prove each test can fail (mutate the tie rule, the octave carry, the fallback condition).
- [ ] **Step 4:** Suites unchanged otherwise (1050 + new tests; Python 472). Commit: `feat: concert and simplest spellings for transposed pitches`.

### Task 2: Analyses use the concert spelling

**Files:** `maiacore/src/maiacore/chord.cpp`, `interval.cpp`, `score.cpp`, `note.cpp`/`note.h` (internal accessor), tests.

- [ ] **Step 1:** Add an internal accessor on `Note` returning `concertSpelling(_writtenPitch, _transposeDiatonic, _transposeChromatic)` (not bound, not documented as public API — spec D5).
- [ ] **Step 2:** Review every use of `getPitch`, `getOctave`, `getPitchClass`, `getPitchStep`, `getAlterSymbol`, `getEnharmonicNote(s)` in `chord.cpp`, `interval.cpp`, `score.cpp` and switch each pitch-relationship use to the concert view; keep a use on the written/sounding view only with a reason. Record the per-site decision list in the report.
- [ ] **Step 3:** Untransposed notes: every existing analysis test passes unchanged. Transposed notes: add tests showing correct concert relationships (e.g. a B♭ clarinet written `D4` against a violin `C4` is a unison; the chord of a horn in F written `B4` with `C4`/`G4` is C major).
- [ ] **Step 4:** Commit: `fix: analyses relate transposed notes by their concert spelling`.

### Task 3: Public getter semantics

**Files:** `maiacore/include/maiacore/note.h`, `maiacore/src/maiacore/note.cpp`, `maiacore/src/maiacore/python_wrapper/py_note.cpp` (and any other binding whose docstring describes these getters), `maialib/maiapy/plots.py`, `maialib/maiapy/sethares_dissonance.py`, tests in `tests-cpp/src/` and `test/`.

- [ ] **Step 1:** Unprefixed pitch getters return the written view (spec D1); `getSounding*` return `simplestSpelling(concert)` and its own octave (D1/D3); acoustic getters unchanged (D2); `getSoundingPitch` no longer glues an arithmetic octave; `transpose()` moves the written pitch once and `toEnharmonicPitch()` respells the written pitch (D6); the fallback stays silent and is documented (D7).
- [ ] **Step 2:** Update every pinned expectation that the decisions change, listing each old → new with its decision (known areas: `note-test.cpp` transposing tests ~138-169, ~1222-1406, ~1874-2080 and the below-floor/ceiling tests; `test_note.py` ~91-120, ~302-326, ~460-499, ~664-802; any analysis test Task 2 did not already cover).
- [ ] **Step 3:** New tests for each spec §3 row through the public API (C++ and Python), for `transpose(0)` being a no-op on a transposed note, for `toEnharmonicPitch()` keeping `getMidiNumber()`, and for the ceiling now accepting spellable `B#11`/`Bx11` concert results; each proven to fail under a targeted mutation.
- [ ] **Step 4:** maiapy display sites switch to the Sounding view (spec §4); their outputs for untransposed scores are unchanged except for Cb/Fb/E#/B#/double-accidental labels.
- [ ] **Step 5:** Doxygen and numpydoc for every getter whose meaning changed, and for the three views as a whole (a short section in `Note`'s class docs). Commit: `feat!: Note's unprefixed pitch getters describe the written part; Sounding getters give the simplest spelling`.

### Task 4: Measure again, document, verify

**Files:** `CHANGELOG.md`, `BUILDING.md`/other docs if they describe these getters.

- [ ] **Step 1:** Re-run Task 0's programs on HEAD; diff against the before files. Every difference must be explained by a spec decision; for untransposed notes the analyses must be identical. Save the diffs and a summary table in the report.
- [ ] **Step 2:** CHANGELOG `[Unreleased]`: the three views and the diatonic transposition under Added/Changed, and every spec §6 item marked **Breaking** with a one-line migration hint (e.g. "use `getSoundingPitch()` where the concert pitch was meant").
- [ ] **Step 3:** Final verification from a clean tree in a brand-new venv: `make dev`, `make cpp-tests`, `make py-tests`, `make validate`, `make msvc-gate` (and `make linux-gate` if WSL has `cmake`). Commit: `docs: CHANGELOG for Note's pitch views and diatonic transposition`.

### Task 5: Infer the diatonic interval when only the chromatic one is given (user decision 2026-10-01)

**Files:** `maiacore/src/maiacore/note.cpp` and `pitch-views.h` (the concert speller), the documentation that describes the fallback (`note.h`, `chord.h`, `score.h`, `py_note.cpp`, `py_chord.cpp`, `py_score.cpp`, `CHANGELOG.md`), tests in `tests-cpp/src/` and `test/`.

- [ ] **Step 1:** In `concertSpelling`, when `transposeDiatonic == 0 && transposeChromatic != 0`, infer the diatonic count by spec D4 (`d = sign(c) · (7 · ⌊|c| / 12⌋ + T[|c| mod 12])`, `T = [0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6]`) and spell diatonically; the chromatic rule stays the fallback only for an alter outside the nine accidentals or an octave outside −1..11. `getTransposeDiatonic()` keeps returning the stored value. Keep the inference free of overflow at the limits of `int`.
- [ ] **Step 2:** Update every pinned expectation the inference changes (the speller's `d = 0` fallback tests, Note tests and docstring examples on intervals with `d = 0, c ≠ 0`), listing each old → new with this decision. New tests: every `|c| mod 12` in both directions, compound intervals (±12, −14, −21, −24), the tritone (±6 → ±3), the new and changed spec §3 rows (written `F#4` on (0, −2) → `E4`; `C4` on (0, −2) → `Bb3`; a B♭ clarinet's written `Cbb4` → fallback `Ab3`), the ceiling (a written `A#11` on (0, 2) is accepted as `B#11`), `getTransposeDiatonic()` still 0, and the analyses (`Interval(E4, Note("F#4", transposeChromatic=-2))` is a unison; `C4` + that note + `G4` is named). Each proven to fail under a targeted mutation.
- [ ] **Step 3:** Every description of the fallback on the branch states the inference (Doxygen and numpydoc of the Sounding getters, `getSoundingPitch()`'s rule, the constructors and `setTransposingInterval()`, the `==`/hash/Chord-display and `getChords`/`Chord` class clauses, the CHANGELOG `[Unreleased]` entries — still relative to v1.10.3). Find them with a search for "fallback", "chromatic rule", "diatonic count of 0", "`<diatonic>`", "Fb4".
- [ ] **Step 4:** Re-run `measure_note_views.py` (label `task5`) and compare with `task4` and with `before`: only rows on intervals with `d = 0, c ≠ 0` may change, each explained by the inference. The fixtures and samples have no chromatic-only `<transpose>` (Task 0's inventory), so the analyses need no re-run unless that inventory says otherwise.
- [ ] **Step 5:** Suites: clang `make cpp-tests`, `make py-tests` in a brand-new venv, `make validate`, the doctests, `make msvc-gate`. Commit: `feat: a transposing interval given only in semitones is spelled by its conventional diatonic interval`.
