# MusicXML <transpose> Reading and Writing (roadmap step 1b) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Read MusicXML `<transpose>` in every measure and per staff, with `<octave-change>` and `<double>`, correcting or ignoring what cannot be followed with one coded warning per element; write `<transpose>` back on export so that a re-imported score keeps its transpositions; report the concert key in `Score::getChords()`; and count the octave doubling in the chord analysis and the piano roll.

**Architecture:** The notes are the single source of truth (spec D1): each pitched `Note` holds its total transposing interval (two ints, `octave-change` folded in) and a new `OctaveDoubling`. `Score::loadXMLFile()` collects a part's `<transpose>` elements while it reads the measures (with the number of `<note>` elements before each one's `<attributes>`), then stamps them on the stored pitched notes staff by staff, a chord as a unit, checking each element's whole scope first with the non-throwing `maiacore::detail::soundsWithinRange()`. `Part::toXML()` derives a per-part plan of `<transpose>` elements from the notes and writes the measure-start ones in schema position; `Measure::toXML()` gains an overload that writes the mid-measure `<attributes>`. `Score::getChords()` computes the concert key of each measure once per call and adds the doubled octaves to its chords. `Part::setTransposingInterval()` stamps a range of measures and a staff, all or none. There is no transposition state on `Measure`, `Part` or `Score`.

**Tech Stack:** C++17 (maiacore), pugixml (reader), pybind11 bindings with numpydoc docstrings, GoogleTest 1.14 (`tests-cpp/src/`, sources listed in `tests-cpp/CMakeLists.txt`), Python `unittest` (`test/`), `lxml` through the 4a validator `test/musicxml/musicxml_check.py`, pandas/plotly (`maialib/maiapy/plots.py`).

**Spec:** `docs/superpowers/specs/2026-10-02-transpose-io-design.md` is binding; every requirement maps to a task (see the self-review at the end). Step 1a's views and the conventional diatonic interval: `docs/superpowers/specs/2026-09-30-note-pitch-views-design.md` (D4). Test infrastructure: `test/musicxml/README.md`.

## Global Constraints

- C++17; no lambda captures a structured binding (a C++20 feature that Clang before 16 rejects); every C++ file a task changes is formatted with `& 'C:\Program Files\LLVM\bin\clang-format.exe' -i <files>` (the clang-format 18.1.6 that `make format-cpp` falls back to; every file touched here is clang-format clean today, so only new code moves) for the 100-column limit.
- `make validate` adds no cpplint or cppcheck finding: every `.cpp`/`.h` under `maiacore/src/maiacore/` that starts using `std::map`, `std::pair`, `std::string`, `std::vector`, `std::optional`, `std::int64_t`, `std::max`/`std::min`/`std::clamp`/`std::all_of` or `std::out_of_range` includes `<map>`, `<utility>`, `<string>`, `<vector>`, `<optional>`, `<cstdint>`, `<algorithm>` or `<stdexcept>` itself; no C-style cast. The new includes clear some `build/include_what_you_use` findings of `part.cpp`, `measure.cpp` and `score.cpp`, so `make validate` may say that baseline findings are no longer reported: leave `scripts/validate-baseline.json` as it is.
- Python in `maialib/` supports Python 3.8–3.14; `test/musicxml/` and every new Python module are Python 3.8-compatible (no `list[str]`-style annotations, no `str.removeprefix`) and clean under `ruff format` / `ruff check` with `pyproject.toml`. Additions to existing modules follow that module's naming (camelCase in `test_note.py` and `maialib/maiapy/plots.py`, snake_case in `test_part_comprehensive.py`, `test_score_comprehensive.py` and the 4a modules).
- Docs, Doxygen, numpydoc, comments and commit messages in technical English. Comments explain the code, never its development history: no task numbers, review rounds, rulings or SHAs in code, tests or fixtures.
- Every new test is proven to fail under a targeted mutation of the code it protects: apply the named mutation with Edit, rebuild what the test runs against, run the test and record the failing output, revert the same Edit exactly, rebuild, rerun green. Record each mutation and its failing output in the task report. `git diff` after the revert shows only the task's intended changes.
- Never stage the user's uncommitted root `.gitignore` change (`musescore/*`); stage files by name, never `git add -A`/`.`; `git status --short` ends every task showing only ` M .gitignore`.
- Commit messages end with these two lines:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Line endings: `core.autocrlf=true`, so every existing file is CRLF in the working tree and LF in the index; edits keep the file's endings (the Edit tool does). New files may be written with LF; git stores LF either way. `test/musicxml/golden/**` is `-text` and written as LF bytes by `dump_score.py`.
- Test environment (memory and spec §7.4): `make dev` only in a brand-new venv, `py -3.12`, created outside the repository at `C:\Users\nyck\AppData\Local\Temp\maialib-1b-venv` (Task 0) with `pip install -r requirements-dev.txt`; the final verification uses a second brand-new venv `C:\Users\nyck\AppData\Local\Temp\maialib-1b-final-venv`. Import checks run from outside the repository root. Read exit codes directly (`$LASTEXITCODE`, `$?`), never through a pipe. Never use `vswhere -latest`.
- **«build»** below means this PowerShell prefix, repeated in every PowerShell call because shell state does not persist: `$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'; $env:INCLUDE = $null; $env:LIB = $null; $py = 'C:\Users\nyck\AppData\Local\Temp\maialib-1b-venv\Scripts\python.exe';` (the clang build needs `VCToolsInstallDir` with `INCLUDE` and `LIB` unset).
- **C++ subset:** «build» `make "PYTHON=$py" build-cpp-tests; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; .\build\Windows\cpp-tests\cpp-tests.exe --gtest_filter='<Suite>.*'; $LASTEXITCODE` (the binary runs from the repository root, where the fixtures' relative paths start).
- **«pytest» X** below means, in Bash: `(cd /c/Users/nyck/Desktop/maialib/test && /c/Users/nyck/AppData/Local/Temp/maialib-1b-venv/Scripts/python.exe -m unittest X -v); echo "exit $?"` — the Python tests run from `test/` against the installed package after «build» `make "PYTHON=$py" dev`.
- Never load a corpus file of 10 MB or more (`Symphony_9th.xml`, `xakypueri.xml`) in a unit test that `make py-tests` or `make cpp-tests` runs; they run only under `make corpus` (Task 6).
- The generated documentation is not regenerated in this step (roadmap release step): `stubs/`, `AI_API_CHEATSHEET.md`, `llms-full.txt`, the `python-tutorial` notebooks.
- Warning format (one `LOG_WARN` per `<transpose>` element and per correction it receives): `[<code>] part "<part name>", measure <number as the file writes it>: <what was done>`. Codes: `transpose-chromatic-not-integer`, `transpose-octave-change-not-integer`, `transpose-pair-corrected`, `transpose-out-of-range`, `for-part-not-modelled`. Phase 4c-1 parses neither text nor format.
- Equality: `OctaveDoubling` takes no part in `Note`'s `==`/`!=` or its Python hash (they keep comparing the concert spelling); the getter's Doxygen and numpydoc say so.
- Out-of-range scope: an element's scope is every pitched note it would stamp before the next `<transpose>` for the same staff or for all staves; the scope is checked before anything is stamped, so a rejected element stamps nothing.
- Fixtures: small `score-partwise` files `test/xml_examples/unit_test/transpose_<rule>.musicxml`, valid against the MusicXML 4.0 schema unless the rule needs an invalid value (the two `*_not_integer` fixtures of `<octave-change>` and `<diatonic>`, which the schema types as `xs:integer`); written octaves stay within the schema's 0–9; every part has a `<key>` in measure 1 (the reader still fails without one). The task that adds fixtures runs `make corpus-update-ledger` and commits only the added `test/musicxml/ledger.json` lines after `git diff test/musicxml/ledger.json` shows no other line changed and `git diff --stat test/musicxml/ledger-external.json` is empty.
- Tests: Python (lxml, the 4a validator) for anything that inspects or validates exported XML; C++ for the model, the reader and the analyses, next to the existing C++ tests. A C++-only serialization API that Python cannot reach (the `Measure::toXML` overload of Task 5, which the bindings do not expose) is tested in C++ on the text it returns.
- Line hints (`~N`) give the line at bd63266; an earlier task's insertions move them. The quoted anchor text decides where an edit goes: every anchor matches exactly once, in the text the earlier tasks leave after clang-format.
- Dump: each note record of `test/musicxml/dump_score.py` gains `"octaveDoubling"` with the enum member's name (`"NONE"`, `"BELOW"`, `"ABOVE"`), and `test/musicxml/golden/test_staves.dump.json` is regenerated in the same task.
- Writer values (spec §5.2): `oc = trunc(c / 12)` toward zero, `<diatonic>d − 7·oc</diatonic>`, `<chromatic>c − 12·oc</chromatic>`, `<octave-change>` only when `oc ≠ 0`; `<diatonic>` always written, with the conventional value when the stored one is 0 and `c ≠ 0`; `<double/>` for `BELOW`, `<double above="yes"/>` for `ABOVE`; inside `<attributes>` after `clef` and `staff-details`; `<for-part>` never written; the header stays MusicXML 3.0.
- Concert key (spec §6.1): key-neutral means `7·c − 12·d == 0` with `d` the diatonic interval the speller uses; majority of written keys compared as (fifths, mode); a tie goes to the first part in score order; fallback: part 0's written key moved by `7·c − 12·d` fifths, kept when within −6..11 (the range `Key`'s constructor accepts), otherwise brought inside by adding or subtracting 12.

## Decisions this plan takes beyond the spec (reviewers: accept or overrule)

1. **`OctaveDoubling` is in the global namespace** (`constants.h`, beside `RhythmFigure`), not `maiacore::OctaveDoubling`: no public maiacore type lives in a `maiacore` namespace (`ClefSign`, `RhythmFigure`, `METRIC`, `Note`, `Part` are global; only the internal `maiacore::detail` exists).
2. **Unpitched notes are not stamped by the reader** (spec §4.1 names only rests): the setter (§3.2) and the writer (§5.1) leave them alone, so stamping them would make an export → import lose their interval.
3. **A rejected element's previous transposition is checked too**: a note that the previous transposition cannot sound either (only a crafted file reaches this) is read untransposed, and the element's one warning says how many. Without this the load could throw, which §4.3 rules out.
4. **A fifth code, `transpose-octave-change-not-integer`**: an `<octave-change>` that is not a whole number (schema-invalid) makes the element ignored like a non-integer `<chromatic>`. A `<diatonic>` that is not a whole number does not match `<chromatic>`, so it is a `transpose-pair-corrected`.
5. **A `number` that is not a positive integer reads as absent** (every staff); one beyond the part's staves applies to no note.
6. **A mid-measure change in a part with more than one staff always carries `number`**: the writer emits staves one after another, so a `<transpose>` without `number` there would also reach the next staves' notes of that measure.
7. **A change whose note is not the first of its chord is written before the chord's first note.**
8. **A doubled octave left out of `getChords()` is reported once per note and per call**, not once per chord it sounds in; the added note is an untransposed note at the concert pitch one octave away.
9. **Decomposition:** a Task 0 records baselines (counts, a fuzz report); the non-integer `<chromatic>`/`<octave-change>` rules move from the corrections task to the reader task (Task 3), because the reader's parser must already decide what such a value means; `conventionalDiatonicInterval` is exposed in Task 3, its first user (absent `<diatonic>`); `spelledDiatonicInterval` (the speller's diatonic interval) is added in Task 2, where `concertSpelling` and `soundsWithinRange` share it, and reused by Tasks 5 and 7; the slow corpus files' round trip runs under `make corpus` through `MAIALIB_SLOW_TESTS=1` (Task 6).

## File map

| File | Responsibility | Tasks |
|---|---|---|
| `maiacore/include/maiacore/constants.h` | `enum class OctaveDoubling { NONE, BELOW, ABOVE }` | 1 |
| `maiacore/include/maiacore/note.h`, `maiacore/src/maiacore/note.cpp` | the `_octaveDoubling` field, `setOctaveDoubling`/`getOctaveDoubling`, `setPitch("rest")` clears it; `soundsWithinRange` (friend), the shared ceiling check; `spelledDiatonicInterval` added to and `conventionalDiatonicInterval` moved to `maiacore::detail`; doc of the inferred interval | 1, 2, 3, 8 |
| `maiacore/src/maiacore/pitch-views.h` | declarations and Doxygen of `soundsWithinRange`, `spelledDiatonicInterval`, `conventionalDiatonicInterval` | 2, 3 |
| `maiacore/include/maiacore/part.h`, `maiacore/src/maiacore/part.cpp` | `Part::setTransposingInterval`; the `<transpose>` plan and its writing in `Part::toXML` | 2, 5 |
| `maiacore/include/maiacore/measure.h`, `maiacore/src/maiacore/measure.cpp` | `Measure::toXML` overload writing XML before chosen notes | 5 |
| `maiacore/include/maiacore/score.h`, `maiacore/src/maiacore/score.cpp` | the reader (collect, check, stamp, warn); the concert key and the doubled octaves in `getChords`; Doxygen of `Score(path)`, `toXML`, `toFile`, `getChords` | 3, 4, 5, 7, 8 |
| `maiacore/src/maiacore/python_wrapper/py_constants.cpp` | binding of `OctaveDoubling` | 1 |
| `maiacore/src/maiacore/python_wrapper/py_note.cpp` | bindings and numpydoc of the doubling; doc of `setPitch`, `setIsNoteOn`, `getSoundingPitch` | 1, 3, 8 |
| `maiacore/src/maiacore/python_wrapper/py_part.cpp` | binding of `setTransposingInterval`; numpydoc of `toXML` | 2, 5 |
| `maiacore/src/maiacore/python_wrapper/py_measure.cpp` | `toXML` bound through `py::overload_cast` | 5 |
| `maiacore/src/maiacore/python_wrapper/py_score.cpp` | numpydoc of `Score(path)`, `toXML`, `toFile`, `getChords` | 3, 4, 5, 7, 8 |
| `maialib/maiapy/plots.py` | `plotPianoRoll` draws the doubled octave | 8 |
| `test/xml_examples/unit_test/transpose_*.musicxml` (14 new) | one fixture per rule | 3, 4 |
| `test/musicxml/ledger.json` | 14 added lines | 3, 4 |
| `tests-cpp/src/note-test.cpp` | `NoteOctaveDoubling` | 1 |
| `tests-cpp/src/pitch-views-test.cpp` | `SoundsWithinRange`, `ConventionalDiatonicInterval`, `SpelledDiatonicInterval` | 2, 3 |
| `tests-cpp/src/part-test.cpp` | `PartSetTransposingInterval` | 2 |
| `tests-cpp/src/measure-test.cpp` | `MeasureSerialization.ToXMLWritesTheInsertionsBeforeTheirNotes` | 5 |
| `tests-cpp/src/score-test.cpp` | `ScoreTransposeRead`, `ScoreTransposeCorrection`, `ScoreConcertKey`, `ScoreOctaveDoubling` | 3, 4, 7, 8 |
| `test/test_note.py`, `test/test_part_comprehensive.py`, `test/test_score_comprehensive.py`, `test/test_maiapy.py` | Python tests of the API, the key column and the piano roll | 1, 2, 7, 8 |
| `test/test_musicxml_transpose.py` (new) | writer tests; round trip and validity of every fixture and corpus file with `<transpose>` | 5, 6 |
| `test/musicxml/dump_score.py`, `test/test_musicxml_dump.py`, `test/musicxml/golden/test_staves.dump.json` | the dump's `octaveDoubling` | 6 |
| `scripts/make-corpus.py`, `test/musicxml/README.md` | `make corpus` runs the round trip of the slow files | 6 |
| `CHANGELOG.md` | `[Unreleased]` entries; the Dvořák known limitation removed | 9 |

`tests-cpp/CMakeLists.txt` does not change: every C++ test goes into a source it already lists.

---

### Task 0: Baseline (no commit)

**Files:** none changed.

**Interfaces:** produces the venv `C:\Users\nyck\AppData\Local\Temp\maialib-1b-venv`, the baseline counts, and the fuzz report `C:\Users\nyck\AppData\Local\Temp\maialib-1b-fuzz-baseline.json` that Task 9 compares with.

- [ ] **Step 1: Brand-new venv.** PowerShell: `py -3.12 -m venv C:\Users\nyck\AppData\Local\Temp\maialib-1b-venv; & 'C:\Users\nyck\AppData\Local\Temp\maialib-1b-venv\Scripts\python.exe' -m pip install -r requirements-dev.txt; $LASTEXITCODE` → 0.
- [ ] **Step 2: Build and install.** «build» `make "PYTHON=$py" dev; $LASTEXITCODE` → 0.
- [ ] **Step 3: Baselines.** «build» `make "PYTHON=$py" cpp-tests; $LASTEXITCODE` → 0; record gtest's `[  PASSED  ] N tests`. «build» `make "PYTHON=$py" py-tests; $LASTEXITCODE` → 0; record `Ran N tests` / `OK (skipped=K)`. «build» `make "PYTHON=$py" validate; $LASTEXITCODE` → 0; record `no new findings (K known)`.
- [ ] **Step 4: Fuzz baseline.** «build» `make "PYTHON=$py" fuzz; $LASTEXITCODE` → 0 (the script always exits 0); record the outcome counts it prints (`300 cases of seed 1:` and one `<outcome>: <count>` line each), then `Copy-Item test\musicxml\fuzz-work\report-seed-1.json C:\Users\nyck\AppData\Local\Temp\maialib-1b-fuzz-baseline.json`.
- [ ] **Step 5: Import check from outside the repository.** Bash: `(cd /c/Users/nyck/AppData/Local/Temp && /c/Users/nyck/AppData/Local/Temp/maialib-1b-venv/Scripts/python.exe -c "import maialib; print(maialib.__version__)"); echo "exit $?"` → the version, exit 0. `git status --short` → ` M .gitignore`.

---

### Task 1: `OctaveDoubling` and the note's doubling

**Files:**
- Modify: `maiacore/include/maiacore/constants.h` (after `c_mapTimeSignatureLower_Duration`, ~75-79), `maiacore/include/maiacore/note.h` (members ~72-74; `setPitch` Doxygen ~302-305; after `setTransposingInterval` ~347; after `isTransposed` ~829), `maiacore/src/maiacore/note.cpp` (constructor init list ~437-438; `setPitch` ~909-918; after `setTransposingInterval` ~928-942), `maiacore/src/maiacore/python_wrapper/py_constants.cpp`, `maiacore/src/maiacore/python_wrapper/py_note.cpp` (`setIsNoteOn` ~332-341, `setPitch` ~359-364, before `setVoice` ~428, before `isGraceNote` ~1053)
- Test: `tests-cpp/src/note-test.cpp` (append), `test/test_note.py` (append)

**Interfaces:** produces `enum class OctaveDoubling { NONE, BELOW, ABOVE };` (global, `maiacore/constants.h`; Python `maialib.OctaveDoubling`), `void Note::setOctaveDoubling(const OctaveDoubling doubling)`, `OctaveDoubling Note::getOctaveDoubling() const` (Python `Note.setOctaveDoubling(doubling)`, `Note.getOctaveDoubling()`). Consumed by Tasks 2, 3, 5, 6, 8.

- [ ] **Step 1: Write the failing C++ tests** — append to `tests-cpp/src/note-test.cpp`:

```cpp
// ===================================================================================================
// OCTAVE DOUBLING
// ===================================================================================================

TEST(NoteOctaveDoubling, aNoteIsNotDoubledUntilItIsSet) {
    Note note("C3");
    EXPECT_EQ(note.getOctaveDoubling(), OctaveDoubling::NONE);
    note.setOctaveDoubling(OctaveDoubling::BELOW);
    EXPECT_EQ(note.getOctaveDoubling(), OctaveDoubling::BELOW);
    note.setOctaveDoubling(OctaveDoubling::ABOVE);
    EXPECT_EQ(note.getOctaveDoubling(), OctaveDoubling::ABOVE);
    const Note copy = note;
    EXPECT_EQ(copy.getOctaveDoubling(), OctaveDoubling::ABOVE);
}

// A rest sounds nothing to double: it refuses the doubling with a warning, as setOctave() does.
TEST(NoteOctaveDoubling, aRestRefusesTheDoublingWithAWarning) {
    Note rest("rest");
    StdoutCapture capture;
    rest.setOctaveDoubling(OctaveDoubling::BELOW);
    EXPECT_EQ(rest.getOctaveDoubling(), OctaveDoubling::NONE);
    EXPECT_NE(capture.str().find("[WARN] Note::setOctaveDoubling: cannot set the octave doubling "
                                 "of a rest; ignoring"),
              std::string::npos)
        << capture.str();
}

// setPitch() to a rest clears the doubling with the transposing interval; setIsNoteOn(false)
// keeps both.
TEST(NoteOctaveDoubling, settingARestClearsTheDoublingAndSilencingKeepsIt) {
    Note cleared = bassClarinet("D4");
    cleared.setOctaveDoubling(OctaveDoubling::BELOW);
    cleared.setPitch("rest");
    EXPECT_EQ(cleared.getOctaveDoubling(), OctaveDoubling::NONE);
    EXPECT_FALSE(cleared.isTransposed());

    Note silenced = bassClarinet("D4");
    silenced.setOctaveDoubling(OctaveDoubling::BELOW);
    silenced.setIsNoteOn(false);
    EXPECT_EQ(silenced.getOctaveDoubling(), OctaveDoubling::BELOW);
    EXPECT_TRUE(silenced.isTransposed());
}

// The doubling is not part of the note's own pitch, nor of what == compares.
TEST(NoteOctaveDoubling, thePitchViewsAndEqualityIgnoreTheDoubling) {
    Note doubled("C3");
    doubled.setOctaveDoubling(OctaveDoubling::BELOW);
    EXPECT_EQ(doubled.getWrittenPitch(), "C3");
    EXPECT_EQ(doubled.getSoundingPitch(), "C3");
    EXPECT_EQ(doubled.getMidiNumber(), 48);
    EXPECT_TRUE(doubled == Note("C3"));
    EXPECT_FALSE(doubled != Note("C3"));
}
```

- [ ] **Step 2: Write the failing Python tests** — append to `test/test_note.py`, before `if __name__ == "__main__":`:

```python
class NoteOctaveDoubling(unittest.TestCase):
    """A note holds the octave doubling of its part (MusicXML <double>)."""

    def testTheEnumNamesTheThreeDoublings(self):
        self.assertEqual(["NONE", "BELOW", "ABOVE"], list(ml.OctaveDoubling.__members__))

    def testANoteIsNotDoubledUntilItIsSet(self):
        note = ml.Note("C3")
        self.assertEqual(note.getOctaveDoubling(), ml.OctaveDoubling.NONE)
        note.setOctaveDoubling(ml.OctaveDoubling.ABOVE)
        self.assertEqual(note.getOctaveDoubling(), ml.OctaveDoubling.ABOVE)

    def testARestRefusesTheDoublingWithAWarning(self):
        rest = ml.Note("rest")
        printed = capturedStdout(lambda: rest.setOctaveDoubling(ml.OctaveDoubling.BELOW))
        self.assertIn(
            "[WARN] Note::setOctaveDoubling: cannot set the octave doubling of a rest", printed
        )
        self.assertEqual(rest.getOctaveDoubling(), ml.OctaveDoubling.NONE)

    def testSettingARestClearsTheDoubling(self):
        note = ml.Note("D4", transposeDiatonic=-8, transposeChromatic=-14)
        note.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        note.setPitch("rest")
        self.assertEqual(note.getOctaveDoubling(), ml.OctaveDoubling.NONE)

    def testEqualityAndHashIgnoreTheDoubling(self):
        doubled = ml.Note("C3")
        doubled.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        self.assertEqual(doubled, ml.Note("C3"))
        self.assertEqual(hash(doubled), hash(ml.Note("C3")))

    def testTheGetterSaysThatEqualityIgnoresIt(self):
        doc = " ".join(ml.Note.getOctaveDoubling.__doc__.split())
        self.assertIn("``==``, ``!=`` and the hash ignore the doubling", doc)
```

- [ ] **Step 3: Run them and see them fail.** C++ subset `NoteOctaveDoubling` → the build fails: `'OctaveDoubling' was not declared in this scope` (clang: `use of undeclared identifier 'OctaveDoubling'`). Python is not run yet (the module is unchanged): «pytest» `test_note.NoteOctaveDoubling` → `AttributeError: module 'maialib' has no attribute 'OctaveDoubling'`.

- [ ] **Step 4: The enum** — in `maiacore/include/maiacore/constants.h`, after the `c_mapTimeSignatureLower_Duration` map (`{512, RhythmFigure::N512TH}, {1024, RhythmFigure::N1024TH}};`), insert:

```cpp

/**
 * @brief Whether a note's part is doubled one octave from what the note sounds, as a MusicXML
 *        `<double>` inside `<transpose>` states it: mixed cello and bass parts are doubled below,
 *        mixed flute and piccolo parts above.
 * @details Each note holds its own (Note::getOctaveDoubling()); a note's pitch getters describe
 *          the note itself, never the doubled octave.
 */
enum class OctaveDoubling {
    NONE,   ///< Not doubled.
    BELOW,  ///< Doubled one octave below: `<double/>`.
    ABOVE,  ///< Doubled one octave above: `<double above="yes"/>`.
};
```

- [ ] **Step 5: The field and the declarations** — in `maiacore/include/maiacore/note.h`:
  - replace the three member lines

```cpp
    bool _inChord;            ///< True if this note is part of a chord.
    int _transposeDiatonic;   ///< Diatonic transposition interval.
    int _transposeChromatic;  ///< Chromatic transposition interval.
```

  with

```cpp
    bool _inChord;                   ///< True if this note is part of a chord.
    int _transposeDiatonic;          ///< Diatonic transposition interval.
    int _transposeChromatic;         ///< Chromatic transposition interval.
    OctaveDoubling _octaveDoubling;  ///< Octave doubling of the note's part (MusicXML <double>).
```

  - in the Doxygen of `setPitch`, replace the two lines

```cpp
     *          so the note then sounds this pitch moved by it; setting a rest also clears the
     *          interval and the in-chord and grace-note flags.
```

  with

```cpp
     *          so the note then sounds this pitch moved by it; setting a rest also clears the
     *          interval, the octave doubling and the in-chord and grace-note flags.
```

  - after `void setTransposingInterval(const int diatonicInterval, const int chromaticInterval);` insert:

```cpp

    /**
     * @brief Sets the octave doubling of the note (see getOctaveDoubling()).
     * @details A rest has nothing to double: on a rest this changes nothing and prints a warning
     *          (LOG_WARN), as setOctave() and setAlter() do.
     * @param doubling OctaveDoubling::NONE, OctaveDoubling::BELOW or OctaveDoubling::ABOVE.
     */
    void setOctaveDoubling(const OctaveDoubling doubling);
```

  - after `bool isTransposed() const;` insert:

```cpp

    /**
     * @brief Returns the octave doubling of the note: whether its part is doubled one octave
     *        below or above what it sounds (MusicXML `<double>`).
     * @details The pitch getters -- written, sounding and acoustic -- describe the note itself,
     *          never the doubled octave. The doubling takes no part in operator==() and
     *          operator!=(), nor in the Python hash, which hashes what operator==() compares: two
     *          notes that differ only in their doubling are equal. A note constructed as a rest,
     *          or set to one with setPitch(), is not doubled; a note silenced with
     *          setIsNoteOn(false) keeps its doubling, as it keeps its transposing interval.
     * @return OctaveDoubling::NONE, OctaveDoubling::BELOW or OctaveDoubling::ABOVE.
     */
    OctaveDoubling getOctaveDoubling() const;
```

- [ ] **Step 6: The implementation** — in `maiacore/src/maiacore/note.cpp`:
  - in the pitch-string constructor's initializer list, replace the two lines `      _transposeChromatic(0),` and `      _voice(1),` with

```cpp
      _transposeChromatic(0),
      _octaveDoubling(OctaveDoubling::NONE),
      _voice(1),
```

  - in `Note::setPitch`, replace the rest branch's two lines `        _transposeChromatic = 0;` and `        _isGraceNote = false;` with

```cpp
        _transposeChromatic = 0;
        _octaveDoubling = OctaveDoubling::NONE;
        _isGraceNote = false;
```

  - before `Pitch Note::computeSoundingPitch() const {` insert:

```cpp
void Note::setOctaveDoubling(const OctaveDoubling doubling) {
    if (!isNoteOn()) {
        LOG_WARN("Note::setOctaveDoubling: cannot set the octave doubling of a rest; ignoring");
        return;
    }

    _octaveDoubling = doubling;
}

OctaveDoubling Note::getOctaveDoubling() const { return _octaveDoubling; }

```

