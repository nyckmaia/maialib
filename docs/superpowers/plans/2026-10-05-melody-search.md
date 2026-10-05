# Melody Search, ScoreCollection and Live Note References (roadmap step 5) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Search every voice of every staff as a melodic line (a chord by its highest sounding note, tied notes as one event, the last window included), name a match's transposition without ever aborting, unify the result columns and threshold names, fix and expose `findAnyMelodyPattern`, make `ScoreCollection` construct, discover and replace its scores correctly, return live references from `Measure.getNote*`, and make `Measure::removeNote`/`addNote` do what they say.

**Architecture:** A new internal unit, `maiacore::detail::melodicLines()` (`melodic-lines.h/.cpp`, next to `pitch-views.h`), turns a score's parts into melodic lines of events; `Score::findMelodyPattern` builds them (and the concert keys of `concertKeys()`, step 1b) once per call and searches every window of every line in a file-local `searchMelodicLines()`, which only for a match computes `transposeSemitones` and, without throwing, `transposeInterval`. The list overload and `findAnyMelodyPattern` share one worker pool (`searchEachPattern()`) over the same lines; the note-event cache and `removeDuplicatePatterns` are deleted, and `findAnyMelodyPattern` keys windows by exact relative positions and durations. Results are a struct (`Score::MelodyPatternRow`, `ScoreCollection::MelodyPatternRow`) instead of tuples; the Python DataFrames are built by one header-only builder (`py_melody_dataframe.h`) that gives every column a fixed dtype, so an empty result keeps its columns. `ScoreCollection` gains a default constructor, `recursive`, English errors, case-insensitive sorted discovery and a strong guarantee in `setDirectoriesPaths`. The Measure bindings return `reference_internal`.

**Tech Stack:** C++17 (maiacore), pybind11 bindings with numpydoc docstrings, pandas (DataFrames built in the bindings), GoogleTest 1.14 (`tests-cpp/src/`, sources listed in `tests-cpp/CMakeLists.txt`), Python `unittest` (`test/`), the 4a corpus ledger (`test/musicxml/`).

**Spec:** `docs/superpowers/specs/2026-10-05-melody-search-design.md` is binding; every requirement maps to a task (see the self-review at the end). Facts with file:line and reproductions: the step 5 fact report (melody search, cache, ScoreCollection, bindings, `removeNote`). Test infrastructure: `test/musicxml/README.md`.

## Global Constraints