- [ ] **Step 7: The bindings** — in `maiacore/src/maiacore/python_wrapper/py_constants.cpp`, after the `RhythmFigure` enum binding (`.value("N1024TH", RhythmFigure::N1024TH);`), insert:

```cpp

    py::enum_<OctaveDoubling>(m, "OctaveDoubling", R"pbdoc(
        Whether a note's part is doubled one octave from what the note sounds, as a MusicXML
        ``<double>`` inside ``<transpose>`` states it: mixed cello and bass parts are doubled
        below, mixed flute and piccolo parts above. See ``Note.getOctaveDoubling``.
    )pbdoc")
        .value("NONE", OctaveDoubling::NONE, "Not doubled.")
        .value("BELOW", OctaveDoubling::BELOW, "Doubled one octave below: ``<double/>``.")
        .value("ABOVE", OctaveDoubling::ABOVE,
               "Doubled one octave above: ``<double above=\"yes\"/>``.");
```

  In `maiacore/src/maiacore/python_wrapper/py_note.cpp`:
  - in the `setIsNoteOn` docstring replace the lines

```
        ``getOctave()`` None). Its transposing interval is kept. ``setIsNoteOn(True)`` leaves a
        sounding note as it is; a rest has no pitch it could sound, so it prints a warning and
        stays a rest -- give it a pitch with ``setPitch``, ``setPitchClass`` or ``setStep``.
```

  with

```
        ``getOctave()`` None). Its transposing interval and octave doubling are kept.
        ``setIsNoteOn(True)`` leaves a sounding note as it is; a rest has no pitch it could
        sound, so it prints a warning and stays a rest -- give it a pitch with ``setPitch``,
        ``setPitchClass`` or ``setStep``.
```

  - in the `setPitch` docstring replace the line `        clears the transposing interval and the in-chord and grace-note flags.` with the two lines

```
        clears the transposing interval, the octave doubling and the in-chord and grace-note
        flags.
```

  - before `    cls.def("setVoice", &Note::setVoice, py::arg("voice"));` insert:

```cpp
    cls.def("setOctaveDoubling", &Note::setOctaveDoubling, py::arg("doubling"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Set the octave doubling of the note (see ``getOctaveDoubling``).

        A rest has nothing to double: on a rest this changes nothing and prints a warning, as
        ``setOctave`` and ``setAlter`` do.

        Parameters
        ----------
        doubling : OctaveDoubling
            ``OctaveDoubling.NONE``, ``OctaveDoubling.BELOW`` or ``OctaveDoubling.ABOVE``.

        Examples
        --------
        >>> note = ml.Note("C3")
        >>> note.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        >>> note.getOctaveDoubling()
        <OctaveDoubling.BELOW: 1>
    )pbdoc");
```

  - before `    cls.def("isGraceNote", &Note::isGraceNote);` insert:

```cpp
    cls.def("getOctaveDoubling", &Note::getOctaveDoubling,
            R"pbdoc(
        Return the octave doubling of the note: whether its part is doubled one octave below or
        above what it sounds (MusicXML ``<double>``).

        The pitch getters -- written, sounding and acoustic -- describe the note itself, never
        the doubled octave. ``==``, ``!=`` and the hash ignore the doubling: two notes that
        differ only in it are equal. A note constructed as a rest, or set to one with
        ``setPitch``, is not doubled; a note silenced with ``setIsNoteOn(False)`` keeps its
        doubling, as it keeps its transposing interval.

        Returns
        -------
        OctaveDoubling
            ``OctaveDoubling.NONE``, ``OctaveDoubling.BELOW`` or ``OctaveDoubling.ABOVE``.

        Examples
        --------
        >>> note = ml.Note("C3")
        >>> note.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        >>> note.getOctaveDoubling(), note == ml.Note("C3")
        (<OctaveDoubling.BELOW: 1>, True)
    )pbdoc");
```

- [ ] **Step 8: Format, build, pass.** clang-format the six changed C++ files. C++ subset `NoteOctaveDoubling` → 4 tests pass. «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_note.NoteOctaveDoubling` → 6 tests OK.

- [ ] **Step 9: Mutations.** (a) In `Note::setPitch` delete `_octaveDoubling = OctaveDoubling::NONE;` → `NoteOctaveDoubling.settingARestClearsTheDoublingAndSilencingKeepsIt` fails (C++); after `make dev`, `testSettingARestClearsTheDoubling` fails. (b) In `setOctaveDoubling` delete the `if (!isNoteOn()) {...}` block → `aRestRefusesTheDoublingWithAWarning` (C++) and `testARestRefusesTheDoublingWithAWarning` (Python) fail. (c) In `Note::operator==` return `computeConcertPitch() == otherNote.computeConcertPitch() && _octaveDoubling == otherNote._octaveDoubling;` → `thePitchViewsAndEqualityIgnoreTheDoubling` and `testEqualityAndHashIgnoreTheDoubling` fail. (d) In the getter docstring replace `the hash ignore the doubling` with `the hash ignore it` → `testTheGetterSaysThatEqualityIgnoresIt` fails. (e) Copy test: in the constructor init list use `_octaveDoubling(OctaveDoubling::ABOVE)` → `aNoteIsNotDoubledUntilItIsSet` fails; the enum test: bind `.value("UNDER", OctaveDoubling::BELOW)` → `testTheEnumNamesTheThreeDoublings` fails. Revert each; rerun green.

- [ ] **Step 10: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0 (baseline + 4); «build» `make "PYTHON=$py" py-tests` → OK (baseline + 6); «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 11: Commit.** `git add maiacore/include/maiacore/constants.h maiacore/include/maiacore/note.h maiacore/src/maiacore/note.cpp maiacore/src/maiacore/python_wrapper/py_constants.cpp maiacore/src/maiacore/python_wrapper/py_note.cpp tests-cpp/src/note-test.cpp test/test_note.py`, message:

```
feat: a note holds the octave doubling of its part (MusicXML <double>)

OctaveDoubling (NONE, BELOW, ABOVE) with Note::setOctaveDoubling() and
getOctaveDoubling(), in C++ and Python. A rest refuses it with a warning,
setPitch("rest") clears it with the interval, and equality and the hash
ignore it.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 2: `Part::setTransposingInterval`, all or none

**Files:**
- Modify: `maiacore/include/maiacore/note.h` (the `@cond` block ~14-22 and the friend ~117-119), `maiacore/src/maiacore/pitch-views.h` (`#include <cstdint>`; append to the namespace), `maiacore/src/maiacore/note.cpp` (anonymous namespace after `throwSoundingPitchAboveCeiling` ~73; `chromaticSpelling` ~116-126; before `concertSpelling` ~365 and its diatonic interval ~382-384; after `concertSpelling` ~392), `maiacore/include/maiacore/part.h` (includes; after `setIsPitched` ~184), `maiacore/src/maiacore/part.cpp` (includes; after `setIsPitched` ~84), `maiacore/src/maiacore/python_wrapper/py_part.cpp` (after `setIsPitched` ~59)
- Test: `tests-cpp/src/pitch-views-test.cpp` (append), `tests-cpp/src/part-test.cpp` (includes; append), `test/test_part_comprehensive.py` (append)

**Interfaces:** consumes `OctaveDoubling`, `Note::setOctaveDoubling` (Task 1). Produces `std::int64_t maiacore::detail::spelledDiatonicInterval(int transposeDiatonic, int transposeChromatic)` (internal; the diatonic interval `concertSpelling()` moves the letter by) — consumed by Tasks 5 and 7; `bool maiacore::detail::soundsWithinRange(const Note& note, int transposeDiatonic, int transposeChromatic)` (internal; never throws; true exactly where `concertSpelling()` of the note's written pitch with that interval returns a spelling) — consumed by Task 4; and `void Part::setTransposingInterval(const int diatonicInterval, const int chromaticInterval, const int measureStart = 0, const int measureEnd = -1, const int staff = -1, const OctaveDoubling doubling = OctaveDoubling::NONE)` (Python `Part.setTransposingInterval(diatonicInterval, chromaticInterval, measureStart=0, measureEnd=-1, staff=-1, doubling=OctaveDoubling.NONE)`) — consumed by Task 5's tests.

- [ ] **Step 1: Failing C++ tests of the predicate** — append to `tests-cpp/src/pitch-views-test.cpp`, and add `using maiacore::detail::soundsWithinRange;` and `using maiacore::detail::spelledDiatonicInterval;` after `using maiacore::detail::simplestSpelling;`:

```cpp
// The diatonic interval the speller moves the letter by: the stored one, or the conventional one
// for the chromatic interval when the stored one is 0.
TEST(SpelledDiatonicInterval, isTheStoredIntervalOrTheConventionalOneForZero) {
    EXPECT_EQ(spelledDiatonicInterval(-1, -2), -1);
    EXPECT_EQ(spelledDiatonicInterval(0, -2), -1);
    EXPECT_EQ(spelledDiatonicInterval(3, 4), 3);  // a stored interval is used as it is
    EXPECT_EQ(spelledDiatonicInterval(0, 0), 0);
    EXPECT_EQ(spelledDiatonicInterval(7, 0), 7);
}

// soundsWithinRange() answers, without throwing, whether concertSpelling() spells the note's
// written pitch with the interval: false exactly where concertSpelling() raises -- below C1b-1,
// or above B11 where the diatonic interval gives no spelling.
TEST(SoundsWithinRange, isTrueExactlyWhereTheConcertSpellingExists) {
    const std::vector<std::string> pitches = {"C1b-1", "C-1",  "C#-1", "D-1",  "B2",
                                              "C4",    "A#11", "B11",  "B#11", "Bx11"};
    const std::vector<std::pair<int, int>> intervals = {
        {0, 0},    {-1, -2}, {0, -2},  {1, 2},    {1, 3},
        {0, 1},    {1, 1},   {7, 12},  {-7, -12}, {0, -200},
        {0, 200},  {1, std::numeric_limits<int>::max()}, {-1, std::numeric_limits<int>::min()}};
    for (const std::string& pitch : pitches) {
        for (const auto& [diatonic, chromatic] : intervals) {
            const bool spelled =
                concertOrError(Pitch(pitch), diatonic, chromatic).rfind("raises: ", 0) != 0;
            EXPECT_EQ(soundsWithinRange(Note(pitch), diatonic, chromatic), spelled)
                << describe(pitch, diatonic, chromatic);
        }
    }
}

TEST(SoundsWithinRange, theEdgesOfTheRange) {
    EXPECT_TRUE(soundsWithinRange(Note("A#11"), 1, 2));     // B#11: the letter B spells it
    EXPECT_FALSE(soundsWithinRange(Note("B11"), 1, 1));     // C12 has no spelling
    EXPECT_FALSE(soundsWithinRange(Note("C#-1"), -1, -2));  // below C1b-1
    EXPECT_TRUE(soundsWithinRange(Note("D-1"), -1, -2));    // C-1, MIDI note 0
    EXPECT_TRUE(soundsWithinRange(Note("Bx11"), 0, 0));     // untransposed: as written
    EXPECT_TRUE(soundsWithinRange(Note("rest"), 0, 200));   // a rest sounds nothing
}
```

- [ ] **Step 2: Failing C++ tests of the setter** — in `tests-cpp/src/part-test.cpp` add `#include <stdexcept>` before `#include <string>` and `#include "test-capture.h"` after the `<vector>` include (separated by a blank line), then append:

```cpp
// ====================
// Transposing interval
// ====================

namespace {
// "(<diatonic>, <chromatic>, <doubling>)" of a note.
std::string intervalOf(const Note& note) {
    std::string doubling = "NONE";
    if (note.getOctaveDoubling() == OctaveDoubling::BELOW) {
        doubling = "BELOW";
    } else if (note.getOctaveDoubling() == OctaveDoubling::ABOVE) {
        doubling = "ABOVE";
    }
    return "(" + std::to_string(note.getTransposeDiatonic()) + ", " +
           std::to_string(note.getTransposeChromatic()) + ", " + doubling + ")";
}

// Three measures of a two-staff part; each staff holds a C4 and a rest.
Part clarinets() {
    Part part("Clarinets", 2);
    part.addMeasure(3);
    for (int m = 0; m < 3; m++) {
        for (int s = 0; s < 2; s++) {
            part.getMeasure(m).addNote(Note("C4"), s);
            part.getMeasure(m).addNote(Note("rest"), s);
        }
    }
    return part;
}
}  // namespace

TEST(PartSetTransposingInterval, stampsThePitchedNotesOfTheRangeAndStaff) {
    Part part = clarinets();
    part.setTransposingInterval(-1, -2, 1, 3, 0, OctaveDoubling::BELOW);
    EXPECT_EQ(intervalOf(part.getMeasure(0).getNote(0, 0)), "(0, 0, NONE)");  // before the range
    EXPECT_EQ(intervalOf(part.getMeasure(1).getNote(0, 0)), "(-1, -2, BELOW)");
    EXPECT_EQ(intervalOf(part.getMeasure(2).getNote(0, 0)), "(-1, -2, BELOW)");
    EXPECT_EQ(intervalOf(part.getMeasure(1).getNote(0, 1)), "(0, 0, NONE)");  // the other staff
    EXPECT_FALSE(part.getMeasure(1).getNote(1, 0).isTransposed());            // a rest
    EXPECT_EQ(part.getMeasure(1).getNote(0, 0).getSoundingPitch(), "Bb3");
}

TEST(PartSetTransposingInterval, theDefaultsStampEveryMeasureAndStaff) {
    Part part = clarinets();
    part.setTransposingInterval(-2, -3);
    for (int m = 0; m < 3; m++) {
        for (int s = 0; s < 2; s++) {
            EXPECT_EQ(intervalOf(part.getMeasure(m).getNote(0, s)), "(-2, -3, NONE)")
                << "measure " << m << ", staff " << s;
        }
    }
}

TEST(PartSetTransposingInterval, unpitchedNotesAreLeftAlone) {
    Part part("Percussion");
    part.addMeasure(1);
    Note unpitched("E4");
    unpitched.setIsPitched(false);
    part.getMeasure(0).addNote(unpitched);
    part.setTransposingInterval(-1, -2);
    EXPECT_FALSE(part.getMeasure(0).getNote(0, 0).isTransposed());
}

// Every note is checked before any changes: one that would have no sounding pitch raises,
// naming it, and the notes before it keep their transposition.
TEST(PartSetTransposingInterval, changesAllTheNotesOrNone) {
    Part part("Piccolo");
    part.addMeasure(2);
    part.getMeasure(0).addNote(Note("C4"));
    part.getMeasure(1).addNote(Note("B11"));
    EXPECT_EQ(thrownFirstLine([&] { part.setTransposingInterval(7, 12); }),
              "[maiacore] Part::setTransposingInterval: with the transposing interval (7, 12), "
              "the written B11 at measure index 1, staff index 0 would sound below the lowest "
              "representable pitch, C1b-1, or above B11 (MIDI note 155) where its letter cannot "
              "spell it; no note was changed.");
    EXPECT_FALSE(part.getMeasure(0).getNote(0, 0).isTransposed());

    Part bass("Contrabass");
    bass.addMeasure(1);
    bass.getMeasure(0).addNote(Note("D2"));
    bass.getMeasure(0).addNote(Note("C#-1"));
    EXPECT_NE(thrownFirstLine([&] { bass.setTransposingInterval(-1, -2); })
                  .find("the written C#-1 at measure index 0, staff index 0"),
              std::string::npos);
    EXPECT_FALSE(bass.getMeasure(0).getNote(0, 0).isTransposed());
}

TEST(PartSetTransposingInterval, invalidIndicesThrowOutOfRange) {
    Part part = clarinets();
    EXPECT_THROW(part.setTransposingInterval(-1, -2, -1), std::out_of_range);
    EXPECT_THROW(part.setTransposingInterval(-1, -2, 2, 1), std::out_of_range);
    EXPECT_THROW(part.setTransposingInterval(-1, -2, 0, 4), std::out_of_range);
    EXPECT_THROW(part.setTransposingInterval(-1, -2, 0, -2), std::out_of_range);
    EXPECT_THROW(part.setTransposingInterval(-1, -2, 0, -1, 2), std::out_of_range);
    EXPECT_THROW(part.setTransposingInterval(-1, -2, 0, -1, -2), std::out_of_range);
    EXPECT_NO_THROW(part.setTransposingInterval(-1, -2, 3, 3));  // an empty range
    for (int m = 0; m < 3; m++) {
        EXPECT_FALSE(part.getMeasure(m).getNote(0, 0).isTransposed());
    }
}
```

- [ ] **Step 3: Failing Python tests** — append to `test/test_part_comprehensive.py`, before `if __name__ == "__main__":`:

```python
class PartSetTransposingIntervalTestCase(unittest.TestCase):
    """Part.setTransposingInterval stamps the pitched notes of a measure range and a staff in
    place, which an edit through Measure.getNote(), a copy, cannot do."""

    def clarinets(self):
        """Two measures of a two-staff part; each staff holds a C4 and a rest."""
        score = ml.Score(["Clarinets"], 2)
        part = score.getPart(0)
        part.setNumStaves(2)
        for m in range(2):
            for s in range(2):
                part.getMeasure(m).addNote(ml.Note("C4"), s)
                part.getMeasure(m).addNote(ml.Note("rest"), s)
        return score

    def test_the_edit_reaches_the_score(self):
        score = self.clarinets()
        score.getPart(0).setTransposingInterval(
            -1, -2, measureStart=1, staff=1, doubling=ml.OctaveDoubling.BELOW
        )
        stamped = score.getPart(0).getMeasure(1).getNote(0, 1)
        self.assertEqual(
            (-1, -2, ml.OctaveDoubling.BELOW, "Bb3"),
            (
                stamped.getTransposeDiatonic(),
                stamped.getTransposeChromatic(),
                stamped.getOctaveDoubling(),
                stamped.getSoundingPitch(),
            ),
        )
        self.assertFalse(score.getPart(0).getMeasure(0).getNote(0, 1).isTransposed())
        self.assertFalse(score.getPart(0).getMeasure(1).getNote(0, 0).isTransposed())

    def test_a_note_without_a_sounding_pitch_changes_nothing(self):
        score = ml.Score(["Piccolo"], 2)
        score.getPart(0).getMeasure(0).addNote(ml.Note("C4"))
        score.getPart(0).getMeasure(1).addNote(ml.Note("B11"))
        with self.assertRaises(RuntimeError) as context:
            score.getPart(0).setTransposingInterval(7, 12)
        self.assertIn("the written B11 at measure index 1, staff index 0", str(context.exception))
        self.assertFalse(score.getPart(0).getMeasure(0).getNote(0).isTransposed())

    def test_invalid_indices_raise_index_error(self):
        part = self.clarinets().getPart(0)
        for arguments in (
            {"measureStart": -1},
            {"measureStart": 2, "measureEnd": 1},
            {"measureEnd": 3},
            {"staff": 2},
        ):
            with self.subTest(**arguments), self.assertRaises(IndexError):
                part.setTransposingInterval(-1, -2, **arguments)
```

- [ ] **Step 4: Run and see them fail.** C++ subset `SpelledDiatonicInterval:SoundsWithinRange:PartSetTransposingInterval` (filter `--gtest_filter='SpelledDiatonicInterval.*:SoundsWithinRange.*:PartSetTransposingInterval.*'`) → build error: `no member named 'spelledDiatonicInterval' in namespace 'maiacore::detail'`, `no member named 'soundsWithinRange' in namespace 'maiacore::detail'` and `no member named 'setTransposingInterval' in 'Part'`.

- [ ] **Step 5: The speller's diatonic interval and the predicate.** In `maiacore/include/maiacore/note.h` replace

```cpp
// The spelling the analyses relate a note by. Declared, with its documentation, in the private
// header pitch-views.h next to the sources; not part of the public API.
namespace maiacore::detail {
Pitch concertPitch(const Note& note);
}  // namespace maiacore::detail
```

with

```cpp
// The spelling the analyses relate a note by, and whether a transposing interval leaves a note a
// sounding pitch. Declared, with their documentation, in the private header pitch-views.h next to
// the sources; not part of the public API.
namespace maiacore::detail {
Pitch concertPitch(const Note& note);
bool soundsWithinRange(const Note& note, int transposeDiatonic, int transposeChromatic);
}  // namespace maiacore::detail
```

and replace `    friend Pitch maiacore::detail::concertPitch(const Note& note);` with

```cpp
    friend Pitch maiacore::detail::concertPitch(const Note& note);
    friend bool maiacore::detail::soundsWithinRange(const Note& note, int transposeDiatonic,
                                                    int transposeChromatic);
```

In `maiacore/src/maiacore/pitch-views.h` add `#include <cstdint>` before `#include "maiacore/pitch.h"` (blank line between), and before `}  // namespace maiacore::detail` insert:

```cpp

/**
 * @brief The diatonic interval the speller moves the letter by: transposeDiatonic, or, when it is
 *        0 while transposeChromatic is not, the diatonic interval conventionally written for
 *        those semitones (see concertSpelling()).
 * @details concertSpelling() and soundsWithinRange() spell with it, so that every user of a stored
 *          transposing interval that needs its letters takes them from here and a stored 0 and
 *          the conventional interval it stands for are one transposition.
 * @param transposeDiatonic The stored diatonic interval.
 * @param transposeChromatic The stored chromatic interval.
 * @return The diatonic interval concertSpelling() uses.
 */
std::int64_t spelledDiatonicInterval(int transposeDiatonic, int transposeChromatic);

/**
 * @brief Whether a note would have a sounding pitch with the transposing interval
 *        (transposeDiatonic, transposeChromatic) in place of its own.
 * @details True exactly when concertSpelling() of the note's written pitch with that interval
 *          returns a spelling: false when the sounding position lies below the lowest
 *          representable pitch, C1b-1 (-0.5), or above B11 (MIDI note 155) where the diatonic
 *          interval gives no spelling (a written A#11 moved up a major second sounds B#11, a
 *          written B11 moved up a minor second has no spelling). A rest, and an untransposed
 *          note, always have one. It never throws, so a caller can check many notes before
 *          changing any: the MusicXML reader checks a <transpose>'s whole scope with it, and
 *          Part::setTransposingInterval() every note of its range.
 * @param note The note; only its written pitch is read.
 * @param transposeDiatonic Letters from the written to the sounding pitch; 0 with a non-zero
 *        transposeChromatic stands for the conventional diatonic interval of those semitones.
 * @param transposeChromatic Semitones from the written to the sounding pitch.
 * @return True if the note sounds a spellable pitch with the interval.
 */
bool soundsWithinRange(const Note& note, int transposeDiatonic, int transposeChromatic);
```

In `maiacore/src/maiacore/note.cpp`:
- after the closing `}` of `throwSoundingPitchAboveCeiling` insert:

```cpp

// True when the chromatic rule cannot spell 'writtenPitch' moved by 'transposeChromatic'
// semitones: the position lies above B11 (MIDI note 155), where a default spelling would need
// octave 12.
bool isAboveTheChromaticCeiling(const Pitch& writtenPitch, const int transposeChromatic) {
    const float soundingSteps =
        writtenPitch.getQuarterToneSteps() + static_cast<float>(transposeChromatic);
    const float highestDefaultSpelling = 12.0f * static_cast<float>(c_maxPitchOctave + 1) +
                                         static_cast<float>(c_diatonicStepSemitones.back());
    return soundingSteps > highestDefaultSpelling;
}
```

- in `chromaticSpelling`, replace

```cpp
    const float soundingSteps =
        writtenPitch.getQuarterToneSteps() + static_cast<float>(transposeChromatic);

    // The default spelling reaches no higher than B11 (MIDI note 155): above it, a position would
    // need octave 12 ("C12" for 156) or lies above the representable range. Such a position is
    // rejected naming the note, before it is converted to a whole number of semitones below.
    const float highestDefaultSpelling = 12.0f * static_cast<float>(c_maxPitchOctave + 1) +
                                         static_cast<float>(c_diatonicStepSemitones.back());
    if (soundingSteps > highestDefaultSpelling) {
        throwSoundingPitchAboveCeiling(writtenPitch, transposeDiatonic, transposeChromatic);
    }
```

with

```cpp
    // The default spelling reaches no higher than B11 (MIDI note 155): above it, a position would
    // need octave 12 ("C12" for 156) or lies above the representable range. Such a position is
    // rejected naming the note, before it is converted to a whole number of semitones below.
    if (isAboveTheChromaticCeiling(writtenPitch, transposeChromatic)) {
        throwSoundingPitchAboveCeiling(writtenPitch, transposeDiatonic, transposeChromatic);
    }

    const float soundingSteps =
        writtenPitch.getQuarterToneSteps() + static_cast<float>(transposeChromatic);
```

- inside `namespace maiacore::detail {`, before `Pitch concertSpelling(const Pitch& written, const int transposeDiatonic,` insert:

```cpp
std::int64_t spelledDiatonicInterval(const int transposeDiatonic, const int transposeChromatic) {
    return (transposeDiatonic != 0) ? std::int64_t{transposeDiatonic}
                                    : conventionalDiatonicInterval(transposeChromatic);
}

```

- in `concertSpelling` replace

```cpp
    const std::int64_t diatonic = (transposeDiatonic != 0)
                                      ? std::int64_t{transposeDiatonic}
                                      : conventionalDiatonicInterval(transposeChromatic);
```

with

```cpp
    const std::int64_t diatonic = spelledDiatonicInterval(transposeDiatonic, transposeChromatic);
```

- after the closing `}` of `maiacore::detail::concertSpelling` (before `Pitch simplestSpelling(`) insert:

```cpp

bool soundsWithinRange(const Note& note, const int transposeDiatonic,
                       const int transposeChromatic) {
    const Pitch& written = note._writtenPitch;
    // A rest sounds nothing, and an untransposed note sounds as written.
    if (written.isRest() || (transposeDiatonic == 0 && transposeChromatic == 0)) {
        return true;
    }
    if (isSoundingPitchBelowFloor(written, transposeChromatic)) {
        return false;
    }

    // concertSpelling() returns a spelling when the diatonic interval spells the position, or
    // when its fallback, the chromatic rule, does, which it can up to B11.
    const std::int64_t diatonic = spelledDiatonicInterval(transposeDiatonic, transposeChromatic);
    return diatonicSpelling(written, diatonic, transposeChromatic).has_value() ||
           !isAboveTheChromaticCeiling(written, transposeChromatic);
}
```

- [ ] **Step 6: The setter.** In `maiacore/include/maiacore/part.h` add `#include <string>` after `#include <iostream>`, and `#include "maiacore/constants.h"` after `#include <vector>` (blank line between); after `void setIsPitched(const bool isPitched = true);` insert:

```cpp

    /**
     * @brief Sets the transposing interval and the octave doubling of the pitched notes of a
     *        range of measures and a staff.
     * @details Stamps every pitched note of the measures [measureStart, measureEnd) on the staff
     *          'staff' with Note::setTransposingInterval() and Note::setOctaveDoubling(); rests
     *          and unpitched notes are left alone. The interval is the total one: an octave
     *          transposition is folded in as 7 letters and 12 semitones per octave (a B-flat bass
     *          clarinet is (-8, -14)). The notes hold the transposition -- no Part, Measure or
     *          Score state records it -- and the MusicXML writer derives its `<transpose>`
     *          elements from them. Every note is checked before any changes, so the call changes
     *          all of them or none.
     * @param diatonicInterval Letters from the written to the sounding pitch (-1 for a B-flat
     *        clarinet).
     * @param chromaticInterval Semitones from the written to the sounding pitch (-2 for a
     *        B-flat clarinet).
     * @param measureStart Index of the first measure (default 0).
     * @param measureEnd Index one past the last measure; -1 (default) for the end of the part.
     * @param staff 0-based staff index; -1 (default) for every staff.
     * @param doubling The octave doubling (default OctaveDoubling::NONE).
     * @throws std::out_of_range If the measures are not a range of the part's measures, or the
     *         staff is neither -1 nor one of the part's staves.
     * @throws std::runtime_error If a note would have no sounding pitch with the interval: below
     *         C1b-1, or above B11 where its letter cannot spell it. The message names the first
     *         such note (measure index, staff index, written pitch); no note is changed.
     */
    void setTransposingInterval(const int diatonicInterval, const int chromaticInterval,
                                const int measureStart = 0, const int measureEnd = -1,
                                const int staff = -1,
                                const OctaveDoubling doubling = OctaveDoubling::NONE);
```

In `maiacore/src/maiacore/part.cpp` replace the include block's first line `#include "maiacore/part.h"` with

```cpp
#include "maiacore/part.h"

#include <stdexcept>
#include <string>
#include <vector>
```

add `#include "pitch-views.h"` after `#include "maiacore/utils.h"`, and after the closing `}` of `Part::setIsPitched` insert:

```cpp

void Part::setTransposingInterval(const int diatonicInterval, const int chromaticInterval,
                                  const int measureStart, const int measureEnd, const int staff,
                                  const OctaveDoubling doubling) {
    const int numMeasures = getNumMeasures();
    const int end = (measureEnd == -1) ? numMeasures : measureEnd;
    if (measureStart < 0 || end < measureStart || end > numMeasures) {
        throw std::out_of_range("Part::setTransposingInterval: the measures [" +
                                std::to_string(measureStart) + ", " + std::to_string(measureEnd) +
                                ") are not a range of the part's " + std::to_string(numMeasures) +
                                " measures");
    }
    if (staff < -1 || staff >= _numStaves) {
        throw std::out_of_range("Part::setTransposingInterval: staff " + std::to_string(staff) +
                                " is neither -1 nor one of the part's " +
                                std::to_string(_numStaves) + " staves");
    }

    // Every pitched note of the range is checked before any is changed, so a note that cannot
    // sound with the interval leaves the part as it was.
    std::vector<Note*> notes;
    for (int m = measureStart; m < end; m++) {
        Measure& measure = _measure[m];
        for (int s = 0; s < measure.getNumStaves(); s++) {
            if (staff != -1 && s != staff) {
                continue;
            }
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                Note& note = measure.getNote(n, s);
                if (!note.isNoteOn() || !note.isPitched()) {
                    continue;
                }
                if (!maiacore::detail::soundsWithinRange(note, diatonicInterval,
                                                         chromaticInterval)) {
                    LOG_ERROR("Part::setTransposingInterval: with the transposing interval (" +
                              std::to_string(diatonicInterval) + ", " +
                              std::to_string(chromaticInterval) + "), the written " +
                              note.getWrittenPitch() + " at measure index " + std::to_string(m) +
                              ", staff index " + std::to_string(s) +
                              " would sound below the lowest representable pitch, C1b-1, or "
                              "above B11 (MIDI note 155) where its letter cannot spell it; no "
                              "note was changed.");
                }
                notes.push_back(&note);
            }
        }
    }

    for (Note* note : notes) {
        note->setTransposingInterval(diatonicInterval, chromaticInterval);
        note->setOctaveDoubling(doubling);
    }
}
```

In `maiacore/src/maiacore/python_wrapper/py_part.cpp`, after `    cls.def("setIsPitched", &Part::setIsPitched, py::arg("isPitched") = true);` insert:

```cpp
    cls.def("setTransposingInterval", &Part::setTransposingInterval,
            py::arg("diatonicInterval"), py::arg("chromaticInterval"),
            py::arg("measureStart") = 0, py::arg("measureEnd") = -1, py::arg("staff") = -1,
            py::arg("doubling") = OctaveDoubling::NONE,
            R"pbdoc(
        Set the transposing interval and the octave doubling of the pitched notes of a range of
        measures and a staff, in place.

        Every pitched note of the measures ``measureStart`` up to, not including, ``measureEnd``
        on the staff ``staff`` gets the interval (see ``Note.setTransposingInterval``) and the
        doubling (see ``Note.setOctaveDoubling``); rests and unpitched notes are left alone. The
        interval is the total one: an octave transposition is folded in as 7 letters and 12
        semitones per octave, so a B-flat bass clarinet is ``(-8, -14)``. The notes hold the
        transposition, and the MusicXML export writes its ``<transpose>`` elements from them.
        This is the way to change a score's transpositions in place: ``Measure.getNote()``
        returns a copy of the note, so a change made on it does not reach the score
        (``Score.forEachNote`` edits in place too). Every note is checked before any changes, so
        the call changes all of them or none.

        Parameters
        ----------
        diatonicInterval : int
            Letters from the written to the sounding pitch, e.g. -1 for a B-flat clarinet.
        chromaticInterval : int
            Semitones from the written to the sounding pitch, e.g. -2 for a B-flat clarinet.
        measureStart : int, default 0
            Index of the first measure.
        measureEnd : int, default -1
            Index one past the last measure; -1 for the end of the part.
        staff : int, default -1
            Zero-based staff index; -1 for every staff.
        doubling : OctaveDoubling, default OctaveDoubling.NONE
            The octave doubling.

        Raises
        ------
        IndexError
            If the measures are not a range of the part's measures, or ``staff`` is neither -1
            nor one of its staves.
        RuntimeError
            If a note would have no sounding pitch with the interval (below ``C1b-1``, or above
            ``B11`` where its letter cannot spell it); the message names the first such note,
            and no note is changed.

        Examples
        --------
        >>> score = ml.Score(["Clarinet"], 2)
        >>> for m in range(2):
        ...     score.getPart(0).getMeasure(m).addNote(ml.Note("D4"))
        >>> score.getPart(0).setTransposingInterval(-1, -2, measureStart=1)
        >>> [score.getPart(0).getMeasure(m).getNote(0).getSoundingPitch() for m in range(2)]
        ['D4', 'C4']
    )pbdoc");
```

- [ ] **Step 7: Format, build, pass.** clang-format the eight changed C++ files. C++ subset `SpelledDiatonicInterval.*:SoundsWithinRange.*:PartSetTransposingInterval.*` → 8 tests pass, and the subset `ConcertSpelling.*` stays green. «build» `make "PYTHON=$py" dev`; «pytest» `test_part_comprehensive.PartSetTransposingIntervalTestCase` → 3 OK.

- [ ] **Step 8: Mutations.** (a) In `soundsWithinRange` replace `return false;` (below the floor) with `return true;` → `SoundsWithinRange.isTrueExactlyWhereTheConcertSpellingExists` (C#-1 with (-1, -2)) and `theEdgesOfTheRange` fail. (b) Return `!isAboveTheChromaticCeiling(written, transposeChromatic);` alone (drop the diatonic branch) → both fail on A#11 (1, 2). (c) In `Part::setTransposingInterval` stamp inside the checking loop (call `setTransposingInterval`/`setOctaveDoubling` right after the check, and drop the second loop) → `changesAllTheNotesOrNone` fails (C4 transposed before B11 raises); after `make dev`, `test_a_note_without_a_sounding_pitch_changes_nothing` fails. (d) Drop `if (staff != -1 && s != staff) { continue; }` → `stampsThePitchedNotesOfTheRangeAndStaff` and `test_the_edit_reaches_the_score` fail. (e) Drop `|| !note.isPitched()` → `unpitchedNotesAreLeftAlone` fails. (f) Replace `staff >= _numStaves` with `staff > _numStaves` → `invalidIndicesThrowOutOfRange` fails (staff 2 of a two-staff part no longer throws); after `make dev`, `test_invalid_indices_raise_index_error` fails for `{"staff": 2}`. (g) Replace `const int end = (measureEnd == -1) ? numMeasures : measureEnd;` with `const int end = measureEnd;` (the default -1 no longer means the end of the part) → `theDefaultsStampEveryMeasureAndStaff` fails (the default call throws `std::out_of_range`). (h) In `spelledDiatonicInterval` return `transposeDiatonic;` alone → `SpelledDiatonicInterval.isTheStoredIntervalOrTheConventionalOneForZero` fails (`(0, -2)` gives 0). Revert each; rerun green.

- [ ] **Step 9: Whole suites.** `make cpp-tests`, `make py-tests`, `make validate` → 0, no new findings.

- [ ] **Step 10: Commit.** `git add maiacore/include/maiacore/note.h maiacore/src/maiacore/pitch-views.h maiacore/src/maiacore/note.cpp maiacore/include/maiacore/part.h maiacore/src/maiacore/part.cpp maiacore/src/maiacore/python_wrapper/py_part.cpp tests-cpp/src/pitch-views-test.cpp tests-cpp/src/part-test.cpp test/test_part_comprehensive.py`, message:

```
feat: Part.setTransposingInterval stamps a range of measures and a staff, all or none

The interval and the octave doubling go on every pitched note of the
range; a note that would have no sounding pitch raises, naming it, before
any note changes. soundsWithinRange() answers that question without
throwing, exactly where concertSpelling() would raise; both take the
letters from spelledDiatonicInterval().

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 3: The reader — every measure, per staff, values

**Files:**
- Create: `test/xml_examples/unit_test/transpose_change_mid_part.musicxml`, `transpose_later_measure.musicxml`, `transpose_per_staff.musicxml`, `transpose_octave_change.musicxml`, `transpose_double.musicxml`, `transpose_after_notes.musicxml`, `transpose_inside_chord.musicxml`, `transpose_without_diatonic.musicxml`, `transpose_chromatic_not_integer.musicxml`, `transpose_octave_change_not_integer.musicxml` (schema-invalid: `<octave-change>1.5</octave-change>`)
- Modify: `maiacore/src/maiacore/pitch-views.h` (Doxygen ~23-26; a declaration), `maiacore/src/maiacore/note.cpp` (`conventionalDiatonicInterval` ~308-323 moved; comment ~378-381), `maiacore/src/maiacore/score.cpp` (includes ~3-16; anonymous namespace ~29-84; STEP 2 ~448-476; the measure loop ~626; the note loop ~792 and ~862; after the measure loop ~864), `maiacore/include/maiacore/score.h` (`Score(path)` Doxygen ~130-139), `maiacore/include/maiacore/note.h` (`getSoundingPitch` Doxygen ~467-471), `maiacore/src/maiacore/python_wrapper/py_note.cpp` (`getSoundingPitch` docstring ~597-600), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (`Score(path)` docstring ~29-38; `getChords` docstring ~475-477), `test/musicxml/ledger.json` (10 added lines)
- Test: `tests-cpp/src/score-test.cpp` (append), `tests-cpp/src/pitch-views-test.cpp` (append)

**Interfaces:** consumes `OctaveDoubling`, `Note::setOctaveDoubling`, `Note::getOctaveDoubling` (Task 1). Produces `std::int64_t maiacore::detail::conventionalDiatonicInterval(int transposeChromatic)` (moved out of `note.cpp`'s anonymous namespace; consumed by Tasks 4 and 5) and, in `score.cpp`'s anonymous namespace, `struct NoteTransposition { int diatonic; int chromatic; OctaveDoubling doubling; }`, `struct TransposeElement { int measureIdx; int notesBefore; int staff; std::string where; std::optional<NoteTransposition> values; std::string ignored; }`, `struct PitchedNote { int measureIdx; int chordPosition; int staff; int index; }` (a chord is read as a unit: `chordPosition` is the position of its first note), `std::string trimmed(const std::string&)`, `std::string trimmedChildText(const pugi::xml_node&, const char*)`, `std::optional<std::int64_t> wholeNumber(const std::string&)`, `int toIntRange(std::int64_t)`, `TransposeElement readTranspose(const pugi::xml_node&, int measureIdx, int notesBefore, const std::string& where)`, `void readTransposeElements(const pugi::xml_node& measure, int measureIdx, const std::string& where, std::vector<TransposeElement>& elements)`, `bool isAtOrAfter(const PitchedNote&, const TransposeElement&)`, `void applyTranspositions(Part&, const std::vector<TransposeElement>&, const std::vector<PitchedNote>&)` — all extended by Task 4; and the score-test helpers `doublingName`, `describeTransposition`, `transposedNotes`, `transposeWarnings`, `kUnitTest`, `kW3c`, `kSamples` (consumed by Tasks 4, 7, 8).

- [ ] **Step 1: The fixtures.** Create each file under `test/xml_examples/unit_test/` with exactly this content.

`transpose_change_mid_part.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A clarinet part that changes its transposition in the middle of the part, then plays
     untransposed: in B-flat (-1, -2) in measure 1, in A (-2, -3) from measure 2, carried into
     measure 3, untransposed (0, 0) from measure 4. Expected: C4 sounds Bb3; C4 sounds A3 and D4
     sounds B3; C4 sounds C4. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Clarinet</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose><diatonic>-2</diatonic><chromatic>-3</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="3">
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="4">
      <attributes>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_later_measure.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A horn part whose only <transpose> comes in measure 2, where a rest precedes the first
     pitched note: measure 1 is untransposed (C4 sounds C4); from measure 2 on, a horn in F
     (-4, -7): C4 sounds F3 and G4 sounds C4. The rest and the unpitched note take no
     transposition. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Horn</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose><diatonic>-4</diatonic><chromatic>-7</chromatic></transpose>
      </attributes>
      <note><rest/><duration>2</duration><voice>1</voice><type>half</type></note>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
    </measure>
    <measure number="3">
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
      <note><unpitched><display-step>E</display-step><display-octave>4</display-octave></unpitched><duration>2</duration><voice>1</voice><type>half</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_per_staff.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- Two clarinets on two staves of one part. Measure 1: staff 1 in B-flat (-1, -2) and staff 2
     in A (-2, -3), each by its number; measure 2: staff 2 alone becomes a horn in F (-4, -7);
     measure 3: a <transpose> without number puts both staves in B-flat. Expected: C5 sounds Bb4
     and A4; D5 sounds C5 and G4; E5 sounds D5 on both staves. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Clarinets</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <staves>2</staves>
        <clef number="1"><sign>G</sign><line>2</line></clef>
        <clef number="2"><sign>G</sign><line>2</line></clef>
        <transpose number="1"><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
        <transpose number="2"><diatonic>-2</diatonic><chromatic>-3</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>4</duration><voice>2</voice><type>whole</type><staff>2</staff></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose number="2"><diatonic>-4</diatonic><chromatic>-7</chromatic></transpose>
      </attributes>
      <note><pitch><step>D</step><octave>5</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>D</step><octave>5</octave></pitch><duration>4</duration><voice>2</voice><type>whole</type><staff>2</staff></note>
    </measure>
    <measure number="3">
      <attributes>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>E</step><octave>5</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type><staff>1</staff></note>
      <backup><duration>4</duration></backup>
      <note><pitch><step>E</step><octave>5</octave></pitch><duration>4</duration><voice>2</voice><type>whole</type><staff>2</staff></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_octave_change.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- <octave-change> folded into the interval: a piccolo (0, 0, +1) whose G5 sounds G6, a B-flat
     bass clarinet (-1, -2, -1) whose D4 sounds C3, and a contrabass (0, 0, -1) whose C3 sounds
     C2. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Piccolo</part-name></score-part>
    <score-part id="P2"><part-name>Bass Clarinet</part-name></score-part>
    <score-part id="P3"><part-name>Contrabass</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><octave-change>1</octave-change></transpose>
      </attributes>
      <note><pitch><step>G</step><octave>5</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P2">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic><octave-change>-1</octave-change></transpose>
      </attributes>
      <note><pitch><step>D</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P3">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>F</sign><line>4</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><octave-change>-1</octave-change></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>3</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_double.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- <double>: a cello part doubled one octave below (<double/>) and a flute part doubled one
     octave above (<double above="yes"/>). Each note sounds as written and holds the doubling:
     C3 BELOW, G4 ABOVE. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Violoncello and Contrabass</part-name></score-part>
    <score-part id="P2"><part-name>Flute and Piccolo</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>F</sign><line>4</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><double/></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>3</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P2">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><double above="yes"/></transpose>
      </attributes>
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_after_notes.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- Document order decides inside a measure: the flute becomes an alto flute (-3, -5) after the
     second note of measure 1, so C5 and D5 sound as written and E5 and F5 sound B4 and C5; G5 in
     measure 2 sounds D5; a <transpose> after the last note of measure 2 returns to untransposed
     from measure 3, where C5 sounds C5. -->
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
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>D</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <attributes>
        <transpose><diatonic>-3</diatonic><chromatic>-5</chromatic></transpose>
      </attributes>
      <note><pitch><step>E</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
      <note><pitch><step>F</step><octave>5</octave></pitch><duration>1</duration><voice>1</voice><type>quarter</type></note>
    </measure>
    <measure number="2">
      <note><pitch><step>G</step><octave>5</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
      <attributes>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic></transpose>
      </attributes>
    </measure>
    <measure number="3">
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_inside_chord.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A chord is read as a unit: a <transpose> written between the notes of a chord applies from
     the first note after the chord. The clarinet is in B-flat (-1, -2) from measure 1; its change
     to A (-2, -3) stands between the E5 and the G5 of the chord C5-E5-G5, so the whole chord
     sounds Bb4-D5-F5, and the D5 after it sounds B4. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Clarinet</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>5</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
      <note><chord/><pitch><step>E</step><octave>5</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
      <attributes>
        <transpose><diatonic>-2</diatonic><chromatic>-3</chromatic></transpose>
      </attributes>
      <note><chord/><pitch><step>G</step><octave>5</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
      <note><pitch><step>D</step><octave>5</octave></pitch><duration>2</duration><voice>1</voice><type>half</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_without_diatonic.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A <transpose> without <diatonic>: the conventional diatonic interval for <chromatic>-2 is
     stored, (-1, -2), without a warning, so F#4 sounds E4. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Trumpet in Bb</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>F</step><alter>1</alter><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_chromatic_not_integer.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A <chromatic> that is not a whole number of semitones (valid: the schema types it
     xs:decimal, for microtones) is ignored with a warning, and the previous transposition stays
     in force: both C4 sound Bb3. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Clarinet</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose><diatonic>-1</diatonic><chromatic>-2.5</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_octave_change_not_integer.musicxml` (invalid against the schema on purpose: `<octave-change>` is `xs:integer`):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- An <octave-change> that is not a whole number (invalid against the schema, which types it
     xs:integer) is ignored with a warning, and the previous transposition stays in force: both
     C4 sound Bb3. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Clarinet</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose><diatonic>-1</diatonic><chromatic>-2</chromatic><octave-change>1.5</octave-change></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

Check them: Bash `(cd /c/Users/nyck/Desktop/maialib && /c/Users/nyck/AppData/Local/Temp/maialib-1b-venv/Scripts/python.exe test/musicxml/musicxml_check.py test/xml_examples/unit_test/transpose_*.musicxml); echo "exit $?"` → every file `valid; errors: none` except `transpose_octave_change_not_integer.musicxml`, `invalid (1 XSD errors)` naming `octave-change`; exit 1 because of that one file.

- [ ] **Step 2: Failing tests.** Append to `tests-cpp/src/pitch-views-test.cpp` (and add `using maiacore::detail::conventionalDiatonicInterval;` beside the other `using` lines; add `#include <cstdint>` after `#include <cstddef>`):

```cpp
// The diatonic interval conventionally written for a number of semitones: seven letters per whole
// octave, plus a second for 1 or 2, a third for 3 or 4, a fourth for 5 and the tritone, a fifth
// for 7, a sixth for 8 or 9, a seventh for 10 or 11, in the direction of the semitones.
TEST(ConventionalDiatonicInterval, isTheLetterCountOfTheConventionalInterval) {
    const std::vector<std::pair<int, std::int64_t>> rows = {
        {0, 0},  {1, 1},   {-2, -1}, {-3, -2}, {6, 3},    {-6, -3},
        {-7, -4}, {-9, -5}, {12, 7},  {-14, -8}, {-26, -15}};
    for (const auto& [chromatic, letters] : rows) {
        EXPECT_EQ(conventionalDiatonicInterval(chromatic), letters) << chromatic;
    }
    EXPECT_EQ(conventionalDiatonicInterval(std::numeric_limits<int>::min()), -1252698795);
}
```

In `tests-cpp/src/score-test.cpp` add `#include <set>` after `#include <mutex>`, then append at the end of the file:

```cpp
// ====================
// MusicXML <transpose>
// ====================

namespace {
const std::string kUnitTest = "./test/xml_examples/unit_test/";
const std::string kW3c = "./test/musicxml/w3c-test-suite/xmlFiles/";
const std::string kSamples = "./maialib/xml-scores-examples/";

std::string doublingName(const OctaveDoubling doubling) {
    switch (doubling) {
        case OctaveDoubling::BELOW:
            return "BELOW";
        case OctaveDoubling::ABOVE:
            return "ABOVE";
        default:
            return "NONE";
    }
}

// A note as "<written> (<diatonic>, <chromatic>, <doubling>) <sounding>".
std::string describeTransposition(const Note& note) {
    return note.getWrittenPitch() + " (" + std::to_string(note.getTransposeDiatonic()) + ", " +
           std::to_string(note.getTransposeChromatic()) + ", " +
           doublingName(note.getOctaveDoubling()) + ") " + note.getSoundingPitch();
}

// Every pitched note of a part, measure by measure and staff by staff, as describeTransposition()
// writes it.
std::vector<std::string> transposedNotes(Score& score, const int partId) {
    std::vector<std::string> notes;
    Part& part = score.getPart(partId);
    for (int m = 0; m < part.getNumMeasures(); m++) {
        const Measure& measure = part.getMeasure(m);
        for (int s = 0; s < measure.getNumStaves(); s++) {
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                const Note& note = measure.getNote(n, s);
                if (note.isNoteOn() && note.isPitched()) {
                    notes.push_back(describeTransposition(note));
                }
            }
        }
    }
    return notes;
}

// How many warnings of the <transpose> reader 'printed' holds.
int transposeWarnings(const std::string& printed) {
    int count = 0;
    for (const char* code : {"[WARN] [transpose-", "[WARN] [for-part-"}) {
        for (size_t at = printed.find(code); at != std::string::npos;
             at = printed.find(code, at + 1)) {
            count++;
        }
    }
    return count;
}
}  // namespace

// A <transpose> applies from where it stands to the next one: a change in the middle of a part is
// followed and carried forward, and <diatonic>0</diatonic><chromatic>0</chromatic> returns to
// untransposed.
TEST(ScoreTransposeRead, aChangeInTheMiddleOfAPartIsFollowed) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_change_mid_part.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-2, -3, NONE) A3",
                                        "D4 (-2, -3, NONE) B3", "C4 (0, 0, NONE) C4"}));
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A part whose first <transpose> comes in a later measure is untransposed up to it; rests and
// unpitched notes take no transposition.
TEST(ScoreTransposeRead, aTransposeInALaterMeasureAppliesFromThere) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_later_measure.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (0, 0, NONE) C4", "C4 (-4, -7, NONE) F3",
                                        "G4 (-4, -7, NONE) C4"}));
    const Note& rest = score.getPart(0).getMeasure(1).getNote(0, 0);
    ASSERT_TRUE(rest.isNoteOff());
    EXPECT_FALSE(rest.isTransposed());
    const Note& unpitched = score.getPart(0).getMeasure(2).getNote(1, 0);
    ASSERT_FALSE(unpitched.isPitched());
    EXPECT_FALSE(unpitched.isTransposed());
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A <transpose number="s"> applies to staff s alone; one without number to every staff.
TEST(ScoreTransposeRead, aNumberedTransposeAppliesToItsStaffAlone) {
    Score score(kUnitTest + "transpose_per_staff.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C5 (-1, -2, NONE) Bb4", "C5 (-2, -3, NONE) A4",
                                        "D5 (-1, -2, NONE) C5", "D5 (-4, -7, NONE) G4",
                                        "E5 (-1, -2, NONE) D5", "E5 (-1, -2, NONE) D5"}));
}

// <octave-change> is folded into the interval: 7 letters and 12 semitones per octave.
TEST(ScoreTransposeRead, anOctaveChangeIsFoldedIntoTheInterval) {
    Score score(kUnitTest + "transpose_octave_change.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"G5 (7, 12, NONE) G6"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"D4 (-8, -14, NONE) C3"}));
    EXPECT_EQ(transposedNotes(score, 2), (std::vector<std::string>{"C3 (-7, -12, NONE) C2"}));
}

// <double/> doubles one octave below, <double above="yes"/> one octave above; the note itself
// sounds as written.
TEST(ScoreTransposeRead, doubleIsReadAsTheOctaveDoubling) {
    Score score(kUnitTest + "transpose_double.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"C3 (0, 0, BELOW) C3"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"G4 (0, 0, ABOVE) G4"}));
}

// Inside a measure, document order decides: a <transpose> after notes applies to the notes after
// it, and one after the measure's last note from the next measure on.
TEST(ScoreTransposeRead, aTransposeAfterNotesAppliesToTheNotesAfterIt) {
    Score score(kUnitTest + "transpose_after_notes.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C5 (0, 0, NONE) C5", "D5 (0, 0, NONE) D5",
                                        "E5 (-3, -5, NONE) B4", "F5 (-3, -5, NONE) C5",
                                        "G5 (-3, -5, NONE) D5", "C5 (0, 0, NONE) C5"}));
}

// A chord is read as a unit: a <transpose> written between the notes of a chord applies from the
// first note after the chord, and every note of the chord keeps the transposition in force at its
// first note.
TEST(ScoreTransposeRead, aTransposeBetweenTheNotesOfAChordAppliesAfterTheChord) {
    Score score(kUnitTest + "transpose_inside_chord.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C5 (-1, -2, NONE) Bb4", "E5 (-1, -2, NONE) D5",
                                        "G5 (-1, -2, NONE) F5", "D5 (-2, -3, NONE) B4"}));
}

// Without <diatonic>, the conventional diatonic interval of <chromatic> is stored, silently.
TEST(ScoreTransposeRead, aTransposeWithoutDiatonicStoresTheConventionalInterval) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_without_diatonic.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"F#4 (-1, -2, NONE) E4"}));
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A <chromatic> that is not a whole number is ignored with one warning; the previous
// transposition stays in force.
TEST(ScoreTransposeRead, aChromaticThatIsNotAWholeNumberIsIgnored) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_chromatic_not_integer.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3"}));
    EXPECT_NE(printed.find("[WARN] [transpose-chromatic-not-integer] part \"Clarinet\", measure 2: "
                           "<chromatic>-2.5</chromatic> is not a whole number of semitones; the "
                           "<transpose> is ignored and the previous transposition stays in "
                           "force.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// Likewise an <octave-change> that is not a whole number.
TEST(ScoreTransposeRead, anOctaveChangeThatIsNotAWholeNumberIsIgnored) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_octave_change_not_integer.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3"}));
    EXPECT_NE(printed.find("[WARN] [transpose-octave-change-not-integer] part \"Clarinet\", "
                           "measure 2: <octave-change>1.5</octave-change> is not a whole number "
                           "of octaves; the <transpose> is ignored and the previous "
                           "transposition stays in force.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// The W3C transposing-instrument files sound as they say: 72c's change from E-flat to B-flat
// clarinet in measure 2 is followed, and 72d's part transposed by an augmented fourth and three
// octaves sounds MIDI note 72 from a written F#1.
TEST(ScoreTransposeRead, theW3cTransposingInstrumentsSoundAsTheFilesSay) {
    Score a(kW3c + "72a-TransposingInstruments.musicxml");
    EXPECT_EQ(describeTransposition(a.getPart(0).getMeasure(0).getNote(0, 0)),
              "D4 (-1, -2, NONE) C4");
    EXPECT_EQ(describeTransposition(a.getPart(1).getMeasure(0).getNote(0, 0)),
              "A4 (-5, -9, NONE) C4");

    Score c(kW3c + "72c-TransposingInstruments-Change.musicxml");
    EXPECT_EQ(transposedNotes(c, 0),
              (std::vector<std::string>{"C4 (2, 3, NONE) Eb4", "C4 (-1, -2, NONE) Bb3",
                                        "C4 (-1, -2, NONE) Bb3"}));

    Score d(kW3c + "72d-TransposingInstruments-scorePitch.musicxml");
    const Note& displayed = d.getPart(9).getMeasure(0).getNote(0, 0);
    EXPECT_EQ(describeTransposition(displayed), "F#1 (24, 42, NONE) C5");
    EXPECT_EQ(displayed.getMidiNumber(), 72);
}

// W3C 41c's piccolo, bass clarinet, contrabassoon and contrabass sound an octave from what they
// read.
TEST(ScoreTransposeRead, theOctaveTransposingPartsOf41cSoundAnOctaveAway) {
    Score score(kW3c + "41c-StaffGroups.musicxml");
    EXPECT_EQ(describeTransposition(score.getPart(0).getMeasure(0).getNote(0, 0)),
              "B4 (7, 12, NONE) B5");
    EXPECT_EQ(describeTransposition(score.getPart(6).getMeasure(0).getNote(0, 0)),
              "B4 (-8, -14, NONE) A3");
    EXPECT_EQ(describeTransposition(score.getPart(8).getMeasure(0).getNote(0, 0)),
              "B2 (-7, -12, NONE) B1");
    EXPECT_EQ(describeTransposition(score.getPart(24).getMeasure(0).getNote(0, 0)),
              "C3 (-7, -12, NONE) C2");
}

// The samples' contrabasses sound an octave below what they read; their other transposing parts
// read as before.
TEST(ScoreTransposeRead, theSamplesContrabassesSoundAnOctaveLower) {
    Score beethoven(kSamples + "Beethoven_Symphony_5_mov_1.xml");
    EXPECT_EQ(describeTransposition(beethoven.getPart(11).getMeasure(0).getNote(1, 0)),
              "G3 (-7, -12, NONE) G2");
    EXPECT_EQ(describeTransposition(beethoven.getPart(2).getMeasure(0).getNote(1, 0)),
              "A4 (-1, -2, NONE) G4");
    EXPECT_EQ(describeTransposition(beethoven.getPart(4).getMeasure(17).getNote(1, 0)),
              "E5 (-5, -9, NONE) G4");

    Score dvorak(kSamples + "Dvorak_Symphony_9_mov_4.mxl");
    EXPECT_EQ(describeTransposition(dvorak.getPart(15).getMeasure(0).getNote(0, 0)),
              "B2 (-7, -12, NONE) B1");
    EXPECT_EQ(describeTransposition(dvorak.getPart(2).getMeasure(7).getNote(1, 0)),
              "Db5 (-2, -3, NONE) Bb4");
}

// Reading <octave-change> changes the chords: test_getchords_poly's contrabass sounds C2 under the
// first chord, and its G2 makes measure 3's first chord a G7 in root position, where its G3 had
// made a G7/F.
TEST(ScoreTransposeRead, anOctaveTranspositionChangesTheChords) {
    Score score(kUnitTest + "test_getchords_poly.musicxml");
    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 9u);
    Chord first = std::get<3>(chords[0]);
    EXPECT_EQ(first.getBassNote().getPitch(), "C2");
    Chord dominant = std::get<3>(chords[5]);
    EXPECT_EQ(dominant.getName(), "G7");
}
```

- [ ] **Step 3: Run them and see them fail.** C++ subset `ConventionalDiatonicInterval.*:ScoreTransposeRead.*` → the build fails: `no member named 'conventionalDiatonicInterval' in namespace 'maiacore::detail'`. After Step 4 (which exposes it) and before Step 5, run the subset again: it builds, `ConventionalDiatonicInterval` passes, and the reader tests fail on today's reader — `aChangeInTheMiddleOfAPartIsFollowed` gets `{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3", "D4 (-1, -2, NONE) C4", "C4 (-1, -2, NONE) Bb3"}`, `anOctaveChangeIsFoldedIntoTheInterval` gets `"G5 (0, 0, NONE) G5"`, `aTransposeBetweenTheNotesOfAChordAppliesAfterTheChord` gets `"D5 (-1, -2, NONE) C5"` for the last note, `aChromaticThatIsNotAWholeNumberIsIgnored` finds no warning.

- [ ] **Step 4: Expose the conventional diatonic interval.** In `maiacore/src/maiacore/note.cpp`, cut the whole definition of `std::int64_t conventionalDiatonicInterval(const int transposeChromatic)` with its comment (the block from `// The diatonic interval conventionally written for a transposing interval of 'transposeChromatic'` to its closing `}`) out of the anonymous namespace, and paste it, unchanged, as the first definition inside `namespace maiacore::detail {` (before `std::int64_t spelledDiatonicInterval(`, which calls it). In `concertSpelling` replace the comment

```cpp
    // A transposing interval given only in semitones -- a MusicXML <transpose> without <diatonic>,
    // or a note given only a chromatic interval -- moves the letter by the diatonic interval
    // conventionally written for those semitones. A diatonic interval that is given is used as it
    // is, even one that disagrees with the chromatic interval.
```

with

```cpp
    // A transposing interval given only in semitones -- a note given only a chromatic interval --
    // moves the letter by the diatonic interval conventionally written for those semitones. A
    // diatonic interval that is given is used as it is, even one that disagrees with the chromatic
    // interval.
```

In `maiacore/src/maiacore/pitch-views.h` (which includes `<cstdint>` since Task 2) replace

```cpp
 *          Inferred diatonic interval: when transposeDiatonic is 0 while transposeChromatic is
 *          not (a MusicXML `<transpose>` without `<diatonic>`, or a Note given only a chromatic
 *          interval), the letter is moved by the diatonic interval conventionally written for
 *          those semitones: 7 letters for each whole octave plus, for the semitones left over, 1
```

with

```cpp
 *          Inferred diatonic interval: when transposeDiatonic is 0 while transposeChromatic is
 *          not (a Note given only a chromatic interval; the MusicXML reader stores the
 *          conventional interval itself), the letter is moved by the diatonic interval
 *          conventionally written for those semitones: 7 letters for each whole octave plus, for
 *          the semitones left over, 1
```

and before `}  // namespace maiacore::detail` insert:

```cpp

/**
 * @brief The diatonic interval conventionally written for a transposing interval of
 *        transposeChromatic semitones.
 * @details Seven letters for each whole octave, plus the letters of the simple interval left
 *          over -- 1 for 1 or 2 semitones (a second), 2 for 3 or 4 (a third), 3 for 5 or 6 (a
 *          fourth, the tritone being an augmented fourth), 4 for 7 (a fifth), 5 for 8 or 9 (a
 *          sixth) and 6 for 10 or 11 (a seventh) -- in the direction of the chromatic interval:
 *          -2 gives -1 (a B-flat clarinet), -9 gives -5 (an E-flat alto saxophone), 12 gives 7 (a
 *          piccolo). concertSpelling() moves the letter by it when the diatonic interval is 0,
 *          and the MusicXML reader stores it for a `<transpose>` without `<diatonic>`.
 * @param transposeChromatic Semitones from the written to the sounding pitch.
 * @return The number of letters, with the sign of transposeChromatic; it always fits an int.
 */
std::int64_t conventionalDiatonicInterval(int transposeChromatic);
```

In `maiacore/include/maiacore/note.h` replace

```cpp
     *          getTransposeDiatonic() is 0 while getTransposeChromatic() is not -- a MusicXML
     *          `<transpose>` without `<diatonic>`, or a note given only a chromatic interval --
     *          the letter is moved by the diatonic interval conventionally written for those
```

with

```cpp
     *          getTransposeDiatonic() is 0 while getTransposeChromatic() is not -- a note given
     *          only a chromatic interval; the MusicXML reader stores the conventional interval
     *          itself for a `<transpose>` without `<diatonic>` -- the letter is moved by the
     *          diatonic interval conventionally written for those
```

In `maiacore/src/maiacore/python_wrapper/py_note.cpp` replace

```
        while ``transposeChromatic`` is not -- a MusicXML ``<transpose>`` without ``<diatonic>``,
        or a note given only ``transposeChromatic`` -- the letter is moved by the diatonic
        interval conventionally written for those semitones: 7 letters for each whole octave,
```

with

```
        while ``transposeChromatic`` is not -- a note given only ``transposeChromatic``; the
        MusicXML reader stores the conventional interval for a ``<transpose>`` without
        ``<diatonic>`` -- the letter is moved by the diatonic interval conventionally written for
        those semitones: 7 letters for each whole octave,
```

In `maiacore/src/maiacore/python_wrapper/py_score.cpp` (the `getChords` docstring) replace

```
        transposing interval -- inferred from the chromatic one for a ``<transpose>`` without
        ``<diatonic>`` -- or, where that gives no spelling, by the fallback described on
```

with

```
        transposing interval -- inferred from the chromatic one when it is 0 -- or, where that
        gives no spelling, by the fallback described on
```

- [ ] **Step 5: The reader's helpers.** In `maiacore/src/maiacore/score.cpp` add `#include <cstdint>` after `#include <atomic>`, `#include <map>` after `#include <locale>`, `#include <string>` after `#include <sstream>`, and `#include <utility>` after `#include <tuple>`. Inside the anonymous namespace, after the closing `}` of `rejectQuarterToneTransposition`, insert:

```cpp

// A transposing interval -- <octave-change> folded in -- and an octave doubling, as a
// <transpose> stamps them on the notes of its staves.
struct NoteTransposition {
    int diatonic = 0;
    int chromatic = 0;
    OctaveDoubling doubling = OctaveDoubling::NONE;
};

// A <transpose> of a part as the reader met it: where it is, the staves it applies to, and what
// it stands for.
struct TransposeElement {
    int measureIdx = 0;   // the index of its measure in the part
    int notesBefore = 0;  // the <note> elements before its <attributes> in that measure
    int staff = -1;       // the 0-based staff its number attribute names; -1 for every staff
    std::string where;    // part "<name>", measure <number as the file writes it>
    // What it stamps; empty when it is ignored, and 'ignored' is then the warning that says why.
    std::optional<NoteTransposition> values;
    std::string ignored;
};

// A pitched note the reader stored: its measure, the position among the measure's <note> elements
// of the first note of its chord (its own position when it is in no chord), its staff, and its
// index among that staff's notes. A chord is read as a unit, from the position of its first note.
struct PitchedNote {
    int measureIdx = 0;
    int chordPosition = 0;
    int staff = 0;
    int index = 0;
};

// 'text' without the white space around it, which MusicXML numbers allow.
std::string trimmed(const std::string& text) {
    const size_t begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    return text.substr(begin, text.find_last_not_of(" \t\r\n") - begin + 1);
}

// The text of the child element 'name' of 'node', trimmed; empty when there is no such child.
std::string trimmedChildText(const pugi::xml_node& node, const char* name) {
    return trimmed(node.child_value(name));
}

// The value of a MusicXML number -- an optional sign, digits and an optional fraction -- when it
// is a whole number; std::nullopt for any other text, a fraction other than zero or an exponent
// included. A magnitude beyond 10^12, far outside every interval a note can take, is held there.
std::optional<std::int64_t> wholeNumber(const std::string& text) {
    size_t i = (!text.empty() && (text[0] == '-' || text[0] == '+')) ? 1 : 0;
    bool hasDigits = false;
    std::int64_t magnitude = 0;
    for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; i++) {
        hasDigits = true;
        magnitude = std::min<std::int64_t>(magnitude * 10 + (text[i] - '0'), 1000000000000);
    }
    if (i < text.size() && text[i] == '.') {
        for (i++; i < text.size() && text[i] == '0'; i++) {
            hasDigits = true;
        }
    }
    if (!hasDigits || i != text.size()) {
        return std::nullopt;
    }
    return (text[0] == '-') ? -magnitude : magnitude;
}

// An interval held to the range of int: one beyond it is held at its limit, where no note can
// sound.
int toIntRange(const std::int64_t value) {
    return static_cast<int>(std::clamp<std::int64_t>(value, std::numeric_limits<int>::min(),
                                                     std::numeric_limits<int>::max()));
}

// Reads one <transpose>: the interval, <octave-change> folded in as 7 letters and 12 semitones
// per octave, and the octave doubling of <double>; an absent <diatonic> stands for the
// conventional diatonic interval of <chromatic>. A <chromatic> or an <octave-change> that is not
// a whole number makes the element ignored. A number attribute that is not a positive integer is
// read as absent.
TransposeElement readTranspose(const pugi::xml_node& transpose, const int measureIdx,
                               const int notesBefore, const std::string& where) {
    TransposeElement element;
    element.measureIdx = measureIdx;
    element.notesBefore = notesBefore;
    element.where = where;
    const std::optional<std::int64_t> number =
        wholeNumber(trimmed(transpose.attribute("number").value()));
    element.staff = (number && *number >= 1) ? toIntRange(*number - 1) : -1;

    const std::string chromaticText = trimmedChildText(transpose, "chromatic");
    const std::optional<std::int64_t> chromatic = wholeNumber(chromaticText);
    if (!chromatic) {
        element.ignored = "[transpose-chromatic-not-integer] " + where + ": <chromatic>" +
                          chromaticText +
                          "</chromatic> is not a whole number of semitones; the <transpose> is "
                          "ignored and the previous transposition stays in force.";
        return element;
    }

    std::int64_t octaves = 0;
    if (transpose.child("octave-change")) {
        const std::string octaveText = trimmedChildText(transpose, "octave-change");
        const std::optional<std::int64_t> value = wholeNumber(octaveText);
        if (!value) {
            element.ignored = "[transpose-octave-change-not-integer] " + where +
                              ": <octave-change>" + octaveText +
                              "</octave-change> is not a whole number of octaves; the "
                              "<transpose> is ignored and the previous transposition stays in "
                              "force.";
            return element;
        }
        octaves = *value;
    }

    // An absent <diatonic> stands for the diatonic interval conventionally written for the
    // chromatic one, stored explicitly.
    std::int64_t diatonic = maiacore::detail::conventionalDiatonicInterval(toIntRange(*chromatic));
    if (transpose.child("diatonic")) {
        const std::optional<std::int64_t> value =
            wholeNumber(trimmedChildText(transpose, "diatonic"));
        if (value) {
            diatonic = *value;
        }
    }

    NoteTransposition values;
    values.diatonic = toIntRange(diatonic + 7 * octaves);
    values.chromatic = toIntRange(*chromatic + 12 * octaves);
    const pugi::xml_node doubled = transpose.child("double");
    if (doubled) {
        values.doubling = (std::string(doubled.attribute("above").value()) == "yes")
                              ? OctaveDoubling::ABOVE
                              : OctaveDoubling::BELOW;
    }
    element.values = values;
    return element;
}

// The <transpose> elements of a measure's <attributes>, in document order, each with the number
// of <note> elements before its <attributes>.
void readTransposeElements(const pugi::xml_node& measure, const int measureIdx,
                           const std::string& where, std::vector<TransposeElement>& elements) {
    int notesBefore = 0;
    for (const pugi::xml_node child : measure.children()) {
        const std::string name = child.name();
        if (name == "note") {
            notesBefore++;
        } else if (name == "attributes") {
            for (const pugi::xml_node transpose : child.children("transpose")) {
                elements.push_back(readTranspose(transpose, measureIdx, notesBefore, where));
            }
        }
    }
}

// Whether 'note' comes at or after 'element' in document order, so that the element reaches it.
// A chord counts from its first note: an element written between its notes reaches none of them.
bool isAtOrAfter(const PitchedNote& note, const TransposeElement& element) {
    return note.measureIdx > element.measureIdx ||
           (note.measureIdx == element.measureIdx && note.chordPosition >= element.notesBefore);
}

// Stamps each pitched note of a part with the <transpose> in force for its staff: the last one
// before the note -- before the first note of its chord, for a chord -- in document order that
// applies to its staff, by its number or with none, and is not ignored. A note before every such
// element stays untransposed. Each ignored element's warning is printed.
void applyTranspositions(Part& part, const std::vector<TransposeElement>& elements,
                         const std::vector<PitchedNote>& notes) {
    std::map<int, std::vector<PitchedNote>> notesByStaff;
    for (const PitchedNote& note : notes) {
        notesByStaff[note.staff].push_back(note);
    }

    // Each element's scope on each staff it applies to: the half-open range of that staff's
    // notes from the element to the next element that applies to the staff.
    std::vector<std::map<int, std::pair<size_t, size_t>>> scopes(elements.size());
    for (const auto& [staff, staffNotes] : notesByStaff) {
        size_t cursor = 0;
        std::optional<size_t> previous;
        for (size_t e = 0; e < elements.size(); e++) {
            if (elements[e].staff != -1 && elements[e].staff != staff) {
                continue;
            }
            while (cursor < staffNotes.size() && !isAtOrAfter(staffNotes[cursor], elements[e])) {
                cursor++;
            }
            if (previous) {
                scopes[*previous][staff].second = cursor;
            }
            scopes[e][staff] = {cursor, staffNotes.size()};
            previous = e;
        }
    }

    std::map<int, NoteTransposition> inForce;
    for (size_t e = 0; e < elements.size(); e++) {
        const TransposeElement& element = elements[e];
        if (!element.values) {
            LOG_WARN(element.ignored);
        }
        for (const auto& [staff, scope] : scopes[e]) {
            if (element.values) {
                inForce[staff] = *element.values;
            }
            const NoteTransposition stamp = inForce[staff];
            const std::vector<PitchedNote>& staffNotes = notesByStaff.at(staff);
            for (size_t i = scope.first; i < scope.second; i++) {
                Note& note = part.getMeasure(staffNotes[i].measureIdx)
                                 .getNote(staffNotes[i].index, staffNotes[i].staff);
                note.setTransposingInterval(stamp.diatonic, stamp.chromatic);
                note.setOctaveDoubling(stamp.doubling);
            }
        }
    }
}
```

- [ ] **Step 6: Wire the reader.** In `Score::loadXMLFile`:
  - replace the whole STEP 2 block, from `        // ===== STEP 2: GET THE PART 'i' TRANSPOSE VALUES ===== //` through the closing `        }` of `if (isTransposedInstrument) {`, with

```cpp
        // ===== STEP 2: THE PART 'p' TRANSPOSITIONS ===== //
        // Each <transpose> is collected measure by measure below, and once the part is read it
        // is stamped on the pitched notes it applies to.
        std::vector<TransposeElement> transposeElements;
        std::vector<PitchedNote> pitchedNotes;
        std::vector<std::string> measureNumbers(_numMeasures);
```

  - before `            // Get the xPath for all notes inside the measure 'm'` insert

```cpp
            // ===== TRANSPOSE ===== //
            measureNumbers[m] = measureNode.node().attribute("number").value();
            readTransposeElements(
                measureNode.node(), m,
                "part \"" + _part[p].getName() + "\", measure " + measureNumbers[m],
                transposeElements);
            // The position of the first note of the chord the current note belongs to: a chord is
            // read as a unit, from its first note.
            int chordStart = 0;

```

  - delete the line `                note.setTransposingInterval(transposeDiatonic, transposeChromatic);`;
  - replace the end of `loadXMLFile`

```cpp
                _part[p].getMeasure(m).addNote(note, staff);
            }
        }
    }
}
```

  with

```cpp
                _part[p].getMeasure(m).addNote(note, staff);
                if (!inChord) {
                    chordStart = n;
                }
                if (isNoteOn && !isUnpitched) {
                    pitchedNotes.push_back(
                        {m, chordStart, staff, _part[p].getMeasure(m).getNumNotes(staff) - 1});
                }
            }
        }

        applyTranspositions(_part[p], transposeElements, pitchedNotes);
    }
}
```

  (the index is read after `addNote`, so a `<staff>` beyond the part's staves still fails in `addNote` with today's error).

- [ ] **Step 7: Document the reader.** In `maiacore/include/maiacore/score.h`, in the Doxygen of `explicit Score(const std::string& filePath);`, replace

```cpp
     *          and a warning is printed. None of these aborts the load.
     * @param filePath Path to the MusicXML file.
```

with

```cpp
     *          and a warning is printed. None of these aborts the load.
     *
     *          A `<transpose>` is read in every measure. It applies to the pitched notes written
     *          after it, in its measure and the following ones, on the staff its `number` names or,
     *          without `number`, on every staff of the part, until the next `<transpose>` for that
     *          staff; inside a measure document order decides, and a chord is read as a unit: a
     *          `<transpose>` between the notes of a chord applies from the first note after the
     *          chord. Each such note is given the interval with `<octave-change>` folded in, 7
     *          letters and 12 semitones per octave (a B-flat bass clarinet's -1, -2 and -1 make
     *          (-8, -14)), and the octave doubling of `<double>` (Note::getOctaveDoubling()); rests
     *          and unpitched notes are left alone. A `<transpose>` without `<diatonic>` is given
     *          the conventional diatonic interval of its `<chromatic>` one (see
     *          Note::getSoundingPitch()). One whose `<chromatic>` or `<octave-change>` is not a
     *          whole number is ignored, leaving the previous transposition in force, with a warning
     *          that starts with [transpose-chromatic-not-integer] or
     *          [transpose-octave-change-not-integer] and names the part and the measure as the file
     *          numbers it.
     * @param filePath Path to the MusicXML file.
```

In `maiacore/src/maiacore/python_wrapper/py_score.cpp`, in the `Score(filePath)` docstring, replace

```
        and a warning is printed. None of these aborts the load.

        Parameters
```

with

```
        and a warning is printed. None of these aborts the load.

        A ``<transpose>`` is read in every measure. It applies to the pitched notes written after
        it, in its measure and the following ones, on the staff its ``number`` names or, without
        ``number``, on every staff of the part, until the next ``<transpose>`` for that staff;
        inside a measure document order decides, and a chord is read as a unit: a ``<transpose>``
        between the notes of a chord applies from the first note after the chord. Each such note is
        given the interval with ``<octave-change>`` folded in, 7 letters and 12 semitones per octave
        (a B-flat bass clarinet's -1, -2 and -1 make ``(-8, -14)``), and the octave doubling of
        ``<double>`` (see ``Note.getOctaveDoubling``); rests and unpitched notes are left alone. A
        ``<transpose>`` without ``<diatonic>`` is given the conventional diatonic interval of its
        ``<chromatic>`` one (see ``Note.getSoundingPitch``). One whose ``<chromatic>`` or
        ``<octave-change>`` is not a whole number is ignored, leaving the previous transposition in
        force, with a warning that starts with ``[transpose-chromatic-not-integer]`` or
        ``[transpose-octave-change-not-integer]`` and names the part and the measure as the file
        numbers it.

        Parameters
```

- [ ] **Step 8: Format, build, pass.** clang-format `note.cpp`, `note.h`, `pitch-views.h`, `score.cpp`, `score.h`, `py_note.cpp`, `py_score.cpp`, `score-test.cpp`, `pitch-views-test.cpp`. C++ subset `ConventionalDiatonicInterval.*:ScoreTransposeRead.*:ConcertSpelling.*` → all pass (15 new: `ConventionalDiatonicInterval` and 14 reader tests; the concert-spelling tests unchanged).

- [ ] **Step 9: Mutations** (C++ subset, rebuild each time). (a) `conventionalDiatonicInterval`: change the table entry for 6 semitones from `3` to `4` → `ConventionalDiatonicInterval` fails. (b) In `applyTranspositions` skip elements of later measures (`if (elements[e].measureIdx > 0) { continue; }` as the first statement of the scope loop's element loop) → `aChangeInTheMiddleOfAPartIsFollowed`, `aTransposeInALaterMeasureAppliesFromThere`, the 72c expectation fail. (c) In `readTranspose` set `element.staff = -1;` unconditionally → `aNumberedTransposeAppliesToItsStaffAlone` fails. (d) Use `toIntRange(diatonic)` and `toIntRange(*chromatic)` (no octaves) → `anOctaveChangeIsFoldedIntoTheInterval`, the 41c, 72d and sample tests fail. (e) Never set `values.doubling` → `doubleIsReadAsTheOctaveDoubling` fails. (f) `isAtOrAfter` returns `note.measureIdx >= element.measureIdx` → `aTransposeAfterNotesAppliesToTheNotesAfterIt` fails. (g) Start `diatonic` at `0` instead of the conventional interval → `aTransposeWithoutDiatonicStoresTheConventionalInterval` fails (`F#4 (0, -2, NONE) E4`). (h) Record every note (`if (true)` for `if (isNoteOn && !isUnpitched)`) → `aTransposeInALaterMeasureAppliesFromThere` fails (the unpitched note is transposed, the rest prints the doubling warning). (i) `wholeNumber` accepts a fraction (replace `text[i] == '0'` with `text[i] >= '0' && text[i] <= '9'`) → `aChromaticThatIsNotAWholeNumberIsIgnored` and `anOctaveChangeThatIsNotAWholeNumberIsIgnored` fail. (j) Drop `octaves` from the chromatic fold only (`toIntRange(*chromatic)`) → `anOctaveTranspositionChangesTheChords` fails with the contrabass back at C3. (k) In the note loop drop the `if (!inChord)` around `chordStart = n;`, so that every note counts from its own position → `aTransposeBetweenTheNotesOfAChordAppliesAfterTheChord` fails (`G5 (-2, -3, NONE) E5`). Revert each; rerun green.

- [ ] **Step 10: The ledger.** «build» `make "PYTHON=$py" dev` → 0. «build» `make "PYTHON=$py" corpus-update-ledger; $LASTEXITCODE` → 0 (it runs every in-repository file, the slow ones and the fetched external corpus: expect a long run). `git diff --stat test/musicxml/ledger-external.json` → empty. `git diff test/musicxml/ledger.json` → exactly ten added lines, between `teste_one_measure.xml` and `unrepresentable_alter_eighth_tone.xml`, and no other change:

```
  "test/xml_examples/unit_test/transpose_after_notes.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_change_mid_part.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_chromatic_not_integer.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_double.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_inside_chord.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_later_measure.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_octave_change.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_octave_change_not_integer.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "invalid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_per_staff.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_without_diatonic.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
```

If any field differs from these, or any other line changed, stop and report the diff instead of committing.

- [ ] **Step 11: Whole suites.** `make cpp-tests` → 0; `make py-tests` → OK (the corpus test covers the ten fixtures); `make validate` → no new findings.

- [ ] **Step 12: Commit.** `git add test/xml_examples/unit_test/transpose_change_mid_part.musicxml test/xml_examples/unit_test/transpose_later_measure.musicxml test/xml_examples/unit_test/transpose_per_staff.musicxml test/xml_examples/unit_test/transpose_octave_change.musicxml test/xml_examples/unit_test/transpose_double.musicxml test/xml_examples/unit_test/transpose_after_notes.musicxml test/xml_examples/unit_test/transpose_inside_chord.musicxml test/xml_examples/unit_test/transpose_without_diatonic.musicxml test/xml_examples/unit_test/transpose_chromatic_not_integer.musicxml test/xml_examples/unit_test/transpose_octave_change_not_integer.musicxml test/musicxml/ledger.json maiacore/src/maiacore/pitch-views.h maiacore/src/maiacore/note.cpp maiacore/include/maiacore/note.h maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_note.cpp maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/score-test.cpp tests-cpp/src/pitch-views-test.cpp`, message:

```
feat!: read <transpose> in every measure, per staff, with octave-change and double

Each <transpose> applies to the pitched notes after it on its staff, or on
every staff without number, up to the next one; a chord is read as a
unit. octave-change is folded into the interval and <double> becomes the
note's octave doubling. A
<transpose> without <diatonic> stores the conventional interval. One whose
<chromatic> or <octave-change> is not a whole number is ignored with a
coded warning. Octave-transposing parts now sound in the right octave.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 4: Reader corrections — the pair, the range, `<for-part>`

**Files:**
- Create: `test/xml_examples/unit_test/transpose_pair_inconsistent.musicxml`, `transpose_diatonic_not_integer.musicxml` (schema-invalid: `<diatonic>-4.5</diatonic>`), `transpose_out_of_range.musicxml`, `transpose_for_part.musicxml`
- Modify: `maiacore/src/maiacore/score.cpp` (`readTranspose`, `readTransposeElements`, `applyTranspositions`, the call after the measure loop), `maiacore/include/maiacore/score.h` (`Score(path)` Doxygen), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (`Score(filePath)` docstring), `test/musicxml/ledger.json` (4 added lines)
- Test: `tests-cpp/src/score-test.cpp` (append)

**Interfaces:** consumes Task 3's reader helpers, `soundsWithinRange` (Task 2), `conventionalDiatonicInterval` (Task 3). Produces `TransposeElement::pairCorrected` (a `std::string`), `bool inSameChord(const PitchedNote&, const PitchedNote&)`, `void applyTranspositions(Part&, const std::vector<TransposeElement>&, const std::vector<PitchedNote>&, const std::vector<std::string>& measureNumbers)`.

- [ ] **Step 1: The fixtures** under `test/xml_examples/unit_test/`.

`transpose_pair_inconsistent.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- <diatonic> against <chromatic>: trumpets in E written with the letters of a fourth and the
     semitones of a major third (3, 4) are corrected to (2, 4), so F#4 sounds A#4; a clarinet's
     explicit <diatonic>0</diatonic> with <chromatic>-2</chromatic> is corrected to (-1, -2); a
     tritone accepts the diminished fifth (-4, -6) and the augmented fourth (-3, -6) alike. Two
     warnings. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Trumpet in E</part-name></score-part>
    <score-part id="P2"><part-name>Clarinet in Bb</part-name></score-part>
    <score-part id="P3"><part-name>Horn in F#</part-name></score-part>
    <score-part id="P4"><part-name>Horn in Gb</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>3</diatonic><chromatic>4</chromatic></transpose>
      </attributes>
      <note><pitch><step>F</step><alter>1</alter><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P2">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>-2</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P3">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-4</diatonic><chromatic>-6</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P4">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-3</diatonic><chromatic>-6</chromatic></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_diatonic_not_integer.musicxml` (invalid against the schema on purpose: `<diatonic>` is `xs:integer`):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A <diatonic> that is not a whole number (invalid against the schema, which types it
     xs:integer) does not match <chromatic>-7</chromatic>: it is replaced by the conventional -4
     with a warning, so G4 sounds C4. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Horn in F</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>-4.5</diatonic><chromatic>-7</chromatic></transpose>
      </attributes>
      <note><pitch><step>G</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_out_of_range.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A <transpose> with which a note of its scope would have no sounding pitch is ignored for its
     whole scope. Piccolo: (0, 0, +1) from measure 1; the (0, 0, +3) of measure 2 would put the
     C9 of measure 3 above B11, so it is ignored and C8 in measure 2 keeps +1 although it could
     sound with +3. Contrabass: (0, 0, -1) from measure 1; the (0, 0, -2) of measure 2 would put
     its Cb0 below C1b-1, so it is ignored; the previous -1 cannot sound that Cb0 either, so the
     chord Cb0-C1 is read untransposed as a unit, C1 included, while the D1 of measure 3 keeps
     -1. -->
<score-partwise version="4.0">
  <part-list>
    <score-part id="P1"><part-name>Piccolo</part-name></score-part>
    <score-part id="P2"><part-name>Contrabass</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><octave-change>1</octave-change></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>9</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><octave-change>3</octave-change></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>8</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="3">
      <note><pitch><step>C</step><octave>9</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
  <part id="P2">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>F</sign><line>4</line></clef>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><octave-change>-1</octave-change></transpose>
      </attributes>
      <note><pitch><step>C</step><octave>1</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="2">
      <attributes>
        <transpose><diatonic>0</diatonic><chromatic>0</chromatic><octave-change>-2</octave-change></transpose>
      </attributes>
      <note><pitch><step>C</step><alter>-1</alter><octave>0</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
      <note><chord/><pitch><step>C</step><octave>1</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
    <measure number="3">
      <note><pitch><step>D</step><octave>1</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`transpose_for_part.musicxml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- A concert score: its notes are written at concert pitch, and <for-part> gives the
     transposition of a part created from it. maialib does not model <for-part>: it is dropped
     with a warning, and C4 sounds C4. -->
<score-partwise version="4.0">
  <defaults>
    <concert-score/>
  </defaults>
  <part-list>
    <score-part id="P1"><part-name>Clarinet in Bb</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key><fifths>0</fifths></key>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
        <for-part>
          <part-transpose><diatonic>-1</diatonic><chromatic>-2</chromatic></part-transpose>
        </for-part>
      </attributes>
      <note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice><type>whole</type></note>
    </measure>
  </part>
</score-partwise>
```

`musicxml_check.py` on the four: `valid; errors: none` for all but `transpose_diatonic_not_integer.musicxml` (`invalid`, naming `diatonic`).

- [ ] **Step 2: Failing tests** — append to `tests-cpp/src/score-test.cpp`:

```cpp
// A <diatonic> that does not match <chromatic> is replaced by the conventional diatonic interval,
// with one warning per <transpose>; an explicit 0 with a non-zero chromatic interval does not
// match either, while a tritone matches the augmented fourth and the diminished fifth alike.
TEST(ScoreTransposeCorrection, aDiatonicIntervalThatDoesNotMatchIsReplaced) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_pair_inconsistent.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"F#4 (2, 4, NONE) A#4"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3"}));
    EXPECT_EQ(transposedNotes(score, 2), (std::vector<std::string>{"C4 (-4, -6, NONE) F#3"}));
    EXPECT_EQ(transposedNotes(score, 3), (std::vector<std::string>{"C4 (-3, -6, NONE) Gb3"}));
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Trumpet in E\", measure 1: "
                           "<diatonic>3</diatonic> does not match <chromatic>4</chromatic>; "
                           "using 2.\n"),
              std::string::npos)
        << printed;
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Clarinet in Bb\", measure 1: "
                           "<diatonic>0</diatonic> does not match <chromatic>-2</chromatic>; "
                           "using -1.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 2) << printed;
}

// The Dvorak sample's trumpets in E, written with the letters of a fourth and the semitones of a
// major third, are read as a major third: a written F#4 sounds A#4.
TEST(ScoreTransposeCorrection, theDvorakTrumpetsInESoundAMajorThirdUp) {
    StdoutCapture capture;
    Score score(kSamples + "Dvorak_Symphony_9_mov_4.mxl");
    const std::string printed = capture.str();
    EXPECT_EQ(describeTransposition(score.getPart(6).getMeasure(7).getNote(1, 0)),
              "F#4 (2, 4, NONE) A#4");
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Trombe I. II. E\", measure "
                           "1: <diatonic>3</diatonic> does not match <chromatic>4</chromatic>; "
                           "using 2.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// A <diatonic> that is not a whole number does not match <chromatic> either.
TEST(ScoreTransposeCorrection, aDiatonicThatIsNotAWholeNumberIsReplaced) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_diatonic_not_integer.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"G4 (-4, -7, NONE) C4"}));
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Horn in F\", measure 1: "
                           "<diatonic>-4.5</diatonic> does not match <chromatic>-7</chromatic>; "
                           "using -4.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// A <transpose> with which a note of its scope would have no sounding pitch is ignored for its
// whole scope -- none of its notes takes it, not even one that could sound with it -- and the
// previous transposition stays in force; a chord with a note that this one cannot sound either is
// read untransposed as a unit, and the warning counts its notes.
TEST(ScoreTransposeCorrection, aTransposeOutOfRangeIsIgnoredForItsWholeScope) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_out_of_range.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C9 (7, 12, NONE) C10", "C8 (7, 12, NONE) C9",
                                        "C9 (7, 12, NONE) C10"}));
    EXPECT_EQ(transposedNotes(score, 1),
              (std::vector<std::string>{"C1 (-7, -12, NONE) C0", "Cb0 (0, 0, NONE) B-1",
                                        "C1 (0, 0, NONE) C1", "D1 (-7, -12, NONE) D0"}));
    EXPECT_NE(printed.find("[WARN] [transpose-out-of-range] part \"Piccolo\", measure 2: the "
                           "written C9 of measure 3, staff 1 would sound outside the "
                           "representable range; the <transpose> is ignored and the previous "
                           "transposition stays in force.\n"),
              std::string::npos)
        << printed;
    EXPECT_NE(printed.find("[WARN] [transpose-out-of-range] part \"Contrabass\", measure 2: the "
                           "written Cb0 of measure 2, staff 1 would sound outside the "
                           "representable range; the <transpose> is ignored and the previous "
                           "transposition stays in force. Its notes that the previous "
                           "transposition cannot sound either are read untransposed, with the "
                           "other notes of their chords (2 in all).\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 2) << printed;
}

// <for-part> is dropped with a warning: a concert score's notes are already at concert pitch.
TEST(ScoreTransposeCorrection, aForPartIsDroppedWithAWarning) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_for_part.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"C4 (0, 0, NONE) C4"}));
    EXPECT_NE(printed.find("[WARN] [for-part-not-modelled] part \"Clarinet in Bb\", measure 1: "
                           "<for-part> is not modelled and is dropped; the notes of a concert "
                           "score are written at concert pitch.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}
```

- [ ] **Step 3: Run and see them fail.** C++ subset `ScoreTransposeCorrection.*` → `aDiatonicIntervalThatDoesNotMatchIsReplaced` gets `F#4 (3, 4, NONE) Bb4` and no warning; `aTransposeOutOfRangeIsIgnoredForItsWholeScope` aborts the load: `C++ exception with description "[maiacore] The sounding pitch of the written pitch 'C9' with transposeDiatonic=21 and transposeChromatic=36 ... above B11"`; `aForPartIsDroppedWithAWarning` finds no warning.

- [ ] **Step 4: The pair check.** In `score.cpp`, add the member `    std::string pairCorrected;  // the warning for a <diatonic> replaced, or empty` after `std::string ignored;` in `TransposeElement`, and in `readTranspose` replace

```cpp
    // An absent <diatonic> stands for the diatonic interval conventionally written for the
    // chromatic one, stored explicitly.
    std::int64_t diatonic = maiacore::detail::conventionalDiatonicInterval(toIntRange(*chromatic));
    if (transpose.child("diatonic")) {
        const std::optional<std::int64_t> value =
            wholeNumber(trimmedChildText(transpose, "diatonic"));
        if (value) {
            diatonic = *value;
        }
    }
```

with

```cpp
    // The diatonic interval conventionally written for the chromatic one: what an absent
    // <diatonic> stands for, stored explicitly, and what replaces one that does not match
    // <chromatic>. The check reads the values as the file writes them, before <octave-change>.
    const std::int64_t conventional =
        maiacore::detail::conventionalDiatonicInterval(toIntRange(*chromatic));
    std::int64_t diatonic = conventional;
    if (transpose.child("diatonic")) {
        const std::string diatonicText = trimmedChildText(transpose, "diatonic");
        const std::optional<std::int64_t> value = wholeNumber(diatonicText);
        // A tritone matches both the augmented fourth, the conventional count, and the
        // diminished fifth, one letter further.
        const std::int64_t semitones = (*chromatic < 0) ? -*chromatic : *chromatic;
        const bool tritone = semitones % 12 == 6;
        const bool matches =
            value && (*value == conventional ||
                      (tritone && *value == conventional + (*chromatic > 0 ? 1 : -1)));
        if (matches) {
            diatonic = *value;
        } else {
            element.pairCorrected = "[transpose-pair-corrected] " + where + ": <diatonic>" +
                                    diatonicText + "</diatonic> does not match <chromatic>" +
                                    chromaticText + "</chromatic>; using " +
                                    std::to_string(conventional) + ".";
        }
    }
```

and update the comment above `readTranspose` to:

```cpp
// Reads one <transpose>: the interval, <octave-change> folded in as 7 letters and 12 semitones
// per octave, and the octave doubling of <double>. An absent <diatonic> stands for the
// conventional diatonic interval of <chromatic>; one that does not match it is replaced by that
// interval, with a warning. A <chromatic> or an <octave-change> that is not a whole number makes
// the element ignored. A number attribute that is not a positive integer is read as absent.
```

- [ ] **Step 5: `<for-part>`.** In `readTransposeElements` replace

```cpp
            for (const pugi::xml_node transpose : child.children("transpose")) {
                elements.push_back(readTranspose(transpose, measureIdx, notesBefore, where));
            }
```

with

```cpp
            for (const pugi::xml_node transpose : child.children("transpose")) {
                elements.push_back(readTranspose(transpose, measureIdx, notesBefore, where));
            }
            for (pugi::xml_node forPart = child.child("for-part"); forPart;
                 forPart = forPart.next_sibling("for-part")) {
                LOG_WARN("[for-part-not-modelled] " + where +
                         ": <for-part> is not modelled and is dropped; the notes of a concert "
                         "score are written at concert pitch.");
            }
```

and its comment to `// The <transpose> elements of a measure's <attributes>, in document order, each with the number\n// of <note> elements before its <attributes>; a <for-part> is reported and dropped.`

- [ ] **Step 6: The range, checked over the whole scope.** Replace the whole `applyTranspositions` function (its comment included) with:

```cpp
// Whether two pitched notes of a staff are notes of one chord.
bool inSameChord(const PitchedNote& a, const PitchedNote& b) {
    return a.measureIdx == b.measureIdx && a.chordPosition == b.chordPosition;
}

// Stamps each pitched note of a part with the <transpose> in force for its staff: the last one
// before the note -- before the first note of its chord, for a chord -- in document order that
// applies to its staff, by its number or with none, and is not ignored. A note before every such
// element stays untransposed. An element is ignored when its value cannot be read, or when a note
// of its scope -- the notes it would stamp, up to the next <transpose> for their staff -- would
// have no sounding pitch with it: it is checked before anything is stamped, so an ignored element
// stamps nothing. The previous transposition stays in force over the scope of an ignored element;
// a chord with a note it cannot sound either is read untransposed, all its notes. Each element
// prints its warnings: the corrected pair, then why it is ignored.
void applyTranspositions(Part& part, const std::vector<TransposeElement>& elements,
                         const std::vector<PitchedNote>& notes,
                         const std::vector<std::string>& measureNumbers) {
    std::map<int, std::vector<PitchedNote>> notesByStaff;
    for (const PitchedNote& note : notes) {
        notesByStaff[note.staff].push_back(note);
    }
    const auto noteAt = [&part](const PitchedNote& pitched) -> Note& {
        return part.getMeasure(pitched.measureIdx).getNote(pitched.index, pitched.staff);
    };

    // Each element's scope on each staff it applies to: the half-open range of that staff's
    // notes from the element to the next element that applies to the staff.
    std::vector<std::map<int, std::pair<size_t, size_t>>> scopes(elements.size());
    for (const auto& [staff, staffNotes] : notesByStaff) {
        size_t cursor = 0;
        std::optional<size_t> previous;
        for (size_t e = 0; e < elements.size(); e++) {
            if (elements[e].staff != -1 && elements[e].staff != staff) {
                continue;
            }
            while (cursor < staffNotes.size() && !isAtOrAfter(staffNotes[cursor], elements[e])) {
                cursor++;
            }
            if (previous) {
                scopes[*previous][staff].second = cursor;
            }
            scopes[e][staff] = {cursor, staffNotes.size()};
            previous = e;
        }
    }

    std::map<int, NoteTransposition> inForce;
    for (size_t e = 0; e < elements.size(); e++) {
        const TransposeElement& element = elements[e];
        if (!element.pairCorrected.empty()) {
            LOG_WARN(element.pairCorrected);
        }

        // Why the element is ignored, if it is: its value cannot be read, or the first note of
        // its scope that cannot sound with it.
        std::string rejection = element.ignored;
        if (rejection.empty()) {
            const NoteTransposition& values = *element.values;
            for (const auto& scope : scopes[e]) {
                const std::vector<PitchedNote>& staffNotes = notesByStaff.at(scope.first);
                for (size_t i = scope.second.first; i < scope.second.second && rejection.empty();
                     i++) {
                    const Note& note = noteAt(staffNotes[i]);
                    if (!maiacore::detail::soundsWithinRange(note, values.diatonic,
                                                             values.chromatic)) {
                        rejection = "[transpose-out-of-range] " + element.where + ": the written " +
                                    note.getWrittenPitch() + " of measure " +
                                    measureNumbers.at(staffNotes[i].measureIdx) + ", staff " +
                                    std::to_string(staffNotes[i].staff + 1) +
                                    " would sound outside the representable range; the "
                                    "<transpose> is ignored and the previous transposition stays "
                                    "in force.";
                    }
                }
                if (!rejection.empty()) {
                    break;
                }
            }
        }

        int untransposed = 0;
        for (const auto& [staff, scope] : scopes[e]) {
            if (rejection.empty()) {
                inForce[staff] = *element.values;
            }
            const NoteTransposition stamp = inForce[staff];
            const std::vector<PitchedNote>& staffNotes = notesByStaff.at(staff);
            // A chord is stamped as a unit: when the transposition cannot sound one of its notes,
            // every note of the chord is read untransposed.
            size_t chordStart = scope.first;
            while (chordStart < scope.second) {
                size_t chordEnd = chordStart;
                bool sounds = true;
                while (chordEnd < scope.second &&
                       inSameChord(staffNotes[chordEnd], staffNotes[chordStart])) {
                    sounds = sounds &&
                             maiacore::detail::soundsWithinRange(noteAt(staffNotes[chordEnd]),
                                                                 stamp.diatonic, stamp.chromatic);
                    chordEnd++;
                }
                if (sounds) {
                    for (size_t i = chordStart; i < chordEnd; i++) {
                        Note& note = noteAt(staffNotes[i]);
                        note.setTransposingInterval(stamp.diatonic, stamp.chromatic);
                        note.setOctaveDoubling(stamp.doubling);
                    }
                } else {
                    untransposed += static_cast<int>(chordEnd - chordStart);
                }
                chordStart = chordEnd;
            }
        }

        if (!rejection.empty()) {
            if (untransposed > 0) {
                rejection +=
                    " Its notes that the previous transposition cannot sound either are "
                    "read untransposed, with the other notes of their chords (" +
                    std::to_string(untransposed) + " in all).";
            }
            LOG_WARN(rejection);
        }
    }
}
```

and replace the call `        applyTranspositions(_part[p], transposeElements, pitchedNotes);` with `        applyTranspositions(_part[p], transposeElements, pitchedNotes, measureNumbers);`.

- [ ] **Step 7: Document the corrections.** In `score.h`, in the `Score(path)` Doxygen, replace

```cpp
     *          that starts with [transpose-chromatic-not-integer] or
     *          [transpose-octave-change-not-integer] and names the part and the measure as the file
     *          numbers it.
```

with

```cpp
     *          that starts with [transpose-chromatic-not-integer] or
     *          [transpose-octave-change-not-integer]. A `<diatonic>` that does not match
     *          `<chromatic>` is replaced by the conventional diatonic interval, so that nothing
     *          sounds different, with a [transpose-pair-corrected] warning; for a tritone both the
     *          augmented fourth and the diminished fifth match, and an explicit 0 with a non-zero
     *          `<chromatic>` does not. A `<transpose>` with which a note of its scope -- the notes
     *          it would apply to, up to the next `<transpose>` for their staff -- would have no
     *          sounding pitch (below C1b-1, or above B11 where its letter cannot spell it) is
     *          ignored for its whole scope, with a [transpose-out-of-range] warning; there the
     *          previous transposition stays in force, and a chord with a note it cannot sound
     *          either is read untransposed. `<for-part>` is not modelled: it is dropped with a
     *          [for-part-not-modelled] warning. Each warning names the part and the measure as the
     *          file numbers it.
```

In `py_score.cpp`, in the `Score(filePath)` docstring, replace

```
        force, with a warning that starts with ``[transpose-chromatic-not-integer]`` or
        ``[transpose-octave-change-not-integer]`` and names the part and the measure as the file
        numbers it.
```

with

```
        force, with a warning that starts with ``[transpose-chromatic-not-integer]`` or
        ``[transpose-octave-change-not-integer]``. A ``<diatonic>`` that does not match
        ``<chromatic>`` is replaced by the conventional diatonic interval, so that nothing sounds
        different, with a ``[transpose-pair-corrected]`` warning; for a tritone both the augmented
        fourth and the diminished fifth match, and an explicit 0 with a non-zero ``<chromatic>``
        does not. A ``<transpose>`` with which a note of its scope -- the notes it would apply to,
        up to the next ``<transpose>`` for their staff -- would have no sounding pitch (below
        ``C1b-1``, or above ``B11`` where its letter cannot spell it) is ignored for its whole
        scope, with a ``[transpose-out-of-range]`` warning; there the previous transposition stays
        in force, and a chord with a note it cannot sound either is read untransposed.
        ``<for-part>`` is not modelled: it is dropped with a ``[for-part-not-modelled]`` warning.
        Each warning names the part and the measure as the file numbers it.
```

- [ ] **Step 8: Format, build, pass.** clang-format `score.cpp`, `score.h`, `py_score.cpp`, `score-test.cpp`. C++ subset `ScoreTransposeCorrection.*:ScoreTransposeRead.*` → all pass (5 new, the 14 reader tests still green).

- [ ] **Step 9: Mutations** (C++ subset, rebuild each time). (a) `const bool matches = value.has_value();` → `aDiatonicIntervalThatDoesNotMatchIsReplaced` and `theDvorakTrumpetsInESoundAMajorThirdUp` fail. (b) Drop `(tritone && ...)` → `aDiatonicIntervalThatDoesNotMatchIsReplaced` fails (three warnings, the horn in F# corrected to (-3, -6)). (c) Make `wholeNumber` truncate a fraction (replace `text[i] == '0'` with `text[i] >= '0' && text[i] <= '9'`) → `aDiatonicThatIsNotAWholeNumberIsReplaced` fails. (d) Check only the element's own measure (add `&& staffNotes[i].measureIdx == element.measureIdx` to the check's condition) → `aTransposeOutOfRangeIsIgnoredForItsWholeScope` fails (the piccolo's element is accepted: C8 takes +3 and the C9 of measure 3, which cannot sound with it, is read untransposed, with no warning). (e) Stamp every chord without the check (`if (true)` for `if (sounds)`) → the same test fails (`Cb0` below the floor: `getSoundingPitch()` raises). (f) Delete the `<for-part>` loop → `aForPartIsDroppedWithAWarning` fails. (g) Check note by note instead of chord by chord (`chordEnd == chordStart` for `inSameChord(staffNotes[chordEnd], staffNotes[chordStart])`) → `aTransposeOutOfRangeIsIgnoredForItsWholeScope` fails (the chord's C1 takes -1, `C1 (-7, -12, NONE) C0`, and the warning counts 1). Revert each; rerun green.

- [ ] **Step 10: The ledger.** «build» `make "PYTHON=$py" dev`; «build» `make "PYTHON=$py" corpus-update-ledger` → 0; `git diff --stat test/musicxml/ledger-external.json` empty; `git diff test/musicxml/ledger.json` → exactly these four added lines and nothing else (stop and report otherwise):

```
  "test/xml_examples/unit_test/transpose_diatonic_not_integer.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "invalid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_for_part.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_out_of_range.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
  "test/xml_examples/unit_test/transpose_pair_inconsistent.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "stable"},
```

- [ ] **Step 11: Whole suites.** `make cpp-tests`, `make py-tests`, `make validate` → 0, no new findings.

- [ ] **Step 12: Commit.** `git add test/xml_examples/unit_test/transpose_pair_inconsistent.musicxml test/xml_examples/unit_test/transpose_diatonic_not_integer.musicxml test/xml_examples/unit_test/transpose_out_of_range.musicxml test/xml_examples/unit_test/transpose_for_part.musicxml test/musicxml/ledger.json maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/score-test.cpp`, message:

```
feat!: correct an inconsistent <transpose> pair and ignore one out of range

A <diatonic> that does not match <chromatic> is replaced by the
conventional interval (the Dvorak trumpets in E, (3, 4), become (2, 4)). A
<transpose> with which a note of its scope would have no sounding pitch is
ignored for its whole scope instead of aborting the load; the previous
transposition stays in force, and a chord with a note it cannot sound
either is read untransposed as a unit. <for-part> is dropped. One coded
warning per correction.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 5: The writer

**Files:**
- Modify: `maiacore/include/maiacore/measure.h` (includes ~4-5; after `toXML` ~497), `maiacore/src/maiacore/measure.cpp` (includes ~3; `toXML` ~531-609), `maiacore/src/maiacore/python_wrapper/py_measure.cpp` (~169), `maiacore/include/maiacore/part.h` (`toXML` Doxygen ~262-268), `maiacore/src/maiacore/part.cpp` (includes; anonymous namespace before `Part::Part`; `toXML` ~190-316), `maiacore/src/maiacore/python_wrapper/py_part.cpp` (~120), `maiacore/include/maiacore/score.h` (`toXML` ~283-289, `toFile` ~298-304 Doxygen), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (~102-105)
- Create: `test/test_musicxml_transpose.py`
- Test: `tests-cpp/src/measure-test.cpp` (after `ToXMLWithComplexContent`)

**Interfaces:** consumes the doubling (Task 1), `maiacore::detail::spelledDiatonicInterval` (Task 2) and the reader (Tasks 3, 4: an export is read back by them). Produces `const std::string Measure::toXML(const int instrumentId, const int identSize, const std::map<std::pair<int, int>, std::string>& beforeNote) const`, and in `test/test_musicxml_transpose.py` the helpers `load(path)`, `export(score)`, `transposes(data, part_index=0)`, `single_part_score(name, measures)` (consumed by Task 6).

- [ ] **Step 1: Failing C++ test.** In `tests-cpp/src/measure-test.cpp` add `#include <map>` and `#include <utility>` beside `<string>`/`<vector>`, and after `TEST(MeasureSerialization, ToXMLWithComplexContent) {...}` insert:

```cpp
// The overload writes the text mapped to (staff, note index) just before that note; with nothing
// mapped it writes what toXML(instrumentId, identSize) writes.
TEST(MeasureSerialization, ToXMLWritesTheInsertionsBeforeTheirNotes) {
    Measure measure(2);
    measure.addNote(Note("C4"), 0);
    measure.addNote(Note("D4"), 0);
    measure.addNote(Note("E4"), 1);

    const std::string xml = measure.toXML(
        1, 2, {{{0, 1}, "<!--before D4-->\n"}, {{1, 0}, "<!--before E4-->\n"}});
    const size_t c4 = xml.find("<step>C</step>");
    const size_t beforeD4 = xml.find("<!--before D4-->");
    const size_t d4 = xml.find("<step>D</step>");
    const size_t beforeE4 = xml.find("<!--before E4-->");
    const size_t e4 = xml.find("<step>E</step>");
    ASSERT_NE(beforeD4, std::string::npos);
    ASSERT_NE(beforeE4, std::string::npos);
    EXPECT_LT(c4, beforeD4);
    EXPECT_LT(beforeD4, d4);
    EXPECT_LT(d4, beforeE4);
    EXPECT_LT(beforeE4, e4);
    EXPECT_EQ(xml.rfind("<note>", d4), xml.find("<note>", beforeD4));

    EXPECT_EQ(measure.toXML(1, 2, {}), measure.toXML(1, 2));
}
```

- [ ] **Step 2: Failing Python tests** — create `test/test_musicxml_transpose.py`:

```python
"""<transpose> through maialib's MusicXML writer, and through an export and an import."""

import contextlib
import io
import sys
import unittest
from pathlib import Path

import maialib as ml

TEST = Path(__file__).resolve().parent
sys.path.insert(0, str(TEST / "musicxml"))

import musicxml_check  # noqa: E402

UNIT_TEST = TEST / "xml_examples" / "unit_test"


def load(path):
    """The score of a MusicXML file, loaded without printing its warnings."""
    with contextlib.redirect_stdout(io.StringIO()):
        return ml.Score(str(path))


def export(score):
    """The score's MusicXML export, as bytes."""
    return score.toXML().encode("utf-8")


def transposes(data, part_index=0):
    """Every <transpose> of a part of an export, in document order: the number of its measure, how
    many <note> elements come before its <attributes> in the measure, its number attribute (or
    None), the text of <diatonic>, <chromatic> and <octave-change> (None when absent), and its
    <double> ("below", "above" or None)."""
    part = musicxml_check.parse_document(data).findall("part")[part_index]
    found = []
    for measure in part.findall("measure"):
        notes = 0
        for child in measure:
            if child.tag == "note":
                notes += 1
            elif child.tag == "attributes":
                for transpose in child.findall("transpose"):
                    double = transpose.find("double")
                    if double is None:
                        doubling = None
                    else:
                        doubling = "above" if double.get("above") == "yes" else "below"
                    found.append(
                        (
                            measure.get("number"),
                            notes,
                            transpose.get("number"),
                            transpose.findtext("diatonic"),
                            transpose.findtext("chromatic"),
                            transpose.findtext("octave-change"),
                            doubling,
                        )
                    )
    return found


def single_part_score(name, measures):
    """A one-part score with a C major key signature in measure 1, which its export needs to be
    loaded again."""
    score = ml.Score([name], measures)
    score.setKeySignature(0, True, 0)
    return score


class TransposeWriterTestCase(unittest.TestCase):
    """The export writes the transpositions its notes hold as <transpose> elements."""

    def test_each_change_is_written_at_the_start_of_its_measure(self):
        data = export(load(UNIT_TEST / "transpose_change_mid_part.musicxml"))
        self.assertEqual(
            [
                ("1", 0, None, "-1", "-2", None, None),
                ("2", 0, None, "-2", "-3", None, None),
                ("4", 0, None, "0", "0", None, None),
            ],
            transposes(data),
        )

    def test_a_change_brought_by_the_first_pitched_note_is_written_at_the_measure_start(self):
        # Measure 2 begins with a rest; its first pitched note brings the change.
        data = export(load(UNIT_TEST / "transpose_later_measure.musicxml"))
        self.assertEqual([("2", 0, None, "-4", "-7", None, None)], transposes(data))

    def test_measure_one_states_the_interval_of_a_first_note_in_a_later_measure(self):
        score = single_part_score("Clarinet in Bb", 2)
        score.getPart(0).getMeasure(1).addNote(
            ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        )
        self.assertEqual([("1", 0, None, "-1", "-2", None, None)], transposes(export(score)))

    def test_number_is_written_only_where_the_staves_differ(self):
        data = export(load(UNIT_TEST / "transpose_per_staff.musicxml"))
        self.assertEqual(
            [
                ("1", 0, "1", "-1", "-2", None, None),
                ("1", 0, "2", "-2", "-3", None, None),
                ("2", 0, "2", "-4", "-7", None, None),
                ("3", 0, None, "-1", "-2", None, None),
            ],
            transposes(data),
        )

    def test_an_octave_transposition_is_written_as_octave_change(self):
        data = export(load(UNIT_TEST / "transpose_octave_change.musicxml"))
        self.assertEqual([("1", 0, None, "0", "0", "1", None)], transposes(data, 0))
        self.assertEqual([("1", 0, None, "-1", "-2", "-1", None)], transposes(data, 1))
        self.assertEqual([("1", 0, None, "0", "0", "-1", None)], transposes(data, 2))

    def test_the_doubling_is_written_as_double(self):
        data = export(load(UNIT_TEST / "transpose_double.musicxml"))
        self.assertEqual([("1", 0, None, "0", "0", None, "below")], transposes(data, 0))
        self.assertEqual([("1", 0, None, "0", "0", None, "above")], transposes(data, 1))

    def test_a_change_after_the_first_pitched_note_is_written_just_before_its_note(self):
        data = export(load(UNIT_TEST / "transpose_after_notes.musicxml"))
        self.assertEqual(
            [("1", 2, None, "-3", "-5", None, None), ("3", 0, None, "0", "0", None, None)],
            transposes(data),
        )
        measure = musicxml_check.parse_document(data).find("part").find("measure")
        self.assertEqual(
            ["attributes", "note", "note", "attributes", "note", "note"],
            [child.tag for child in measure],
        )
        self.assertEqual("E", measure[4].findtext("pitch/step"))

    def test_a_diatonic_interval_of_zero_is_written_as_the_conventional_one(self):
        # A stored 0 and the conventional interval it stands for are one transposition, so the
        # second measure changes nothing.
        score = single_part_score("Clarinet in Bb", 2)
        score.getPart(0).getMeasure(0).addNote(ml.Note("D4", transposeChromatic=-2))
        score.getPart(0).getMeasure(1).addNote(
            ml.Note("D4", transposeDiatonic=-1, transposeChromatic=-2)
        )
        self.assertEqual([("1", 0, None, "-1", "-2", None, None)], transposes(export(score)))

    def test_attributes_are_opened_for_a_transpose_alone(self):
        score = single_part_score("Clarinet", 2)
        score.getPart(0).getMeasure(0).addNote(
            ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2)
        )
        score.getPart(0).getMeasure(1).addNote(
            ml.Note("C4", transposeDiatonic=-2, transposeChromatic=-3)
        )
        second = musicxml_check.parse_document(export(score)).find("part").findall("measure")[1]
        self.assertEqual(["transpose"], [child.tag for child in second.find("attributes")])

    def test_a_chord_whose_notes_transpose_differently_cannot_be_written(self):
        score = single_part_score("Clarinets", 1)
        measure = score.getPart(0).getMeasure(0)
        measure.addNote(ml.Note("C4", transposeDiatonic=-1, transposeChromatic=-2))
        measure.addNote(ml.Note("E4", inChord=True, transposeDiatonic=-2, transposeChromatic=-3))
        with self.assertRaises(RuntimeError) as context:
            score.toXML()
        self.assertIn("Part 'Clarinets', measure 1, staff 1", str(context.exception))

    def test_every_fixture_exports_valid_musicxml(self):
        for path in sorted(UNIT_TEST.glob("transpose_*.musicxml")):
            with self.subTest(fixture=path.name):
                report = musicxml_check.check_bytes(export(load(path)))
                self.assertTrue(report.xsd_valid, report.xsd_errors[:3])
                self.assertEqual([], report.errors)

    def test_the_hash_of_a_score_follows_its_transpositions(self):
        score = single_part_score("Clarinet", 1)
        score.getPart(0).getMeasure(0).addNote(ml.Note("C4"))
        untransposed = hash(score)
        score.getPart(0).setTransposingInterval(-1, -2)
        self.assertNotEqual(untransposed, hash(score))


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 3: Run and see them fail.** C++ subset `MeasureSerialization.*` → build error: `no matching member function for call to 'toXML'`. «pytest» `test_musicxml_transpose.TransposeWriterTestCase` (installed module from Task 4) → every expectation of `<transpose>` fails (`[] != [...]`), the chord test fails (`RuntimeError not raised`), the hash test fails (equal hashes); `test_every_fixture_exports_valid_musicxml` passes already (nothing is written yet).

- [ ] **Step 4: `Measure::toXML` with insertions.** In `measure.h` add `#include <map>` before `#include <string>` and `#include <utility>` before `#include <vector>`; after `const std::string toXML(const int instrumentId = 1, const int identSize = 2) const;` insert:

```cpp

    /**
     * @brief Serializes the measure to MusicXML, writing extra XML before chosen notes.
     * @details As toXML(instrumentId, identSize), and the text mapped to a (staff, note index)
     *          pair is written just before that note: Part::toXML() writes a change of
     *          transposition in the middle of a measure as an `<attributes>` there.
     * @param instrumentId Instrument index.
     * @param identSize Indentation size.
     * @param beforeNote XML text to write before a note, keyed by its 0-based staff and its index
     *        among that staff's notes.
     * @return MusicXML string for the measure.
     */
    const std::string toXML(const int instrumentId, const int identSize,
                            const std::map<std::pair<int, int>, std::string>& beforeNote) const;
```

In `measure.cpp` add `#include <map>`, `#include <string>` and `#include <utility>` after `#include <iostream>`; replace the line `const std::string Measure::toXML(const int instrumentId, const int identSize) const {` with

```cpp
const std::string Measure::toXML(const int instrumentId, const int identSize) const {
    return toXML(instrumentId, identSize, {});
}

const std::string Measure::toXML(
    const int instrumentId, const int identSize,
    const std::map<std::pair<int, int>, std::string>& beforeNote) const {
```

and inside the staff loop replace `            xml.append(currentStave[n].toXML(instrumentId, identSize));` with

```cpp
            const auto insertion = beforeNote.find({s, n});
            if (insertion != beforeNote.end()) {
                xml.append(insertion->second);
            }
            xml.append(currentStave[n].toXML(instrumentId, identSize));
```

In `py_measure.cpp` replace `    cls.def("toXML", &Measure::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2);` with

```cpp
    cls.def("toXML", py::overload_cast<const int, const int>(&Measure::toXML, py::const_),
            py::arg("instrumentId") = 1, py::arg("identSize") = 2);
```

- [ ] **Step 5: The plan and `Part::toXML`.** In `part.cpp` extend the standard includes to

```cpp
#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
```

and before `Part::Part(const std::string& partName, ...` insert:

```cpp
namespace {
// A pitched note's transposition as the MusicXML writer writes it: the diatonic interval the note
// is spelled with (the conventional one for the chromatic interval when the stored one is 0), the
// chromatic interval and the octave doubling. A stored 0 and the conventional interval it stands
// for are one transposition.
struct WrittenTransposition {
    std::int64_t diatonic = 0;
    std::int64_t chromatic = 0;
    OctaveDoubling doubling = OctaveDoubling::NONE;

    bool operator==(const WrittenTransposition& other) const {
        return diatonic == other.diatonic && chromatic == other.chromatic &&
               doubling == other.doubling;
    }
    bool operator!=(const WrittenTransposition& other) const { return !(*this == other); }
};

WrittenTransposition writtenTransposition(const Note& note) {
    WrittenTransposition transposition;
    transposition.diatonic = maiacore::detail::spelledDiatonicInterval(
        note.getTransposeDiatonic(), note.getTransposeChromatic());
    transposition.chromatic = note.getTransposeChromatic();
    transposition.doubling = note.getOctaveDoubling();
    return transposition;
}

// The transposition of the first pitched note of a staff of a measure, if it has one.
std::optional<WrittenTransposition> firstPitchedTransposition(const Measure& measure,
                                                              const int staff) {
    if (staff >= measure.getNumStaves()) {
        return std::nullopt;
    }
    for (int n = 0; n < measure.getNumNotes(staff); n++) {
        const Note& note = measure.getNote(n, staff);
        if (note.isNoteOn() && note.isPitched()) {
            return writtenTransposition(note);
        }
    }
    return std::nullopt;
}

// A <transpose> element: number="staff + 1", or no number when 'staff' is -1. The interval is
// unfolded: <octave-change> takes the chromatic interval's whole octaves, rounded toward zero,
// and <diatonic> and <chromatic> what remains of each; <octave-change> is written only when it is
// not 0, as MusicXML asks for intervals of less than an octave.
std::string transposeXML(const WrittenTransposition& transposition, const int staff,
                         const int identSize) {
    const std::int64_t octaves = transposition.chromatic / 12;
    const std::string inner = Helper::generateIdentation(5, identSize);
    std::string xml = Helper::generateIdentation(4, identSize) + "<transpose";
    if (staff >= 0) {
        xml.append(" number=\"" + std::to_string(staff + 1) + "\"");
    }
    xml.append(">\n");
    xml.append(inner + "<diatonic>" + std::to_string(transposition.diatonic - 7 * octaves) +
               "</diatonic>\n");
    xml.append(inner + "<chromatic>" + std::to_string(transposition.chromatic - 12 * octaves) +
               "</chromatic>\n");
    if (octaves != 0) {
        xml.append(inner + "<octave-change>" + std::to_string(octaves) + "</octave-change>\n");
    }
    if (transposition.doubling == OctaveDoubling::BELOW) {
        xml.append(inner + "<double/>\n");
    } else if (transposition.doubling == OctaveDoubling::ABOVE) {
        xml.append(inner + "<double above=\"yes\"/>\n");
    }
    xml.append(Helper::generateIdentation(4, identSize) + "</transpose>\n");
    return xml;
}

// Where a part's <transpose> elements go, derived from its notes, which hold the transpositions:
// the elements of each measure's <attributes>, and the <attributes> written before a note, keyed
// by (staff, note index).
struct TransposePlan {
    std::vector<std::string> measureStart;
    std::vector<std::map<std::pair<int, int>, std::string>> beforeNote;
};

// Each staff's transposition in force is that of its last pitched note; rests and unpitched notes
// change nothing. Measure 1 states each staff's first pitched note's transposition, which applies
// from the start of the part, when it is not (0, 0, NONE). A later change goes into the
// <attributes> at the start of its measure when the staff's first pitched note there brings it,
// and otherwise into an <attributes> written just before the first note of the chord that brings
// it. A <transpose> has no number when every staff has the same transposition at that point; a
// change in the middle of a measure of a part with more than one staff always has one, as the
// staves are written one after another. A chord whose notes differ cannot be written.
TransposePlan transposePlan(const Part& part, const int identSize) {
    const int numMeasures = part.getNumMeasures();
    TransposePlan plan;
    plan.measureStart.resize(numMeasures);
    plan.beforeNote.resize(numMeasures);

    int numStaves = part.getNumStaves();
    for (int m = 0; m < numMeasures; m++) {
        numStaves = std::max(numStaves, part.getMeasure(m).getNumStaves());
    }

    // The transposition in force on each staff, from the start: that of its first pitched note.
    std::vector<WrittenTransposition> current(numStaves);
    std::vector<bool> found(numStaves, false);
    for (int m = 0; m < numMeasures; m++) {
        for (int s = 0; s < numStaves; s++) {
            if (found[s]) {
                continue;
            }
            const std::optional<WrittenTransposition> first =
                firstPitchedTransposition(part.getMeasure(m), s);
            if (first) {
                current[s] = *first;
                found[s] = true;
            }
        }
    }

    for (int m = 0; m < numMeasures; m++) {
        const Measure& measure = part.getMeasure(m);

        // At the start of the measure: in measure 1, every staff that is transposed or doubled;
        // later, every staff whose first pitched note in the measure changes its transposition.
        std::vector<int> changed;
        for (int s = 0; s < numStaves; s++) {
            if (m == 0) {
                if (current[s] != WrittenTransposition{}) {
                    changed.push_back(s);
                }
                continue;
            }
            const std::optional<WrittenTransposition> first =
                firstPitchedTransposition(measure, s);
            if (first && *first != current[s]) {
                current[s] = *first;
                changed.push_back(s);
            }
        }
        if (!changed.empty()) {
            const WrittenTransposition shared = current[changed.front()];
            const bool everyStaffAlike =
                std::all_of(current.begin(), current.end(),
                            [&shared](const WrittenTransposition& t) { return t == shared; });
            if (everyStaffAlike) {
                plan.measureStart[m] = transposeXML(shared, -1, identSize);
            } else {
                for (const int s : changed) {
                    plan.measureStart[m] += transposeXML(current[s], s, identSize);
                }
            }
        }

        // Inside the measure: a pitched note whose transposition differs from that of the
        // previous pitched note of its staff, after the staff's first one there.
        for (int s = 0; s < measure.getNumStaves(); s++) {
            bool firstPitched = true;
            int chordStart = 0;
            std::optional<WrittenTransposition> chordTransposition;
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                const Note& note = measure.getNote(n, s);
                if (!note.inChord()) {
                    chordStart = n;
                    chordTransposition.reset();
                }
                if (!note.isNoteOn() || !note.isPitched()) {
                    continue;
                }
                const WrittenTransposition transposition = writtenTransposition(note);
                if (chordTransposition && *chordTransposition != transposition) {
                    LOG_ERROR("Part '" + part.getName() + "', measure " + std::to_string(m + 1) +
                              ", staff " + std::to_string(s + 1) +
                              ": the notes of a chord have different transposing intervals or "
                              "octave doublings, which one MusicXML <transpose> cannot express. "
                              "Give every note of the chord the same interval and doubling.");
                }
                chordTransposition = transposition;
                if (firstPitched) {
                    firstPitched = false;
                    continue;
                }
                if (transposition != current[s]) {
                    current[s] = transposition;
                    const int number = (numStaves > 1) ? s : -1;
                    plan.beforeNote[m][{s, chordStart}] =
                        Helper::generateIdentation(3, identSize) + "<attributes>\n" +
                        transposeXML(transposition, number, identSize) +
                        Helper::generateIdentation(3, identSize) + "</attributes>\n";
                }
            }
        }
    }
    return plan;
}
}  // namespace

```

In `Part::toXML`:
- replace

```cpp
    std::string xml;

    const int numMeasures = getNumMeasures();
```

  (the first lines of the function; `std::string xml;` occurs nowhere else in `part.cpp`) with

```cpp
    std::string xml;

    const int numMeasures = getNumMeasures();
    const TransposePlan transposes = transposePlan(*this, identSize);
```

- replace

```cpp
        for (const auto& clef : measureClefs) {
            attributeChanged |= clef.isClefChanged();
        }
```

  with

```cpp
        for (const auto& clef : measureClefs) {
            attributeChanged |= clef.isClefChanged();
        }
        attributeChanged |= !transposes.measureStart[m].empty();
```

- replace

```cpp
            xml.append(Helper::generateIdentation(4, identSize) + "</staff-details>\n");
        }
```

  with

```cpp
            xml.append(Helper::generateIdentation(4, identSize) + "</staff-details>\n");
        }

        // After clef and staff-details, as the MusicXML content model of <attributes> requires.
        xml.append(transposes.measureStart[m]);
```

- replace `        xml.append(_measure[m].toXML(instrumentId, identSize));` with `        xml.append(_measure[m].toXML(instrumentId, identSize, transposes.beforeNote[m]));`.

- [ ] **Step 6: Document the writer.** In `part.h` replace the `toXML` Doxygen block

```cpp
    /**
     * @brief Serializes the part to MusicXML format.
     * @param instrumentId Instrument index (default: 1).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the part.
     */
```

with

```cpp
    /**
     * @brief Serializes the part to MusicXML format.
     * @details The transpositions are written from the notes, which hold them: for each staff, a
     *          `<transpose>` in measure 1 when its first pitched note is transposed or doubled
     *          (it applies from the start of the part), and one wherever a pitched note's interval
     *          or doubling differs from that of the staff's previous pitched note -- in the
     *          measure's `<attributes>` when the note is the staff's first pitched note there,
     *          otherwise in an `<attributes>` written just before the first note of its chord.
     *          Rests and unpitched notes change nothing. A `<transpose>` has no `number` when
     *          every staff has the same transposition at that point; a change in the middle of a
     *          measure of a part with more than one staff always has one. The interval is
     *          unfolded into `<diatonic>`, `<chromatic>` and, from an octave on,
     *          `<octave-change>` (its whole octaves, rounded toward zero), with the diatonic
     *          interval the note is spelled with -- the conventional one when the stored one is 0
     *          -- and `<double/>` or `<double above="yes"/>` states the doubling. The elements go
     *          after `<clef>` and `<staff-details>`; an `<attributes>` is opened for them when
     *          nothing else needs one.
     * @param instrumentId Instrument index (default: 1).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the part.
     * @throws std::runtime_error If the notes of a chord have different transposing intervals or
     *         octave doublings, which one `<transpose>` cannot express; the message names the
     *         part, the measure and the staff.
     */
```

In `score.h` replace

```cpp
     * @brief Exports the score to MusicXML format.
     * @details Generates a complete MusicXML string, including metadata and all parts.
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string.
     */
```

with

```cpp
     * @brief Exports the score to MusicXML format.
     * @details Generates a complete MusicXML string, including metadata and all parts. Each
     *          part's transpositions are written as `<transpose>` elements (see Part::toXML()).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string.
     * @throws std::runtime_error If a part cannot be written (see Part::toXML()).
     */
```

and

```cpp
     * @param compressedXML True to save as .mxl (compressed).
     * @param identSize Indentation size (default: 2).
     */
```

with

```cpp
     * @param compressedXML True to save as .mxl (compressed).
     * @param identSize Indentation size (default: 2).
     * @throws std::runtime_error If fileName is empty, the file cannot be opened, or a part
     *         cannot be written (see Part::toXML()).
     */
```

In `py_part.cpp` replace `    cls.def("toXML", &Part::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2);` with

```cpp
    cls.def("toXML", &Part::toXML, py::arg("instrumentId") = 1, py::arg("identSize") = 2,
            R"pbdoc(
        Return the part's measures as MusicXML text.

        The transpositions are written from the notes, which hold them: for each staff, a
        ``<transpose>`` in measure 1 when its first pitched note is transposed or doubled, and
        one wherever a pitched note's interval or octave doubling differs from that of the
        staff's previous pitched note -- in the measure's ``<attributes>`` when the note is the
        staff's first pitched note there, otherwise in an ``<attributes>`` just before it. Rests
        and unpitched notes change nothing. A ``<transpose>`` has no ``number`` when every staff
        has the same transposition there; the interval is unfolded into ``<diatonic>``,
        ``<chromatic>`` and, from an octave on, ``<octave-change>``, and ``<double/>`` or
        ``<double above="yes"/>`` states the doubling.

        Parameters
        ----------
        instrumentId : int, default 1
            Zero-based index of the part in its score.
        identSize : int, default 2
            Number of spaces per indentation level.

        Returns
        -------
        str
            The ``<measure>`` elements of the part.

        Raises
        ------
        RuntimeError
            If the notes of a chord have different transposing intervals or octave doublings,
            which one ``<transpose>`` cannot express; the message names the part, the measure
            and the staff.
    )pbdoc");
```

In `py_score.cpp` replace

```cpp
    cls.def("toXML", &Score::toXML, py::arg("identSize") = 2);
    cls.def("toJSON", &Score::toJSON);
    cls.def("toFile", &Score::toFile, py::arg("fileName"), py::arg("compressedXML") = false,
            py::arg("identSize") = 2);
```

with

```cpp
    cls.def("toXML", &Score::toXML, py::arg("identSize") = 2,
            R"pbdoc(
        Return the score as MusicXML text. Each part's transpositions are written as
        ``<transpose>`` elements (see ``Part.toXML``).

        Raises
        ------
        RuntimeError
            If a part cannot be written: the notes of a chord have different transposing
            intervals or octave doublings (see ``Part.toXML``).
    )pbdoc");
    cls.def("toJSON", &Score::toJSON);
    cls.def("toFile", &Score::toFile, py::arg("fileName"), py::arg("compressedXML") = false,
            py::arg("identSize") = 2,
            R"pbdoc(
        Write the score as MusicXML to ``fileName`` plus ``.xml``, or to ``fileName`` plus
        ``.mxl`` when ``compressedXML`` is True. Each part's transpositions are written as
        ``<transpose>`` elements (see ``Part.toXML``).

        Raises
        ------
        RuntimeError
            If ``fileName`` is empty, the file cannot be opened, or a part cannot be written
            (see ``Part.toXML``).
    )pbdoc");
```

- [ ] **Step 7: Format, build, pass.** clang-format the nine changed C++ files. C++ subset `MeasureSerialization.*` → pass. `& $py -m ruff format test\test_musicxml_transpose.py; & $py -m ruff check test\test_musicxml_transpose.py` → clean. «build» `make "PYTHON=$py" dev`; «pytest» `test_musicxml_transpose.TransposeWriterTestCase` → 12 OK.

- [ ] **Step 8: Mutations** (`make dev` before each Python run). (a) In `transposeXML` write `number` for every staff (`if (true)` for `if (staff >= 0)`, and `std::max(staff, 0) + 1` for `staff + 1`) → `test_number_is_written_only_where_the_staves_differ` and `test_each_change_is_written_at_the_start_of_its_measure` fail. (b) `const std::int64_t octaves = 0;` → `test_an_octave_transposition_is_written_as_octave_change` fails. (c) Write `<double/>` for both doublings → `test_the_doubling_is_written_as_double` fails. (d) Key the mid-measure insertion `{s, 0}` instead of `{s, chordStart}` → `test_a_change_after_the_first_pitched_note_is_written_just_before_its_note` fails. (e) In `firstPitchedTransposition` drop `note.isNoteOn() &&` (a rest then counts as the first pitched note) → `test_a_change_brought_by_the_first_pitched_note_is_written_at_the_measure_start` fails. (f) Look for each staff's first pitched note in measure 1 only (`m < 1` for `m < numMeasures` in the `found` loop) → `test_measure_one_states_the_interval_of_a_first_note_in_a_later_measure` fails. (g) `transposition.diatonic = note.getTransposeDiatonic();` in `writtenTransposition` → `test_a_diatonic_interval_of_zero_is_written_as_the_conventional_one` fails. (h) Delete `attributeChanged |= !transposes.measureStart[m].empty();` → `test_attributes_are_opened_for_a_transpose_alone` fails. (i) Delete the `LOG_ERROR` chord check → `test_a_chord_whose_notes_transpose_differently_cannot_be_written` fails. (j) Append `transposes.measureStart[m]` before the clefs instead of after `staff-details` → `test_every_fixture_exports_valid_musicxml` fails (`Element 'clef': This element is not expected`). (k) Do not append `transposes.measureStart[m]` → `test_the_hash_of_a_score_follows_its_transpositions` fails. (l) In `Measure::toXML` skip the insertion → `ToXMLWritesTheInsertionsBeforeTheirNotes` fails (C++). Revert each; rerun green.

- [ ] **Step 9: Whole suites.** `make cpp-tests` → 0; `make py-tests` → OK — the corpus test must still match the ledger: the exports of the transposing corpus files now carry `<transpose>` and stay `valid`/`stable` where they were (stop and report any ledger mismatch); `make validate` → no new findings.

- [ ] **Step 10: Commit.** `git add maiacore/include/maiacore/measure.h maiacore/src/maiacore/measure.cpp maiacore/src/maiacore/python_wrapper/py_measure.cpp maiacore/include/maiacore/part.h maiacore/src/maiacore/part.cpp maiacore/src/maiacore/python_wrapper/py_part.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_score.cpp test/test_musicxml_transpose.py tests-cpp/src/measure-test.cpp`, message:

```
feat!: the MusicXML writer writes <transpose> from the notes' transpositions

For each staff, measure 1 states the first pitched note's transposition,
and every change is written where the note that brings it is: in the
measure's <attributes>, or in one just before the note. number only where
the staves differ; octave-change unfolded toward zero; <diatonic> always,
the conventional one for a stored 0; <double>. A chord whose notes
transpose differently raises. Score and Part hashes change for scores with
transpositions.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 6: Round trip, validity, the dump

**Files:**
- Modify: `test/musicxml/dump_score.py` (`note_record` ~58-81), `test/test_musicxml_dump.py` (after `test_a_transposed_note_is_dumped_with_its_interval_and_sounding_pitch`), `test/musicxml/golden/test_staves.dump.json` (regenerated), `test/test_musicxml_transpose.py` (imports; new class), `scripts/make-corpus.py`, `test/musicxml/README.md` (Commands)

**Interfaces:** consumes the reader (Tasks 3, 4), the writer and `test_musicxml_transpose.py`'s helpers (Task 5), `corpus.corpus_files()`, `corpus.is_slow()`, `corpus.load_ledger()`, `corpus.LEDGER`, `musicxml_check.check_bytes()`, `musicxml_check.read_mxl()`. Produces the dump field `"octaveDoubling"`, the test class `TransposeRoundTripTestCase`, and the environment variable `MAIALIB_SLOW_TESTS=1` that `make corpus` sets.

- [ ] **Step 1: Failing dump test** — in `test/test_musicxml_dump.py`, after `test_a_transposed_note_is_dumped_with_its_interval_and_sounding_pitch`, insert:

```python
    def test_a_note_is_dumped_with_its_octave_doubling(self):
        note = ml.Note("C3")
        self.assertEqual("NONE", dump_score.note_record(note)["octaveDoubling"])
        note.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        self.assertEqual("BELOW", dump_score.note_record(note)["octaveDoubling"])
```

«pytest» `test_musicxml_dump` → `KeyError: 'octaveDoubling'`.

- [ ] **Step 2: The dump field** — in `dump_score.py`'s `note_record`, after `        "transpose": [_safe(note.getTransposeDiatonic), _safe(note.getTransposeChromatic)],` insert `        "octaveDoubling": _safe(lambda: note.getOctaveDoubling().name),`. Regenerate the golden: `(cd /c/Users/nyck/Desktop/maialib/test && /c/Users/nyck/AppData/Local/Temp/maialib-1b-venv/Scripts/python.exe musicxml/dump_score.py xml_examples/unit_test/test_staves.xml musicxml/golden/test_staves.dump.json); echo "exit $?"` → 0. `git diff --stat test/musicxml/golden/test_staves.dump.json` → 16 insertions, 0 deletions; `git diff` shows only `+         "octaveDoubling": "NONE",` lines, each between `"grace"` and `"on"`. «pytest» `test_musicxml_dump` → OK.

- [ ] **Step 3: Failing round-trip tests** — in `test/test_musicxml_transpose.py` add `import os`, `import re` and `import tempfile` to the standard imports (in alphabetical order), `import corpus  # noqa: E402` just before `import musicxml_check  # noqa: E402` (ruff's import order), and after `UNIT_TEST = ...` insert:

```python
REPO = TEST.parent
SLOW_TESTS = os.environ.get("MAIALIB_SLOW_TESTS") == "1"
TRANSPOSE_ELEMENT = re.compile(r"Element '(transpose|diatonic|chromatic|octave-change|double)'")

# Every corpus file with a <transpose>, the transpose_*.musicxml fixtures aside, as
# repository-relative paths.
CORPUS_WITH_TRANSPOSE = (
    "maialib/xml-scores-examples/Beethoven_Symphony_5_mov_1.xml",
    "maialib/xml-scores-examples/Dvorak_Symphony_9_mov_4.mxl",
    "maialib/xml-scores-examples/Mahler_Symphony_8_Finale.mxl",
    "maialib/xml-scores-examples/Mozart_Requiem_Introitus.mxl",
    "maialib/xml-scores-examples/Strauss_Also_Sprach_Zarathustra.mxl",
    "test/musicxml/w3c-test-suite/xmlFiles/41c-StaffGroups.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72a-TransposingInstruments.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72b-TransposingInstruments-Full.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72c-TransposingInstruments-Change.musicxml",
    "test/musicxml/w3c-test-suite/xmlFiles/72d-TransposingInstruments-scorePitch.musicxml",
    "test/xml_examples/Beethoven/Symphony_5th_1Mov.xml",
    "test/xml_examples/Beethoven/big_files/Symphony_9th.xml",
    "test/xml_examples/unit_test/test_compressed_file.mxl",
    "test/xml_examples/unit_test/test_getChords.xml",
    "test/xml_examples/unit_test/test_getchords_poly.musicxml",
    "test/xml_examples/unit_test/test_multiple_instruments2.xml",
    "test/xml_examples/unit_test/test_multiple_instruments3.musicxml",
    "test/xml_examples/unit_test/test_pattern.musicxml",
    "test/xml_examples/unit_test/test_stack_chords.xml",
    "test/xml_examples/unit_test/test_stack_multiple_staves.xml",
    "test/xml_examples/unit_test/xakypueri.xml",
)

# They do not load: a first measure without <key>, two clefs in one measure; W3C 72d covers
# 72b's transpositions.
NOT_LOADABLE = (
    "maialib/xml-scores-examples/Mozart_Requiem_Introitus.mxl",
    "test/musicxml/w3c-test-suite/xmlFiles/72b-TransposingInstruments-Full.musicxml",
)
```

after `single_part_score` insert:

```python
def note_transpositions(score):
    """Each sounding pitched note's written pitch, sounding pitch, interval and octave doubling,
    keyed by its part, measure, staff and index."""
    found = {}
    for p in range(score.getNumParts()):
        part = score.getPart(p)
        for m in range(part.getNumMeasures()):
            measure = part.getMeasure(m)
            for s in range(measure.getNumStaves()):
                for n in range(measure.getNumNotes(s)):
                    note = measure.getNote(n, s)
                    if note.isNoteOn() and note.isPitched():
                        found[(p, m, s, n)] = (
                            note.getWrittenPitch(),
                            note.getSoundingPitch(),
                            note.getTransposeDiatonic(),
                            note.getTransposeChromatic(),
                            note.getOctaveDoubling().name,
                        )
    return found


def reloaded(data):
    """The score an export loads back as, and what the load printed."""
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "round-trip.musicxml"
        path.write_bytes(data)
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            score = ml.Score(str(path))
    return score, printed.getvalue()


def contains_transpose(name):
    """Whether a corpus file's MusicXML document holds a <transpose>."""
    data = (REPO / name).read_bytes()
    if data[:2] == b"PK":
        data = musicxml_check.read_mxl(data)[0] or b""
    return b"<transpose" in data
```

and before `if __name__ == "__main__":` insert:

```python
class TransposeRoundTripTestCase(unittest.TestCase):
    """Export -> import keeps each note's sounding pitch, spelling, interval and doubling, and
    each corpus file's export is valid where the 4a ledger says the file's export is."""

    def check_round_trip(self, path, ledger_record=None):
        """The round trip of a file; with its ledger record, also the export's validity. The
        fixtures' exports are validated by TransposeWriterTestCase."""
        score = load(path)
        data = export(score)
        again, _ = reloaded(data)
        self.assertEqual(note_transpositions(score), note_transpositions(again))
        if ledger_record is None:
            return
        report = musicxml_check.check_bytes(data)
        self.assertEqual(
            [], [error for error in report.xsd_errors if TRANSPOSE_ELEMENT.search(error)]
        )
        if ledger_record.get("export_xsd") == "valid":
            self.assertTrue(report.xsd_valid, report.xsd_errors[:3])
        self.assertEqual(ledger_record.get("export_errors", []), report.errors)

    def test_every_fixture_keeps_its_transpositions(self):
        fixtures = sorted(UNIT_TEST.glob("transpose_*.musicxml"))
        self.assertEqual(14, len(fixtures))
        for path in fixtures:
            with self.subTest(fixture=path.name):
                self.check_round_trip(path)

    def test_every_corpus_file_with_a_transpose_keeps_its_transpositions(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        for name in CORPUS_WITH_TRANSPOSE:
            if name in NOT_LOADABLE or corpus.is_slow(name):
                continue
            with self.subTest(file=name):
                self.check_round_trip(REPO / name, ledger[name])

    @unittest.skipUnless(SLOW_TESTS, "the slow corpus files run under `make corpus`")
    def test_the_slow_corpus_files_keep_their_transpositions(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        for name in CORPUS_WITH_TRANSPOSE:
            if corpus.is_slow(name):
                with self.subTest(file=name):
                    self.check_round_trip(REPO / name, ledger[name])

    def test_the_list_holds_every_corpus_file_with_a_transpose(self):
        found = {
            name
            for name in corpus.corpus_files()
            if not corpus.is_slow(name) and "/transpose_" not in name and contains_transpose(name)
        }
        expected = {name for name in CORPUS_WITH_TRANSPOSE if not corpus.is_slow(name)}
        self.assertEqual(expected, found)

    def test_a_stored_diatonic_interval_of_zero_comes_back_as_the_conventional_one(self):
        score = single_part_score("Clarinet in Bb", 1)
        score.getPart(0).getMeasure(0).addNote(ml.Note("F#4", transposeChromatic=-2))
        again, printed = reloaded(export(score))
        note = again.getPart(0).getMeasure(0).getNote(0)
        self.assertEqual(
            (-1, -2, "E4"),
            (note.getTransposeDiatonic(), note.getTransposeChromatic(), note.getSoundingPitch()),
        )
        self.assertNotIn("[WARN]", printed)
```

- [ ] **Step 4: Run them.** «pytest» `test_musicxml_transpose.TransposeRoundTripTestCase` → OK with one skip (the slow files). These tests check, across the whole corpus, what Tasks 3–5 implement, so they pass on arrival; the mutations of Step 7 are their failing runs (the dump test of Step 1 failed first in the usual way). Then Bash `(cd /c/Users/nyck/Desktop/maialib/test && MAIALIB_SLOW_TESTS=1 /c/Users/nyck/AppData/Local/Temp/maialib-1b-venv/Scripts/python.exe -m unittest test_musicxml_transpose.TransposeRoundTripTestCase.test_the_slow_corpus_files_keep_their_transpositions -v); echo "exit $?"` → OK (about two minutes: `Symphony_9th.xml` takes most of it). If a corpus file fails here for a reason that has nothing to do with `<transpose>` -- `Mahler_Symphony_8_Finale.mxl`, whose ledger round trip is `unstable`, is the likeliest -- stop and report it; do not weaken an assertion or drop the file.

- [ ] **Step 5: `make corpus` runs the slow round trip.** In `scripts/make-corpus.py` replace `import argparse\nimport sys` with `import argparse\nimport os\nimport sys`, `from build_utils import REPO_ROOT` with `from build_utils import REPO_ROOT, run_step`, and before `    if failed:` insert:

```python
    if not arguments.update_ledger and not failed:
        # `make py-tests` skips the slow corpus files; their <transpose> round trip runs here,
        # once the corpus matches its ledgers, so that a ledger difference is always reported.
        os.environ["MAIALIB_SLOW_TESTS"] = "1"
        run_step(
            [
                sys.executable,
                "-m",
                "unittest",
                "test_musicxml_transpose.TransposeRoundTripTestCase",
            ],
            "the <transpose> round trip of the corpus",
            cwd=str(REPO_ROOT / "test"),
        )
```

In `test/musicxml/README.md` replace `- \`make corpus\` runs every file, slow ones included.` with

```
- `make corpus` runs every file, slow ones included, then, when every file matches its ledger,
  the `<transpose>` round trip of `test_musicxml_transpose.py` with `MAIALIB_SLOW_TESTS=1`, which
  adds the slow corpus files that `make py-tests` skips.
```

- [ ] **Step 6: `make corpus`.** «build» `make "PYTHON=$py" corpus; $LASTEXITCODE` → 0: no ledger difference (in-repository and external), and the round trip runs its slow test (`test_the_slow_corpus_files_keep_their_transpositions ... ok`, not `skipped`). Any ledger difference is reported, not committed.

- [ ] **Step 7: Mutations.** (a) In `transposeXML` drop the `<double>` lines, `make dev` → `test_every_fixture_keeps_its_transpositions` fails (`transpose_double`: `BELOW` against `NONE`). (b) In `transposeXML` delete the `if (octaves != 0) {...}` block that writes `<octave-change>` (the unfolded `<diatonic>` and `<chromatic>` stay), `make dev` → `test_every_corpus_file_with_a_transpose_keeps_its_transpositions` fails (41c's contrabass reads back an octave higher); with `MAIALIB_SLOW_TESTS=1` the slow test fails too (xakypueri). (c) Remove `"test/xml_examples/unit_test/test_pattern.musicxml",` from `CORPUS_WITH_TRANSPOSE` → `test_the_list_holds_every_corpus_file_with_a_transpose` fails. (d) Write the stored diatonic interval (`transposition.diatonic = note.getTransposeDiatonic();`), `make dev` → `test_a_stored_diatonic_interval_of_zero_comes_back_as_the_conventional_one` fails (the reload prints `[transpose-pair-corrected]`). (e) Dump: drop the `"octaveDoubling"` line → `test_a_note_is_dumped_with_its_octave_doubling` and both golden tests fail. (f) In `make-corpus.py` drop the `os.environ[...]` line → `make corpus` reports the slow test `skipped`; this is an observation of `make corpus`'s output, not a failing test (record the output). Revert each; `make dev`; rerun green.

- [ ] **Step 8: Whole suites.** `make py-tests` → OK; `ruff format --check` and `ruff check` on `test/test_musicxml_transpose.py`, `test/musicxml/dump_score.py`, `test/test_musicxml_dump.py`, `scripts/make-corpus.py` → clean.

- [ ] **Step 9: Commit.** `git add test/musicxml/dump_score.py test/test_musicxml_dump.py test/musicxml/golden/test_staves.dump.json test/test_musicxml_transpose.py scripts/make-corpus.py test/musicxml/README.md`, message:

```
test: export and import keep each note's transposition; the dump records the doubling

Every transpose_*.musicxml fixture and every corpus file with a
<transpose> keeps each note's sounding pitch, spelling, interval and
doubling through toXML() and a reload, and a corpus file's export has no
schema error about a <transpose>. make corpus adds the slow files once
the corpus matches its ledgers. dump_score.py
records octaveDoubling; the golden dump changes accordingly.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 7: The concert key in `getChords`

**Files:**
- Modify: `maiacore/src/maiacore/score.cpp` (anonymous namespace; `getChordsPerEachNoteEvent` ~2859-2943), `maiacore/include/maiacore/score.h` (`getChords` Doxygen ~719), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (`getChords` docstring ~467-500)
- Test: `tests-cpp/src/score-test.cpp` (append), `test/test_score_comprehensive.py` (new class after `ScoreAnalysisTestCase`)

**Interfaces:** consumes `spelledDiatonicInterval` (Task 2), the reader (Tasks 3, 4). Produces, in `score.cpp`'s anonymous namespace, `std::vector<std::pair<std::int64_t, std::int64_t>> intervalsPerMeasure(const Part& part)`, `int keyRangeFifths(std::int64_t fifths)`, `std::vector<Key> concertKeys(const std::vector<Part>& parts)`; in `score-test.cpp` the helpers `chordKeys`, `KeyedPart`, `keyedScore`, `wholeNote` (consumed by Task 8).

- [ ] **Step 1: Failing C++ tests** — append to `tests-cpp/src/score-test.cpp`:

```cpp
// ====================
// The concert key of getChords()
// ====================

namespace {
// The (fifths, major) of the key of every chord getChords() finds.
std::set<std::pair<int, bool>> chordKeys(Score& score,
                                         const nlohmann::json& config = nlohmann::json()) {
    std::set<std::pair<int, bool>> keys;
    for (const auto& chord : score.getChords(config)) {
        keys.insert({std::get<2>(chord).getFifthCircle(), std::get<2>(chord).isMajorMode() != 0});
    }
    return keys;
}

// A whole note written 'pitch' on an instrument transposing by (diatonic, chromatic).
Note wholeNote(const std::string& pitch, const int diatonic = 0, const int chromatic = 0) {
    return transposingNote(pitch, diatonic, chromatic, RhythmFigure::WHOLE);
}

// One part of a one-measure score: its name, its written key and the whole note it plays.
struct KeyedPart {
    std::string name;
    int fifths;
    bool major;
    Note note;
};

Score keyedScore(const std::vector<KeyedPart>& parts) {
    std::vector<std::string> names;
    for (const KeyedPart& part : parts) {
        names.push_back(part.name);
    }
    Score score(names, 1);
    for (size_t p = 0; p < parts.size(); p++) {
        Measure& measure = score.getPart(static_cast<int>(p)).getMeasure(0);
        measure.setKey(parts[p].fifths, parts[p].major);
        measure.addNote(parts[p].note);
    }
    return score;
}
}  // namespace

// The W3C transposing-instrument files report the concert key: 72a's piano gives C major against
// the trumpet's written D and the horn's written A; 72c, whose only part transposes, gives B-flat
// major in both its measures, as an E-flat clarinet in G and then a B-flat clarinet in C; 72d's
// untransposed parts give G major.
TEST(ScoreConcertKey, theW3cTransposingInstrumentsReportTheConcertKey) {
    Score a(kW3c + "72a-TransposingInstruments.musicxml");
    EXPECT_EQ(chordKeys(a), (std::set<std::pair<int, bool>>{{0, true}}));
    Score c(kW3c + "72c-TransposingInstruments-Change.musicxml");
    EXPECT_EQ(chordKeys(c), (std::set<std::pair<int, bool>>{{-2, true}}));
    Score d(kW3c + "72d-TransposingInstruments-scorePitch.musicxml");
    EXPECT_EQ(chordKeys(d), (std::set<std::pair<int, bool>>{{1, true}}));
}

// test_pattern's horn in F is written in G major; its trombone gives the concert key, C major.
TEST(ScoreConcertKey, aHornWrittenInGDoesNotGiveTheKey) {
    Score score(kUnitTest + "test_pattern.musicxml");
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{0, true}}));
}