- C++17; no lambda captures a structured binding; every C++ file a task changes is formatted with `& 'C:\Program Files\LLVM\bin\clang-format.exe' -i <files>` (clang-format 18.1.6) before its tests run. The code below is already in clang-format 18 form: an anchor quoted from code an earlier task added matches that task's formatted text exactly once.
- `make validate` adds no cpplint or cppcheck finding (measured on the finished tree: "no new findings (46 known)", with 19 baseline findings no longer reported — leave `scripts/validate-baseline.json` as it is). Every `.cpp`/`.h` under `maiacore/src/maiacore/` that starts using a standard facility includes its header (`<algorithm>`, `<cctype>`, `<filesystem>`, `<functional>`, `<numeric>`, `<stdexcept>`, `<system_error>`, `<tuple>`, `<utility>`, `<vector>`); no C-style cast.
- Python in `maialib/` supports 3.8–3.14; test code added to an existing module follows its naming (camelCase helpers in `test_measure_comprehensive.py` and `test_score_comprehensive.py`, as they already use); the new `test/test_score_collection.py` passes `ruff check` and `ruff format --check` with `pyproject.toml`, and the lines added to existing modules add no `ruff check` finding beyond the module's existing kinds (no `SIM117`: one `with` for `subTest` and `assertRaises`).
- Docs, Doxygen, numpydoc, comments and commit messages in technical English. Comments explain the code, never its development history: no task numbers, review rounds, rulings or SHAs in code, tests or fixtures.
- Every new test is proven to fail under a targeted mutation of the code it protects: apply the named mutation with Edit, rebuild what the test runs against, run the test and record the failing output, revert the same Edit exactly, rebuild, rerun green. Record each mutation and its failing output in the task report. `git diff` after the revert shows only the task's intended changes. A test that cannot fail today is made exact.
- Never stage the user's uncommitted root `.gitignore` change (`musescore/*`); stage files by name, never `git add -A`/`.`; `git status --short` ends every task showing only ` M .gitignore`.
- `make dev` regenerates `AI_API_CHEATSHEET.md` and `llms-full.txt` when the API changes; every task restores them with `git checkout -- AI_API_CHEATSHEET.md llms-full.txt` before committing (they are regenerated at release, with the stubs and the notebooks' outputs).
- Commit messages end with these two lines:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Line endings: `core.autocrlf=true`, so every existing file is CRLF in the working tree and LF in the index; edits keep the file's endings (the Edit tool does). New files may be written with LF; git stores LF either way.
- Test environment: `make dev` only in a brand-new venv, `py -3.12`, created outside the repository at `C:\Users\nyck\AppData\Local\Temp\maialib-5-venv` (Task 0) with `pip install -r requirements-dev.txt`; the final verification uses a second brand-new venv `C:\Users\nyck\AppData\Local\Temp\maialib-5-final-venv`. Import checks run from outside the repository root. Read exit codes directly (`$LASTEXITCODE`, `$?`), never through a pipe. The Makefile's `PYTHON` is set only on the command line. Never use `vswhere -latest`.
- **«build»** below means this PowerShell prefix, repeated in every PowerShell call because shell state does not persist: `$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'; $env:INCLUDE = $null; $env:LIB = $null; $py = 'C:\Users\nyck\AppData\Local\Temp\maialib-5-venv\Scripts\python.exe';`
- **C++ subset:** «build» `make "PYTHON=$py" build-cpp-tests; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; .\build\Windows\cpp-tests\cpp-tests.exe --gtest_filter='<filter>'; $LASTEXITCODE` (the binary runs from the repository root, where the fixtures' relative paths start).
- **«pytest» X** below means, in Bash: `(cd /c/Users/nyck/Desktop/maialib/test && /c/Users/nyck/AppData/Local/Temp/maialib-5-venv/Scripts/python.exe -m unittest X -v); echo "exit $?"` — the Python tests run from `test/` against the package installed by «build» `make "PYTHON=$py" dev`.
- Never load a corpus file of 10 MB or more (`Symphony_9th.xml`, `xakypueri.xml`) in a unit test; the melody tests use the small fixtures below, the Bach samples and `test/xml_examples/Bach|Beethoven` (none of them 10 MB).
- This machine's antivirus adds an entry to every process's `TEMP`: a test that lists a directory lists one it created itself (`tempfile.TemporaryDirectory()`, or a uniquely named directory under `std::filesystem::temp_directory_path()`), never `TEMP` itself.
- Fixtures: small `score-partwise` 4.0 files `test/xml_examples/unit_test/melody_<rule>.musicxml`, valid against the MusicXML 4.0 schema (checked: `musicxml_check.py` reports `valid; errors: none; warnings: none` for each), with a `<key>` in measure 1 of every part. The task that adds fixtures runs «build» `make "PYTHON=$py" corpus-update-ledger` and commits only the added `test/musicxml/ledger.json` lines after `git diff test/musicxml/ledger.json` shows no other line changed and `git diff --stat test/musicxml/ledger-external.json` is empty. Each fixture's expected ledger line is given in its task (measured with the current reader: every one loads, exports valid XML and is `"roundtrip": "unstable"`, like `melody_patterns_quarter_tone_apart.xml`).
- Names (spec §4.3, D6): C++ thresholds `intervalSimilarityThreshold`, `rhythmSimilarityThreshold` in every overload (C++ and Python); result fields and columns `partName`, `measure`, `staff`, `voice`, `writtenKey`, `concertKey`, `transposeInterval`, `transposeSemitones`, `writtenPitches`, `soundingPitches`, `semitonesDiff`, `rhythmDiff`, `intervalSimilarity`, `rhythmSimilarity`, `totalSimilarity`; list overloads prepend `patternIdx`; collections prepend `fileName`, `composerName`, `scoreTitle` (after `patternIdx`); `findAnyMelodyPatternDataFrame` prepends `patternIdx`, `patternPitches`. Column dtypes: text `str` (pandas' string dtype for `dtype=str`: `str` under pandas 3, `object` under pandas 2), integers `int64`, reals `float64`, lists `object`.
- Error messages (first line, after `[maiacore] `): `Score::findMelodyPattern: a melody pattern needs at least 2 notes, and this one has <n>`; `Score::findAnyMelodyPattern: patternNumNotes must be at least 2, and it is <n>`; `Score::findAnyMelodyPattern: minOccurrences must be at least 1, and it is <n>`; `ScoreCollection::findMelodyPattern: a melody pattern needs at least 2 notes, and this one has <n>`; `ScoreCollection: '<path>' is not a directory, or does not exist`; `ScoreCollection: cannot read the directory '<path>'`. `std::out_of_range` messages (no prefix): `Measure::addNote: staff <s> is outside the measure's <n> staves`, `Measure::addNote: position <p> is past the end of staff <s>, which has <n> notes`, `Measure::removeNote: staff ...` / `note <i> is outside staff <s>, which has <n> notes`, `Measure::getNoteOn: ...`, `Measure::getNoteOff: ...`, `ScoreCollection::removeScore: index <i> is outside the collection of <n> scores`.
- Line hints (`~N`) give the line at cab545b; an earlier task's insertions move them. The quoted anchor text decides where an edit goes.

## Review Focus

The input classes and failure modes the spec implies but no other task test exercises, most likely to bite a user first; each line names the test that pins it and its task.

1. **`findAnyMelodyPatternDataFrame()` at its defaults (thresholds 1.0, `minOccurrences=2`: exact repetitions, transposed or not) on an orchestral score** still returns a large table, because doubled parts repeat each other's windows (Beethoven 5 sample, 13,675 notes: 2,164 patterns, 489,196 rows, 14.1 s on this machine) — expected: it completes, and its numpydoc gives these numbers and says that lower thresholds can give millions of rows. Pinned by `ScoreFindAnyMelodyPatternTestCase.test_the_docstring_gives_the_size_at_the_defaults` (Task 5).
2. **A chord tied to a chord** (piano writing): it is one event when its highest note continues the highest note, and a tie on an inner note alone extends nothing — expected: `E4` (two quarters), `G4`; and `E4`, `E4`. Pinned by `MelodicLines.ATiedChordIsOneEventWhenItsHighestNoteIsTied` (Task 3).
3. **A collection directory holding a subdirectory named like a score (`old.xml/`) and a file whose extension is outside the ANSI code page (`notes.ωδή`)** — expected: both skipped, the scores load (a directory is not loaded as a score; reading a non-ASCII extension never throws on Windows). Pinned by `ScoreCollectionConstructionTestCase.test_a_directory_named_like_a_score_and_a_non_ascii_name_are_skipped` (Task 6).
4. **A part with more measures than the first part** (`Part.addMeasure` on one part) — expected: its matches beyond the first part's measures have `concertKey == ""`, `writtenKey` set, and the search does not index past the concert keys. Pinned by `ScoreMelodyPatternSearch.AMeasureBeyondTheFirstPartHasNoConcertKey` (Task 4).
5. **A staff count that grows mid-part** (a measure with more staves than the first, as `Part.addStaves` leaves it) — expected: the new staff's voices are lines from that measure on. Pinned by `MelodicLines.AStaffAddedMidPartIsALineFromThatMeasure` (Task 3).

## Decisions this plan takes beyond the spec (reviewers: accept or overrule)

Decisions 5, 6 and 9 and the `minOccurrences` rule of §4.4 are now also in the spec (amended on 2026-10-05); they stay here for the reviewers of this plan.

1. **Results are structs, not tuples.** `Score::MelodyPatternRow` (15 named fields, `operator==` field by field), `Score::FoundMelodyPattern {pattern, matches}` (so the Python binding can give `patternPitches`) and `ScoreCollection::MelodyPatternRow {fileName, composerName, scoreTitle, match}`; `MelodyPatternTable` stays a vector of rows. The C++ search is not bound to Python, so only C++ callers see the change (breaking, listed in the CHANGELOG).
2. **The rows are sorted in C++.** `Score::findMelodyPattern` sorts its table stably by `measure` (rows are produced by line, so equal measures keep part, staff, voice and window order); the list overload is therefore already ordered by (`patternIdx`, `measure`, …); `ScoreCollection` sorts stably by `scoreTitle`, and its list overload's binding sorts the pattern-ordered rows stably by `scoreTitle`, giving (`scoreTitle`, `patternIdx`, …). No pandas sort remains.
3. **`ScoreCollection::findMelodyPattern` (list) returns one table per pattern** (each holding that pattern's matches in every score), as `Score`'s list overload does; it returned one table per score.
4. **An event stands for its representative note:** the highest sounding note of a chord, with that note's duration and ties; ties are merged after the chords are complete, so a tie counts only between representatives. A `<chord/>` note that follows no note of its voice on its staff (a chord spread over two staves) joins no event.
5. **Tied durations are summed exactly in ticks at `lcm(divisionsA, divisionsB, 1024)`**: `Duration` names a tick count by repeatedly adding half of a base length that can be 1 or 0 at small divisions, and loops forever there (`Duration(2, 3)` or `Duration(1, 3)` never return — an existing defect of `Helper::ticks2rhythmFigure`, see the report); a multiple of 1024 divisions keeps that half at least 2.
6. **`findAnyMelodyPattern` keys a window by each event's exact position relative to the window's first sounding note (infinity for a rest) and its quarter duration**, in a `std::set`, keeping the first window in line order. This is the spec's "intervals and durations exact" with rests told apart from notes (`[C4, rest, E4]` and `[C4, rest, C4]` are different patterns, which the comparison's "rest interval = 0" rule would merge). A pattern is kept when it has at least `minOccurrences` matches (its own window included; at least 1, default 2), and the thresholds default to 1.0 in C++ and Python (spec §4.4, as amended); `minOccurrences` comes right after the thresholds, before the callbacks, in both.
7. **`transposeInterval` checks `Note::isQuarterTone()` before building the `Interval`, and catches `std::runtime_error` only for the remaining unnameable intervals** (augmented ninth): `LOG_ERROR` resolves a stack trace for every throw, so throwing is kept off the common quarter-tone path; it runs only for matches.
8. **A pattern longer than every line returns an empty result in every overload**; the `"The melody pattern is bigger than the score"` error (score note count) is gone.
9. **`Measure::addNote` raises `std::out_of_range` for any staff outside the measure** (negative or too large; too large raised `RuntimeError` "Invalid 'staveId'"), and inserts a list all or nothing; **`Measure::getNoteOn`/`getNoteOff` raise `std::out_of_range` for an index at or past the staff's count of notes on (or rests)**, where they returned another note — a live reference to the wrong note is worse than a copy of it. The four getters share one implementation (the non-const ones call the const ones through `std::as_const` and a `const_cast` of the result).
10. **`ScoreCollection::setDirectoriesPaths` has a strong guarantee**: every path is checked and listed, and every file loaded, before the directories and scores are replaced; a failure leaves the collection as it was. Directories are loaded in the given order, each one's files sorted by path (`std::filesystem::path` order: `B.XML` before `a.xml`). The extension is read with `path::extension().u8string()`, which cannot throw for a name outside the ANSI code page; loading such a file stays with phase 4c-1.
11. **`ScoreCollection` searches reject a pattern of fewer than 2 notes even when the collection is empty**, with their own method name, so an empty collection and a full one fail the same way. The check is one function, `maiacore::detail::requireTwoNotes` (`melodic-lines.h`), which `Score` makes once per pattern search (in `searchMelodicLines`) and `ScoreCollection` once per call before its scores are searched.
12. **Decomposition:** Task 0 records baselines; the suggested tasks 4 (C++ search) and 5 (Score DataFrames) are one task, Task 4, because changing `Score::MelodyPatternRow` breaks `py_score.cpp`, and every commit must build the module; Task 4 adapts `score_collection.cpp` to the struct in a few lines that Task 6 replaces. The cache is removed in Task 5 with `findAnyMelodyPattern`, its only user, instead of Task 4. The fixtures go with the first task whose tests load them (Tasks 3, 4, 5). The final task (7) holds the README, the notebooks, the CHANGELOG and the verification.

## File map

| File | Responsibility | Tasks |
|---|---|---|
| `maiacore/include/maiacore/measure.h`, `maiacore/src/maiacore/measure.cpp` | `addNote` (list order, bounds), `removeNote` (one note, bounds), `getNoteOn`/`getNoteOff` bounds | 1, 2 |
| `maiacore/src/maiacore/python_wrapper/py_measure.cpp` | numpydoc of `addNote`/`removeNote`; live `getNote`/`getNoteOn`/`getNoteOff` | 1, 2 |
| `maiacore/src/maiacore/python_wrapper/py_part.cpp` | `setTransposingInterval` numpydoc no longer says `getNote()` copies | 2 |
| `maiacore/src/maiacore/melodic-lines.h`, `melodic-lines.cpp` (new) | `maiacore::detail::MelodicEvent`, `MelodicLine`, `melodicLines()`; `requireTwoNotes()`, the pattern-length check of every melody search | 3, 4 |
| `maiacore/include/maiacore/score.h`, `maiacore/src/maiacore/score.cpp` | `MelodyPatternRow` struct, thresholds, search over lines, transposition, worker pool; `FoundMelodyPattern`, `findAnyMelodyPattern`; cache and `removeDuplicatePatterns` removed | 4, 5 |
| `maiacore/src/maiacore/python_wrapper/py_melody_dataframe.h` (new) | `maiacore_python::MelodyDataFrame`, the typed DataFrame builder | 4 |
| `maiacore/src/maiacore/python_wrapper/py_score.cpp` | `findMelodyPatternDataFrame` ×2, `findAnyMelodyPatternDataFrame` | 4, 5 |
| `maiacore/include/maiacore/score_collection.h`, `score_collection.cpp`, `python_wrapper/py_score_collection.cpp` | ScoreCollection §5 and its searches | 4 (adaptation), 6 |
| `test/xml_examples/unit_test/melody_*.musicxml` (7 new), `test/musicxml/ledger.json` (7 lines) | fixtures | 3, 4, 5 |
| `test/xml_examples/unit_test/melody_patterns_quarter_tone_apart.xml` | its comment (the last window now starts on the quarter tone) | 5 |
| `test/test_musicxml_transpose.py` | `CORPUS_WITH_TRANSPOSE` lists the new transposing fixture | 3 |
| `tests-cpp/CMakeLists.txt`, `tests-cpp/src/melodic-lines-test.cpp` (new) | `MelodicLines` | 3 |
| `tests-cpp/src/measure-test.cpp` | `MeasureNoteRemoval`, `MeasureNoteAddition`, `MeasureNoteRetrieval` | 1, 2 |
| `tests-cpp/src/score-test.cpp` | `ScoreMelodyPatternSearch` | 4, 5 |
| `tests-cpp/src/score-collection-test.cpp` | exact tests, §5 rules | 6 |
| `test/test_measure_comprehensive.py`, `test/test_part_comprehensive.py`, `test/test_score_comprehensive.py`, `test/test_score_collection.py` (new) | Python tests | 1, 2, 4, 5, 6 |
| `README.md`, `python-tutorial/03_advanced/01_pattern_finding.ipynb`, `python-tutorial/find_pattern.ipynb`, `CHANGELOG.md` | docs | 7 |

---

### Task 0: Baseline (no commit)

**Files:** none changed.

**Interfaces:** produces the venv `C:\Users\nyck\AppData\Local\Temp\maialib-5-venv`, the baseline counts and `C:\Users\nyck\AppData\Local\Temp\maialib-5-fuzz-baseline.json`.

- [ ] **Step 1: Brand-new venv.** PowerShell: `py -3.12 -m venv C:\Users\nyck\AppData\Local\Temp\maialib-5-venv; & 'C:\Users\nyck\AppData\Local\Temp\maialib-5-venv\Scripts\python.exe' -m pip install -r requirements-dev.txt; $LASTEXITCODE` → 0.
- [ ] **Step 2: Build and install.** «build» `make "PYTHON=$py" dev; $LASTEXITCODE` → 0; then `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`.
- [ ] **Step 3: Baselines.** «build» `make "PYTHON=$py" cpp-tests; $LASTEXITCODE` → 0, `[  PASSED  ] 1156 tests.` (measured at cab545b). «build» `make "PYTHON=$py" py-tests; $LASTEXITCODE` → 0, `Ran 653 tests` / `OK (skipped=1)`. «build» `make "PYTHON=$py" validate; $LASTEXITCODE` → 0, `no new findings`; record the counts printed.
- [ ] **Step 4: Fuzz baseline.** «build» `make "PYTHON=$py" fuzz; $LASTEXITCODE` → 0; record the outcome counts, then `Copy-Item test\musicxml\fuzz-work\report-seed-1.json C:\Users\nyck\AppData\Local\Temp\maialib-5-fuzz-baseline.json`.
- [ ] **Step 5: Import check from outside the repository.** Bash: `(cd /c/Users/nyck/AppData/Local/Temp && /c/Users/nyck/AppData/Local/Temp/maialib-5-venv/Scripts/python.exe -c "import maialib; print(maialib.__version__)"); echo "exit $?"` → the version, exit 0. `git status --short` → ` M .gitignore`.

---

### Task 1: `Measure::removeNote` removes one note; `addNote` inserts in list order

**Files:**
- Modify: `maiacore/include/maiacore/measure.h` (Doxygen of the four `addNote` and of `removeNote`, ~154-192), `maiacore/src/maiacore/measure.cpp` (includes ~3-6; `addNote` ×4 and `removeNote`, ~116-160), `maiacore/src/maiacore/python_wrapper/py_measure.cpp` (`addNote` ×4 and `removeNote`, ~46-61)
- Test: `tests-cpp/src/measure-test.cpp` (the `MeasureNoteRemoval` tests ~671-707 replaced; new `MeasureNoteAddition` tests), `test/test_measure_comprehensive.py` (`test_remove_note` ~209-216 made exact; new class `MeasureEditTestCase`)

**Interfaces:** unchanged signatures: `void Measure::addNote(const Note& note, const int staveId = 0, int position = -1)`, `void Measure::addNote(const std::vector<Note>& noteVec, const int staveId = 0, int position = -1)`, `void Measure::addNote(const std::string& pitchClass, const int staveId = 0, int position = -1)`, `void Measure::addNote(const std::vector<std::string>& pitchClassVec, const int staveId = 0, int position = -1)`, `void Measure::removeNote(const int noteId, const int staveId = 0)`; Python `Measure.addNote(note|noteVec|pitchClass|pitchClassVec, staveId=0, position=-1)`, `Measure.removeNote(noteId, staveId=0)`. Produces the file-local `requireStaff(const char* method, const int staveId, const size_t numStaves)` in `measure.cpp`, which Task 2 uses.

- [ ] **Step 1: Write the failing C++ tests** — in `tests-cpp/src/measure-test.cpp`, add `#include <stdexcept>` after `#include <map>`, and replace everything from `TEST(MeasureNoteRemoval, RemoveSingleNote) {` (~671) up to, not including, `TEST(MeasureNoteRemoval, ClearEmptyMeasure) {` (~709) with:

```cpp
namespace {
// The written pitches of a staff of a measure, in order.
std::vector<std::string> pitchesOn(const Measure& measure, const int staff = 0) {
    std::vector<std::string> pitches;
    for (int n = 0; n < measure.getNumNotes(staff); n++) {
        pitches.push_back(measure.getNote(n, staff).getWrittenPitch());
    }
    return pitches;
}
}  // namespace

// removeNote() removes exactly the note at its index: the first, a middle one and the last.
TEST(MeasureNoteRemoval, RemoveSingleNote) {
    Measure measure;
    measure.addNote(std::vector<std::string>{"C4", "D4", "E4", "F4"}, 0);

    measure.removeNote(0, 0);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"D4", "E4", "F4"}));
    measure.removeNote(1, 0);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"D4", "F4"}));
    measure.removeNote(1, 0);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"D4"}));
}

TEST(MeasureNoteRemoval, RemoveAllNotes) {
    Measure measure;

    measure.addNote(Note("C4"), 0);
    measure.addNote(Note("E4"), 0);
    measure.addNote(Note("G4"), 0);

    measure.clear();
    EXPECT_EQ(measure.getNumNotes(), 0);
}

// Removing from one staff leaves the other untouched.
TEST(MeasureNoteRemoval, RemoveNotesFromSpecificStave) {
    Measure measure(2);
    measure.addNote(std::vector<std::string>{"C5", "D5"}, 0);
    measure.addNote(Note("C3"), 1);

    measure.removeNote(0, 0);

    EXPECT_EQ(pitchesOn(measure, 0), (std::vector<std::string>{"D5"}));
    EXPECT_EQ(pitchesOn(measure, 1), (std::vector<std::string>{"C3"}));
}

// An index or a staff outside the measure raises std::out_of_range and changes nothing.
TEST(MeasureNoteRemoval, AnIndexOrAStaffOutsideTheMeasureRaises) {
    Measure measure;
    measure.addNote(std::vector<std::string>{"C4", "D4"}, 0);

    EXPECT_THROW(measure.removeNote(2, 0), std::out_of_range);
    EXPECT_THROW(measure.removeNote(-1, 0), std::out_of_range);
    EXPECT_THROW(measure.removeNote(0, 1), std::out_of_range);
    EXPECT_THROW(measure.removeNote(0, -1), std::out_of_range);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"C4", "D4"}));
}

// A list of notes is inserted in list order, at the given position or at the end.
TEST(MeasureNoteAddition, AListIsInsertedInListOrder) {
    Measure measure;
    measure.addNote(std::vector<std::string>{"C4", "D4"}, 0);

    measure.addNote(std::vector<Note>{Note("A4"), Note("B4")}, 0, 1);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"C4", "A4", "B4", "D4"}));
    measure.addNote(std::vector<std::string>{"E5", "F5"}, 0, 0);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"E5", "F5", "C4", "A4", "B4", "D4"}));
    measure.addNote(std::vector<std::string>{"G5", "A5"}, 0);
    EXPECT_EQ(pitchesOn(measure),
              (std::vector<std::string>{"E5", "F5", "C4", "A4", "B4", "D4", "G5", "A5"}));
}

// A position past the end of the staff, or a staff outside the measure, raises
// std::out_of_range and adds nothing; the end itself is a valid position.
TEST(MeasureNoteAddition, APositionPastTheEndOrAStaffOutsideTheMeasureRaises) {
    Measure measure;
    measure.addNote(Note("C4"), 0);

    EXPECT_THROW(measure.addNote(Note("D4"), 0, 2), std::out_of_range);
    EXPECT_THROW(measure.addNote(std::vector<std::string>{"D4", "E4"}, 0, 2), std::out_of_range);
    EXPECT_THROW(measure.addNote(Note("D4"), -1), std::out_of_range);
    EXPECT_THROW(measure.addNote(Note("D4"), 1), std::out_of_range);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"C4"}));

    measure.addNote(Note("D4"), 0, 1);
    EXPECT_EQ(pitchesOn(measure), (std::vector<std::string>{"C4", "D4"}));
}
```

- [ ] **Step 2: Write the failing Python tests** — in `test/test_measure_comprehensive.py`, replace `test_remove_note` (~209-216):

```python
    def test_remove_note(self):
        """Test removing a note"""
        self.measure.addNote("F4")
        self.measure.addNote("G4")
        initial_count = self.measure.getNumNotes()
        self.measure.removeNote(0)
        # removeNote should work without crashing
        self.assertIsInstance(self.measure.getNumNotes(), int)
```

  with

```python
    def test_remove_note(self):
        """removeNote(0) removes the first note and only it"""
        self.measure.addNote("F4")
        self.measure.addNote("G4")
        self.measure.removeNote(0)
        self.assertEqual(self.measure.getNumNotes(), 1)
        self.assertEqual(self.measure.getNote(0).getWrittenPitch(), "G4")
```

  and insert before `if __name__ == "__main__":` (two blank lines before and after the class):

```python
class MeasureEditTestCase(unittest.TestCase):
    """removeNote removes one note; addNote inserts a list in order; both check their indices."""

    def pitches(self, measure, staff=0):
        return [
            measure.getNote(i, staff).getWrittenPitch() for i in range(measure.getNumNotes(staff))
        ]

    def test_remove_note_removes_exactly_one_note(self):
        measure = ml.Measure()
        measure.addNote(["C4", "D4", "E4", "F4"])
        measure.removeNote(2)
        self.assertEqual(self.pitches(measure), ["C4", "D4", "F4"])
        measure.removeNote(0)
        self.assertEqual(self.pitches(measure), ["D4", "F4"])

    def test_remove_note_outside_the_measure_raises_index_error(self):
        measure = ml.Measure()
        measure.addNote(["C4", "D4"])
        for note_id, stave_id in ((2, 0), (-1, 0), (0, 1), (0, -1)):
            with self.subTest(noteId=note_id, staveId=stave_id), self.assertRaises(IndexError):
                measure.removeNote(note_id, stave_id)
        self.assertEqual(self.pitches(measure), ["C4", "D4"])

    def test_add_note_inserts_a_list_in_list_order(self):
        measure = ml.Measure()
        measure.addNote(["C4", "D4"])
        measure.addNote([ml.Note("A4"), ml.Note("B4")], 0, 1)
        measure.addNote(["E5", "F5"], 0, 0)
        self.assertEqual(self.pitches(measure), ["E5", "F5", "C4", "A4", "B4", "D4"])

    def test_add_note_past_the_end_or_on_a_missing_staff_raises_index_error(self):
        measure = ml.Measure()
        measure.addNote("C4")
        for args in ((ml.Note("D4"), 0, 2), (["D4", "E4"], 0, 2), (ml.Note("D4"), -1), ("D4", 1)):
            with self.subTest(args=args), self.assertRaises(IndexError):
                measure.addNote(*args)
        self.assertEqual(self.pitches(measure), ["C4"])
```

- [ ] **Step 3: Run them and see them fail.** C++ subset `MeasureNote*` → `RemoveSingleNote` fails (`["C4","D4","E4","F4"]` after `removeNote(0, 0)`: the range `[0, 0)` erases nothing), `AnIndexOrAStaffOutsideTheMeasureRaises` fails ("Expected: measure.removeNote(2, 0) throws an exception of type std::out_of_range. Actual: it throws nothing" — or crashes on the erase past the end: record which), `AListIsInsertedInListOrder` fails (`B4, A4` reversed), `APositionPastTheEndOrAStaffOutsideTheMeasureRaises` fails or crashes. Python is not rebuilt yet: «pytest» `test_measure_comprehensive.MeasureNotesTestCase.test_remove_note test_measure_comprehensive.MeasureEditTestCase` → `test_remove_note` fails (`2 != 1`), the edit tests fail (`removeNote(2)` leaves `[F4]`; `MemoryError: bad allocation` for the out-of-range indices; reversed list).

- [ ] **Step 4: The implementation** — in `maiacore/src/maiacore/measure.cpp`:
  - after `#include <map>` insert `#include <stdexcept>`, and after `#include <utility>` insert `#include <vector>`;
  - replace everything from `void Measure::addNote(const Note& note, const int staveId, int position) {` (~116) through the closing `}` of `void Measure::removeNote(const int noteId, const int staveId) {` (~160) with:

```cpp
namespace {
// Rejects a staff outside the measure, naming the method and the staff.
void requireStaff(const char* method, const int staveId, const size_t numStaves) {
    if (staveId < 0 || staveId >= static_cast<int>(numStaves)) {
        throw std::out_of_range(std::string(method) + ": staff " + std::to_string(staveId) +
                                " is outside the measure's " + std::to_string(numStaves) +
                                " staves");
    }
}
}  // namespace

void Measure::addNote(const Note& note, const int staveId, int position) {
    addNote(std::vector<Note>{note}, staveId, position);
}

void Measure::addNote(const std::vector<Note>& noteVec, const int staveId, int position) {
    PROFILE_FUNCTION();
    requireStaff("Measure::addNote", staveId, _note.size());
    auto& stave = _note[staveId];

    if (position < 0) {
        position = static_cast<int>(stave.size());  // append to the end of the staff
    }
    if (position > static_cast<int>(stave.size())) {
        throw std::out_of_range("Measure::addNote: position " + std::to_string(position) +
                                " is past the end of staff " + std::to_string(staveId) +
                                ", which has " + std::to_string(stave.size()) + " notes");
    }

    stave.insert(stave.begin() + position, noteVec.begin(), noteVec.end());
}

void Measure::addNote(const std::string& pitch, const int staveId, int position) {
    addNote(std::vector<Note>{Note(pitch)}, staveId, position);
}

void Measure::addNote(const std::vector<std::string>& pitchClassVec, const int staveId,
                      int position) {
    std::vector<Note> notes;
    notes.reserve(pitchClassVec.size());
    for (const auto& pitch : pitchClassVec) {
        notes.emplace_back(pitch);
    }
    addNote(notes, staveId, position);
}

void Measure::removeNote(const int noteId, const int staveId) {
    requireStaff("Measure::removeNote", staveId, _note.size());
    auto& stave = _note[staveId];
    if (noteId < 0 || noteId >= static_cast<int>(stave.size())) {
        throw std::out_of_range("Measure::removeNote: note " + std::to_string(noteId) +
                                " is outside staff " + std::to_string(staveId) + ", which has " +
                                std::to_string(stave.size()) + " notes");
    }
    stave.erase(stave.begin() + noteId);
}
```

- [ ] **Step 5: The Doxygen** — in `maiacore/include/maiacore/measure.h`, replace the four `addNote` comments and the `removeNote` comment (~154-191) so that the declarations read:

```cpp
    /**
     * @brief Inserts a copy of a note into a staff of the measure.
     * @param note Note to add.
     * @param staveId Staff index (default: 0).
     * @param position Index the note takes on the staff, from 0 to getNumNotes(staveId); a
     *        negative value (the default, -1) appends it.
     * @throws std::out_of_range If staveId is not a staff of the measure, or position is past
     *         the end of the staff; the measure is unchanged.
     */
    void addNote(const Note& note, const int staveId = 0, int position = -1);

    /**
     * @brief Inserts copies of notes into a staff of the measure, in list order: the first takes
     *        index position, the next position + 1, and so on.
     * @param noteVec Notes to add.
     * @param staveId Staff index (default: 0).
     * @param position Index the first note takes, from 0 to getNumNotes(staveId); a negative
     *        value (the default, -1) appends them.
     * @throws std::out_of_range If staveId is not a staff of the measure, or position is past
     *         the end of the staff; the measure is unchanged.
     */
    void addNote(const std::vector<Note>& noteVec, const int staveId = 0, int position = -1);

    /**
     * @brief Inserts a new note of the given pitch, as addNote(Note(pitchClass), staveId,
     *        position) does.
     * @param pitchClass Pitch string (e.g., "C4").
     * @param staveId Staff index (default: 0).
     * @param position Index the note takes on the staff; a negative value appends it.
     * @throws std::out_of_range As the Note overload throws.
     */
    void addNote(const std::string& pitchClass, const int staveId = 0, int position = -1);

    /**
     * @brief Inserts new notes of the given pitches in list order, as the std::vector<Note>
     *        overload does; nothing is inserted when a pitch is not valid.
     * @param pitchClassVec Pitch strings.
     * @param staveId Staff index (default: 0).
     * @param position Index the first note takes; a negative value appends them.
     * @throws std::out_of_range As the std::vector<Note> overload throws.
     */
    void addNote(const std::vector<std::string>& pitchClassVec, const int staveId = 0,
                 int position = -1);

    /**
     * @brief Removes the note at an index of a staff: exactly that one note.
     * @param noteId Index of the note on the staff, from 0 to getNumNotes(staveId) - 1.
     * @param staveId Staff index (default: 0).
     * @throws std::out_of_range If staveId or noteId is outside the measure; the measure is
     *         unchanged.
     */
    void removeNote(const int noteId, const int staveId = 0);
```

- [ ] **Step 6: The numpydoc** — in `maiacore/src/maiacore/python_wrapper/py_measure.cpp`, replace the four `cls.def("addNote", ...)` and the `cls.def("removeNote", ...)` (~46-61, from `    cls.def("addNote", py::overload_cast<const Note&, const int, int>(&Measure::addNote),` through `    cls.def("removeNote", &Measure::removeNote, py::arg("noteId"), py::arg("staveId") = 0);`) with:

```cpp
    cls.def("addNote", py::overload_cast<const Note&, const int, int>(&Measure::addNote),
            py::arg("note"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert a copy of a note into a staff of the measure.

        Parameters
        ----------
        note : Note
            The note.
        staveId : int, optional
            Staff index (default: 0).
        position : int, optional
            Index the note takes on the staff, from 0 to ``getNumNotes(staveId)``; -1 (the
            default), or any negative value, appends it.

        Raises
        ------
        IndexError
            If ``staveId`` is not a staff of the measure, or ``position`` is past the end of the
            staff; the measure is unchanged.
    )pbdoc");
    cls.def("addNote",
            py::overload_cast<const std::vector<Note>&, const int, int>(&Measure::addNote),
            py::arg("noteVec"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert copies of notes into a staff of the measure, in list order.

        Parameters
        ----------
        noteVec : list of Note
            The notes; the first takes index ``position``, the next ``position + 1``, and so on.
        staveId : int, optional
            Staff index (default: 0).
        position : int, optional
            Index the first note takes, from 0 to ``getNumNotes(staveId)``; -1 (the default), or
            any negative value, appends them.

        Raises
        ------
        IndexError
            If ``staveId`` is not a staff of the measure, or ``position`` is past the end of the
            staff; the measure is unchanged.

        Examples
        --------
        >>> measure = ml.Measure()
        >>> measure.addNote("C4")
        >>> measure.addNote([ml.Note("A4"), ml.Note("B4")], 0, 0)
        >>> [measure.getNote(i).getPitch() for i in range(measure.getNumNotes())]
        ['A4', 'B4', 'C4']
    )pbdoc");
    cls.def("addNote", py::overload_cast<const std::string&, const int, int>(&Measure::addNote),
            py::arg("pitchClass"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert a new note of the given pitch into a staff of the measure, as
        ``addNote(Note(pitchClass), staveId, position)`` does.
    )pbdoc");
    cls.def("addNote",
            py::overload_cast<const std::vector<std::string>&, const int, int>(&Measure::addNote),
            py::arg("pitchClassVec"), py::arg("staveId") = 0, py::arg("position") = -1,
            R"pbdoc(
        Insert new notes of the given pitches into a staff of the measure, in list order, as
        ``addNote([Note(p) for p in pitchClassVec], staveId, position)`` does.
    )pbdoc");

    cls.def("removeNote", &Measure::removeNote, py::arg("noteId"), py::arg("staveId") = 0,
            R"pbdoc(
        Remove the note at an index of a staff.

        Parameters
        ----------
        noteId : int
            Index of the note on the staff, from 0 to ``getNumNotes(staveId) - 1``.
        staveId : int, optional
            Staff index (default: 0).

        Raises
        ------
        IndexError
            If ``staveId`` or ``noteId`` is outside the measure; the measure is unchanged.

        Examples
        --------
        >>> measure = ml.Measure()
        >>> measure.addNote(["C4", "D4", "E4"])
        >>> measure.removeNote(1)
        >>> [measure.getNote(i).getPitch() for i in range(measure.getNumNotes())]
        ['C4', 'E4']
    )pbdoc");
```

- [ ] **Step 7: Format, build, pass.** clang-format `measure.h`, `measure.cpp`, `py_measure.cpp`, `measure-test.cpp`. C++ subset `Measure*` → all pass. «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_measure_comprehensive` → OK.

- [ ] **Step 8: Mutations.** (a) In `removeNote` replace `stave.erase(stave.begin() + noteId);` with `stave.erase(stave.begin(), stave.begin() + noteId);` → `RemoveSingleNote`, `RemoveNotesFromSpecificStave` (C++), `test_remove_note` and `test_remove_note_removes_exactly_one_note` (Python) fail. (b) In `removeNote`'s note check replace `throw std::out_of_range(` with `throw std::runtime_error(` → `AnIndexOrAStaffOutsideTheMeasureRaises` and `test_remove_note_outside_the_measure_raises_index_error` fail; the same in `requireStaff` → the staff cases of both and `APositionPastTheEndOrAStaffOutsideTheMeasureRaises` / `test_add_note_past_the_end_or_on_a_missing_staff_raises_index_error` fail; the same in the position check → the position cases fail. (c) In the `std::vector<Note>` overload replace `stave.insert(stave.begin() + position, noteVec.begin(), noteVec.end());` with `for (const Note& note : noteVec) { stave.insert(stave.begin() + position, note); }` → `AListIsInsertedInListOrder` and `test_add_note_inserts_a_list_in_list_order` fail (reversed). Revert each; rerun green.

- [ ] **Step 9: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (1156 + 3); «build» `make "PYTHON=$py" py-tests` → OK (653 + 4); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 10: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/include/maiacore/measure.h maiacore/src/maiacore/measure.cpp maiacore/src/maiacore/python_wrapper/py_measure.cpp tests-cpp/src/measure-test.cpp test/test_measure_comprehensive.py`, message:

```
fix: Measure::removeNote removes one note; addNote inserts in list order

removeNote(noteId, staff) erased the range [0, noteId) and read outside
the measure for a bad index; it now removes exactly that note and raises
std::out_of_range (Python IndexError) for an index or a staff outside the
measure. addNote with a list inserted the items in reverse order at a
fixed position, and a position past the end was undefined behaviour: the
list is inserted in order, all or nothing, and a position past the end or
a staff outside the measure raises std::out_of_range.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 2: Live note references from `Measure.getNote*`

**Files:**
- Modify: `maiacore/src/maiacore/measure.cpp` (`getNoteOn`/`getNoteOff` ×2, ~292-432), `maiacore/include/maiacore/measure.h` (their Doxygen, ~374-404), `maiacore/src/maiacore/python_wrapper/py_measure.cpp` (`getNote`, `getNoteOn`, `getNoteOff`, ~108-143), `maiacore/src/maiacore/python_wrapper/py_part.cpp` (`setTransposingInterval` numpydoc, ~73-76)
- Test: `tests-cpp/src/measure-test.cpp` (after Task 1's `APositionPastTheEndOrAStaffOutsideTheMeasureRaises`), `test/test_measure_comprehensive.py` (new class `MeasureLiveNoteTestCase`), `test/test_part_comprehensive.py` (class docstring ~331-332)

**Interfaces:** consumes `requireStaff` (Task 1). Python `Measure.getNote(noteId, staveId=0)`, `Measure.getNoteOn(noteOnId, staveId=0)`, `Measure.getNoteOff(noteOffId, staveId=0)` return `Note` references with `reference_internal` (they keep the measure's Python object alive). C++ signatures unchanged; `getNoteOn`/`getNoteOff` throw `std::out_of_range` for an index at or past the staff's count of notes on (rests).

- [ ] **Step 1: Write the failing C++ test** — in `tests-cpp/src/measure-test.cpp`, after the closing `}` of `TEST(MeasureNoteAddition, APositionPastTheEndOrAStaffOutsideTheMeasureRaises)`, insert:

```cpp
// getNoteOn() and getNoteOff() count only the notes, or only the rests, of the staff: an index
// past them raises std::out_of_range instead of answering another note.
TEST(MeasureNoteRetrieval, NoteOnAndNoteOffIndicesCountOnlyTheirKind) {
    Measure measure;
    measure.addNote(std::vector<std::string>{"C4", "rest", "E4"}, 0);
    const Measure& constMeasure = measure;

    EXPECT_EQ(measure.getNoteOn(1, 0).getWrittenPitch(), "E4");
    EXPECT_EQ(measure.getNoteOff(0, 0).getWrittenPitch(), "rest");
    EXPECT_THROW(measure.getNoteOn(2, 0), std::out_of_range);
    EXPECT_THROW(measure.getNoteOff(1, 0), std::out_of_range);
    EXPECT_THROW(constMeasure.getNoteOn(2, 0), std::out_of_range);
    EXPECT_THROW(constMeasure.getNoteOff(1, 0), std::out_of_range);
}
```

- [ ] **Step 2: Write the failing Python tests** — in `test/test_measure_comprehensive.py`, insert before `class MeasureEditTestCase(unittest.TestCase):` (two blank lines after):

```python
class MeasureLiveNoteTestCase(unittest.TestCase):
    """getNote, getNoteOn and getNoteOff return live references: an edit reaches the score."""

    def flute(self):
        score = ml.Score(["Flute"], 1)
        score.getPart(0).getMeasure(0).addNote(["C4", "rest", "E4"])
        return score

    def test_an_edit_through_get_note_reaches_the_score(self):
        score = self.flute()
        score.getPart(0).getMeasure(0).getNote(0).setPitch("D4")
        self.assertEqual(score.getPart(0).getMeasure(0).getNote(0).getWrittenPitch(), "D4")

    def test_an_edit_through_get_note_on_reaches_the_score(self):
        score = self.flute()
        score.getPart(0).getMeasure(0).getNoteOn(1).setPitch("F4")
        self.assertEqual(score.getPart(0).getMeasure(0).getNote(2).getWrittenPitch(), "F4")

    def test_an_edit_through_get_note_off_reaches_the_score(self):
        score = self.flute()
        score.getPart(0).getMeasure(0).getNoteOff(0).setDuration(2.0)
        self.assertEqual(score.getPart(0).getMeasure(0).getNote(1).getQuarterDuration(), 2.0)

    def test_the_docstrings_say_how_long_the_reference_is_valid(self):
        for method in (ml.Measure.getNote, ml.Measure.getNoteOn, ml.Measure.getNoteOff):
            with self.subTest(method=method.__name__):
                doc = " ".join(method.__doc__.split())
                self.assertIn("valid until the measure gains or loses notes", doc)

    def test_an_index_past_the_notes_or_the_rests_raises(self):
        measure = self.flute().getPart(0).getMeasure(0)
        with self.assertRaises(IndexError):
            measure.getNoteOn(2)
        with self.assertRaises(IndexError):
            measure.getNoteOff(1)
```

- [ ] **Step 3: Run them and see them fail.** C++ subset `MeasureNoteRetrieval.*` → `NoteOnAndNoteOffIndicesCountOnlyTheirKind` fails: `measure.getNoteOn(2, 0)` throws nothing (it returns `E4`, the last note). «pytest» `test_measure_comprehensive.MeasureLiveNoteTestCase` → the three edit tests fail (`'C4' != 'D4'`, `'E4' != 'F4'`, `1.0 != 2.0`: edits reach a copy), the docstring test fails, `test_an_index_past_the_notes_or_the_rests_raises` fails (`IndexError not raised`).

- [ ] **Step 4: The C++ bounds** — in `maiacore/src/maiacore/measure.cpp`, replace everything from `const Note& Measure::getNoteOn(const int noteOnId, const int staveId) const {` (~292) through the closing `}` of `Note& Measure::getNoteOff(const int noteOffId, const int staveId) {` (just before `int Measure::getNumNotesOn() const {`, ~434) with:

```cpp
const Note& Measure::getNoteOn(const int noteOnId, const int staveId) const {
    requireStaff("Measure::getNoteOn", staveId, _note.size());
    int count = 0;
    for (const Note& note : _note[staveId]) {
        if (note.isNoteOn() && count++ == noteOnId) {
            return note;
        }
    }
    throw std::out_of_range("Measure::getNoteOn: note on " + std::to_string(noteOnId) +
                            " is outside staff " + std::to_string(staveId) + ", which has " +
                            std::to_string(count) + " notes on");
}

Note& Measure::getNoteOn(const int noteOnId, const int staveId) {
    return const_cast<Note&>(std::as_const(*this).getNoteOn(noteOnId, staveId));
}

const Note& Measure::getNoteOff(const int noteOffId, const int staveId) const {
    requireStaff("Measure::getNoteOff", staveId, _note.size());
    int count = 0;
    for (const Note& note : _note[staveId]) {
        if (note.isNoteOff() && count++ == noteOffId) {
            return note;
        }
    }
    throw std::out_of_range("Measure::getNoteOff: rest " + std::to_string(noteOffId) +
                            " is outside staff " + std::to_string(staveId) + ", which has " +
                            std::to_string(count) + " rests");
}

Note& Measure::getNoteOff(const int noteOffId, const int staveId) {
    return const_cast<Note&>(std::as_const(*this).getNoteOff(noteOffId, staveId));
}
```

  and in `maiacore/include/maiacore/measure.h` give the four getters (~374-404) these comments:

```cpp
    /**
     * @brief Returns a const reference to a sounding note (note on) by its index among the
     *        staff's sounding notes.
     * @param noteOnId Index among the staff's notes on, from 0 to getNumNotesOn(staveId) - 1.
     * @param staveId Staff index (default: 0).
     * @return Const reference to Note.
     * @throws std::out_of_range If staveId is not a staff of the measure, or noteOnId is not an
     *         index among its notes on.
     */
    const Note& getNoteOn(const int noteOnId, const int staveId = 0) const;

    /**
     * @brief Returns a reference to a sounding note (note on) by its index among the staff's
     *        sounding notes; valid until the staff gains or loses notes.
     * @param noteOnId Index among the staff's notes on, from 0 to getNumNotesOn(staveId) - 1.
     * @param staveId Staff index (default: 0).
     * @return Reference to Note.
     * @throws std::out_of_range As the const overload throws.
     */
    Note& getNoteOn(const int noteOnId, const int staveId = 0);

    /**
     * @brief Returns a const reference to a rest (note off) by its index among the staff's rests.
     * @param noteOffId Index among the staff's rests, from 0 to getNumNotesOff(staveId) - 1.
     * @param staveId Staff index (default: 0).
     * @return Const reference to Note.
     * @throws std::out_of_range If staveId is not a staff of the measure, or noteOffId is not an
     *         index among its rests.
     */
    const Note& getNoteOff(const int noteOffId, const int staveId = 0) const;

    /**
     * @brief Returns a reference to a rest (note off) by its index among the staff's rests;
     *        valid until the staff gains or loses notes.
     * @param noteOffId Index among the staff's rests, from 0 to getNumNotesOff(staveId) - 1.
     * @param staveId Staff index (default: 0).
     * @return Reference to Note.
     * @throws std::out_of_range As the const overload throws.
     */
    Note& getNoteOff(const int noteOffId, const int staveId = 0);
```

- [ ] **Step 5: The bindings** — in `maiacore/src/maiacore/python_wrapper/py_measure.cpp`, replace everything from `    cls.def("getNote", py::overload_cast<const int, const int>(&Measure::getNote),` (~108) through the const `getNoteOff` registration that ends `            py::return_value_policy::reference_internal);` (~143) — the non-const `getNote` with its docstring and the two unreachable const registrations of each getter (pybind11 tries the non-const one first, and a Python object is never const) — with:

```cpp
    cls.def("getNote", py::overload_cast<const int, const int>(&Measure::getNote),
            py::arg("noteId"), py::arg("staveId") = 0, py::return_value_policy::reference_internal,
            R"pbdoc(
        Get the note at a given index on a given stave, as a live reference.

        An edit made through the returned note reaches the measure and its score. The reference
        is valid until the measure gains or loses notes (``addNote``, ``removeNote``, ``clear``,
        or an edit of its part or score that rebuilds the measure); fetch the note again after
        that. ``Chord.getNote`` and ``Part.getMeasures`` return copies.

        Parameters
        ----------
        noteId : int
            Index of the note within the stave, in ``0 .. getNumNotes(staveId) - 1``.
        staveId : int, optional
            Stave index (default: 0).

        Returns
        -------
        Note
            The note at ``noteId`` on ``staveId``.

        Raises
        ------
        IndexError
            If ``staveId`` or ``noteId`` is negative or out of range (e.g. on an empty stave).

        Examples
        --------
        >>> score = ml.Score(["Flute"], 1)
        >>> score.getPart(0).getMeasure(0).addNote("C4")
        >>> score.getPart(0).getMeasure(0).getNote(0).setPitch("D4")
        >>> score.getPart(0).getMeasure(0).getNote(0).getPitch()
        'D4'
    )pbdoc");

    cls.def("getNoteOn", py::overload_cast<const int, const int>(&Measure::getNoteOn),
            py::arg("noteOnId"), py::arg("staveId") = 0,
            py::return_value_policy::reference_internal,
            R"pbdoc(
        Get the sounding note (not a rest) at a given index among the stave's sounding notes, as
        a live reference.

        An edit made through the returned note reaches the measure and its score. The reference
        is valid until the measure gains or loses notes; fetch the note again after that.

        Parameters
        ----------
        noteOnId : int
            Index among the sounding notes of the stave, in ``0 .. getNumNotesOn(staveId) - 1``.
        staveId : int, optional
            Stave index (default: 0).

        Raises
        ------
        IndexError
            If ``staveId`` is not a stave of the measure, or ``noteOnId`` is negative or not
            below the stave's number of sounding notes.
    )pbdoc");

    cls.def("getNoteOff", py::overload_cast<const int, const int>(&Measure::getNoteOff),
            py::arg("noteOffId"), py::arg("staveId") = 0,
            py::return_value_policy::reference_internal,
            R"pbdoc(
        Get the rest at a given index among the stave's rests, as a live reference.

        An edit made through the returned note reaches the measure and its score. The reference
        is valid until the measure gains or loses notes; fetch the note again after that.

        Parameters
        ----------
        noteOffId : int
            Index among the rests of the stave, in ``0 .. getNumNotesOff(staveId) - 1``.
        staveId : int, optional
            Stave index (default: 0).

        Raises
        ------
        IndexError
            If ``staveId`` is not a stave of the measure, or ``noteOffId`` is negative or not
            below the stave's number of rests.
    )pbdoc");
```

- [ ] **Step 6: The texts that said `getNote()` copies.** In `maiacore/src/maiacore/python_wrapper/py_part.cpp` replace

```
        This is the way to change a score's transpositions in place: ``Measure.getNote()``
        returns a copy of the note, so a change made on it does not reach the score
        (``Score.forEachNote`` edits in place too). Every note is checked before any changes, so
        the call changes all of them or none. An export -> import keeps the pair only when it is
```

  with

```
        It changes the notes in place, as an edit through ``Measure.getNote()`` (a live
        reference) or ``Score.forEachNote`` does one note at a time. Every note is checked
        before any changes, so the call changes all of them or none. An export -> import keeps
        the pair only when it is
```

  and in `test/test_part_comprehensive.py` replace the class docstring

```python
    """Part.setTransposingInterval stamps the pitched notes of a measure range and a staff in
    place, which an edit through Measure.getNote(), a copy, cannot do."""
```

  with

```python
    """Part.setTransposingInterval stamps the pitched notes of a measure range and a staff in
    place, all of them or none."""
```

  (The CHANGELOG line that says the same is rewritten in Task 7.)

- [ ] **Step 7: Format, build, pass.** clang-format `measure.h`, `measure.cpp`, `py_measure.cpp`, `py_part.cpp`, `measure-test.cpp`. C++ subset `Measure*` → pass. «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_measure_comprehensive test_part_comprehensive test_musicxml_transpose` → OK (`test_musicxml_transpose` and `maiapy/plots.py` read notes through `getNote` and only read them).

- [ ] **Step 8: Mutations.** (a) Remove `py::return_value_policy::reference_internal,` from the `getNote` registration → `test_an_edit_through_get_note_reaches_the_score` fails; the same for `getNoteOn` and `getNoteOff` → their tests fail. (b) In the `getNoteOff` docstring replace `valid until the measure gains or loses notes` with `valid while the measure keeps its notes` → `test_the_docstrings_say_how_long_the_reference_is_valid` fails (subTest `getNoteOff`). (c) In the const `getNoteOn` replace the `throw std::out_of_range(...)` statement after the loop with `return _note[staveId].back();` → `NoteOnAndNoteOffIndicesCountOnlyTheirKind` and `test_an_index_past_the_notes_or_the_rests_raises` fail; the same in `getNoteOff` → the `getNoteOff` assertions fail. Revert each; rerun green.

- [ ] **Step 9: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (Task 1's + 1); «build» `make "PYTHON=$py" py-tests` → OK (Task 1's + 5); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 10: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/include/maiacore/measure.h maiacore/src/maiacore/measure.cpp maiacore/src/maiacore/python_wrapper/py_measure.cpp maiacore/src/maiacore/python_wrapper/py_part.cpp tests-cpp/src/measure-test.cpp test/test_measure_comprehensive.py test/test_part_comprehensive.py`, message:

```
feat: Measure.getNote, getNoteOn and getNoteOff return live references

The non-const overloads were bound without a return policy, so Python got
copies and an edit never reached the score. They now return
reference_internal references, valid until the measure gains or loses
notes, as their numpydoc says. getNoteOn and getNoteOff raise
std::out_of_range (IndexError) for an index past the staff's notes on or
rests, where they returned another note.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 3: Melodic lines (`maiacore::detail::melodicLines`)

**Files:**
- Create: `maiacore/src/maiacore/melodic-lines.h`, `maiacore/src/maiacore/melodic-lines.cpp` (the root `CMakeLists.txt` globs `maiacore/src/maiacore/*.cpp`, and every build configures again, so nothing else lists them)
- Create (fixtures): `test/xml_examples/unit_test/melody_staves_and_voices.musicxml`, `melody_chord_top_note.musicxml`, `melody_tie_across_barline.musicxml`, `melody_transposing_instrument.musicxml`
- Modify: `tests-cpp/CMakeLists.txt` (source list), `test/musicxml/ledger.json` (4 added lines), `test/test_musicxml_transpose.py` (`CORPUS_WITH_TRANSPOSE`, ~27-49)
- Test: `tests-cpp/src/melodic-lines-test.cpp` (new)

**Interfaces:** produces, in `maiacore/src/maiacore/melodic-lines.h` (internal, not bound):
`struct maiacore::detail::MelodicEvent { Note note; int measureIdx = 0; };`
`struct maiacore::detail::MelodicLine { int partIdx = 0; int staff = 0; int voice = 0; std::vector<MelodicEvent> events; };`
`std::vector<maiacore::detail::MelodicLine> maiacore::detail::melodicLines(const std::vector<Part>& parts);`
Consumed by Tasks 4 and 5. Nothing else changes in this task, so the search still behaves as before.

- [ ] **Step 1: The fixtures.** Create the four files with exactly this content (each is valid MusicXML 4.0).

`test/xml_examples/unit_test/melody_staves_and_voices.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A piano part whose upper staff holds voices 1 and 2 and whose lower staff holds voice 5, as
     MuseScore numbers the voices of a second staff. Three melodic lines: staff 0 voice 1
     E5 F5 G5 A5 B5 C6 D6, staff 0 voice 2 C5 B4 A4, staff 1 voice 5 C3 D3 E3 F3 G3 A3 B3 C4. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Piano</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <staves>2</staves>
        <clef number="1"><sign>G</sign><line>2</line></clef>
        <clef number="2"><sign>F</sign><line>4</line></clef>
      </attributes>
      <note><pitch><step>E</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><staff>1</staff></note>
      <note><pitch><step>F</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><staff>1</staff></note>
      <note><pitch><step>G</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><staff>1</staff></note>
      <note><pitch><step>A</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>2</duration><voice>2</voice><type>half</type><staff>1</staff></note>
      <note><pitch><step>B</step><octave>4</octave></pitch><duration>2</duration><voice>2</voice><type>half</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>C</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
      <note><pitch><step>D</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
      <note><pitch><step>E</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
      <note><pitch><step>F</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
    </measure>
    <measure number="2">
      <note><pitch><step>B</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><staff>1</staff></note>
      <note><pitch><step>C</step><octave>6</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><staff>1</staff></note>
      <note><pitch><step>D</step><octave>6</octave></pitch><duration>2</duration><voice>1</voice><type>half</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>A</step><octave>4</octave></pitch><duration>4</duration><voice>2</voice><type>whole</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>G</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
      <note><pitch><step>A</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
      <note><pitch><step>B</step><octave>3</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>5</voice><type>quarter</type><staff>2</staff></note>
    </measure>
  </part>
</score-partwise>
```

`test/xml_examples/unit_test/melody_chord_top_note.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- One line of four events: the chord C4 E4 G4, written from the bottom up, stands for its
     highest note G4; the grace note A4 is no event; D5; the chord written from the top down,
     B4 G4, stands for B4; a rest. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Piano</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><chord/><pitch><step>E</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><chord/><pitch><step>G</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><grace/><pitch><step>A</step><octave>4</octave></pitch><voice>1</voice><type>eighth</type></note>
      <note><pitch><step>D</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>B</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><chord/><pitch><step>G</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><rest/><duration>1</duration><voice>1</voice><type>quarter</type></note>
    </measure>
  </part>
</score-partwise>
```

`test/xml_examples/unit_test/melody_tie_across_barline.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- One line: C4, D4, E4 tied across the barline (a half note in measure 1 and a quarter note
     in measure 2: one event of three quarters, in measure 1), F4, G4. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Flute</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>E</step><octave>4</octave></pitch><duration>2</duration><tie type="start"/><voice>1</voice><type>half</type><notations><tied type="start"/></notations></note>
    </measure>
    <measure number="2">
      <note><pitch><step>E</step><octave>4</octave></pitch><duration>1</duration><tie type="stop"/><voice>1</voice><type>quarter</type><notations><tied type="stop"/></notations></note>
      <note><pitch><step>F</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
    </measure>
  </part>
</score-partwise>
```

`test/xml_examples/unit_test/melody_transposing_instrument.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A clarinet in B-flat writes D4 E4 F#4 G4 in D major and sounds C4 D4 E4 F4; a violin plays
     C5 B4 A4 G4 in C major, the concert key. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Clarinet in Bb</part-name></score-part>
    <score-part id="P2"><part-name>Violin</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>2</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>E</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>F</step><alter>1</alter><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
    </measure>
  </part>
  <part id="P2">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>B</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>A</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
    </measure>
  </part>
</score-partwise>
```

  Check them: Bash `(cd /c/Users/nyck/Desktop/maialib && /c/Users/nyck/AppData/Local/Temp/maialib-5-venv/Scripts/python.exe test/musicxml/musicxml_check.py test/xml_examples/unit_test/melody_staves_and_voices.musicxml test/xml_examples/unit_test/melody_chord_top_note.musicxml test/xml_examples/unit_test/melody_tie_across_barline.musicxml test/xml_examples/unit_test/melody_transposing_instrument.musicxml); echo "exit $?"` → each `valid; errors: none; warnings: none`, exit 0.

- [ ] **Step 2: Write the failing tests** — create `tests-cpp/src/melodic-lines-test.cpp`:

```cpp
// The melodic lines the melody search reads (melodic-lines.h): one per part, staff and voice; a
// chord by its highest sounding note; tied notes as one event; rests as events; grace notes left
// out. The design is docs/superpowers/specs/2026-10-05-melody-search-design.md, section 3.

#include "melodic-lines.h"

#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/part.h"
#include "maiacore/score.h"

using maiacore::detail::MelodicEvent;
using maiacore::detail::MelodicLine;
using maiacore::detail::melodicLines;

namespace {
// The parts of a score, in score order.
std::vector<Part> partsOf(Score& score) {
    std::vector<Part> parts;
    for (int p = 0; p < score.getNumParts(); p++) {
        parts.push_back(score.getPart(p));
    }
    return parts;
}

// The lines of a score, built from a copy of its parts.
std::vector<MelodicLine> linesOf(Score& score) { return melodicLines(partsOf(score)); }

// The (part, staff, voice) of each line.
std::vector<std::tuple<int, int, int>> keysOf(const std::vector<MelodicLine>& lines) {
    std::vector<std::tuple<int, int, int>> keys;
    for (const MelodicLine& line : lines) {
        keys.emplace_back(line.partIdx, line.staff, line.voice);
    }
    return keys;
}

// The written pitch of each event of a line.
std::vector<std::string> pitchesOf(const MelodicLine& line) {
    std::vector<std::string> pitches;
    for (const MelodicEvent& event : line.events) {
        pitches.push_back(event.note.getWrittenPitch());
    }
    return pitches;
}
}  // namespace

// One line per voice of each staff, listed by staff, then voice; the lower staff's voice 5 is a
// line of its own, and the voices of one staff never mix.
TEST(MelodicLines, EachVoiceOfEachStaffIsALine) {
    Score score("./test/xml_examples/unit_test/melody_staves_and_voices.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(keysOf(lines),
              (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 0, 2}, {0, 1, 5}}));
    EXPECT_EQ(pitchesOf(lines[0]),
              (std::vector<std::string>{"E5", "F5", "G5", "A5", "B5", "C6", "D6"}));
    EXPECT_EQ(pitchesOf(lines[1]), (std::vector<std::string>{"C5", "B4", "A4"}));
    EXPECT_EQ(pitchesOf(lines[2]),
              (std::vector<std::string>{"C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4"}));
    EXPECT_EQ(lines[2].events[4].measureIdx, 1);
}

// Lines are listed by part first: a two-part score lists the first part's lines, then the
// second's.
TEST(MelodicLines, LinesAreListedByPart) {
    Score score("./test/xml_examples/unit_test/melody_transposing_instrument.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(keysOf(lines), (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {1, 0, 1}}));
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"D4", "E4", "F#4", "G4"}));
}

// A chord is one event, which stands for its highest sounding note, written first or last; a
// grace note is no event; a rest is one.
TEST(MelodicLines, AChordStandsForItsHighestNote) {
    Score score("./test/xml_examples/unit_test/melody_chord_top_note.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"G4", "D5", "B4", "rest"}));
}

// The highest note is the highest it sounds: a horn in F's written C5 sounds F4, below a written
// G4 of the same chord on an untransposed note.
TEST(MelodicLines, TheHighestNoteIsTheHighestSounding) {
    Score score({"Horn in F"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    measure.addNote(Note("G4"));
    Note horn("C5");
    horn.setTransposingInterval(-4, -7);
    horn.setIsInChord(true);
    measure.addNote(horn);

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"G4"}));
}

// A note tied to the previous event's note extends it: the event keeps its first note's written
// pitch and measure, and its duration is the sum.
TEST(MelodicLines, ATiedNoteExtendsTheEventItContinues) {
    Score score("./test/xml_examples/unit_test/melody_tie_across_barline.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"C4", "D4", "E4", "F4", "G4"}));
    std::vector<float> durations;
    std::vector<int> measures;
    for (const MelodicEvent& event : lines[0].events) {
        durations.push_back(event.note.getQuarterDuration());
        measures.push_back(event.measureIdx);
    }
    EXPECT_EQ(durations, (std::vector<float>{1.0f, 1.0f, 3.0f, 1.0f, 2.0f}));
    EXPECT_EQ(measures, (std::vector<int>{0, 0, 0, 1, 1}));
}

// Only a tie stop at the same sounding position continues an event. The durations add exactly
// in ticks at a multiple of 1024 divisions, whatever divisions each note counts in: a quarter at
// 256 divisions and an eighth at 2 make 1536 ticks at 1024.
TEST(MelodicLines, OnlyATieStopAtTheSamePitchContinuesAnEvent) {
    Score score({"Flute"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    Note first("C4");
    first.setDuration(256, 256);
    first.setTieStart();
    Note tied("C4");
    tied.setDuration(1, 2);
    tied.setTieStop();
    Note otherPitch("D4");
    otherPitch.setTieStop();
    measure.addNote({first, tied, otherPitch});

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"C4", "D4"}));
    EXPECT_EQ(lines[0].events[0].note.getDurationTicks(), 1536);
    EXPECT_EQ(lines[0].events[0].note.getDivisionsPerQuarterNote(), 1024);
    EXPECT_FLOAT_EQ(lines[0].events[0].note.getQuarterDuration(), 1.5f);
}

// A chord note that follows no note of its voice on its staff -- the rest of a chord written on
// another staff -- joins no event.
TEST(MelodicLines, AChordNoteWithoutItsFirstNoteOnTheStaffIsNoEvent) {
    Score score({"Piano"}, 1);
    score.getPart(0).getMeasure(0).setNumStaves(2);
    Measure& measure = score.getPart(0).getMeasure(0);
    measure.addNote(Note("C5"), 0);
    Note lower("C3");
    lower.setIsInChord(true);
    measure.addNote(lower, 1);

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(keysOf(lines), (std::vector<std::tuple<int, int, int>>{{0, 0, 1}}));
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"C5"}));
}

// A chord tied to a chord is one event when its highest note continues the highest note; a tie
// on an inner note alone extends nothing.
TEST(MelodicLines, ATiedChordIsOneEventWhenItsHighestNoteIsTied) {
    const auto chord = [](const char* lower, const char* upper, const bool stop) {
        Note bottom(lower);
        Note top(upper);
        top.setIsInChord(true);
        if (stop) {
            bottom.setTieStop();
            top.setTieStop();
        } else {
            bottom.setTieStart();
            top.setTieStart();
        }
        return std::vector<Note>{bottom, top};
    };
    Score tied({"Piano"}, 1);
    Measure& measure = tied.getPart(0).getMeasure(0);
    measure.addNote(chord("C4", "E4", false));
    measure.addNote(chord("C4", "E4", true));
    measure.addNote(Note("G4"));

    const std::vector<MelodicLine> lines = linesOf(tied);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"E4", "G4"}));
    EXPECT_FLOAT_EQ(lines[0].events[0].note.getQuarterDuration(), 2.0f);

    Score inner({"Piano"}, 1);
    Measure& innerMeasure = inner.getPart(0).getMeasure(0);
    std::vector<Note> first = chord("C4", "E4", false);
    first[1].removeTies();
    std::vector<Note> second = chord("C4", "E4", true);
    second[1].removeTies();
    innerMeasure.addNote(first);
    innerMeasure.addNote(second);
    EXPECT_EQ(pitchesOf(linesOf(inner)[0]), (std::vector<std::string>{"E4", "E4"}));
}

// A measure with more staves than the part's first -- the staff count can grow mid-part -- adds
// the lines of its new staff from that measure on.
TEST(MelodicLines, AStaffAddedMidPartIsALineFromThatMeasure) {
    Score score({"Piano"}, 2);
    score.getPart(0).getMeasure(0).addNote(Note("C5"));
    Measure& second = score.getPart(0).getMeasure(1);
    second.setNumStaves(2);
    second.addNote(Note("D5"), 0);
    second.addNote(Note("C3"), 1);

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(keysOf(lines), (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 1, 1}}));
    EXPECT_EQ(pitchesOf(lines[1]), (std::vector<std::string>{"C3"}));
    EXPECT_EQ(lines[1].events[0].measureIdx, 1);
}
```

  and in `tests-cpp/CMakeLists.txt` add `    src/melodic-lines-test.cpp` after `    src/pitch-views-test.cpp`.

- [ ] **Step 3: Run them and see them fail.** C++ subset `MelodicLines.*` → the build fails: `'melodic-lines.h' file not found`.

- [ ] **Step 4: The header** — create `maiacore/src/maiacore/melodic-lines.h`:

```cpp
#pragma once

#include <vector>

#include "maiacore/note.h"

class Part;

// The melodic lines that the melody search reads. Internal to maiacore: this header lives next to
// the sources, is not among the public headers, and nothing here is bound to Python.
namespace maiacore::detail {

/**
 * @brief One event of a melodic line: a note, a chord or a rest, with the notes tied to it.
 */
struct MelodicEvent {
    /**
     * @brief The note that stands for the event: the note itself; for a chord, its highest note
     *        by sounding exact position (Note::getQuarterToneSteps()), the first written of equal
     *        ones. A note tied to it adds its duration: the event keeps this note's written pitch
     *        and the measure where it starts, with the summed duration.
     */
    Note note;
    int measureIdx = 0;  ///< 0-based index of the measure where the event starts.
};

/**
 * @brief The events of one voice on one staff of one part, in measure order.
 */
struct MelodicLine {
    int partIdx = 0;  ///< 0-based index of the part.
    int staff = 0;    ///< 0-based staff.
    int voice = 0;    ///< The voice as written (Note::getVoice()).
    std::vector<MelodicEvent> events;
};

/**
 * @brief The melodic lines of a score's parts: one per part, staff and voice that occurs on that
 *        staff, listed by part, by staff, then by voice in ascending order.
 * @details A note without `<chord/>` starts an event, and the chord notes written right after it
 *          on its staff, in its voice, join that event; a chord note that follows no note of its
 *          voice on its staff (a chord spread over two staves) joins no event. A grace note is
 *          not an event. A rest is an event. A note whose tie list holds "stop" and that sounds
 *          the exact position of the previous event of its line, itself a note, extends that
 *          event: its duration is added, exactly, and it starts no event of its own.
 * @param parts The parts, in score order.
 * @return The lines; a line has at least one event.
 */
std::vector<MelodicLine> melodicLines(const std::vector<Part>& parts);

}  // namespace maiacore::detail
```

- [ ] **Step 5: The implementation** — create `maiacore/src/maiacore/melodic-lines.cpp`:

```cpp
#include "melodic-lines.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/measure.h"
#include "maiacore/part.h"

namespace maiacore::detail {

namespace {
// Whether the note's tie list holds a tie stop: the note continues a note tied to it.
bool stopsATie(const Note& note) {
    const std::vector<std::string> ties = note.getTie();
    return std::find(ties.begin(), ties.end(), "stop") != ties.end();
}

// Whether 'next' extends the event that 'previous' stands for: both are notes at the same
// sounding exact position, and 'next' stops a tie.
bool continuesTheTie(const Note& previous, const Note& next) {
    return previous.isNoteOn() && next.isNoteOn() && stopsATie(next) &&
           previous.getQuarterToneSteps() == next.getQuarterToneSteps();
}

// Adds the duration of 'tied' to 'note', in ticks at the least common multiple of their
// divisions per quarter note and 1024, so the sum is exact. Duration names a tick count by a
// rhythm figure whose length is the divisions times a power of two down to 1/256, then adds half
// of that length per dot: with a multiple of 1024 divisions that half is never 0, so the sum of
// two tuplet notes, which no longer carries their tuplet ratio, still gets a figure.
void addDuration(Note& note, const Note& tied) {
    const int noteDivisions = note.getDivisionsPerQuarterNote();
    const int tiedDivisions = tied.getDivisionsPerQuarterNote();
    const int divisions = std::lcm(std::lcm(noteDivisions, tiedDivisions), 1024);
    const int ticks = note.getDurationTicks() * (divisions / noteDivisions) +
                      tied.getDurationTicks() * (divisions / tiedDivisions);
    note.setDuration(ticks, divisions);
}

// Lets a chord note stand for its event when it sounds higher than the note that does.
void joinChord(MelodicEvent& event, const Note& chordNote) {
    if (!chordNote.isNoteOn()) {
        return;
    }
    if (event.note.isNoteOff() ||
        chordNote.getQuarterToneSteps() > event.note.getQuarterToneSteps()) {
        event.note = chordNote;
    }
}

// The events of a line with every tied continuation folded into the event it continues.
std::vector<MelodicEvent> mergeTies(std::vector<MelodicEvent> events) {
    std::vector<MelodicEvent> merged;
    merged.reserve(events.size());
    for (MelodicEvent& event : events) {
        if (!merged.empty() && continuesTheTie(merged.back().note, event.note)) {
            addDuration(merged.back().note, event.note);
            continue;
        }
        merged.push_back(std::move(event));
    }
    return merged;
}
}  // namespace

std::vector<MelodicLine> melodicLines(const std::vector<Part>& parts) {
    std::vector<MelodicLine> lines;
    for (int p = 0; p < static_cast<int>(parts.size()); p++) {
        const Part& part = parts[p];
        // The part's lines keyed by (staff, voice): std::map lists them by staff, then by voice.
        std::map<std::pair<int, int>, MelodicLine> partLines;
        for (int m = 0; m < part.getNumMeasures(); m++) {
            const Measure& measure = part.getMeasure(m);
            for (int s = 0; s < measure.getNumStaves(); s++) {
                // The line of the last note written on this staff that started an event: the
                // chord notes written after it join that event.
                MelodicLine* open = nullptr;
                for (int n = 0; n < measure.getNumNotes(s); n++) {
                    const Note& note = measure.getNote(n, s);
                    if (note.isGraceNote()) {
                        continue;
                    }
                    if (note.inChord()) {
                        if (open != nullptr && open->voice == note.getVoice()) {
                            joinChord(open->events.back(), note);
                        }
                        continue;
                    }
                    MelodicLine& line = partLines[{s, note.getVoice()}];
                    line.partIdx = p;
                    line.staff = s;
                    line.voice = note.getVoice();
                    line.events.push_back({note, m});
                    open = &line;
                }
            }
        }
        for (auto& entry : partLines) {
            entry.second.events = mergeTies(std::move(entry.second.events));
            lines.push_back(std::move(entry.second));
        }
    }
    return lines;
}

}  // namespace maiacore::detail
```

- [ ] **Step 6: Format, build, pass.** clang-format the three new C++ files. C++ subset `MelodicLines.*` → 9 tests pass.

- [ ] **Step 7: The transposing fixture joins the round-trip list.** `test_musicxml_transpose.TransposeRoundTripTestCase.test_the_list_holds_every_corpus_file_with_a_transpose` requires every corpus file with a `<transpose>` in `CORPUS_WITH_TRANSPOSE`: in `test/test_musicxml_transpose.py` insert `    "test/xml_examples/unit_test/melody_transposing_instrument.musicxml",` before `    "test/xml_examples/unit_test/test_compressed_file.mxl",`.

- [ ] **Step 8: Ledger.** «build» `make "PYTHON=$py" dev` → 0 (the module must include this task's source); «build» `make "PYTHON=$py" corpus-update-ledger` → 0; `git diff test/musicxml/ledger.json` shows exactly these four added lines and no other change (and `git diff --stat test/musicxml/ledger-external.json` is empty):

```
  "test/xml_examples/unit_test/melody_chord_top_note.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
  "test/xml_examples/unit_test/melody_staves_and_voices.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
  "test/xml_examples/unit_test/melody_tie_across_barline.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
  "test/xml_examples/unit_test/melody_transposing_instrument.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
```

  If any other line changed, restore the file and stop: report it.

- [ ] **Step 9: Mutations** (each in `melodic-lines.cpp`; rebuild with the C++ subset `MelodicLines.*`). (a) Replace `partLines[{s, note.getVoice()}]` with `partLines[{s, 1}]` → `EachVoiceOfEachStaffIsALine` fails. (b) Replace `line.partIdx = p;` with `line.partIdx = 0;` → `LinesAreListedByPart` fails. (c) Make `joinChord` return at its start (`return;` as its first statement) → `AChordStandsForItsHighestNote` and `ATiedChordIsOneEventWhenItsHighestNoteIsTied` fail. (d) Delete the `if (note.isGraceNote()) { continue; }` block → `AChordStandsForItsHighestNote` fails (`A4` becomes an event). (e) In `joinChord` compare written positions: replace `chordNote.getQuarterToneSteps() > event.note.getQuarterToneSteps()` with `Note(chordNote.getWrittenPitch()).getQuarterToneSteps() > Note(event.note.getWrittenPitch()).getQuarterToneSteps()` → `TheHighestNoteIsTheHighestSounding` fails (`C5`). (f) Make `mergeTies` return `events` unchanged (`return events;` as its first statement) → `ATiedNoteExtendsTheEventItContinues` and `ATiedChordIsOneEventWhenItsHighestNoteIsTied` fail. (g) In `continuesTheTie` delete `stopsATie(next) &&` → `ATiedChordIsOneEventWhenItsHighestNoteIsTied` fails (the inner-tie chords merge); delete `&& previous.getQuarterToneSteps() == next.getQuarterToneSteps()` → `OnlyATieStopAtTheSamePitchContinuesAnEvent` fails (`D4` merges). (h) Replace `std::lcm(std::lcm(noteDivisions, tiedDivisions), 1024)` with `std::lcm(noteDivisions, tiedDivisions)` → `OnlyATieStopAtTheSamePitchContinuesAnEvent` fails (`384` ticks at `256`). (i) Replace `if (note.inChord()) {` with `if (note.inChord() && open != nullptr) {` → `AChordNoteWithoutItsFirstNoteOnTheStaffIsNoEvent` fails (`C3` becomes a line). (j) Replace `for (int s = 0; s < measure.getNumStaves(); s++) {` with `for (int s = 0; s < part.getMeasure(0).getNumStaves(); s++) {` → `AStaffAddedMidPartIsALineFromThatMeasure` fails. Revert each; rerun green.

- [ ] **Step 10: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (Task 2's + 9); «build» `make "PYTHON=$py" py-tests` → OK (Task 2's count; the corpus test now covers the four fixtures, and the round trip the transposing one); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 11: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/src/maiacore/melodic-lines.h maiacore/src/maiacore/melodic-lines.cpp tests-cpp/CMakeLists.txt tests-cpp/src/melodic-lines-test.cpp test/xml_examples/unit_test/melody_staves_and_voices.musicxml test/xml_examples/unit_test/melody_chord_top_note.musicxml test/xml_examples/unit_test/melody_tie_across_barline.musicxml test/xml_examples/unit_test/melody_transposing_instrument.musicxml test/musicxml/ledger.json test/test_musicxml_transpose.py`, message:

```
feat: melodic lines: every voice of every staff, chords by their top note

maiacore::detail::melodicLines() lists one line per part, staff and voice,
in measure order. A chord is one event represented by its highest sounding
note; a note tied to the previous event at the same sounding pitch extends
it, with the durations summed exactly; a rest is an event and a grace note
is not. Four MusicXML fixtures cover staves and voices, chords, ties and a
transposing instrument.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 4: The search over melodic lines, its transposition and its DataFrames

**Files:**
- Create (fixtures): `test/xml_examples/unit_test/melody_last_window.musicxml`, `test/xml_examples/unit_test/melody_unnameable_transposition.musicxml`
- Create: `maiacore/src/maiacore/python_wrapper/py_melody_dataframe.h`
- Modify: `maiacore/src/maiacore/melodic-lines.h`, `maiacore/src/maiacore/melodic-lines.cpp` (`requireTwoNotes` added), `maiacore/include/maiacore/score.h` (includes ~3-6; the melody block from `MelodyPatternRow` through the list overload of `findMelodyPattern`, ~553-673), `maiacore/src/maiacore/score.cpp` (includes ~3-29; `rejectQuarterToneTransposition` ~57-87 deleted; both `findMelodyPattern` overloads ~2117-2352), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (includes; both `findMelodyPatternDataFrame` ~171-400), `maiacore/src/maiacore/score_collection.cpp` (the two `emplace_back` of rows, ~118-124 and ~162-178: an adaptation Task 6 replaces), `test/musicxml/ledger.json` (2 added lines)
- Test: `tests-cpp/src/score-test.cpp` (melody block ~1036-1095 and ~1205-1242), `test/test_score_comprehensive.py` (`ScoreMelodyPatternSearchTestCase` ~341-379 and ~450-465; new class `ScoreMelodyPatternDataFrameTestCase`)

**Interfaces:** consumes `maiacore::detail::melodicLines` (Task 3) and the file-local `concertKeys(const std::vector<Part>&)` of `score.cpp` (step 1b; reused, not duplicated). Produces:
`struct Score::MelodyPatternRow { std::string partName; int measure; int staff; int voice; std::string writtenKey; std::string concertKey; std::string transposeInterval; float transposeSemitones; std::vector<std::string> writtenPitches; std::vector<std::string> soundingPitches; std::vector<float> semitonesDiff; std::vector<float> rhythmDiff; float intervalSimilarity; float rhythmSimilarity; float totalSimilarity; bool operator==(const MelodyPatternRow&) const; };`, `typedef std::vector<MelodyPatternRow> Score::MelodyPatternTable;`,
`MelodyPatternTable Score::findMelodyPattern(const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold = 0.5, const float rhythmSimilarityThreshold = 0.5, <five callbacks as before>) const;`,
`std::vector<MelodyPatternTable> Score::findMelodyPattern(const std::vector<std::vector<Note>>& melodyPatterns, const float intervalSimilarityThreshold = 0.5, const float rhythmSimilarityThreshold = 0.5, <five callbacks>) const;`;
in `melodic-lines.h`: `void maiacore::detail::requireTwoNotes(const std::string& method, size_t numNotes)` (Task 6 reuses it);
file-local in `score.cpp`: `struct MelodySearchCallbacks`, `struct MelodySearchInput { const std::vector<Part>& parts; std::vector<MelodicLine> lines; std::vector<Key> concertKeys; }`, `const Note* firstSoundingNote(const std::vector<Note>&)`, `template <typename Visit> void forEachWindow(const MelodicLine& line, const size_t length, const Visit& visit)` (calls `visit(start, window)` for every window of the line, the last one included), `std::pair<std::string, float> transposition(const std::vector<Note>& pattern, const std::vector<Note>& window)`, `Score::MelodyPatternTable searchMelodicLines(const MelodySearchInput&, const std::vector<Note>&, float, float, const MelodySearchCallbacks&)`, `std::vector<Score::MelodyPatternTable> searchEachPattern(const MelodySearchInput&, const std::vector<std::vector<Note>>&, float, float, const MelodySearchCallbacks&)` (Task 5 reuses them);
`class maiacore_python::MelodyDataFrame { enum class Kind { Text, Integer, Real, List }; explicit MelodyDataFrame(const std::vector<std::pair<std::string, Kind>>& leading); void appendRow(const py::list& leading, const Score::MelodyPatternRow& match); py::object build() const; }` (Tasks 5, 6);
Python `Score.findMelodyPatternDataFrame(melodyPattern, intervalSimilarityThreshold=0.5, rhythmSimilarityThreshold=0.5, intervalsSimilarityCallback=None, rhythmSimilarityCallback=None, totalIntervalSimilarityCallback=None, totalRhythmSimilarityCallback=None, totalSimilarityCallback=None)` and its list overload with the same keywords (`melodyPatterns`).
`findAnyMelodyPattern` and its cache are untouched here (Task 5).

- [ ] **Step 1: The fixtures.** Create:

`test/xml_examples/unit_test/melody_last_window.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- One line, C4 D4 E4 G4: the pattern E4 G4 matches only the last two-note window, and the
     pattern C4 D4 E4 G4 only the window that is the whole line. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Flute</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>E</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
    </measure>
  </part>
</score-partwise>
```

`test/xml_examples/unit_test/melody_unnameable_transposition.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- One line: Cx5 Dx5 Ex5, then the quarter tones C1x4 D1x4 E1x4 (each a quarter tone above
     C4, D4 and E4). Both groups rise by two whole tones, as C4 D4 E4 does, but no interval name
     relates C4 to Cx5 (an augmented ninth) or to C1x4 (a quarter tone). -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Flute</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>3</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><alter>2</alter><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><accidental>double-sharp</accidental></note>
      <note><pitch><step>D</step><alter>2</alter><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><accidental>double-sharp</accidental></note>
      <note><pitch><step>E</step><alter>2</alter><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><accidental>double-sharp</accidental></note>
    </measure>
    <measure number="2">
      <note><pitch><step>C</step><alter>0.5</alter><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><accidental>quarter-sharp</accidental></note>
      <note><pitch><step>D</step><alter>0.5</alter><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><accidental>quarter-sharp</accidental></note>
      <note><pitch><step>E</step><alter>0.5</alter><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type><accidental>quarter-sharp</accidental></note>
    </measure>
  </part>
</score-partwise>
```

  Check both with `musicxml_check.py` as in Task 3 → `valid; errors: none; warnings: none`.

- [ ] **Step 2: Write the failing C++ tests** — in `tests-cpp/src/score-test.cpp`:
  - after `#include <algorithm>` insert `#include <cmath>`;
  - replace everything from the `namespace {` that opens `// The rejection of a segment that starts on a quarter tone, naming the note and where it is.` (~1036) through the closing `}` of `TEST(ScoreMelodyPatternSearch, APatternStartingOnAQuarterToneIsRejectedByName)` (~1095) — `segmentRejection` and the three quarter-tone rejection tests, whose behaviour this task removes deliberately — with:

```cpp
namespace {
// The written pitches of a table's matches, one list per row.
std::vector<std::vector<std::string>> writtenPitchesOf(const Score::MelodyPatternTable& table) {
    std::vector<std::vector<std::string>> pitches;
    for (const Score::MelodyPatternRow& row : table) {
        pitches.push_back(row.writtenPitches);
    }
    return pitches;
}

// The (measure, staff, voice) of a table's matches, one per row.
std::vector<std::tuple<int, int, int>> placesOf(const Score::MelodyPatternTable& table) {
    std::vector<std::tuple<int, int, int>> places;
    for (const Score::MelodyPatternRow& row : table) {
        places.emplace_back(row.measure, row.staff, row.voice);
    }
    return places;
}

// Notes of the given pitches, each a quarter note.
std::vector<Note> quarters(const std::vector<std::string>& pitches) {
    std::vector<Note> notes;
    for (const std::string& pitch : pitches) {
        notes.emplace_back(pitch);
    }
    return notes;
}
}  // namespace

// A melodic interval needs two notes: a shorter pattern is rejected, naming the method and the
// length, also when it is empty.
TEST(ScoreMelodyPatternSearch, APatternOfFewerThanTwoNotesIsRejected) {
    Score score("./test/xml_examples/unit_test/melody_last_window.musicxml");

    EXPECT_EQ(thrownFirstLine([&] { score.findMelodyPattern(quarters({"C4"})); }),
              "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, and "
              "this one has 1");
    EXPECT_EQ(thrownFirstLine([&] { score.findMelodyPattern(std::vector<Note>{}); }),
              "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, and "
              "this one has 0");
}

// The list overload searches each pattern on a worker thread. A pattern whose search raises fails
// the whole call with that error, exactly as the single-pattern overload does, instead of leaving
// an empty table behind.
TEST(ScoreMelodyPatternSearch, AFailingPatternFailsTheListOverloadToo) {
    Score score("./test/xml_examples/unit_test/melody_last_window.musicxml");
    const std::vector<std::vector<Note>> patterns = {quarters({"C4", "D4"}), quarters({"C4"})};

    const std::string single = thrownFirstLine([&] { score.findMelodyPattern(patterns[1]); });
    ASSERT_FALSE(single.empty());

    const std::string list = thrownFirstLine([&] { score.findMelodyPattern(patterns); });
    EXPECT_EQ(list, single);
}

// The last window of a line is searched: E4 G4 is the end of C4 D4 E4 G4, and C4 D4 E4 G4 is the
// whole line.
TEST(ScoreMelodyPatternSearch, TheLastWindowIsSearched) {
    Score score("./test/xml_examples/unit_test/melody_last_window.musicxml");

    EXPECT_EQ(writtenPitchesOf(score.findMelodyPattern(quarters({"E4", "G4"}), 1.0f, 1.0f)),
              (std::vector<std::vector<std::string>>{{"E4", "G4"}}));
    EXPECT_EQ(
        writtenPitchesOf(score.findMelodyPattern(quarters({"C4", "D4", "E4", "G4"}), 1.0f, 1.0f)),
        (std::vector<std::vector<std::string>>{{"C4", "D4", "E4", "G4"}}));
}

// Every voice of every staff is a line, the lower staff's voice 5 included, and no window spans
// two lines. The matches are sorted by measure; the two of measure 0 keep the order of their
// lines (staff 0 before staff 1).
TEST(ScoreMelodyPatternSearch, EveryVoiceOfEveryStaffIsSearched) {
    Score score("./test/xml_examples/unit_test/melody_staves_and_voices.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C4", "D4", "E4", "F4"}), 1.0f, 1.0f);

    EXPECT_EQ(placesOf(table),
              (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 1, 5}, {1, 1, 5}}));
    EXPECT_EQ(writtenPitchesOf(table),
              (std::vector<std::vector<std::string>>{
                  {"G5", "A5", "B5", "C6"}, {"C3", "D3", "E3", "F3"}, {"G3", "A3", "B3", "C4"}}));
}

// Rows are sorted by measure; rows of one measure keep the order of their lines. E4-F4 rises a
// semitone: in staff 0 voice 1 from E5 (measure 0) and B5 (measure 1), in staff 1 voice 5 from E3
// (measure 0) and B3 (measure 1).
TEST(ScoreMelodyPatternSearch, RowsAreSortedByMeasureThenLine) {
    Score score("./test/xml_examples/unit_test/melody_staves_and_voices.musicxml");
    const auto table = score.findMelodyPattern(quarters({"E4", "F4"}), 1.0f, 1.0f);

    EXPECT_EQ(placesOf(table),
              (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 1, 5}, {1, 0, 1}, {1, 1, 5}}));
}

// A chord is one event, represented by its highest sounding note whatever order its notes are
// written in; a grace note is no event.
TEST(ScoreMelodyPatternSearch, AChordIsSearchedByItsHighestNote) {
    Score score("./test/xml_examples/unit_test/melody_chord_top_note.musicxml");
    const auto table = score.findMelodyPattern(quarters({"G4", "D5", "B4"}), 1.0f, 1.0f);

    EXPECT_EQ(writtenPitchesOf(table), (std::vector<std::vector<std::string>>{{"G4", "D5", "B4"}}));
}

// A tied note is one event with the summed duration, in the measure where it starts: the window
// D4 E4 F4 has the rhythm quarter, dotted half, quarter.
TEST(ScoreMelodyPatternSearch, ATiedNoteIsOneEvent) {
    Score score("./test/xml_examples/unit_test/melody_tie_across_barline.musicxml");
    std::vector<Note> pattern = quarters({"D4", "E4", "F4"});
    pattern[1].setDuration(3.0f);

    const auto table = score.findMelodyPattern(pattern, 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0].measure, 0);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"D4", "E4", "F4"}));
    EXPECT_EQ(table[0].rhythmDiff, (std::vector<float>{0.0f, 0.0f, 0.0f}));
}

// A transposition without a name -- C4 to Cx5, an augmented ninth, or to the quarter tone C1x4
// -- leaves transposeInterval empty and the search goes on; transposeSemitones holds the exact
// interval.
TEST(ScoreMelodyPatternSearch, ATranspositionWithoutANameLeavesItEmpty) {
    Score score("./test/xml_examples/unit_test/melody_unnameable_transposition.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C4", "D4", "E4"}), 1.0f, 1.0f);

    ASSERT_EQ(table.size(), 2u);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"Cx5", "Dx5", "Ex5"}));
    EXPECT_EQ(table[0].transposeInterval, "");
    EXPECT_FLOAT_EQ(table[0].transposeSemitones, 14.0f);
    EXPECT_EQ(table[1].writtenPitches, (std::vector<std::string>{"C1x4", "D1x4", "E1x4"}));
    EXPECT_EQ(table[1].transposeInterval, "");
    EXPECT_FLOAT_EQ(table[1].transposeSemitones, 0.5f);
}

// A pattern that starts on a quarter tone is searched like any other: its interval contour is
// compared exactly, and no transposition from it has a name.
TEST(ScoreMelodyPatternSearch, APatternStartingOnAQuarterToneIsSearched) {
    Score score("./test/xml_examples/unit_test/melody_unnameable_transposition.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C1x4", "D1x4"}), 1.0f, 1.0f);

    EXPECT_EQ(writtenPitchesOf(table),
              (std::vector<std::vector<std::string>>{
                  {"Cx5", "Dx5"}, {"Dx5", "Ex5"}, {"C1x4", "D1x4"}, {"D1x4", "E1x4"}}));
    for (const Score::MelodyPatternRow& row : table) {
        EXPECT_EQ(row.transposeInterval, "");
    }
    EXPECT_FLOAT_EQ(table[2].transposeSemitones, 0.0f);
}

// A pattern or a window without a sounding note has no transposition: NaN, and no name.
TEST(ScoreMelodyPatternSearch, AWindowOfRestsHasNoTransposition) {
    Score score({"Flute"}, 1);
    score.getPart(0).getMeasure(0).addNote(quarters({"rest", "rest"}));

    const auto table = score.findMelodyPattern(quarters({"rest", "rest"}), 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"rest", "rest"}));
    EXPECT_EQ(table[0].transposeInterval, "");
    EXPECT_TRUE(std::isnan(table[0].transposeSemitones));
}
```

  - replace everything from the comment `// Only each part's first-voice melody is searched -- chords and other voices are skipped -- so a` (~1205) through the closing `}` of `TEST(ScoreMelodyPatternSearch, ATransposingPartIsComparedByThePitchesItSounds)` (~1242) with:

```cpp
// A pattern longer than every line finds nothing, however many notes the score has: here voice 1
// has one event and voice 2 two, in a score of three notes.
TEST(ScoreMelodyPatternSearch, APatternLongerThanEveryLineFindsNoMatch) {
    Score score({"Flute"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    measure.addNote(Note("C4"));
    for (const char* pitch : {"D4", "E4"}) {
        Note secondVoice(pitch);
        secondVoice.setVoice(2);
        measure.addNote(secondVoice);
    }
    const std::vector<Note> pattern = quarters({"C4", "D4", "E4"});
    ASSERT_EQ(score.getNumNotes(), 3);

    EXPECT_TRUE(score.findMelodyPattern(pattern).empty());
    const auto tables = score.findMelodyPattern(std::vector<std::vector<Note>>{pattern});
    ASSERT_EQ(tables.size(), 1u);
    EXPECT_TRUE(tables[0].empty());
    EXPECT_TRUE(score.findAnyMelodyPattern(3).empty());
}

// A part with more measures than the first part: the concert key, computed for the first part's
// measures, is empty beyond them, and the search goes on.
TEST(ScoreMelodyPatternSearch, AMeasureBeyondTheFirstPartHasNoConcertKey) {
    Score score({"Flute", "Oboe"}, 1);
    score.getPart(1).addMeasure(1);
    score.getPart(1).getMeasure(1).addNote(quarters({"C4", "D4"}));

    const auto table = score.findMelodyPattern(quarters({"C4", "D4"}), 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(std::make_tuple(table[0].partName, table[0].measure),
              std::make_tuple(score.getPartName(1), 1));
    EXPECT_EQ(table[0].writtenKey, "C");
    EXPECT_EQ(table[0].concertKey, "");
}

// ===== Parts of transposing instruments ===== //

// A horn in F's written B4, C#5 and D#5 sound E4, F#4 and G#4, the pattern itself: the window is
// the pattern at a unison, named "P1" without a trailing space.
TEST(ScoreMelodyPatternSearch, ATransposingPartIsComparedByThePitchesItSounds) {
    Score score({"Horn in F"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    for (const char* written : {"B4", "C#5", "D#5", "B4"}) {
        measure.addNote(hornInF(written));
    }

    const auto table = score.findMelodyPattern(quarters({"E4", "F#4", "G#4"}), 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0].transposeInterval, "P1");
    EXPECT_FLOAT_EQ(table[0].transposeSemitones, 0.0f);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"B4", "C#5", "D#5"}));
    EXPECT_EQ(table[0].soundingPitches, (std::vector<std::string>{"E4", "F#4", "G#4"}));
    EXPECT_FLOAT_EQ(table[0].totalSimilarity, 1.0f);
}

// A clarinet in B-flat's part: its written key and pitches, the pitches it sounds and the
// score's concert key, which the violin's C major gives.
TEST(ScoreMelodyPatternSearch, AMatchReportsTheWrittenAndTheConcertKey) {
    Score score("./test/xml_examples/unit_test/melody_transposing_instrument.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C4", "D4", "E4"}), 1.0f, 1.0f);

    ASSERT_EQ(table.size(), 1u);
    const Score::MelodyPatternRow& row = table[0];
    EXPECT_EQ(row.partName, "Clarinet in Bb");
    EXPECT_EQ(std::make_tuple(row.measure, row.staff, row.voice), std::make_tuple(0, 0, 1));
    EXPECT_EQ(row.writtenKey, "D");
    EXPECT_EQ(row.concertKey, "C");
    EXPECT_EQ(row.transposeInterval, "P1");
    EXPECT_EQ(row.writtenPitches, (std::vector<std::string>{"D4", "E4", "F#4"}));
    EXPECT_EQ(row.soundingPitches, (std::vector<std::string>{"C4", "D4", "E4"}));
    EXPECT_EQ(row.semitonesDiff, (std::vector<float>{0.0f, 0.0f}));
    EXPECT_FLOAT_EQ(row.intervalSimilarity, 1.0f);
    EXPECT_FLOAT_EQ(row.rhythmSimilarity, 1.0f);
}
```

- [ ] **Step 3: Write the failing Python tests** — in `test/test_score_comprehensive.py`:
  - replace the two methods `test_a_failing_pattern_fails_the_list_overload_too` and `test_a_pattern_starting_on_a_quarter_tone_is_rejected_by_name` of `ScoreMelodyPatternSearchTestCase` (~341-379) with:

```python
    def test_a_failing_pattern_fails_the_list_overload_too(self):
        """A pattern of one note has no melodic interval, so its search raises -- through the
        list overload exactly as through the single-pattern one, instead of answering an empty
        DataFrame."""
        score = ml.Score("./xml_examples/unit_test/test_quarter_tones.musicxml")
        pattern = [ml.Note("C4")]

        with self.assertRaises(RuntimeError) as single:
            score.findMelodyPatternDataFrame(pattern)
        with self.assertRaises(RuntimeError) as listed:
            score.findMelodyPatternDataFrame([[ml.Note("C4"), ml.Note("D4")], pattern])

        message = str(listed.exception).splitlines()[0]
        self.assertEqual(
            message,
            "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, and "
            "this one has 1",
        )
        self.assertEqual(message, str(single.exception).splitlines()[0])

    def test_a_quarter_tone_does_not_stop_the_search(self):
        """A window that starts on a quarter tone is compared exactly; its transposition has no
        name."""
        score = ml.Score("./xml_examples/unit_test/test_quarter_tones.musicxml")
        table = score.findMelodyPatternDataFrame([ml.Note("C4"), ml.Note("D4")], 0.0, 0.0)
        starts = table["writtenPitches"].map(lambda pitches: pitches[0] == "C1x4")
        self.assertGreater(int(starts.sum()), 0)
        self.assertEqual(set(table[starts]["transposeInterval"]), {""})
        self.assertEqual(set(table[starts]["transposeSemitones"]), {0.5})
```

  - in `test_a_pattern_longer_than_every_melody_finds_no_match` replace the docstring

```python
        """Only each part's first-voice melody is searched -- chords and other voices are
        skipped -- so a pattern can be longer than every melody without being longer than the
        score. No window of a melody fits it, and neither overload finds a match."""
```

    with

```python
        """A pattern longer than every melodic line finds nothing, however many notes the score
        has: voice 1 has one event and voice 2 two."""
```

    and `        pattern = [ml.Note("C4"), ml.Note("D4")]` (its first line after the loop) with `        pattern = [ml.Note("C4"), ml.Note("D4"), ml.Note("E4")]` (voice 2's two events now form a line, which a two-note pattern would match);
  - insert before `if __name__ == "__main__":` (two blank lines before and after):

```python
MATCH_COLUMNS = [
    "partName",
    "measure",
    "staff",
    "voice",
    "writtenKey",
    "concertKey",
    "transposeInterval",
    "transposeSemitones",
    "writtenPitches",
    "soundingPitches",
    "semitonesDiff",
    "rhythmDiff",
    "intervalSimilarity",
    "rhythmSimilarity",
    "totalSimilarity",
]


def quarters(*pitches):
    return [ml.Note(pitch) for pitch in pitches]


class ScoreMelodyPatternDataFrameTestCase(unittest.TestCase):
    """The DataFrames of the melody search: columns, dtypes, order and the new rules."""

    def test_the_columns_of_a_match(self):
        score = ml.Score("./xml_examples/unit_test/melody_transposing_instrument.musicxml")
        table = score.findMelodyPatternDataFrame(quarters("C4", "D4", "E4"), 1.0, 1.0)
        self.assertEqual(list(table.columns), MATCH_COLUMNS)
        row = table.iloc[0].to_dict()
        self.assertEqual(
            {key: row[key] for key in MATCH_COLUMNS[:8]},
            {
                "partName": "Clarinet in Bb",
                "measure": 0,
                "staff": 0,
                "voice": 1,
                "writtenKey": "D",
                "concertKey": "C",
                "transposeInterval": "P1",
                "transposeSemitones": 0.0,
            },
        )
        self.assertEqual(list(row["writtenPitches"]), ["D4", "E4", "F#4"])
        self.assertEqual(list(row["soundingPitches"]), ["C4", "D4", "E4"])
        self.assertEqual(len(table), 1)

    def test_an_empty_result_has_every_column_and_dtype(self):
        score = ml.Score("./xml_examples/unit_test/melody_last_window.musicxml")
        matched = score.findMelodyPatternDataFrame(quarters("C4", "D4"), 1.0, 1.0)
        empty = score.findMelodyPatternDataFrame(quarters("C4", "C6"), 1.0, 1.0)
        self.assertEqual(len(empty), 0)
        self.assertEqual(list(empty.dtypes.items()), list(matched.dtypes.items()))
        for column in ("measure", "staff", "voice"):
            self.assertEqual(str(matched[column].dtype), "int64", column)
        for column in ("transposeSemitones", "intervalSimilarity", "totalSimilarity"):
            self.assertEqual(str(matched[column].dtype), "float64", column)

        listed = score.findMelodyPatternDataFrame([quarters("C4", "D4")], 1.0, 1.0)
        empty_list = score.findMelodyPatternDataFrame([quarters("C4", "C6")], 1.0, 1.0)
        self.assertEqual(list(listed.columns), ["patternIdx"] + MATCH_COLUMNS)
        self.assertEqual(list(empty_list.dtypes.items()), list(listed.dtypes.items()))

    def test_every_voice_of_every_staff_is_searched_and_rows_sort_by_measure(self):
        score = ml.Score("./xml_examples/unit_test/melody_staves_and_voices.musicxml")
        table = score.findMelodyPatternDataFrame(quarters("C4", "D4", "E4", "F4"), 1.0, 1.0)
        self.assertEqual(
            list(zip(table["measure"], table["staff"], table["voice"])),
            [(0, 0, 1), (0, 1, 5), (1, 1, 5)],
        )

    def test_the_list_overload_sorts_by_pattern_then_measure(self):
        score = ml.Score("./xml_examples/unit_test/melody_staves_and_voices.musicxml")
        table = score.findMelodyPatternDataFrame(
            [quarters("C4", "D4", "E4", "F4"), quarters("C5", "B4")], 1.0, 1.0
        )
        self.assertEqual(
            list(zip(table["patternIdx"], table["measure"], table["staff"])),
            [(0, 0, 0), (0, 0, 1), (0, 1, 1), (1, 0, 0)],
        )

    def test_a_transposition_without_a_name_does_not_stop_the_search(self):
        score = ml.Score("./xml_examples/unit_test/melody_unnameable_transposition.musicxml")
        table = score.findMelodyPatternDataFrame(quarters("C4", "D4", "E4"), 1.0, 1.0)
        self.assertEqual(list(table["transposeInterval"]), ["", ""])
        self.assertEqual(list(table["transposeSemitones"]), [14.0, 0.5])

    def test_a_pattern_of_fewer_than_two_notes_raises(self):
        score = ml.Score("./xml_examples/unit_test/melody_last_window.musicxml")
        for pattern in ([], quarters("C4")):
            with self.subTest(length=len(pattern)), self.assertRaises(RuntimeError) as context:
                score.findMelodyPatternDataFrame(pattern)
            self.assertEqual(
                str(context.exception).splitlines()[0],
                "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, "
                f"and this one has {len(pattern)}",
            )

    def test_the_thresholds_have_the_same_names_in_both_overloads(self):
        score = ml.Score("./xml_examples/unit_test/melody_last_window.musicxml")
        single = score.findMelodyPatternDataFrame(
            quarters("C4", "D4"), intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0
        )
        listed = score.findMelodyPatternDataFrame(
            [quarters("C4", "D4")], intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0
        )
        self.assertEqual((len(single), len(listed)), (2, 2))
        with self.assertRaises(TypeError):
            score.findMelodyPatternDataFrame(
                quarters("C4", "D4"), totalIntervalsSimilarityThreshold=1.0
            )
```

- [ ] **Step 4: Run them and see them fail.** C++ subset `ScoreMelodyPatternSearch.*` → the build fails (`no member named 'writtenPitches' in 'std::tuple<...>'`). Python: «pytest» `test_score_comprehensive.ScoreMelodyPatternSearchTestCase test_score_comprehensive.ScoreMelodyPatternDataFrameTestCase` → the new tests fail: `KeyError: 'measure'`/column lists differ, the quarter-tone test raises the old rejection, and `test_a_failing_pattern_fails_the_list_overload_too` gets the old helper's message (`referenceMelody and otherMelody must have 2 elements minimum!`); record each.

- [ ] **Step 5: The header** — in `maiacore/include/maiacore/score.h`:
  - insert `#include <functional>` before `#include <initializer_list>` and `#include <tuple>` after `#include <string>`;
  - replace everything from `    /**` that opens `     * @brief Table row type for melodic pattern search results.` (~553) through the end of the list overload's declaration `        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;` that comes before `     * @brief Finds all possible melodic patterns of a given length in the score.` (~673) with:

```cpp
    /**
     * @brief One match of a melodic pattern: a window of a melodic line of the score.
     * @details The melody search reads every voice of every staff of every part as a melodic
     *          line of events -- a note, a chord by its highest sounding note, or a rest; a
     *          tied note extends the event it is tied to -- and compares the pattern with every
     *          window of as many consecutive events of one line (see findMelodyPattern()).
     */
    struct MelodyPatternRow {
        std::string partName;    ///< The part.
        int measure = 0;         ///< 0-based index of the measure of the window's first event.
        int staff = 0;           ///< 0-based staff of the line.
        int voice = 0;           ///< Voice of the line, as written.
        std::string writtenKey;  ///< The part's written key at that measure (Key::getName()).
        /// The score's concert key at that measure, by the rule of getChords().
        std::string concertKey;
        /// The interval from the pattern's first sounding note to the window's, named at concert
        /// spelling with its direction ("M2 asc", "P1"); empty when it has no name or when the
        /// pattern or the window has no sounding note.
        std::string transposeInterval;
        /// That interval in exact semitones (quarter tones included); NaN when the pattern or
        /// the window has no sounding note.
        float transposeSemitones = 0.0f;
        std::vector<std::string> writtenPitches;   ///< The window's written pitches ("rest").
        std::vector<std::string> soundingPitches;  ///< The window's sounding pitches ("rest").
        std::vector<float> semitonesDiff;          ///< Per-interval differences (pattern size - 1).
        std::vector<float> rhythmDiff;             ///< Per-duration differences (pattern size).
        float intervalSimilarity = 0.0f;           ///< The interval similarity.
        float rhythmSimilarity = 0.0f;             ///< The rhythm similarity.
        float totalSimilarity = 0.0f;              ///< The combined similarity.

        /**
         * @brief Field-by-field equality; a NaN transposeSemitones equals no value, NaN included.
         */
        bool operator==(const MelodyPatternRow& other) const {
            return std::tie(partName, measure, staff, voice, writtenKey, concertKey,
                            transposeInterval, transposeSemitones, writtenPitches, soundingPitches,
                            semitonesDiff, rhythmDiff, intervalSimilarity, rhythmSimilarity,
                            totalSimilarity) ==
                   std::tie(other.partName, other.measure, other.staff, other.voice,
                            other.writtenKey, other.concertKey, other.transposeInterval,
                            other.transposeSemitones, other.writtenPitches, other.soundingPitches,
                            other.semitonesDiff, other.rhythmDiff, other.intervalSimilarity,
                            other.rhythmSimilarity, other.totalSimilarity);
        }
    };

    /**
     * @brief The matches of one pattern, sorted stably by measure: matches of one measure keep
     *        the order of their lines (part, staff, voice) and windows.
     */
    typedef std::vector<MelodyPatternRow> MelodyPatternTable;

    /**
     * @brief Searches every melodic line of the score for a melodic pattern.
     * @param melodyPattern The pattern: at least 2 notes; rests are allowed.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match (0.0-1.0).
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match (0.0-1.0).
     * @param intervalsSimilarityCallback Replaces the interval differences
     *        (Helper::getSemitonesDifferenceBetweenMelodies()); give
     *        totalIntervalSimilarityCallback with it.
     * @param rhythmSimilarityCallback Replaces the duration differences
     *        (Helper::getDurationDifferenceBetweenRhythms()); give totalRhythmSimilarityCallback
     *        with it.
     * @param totalIntervalSimilarityCallback Reduces the interval differences to a similarity;
     *        used only with intervalsSimilarityCallback.
     * @param totalRhythmSimilarityCallback Reduces the duration differences to a similarity;
     *        used only with rhythmSimilarityCallback.
     * @param totalSimilarityCallback Combines the two similarities; the default is their mean.
     * @return One row per match, sorted stably by measure: rows of one measure keep the order of
     *         their lines (part, staff, voice) and windows.
     * @throws std::runtime_error If the pattern has fewer than 2 notes; the message names the
     *         method and the length.
     * @throws std::bad_function_call If intervalsSimilarityCallback or rhythmSimilarityCallback
     *         is given without its total callback.
     * @details **Melodic lines.** Every voice that occurs on a staff of a part is a line: its
     *          events in measure order. A note without `<chord/>` starts an event, and the chord
     *          notes written after it join it; the event stands for its highest note by sounding
     *          exact position. A note tied to the previous event of its line at the same sounding
     *          position extends that event, which keeps its first note's written pitch and
     *          measure and adds the durations. A rest is an event; a grace note is not.
     *
     *          **Windows.** Every window of as many consecutive events of one line as the pattern
     *          has notes, the last one included; no window spans two lines. A pattern longer
     *          than every line finds nothing.
     *
     *          **Comparison.** The melodic intervals of the pattern and of the window are
     *          compared at sounding exact positions, so the comparison is transposition-invariant
     *          and quarter tones count as half semitones; an interval to or from a rest is 0.
     *          Durations are divided by each sequence's longest. Similarity is 1 / (1 + the
     *          Euclidean norm of the differences) unless a callback replaces it, and a window
     *          matches when both similarities reach their thresholds.
     *
     *          **Transposition.** For a match only, transposeSemitones and transposeInterval
     *          relate the pattern's first sounding note to the window's; an interval without a
     *          name (an augmented ninth C4 -> Cx5, any quarter-tone interval) leaves
     *          transposeInterval empty and never stops the search.
     * @note Each call builds the melodic lines from the score as it is. The search reads the
     *       score and writes nothing another thread shares, so any number of threads may search
     *       the same score at once. Modifying the score, its parts, measures or notes while a
     *       search runs is not safe.
     */
    MelodyPatternTable findMelodyPattern(
        const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold = 0.5,
        const float rhythmSimilarityThreshold = 0.5,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;

    /**
     * @brief Searches the score for several melodic patterns, returning a table for each
     * pattern.
     * @details Builds the melodic lines once and searches each pattern on a worker thread, as
     *          the single-pattern overload searches it.
     * @param melodyPatterns The patterns.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match.
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match.
     * @param intervalsSimilarityCallback See the single-pattern overload.
     * @param rhythmSimilarityCallback See the single-pattern overload.
     * @param totalIntervalSimilarityCallback See the single-pattern overload.
     * @param totalRhythmSimilarityCallback See the single-pattern overload.
     * @param totalSimilarityCallback See the single-pattern overload.
     * @return One table per pattern, in pattern order.
     * @throws std::runtime_error Or std::bad_function_call, as the single-pattern overload throws
     *         them, for any pattern: the first such exception, in pattern order, is rethrown once
     *         every pattern has been searched.
     * @note Any number of threads may search the same score at once. Modifying the score, its
     * parts, measures or notes while a search runs is not safe.
     */
    std::vector<MelodyPatternTable> findMelodyPattern(
        const std::vector<std::vector<Note>>& melodyPatterns,
        const float intervalSimilarityThreshold = 0.5, const float rhythmSimilarityThreshold = 0.5,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;
```

- [ ] **Step 6: The search** — the check of a pattern's length goes next to the melodic lines, where `ScoreCollection` (Task 6) reaches it too:
  - in `maiacore/src/maiacore/melodic-lines.h` insert `#include <string>` before `#include <vector>`, and before `}  // namespace maiacore::detail` insert (one blank line after):

```cpp
/**
 * @brief Rejects a melody pattern of fewer than 2 notes: a melodic interval needs two. Every
 *        melody search makes this check, naming itself.
 * @param method The searching method, which the message names first
 *        ("Score::findMelodyPattern").
 * @param numNotes The pattern's number of notes.
 * @throws std::runtime_error If numNotes is less than 2: "<method>: a melody pattern needs at
 *         least 2 notes, and this one has <numNotes>".
 */
void requireTwoNotes(const std::string& method, size_t numNotes);
```

  - in `maiacore/src/maiacore/melodic-lines.cpp` insert `#include "maiacore/log.h"` before `#include "maiacore/measure.h"`, and before its last line, `}  // namespace maiacore::detail`, insert (one blank line after):

```cpp
void requireTwoNotes(const std::string& method, const size_t numNotes) {
    if (numNotes < 2) {
        LOG_ERROR(method + ": a melody pattern needs at least 2 notes, and this one has " +
                  std::to_string(numNotes));
    }
}
```

  In `maiacore/src/maiacore/score.cpp`:
  - after `#include <exception>` insert `#include <functional>`; after `#include <sstream>` insert `#include <stdexcept>`; after `#include "maiacore/utils.h"` insert `#include "melodic-lines.h"`; after `using maiacore::detail::concertPitch;` insert `using maiacore::detail::MelodicLine;`, `using maiacore::detail::melodicLines;` and `using maiacore::detail::requireTwoNotes;`;
  - delete `rejectQuarterToneTransposition` with its comment: everything from `// Rejects a melody-pattern search whose pattern, or whose segment of the score, starts on a quarter` (~57) up to, not including, `// A transposing interval -- <octave-change> folded in -- and an octave doubling, as a` (~89);
  - replace everything from `Score::MelodyPatternTable Score::findMelodyPattern(` (~2117) through the closing `}` of the list overload (the line before `void Score::removeDuplicatePatterns(std::vector<std::vector<Note>>* patterns) const {`, ~2352) with:

```cpp
namespace {
// The callbacks of a melody search, passed on unchanged to every pattern it searches.
struct MelodySearchCallbacks {
    std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)> intervals;
    std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)> rhythm;
    std::function<float(const std::vector<float>&)> totalIntervals;
    std::function<float(const std::vector<float>&)> totalRhythm;
    std::function<float(float, float)> total;
};

// What every melody search reads: the parts, their melodic lines and the concert key of each
// measure, built once per call.
struct MelodySearchInput {
    const std::vector<Part>& parts;
    std::vector<MelodicLine> lines;
    std::vector<Key> concertKeys;
};

// The first note of 'notes' that is not a rest; nullptr when every one is a rest.
const Note* firstSoundingNote(const std::vector<Note>& notes) {
    for (const Note& note : notes) {
        if (note.isNoteOn()) {
            return &note;
        }
    }
    return nullptr;
}

// The transposition from a pattern to a window that matches it: the interval from the pattern's
// first sounding note to the window's, in exact semitones, and its name at concert spelling with
// its direction ("M2 asc", "P1"). NaN and no name when either has no sounding note; no name when
// the interval has none, which Interval reports by throwing (an augmented ninth C4 -> Cx5) and
// cannot build for a quarter tone, which is checked first.
std::pair<std::string, float> transposition(const std::vector<Note>& pattern,
                                            const std::vector<Note>& window) {
    const Note* from = firstSoundingNote(pattern);
    const Note* to = firstSoundingNote(window);
    if (from == nullptr || to == nullptr) {
        return {std::string(), std::numeric_limits<float>::quiet_NaN()};
    }
    const float semitones = to->getQuarterToneSteps() - from->getQuarterToneSteps();
    if (from->isQuarterTone() || to->isQuarterTone()) {
        return {std::string(), semitones};
    }
    try {
        const Interval interval(*from, *to);
        const std::string name = interval.getName();
        const std::string direction = interval.getDirection();
        return {direction.empty() ? name : name + " " + direction, semitones};
    } catch (const std::runtime_error&) {
        return {std::string(), semitones};
    }
}

// Calls 'visit(start, window)' for every window of 'length' consecutive events of 'line', in line
// order, the last one included: 'window' holds the notes of the events from index 'start' on. A
// line shorter than 'length' has no window.
template <typename Visit>
void forEachWindow(const MelodicLine& line, const size_t length, const Visit& visit) {
    std::vector<Note> window;
    window.reserve(length);
    for (size_t start = 0; start + length <= line.events.size(); start++) {
        window.clear();
        for (size_t offset = 0; offset < length; offset++) {
            window.push_back(line.events[start + offset].note);
        }
        visit(start, window);
    }
}

// Searches every window of every melodic line for 'pattern'. Rows are appended line by line,
// window by window, then sorted stably by measure.
Score::MelodyPatternTable searchMelodicLines(const MelodySearchInput& input,
                                             const std::vector<Note>& pattern,
                                             const float intervalSimilarityThreshold,
                                             const float rhythmSimilarityThreshold,
                                             const MelodySearchCallbacks& callbacks) {
    requireTwoNotes("Score::findMelodyPattern", pattern.size());
    const size_t length = pattern.size();
    Score::MelodyPatternTable table;
    for (const MelodicLine& line : input.lines) {
        const Part& part = input.parts.at(line.partIdx);
        forEachWindow(line, length, [&](const size_t start, const std::vector<Note>& window) {
            const std::vector<float> semitonesDiff =
                (callbacks.intervals == nullptr)
                    ? Helper::getSemitonesDifferenceBetweenMelodies(pattern, window)
                    : callbacks.intervals(pattern, window);
            const std::vector<float> rhythmDiff =
                (callbacks.rhythm == nullptr)
                    ? Helper::getDurationDifferenceBetweenRhythms(pattern, window)
                    : callbacks.rhythm(pattern, window);
            const float intervalSimilarity =
                (callbacks.intervals == nullptr)
                    ? Helper::calculateMelodyEuclideanSimilarity(semitonesDiff)
                    : callbacks.totalIntervals(semitonesDiff);
            const float rhythmSimilarity =
                (callbacks.rhythm == nullptr)
                    ? Helper::calculateRhythmicEuclideanSimilarity(rhythmDiff)
                    : callbacks.totalRhythm(rhythmDiff);
            if (intervalSimilarity < intervalSimilarityThreshold ||
                rhythmSimilarity < rhythmSimilarityThreshold) {
                return;
            }

            Score::MelodyPatternRow row;
            const int measureIdx = line.events[start].measureIdx;
            row.partName = part.getName();
            row.measure = measureIdx;
            row.staff = line.staff;
            row.voice = line.voice;
            row.writtenKey = part.getMeasure(measureIdx).getKey().getName();
            if (measureIdx < static_cast<int>(input.concertKeys.size())) {
                row.concertKey = input.concertKeys[measureIdx].getName();
            }
            std::tie(row.transposeInterval, row.transposeSemitones) =
                transposition(pattern, window);
            for (const Note& note : window) {
                row.writtenPitches.push_back(note.getWrittenPitch());
                row.soundingPitches.push_back(note.getSoundingPitch());
            }
            row.semitonesDiff = semitonesDiff;
            row.rhythmDiff = rhythmDiff;
            row.intervalSimilarity = intervalSimilarity;
            row.rhythmSimilarity = rhythmSimilarity;
            row.totalSimilarity = (callbacks.total == nullptr)
                                      ? (intervalSimilarity + rhythmSimilarity) / 2.0f
                                      : callbacks.total(intervalSimilarity, rhythmSimilarity);
            table.push_back(std::move(row));
        });
    }
    std::stable_sort(table.begin(), table.end(),
                     [](const Score::MelodyPatternRow& a, const Score::MelodyPatternRow& b) {
                         return a.measure < b.measure;
                     });
    return table;
}

// Searches each pattern on a worker thread. An exception cannot leave a worker thread, so each
// pattern's search stores the one it raised in its own slot, and the first of them, in pattern
// order, is rethrown once every worker has joined: a failing pattern fails the whole call, as it
// does for a single pattern. Each worker takes the next pattern nobody has claimed until none is
// left, so every pattern is searched however many there are; the thread count only bounds how
// many run at once. Workers write distinct elements of 'tables' and 'errors' and only read the
// input, so no lock is needed.
std::vector<Score::MelodyPatternTable> searchEachPattern(
    const MelodySearchInput& input, const std::vector<std::vector<Note>>& patterns,
    const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
    const MelodySearchCallbacks& callbacks) {
    const size_t numPatterns = patterns.size();
    std::vector<Score::MelodyPatternTable> tables(numPatterns);
    std::vector<std::exception_ptr> errors(numPatterns);
    std::atomic<size_t> nextPattern{0};
    auto worker = [&]() {
        for (size_t idx = nextPattern++; idx < numPatterns; idx = nextPattern++) {
            try {
                tables[idx] = searchMelodicLines(input, patterns[idx], intervalSimilarityThreshold,
                                                 rhythmSimilarityThreshold, callbacks);
            } catch (...) {
                errors[idx] = std::current_exception();
            }
        }
    };

    // hardware_concurrency() answers 0 when it cannot tell, hence at least one thread.
    const size_t numThreads = std::max<size_t>(
        1, std::min(numPatterns, static_cast<size_t>(std::thread::hardware_concurrency())));
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (size_t t = 0; t < numThreads; ++t) {
        threads.emplace_back(worker);
    }
    for (auto& thread : threads) {
        thread.join();
    }

    for (const auto& error : errors) {
        if (error) {
            std::rethrow_exception(error);
        }
    }
    return tables;
}
}  // namespace

Score::MelodyPatternTable Score::findMelodyPattern(
    const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
    const std::function<float(float, float)> totalSimilarityCallback) const {
    const MelodySearchInput input{_part, melodicLines(_part), concertKeys(_part)};
    return searchMelodicLines(
        input, melodyPattern, intervalSimilarityThreshold, rhythmSimilarityThreshold,
        {intervalsSimilarityCallback, rhythmSimilarityCallback, totalIntervalSimilarityCallback,
         totalRhythmSimilarityCallback, totalSimilarityCallback});
}

std::vector<Score::MelodyPatternTable> Score::findMelodyPattern(
    const std::vector<std::vector<Note>>& melodyPatterns, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
    const std::function<float(float, float)> totalSimilarityCallback) const {
    if (melodyPatterns.empty()) {
        return {};
    }
    const MelodySearchInput input{_part, melodicLines(_part), concertKeys(_part)};
    return searchEachPattern(
        input, melodyPatterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
        {intervalsSimilarityCallback, rhythmSimilarityCallback, totalIntervalSimilarityCallback,
         totalRhythmSimilarityCallback, totalSimilarityCallback});
}
```

- [ ] **Step 7: Keep `ScoreCollection` compiling** — `score_collection.cpp` reads `Score::MelodyPatternRow` with `std::get`; until Task 6 rewrites it, build its tuples from the fields. Replace

```cpp
            results.emplace_back(score.getFileName(), score.getComposerName(), score.getTitle(),
                                 std::get<0>(row), std::get<1>(row), std::get<2>(row),
                                 std::get<3>(row), std::get<4>(row), std::get<5>(row),
                                 std::get<6>(row), std::get<7>(row), std::get<8>(row),
                                 std::get<9>(row), std::get<10>(row));
```

  with

```cpp
            results.emplace_back(score.getFileName(), score.getComposerName(), score.getTitle(),
                                 row.partName, row.measure, row.staff, row.writtenKey,
                                 row.transposeInterval, row.writtenPitches, row.semitonesDiff,
                                 row.rhythmDiff, row.intervalSimilarity, row.rhythmSimilarity,
                                 row.totalSimilarity);
```

  and the list overload's `extendedTable.emplace_back(patternIdx, ...);` (from `                extendedTable.emplace_back(patternIdx,` through its closing `                );`) with

```cpp
                extendedTable.emplace_back(patternIdx, score.getFileName(), score.getComposerName(),
                                           score.getTitle(), row.partName, row.measure, row.staff,
                                           row.writtenKey, row.transposeInterval,
                                           row.writtenPitches, row.semitonesDiff, row.rhythmDiff,
                                           row.intervalSimilarity, row.rhythmSimilarity,
                                           row.totalSimilarity);
```

- [ ] **Step 8: The DataFrame builder** — create `maiacore/src/maiacore/python_wrapper/py_melody_dataframe.h`:

```cpp
#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/score.h"

namespace maiacore_python {

namespace py = pybind11;

// The pandas DataFrame of a melody search, built column by column. Every column has a fixed
// dtype, so a search without a match gives an empty DataFrame with every column, and with the
// dtypes of a search with matches: text columns `str`, integer columns `int64`, real columns
// `float64` and list columns `object`.
class MelodyDataFrame {
   public:
    enum class Kind { Text, Integer, Real, List };

    // 'leading' names the columns that come before the match's own (patternIdx, fileName, ...),
    // with their kinds.
    explicit MelodyDataFrame(const std::vector<std::pair<std::string, Kind>>& leading)
        : _numLeading(leading.size()) {
        for (const auto& column : leading) {
            _columns.push_back({column.first, column.second, py::list()});
        }
        const std::vector<std::pair<std::string, Kind>> matchColumns = {
            {"partName", Kind::Text},           {"measure", Kind::Integer},
            {"staff", Kind::Integer},           {"voice", Kind::Integer},
            {"writtenKey", Kind::Text},         {"concertKey", Kind::Text},
            {"transposeInterval", Kind::Text},  {"transposeSemitones", Kind::Real},
            {"writtenPitches", Kind::List},     {"soundingPitches", Kind::List},
            {"semitonesDiff", Kind::List},      {"rhythmDiff", Kind::List},
            {"intervalSimilarity", Kind::Real}, {"rhythmSimilarity", Kind::Real},
            {"totalSimilarity", Kind::Real}};
        for (const auto& column : matchColumns) {
            _columns.push_back({column.first, column.second, py::list()});
        }
    }

    // Appends a row: 'leading' holds the values of the leading columns, in their order.
    void appendRow(const py::list& leading, const Score::MelodyPatternRow& match) {
        if (leading.size() != _numLeading) {
            throw std::logic_error("MelodyDataFrame: a row needs one value per leading column");
        }
        size_t c = 0;
        for (const py::handle value : leading) {
            _columns[c++].values.append(value);
        }
        const py::object values[] = {py::cast(match.partName),
                                     py::cast(match.measure),
                                     py::cast(match.staff),
                                     py::cast(match.voice),
                                     py::cast(match.writtenKey),
                                     py::cast(match.concertKey),
                                     py::cast(match.transposeInterval),
                                     py::cast(match.transposeSemitones),
                                     py::cast(match.writtenPitches),
                                     py::cast(match.soundingPitches),
                                     py::cast(match.semitonesDiff),
                                     py::cast(match.rhythmDiff),
                                     py::cast(match.intervalSimilarity),
                                     py::cast(match.rhythmSimilarity),
                                     py::cast(match.totalSimilarity)};
        for (const py::object& value : values) {
            _columns[c++].values.append(value);
        }
    }

    // The DataFrame, with the rows in the order they were appended.
    py::object build() const {
        const py::module_ pandas = py::module_::import("pandas");
        const py::object text = py::module_::import("builtins").attr("str");
        py::dict data;
        for (const Column& column : _columns) {
            py::object dtype;
            switch (column.kind) {
                case Kind::Text:
                    dtype = text;
                    break;
                case Kind::Integer:
                    dtype = py::str("int64");
                    break;
                case Kind::Real:
                    dtype = py::str("float64");
                    break;
                case Kind::List:
                    dtype = py::str("object");
                    break;
            }
            data[py::str(column.name)] =
                pandas.attr("Series")(column.values, py::arg("dtype") = dtype);
        }
        return pandas.attr("DataFrame")(data);
    }

   private:
    struct Column {
        std::string name;
        Kind kind;
        py::list values;
    };
    size_t _numLeading;
    std::vector<Column> _columns;
};

}  // namespace maiacore_python
```

- [ ] **Step 9: The bindings** — in `maiacore/src/maiacore/python_wrapper/py_score.cpp`:
  - after `#include "nlohmann/json.hpp"` insert `#include "py_melody_dataframe.h"`, and after `using namespace pybind11::literals;` insert `using maiacore_python::MelodyDataFrame;`;
  - replace everything from the `    cls.def(` whose next line is `        "findMelodyPatternDataFrame",` (~171) through the list overload's closing `    )pbdoc");` (~400, just before `    // cls.def(`) with:

```cpp
    cls.def(
        "findMelodyPatternDataFrame",
        [](const Score& score, const std::vector<Note>& melodyPattern,
           const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {
            const Score::MelodyPatternTable table =
                score.findMelodyPattern(melodyPattern, intervalSimilarityThreshold,
                                        rhythmSimilarityThreshold, intervalsSimilarityCallback,
                                        rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                                        totalRhythmSimilarityCallback, totalSimilarityCallback);
            MelodyDataFrame frame({});
            for (const Score::MelodyPatternRow& row : table) {
                frame.appendRow(py::list(), row);
            }
            return frame.build();
        },
        py::arg("melodyPattern"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
        R"pbdoc(
        Search every melodic line of the score for a melodic pattern.

        Every voice of every staff of every part is a melodic line: its events in measure
        order. A note without ``<chord/>`` starts an event, and the chord notes written after it
        join it: a chord is represented by its highest sounding note. A note tied to the previous
        event of its line at the same sounding pitch extends that event, which keeps its first
        note's written pitch and measure and adds the durations. A rest is an event (its melodic
        interval counts as 0); a grace note is not. Every window of as many consecutive events of
        one line as the pattern has notes, the last one included, is compared with the pattern;
        no window spans two voices, staves or parts.

        The melodic intervals are compared at sounding exact positions, so the comparison is
        transposition-invariant and a quarter tone counts as half a semitone
        (``Helper.getSemitonesDifferenceBetweenMelodies``); the durations are divided by each
        sequence's longest (``Helper.getDurationDifferenceBetweenRhythms``). Each list of
        differences is reduced to a similarity, ``1 / (1 + norm)`` unless a callback replaces
        it, and a window matches when both similarities reach their thresholds. The search
        builds the lines from the score as it is at each call.

        Parameters
        ----------
        melodyPattern : list of Note
            The pattern: at least 2 notes; rests are allowed.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
            ``f(pattern, window) -> list of float``, replacing the interval differences. Give
            ``totalIntervalSimilarityCallback`` with it.
        rhythmSimilarityCallback : callable, optional
            ``f(pattern, window) -> list of float``, replacing the duration differences. Give
            ``totalRhythmSimilarityCallback`` with it.
        totalIntervalSimilarityCallback : callable, optional
            ``f(differences) -> float``, reducing the interval differences to a similarity; used
            only with ``intervalsSimilarityCallback``.
        totalRhythmSimilarityCallback : callable, optional
            ``f(differences) -> float``, reducing the duration differences to a similarity; used
            only with ``rhythmSimilarityCallback``.
        totalSimilarityCallback : callable, optional
            ``f(intervalSimilarity, rhythmSimilarity) -> float``; the default is their mean.

        Returns
        -------
        pandas.DataFrame
            One row per match, sorted stably by ``measure``: the matches of one measure keep the
            order of their parts, staves, voices and windows. An empty DataFrame, with every
            column and its dtype, when nothing matches. The columns:

            - ``partName`` (str), ``measure`` (int, the 0-based measure index of the window's
              first event), ``staff`` (int, 0-based), ``voice`` (int, as written);
            - ``writtenKey`` (str), the part's written key at that measure, and ``concertKey``
              (str), the score's concert key there, by the rule of ``getChords``;
            - ``transposeInterval`` (str), the interval from the pattern's first sounding note
              to the window's, named at concert spelling with its direction (``"M2 asc"``,
              ``"P1"``): empty when that interval has no name (an augmented ninth ``C4`` ->
              ``Cx5``, any quarter-tone interval) or when the pattern or the window has no
              sounding note; ``transposeSemitones`` (float), that interval in exact semitones,
              ``NaN`` when either has no sounding note;
            - ``writtenPitches`` and ``soundingPitches`` (list of str), the window's pitches as
              its part writes them and as they sound (``"rest"`` for a rest);
            - ``semitonesDiff`` (list of float, one per interval) and ``rhythmDiff`` (list of
              float, one per event), the differences;
            - ``intervalSimilarity``, ``rhythmSimilarity`` and ``totalSimilarity`` (float).

        Raises
        ------
        RuntimeError
            If the pattern has fewer than 2 notes, or if ``intervalsSimilarityCallback`` or
            ``rhythmSimilarityCallback`` is given without its total callback ("bad function
            call"). A quarter tone, or a transposition without a name, never stops the search.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> pattern = [ml.Note("G2"), ml.Note("D3"), ml.Note("B3")]
        >>> table = score.findMelodyPatternDataFrame(pattern, 1.0, 1.0)
        >>> len(table) > 0, float(table["totalSimilarity"].min())
        (True, 1.0)
    )pbdoc");

    cls.def(
        "findMelodyPatternDataFrame",
        [](const Score& score, const std::vector<std::vector<Note>>& melodyPatterns,
           const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {
            // The search runs each pattern on a worker thread, and a worker that calls, copies or
            // destroys a Python callback takes the GIL to do it. Holding the GIL here while the
            // workers run would deadlock, so it is released for the search alone, inside this
            // lambda; it is held again when the lambda returns, before the DataFrame is built.
            // The search only reads the score, so other threads may search it meanwhile.
            const auto tables = [&] {
                py::gil_scoped_release release;
                return score.findMelodyPattern(
                    melodyPatterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
                    intervalsSimilarityCallback, rhythmSimilarityCallback,
                    totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                    totalSimilarityCallback);
            }();
            MelodyDataFrame frame({{"patternIdx", MelodyDataFrame::Kind::Integer}});
            for (size_t idx = 0; idx < tables.size(); idx++) {
                for (const Score::MelodyPatternRow& row : tables[idx]) {
                    frame.appendRow(py::list(py::make_tuple(idx)), row);
                }
            }
            return frame.build();
        },
        py::arg("melodyPatterns"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Search every melodic line of the score for several melodic patterns, each on a worker
        thread.

        Every pattern is searched exactly as the single-pattern overload searches it, however
        many patterns there are, with the same thresholds and callbacks. The search releases the
        GIL while it runs, so a Python callback is called from the worker threads, one call at a
        time, each taking the GIL, and other Python threads run meanwhile. The search only reads
        the score, so any number of threads may search the same score at once; no thread may
        modify the score, its parts, measures or notes while a search of it runs.

        Parameters
        ----------
        melodyPatterns : list of list of Note
            The patterns, each of at least 2 notes.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of the single-pattern overload, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            ``patternIdx`` (int, the pattern's index in ``melodyPatterns``) followed by the
            single-pattern overload's columns; sorted by ``patternIdx``, then as the
            single-pattern overload sorts. An empty DataFrame, with every column and its dtype,
            when nothing matches.

        Raises
        ------
        RuntimeError
            If the search for any pattern raises, for the reasons the single-pattern overload
            gives: the first such error, in pattern order, once every pattern has been searched
            -- never an empty result in its place.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> patterns = [[ml.Note("G2"), ml.Note("D3")], [ml.Note("D3"), ml.Note("B3")]]
        >>> table = score.findMelodyPatternDataFrame(patterns, 1.0, 1.0)
        >>> sorted(table["patternIdx"].unique().tolist())
        [0, 1]
    )pbdoc");
```

- [ ] **Step 10: Format, build, pass.** clang-format `melodic-lines.h`, `melodic-lines.cpp`, `score.h`, `score.cpp`, `score_collection.cpp`, `py_melody_dataframe.h`, `py_score.cpp`, `score-test.cpp`. C++ subset `MelodicLines.*:ScoreMelodyPatternSearch.*:ScoreCollection*` → pass (`PatternsAQuarterToneApartAreNotMergedAsDuplicates` still finds 3: `findAnyMelodyPattern` keeps its old windows until Task 5). «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_score_comprehensive` → OK.

- [ ] **Step 11: Ledger.** «build» `make "PYTHON=$py" corpus-update-ledger` → 0; `git diff test/musicxml/ledger.json` shows exactly:

```
  "test/xml_examples/unit_test/melody_last_window.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
  "test/xml_examples/unit_test/melody_unnameable_transposition.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
```

- [ ] **Step 12: Mutations** (C++ in `score.cpp`, rebuilt with the C++ subset; for the Python tests also «build» `make "PYTHON=$py" dev`). (a) In `requireTwoNotes` (`melodic-lines.cpp`) replace `numNotes < 2` with `numNotes < 1` → `APatternOfFewerThanTwoNotesIsRejected` and `test_a_pattern_of_fewer_than_two_notes_raises` fail (the helper's own message), and `test_a_failing_pattern_fails_the_list_overload_too` fails. (b) In `searchEachPattern` replace `errors[idx] = std::current_exception();` with `(void)0;` → `AFailingPatternFailsTheListOverloadToo` fails. (c) In `forEachWindow` replace `start + length <= line.events.size()` with `start + length < line.events.size()` → `TheLastWindowIsSearched` fails. (d) In `searchMelodicLines`, as the loop body's first statement, add `if (line.voice != 1) { continue; }` → `EveryVoiceOfEveryStaffIsSearched` and `test_every_voice_of_every_staff_is_searched_and_rows_sort_by_measure` fail. (e) Delete the `std::stable_sort(...)` call → `RowsAreSortedByMeasureThenLine` fails. (f) In `transposition` delete the `try {` … `} catch (const std::runtime_error&) { … }` wrapper, keeping its body → `ATranspositionWithoutANameLeavesItEmpty` and `test_a_transposition_without_a_name_does_not_stop_the_search` fail (`Unable to compute the interval [C4, Cx5]`). (g) In `transposition` replace `to->getQuarterToneSteps() - from->getQuarterToneSteps()` with `static_cast<float>(to->getMidiNumber() - from->getMidiNumber())` → `ATranspositionWithoutANameLeavesItEmpty` fails (`1` for the quarter tone). (h) In `transposition`, before `const float semitones`, add `if (from->isQuarterTone()) { LOG_ERROR("quarter-tone pattern"); }` → `APatternStartingOnAQuarterToneIsSearched` fails; with `to->isQuarterTone()` instead → `test_a_quarter_tone_does_not_stop_the_search` fails. (i) Replace `std::numeric_limits<float>::quiet_NaN()` with `0.0f` → `AWindowOfRestsHasNoTransposition` fails. (j) Replace `return {direction.empty() ? name : name + " " + direction, semitones};` with `return {name + " " + direction, semitones};` → `ATransposingPartIsComparedByThePitchesItSounds` and `AMatchReportsTheWrittenAndTheConcertKey` fail (`"P1 "`). (k) Replace `row.concertKey = input.concertKeys[measureIdx].getName();` with `row.concertKey = row.writtenKey;` → `AMatchReportsTheWrittenAndTheConcertKey` and `test_the_columns_of_a_match` fail; replace the whole `if (measureIdx < ...) { ... }` with `row.concertKey = input.concertKeys.at(measureIdx).getName();` → `AMeasureBeyondTheFirstPartHasNoConcertKey` fails (`std::out_of_range`). (l) Replace `row.soundingPitches.push_back(note.getSoundingPitch());` with `row.soundingPitches.push_back(note.getWrittenPitch());` → `AMatchReportsTheWrittenAndTheConcertKey` fails. (m) In `melodic-lines.cpp` replace `partLines[{s, note.getVoice()}]` with `partLines[{s, 1}]` → `APatternLongerThanEveryLineFindsNoMatch` and `test_a_pattern_longer_than_every_melody_finds_no_match` fail (one line `C4 D4 E4`). (n) In `py_melody_dataframe.h` replace `{"concertKey", Kind::Text}` with `{"concert", Kind::Text}` → `test_the_columns_of_a_match` fails; replace `dtype = py::str("int64");` with `dtype = py::str("object");` → `test_an_empty_result_has_every_column_and_dtype` fails. (o) In the list binding, after `return frame.build();` is computed, sort by measure: replace `return frame.build();` (list overload only) with `return frame.build().attr("sort_values")("measure", "kind"_a = "stable");` → `test_the_list_overload_sorts_by_pattern_then_measure` fails. (p) Rename the list overload's `py::arg("intervalSimilarityThreshold")` to `py::arg("totalIntervalsSimilarityThreshold")` → `test_the_thresholds_have_the_same_names_in_both_overloads` fails. Revert each; rerun green.

- [ ] **Step 13: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (Task 3's + 9: 12 new, 3 removed); «build» `make "PYTHON=$py" py-tests` → OK (Task 3's + 7); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 14: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/src/maiacore/melodic-lines.h maiacore/src/maiacore/melodic-lines.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/score.cpp maiacore/src/maiacore/score_collection.cpp maiacore/src/maiacore/python_wrapper/py_melody_dataframe.h maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/score-test.cpp test/test_score_comprehensive.py test/xml_examples/unit_test/melody_last_window.musicxml test/xml_examples/unit_test/melody_unnameable_transposition.musicxml test/musicxml/ledger.json`, message:

```
feat: the melody search reads every melodic line, to its last window

Score::findMelodyPattern searches every voice of every staff as its own
line (chords by their highest sounding note, ties merged), every window
of it including the last. A pattern of fewer than 2 notes raises, naming
the method and the length; one longer than every line finds nothing.
transposeInterval is computed for matches only and is empty when the
interval has no name, so an augmented ninth or a quarter tone no longer
aborts the search; transposeSemitones gives the exact interval.

Results are a struct with the unified names (measure, staff, voice,
writtenKey, concertKey, writtenPitches, soundingPitches,
intervalSimilarity, ...) and the thresholds are intervalSimilarityThreshold
and rhythmSimilarityThreshold in both overloads. The DataFrames have a
fixed dtype per column, also when empty, and are sorted by pattern, then
measure, then line.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 5: `findAnyMelodyPattern` fixed and exposed; the note-event cache removed

**Files:**
- Create (fixture): `test/xml_examples/unit_test/melody_duplicate_patterns.musicxml`
- Modify: `maiacore/include/maiacore/score.h` (`NoteEvent`, the cache members, `collectNoteEventsPerPart`, `removeDuplicatePatterns` ~46-74; the cache lines of the copy constructor and `operator=` ~512-515, ~543-546; `FoundMelodyPattern` after `MelodyPatternTable`; the `findAnyMelodyPattern` declaration), `maiacore/src/maiacore/score.cpp` (`collectNoteEventsPerPart` deleted; `distinctWindows` added; `removeDuplicatePatterns` and `findAnyMelodyPattern` replaced), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (the commented-out `findAnyMelodyPatternDataFrame`, ~402-509), `test/xml_examples/unit_test/melody_patterns_quarter_tone_apart.xml` (its comment), `test/musicxml/ledger.json` (1 added line)
- Test: `tests-cpp/src/score-test.cpp` (`PatternsAQuarterToneApartAreNotMergedAsDuplicates` and two new tests; one after `AWindowOfRestsHasNoTransposition`), `test/test_score_comprehensive.py` (new class `ScoreFindAnyMelodyPatternTestCase`)

**Interfaces:** consumes `MelodySearchInput`, `MelodySearchCallbacks`, `firstSoundingNote`, `forEachWindow`, `searchEachPattern` (Task 4) and `MelodyDataFrame` (Task 4). Produces `struct Score::FoundMelodyPattern { std::vector<Note> pattern; MelodyPatternTable matches; };`, `std::vector<FoundMelodyPattern> Score::findAnyMelodyPattern(const int patternNumNotes = 5, const float intervalSimilarityThreshold = 1.0f, const float rhythmSimilarityThreshold = 1.0f, const int minOccurrences = 2, <five callbacks>) const;` (only the patterns with at least `minOccurrences` matches), file-local `std::vector<std::vector<Note>> distinctWindows(const std::vector<MelodicLine>& lines, const size_t length)`, and Python `Score.findAnyMelodyPatternDataFrame(patternNumNotes=5, intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0, minOccurrences=2, intervalsSimilarityCallback=None, rhythmSimilarityCallback=None, totalIntervalSimilarityCallback=None, totalRhythmSimilarityCallback=None, totalSimilarityCallback=None)`.

- [ ] **Step 1: The fixture** — create `test/xml_examples/unit_test/melody_duplicate_patterns.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- One line, C4 D4 C4 D4 in quarter notes, then E4 F#4 in half notes. Its two-note windows are
     C4-D4, D4-C4, C4-D4 again (the pattern of the first), D4-E4 (a quarter and a half) and
     E4-F#4, the interval of C4-D4 in another rhythm: four distinct patterns. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Flute</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
    </measure>
    <measure number="2">
      <note><pitch><step>E</step><octave>4</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
      <note><pitch><step>F</step><alter>1</alter><octave>4</octave></pitch><duration>2</duration><voice>1</voice><type>half</type><accidental>sharp</accidental></note>
    </measure>
  </part>
</score-partwise>
```

  Check it with `musicxml_check.py` → `valid; errors: none; warnings: none`. In `test/xml_examples/unit_test/melody_patterns_quarter_tone_apart.xml` the comment says the quarter tone "sits where no two-note window starts on it", which the last window now contradicts: replace

```xml
      <!-- C4 D4 C4 D1b4 E4: the two-note windows C4-D4 (+2 semitones) and C4-D1b4 (+1.5) are
           different melodic patterns, a quarter tone apart, although D1b4 rounds to the MIDI
           number of D4. The quarter tone sits where no two-note window starts on it. -->
```

  with

```xml
      <!-- C4 D4 C4 D1b4 E4: the two-note windows C4-D4 (+2 semitones) and C4-D1b4 (+1.5) are
           different melodic patterns, a quarter tone apart, although D1b4 rounds to the MIDI
           number of D4. The last window, D1b4-E4 (+2.5), is a fourth pattern. -->
```

- [ ] **Step 2: Write the failing C++ tests** — in `tests-cpp/src/score-test.cpp`:
  - replace `TEST(ScoreMelodyPatternSearch, PatternsAQuarterToneApartAreNotMergedAsDuplicates)` with its two-line comment (from `// C4-D4 (+2) and C4-D1b4 (+1.5) are two patterns a quarter tone apart, not duplicates: comparing` through the test's closing `}`) with:

```cpp
// C4-D4 (+2) and C4-D1b4 (+1.5) are two patterns a quarter tone apart, not duplicates. The line
// C4 D4 C4 D1b4 E4 has four distinct two-note windows: C4-D4, D4-C4, C4-D1b4 and, the last,
// D1b4-E4 (+2.5).
TEST(ScoreMelodyPatternSearch, PatternsAQuarterToneApartAreNotMergedAsDuplicates) {
    Score score("./test/xml_examples/unit_test/melody_patterns_quarter_tone_apart.xml");
    const auto found = score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1);
    EXPECT_EQ(found.size(), 4u);
}

// Two windows are one pattern when their intervals and their durations are equal; the first
// window is kept. C4-D4 in quarter notes occurs twice; E4-F#4 has its interval in half notes and
// is another pattern. Each pattern is searched with the given thresholds, so C4-D4 also matches
// E4-F#4, whose rhythm has the same proportions. With minOccurrences 1 every pattern is kept.
TEST(ScoreMelodyPatternSearch, FindAnyMelodyPatternKeepsTheFirstOfEqualWindows) {
    Score score("./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml");
    const auto found = score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1);

    std::vector<std::vector<std::string>> patterns;
    std::vector<float> firstDurations;
    for (const Score::FoundMelodyPattern& entry : found) {
        std::vector<std::string> pitches;
        for (const Note& note : entry.pattern) {
            pitches.push_back(note.getWrittenPitch());
        }
        patterns.push_back(pitches);
        firstDurations.push_back(entry.pattern[0].getQuarterDuration());
    }
    EXPECT_EQ(patterns, (std::vector<std::vector<std::string>>{
                            {"C4", "D4"}, {"D4", "C4"}, {"D4", "E4"}, {"E4", "F#4"}}));
    EXPECT_EQ(firstDurations, (std::vector<float>{1.0f, 1.0f, 1.0f, 2.0f}));
    ASSERT_EQ(found.size(), 4u);
    EXPECT_EQ(writtenPitchesOf(found[0].matches),
              (std::vector<std::vector<std::string>>{{"C4", "D4"}, {"C4", "D4"}, {"E4", "F#4"}}));
}

// A pattern needs at least 2 notes: a smaller length is rejected by name, never a crash.
TEST(ScoreMelodyPatternSearch, FindAnyMelodyPatternRejectsFewerThanTwoNotes) {
    Score score("./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml");
    for (const int length : {1, 0, -1}) {
        EXPECT_EQ(thrownFirstLine([&] { score.findAnyMelodyPattern(length); }),
                  "[maiacore] Score::findAnyMelodyPattern: patternNumNotes must be at least 2, "
                  "and it is " +
                      std::to_string(length))
            << length;
    }
}

// A pattern is kept when it has at least minOccurrences matches, its own window included. At the
// default thresholds of 1 and the default of 2, C4-D4 (matched by C4-D4 twice and E4-F#4) and
// E4-F#4 (the same three) are kept; D4-C4 and D4-E4, which occur once, are not.
TEST(ScoreMelodyPatternSearch, FindAnyMelodyPatternKeepsPatternsThatOccurAtLeastMinOccurrences) {
    Score score("./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml");
    const auto firstPitches = [](const std::vector<Score::FoundMelodyPattern>& found) {
        std::vector<std::string> pitches;
        for (const Score::FoundMelodyPattern& entry : found) {
            pitches.push_back(entry.pattern[0].getWrittenPitch() + "-" +
                              entry.pattern[1].getWrittenPitch());
        }
        return pitches;
    };

    EXPECT_EQ(firstPitches(score.findAnyMelodyPattern(2)),
              (std::vector<std::string>{"C4-D4", "E4-F#4"}));
    EXPECT_EQ(firstPitches(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 3)),
              (std::vector<std::string>{"C4-D4", "E4-F#4"}));
    EXPECT_TRUE(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 4).empty());
    for (const int minOccurrences : {0, -1}) {
        EXPECT_EQ(
            thrownFirstLine([&] { score.findAnyMelodyPattern(2, 1.0f, 1.0f, minOccurrences); }),
            "[maiacore] Score::findAnyMelodyPattern: minOccurrences must be at least 1, "
            "and it is " +
                std::to_string(minOccurrences))
            << minOccurrences;
    }
}
```

  - after the closing `}` of `TEST(ScoreMelodyPatternSearch, AWindowOfRestsHasNoTransposition)` insert (one blank line before):

```cpp
// Each search reads the score as it is: a note added after a search is found by the next one.
TEST(ScoreMelodyPatternSearch, ASearchSeesTheEditsMadeBeforeIt) {
    Score score({"Flute"}, 2);
    score.getPart(0).getMeasure(0).addNote(quarters({"C4", "D4"}));
    ASSERT_EQ(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1).size(), 1u);
    ASSERT_TRUE(score.findMelodyPattern(quarters({"D4", "C#5"}), 1.0f, 1.0f).empty());

    score.getPart(0).getMeasure(1).addNote(Note("C#5"));

    EXPECT_EQ(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1).size(), 2u);
    EXPECT_EQ(writtenPitchesOf(score.findMelodyPattern(quarters({"D4", "C#5"}), 1.0f, 1.0f)),
              (std::vector<std::vector<std::string>>{{"D4", "C#5"}}));
}
```

- [ ] **Step 3: Write the failing Python tests** — in `test/test_score_comprehensive.py`, insert before `if __name__ == "__main__":` (two blank lines before and after):

```python
class ScoreFindAnyMelodyPatternTestCase(unittest.TestCase):
    """findAnyMelodyPatternDataFrame: every distinct window as a pattern, with its matches."""

    def test_equal_windows_are_one_pattern_and_rhythm_tells_them_apart(self):
        score = ml.Score("./xml_examples/unit_test/melody_duplicate_patterns.musicxml")
        table = score.findAnyMelodyPatternDataFrame(2, 1.0, 1.0, minOccurrences=1)
        self.assertEqual(list(table.columns), ["patternIdx", "patternPitches"] + MATCH_COLUMNS)
        patterns = table.drop_duplicates("patternIdx")
        self.assertEqual(
            [list(pitches) for pitches in patterns["patternPitches"]],
            [["C4", "D4"], ["D4", "C4"], ["D4", "E4"], ["E4", "F#4"]],
        )
        first_pattern = table[table["patternIdx"] == 0]
        self.assertEqual(
            [list(pitches) for pitches in first_pattern["writtenPitches"]],
            [["C4", "D4"], ["C4", "D4"], ["E4", "F#4"]],
        )

    def test_min_occurrences_keeps_the_patterns_that_repeat(self):
        """C4-D4 and E4-F#4 have three matches each; D4-C4 and D4-E4 one. The kept patterns are
        numbered from 0."""
        score = ml.Score("./xml_examples/unit_test/melody_duplicate_patterns.musicxml")
        for min_occurrences, expected in (
            (2, [["C4", "D4"], ["E4", "F#4"]]),
            (3, [["C4", "D4"], ["E4", "F#4"]]),
            (4, []),
        ):
            with self.subTest(minOccurrences=min_occurrences):
                table = score.findAnyMelodyPatternDataFrame(2, minOccurrences=min_occurrences)
                patterns = table.drop_duplicates("patternIdx")
                self.assertEqual([list(p) for p in patterns["patternPitches"]], expected)
                self.assertEqual(list(patterns["patternIdx"]), list(range(len(expected))))

    def test_fewer_than_two_notes_or_occurrences_below_one_raise(self):
        score = ml.Score("./xml_examples/unit_test/melody_duplicate_patterns.musicxml")
        for length in (1, 0, -1):
            with self.subTest(length=length), self.assertRaises(RuntimeError):
                score.findAnyMelodyPatternDataFrame(length)
        for min_occurrences in (0, -1):
            with self.subTest(minOccurrences=min_occurrences), self.assertRaises(
                RuntimeError
            ) as context:
                score.findAnyMelodyPatternDataFrame(2, minOccurrences=min_occurrences)
            self.assertEqual(
                str(context.exception).splitlines()[0],
                "[maiacore] Score::findAnyMelodyPattern: minOccurrences must be at least 1, and it "
                f"is {min_occurrences}",
            )

    def test_no_pattern_kept_gives_an_empty_dataframe_with_every_column(self):
        score = ml.Score("./xml_examples/unit_test/melody_duplicate_patterns.musicxml")
        matched = score.findAnyMelodyPatternDataFrame(2)
        self.assertEqual(str(matched["patternIdx"].dtype), "int64")
        for empty in (
            score.findAnyMelodyPatternDataFrame(7),
            score.findAnyMelodyPatternDataFrame(2, minOccurrences=4),
        ):
            self.assertEqual(len(empty), 0)
            self.assertEqual(str(empty["patternIdx"].dtype), "int64")
            self.assertEqual(list(empty.dtypes.items()), list(matched.dtypes.items()))

    def test_the_docstring_gives_the_size_at_the_defaults(self):
        doc = " ".join(ml.Score.findAnyMelodyPatternDataFrame.__doc__.split())
        self.assertIn("gives 2,164 patterns and 489,196 rows", doc)

    def test_the_defaults_keep_exact_repetitions_transposed_or_not(self):
        """C4 D4 E4 F4 G4 recurs a tone higher as D4 E4 F#4 G4 A4, an exact repetition; the
        variant C4 D4 E4 F4 G#4 matches only at lower thresholds. No other five-note window
        repeats."""
        score = ml.Score(["Flute"], 3)
        score.getPart(0).getMeasure(0).addNote(["C4", "D4", "E4", "F4", "G4"])
        score.getPart(0).getMeasure(1).addNote(["D4", "E4", "F#4", "G4", "A4"])
        score.getPart(0).getMeasure(2).addNote(["C4", "D4", "E4", "F4", "G#4"])
        table = score.findAnyMelodyPatternDataFrame()
        self.assertTrue(table.equals(score.findAnyMelodyPatternDataFrame(5, 1.0, 1.0, 2)))
        self.assertEqual(
            [
                (index, list(pitches), interval)
                for index, pitches, interval in zip(
                    table["patternIdx"], table["writtenPitches"], table["transposeInterval"]
                )
            ],
            [
                (0, ["C4", "D4", "E4", "F4", "G4"], "P1"),
                (0, ["D4", "E4", "F#4", "G4", "A4"], "M2 asc"),
            ],
        )
        self.assertEqual(
            len(score.findAnyMelodyPatternDataFrame(5, 0.5, 0.5).query("patternIdx == 0")), 3
        )
```

- [ ] **Step 4: Run them and see them fail.** C++ subset `ScoreMelodyPatternSearch.*` → the build fails (`no type named 'FoundMelodyPattern' in 'Score'`). «pytest» `test_score_comprehensive.ScoreFindAnyMelodyPatternTestCase` → `AttributeError: 'maialib.maiacore.Score' object has no attribute 'findAnyMelodyPatternDataFrame'`.

- [ ] **Step 5: The header** — in `maiacore/include/maiacore/score.h`:
  - delete everything from the `    /**` that opens `     * @brief Internal structure to represent a note event in the score.` (~46) through `    void removeDuplicatePatterns(std::vector<std::vector<Note>>* patterns) const;` (~74) and the blank line after it;
  - in the copy constructor and in `operator=`, delete the blank line and the three lines `        // Invalidate the per-part note-event cache - it is rebuilt when needed`, `        _isNoteEventsPerPartCached = false;`, `        _cachedNoteEventsPerPart.clear();` (both occurrences);
  - after `    typedef std::vector<MelodyPatternRow> MelodyPatternTable;` insert (one blank line before):

```cpp
    /**
     * @brief A pattern that findAnyMelodyPattern() found in the score, with its matches.
     */
    struct FoundMelodyPattern {
        std::vector<Note> pattern;   ///< The window's events, as the search compares them.
        MelodyPatternTable matches;  ///< Its matches, as findMelodyPattern() returns them.
    };
```

  - replace the `findAnyMelodyPattern` declaration with its comment (from the `    /**` that opens `     * @brief Finds all possible melodic patterns of a given length in the score.` through its `        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;`) with:

```cpp
    /**
     * @brief Finds every distinct melodic pattern of a given length in the score, with its
     *        matches.
     * @details Every window of patternNumNotes events of a melodic line (see
     *          findMelodyPattern()) is a pattern. Two windows are the same pattern when their
     *          events are the same notes and rests at the same exact positions relative to their
     *          first sounding note (quarter tones included) and have the same durations, exactly;
     *          of equal windows the first, in line order, is kept. Each pattern is then searched
     *          as the list overload of findMelodyPattern() searches it, and kept when it has at
     *          least minOccurrences matches, its own window included. At the default thresholds
     *          of 1 a match is an exact repetition, transposed or not.
     * @param patternNumNotes Number of events in each pattern: at least 2.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match (default 1).
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match (default 1).
     * @param minOccurrences Minimum number of matches of a kept pattern: at least 1 (default
     *        2, a pattern that repeats).
     * @param intervalsSimilarityCallback See findMelodyPattern().
     * @param rhythmSimilarityCallback See findMelodyPattern().
     * @param totalIntervalSimilarityCallback See findMelodyPattern().
     * @param totalRhythmSimilarityCallback See findMelodyPattern().
     * @param totalSimilarityCallback See findMelodyPattern().
     * @return The kept patterns in line order of their first window, each with its matches.
     * @throws std::runtime_error If patternNumNotes is less than 2 or minOccurrences less than 1,
     *         or as findMelodyPattern() throws.
     * @note Any number of threads may search the same score at once. Modifying the score, its
     * parts, measures or notes while a search runs is not safe.
     */
    std::vector<FoundMelodyPattern> findAnyMelodyPattern(
        const int patternNumNotes = 5, const float intervalSimilarityThreshold = 1.0f,
        const float rhythmSimilarityThreshold = 1.0f, const int minOccurrences = 2,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)> totalSimilarityCallback = nullptr) const;
```

- [ ] **Step 6: The implementation** — in `maiacore/src/maiacore/score.cpp`:
  - delete `Score::collectNoteEventsPerPart()`: everything from `std::vector<std::vector<Score::NoteEvent>> Score::collectNoteEventsPerPart() const {` through its closing `}` and the blank line after it;
  - in the anonymous namespace Task 4 added, insert after the closing `}` of `searchEachPattern` (the `}` after `    return tables;`), before `}  // namespace` (one blank line before):

```cpp
// The distinct windows of 'length' events of the lines, in line order. A window is keyed by each
// event's exact position relative to the window's first sounding note -- infinity for a rest --
// and its duration in quarter notes, so equal keys are the same pattern transposed.
std::vector<std::vector<Note>> distinctWindows(const std::vector<MelodicLine>& lines,
                                               const size_t length) {
    std::vector<std::vector<Note>> windows;
    std::set<std::vector<float>> seen;
    for (const MelodicLine& line : lines) {
        forEachWindow(line, length, [&](size_t /*start*/, const std::vector<Note>& window) {
            const Note* first = firstSoundingNote(window);
            std::vector<float> key;
            key.reserve(2 * length);
            for (const Note& note : window) {
                key.push_back(note.isNoteOn()
                                  ? note.getQuarterToneSteps() - first->getQuarterToneSteps()
                                  : std::numeric_limits<float>::infinity());
                key.push_back(note.getQuarterDuration());
            }
            if (seen.insert(key).second) {
                windows.push_back(window);
            }
        });
    }
    return windows;
}
```

  - replace everything from `void Score::removeDuplicatePatterns(std::vector<std::vector<Note>>* patterns) const {` through the closing `}` of `Score::findAnyMelodyPattern` (the line before `bool Score::haveAnacrusisMeasure() const { return _haveAnacrusisMeasure; }`) with:

```cpp
std::vector<Score::FoundMelodyPattern> Score::findAnyMelodyPattern(
    const int patternNumNotes, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold, const int minOccurrences,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
    const std::function<float(float, float)> totalSimilarityCallback) const {
    if (patternNumNotes < 2) {
        LOG_ERROR("Score::findAnyMelodyPattern: patternNumNotes must be at least 2, and it is " +
                  std::to_string(patternNumNotes));
    }
    if (minOccurrences < 1) {
        LOG_ERROR("Score::findAnyMelodyPattern: minOccurrences must be at least 1, and it is " +
                  std::to_string(minOccurrences));
    }
    const MelodySearchInput input{_part, melodicLines(_part), concertKeys(_part)};
    std::vector<std::vector<Note>> patterns =
        distinctWindows(input.lines, static_cast<size_t>(patternNumNotes));
    std::vector<MelodyPatternTable> tables = searchEachPattern(
        input, patterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
        {intervalsSimilarityCallback, rhythmSimilarityCallback, totalIntervalSimilarityCallback,
         totalRhythmSimilarityCallback, totalSimilarityCallback});

    std::vector<FoundMelodyPattern> found;
    found.reserve(patterns.size());
    for (size_t i = 0; i < patterns.size(); i++) {
        if (tables[i].size() >= static_cast<size_t>(minOccurrences)) {
            found.push_back({std::move(patterns[i]), std::move(tables[i])});
        }
    }
    return found;
}
```

- [ ] **Step 7: The binding** — in `maiacore/src/maiacore/python_wrapper/py_score.cpp`, replace the commented-out binding, from `    // cls.def(` (the line before `    // "findAnyMelodyPatternDataFrame",`, ~402) through `    // );` (~509), with:

```cpp
    cls.def(
        "findAnyMelodyPatternDataFrame",
        [](const Score& score, const int patternNumNotes, const float intervalSimilarityThreshold,
           const float rhythmSimilarityThreshold, const int minOccurrences,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {
            // The patterns are searched on worker threads, as the list overload of
            // findMelodyPatternDataFrame searches them, so the GIL is released the same way.
            const auto found = [&] {
                py::gil_scoped_release release;
                return score.findAnyMelodyPattern(
                    patternNumNotes, intervalSimilarityThreshold, rhythmSimilarityThreshold,
                    minOccurrences, intervalsSimilarityCallback, rhythmSimilarityCallback,
                    totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                    totalSimilarityCallback);
            }();
            MelodyDataFrame frame({{"patternIdx", MelodyDataFrame::Kind::Integer},
                                   {"patternPitches", MelodyDataFrame::Kind::List}});
            for (size_t idx = 0; idx < found.size(); idx++) {
                std::vector<std::string> pitches;
                for (const Note& note : found[idx].pattern) {
                    pitches.push_back(note.getWrittenPitch());
                }
                const py::list leading(py::make_tuple(idx, pitches));
                for (const Score::MelodyPatternRow& row : found[idx].matches) {
                    frame.appendRow(leading, row);
                }
            }
            return frame.build();
        },
        py::arg("patternNumNotes") = 5, py::arg("intervalSimilarityThreshold") = 1.0f,
        py::arg("rhythmSimilarityThreshold") = 1.0f, py::arg("minOccurrences") = 2,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Find every distinct melodic pattern of a given length in the score, with its matches.

        Every window of ``patternNumNotes`` events of a melodic line (see
        ``findMelodyPatternDataFrame``) is a pattern. Two windows are the same pattern when
        their events are the same notes and rests at the same exact positions relative to their
        first sounding note (quarter tones included) and have the same durations, exactly; of
        equal windows the first, in line order, is kept. Each pattern is then searched as the
        list overload of ``findMelodyPatternDataFrame`` searches it -- so a pattern finds at
        least its own window -- and kept when it has at least ``minOccurrences`` matches. At the
        default thresholds of 1 a match is an exact repetition of the pattern, transposed or not,
        and the default ``minOccurrences=2`` keeps the patterns that repeat: the Beethoven 5
        sample, 13,675 notes, gives 2,164 patterns and 489,196 rows. Lower thresholds
        also count close variants, and can give millions of rows for such a score. The GIL is
        released while the patterns are searched.

        Parameters
        ----------
        patternNumNotes : int, default 5
            Number of events in each pattern: at least 2.
        intervalSimilarityThreshold : float, default 1.0
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 1.0
            Minimum rhythm similarity of a match, from 0 to 1.
        minOccurrences : int, default 2
            Minimum number of matches of a kept pattern, its own window included: at least 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of ``findMelodyPatternDataFrame``, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            One row per match of a kept pattern: ``patternIdx`` (int, the kept pattern's number,
            in line order of its first window) and ``patternPitches`` (list of str, the pattern's written pitches,
            ``"rest"`` for a rest), followed by the columns of ``findMelodyPatternDataFrame``;
            sorted by ``patternIdx``, then as ``findMelodyPatternDataFrame`` sorts. An empty
            DataFrame, with every column and its dtype, when no pattern is kept.

        Raises
        ------
        RuntimeError
            If ``patternNumNotes`` is less than 2 or ``minOccurrences`` less than 1, or for a
            callback without its total callback.

        Examples
        --------
        >>> score = ml.Score(["Flute"], 1)
        >>> score.getPart(0).getMeasure(0).addNote(["C4", "D4", "C4", "D4"])
        >>> table = score.findAnyMelodyPatternDataFrame(2)
        >>> table[["patternIdx", "patternPitches", "measure", "writtenPitches"]].values.tolist()
        [[0, ['C4', 'D4'], 0, ['C4', 'D4']], [0, ['C4', 'D4'], 0, ['C4', 'D4']]]
        >>> score.findAnyMelodyPatternDataFrame(2, minOccurrences=1)["patternIdx"].unique().tolist()
        [0, 1]
    )pbdoc");
```

- [ ] **Step 8: Format, build, pass.** clang-format `score.h`, `score.cpp`, `py_score.cpp`, `score-test.cpp`. `git grep -n -w -e NoteEvent -e removeDuplicatePatterns -e collectNoteEventsPerPart -e _cachedNoteEventsPerPart -e _isNoteEventsPerPartCached -- maiacore` → nothing (`-w`, so `getChordsPerEachNoteEvent` is not reported). C++ subset `ScoreMelodyPatternSearch.*` → pass. «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_score_comprehensive` → OK.

- [ ] **Step 9: Ledger.** «build» `make "PYTHON=$py" corpus-update-ledger` → 0; `git diff test/musicxml/ledger.json` shows exactly (the comment edit changes no field of `melody_patterns_quarter_tone_apart.xml`):

```
  "test/xml_examples/unit_test/melody_duplicate_patterns.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
```

- [ ] **Step 10: Mutations.** (a) In `forEachWindow`, which `distinctWindows` calls (the only `for (size_t start = 0; start + length <= line.events.size(); start++) {` of `score.cpp`), replace `start + length <= line.events.size()` with `start + length < line.events.size()` → `PatternsAQuarterToneApartAreNotMergedAsDuplicates` (3) and `FindAnyMelodyPatternKeepsTheFirstOfEqualWindows` fail. (b) Delete `key.push_back(note.getQuarterDuration());` → `FindAnyMelodyPatternKeepsTheFirstOfEqualWindows` and `test_equal_windows_are_one_pattern_and_rhythm_tells_them_apart` fail (`E4-F#4` merges into `C4-D4`). (c) Replace `if (seen.insert(key).second) {` with `if (seen.insert(key).second || true) {` → both fail (5 windows). (d) In `findAnyMelodyPattern` replace the `LOG_ERROR(...)` of the `patternNumNotes < 2` check with `return {};` → `FindAnyMelodyPatternRejectsFewerThanTwoNotes` and `test_fewer_than_two_notes_or_occurrences_below_one_raise` fail; replace the `LOG_ERROR(...)` of the `minOccurrences < 1` check with `return {};` → `FindAnyMelodyPatternKeepsPatternsThatOccurAtLeastMinOccurrences` and the same Python test fail. (e) Replace `if (tables[i].size() >= static_cast<size_t>(minOccurrences)) {` with `if (tables[i].size() > static_cast<size_t>(minOccurrences)) {` → `FindAnyMelodyPatternKeepsPatternsThatOccurAtLeastMinOccurrences` (nothing kept at 3) and `test_min_occurrences_keeps_the_patterns_that_repeat` fail; with `if (true) {` (no filter) → both fail, and so does `test_the_defaults_keep_exact_repetitions_transposed_or_not`. (f) In `findAnyMelodyPattern` declare the input `static const MelodySearchInput input{...}` and run the C++ subset with the filter `ScoreMelodyPatternSearch.ASearchSeesTheEditsMadeBeforeIt` alone (under a wider filter the static input refers to the parts of a score an earlier test destroyed, and the run can crash instead) → `ASearchSeesTheEditsMadeBeforeIt` fails (1 pattern after the edit). (g) In the binding replace `{"patternPitches", MelodyDataFrame::Kind::List}` with `{"patternPitches", MelodyDataFrame::Kind::Text}` → `test_equal_windows_are_one_pattern_and_rhythm_tells_them_apart` fails (the lists become strings); in `py_melody_dataframe.h` replace `dtype = py::str("int64");` with `dtype = py::str("object");` → `test_no_pattern_kept_gives_an_empty_dataframe_with_every_column` fails (`patternIdx` is `object`, not `int64`). (h) In the numpydoc replace `2,164 patterns and 489,196 rows` with `2,164 patterns` → `test_the_docstring_gives_the_size_at_the_defaults` fails. (i) Replace the binding's `py::arg("intervalSimilarityThreshold") = 1.0f` with `= 0.5f` → `test_the_defaults_keep_exact_repetitions_transposed_or_not` fails (the `G#4` variant joins pattern 0); `py::arg("minOccurrences") = 2` with `= 1` → it fails (every window is kept); `py::arg("patternNumNotes") = 5` with `= 4` → it fails. In C++ replace the declaration's `const int minOccurrences = 2` with `= 1` → `FindAnyMelodyPatternKeepsPatternsThatOccurAtLeastMinOccurrences` fails. Revert each; rerun green.

- [ ] **Step 11: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (Task 4's + 4); «build» `make "PYTHON=$py" py-tests` → OK (Task 4's + 6); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 12: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/include/maiacore/score.h maiacore/src/maiacore/score.cpp maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/score-test.cpp test/test_score_comprehensive.py test/xml_examples/unit_test/melody_duplicate_patterns.musicxml test/xml_examples/unit_test/melody_patterns_quarter_tone_apart.xml test/musicxml/ledger.json`, message:

```
feat: findAnyMelodyPattern keeps one window per pattern, in Python too

Every window of the melodic lines is a pattern; equal windows (the same
exact positions relative to their first sounding note and the same
durations) are one pattern, the first kept. The old deduplication kept
the k-1 later copies of k equal patterns and merged [q, q] with [h, h].
A pattern is kept when it has at least minOccurrences matches (default
2); at the default thresholds of 1 a match is an exact repetition,
transposed or not. patternNumNotes below 2 and minOccurrences below 1
raise; patternNumNotes 0 divided by zero. The search builds its lines at
each call, so the note-event cache, which an edit left dangling, is gone
and concurrent searches are safe. Score.findAnyMelodyPatternDataFrame()
is new.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 6: `ScoreCollection`: construction, discovery, edits and searches

**Files:**
- Modify (whole files): `maiacore/include/maiacore/score_collection.h`, `maiacore/src/maiacore/score_collection.cpp`, `maiacore/src/maiacore/python_wrapper/py_score_collection.cpp`
- Test: `tests-cpp/src/score-collection-test.cpp` (whole file: the `EXPECT_GE(size_t, 0)` tests become exact ones on the small fixtures), `test/test_score_collection.py` (new)

**Interfaces:** consumes `Score::MelodyPatternRow`/`MelodyPatternTable` (Task 4), `maiacore::detail::requireTwoNotes` (`melodic-lines.h`, Task 4), `MelodyDataFrame` (Task 4) and the fixtures `melody_last_window.musicxml` (Task 4) and `melody_duplicate_patterns.musicxml` (Task 5). Produces:
`struct ScoreCollection::MelodyPatternRow { std::string fileName; std::string composerName; std::string scoreTitle; Score::MelodyPatternRow match; };`, `typedef std::vector<MelodyPatternRow> ScoreCollection::MelodyPatternTable;`,
`ScoreCollection();`, `explicit ScoreCollection(const std::string& directoryPath, const bool recursive = false);`, `explicit ScoreCollection(const std::vector<std::string>& directoriesPaths, const bool recursive = false);`, `void setDirectoriesPaths(const std::vector<std::string>& directoriesPaths, const bool recursive = false);`, `void removeScore(const int scoreIdx);` (throws `std::out_of_range`),
`MelodyPatternTable findMelodyPattern(const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold = 0.5f, const float rhythmSimilarityThreshold = 0.5f, <five callbacks by const reference>) const;`, `std::vector<MelodyPatternTable> findMelodyPattern(const std::vector<std::vector<Note>>& melodyPatterns, <same>) const;` (one table per pattern);
Python `ScoreCollection()`, `ScoreCollection(directoryPath, recursive=False)`, `ScoreCollection(directoriesPaths, recursive=False)`, `setDirectoriesPaths(directoriesPaths, recursive=False)`, `removeScore(scoreIdx)` (IndexError), `findMelodyPatternDataFrame(melodyPattern | melodyPatterns, intervalSimilarityThreshold=0.5, rhythmSimilarityThreshold=0.5, …)`. The private typedefs `ExtendedMelodyPatternRow`, `ExtendedMultiMelodyPatternRow` and their tables are removed.

- [ ] **Step 1: Write the failing C++ tests** — replace the content of `tests-cpp/src/score-collection-test.cpp` with:

```cpp
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/score_collection.h"
#include "test-capture.h"

// Test directories with XML files
const std::string BACH_DIR = "./test/xml_examples/Bach";
const std::string BEETHOVEN_DIR = "./test/xml_examples/Beethoven";
const std::string UNIT_TEST_DIR = "./test/xml_examples/unit_test";

// ============================================================================
// Constructor Tests
// ============================================================================

namespace {
const std::string LAST_WINDOW = "./test/xml_examples/unit_test/melody_last_window.musicxml";
const std::string DUPLICATES = "./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml";

// A directory of its own under the system's temporary directory, removed with its contents when
// the object is destroyed.
class TemporaryDirectory {
   public:
    TemporaryDirectory()
        : _path(std::filesystem::temp_directory_path() /
                ("maialib-collection-test-" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))) {
        std::filesystem::create_directories(_path);
    }
    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    const std::filesystem::path& path() const { return _path; }

    // Copies a score into the directory as 'name', which may name a subdirectory.
    void addCopy(const std::string& source, const std::string& name) const {
        std::filesystem::create_directories((_path / name).parent_path());
        std::filesystem::copy_file(source, _path / name);
    }

   private:
    std::filesystem::path _path;
};

// The file names of a collection's scores, in collection order.
std::vector<std::string> fileNamesOf(const ScoreCollection& collection) {
    std::vector<std::string> names;
    for (const Score& score : collection.getScores()) {
        names.push_back(score.getFileName());
    }
    return names;
}

// The file name and the written pitches of each row of a table.
std::vector<std::pair<std::string, std::vector<std::string>>> rowsOf(
    const ScoreCollection::MelodyPatternTable& table) {
    std::vector<std::pair<std::string, std::vector<std::string>>> rows;
    for (const ScoreCollection::MelodyPatternRow& row : table) {
        rows.emplace_back(row.fileName, row.match.writtenPitches);
    }
    return rows;
}
}  // namespace

TEST(ScoreCollectionConstructor, DefaultConstructor) {
    ScoreCollection collection;
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_EQ(collection.getNumDirectories(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, SingleDirectoryConstructor) {
    ScoreCollection collection(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);
    EXPECT_GT(collection.getNumScores(), 0);  // Should load Bach files
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, MultipleDirectoriesConstructor) {
    std::vector<std::string> dirs = {BACH_DIR, BEETHOVEN_DIR};
    ScoreCollection collection(dirs);

    EXPECT_EQ(collection.getNumDirectories(), 2);
    EXPECT_GT(collection.getNumScores(), 0);  // Should load from both directories
}

TEST(ScoreCollectionConstructor, EmptyDirectoryPath) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

// A path that does not exist, or that is not a directory, raises a std::runtime_error whose
// message, in English, names it.
TEST(ScoreCollectionConstructor, APathThatIsNotADirectoryRaises) {
    for (const std::string& path :
         {std::string("./test/xml_examples/no-such-directory"), LAST_WINDOW, std::string()}) {
        EXPECT_EQ(
            thrownFirstLine([&] { ScoreCollection collection(path); }),
            "[maiacore] ScoreCollection: '" + path + "' is not a directory, or does not exist")
            << path;
    }
    EXPECT_EQ(thrownFirstLine([&] {
                  ScoreCollection collection(std::vector<std::string>{BACH_DIR, "missing"});
              }),
              "[maiacore] ScoreCollection: 'missing' is not a directory, or does not exist");
}

// Extensions match without regard to case, other files are skipped, subdirectories are read only
// when recursive, and the files load in sorted path order.
TEST(ScoreCollectionConstructor, DiscoveryIgnoresCaseSortsAndRecursesOnRequest) {
    TemporaryDirectory directory;
    directory.addCopy(LAST_WINDOW, "c.MusicXML");
    directory.addCopy(LAST_WINDOW, "a.xml");
    directory.addCopy(LAST_WINDOW, "B.XML");
    directory.addCopy(LAST_WINDOW, "notes.txt");
    directory.addCopy(LAST_WINDOW, "sub/d.xml");
    const std::string path = directory.path().string();

    StdoutCapture quiet;
    EXPECT_EQ(fileNamesOf(ScoreCollection(path)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML"}));
    EXPECT_EQ(fileNamesOf(ScoreCollection(path, true)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML", "d.xml"}));
    EXPECT_EQ(fileNamesOf(ScoreCollection(std::vector<std::string>{path}, true)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML", "d.xml"}));
}

TEST(ScoreCollectionConstructor, EmptyDirectoryList) {
    std::vector<std::string> empty_dirs;
    ScoreCollection collection(std::vector<std::string>{});

    EXPECT_EQ(collection.getNumDirectories(), 0);
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

// ============================================================================
// Directory Management Tests
// ============================================================================

TEST(ScoreCollectionDirectories, GetDirectoriesPaths) {
    std::vector<std::string> dirs = {BACH_DIR, BEETHOVEN_DIR};
    ScoreCollection collection(dirs);

    std::vector<std::string> retrieved = collection.getDirectoriesPaths();
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved[0], BACH_DIR);
    EXPECT_EQ(retrieved[1], BEETHOVEN_DIR);
}

TEST(ScoreCollectionDirectories, SetDirectoriesPaths) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumDirectories(), 0);

    std::vector<std::string> dirs = {BACH_DIR};
    collection.setDirectoriesPaths(dirs);

    EXPECT_EQ(collection.getNumDirectories(), 1);
    EXPECT_GT(collection.getNumScores(), 0);  // Should auto-load
}

// setDirectoriesPaths() replaces the directories and the scores, those added with addScore()
// included: the Bach directory's two files, then the Beethoven directory's three.
TEST(ScoreCollectionDirectories, SetDirectoriesReloads) {
    ScoreCollection collection(BACH_DIR);
    collection.addScore(LAST_WINDOW);
    ASSERT_EQ(collection.getNumScores(), 3);

    collection.setDirectoriesPaths({BEETHOVEN_DIR});

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{BEETHOVEN_DIR}));
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"Beethoven_quartet_133.xml", "Beethoven_quartet_Op133.xml",
                                        "Symphony_5th_1Mov.xml"}));
}

// A directory that cannot be loaded leaves the collection as it was.
TEST(ScoreCollectionDirectories, AFailedReloadChangesNothing) {
    ScoreCollection collection(BACH_DIR);

    EXPECT_THROW(collection.setDirectoriesPaths({BEETHOVEN_DIR, "missing"}), std::runtime_error);

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{BACH_DIR}));
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"cello_suite_1_violin.xml", "prelude_1_BWV_846.xml"}));
}

TEST(ScoreCollectionDirectories, AddDirectory) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.addDirectory(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);

    collection.addDirectory(BEETHOVEN_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 2);
}

TEST(ScoreCollectionDirectories, AddDirectoryDoesNotAutoLoad) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addDirectory(BACH_DIR);

    // addDirectory only adds to list, doesn't load files
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionDirectories, GetNumDirectories) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumDirectories(), 0);

    collection.addDirectory(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);

    collection.addDirectory(BEETHOVEN_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 2);
}

// ============================================================================
// Score Management Tests
// ============================================================================

TEST(ScoreCollectionScores, AddScoreByObject) {
    ScoreCollection collection(std::vector<std::string>{});
    Score score("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    collection.addScore(score);
    EXPECT_EQ(collection.getNumScores(), 1);
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionScores, AddScoreByFilePath) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, AddScoreByFilePathList) {
    ScoreCollection collection(std::vector<std::string>{});
    std::vector<std::string> files = {"./test/xml_examples/Bach/prelude_1_BWV_846.xml",
                                      "./test/xml_examples/Bach/cello_suite_1_violin.xml"};

    collection.addScore(files);
    EXPECT_EQ(collection.getNumScores(), 2);
}

TEST(ScoreCollectionScores, AddMultipleScoresByObject) {
    ScoreCollection collection(std::vector<std::string>{});

    Score score1("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    Score score2("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection.addScore(score1);
    collection.addScore(score2);

    EXPECT_EQ(collection.getNumScores(), 2);
}

TEST(ScoreCollectionScores, GetNumScores) {
    ScoreCollection collection(BACH_DIR);
    int num_scores = collection.getNumScores();

    EXPECT_GT(num_scores, 0);  // Bach dir should have at least 1 file
}

TEST(ScoreCollectionScores, GetScoresNonConst) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    std::vector<Score>& scores = collection.getScores();
    EXPECT_EQ(scores.size(), 1);

    // Can modify through reference
    scores.clear();
    EXPECT_EQ(collection.getNumScores(), 0);
}

TEST(ScoreCollectionScores, GetScoresConst) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    const ScoreCollection& const_ref = collection;
    const std::vector<Score>& scores = const_ref.getScores();

    EXPECT_EQ(scores.size(), 1);
}

TEST(ScoreCollectionScores, IsEmpty) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_TRUE(collection.isEmpty());

    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionScores, Clear) {
    ScoreCollection collection(BACH_DIR);
    EXPECT_GT(collection.getNumScores(), 0);

    collection.clear();
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionScores, ClearDoesNotAffectDirectories) {
    ScoreCollection collection(BACH_DIR);
    int num_dirs = collection.getNumDirectories();

    collection.clear();

    EXPECT_EQ(collection.getNumDirectories(), num_dirs);  // Directories unchanged
}

TEST(ScoreCollectionScores, RemoveScoreByIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    EXPECT_EQ(collection.getNumScores(), 2);

    collection.removeScore(0);
    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, RemoveScoreLastIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection.removeScore(1);  // Remove last
    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, RemoveScoreInvalidIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    EXPECT_THROW(collection.removeScore(10), std::out_of_range);
    EXPECT_THROW(collection.removeScore(1), std::out_of_range);
    EXPECT_THROW(collection.removeScore(-1), std::out_of_range);
    EXPECT_EQ(collection.getNumScores(), 1);  // Unchanged
}

// ============================================================================
// Merge Tests
// ============================================================================

TEST(ScoreCollectionMerge, MergeCollections) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumScores(), 2);
}

TEST(ScoreCollectionMerge, MergeDirectories) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addDirectory(BACH_DIR);

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addDirectory(BEETHOVEN_DIR);

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumDirectories(), 2);
}

TEST(ScoreCollectionMerge, MergeBothScoresAndDirectories) {
    ScoreCollection collection1(BACH_DIR);
    int scores1 = collection1.getNumScores();
    int dirs1 = collection1.getNumDirectories();

    ScoreCollection collection2(BEETHOVEN_DIR);
    int scores2 = collection2.getNumScores();
    int dirs2 = collection2.getNumDirectories();

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumScores(), scores1 + scores2);
    EXPECT_EQ(collection1.getNumDirectories(), dirs1 + dirs2);
}

TEST(ScoreCollectionMerge, MergeEmptyCollection) {
    ScoreCollection collection1(BACH_DIR);
    int original_count = collection1.getNumScores();

    ScoreCollection empty_collection(std::vector<std::string>{});
    collection1.merge(empty_collection);

    EXPECT_EQ(collection1.getNumScores(), original_count);  // Unchanged
}

TEST(ScoreCollectionMerge, MergeIntoEmptyCollection) {
    ScoreCollection empty_collection(std::vector<std::string>{});
    ScoreCollection collection(BACH_DIR);
    int count = collection.getNumScores();

    empty_collection.merge(collection);

    EXPECT_EQ(empty_collection.getNumScores(), count);
}

// ============================================================================
// Operator Tests
// ============================================================================

TEST(ScoreCollectionOperator, PlusOperator) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    ScoreCollection merged = collection1 + collection2;

    EXPECT_EQ(merged.getNumScores(), 2);
    // Original collections should be unchanged
    EXPECT_EQ(collection1.getNumScores(), 1);
    EXPECT_EQ(collection2.getNumScores(), 1);
}

TEST(ScoreCollectionOperator, PlusOperatorChaining) {
    ScoreCollection c1(std::vector<std::string>{});
    c1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection c2(std::vector<std::string>{});
    c2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    ScoreCollection c3(std::vector<std::string>{});
    c3.addScore("./test/xml_examples/Beethoven/Beethoven_quartet_133.xml");

    ScoreCollection merged = c1 + c2 + c3;

    EXPECT_EQ(merged.getNumScores(), 3);
}

// ============================================================================
// Pattern Finding Tests - Single Pattern
// ============================================================================

// Each score's matches, with its file name; scores with equal titles keep the collection's order.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternBasic) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);

    const auto table = collection.findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f);

    EXPECT_EQ(rowsOf(table), (std::vector<std::pair<std::string, std::vector<std::string>>>{
                                 {"melody_last_window.musicxml", {"C4", "D4"}},
                                 {"melody_last_window.musicxml", {"D4", "E4"}},
                                 {"melody_duplicate_patterns.musicxml", {"C4", "D4"}},
                                 {"melody_duplicate_patterns.musicxml", {"C4", "D4"}},
                                 {"melody_duplicate_patterns.musicxml", {"E4", "F#4"}}}));
}

// The rows are sorted stably by score title.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternSortsByScoreTitle) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);
    collection.getScores()[0].setTitle("B");
    collection.getScores()[1].setTitle("A");

    const auto table = collection.findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f);

    std::vector<std::string> titles;
    for (const auto& row : table) {
        titles.push_back(row.scoreTitle);
    }
    EXPECT_EQ(titles, (std::vector<std::string>{"A", "A", "A", "B", "B"}));
}

// Thresholds of 1 keep only exact matches: D4-E4 in a quarter and a half is no C4-D4 in quarters.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternHighThresholds) {
    ScoreCollection collection;
    collection.addScore(DUPLICATES);

    EXPECT_EQ(collection.findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f).size(), 3u);
    EXPECT_EQ(collection.findMelodyPattern({Note("C4"), Note("D4")}, 0.5f, 0.5f).size(), 4u);
}

// An empty collection finds nothing, but a pattern of fewer than 2 notes is rejected all the
// same.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternEmptyCollection) {
    ScoreCollection collection;

    EXPECT_TRUE(collection.findMelodyPattern({Note("C4"), Note("D4")}).empty());
    EXPECT_EQ(thrownFirstLine([&] { collection.findMelodyPattern(std::vector<Note>{Note("C4")}); }),
              "[maiacore] ScoreCollection::findMelodyPattern: a melody pattern needs at least 2 "
              "notes, and this one has 1");
}

// ============================================================================
// Pattern Finding Tests - Multiple Patterns
// ============================================================================

// One table per pattern, each with the pattern's matches in every score.
TEST(ScoreCollectionMultiPattern, FindMultipleMelodyPatterns) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);
    const std::vector<std::vector<Note>> patterns = {
        {Note("C4"), Note("D4")}, {Note("E4"), Note("G4")}, {Note("C4"), Note("B3"), Note("A3")}};

    const auto tables = collection.findMelodyPattern(patterns, 1.0f, 1.0f);

    ASSERT_EQ(tables.size(), 3u);
    EXPECT_EQ(tables[0].size(), 5u);
    EXPECT_EQ(rowsOf(tables[1]), (std::vector<std::pair<std::string, std::vector<std::string>>>{
                                     {"melody_last_window.musicxml", {"E4", "G4"}}}));
    EXPECT_TRUE(tables[2].empty());
}

// Each table holds what the single-pattern search of its pattern finds, in the same order.
TEST(ScoreCollectionMultiPattern, EachTableIsTheSinglePatternSearch) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);
    collection.getScores()[0].setTitle("B");
    collection.getScores()[1].setTitle("A");
    const std::vector<std::vector<Note>> patterns = {{Note("C4"), Note("D4")},
                                                     {Note("D4"), Note("C4")}};

    const auto tables = collection.findMelodyPattern(patterns, 0.5f, 0.5f);

    ASSERT_EQ(tables.size(), 2u);
    for (size_t p = 0; p < patterns.size(); p++) {
        EXPECT_EQ(rowsOf(tables[p]), rowsOf(collection.findMelodyPattern(patterns[p], 0.5f, 0.5f)))
            << p;
    }
}

// A pattern of fewer than 2 notes is rejected, even in an empty collection.
TEST(ScoreCollectionMultiPattern, AShortPatternIsRejected) {
    ScoreCollection collection;
    EXPECT_THROW(collection.findMelodyPattern(std::vector<std::vector<Note>>{{Note("C4")}}),
                 std::runtime_error);
}

TEST(ScoreCollectionMultiPattern, EmptyPatternList) {
    ScoreCollection collection(BACH_DIR);

    std::vector<std::vector<Note>> empty_patterns;

    auto results = collection.findMelodyPattern(empty_patterns);

    EXPECT_EQ(results.size(), 0);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(ScoreCollectionIntegration, FullWorkflow) {
    // Create collection with directory
    ScoreCollection collection(BACH_DIR);
    int initial_count = collection.getNumScores();
    EXPECT_GT(initial_count, 0);

    // Add more scores manually
    collection.addScore("./test/xml_examples/Beethoven/Beethoven_quartet_133.xml");
    EXPECT_EQ(collection.getNumScores(), initial_count + 1);

    // Remove a score
    collection.removeScore(0);
    EXPECT_EQ(collection.getNumScores(), initial_count);

    // Clear all
    collection.clear();
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionIntegration, MergeAndSearch) {
    ScoreCollection collection1(BACH_DIR);
    ScoreCollection collection2(BEETHOVEN_DIR);

    ScoreCollection merged = collection1 + collection2;

    EXPECT_GT(merged.getNumScores(), 0);
    EXPECT_EQ(merged.getNumDirectories(), 2);

    // A merged collection searches the scores of both
    ScoreCollection first;
    first.addScore(LAST_WINDOW);
    ScoreCollection second;
    second.addScore(DUPLICATES);
    const auto table = (first + second).findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f);
    EXPECT_EQ(table.size(), 5u);
}

TEST(ScoreCollectionIntegration, SetDirectoriesMultipleTimes) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.setDirectoriesPaths({BACH_DIR});
    EXPECT_EQ(collection.getNumScores(), 2);

    collection.setDirectoriesPaths({BEETHOVEN_DIR});
    EXPECT_EQ(collection.getNumScores(), 3);

    collection.setDirectoriesPaths({BEETHOVEN_DIR});
    EXPECT_EQ(collection.getNumScores(), 3);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ScoreCollectionEdgeCases, RemoveFromEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});

    EXPECT_THROW(collection.removeScore(0), std::out_of_range);
    EXPECT_EQ(collection.getNumScores(), 0);
}

TEST(ScoreCollectionEdgeCases, ClearEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.clear();  // Should not crash
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionEdgeCases, MergeWithSelf) {
    ScoreCollection collection(BACH_DIR);
    int original_count = collection.getNumScores();

    collection.merge(collection);  // Merge with itself

    EXPECT_EQ(collection.getNumScores(), original_count * 2);  // Should duplicate
}

TEST(ScoreCollectionEdgeCases, LargeCollection) {
    ScoreCollection collection(UNIT_TEST_DIR);

    // Unit test dir has many files - test performance doesn't degrade
    EXPECT_GT(collection.getNumScores(), 10);

    // Clear should work efficiently
    collection.clear();
    EXPECT_TRUE(collection.isEmpty());
}
```

  (Against the current file: the `EXPECT_GE(results.size(), 0)` search tests on the Bach and Beethoven directories, which cannot fail, are replaced by exact tests on the two small fixtures; `FindMelodyPatternMultipleScores`, `MultiplePatternsSingleScore` and `MultiplePatternsMultipleScores` are removed, their roles taken by `FindMelodyPatternBasic`, `FindMultipleMelodyPatterns` and `EachTableIsTheSinglePatternSearch`; `RemoveScoreInvalidIndex`, `RemoveFromEmptyCollection`, `SetDirectoriesReloads`, `SetDirectoriesMultipleTimes`, `FindMelodyPatternEmptyCollection` and `MergeAndSearch` assert the new rules exactly.)

- [ ] **Step 2: Write the failing Python tests** — create `test/test_score_collection.py`:

```python
"""ScoreCollection: construction, directory discovery, edits and the melody search's DataFrames."""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

import maialib as ml

HERE = os.path.dirname(os.path.abspath(__file__))
UNIT_TEST = os.path.join(HERE, "xml_examples", "unit_test")
LAST_WINDOW = os.path.join(UNIT_TEST, "melody_last_window.musicxml")
DUPLICATES = os.path.join(UNIT_TEST, "melody_duplicate_patterns.musicxml")
BACH = os.path.join(HERE, "xml_examples", "Bach")
BEETHOVEN = os.path.join(HERE, "xml_examples", "Beethoven")

MATCH_COLUMNS = [
    "partName",
    "measure",
    "staff",
    "voice",
    "writtenKey",
    "concertKey",
    "transposeInterval",
    "transposeSemitones",
    "writtenPitches",
    "soundingPitches",
    "semitonesDiff",
    "rhythmDiff",
    "intervalSimilarity",
    "rhythmSimilarity",
    "totalSimilarity",
]
SCORE_COLUMNS = ["fileName", "composerName", "scoreTitle"]


def fileNames(collection):
    return [score.getFileName() for score in collection.getScores()]


def twoScores():
    """A collection of the two small fixtures, the last-window one titled B, the other A."""
    collection = ml.ScoreCollection()
    collection.addScore(LAST_WINDOW)
    collection.addScore(DUPLICATES)
    collection.getScores()[0].setTitle("B")
    collection.getScores()[1].setTitle("A")
    return collection


class ScoreCollectionConstructionTestCase(unittest.TestCase):
    def test_the_default_constructor_builds_an_empty_collection(self):
        collection = ml.ScoreCollection()
        self.assertEqual(collection.getNumScores(), 0)
        self.assertEqual(collection.getNumDirectories(), 0)

    def test_a_path_that_is_not_a_directory_raises_runtime_error_naming_it(self):
        for path in ("no-such-directory", LAST_WINDOW, ""):
            with self.subTest(path=path), self.assertRaises(RuntimeError) as context:
                ml.ScoreCollection(path)
            self.assertEqual(
                str(context.exception).splitlines()[0],
                f"[maiacore] ScoreCollection: '{path}' is not a directory, or does not exist",
            )

    def test_discovery_ignores_case_sorts_and_recurses_on_request(self):
        with tempfile.TemporaryDirectory() as directory:
            for name in ("c.MusicXML", "a.xml", "B.XML", "notes.txt", os.path.join("sub", "d.xml")):
                os.makedirs(os.path.dirname(os.path.join(directory, name)), exist_ok=True)
                shutil.copyfile(LAST_WINDOW, os.path.join(directory, name))

            self.assertEqual(
                fileNames(ml.ScoreCollection(directory)), ["B.XML", "a.xml", "c.MusicXML"]
            )
            self.assertEqual(
                fileNames(ml.ScoreCollection(directory, recursive=True)),
                ["B.XML", "a.xml", "c.MusicXML", "d.xml"],
            )
            collection = ml.ScoreCollection()
            collection.setDirectoriesPaths([directory], recursive=True)
            self.assertEqual(fileNames(collection), ["B.XML", "a.xml", "c.MusicXML", "d.xml"])

    def test_a_directory_named_like_a_score_and_a_non_ascii_name_are_skipped(self):
        with tempfile.TemporaryDirectory() as directory:
            os.makedirs(os.path.join(directory, "old.xml"))
            with open(os.path.join(directory, "notes.ωδή"), "w", encoding="utf-8") as text:
                text.write("not a score")
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))

            self.assertEqual(fileNames(ml.ScoreCollection(directory, recursive=True)), ["a.xml"])

    def test_set_directories_paths_replaces_the_scores(self):
        collection = ml.ScoreCollection(BACH)
        collection.addScore(LAST_WINDOW)
        collection.setDirectoriesPaths([BEETHOVEN])
        collection.setDirectoriesPaths([BEETHOVEN])
        self.assertEqual(
            fileNames(collection),
            ["Beethoven_quartet_133.xml", "Beethoven_quartet_Op133.xml", "Symphony_5th_1Mov.xml"],
        )

    def test_remove_score_outside_the_collection_raises_index_error(self):
        """Run in a child process: an unchecked negative index crashes the interpreter."""
        code = (
            "import maialib as ml\n"
            "c = ml.ScoreCollection()\n"
            f"c.addScore({LAST_WINDOW!r})\n"
            "for i in (-1, 1, 5):\n"
            "    try:\n"
            "        c.removeScore(i)\n"
            "        print('RESULT nothing-raised', i)\n"
            "    except IndexError:\n"
            "        pass\n"
            "print('RESULT', c.getNumScores())\n"
        )
        completed = subprocess.run(
            [sys.executable, "-c", code], capture_output=True, encoding="utf-8", timeout=120
        )
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, ["RESULT 1"])


class ScoreCollectionMelodySearchTestCase(unittest.TestCase):
    def test_the_columns_start_with_the_score(self):
        table = twoScores().findMelodyPatternDataFrame([ml.Note("C4"), ml.Note("D4")], 1.0, 1.0)
        self.assertEqual(list(table.columns), SCORE_COLUMNS + MATCH_COLUMNS)
        self.assertEqual(list(table["scoreTitle"]), ["A", "A", "A", "B", "B"])
        self.assertEqual(list(table.index), [0, 1, 2, 3, 4])

    def test_the_list_overload_adds_the_pattern_index_first(self):
        patterns = [[ml.Note("C4"), ml.Note("D4")], [ml.Note("E4"), ml.Note("G4")]]
        table = twoScores().findMelodyPatternDataFrame(patterns, 1.0, 1.0)
        self.assertEqual(list(table.columns), ["patternIdx"] + SCORE_COLUMNS + MATCH_COLUMNS)
        self.assertEqual(
            list(zip(table["scoreTitle"], table["patternIdx"])),
            [("A", 0), ("A", 0), ("A", 0), ("B", 0), ("B", 0), ("B", 1)],
        )

    def test_an_empty_result_is_an_empty_dataframe_with_every_column_and_dtype(self):
        pattern = [ml.Note("C4"), ml.Note("D4")]
        matched = twoScores().findMelodyPatternDataFrame(pattern, 1.0, 1.0)
        listed = twoScores().findMelodyPatternDataFrame([pattern], 1.0, 1.0)
        searches = {
            "empty collection": ml.ScoreCollection().findMelodyPatternDataFrame(pattern),
            "no match": twoScores().findMelodyPatternDataFrame(
                [ml.Note("C4"), ml.Note("C6")], 1.0, 1.0
            ),
        }
        for name, table in searches.items():
            with self.subTest(search=name):
                self.assertEqual(len(table), 0)
                self.assertEqual(list(table.dtypes.items()), list(matched.dtypes.items()))
        empty = ml.ScoreCollection().findMelodyPatternDataFrame([pattern])
        self.assertEqual(len(empty), 0)
        self.assertEqual(list(empty.dtypes.items()), list(listed.dtypes.items()))
        self.assertEqual(str(matched["measure"].dtype), "int64")
        self.assertEqual(str(listed["patternIdx"].dtype), "int64")

    def test_a_pattern_of_fewer_than_two_notes_raises_even_in_an_empty_collection(self):
        with self.assertRaises(RuntimeError):
            ml.ScoreCollection().findMelodyPatternDataFrame([ml.Note("C4")])
        with self.assertRaises(RuntimeError):
            ml.ScoreCollection().findMelodyPatternDataFrame([[ml.Note("C4"), ml.Note("D4")], []])

    def test_the_thresholds_have_the_unified_names(self):
        table = twoScores().findMelodyPatternDataFrame(
            [ml.Note("C4"), ml.Note("D4")],
            intervalSimilarityThreshold=1.0,
            rhythmSimilarityThreshold=1.0,
        )
        self.assertEqual(len(table), 5)
        with self.assertRaises(TypeError):
            twoScores().findMelodyPatternDataFrame(
                [ml.Note("C4"), ml.Note("D4")], totalIntervalsSimilarityThreshold=1.0
            )


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 3: Run them and see them fail.** C++ subset `ScoreCollection*` → the build fails (`call to constructor of 'ScoreCollection' is ambiguous` for `ScoreCollection collection;`, `no type named 'MelodyPatternTable' in 'ScoreCollection'`). «pytest» `test_score_collection` → the constructor tests fail (`UnicodeDecodeError` instead of `RuntimeError` for a missing directory; `[]` from discovery of `B.XML`/`c.MusicXML`; 5 scores after two `setDirectoriesPaths`), the removal test fails (the child dies on `removeScore(-1)`), the search tests fail (`KeyError: 'scoreTitle'`, old columns).

- [ ] **Step 4: The header** — replace the content of `maiacore/include/maiacore/score_collection.h` with:

```cpp
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "maiacore/score.h"

/**
 * @brief Represents a collection of musical scores, supporting batch analysis and management.
 *
 * The ScoreCollection class provides methods for loading, managing, and analyzing multiple Score
 * objects. It is designed for large-scale musicological research, corpus studies, and batch
 * processing of MusicXML files.
 */
class ScoreCollection {
   public:
    /**
     * @brief One match of a melodic pattern in a score of the collection.
     */
    struct MelodyPatternRow {
        std::string fileName;           ///< The score's file name (Score::getFileName()).
        std::string composerName;       ///< The score's composer (Score::getComposerName()).
        std::string scoreTitle;         ///< The score's title (Score::getTitle()).
        Score::MelodyPatternRow match;  ///< The match, as Score::findMelodyPattern() gives it.
    };

    /**
     * @brief The matches of one pattern in every score, sorted stably by score title: the matches
     *        of one score keep the order Score::findMelodyPattern() gives them, and scores with
     *        the same title keep the collection's order.
     */
    typedef std::vector<MelodyPatternRow> MelodyPatternTable;

    /**
     * @brief Constructs an empty collection: no directory and no score.
     */
    ScoreCollection();

    /**
     * @brief Constructs a collection of the MusicXML files of a directory.
     * @param directoryPath A directory; its files are loaded as setDirectoriesPaths() loads them.
     * @param recursive True to load the files of its subdirectories, at any depth, too.
     * @throws std::runtime_error If the path is not a directory, as setDirectoriesPaths() throws.
     */
    explicit ScoreCollection(const std::string& directoryPath, const bool recursive = false);

    /**
     * @brief Constructs a collection of the MusicXML files of several directories.
     * @param directoriesPaths The directories; their files are loaded as setDirectoriesPaths()
     *        loads them.
     * @param recursive True to load the files of their subdirectories, at any depth, too.
     * @throws std::runtime_error If a path is not a directory, as setDirectoriesPaths() throws.
     */
    explicit ScoreCollection(const std::vector<std::string>& directoriesPaths,
                             const bool recursive = false);

    /**
     * @brief Returns the list of directory paths associated with the collection.
     * @return Vector of directory path strings.
     */
    std::vector<std::string> getDirectoriesPaths() const;

    /**
     * @brief Replaces the collection's directories and scores with those of the given
     *        directories.
     * @details Loads every file whose extension is `.xml`, `.mxl` or `.musicxml`, compared
     *          without regard to case, directory by directory in the given order and, within a
     *          directory, in sorted path order. Every path is checked before anything is loaded,
     *          and the collection changes only when every file has loaded.
     * @param directoriesPaths The directories; an empty list empties the collection.
     * @param recursive True to load the files of their subdirectories, at any depth, too.
     * @throws std::runtime_error If a path does not exist, is not a directory or cannot be read;
     *         the message names it. A file that fails to load raises its own error.
     */
    void setDirectoriesPaths(const std::vector<std::string>& directoriesPaths,
                             const bool recursive = false);

    /**
     * @brief Adds a directory path to the collection (does not reload files automatically).
     * @param directoryPath Directory path string.
     */
    void addDirectory(const std::string& directoryPath);

    /**
     * @brief Adds a Score object to the collection.
     * @param score Score object to add.
     */
    void addScore(const Score& score);

    /**
     * @brief Loads a Score from a file path and adds it to the collection.
     * @param filePath Path to a MusicXML file.
     */
    void addScore(const std::string& filePath);

    /**
     * @brief Loads multiple Scores from file paths and adds them to the collection.
     * @param filePaths Vector of MusicXML file paths.
     */
    void addScore(const std::vector<std::string>& filePaths);

    /**
     * @brief Removes all scores from the collection; its directories are kept.
     */
    void clear();

    /**
     * @brief Returns the number of directories in the collection.
     * @return Number of directories.
     */
    int getNumDirectories() const;

    /**
     * @brief Returns the number of scores in the collection.
     * @return Number of Score objects.
     */
    int getNumScores() const;

    /**
     * @brief Returns a reference to the vector of Score objects (modifiable).
     * @return Reference to vector of Score.
     */
    std::vector<Score>& getScores();

    /**
     * @brief Returns a const reference to the vector of Score objects.
     * @return Const reference to vector of Score.
     */
    const std::vector<Score>& getScores() const;

    /**
     * @brief Returns true if the collection contains no scores.
     * @return True if empty.
     */
    bool isEmpty() const;

    /**
     * @brief Merges another ScoreCollection into this one, combining directories and scores.
     * @param other Another ScoreCollection.
     */
    void merge(const ScoreCollection& other);

    /**
     * @brief Removes a score from the collection by its index.
     * @param scoreIdx Index of the score to remove, from 0 to getNumScores() - 1.
     * @throws std::out_of_range If scoreIdx is negative or not below getNumScores(); the
     *         collection is unchanged.
     */
    void removeScore(const int scoreIdx);

    /**
     * @brief Searches every score of the collection for a melodic pattern.
     * @details Each score is searched as Score::findMelodyPattern() searches it.
     * @param melodyPattern The pattern: at least 2 notes.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match.
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match.
     * @param intervalsSimilarityCallback See Score::findMelodyPattern().
     * @param rhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalIntervalSimilarityCallback See Score::findMelodyPattern().
     * @param totalRhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalSimilarityCallback See Score::findMelodyPattern().
     * @return The matches in every score, sorted stably by score title.
     * @throws std::runtime_error If the pattern has fewer than 2 notes, even for an empty
     *         collection, or as Score::findMelodyPattern() throws.
     */
    MelodyPatternTable findMelodyPattern(
        const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold = 0.5f,
        const float rhythmSimilarityThreshold = 0.5f,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    /**
     * @brief Searches every score of the collection for several melodic patterns.
     * @details Each score is searched as the list overload of Score::findMelodyPattern()
     *          searches it.
     * @param melodyPatterns The patterns.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match.
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match.
     * @param intervalsSimilarityCallback See Score::findMelodyPattern().
     * @param rhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalIntervalSimilarityCallback See Score::findMelodyPattern().
     * @param totalRhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalSimilarityCallback See Score::findMelodyPattern().
     * @return One table per pattern, in pattern order, each with the pattern's matches in every
     *         score, sorted stably by score title.
     * @throws std::runtime_error If a pattern has fewer than 2 notes, even for an empty
     *         collection, or as Score::findMelodyPattern() throws.
     */
    std::vector<MelodyPatternTable> findMelodyPattern(
        const std::vector<std::vector<Note>>& melodyPatterns,
        const float intervalSimilarityThreshold = 0.5f,
        const float rhythmSimilarityThreshold = 0.5f,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    /**
     * @brief Merges two ScoreCollections using the + operator.
     * @param other Another ScoreCollection.
     * @return New ScoreCollection containing all scores and directories from both.
     */
    ScoreCollection operator+(const ScoreCollection& other) const {
        ScoreCollection sc = *this;
        sc.merge(other);
        return sc;
    }

   private:
    std::vector<std::string> _directoriesPaths;  ///< List of directories containing score files.
    std::vector<Score> _scores;                  ///< Vector of loaded Score objects.
};
```

- [ ] **Step 5: The implementation** — replace the content of `maiacore/src/maiacore/score_collection.cpp` with (this also drops Task 4's adaptation):

```cpp
#include "maiacore/score_collection.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "maiacore/log.h"
#include "melodic-lines.h"

using maiacore::detail::requireTwoNotes;

namespace {
// Whether the file's extension is .xml, .mxl or .musicxml, in any case. The extension is read as
// UTF-8, which never fails, whatever characters the rest of the name holds.
bool isMusicXMLFile(const std::filesystem::path& path) {
    std::string extension = path.extension().u8string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".xml" || extension == ".mxl" || extension == ".musicxml";
}

// The MusicXML files of 'directory' (and of its subdirectories at any depth when 'recursive'),
// in sorted path order. Every failure is reported in English, naming the path: the messages of
// std::filesystem's own exceptions are localised, and Python cannot always decode them.
template <typename Iterator>
std::vector<std::filesystem::path> musicXMLFilesOf(const std::string& directory) {
    std::vector<std::filesystem::path> files;
    std::error_code error;
    for (Iterator it(directory, error); !error && it != Iterator(); it.increment(error)) {
        std::error_code typeError;
        if (it->is_regular_file(typeError) && isMusicXMLFile(it->path())) {
            files.push_back(it->path());
        }
    }
    if (error) {
        LOG_ERROR("ScoreCollection: cannot read the directory '" + directory + "'");
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::filesystem::path> musicXMLFiles(const std::string& directory,
                                                 const bool recursive) {
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        LOG_ERROR("ScoreCollection: '" + directory + "' is not a directory, or does not exist");
    }
    return recursive ? musicXMLFilesOf<std::filesystem::recursive_directory_iterator>(directory)
                     : musicXMLFilesOf<std::filesystem::directory_iterator>(directory);
}

// The rows of 'table', a search of 'score', with the score's file name, composer and title.
void appendRows(const Score& score, const Score::MelodyPatternTable& table,
                ScoreCollection::MelodyPatternTable* rows) {
    for (const Score::MelodyPatternRow& match : table) {
        rows->push_back({score.getFileName(), score.getComposerName(), score.getTitle(), match});
    }
}

void sortByScoreTitle(ScoreCollection::MelodyPatternTable* rows) {
    std::stable_sort(
        rows->begin(), rows->end(),
        [](const ScoreCollection::MelodyPatternRow& a, const ScoreCollection::MelodyPatternRow& b) {
            return a.scoreTitle < b.scoreTitle;
        });
}
}  // namespace

ScoreCollection::ScoreCollection() = default;

ScoreCollection::ScoreCollection(const std::string& directoryPath, const bool recursive) {
    setDirectoriesPaths({directoryPath}, recursive);
}

ScoreCollection::ScoreCollection(const std::vector<std::string>& directoriesPaths,
                                 const bool recursive) {
    setDirectoriesPaths(directoriesPaths, recursive);
}

std::vector<std::string> ScoreCollection::getDirectoriesPaths() const { return _directoriesPaths; }

void ScoreCollection::setDirectoriesPaths(const std::vector<std::string>& directoriesPaths,
                                          const bool recursive) {
    std::vector<std::vector<std::filesystem::path>> files;
    files.reserve(directoriesPaths.size());
    for (const std::string& directory : directoriesPaths) {
        files.push_back(musicXMLFiles(directory, recursive));
    }

    std::vector<Score> scores;
    for (const auto& directoryFiles : files) {
        for (const std::filesystem::path& file : directoryFiles) {
            LOG_INFO("Loading: " << file.filename().string());
            scores.emplace_back(file.string());
        }
    }
    _directoriesPaths = directoriesPaths;
    _scores = std::move(scores);
}

void ScoreCollection::addDirectory(const std::string& directoryPath) {
    _directoriesPaths.push_back(directoryPath);
}

void ScoreCollection::addScore(const Score& score) { _scores.push_back(score); }

void ScoreCollection::addScore(const std::string& filePath) { addScore(Score(filePath)); }

void ScoreCollection::addScore(const std::vector<std::string>& filePaths) {
    for (const auto& fp : filePaths) {
        addScore(fp);
    }
}

void ScoreCollection::clear() { _scores.clear(); }

int ScoreCollection::getNumDirectories() const {
    return static_cast<int>(_directoriesPaths.size());
}

int ScoreCollection::getNumScores() const { return _scores.size(); }

std::vector<Score>& ScoreCollection::getScores() { return _scores; }

const std::vector<Score>& ScoreCollection::getScores() const { return _scores; }

bool ScoreCollection::isEmpty() const { return _scores.empty(); }

void ScoreCollection::merge(const ScoreCollection& other) {
    // Detect self-merge to avoid iterator invalidation (undefined behavior)
    if (this == &other) {
        ScoreCollection copy = other;  // Make a copy
        merge(copy);                   // Merge with the copy
        return;
    }

    // Merge directories paths
    for (const auto& dir : other.getDirectoriesPaths()) {
        _directoriesPaths.push_back(dir);
    }

    // Merge Score objects
    for (const auto& sc : other.getScores()) {
        _scores.push_back(sc);
    }
}

void ScoreCollection::removeScore(const int scoreIdx) {
    if (scoreIdx < 0 || scoreIdx >= static_cast<int>(_scores.size())) {
        throw std::out_of_range("ScoreCollection::removeScore: index " + std::to_string(scoreIdx) +
                                " is outside the collection of " + std::to_string(_scores.size()) +
                                " scores");
    }

    _scores.erase(_scores.begin() + scoreIdx);
}

ScoreCollection::MelodyPatternTable ScoreCollection::findMelodyPattern(
    const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
    const std::function<float(float, float)>& totalSimilarityCallback) const {
    // Checked here as well, so that an empty collection rejects the pattern as a full one does.
    requireTwoNotes("ScoreCollection::findMelodyPattern", melodyPattern.size());
    MelodyPatternTable rows;
    for (const Score& score : _scores) {
        appendRows(
            score,
            score.findMelodyPattern(melodyPattern, intervalSimilarityThreshold,
                                    rhythmSimilarityThreshold, intervalsSimilarityCallback,
                                    rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                                    totalRhythmSimilarityCallback, totalSimilarityCallback),
            &rows);
    }
    sortByScoreTitle(&rows);
    return rows;
}

std::vector<ScoreCollection::MelodyPatternTable> ScoreCollection::findMelodyPattern(
    const std::vector<std::vector<Note>>& melodyPatterns, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
    const std::function<float(float, float)>& totalSimilarityCallback) const {
    for (const std::vector<Note>& pattern : melodyPatterns) {
        requireTwoNotes("ScoreCollection::findMelodyPattern", pattern.size());
    }
    std::vector<MelodyPatternTable> tables(melodyPatterns.size());
    for (const Score& score : _scores) {
        const std::vector<Score::MelodyPatternTable> scoreTables = score.findMelodyPattern(
            melodyPatterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
            intervalsSimilarityCallback, rhythmSimilarityCallback, totalIntervalSimilarityCallback,
            totalRhythmSimilarityCallback, totalSimilarityCallback);
        for (size_t p = 0; p < scoreTables.size(); p++) {
            appendRows(score, scoreTables[p], &tables[p]);
        }
    }
    for (MelodyPatternTable& table : tables) {
        sortByScoreTitle(&table);
    }
    return tables;
}
```

- [ ] **Step 6: The bindings** — replace the content of `maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` with:

```cpp
#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/score_collection.h"
#include "py_melody_dataframe.h"

namespace py = pybind11;
using namespace pybind11::literals;
using maiacore_python::MelodyDataFrame;

void ScoreCollectionClass(const py::module& m) {
    m.doc() = "ScoreCollection class binding";

    // bindings to ScoreCollection class
    py::class_<ScoreCollection> cls(m, "ScoreCollection");
    cls.def(py::init<>(), R"pbdoc(
        Create an empty collection: no directory and no score.
    )pbdoc");

    cls.def(py::init<const std::string&, const bool>(), py::arg("directoryPath"),
            py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Create a collection of the MusicXML files of a directory, as ``setDirectoriesPaths``
        loads them.

        Parameters
        ----------
        directoryPath : str
            The directory.
        recursive : bool, default False
            True to load the files of its subdirectories, at any depth, too.

        Raises
        ------
        RuntimeError
            If the path does not exist, is not a directory or cannot be read; the message names
            it.
    )pbdoc");

    cls.def(py::init<const std::vector<std::string>&, const bool>(), py::arg("directoriesPaths"),
            py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Create a collection of the MusicXML files of several directories, as
        ``setDirectoriesPaths`` loads them.

        Parameters
        ----------
        directoriesPaths : list of str
            The directories; ``[]`` gives an empty collection.
        recursive : bool, default False
            True to load the files of their subdirectories, at any depth, too.

        Raises
        ------
        RuntimeError
            If a path does not exist, is not a directory or cannot be read; the message names it.
    )pbdoc");

    cls.def("getDirectoriesPaths", &ScoreCollection::getDirectoriesPaths);
    cls.def("setDirectoriesPaths", &ScoreCollection::setDirectoriesPaths,
            py::arg("directoriesPaths"), py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Replace the collection's directories and scores with those of the given directories.

        Every file whose extension is ``.xml``, ``.mxl`` or ``.musicxml``, compared without
        regard to case, is loaded, directory by directory in the given order and, within a
        directory, in sorted path order; subdirectories only when ``recursive`` is True. Every
        path is checked before anything is loaded, and the collection changes only when every
        file has loaded. Scores added with ``addScore`` are replaced too.

        Parameters
        ----------
        directoriesPaths : list of str
            The directories; ``[]`` empties the collection.
        recursive : bool, default False
            True to load the files of their subdirectories, at any depth, too.

        Raises
        ------
        RuntimeError
            If a path does not exist, is not a directory or cannot be read; the message names
            it. A file that fails to load raises its own error.
    )pbdoc");

    cls.def("addDirectory", &ScoreCollection::addDirectory, py::arg("directoryPath"));

    cls.def("addScore", py::overload_cast<const Score&>(&ScoreCollection::addScore),
            py::arg("score"));
    cls.def("addScore", py::overload_cast<const std::string&>(&ScoreCollection::addScore),
            py::arg("filePath"));
    cls.def("addScore",
            py::overload_cast<const std::vector<std::string>&>(&ScoreCollection::addScore),
            py::arg("filePaths"));

    cls.def("clear", &ScoreCollection::clear);
    cls.def("getNumDirectories", &ScoreCollection::getNumDirectories);
    cls.def("getNumScores", &ScoreCollection::getNumScores);

    cls.def("getScores", py::overload_cast<>(&ScoreCollection::getScores),
            py::return_value_policy::reference_internal);
    cls.def("getScores", py::overload_cast<>(&ScoreCollection::getScores, py::const_),
            py::return_value_policy::reference_internal);

    cls.def("isEmpty", &ScoreCollection::isEmpty);
    cls.def("merge", &ScoreCollection::merge, py::arg("other"));
    cls.def("removeScore", &ScoreCollection::removeScore, py::arg("scoreIdx"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Remove the score at an index of the collection.

        Parameters
        ----------
        scoreIdx : int
            Index of the score, from 0 to ``getNumScores() - 1``.

        Raises
        ------
        IndexError
            If ``scoreIdx`` is negative or not below ``getNumScores()``; the collection is
            unchanged.
    )pbdoc");

    cls.def(
        "findMelodyPatternDataFrame",
        [](const ScoreCollection& collection, const std::vector<Note>& melodyPattern,
           const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
           const std::function<float(float, float)>& totalSimilarityCallback) {
            const ScoreCollection::MelodyPatternTable table = collection.findMelodyPattern(
                melodyPattern, intervalSimilarityThreshold, rhythmSimilarityThreshold,
                intervalsSimilarityCallback, rhythmSimilarityCallback,
                totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                totalSimilarityCallback);
            MelodyDataFrame frame({{"fileName", MelodyDataFrame::Kind::Text},
                                   {"composerName", MelodyDataFrame::Kind::Text},
                                   {"scoreTitle", MelodyDataFrame::Kind::Text}});
            for (const ScoreCollection::MelodyPatternRow& row : table) {
                frame.appendRow(
                    py::list(py::make_tuple(row.fileName, row.composerName, row.scoreTitle)),
                    row.match);
            }
            return frame.build();
        },
        py::arg("melodyPattern"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Search every score of the collection for a melodic pattern.

        Each score is searched as ``Score.findMelodyPatternDataFrame(melodyPattern, ...)``
        searches it, with the same thresholds and callbacks.

        Parameters
        ----------
        melodyPattern : list of Note
            The pattern: at least 2 notes.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of ``Score.findMelodyPatternDataFrame``, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            The matches in every score: ``fileName``, ``composerName`` and ``scoreTitle`` (str),
            followed by ``Score.findMelodyPatternDataFrame``'s columns; sorted stably by
            ``scoreTitle``, each score's matches in the order the score gives them. An empty
            DataFrame, with every column and its dtype, when nothing matches or the collection
            is empty.

        Raises
        ------
        RuntimeError
            If the pattern has fewer than 2 notes, even for an empty collection, or if the
            search of any score raises, for the reasons ``Score.findMelodyPatternDataFrame``
            gives.

        Examples
        --------
        >>> collection = ml.ScoreCollection()
        >>> collection.addScore(ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)))
        >>> table = collection.findMelodyPatternDataFrame([ml.Note("G2"), ml.Note("D3")], 1.0, 1.0)
        >>> list(table.columns[:4])
        ['fileName', 'composerName', 'scoreTitle', 'partName']
    )pbdoc");

    cls.def(
        "findMelodyPatternDataFrame",
        [](const ScoreCollection& collection, const std::vector<std::vector<Note>>& melodyPatterns,
           const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
           const std::function<float(float, float)>& totalSimilarityCallback) {
            // Each score's search runs its patterns on worker threads, and a worker that calls,
            // copies or destroys a Python callback takes the GIL to do it. Holding the GIL here
            // while the workers run would deadlock, so it is released for the search alone, inside
            // this lambda; it is held again when the lambda returns, before the DataFrame is
            // built. The search only reads the collection and its scores, so other threads may
            // search them meanwhile.
            const auto tables = [&] {
                py::gil_scoped_release release;
                return collection.findMelodyPattern(
                    melodyPatterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
                    intervalsSimilarityCallback, rhythmSimilarityCallback,
                    totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                    totalSimilarityCallback);
            }();

            // Every row with its pattern's index, in pattern order, then sorted stably by score
            // title: rows of one title keep the pattern order and each table's order.
            std::vector<std::pair<size_t, const ScoreCollection::MelodyPatternRow*>> rows;
            for (size_t idx = 0; idx < tables.size(); idx++) {
                for (const ScoreCollection::MelodyPatternRow& row : tables[idx]) {
                    rows.emplace_back(idx, &row);
                }
            }
            std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
                return a.second->scoreTitle < b.second->scoreTitle;
            });

            MelodyDataFrame frame({{"patternIdx", MelodyDataFrame::Kind::Integer},
                                   {"fileName", MelodyDataFrame::Kind::Text},
                                   {"composerName", MelodyDataFrame::Kind::Text},
                                   {"scoreTitle", MelodyDataFrame::Kind::Text}});
            for (const auto& [idx, row] : rows) {
                frame.appendRow(py::list(py::make_tuple(idx, row->fileName, row->composerName,
                                                        row->scoreTitle)),
                                row->match);
            }
            return frame.build();
        },
        py::arg("melodyPatterns"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Search every score of the collection for several melodic patterns.

        Each score is searched as ``Score.findMelodyPatternDataFrame(melodyPatterns, ...)``
        searches it: every pattern, each on a worker thread, with the same thresholds and
        callbacks. The search releases the GIL while it runs, so a Python callback is called
        from the worker threads, one call at a time, each taking the GIL, and other Python
        threads run meanwhile. The search only reads the collection and its scores, so any
        number of threads may search them at once; no thread may modify the collection, its
        scores, or their parts, measures or notes while a search of them runs.

        Parameters
        ----------
        melodyPatterns : list of list of Note
            The patterns, each of at least 2 notes.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of ``Score.findMelodyPatternDataFrame``, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            The matches of every pattern in every score: ``patternIdx`` (int, the pattern's
            index in ``melodyPatterns``), ``fileName``, ``composerName`` and ``scoreTitle``
            (str), followed by ``Score.findMelodyPatternDataFrame``'s columns; sorted stably by
            ``scoreTitle``, then ``patternIdx``, each score's matches in the order the score
            gives them. An empty DataFrame, with every column and its dtype, when nothing
            matches or the collection is empty.

        Raises
        ------
        RuntimeError
            If a pattern has fewer than 2 notes, even for an empty collection, or if the search
            for any pattern in any score raises, for the reasons
            ``Score.findMelodyPatternDataFrame`` gives, never leaving an empty result in its
            place.

        Examples
        --------
        >>> collection = ml.ScoreCollection()
        >>> collection.addScore(ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)))
        >>> patterns = [[ml.Note("G2"), ml.Note("D3")], [ml.Note("D3"), ml.Note("B3")]]
        >>> table = collection.findMelodyPatternDataFrame(patterns, 1.0, 1.0)
        >>> sorted(table["patternIdx"].unique().tolist())
        [0, 1]
    )pbdoc");

    // Default Python 'print' function:
    cls.def("__repr__", [](const ScoreCollection& scoreCollection) {
        return "<ScoreCollection - " + std::to_string(scoreCollection.getNumScores()) + " scores>";
    });

    cls.def("__hash__", [](const ScoreCollection& scoreCollection) {
        std::string temp;
        for (const auto& dir : scoreCollection.getDirectoriesPaths()) {
            temp += dir;
        }
        return std::hash<std::string>{}(temp);
    });

    cls.def("__sizeof__",
            [](const ScoreCollection& scoreCollection) { return sizeof(scoreCollection); });

    cls.def(py::self + py::self);
}
```

- [ ] **Step 7: Format, build, pass.** clang-format the four C++ files; `ruff check test/test_score_collection.py` and `ruff format --check test/test_score_collection.py` (from the venv's `Scripts`) → clean. C++ subset `ScoreCollection*` → pass. «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_score_collection test_score_comprehensive` → OK (the callback tests of `test_score_comprehensive` build `ml.ScoreCollection([])` and search it through the list overload).

- [ ] **Step 8: Mutations.** (a) Make the default constructor load the current directory: `ScoreCollection::ScoreCollection() { setDirectoriesPaths({"."}); }` → `DefaultConstructor` and `test_the_default_constructor_builds_an_empty_collection` fail. (b) In `musicXMLFiles` delete the `is_directory` check → `APathThatIsNotADirectoryRaises` and `test_a_path_that_is_not_a_directory_raises_runtime_error_naming_it` fail (the iterator's "cannot read the directory" message, or nothing, instead). (c) In `isMusicXMLFile` delete the `std::transform(...)` → `DiscoveryIgnoresCaseSortsAndRecursesOnRequest` and `test_discovery_ignores_case_sorts_and_recurses_on_request` fail (`B.XML`, `c.MusicXML` skipped); delete `std::sort(files.begin(), files.end());` → both fail (NTFS lists `a.xml` first); in `musicXMLFiles` always use `std::filesystem::directory_iterator` → both fail (`d.xml` missing). (d) Replace `if (it->is_regular_file(typeError) && isMusicXMLFile(it->path())) {` with `if (isMusicXMLFile(it->path())) {` → `test_a_directory_named_like_a_score_and_a_non_ascii_name_are_skipped` fails (the directory `old.xml` is loaded); replace `path.extension().u8string()` with `path.extension().string()` → it fails on Windows (the extension `.ωδή` of `notes.ωδή` is outside the ANSI code page, and `path::string()` throws a system error whose localised message Python cannot decode). (e) Replace `_scores = std::move(scores);` with `_scores.insert(_scores.end(), scores.begin(), scores.end());` → `SetDirectoriesReloads`, `SetDirectoriesMultipleTimes` and `test_set_directories_paths_replaces_the_scores` fail. (f) Move `_directoriesPaths = directoriesPaths;` to the start of `setDirectoriesPaths` → `AFailedReloadChangesNothing` fails. (g) In `removeScore` replace `throw std::out_of_range(` with `throw std::runtime_error(` → `RemoveScoreInvalidIndex`, `RemoveFromEmptyCollection` and `test_remove_score_outside_the_collection_raises_index_error` fail. (h) In `appendRows` pass `score.getTitle()` for the file name → `FindMelodyPatternBasic`, `FindMultipleMelodyPatterns` fail. (i) Delete the `sortByScoreTitle(&rows);` call of the single overload → `FindMelodyPatternSortsByScoreTitle` and `test_the_columns_start_with_the_score` fail; delete the loop that sorts the list overload's tables → `EachTableIsTheSinglePatternSearch` fails; in the list binding delete the `std::stable_sort(...)` → `test_the_list_overload_adds_the_pattern_index_first` fails. (j) In the single overload call `score.findMelodyPattern(melodyPattern)` without the thresholds and callbacks → `FindMelodyPatternHighThresholds` fails (4 rows at 1.0). (k) Delete the single overload's `requireTwoNotes("ScoreCollection::findMelodyPattern", melodyPattern.size());` → `FindMelodyPatternEmptyCollection` and `test_a_pattern_of_fewer_than_two_notes_raises_even_in_an_empty_collection` fail; delete the list overload's loop of `requireTwoNotes` → `AShortPatternIsRejected` fails. (l) Make the list overload return one table per score (`tables` sized and filled per score, as before) → `FindMultipleMelodyPatterns` fails. (m) In the single overload's loop add `break;` after `appendRows(...)` → `MergeAndSearch` fails (2 rows). (n) In `py_melody_dataframe.h` replace `dtype = py::str("int64");` with `dtype = py::str("object");` → `test_an_empty_result_is_an_empty_dataframe_with_every_column_and_dtype` fails. (o) Rename the single binding's `py::arg("intervalSimilarityThreshold")` to `py::arg("totalIntervalsSimilarityThreshold")` → `test_the_thresholds_have_the_unified_names` fails. Revert each; rerun green.

- [ ] **Step 9: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (Task 5's + 3: 6 new, 3 removed); «build» `make "PYTHON=$py" py-tests` → OK (Task 5's + 11); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 10: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/include/maiacore/score_collection.h maiacore/src/maiacore/score_collection.cpp maiacore/src/maiacore/python_wrapper/py_score_collection.cpp tests-cpp/src/score-collection-test.cpp test/test_score_collection.py`, message:

```
fix: ScoreCollection constructs, discovers and replaces its scores

ScoreCollection() builds an empty collection (it was ambiguous in C++
and raised UnicodeDecodeError in Python, like a missing directory, which
now raises RuntimeError naming the path in English). Discovery matches
.xml, .mxl and .musicxml without regard to case, loads in sorted path
order and reads subdirectories when recursive=True. setDirectoriesPaths
replaces the scores, all or nothing, instead of appending them;
removeScore raises IndexError for any index outside the collection, where
-1 crashed. The searches give the unified columns after fileName,
composerName and scoreTitle, sorted stably by scoreTitle, and an empty
DataFrame with every column when nothing matches.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 7: README, notebooks, CHANGELOG, final verification

**Files:**
- Modify: `README.md` (Example 3, ~338-375), `python-tutorial/03_advanced/01_pattern_finding.ipynb` (cell 2), `python-tutorial/find_pattern.ipynb` (cell 4), `CHANGELOG.md` (`[Unreleased]`: line ~14, ~26, ~49; a group in `### Added` before `- Add \`Helper.getLibraryVersion()\``; entries at the end of `### Fix`)

**Interfaces:** consumes everything above.

- [ ] **Step 1: README.** Re-run the example against the packaged sample, from outside the repository: Bash `(cd /c/Users/nyck/AppData/Local/Temp && /c/Users/nyck/AppData/Local/Temp/maialib-5-venv/Scripts/python.exe -c "
import maialib as ml
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Beethoven_Symphony_5th))
pattern = [ml.Note('G4'), ml.Note('F4'), ml.Note('E4'), ml.Note('D4')]
df_patterns = score.findMelodyPatternDataFrame(melodyPattern=pattern, intervalSimilarityThreshold=0.8, rhythmSimilarityThreshold=0.5)
print(f'Found {len(df_patterns)} occurrences of the pattern')
print(df_patterns[['partName', 'measure', 'staff', 'voice', 'transposeInterval', 'totalSimilarity']])
"); echo "exit $?"`. Measured on the finished tree (pandas 3.0.5) it prints:

```
Found 10 occurrences of the pattern
       partName  measure  staff  voice transposeInterval  totalSimilarity
0        Violas      166      0      1           M2 desc              1.0
1  Violoncellos      166      0      1           M2 desc              1.0
2     Violins 1      406      0      1            m3 asc              1.0
3     Violins 2      406      0      1            m3 asc              1.0
4     Violins 1      409      0      1            m7 asc              1.0
5     Violins 2      409      0      1            m7 asc              1.0
6         Flute      452      0      2            P4 asc              1.0
7   Bb Clarinet      452      0      2            P4 asc              1.0
8       Bassoon      452      0      2           P5 desc              1.0
9        Violas      460      0      1           M6 desc              1.0
```

  If it prints anything else, use what it prints and say why in the task report. In `README.md` replace

```python
# Find all occurrences
df_patterns = score.findMelodyPatternDataFrame(
    melodyPattern=pattern,
    totalIntervalsSimilarityThreshold=0.8,
    totalRhythmSimilarityThreshold=0.5
)

print(f"Found {len(df_patterns)} occurrences of the pattern")
print(df_patterns[['partName', 'measureId', 'transposeInterval', 'totalSimilarity']])
```

  with

```python
# Find all occurrences, in every voice of every staff
df_patterns = score.findMelodyPatternDataFrame(
    melodyPattern=pattern,
    intervalSimilarityThreshold=0.8,
    rhythmSimilarityThreshold=0.5
)

print(f"Found {len(df_patterns)} occurrences of the pattern")
print(df_patterns[['partName', 'measure', 'staff', 'voice', 'transposeInterval', 'totalSimilarity']])
```

  and the output block that follows `**Output**` (from `Found 6 occurrences of the pattern` through `5     Violas        460           M6 desc              1.0`) with the ten-row output above.

- [ ] **Step 2: Notebooks.** Both call the unbound `score.findMelodyPattern(...)`. In `python-tutorial/find_pattern.ipynb` and in `python-tutorial/03_advanced/01_pattern_finding.ipynb` replace the source line `    "df = score.findMelodyPattern(melodyPattern)\n",` with `    "df = score.findMelodyPatternDataFrame(melodyPattern)\n",`; in `python-tutorial/03_advanced/01_pattern_finding.ipynb` (one directory deeper) also replace `    "score = ml.Score('../test/xml_examples/Beethoven/Symphony_5th_1Mov.xml')\n",` with `    "score = ml.Score('../../test/xml_examples/Beethoven/Symphony_5th_1Mov.xml')\n",`. Check both cells run: Bash `(cd /c/Users/nyck/Desktop/maialib/python-tutorial && /c/Users/nyck/AppData/Local/Temp/maialib-5-venv/Scripts/python.exe -c "
import json
for path, cell, cwd in (('find_pattern.ipynb', 4, '.'), ('03_advanced/01_pattern_finding.ipynb', 2, '03_advanced')):
    import os
    source = ''.join(json.load(open(path, encoding='utf-8'))['cells'][cell]['source'])
    here = os.getcwd(); os.chdir(cwd); namespace = {}; exec(source, namespace); os.chdir(here)
    print(path, len(namespace['df']), list(namespace['df'].columns[:4]))
"); echo "exit $?"` → both print a row count and `['partName', 'measure', 'staff', 'voice']`, exit 0. Outputs stay unsaved (the notebooks are regenerated at release).

- [ ] **Step 3: CHANGELOG, existing lines.** In `CHANGELOG.md`:
  - line ~14: replace `; and the melody-pattern search (\`Score.findMelodyPatternDataFrame()\`) raises for a pattern, or a segment of the score, that starts on a quarter tone, naming the note and, for a segment, its part, measure and stave.` with `; and the melody-pattern search (\`Score.findMelodyPatternDataFrame()\`) compares quarter tones exactly, a match whose transposition is a quarter tone having an empty \`transposeInterval\` and its exact \`transposeSemitones\`.`
  - line ~26: replace `In Python it changes a score's transpositions in place, which an edit through \`Measure.getNote()\`, a copy, cannot` with `In Python it changes a score's transpositions in place, all or none`
  - line ~49 (the entry that begins ``- `Score.findMelodyPatternDataFrame()` with a single pattern, and the C++ single-pattern``): replace the whole line with

```markdown
- `Score.findMelodyPatternDataFrame()` with a single pattern, and the C++ single-pattern `Score::findMelodyPattern()`, raised `ValueError` (`std::length_error` in C++) for a pattern longer than the first-voice melodies of all the parts together but not longer than the whole score, and `ScoreCollection.findMelodyPatternDataFrame()` with a single pattern passed that error on. A pattern longer than every melodic line now finds no match, and every overload returns an empty result (see the melody-search entries below)
```

- [ ] **Step 4: CHANGELOG, Added.** Before the line that begins ``- Add `Helper.getLibraryVersion()` `` insert:

```markdown
- **Melody search over every melodic line, `findAnyMelodyPatternDataFrame()` and `ScoreCollection` discovery**
  - `Score.findAnyMelodyPatternDataFrame(patternNumNotes=5, intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0, minOccurrences=2, ...)` gives the distinct melodic patterns of a length that have at least `minOccurrences` matches (by default, the patterns that recur exactly, transposed or not), with their matches: the columns of `findMelodyPatternDataFrame()` after `patternIdx` and `patternPitches` (the pattern's written pitches). Windows that are the same notes and rests at the same exact positions relative to their first sounding note, with the same durations, are one pattern, the first kept. In C++, `Score::findAnyMelodyPattern()` returns `std::vector<Score::FoundMelodyPattern>` (`pattern`, `matches`)
  - The melody-search results have the columns `voice` (as written), `concertKey` (the score's concert key at the measure, as `getChords()` computes it), `transposeSemitones` (the exact interval from the pattern's first sounding note to the match's, quarter tones included; `NaN` when either has none) and `soundingPitches`
  - `ScoreCollection()` builds an empty collection; the directory constructors and `setDirectoriesPaths()` take `recursive=False`, which reads subdirectories at any depth when `True`
```

- [ ] **Step 5: CHANGELOG, Fix.** At the end of `### Fix` (after its last line, the `**Breaking:** Exports write \`<transpose>\`` entry, before `### Removed`) insert the entries below; in the first one, use Step 1's measured counts if they differ from "finds 10 where it found 6":

```markdown
- **Breaking:** The melody search (`Score.findMelodyPatternDataFrame()`, `ScoreCollection.findMelodyPatternDataFrame()`, `Score::findMelodyPattern()`) searches every voice of every staff of every part as its own melodic line, where it searched one line per part made of the voice-1 notes of all its staves: the lower staves of the packaged samples (voices 5 and up) and every other voice were never searched, and a window could jump between staves. A chord is one event represented by its highest sounding note (it entered by its first-written note, usually the lowest); a note tied to the previous event at the same pitch extends it (each tied piece was an event); a grace note is not an event. Matches change for every score with chords, ties, several voices or several staves: the README's Beethoven example finds 10 where it found 6
- **Breaking:** The last window of each melodic line is searched: a pattern equal to the end of a melody, or to a whole melody, was never found, and `findAnyMelodyPattern` missed it too
- **Breaking:** The result columns are `partName`, `measure` (was `measureId`), `staff` (was `staveId`), `voice`, `writtenKey` (was `writtenClefKey`, which held the written key), `concertKey`, `transposeInterval`, `transposeSemitones`, `writtenPitches` (was `segmentWrittenPitch`), `soundingPitches`, `semitonesDiff`, `rhythmDiff`, `intervalSimilarity`, `rhythmSimilarity` and `totalSimilarity` (were `totalIntervalSimilarity` and `totalRhythmSimilarity`); the list overloads put `patternIdx` first, and `ScoreCollection` puts `fileName` (was `filename`), `composerName` and `scoreTitle` first. The thresholds are `intervalSimilarityThreshold` and `rhythmSimilarityThreshold` in every overload (`totalIntervalsSimilarityThreshold` and `totalRhythmSimilarityThreshold` in all but one). In C++ the rows are structs (`Score::MelodyPatternRow`, `ScoreCollection::MelodyPatternRow`) instead of tuples, and `ScoreCollection::findMelodyPattern()` with a list returns one table per pattern instead of one per score
- **Breaking:** A search without a match returns an empty DataFrame with every column and its dtype, where `ScoreCollection.findMelodyPatternDataFrame()` raised `KeyError: 'scoreTitle'`, or `ValueError` for an empty collection given a list of patterns. Rows are sorted stably by `scoreTitle` (collections), then `patternIdx` (lists), then `measure`, then part, staff and voice; the list overload of `Score` sorted by `measureId` alone, and not stably
- **Breaking:** `transposeInterval` is computed for matches only and never stops the search: an interval without a name — an augmented ninth such as `C4` to `Cx5` (the Chopin sample with `C4 D4 E4` raised "Unable to compute the interval [C4, Cx5]") or any quarter-tone interval — leaves it empty, and a unison is `"P1"` without a trailing space. The melody search no longer rejects a pattern or a window that starts on a quarter tone; `Interval` and `Chord` still do
- **Breaking:** A melody pattern of fewer than 2 notes raises `RuntimeError` naming the method and the length (an empty pattern returned an empty result, and one note raised an unrelated helper error); `findAnyMelodyPattern(n)` with `n < 2` raises, where `0` crashed the process and a negative `n` raised "vector too long". A pattern longer than every melodic line finds nothing, where one longer than the score's note count raised "The melody pattern is bigger than the score"
- **Breaking:** `Score::findAnyMelodyPattern()` kept, of k equal patterns, the k-1 later ones, and merged patterns whose consecutive durations differed alike (`[q, q]` with `[h, h]`); it keeps the first of each pattern, compares durations exactly, and keeps only the patterns with at least `minOccurrences` matches (a new parameter after the thresholds: default 2, at least 1; the thresholds keep their default of 1.0). Its note-event cache, which an edit of the score left pointing to freed notes, is gone: every search builds its lines from the score as it is, and concurrent searches are safe
- **Breaking:** `Measure.getNote()`, `getNoteOn()` and `getNoteOff()` return live references: an edit through them reaches the score, and the reference is valid until the measure gains or loses notes. They returned copies. `getNoteOn()` and `getNoteOff()` raise `IndexError` for an index past the staff's notes on (rests), where they returned another note
- **Breaking:** `Measure.removeNote(noteId, staff)` removes exactly that note: it erased the notes before it (`removeNote(0)` removed nothing), and an index or a staff outside the measure, which was undefined behaviour, raises `IndexError` (C++ `std::out_of_range`). `Measure.addNote()` with a list inserts it in list order (it was reversed), all or nothing; a position past the end or a staff outside the measure raises `IndexError` (a staff number too large raised `RuntimeError`)
- **Breaking:** `ScoreCollection.setDirectoriesPaths()` replaces the collection's scores, all or nothing, where it appended them (1, then 2, then 3 scores). Discovery matches `.xml`, `.mxl` and `.musicxml` without regard to case (`.XML` and `.MusicXML` were skipped) and loads in sorted path order (it used the file system's). `ml.ScoreCollection()` builds an empty collection, and a path that does not exist or is not a directory raises `RuntimeError` naming it — both raised `UnicodeDecodeError` from a localised message; in C++ `ScoreCollection collection;` was ambiguous. `removeScore()` raises `IndexError` for any index outside the collection, where `removeScore(-1)` crashed the interpreter
```

- [ ] **Step 6: Remaining docs.** Bash `git -C /c/Users/nyck/Desktop/maialib grep -n -e "'measureId'" -e "writtenClefKey" -e "segmentWrittenPitch" -e "totalIntervalsSimilarityThreshold" -e "totalRhythmSimilarityThreshold" -e "returns a copy of the note" -e "_cachedNoteEventsPerPart" -- '*.md' '*.h' '*.cpp' '*.py' '*.ipynb' ':!docs/superpowers' ':!AI_API_CHEATSHEET.md' ':!llms-full.txt' ':!DEVELOPMENT_PLAN.md' ':!CHANGELOG.md' ':!test/'` → no line (the tests under `test/` pass `totalIntervalsSimilarityThreshold=1.0` on purpose, to assert the `TypeError`; the CHANGELOG names the old names on purpose; `DEVELOPMENT_PLAN.md` is a historical plan; the generated `AI_API_CHEATSHEET.md` and `llms-full.txt` are regenerated at release). Commit `git add README.md CHANGELOG.md python-tutorial/find_pattern.ipynb python-tutorial/03_advanced/01_pattern_finding.ipynb`, message:

```
docs: changelog, README and notebooks of the melody search over every line

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

- [ ] **Step 7: Final verification (spec §7)** from the committed tree, in a second brand-new venv:
  1. PowerShell: `py -3.12 -m venv C:\Users\nyck\AppData\Local\Temp\maialib-5-final-venv; & 'C:\Users\nyck\AppData\Local\Temp\maialib-5-final-venv\Scripts\python.exe' -m pip install -r requirements-dev.txt; $LASTEXITCODE` → 0; in «build» below use `$py = 'C:\Users\nyck\AppData\Local\Temp\maialib-5-final-venv\Scripts\python.exe'`.
  2. «build» `make "PYTHON=$py" dev` → 0; `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`.
  3. «build» `make "PYTHON=$py" cpp-tests` twice → 0 both times, `[  PASSED  ] 1185 tests.` (1156 + 3 + 1 + 9 + 9 + 4 + 3; measured on the finished tree).
  4. «build» `make "PYTHON=$py" py-tests` → OK, `Ran 686 tests` (653 + 4 + 5 + 7 + 6 + 11), `OK (skipped=1)`; record the duration.
  5. «build» `make "PYTHON=$py" validate` → no new findings (measured: "no new findings (46 known)" and "19 baseline finding(s) no longer reported").
  6. «build» `make "PYTHON=$py" corpus` (the external corpus fetched) → 0: neither ledger differs (the seven added lines are in the ledger), and the slow round trip ran.
  7. «build» `make "PYTHON=$py" msvc-gate` → 0.
  8. «build» `make "PYTHON=$py" linux-gate` → 0. If it exits 2 listing missing apt packages (this machine's WSL has no `cmake` and no password-less sudo), run the Linux route instead and report it: in Bash, `git -C /c/Users/nyck/Desktop/maialib -c core.autocrlf=false -c core.eol=lf archive HEAD` extracted under `/var/tmp/maialib-5` in WSL; there a venv with `requirements-dev.txt` plus `cmake` from pip; `make "PYTHON=<venv>/bin/python" dev` and `make "PYTHON=<venv>/bin/python" py-tests` → 0; record the count (686); remove `/var/tmp/maialib-5` afterwards.
  9. «build» `make "PYTHON=$py" fuzz` → 0; compare `test\musicxml\fuzz-work\report-seed-1.json` with `C:\Users\nyck\AppData\Local\Temp\maialib-5-fuzz-baseline.json` by outcome, not case by case — the seven new fixtures shift the list of files the cases pick from (`test/musicxml/README.md`), and a mutated `<staff>` beyond the part's staves now raises `IndexError` where it raised `RuntimeError`: no `crash:*` or `timeout:*` outcome, and no outcome that the baseline does not have; explain each outcome count that changed. The fuzz worker's analyses do not run the melody search, so a finding there is outside this step's code unless it involves `Measure::addNote`.
  10. Import check from outside the repository (Task 0, Step 5) with the final venv; `git status --short` → ` M .gitignore` only; `git log --oneline "$(git log -1 --format=%H -- docs/superpowers/plans/2026-10-05-melody-search.md)..HEAD"` lists exactly the seven task commits (Tasks 1-7), which follow the plan's last commit.
  Put every count, duration and comparison in the task report.

---

## Self-review (done while writing)

- **Spec coverage.** §1 problems → Task 3/4 (one line per part, voices, staves, chords, ties), Task 4 (last window; unnameable transposition), Task 5 (`findAnyMelodyPattern` exposure, `(0)`/`(-1)`, deduplication, cache), Task 4/6 (names; empty results), Task 6 (ScoreCollection), Task 2 (copies), Task 1 (`removeNote`, `addNote`). §2 D1 → Tasks 3, 4 (`staff`, `voice` columns; no window crosses lines); D2 → Task 2 (numpydoc validity sentence); D3 → Task 6; D4 → Task 3 (summed duration); D5 → Task 5; D6 → Tasks 4, 6. §3 lines (part, staff, voice ascending; highest sounding note; tie same sounding pitch + stop; first note's written pitch and measure; rests; grace) → Task 3 (`MelodicLines.*`), windows `N − L + 1` → Task 4 (`TheLastWindowIsSearched`), pattern length (< 2 incl. empty raises naming method and length; `findAnyMelodyPattern(n < 2)`; longer than every line → empty) → Tasks 4, 5; no cache → Task 5 (members, function, invalidation lines removed; `ASearchSeesTheEditsMadeBeforeIt`). §4.1 unchanged (the helpers are untouched). §4.2 → Task 4 (matches only; `transposeSemitones` exact, NaN; `transposeInterval` concert spelling with direction, `"P1"`, empty for no name or all rests; no quarter-tone rejection in the search; `Interval`/`Chord` unchanged). §4.3 → Task 4 (columns, types, dtypes of empty results, thresholds in C++ and Python, `patternIdx`, sort), Task 6 (`fileName`, `composerName`, `scoreTitle`, sort). §4.4 → Task 5 (same lines; exact positions and durations; first kept; `minOccurrences` at least 1, default 2; thresholds default 1.0 in C++ and Python; `patternIdx`, `patternPitches`). §5 → Task 6 (default constructor C++/Python; `recursive`; English `RuntimeError` naming the path; case-insensitive; recursive only on request; sorted; `setDirectoriesPaths` replaces; `addDirectory` and `clear()` unchanged; `removeScore` `std::out_of_range`; searches; loading isolation left to 4c-1). §6 → Task 2 (live references; numpydoc; `Chord.getNote` and `Part.getMeasures` untouched; texts updated: `py_part.cpp`, test docstring, CHANGELOG in Task 7), Task 1 (`removeNote`, `addNote`). §7 fixtures → Tasks 3 (two staves with several voices; chord with highest note written after the lowest; tie across a barline; transposing instrument), 4 (last window; unnameable interval and quarter tone), 5 (duplicates with same and different rhythms), each with its ledger line; tests made exact (`EXPECT_GE` on `size_t` in Task 6, `removeNote(0)` with `<=` in Task 1, Python `removeNote` in Task 1); dedupe count 3 → 4 and the quarter-tone rejection tests changed deliberately (Tasks 4, 5); docs → every task's Doxygen/numpydoc, Task 7 (README re-run, notebooks); stubs, cheatsheet and `llms-full.txt` at release (every task restores the two generated files); CHANGELOG → Task 7 (every listed breaking change and addition); verification → Task 7 Step 7. §8 → nothing (4c-1 isolation, `Part.addStaves`, `Chord.removeNote` untouched).
- **Interfaces are consistent across tasks:** `requireStaff` (1 → 2); `melodicLines`, `MelodicLine`, `MelodicEvent` (3 → 4, 5); `MelodyPatternRow`, `MelodyPatternTable`, `MelodySearchInput`, `MelodySearchCallbacks`, `firstSoundingNote`, `searchEachPattern`, `MelodyDataFrame` (4 → 5, 6); `FoundMelodyPattern` (5); `ScoreCollection::MelodyPatternRow` (6). Task 4's adaptation of `score_collection.cpp` is replaced whole in Task 6.
- **Measured facts the tests rely on** (a scratch copy of cab545b with every task applied, built with clang 18 and its Python module installed in a fresh venv): 1185 C++ tests and 686 Python tests pass; `make validate` clean; the seven fixtures valid and their ledger lines as given; the README output; the docstring examples (`[[0, ['C4', 'D4'], 0, ['C4', 'D4']], [0, ['C4', 'D4'], 0, ['C4', 'D4']]]` and `[0, 1]` with `minOccurrences=1`, `['A4', 'B4', 'C4']`, `['C4', 'E4']`, `'D4'`, `(True, 1.0)`, `[0, 1]`, `['fileName', 'composerName', 'scoreTitle', 'partName']`); the Beethoven `findAnyMelodyPatternDataFrame()` size and time at the defaults (2,164 patterns, 489,196 rows, 14.1 s); the state after Task 4 alone (C++ tests, 1174 at that point without Tasks 1-2) builds and passes.