// The Beethoven 5 sample's C trumpet and timpani are untransposed and written without a key
// signature; the eight other untransposed parts, contrabasses included, write C minor's three
// flats, which win.
TEST(ScoreConcertKey, theBeethovenSampleIsInCMinor) {
    Score score(kSamples + "Beethoven_Symphony_5_mov_1.xml");
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-3, false}}));
}

TEST(ScoreConcertKey, theMostFrequentKeyWins) {
    Score score = keyedScore({{"Timpani", 0, true, wholeNote("C3")},
                              {"Violin", -3, false, wholeNote("C4")},
                              {"Viola", -3, false, wholeNote("G3")}});
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-3, false}}));
}

// A part transposed by whole octaves only is key-neutral: the piccolo counts with the flute, and
// their G major outweighs the timpani's C major.
TEST(ScoreConcertKey, anOctaveTranspositionIsKeyNeutral) {
    Score score = keyedScore({{"Piccolo", 1, true, wholeNote("D5", 7, 12)},
                              {"Timpani", 0, true, wholeNote("G2")},
                              {"Flute", 1, true, wholeNote("D5")}});
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{1, true}}));
}

TEST(ScoreConcertKey, aTieGoesToTheFirstPart) {
    Score score = keyedScore(
        {{"Flute", -1, true, wholeNote("C5")}, {"Oboe", 2, true, wholeNote("D5")}});
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-1, true}}));
}

TEST(ScoreConcertKey, percussionDoesNotCount) {
    Score score = keyedScore({{"Violin", -1, true, wholeNote("C4")},
                              {"Snare Drum", 0, true, wholeNote("C4")},
                              {"Bass Drum", 0, true, wholeNote("C4")}});
    score.getPart(1).setIsPitched(false);
    score.getPart(2).setIsPitched(false);
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-1, true}}));
}

// A part's interval in a measure without pitched notes is that of its last pitched note before
// the measure, or else of its first one after: the horn rests in measures 1 and 3 and still
// transposes there, so its written G major does not count, and the violin gives the key.
TEST(ScoreConcertKey, aMeasureOfRestsTakesTheIntervalOfTheNeighbouringNotes) {
    Score score({"Horn in F", "Violin"}, 3);
    for (int m = 0; m < 3; m++) {
        for (int p = 0; p < 2; p++) {
            Measure& measure = score.getPart(p).getMeasure(m);
            measure.setNumber(m);
            measure.setKey(p == 0 ? 1 : 0, true);
        }
        score.getPart(1).getMeasure(m).addNote(wholeNote("C4"));
    }
    score.getPart(0).getMeasure(0).addNote(Note("rest", RhythmFigure::WHOLE));
    score.getPart(0).getMeasure(1).addNote(wholeNote("D5", -4, -7));
    score.getPart(0).getMeasure(2).addNote(Note("rest", RhythmFigure::WHOLE));
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{0, true}}));
}

// A pitched part without any pitched note counts as untransposed, with its written key.
TEST(ScoreConcertKey, aPartWithoutNotesCountsAsUntransposed) {
    Score score({"Violin", "Flute 2", "Flute 3"}, 1);
    score.getPart(0).getMeasure(0).setKey(0, true);
    score.getPart(0).getMeasure(0).addNote(wholeNote("C4"));
    score.getPart(1).getMeasure(0).setKey(-1, true);
    score.getPart(2).getMeasure(0).setKey(-1, true);
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-1, true}}));
}

// When every pitched part transposes, the first part's written key is moved by its interval and
// brought by twelves into -6..11 fifths: a trumpet in D written with 11 fifths sounds 13, held as
// 1; a B-flat clarinet written with -6 fifths, minor, sounds -8, held as 4, minor.
TEST(ScoreConcertKey, whenEveryPartTransposesTheFirstPartsKeyIsMovedAndWrapped) {
    Score trumpet = keyedScore({{"Trumpet in D", 11, true, wholeNote("C4", 1, 2)}});
    EXPECT_EQ(chordKeys(trumpet), (std::set<std::pair<int, bool>>{{1, true}}));
    Score clarinet = keyedScore({{"Clarinet in Bb", -6, false, wholeNote("C4", -1, -2)}});
    EXPECT_EQ(chordKeys(clarinet), (std::set<std::pair<int, bool>>{{4, false}}));
}

// Every part counts, also those partNames leaves out: the violin, in F major, gives the key of the
// clarinet's chords.
TEST(ScoreConcertKey, partsLeftOutOfTheAnalysisStillCount) {
    Score score = keyedScore({{"Clarinet in Bb", 2, true, wholeNote("D4", -1, -2)},
                              {"Violin", -1, true, wholeNote("F4")}});
    nlohmann::json config;
    config["partNames"] = std::vector<std::string>{"Clarinet in Bb"};
    EXPECT_EQ(chordKeys(score, config), (std::set<std::pair<int, bool>>{{-1, true}}));
}
```

- [ ] **Step 2: Failing Python test** — in `test/test_score_comprehensive.py`, after the `ScoreAnalysisTestCase` class, insert:

```python
class ScoreConcertKeyTestCase(unittest.TestCase):
    """getChordsDataFrame's key column is the concert key."""

    def test_the_key_column_is_the_concert_key(self):
        """W3C 72a's trumpet in B-flat is written in D major and its horn in E-flat in A major;
        its untransposed piano gives the concert key, C major."""
        score = ml.Score("./musicxml/w3c-test-suite/xmlFiles/72a-TransposingInstruments.musicxml")
        table = score.getChordsDataFrame()
        keys = {(key.getFifthCircle(), bool(key.isMajorMode())) for key in table["key"]}
        self.assertEqual({(0, True)}, keys)
```

- [ ] **Step 3: Run and see them fail.** C++ subset `ScoreConcertKey.*` → 72a reports `{(2, true)}`, 72c `{(0, true), (1, true)}`, `test_pattern` `{(1, true)}`, `theMostFrequentKeyWins` `{(0, true)}`, the rests test `{(1, true)}`, the part without notes `{(0, true)}`, the wrap test `{(11, true)}`, `partsLeftOutOfTheAnalysisStillCount` `{(2, true)}`. Four pass by accident, because today's key is part 0's and part 0 is the expected one there: `anOctaveTranspositionIsKeyNeutral`, `aTieGoesToTheFirstPart`, `percussionDoesNotCount` and `theBeethovenSampleIsInCMinor`; their failing runs are the mutations (k), (b), (c) and (j) of Step 7. «pytest» `test_score_comprehensive.ScoreConcertKeyTestCase` → `{(2, True)} != {(0, True)}`.

- [ ] **Step 4: The concert key.** In `score.cpp`'s anonymous namespace, after `applyTranspositions`, insert:

```cpp

// A part's transposing interval at each measure, as the concert key reads it: that of the
// measure's first pitched note, staves in order; without one, that of the part's last pitched
// note before the measure; without one, that of its first pitched note after it; without any,
// untransposed. Each is (the diatonic interval the speller uses, the chromatic interval).
std::vector<std::pair<std::int64_t, std::int64_t>> intervalsPerMeasure(const Part& part) {
    using TransposingInterval = std::pair<std::int64_t, std::int64_t>;
    const int numMeasures = part.getNumMeasures();
    std::vector<std::optional<TransposingInterval>> first(numMeasures);
    std::vector<std::optional<TransposingInterval>> last(numMeasures);
    for (int m = 0; m < numMeasures; m++) {
        const Measure& measure = part.getMeasure(m);
        for (int s = 0; s < measure.getNumStaves(); s++) {
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                const Note& note = measure.getNote(n, s);
                if (!note.isNoteOn() || !note.isPitched()) {
                    continue;
                }
                const TransposingInterval interval{
                    maiacore::detail::spelledDiatonicInterval(note.getTransposeDiatonic(),
                                                              note.getTransposeChromatic()),
                    note.getTransposeChromatic()};
                if (!first[m]) {
                    first[m] = interval;
                }
                last[m] = interval;
            }
        }
    }

    int firstPitchedMeasure = -1;
    for (int m = 0; m < numMeasures && firstPitchedMeasure < 0; m++) {
        if (first[m]) {
            firstPitchedMeasure = m;
        }
    }
    std::vector<TransposingInterval> intervals(numMeasures, TransposingInterval{0, 0});
    std::optional<TransposingInterval> before;
    for (int m = 0; m < numMeasures; m++) {
        if (first[m]) {
            intervals[m] = *first[m];
        } else if (before) {
            intervals[m] = *before;
        } else if (firstPitchedMeasure >= 0) {
            intervals[m] = *first[firstPitchedMeasure];
        }
        if (last[m]) {
            before = last[m];
        }
    }
    return intervals;
}

// The fifths of a key brought into -6..11, the range Key accepts: a value inside it is kept, one
// outside it becomes the enharmonically equal value inside it, twelve fifths away (13 gives 1,
// -8 gives 4).
int keyRangeFifths(std::int64_t fifths) {
    if (fifths > 11) {
        fifths -= 12 * ((fifths - 11 + 11) / 12);
    }
    if (fifths < -6) {
        fifths += 12 * ((-6 - fifths + 11) / 12);
    }
    return static_cast<int>(fifths);
}

// The concert key of each measure: the most frequent written key among the pitched parts whose
// interval at the measure is key-neutral, 7 * chromatic - 12 * diatonic == 0 (untransposed, or
// transposed by whole octaves only). A key is its fifths and its mode; a tie goes to the key of
// the first such part in score order. When no pitched part is key-neutral there, part 0's written
// key moved by its interval's 7 * chromatic - 12 * diatonic fifths, brought into the range Key
// accepts, with part 0's mode.
std::vector<Key> concertKeys(const std::vector<Part>& parts) {
    if (parts.empty()) {
        return {};
    }
    const int numMeasures = parts.at(0).getNumMeasures();
    std::vector<std::vector<std::pair<std::int64_t, std::int64_t>>> intervals;
    intervals.reserve(parts.size());
    for (const Part& part : parts) {
        intervals.push_back(intervalsPerMeasure(part));
    }

    std::vector<Key> keys(numMeasures);
    for (int m = 0; m < numMeasures; m++) {
        // Each key-neutral part's written key and how many parts write it, in score order.
        std::vector<std::pair<Key, int>> tallies;
        for (size_t p = 0; p < parts.size(); p++) {
            if (!parts[p].isPitched() || m >= parts[p].getNumMeasures()) {
                continue;
            }
            const std::pair<std::int64_t, std::int64_t>& interval = intervals[p][m];
            if (7 * interval.second - 12 * interval.first != 0) {
                continue;
            }
            const Key written = parts[p].getMeasure(m).getKey();
            const auto tally =
                std::find_if(tallies.begin(), tallies.end(),
                             [&written](const std::pair<Key, int>& candidate) {
                                 return candidate.first.getFifthCircle() ==
                                            written.getFifthCircle() &&
                                        candidate.first.isMajorMode() == written.isMajorMode();
                             });
            if (tally == tallies.end()) {
                tallies.emplace_back(written, 1);
            } else {
                tally->second++;
            }
        }
        if (!tallies.empty()) {
            // std::max_element keeps the first of equal counts: the key of the earliest part.
            keys[m] = std::max_element(tallies.begin(), tallies.end(),
                                       [](const std::pair<Key, int>& a,
                                          const std::pair<Key, int>& b) {
                                           return a.second < b.second;
                                       })
                          ->first;
            continue;
        }
        const Key written = parts.at(0).getMeasure(m).getKey();
        const std::pair<std::int64_t, std::int64_t>& interval = intervals.at(0)[m];
        keys[m] = Key(keyRangeFifths(written.getFifthCircle() + 7 * interval.second -
                                     12 * interval.first),
                      written.isMajorMode() != 0);
    }
    return keys;
}
```

In `Score::getChordsPerEachNoteEvent`, before `    for (const float startTime : uniqueStartTime) {` insert

```cpp
    // The concert key of each measure, reported with each of its chords.
    const std::vector<Key> keys = concertKeys(_part);

```

and replace

```cpp
        // Get the current measure Key
        const int measureIdx = measurePtr->getNumber();
        const Key& key = _part.at(0).getMeasure(measureIdx).getKey();
```

with

```cpp
        const int measureIdx = measurePtr->getNumber();
        const Key& key = keys.at(measureIdx);
```

- [ ] **Step 5: Document it.** In `score.h`, in `getChords`' Doxygen, replace `     *          3. **Key** (Key object): Prevailing key signature at this position` with

```cpp
     *          3. **Key** (Key object): the concert key of the chord's measure -- the most
     *             frequent written key among the pitched parts that are untransposed there, or
     *             transposed by whole octaves only (a key is its fifths and its mode; a tie goes
     *             to the part that comes first; every pitched part counts, also those `partNames`
     *             leaves out, and unpitched parts never do). A part's transposition at a measure
     *             is that of its first pitched note there; in a measure without one, that of its
     *             last pitched note before, or else of its first one after. When every pitched
     *             part transposes, the first part's written key moved by its interval, by
     *             7 fifths per semitone less 12 per letter, and brought by twelves into -6..11
     *             fifths, the range Key accepts (13 becomes 1)
```

In `py_score.cpp`'s `getChords` docstring replace

```
            measure); the key of that measure in the score's first part, as that part writes it,
            even when ``partNames`` leaves the part out; the chord; and whether every note of
            the chord starts at that onset.
```

with

```
            measure); the concert key of that measure (see above); the chord; and whether
            every note of the chord starts at that onset.
```

and replace its lines

```
        are untransposed notes at that pitch (see ``Chord``).

        Parameters
```

with

```
        are untransposed notes at that pitch (see ``Chord``).

        The key reported with each chord is the concert key of its measure: the written key
        that most pitched parts have there among those untransposed in that measure, or
        transposed by whole octaves only -- a key is its fifths and its mode, and a tie goes to
        the part that comes first. Unpitched parts do not count; every pitched part does, also
        those ``partNames`` leaves out. A part's transposition at a measure is that of its first
        pitched note there; in a measure without one, that of its last pitched note before, or
        else of its first one after. When every pitched part transposes, it is the first part's
        written key moved by that part's transposing interval -- 7 fifths per semitone, less 12
        per letter, so -2 fifths for a B-flat clarinet -- and brought by twelves into the range
        ``Key`` accepts, -6 to 11 fifths (13 becomes 1).

        Parameters
```

- [ ] **Step 6: Format, build, pass.** clang-format `score.cpp`, `score.h`, `py_score.cpp`, `score-test.cpp`. C++ subset `ScoreConcertKey.*:ScoreGetChords.*` → pass. `make dev`; «pytest» `test_score_comprehensive.ScoreConcertKeyTestCase` → OK.

- [ ] **Step 7: Mutations** (C++ subset). (a) `keys[m] = tallies.front().first;` → `theMostFrequentKeyWins` fails. (b) Compare with `a.second <= b.second` in `max_element` (the last of equal counts wins) → `aTieGoesToTheFirstPart` fails. (c) Drop `!parts[p].isPitched() ||` → `percussionDoesNotCount` fails. (d) In `intervalsPerMeasure` leave measures without a pitched note at `{0, 0}` (delete the `else if` branches) → `aMeasureOfRestsTakesTheIntervalOfTheNeighbouringNotes` fails. (e) In `intervalsPerMeasure` initialise `intervals` with `Interval{1, 0}` instead of `Interval{0, 0}` (a part without pitched notes then counts as transposed; every other part's measures are overwritten) → `aPartWithoutNotesCountsAsUntransposed` fails. (f) `keyRangeFifths` returns `static_cast<int>(fifths)` unchanged → `whenEveryPartTransposesTheFirstPartsKeyIsMovedAndWrapped` fails (`Key` raises for 13). (g) Count only part 0 (`if (p > 0) { continue; }` first in the loop) → `partsLeftOutOfTheAnalysisStillCount` fails. (h) Use measure 0's interval for every measure (`intervals[p][0]`) → the 72c expectation fails (`{(-3, true), (-2, true)}`). (i) Report part 0's written key again (`_part.at(0).getMeasure(measureIdx).getKey()` for `keys.at(measureIdx)`), `make dev` → `test_the_key_column_is_the_concert_key` fails, and so do the C++ file tests. (j) `std::min_element` for `std::max_element` → `theMostFrequentKeyWins` and `theBeethovenSampleIsInCMinor` fail (`{(0, true)}`). (k) Test `interval.first != 0 || interval.second != 0` instead of `7 * interval.second - 12 * interval.first != 0` → `anOctaveTranspositionIsKeyNeutral` fails (`{(0, true)}`). Revert each; rerun green.

- [ ] **Step 8: Whole suites.** `make cpp-tests`, `make py-tests`, `make validate` → 0, no new findings.

- [ ] **Step 9: Commit.** `git add maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/score-test.cpp test/test_score_comprehensive.py`, message:

```
feat!: getChords reports the concert key

The key of each chord is the most frequent written key among the pitched
parts that are untransposed at its measure or transposed by whole octaves
(a tie goes to the first part); when every pitched part transposes, the
first part's written key moved by its interval and wrapped into -6..11. It
was the first part's written key, which disagreed with the concert-pitch
chords whenever that part transposes.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 8: The doubling in `getChords` and `plotPianoRoll`

**Files:**
- Modify: `maiacore/src/maiacore/score.cpp` (anonymous namespace; the chord loop of `getChordsPerEachNoteEvent`), `maiacore/include/maiacore/score.h` (`getChords` Doxygen), `maiacore/include/maiacore/note.h` (`getOctaveDoubling` Doxygen), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (`getChords` docstring), `maiacore/src/maiacore/python_wrapper/py_note.cpp` (`getOctaveDoubling` docstring), `maialib/maiapy/plots.py` (`_score2DataFrame` ~18-181, `plotPianoRoll` ~293-320)
- Test: `tests-cpp/src/score-test.cpp` (append), `test/test_maiapy.py` (new class)

**Interfaces:** consumes `Note::getOctaveDoubling` (Task 1), the score-test helpers (Tasks 3, 7). Produces `std::optional<Note> octaveDoublingNote(const Note& note, std::set<const Note*>& reported)` in `score.cpp`'s anonymous namespace, and `_score2DataFrame(score, kwargs, octaveDoublings=False)` in `plots.py`.

- [ ] **Step 1: Failing C++ tests** — append to `tests-cpp/src/score-test.cpp`:

```cpp
// ====================
// The octave doubling in getChords()
// ====================

namespace {
// The pitches a chord sounds, lowest first.
std::vector<std::string> soundingPitches(const Chord& chord) {
    std::vector<std::string> pitches;
    for (const Note& note : chord.getNotes()) {
        pitches.push_back(note.getSoundingPitch());
    }
    return pitches;
}

Note doubledNote(Note note, const OctaveDoubling doubling) {
    note.setOctaveDoubling(doubling);
    return note;
}
}  // namespace

TEST(ScoreOctaveDoubling, aDoubledNoteAddsItsOctaveToTheChord) {
    Score score({"Violoncello and Contrabass", "Flute and Piccolo"}, 1);
    score.getPart(0).getMeasure(0).addNote(doubledNote(wholeNote("C3"), OctaveDoubling::BELOW));
    score.getPart(1).getMeasure(0).addNote(doubledNote(wholeNote("G4"), OctaveDoubling::ABOVE));
    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 1u);
    EXPECT_EQ(soundingPitches(std::get<3>(chords[0])),
              (std::vector<std::string>{"C2", "C3", "G4", "G5"}));
}

// The doubled octave is an octave from what the note sounds: a bass clarinet's written D4 sounds
// C3, and its doubling below C2.
TEST(ScoreOctaveDoubling, theDoubledOctaveIsAnOctaveFromWhatTheNoteSounds) {
    Score score({"Bass Clarinet"}, 1);
    score.getPart(0).getMeasure(0).addNote(
        doubledNote(wholeNote("D4", -8, -14), OctaveDoubling::BELOW));
    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 1u);
    EXPECT_EQ(soundingPitches(std::get<3>(chords[0])), (std::vector<std::string>{"C2", "C3"}));
}

// A doubled octave outside the representable range is left out of every chord the note sounds in,
// with one warning for the note.
TEST(ScoreOctaveDoubling, aDoubledOctaveOutOfRangeIsLeftOutWithOneWarning) {
    Score score({"Contrabass", "Violin"}, 1);
    score.getPart(0).getMeasure(0).addNote(doubledNote(wholeNote("C-1"), OctaveDoubling::BELOW));
    for (int i = 0; i < 4; i++) {
        score.getPart(1).getMeasure(0).addNote(Note("G4"));
    }
    StdoutCapture capture;
    const auto chords = score.getChords();
    const std::string printed = capture.str();
    ASSERT_EQ(chords.size(), 4u);
    for (const auto& chord : chords) {
        EXPECT_EQ(soundingPitches(std::get<3>(chord)), (std::vector<std::string>{"C-1", "G4"}));
    }
    const std::string warning =
        "[WARN] Score::getChords: the octave doubling of the written C-1, which sounds C-1, lies "
        "outside the representable range and is left out of the chords.\n";
    const size_t first = printed.find(warning);
    EXPECT_NE(first, std::string::npos) << printed;
    EXPECT_EQ(printed.find(warning, first + 1), std::string::npos) << printed;
}
```

- [ ] **Step 2: Failing Python test** — in `test/test_maiapy.py`, before `if __name__ == "__main__":`, insert:

```python
class OctaveDoublingInThePianoRoll(unittest.TestCase):
    """The piano roll draws a doubled note twice: at what it sounds, and one octave below or
    above."""

    def testThePianoRollDrawsTheDoubledOctave(self):
        score = ml.Score(["Violoncello and Contrabass", "Flute and Piccolo"], 1)
        cello = ml.Note("C3")
        cello.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        score.getPart(0).getMeasure(0).addNote(cello)
        flute = ml.Note("G4")
        flute.setOctaveDoubling(ml.OctaveDoubling.ABOVE)
        score.getPart(1).getMeasure(0).addNote(flute)
        _, data = ml.plotPianoRoll(score)
        self.assertEqual(list(data["notePitch"]), ["C3", "C2", "G4", "G5"])
        self.assertEqual(list(data["midiValue"]), [48, 36, 67, 79])

    def testThePartsActivityDrawsEachNoteOnce(self):
        score = ml.Score(["Violoncello and Contrabass"], 1)
        cello = ml.Note("C3")
        cello.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        score.getPart(0).getMeasure(0).addNote(cello)
        _, data = ml.plotPartsActivity(score)
        self.assertEqual(1, len(data))
```

- [ ] **Step 3: Run and see them fail.** C++ subset `ScoreOctaveDoubling.*` → `{"C3", "G4"}` instead of four pitches; the warning is not printed. «pytest» `test_maiapy.OctaveDoublingInThePianoRoll` → `['C3', 'G4'] != ['C3', 'C2', 'G4', 'G5']` (the parts-activity test passes and stays as the guard of Step 7's mutation).

- [ ] **Step 4: The doubled octave in the chords.** In `score.cpp`'s anonymous namespace, after `concertKeys`, insert:

```cpp

// The note an octave doubling adds to the chords: an untransposed note at the concert pitch of
// 'note', one octave below (OctaveDoubling::BELOW) or above (ABOVE), with its duration, voice and
// staff. std::nullopt when the note is not doubled, or when that octave lies outside octaves
// -1..11 or below C1b-1; the latter is reported once per note, through 'reported'.
std::optional<Note> octaveDoublingNote(const Note& note, std::set<const Note*>& reported) {
    const OctaveDoubling doubling = note.getOctaveDoubling();
    if (doubling == OctaveDoubling::NONE || !note.isNoteOn()) {
        return std::nullopt;
    }
    const Pitch concert = concertPitch(note);
    const int shift = (doubling == OctaveDoubling::ABOVE) ? 1 : -1;
    const int octave = concert.getOctave().value() + shift;
    if (octave < c_minPitchOctave || octave > c_maxPitchOctave ||
        concert.getQuarterToneSteps() + 12.0f * static_cast<float>(shift) < -0.5f) {
        if (reported.insert(&note).second) {
            LOG_WARN("Score::getChords: the octave doubling of the written " +
                     note.getWrittenPitch() + ", which sounds " + concert.getPitch() +
                     ", lies outside the representable range and is left out of the chords.");
        }
        return std::nullopt;
    }
    Note doubled(Pitch(concert.getPitchStep(), concert.getAlter(), octave).getPitch());
    doubled.setDuration(note.getDuration());
    doubled.setVoice(note.getVoice());
    doubled.setStaff(note.getStaff());
    return doubled;
}
```

In `getChordsPerEachNoteEvent`, after the `keys` line add

```cpp
    // The notes whose doubled octave cannot be represented, each reported once.
    std::set<const Note*> unrepresentedDoublings;
```

and replace

```cpp
        Chord chord;
        for (const auto& noteData : currentChordData.noteData) {
            chord.addNote(*noteData.notePtr);
        }
```

with

```cpp
        Chord chord;
        for (const auto& noteData : currentChordData.noteData) {
            chord.addNote(*noteData.notePtr);
            const std::optional<Note> doubled =
                octaveDoublingNote(*noteData.notePtr, unrepresentedDoublings);
            if (doubled) {
                chord.addNote(*doubled);
            }
        }
```

- [ ] **Step 5: The piano roll.** In `maialib/maiapy/plots.py` replace the signature line `def _score2DataFrame(score: mc.Score, kwargs) -> Tuple[pd.DataFrame, str, str]:` with

```python
def _score2DataFrame(
    score: mc.Score, kwargs, octaveDoublings: bool = False
) -> Tuple[pd.DataFrame, str, str]:
```

add to its docstring after the Kwargs block:

```
    Args (keyword):
       octaveDoublings (bool): Also list, for each note with an octave doubling, its doubled
          octave: one octave below or above what it sounds, unless that octave lies outside
          octaves -1 to 11
```

and replace

```python
                    # Add 'noteData' object to the list
                    plotData["notesData"].append(noteData)
```

with

```python
                    # Add 'noteData' object to the list
                    plotData["notesData"].append(noteData)

                    # A doubled note is listed again, one octave below or above what it sounds
                    doubling = currentNote.getOctaveDoubling()
                    if octaveDoublings and doubling != mc.OctaveDoubling.NONE:
                        shift = 1 if doubling == mc.OctaveDoubling.ABOVE else -1
                        doubledOctave = currentNote.getSoundingOctave() + shift
                        doubledMidi = midiValue + 12 * shift
                        if -1 <= doubledOctave <= 11 and doubledMidi >= 0:
                            doubledPitch = currentNote.getSoundingPitchClass() + str(doubledOctave)
                            plotData["notesData"].append(
                                {
                                    **noteData,
                                    "midiValue": doubledMidi,
                                    "notePitch": doubledPitch,
                                }
                            )
```

In `plotPianoRoll` replace `    df, author, work_title = _score2DataFrame(score, kwargs)` with `    df, author, work_title = _score2DataFrame(score, kwargs, octaveDoublings=True)` and add to its docstring, after `Plots a piano roll graph showing the musical activity of each score instrument`, the paragraph:

```

    A note with an octave doubling (Note.getOctaveDoubling()) is drawn twice: at what it sounds
    and one octave below or above; a doubled octave outside octaves -1 to 11 is not drawn.
```

- [ ] **Step 6: Document the doubling in the analyses.** In `score.h`, in `getChords`' Doxygen, after the `includeUnpitched` line `     *          - \`includeUnpitched\` (boolean): Include percussion/unpitched elements`, insert:

```cpp
     *
     *          A note with an octave doubling (Note::getOctaveDoubling()) also adds, to each chord
     *          it sounds in, an untransposed note at its concert pitch one octave below or above;
     *          a doubled octave outside octaves -1..11, or below C1b-1, is left out, with one
     *          warning per note.
```

In `py_score.cpp`'s `getChords` docstring, before the paragraph `        The key reported with each chord ...`, insert:

```
        A note with an octave doubling (``Note.getOctaveDoubling``) also adds, to each chord it
        sounds in, an untransposed note at its concert pitch one octave below or above; a
        doubled octave outside octaves -1 to 11, or below ``C1b-1``, is left out, with a warning
        printed once for the note.

```

In `note.h`, in `getOctaveDoubling`'s Doxygen, replace `     *          never the doubled octave. The doubling takes no part in operator==() and` with

```cpp
     *          never the doubled octave, which Score::getChords() adds to each chord the note
     *          sounds in. The doubling takes no part in operator==() and
```

In `py_note.cpp`, in `getOctaveDoubling`'s docstring, replace `        the doubled octave. ``==``, ``!=`` and the hash ignore the doubling: two notes that` with

```
        the doubled octave, which ``Score.getChords`` and ``plotPianoRoll`` add. ``==``, ``!=``
        and the hash ignore the doubling: two notes that
```

- [ ] **Step 7: Format, build, pass, mutate.** clang-format `score.cpp`, `score.h`, `note.h`, `py_score.cpp`, `py_note.cpp`, `score-test.cpp`; `ruff format maialib/maiapy/plots.py test/test_maiapy.py` changes only the new lines (inspect `git diff`). C++ subset `ScoreOctaveDoubling.*` → 3 pass; `make dev`; «pytest» `test_maiapy` and `test_note.NoteOctaveDoubling` → OK. Mutations: (a) never add the doubled note → `aDoubledNoteAddsItsOctaveToTheChord`, `theDoubledOctaveIsAnOctaveFromWhatTheNoteSounds` fail; (b) `const int shift = -1;` → the first test fails (`G3` for `G5`); (c) build the doubled note from the written pitch (`note.getWrittenPitch()`'s octave instead of the concert one: replace `concertPitch(note)` with `Pitch(note.getWrittenPitch())`) → `theDoubledOctaveIsAnOctaveFromWhatTheNoteSounds` fails (`C3` and `D3`); (d) report every time (`if (true)` for `if (reported.insert(&note).second)`) → `aDoubledOctaveOutOfRangeIsLeftOutWithOneWarning` fails (four warnings); (e) drop the range check → the same test fails (`Pitch` raises for `C-2`); (f) Python: call `_score2DataFrame(score, kwargs)` in `plotPianoRoll` → `testThePianoRollDrawsTheDoubledOctave` fails; (g) default `octaveDoublings: bool = True` → `testThePartsActivityDrawsEachNoteOnce` fails. Revert each; rerun green.

- [ ] **Step 8: Whole suites.** `make cpp-tests`, `make py-tests`, `make validate` → 0, no new findings.

- [ ] **Step 9: Commit.** `git add maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/include/maiacore/note.h maiacore/src/maiacore/python_wrapper/py_score.cpp maiacore/src/maiacore/python_wrapper/py_note.cpp maialib/maiapy/plots.py tests-cpp/src/score-test.cpp test/test_maiapy.py`, message:

```
feat: getChords and plotPianoRoll count the octave doubling

A doubled note adds to each chord it sounds in an untransposed note one
octave below or above its concert pitch, at the same onset and duration;
one outside the representable range is left out with one warning per
note. The piano roll draws the doubled octave too. The melody search
ignores the doubling.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 9: Changelog, remaining docs, final verification

**Files:**
- Modify: `CHANGELOG.md` (`[Unreleased]`: `### Added` after line ~23; line ~23 removed; `### Fix`; `### Build and tooling`)

**Interfaces:** consumes everything above.

- [ ] **Step 1: Remove the known limitation.** Delete the line that begins `  - **Known limitation:** a \`<transpose>\` whose \`<diatonic>\` and \`<chromatic>\` disagree is followed literally.` (the reader checks the pair now).

- [ ] **Step 2: Added.** After the `Pitch views of a note, and diatonic transposition` group (its last sub-bullet is the remaining `**Known limitation:**` line about spelling-based pitch classes), insert:

```markdown
- **MusicXML `<transpose>`.** `Score(path)` reads a `<transpose>` in every measure and per staff (`number`), with `<octave-change>` and `<double>`, and the export writes `<transpose>` elements derived from the notes, so a re-imported score keeps its transpositions
  - Each note holds its transposition: the reader stamps every pitched note with the `<transpose>` in force for its staff, from the notes written after it in its measure up to the next `<transpose>` for that staff or for every staff. `<octave-change>` is folded into the note's interval, 7 letters and 12 semitones per octave (a B-flat bass clarinet's `-1`, `-2`, `-1` is `(-8, -14)`)
  - `OctaveDoubling` (`NONE`, `BELOW`, `ABOVE`; C++ and Python) with `Note.setOctaveDoubling()` and `Note.getOctaveDoubling()` holds a `<double/>` or `<double above="yes"/>`. `Score.getChords()` adds, for each doubled note, an untransposed note one octave below or above its concert pitch to each chord it sounds in (a doubled octave outside the representable range is left out with one warning per note), and so do `getChordsDataFrame()`, the chord qualities, the Sethares dissonance and the chord plots built on it; `plotPianoRoll()` draws the doubled octave too; the melody-pattern search ignores it. `==` and the hash ignore the doubling; a rest refuses it with a warning, and `setPitch("rest")` clears it
  - `Part.setTransposingInterval(diatonicInterval, chromaticInterval, measureStart=0, measureEnd=-1, staff=-1, doubling=OctaveDoubling.NONE)` stamps the pitched notes of a range of measures and a staff, all or none: a note that would have no sounding pitch raises `RuntimeError` naming it, and no note changes. In Python it changes a score's transpositions in place, which an edit through `Measure.getNote()`, a copy, cannot
  - The export writes, for each staff, a `<transpose>` in measure 1 when its first pitched note is transposed or doubled, and one wherever the interval or the doubling changes: in the measure's `<attributes>` when the staff's first pitched note there brings the change, otherwise in an `<attributes>` written just before the note; without `number` when every staff has the same transposition there. The interval is unfolded into `<diatonic>`, `<chromatic>` and, from an octave on, `<octave-change>`; a stored diatonic interval of 0 is written as the conventional one. A chord whose notes have different intervals or doublings cannot be written: the export raises `RuntimeError` naming the part, the measure and the staff
  - The reader corrects or ignores a `<transpose>` with one warning per correction, which starts with a code: `[transpose-chromatic-not-integer]` and `[transpose-octave-change-not-integer]`, a value that is not a whole number, ignored; `[transpose-pair-corrected]`, a `<diatonic>` that does not match `<chromatic>`, replaced by the conventional diatonic interval (a tritone accepts the augmented fourth and the diminished fifth); `[transpose-out-of-range]`, an interval with which a note of its scope would have no sounding pitch, ignored for its whole scope. An ignored `<transpose>` leaves the previous transposition in force, and a note that cannot sound with that one either is read untransposed. `<for-part>` is not modelled: `[for-part-not-modelled]` reports it and it is dropped
```

- [ ] **Step 3: Fix (behaviour changes).** At the end of `### Fix` (after its last `**Breaking:**` line), insert:

```markdown
- **Breaking:** `Score(path)` read `<transpose>` in a part's first measure only and ignored `<octave-change>`, `<double>` and `number`, taking the first `<diatonic>` and the first `<chromatic>` of that measure even from different elements: octave-transposing parts — contrabasses, contrabassoons, piccolos, celestas, bass clarinets — sounded an octave from what the file says, and a change of transposition was ignored (W3C 72c kept its E-flat clarinet in measure 2). Sounding pitches, `Score.getChords()` and everything built on it change for such files: in the Beethoven 5 sample 118 of 1,462 chord names and 721 bass notes change, and the Dvořák and Mahler 8 samples, W3C 41c and `test_getchords_poly.musicxml` change too
- **Breaking:** `Score.getChords()` and `Score.getChordsDataFrame()` report the concert key of each chord's measure: the most frequent written key among the pitched parts that are untransposed there or transposed by whole octaves (a tie goes to the first part), or, when every pitched part transposes, the first part's written key moved by its interval and brought into -6..11 fifths. They reported the first part's written key, which disagreed with the concert-pitch chords whenever that part transposes (W3C 72a reported D major for a C major chord)
- **Breaking:** A `<transpose>` whose `<diatonic>` and `<chromatic>` disagree is corrected: the Dvořák sample's trumpets in E, `(3, 4)`, are read as `(2, 4)`, so a written `F#4` sounds `A#4` (it sounded `Bb4`). A `<transpose>` without `<diatonic>` stores the conventional diatonic interval, where it stored 0
- **Breaking:** A `<transpose>` that would push a note out of the representable range no longer aborts the load with `RuntimeError`; it is ignored with a warning
- **Breaking:** Exports write `<transpose>`, so `Score.__hash__` and `Part.__hash__`, which hash the export, change for scores with transpositions; an export used to lose every transposition, so a re-imported score sounded its written pitches
```

- [ ] **Step 4: Build and tooling.** At the end of `### Build and tooling`, before the first `**Breaking:**` line of that section, insert:

```markdown
- `make corpus` also runs the export/import round trip of `<transpose>` (`test/test_musicxml_transpose.py`) with `MAIALIB_SLOW_TESTS=1`, which adds the slow corpus files that `make py-tests` skips; the MusicXML model dump records each note's octave doubling
```

- [ ] **Step 5: Remaining docs.** Bash `git -C /c/Users/nyck/Desktop/maialib grep -n -e "as that part writes it" -e "followed literally" -e "measure\[1\]/attributes/transpose" -- '*.md' '*.h' '*.cpp' '*.py' ':!docs/superpowers' ':!AI_API_CHEATSHEET.md' ':!llms-full.txt'` → no line (the generated `AI_API_CHEATSHEET.md` and `llms-full.txt` are regenerated at release). Commit `git add CHANGELOG.md`, message:

```
docs: changelog of MusicXML <transpose> reading and writing

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

- [ ] **Step 6: Final verification (spec §7.4)** from the committed tree, in a second brand-new venv:
  1. PowerShell: `py -3.12 -m venv C:\Users\nyck\AppData\Local\Temp\maialib-1b-final-venv; & 'C:\Users\nyck\AppData\Local\Temp\maialib-1b-final-venv\Scripts\python.exe' -m pip install -r requirements-dev.txt; $LASTEXITCODE` → 0; in «build» below use `$py = 'C:\Users\nyck\AppData\Local\Temp\maialib-1b-final-venv\Scripts\python.exe'`.
  2. «build» `make "PYTHON=$py" dev` → 0.
  3. «build» `make "PYTHON=$py" cpp-tests` twice → 0 both times; record the count (Task 0's plus the new ones: 4 + 8 + 15 + 5 + 1 + 11 + 3 = 47 more).
  4. «build» `make "PYTHON=$py" py-tests` → OK; record the count and duration (Task 0's plus 6 + 3 + 12 + 6 + 1 + 2 = 30 more, one of them skipped).
  5. «build» `make "PYTHON=$py" validate` → no new findings.
  6. «build» `make "PYTHON=$py" corpus` (the external corpus is fetched) → 0: neither ledger differs, and the slow round trip ran.
  7. «build» `make "PYTHON=$py" msvc-gate` → 0.
  8. «build» `make "PYTHON=$py" linux-gate` → 0 (the Python tests on Linux with GCC, in WSL). If it exits 2 listing missing apt packages (this machine's WSL has no `cmake` and no password-less sudo), run the Linux route instead and report it: in Bash, `git -C /c/Users/nyck/Desktop/maialib -c core.autocrlf=false -c core.eol=lf archive HEAD` (a plain `git archive` here writes CRLF) extracted under `/var/tmp/maialib-1b` in WSL; there a venv with `requirements-dev.txt` plus `cmake` from pip; `make "PYTHON=<venv>/bin/python" dev` and `make "PYTHON=<venv>/bin/python" py-tests` → 0; record the count; remove `/var/tmp/maialib-1b` afterwards.
  9. «build» `make "PYTHON=$py" fuzz` → 0; compare `test\musicxml\fuzz-work\report-seed-1.json` with `C:\Users\nyck\AppData\Local\Temp\maialib-1b-fuzz-baseline.json` by outcome, not case by case — the 14 new fixtures shift the list of files the cases pick from (`test/musicxml/README.md`): no `crash:*` or `timeout:*` outcome, and no outcome that the baseline does not have; explain each outcome count that changed. A finding that involves a `<transpose>` is fixed before the branch is finished.
  10. Import check from outside the repository (Task 0, Step 5) with the final venv; `git status --short` → ` M .gitignore` only; `git log --oneline bd63266..HEAD` lists the nine task commits (Tasks 1-9; Task 0 commits nothing) after the commit that amended this plan before Task 1.
  Put every count, duration and comparison in the task report.

---

## Self-review (done while writing)

- **Spec coverage.** §1 problems → Tasks 3 (first measure only, octave-change, changes), 5 (writer), 7 (written key), 4 (abort on range, pair). §2 D1 → Tasks 3, 5 (notes hold the state; no Measure/Part/Score state, no getter); D2 → Task 7; D3 → Task 3 (non-integer chromatic ignored, no API change); D4 → Tasks 1, 8. §3.1 → Task 1 (enum, setter refusal with `LOG_WARN`, `setPitch("rest")`, getters keep the note's own pitch) and Task 3 (folded interval); "the writer writes the inferred value" → Task 5. §3.2 → Task 2 (half-open range, `-1` defaults, atomic, out_of_range indices, the Python in-place role in the docstring; no getter). §4.1 → Task 3 (every measure, `number` parsed as a whole number, document order, a chord read as a unit, rests not stamped, untransposed without `<transpose>`). §4.2 → Task 3 (absent `<diatonic>` conventional and silent, `octave-change` folded, `<double>`). §4.3 → Tasks 3 (non-integer chromatic), 4 (pair before folding, tritone, explicit 0; out of range over the whole scope, the fallback chord by chord; `<for-part>`; `<concert-score/>` has no effect — fixture `transpose_for_part`). §5.1 → Task 5 (measure 1 from the first pitched note, later changes at the measure start or before the note, back to 0/0, `number`, chord error, `<attributes>` opened). §5.2 → Task 5 (unfold, `<diatonic>` always and inferred, `<double>`, position, no `<for-part>`, header unchanged, hashes — tested). §5.3 → Task 6 (sounding pitch, spelling, doubling; the stored-0 exception). §6.1 → Task 7 (majority of key-neutral pitched parts, tie, fallback with the wrap, interval at a measure, mode, docs; `Chord::getDegree` needs no change). §6.2 → Task 8 (getChords and what derives from it, `plotPianoRoll`, melody search untouched). §6.3 → Task 3 (`anOctaveTranspositionChangesTheChords`) and Task 9 (CHANGELOG). §7.1 → Tasks 3, 4 (14 fixtures, one per rule). §7.2 → each task's tests (reader fixtures and 72a/72c/72d/41c/Beethoven/Dvořák; writer; round trip; setter; concert key incl. tie, percussion, wrap; doubling; `OctaveDoubling` API). §7.3 → Tasks 3, 4 (added ledger lines), 6 (`make corpus` unchanged, dump field, golden). §7.4 → Task 9. §8 → Task 9 (the generated docs stay for the release step, per the Global Constraints). §9 → nothing to do.
- **Interfaces are consistent across tasks:** `OctaveDoubling` (1 → 2-8), `soundsWithinRange` (2 → 4), `conventionalDiatonicInterval` (3 → 4), `spelledDiatonicInterval` (2 → 5, 7), `TransposeElement`/`PitchedNote`/`applyTranspositions` (3 → 4), `Measure::toXML(…, beforeNote)` (5), `load`/`export`/`transposes`/`single_part_score` (5 → 6), `kUnitTest`/`kW3c`/`kSamples`/`describeTransposition`/`transposedNotes`/`transposeWarnings` (3 → 4, 7), `wholeNote`/`keyedScore` (7 → 8).
- **Measured facts the tests rely on** (checked with today's build): the positions of the corpus notes named in Tasks 3 and 4; the parts and written keys of 72a (Trumpet in Bb 2, Horn in Eb 3, Piano 0), 72d (Trumpet in C and "MusicXML Part" 1), `test_pattern` (Horn 1, Trumpet 2, Trombone 0) and the Beethoven 5 sample (C Trumpet, Timpani 0 major; the eight others -3 minor); `test_getchords_poly`'s chords; the Dvořák trumpets' `<transpose>` (measure 1, `3`, `4`, part "Trombe I. II. E" after the reader joins its line break); every corpus file with a `<transpose>` keeps its notes, in number and written pitch, through an export and a reload, the slow ones included; an API-built score without a key signature exports no `<key>` and cannot be reloaded, hence `single_part_score`.
