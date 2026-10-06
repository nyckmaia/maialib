# Import Report, Unicode Paths and Collection Isolation (phase 4c-1a) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every value the MusicXML reader corrects and every element it drops becomes a structured record of an import report (`Score.getImportIssues()`, a DataFrame, one summary line instead of per-event `[WARN]` lines), text from the file reaches Python as valid UTF-8, fatal input raises with pugixml's description and offset or the archive's problem, any path opens on Windows and stays UTF-8, a `ScoreCollection` skips and lists the files that fail, and the corpus and fuzz tooling records report codes and exit statuses, runs subsets and has an acceptance mode.

**Architecture:** A public struct `ImportIssue` (`maiacore/include/maiacore/import-issue.h`) and a private unit `maiacore::detail` import report (`maiacore/src/maiacore/import-report.h/.cpp`: `validUtf8()`, the code catalogue, `makeIssue()`, `droppedElements()`, `importSummary()`) serve `Score::loadXMLFile`, which appends records to a new member `_importIssues` where it printed or corrected silently, runs one extra pass over the document for the closed element list, and prints one summary line. Files are opened through `std::filesystem::u8path` (wide on Windows) and parsed from a buffer; `.mxl` problems are named. `ScoreCollection` loads each file in isolation and keeps `(path, message)` failures. The loading bindings redirect C++ output through a writer that never fails on an unencodable character (`py_console.h`).

**Tech Stack:** C++17 (maiacore, pugixml 1.15, miniz-cpp), pybind11 bindings with numpydoc docstrings, pandas (DataFrame built in the binding), GoogleTest 1.14 (`tests-cpp/src/`, sources listed in `tests-cpp/CMakeLists.txt`), Python `unittest` (`test/`), the 4a corpus ledger and fuzz driver (`test/musicxml/`).

**Spec:** `docs/superpowers/specs/2026-10-06-import-report-design.md` is binding; it fixes what §5 of the umbrella (`docs/superpowers/specs/2026-10-01-musicxml-robustness-design.md`) leaves open, and the umbrella's §5 and §6 (closed element list) bind too. Every requirement maps to a task (see the self-review at the end). Test infrastructure: `test/musicxml/README.md`.

## Global Constraints

- C++17; no lambda captures a structured binding; every C++ file a task changes is formatted with `& 'C:\Program Files\LLVM\bin\clang-format.exe' -i <files>` (clang-format 18.1.6) before its tests run. The code below is already in clang-format 18 form: an anchor quoted from code an earlier task added matches that task's formatted text exactly once.
- `make validate` adds no cpplint or cppcheck finding (measured on the finished tree: `Validation: no new findings (44 known).` and `21 baseline finding(s) no longer reported` — leave `scripts/validate-baseline.json` as it is). Every `.cpp`/`.h` that starts using a standard facility includes its header; no C-style cast.
- Python in `maialib/` and the scripts supports 3.8–3.14 (`from __future__ import annotations` where a module already has it). Test code added to an existing module follows its naming (camelCase helpers in `test_score_comprehensive.py`); lines added to `test/musicxml/*.py`, `scripts/make-*.py`, `test_musicxml_corpus.py`, `test_musicxml_fuzz.py`, `test_musicxml_transpose.py` and `test_score_collection.py` pass `ruff check` and `ruff format --check` (`pyproject.toml`); `test_score_comprehensive.py` already has unformatted lines and findings, so there only the lines a task adds must add no finding of a new kind.
- Docs, Doxygen, numpydoc, comments and commit messages in technical English. Comments explain the code, never its development history: no task numbers, review rounds, rulings or SHAs in code, tests or fixtures.
- Every new test is proven to fail under a targeted mutation of the code it protects: apply the named mutation with Edit, rebuild what the test runs against, run the test and record the failing output, revert the same Edit exactly, rebuild, rerun green. Record each mutation and its failing output in the task report. `git diff` after the revert shows only the task's intended changes. A test that cannot fail today is made exact.
- Tests hold on Windows and Linux: no expectation that depends on the platform without a platform-aware predicate. A test that needs a file writes it into a directory of its own (`TemporaryFile` in `tests-cpp/src/test-files.h`, `tempfile.TemporaryDirectory()` in Python), never into `TEMP` itself: this machine's antivirus adds an entry to every process's `TEMP`.
- Never load a corpus file of 10 MB or more (`Symphony_9th.xml`, `xakypueri.xml`) in a unit test.
- Never stage the user's uncommitted root `.gitignore` change (`musescore/*`); stage files by name, never `git add -A`/`.`; `git status --short` ends every task showing only ` M .gitignore`.
- `make dev` regenerates `AI_API_CHEATSHEET.md` and `llms-full.txt` when the API changes; every task restores them with `git checkout -- AI_API_CHEATSHEET.md llms-full.txt` before committing (they are regenerated at release, with the stubs `maialib/maiacore/*.pyi`).
- Commit messages end with these two lines:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Line endings: `core.autocrlf=true`, so every existing file is CRLF in the working tree and LF in the index; edits keep the file's endings (the Edit tool does). New files may be written with LF; git stores LF either way.
- Test environment: `make dev` only in a brand-new venv, `py -3.12`, created outside the repository at `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-venv` (Task 0) with `pip install -r requirements-dev.txt`; the final verification uses a second brand-new venv `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-final-venv`. Import checks run from outside the repository root. Read exit codes directly (`$LASTEXITCODE`, `$?`), never through a pipe. The Makefile's `PYTHON` is set only on the command line. Never use `vswhere -latest`. The Windows ANSI code page here is 1252: a test that needs a name no ANSI code page holds uses Japanese (`日本`).
- **«build»** below means this PowerShell prefix, repeated in every PowerShell call because shell state does not persist: `$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'; $env:INCLUDE = $null; $env:LIB = $null; $py = 'C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-venv\Scripts\python.exe';`
- **C++ subset:** «build» `make "PYTHON=$py" build-cpp-tests; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }; .\build\Windows\cpp-tests\cpp-tests.exe --gtest_filter='<filter>'; $LASTEXITCODE` (the binary runs from the repository root, where the fixtures' relative paths start).
- **«pytest» X** below means, in Bash: `(cd /c/Users/nyck/Desktop/maialib/test && /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-venv/Scripts/python.exe -m unittest X -v); echo "exit $?"` — the Python tests run from `test/` against the package installed by «build» `make "PYTHON=$py" dev`. In a Bash heredoc or `-c` string the tool may swallow backslashes: write any script that holds one with the Write tool.
- Fixtures and the ledger: every file under `test/xml_examples` has its line in `test/musicxml/ledger.json`. A task that adds a fixture runs «build» `make "PYTHON=$py" corpus-update-ledger` and commits only the added lines after `git diff test/musicxml/ledger.json` shows no other line changed and `git diff --stat test/musicxml/ledger-external.json` is empty; ledger changes are reviewed like code — every changed line is explained in its commit.
- Line hints (`~N`) give the line in the file as the previous task leaves it (Task 1: at `b620ed1`). The quoted anchor text decides where an edit goes; each block below replaces its first block exactly once, in the order given.
- Code names (spec §2): `ImportIssue` fields `code`, `kind`, `partIndex`, `partName`, `measureNumber`, `measureIndex`, `element`, `found`, `used`, `message`; kinds `"corrected"`, `"dropped"`; codes `ACCIDENTAL_ALTER_MISMATCH`, `ACCIDENTAL_NAME_UNKNOWN`, `ALTER_OFF_GRID`, `DIVISIONS_MISSING`, `ELEMENT_NOT_MODELLED`, `FOR_PART_NOT_MODELLED`, `PART_NAME_DUPLICATE`, `STAFF_CLAMPED`, `TRANSPOSE_CHROMATIC_NOT_INTEGER`, `TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER`, `TRANSPOSE_OUT_OF_RANGE`, `TRANSPOSE_PAIR_CORRECTED`, `TUPLET_CLAMPED`, `VOICE_NOT_POSITIVE`; summary line, verbatim: `[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on export); see Score.getImportIssues()`; collection summary: `[maiacore] ScoreCollection: <k> of <n> files failed to load; see ScoreCollection.getLoadErrors()`.
- Error messages (first line, after `[maiacore] `): `Score: cannot open '<path>'`; `Score: '<path>' is not well-formed XML: <pugixml description> (byte offset <n>)`; `Score: '<path>' is not a readable MusicXML archive: <problem>` with problems `it is not a zip archive`, `it has no META-INF/container.xml`, `its META-INF/container.xml names no rootfile`, `the rootfile '<name>' is not in the archive`, `its entry '<name>' cannot be read`; `Score: the rootfile '<name>' of '<path>' is not well-formed XML: <description> (byte offset <n>)`.
- Baselines (measured at `b620ed1` in a scratch copy): `[  PASSED  ] 1188 tests.` (C++), `Ran 690 tests` / `OK (skipped=1)` (Python).

## Review Focus

The input classes and failure modes the spec implies but no other task test exercises, most likely to bite a user first; each line names the test that pins it and its task.

1. **A file name the console cannot encode** — `日本.xml` loaded from a script whose output is piped or redirected (on Windows `sys.stdout` is then in the ANSI code page): the summary line and the collection's `Loading:` line name the file, and pybind11's redirection flushes them from a destructor, where the `UnicodeEncodeError` ends the process (0xC0000409), although every byte is valid UTF-8. Expected: the load succeeds and the line is written with backslash escapes. Pinned by `ScoreUnicodePathTestCase.test_a_name_the_console_cannot_encode_does_not_end_the_interpreter` and `ScoreCollectionConstructionTestCase.test_non_ascii_file_names_load_with_their_utf8_names` (Task 5; both run a child with `PYTHONIOENCODING=ascii`).
2. **A title, composer or part name whose bytes are not UTF-8** (Latin-1 text in a file that declares UTF-8, which pugixml does not check) — `getTitle()`, `getComposerName()` and `getPartsNames()` raised `UnicodeDecodeError`. Expected: the names are held with U+FFFD in place of each invalid byte, as the records are. Pinned by `ScoreImportReport.NamesThatAreNotUtf8AreHeldAsValidUtf8` and `ScoreImportReportTestCase.test_names_that_are_not_utf8_are_read_with_replacement_characters` (Task 1).
3. **A compressed file named `SCORE.MXL`** — `ScoreCollection` discovers it (extensions compare without case), but `Score` tested the last three characters against `mxl` and parsed the archive as XML. Expected: it loads. Pinned by `ScoreUnicodePath.AnUppercaseMxlExtensionIsReadAsAnArchive` and `ScoreUnicodePathTestCase.test_an_uppercase_mxl_extension_is_read_as_an_archive` (Task 5).
4. **A score maialib exported, loaded again** — a user who checks `getImportIssues()` after a round trip expects nothing. Measured: no correction for a loaded score; seven dropped paths, all header and part-list elements the writer invents (`defaults`, `identification/encoding`, `identification/rights`, `part-abbreviation`, `part-abbreviation-display`, `part-name-display`, `score-instrument`); an API-built score's export has no `<divisions>`, so its reload also records `DIVISIONS_MISSING`. Pinned, so that phase 4b's writer changes show, by `ScoreImportReport.AnExportLoadedAgainRecordsOnlyTheElementsTheWriterInvents` (Task 3) and `TransposeRoundTripTestCase.test_a_stored_diatonic_interval_of_zero_comes_back_as_the_conventional_one` (Task 2).
5. **A collection directory whose name is outside the ANSI code page** (`músicas` holding `日本.xml` and a file that fails) — expected: the scores load and both the scores' paths and `getLoadErrors()` paths are UTF-8. Pinned by `ScoreCollectionDirectories.ANonAsciiDirectoryListsItsFailuresInUtf8` and `ScoreCollectionConstructionTestCase.test_a_non_ascii_directory_lists_its_failures_with_utf8_paths` (Task 6).

## Decisions this plan takes beyond the spec (reviewers: accept or overrule)

1. **Decomposition.** The suggested eight tasks stand, with a Task 0 for baselines, and one move: the three accidental/`<alter>` warnings become records in Task 1, not Task 2, because they are the ones that quote raw note text (the 0xC0000409 crash) and the report's storage, copy, DataFrame and summary need real records to be tested. Task 2 converts the `<transpose>` warnings and the silent corrections.
2. **The catalogue holds code and kind only** (`issueCatalogue()`, sorted, tested for UPPER_SNAKE and uniqueness); the element varies per record (`<actual-notes>` or `<normal-notes>` of a tuplet) and the message is built where the correction is made. The umbrella §6 policy table (`docs/musicxml-import-policy.md`), the catalogue's message templates and its meta-test are deferred to phase 4c-2, which builds the document-order reader the table describes (spec §7); they are not written here.
3. **Kinds and the summary count.** The four `TRANSPOSE_*` codes are `corrected` (the notes' transposition is replaced by the one in force); `FOR_PART_NOT_MODELLED` and `ELEMENT_NOT_MODELLED` are `dropped`. The summary's `<n>` counts `corrected` records and `<m>` the distinct `element` paths of `dropped` records, so a `<for-part>` counts as one element type however many there are.
4. **Field conventions.** The three accidental codes have `used` = the note's written pitch as stored (`C4`, `C1x4`); `ALTER_OFF_GRID` and `ACCIDENTAL_ALTER_MISMATCH` have `element` `note/pitch/alter`, `ACCIDENTAL_NAME_UNKNOWN` `note/accidental`. An ignored `<transpose>` and a dropped element have `used` empty; `TRANSPOSE_OUT_OF_RANGE` has `found` `diatonic <d>, chromatic <c>` (octaves folded in). `found` is the element's text as written (untrimmed); `PART_NAME_DUPLICATE` has the renamed part's index and new name and no measure; `DIVISIONS_MISSING` names measure 1 and `used` `256`. Messages carry no part or measure (the fields do) and no bracketed prefix.
5. **Voice, staff and tuplet values.** A record is made only when the element is present and its text, trimmed, is not a positive whole number (an absent optional element, `<voice>` or `<staff>`, takes the standard's default without a record; `<voice> 3 </voice>` is valid). The exception is `<actual-notes>` and `<normal-notes>`, which `<time-modification>` requires: they are recorded when absent (`found` empty). Negative voices and tuplet values, which were kept, are read as 1 like 0 (`<= 0` instead of `== 0`); a text that starts with digits keeps `atoi`'s value (`2abc` is voice 2, recorded).
6. **The closed element list, concretely** (`heldOutsideMeasures()`, `heldInMeasures()` in `import-report.cpp`): umbrella §6 items 1 and 3, plus the elements the writer regenerates from held values (`note/type`, `note/dot`, `note/accidental`, `note/notations/tied`, the containers `attributes`, `note/notations`, `attributes/staff-details`, `identification`, `part-list/score-part/midi-instrument`). `<backup>` and `<forward>` are held as the umbrella lists them, although the reader still ignores `<forward>` (4c-2). `attributes/for-part` keeps its own record and is skipped by the pass. Paths start at the measure inside a measure and at the root's children elsewhere; the pass is one iterative walk.
7. **Fatal errors.** A file that cannot be opened says `cannot open` without an offset (nothing was parsed); a parse error gives pugixml's `description()` and `(byte offset N)`. An `.mxl` shorter than 22 bytes, the size of the smallest zip archive, is `not a zip archive` without reaching miniz, whose comment scan reads past a buffer of 1 to 3 bytes.
8. **Opening files.** Both formats are read through one `std::ifstream` on `std::filesystem::u8path(filePath)` — a wide-character path on Windows — and the XML is parsed with `load_buffer`, instead of `load_file(const wchar_t*)` plus a separate `.mxl` stream: one code path, and the same parse (`load_file` reads the whole file and parses the buffer). A path that is not UTF-8 cannot be opened. A file is read as an archive when its extension is `.mxl` in any case (Review Focus 3) or when its bytes start with the zip signature `PK` (spec §3), so a zip archive named `.xml` loads; a file named `.mxl` that is not a zip archive raises `it is not a zip archive`, never read as text. The inner document of an archive is still parsed with `load_string` (UTF-16 inner files are 4c-1b, umbrella §8 item 13).
9. **The console.** The loading bindings (`Score(filePath)`, the `ScoreCollection` constructors, `setDirectoriesPaths`, the three `addScore`) use a call guard, `ConsoleRedirect` (`py_console.h`), that writes through a wrapper of `sys.stdout`/`sys.stderr` which escapes a character the stream cannot encode (Review Focus 1). The other bindings keep pybind11's plain redirection.
10. **Names as valid UTF-8.** The title, composer and part names are stored through `validUtf8()` (Review Focus 2); other strings read into the model (stem, beam, tie types, articulation names, barline fields) are not, and `Part::getShortName()` still cuts the name byte by byte (see the report).
11. **`getLoadErrors()`** returns `(path, message)` pairs: the path as loaded (UTF-8) and the first line of the error's message, without the source location and stack trace `LOG_ERROR` appends, both through `validUtf8()`. Each load (constructor, `setDirectoriesPaths`, `addScore` with one path or several) replaces the list; `addScore(Score)`, `clear()`, `merge()` and copies leave or carry it. `addScore(path)` is `addScore([path])`, which no longer raises for a file that fails.
12. **Tooling.** `exit` is recorded for every non-zero status that is not a timeout, mid-stage crashes included (their records already said `crash`; a crash's status differs between platforms, which the README says). `codes` is set in the record right after `load`. The sidecar logic is generic (`<ledger>-codes.json`, written when the ledger would exceed 500,000 bytes, removed when it fits again) and `load_ledger` merges it. `CORPUS_ARGS` runs that leave files out (`--skip-slow`, `--filter`) compare and update only the files examined and skip the `<transpose>` round trip; `corpus.default_workers()` is the one definition of the worker count, used by `run_corpus` and `fuzz.run`.
13. **Fixtures.** The rule tests write their files from strings (`TemporaryFile`, `minimalScore()` in `tests-cpp/src/test-files.h`; `tempfile` in Python); one committed fixture, `test/xml_examples/unit_test/import_report_dropped.musicxml` (valid MusicXML 4.0), carries the dropped-element list, its DataFrame and the corpus `codes`. The existing fixtures already produce every other code but `STAFF_CLAMPED`, `TUPLET_CLAMPED` and `VOICE_NOT_POSITIVE` (measured with Task 7's ledger).
14. **The external ledger in Task 5.** Loading the non-ASCII files from their own paths gives the records the ASCII copies gave: a scratch run of 60 random non-ASCII external files (loads and `IndexError`s) matched their ledger lines exactly. Task 5 therefore expects `make corpus` to pass with both ledgers unchanged, and says so in its commit. In Task 7 every loaded file gains `codes`, which for the external corpus go to the sidecar, so `ledger-external.json` itself never changes in this phase (measured: byte-identical after the full update).

## File map

| File | Responsibility | Tasks |
|---|---|---|
| `maiacore/include/maiacore/import-issue.h` (new) | `struct ImportIssue` | 1 |
| `maiacore/src/maiacore/import-report.h`, `import-report.cpp` (new) | `validUtf8`, `IssueLocation`, `issueCatalogue`, `makeIssue`, `importSummary`; `droppedElements` and the closed element list | 1, 2, 3 |
| `maiacore/include/maiacore/score.h` | `_importIssues`, `getImportIssues()`, copy semantics, Doxygen of `Score(filePath)` | 1–5 |
| `maiacore/src/maiacore/score.cpp` | the records, the summary line, the dropped pass, fatal messages, opening by UTF-8 path | 1–5 |
| `maiacore/src/maiacore/python_wrapper/py_score.cpp` | `ImportIssue` class, `getImportIssues`, `getImportIssuesDataFrame`, numpydoc, console guard | 1–5 |
| `maiacore/src/maiacore/python_wrapper/py_console.h` (new) | `tolerantStream`, `ConsoleRedirect` | 5 |
| `maiacore/include/maiacore/score_collection.h`, `score_collection.cpp`, `python_wrapper/py_score_collection.cpp` | UTF-8 paths; isolation, `getLoadErrors`, guards | 5, 6 |
| `tests-cpp/CMakeLists.txt`, `tests-cpp/src/import-report-test.cpp` (new), `tests-cpp/src/test-files.h` (new) | unit tests of the report; temporary score files | 1, 3 |
| `tests-cpp/src/score-test.cpp`, `tests-cpp/src/score-collection-test.cpp` | Score and ScoreCollection tests | 1–6 |
| `test/test_score_comprehensive.py`, `test/test_musicxml_dump.py`, `test/test_musicxml_transpose.py`, `test/test_score_collection.py` | Python tests | 1–6 |
| `test/xml_examples/unit_test/*.xml` (five comments), `test/musicxml/dump_score.py` (docstring) | wording: records, not warnings | 1 |
| `test/xml_examples/unit_test/import_report_dropped.musicxml` (new), `test/musicxml/ledger.json` | the dropped-element fixture and its line | 3 |
| `test/musicxml/corpus_worker.py`, `test/musicxml/README.md`, `test/test_musicxml_corpus.py` | no ASCII copy | 5 |
| `test/musicxml/corpus.py`, `corpus_worker.py`, `fuzz.py`, `scripts/make-corpus.py`, `scripts/make-fuzz.py`, `Makefile`, `test/musicxml/README.md`, `test/test_musicxml_corpus.py`, `test/test_musicxml_fuzz.py`, `test/musicxml/ledger*.json` | `exit`, `codes`, sidecar, `CORPUS_ARGS`, `--accept`; the regenerated ledgers | 7 |
| `CHANGELOG.md` | `[Unreleased]` | 8 |

---

### Task 0: Baseline (no commit)

**Files:** none changed.

**Interfaces:** produces the venv `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-venv`, the baseline counts and `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-fuzz-baseline.json`.

- [ ] **Step 1: Brand-new venv.** PowerShell: `py -3.12 -m venv C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-venv; & 'C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-venv\Scripts\python.exe' -m pip install -r requirements-dev.txt; $LASTEXITCODE` → 0.
- [ ] **Step 2: Build and install.** «build» `make "PYTHON=$py" dev; $LASTEXITCODE` → 0; then `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`.
- [ ] **Step 3: Baselines.** «build» `make "PYTHON=$py" cpp-tests; $LASTEXITCODE` → 0, `[  PASSED  ] 1188 tests.`. «build» `make "PYTHON=$py" py-tests; $LASTEXITCODE` → 0, `Ran 690 tests` / `OK (skipped=1)`. «build» `make "PYTHON=$py" validate; $LASTEXITCODE` → 0, `no new findings`; record the counts it prints.
- [ ] **Step 4: Fuzz baseline.** «build» `make "PYTHON=$py" fuzz; $LASTEXITCODE` → 0; record the outcome counts, then `Copy-Item test\musicxml\fuzz-work\report-seed-1.json C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-fuzz-baseline.json`.
- [ ] **Step 5: Import check from outside the repository.** Bash: `(cd /c/Users/nyck/AppData/Local/Temp && /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-venv/Scripts/python.exe -c "import maialib; print(maialib.__version__)"); echo "exit $?"` → the version, exit 0. `git status --short` → ` M .gitignore`.
- [ ] **Step 6: The crash this phase ends** (for the report). Write with the Write tool `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-crash.py`:

```python
import os
import subprocess
import sys
import tempfile

SOURCE = "test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml"
CHILD = "import maialib, sys; maialib.Score(sys.argv[1]); print('loaded')"
with open(SOURCE, "rb") as source:
    data = source.read().replace(b"<alter>0.25</alter>", b"<alter>" + bytes([0xE9]) + b"</alter>")
with tempfile.TemporaryDirectory() as folder:
    path = os.path.join(folder, "latin1.xml")
    with open(path, "wb") as target:
        target.write(data)
    done = subprocess.run([sys.executable, "-c", CHILD, path], capture_output=True)
print("exit", done.returncode, done.stdout)
```

  Bash, from the repository root: `/c/Users/nyck/AppData/Local/Temp/maialib-4c1a-venv/Scripts/python.exe /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-crash.py` → `exit 3221226505 b''` (0xC0000409) today; Task 1 makes it `exit 0` with `loaded`.

---

### Task 1: The import report, its first records, and valid UTF-8

**Files:**
- Create: `maiacore/include/maiacore/import-issue.h`, `maiacore/src/maiacore/import-report.h`, `maiacore/src/maiacore/import-report.cpp`, `tests-cpp/src/import-report-test.cpp`, `tests-cpp/src/test-files.h`
- Modify: `maiacore/include/maiacore/score.h` (includes; member ~46; constructor Doxygen ~105; `getImportIssues` after `getFileName` ~345; copy constructor and assignment ~477-510), `maiacore/src/maiacore/score.cpp` (includes ~24-37; `AccidentalCorrection` ~81; `clear()` ~603; getter ~636; part names ~750; title ~844; the accidental block ~1143-1225; end of `loadXMLFile` ~1309), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (includes; `ImportIssue` class and DataFrame builder before the `Score` class; `Score(filePath)` numpydoc; `getImportIssues`, `getImportIssuesDataFrame` after `getFileName`), `tests-cpp/CMakeLists.txt`, five fixture comments, `test/musicxml/dump_score.py` (docstring)
- Test: `tests-cpp/src/import-report-test.cpp`, `tests-cpp/src/score-test.cpp` (`ScoreQuarterToneRead` ~823-1035 rewritten to assert records; new `ScoreImportReport`), `test/test_score_comprehensive.py` (`ScoreQuarterToneReadTestCase` ~104-195; new `ScoreImportReportTestCase`), `test/test_musicxml_dump.py` (~22, ~93-107, ~162-171)

**Interfaces:**
- Produces (C++): `struct ImportIssue { std::string code; std::string kind; int partIndex = -1; std::string partName; std::string measureNumber; int measureIndex = -1; std::string element; std::string found; std::string used; std::string message; bool operator==(const ImportIssue&) const; }`; `const std::vector<ImportIssue>& Score::getImportIssues() const`; in `maiacore::detail` (`import-report.h`): `std::string validUtf8(const std::string& text)`, `struct IssueLocation { int partIndex = -1; std::string partName; std::string measureNumber; int measureIndex = -1; }`, `struct IssueCode { const char* code; const char* kind; }`, `const std::vector<IssueCode>& issueCatalogue()` (sorted by code), `ImportIssue makeIssue(const std::string& code, const IssueLocation& location, const std::string& element, const std::string& found, const std::string& used, const std::string& message)` (throws `std::logic_error` for a code outside the catalogue; every text through `validUtf8`), `std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues)`; the file-local `struct AccidentalCorrection { const char* code; const char* element; std::string found; std::string message; }` in `score.cpp`. Test helpers: `TemporaryFile(name, content)` with `path()` (UTF-8), `kMinimalAttributes`, `kWholeC4`, `minimalScore(content, attributes = kMinimalAttributes)` in `tests-cpp/src/test-files.h`; `correctionCodes(const Score&)` and `alterOffGrid(found)` in `score-test.cpp`.
- Produces (Python): `maialib.ImportIssue` (read-only attributes named as the fields, `==`, `repr`), `Score.getImportIssues() -> list[ImportIssue]`, `Score.getImportIssuesDataFrame() -> pandas.DataFrame` (columns in field order; text `str`, `partIndex`/`measureIndex` `int64`); helpers `correctionCodes`, `issueFields`, `ISSUE_FIELDS`, `summaryLine`, `alterOffGrid` in `test_score_comprehensive.py`.

- [ ] **Step 1: Write the failing C++ tests.** Apply, in order:

**`tests-cpp/CMakeLists.txt` ~44: replace**

````cmake
    src/melodic-lines-test.cpp
)
````

with

````cmake
    src/melodic-lines-test.cpp
    src/import-report-test.cpp
)
````


**Create `tests-cpp/src/test-files.h`**

````cpp
#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

// A file holding 'content', byte for byte, in a directory of its own under the system's temporary
// directory; the directory is removed with the file when the object is destroyed. 'name' is the
// file's name in UTF-8, extension included: maialib decides from it how to read the file.
class TemporaryFile {
   public:
    TemporaryFile(const std::string& name, const std::string& content)
        : _directory(std::filesystem::temp_directory_path() /
                     ("maialib-test-" +
                      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))),
          _path(_directory / std::filesystem::u8path(name)) {
        std::filesystem::create_directories(_directory);
        std::ofstream out(_path, std::ios::binary);
        out << content;
    }
    ~TemporaryFile() {
        std::error_code ignored;
        std::filesystem::remove_all(_directory, ignored);
    }
    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    // The file's path in UTF-8, as Score takes it.
    std::string path() const { return _path.u8string(); }

   private:
    std::filesystem::path _directory;
    std::filesystem::path _path;
};

// The <attributes> of the first measure of minimalScore() unless it is given others.
inline const std::string kMinimalAttributes =
    "<attributes><divisions>1</divisions><key><fifths>0</fifths></key><time><beats>4</beats>"
    "<beat-type>4</beat-type></time><clef><sign>G</sign><line>2</line></clef></attributes>";

// A whole-note C4 of voice 1.
inline const std::string kWholeC4 =
    "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice>"
    "<type>whole</type></note>";

// A MusicXML 4.0 score of one part, "Music", and one measure, numbered "1", holding 'content'
// after 'attributes'. Every element it adds is one the model holds.
inline std::string minimalScore(const std::string& content,
                                const std::string& attributes = kMinimalAttributes) {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<score-partwise version=\"4.0\"><part-list><score-part id=\"P1\"><part-name>Music"
           "</part-name></score-part></part-list><part id=\"P1\"><measure number=\"1\">" +
           attributes + content + "</measure></part></score-partwise>\n";
}
````


**Create `tests-cpp/src/import-report-test.cpp`**

````cpp
#include "import-report.h"

#include <gtest/gtest.h>

#include <cctype>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using maiacore::detail::importSummary;
using maiacore::detail::issueCatalogue;
using maiacore::detail::IssueCode;
using maiacore::detail::IssueLocation;
using maiacore::detail::makeIssue;
using maiacore::detail::validUtf8;

namespace {
const std::string kReplacement = "\xEF\xBF\xBD";  // U+FFFD in UTF-8
}  // namespace

// Valid UTF-8 passes unchanged: ASCII, and two-, three- and four-byte sequences at the edges of
// their ranges.
TEST(ImportReportUtf8, ValidTextIsUnchanged) {
    for (const std::string& text :
         {std::string("C4 <alter>"), std::string("can\xC3\xA7\xC3\xA3o"),
          std::string("\xE6\x97\xA5\xE6\x9C\xAC"), std::string("\xF0\x9D\x84\x9E"),
          std::string("\xED\x9F\xBF"), std::string("\xEE\x80\x80"), std::string("\xF4\x8F\xBF\xBF"),
          std::string("")}) {
        EXPECT_EQ(validUtf8(text), text);
    }
}

// Each byte that belongs to no well-formed sequence becomes one U+FFFD; the bytes around it stay.
TEST(ImportReportUtf8, EachInvalidByteBecomesAReplacementCharacter) {
    EXPECT_EQ(validUtf8("\xE9"), kReplacement);                     // Latin-1 e acute
    EXPECT_EQ(validUtf8("a\xE9z"), "a" + kReplacement + "z");       // in the middle
    EXPECT_EQ(validUtf8("\x80"), kReplacement);                     // stray continuation
    EXPECT_EQ(validUtf8("\xC3"), kReplacement);                     // truncated at the end
    EXPECT_EQ(validUtf8("\xC0\xAF"), kReplacement + kReplacement);  // overlong '/'
    EXPECT_EQ(validUtf8("\xE0\x80\xAF"), kReplacement + kReplacement + kReplacement);
    EXPECT_EQ(validUtf8("\xED\xA0\x80"), kReplacement + kReplacement + kReplacement);  // surrogate
    EXPECT_EQ(validUtf8("\xF4\x90\x80\x80"),
              kReplacement + kReplacement + kReplacement + kReplacement);  // above U+10FFFF
    EXPECT_EQ(validUtf8("\xE6\x97x"), kReplacement + kReplacement + "x");  // cut short
}

// Every code is UPPER_SNAKE, listed once, in sorted order, with the kind "corrected" or "dropped".
TEST(ImportReportCatalogue, CodesAreUpperSnakeSortedAndUnique) {
    std::string previous;
    for (const IssueCode& entry : issueCatalogue()) {
        const std::string code = entry.code;
        ASSERT_FALSE(code.empty());
        for (const char c : code) {
            EXPECT_TRUE(std::isupper(static_cast<unsigned char>(c)) || c == '_') << code;
        }
        EXPECT_LT(previous, code);
        previous = code;
        EXPECT_TRUE(std::string(entry.kind) == "corrected" || std::string(entry.kind) == "dropped")
            << code;
    }
}

// A record takes its kind from the catalogue and every text through validUtf8(); a code outside
// the catalogue is a programming error.
TEST(ImportReportRecord, ARecordTakesItsKindAndSanitisesItsText) {
    const ImportIssue issue = makeIssue("ALTER_OFF_GRID", IssueLocation{0, "Viol\xE9", "1\xE9", 3},
                                        "note/\xE9", "\xE9", "C4", "The <alter> value '\xE9' ...");
    EXPECT_EQ(issue.code, "ALTER_OFF_GRID");
    EXPECT_EQ(issue.kind, "corrected");
    EXPECT_EQ(issue.partIndex, 0);
    EXPECT_EQ(issue.partName, "Viol" + kReplacement);
    EXPECT_EQ(issue.measureNumber, "1" + kReplacement);
    EXPECT_EQ(issue.measureIndex, 3);
    EXPECT_EQ(issue.element, "note/" + kReplacement);
    EXPECT_EQ(issue.found, kReplacement);
    EXPECT_EQ(issue.used, "C4");
    EXPECT_EQ(issue.message, "The <alter> value '" + kReplacement + "' ...");

    EXPECT_THROW(makeIssue("NOT_A_CODE", IssueLocation{}, "", "", "", ""), std::logic_error);
}

// The summary counts the corrections and the distinct elements of the dropped records.
TEST(ImportReportSummary, TheSummaryCountsCorrectionsAndDroppedElementTypes) {
    ImportIssue corrected;
    corrected.kind = "corrected";
    ImportIssue lyric;
    lyric.kind = "dropped";
    lyric.element = "note/lyric";
    ImportIssue direction = lyric;
    direction.element = "direction";
    EXPECT_EQ(importSummary("a.xml", {corrected, corrected, lyric, direction, lyric}),
              "[maiacore] a.xml: 2 corrections, 2 element types not modelled (dropped on "
              "export); see Score.getImportIssues()");
    EXPECT_EQ(importSummary("b\xE9.xml", {corrected}),
              "[maiacore] b" + kReplacement +
                  ".xml: 1 corrections, 0 element types not modelled (dropped on export); see "
                  "Score.getImportIssues()");
}
````


**`tests-cpp/src/score-test.cpp` ~24: replace**

````cpp
#include "test-capture.h"
#include "test-locale.h"
````

with

````cpp
#include "test-capture.h"
#include "test-files.h"
#include "test-locale.h"
````


**`tests-cpp/src/score-test.cpp` ~823: replace**

````cpp
    // library spells. It carries a usable <alter>1</alter>, so the whole load must not abort;
    // it must degrade to the <alter> value, same as if no <accidental> had been present.
    Score score("./test/xml_examples/unit_test/quarter_tone_unknown_accidental_name.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C#4");
}

TEST(ScoreQuarterToneRead, UnrepresentableAlterTripleSharpFallsBackToNatural) {
    // <alter>3</alter>, no <accidental> at all. A triple sharp is outside the nine values
    // Helper::alterValue2symbol() can spell -- must load as natural with a warning, not throw.
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_triple_sharp.xml");
````

with

````cpp
    // library spells. It carries a usable <alter>1</alter>, so the whole load must not abort;
    // it must degrade to the <alter> value, same as if no <accidental> had been present, and
    // record the name it could not use.
    Score score("./test/xml_examples/unit_test/quarter_tone_unknown_accidental_name.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C#4");
    ImportIssue expected;
    expected.code = "ACCIDENTAL_NAME_UNKNOWN";
    expected.kind = "corrected";
    expected.partIndex = 0;
    expected.partName = "Music";
    expected.measureNumber = "1";
    expected.measureIndex = 0;
    expected.element = "note/accidental";
    expected.found = "natural-sharp";
    expected.used = "C#4";
    expected.message =
        "The <accidental> name 'natural-sharp' is not one this library can spell; the note is "
        "read from its <alter>, or as natural without one.";
    EXPECT_EQ(score.getImportIssues(), std::vector<ImportIssue>{expected});
}

TEST(ScoreQuarterToneRead, UnrepresentableAlterTripleSharpFallsBackToNatural) {
    // <alter>3</alter>, no <accidental> at all. A triple sharp is outside the nine values
    // Helper::alterValue2symbol() can spell -- must load as natural with a record, not throw.
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_triple_sharp.xml");
````


**`tests-cpp/src/score-test.cpp` ~864: replace**

````cpp

// Loads a one-note score whose only note is a C4 with the given <alter> text and no
// <accidental>, and returns its pitch and what the load printed.
std::pair<std::string, std::string> loadWithAlterText(const std::string& alterText) {
    std::ifstream in("./test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml");
````

with

````cpp

// The codes of a score's "corrected" records, in report order.
std::vector<std::string> correctionCodes(const Score& score) {
    std::vector<std::string> codes;
    for (const ImportIssue& issue : score.getImportIssues()) {
        if (issue.kind == "corrected") {
            codes.push_back(issue.code);
        }
    }
    return codes;
}

// The record of an <alter> this library cannot spell, on the C4 of measure "1" of the part
// "Music".
ImportIssue alterOffGrid(const std::string& found) {
    ImportIssue issue;
    issue.code = "ALTER_OFF_GRID";
    issue.kind = "corrected";
    issue.partIndex = 0;
    issue.partName = "Music";
    issue.measureNumber = "1";
    issue.measureIndex = 0;
    issue.element = "note/pitch/alter";
    issue.found = found;
    issue.used = "C4";
    issue.message = "The <alter> value '" + found +
                    "' is not one this library can spell (a multiple of 0.5 from -2 to 2); the "
                    "note is read as natural, its pitch off by that amount.";
    return issue;
}

// What loading a one-note score gave: the note's pitch, the import report and what was printed.
struct LoadedNote {
    std::string pitch;
    std::vector<ImportIssue> issues;
    std::string printed;
};

// Loads a one-note score whose only note is a C4 with the given <alter> text and no
// <accidental>.
LoadedNote loadWithAlterText(const std::string& alterText) {
    std::ifstream in("./test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml");
````


**`tests-cpp/src/score-test.cpp` ~877: replace**

````cpp

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "maialib_alter_text.xml";
    {
        std::ofstream out(path);
        out << xml;
    }

    std::string pitch;
    std::string printed;
    {
        StdoutCapture capture;
        Score score(path.string());
        pitch = score.getPart(0).getMeasure(0).getNote(0, 0).getPitch();
        printed = capture.str();
    }
    std::filesystem::remove(path);
    return {pitch, printed};
}
}  // namespace

// A quarter tone given by <alter> alone, with no <accidental>: the form a quarter tone takes when
// its accidental carries through the measure.
TEST(ScoreQuarterToneRead, AlterWithoutAccidentalIsReadAsAQuarterTone) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}
````

with

````cpp

    const TemporaryFile file("maialib_alter_text.xml", xml);
    LoadedNote loaded;
    {
        StdoutCapture capture;
        Score score(file.path());
        loaded.pitch = score.getPart(0).getMeasure(0).getNote(0, 0).getPitch();
        loaded.issues = score.getImportIssues();
        loaded.printed = capture.str();
    }
    return loaded;
}

// The line a load prints when its report holds 'corrections' corrections and nothing dropped.
std::string summaryLine(const std::string& fileName, const int corrections) {
    return "[maiacore] " + fileName + ": " + std::to_string(corrections) +
           " corrections, 0 element types not modelled (dropped on export); see "
           "Score.getImportIssues()\n";
}
}  // namespace

// A quarter tone given by <alter> alone, with no <accidental>: the form a quarter tone takes when
// its accidental carries through the measure.
TEST(ScoreQuarterToneRead, AlterWithoutAccidentalIsReadAsAQuarterTone) {
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}
````


**`tests-cpp/src/score-test.cpp` ~919: replace**

````cpp
    ASSERT_EQ(std::string(formatted), "0,5") << "the locale does not use a decimal comma";

    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

````

with

````cpp
    ASSERT_EQ(std::string(formatted), "0,5") << "the locale does not use a decimal comma";

    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}

````


**`tests-cpp/src/score-test.cpp` ~940: replace**

````cpp

    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// The whole <alter> text must be the number, surrounding whitespace aside: "1,5" is not a sharp
// followed by noise, and a number with trailing text is not a number.
TEST(ScoreQuarterToneRead, AlterTextMustBeANumberAndNothingElse) {
    for (const std::string text : {"1,5", "0.5abc", "0.5.5", "sharp"}) {
        const auto [pitch, printed] = loadWithAlterText(text);
        EXPECT_EQ(pitch, "C4") << "<alter>" << text << "</alter>";
        EXPECT_NE(printed.find("[WARN] Unrepresentable <alter> value '" + text + "'"),
                  std::string::npos)
            << printed;
    }

    for (const std::string text : {" 0.5 ", "\n-1.5\n", "+0.5"}) {
        const auto [pitch, printed] = loadWithAlterText(text);
        EXPECT_EQ(pitch, text.find("-1.5") != std::string::npos ? "C3b4" : "C1x4")
            << "<alter>" << text << "</alter>";
        EXPECT_EQ(printed.find("[WARN]"), std::string::npos) << printed;
    }
}

// Near a quarter-tone sharp is not a quarter-tone sharp: the reader never rounds to the nearest
// representable pitch, it reads the note as natural and says so, naming the value.
TEST(ScoreQuarterToneRead, AlterNearAQuarterToneIsNotSnappedOntoIt) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_near_quarter_tone.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C4");
    EXPECT_NE(capture.str().find("[WARN] Unrepresentable <alter> value '0.46'"), std::string::npos)
        << capture.str();
}

// A recognised <accidental> wins over a disagreeing <alter>, and the disagreement is reported.
TEST(ScoreQuarterToneRead, DisagreeingAccidentalWinsWithAWarning) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_accidental_alter_disagree.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
    EXPECT_NE(capture.str().find("[WARN] The <accidental> 'quarter-sharp' and the <alter> '1' of "
                                 "this note disagree"),
              std::string::npos)
        << capture.str();
}

TEST(ScoreQuarterToneRead, AgreeingAccidentalAndAlterReadWithoutAWarning) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_tartini.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// "sharp-sharp" is MusicXML's double sharp drawn as two sharp signs: a recognised name.
TEST(ScoreQuarterToneRead, SharpSharpAccidentalIsADoubleSharp) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/accidental_sharp_sharp.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "Cx4");
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}
````

with

````cpp

    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}

// The whole <alter> text must be the number, surrounding whitespace aside: "1,5" is not a sharp
// followed by noise, and a number with trailing text is not a number. A text that is not one is
// recorded, and the load prints one summary line; a valid one records and prints nothing.
TEST(ScoreQuarterToneRead, AlterTextMustBeANumberAndNothingElse) {
    for (const std::string text : {"1,5", "0.5abc", "0.5.5", "sharp"}) {
        const LoadedNote loaded = loadWithAlterText(text);
        EXPECT_EQ(loaded.pitch, "C4") << "<alter>" << text << "</alter>";
        EXPECT_EQ(loaded.issues, std::vector<ImportIssue>{alterOffGrid(text)});
        EXPECT_EQ(loaded.printed, summaryLine("maialib_alter_text.xml", 1));
    }

    for (const std::string text : {" 0.5 ", "\n-1.5\n", "+0.5"}) {
        const LoadedNote loaded = loadWithAlterText(text);
        EXPECT_EQ(loaded.pitch, text.find("-1.5") != std::string::npos ? "C3b4" : "C1x4")
            << "<alter>" << text << "</alter>";
        EXPECT_EQ(loaded.issues, std::vector<ImportIssue>{});
        EXPECT_EQ(loaded.printed, "");
    }
}

// An <alter> whose bytes are not UTF-8 is recorded with U+FFFD in place of each invalid byte, in
// the record and in its message: Python decodes both as UTF-8.
TEST(ScoreQuarterToneRead, AnAlterThatIsNotUtf8IsRecordedAsValidUtf8) {
    const LoadedNote loaded = loadWithAlterText("\xE9");
    EXPECT_EQ(loaded.pitch, "C4");
    EXPECT_EQ(loaded.issues, std::vector<ImportIssue>{alterOffGrid("\xEF\xBF\xBD")});
}

// Near a quarter-tone sharp is not a quarter-tone sharp: the reader never rounds to the nearest
// representable pitch, it reads the note as natural and records the value.
TEST(ScoreQuarterToneRead, AlterNearAQuarterToneIsNotSnappedOntoIt) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_near_quarter_tone.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C4");
    EXPECT_EQ(score.getImportIssues(), std::vector<ImportIssue>{alterOffGrid("0.46")});
    EXPECT_EQ(capture.str(), summaryLine("unrepresentable_alter_near_quarter_tone.xml", 1));
}

// A recognised <accidental> wins over a disagreeing <alter>, and the disagreement is recorded.
TEST(ScoreQuarterToneRead, DisagreeingAccidentalWinsWithARecord) {
    Score score("./test/xml_examples/unit_test/quarter_tone_accidental_alter_disagree.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
    ImportIssue expected = alterOffGrid("1");
    expected.code = "ACCIDENTAL_ALTER_MISMATCH";
    expected.used = "C1x4";
    expected.message =
        "The <accidental> 'quarter-sharp' and the <alter> '1' of this note disagree; the "
        "<accidental> is used.";
    EXPECT_EQ(score.getImportIssues(), std::vector<ImportIssue>{expected});
}

TEST(ScoreQuarterToneRead, AgreeingAccidentalAndAlterReadWithoutARecord) {
    Score score("./test/xml_examples/unit_test/quarter_tone_tartini.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}

// "sharp-sharp" is MusicXML's double sharp drawn as two sharp signs: a recognised name.
TEST(ScoreQuarterToneRead, SharpSharpAccidentalIsADoubleSharp) {
    Score score("./test/xml_examples/unit_test/accidental_sharp_sharp.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "Cx4");
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}
````


**`tests-cpp/src/score-test.cpp` ~1019: replace**

````cpp
    std::vector<std::string> readBack;
    std::string printed;
    {
        StdoutCapture capture;
        Score reread(written);
        readBack = writtenPitches(reread);
        printed = capture.str();
    }
    std::filesystem::remove(written);

    EXPECT_EQ(readBack, pitches);
    EXPECT_EQ(printed.find("[WARN]"), std::string::npos) << printed;
}
````

with

````cpp
    std::vector<std::string> readBack;
    std::vector<std::string> corrections;
    {
        Score reread(written);
        readBack = writtenPitches(reread);
        corrections = correctionCodes(reread);
    }
    std::filesystem::remove(written);

    EXPECT_EQ(readBack, pitches);
    EXPECT_EQ(corrections, std::vector<std::string>{});
}

// ====================
// The import report
// ====================

// A score built through the API has an empty report.
TEST(ScoreImportReport, AScoreBuiltThroughTheApiHasAnEmptyReport) {
    Score score({"Piano"}, 2);
    EXPECT_EQ(score.getImportIssues(), std::vector<ImportIssue>{});
}

// The report describes the load: a copy and an assignment carry it, edits and exports leave it,
// and clear() empties it.
TEST(ScoreImportReport, TheReportIsCopiedKeptThroughEditsAndEmptiedByClear) {
    Score original("./test/xml_examples/unit_test/unrepresentable_alter_near_quarter_tone.xml");
    const std::vector<ImportIssue> report = {alterOffGrid("0.46")};
    ASSERT_EQ(original.getImportIssues(), report);

    const Score copy(original);
    EXPECT_EQ(copy.getImportIssues(), report);
    Score assigned({"Piano"}, 1);
    assigned = original;
    EXPECT_EQ(assigned.getImportIssues(), report);

    original.setTitle("Edited");
    original.getPart(0).getMeasure(0).addNote(Note("D4"));
    original.toXML();
    EXPECT_EQ(original.getImportIssues(), report);

    original.clear();
    EXPECT_EQ(original.getImportIssues(), std::vector<ImportIssue>{});
    EXPECT_EQ(copy.getImportIssues(), report);
}

// A title, a composer and a part name whose bytes are not UTF-8 are held with U+FFFD in place of
// each invalid byte: their getters return them to Python, which decodes them as UTF-8.
TEST(ScoreImportReport, NamesThatAreNotUtf8AreHeldAsValidUtf8) {
    std::string text = minimalScore(kWholeC4);
    text.replace(text.find("<part-list>"), 0,
                 "<work><work-title>Sonata \xE9</work-title></work><identification><creator "
                 "type=\"composer\">Jos\xE9</creator></identification>");
    text.replace(text.find("<part-name>Music"), 16, "<part-name>M\xFAsica");
    const TemporaryFile file("names.musicxml", text);

    Score score(file.path());
    EXPECT_EQ(score.getTitle(), "Sonata \xEF\xBF\xBD");
    EXPECT_EQ(score.getComposerName(), "Jos\xEF\xBF\xBD");
    EXPECT_EQ(score.getPartsNames(), std::vector<std::string>{"M\xEF\xBF\xBDsica"});
}

// A load with an empty report prints nothing; one with records prints exactly one summary line,
// however many records it holds.
TEST(ScoreImportReport, ALoadPrintsOneSummaryLineOnlyWhenTheReportIsNotEmpty) {
    const TemporaryFile clean("clean.musicxml", minimalScore(kWholeC4));
    const std::string offGrid =
        "<note><pitch><step>C</step><alter>0.3</alter><octave>4</octave></pitch>"
        "<duration>2</duration><voice>1</voice><type>half</type></note>";
    const TemporaryFile corrected("corrected.musicxml", minimalScore(offGrid + offGrid));

    StdoutCapture capture;
    { Score score(clean.path()); }
    EXPECT_EQ(capture.str(), "");
    Score score(corrected.path());
    EXPECT_EQ(correctionCodes(score),
              (std::vector<std::string>{"ALTER_OFF_GRID", "ALTER_OFF_GRID"}));
    EXPECT_EQ(capture.str(), summaryLine("corrected.musicxml", 2));
}
````

- [ ] **Step 2: Write the failing Python tests.** Apply, in order:

**`test/test_score_comprehensive.py` ~16: replace**

````python
import unittest

````

with

````python
import unittest

import pandas

````


**`test/test_score_comprehensive.py` ~39: replace**

````python
            continue
    return None


````

with

````python
            continue
    return None


def correctionCodes(score):
    """The codes of the score's "corrected" import records, in report order."""
    return [issue.code for issue in score.getImportIssues() if issue.kind == "corrected"]


def issueFields(issue):
    """An import record as a dict of its fields."""
    return {name: getattr(issue, name) for name in ISSUE_FIELDS}


# The fields of an ImportIssue, which are also the columns of getImportIssuesDataFrame().
ISSUE_FIELDS = [
    "code",
    "kind",
    "partIndex",
    "partName",
    "measureNumber",
    "measureIndex",
    "element",
    "found",
    "used",
    "message",
]


def summaryLine(fileName, corrections):
    """The line a load prints for a report of 'corrections' corrections and nothing dropped."""
    return (
        f"[maiacore] {fileName}: {corrections} corrections, 0 element types not modelled "
        "(dropped on export); see Score.getImportIssues()\n"
    )


def alterOffGrid(found, used="C4"):
    """The record of an <alter> this library cannot spell, on the first note of measure "1" of
    the part "Music"."""
    return {
        "code": "ALTER_OFF_GRID",
        "kind": "corrected",
        "partIndex": 0,
        "partName": "Music",
        "measureNumber": "1",
        "measureIndex": 0,
        "element": "note/pitch/alter",
        "found": found,
        "used": used,
        "message": f"The <alter> value '{found}' is not one this library can spell (a multiple "
        "of 0.5 from -2 to 2); the note is read as natural, its pitch off by that amount.",
    }


````


**`test/test_score_comprehensive.py` ~104: replace**

````python
        spells but carries a usable <alter>1</alter>; the load must degrade to that
        value instead of raising and aborting."""
        self.assertEqual(self._first_note_pitch("quarter_tone_unknown_accidental_name.xml"), "C#4")

    def test_unrepresentable_alter_falls_back_to_natural_instead_of_raising(self):
        """<alter>3</alter>, no <accidental> at all: a triple sharp is outside the nine
        values this library's accidental vocabulary can spell. The load must degrade to
        natural, with a warning, rather than raising and aborting -- this is the path a
        real ml.Score() user actually hits, not just the underlying C++ function."""
        self.assertEqual(self._first_note_pitch("unrepresentable_alter_triple_sharp.xml"), "C4")

    def _pitches_and_output(self, fileName):
        """Every written pitch of the loaded score's first part, and what the load printed."""
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            score = ml.Score(f"./xml_examples/unit_test/{fileName}")
        return writtenPitches(score), buffer.getvalue()

    def test_alter_without_accidental_is_read_as_a_quarter_tone(self):
        """<alter>0.5</alter> and <alter>-1.5</alter> with no <accidental>: the form a quarter
        tone takes when its accidental carries through the measure."""
        pitches, printed = self._pitches_and_output("quarter_tone_alter_only.xml")
        self.assertEqual(pitches, ["C1x4", "E3b4"])
        self.assertNotIn("[WARN]", printed)

````

with

````python
        spells but carries a usable <alter>1</alter>; the load must degrade to that
        value instead of raising and aborting, and record the name."""
        self.assertEqual(self._first_note_pitch("quarter_tone_unknown_accidental_name.xml"), "C#4")
        score = ml.Score("./xml_examples/unit_test/quarter_tone_unknown_accidental_name.xml")
        self.assertEqual(
            [issueFields(issue) for issue in score.getImportIssues()],
            [
                {
                    **alterOffGrid("natural-sharp", "C#4"),
                    "code": "ACCIDENTAL_NAME_UNKNOWN",
                    "element": "note/accidental",
                    "message": "The <accidental> name 'natural-sharp' is not one this library "
                    "can spell; the note is read from its <alter>, or as natural without one.",
                }
            ],
        )

    def test_unrepresentable_alter_falls_back_to_natural_instead_of_raising(self):
        """<alter>3</alter>, no <accidental> at all: a triple sharp is outside the nine
        values this library's accidental vocabulary can spell. The load must degrade to
        natural, with a record, rather than raising and aborting -- this is the path a
        real ml.Score() user actually hits, not just the underlying C++ function."""
        self.assertEqual(self._first_note_pitch("unrepresentable_alter_triple_sharp.xml"), "C4")

    def _pitches_and_output(self, fileName):
        """Every written pitch of the loaded score's first part, its import records as dicts,
        and what the load printed."""
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            score = ml.Score(f"./xml_examples/unit_test/{fileName}")
        issues = [issueFields(issue) for issue in score.getImportIssues()]
        return writtenPitches(score), issues, buffer.getvalue()

    def test_alter_without_accidental_is_read_as_a_quarter_tone(self):
        """<alter>0.5</alter> and <alter>-1.5</alter> with no <accidental>: the form a quarter
        tone takes when its accidental carries through the measure."""
        pitches, issues, printed = self._pitches_and_output("quarter_tone_alter_only.xml")
        self.assertEqual(pitches, ["C1x4", "E3b4"])
        self.assertEqual([issue for issue in issues if issue["kind"] == "corrected"], [])

````


**`test/test_score_comprehensive.py` ~139: replace**

````python
            self.assertEqual(locale.localeconv()["decimal_point"], ",")
            pitches, printed = self._pitches_and_output("quarter_tone_alter_only.xml")
        finally:
            locale.setlocale(locale.LC_NUMERIC, previous)
        self.assertEqual(pitches, ["C1x4", "E3b4"])
        self.assertNotIn("[WARN]", printed)

    def test_alter_near_a_quarter_tone_is_not_snapped_onto_it(self):
        """<alter>0.46</alter> is near a quarter-tone sharp but is not one: the note reads as
        natural, with a warning naming the value, never rounded to the nearest pitch."""
        pitches, printed = self._pitches_and_output("unrepresentable_alter_near_quarter_tone.xml")
        self.assertEqual(pitches, ["C4"])
        self.assertIn("[WARN] Unrepresentable <alter> value '0.46'", printed)

    def test_disagreeing_accidental_wins_with_a_warning(self):
        """<accidental>quarter-sharp</accidental> with <alter>1</alter>: the accidental wins,
        and the disagreement is reported."""
        pitches, printed = self._pitches_and_output("quarter_tone_accidental_alter_disagree.xml")
        self.assertEqual(pitches, ["C1x4"])
        self.assertIn(
            "[WARN] The <accidental> 'quarter-sharp' and the <alter> '1' of this note disagree",
            printed,
        )

    def test_agreeing_accidental_and_alter_read_without_a_warning(self):
        pitches, printed = self._pitches_and_output("quarter_tone_tartini.xml")
        self.assertEqual(pitches, ["C1x4"])
        self.assertNotIn("[WARN]", printed)

    def test_sharp_sharp_accidental_is_a_double_sharp(self):
        """"sharp-sharp" is MusicXML's double sharp drawn as two sharp signs."""
        pitches, printed = self._pitches_and_output("accidental_sharp_sharp.xml")
        self.assertEqual(pitches, ["Cx4"])
        self.assertNotIn("[WARN]", printed)

````

with

````python
            self.assertEqual(locale.localeconv()["decimal_point"], ",")
            pitches, issues, printed = self._pitches_and_output("quarter_tone_alter_only.xml")
        finally:
            locale.setlocale(locale.LC_NUMERIC, previous)
        self.assertEqual(pitches, ["C1x4", "E3b4"])
        self.assertEqual([issue for issue in issues if issue["kind"] == "corrected"], [])

    def test_alter_near_a_quarter_tone_is_not_snapped_onto_it(self):
        """<alter>0.46</alter> is near a quarter-tone sharp but is not one: the note reads as
        natural, with a record naming the value, never rounded to the nearest pitch; the load
        prints one summary line."""
        fileName = "unrepresentable_alter_near_quarter_tone.xml"
        pitches, issues, printed = self._pitches_and_output(fileName)
        self.assertEqual(pitches, ["C4"])
        self.assertEqual(issues, [alterOffGrid("0.46")])
        self.assertEqual(printed, summaryLine(fileName, 1))

    def test_disagreeing_accidental_wins_with_a_record(self):
        """<accidental>quarter-sharp</accidental> with <alter>1</alter>: the accidental wins,
        and the disagreement is recorded."""
        pitches, issues, _ = self._pitches_and_output("quarter_tone_accidental_alter_disagree.xml")
        self.assertEqual(pitches, ["C1x4"])
        self.assertEqual(
            issues,
            [
                {
                    **alterOffGrid("1", "C1x4"),
                    "code": "ACCIDENTAL_ALTER_MISMATCH",
                    "message": "The <accidental> 'quarter-sharp' and the <alter> '1' of this "
                    "note disagree; the <accidental> is used.",
                }
            ],
        )

    def test_agreeing_accidental_and_alter_read_without_a_record(self):
        pitches, issues, _ = self._pitches_and_output("quarter_tone_tartini.xml")
        self.assertEqual(pitches, ["C1x4"])
        self.assertEqual([issue for issue in issues if issue["kind"] == "corrected"], [])

    def test_sharp_sharp_accidental_is_a_double_sharp(self):
        """"sharp-sharp" is MusicXML's double sharp drawn as two sharp signs."""
        pitches, issues, _ = self._pitches_and_output("accidental_sharp_sharp.xml")
        self.assertEqual(pitches, ["Cx4"])
        self.assertEqual([issue for issue in issues if issue["kind"] == "corrected"], [])

````


**`test/test_score_comprehensive.py` ~185: replace**

````python
            original.toFile(base, False)
            buffer = io.StringIO()
            with contextlib.redirect_stdout(buffer):
                reread = ml.Score(base + ".xml")
            readBack = writtenPitches(reread)

        self.assertEqual(readBack, pitches)
        self.assertNotIn("[WARN]", buffer.getvalue())

````

with

````python
            original.toFile(base, False)
            reread = ml.Score(base + ".xml")
            readBack = writtenPitches(reread)

        self.assertEqual(readBack, pitches)
        self.assertEqual(correctionCodes(reread), [])


class ScoreImportReportTestCase(unittest.TestCase):
    """Score.getImportIssues(), its DataFrame, and what a load prints."""

    DISAGREE = "./xml_examples/unit_test/quarter_tone_accidental_alter_disagree.xml"

    def test_a_score_built_through_the_api_has_an_empty_report(self):
        score = ml.Score(["Piano"], 2)
        self.assertEqual(score.getImportIssues(), [])
        frame = score.getImportIssuesDataFrame()
        self.assertEqual(list(frame.columns), ISSUE_FIELDS)
        self.assertEqual(len(frame), 0)
        loaded = ml.Score(self.DISAGREE).getImportIssuesDataFrame()
        self.assertEqual(list(frame.dtypes.items()), list(loaded.dtypes.items()))

    def test_the_dataframe_has_one_row_per_record_and_fixed_dtypes(self):
        score = ml.Score(self.DISAGREE)
        frame = score.getImportIssuesDataFrame()
        self.assertEqual(
            frame.to_dict("records"), [issueFields(issue) for issue in score.getImportIssues()]
        )
        self.assertEqual(len(frame), 1)
        textDtype = str(pandas.Series([], dtype=str).dtype)
        for column in ISSUE_FIELDS:
            expected = "int64" if column in ("partIndex", "measureIndex") else textDtype
            self.assertEqual(str(frame[column].dtype), expected, column)

    def test_a_copy_keeps_the_report_and_clear_empties_it(self):
        score = ml.Score(self.DISAGREE)
        report = [issueFields(issue) for issue in score.getImportIssues()]
        collection = ml.ScoreCollection()
        collection.addScore(score)  # copies the score
        score.setTitle("Edited")
        score.toXML()
        self.assertEqual([issueFields(issue) for issue in score.getImportIssues()], report)
        score.clear()
        self.assertEqual(score.getImportIssues(), [])
        copy = collection.getScores()[0]
        self.assertEqual([issueFields(issue) for issue in copy.getImportIssues()], report)

    def test_records_compare_by_value(self):
        first = ml.Score(self.DISAGREE).getImportIssues()
        second = ml.Score(self.DISAGREE).getImportIssues()
        self.assertTrue(first[0] == second[0])
        self.assertIn("ACCIDENTAL_ALTER_MISMATCH", repr(first[0]))

    def test_names_that_are_not_utf8_are_read_with_replacement_characters(self):
        """A title and a part name whose bytes are not UTF-8 (Latin-1 bytes in a file that
        declares UTF-8) are read as valid UTF-8, with U+FFFD in place of each invalid byte."""
        with open("./xml_examples/unit_test/quarter_tone_tartini.xml", "rb") as source:
            data = source.read()
        data = data.replace(
            b"<part-list>", b"<work><work-title>Sonata \xe9</work-title></work><part-list>"
        )
        data = data.replace(b"<part-name>Music", b"<part-name>M\xfasica")
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "latin1-names.xml")
            with open(path, "wb") as target:
                target.write(data)
            score = ml.Score(path)
        self.assertEqual(score.getTitle(), "Sonata \ufffd")
        self.assertEqual(score.getPartsNames(), ["M\ufffdsica"])

    def test_an_alter_that_is_not_utf8_does_not_end_the_interpreter(self):
        """Run in a child process, whose standard output is a pipe: a message quoting bytes
        that are not UTF-8 must not end the process. The record holds U+FFFD in their place."""
        code = (
            "import json, os, tempfile\n"
            "import maialib as ml\n"
            "source = open('./xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml', "
            "'rb').read()\n"
            "source = source.replace(b'<alter>0.25</alter>', b'<alter>\\xe9</alter>')\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            "    path = os.path.join(directory, 'latin1.xml')\n"
            "    open(path, 'wb').write(source)\n"
            "    score = ml.Score(path)\n"
            "    print('RESULT', json.dumps([(i.code, i.found) for i in score.getImportIssues()]))\n"
        )
        completed = runChild(code)
        self.assertIsNotNone(completed)
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        self.assertEqual(resultLine(completed), 'RESULT [["ALTER_OFF_GRID", "\\ufffd"]]')
        self.assertIn(summaryLine("latin1.xml", 1), completed.stdout.replace("\r\n", "\n"))

````


**`test/test_musicxml_dump.py` ~22: replace**

````python

# Its <accidental> name is unknown to maialib, which prints a warning while it loads the score.
WARNING_FIXTURE = (
````

with

````python

# Its <accidental> name is unknown to maialib, which records a correction while it loads the score
# and prints the summary line of its import report.
WARNING_FIXTURE = (
````


**`test/test_musicxml_dump.py` ~93: replace**

````python

    def test_the_command_line_prints_only_the_dump_and_warnings_go_to_stderr(self):
        command = [sys.executable, str(MUSICXML / "dump_score.py"), str(WARNING_FIXTURE)]
        done = subprocess.run(command, capture_output=True, timeout=120)
        self.assertEqual(0, done.returncode, done.stderr.decode("utf-8", "replace"))
        try:
            printed = json.loads(done.stdout)
        except ValueError:
            self.fail(f"stdout is not only the JSON dump: {done.stdout[:200]!r}")
        with contextlib.redirect_stdout(io.StringIO()):  # where the load's warning goes
            expected = dump_score.dump_score(ml.Score(str(WARNING_FIXTURE)))
        self.assertEqual(expected, printed)
        self.assertNotIn(b"\r", done.stdout)
        self.assertIn(b"[WARN] Unrecognized <accidental> name 'natural-sharp'", done.stderr)

````

with

````python

    def test_the_command_line_prints_only_the_dump_and_the_load_summary_goes_to_stderr(self):
        command = [sys.executable, str(MUSICXML / "dump_score.py"), str(WARNING_FIXTURE)]
        done = subprocess.run(command, capture_output=True, timeout=120)
        self.assertEqual(0, done.returncode, done.stderr.decode("utf-8", "replace"))
        try:
            printed = json.loads(done.stdout)
        except ValueError:
            self.fail(f"stdout is not only the JSON dump: {done.stdout[:200]!r}")
        with contextlib.redirect_stdout(io.StringIO()):  # where the load's summary goes
            expected = dump_score.dump_score(ml.Score(str(WARNING_FIXTURE)))
        self.assertEqual(expected, printed)
        self.assertNotIn(b"\r", done.stdout)
        self.assertEqual(
            b"[maiacore] quarter_tone_unknown_accidental_name.xml: 1 corrections, 0 element types "
            b"not modelled (dropped on export); see Score.getImportIssues()",
            done.stderr.strip(),
        )

````


**`test/test_musicxml_dump.py` ~162: replace**

````python
        note = measure["staves"][0]["notes"][0]
        self.assertEqual(error, dump["title"])
        self.assertEqual(error, dump["composer"])
        self.assertEqual(error, part["name"])
        self.assertEqual(error, part["short_name"])
````

with

````python
        note = measure["staves"][0]["notes"][0]
        # The title, the composer and the part name are read as valid UTF-8, U+FFFD in place of
        # each invalid byte; the short name, cut from the part name byte by byte, is not.
        self.assertEqual("Caf\ufffd", dump["title"])
        self.assertEqual("Caf\ufffd", dump["composer"])
        self.assertEqual("Caf\ufffd", part["name"])
        self.assertEqual(error, part["short_name"])
````

- [ ] **Step 3: Run them and see them fail.** C++ subset `ImportReport*:ScoreImportReport*:ScoreQuarterTone*` → the build fails: `fatal error: 'import-report.h' file not found` (import-report-test.cpp) and `no member named 'getImportIssues' in 'Score'` (score-test.cpp). «pytest» `test_score_comprehensive.ScoreQuarterToneReadTestCase test_score_comprehensive.ScoreImportReportTestCase test_musicxml_dump` → errors `AttributeError: ... object has no attribute 'getImportIssues'`; `test_an_alter_that_is_not_utf8_does_not_end_the_interpreter` fails with `3221226505 != 0`; `test_names_that_are_not_utf8_are_read_with_replacement_characters` with `UnicodeDecodeError`; the dump tests with the `[WARN] Unrecognized <accidental> name` line where the summary line is expected and `{'error': 'UnicodeDecodeError'} != 'Caf\ufffd'`.

- [ ] **Step 4: The report unit, `Score` and its records.** Apply, in order:

**Create `maiacore/include/maiacore/import-issue.h`**

````cpp
#pragma once

#include <string>
#include <tuple>

/**
 * @brief One record of the import report of a score loaded from a MusicXML file: a value the
 *        reader corrected, or an element it dropped.
 * @details Score::getImportIssues() returns the records of the load, in the order the reader made
 *          them. A record never changes after the load: edits and exports of the score leave the
 *          report as it is.
 */
struct ImportIssue {
    /// Stable identifier in UPPER_SNAKE case, such as "ALTER_OFF_GRID"; a code is never renamed.
    std::string code;
    /// "corrected" when a value read was replaced or filled, "dropped" when an element was
    /// removed.
    std::string kind;
    int partIndex = -1;    ///< 0-based index of the part; -1 when the record is about no one part.
    std::string partName;  ///< The part's name in the score; empty when partIndex is -1.
    /// The measure's number attribute as the file writes it; empty when the record is about no
    /// one measure.
    std::string measureNumber;
    int measureIndex = -1;  ///< 0-based index of the measure; -1 when measureNumber is empty.
    /// The element's path: from its measure for an element inside a measure
    /// ("note/pitch/alter"), from the score's root element otherwise
    /// ("part-list/score-part/part-name").
    std::string element;
    /// The value read; empty when the element is absent. For a "dropped" element of the closed
    /// element list, the number of such elements in the file.
    std::string found;
    /// The value stored in the score; empty when nothing of the element is stored.
    std::string used;
    std::string message;  ///< What happened, in English.

    /**
     * @brief Field-by-field equality.
     */
    bool operator==(const ImportIssue& other) const {
        return std::tie(code, kind, partIndex, partName, measureNumber, measureIndex, element,
                        found, used, message) == std::tie(other.code, other.kind, other.partIndex,
                                                          other.partName, other.measureNumber,
                                                          other.measureIndex, other.element,
                                                          other.found, other.used, other.message);
    }
};
````


**Create `maiacore/src/maiacore/import-report.h`**

````cpp
#pragma once

#include <string>
#include <vector>

#include "maiacore/import-issue.h"

namespace maiacore::detail {

// 'text' as valid UTF-8 (RFC 3629): every byte that does not belong to a well-formed sequence --
// a stray continuation byte, a truncated or overlong sequence, a surrogate, a code point above
// U+10FFFF -- is replaced by U+FFFD. Text taken from a file passes through it before it reaches
// a record, a message or the console: Python decodes all three as UTF-8.
std::string validUtf8(const std::string& text);

// Where a record of the import report is: a part, and a measure of it, or neither.
struct IssueLocation {
    int partIndex = -1;
    std::string partName;
    std::string measureNumber;  // as the file writes it
    int measureIndex = -1;
};

// A code of the import report and the kind of every record that carries it.
struct IssueCode {
    const char* code;
    const char* kind;  // "corrected" or "dropped"
};

// Every code the reader records, sorted by code.
const std::vector<IssueCode>& issueCatalogue();

// A record with the catalogue's kind for 'code'; every text is passed through validUtf8().
// Throws std::logic_error for a code that is not in the catalogue.
ImportIssue makeIssue(const std::string& code, const IssueLocation& location,
                      const std::string& element, const std::string& found, const std::string& used,
                      const std::string& message);

// The line printed after a load whose report is not empty, without its line break:
// "[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on export); see
// Score.getImportIssues()", where n counts the "corrected" records and m the distinct elements of
// the "dropped" ones.
std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues);

}  // namespace maiacore::detail
````


**Create `maiacore/src/maiacore/import-report.cpp`**

````cpp
#include "import-report.h"

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace maiacore::detail {

std::string validUtf8(const std::string& text) {
    static const char kReplacement[] = "\xEF\xBF\xBD";  // U+FFFD
    std::string valid;
    valid.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        // The length of the sequence 'lead' starts, and the range its second byte must be in.
        size_t length = 0;
        unsigned char low = 0x80;
        unsigned char high = 0xBF;
        if (lead < 0x80) {
            length = 1;
        } else if (lead >= 0xC2 && lead <= 0xDF) {
            length = 2;
        } else if (lead == 0xE0) {
            length = 3;
            low = 0xA0;  // no overlong form
        } else if (lead == 0xED) {
            length = 3;
            high = 0x9F;  // no surrogate
        } else if (lead >= 0xE1 && lead <= 0xEF) {
            length = 3;
        } else if (lead == 0xF0) {
            length = 4;
            low = 0x90;  // no overlong form
        } else if (lead >= 0xF1 && lead <= 0xF3) {
            length = 4;
        } else if (lead == 0xF4) {
            length = 4;
            high = 0x8F;  // nothing above U+10FFFF
        }
        bool wellFormed = length > 0 && i + length <= text.size();
        for (size_t k = 1; wellFormed && k < length; k++) {
            const unsigned char byte = static_cast<unsigned char>(text[i + k]);
            wellFormed = (k == 1) ? (byte >= low && byte <= high) : (byte >= 0x80 && byte <= 0xBF);
        }
        if (wellFormed) {
            valid.append(text, i, length);
            i += length;
        } else {
            valid += kReplacement;
            i++;
        }
    }
    return valid;
}

const std::vector<IssueCode>& issueCatalogue() {
    static const std::vector<IssueCode> catalogue = {
        {"ACCIDENTAL_ALTER_MISMATCH", "corrected"},
        {"ACCIDENTAL_NAME_UNKNOWN", "corrected"},
        {"ALTER_OFF_GRID", "corrected"},
    };
    return catalogue;
}

ImportIssue makeIssue(const std::string& code, const IssueLocation& location,
                      const std::string& element, const std::string& found, const std::string& used,
                      const std::string& message) {
    for (const IssueCode& entry : issueCatalogue()) {
        if (code == entry.code) {
            ImportIssue issue;
            issue.code = code;
            issue.kind = entry.kind;
            issue.partIndex = location.partIndex;
            issue.partName = validUtf8(location.partName);
            issue.measureNumber = validUtf8(location.measureNumber);
            issue.measureIndex = location.measureIndex;
            issue.element = validUtf8(element);
            issue.found = validUtf8(found);
            issue.used = validUtf8(used);
            issue.message = validUtf8(message);
            return issue;
        }
    }
    throw std::logic_error("makeIssue: '" + code + "' is not in the import-report catalogue");
}

std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues) {
    int corrections = 0;
    std::set<std::string> dropped;
    for (const ImportIssue& issue : issues) {
        if (issue.kind == "corrected") {
            corrections++;
        } else {
            dropped.insert(issue.element);
        }
    }
    return "[maiacore] " + validUtf8(fileName) + ": " + std::to_string(corrections) +
           " corrections, " + std::to_string(dropped.size()) +
           " element types not modelled (dropped on export); see Score.getImportIssues()";
}

}  // namespace maiacore::detail
````


**`maiacore/include/maiacore/score.h` ~12: replace**

````cpp
#include "maiacore/constants.h"
#include "maiacore/key.h"
````

with

````cpp
#include "maiacore/constants.h"
#include "maiacore/import-issue.h"
#include "maiacore/key.h"
````


**`maiacore/include/maiacore/score.h` ~46: replace**

````cpp
    bool _haveAnacrusisMeasure;       ///< True if the score contains an anacrusis (pickup) measure.

````

with

````cpp
    bool _haveAnacrusisMeasure;       ///< True if the score contains an anacrusis (pickup) measure.
    std::vector<ImportIssue> _importIssues;  ///< The import report of the loaded file.

````


**`maiacore/include/maiacore/score.h` ~105: replace**

````cpp
     *          alters this library can spell (a multiple of 0.5 from -2 to 2). An `<accidental>`
     *          name this library cannot spell warns and falls back to `<alter>`; an `<alter>` it
     *          cannot spell (3, the eighth tone 0.25, or 0.46, near a quarter tone but not one)
     *          warns and leaves the note natural, never rounded to the nearest pitch. When a
     *          recognised `<accidental>` and the `<alter>` disagree, the `<accidental>` is used
     *          and a warning is printed. None of these aborts the load.
     *
````

with

````cpp
     *          alters this library can spell (a multiple of 0.5 from -2 to 2). An `<accidental>`
     *          name this library cannot spell falls back to `<alter>` (record
     *          ACCIDENTAL_NAME_UNKNOWN); an `<alter>` it cannot spell (3, the eighth tone 0.25, or
     *          0.46, near a quarter tone but not one) leaves the note natural, never rounded to
     *          the nearest pitch (ALTER_OFF_GRID). When a recognised `<accidental>` and the
     *          `<alter>` disagree, the `<accidental>` is used (ACCIDENTAL_ALTER_MISMATCH). None of
     *          these aborts the load.
     *
     *          Every value the reader corrects is a record of the import report
     *          (getImportIssues()), with its part, its measure, the element, the value found and
     *          the value used. When the report is not empty, the load prints one line:
     *          "[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on
     *          export); see Score.getImportIssues()". Text taken from the file reaches a record,
     *          a message or the console as valid UTF-8: a byte that is not is replaced by U+FFFD.
     *
````


**`maiacore/include/maiacore/score.h` ~345: replace**

````cpp
    /**
     * @brief Returns true if the MusicXML file contains <type> tags for notes.
````

with

````cpp
    /**
     * @brief Returns the import report: what the reader corrected or dropped while loading the
     *        file, in the order it did.
     * @details The report describes the load. It is copied with the score, emptied by clear(),
     *          and left unchanged by edits and exports; a score built through the API has an empty
     *          report.
     * @return The records, one ImportIssue each.
     */
    const std::vector<ImportIssue>& getImportIssues() const;

    /**
     * @brief Returns true if the MusicXML file contains <type> tags for notes.
````


**`maiacore/include/maiacore/score.h` ~477: replace**

````cpp
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
        _stackedChords = other._stackedChords;
        _haveAnacrusisMeasure = other._haveAnacrusisMeasure;

        // Deep copy of XML document
        _doc.reset(other._doc);
    }
````

with

````cpp
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
        _stackedChords = other._stackedChords;
        _haveAnacrusisMeasure = other._haveAnacrusisMeasure;
        _importIssues = other._importIssues;

        // Deep copy of XML document
        _doc.reset(other._doc);
    }
````


**`maiacore/include/maiacore/score.h` ~503: replace**

````cpp
        _haveTypeTag = other._haveTypeTag;
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
        _stackedChords = other._stackedChords;
        _haveAnacrusisMeasure = other._haveAnacrusisMeasure;

        // Deep copy of XML document
        _doc.reset(other._doc);

        return *this;
````

with

````cpp
        _haveTypeTag = other._haveTypeTag;
        _isLoadedXML = other._isLoadedXML;
        _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
        _stackedChords = other._stackedChords;
        _haveAnacrusisMeasure = other._haveAnacrusisMeasure;
        _importIssues = other._importIssues;

        // Deep copy of XML document
        _doc.reset(other._doc);

        return *this;
````


**`maiacore/src/maiacore/score.cpp` ~24: replace**

````cpp
// #include "cherno/instrumentor.h"
#include "maiacore/clef.h"
````

with

````cpp
// #include "cherno/instrumentor.h"
#include "import-report.h"
#include "maiacore/clef.h"
````


**`maiacore/src/maiacore/score.cpp` ~34: replace**

````cpp
using maiacore::detail::concertPitch;
using maiacore::detail::MelodicLine;
````

with

````cpp
using maiacore::detail::concertPitch;
using maiacore::detail::IssueLocation;
using maiacore::detail::makeIssue;
using maiacore::detail::MelodicLine;
````


**`maiacore/src/maiacore/score.cpp` ~81: replace**

````cpp
    std::string pairCorrected;  // the warning for a <diatonic> replaced, or empty
};
````

with

````cpp
    std::string pairCorrected;  // the warning for a <diatonic> replaced, or empty
};

// A correction of a note's accidental, recorded once the note's pitch -- the value used -- is
// known.
struct AccidentalCorrection {
    const char* code;
    const char* element;
    std::string found;
    std::string message;
};
````


**`maiacore/src/maiacore/score.cpp` ~603: replace**

````cpp
    _lcmDivisionsPerQuarterNote = 0;
}
````

with

````cpp
    _lcmDivisionsPerQuarterNote = 0;
    _importIssues.clear();
}
````


**`maiacore/src/maiacore/score.cpp` ~636: replace**

````cpp
std::string Score::getFileName() const { return _fileName; }

````

with

````cpp
std::string Score::getFileName() const { return _fileName; }

const std::vector<ImportIssue>& Score::getImportIssues() const { return _importIssues; }

````


**`maiacore/src/maiacore/score.cpp` ~750: replace**

````cpp

        partsNameVec.push_back(rawPartName);
    }
````

with

````cpp

        // Held as valid UTF-8, which every getter of the name returns to Python.
        partsNameVec.push_back(maiacore::detail::validUtf8(rawPartName));
    }
````


**`maiacore/src/maiacore/score.cpp` ~844: replace**

````cpp

    setTitle(workTitle);
    setComposerName(composerName);

````

with

````cpp

    setTitle(maiacore::detail::validUtf8(workTitle));
    setComposerName(maiacore::detail::validUtf8(composerName));

````


**`maiacore/src/maiacore/score.cpp` ~1143: replace**

````cpp
                    std::string alterSymbol;
                    if (!isUnpitched) {
````

with

````cpp
                    std::string alterSymbol;
                    std::vector<AccidentalCorrection> accidentalCorrections;
                    if (!isUnpitched) {
````


**`maiacore/src/maiacore/score.cpp` ~1157: replace**

````cpp
                            // throw on an unrecognised one, but that contract must not abort a
                            // whole score load here: warn and fall through to <alter>, then
                            // natural, exactly as if no <accidental> had been present.
                            bool accidentalRecognised = false;
                            if (!accidentalTag.empty()) {
                                try {
                                    alterSymbol = Helper::alterName2symbol(accidentalTag);
                                    accidentalRecognised = true;
                                } catch (const std::runtime_error& e) {
                                    LOG_WARN("Unrecognized <accidental> name '"
                                             << accidentalTag
                                             << "'; falling back to <alter> or natural. "
                                             << e.what());
                                }
                            }

                            // Only the nine alters this library can spell (-2 to 2 in steps of
                            // 0.5) are read from <alter>. Anything else -- a triple accidental
                            // such as 3, a microtone such as the eighth tone 0.25, a value near
                            // the grid such as 0.46, or text that is not a number -- is a real
                            // limitation of that vocabulary, not something to solve here, and
                            // must not abort the load: the note is read as natural, with a
                            // warning. Never invent a spelling for it, and never round to the
                            // nearest representable pitch -- a silent wrong pitch is worse than
                            // a loud dropped accidental in a library used for musical analysis.
                            const std::optional<float> alterValue =
                                alterTag.empty() ? std::nullopt : spellableAlterValue(alterTag);

                            if (!accidentalRecognised && !alterTag.empty()) {
                                if (alterValue.has_value()) {
                                    alterSymbol = Helper::alterValue2symbol(alterValue.value());
                                } else {
                                    LOG_WARN("Unrepresentable <alter> value '"
                                             << alterTag
                                             << "': this library's accidental vocabulary "
                                             << "spells only multiples of 0.5 from -2 to 2, so "
                                             << "the note was read as natural (its pitch is off "
                                             << "by that amount).");
                                }
                            }

                            // An <accidental> that is recognised wins over a disagreeing <alter>,
                            // but the disagreement is reported: arrow glyphs, for one, also mark
                            // microtones other than the quarter tone.
                            if (accidentalRecognised && !alterTag.empty() &&
                                (!alterValue.has_value() ||
                                 alterValue.value() != Helper::alterSymbol2Value(alterSymbol))) {
                                LOG_WARN("The <accidental> '"
                                         << accidentalTag << "' and the <alter> '" << alterTag
                                         << "' of this note disagree; the <accidental> is used.");
                            }
                        }
                    }

                    octave = (!isUnpitched)
                                 ? atoi(node.child("pitch").child_value("octave"))
                                 : atoi(node.child("unpitched").child_value("display-octave"));
                    pitch = step + alterSymbol + std::to_string(octave);
                }
````

with

````cpp
                            // throw on an unrecognised one, but that contract must not abort a
                            // whole score load here: record it and fall through to <alter>, then
                            // natural, exactly as if no <accidental> had been present.
                            bool accidentalRecognised = false;
                            if (!accidentalTag.empty()) {
                                try {
                                    alterSymbol = Helper::alterName2symbol(accidentalTag);
                                    accidentalRecognised = true;
                                } catch (const std::runtime_error&) {
                                    accidentalCorrections.push_back(
                                        {"ACCIDENTAL_NAME_UNKNOWN", "note/accidental",
                                         accidentalTag,
                                         "The <accidental> name '" + accidentalTag +
                                             "' is not one this library can spell; the note is "
                                             "read from its <alter>, or as natural without "
                                             "one."});
                                }
                            }

                            // Only the nine alters this library can spell (-2 to 2 in steps of
                            // 0.5) are read from <alter>. Anything else -- a triple accidental
                            // such as 3, a microtone such as the eighth tone 0.25, a value near
                            // the grid such as 0.46, or text that is not a number -- is a real
                            // limitation of that vocabulary, not something to solve here, and
                            // must not abort the load: the note is read as natural, and the
                            // correction is recorded. Never invent a spelling for it, and never
                            // round to the nearest representable pitch -- a silent wrong pitch
                            // is worse than a recorded dropped accidental in a library used for
                            // musical analysis.
                            const std::optional<float> alterValue =
                                alterTag.empty() ? std::nullopt : spellableAlterValue(alterTag);

                            if (!accidentalRecognised && !alterTag.empty()) {
                                if (alterValue.has_value()) {
                                    alterSymbol = Helper::alterValue2symbol(alterValue.value());
                                } else {
                                    accidentalCorrections.push_back(
                                        {"ALTER_OFF_GRID", "note/pitch/alter", alterTag,
                                         "The <alter> value '" + alterTag +
                                             "' is not one this library can spell (a multiple "
                                             "of 0.5 from -2 to 2); the note is read as "
                                             "natural, its pitch off by that amount."});
                                }
                            }

                            // An <accidental> that is recognised wins over a disagreeing <alter>,
                            // but the disagreement is recorded: arrow glyphs, for one, also mark
                            // microtones other than the quarter tone.
                            if (accidentalRecognised && !alterTag.empty() &&
                                (!alterValue.has_value() ||
                                 alterValue.value() != Helper::alterSymbol2Value(alterSymbol))) {
                                accidentalCorrections.push_back(
                                    {"ACCIDENTAL_ALTER_MISMATCH", "note/pitch/alter", alterTag,
                                     "The <accidental> '" + accidentalTag + "' and the <alter> '" +
                                         alterTag +
                                         "' of this note disagree; the <accidental> is used."});
                            }
                        }
                    }

                    octave = (!isUnpitched)
                                 ? atoi(node.child("pitch").child_value("octave"))
                                 : atoi(node.child("unpitched").child_value("display-octave"));
                    pitch = step + alterSymbol + std::to_string(octave);
                    for (const AccidentalCorrection& correction : accidentalCorrections) {
                        _importIssues.push_back(makeIssue(
                            correction.code, {p, _part[p].getName(), measureNumbers[m], m},
                            correction.element, correction.found, pitch, correction.message));
                    }
                }
````


**`maiacore/src/maiacore/score.cpp` ~1309: replace**

````cpp
        applyTranspositions(_part[p], transposeElements, pitchedNotes, measureNumbers);
    }
````

with

````cpp
        applyTranspositions(_part[p], transposeElements, pitchedNotes, measureNumbers);
    }

    if (!_importIssues.empty()) {
        std::cout << maiacore::detail::importSummary(_fileName, _importIssues) << std::endl;
    }
````

- [ ] **Step 5: The binding.** In `maiacore/src/maiacore/python_wrapper/py_score.cpp` apply, in order:

**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~2: replace**

````cpp
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/measure.h"
````

with

````cpp
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <utility>
#include <vector>

#include "maiacore/import-issue.h"
#include "maiacore/measure.h"
````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~15: replace**

````cpp

void ScoreClass(const py::module& m) {
    m.doc() = "Score class binding";

````

with

````cpp

namespace {
// The import report as a DataFrame: one column per ImportIssue field, in field order, text
// columns `str` and index columns `int64`, so an empty report keeps every column and dtype.
py::object importIssuesDataFrame(const std::vector<ImportIssue>& issues) {
    const std::vector<std::pair<const char*, bool>> columns = {
        {"code", true},          {"kind", true},          {"partIndex", false}, {"partName", true},
        {"measureNumber", true}, {"measureIndex", false}, {"element", true},    {"found", true},
        {"used", true},          {"message", true}};
    std::vector<py::list> values(columns.size());
    for (const ImportIssue& issue : issues) {
        values[0].append(issue.code);
        values[1].append(issue.kind);
        values[2].append(issue.partIndex);
        values[3].append(issue.partName);
        values[4].append(issue.measureNumber);
        values[5].append(issue.measureIndex);
        values[6].append(issue.element);
        values[7].append(issue.found);
        values[8].append(issue.used);
        values[9].append(issue.message);
    }
    const py::module_ pandas = py::module_::import("pandas");
    const py::object text = py::module_::import("builtins").attr("str");
    py::dict data;
    for (size_t c = 0; c < columns.size(); c++) {
        const py::object dtype = columns[c].second ? text : py::str("int64");
        data[py::str(columns[c].first)] =
            pandas.attr("Series")(values[c], py::arg("dtype") = dtype);
    }
    return pandas.attr("DataFrame")(data);
}
}  // namespace

void ScoreClass(const py::module& m) {
    m.doc() = "Score class binding";

    py::class_<ImportIssue> issue(m, "ImportIssue", R"pbdoc(
        One record of a score's import report: a value the MusicXML reader corrected, or an
        element it dropped. ``Score.getImportIssues()`` returns them; the fields are read-only.

        Attributes
        ----------
        code : str
            Stable identifier in UPPER_SNAKE case, such as ``"ALTER_OFF_GRID"``; a code is never
            renamed.
        kind : str
            ``"corrected"`` when a value read was replaced or filled, ``"dropped"`` when an
            element was removed.
        partIndex : int
            0-based index of the part; -1 when the record is about no one part.
        partName : str
            The part's name in the score; empty when ``partIndex`` is -1.
        measureNumber : str
            The measure's ``number`` attribute as the file writes it; empty when the record is
            about no one measure.
        measureIndex : int
            0-based index of the measure; -1 when ``measureNumber`` is empty.
        element : str
            The element's path: from its measure inside a measure (``"note/pitch/alter"``), from
            the score's root element otherwise (``"part-list/score-part/part-name"``).
        found : str
            The value read; empty when the element is absent.
        used : str
            The value stored in the score; empty when nothing of the element is stored.
        message : str
            What happened, in English.
    )pbdoc");
    issue.def_readonly("code", &ImportIssue::code);
    issue.def_readonly("kind", &ImportIssue::kind);
    issue.def_readonly("partIndex", &ImportIssue::partIndex);
    issue.def_readonly("partName", &ImportIssue::partName);
    issue.def_readonly("measureNumber", &ImportIssue::measureNumber);
    issue.def_readonly("measureIndex", &ImportIssue::measureIndex);
    issue.def_readonly("element", &ImportIssue::element);
    issue.def_readonly("found", &ImportIssue::found);
    issue.def_readonly("used", &ImportIssue::used);
    issue.def_readonly("message", &ImportIssue::message);
    issue.def(py::self == py::self);
    issue.def("__repr__", [](const ImportIssue& record) {
        return "<ImportIssue " + record.code + " " + record.element + ": " + record.message + ">";
    });

````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~35: replace**

````cpp
        An ``<accidental>`` name this library cannot spell (see ``Helper.alterName2symbol``)
        prints a warning and falls back to ``<alter>``; an ``<alter>`` value it cannot spell
        (e.g. 3, the eighth tone 0.25, or 0.46, near a quarter tone but not one) prints a
        warning and leaves the note natural, never rounded to the nearest pitch. When a
        recognised ``<accidental>`` and the ``<alter>`` disagree, the ``<accidental>`` is used
        and a warning is printed. None of these aborts the load.

````

with

````cpp
        An ``<accidental>`` name this library cannot spell (see ``Helper.alterName2symbol``)
        falls back to ``<alter>`` (record ``ACCIDENTAL_NAME_UNKNOWN``); an ``<alter>`` value it
        cannot spell (e.g. 3, the eighth tone 0.25, or 0.46, near a quarter tone but not one)
        leaves the note natural, never rounded to the nearest pitch (``ALTER_OFF_GRID``). When a
        recognised ``<accidental>`` and the ``<alter>`` disagree, the ``<accidental>`` is used
        (``ACCIDENTAL_ALTER_MISMATCH``). None of these aborts the load.

        Every value the reader corrects is a record of the import report
        (``getImportIssues()``), with its part, its measure, the element, the value found and the
        value used. When the report is not empty, the load prints one line, ``[maiacore] <file>:
        <n> corrections, <m> element types not modelled (dropped on export); see
        Score.getImportIssues()``. Text taken from the file reaches a record, a message or the
        console as valid UTF-8: a byte that is not is replaced by U+FFFD.

````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~106: replace**

````cpp
    cls.def("getFileName", &Score::getFileName);
    cls.def("setTitle", &Score::setTitle, py::arg("scoreTitle"));
````

with

````cpp
    cls.def("getFileName", &Score::getFileName);

    cls.def("getImportIssues", &Score::getImportIssues, R"pbdoc(
        Return the import report: what the MusicXML reader corrected or dropped while loading the
        file, in the order it did.

        The report describes the load: it is copied with the score, emptied by ``clear()``, and
        left unchanged by edits and exports. A score built through the API has an empty report.

        Returns
        -------
        list of ImportIssue
            The records.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> [issue.code for issue in score.getImportIssues() if issue.kind == "corrected"]
        []
    )pbdoc");

    cls.def(
        "getImportIssuesDataFrame",
        [](const Score& score) { return importIssuesDataFrame(score.getImportIssues()); },
        R"pbdoc(
        Return the import report (``getImportIssues()``) as a pandas DataFrame.

        Returns
        -------
        pandas.DataFrame
            One row per record and one column per field, in this order: ``code``, ``kind``,
            ``partIndex``, ``partName``, ``measureNumber``, ``measureIndex``, ``element``,
            ``found``, ``used``, ``message``. The text columns have the ``str`` dtype and the
            two index columns ``int64``; an empty report gives an empty DataFrame with every
            column.
    )pbdoc");
    cls.def("setTitle", &Score::setTitle, py::arg("scoreTitle"));
````

- [ ] **Step 6: Fixture comments and the dump tool's docstring** say record, not warning (no ledger line changes: comments are not records):

**`test/musicxml/dump_score.py` ~4: replace**

````python
ASCII with LF line endings on every platform. What maialib prints while it loads the score and
the dump reads it, such as its warnings, goes to stderr, so that stdout holds only the dump.
Every value comes from maialib's public API through ``_safe``: a getter that raises, such as a
````

with

````python
ASCII with LF line endings on every platform. What maialib prints while it loads the score and
the dump reads it, such as the summary line of its import report, goes to stderr, so that
stdout holds only the dump.
Every value comes from maialib's public API through ``_safe``: a getter that raises, such as a
````


**`test/xml_examples/unit_test/accidental_sharp_sharp.xml` ~26: replace**

````xml
           sharp signs, rather than the "x" glyph of "double-sharp". It is a recognised name, so
           it must read as a double sharp with no warning. Expected: Cx4. -->
      <note>
````

with

````xml
           sharp signs, rather than the "x" glyph of "double-sharp". It is a recognised name, so
           it must read as a double sharp with no record. Expected: Cx4. -->
      <note>
````


**`test/xml_examples/unit_test/quarter_tone_accidental_alter_disagree.xml` ~26: replace**

````xml
           <accidental> wins, as it does whenever it is recognised, and the disagreement is
           reported with a warning. Expected: C1x4. -->
      <note>
````

with

````xml
           <accidental> wins, as it does whenever it is recognised, and the disagreement is
           recorded (ACCIDENTAL_ALTER_MISMATCH). Expected: C1x4. -->
      <note>
````


**`test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml` ~27: replace**

````xml
           Helper::alterValue2symbol() can spell. Same expectation as the triple-sharp
           fixture: load as natural, with a warning naming the value, not a throw. -->
      <note>
````

with

````xml
           Helper::alterValue2symbol() can spell. Same expectation as the triple-sharp
           fixture: load as natural, with a record naming the value, not a throw. -->
      <note>
````


**`test/xml_examples/unit_test/unrepresentable_alter_near_quarter_tone.xml` ~26: replace**

````xml
           The reader never rounds to the nearest representable pitch, so this must load as
           natural, with a warning naming the value. Expected: C4. -->
      <note>
````

with

````xml
           The reader never rounds to the nearest representable pitch, so this must load as
           natural, with a record naming the value. Expected: C4. -->
      <note>
````


**`test/xml_examples/unit_test/unrepresentable_alter_triple_sharp.xml` ~26: replace**

````xml
           nine values Helper::alterValue2symbol() can spell (-2..2 in 0.5 steps): a
           limit of the accidental vocabulary. It must load as natural, with a warning,
           rather than aborting. -->
````

with

````xml
           nine values Helper::alterValue2symbol() can spell (-2..2 in 0.5 steps): a
           limit of the accidental vocabulary. It must load as natural, with a record,
           rather than aborting. -->
````

- [ ] **Step 7: Format, build, pass.** clang-format `import-issue.h`, `import-report.h`, `import-report.cpp`, `score.h`, `score.cpp`, `py_score.cpp`, `import-report-test.cpp`, `test-files.h`, `score-test.cpp`. C++ subset `ImportReport*:ScoreImportReport*:ScoreQuarterTone*:ScoreCopy*` → all pass. «build» `make "PYTHON=$py" dev` → 0. «pytest» `test_score_comprehensive.ScoreQuarterToneReadTestCase test_score_comprehensive.ScoreImportReportTestCase test_musicxml_dump` → OK. Task 0's crash script → `exit 0`, its stdout `[maiacore] latin1.xml: 1 corrections, 0 element types not modelled (dropped on export); see Score.getImportIssues()` and `loaded`.

- [ ] **Step 8: Mutations.** (a) In `validUtf8` replace `valid += kReplacement;` with `valid += text[i];` → `ImportReportUtf8.EachInvalidByteBecomesAReplacementCharacter`, `ImportReportRecord.ARecordTakesItsKindAndSanitisesItsText`, `ScoreQuarterToneRead.AnAlterThatIsNotUtf8IsRecordedAsValidUtf8`, `ScoreImportReport.NamesThatAreNotUtf8AreHeldAsValidUtf8` fail; after `make dev`, `test_an_alter_that_is_not_utf8_does_not_end_the_interpreter` (`1 != 0`: the summary line quotes only `latin1.xml`, so nothing prints the raw byte, and the child exits 1 with `UnicodeDecodeError` reading `i.found`) and `test_names_that_are_not_utf8_are_read_with_replacement_characters` (`UnicodeDecodeError`) fail. (b) In the copy constructor of `score.h` delete `_importIssues = other._importIssues;` → `TheReportIsCopiedKeptThroughEditsAndEmptiedByClear` and `test_a_copy_keeps_the_report_and_clear_empties_it` (the collection's copy has an empty report) fail. (c) In `Score::clear()` delete `_importIssues.clear();` → the same two fail at the `clear()` assertion. (d) At the end of `loadXMLFile` replace `if (!_importIssues.empty()) {` with `if (true) {` → `ALoadPrintsOneSummaryLineOnlyWhenTheReportIsNotEmpty` and `AlterTextMustBeANumberAndNothingElse` (a valid text prints a line) fail. (e) In `importSummary` replace `if (issue.kind == "corrected") {` with `if (true) {` → `ImportReportSummary.TheSummaryCountsCorrectionsAndDroppedElementTypes` fails. (f) Delete the `accidentalCorrections.push_back(` of `ALTER_OFF_GRID` (its whole statement) → `AlterTextMustBeANumberAndNothingElse`, `AlterNearAQuarterToneIsNotSnappedOntoIt`, `test_alter_near_a_quarter_tone_is_not_snapped_onto_it` fail. (g) In the part-name loop replace `partsNameVec.push_back(maiacore::detail::validUtf8(rawPartName));` with `partsNameVec.push_back(rawPartName);` → `NamesThatAreNotUtf8AreHeldAsValidUtf8` fails. (h) In `makeIssue` replace `issue.element = validUtf8(element);` with `issue.element = element;` → `ARecordTakesItsKindAndSanitisesItsText` fails. (i) In `validUtf8` replace `} else if (lead >= 0xC2 && lead <= 0xDF) {` with `} else if (lead >= 0xC4 && lead <= 0xDF) {` → `ImportReportUtf8.ValidTextIsUnchanged` fails (`can\xC3\xA7\xC3\xA3o` gains U+FFFD). (j) In `issueCatalogue()` swap the first two entries (`ACCIDENTAL_NAME_UNKNOWN` before `ACCIDENTAL_ALTER_MISMATCH`) → `ImportReportCatalogue.CodesAreUpperSnakeSortedAndUnique` fails (`EXPECT_LT`). (k) In `importIssuesDataFrame` replace `py::str("int64")` with `text` → after `make dev`, `test_the_dataframe_has_one_row_per_record_and_fixed_dtypes` fails (`partIndex` is not `int64`). (l) Delete `issue.def(py::self == py::self);` → after `make dev`, `test_records_compare_by_value` fails (two records compare by identity). (m) Delete the `accidentalCorrections.push_back(` of `ACCIDENTAL_ALTER_MISMATCH` (its whole statement) → `ScoreQuarterToneRead.DisagreeingAccidentalWinsWithARecord` and, after `make dev`, `test_disagreeing_accidental_wins_with_a_record` fail. (n) Delete the `accidentalCorrections.push_back(` of `ACCIDENTAL_NAME_UNKNOWN` (its whole statement) → `ScoreQuarterToneRead.UnrecognisedAccidentalNameFallsBackToAlterInsteadOfAborting` and, after `make dev`, `test_unrecognised_accidental_name_falls_back_instead_of_raising` fail. Revert each; rerun green. `ScoreImportReport.AScoreBuiltThroughTheApiHasAnEmptyReport` and `test_a_score_built_through_the_api_has_an_empty_report` pin invariants that no mutation of the reader can fail (a score built through the API starts with an empty report; an empty DataFrame has every column, with the dtypes of a loaded one): record them in the task report as invariants, not as mutation-proven tests.

- [ ] **Step 9: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1198 tests.` (1188 + 10). «build» `make "PYTHON=$py" py-tests` → OK, `Ran 696 tests` (690 + 6), `OK (skipped=1)`; the corpus test passes with the ledger unchanged. «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 10: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/include/maiacore/import-issue.h maiacore/src/maiacore/import-report.h maiacore/src/maiacore/import-report.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/score.cpp maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/CMakeLists.txt tests-cpp/src/import-report-test.cpp tests-cpp/src/test-files.h tests-cpp/src/score-test.cpp test/test_score_comprehensive.py test/test_musicxml_dump.py test/musicxml/dump_score.py test/xml_examples/unit_test/accidental_sharp_sharp.xml test/xml_examples/unit_test/quarter_tone_accidental_alter_disagree.xml test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml test/xml_examples/unit_test/unrepresentable_alter_near_quarter_tone.xml test/xml_examples/unit_test/unrepresentable_alter_triple_sharp.xml`, message:

```
feat: the import report, its accidental records and valid UTF-8 text

Score.getImportIssues() and Score.getImportIssuesDataFrame() return what
the MusicXML reader corrected while loading: one ImportIssue (C++ struct,
Python class) per correction, with its code, kind, part, measure as
written and as an index, element path, the value found and the value
used. The report is copied with the score, emptied by clear() and left
unchanged by edits and exports; a score built through the API has none.

The three accidental warnings become the records ACCIDENTAL_NAME_UNKNOWN,
ALTER_OFF_GRID and ACCIDENTAL_ALTER_MISMATCH, which name the part and the
measure the warnings did not name. Instead of a [WARN] line per note, a
load whose report is not empty prints one summary line.

Text taken from the file reaches a record, a message or the console as
valid UTF-8, each invalid byte replaced by U+FFFD: an <alter> holding a
Latin-1 byte ended the process with 0xC0000409, from the destructor of
pybind11's redirected stream. The title, the composer and the part names
are held as valid UTF-8 too, where their getters raised
UnicodeDecodeError.

The fixtures' comments and the dump tool's docstring say record instead
of warning; the dump test expects the summary line on stderr and the
names with U+FFFD.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 2: `<transpose>` records and the silent corrections

**Files:**
- Modify: `maiacore/src/maiacore/import-report.cpp` (catalogue ~62), `maiacore/src/maiacore/score.cpp` (`TransposeElement` ~79; `ignoredTranspose` before `trimmed` ~103; `readTranspose` ~141-215; `readTransposeElements` ~228; `applyTranspositions` ~270-380; `isPositiveWholeNumber` before `toIntRange`; duplicate part names ~781-801; divisions ~927; the measure's location ~1070; tuplet values ~1120; voice and staff ~1238; the call of `applyTranspositions` ~1336), `maiacore/include/maiacore/score.h` (constructor Doxygen ~132), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (`Score(filePath)` numpydoc ~144), `maiacore/include/maiacore/part.h` (~200), `maiacore/src/maiacore/python_wrapper/py_part.cpp` (~76, ~187)
- Test: `tests-cpp/src/score-test.cpp` (the `<transpose>` tests ~1648-1873 assert records; new `ScoreSilentCorrections` before `// The concert key of getChords()`), `test/test_musicxml_transpose.py` (`load` ~64, `reloaded` ~138, the zero-diatonic test ~412 and a new test after it)

**Interfaces:**
- Consumes: `makeIssue`, `IssueLocation`, `issueCatalogue`, `ImportIssue`, `correctionCodes`, `TemporaryFile`, `minimalScore`, `kMinimalAttributes`, `kWholeC4` (Task 1).
- Produces: catalogue entries `DIVISIONS_MISSING`, `FOR_PART_NOT_MODELLED` (dropped), `PART_NAME_DUPLICATE`, `STAFF_CLAMPED`, `TRANSPOSE_CHROMATIC_NOT_INTEGER`, `TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER`, `TRANSPOSE_OUT_OF_RANGE`, `TRANSPOSE_PAIR_CORRECTED`, `TUPLET_CLAMPED`, `VOICE_NOT_POSITIVE` (all `corrected` but `FOR_PART_NOT_MODELLED`); in `score.cpp`: `TransposeElement` with `ignoredCode`, `ignoredText`, `chromaticText`, `pairCorrected`, `diatonicText`, `conventionalDiatonic` instead of `where`, `ignored`, `pairCorrected` strings; `TransposeElement readTranspose(const pugi::xml_node&, int measureIdx, int notesBefore)`; `void readTransposeElements(const pugi::xml_node& measure, const IssueLocation& location, std::vector<TransposeElement>& elements, std::vector<ImportIssue>& issues)`; `void applyTranspositions(Part& part, int partIndex, const std::vector<TransposeElement>&, const std::vector<PitchedNote>&, const std::vector<std::string>& measureNumbers, std::vector<ImportIssue>& issues)`; `ImportIssue ignoredTranspose(const TransposeElement&, const IssueLocation&)`; `bool isPositiveWholeNumber(const std::string& text)`; the local `measureLocation` in the measure loop of `loadXMLFile`. Test helpers `transposeRecords(const Score&)`, `transposeIssue(...)`, `musicIssue(...)`, `quarterC4(extra)` in `score-test.cpp`; `reloaded(data)` in `test_musicxml_transpose.py` returns `(score, corrections)`.

- [ ] **Step 1: Write the failing C++ tests.** In `tests-cpp/src/score-test.cpp` apply, in order:

**`tests-cpp/src/score-test.cpp` ~1648: replace**

````cpp

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
````

with

````cpp

// The records of the <transpose> reader in a score's report, in report order.
std::vector<ImportIssue> transposeRecords(const Score& score) {
    std::vector<ImportIssue> records;
    for (const ImportIssue& issue : score.getImportIssues()) {
        if (issue.code.rfind("TRANSPOSE_", 0) == 0 || issue.code == "FOR_PART_NOT_MODELLED") {
            records.push_back(issue);
        }
    }
    return records;
}

// A record of the <transpose> reader; FOR_PART_NOT_MODELLED is "dropped", every other code
// "corrected".
ImportIssue transposeIssue(const std::string& code, const int partIndex,
                           const std::string& partName, const std::string& measureNumber,
                           const int measureIndex, const std::string& element,
                           const std::string& found, const std::string& used,
                           const std::string& message) {
    ImportIssue issue;
    issue.code = code;
    issue.kind = (code == "FOR_PART_NOT_MODELLED") ? "dropped" : "corrected";
    issue.partIndex = partIndex;
    issue.partName = partName;
    issue.measureNumber = measureNumber;
    issue.measureIndex = measureIndex;
    issue.element = element;
    issue.found = found;
    issue.used = used;
    issue.message = message;
    return issue;
}
}  // namespace

// A <transpose> applies from where it stands to the next one: a change in the middle of a part is
// followed and carried forward, and <diatonic>0</diatonic><chromatic>0</chromatic> returns to
// untransposed.
TEST(ScoreTransposeRead, aChangeInTheMiddleOfAPartIsFollowed) {
    Score score(kUnitTest + "transpose_change_mid_part.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-2, -3, NONE) A3",
                                        "D4 (-2, -3, NONE) B3", "C4 (0, 0, NONE) C4"}));
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}

// A part whose first <transpose> comes in a later measure is untransposed up to it; rests and
// unpitched notes take no transposition.
TEST(ScoreTransposeRead, aTransposeInALaterMeasureAppliesFromThere) {
    Score score(kUnitTest + "transpose_later_measure.musicxml");
````


**`tests-cpp/src/score-test.cpp` ~1687: replace**

````cpp
    EXPECT_FALSE(unpitched.isTransposed());
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}
````

with

````cpp
    EXPECT_FALSE(unpitched.isTransposed());
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}
````


**`tests-cpp/src/score-test.cpp` ~1737: replace**

````cpp
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
````

with

````cpp
TEST(ScoreTransposeRead, aTransposeWithoutDiatonicStoresTheConventionalInterval) {
    Score score(kUnitTest + "transpose_without_diatonic.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"F#4 (-1, -2, NONE) E4"}));
    EXPECT_EQ(correctionCodes(score), std::vector<std::string>{});
}

// A <chromatic> that is not a whole number is ignored with one record; the previous
// transposition stays in force.
TEST(ScoreTransposeRead, aChromaticThatIsNotAWholeNumberIsIgnored) {
    Score score(kUnitTest + "transpose_chromatic_not_integer.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3"}));
    EXPECT_EQ(transposeRecords(score),
              std::vector<ImportIssue>{transposeIssue(
                  "TRANSPOSE_CHROMATIC_NOT_INTEGER", 0, "Clarinet", "2", 1,
                  "attributes/transpose/chromatic", "-2.5", "",
                  "<chromatic>-2.5</chromatic> is not a whole number of semitones; the "
                  "<transpose> is ignored and the previous transposition stays in force.")});
}

// Likewise an <octave-change> that is not a whole number.
TEST(ScoreTransposeRead, anOctaveChangeThatIsNotAWholeNumberIsIgnored) {
    Score score(kUnitTest + "transpose_octave_change_not_integer.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3"}));
    EXPECT_EQ(transposeRecords(score),
              std::vector<ImportIssue>{transposeIssue(
                  "TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER", 0, "Clarinet", "2", 1,
                  "attributes/transpose/octave-change", "1.5", "",
                  "<octave-change>1.5</octave-change> is not a whole number of octaves; the "
                  "<transpose> is ignored and the previous transposition stays in force.")});
}
````


**`tests-cpp/src/score-test.cpp` ~1843: replace**

````cpp
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
````

with

````cpp
// A <diatonic> that does not match <chromatic> is replaced by the conventional diatonic interval,
// with one record per <transpose>; an explicit 0 with a non-zero chromatic interval does not
// match either, while a tritone matches the augmented fourth and the diminished fifth alike.
TEST(ScoreTransposeCorrection, aDiatonicIntervalThatDoesNotMatchIsReplaced) {
    Score score(kUnitTest + "transpose_pair_inconsistent.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"F#4 (2, 4, NONE) A#4"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3"}));
    EXPECT_EQ(transposedNotes(score, 2), (std::vector<std::string>{"C4 (-4, -6, NONE) F#3"}));
    EXPECT_EQ(transposedNotes(score, 3), (std::vector<std::string>{"C4 (-3, -6, NONE) Gb3"}));
    EXPECT_EQ(transposeRecords(score),
              (std::vector<ImportIssue>{
                  transposeIssue("TRANSPOSE_PAIR_CORRECTED", 0, "Trumpet in E", "1", 0,
                                 "attributes/transpose/diatonic", "3", "2",
                                 "<diatonic>3</diatonic> does not match <chromatic>4</chromatic>; "
                                 "using 2."),
                  transposeIssue("TRANSPOSE_PAIR_CORRECTED", 1, "Clarinet in Bb", "1", 0,
                                 "attributes/transpose/diatonic", "0", "-1",
                                 "<diatonic>0</diatonic> does not match <chromatic>-2</chromatic>; "
                                 "using -1.")}));
}

// The Dvorak sample's trumpets in E, written with the letters of a fourth and the semitones of a
// major third, are read as a major third: a written F#4 sounds A#4.
TEST(ScoreTransposeCorrection, theDvorakTrumpetsInESoundAMajorThirdUp) {
    Score score(kSamples + "Dvorak_Symphony_9_mov_4.mxl");
    EXPECT_EQ(describeTransposition(score.getPart(6).getMeasure(7).getNote(1, 0)),
              "F#4 (2, 4, NONE) A#4");
    EXPECT_EQ(transposeRecords(score),
              std::vector<ImportIssue>{transposeIssue(
                  "TRANSPOSE_PAIR_CORRECTED", 6, "Trombe I. II. E", "1", 0,
                  "attributes/transpose/diatonic", "3", "2",
                  "<diatonic>3</diatonic> does not match <chromatic>4</chromatic>; using 2.")});
}

// A <diatonic> that is not a whole number does not match <chromatic> either.
TEST(ScoreTransposeCorrection, aDiatonicThatIsNotAWholeNumberIsReplaced) {
    Score score(kUnitTest + "transpose_diatonic_not_integer.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"G4 (-4, -7, NONE) C4"}));
    EXPECT_EQ(
        transposeRecords(score),
        std::vector<ImportIssue>{transposeIssue(
            "TRANSPOSE_PAIR_CORRECTED", 0, "Horn in F", "1", 0, "attributes/transpose/diatonic",
            "-4.5", "-4",
            "<diatonic>-4.5</diatonic> does not match <chromatic>-7</chromatic>; using -4.")});
}

// A <transpose> with which a note of its scope would have no sounding pitch is ignored for its
// whole scope -- none of its notes takes it, not even one that could sound with it -- and the
// previous transposition stays in force; a chord with a note that this one cannot sound either is
// read untransposed as a unit, and the record counts its notes.
TEST(ScoreTransposeCorrection, aTransposeOutOfRangeIsIgnoredForItsWholeScope) {
    Score score(kUnitTest + "transpose_out_of_range.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C9 (7, 12, NONE) C10", "C8 (7, 12, NONE) C9",
                                        "C9 (7, 12, NONE) C10"}));
    EXPECT_EQ(transposedNotes(score, 1),
              (std::vector<std::string>{"C1 (-7, -12, NONE) C0", "Cb0 (0, 0, NONE) B-1",
                                        "C1 (0, 0, NONE) C1", "D1 (-7, -12, NONE) D0"}));
    EXPECT_EQ(
        transposeRecords(score),
        (std::vector<ImportIssue>{
            transposeIssue("TRANSPOSE_OUT_OF_RANGE", 0, "Piccolo", "2", 1, "attributes/transpose",
                           "diatonic 21, chromatic 36", "",
                           "The written C9 of measure 3, staff 1 would sound outside the "
                           "representable range; the <transpose> is ignored and the previous "
                           "transposition stays in force."),
            transposeIssue("TRANSPOSE_OUT_OF_RANGE", 1, "Contrabass", "2", 1,
                           "attributes/transpose", "diatonic -14, chromatic -24", "",
                           "The written Cb0 of measure 2, staff 1 would sound outside the "
                           "representable range; the <transpose> is ignored and the previous "
                           "transposition stays in force. Its notes that the previous "
                           "transposition cannot sound either are read untransposed, with the "
                           "other notes of their chords (2 in all).")}));
}

// <for-part> is dropped with a record: a concert score's notes are already at concert pitch.
TEST(ScoreTransposeCorrection, aForPartIsDroppedWithARecord) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_for_part.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"C4 (0, 0, NONE) C4"}));
    EXPECT_EQ(
        transposeRecords(score),
        std::vector<ImportIssue>{transposeIssue(
            "FOR_PART_NOT_MODELLED", 0, "Clarinet in Bb", "1", 0, "attributes/for-part", "", "",
            "<for-part> is not modelled and is dropped; the notes of a concert score are "
            "written at concert pitch.")});
    EXPECT_EQ(capture.str(),
              "[maiacore] transpose_for_part.musicxml: 0 corrections, 1 element types not "
              "modelled (dropped on export); see Score.getImportIssues()\n");
}

// ====================
// Values the reader replaces: part names, divisions, voices, staves and tuplet ratios
// ====================

namespace {
// A record of the reader on measure "1" of the part "Music", or on no measure when measureNumber
// is empty.
ImportIssue musicIssue(const std::string& code, const std::string& measureNumber,
                       const std::string& element, const std::string& found,
                       const std::string& used, const std::string& message) {
    ImportIssue issue;
    issue.code = code;
    issue.kind = "corrected";
    issue.partIndex = 0;
    issue.partName = "Music";
    issue.measureNumber = measureNumber;
    issue.measureIndex = measureNumber.empty() ? -1 : 0;
    issue.element = element;
    issue.found = found;
    issue.used = used;
    issue.message = message;
    return issue;
}

// A quarter-note C4 with 'extra' after its <duration>.
std::string quarterC4(const std::string& extra) {
    return "<note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration>" + extra +
           "<type>quarter</type></note>";
}
}  // namespace

// Parts that share a name are told apart by a suffix, in part order, and each renamed part is
// recorded; a part whose name is its own is not.
TEST(ScoreSilentCorrections, ADuplicatePartNameIsSuffixedAndRecorded) {
    const std::string measure =
        "<measure number=\"1\">" + kMinimalAttributes + kWholeC4 + "</measure></part>";
    const TemporaryFile file(
        "duplicate-names.musicxml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<score-partwise version=\"4.0\"><part-list>"
        "<score-part id=\"P1\"><part-name>Music</part-name></score-part>"
        "<score-part id=\"P2\"><part-name>Music</part-name></score-part>"
        "<score-part id=\"P3\"><part-name>Flute</part-name></score-part></part-list>"
        "<part id=\"P1\">" +
            measure + "<part id=\"P2\">" + measure + "<part id=\"P3\">" + measure +
            "</score-partwise>\n");

    Score score(file.path());
    EXPECT_EQ(score.getPartsNames(), (std::vector<std::string>{"Music 1", "Music 2", "Flute"}));
    ImportIssue second =
        musicIssue("PART_NAME_DUPLICATE", "", "part-list/score-part/part-name", "Music", "Music 2",
                   "More than one part has the name 'Music'; this one is named "
                   "'Music 2'.");
    second.partIndex = 1;
    second.partName = "Music 2";
    ImportIssue first = second;
    first.partIndex = 0;
    first.partName = "Music 1";
    first.used = "Music 1";
    first.message = "More than one part has the name 'Music'; this one is named 'Music 1'.";
    EXPECT_EQ(score.getImportIssues(), (std::vector<ImportIssue>{first, second}));
}

// A part whose first measure has no <divisions> is read at 256 divisions per quarter note, and
// recorded.
TEST(ScoreSilentCorrections, MissingDivisionsAreRecordedWithTheDefaultUsed) {
    std::string attributes = kMinimalAttributes;
    attributes.erase(attributes.find("<divisions>1</divisions>"), 24);
    const std::string note =
        "<note><pitch><step>C</step><octave>4</octave></pitch><duration>1024</duration>"
        "<voice>1</voice><type>whole</type></note>";
    const TemporaryFile file("no-divisions.musicxml", minimalScore(note, attributes));

    Score score(file.path());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getDivisionsPerQuarterNote(), 256);
    EXPECT_EQ(score.getImportIssues(),
              std::vector<ImportIssue>{musicIssue(
                  "DIVISIONS_MISSING", "1", "attributes/divisions", "", "256",
                  "The part's first measure has no <divisions>; 256 divisions per quarter note "
                  "are used.")});
}

// A <voice> that is not a positive whole number is read as voice 1 -- or as the digits it starts
// with -- and recorded; an absent <voice>, or one with white space around its number, is not.
TEST(ScoreSilentCorrections, AVoiceThatIsNotPositiveIsRecorded) {
    const TemporaryFile file(
        "voices.musicxml",
        minimalScore(quarterC4("<voice>0</voice>") + quarterC4("<voice>-2</voice>") +
                     quarterC4("<voice>2abc</voice>") + quarterC4("") +
                     quarterC4("<voice> 3 </voice>")));

    Score score(file.path());
    std::vector<int> voices;
    for (int n = 0; n < score.getPart(0).getMeasure(0).getNumNotes(0); n++) {
        voices.push_back(score.getPart(0).getMeasure(0).getNote(n, 0).getVoice());
    }
    EXPECT_EQ(voices, (std::vector<int>{1, 1, 2, 1, 3}));
    const auto voiceIssue = [](const std::string& found, const std::string& used) {
        return musicIssue("VOICE_NOT_POSITIVE", "1", "note/voice", found, used,
                          "<voice>" + found + "</voice> is not a positive whole number; voice " +
                              used + " is used.");
    };
    EXPECT_EQ(score.getImportIssues(),
              (std::vector<ImportIssue>{voiceIssue("0", "1"), voiceIssue("-2", "1"),
                                        voiceIssue("2abc", "2")}));
}

// A <staff> that is not a positive whole number is read as the first staff and recorded; an
// absent <staff> is not.
TEST(ScoreSilentCorrections, AStaffThatIsNotPositiveIsRecorded) {
    const TemporaryFile file("staves.musicxml",
                             minimalScore(quarterC4("<voice>1</voice><staff>0</staff>") +
                                          quarterC4("<voice>1</voice><staff>1</staff>") +
                                          quarterC4("<voice>1</voice>")));

    Score score(file.path());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNumNotes(0), 3);
    EXPECT_EQ(score.getImportIssues(),
              std::vector<ImportIssue>{
                  musicIssue("STAFF_CLAMPED", "1", "note/staff", "0", "1",
                             "<staff>0</staff> is not a positive whole number; staff 1 is used.")});
}

// A <time-modification> whose <actual-notes> or <normal-notes> is not a positive whole number --
// absent included, which the standard requires -- is read with 1 there, and each is recorded; a
// note without <time-modification> is no tuplet and records nothing.
TEST(ScoreSilentCorrections, ATupletValueThatIsNotPositiveIsRecorded) {
    const TemporaryFile file(
        "tuplets.musicxml",
        minimalScore(quarterC4("<voice>1</voice><time-modification><actual-notes>0</actual-notes>"
                               "</time-modification>") +
                     quarterC4("<voice>1</voice><time-modification><actual-notes>3</actual-notes>"
                               "<normal-notes>2</normal-notes></time-modification>") +
                     quarterC4("<voice>1</voice>")));

    Score score(file.path());
    EXPECT_EQ(score.getImportIssues(),
              (std::vector<ImportIssue>{
                  musicIssue("TUPLET_CLAMPED", "1", "note/time-modification/actual-notes", "0", "1",
                             "<actual-notes>0</actual-notes> is not a positive whole number; 1 is "
                             "used."),
                  musicIssue("TUPLET_CLAMPED", "1", "note/time-modification/normal-notes", "", "1",
                             "<normal-notes></normal-notes> is not a positive whole number; 1 is "
                             "used.")}));
}
````

- [ ] **Step 2: Write the failing Python tests.** In `test/test_musicxml_transpose.py` apply, in order:

**`test/test_musicxml_transpose.py` ~64: replace**

````python
def load(path):
    """The score of a MusicXML file, loaded without printing its warnings."""
    with contextlib.redirect_stdout(io.StringIO()):
````

with

````python
def load(path):
    """The score of a MusicXML file, loaded without printing the summary of its import report."""
    with contextlib.redirect_stdout(io.StringIO()):
````


**`test/test_musicxml_transpose.py` ~138: replace**

````python
def reloaded(data):
    """The score an export loads back as, and what the load printed."""
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "round-trip.musicxml"
        path.write_bytes(data)
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            score = ml.Score(str(path))
    return score, printed.getvalue()

````

with

````python
def reloaded(data):
    """The score an export loads back as, and the codes of the corrections the load recorded."""
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "round-trip.musicxml"
        path.write_bytes(data)
        with contextlib.redirect_stdout(io.StringIO()):
            score = ml.Score(str(path))
    corrections = [issue.code for issue in score.getImportIssues() if issue.kind == "corrected"]
    return score, corrections

````


**`test/test_musicxml_transpose.py` ~412: replace**

````python
        score.getPart(0).getMeasure(0).addNote(ml.Note("F#4", transposeChromatic=-2))
        again, printed = reloaded(export(score))
        note = again.getPart(0).getMeasure(0).getNote(0)
        self.assertEqual(
            (-1, -2, "E4"),
            (note.getTransposeDiatonic(), note.getTransposeChromatic(), note.getSoundingPitch()),
        )
        self.assertNotIn("[WARN]", printed)

````

with

````python
        score.getPart(0).getMeasure(0).addNote(ml.Note("F#4", transposeChromatic=-2))
        again, corrections = reloaded(export(score))
        note = again.getPart(0).getMeasure(0).getNote(0)
        self.assertEqual(
            (-1, -2, "E4"),
            (note.getTransposeDiatonic(), note.getTransposeChromatic(), note.getSoundingPitch()),
        )
        # Its export has no <divisions>; no <transpose> is corrected.
        self.assertIn("DIVISIONS_MISSING", corrections)
        self.assertEqual([], [code for code in corrections if code.startswith("TRANSPOSE_")])

    def test_a_transpose_record_names_its_part_and_measure(self):
        """The reader's <transpose> records reach Python with their fields."""
        score = load(REPO / "test/xml_examples/unit_test/transpose_pair_inconsistent.musicxml")
        found = [
            (
                issue.code,
                issue.partName,
                issue.measureNumber,
                issue.element,
                issue.found,
                issue.used,
            )
            for issue in score.getImportIssues()
            if issue.code == "TRANSPOSE_PAIR_CORRECTED"
        ]
        self.assertEqual(
            [
                (
                    "TRANSPOSE_PAIR_CORRECTED",
                    "Trumpet in E",
                    "1",
                    "attributes/transpose/diatonic",
                    "3",
                    "2",
                ),
                (
                    "TRANSPOSE_PAIR_CORRECTED",
                    "Clarinet in Bb",
                    "1",
                    "attributes/transpose/diatonic",
                    "0",
                    "-1",
                ),
            ],
            found,
        )

````

- [ ] **Step 3: Run them and see them fail.** C++ subset `ScoreTransposeRead*:ScoreTransposeCorrection*:ScoreSilentCorrections*` → it builds; the record tests fail with `transposeRecords(score)` `Which is: {}` (`aChromaticThatIsNotAWholeNumberIsIgnored`, `anOctaveChangeThatIsNotAWholeNumberIsIgnored`, `aDiatonicIntervalThatDoesNotMatchIsReplaced`, `theDvorakTrumpetsInESoundAMajorThirdUp`, `aDiatonicThatIsNotAWholeNumberIsReplaced`, `aTransposeOutOfRangeIsIgnoredForItsWholeScope`, `aForPartIsDroppedWithARecord`, the last also with `[WARN] [for-part-not-modelled] ...` printed instead of the summary), `ADuplicatePartNameIsSuffixedAndRecorded`, `MissingDivisionsAreRecordedWithTheDefaultUsed` and `AStaffThatIsNotPositiveIsRecorded` with an empty report, `AVoiceThatIsNotPositiveIsRecorded` with voices `{ 1, -2, 2, 1, 3 }` and an empty report, `ATupletValueThatIsNotPositiveIsRecorded` with an empty report. «pytest» `test_musicxml_transpose.TransposeRoundTripTestCase` → `test_a_transpose_record_names_its_part_and_measure` fails (`[] != [...]`).

- [ ] **Step 4: The records.** Apply, in order:

**`maiacore/src/maiacore/import-report.cpp` ~62: replace**

````cpp
        {"ALTER_OFF_GRID", "corrected"},
    };
````

with

````cpp
        {"ALTER_OFF_GRID", "corrected"},
        {"DIVISIONS_MISSING", "corrected"},
        {"FOR_PART_NOT_MODELLED", "dropped"},
        {"PART_NAME_DUPLICATE", "corrected"},
        {"STAFF_CLAMPED", "corrected"},
        {"TRANSPOSE_CHROMATIC_NOT_INTEGER", "corrected"},
        {"TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER", "corrected"},
        {"TRANSPOSE_OUT_OF_RANGE", "corrected"},
        {"TRANSPOSE_PAIR_CORRECTED", "corrected"},
        {"TUPLET_CLAMPED", "corrected"},
        {"VOICE_NOT_POSITIVE", "corrected"},
    };
````


**`maiacore/src/maiacore/score.cpp` ~79: replace**

````cpp
    int staff = -1;       // the 0-based staff its number attribute names; -1 for every staff
    std::string where;    // part "<name>", measure <number as the file writes it>
    // What it stamps; empty when it is ignored, and 'ignored' is then the warning that says why.
    std::optional<NoteTransposition> values;
    std::string ignored;
    std::string pairCorrected;  // the warning for a <diatonic> replaced, or empty
};
````

with

````cpp
    int staff = -1;       // the 0-based staff its number attribute names; -1 for every staff
    // What it stamps; empty when it is ignored, and 'ignoredCode' then says why.
    std::optional<NoteTransposition> values;
    // TRANSPOSE_CHROMATIC_NOT_INTEGER or TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER when the text of that
    // element, 'ignoredText', is not a whole number; empty otherwise.
    std::string ignoredCode;
    std::string ignoredText;
    std::string chromaticText;  // <chromatic> as the file writes it, trimmed
    // A <diatonic> that does not match <chromatic>: its text, trimmed, and the conventional
    // diatonic interval used instead.
    bool pairCorrected = false;
    std::string diatonicText;
    std::int64_t conventionalDiatonic = 0;
};
````


**`maiacore/src/maiacore/score.cpp` ~103: replace**

````cpp
    int index = 0;
};

// 'text' without the white space around it, which MusicXML numbers allow.
````

with

````cpp
    int index = 0;
};

// The record of a <transpose> that is ignored: why, and the text that is not a whole number.
ImportIssue ignoredTranspose(const TransposeElement& element, const IssueLocation& location) {
    const bool chromatic = element.ignoredCode == "TRANSPOSE_CHROMATIC_NOT_INTEGER";
    const std::string name = chromatic ? "chromatic" : "octave-change";
    return makeIssue(element.ignoredCode, location, "attributes/transpose/" + name,
                     element.ignoredText, "",
                     "<" + name + ">" + element.ignoredText + "</" + name +
                         "> is not a whole number of " + (chromatic ? "semitones" : "octaves") +
                         "; the <transpose> is ignored and the previous transposition stays in "
                         "force.");
}

// 'text' without the white space around it, which MusicXML numbers allow.
````


**`maiacore/src/maiacore/score.cpp` ~141: replace**

````cpp

// An interval held to the range of int: one beyond it is held at its limit, where no note can
````

with

````cpp

// Whether 'text', trimmed, is a whole number of at least 1 (see wholeNumber()).
bool isPositiveWholeNumber(const std::string& text) {
    const std::optional<std::int64_t> value = wholeNumber(trimmed(text));
    return value && *value >= 1;
}

// An interval held to the range of int: one beyond it is held at its limit, where no note can
````


**`maiacore/src/maiacore/score.cpp` ~151: replace**

````cpp
// conventional diatonic interval of <chromatic>; one that does not match it is replaced by that
// interval, with a warning. A <chromatic> or an <octave-change> that is not a whole number makes
// the element ignored. A number attribute that is not a positive integer is read as absent.
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
````

with

````cpp
// conventional diatonic interval of <chromatic>; one that does not match it is replaced by that
// interval, which is recorded. A <chromatic> or an <octave-change> that is not a whole number
// makes the element ignored. A number attribute that is not a positive integer is read as absent.
TransposeElement readTranspose(const pugi::xml_node& transpose, const int measureIdx,
                               const int notesBefore) {
    TransposeElement element;
    element.measureIdx = measureIdx;
    element.notesBefore = notesBefore;
    const std::optional<std::int64_t> number =
        wholeNumber(trimmed(transpose.attribute("number").value()));
    element.staff = (number && *number >= 1) ? toIntRange(*number - 1) : -1;

    element.chromaticText = trimmedChildText(transpose, "chromatic");
    const std::optional<std::int64_t> chromatic = wholeNumber(element.chromaticText);
    if (!chromatic) {
        element.ignoredCode = "TRANSPOSE_CHROMATIC_NOT_INTEGER";
        element.ignoredText = element.chromaticText;
        return element;
    }

    std::int64_t octaves = 0;
    if (transpose.child("octave-change")) {
        const std::string octaveText = trimmedChildText(transpose, "octave-change");
        const std::optional<std::int64_t> value = wholeNumber(octaveText);
        if (!value) {
            element.ignoredCode = "TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER";
            element.ignoredText = octaveText;
            return element;
````


**`maiacore/src/maiacore/score.cpp` ~207: replace**

````cpp
        } else {
            element.pairCorrected = "[transpose-pair-corrected] " + where + ": <diatonic>" +
                                    diatonicText + "</diatonic> does not match <chromatic>" +
                                    chromaticText + "</chromatic>; using " +
                                    std::to_string(conventional) + ".";
        }
````

with

````cpp
        } else {
            element.pairCorrected = true;
            element.diatonicText = diatonicText;
            element.conventionalDiatonic = conventional;
        }
````


**`maiacore/src/maiacore/score.cpp` ~228: replace**

````cpp
// The <transpose> elements of a measure's <attributes>, in document order, each with the number
// of <note> elements before its <attributes>; a <for-part> is reported and dropped.
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
            for (pugi::xml_node forPart = child.child("for-part"); forPart;
                 forPart = forPart.next_sibling("for-part")) {
                LOG_WARN("[for-part-not-modelled] " + where +
                         ": <for-part> is not modelled and is dropped; the notes of a concert "
                         "score are written at concert pitch.");
            }
````

with

````cpp
// The <transpose> elements of a measure's <attributes>, in document order, each with the number
// of <note> elements before its <attributes>; a <for-part> is dropped and recorded in 'issues'.
void readTransposeElements(const pugi::xml_node& measure, const IssueLocation& location,
                           std::vector<TransposeElement>& elements,
                           std::vector<ImportIssue>& issues) {
    int notesBefore = 0;
    for (const pugi::xml_node child : measure.children()) {
        const std::string name = child.name();
        if (name == "note") {
            notesBefore++;
        } else if (name == "attributes") {
            for (const pugi::xml_node transpose : child.children("transpose")) {
                elements.push_back(readTranspose(transpose, location.measureIndex, notesBefore));
            }
            for (pugi::xml_node forPart = child.child("for-part"); forPart;
                 forPart = forPart.next_sibling("for-part")) {
                issues.push_back(makeIssue("FOR_PART_NOT_MODELLED", location, "attributes/for-part",
                                           "", "",
                                           "<for-part> is not modelled and is dropped; the notes "
                                           "of a concert score are written at concert pitch."));
            }
````


**`maiacore/src/maiacore/score.cpp` ~270: replace**

````cpp
// a chord with a note it cannot sound either is read untransposed, all its notes. Each element
// prints its warnings: the corrected pair, then why it is ignored.
void applyTranspositions(Part& part, const std::vector<TransposeElement>& elements,
                         const std::vector<PitchedNote>& notes,
                         const std::vector<std::string>& measureNumbers) {
    std::map<int, std::vector<PitchedNote>> notesByStaff;
````

with

````cpp
// a chord with a note it cannot sound either is read untransposed, all its notes. Each element
// adds its records to 'issues': the corrected pair, then why it is ignored.
void applyTranspositions(Part& part, const int partIndex,
                         const std::vector<TransposeElement>& elements,
                         const std::vector<PitchedNote>& notes,
                         const std::vector<std::string>& measureNumbers,
                         std::vector<ImportIssue>& issues) {
    std::map<int, std::vector<PitchedNote>> notesByStaff;
````


**`maiacore/src/maiacore/score.cpp` ~306: replace**

````cpp
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
````

with

````cpp
        const TransposeElement& element = elements[e];
        const IssueLocation location{partIndex, part.getName(),
                                     measureNumbers.at(element.measureIdx), element.measureIdx};
        if (element.pairCorrected) {
            issues.push_back(makeIssue(
                "TRANSPOSE_PAIR_CORRECTED", location, "attributes/transpose/diatonic",
                element.diatonicText, std::to_string(element.conventionalDiatonic),
                "<diatonic>" + element.diatonicText + "</diatonic> does not match <chromatic>" +
                    element.chromaticText + "</chromatic>; using " +
                    std::to_string(element.conventionalDiatonic) + "."));
        }

        // Why the element is out of range, if it is: the first note of its scope that cannot
        // sound with it.
        std::string rejection;
        if (element.values) {
            const NoteTransposition& values = *element.values;
            for (const auto& scope : scopes[e]) {
                const std::vector<PitchedNote>& staffNotes = notesByStaff.at(scope.first);
                for (size_t i = scope.second.first; i < scope.second.second && rejection.empty();
                     i++) {
                    const Note& note = noteAt(staffNotes[i]);
                    if (!maiacore::detail::soundsWithinRange(note, values.diatonic,
                                                             values.chromatic)) {
                        rejection = "The written " + note.getWrittenPitch() + " of measure " +
                                    measureNumbers.at(staffNotes[i].measureIdx) + ", staff " +
````


**`maiacore/src/maiacore/score.cpp` ~337: replace**

````cpp

        int untransposed = 0;
        for (const auto& [staff, scope] : scopes[e]) {
            if (rejection.empty()) {
                inForce[staff] = *element.values;
````

with

````cpp

        const bool stamps = element.values && rejection.empty();
        int untransposed = 0;
        for (const auto& [staff, scope] : scopes[e]) {
            if (stamps) {
                inForce[staff] = *element.values;
````


**`maiacore/src/maiacore/score.cpp` ~370: replace**

````cpp

        if (!rejection.empty()) {
            if (untransposed > 0) {
                rejection +=
                    " Its notes that the previous transposition cannot sound either are "
                    "read untransposed, with the other notes of their chords (" +
                    std::to_string(untransposed) + " in all).";
            }
            LOG_WARN(rejection);
        }
````

with

````cpp

        // Notes that the transposition in force cannot sound either are read untransposed, with
        // the other notes of their chords.
        const std::string untransposedNotes =
            (untransposed > 0)
                ? " Its notes that the previous transposition cannot sound either are read "
                  "untransposed, with the other notes of their chords (" +
                      std::to_string(untransposed) + " in all)."
                : "";
        if (!element.ignoredCode.empty()) {
            ImportIssue issue = ignoredTranspose(element, location);
            issue.message += untransposedNotes;
            issues.push_back(issue);
        } else if (!rejection.empty()) {
            rejection += untransposedNotes;
            issues.push_back(makeIssue("TRANSPOSE_OUT_OF_RANGE", location, "attributes/transpose",
                                       "diatonic " + std::to_string(element.values->diatonic) +
                                           ", chromatic " +
                                           std::to_string(element.values->chromatic),
                                       "", rejection));
        }
````


**`maiacore/src/maiacore/score.cpp` ~781: replace**

````cpp
    if (hasDuplicatesPartNames(partsNameVec)) {
        LOG_INFO("Adding part names index suffix to better identification");

        // Adding part names index suffix to better identification
        auto modifyNames = [](std::vector<std::string>& vec) {
````

with

````cpp
    if (hasDuplicatesPartNames(partsNameVec)) {
        const std::vector<std::string> writtenNames = partsNameVec;

        // Parts that share a name are told apart by a suffix: " 1", " 2", ... in part order.
        auto modifyNames = [](std::vector<std::string>& vec) {
````


**`maiacore/src/maiacore/score.cpp` ~801: replace**

````cpp

        // Adding part names index suffix to better identification
        modifyNames(partsNameVec);
    }
````

with

````cpp

        modifyNames(partsNameVec);
        for (size_t n = 0; n < partsNameVec.size(); n++) {
            if (partsNameVec[n] != writtenNames[n]) {
                _importIssues.push_back(
                    makeIssue("PART_NAME_DUPLICATE",
                              IssueLocation{static_cast<int>(n), partsNameVec[n], "", -1},
                              "part-list/score-part/part-name", writtenNames[n], partsNameVec[n],
                              "More than one part has the name '" + writtenNames[n] +
                                  "'; this one is named '" + partsNameVec[n] + "'."));
            }
        }
    }
````


**`maiacore/src/maiacore/score.cpp` ~927: replace**

````cpp
                firstDivisionsTemp = firstMeasureDivisionsPerQuarterNote.node().text().as_int();
            }
````

with

````cpp
                firstDivisionsTemp = firstMeasureDivisionsPerQuarterNote.node().text().as_int();
            } else {
                _importIssues.push_back(makeIssue(
                    "DIVISIONS_MISSING",
                    IssueLocation{p, _part[p].getName(),
                                  firstMeasureNode.node().attribute("number").value(), 0},
                    "attributes/divisions", "", "256",
                    "The part's first measure has no <divisions>; 256 divisions per quarter note "
                    "are used."));
            }
````


**`maiacore/src/maiacore/score.cpp` ~1070: replace**

````cpp
            measureNumbers[m] = measureNode.node().attribute("number").value();
            readTransposeElements(
                measureNode.node(), m,
                "part \"" + _part[p].getName() + "\", measure " + measureNumbers[m],
                transposeElements);
            // The position of the first note of the chord the current note belongs to: a chord is
````

with

````cpp
            measureNumbers[m] = measureNode.node().attribute("number").value();
            const IssueLocation measureLocation{p, _part[p].getName(), measureNumbers[m], m};
            readTransposeElements(measureNode.node(), measureLocation, transposeElements,
                                  _importIssues);
            // The position of the first note of the chord the current note belongs to: a chord is
````


**`maiacore/src/maiacore/score.cpp` ~1120: replace**

````cpp
                staff = atoi(node.child_value("staff")) - 1;
                tupleActualNotes =
                    atoi(node.child("time-modification").child_value("actual-notes"));
                tupleNormalNotes =
                    atoi(node.child("time-modification").child_value("normal-notes"));
                tupleNormalType = node.child("time-modification").child_value("normal-type");
                if (tupleActualNotes == 0) {
                    tupleActualNotes = 1;
                }

                if (tupleNormalNotes == 0) {
                    tupleNormalNotes = 1;
                }
````

with

````cpp
                staff = atoi(node.child_value("staff")) - 1;
                const pugi::xml_node timeModification = node.child("time-modification");
                tupleActualNotes = atoi(timeModification.child_value("actual-notes"));
                tupleNormalNotes = atoi(timeModification.child_value("normal-notes"));
                tupleNormalType = timeModification.child_value("normal-type");
                if (tupleActualNotes <= 0) {
                    tupleActualNotes = 1;
                }

                if (tupleNormalNotes <= 0) {
                    tupleNormalNotes = 1;
                }

                // A tuplet ratio is two positive whole numbers; any other value is recorded with
                // the one used.
                if (timeModification) {
                    for (const char* name : {"actual-notes", "normal-notes"}) {
                        const std::string text = timeModification.child_value(name);
                        if (!isPositiveWholeNumber(text)) {
                            const std::string used = std::to_string(
                                std::string(name) == "actual-notes" ? tupleActualNotes
                                                                    : tupleNormalNotes);
                            _importIssues.push_back(makeIssue(
                                "TUPLET_CLAMPED", measureLocation,
                                std::string("note/time-modification/") + name, text, used,
                                "<" + std::string(name) + ">" + text + "</" + name +
                                    "> is not a positive whole number; " + used + " is used."));
                        }
                    }
                }
````


**`maiacore/src/maiacore/score.cpp` ~1238: replace**

````cpp
                    for (const AccidentalCorrection& correction : accidentalCorrections) {
                        _importIssues.push_back(makeIssue(
                            correction.code, {p, _part[p].getName(), measureNumbers[m], m},
                            correction.element, correction.found, pitch, correction.message));
                    }
                }

                if (voice == 0) {
                    voice = 1;
                }

                if (staff <= 0) {
                    staff = 0;
                }
````

with

````cpp
                    for (const AccidentalCorrection& correction : accidentalCorrections) {
                        _importIssues.push_back(makeIssue(correction.code, measureLocation,
                                                          correction.element, correction.found,
                                                          pitch, correction.message));
                    }
                }

                // A voice or a staff that is not a positive whole number is read as voice 1 or
                // the first staff -- or, for a text that starts with digits, as those digits --
                // and recorded when the file writes it.
                if (voice <= 0) {
                    voice = 1;
                }
                if (node.child("voice") && !isPositiveWholeNumber(node.child_value("voice"))) {
                    const std::string text = node.child_value("voice");
                    _importIssues.push_back(makeIssue(
                        "VOICE_NOT_POSITIVE", measureLocation, "note/voice", text,
                        std::to_string(voice),
                        "<voice>" + text + "</voice> is not a positive whole number; voice " +
                            std::to_string(voice) + " is used."));
                }

                if (staff <= 0) {
                    staff = 0;
                }
                if (node.child("staff") && !isPositiveWholeNumber(node.child_value("staff"))) {
                    const std::string text = node.child_value("staff");
                    _importIssues.push_back(makeIssue(
                        "STAFF_CLAMPED", measureLocation, "note/staff", text,
                        std::to_string(staff + 1),
                        "<staff>" + text + "</staff> is not a positive whole number; staff " +
                            std::to_string(staff + 1) + " is used."));
                }
````


**`maiacore/src/maiacore/score.cpp` ~1336: replace**

````cpp

        applyTranspositions(_part[p], transposeElements, pitchedNotes, measureNumbers);
    }
````

with

````cpp

        applyTranspositions(_part[p], p, transposeElements, pitchedNotes, measureNumbers,
                            _importIssues);
    }
````

- [ ] **Step 5: The Doxygen and numpydoc of `Score(filePath)`, and of `Part.setTransposingInterval` and `Part.toXML`, which named the old warning.** Apply, in order:

**`maiacore/include/maiacore/score.h` ~132: replace**

````cpp
     *          Note::getSoundingPitch()). One whose `<chromatic>` or `<octave-change>` is not a
     *          whole number is ignored, leaving the previous transposition in force, with a warning
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
     * @param filePath Path to the MusicXML file.
````

with

````cpp
     *          Note::getSoundingPitch()). One whose `<chromatic>` or `<octave-change>` is not a
     *          whole number is ignored, leaving the previous transposition in force (records
     *          TRANSPOSE_CHROMATIC_NOT_INTEGER and TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER). A
     *          `<diatonic>` that does not match `<chromatic>` is replaced by the conventional
     *          diatonic interval, so that nothing sounds different (TRANSPOSE_PAIR_CORRECTED); for
     *          a tritone both the augmented fourth and the diminished fifth match, and an explicit
     *          0 with a non-zero `<chromatic>` does not. A `<transpose>` with which a note of its
     *          scope -- the notes it would apply to, up to the next `<transpose>` for their staff
     *          -- would have no sounding pitch (below C1b-1, or above B11 where its letter cannot
     *          spell it) is ignored for its whole scope (TRANSPOSE_OUT_OF_RANGE); there the
     *          previous transposition stays in force, and a chord with a note it cannot sound
     *          either is read untransposed. `<for-part>` is not modelled: it is dropped
     *          (FOR_PART_NOT_MODELLED).
     *
     *          The reader also records these corrections: parts that share a name
     *          are told apart by a suffix, " 1", " 2", ... (PART_NAME_DUPLICATE); a part whose
     *          first measure has no `<divisions>` is read at 256 divisions per quarter note
     *          (DIVISIONS_MISSING); a `<voice>` or a `<staff>` that is not a positive whole number
     *          is read as voice 1 or the first staff, or as the digits its text starts with
     *          (VOICE_NOT_POSITIVE, STAFF_CLAMPED); an `<actual-notes>` or `<normal-notes>` of a
     *          `<time-modification>` that is not a positive whole number is read as 1
     *          (TUPLET_CLAMPED).
     * @param filePath Path to the MusicXML file.
````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~144: replace**

````cpp
        ``<octave-change>`` is not a whole number is ignored, leaving the previous transposition in
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

````

with

````cpp
        ``<octave-change>`` is not a whole number is ignored, leaving the previous transposition in
        force (records ``TRANSPOSE_CHROMATIC_NOT_INTEGER`` and
        ``TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER``). A ``<diatonic>`` that does not match
        ``<chromatic>`` is replaced by the conventional diatonic interval, so that nothing sounds
        different (``TRANSPOSE_PAIR_CORRECTED``); for a tritone both the augmented fourth and the
        diminished fifth match, and an explicit 0 with a non-zero ``<chromatic>`` does not. A
        ``<transpose>`` with which a note of its scope -- the notes it would apply to, up to the
        next ``<transpose>`` for their staff -- would have no sounding pitch (below ``C1b-1``, or
        above ``B11`` where its letter cannot spell it) is ignored for its whole scope
        (``TRANSPOSE_OUT_OF_RANGE``); there the previous transposition stays in force, and a chord
        with a note it cannot sound either is read untransposed. ``<for-part>`` is not modelled:
        it is dropped (``FOR_PART_NOT_MODELLED``).

        The reader also records these corrections: parts that share a name are
        told apart by a suffix, ``" 1"``, ``" 2"``, ... (``PART_NAME_DUPLICATE``); a part whose
        first measure has no ``<divisions>`` is read at 256 divisions per quarter note
        (``DIVISIONS_MISSING``); a ``<voice>`` or a ``<staff>`` that is not a positive whole
        number is read as voice 1 or the first staff, or as the digits its text starts with
        (``VOICE_NOT_POSITIVE``, ``STAFF_CLAMPED``); an ``<actual-notes>`` or ``<normal-notes>``
        of a ``<time-modification>`` that is not a positive whole number is read as 1
        (``TUPLET_CLAMPED``).

````

**`maiacore/include/maiacore/part.h` ~200: replace**

````cpp
     *          conventional one for its chromatic interval (or the diminished-fifth tritone);
     *          another pair is read back corrected (same sound, conventional spelling) with a
     *          [transpose-pair-corrected] warning.
````

with

````cpp
     *          conventional one for its chromatic interval (or the diminished-fifth tritone);
     *          another pair is read back corrected (same sound, conventional spelling), with a
     *          TRANSPOSE_PAIR_CORRECTED record in the import report (Score::getImportIssues()).
````

**`maiacore/src/maiacore/python_wrapper/py_part.cpp` ~76 and ~187 (two places, the same text): replace each**

````cpp
        conventional spelling) with a ``[transpose-pair-corrected]`` warning.
````

with

````cpp
        conventional spelling), with a ``TRANSPOSE_PAIR_CORRECTED`` record in the import report.
````

- [ ] **Step 6: Format, build, pass.** clang-format `import-report.cpp`, `score.cpp`, `score.h`, `py_score.cpp`, `part.h`, `py_part.cpp`, `score-test.cpp`. C++ subset `ScoreTransposeRead*:ScoreTransposeCorrection*:ScoreSilentCorrections*:ScoreImportReport*:ScoreQuarterTone*:ImportReport*` → all pass. «build» `make "PYTHON=$py" dev` → 0. «pytest» `test_musicxml_transpose test_score_comprehensive.ScoreImportReportTestCase` → OK. Bash: `git -C /c/Users/nyck/Desktop/maialib grep -n "LOG_WARN\|LOG_INFO" -- maiacore/src/maiacore/score.cpp` → only `Score::getChords` (one `LOG_WARN`, not a load) and `Score::info()` remain.

- [ ] **Step 7: Mutations.** (a) Replace `if (voice <= 0) {` with `if (voice == 0) {` → `AVoiceThatIsNotPositiveIsRecorded` fails (voice -2 kept). (b) Replace `if (timeModification) {` with `if (false) {` → `ATupletValueThatIsNotPositiveIsRecorded` fails. (c) In `readTransposeElements` delete the statement `issues.push_back(makeIssue("FOR_PART_NOT_MODELLED", ...));` (the loop's whole body) → `aForPartIsDroppedWithARecord` fails. (d) Replace `if (element.pairCorrected) {` with `if (element.values) {` → every applied `<transpose>` records a pair: `aChangeInTheMiddleOfAPartIsFollowed`, `aDiatonicIntervalThatDoesNotMatchIsReplaced` and, after `make dev`, `test_a_stored_diatonic_interval_of_zero_comes_back_as_the_conventional_one` and `test_a_transpose_record_names_its_part_and_measure` fail. (e) Replace `rejection += untransposedNotes;` with `rejection += "";` → `aTransposeOutOfRangeIsIgnoredForItsWholeScope` fails (the count sentence is gone). (f) Replace `if (partsNameVec[n] != writtenNames[n]) {` with `if (false) {` → `ADuplicatePartNameIsSuffixedAndRecorded` fails. (g) Replace the `} else {` that opens the `DIVISIONS_MISSING` record with `} else if (false) {` → `MissingDivisionsAreRecordedWithTheDefaultUsed` and, after `make dev`, `test_a_stored_diatonic_interval_of_zero_comes_back_as_the_conventional_one` fail. (h) Replace `if (node.child("staff") && !isPositiveWholeNumber(node.child_value("staff"))) {` with `if (false) {` → `AStaffThatIsNotPositiveIsRecorded` fails. (i) In `ignoredTranspose` pass `""` instead of `element.ignoredText` as `found` (the `makeIssue` argument after `"attributes/transpose/" + name`) → `aChromaticThatIsNotAWholeNumberIsIgnored` and `anOctaveChangeThatIsNotAWholeNumberIsIgnored` fail. (j) In the `TRANSPOSE_PAIR_CORRECTED` record pass `element.diatonicText` instead of `std::to_string(element.conventionalDiatonic)` as `used` → `aDiatonicThatIsNotAWholeNumberIsReplaced` and `aDiatonicIntervalThatDoesNotMatchIsReplaced` fail. Revert each; rerun green.

- [ ] **Step 8: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1203 tests.` (1198 + 5). «build» `make "PYTHON=$py" py-tests` → OK, `Ran 697 tests` (696 + 1), the ledger unchanged. «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 9: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/src/maiacore/import-report.cpp maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_score.cpp maiacore/include/maiacore/part.h maiacore/src/maiacore/python_wrapper/py_part.cpp tests-cpp/src/score-test.cpp test/test_musicxml_transpose.py`, message:

```
feat: <transpose> and the silent corrections become import records

The <transpose> warnings become records with UPPER_SNAKE codes and no
bracketed prefix: TRANSPOSE_CHROMATIC_NOT_INTEGER,
TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER, TRANSPOSE_PAIR_CORRECTED and
TRANSPOSE_OUT_OF_RANGE (corrected) and FOR_PART_NOT_MODELLED (dropped).
TransposeElement keeps the values found and used instead of finished
messages; part and measure are the record's fields.

What the reader corrected without a word is recorded: a part renamed
because another has its name (PART_NAME_DUPLICATE, which replaces the
[INFO] line), a first measure without <divisions> read at 256
(DIVISIONS_MISSING), and a <voice>, <staff>, <actual-notes> or
<normal-notes> that is not a positive whole number (VOICE_NOT_POSITIVE,
STAFF_CLAMPED, TUPLET_CLAMPED). An absent element takes the standard's
default without a record. A negative voice or tuplet value, which was
kept, is read as 1, as 0 was.

The tests that captured the warnings assert the records. The export of
an API-built score has no <divisions>, so its reload records
DIVISIONS_MISSING; the round-trip test of a zero diatonic interval now
asserts that no <transpose> is corrected.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 3: Dropped elements, by the closed element list

**Files:**
- Create: `test/xml_examples/unit_test/import_report_dropped.musicxml`
- Modify: `maiacore/src/maiacore/import-report.h` (include ~6; `droppedElements` before `importSummary` ~38), `maiacore/src/maiacore/import-report.cpp` (includes; the closed element list; catalogue; `droppedElements`), `maiacore/src/maiacore/score.cpp` (end of `loadXMLFile` ~1430), `maiacore/include/maiacore/score.h` (constructor Doxygen ~153), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (numpydoc ~164), `test/musicxml/ledger.json` (one line)
- Test: `tests-cpp/src/import-report-test.cpp` (`ImportReportDropped`), `tests-cpp/src/score-test.cpp` (the for-part summary ~1940; two new `ScoreImportReport` tests ~1123), `test/test_score_comprehensive.py` (`ScoreImportReportTestCase` ~308)

**Interfaces:**
- Consumes: `makeIssue`, `IssueLocation`, `ImportIssue`, `importSummary`, `TemporaryFile` (Task 1); `aForPartIsDroppedWithARecord` (Task 2).
- Produces: `std::vector<ImportIssue> maiacore::detail::droppedElements(const pugi::xml_document& document)` — one `ELEMENT_NOT_MODELLED` record (`dropped`, `partIndex` -1, `measureIndex` -1, `found` the count, `used` empty, message `'<path>' is not held by the model and is dropped on export (<n> in the file).`) per path outside the closed list, in path order; the catalogue entry `ELEMENT_NOT_MODELLED` (`dropped`); the file-local `heldOutsideMeasures()`, `heldInMeasures()`, `reportedInMeasures()`. `Score::loadXMLFile` appends the dropped records after the corrections. Test helpers `notModelled(element, count)`, `codesAndElements(const Score&)` in `score-test.cpp`, `droppedIn(text)` in `import-report-test.cpp`.

- [ ] **Step 1: The fixture.** Create it, then check it is valid: Bash `/c/Users/nyck/AppData/Local/Temp/maialib-4c1a-venv/Scripts/python.exe /c/Users/nyck/Desktop/maialib/test/musicxml/musicxml_check.py /c/Users/nyck/Desktop/maialib/test/xml_examples/unit_test/import_report_dropped.musicxml` → `valid; errors: none; warnings: none`.

**Create `test/xml_examples/unit_test/import_report_dropped.musicxml`**

````xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 4.0 Partwise//EN" "http://www.musicxml.org/dtds/partwise.dtd">
<!-- Elements the model does not hold, each recorded once per path with its count
     (ELEMENT_NOT_MODELLED) and dropped on export: <movement-title>, <encoding>,
     <score-instrument>, a <direction> with a <staff> of its own (not a note's <staff>), two
     <lyric> elements and a <fermata>. Every other element is held. Expected: 0 corrections and
     6 element types not modelled. -->
<score-partwise version="4.0">
  <movement-title>Dropped</movement-title>
  <identification>
    <creator type="composer">Anonymous</creator>
    <encoding>
      <software>hand-written</software>
    </encoding>
  </identification>
  <part-list>
    <score-part id="P1">
      <part-name>Voice</part-name>
      <score-instrument id="P1-I1">
        <instrument-name>Voice</instrument-name>
      </score-instrument>
    </score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key>
          <fifths>0</fifths>
        </key>
        <time>
          <beats>2</beats>
          <beat-type>4</beat-type>
        </time>
        <clef>
          <sign>G</sign>
          <line>2</line>
        </clef>
      </attributes>
      <direction placement="above">
        <direction-type>
          <words>dolce</words>
        </direction-type>
        <staff>1</staff>
      </direction>
      <note>
        <pitch>
          <step>C</step>
          <octave>4</octave>
        </pitch>
        <duration>1</duration>
        <voice>1</voice>
        <type>quarter</type>
        <lyric number="1">
          <syllabic>single</syllabic>
          <text>la</text>
        </lyric>
      </note>
      <note>
        <pitch>
          <step>D</step>
          <octave>4</octave>
        </pitch>
        <duration>1</duration>
        <voice>1</voice>
        <type>quarter</type>
        <notations>
          <fermata type="upright"/>
        </notations>
        <lyric number="1">
          <syllabic>single</syllabic>
          <text>la</text>
        </lyric>
      </note>
    </measure>
  </part>
</score-partwise>
````

- [ ] **Step 2: Write the failing C++ tests.** Apply, in order:

**`tests-cpp/src/import-report-test.cpp` ~8: replace**

````cpp
#include <string>
#include <vector>

using maiacore::detail::importSummary;
````

with

````cpp
#include <string>
#include <utility>
#include <vector>

using maiacore::detail::droppedElements;
using maiacore::detail::importSummary;
````


**`tests-cpp/src/import-report-test.cpp` ~81: replace**

````cpp
    EXPECT_THROW(makeIssue("NOT_A_CODE", IssueLocation{}, "", "", "", ""), std::logic_error);
}
````

with

````cpp
    EXPECT_THROW(makeIssue("NOT_A_CODE", IssueLocation{}, "", "", "", ""), std::logic_error);
}

namespace {
// The (element, found) pairs of the dropped records of a document.
std::vector<std::pair<std::string, std::string>> droppedIn(const std::string& text) {
    pugi::xml_document document;
    EXPECT_TRUE(document.load_string(text.c_str()));
    std::vector<std::pair<std::string, std::string>> dropped;
    for (const ImportIssue& issue : droppedElements(document)) {
        EXPECT_EQ(issue.code, "ELEMENT_NOT_MODELLED");
        EXPECT_EQ(issue.kind, "dropped");
        EXPECT_EQ(issue.partIndex, -1);
        EXPECT_EQ(issue.measureIndex, -1);
        EXPECT_EQ(issue.used, "");
        dropped.emplace_back(issue.element, issue.found);
    }
    return dropped;
}
}  // namespace

// The closed element list is matched by path: a <staff> or a <type> is held in a note, not in a
// <direction> or anywhere else; an element outside the list is counted once with everything in
// it, and every element of the list is held, the children of <articulations> whatever their
// names. <for-part> has a record of its own and is not counted.
TEST(ImportReportDropped, TheClosedListIsMatchedByPath) {
    const std::string text =
        "<score-partwise><credit><credit-words>x</credit-words></credit>"
        "<part-list><score-part id=\"P1\"><part-name>A</part-name><score-instrument id=\"I\">"
        "<instrument-name>A</instrument-name></score-instrument></score-part></part-list>"
        "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>1</divisions><clef>"
        "<sign>G</sign><line>2</line><clef-octave-change>-1</clef-octave-change></clef>"
        "<for-part><part-clef><sign>G</sign></part-clef></for-part></attributes>"
        "<direction><direction-type><words>p</words></direction-type><staff>1</staff></direction>"
        "<note><pitch><step>C</step><octave>4</octave></pitch><duration>1</duration><voice>1"
        "</voice><type>quarter</type><staff>1</staff><notations><articulations><accent/>"
        "<strong-accent/></articulations><fermata/></notations><lyric><text>a</text></lyric>"
        "</note><note><rest/><duration>1</duration><type>quarter</type><lyric><text>b</text>"
        "</lyric></note></measure></part></score-partwise>";
    EXPECT_EQ(droppedIn(text), (std::vector<std::pair<std::string, std::string>>{
                                   {"attributes/clef/clef-octave-change", "1"},
                                   {"credit", "1"},
                                   {"direction", "1"},
                                   {"note/lyric", "2"},
                                   {"note/notations/fermata", "1"},
                                   {"part-list/score-part/score-instrument", "1"}}));
}

// A document whose every element is held drops nothing; the message names the path and the count.
TEST(ImportReportDropped, ADocumentOfHeldElementsDropsNothing) {
    EXPECT_EQ(droppedIn("<score-partwise><work><work-title>T</work-title></work><part-list>"
                        "<score-part id=\"P1\"><part-name>A</part-name></score-part></part-list>"
                        "<part id=\"P1\"><measure number=\"1\"><backup><duration>1</duration>"
                        "</backup><forward><duration>1</duration></forward><barline><bar-style>"
                        "light-heavy</bar-style><repeat direction=\"backward\"/></barline>"
                        "</measure></part></score-partwise>"),
              (std::vector<std::pair<std::string, std::string>>{}));
    pugi::xml_document document;
    document.load_string("<score-partwise><print/><print/></score-partwise>");
    EXPECT_EQ(droppedElements(document).at(0).message,
              "'print' is not held by the model and is dropped on export (2 in the file).");
}
````


**`tests-cpp/src/score-test.cpp` ~1123: replace**

````cpp

// A title, a composer and a part name whose bytes are not UTF-8 are held with U+FFFD in place of
````

with

````cpp

namespace {
// The record of an element outside the closed element list.
ImportIssue notModelled(const std::string& element, const std::string& count) {
    ImportIssue issue;
    issue.code = "ELEMENT_NOT_MODELLED";
    issue.kind = "dropped";
    issue.element = element;
    issue.found = count;
    issue.message = "'" + element + "' is not held by the model and is dropped on export (" +
                    count + " in the file).";
    return issue;
}

// The (code, element) of each record of a score's report.
std::vector<std::pair<std::string, std::string>> codesAndElements(const Score& score) {
    std::vector<std::pair<std::string, std::string>> records;
    for (const ImportIssue& issue : score.getImportIssues()) {
        records.emplace_back(issue.code, issue.element);
    }
    return records;
}
}  // namespace

// Each element path outside the closed element list is one "dropped" record, after the
// corrections, with its count; the summary counts the paths.
TEST(ScoreImportReport, DroppedElementsAreRecordedOncePerPathWithTheirCount) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/import_report_dropped.musicxml");
    EXPECT_EQ(score.getImportIssues(),
              (std::vector<ImportIssue>{
                  notModelled("direction", "1"), notModelled("identification/encoding", "1"),
                  notModelled("movement-title", "1"), notModelled("note/lyric", "2"),
                  notModelled("note/notations/fermata", "1"),
                  notModelled("part-list/score-part/score-instrument", "1")}));
    EXPECT_EQ(capture.str(),
              "[maiacore] import_report_dropped.musicxml: 0 corrections, 6 element types not "
              "modelled (dropped on export); see Score.getImportIssues()\n");
}

// A score maialib exported, loaded again, records no correction; its dropped records are the
// header and part-list elements the writer invents, which the model does not hold.
TEST(ScoreImportReport, AnExportLoadedAgainRecordsOnlyTheElementsTheWriterInvents) {
    Score original("./test/xml_examples/unit_test/test_chord.xml");
    const TemporaryFile exported("export.musicxml", original.toXML());
    Score again(exported.path());
    EXPECT_EQ(codesAndElements(again),
              (std::vector<std::pair<std::string, std::string>>{
                  {"ELEMENT_NOT_MODELLED", "defaults"},
                  {"ELEMENT_NOT_MODELLED", "identification/encoding"},
                  {"ELEMENT_NOT_MODELLED", "identification/rights"},
                  {"ELEMENT_NOT_MODELLED", "part-list/score-part/part-abbreviation"},
                  {"ELEMENT_NOT_MODELLED", "part-list/score-part/part-abbreviation-display"},
                  {"ELEMENT_NOT_MODELLED", "part-list/score-part/part-name-display"},
                  {"ELEMENT_NOT_MODELLED", "part-list/score-part/score-instrument"}}));
}

// A title, a composer and a part name whose bytes are not UTF-8 are held with U+FFFD in place of
````


**`tests-cpp/src/score-test.cpp` ~1940: replace**

````cpp
    EXPECT_EQ(capture.str(),
              "[maiacore] transpose_for_part.musicxml: 0 corrections, 1 element types not "
              "modelled (dropped on export); see Score.getImportIssues()\n");
````

with

````cpp
    EXPECT_EQ(capture.str(),
              "[maiacore] transpose_for_part.musicxml: 0 corrections, 2 element types not "
              "modelled (dropped on export); see Score.getImportIssues()\n");
````

- [ ] **Step 3: Write the failing Python test.** Apply:

**`test/test_score_comprehensive.py` ~308: replace**

````python
        self.assertEqual([issueFields(issue) for issue in copy.getImportIssues()], report)

````

with

````python
        self.assertEqual([issueFields(issue) for issue in copy.getImportIssues()], report)

    def test_dropped_elements_are_one_row_per_path_with_their_count(self):
        score = ml.Score("./xml_examples/unit_test/import_report_dropped.musicxml")
        frame = score.getImportIssuesDataFrame()
        self.assertEqual(
            frame[["code", "kind", "element", "found", "partIndex"]].values.tolist(),
            [
                ["ELEMENT_NOT_MODELLED", "dropped", "direction", "1", -1],
                ["ELEMENT_NOT_MODELLED", "dropped", "identification/encoding", "1", -1],
                ["ELEMENT_NOT_MODELLED", "dropped", "movement-title", "1", -1],
                ["ELEMENT_NOT_MODELLED", "dropped", "note/lyric", "2", -1],
                ["ELEMENT_NOT_MODELLED", "dropped", "note/notations/fermata", "1", -1],
                [
                    "ELEMENT_NOT_MODELLED",
                    "dropped",
                    "part-list/score-part/score-instrument",
                    "1",
                    -1,
                ],
            ],
        )

````

- [ ] **Step 4: Run them and see them fail.** C++ subset `ImportReport*:ScoreImportReport*:ScoreTransposeCorrection*` → the build fails: `use of undeclared identifier 'droppedElements'` (import-report-test.cpp). «pytest» `test_score_comprehensive.ScoreImportReportTestCase.test_dropped_elements_are_one_row_per_path_with_their_count` → fails (`[] != [[...]]`).

- [ ] **Step 5: The closed element list and the pass.** Apply, in order:

**`maiacore/src/maiacore/import-report.h` ~6: replace**

````cpp
#include "maiacore/import-issue.h"

````

with

````cpp
#include "maiacore/import-issue.h"
#include "pugi/pugixml.hpp"

````


**`maiacore/src/maiacore/import-report.h` ~38: replace**

````cpp

// The line printed after a load whose report is not empty, without its line break:
````

with

````cpp

// One "dropped" record, ELEMENT_NOT_MODELLED, per element path of 'document' outside the closed
// element list -- the elements the model holds -- in path order, its count in 'found'. Paths are
// matched whole, because names such as <type> and <staff> mean different things in different
// places; an element outside the list is counted once, with everything in it. Inside a measure
// the path starts at the measure ("note/lyric"), elsewhere at the root ("credit").
std::vector<ImportIssue> droppedElements(const pugi::xml_document& document);

// The line printed after a load whose report is not empty, without its line break:
````


**`maiacore/src/maiacore/import-report.cpp` ~2: replace**

````cpp

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace maiacore::detail {

````

with

````cpp

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace maiacore::detail {

namespace {
// The closed element list: the elements the model holds, by path. Outside a measure the path
// starts at the root element's children; a part's measure starts a path of its own, from the
// measure's children. "<path>/*" holds every child of <path>, whatever its name. The children of
// a held element are checked in turn; an element that is not held is dropped with everything in
// it.
const std::set<std::string>& heldOutsideMeasures() {
    static const std::set<std::string> held = {
        "work",
        "work/work-title",
        "identification",
        "identification/creator",
        "part-list",
        "part-list/score-part",
        "part-list/score-part/part-name",
        "part-list/score-part/midi-instrument",
        "part-list/score-part/midi-instrument/midi-unpitched",
        "part",
    };
    return held;
}

const std::set<std::string>& heldInMeasures() {
    static const std::set<std::string> held = {
        "attributes",
        "attributes/divisions",
        "attributes/key",
        "attributes/key/fifths",
        "attributes/key/mode",
        "attributes/time",
        "attributes/time/beats",
        "attributes/time/beat-type",
        "attributes/staves",
        "attributes/clef",
        "attributes/clef/sign",
        "attributes/clef/line",
        "attributes/staff-details",
        "attributes/staff-details/staff-lines",
        "attributes/transpose",
        "attributes/transpose/diatonic",
        "attributes/transpose/chromatic",
        "attributes/transpose/octave-change",
        "attributes/transpose/double",
        "barline",
        "barline/bar-style",
        "barline/repeat",
        "backup",
        "backup/duration",
        "forward",
        "forward/duration",
        "forward/voice",
        "forward/staff",
        "note",
        "note/pitch",
        "note/pitch/step",
        "note/pitch/alter",
        "note/pitch/octave",
        "note/unpitched",
        "note/unpitched/display-step",
        "note/unpitched/display-octave",
        "note/rest",
        "note/chord",
        "note/grace",
        "note/duration",
        "note/voice",
        "note/type",
        "note/dot",
        "note/accidental",
        "note/stem",
        "note/staff",
        "note/time-modification",
        "note/time-modification/actual-notes",
        "note/time-modification/normal-notes",
        "note/time-modification/normal-type",
        "note/instrument",
        "note/beam",
        "note/tie",
        "note/notations",
        "note/notations/articulations",
        "note/notations/articulations/*",
        "note/notations/slur",
        "note/notations/tied",
    };
    return held;
}

// Elements inside a measure that the reader drops with a record of their own, so that the
// closed-list pass neither counts them nor looks into them.
const std::set<std::string>& reportedInMeasures() {
    static const std::set<std::string> reported = {"attributes/for-part"};
    return reported;
}
}  // namespace

````


**`maiacore/src/maiacore/import-report.cpp` ~63: replace**

````cpp
        {"DIVISIONS_MISSING", "corrected"},
        {"FOR_PART_NOT_MODELLED", "dropped"},
````

with

````cpp
        {"DIVISIONS_MISSING", "corrected"},
        {"ELEMENT_NOT_MODELLED", "dropped"},
        {"FOR_PART_NOT_MODELLED", "dropped"},
````


**`maiacore/src/maiacore/import-report.cpp` ~98: replace**

````cpp

std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues) {
````

with

````cpp

std::vector<ImportIssue> droppedElements(const pugi::xml_document& document) {
    // An element whose children are still to be checked: its path, and whether the path starts
    // at a measure.
    struct Pending {
        pugi::xml_node element;
        std::string path;
        bool inMeasure;
    };
    std::map<std::string, int> counts;
    std::vector<Pending> pending = {{document.document_element(), "", false}};
    while (!pending.empty()) {
        const Pending current = pending.back();
        pending.pop_back();
        const std::set<std::string>& held =
            current.inMeasure ? heldInMeasures() : heldOutsideMeasures();
        const bool everyChildHeld = held.count(current.path + "/*") > 0;
        for (const pugi::xml_node child : current.element.children()) {
            if (child.type() != pugi::node_element) {
                continue;
            }
            const std::string path =
                current.path.empty() ? child.name() : current.path + "/" + child.name();
            if (!current.inMeasure && path == "part/measure") {
                pending.push_back({child, "", true});
            } else if (current.inMeasure && reportedInMeasures().count(path) > 0) {
                continue;
            } else if (everyChildHeld || held.count(path) > 0) {
                pending.push_back({child, path, current.inMeasure});
            } else {
                counts[path]++;
            }
        }
    }

    std::vector<ImportIssue> issues;
    for (const auto& entry : counts) {
        const std::string count = std::to_string(entry.second);
        issues.push_back(makeIssue("ELEMENT_NOT_MODELLED", IssueLocation{}, entry.first, count, "",
                                   "'" + entry.first +
                                       "' is not held by the model and is dropped on export (" +
                                       count + " in the file)."));
    }
    return issues;
}

std::string importSummary(const std::string& fileName, const std::vector<ImportIssue>& issues) {
````


**`maiacore/src/maiacore/score.cpp` ~1430: replace**

````cpp
                            _importIssues);
    }

    if (!_importIssues.empty()) {
````

with

````cpp
                            _importIssues);
    }

    // The elements outside the closed element list, once per path, after the corrections.
    const std::vector<ImportIssue> dropped = maiacore::detail::droppedElements(_doc);
    _importIssues.insert(_importIssues.end(), dropped.begin(), dropped.end());

    if (!_importIssues.empty()) {
````

- [ ] **Step 6: The Doxygen and numpydoc.** Apply:

**`maiacore/include/maiacore/score.h` ~153: replace**

````cpp
     *          (TUPLET_CLAMPED).
     * @param filePath Path to the MusicXML file.
````

with

````cpp
     *          (TUPLET_CLAMPED).
     *
     *          An element the model does not hold -- one outside the closed element list, matched
     *          by its path, so that a `<staff>` in a `<direction>` is not a note's `<staff>` -- is
     *          dropped with everything in it, and dropped again on export. The report has one
     *          "dropped" record per such path (ELEMENT_NOT_MODELLED), after the corrections, with
     *          the number of such elements in the file as `found`.
     * @param filePath Path to the MusicXML file.
````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~164: replace**

````cpp
        (``TUPLET_CLAMPED``).

````

with

````cpp
        (``TUPLET_CLAMPED``).

        An element the model does not hold -- one outside the closed element list, matched by its
        path, so that a ``<staff>`` in a ``<direction>`` is not a note's ``<staff>`` -- is dropped
        with everything in it, and dropped again on export. The report has one ``"dropped"``
        record per such path (``ELEMENT_NOT_MODELLED``), after the corrections, with the number of
        such elements in the file as ``found``.

````

- [ ] **Step 7: Format, build, pass.** clang-format `import-report.h`, `import-report.cpp`, `score.cpp`, `score.h`, `py_score.cpp`, `import-report-test.cpp`, `score-test.cpp`. C++ subset `ImportReport*:ScoreImportReport*:ScoreTransposeCorrection*:ScoreQuarterTone*` → all pass (the accidental fixtures hold only held elements, so their summary lines still say `0 element types`). «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_score_comprehensive.ScoreImportReportTestCase test_musicxml_dump` → OK.

- [ ] **Step 8: The fixture's ledger line.** «build» `make "PYTHON=$py" corpus-update-ledger; $LASTEXITCODE` → 0. `git diff test/musicxml/ledger.json` shows exactly this added line and nothing else (no record holds report data yet, so no other line changes):

```
+  "test/xml_examples/unit_test/import_report_dropped.musicxml": {"export": "ok", "export_errors": [], "export_xml": "well-formed", "export_xsd": "valid", "input": "valid", "load": "ok", "roundtrip": "unstable"},
```

  and `git diff --stat test/musicxml/ledger-external.json` is empty. The update runs every corpus file, the slow ones and the external corpus included (about 18 minutes here); the options to run a subset come in Task 7.

- [ ] **Step 9: Mutations.** (a) Delete the line `"note/notations/articulations/*",` from `heldInMeasures()` → `ImportReportDropped.TheClosedListIsMatchedByPath` fails (`note/notations/articulations/accent` and `.../strong-accent` dropped). (b) Replace `} else if (current.inMeasure && reportedInMeasures().count(path) > 0) {` with `} else if (false) {` → `TheClosedListIsMatchedByPath` fails (`attributes/for-part` counted). (c) Replace `counts[path]++;` with `counts[path] = 1;` → `TheClosedListIsMatchedByPath`, `ADocumentOfHeldElementsDropsNothing`, `DroppedElementsAreRecordedOncePerPathWithTheirCount` and `test_dropped_elements_are_one_row_per_path_with_their_count` fail (`note/lyric` counted 1). (d) In `loadXMLFile` replace `_importIssues.insert(_importIssues.end(), dropped.begin(), dropped.end());` with `(void)dropped;` → `DroppedElementsAreRecordedOncePerPathWithTheirCount`, `AnExportLoadedAgainRecordsOnlyTheElementsTheWriterInvents`, `aForPartIsDroppedWithARecord` (`1 element types`) and the Python test fail. (e) Replace `"note/staff",` in `heldInMeasures()` with `"direction/staff",` → `TheClosedListIsMatchedByPath` fails (`note/staff` dropped; a `<direction>`'s `<staff>` is still inside the dropped `<direction>`). Revert each; rerun green.

- [ ] **Step 10: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1207 tests.` (1203 + 4). «build» `make "PYTHON=$py" py-tests` → OK, `Ran 698 tests` (697 + 1). «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 11: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/src/maiacore/import-report.h maiacore/src/maiacore/import-report.cpp maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/import-report-test.cpp tests-cpp/src/score-test.cpp test/test_score_comprehensive.py test/xml_examples/unit_test/import_report_dropped.musicxml test/musicxml/ledger.json`, message:

```
feat: the import report records the elements the model does not hold

One extra pass over the document compares every element with the
closed element list, matched by path, so that a <staff> in a
<direction> is not a note's <staff>: each path outside the list is one
"dropped" record, ELEMENT_NOT_MODELLED, with the number of such
elements in the file, after the corrections. An element outside the
list is counted with everything in it; <for-part> keeps its own record.
The summary line counts these paths as element types not modelled.

A score maialib exported records, when loaded again, the seven header
and part-list elements its writer invents, which the model does not
hold; a test pins them for the writer's phase.

The new fixture import_report_dropped.musicxml holds six such paths.
ledger.json: one line added, the new fixture's (valid, loads, exports
valid XML, round trip unstable like the other hand-written fixtures).

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 4: Fatal input names the file and the problem

**Files:**
- Modify: `maiacore/src/maiacore/score.cpp` (includes ~7; `fileBytes`, `notWellFormed`, `mxlRootfile` before `trimmed` ~122; the loading block of `loadXMLFile` ~717-760), `maiacore/include/maiacore/score.h` (`@throws` of `Score(filePath)`), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (Raises of `Score(filePath)`)
- Test: `tests-cpp/src/score-test.cpp` (new `ScoreFatalInput` before `NamesThatAreNotUtf8AreHeldAsValidUtf8`), `test/test_score_comprehensive.py` (new `CONTAINER` and `ScoreFatalInputTestCase` after `ScoreLoadingTestCase`)

**Interfaces:**
- Consumes: `validUtf8` (Task 1), `thrownFirstLine` (`test-capture.h`), `TemporaryFile`, `minimalScore`, `kWholeC4` (Task 1).
- Produces (file-local in `score.cpp`): `std::optional<std::vector<unsigned char>> fileBytes(const std::string& path)`, `std::string notWellFormed(const pugi::xml_parse_result& result, const std::string& document)`, `std::pair<std::string, std::string> mxlRootfile(const std::vector<unsigned char>& bytes, const std::string& archive)` (the rootfile's content and name). `loadXMLFile` parses plain files with `load_buffer`. Task 5 changes `fileBytes` and the extension test.

- [ ] **Step 1: Write the failing C++ tests.** Apply:

**`tests-cpp/src/score-test.cpp` ~1177: replace**

````cpp
                  {"ELEMENT_NOT_MODELLED", "part-list/score-part/score-instrument"}}));
}
````

with

````cpp
                  {"ELEMENT_NOT_MODELLED", "part-list/score-part/score-instrument"}}));
}

// ====================
// Fatal input
// ====================

// A file that is not well-formed XML raises with pugixml's description of the error and the byte
// offset where the parser stopped.
TEST(ScoreFatalInput, AFileThatIsNotWellFormedXmlGivesTheParsersDescriptionAndOffset) {
    const TemporaryFile file("broken.musicxml", "<score-partwise><part-list></score-partwise>");
    EXPECT_EQ(thrownFirstLine([&file] { Score score(file.path()); }),
              "[maiacore] Score: '" + file.path() +
                  "' is not well-formed XML: Start-end tags mismatch (byte offset 29)");
}

// A file that cannot be opened is named.
TEST(ScoreFatalInput, AFileThatCannotBeOpenedIsNamed) {
    EXPECT_EQ(thrownFirstLine([] { Score score("./test/xml_examples/missing.xml"); }),
              "[maiacore] Score: cannot open './test/xml_examples/missing.xml'");
}

// An .mxl that is not a zip archive -- a plain MusicXML file, or a file too short to be one --
// is named with the problem.
TEST(ScoreFatalInput, AnMxlThatIsNotAZipArchiveIsNamed) {
    for (const std::string& content : {minimalScore(kWholeC4), std::string("PK")}) {
        const TemporaryFile file("plain.mxl", content);
        EXPECT_EQ(thrownFirstLine([&file] { Score score(file.path()); }),
                  "[maiacore] Score: '" + file.path() +
                      "' is not a readable MusicXML archive: it is not a zip archive");
    }
}
````

- [ ] **Step 2: Write the failing Python tests.** Apply, in order:

**`test/test_score_comprehensive.py` ~16: replace**

````python
import unittest

````

with

````python
import unittest
import zipfile

````


**`test/test_score_comprehensive.py` ~129: replace**

````python
        self.assertIn("test_chord.xml", score.getFilePath())

````

with

````python
        self.assertIn("test_chord.xml", score.getFilePath())


# The META-INF/container.xml of an .mxl archive whose rootfile is score.xml.
CONTAINER = (
    b'<?xml version="1.0" encoding="UTF-8"?><container><rootfiles>'
    b'<rootfile full-path="score.xml"/></rootfiles></container>'
)


class ScoreFatalInputTestCase(unittest.TestCase):
    """What Score(path) raises for a file it cannot read: the first line of the message."""

    def assertLoadFails(self, path, message):
        with self.assertRaises(RuntimeError) as raised:
            ml.Score(path)
        self.assertEqual(str(raised.exception).splitlines()[0], message)

    def archive(self, directory, entries):
        """An .mxl archive in 'directory' holding 'entries' (name -> bytes); its path."""
        path = os.path.join(directory, "score.mxl")
        with zipfile.ZipFile(path, "w") as archive:
            for name, data in entries.items():
                archive.writestr(name, data)
        return path

    def test_a_missing_file_is_named(self):
        self.assertLoadFails(
            "./nonexistent_file.xml", "[maiacore] Score: cannot open './nonexistent_file.xml'"
        )

    def test_a_file_that_is_not_well_formed_gives_the_description_and_offset(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "broken.xml")
            with open(path, "wb") as broken:
                broken.write(b"<score-partwise><part-list></score-partwise>")
            self.assertLoadFails(
                path,
                f"[maiacore] Score: '{path}' is not well-formed XML: Start-end tags mismatch "
                "(byte offset 29)",
            )

    def test_an_mxl_that_is_not_a_zip_archive_is_named(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "plain.mxl")
            with open(path, "wb") as plain:
                plain.write(b'<score-partwise version="4.0"/>')
            self.assertLoadFails(
                path,
                f"[maiacore] Score: '{path}' is not a readable MusicXML archive: it is not a zip "
                "archive",
            )

    def test_an_mxl_without_its_container_is_named(self):
        with tempfile.TemporaryDirectory() as directory:
            path = self.archive(directory, {"score.xml": b"<score-partwise/>"})
            self.assertLoadFails(
                path,
                f"[maiacore] Score: '{path}' is not a readable MusicXML archive: it has no "
                "META-INF/container.xml",
            )

    def test_an_mxl_whose_container_names_no_rootfile_is_named(self):
        with tempfile.TemporaryDirectory() as directory:
            path = self.archive(directory, {"META-INF/container.xml": b"<container/>"})
            self.assertLoadFails(
                path,
                f"[maiacore] Score: '{path}' is not a readable MusicXML archive: its "
                "META-INF/container.xml names no rootfile",
            )

    def test_an_mxl_whose_rootfile_is_missing_is_named(self):
        with tempfile.TemporaryDirectory() as directory:
            path = self.archive(directory, {"META-INF/container.xml": CONTAINER})
            self.assertLoadFails(
                path,
                f"[maiacore] Score: '{path}' is not a readable MusicXML archive: the rootfile "
                "'score.xml' is not in the archive",
            )

    def test_an_mxl_whose_rootfile_is_not_well_formed_names_both(self):
        with tempfile.TemporaryDirectory() as directory:
            path = self.archive(
                directory,
                {"META-INF/container.xml": CONTAINER, "score.xml": b"<score-partwise><part"},
            )
            self.assertLoadFails(
                path,
                f"[maiacore] Score: the rootfile 'score.xml' of '{path}' is not well-formed XML: "
                "Error parsing start element tag (byte offset 20)",
            )

````

- [ ] **Step 3: Run them and see them fail.** C++ subset `ScoreFatalInput.AFileThat*` → both fail, their first lines `[maiacore] Unable to load the file: <path>`. Do not run `AnMxlThatIsNotAZipArchiveIsNamed` before Step 4: its two-byte `PK` file makes miniz read past its buffer, undefined behaviour that a Debug build with checked iterators turns into an abort. «pytest» `test_score_comprehensive.ScoreFatalInputTestCase` → the seven tests fail: `[maiacore] Unable to load the file: ...` where the description and offset are expected, `not found` for the archive without a container or without its rootfile, and `didn't find end of central directory signature` for the plain `.mxl`.

- [ ] **Step 4: The messages.** In `maiacore/src/maiacore/score.cpp` apply, in order:

**`maiacore/src/maiacore/score.cpp` ~7: replace**

````cpp
#include <filesystem>  // Para std::filesystem::absolute
#include <functional>
#include <future>
#include <iostream>
#include <limits>  // std::numeric_limits
#include <locale>
#include <map>
#include <optional>
````

with

````cpp
#include <filesystem>  // Para std::filesystem::absolute
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <limits>  // std::numeric_limits
#include <locale>
#include <map>
#include <memory>
#include <optional>
````


**`maiacore/src/maiacore/score.cpp` ~122: replace**

````cpp
                         "force.");
}
````

with

````cpp
                         "force.");
}

// The bytes of the file at 'path', or std::nullopt when it cannot be opened.
std::optional<std::vector<unsigned char>> fileBytes(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return std::nullopt;
    }
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(stream),
                                      std::istreambuf_iterator<char>());
}

// Why pugixml could not load a document: its description and the byte offset where it stopped.
// 'document' names the document in the message, quoted.
std::string notWellFormed(const pugi::xml_parse_result& result, const std::string& document) {
    return "Score: " + document + " is not well-formed XML: " + result.description() +
           " (byte offset " + std::to_string(result.offset) + ")";
}

// The MusicXML document of an .mxl archive -- the first rootfile its META-INF/container.xml
// names -- and the rootfile's name. 'archive' names the archive in the messages, quoted. Throws
// std::runtime_error when the bytes are not a zip archive, the archive has no
// META-INF/container.xml or an entry cannot be read, the container names no rootfile, or the
// rootfile is not in the archive.
std::pair<std::string, std::string> mxlRootfile(const std::vector<unsigned char>& bytes,
                                                const std::string& archive) {
    const std::string unreadable = "Score: " + archive + " is not a readable MusicXML archive: ";
    // The smallest zip archive is its 22-byte end-of-central-directory record.
    std::unique_ptr<miniz_cpp::zip_file> zip;
    if (bytes.size() >= 22) {
        try {
            zip = std::make_unique<miniz_cpp::zip_file>(bytes);
        } catch (const std::exception&) {
            zip.reset();
        }
    }
    if (!zip) {
        LOG_ERROR(unreadable + "it is not a zip archive");
    }
    const auto read = [&zip, &unreadable](const std::string& name) {
        std::string content;
        bool readable = true;
        try {
            content = zip->read(name);
        } catch (const std::exception&) {
            readable = false;
        }
        if (!readable) {
            LOG_ERROR(unreadable + "its entry '" + maiacore::detail::validUtf8(name) +
                      "' cannot be read");
        }
        return content;
    };

    if (!zip->has_file("META-INF/container.xml")) {
        LOG_ERROR(unreadable + "it has no META-INF/container.xml");
    }
    pugi::xml_document container;
    container.load_string(read("META-INF/container.xml").c_str());
    const std::string rootfile = container.select_node("/container/rootfiles/rootfile")
                                     .node()
                                     .attribute("full-path")
                                     .value();
    if (rootfile.empty()) {
        LOG_ERROR(unreadable + "its META-INF/container.xml names no rootfile");
    }
    if (!zip->has_file(rootfile)) {
        LOG_ERROR(unreadable + "the rootfile '" + maiacore::detail::validUtf8(rootfile) +
                  "' is not in the archive");
    }
    return {read(rootfile), rootfile};
}
````


**`maiacore/src/maiacore/score.cpp` ~717: replace**

````cpp

    std::vector<std::string> result2 = Helper::splitString(filePath, '/');
    const std::string fileName = result2[result2.size() - 1];
    pugi::xml_parse_result isLoad;

    if (fileExtension == "mxl") {
        // LOG_DEBUG("Decompressing file...");
        miniz_cpp::zip_file file(filePath);

        // Read the internal META-INF/container.xml file
        const std::string containerFile = file.read("META-INF/container.xml");
        pugi::xml_document containerXML;
        containerXML.load_string(containerFile.c_str());

        const std::string xPathInternalXMLFile = "/container/rootfiles/rootfile";

        const std::string internalXMLFileName =
            containerXML.select_node(xPathInternalXMLFile.c_str())
                .node()
                .attribute("full-path")
                .value();

        const std::string fileContent = file.read(internalXMLFileName);
        isLoad = _doc.load_string(fileContent.c_str());
    } else {
        // Try to parse the XML file:
        isLoad = _doc.load_file(filePath.c_str());
    }

    // Error checking:
    if (!isLoad) {
        LOG_ERROR("Unable to load the file: " + filePath);
        return;
    }
````

with

````cpp

    // The path as messages quote it.
    const std::string shownPath = "'" + maiacore::detail::validUtf8(filePath) + "'";
    const std::optional<std::vector<unsigned char>> bytes = fileBytes(filePath);
    if (!bytes) {
        LOG_ERROR("Score: cannot open " + shownPath);
    }

    if (fileExtension == "mxl") {
        const auto [document, rootfile] = mxlRootfile(*bytes, shownPath);
        const pugi::xml_parse_result result = _doc.load_string(document.c_str());
        if (!result) {
            LOG_ERROR(notWellFormed(
                result,
                "the rootfile '" + maiacore::detail::validUtf8(rootfile) + "' of " + shownPath));
        }
    } else {
        const pugi::xml_parse_result result = _doc.load_buffer(bytes->data(), bytes->size());
        if (!result) {
            LOG_ERROR(notWellFormed(result, shownPath));
        }
    }
````

- [ ] **Step 5: The Doxygen and numpydoc.** Apply:

**`maiacore/include/maiacore/score.h` ~160: replace**

````cpp
     * @param filePath Path to the MusicXML file.
     */
````

with

````cpp
     * @param filePath Path to the MusicXML file.
     * @throws std::runtime_error If the path is too short to name a file; the file cannot be
     *         opened ("Score: cannot open '<path>'"); it is not well-formed XML (pugixml's
     *         description and the byte offset where it stopped); an `.mxl` is not a readable
     *         archive (the file and the problem: not a zip archive, no META-INF/container.xml, no
     *         rootfile named, the rootfile not in the archive or not well-formed XML); or the
     *         document lacks the MusicXML part and measure elements.
     */
````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~179: replace**

````cpp
        RuntimeError
            If the path is too short to name a file, the file cannot be loaded, or it lacks
            the MusicXML part and measure elements.

````

with

````cpp
        RuntimeError
            If the path is too short to name a file; the file cannot be opened
            (``Score: cannot open '<path>'``); it is not well-formed XML (the parser's
            description and the byte offset where it stopped); an ``.mxl`` is not a readable
            archive (the file and the problem: not a zip archive, no ``META-INF/container.xml``,
            no rootfile named, the rootfile not in the archive or not well-formed XML); or the
            document lacks the MusicXML part and measure elements.

````

- [ ] **Step 6: Format, build, pass.** clang-format `score.cpp`, `score.h`, `py_score.cpp`, `score-test.cpp`. C++ subset `ScoreFatalInput*:ScoreConstructor*:ScoreImportReport*` → all pass. «build» `make "PYTHON=$py" dev` → 0; «pytest» `test_score_comprehensive.ScoreFatalInputTestCase test_score_comprehensive.ScoreLoadingTestCase` → OK.

- [ ] **Step 7: Mutations.** (a) In `notWellFormed` replace `" (byte offset " + std::to_string(result.offset) + ")"` with `""` → `AFileThatIsNotWellFormedXmlGivesTheParsersDescriptionAndOffset`, `test_a_file_that_is_not_well_formed_gives_the_description_and_offset` and `test_an_mxl_whose_rootfile_is_not_well_formed_names_both` fail. (b) Replace `if (!zip->has_file("META-INF/container.xml")) {` with `if (false) {` → `test_an_mxl_without_its_container_is_named` fails (`its entry 'META-INF/container.xml' cannot be read`). (c) Replace `if (rootfile.empty()) {` with `if (false) {` → `test_an_mxl_whose_container_names_no_rootfile_is_named` fails (`the rootfile '' is not in the archive`). (d) Replace `if (!zip->has_file(rootfile)) {` with `if (false) {` → `test_an_mxl_whose_rootfile_is_missing_is_named` fails (`its entry 'score.xml' cannot be read`). (e) Replace `LOG_ERROR("Score: cannot open " + shownPath);` with `LOG_ERROR("Score: cannot open the file");` → `AFileThatCannotBeOpenedIsNamed` and `test_a_missing_file_is_named` fail. (f) In `mxlRootfile` replace `zip.reset();` with `throw;` → `AnMxlThatIsNotAZipArchiveIsNamed` and `test_an_mxl_that_is_not_a_zip_archive_is_named` fail (miniz's `didn't find end of central directory signature`). Revert each; rerun green.

- [ ] **Step 8: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1210 tests.` (1207 + 3). «build» `make "PYTHON=$py" py-tests` → OK, `Ran 705 tests` (698 + 7), the ledger unchanged (a file that fails still raises `RuntimeError`). «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 9: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/src/maiacore/score.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_score.cpp tests-cpp/src/score-test.cpp test/test_score_comprehensive.py`, message:

```
fix: a file that cannot be read raises with the file and the problem

Score(path) said "Unable to load the file: <path>" whatever went wrong,
and an .mxl archive raised miniz's own words ("not found", "didn't find
end of central directory signature") without naming the file. It now
says "Score: cannot open '<path>'" for a file it cannot open, gives
pugixml's description and the byte offset where the parser stopped for
one that is not well-formed XML, and names the archive and its problem:
not a zip archive, no META-INF/container.xml, no rootfile named, the
rootfile not in the archive, or the rootfile not well-formed. An .mxl
shorter than the smallest zip archive (22 bytes) is not handed to
miniz, whose comment scan reads past a buffer of 1 to 3 bytes.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 5: Unicode paths, and a console that cannot end the interpreter

**Files:**
- Create: `maiacore/src/maiacore/python_wrapper/py_console.h`
- Modify: `maiacore/src/maiacore/score.cpp` (`<cctype>`; `fileBytes` ~127; the extension ~789), `maiacore/src/maiacore/score_collection.cpp` (include ~11; `musicXMLFilesOf` and `musicXMLFiles` ~44-95; `setDirectoriesPaths` ~134), `maiacore/include/maiacore/score.h` (Doxygen ~101, ~159, ~362), `maiacore/src/maiacore/python_wrapper/py_score.cpp` (include, `using`, the guard and numpydoc of `Score(filePath)`), `maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` (include, `using`, the guards of the two constructors and `setDirectoriesPaths`, its numpydoc), `test/musicxml/corpus_worker.py` (docstring; `loadable_path` removed), `test/musicxml/README.md` (~41)
- Test: `tests-cpp/src/score-test.cpp` (new `ScoreUnicodePath` before `NamesThatAreNotUtf8AreHeldAsValidUtf8`), `tests-cpp/src/score-collection-test.cpp` (`TemporaryDirectory::utf8()` and `addCopy` ~45; a new test before `EmptyDirectoryList` ~128), `test/test_score_comprehensive.py` (`import shutil`; new `ScoreUnicodePathTestCase` before `ScoreImportReportTestCase`), `test/test_score_collection.py` (`runChild` ~44; `fileNamesAreUtf8` removed ~86; the non-ASCII test ~233 rewritten), `test/test_musicxml_corpus.py` (~384-410)

**Interfaces:**
- Consumes: `fileBytes` (Task 4), `validUtf8` (Task 1), `TemporaryFile`, `StdoutCapture` (`test-capture.h`).
- Produces: `fileBytes` opens through `std::filesystem::u8path`; an archive recognised by a lowercase compare of the last four characters with `.mxl` or by the zip signature `PK` at the start of the bytes; in `maiacore_python` (`py_console.h`): `py::object tolerantStream(const char* name)` and the call guard `class ConsoleRedirect` (default-constructible; redirects `std::cout`/`std::cerr`), used by `Score(filePath)`, both `ScoreCollection` directory constructors and `setDirectoriesPaths` (Task 6 adds the three `addScore`); `ScoreCollection` builds its paths with `std::filesystem::u8path` and passes `file.u8string()` to `Score`; `TemporaryDirectory::utf8()` in `score-collection-test.cpp`; `runChild(code, environment=None)` in `test_score_collection.py`.

- [ ] **Step 1: Write the failing C++ tests.** Apply, in order:

**`tests-cpp/src/score-test.cpp` ~1209: replace**

````cpp

// A title, a composer and a part name whose bytes are not UTF-8 are held with U+FFFD in place of
````

with

````cpp

// ====================
// Unicode paths
// ====================

namespace {
// The bytes of a file.
std::string fileContent(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream content;
    content << in.rdbuf();
    return content.str();
}
}  // namespace

// A UTF-8 path with characters outside every ANSI code page opens, uncompressed and compressed,
// and the score keeps its name and path as UTF-8.
TEST(ScoreUnicodePath, ANonAsciiPathLoadsAndKeepsItsNameAsUtf8) {
    const std::string name = "can\xC3\xA7\xC3\xA3o \xE6\x97\xA5\xE6\x9C\xAC";
    const TemporaryFile xml(name + ".xml",
                            fileContent("./test/xml_examples/unit_test/quarter_tone_tartini.xml"));
    const TemporaryFile mxl(name + ".mxl",
                            fileContent("./test/xml_examples/unit_test/test_compressed_file.mxl"));
    for (const TemporaryFile* file : {&xml, &mxl}) {
        StdoutCapture quiet;
        Score score(file->path());
        EXPECT_GT(score.getNumNotes(), 0) << file->path();
        EXPECT_EQ(score.getFilePath(), file->path());
        EXPECT_EQ(score.getFileName(), name + file->path().substr(file->path().size() - 4));
    }
}

// An .mxl archive is recognised by its extension in any case: SCORE.MXL loads, and TEXT.MXL,
// which holds MusicXML text, is refused as an archive that is not a zip archive.
TEST(ScoreUnicodePath, AnUppercaseMxlExtensionIsReadAsAnArchive) {
    const TemporaryFile file("SCORE.MXL",
                             fileContent("./test/xml_examples/unit_test/test_compressed_file.mxl"));
    {
        StdoutCapture quiet;
        Score score(file.path());
        EXPECT_GT(score.getNumNotes(), 0);
    }
    const TemporaryFile text("TEXT.MXL",
                             fileContent("./test/xml_examples/unit_test/quarter_tone_tartini.xml"));
    EXPECT_EQ(thrownFirstLine([&text] { Score score(text.path()); }),
              "[maiacore] Score: '" + text.path() +
                  "' is not a readable MusicXML archive: it is not a zip archive");
}

// A zip archive is also recognised by the signature its bytes start with, "PK", whatever its name.
TEST(ScoreUnicodePath, AZipArchiveNamedXmlIsReadAsAnArchive) {
    const TemporaryFile file("score.xml",
                             fileContent("./test/xml_examples/unit_test/test_compressed_file.mxl"));
    StdoutCapture quiet;
    Score score(file.path());
    EXPECT_GT(score.getNumNotes(), 0);
}

// A title, a composer and a part name whose bytes are not UTF-8 are held with U+FFFD in place of
````


**`tests-cpp/src/score-collection-test.cpp` ~45: replace**

````cpp

    // Copies a score into the directory as 'name', which may name a subdirectory.
    void addCopy(const std::string& source, const std::string& name) const {
        std::filesystem::create_directories((_path / name).parent_path());
        std::filesystem::copy_file(source, _path / name);
    }
````

with

````cpp

    // The directory's path in UTF-8, as ScoreCollection takes it.
    std::string utf8() const { return _path.u8string(); }

    // Copies a score into the directory as 'name', UTF-8, which may name a subdirectory.
    void addCopy(const std::string& source, const std::string& name) const {
        const std::filesystem::path target = _path / std::filesystem::u8path(name);
        std::filesystem::create_directories(target.parent_path());
        std::filesystem::copy_file(source, target);
    }
````


**`tests-cpp/src/score-collection-test.cpp` ~128: replace**

````cpp
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
````

with

````cpp
    directory.addCopy(LAST_WINDOW, "sub/d.xml");
    const std::string path = directory.utf8();

    StdoutCapture quiet;
    EXPECT_EQ(fileNamesOf(ScoreCollection(path)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML"}));
    EXPECT_EQ(fileNamesOf(ScoreCollection(path, true)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML", "d.xml"}));
    EXPECT_EQ(fileNamesOf(ScoreCollection(std::vector<std::string>{path}, true)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML", "d.xml"}));
}

// Files whose names have characters outside every ANSI code page load, and keep their names and
// paths as UTF-8.
TEST(ScoreCollectionConstructor, ANonAsciiFileNameLoadsWithItsUtf8Name) {
    TemporaryDirectory directory;
    const std::string portuguese = "can\xC3\xA7\xC3\xA3o.xml";
    const std::string japanese = "\xE6\x97\xA5\xE6\x9C\xAC.xml";
    directory.addCopy(LAST_WINDOW, japanese);
    directory.addCopy(LAST_WINDOW, portuguese);

    StdoutCapture quiet;
    const ScoreCollection collection(directory.utf8());
    EXPECT_EQ(fileNamesOf(collection), (std::vector<std::string>{portuguese, japanese}));
    EXPECT_EQ(collection.getScores().at(1).getFilePath(),
              (directory.path() / std::filesystem::u8path(japanese)).u8string());
}
````

- [ ] **Step 2: Write the failing Python tests.** Apply, in order:

**`test/test_score_comprehensive.py` ~11: replace**

````python
import os
import subprocess
````

with

````python
import os
import shutil
import subprocess
````


**`test/test_score_comprehensive.py` ~359: replace**

````python
        self.assertEqual(correctionCodes(reread), [])

````

with

````python
        self.assertEqual(correctionCodes(reread), [])


class ScoreUnicodePathTestCase(unittest.TestCase):
    """Paths with characters outside the ANSI code page, and the console that cannot print them."""

    NAME = "can\u00e7\u00e3o \u65e5\u672c"

    def test_a_non_ascii_path_loads_and_keeps_its_name(self):
        sources = {
            ".xml": "./xml_examples/unit_test/quarter_tone_tartini.xml",
            ".mxl": "./xml_examples/unit_test/test_compressed_file.mxl",
        }
        with tempfile.TemporaryDirectory() as directory:
            for suffix, source in sources.items():
                with self.subTest(suffix=suffix):
                    path = os.path.join(directory, self.NAME + suffix)
                    shutil.copyfile(source, path)
                    with contextlib.redirect_stdout(io.StringIO()):
                        score = ml.Score(path)
                    self.assertGreater(score.getNumNotes(), 0)
                    self.assertEqual(score.getFileName(), self.NAME + suffix)
                    self.assertEqual(score.getFilePath(), path)

    def test_an_uppercase_mxl_extension_is_read_as_an_archive(self):
        """SCORE.MXL loads; TEXT.MXL, which holds MusicXML text, is refused as an archive."""
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "SCORE.MXL")
            shutil.copyfile("./xml_examples/unit_test/test_compressed_file.mxl", path)
            with contextlib.redirect_stdout(io.StringIO()):
                score = ml.Score(path)
            self.assertGreater(score.getNumNotes(), 0)
            text = os.path.join(directory, "TEXT.MXL")
            shutil.copyfile("./xml_examples/unit_test/quarter_tone_tartini.xml", text)
            with self.assertRaises(RuntimeError) as raised:
                ml.Score(text)
            self.assertEqual(
                str(raised.exception).splitlines()[0],
                f"[maiacore] Score: '{text}' is not a readable MusicXML archive: it is not a zip "
                "archive",
            )

    def test_a_zip_archive_named_xml_is_read_as_an_archive(self):
        """An archive is also recognised by the zip signature its bytes start with, "PK"."""
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "score.xml")
            shutil.copyfile("./xml_examples/unit_test/test_compressed_file.mxl", path)
            with contextlib.redirect_stdout(io.StringIO()):
                score = ml.Score(path)
            self.assertGreater(score.getNumNotes(), 0)

    def test_a_name_the_console_cannot_encode_does_not_end_the_interpreter(self):
        """Run in a child process whose standard output is ASCII: the summary line names the file,
        which the stream cannot encode; it is written with backslash escapes."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            "    path = os.path.join(directory, '\\u65e5\\u672c.xml')\n"
            "    shutil.copyfile('./xml_examples/unit_test/"
            "unrepresentable_alter_near_quarter_tone.xml', path)\n"
            "    score = ml.Score(path)\n"
            "    print('RESULT', ascii(score.getFileName()))\n"
        )
        completed = subprocess.run(
            [sys.executable, "-c", code],
            cwd=os.path.dirname(os.path.abspath(__file__)),
            capture_output=True,
            encoding="ascii",
            errors="replace",
            env={**os.environ, "PYTHONIOENCODING": "ascii"},
            timeout=60,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        self.assertEqual(resultLine(completed), "RESULT '\\u65e5\\u672c.xml'")
        self.assertIn(
            "[maiacore] \\u65e5\\u672c.xml: 1 corrections, 0 element types not modelled "
            "(dropped on export); see Score.getImportIssues()",
            completed.stdout,
        )

````


**`test/test_score_collection.py` ~44: replace**

````python

def runChild(code):
    """Run the Python source 'code' in a child process, from this directory, so that a crash of
    the interpreter shows as the child's exit code instead of ending the test run."""
    return subprocess.run(
        [sys.executable, "-c", code],
        cwd=HERE,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        timeout=120,
    )
````

with

````python

def runChild(code, environment=None):
    """Run the Python source 'code' in a child process, from this directory, so that a crash of
    the interpreter shows as the child's exit code instead of ending the test run; 'environment'
    adds variables to the child's environment."""
    return subprocess.run(
        [sys.executable, "-c", code],
        cwd=HERE,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        timeout=120,
        env={**os.environ, **(environment or {})},
    )
````


**`test/test_score_collection.py` ~86: replace**

````python
        restore()


def fileNamesAreUtf8():
    """Whether Score keeps a file name as UTF-8. It keeps the name in the ANSI code page, which
    is UTF-8 everywhere except on Windows with a code page other than 65001."""
    if os.name != "nt":
        return True
    import ctypes

    return ctypes.windll.kernel32.GetACP() == 65001

````

with

````python
        restore()

````


**`test/test_score_collection.py` ~233: replace**

````python

    def test_a_non_ascii_file_name_does_not_crash_the_interpreter(self):
        """Run in a child process: logging the name of a file such as 'canção.xml'
        must not end the interpreter. The file either loads or raises RuntimeError.

        A search of the loaded collection builds its fileName column from the name in the ANSI
        code page: it succeeds where that is UTF-8, and raises UnicodeDecodeError on Windows
        with another code page. This pins the current behaviour of non-ASCII paths, which are
        not supported yet, so that a change to it is noticed."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            f"    shutil.copyfile({LAST_WINDOW!r}, os.path.join(directory, 'can\u00e7\u00e3o.xml'))\n"
            "    try:\n"
            "        collection = ml.ScoreCollection(directory)\n"
            "        print('RESULT loaded', collection.getNumScores())\n"
            "    except RuntimeError:\n"
            "        print('RESULT RuntimeError')\n"
            "    else:\n"
            "        try:\n"
            "            collection.findMelodyPatternDataFrame([ml.Note('C4'), ml.Note('D4')])\n"
            "            print('RESULT searched')\n"
            "        except UnicodeDecodeError:\n"
            "            print('RESULT UnicodeDecodeError')\n"
        )
        completed = runChild(code)
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        search = "RESULT searched" if fileNamesAreUtf8() else "RESULT UnicodeDecodeError"
        self.assertIn(results, (["RESULT loaded 1", search], ["RESULT RuntimeError"]))

````

with

````python

    def test_non_ascii_file_names_load_with_their_utf8_names(self):
        """Run in a child process whose standard output is ASCII: 'canção.xml' and '日本.xml'
        load, keep their names as UTF-8, and name the search's rows; the "Loading:" lines that
        name them are written with backslash escapes."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            "    for name in ('can\\u00e7\\u00e3o.xml', '\\u65e5\\u672c.xml'):\n"
            f"        shutil.copyfile({LAST_WINDOW!r}, os.path.join(directory, name))\n"
            "    collection = ml.ScoreCollection(directory)\n"
            "    table = collection.findMelodyPatternDataFrame([ml.Note('C4'), ml.Note('D4')])\n"
            "    names = [score.getFileName() for score in collection.getScores()]\n"
            "    print('RESULT', ascii(names), ascii(sorted(set(table['fileName']))))\n"
        )
        completed = runChild(code, {"PYTHONIOENCODING": "ascii"})
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        names = "['can\\xe7\\xe3o.xml', '\\u65e5\\u672c.xml']"
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, [f"RESULT {names} {names}"])
        self.assertIn("Loading: \\u65e5\\u672c.xml", completed.stdout)

````


**`test/test_musicxml_corpus.py` ~384: replace**

````python
    def test_a_file_whose_path_is_not_ascii_has_the_record_of_its_content(self):
        # maialib cannot open a path with a character outside ASCII on Windows, where these
        # loads succeed only through the worker's ASCII-named copy. On Linux maialib opens such a
        # path itself, so there the test passes with or without the copy.
        ledger = corpus.load_ledger(corpus.LEDGER)
        environmental = entries_a_bare_process_leaves()
        for name in LOADABLE_FILES:
            with self.subTest(name=name), tempfile.TemporaryDirectory() as folder:
                path = Path(folder) / ("partitura_\u00e9" + Path(name).suffix)
                shutil.copyfile(str(corpus.REPO_ROOT / name), str(path))
````

with

````python
    def test_a_file_whose_path_is_not_ascii_has_the_record_of_its_content(self):
        # maialib opens such a path itself on every platform, a name outside the Windows ANSI
        # code page included; the worker loads the file from its own path.
        ledger = corpus.load_ledger(corpus.LEDGER)
        environmental = entries_a_bare_process_leaves()
        for name in LOADABLE_FILES:
            with self.subTest(name=name), tempfile.TemporaryDirectory() as folder:
                path = Path(folder) / ("partitura_\u00e9_\u65e5\u672c" + Path(name).suffix)
                shutil.copyfile(str(corpus.REPO_ROOT / name), str(path))
````


**`test/test_musicxml_corpus.py` ~405: replace**

````python
                # The worker ends without an error and leaves none of its own entries in its
                # temporary directory, where it made the copy; entries that any new process gets
                # there from its environment are not the worker's.
                self.assertEqual(0, done.returncode, done.stderr.decode("utf-8", "replace"))
````

with

````python
                # The worker ends without an error and leaves none of its own entries in its
                # temporary directory; entries that any new process gets there from its
                # environment are not the worker's.
                self.assertEqual(0, done.returncode, done.stderr.decode("utf-8", "replace"))
````

- [ ] **Step 3: Run them and see them fail.** C++ subset `ScoreUnicodePath*:ScoreCollectionConstructor*` → on Windows `ANonAsciiPathLoadsAndKeepsItsNameAsUtf8` fails (`[maiacore] Score: cannot open '...\canção 日本.xml'`), `ANonAsciiFileNameLoadsWithItsUtf8Name` fails (`std::system_error` from `file.string()` for `日本.xml`, or `cannot open`), and on every platform `AnUppercaseMxlExtensionIsReadAsAnArchive` and `AZipArchiveNamedXmlIsReadAsAnArchive` fail (`is not well-formed XML`). «pytest» `test_score_comprehensive.ScoreUnicodePathTestCase test_score_collection.ScoreCollectionConstructionTestCase test_musicxml_corpus.WorkerProcessTestCase` → on Windows the non-ASCII tests fail (`cannot open`; the collection child exits 1 with `UnicodeDecodeError`), `test_an_uppercase_mxl_extension_is_read_as_an_archive` and `test_a_zip_archive_named_xml_is_read_as_an_archive` fail everywhere, and `test_a_name_the_console_cannot_encode_does_not_end_the_interpreter` fails (`RuntimeError` on Windows; once the path opens, `3221226505 != 0`).

- [ ] **Step 4: Opening by UTF-8 path.** Apply, in order:

**`maiacore/src/maiacore/score.cpp` ~4: replace**

````cpp
#include <atomic>
#include <cstdint>
````

with

````cpp
#include <atomic>
#include <cctype>
#include <cstdint>
````


**`maiacore/src/maiacore/score.cpp` ~127: replace**

````cpp

// The bytes of the file at 'path', or std::nullopt when it cannot be opened.
std::optional<std::vector<unsigned char>> fileBytes(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
````

with

````cpp

// The bytes of the file at 'path', a UTF-8 path, or std::nullopt when it cannot be opened -- a
// path that is not UTF-8 included. The file is opened through std::filesystem::path, which is a
// wide-character path on Windows, so the ANSI code page does not limit the names it can open.
std::optional<std::vector<unsigned char>> fileBytes(const std::string& path) {
    std::filesystem::path file;
    try {
        file = std::filesystem::u8path(path);
    } catch (const std::exception&) {
        return std::nullopt;
    }
    std::ifstream stream(file, std::ios::binary);
    if (!stream) {
````


**`maiacore/src/maiacore/score.cpp` ~789: replace**

````cpp

    const std::string fileExtension = filePath.substr(filePath.size() - 3, filePath.size());

    // The path as messages quote it.
    const std::string shownPath = "'" + maiacore::detail::validUtf8(filePath) + "'";
    const std::optional<std::vector<unsigned char>> bytes = fileBytes(filePath);
    if (!bytes) {
        LOG_ERROR("Score: cannot open " + shownPath);
    }

    if (fileExtension == "mxl") {
        const auto [document, rootfile] = mxlRootfile(*bytes, shownPath);
````

with

````cpp

    // The extension, in lowercase.
    std::string extension = filePath.substr(filePath.size() - 4);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // The path as messages quote it.
    const std::string shownPath = "'" + maiacore::detail::validUtf8(filePath) + "'";
    const std::optional<std::vector<unsigned char>> bytes = fileBytes(filePath);
    if (!bytes) {
        LOG_ERROR("Score: cannot open " + shownPath);
    }

    // A file named .mxl, in any case, is read as an archive, and so is any file that starts with
    // the zip signature "PK"; any other file is read as MusicXML text.
    const bool zipSignature = bytes->size() >= 2 && (*bytes)[0] == 'P' && (*bytes)[1] == 'K';
    if (extension == ".mxl" || zipSignature) {
        const auto [document, rootfile] = mxlRootfile(*bytes, shownPath);
````


**`maiacore/src/maiacore/score_collection.cpp` ~11: replace**

````cpp

#include "maiacore/log.h"
````

with

````cpp

#include "import-report.h"
#include "maiacore/log.h"
````


**`maiacore/src/maiacore/score_collection.cpp` ~44: replace**

````cpp

// The MusicXML files of 'directory' (and of its subdirectories at any depth when 'recursive'),
// in sorted path order. A subdirectory the user has no permission to read is skipped; 'directory'
// itself must be readable. Every failure is reported in English, naming the directory that could
// not be read -- the caller's own string for 'directory', the UTF-8 form of a subdirectory's
// path: the messages of std::filesystem's own exceptions are localised, and Python cannot always
// decode them.
template <typename Iterator>
std::vector<std::filesystem::path> musicXMLFilesOf(const std::string& directory) {
    const std::filesystem::path directoryPath(directory);
    std::error_code error;
    // Opened once without skip_permission_denied, which would make a 'directory' the user cannot
    // read look empty.
    const std::filesystem::directory_iterator probe(directoryPath, error);
    if (error) {
        LOG_ERROR("ScoreCollection: cannot read the directory '" + directory + "'");
    }

    std::vector<std::filesystem::path> files;
    std::string failed = directory;
    for (Iterator it(directory, std::filesystem::directory_options::skip_permission_denied, error);
         !error && it != Iterator(); it.increment(error)) {
        std::error_code typeError;
        if (it->is_regular_file(typeError) && isMusicXMLFile(it->path())) {
            files.push_back(it->path());
        }
        const std::filesystem::path next = readNext(it);
        failed = (next == directoryPath) ? directory : next.u8string();
    }
````

with

````cpp

// The MusicXML files of 'directory', a UTF-8 path (and of its subdirectories at any depth when
// 'recursive'), in sorted path order. A subdirectory the user has no permission to read is
// skipped; 'directory' itself must be readable. Every failure is reported in English, naming the
// directory that could not be read -- the caller's own string for 'directory', the UTF-8 form of
// a subdirectory's path: the messages of std::filesystem's own exceptions are localised, and
// Python cannot always decode them.
template <typename Iterator>
std::vector<std::filesystem::path> musicXMLFilesOf(const std::string& directory) {
    const std::filesystem::path directoryPath = std::filesystem::u8path(directory);
    std::error_code error;
    // Opened once without skip_permission_denied, which would make a 'directory' the user cannot
    // read look empty.
    const std::filesystem::directory_iterator probe(directoryPath, error);
    if (error) {
        LOG_ERROR("ScoreCollection: cannot read the directory '" +
                  maiacore::detail::validUtf8(directory) + "'");
    }

    std::vector<std::filesystem::path> files;
    std::string failed = maiacore::detail::validUtf8(directory);
    for (Iterator it(directoryPath, std::filesystem::directory_options::skip_permission_denied,
                     error);
         !error && it != Iterator(); it.increment(error)) {
        std::error_code typeError;
        if (it->is_regular_file(typeError) && isMusicXMLFile(it->path())) {
            files.push_back(it->path());
        }
        const std::filesystem::path next = readNext(it);
        failed = (next == directoryPath) ? maiacore::detail::validUtf8(directory) : next.u8string();
    }
````


**`maiacore/src/maiacore/score_collection.cpp` ~81: replace**

````cpp
                                                 const bool recursive) {
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        LOG_ERROR("ScoreCollection: '" + directory + "' is not a directory, or does not exist");
    }
````

with

````cpp
                                                 const bool recursive) {
    // A path that is not UTF-8 names no directory.
    bool isDirectory = false;
    try {
        std::error_code error;
        isDirectory = std::filesystem::is_directory(std::filesystem::u8path(directory), error);
    } catch (const std::exception&) {
        isDirectory = false;
    }
    if (!isDirectory) {
        LOG_ERROR("ScoreCollection: '" + maiacore::detail::validUtf8(directory) +
                  "' is not a directory, or does not exist");
    }
````


**`maiacore/src/maiacore/score_collection.cpp` ~134: replace**

````cpp
            try {
                scores.emplace_back(file.string());
            } catch (const std::exception& loadError) {
````

with

````cpp
            try {
                scores.emplace_back(file.u8string());
            } catch (const std::exception& loadError) {
````

- [ ] **Step 5: The console guard and the bindings.** Apply, in order:

**Create `maiacore/src/maiacore/python_wrapper/py_console.h`**

````cpp
#pragma once

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>

#include <iostream>

namespace maiacore_python {

namespace py = pybind11;

// Python's sys.stdout or sys.stderr ('name') behind a writer that does not fail on a character
// the stream cannot encode, such as a file name in Japanese on a stream in the Windows ANSI code
// page: that text is written with backslash escapes instead. pybind11's redirection writes the
// C++ output to the stream from a destructor too, where an exception ends the process.
inline py::object tolerantStream(const char* name) {
    const py::object stream = py::module_::import("sys").attr(name);
    const py::object write = stream.attr("write");
    const py::cpp_function tolerantWrite([stream, write](const py::str& text) {
        try {
            write(text);
        } catch (py::error_already_set& error) {
            if (!error.matches(PyExc_UnicodeEncodeError)) {
                throw;
            }
            const py::object encoding = py::getattr(stream, "encoding", py::none());
            const py::str codec = encoding.is_none() ? py::str("ascii") : py::str(encoding);
            write(text.attr("encode")(codec, "backslashreplace").attr("decode")(codec));
        }
    });
    return py::module_::import("types").attr("SimpleNamespace")(
        py::arg("write") = tolerantWrite, py::arg("flush") = stream.attr("flush"));
}

// The call guard of the bindings that load scores: while the call runs, std::cout and std::cerr
// go to Python's sys.stdout and sys.stderr through tolerantStream().
class ConsoleRedirect {
   public:
    ConsoleRedirect()
        : _out(std::cout, tolerantStream("stdout")), _err(std::cerr, tolerantStream("stderr")) {}

   private:
    py::scoped_ostream_redirect _out;
    py::scoped_ostream_redirect _err;
};

}  // namespace maiacore_python
````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~14: replace**

````cpp
#include "nlohmann/json.hpp"
#include "py_melody_dataframe.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
using namespace pybind11::literals;
using maiacore_python::MelodyDataFrame;
````

with

````cpp
#include "nlohmann/json.hpp"
#include "py_console.h"
#include "py_melody_dataframe.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
using namespace pybind11::literals;
using maiacore_python::ConsoleRedirect;
using maiacore_python::MelodyDataFrame;
````


**`maiacore/src/maiacore/python_wrapper/py_score.cpp` ~110: replace**

````cpp

    cls.def(py::init<const std::string&>(), py::arg("filePath"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Load a score from a MusicXML file (``.xml``, ``.musicxml`` or compressed ``.mxl``).

````

with

````cpp

    cls.def(py::init<const std::string&>(), py::arg("filePath"), py::call_guard<ConsoleRedirect>(),
            R"pbdoc(
        Load a score from a MusicXML file (``.xml``, ``.musicxml`` or compressed ``.mxl``, the
        extension in any case; a file that starts with the zip signature ``PK`` is read as an
        archive whatever its name). Any path opens on every platform, whatever its characters, and
        ``getFileName()`` and ``getFilePath()`` return it as given. What the load prints goes to
        ``sys.stdout``; a character the stream cannot encode is written as a backslash escape.

````


**`maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` ~12: replace**

````cpp
#include "maiacore/score_collection.h"
#include "py_melody_dataframe.h"

namespace py = pybind11;
using namespace pybind11::literals;
using maiacore_python::MelodyDataFrame;
````

with

````cpp
#include "maiacore/score_collection.h"
#include "py_console.h"
#include "py_melody_dataframe.h"

namespace py = pybind11;
using namespace pybind11::literals;
using maiacore_python::ConsoleRedirect;
using maiacore_python::MelodyDataFrame;
````


**`maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` ~28: replace**

````cpp
    cls.def(py::init<const std::string&, const bool>(), py::arg("directoryPath"),
            py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
````

with

````cpp
    cls.def(py::init<const std::string&, const bool>(), py::arg("directoryPath"),
            py::arg("recursive") = false, py::call_guard<ConsoleRedirect>(),
            R"pbdoc(
````


**`maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` ~48: replace**

````cpp
    cls.def(py::init<const std::vector<std::string>&, const bool>(), py::arg("directoriesPaths"),
            py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
````

with

````cpp
    cls.def(py::init<const std::vector<std::string>&, const bool>(), py::arg("directoriesPaths"),
            py::arg("recursive") = false, py::call_guard<ConsoleRedirect>(),
            R"pbdoc(
````


**`maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` ~70: replace**

````cpp
            py::arg("directoriesPaths"), py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Replace the collection's directories and scores with those of the given directories.

        Every file whose extension is ``.xml``, ``.mxl`` or ``.musicxml``, compared without
        regard to case, is loaded, directory by directory in the given order and, within a
        directory, in sorted path order; subdirectories only when ``recursive`` is True. Every
        path is checked before anything is loaded, and the collection changes only when every
        file has loaded. Scores added with ``addScore`` are replaced too. A subdirectory the user
        has no permission to read is skipped.

````

with

````cpp
            py::arg("directoriesPaths"), py::arg("recursive") = false,
            py::call_guard<ConsoleRedirect>(),
            R"pbdoc(
        Replace the collection's directories and scores with those of the given directories.

        Every file whose extension is ``.xml``, ``.mxl`` or ``.musicxml``, compared without
        regard to case, whatever the characters of its name, is loaded, directory by directory
        in the given order and, within a directory, in sorted path order; subdirectories only
        when ``recursive`` is True. Every path is checked before anything is loaded, and the
        collection changes only when every file has loaded. Scores added with ``addScore`` are
        replaced too. A subdirectory the user has no permission to read is skipped.

````

- [ ] **Step 6: The Doxygen.** Apply, in order:

**`maiacore/include/maiacore/score.h` ~101: replace**

````cpp
     * @brief Constructs a new Score object by loading a MusicXML file.
     * @details Supported formats: *.xml, *.musicxml, *.mxl (compressed).
     *
````

with

````cpp
     * @brief Constructs a new Score object by loading a MusicXML file.
     * @details Supported formats: *.xml, *.musicxml, *.mxl (compressed; the extension in any
     *          case, or any file that starts with the zip signature "PK"). The path is UTF-8, and
     *          any name opens on every platform: on Windows the file is opened through a
     *          wide-character path, whatever the ANSI code page.
     *
````


**`maiacore/include/maiacore/score.h` ~159: replace**

````cpp
     *          the number of such elements in the file as `found`.
     * @param filePath Path to the MusicXML file.
     * @throws std::runtime_error If the path is too short to name a file; the file cannot be
````

with

````cpp
     *          the number of such elements in the file as `found`.
     * @param filePath Path to the MusicXML file, in UTF-8.
     * @throws std::runtime_error If the path is too short to name a file; the file cannot be
````


**`maiacore/include/maiacore/score.h` ~362: replace**

````cpp
    /**
     * @brief Returns the file path of the loaded MusicXML file.
     * @return File path string.
     */
    std::string getFilePath() const;

    /**
     * @brief Returns the file name of the loaded MusicXML file.
     * @return File name string.
````

with

````cpp
    /**
     * @brief Returns the file path of the loaded MusicXML file, as it was given (UTF-8).
     * @return File path string.
     */
    std::string getFilePath() const;

    /**
     * @brief Returns the file name of the loaded MusicXML file: its path after the last '/' or
     *        '\\' (UTF-8).
     * @return File name string.
````

- [ ] **Step 7: The corpus worker loads every file from its own path.** Apply, in order:

**`test/musicxml/corpus_worker.py` ~7: replace**

````python
crash or hang to the first stage still pending. The stages, in order: input (the file itself
against the MusicXML 4.0 schema), load (maialib.Score, from an ASCII-named copy of the file when
its path is not ASCII), analyses (only with --analyses: chords and the intervals between
consecutive notes), export (Score.toXML), the export's checks (export_xml: well-formed;
export_xsd: the schema; export_errors: the semantic errors), and roundtrip (the export loaded and
exported again, compared without its encoding date). A stage that cannot run because of an
earlier result is "n/a".
"""

from __future__ import annotations

import contextlib
import io
import json
import re
import shutil
import sys
import tempfile
from pathlib import Path
from typing import Any, Dict, Iterator

````

with

````python
crash or hang to the first stage still pending. The stages, in order: input (the file itself
against the MusicXML 4.0 schema), load (maialib.Score, from the file's own path), analyses (only
with --analyses: chords and the intervals between consecutive notes), export (Score.toXML), the
export's checks (export_xml: well-formed; export_xsd: the schema; export_errors: the semantic
errors), and roundtrip (the export loaded and exported again, compared without its encoding
date). A stage that cannot run because of an earlier result is "n/a".
"""

from __future__ import annotations

import io
import json
import re
import sys
import tempfile
from pathlib import Path
from typing import Any, Dict

````


**`test/musicxml/corpus_worker.py` ~110: replace**

````python

@contextlib.contextmanager
def loadable_path(path: Path) -> Iterator[Path]:
    """``path`` when it is ASCII; otherwise an ASCII-named temporary copy of the file, removed when
    the with block ends."""
    if str(path).isascii():
        yield path
        return
    # maialib cannot open a non-ASCII path on Windows; the copy keeps the ledger about the
    # MusicXML content. It keeps the suffix, from which maialib decides how to read the file.
    with tempfile.TemporaryDirectory() as folder:
        copy = Path(folder) / ("score" + path.suffix)
        shutil.copyfile(str(path), str(copy))
        yield copy


def examine(path: Path, analyses: bool) -> None:
    record = new_record(analyses)
    record["input"] = input_status(path)
    emit(record)
    # The copy, if any, lasts until the record is final: longer than maialib reads the file.
    with loadable_path(path) as source:
        examine_with_maialib(source, record, analyses)

````

with

````python

def examine(path: Path, analyses: bool) -> None:
    record = new_record(analyses)
    record["input"] = input_status(path)
    emit(record)
    examine_with_maialib(path, record, analyses)

````


**`test/musicxml/README.md` ~41: replace**

````markdown

A file whose path has a character outside ASCII is loaded by maialib from an ASCII-named temporary
copy, because maialib cannot open such a path on Windows: the ledger records what maialib makes of
the MusicXML content, whatever the file's name. The validator reads the file itself.

````

with

````markdown

maialib and the validator read every file from its own path, whatever characters it holds: the
ledger records what maialib does with the real file, on every platform.

````

- [ ] **Step 8: Format, build, pass.** clang-format `score.cpp`, `score_collection.cpp`, `score.h`, `py_console.h`, `py_score.cpp`, `py_score_collection.cpp`, `score-test.cpp`, `score-collection-test.cpp`. C++ subset `ScoreUnicodePath*:ScoreCollection*:ScoreFatalInput*` → all pass. «build» `make "PYTHON=$py" dev` → 0. «pytest» `test_score_comprehensive.ScoreUnicodePathTestCase test_score_collection test_musicxml_corpus.WorkerProcessTestCase` → OK. `ruff check` and `ruff format --check` on `test/musicxml/corpus_worker.py test/test_musicxml_corpus.py test/test_score_collection.py` → clean.

- [ ] **Step 9: Mutations.** (a) In `fileBytes` replace `std::ifstream stream(file, std::ios::binary);` with `std::ifstream stream(path, std::ios::binary);` → on Windows `ScoreUnicodePath.ANonAsciiPathLoadsAndKeepsItsNameAsUtf8` and `ScoreCollectionConstructor.ANonAsciiFileNameLoadsWithItsUtf8Name` fail with `cannot open` (measured), and after `make dev` the Python non-ASCII tests. (b) Delete the `std::transform(extension.begin(), ...` statement → `AnUppercaseMxlExtensionIsReadAsAnArchive` and `test_an_uppercase_mxl_extension_is_read_as_an_archive` fail (`TEXT.MXL`, which does not start with `PK`, is read as MusicXML text and loads; `SCORE.MXL` still loads by its signature). (c) In `py_console.h` replace `if (!error.matches(PyExc_UnicodeEncodeError)) {` with `if (true) {` → after `make dev`, `test_a_name_the_console_cannot_encode_does_not_end_the_interpreter` and `test_non_ascii_file_names_load_with_their_utf8_names` fail with `3221226505 != 0` (measured). (d) In `setDirectoriesPaths` replace `scores.emplace_back(file.u8string());` with `scores.emplace_back(file.string());` → on Windows `ANonAsciiFileNameLoadsWithItsUtf8Name` fails (`std::system_error` for `日本.xml`). (e) Replace `if (extension == ".mxl" || zipSignature) {` with `if (extension == ".mxl") {` → `AZipArchiveNamedXmlIsReadAsAnArchive` and, after `make dev`, `test_a_zip_archive_named_xml_is_read_as_an_archive` fail (`is not well-formed XML`). (f) Replace it with `if (zipSignature) {` → `AnUppercaseMxlExtensionIsReadAsAnArchive`, `ScoreFatalInput.AnMxlThatIsNotAZipArchiveIsNamed` and, after `make dev`, `test_an_uppercase_mxl_extension_is_read_as_an_archive` and `test_an_mxl_that_is_not_a_zip_archive_is_named` fail (a file named `.mxl` that holds text is read as MusicXML, and the expected `RuntimeError` is not the one raised). Revert each; rerun green (rebuild the module after (c), (e) and (f)).

- [ ] **Step 10: Whole suites and the corpus.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1214 tests.` (1210 + 4). «build» `make "PYTHON=$py" py-tests` → OK, `Ran 709 tests` (705 + 4). «build» `make "PYTHON=$py" validate` → no new findings. «build» `make "PYTHON=$py" corpus; $LASTEXITCODE` (the external corpus fetched) → 0: neither ledger differs. The 750 external files with non-ASCII paths, loaded from their own paths now, give the records their ASCII copies gave (measured on a scratch copy: 60 of them chosen at random, every record identical); if a line differs, stop and report it with the file.

- [ ] **Step 11: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/src/maiacore/score.cpp maiacore/src/maiacore/score_collection.cpp maiacore/include/maiacore/score.h maiacore/src/maiacore/python_wrapper/py_console.h maiacore/src/maiacore/python_wrapper/py_score.cpp maiacore/src/maiacore/python_wrapper/py_score_collection.cpp tests-cpp/src/score-test.cpp tests-cpp/src/score-collection-test.cpp test/test_score_comprehensive.py test/test_score_collection.py test/test_musicxml_corpus.py test/musicxml/corpus_worker.py test/musicxml/README.md`, message:

```
fix: any path opens on Windows and stays UTF-8; the console cannot end a load

Score(path) handed a UTF-8 path to narrow-character file functions,
which Windows reads in the ANSI code page: "canção.xml" did not open.
Files are now opened through std::filesystem::u8path, a wide-character
path on Windows, and parsed from the bytes read; an .mxl archive is
recognised by its extension in any case ("SCORE.MXL" was parsed as XML)
and by the zip signature its bytes start with, "PK", so a zip archive
named .xml loads; a file named .mxl that is not a zip archive raises.
ScoreCollection builds its paths the same way and passes UTF-8 to
Score: getFileName() and getFilePath() raised UnicodeDecodeError for a
name such as "canção.xml", and a name outside the code page, such as
"日本.xml", made the constructor raise.

pybind11's redirection writes C++ output to sys.stdout from a
destructor, where a UnicodeEncodeError ends the process: a summary line
naming "日本.xml" did that whenever the output was piped or redirected.
The loading bindings write through a stream that escapes what the
console cannot encode.

The corpus worker loads every file from its own path; its ASCII-named
copy is gone. Both ledgers are unchanged: make corpus, the external
corpus included, gives every non-ASCII file the record its copy gave.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 6: `ScoreCollection` skips and lists the files that fail

**Files:**
- Modify: `maiacore/include/maiacore/score_collection.h` (`<utility>`; `setDirectoriesPaths` Doxygen ~68; `addScore` ×2 and the new `getLoadErrors` ~95; member ~227), `maiacore/src/maiacore/score_collection.cpp` (`<iostream>`; `loadInto`, `printLoadErrors` before `appendRows` ~100; `setDirectoriesPaths`, `getLoadErrors`, `addScore` ~139-180), `maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` (`setDirectoriesPaths` numpydoc ~77-92; the three `addScore` and `getLoadErrors`)
- Test: `tests-cpp/src/score-collection-test.cpp` (`AFileThatFailsToLoadChangesNothing` ~218 replaced by three tests), `test/test_score_collection.py` (`import io`; `test_a_file_that_fails_to_load_raises_naming_it_and_changes_nothing` ~178 replaced by three tests)

**Interfaces:**
- Consumes: `validUtf8` (Task 1), `ConsoleRedirect` (Task 5), `TemporaryDirectory::utf8()`, `addCopy` (Task 5), `fileNamesOf`, `LAST_WINDOW`, `DUPLICATES` (`score-collection-test.cpp`).
- Produces: `std::vector<std::pair<std::string, std::string>> ScoreCollection::getLoadErrors() const`; member `_loadErrors`; file-local `void loadInto(const std::string& path, std::vector<Score>& scores, std::vector<std::pair<std::string, std::string>>& errors)` and `void printLoadErrors(const std::vector<std::pair<std::string, std::string>>& errors, size_t numFiles)`; `addScore(path)` delegates to `addScore(std::vector<std::string>{path})`. Python: `ScoreCollection.getLoadErrors() -> list[tuple[str, str]]`; the three `addScore` use `ConsoleRedirect`. Test helpers `notXml(path)`, `failedLine(failed, total)`, `LoadErrors` in `score-collection-test.cpp`.

- [ ] **Step 1: Write the failing C++ tests.** Apply:

**`tests-cpp/src/score-collection-test.cpp` ~218: replace**

````cpp

// A file that fails to load, after one that loads, leaves the collection as it was too.
TEST(ScoreCollectionDirectories, AFileThatFailsToLoadChangesNothing) {
    TemporaryDirectory directory;
    directory.addCopy(LAST_WINDOW, "a.xml");
    std::ofstream(directory.path() / "b.xml") << "not a score";
    ScoreCollection collection(BACH_DIR);

    {
        StdoutCapture quiet;
        EXPECT_THROW(collection.setDirectoriesPaths({directory.path().string()}),
                     std::runtime_error);
    }

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{BACH_DIR}));
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"cello_suite_1_violin.xml", "prelude_1_BWV_846.xml"}));
}
````

with

````cpp

namespace {
// The first line of the error that loading a file that is not XML, 'not a score', raises.
std::string notXml(const std::string& path) {
    return "[maiacore] Score: '" + path +
           "' is not well-formed XML: No document element found (byte offset 11)";
}

// The line a load prints when 'failed' of its 'total' files fail.
std::string failedLine(const int failed, const int total) {
    return "[maiacore] ScoreCollection: " + std::to_string(failed) + " of " +
           std::to_string(total) + " files failed to load; see ScoreCollection.getLoadErrors()\n";
}

typedef std::vector<std::pair<std::string, std::string>> LoadErrors;
}  // namespace

// A file that fails to load is skipped and listed with the first line of its error; the files
// that load replace the collection's scores, and one line says how many failed. A load in which
// every file loads empties the list.
TEST(ScoreCollectionDirectories, AFileThatFailsToLoadIsSkippedAndListed) {
    TemporaryDirectory directory;
    directory.addCopy(LAST_WINDOW, "a.xml");
    std::ofstream(directory.path() / "b.xml") << "not a score";
    const std::string broken = (directory.path() / "b.xml").u8string();
    ScoreCollection collection(BACH_DIR);
    EXPECT_EQ(collection.getLoadErrors(), LoadErrors{});

    {
        StdoutCapture capture;
        collection.setDirectoriesPaths({directory.utf8()});
        EXPECT_NE(capture.str().find(failedLine(1, 2)), std::string::npos) << capture.str();
    }

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{directory.utf8()}));
    EXPECT_EQ(fileNamesOf(collection), (std::vector<std::string>{"a.xml"}));
    EXPECT_EQ(collection.getLoadErrors(), (LoadErrors{{broken, notXml(broken)}}));

    StdoutCapture quiet;
    collection.setDirectoriesPaths({BACH_DIR});
    EXPECT_EQ(collection.getLoadErrors(), LoadErrors{});
}

// addScore() with a path, or with several, skips each file that fails to load and lists it; the
// list is that of the last load. addScore() with a Score, which loads nothing, leaves it.
TEST(ScoreCollectionScores, AddScoreSkipsAndListsTheFilesThatFailToLoad) {
    ScoreCollection collection;
    StdoutCapture capture;

    collection.addScore("./missing.xml");
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_EQ(collection.getLoadErrors(),
              (LoadErrors{{"./missing.xml", "[maiacore] Score: cannot open './missing.xml'"}}));
    EXPECT_EQ(capture.str(), failedLine(1, 1));

    collection.addScore(std::vector<std::string>{LAST_WINDOW, "./missing.xml", DUPLICATES});
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"melody_last_window.musicxml",
                                        "melody_duplicate_patterns.musicxml"}));
    EXPECT_EQ(collection.getLoadErrors(),
              (LoadErrors{{"./missing.xml", "[maiacore] Score: cannot open './missing.xml'"}}));

    collection.addScore(Score({"Piano"}, 1));
    EXPECT_EQ(collection.getLoadErrors().size(), 1u);
    collection.addScore(LAST_WINDOW);
    EXPECT_EQ(collection.getLoadErrors(), LoadErrors{});
    EXPECT_EQ(collection.getNumScores(), 4);
}

// A directory whose name and files' names are outside every ANSI code page: its scores load, and
// the paths of the scores and of the files that fail are UTF-8.
TEST(ScoreCollectionDirectories, ANonAsciiDirectoryListsItsFailuresInUtf8) {
    TemporaryDirectory directory;
    const std::string music = "m\xC3\xBAsicas";
    const std::string japanese = "\xE6\x97\xA5\xE6\x9C\xAC.xml";
    directory.addCopy(LAST_WINDOW, music + "/" + japanese);
    directory.addCopy(LAST_WINDOW, music + "/ruim.xml");
    const std::filesystem::path folder = directory.path() / std::filesystem::u8path(music);
    std::ofstream(folder / "ruim.xml", std::ios::trunc) << "not a score";
    const std::string broken = (folder / "ruim.xml").u8string();

    StdoutCapture quiet;
    const ScoreCollection collection(folder.u8string());
    EXPECT_EQ(fileNamesOf(collection), (std::vector<std::string>{japanese}));
    EXPECT_EQ(collection.getScores().at(0).getFilePath(),
              (folder / std::filesystem::u8path(japanese)).u8string());
    EXPECT_EQ(collection.getLoadErrors(), (LoadErrors{{broken, notXml(broken)}}));
}
````

- [ ] **Step 2: Write the failing Python tests.** Apply, in order:

**`test/test_score_collection.py` ~4: replace**

````python
import getpass
import os
````

with

````python
import getpass
import io
import os
````


**`test/test_score_collection.py` ~178: replace**

````python

    def test_a_file_that_fails_to_load_raises_naming_it_and_changes_nothing(self):
        with tempfile.TemporaryDirectory() as directory:
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))
            broken = os.path.join(directory, "b.xml")
            with open(broken, "w", encoding="utf-8") as text:
                text.write("not a score")
            collection = ml.ScoreCollection()
            collection.addScore(DUPLICATES)

            with self.assertRaises(RuntimeError) as context:
                collection.setDirectoriesPaths([directory])
            self.assertTrue(
                str(context.exception).startswith(f"{broken}: [maiacore] "),
                str(context.exception).splitlines()[0],
            )
            self.assertEqual(fileNames(collection), ["melody_duplicate_patterns.musicxml"])
            self.assertEqual(collection.getNumDirectories(), 0)

````

with

````python

    def test_a_file_that_fails_to_load_is_skipped_and_listed(self):
        with tempfile.TemporaryDirectory() as directory:
            shutil.copyfile(LAST_WINDOW, os.path.join(directory, "a.xml"))
            broken = os.path.join(directory, "b.xml")
            with open(broken, "w", encoding="utf-8") as text:
                text.write("not a score")
            collection = ml.ScoreCollection()
            collection.addScore(DUPLICATES)

            printed = io.StringIO()
            with contextlib.redirect_stdout(printed):
                collection.setDirectoriesPaths([directory])
            self.assertEqual(fileNames(collection), ["a.xml"])
            self.assertEqual(collection.getDirectoriesPaths(), [directory])
            self.assertEqual(
                collection.getLoadErrors(),
                [
                    (
                        broken,
                        f"[maiacore] Score: '{broken}' is not well-formed XML: No document "
                        "element found (byte offset 11)",
                    )
                ],
            )
            self.assertIn(
                "[maiacore] ScoreCollection: 1 of 2 files failed to load; see "
                "ScoreCollection.getLoadErrors()",
                printed.getvalue(),
            )

    def test_add_score_by_path_lists_a_failure_and_prints_to_sys_stdout(self):
        """addScore prints to sys.stdout, which redirect_stdout catches."""
        collection = ml.ScoreCollection()
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            collection.addScore("./missing.xml")
            collection.addScore([LAST_WINDOW, "./missing.xml"])
        self.assertEqual(fileNames(collection), ["melody_last_window.musicxml"])
        self.assertEqual(
            collection.getLoadErrors(),
            [("./missing.xml", "[maiacore] Score: cannot open './missing.xml'")],
        )
        self.assertEqual(
            printed.getvalue().count("files failed to load; see ScoreCollection.getLoadErrors()"),
            2,
        )

    def test_a_non_ascii_directory_lists_its_failures_with_utf8_paths(self):
        """Run in a child process whose standard output is ASCII: a directory 'músicas' holding
        '日本.xml', which loads, and 'ruim.xml', which does not."""
        code = (
            "import os, shutil, tempfile\n"
            "import maialib as ml\n"
            "with tempfile.TemporaryDirectory() as directory:\n"
            "    folder = os.path.join(directory, 'm\\u00fasicas')\n"
            "    os.makedirs(folder)\n"
            f"    shutil.copyfile({LAST_WINDOW!r}, os.path.join(folder, '\\u65e5\\u672c.xml'))\n"
            "    with open(os.path.join(folder, 'ruim.xml'), 'w') as text:\n"
            "        text.write('not a score')\n"
            "    collection = ml.ScoreCollection(folder)\n"
            "    paths = [path for path, _ in collection.getLoadErrors()]\n"
            "    names = [score.getFileName() for score in collection.getScores()]\n"
            "    print('RESULT', ascii(names), paths == [os.path.join(folder, 'ruim.xml')])\n"
        )
        completed = runChild(code, {"PYTHONIOENCODING": "ascii"})
        self.assertEqual(completed.returncode, 0, completed.stderr[-2000:])
        results = [line for line in completed.stdout.splitlines() if line.startswith("RESULT")]
        self.assertEqual(results, ["RESULT ['\\u65e5\\u672c.xml'] True"])

````

- [ ] **Step 3: Run them and see them fail.** C++ subset `ScoreCollection*` → the build fails: `no member named 'getLoadErrors' in 'ScoreCollection'`. «pytest» `test_score_collection.ScoreCollectionConstructionTestCase` → `test_a_file_that_fails_to_load_is_skipped_and_listed` errors (`RuntimeError: <path>: [maiacore] Score: ...`), `test_add_score_by_path_lists_a_failure_and_prints_to_sys_stdout` errors (`RuntimeError: [maiacore] Score: cannot open './missing.xml'`), `test_a_non_ascii_directory_lists_its_failures_with_utf8_paths` fails (the child raises `RuntimeError` for `ruim.xml`).

- [ ] **Step 4: Isolation.** Apply, in order:

**`maiacore/include/maiacore/score_collection.h` ~4: replace**

````cpp
#include <string>
#include <vector>
````

with

````cpp
#include <string>
#include <utility>
#include <vector>
````


**`maiacore/include/maiacore/score_collection.h` ~68: replace**

````cpp
     *          without regard to case, directory by directory in the given order and, within a
     *          directory, in sorted path order. Every path is checked before anything is loaded,
     *          and the collection changes only when every file has loaded. A subdirectory the
     *          user has no permission to read is skipped.
     * @param directoriesPaths The directories; an empty list empties the collection.
     * @param recursive True to load the files of their subdirectories, at any depth, too.
     * @throws std::runtime_error If a path does not exist or is not a directory, or a directory
     *         cannot be read: the message names the path that failed, a given path or one of its
     *         subdirectories. If a file fails to load: the message is the file's path, ": " and
     *         the message of the load's error.
     */
````

with

````cpp
     *          without regard to case, directory by directory in the given order and, within a
     *          directory, in sorted path order. Every directory is checked and listed before
     *          anything is loaded or changed. A file that fails to load is skipped: it is listed
     *          by getLoadErrors(), and one line says how many files failed. A subdirectory the
     *          user has no permission to read is skipped.
     * @param directoriesPaths The directories, UTF-8 paths; an empty list empties the
     *        collection.
     * @param recursive True to load the files of their subdirectories, at any depth, too.
     * @throws std::runtime_error If a path does not exist or is not a directory, or a directory
     *         cannot be read: the message names the path that failed, a given path or one of its
     *         subdirectories, and the collection is unchanged.
     */
````


**`maiacore/include/maiacore/score_collection.h` ~95: replace**

````cpp
     * @brief Loads a Score from a file path and adds it to the collection.
     * @param filePath Path to a MusicXML file.
     */
    void addScore(const std::string& filePath);

    /**
     * @brief Loads multiple Scores from file paths and adds them to the collection.
     * @param filePaths Vector of MusicXML file paths.
     */
    void addScore(const std::vector<std::string>& filePaths);

````

with

````cpp
     * @brief Loads a Score from a file path and adds it to the collection.
     * @details A file that fails to load is skipped and listed by getLoadErrors(), as
     *          setDirectoriesPaths() lists it.
     * @param filePath Path to a MusicXML file, in UTF-8.
     */
    void addScore(const std::string& filePath);

    /**
     * @brief Loads multiple Scores from file paths and adds them to the collection, in order.
     * @details Each file that fails to load is skipped and listed by getLoadErrors(), as
     *          setDirectoriesPaths() lists it.
     * @param filePaths MusicXML file paths, in UTF-8.
     */
    void addScore(const std::vector<std::string>& filePaths);

    /**
     * @brief The files that failed to load in the collection's last load: the constructor,
     *        setDirectoriesPaths() or addScore() with paths.
     * @details Each failure is the file's path and the first line of its error's message, in
     *          load order; both are valid UTF-8. A load in which every file loaded empties the
     *          list; the other methods leave it as it is.
     * @return (path, message) pairs.
     */
    std::vector<std::pair<std::string, std::string>> getLoadErrors() const;

````


**`maiacore/include/maiacore/score_collection.h` ~227: replace**

````cpp
    std::vector<Score> _scores;                  ///< Vector of loaded Score objects.
};
````

with

````cpp
    std::vector<Score> _scores;                  ///< Vector of loaded Score objects.
    /// The files that failed to load in the last load, and why.
    std::vector<std::pair<std::string, std::string>> _loadErrors;
};
````


**`maiacore/src/maiacore/score_collection.cpp` ~5: replace**

````cpp
#include <filesystem>
#include <stdexcept>
````

with

````cpp
#include <filesystem>
#include <iostream>
#include <stdexcept>
````


**`maiacore/src/maiacore/score_collection.cpp` ~100: replace**

````cpp

// The rows of 'table', a search of 'score', with the score's file name, composer and title.
````

with

````cpp

// Loads the score at 'path', UTF-8, into 'scores'. A file that fails to load is skipped: its path
// and the first line of the error's message -- what went wrong, without the source location and
// stack trace LOG_ERROR adds -- go to 'errors', as valid UTF-8.
void loadInto(const std::string& path, std::vector<Score>& scores,
              std::vector<std::pair<std::string, std::string>>& errors) {
    try {
        scores.emplace_back(path);
    } catch (const std::exception& error) {
        const std::string message = error.what();
        errors.emplace_back(maiacore::detail::validUtf8(path),
                            maiacore::detail::validUtf8(message.substr(0, message.find('\n'))));
    }
}

// The line that says how many of a load's files failed, printed when any did.
void printLoadErrors(const std::vector<std::pair<std::string, std::string>>& errors,
                     const size_t numFiles) {
    if (!errors.empty()) {
        std::cout << "[maiacore] ScoreCollection: " << errors.size() << " of " << numFiles
                  << " files failed to load; see ScoreCollection.getLoadErrors()" << std::endl;
    }
}

// The rows of 'table', a search of 'score', with the score's file name, composer and title.
````


**`maiacore/src/maiacore/score_collection.cpp` ~139: replace**

````cpp
    std::vector<Score> scores;
    for (const auto& directoryFiles : files) {
        for (const std::filesystem::path& file : directoryFiles) {
            // Logged as UTF-8, which Python's redirected stdout requires: the ANSI code page form
            // of a name with an accented letter is not valid UTF-8.
            LOG_INFO("Loading: " << file.filename().u8string());
            try {
                scores.emplace_back(file.u8string());
            } catch (const std::exception& loadError) {
                throw std::runtime_error(file.u8string() + ": " + loadError.what());
            }
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
````

with

````cpp
    std::vector<Score> scores;
    std::vector<std::pair<std::string, std::string>> errors;
    size_t numFiles = 0;
    for (const auto& directoryFiles : files) {
        for (const std::filesystem::path& file : directoryFiles) {
            // Logged as UTF-8, which Python's redirected stdout requires: the ANSI code page form
            // of a name with an accented letter is not valid UTF-8.
            LOG_INFO("Loading: " << file.filename().u8string());
            loadInto(file.u8string(), scores, errors);
            numFiles++;
        }
    }
    _directoriesPaths = directoriesPaths;
    _scores = std::move(scores);
    _loadErrors = std::move(errors);
    printLoadErrors(_loadErrors, numFiles);
}

std::vector<std::pair<std::string, std::string>> ScoreCollection::getLoadErrors() const {
    return _loadErrors;
}

void ScoreCollection::addDirectory(const std::string& directoryPath) {
    _directoriesPaths.push_back(directoryPath);
}

void ScoreCollection::addScore(const Score& score) { _scores.push_back(score); }

void ScoreCollection::addScore(const std::string& filePath) {
    addScore(std::vector<std::string>{filePath});
}

void ScoreCollection::addScore(const std::vector<std::string>& filePaths) {
    std::vector<std::pair<std::string, std::string>> errors;
    for (const std::string& filePath : filePaths) {
        loadInto(filePath, _scores, errors);
    }
    _loadErrors = std::move(errors);
    printLoadErrors(_loadErrors, filePaths.size());
}
````

- [ ] **Step 5: The binding.** In `maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` apply, in order:

**`maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` ~77: replace**

````cpp
        in the given order and, within a directory, in sorted path order; subdirectories only
        when ``recursive`` is True. Every path is checked before anything is loaded, and the
        collection changes only when every file has loaded. Scores added with ``addScore`` are
        replaced too. A subdirectory the user has no permission to read is skipped.

````

with

````cpp
        in the given order and, within a directory, in sorted path order; subdirectories only
        when ``recursive`` is True. Every directory is checked and listed before anything is
        loaded or changed. A file that fails to load is skipped: ``getLoadErrors()`` lists it, and
        one line says how many files failed. Scores added with ``addScore`` are replaced too. A
        subdirectory the user has no permission to read is skipped.

````


**`maiacore/src/maiacore/python_wrapper/py_score_collection.cpp` ~92: replace**

````cpp
            If a path does not exist or is not a directory, or a directory cannot be read: the
            message names the path that failed, a given path or one of its subdirectories. If a
            file fails to load: the message is the file's path, ``": "`` and the message of
            the load's error. The collection is unchanged.
    )pbdoc");

    cls.def("addDirectory", &ScoreCollection::addDirectory, py::arg("directoryPath"));

    cls.def("addScore", py::overload_cast<const Score&>(&ScoreCollection::addScore),
            py::arg("score"));
    cls.def("addScore", py::overload_cast<const std::string&>(&ScoreCollection::addScore),
            py::arg("filePath"));
    cls.def("addScore",
            py::overload_cast<const std::vector<std::string>&>(&ScoreCollection::addScore),
            py::arg("filePaths"));

````

with

````cpp
            If a path does not exist or is not a directory, or a directory cannot be read: the
            message names the path that failed, a given path or one of its subdirectories. The
            collection is unchanged.
    )pbdoc");

    cls.def("addDirectory", &ScoreCollection::addDirectory, py::arg("directoryPath"));

    cls.def("addScore", py::overload_cast<const Score&>(&ScoreCollection::addScore),
            py::arg("score"), py::call_guard<ConsoleRedirect>(), R"pbdoc(
        Add a copy of a score to the collection.
    )pbdoc");
    cls.def("addScore", py::overload_cast<const std::string&>(&ScoreCollection::addScore),
            py::arg("filePath"), py::call_guard<ConsoleRedirect>(), R"pbdoc(
        Load a score from a file and add it to the collection.

        A file that fails to load is skipped: ``getLoadErrors()`` lists it, and one line says so.
        What the load prints goes to ``sys.stdout``.

        Parameters
        ----------
        filePath : str
            Path to a MusicXML file.
    )pbdoc");
    cls.def("addScore",
            py::overload_cast<const std::vector<std::string>&>(&ScoreCollection::addScore),
            py::arg("filePaths"), py::call_guard<ConsoleRedirect>(), R"pbdoc(
        Load scores from files and add them to the collection, in order.

        Each file that fails to load is skipped: ``getLoadErrors()`` lists it, and one line says
        how many files failed. What the loads print goes to ``sys.stdout``.

        Parameters
        ----------
        filePaths : list of str
            Paths to MusicXML files.
    )pbdoc");

    cls.def("getLoadErrors", &ScoreCollection::getLoadErrors, R"pbdoc(
        Return the files that failed to load in the collection's last load: the constructor,
        ``setDirectoriesPaths`` or ``addScore`` with paths.

        A load in which every file loaded empties the list; the other methods leave it as it is.

        Returns
        -------
        list of tuple of (str, str)
            Each failure's file path and the first line of its error's message, in load order.

        Examples
        --------
        >>> collection = ml.ScoreCollection()
        >>> collection.addScore("missing.xml")
        [maiacore] ScoreCollection: 1 of 1 files failed to load; see ScoreCollection.getLoadErrors()
        >>> collection.getLoadErrors()
        [('missing.xml', "[maiacore] Score: cannot open 'missing.xml'")]
    )pbdoc");

````

- [ ] **Step 6: Format, build, pass.** clang-format `score_collection.h`, `score_collection.cpp`, `py_score_collection.cpp`, `score-collection-test.cpp`. C++ subset `ScoreCollection*` → all pass. «build» `make "PYTHON=$py" dev` → 0. «pytest» `test_score_collection` → OK. `ruff check` and `ruff format --check` on `test/test_score_collection.py` → clean.

- [ ] **Step 7: Mutations.** (a) In `addScore(const std::vector<std::string>&)` replace `_loadErrors = std::move(errors);` with `_loadErrors.clear();` → `AddScoreSkipsAndListsTheFilesThatFailToLoad` and `test_add_score_by_path_lists_a_failure_and_prints_to_sys_stdout` fail. (b) In `loadInto` replace `message.substr(0, message.find('\n'))` with `message` → `AFileThatFailsToLoadIsSkippedAndListed`, `ANonAsciiDirectoryListsItsFailuresInUtf8` and `test_a_file_that_fails_to_load_is_skipped_and_listed` fail (the stack trace follows the first line). (c) In `printLoadErrors` replace `if (!errors.empty()) {` with `if (false) {` → `AFileThatFailsToLoadIsSkippedAndListed`, `AddScoreSkipsAndListsTheFilesThatFailToLoad` and the two Python tests that count the line fail. (d) In the binding of `addScore` with `filePath` replace `py::arg("filePath"), py::call_guard<ConsoleRedirect>(),` with `py::arg("filePath"),` → after `make dev`, `test_add_score_by_path_lists_a_failure_and_prints_to_sys_stdout` fails (`1 != 2`: the line went to the C runtime's stdout). (e) In `setDirectoriesPaths` replace `_loadErrors = std::move(errors);` with `_loadErrors.clear();` → `AFileThatFailsToLoadIsSkippedAndListed` and `ANonAsciiDirectoryListsItsFailuresInUtf8` fail. Revert each; rerun green.

- [ ] **Step 8: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1216 tests.` (1214 + 2: one test replaced, two added). «build» `make "PYTHON=$py" py-tests` → OK, `Ran 711 tests` (709 + 2). «build» `make "PYTHON=$py" validate` → no new findings.

- [ ] **Step 9: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add maiacore/include/maiacore/score_collection.h maiacore/src/maiacore/score_collection.cpp maiacore/src/maiacore/python_wrapper/py_score_collection.cpp tests-cpp/src/score-collection-test.cpp test/test_score_collection.py`, message:

```
fix: ScoreCollection skips the files that fail to load and lists them

One file that failed to load aborted the whole load: the constructor,
setDirectoriesPaths() and addScore() with paths raised, and the
collection kept its old scores. Each file is now loaded on its own: a
file that fails is skipped, getLoadErrors() returns its path and the
first line of its error for the last load, and one line says how many
files failed. A directory that does not exist, is not a directory or
cannot be read still raises before anything changes.

The addScore() overloads printed to the C runtime's stdout instead of
Python's sys.stdout; they redirect their output as the other loading
methods do.

The all-or-nothing tests become isolation tests.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 7: Test tooling — `exit` and `codes` in the ledger, `CORPUS_ARGS`, `make fuzz --accept`

**Files:**
- Modify: `test/musicxml/corpus.py` (`LEDGER_LIMIT` ~34; `default_workers`, `select` before `is_slow` ~54; `run_one` ~98-124; `run_corpus`, `codes_path`, `load_ledger`, `_ledger_bytes`, `write_ledger`, `ledger_after` ~124-200), `test/musicxml/corpus_worker.py` (docstring ~12; `codes` after `load` ~124), `test/musicxml/fuzz.py` (`import os` removed ~13; `run` ~309), `scripts/make-corpus.py`, `scripts/make-fuzz.py`, `Makefile` (`corpus`, `corpus-update-ledger`, the `fuzz` comment ~126-140), `test/musicxml/README.md` (ledger table ~31, sidecar ~43, commands ~93), `test/musicxml/ledger.json`, `test/musicxml/ledger-external-codes.json` (new, generated)
- Test: `test/test_musicxml_corpus.py` (the stand-ins ~75, ~100; `LedgerLogicTestCase` ~234-267; `WorkerStagesTestCase`; `WorkerProcessTestCase` ~319-345), `test/test_musicxml_fuzz.py` (imports; `SCRIPTS`; `make_fuzz`, `AcceptanceTestCase` at the end)

**Interfaces:**
- Consumes: `Score.getImportIssues()` (Task 1), the worker's own-path loading (Task 5).
- Produces (`corpus.py`): `LEDGER_LIMIT = 500_000`; `default_workers() -> int` (`max(2, cpu // 2)`, the only definition; `run_corpus` and `fuzz.run` use it); `select(names, substring=None, skip_slow=False) -> list[str]`; `codes_path(path) -> Path` (`<stem>-codes.json`); `load_ledger(path)` merges the sidecar; `write_ledger(path, records)` moves `codes` to the sidecar when the ledger would exceed `LEDGER_LIMIT` bytes and removes a stale sidecar otherwise; `ledger_after(old, actual, complete) -> dict`; `run_one` adds `"exit": <status>` for a status other than 0 that is not a timeout. The worker adds `"codes": sorted distinct codes` after `load` when the report is not empty. `make-corpus.py` options `--in-repo-only`, `--skip-slow`, `--filter SUBSTRING`, `--workers N`; `make-fuzz.py --accept` (exit 1 when an outcome is worth minimising); the Makefile passes `$(CORPUS_ARGS)`.

- [ ] **Step 1: Write the failing tests.** Apply, in order:

**`test/test_musicxml_corpus.py` ~75: replace**

````python
        pass

    def toXML(self):
        return EXPORT
````

with

````python
        pass

    def getImportIssues(self):
        return []

    def toXML(self):
        return EXPORT
````


**`test/test_musicxml_corpus.py` ~100: replace**

````python
class Score:
    def __init__(self, path):
        pass

    def toXML(self):
        return "<score-partwise/>"
````

with

````python
class Score:
    def __init__(self, path):
        pass

    def getImportIssues(self):
        return []

    def toXML(self):
        return "<score-partwise/>"
````


**`test/test_musicxml_corpus.py` ~234: replace**

````python

    def test_an_update_drops_the_note_when_no_alternative_still_holds(self):
````

with

````python

    def test_a_partial_update_keeps_the_entries_of_the_files_it_did_not_examine(self):
        old = {"a.xml": FINISHED, SMALL_FILE: FINISHED}
        actual = {SMALL_FILE: {**FINISHED, "load": "IndexError"}}
        self.assertEqual(
            {"a.xml": FINISHED, SMALL_FILE: {**FINISHED, "load": "IndexError"}},
            corpus.ledger_after(old, actual, complete=False),
        )
        self.assertEqual(
            {SMALL_FILE: {**FINISHED, "load": "IndexError"}},
            corpus.ledger_after(old, actual, complete=True),
        )

    def test_a_selection_keeps_the_files_with_the_substring_and_can_leave_out_slow_ones(self):
        names = [SMALL_FILE, SLOW_FILE, LOADABLE_FILES[0]]
        self.assertEqual([SMALL_FILE, SLOW_FILE], corpus.select(names, "xml_examples"))
        self.assertEqual([SMALL_FILE], corpus.select(names, "xml_examples", skip_slow=True))
        self.assertEqual([SMALL_FILE, LOADABLE_FILES[0]], corpus.select(names, skip_slow=True))
        self.assertEqual(names, corpus.select(names))

    def test_codes_move_to_a_sidecar_only_when_the_ledger_would_exceed_the_limit(self):
        records = {
            "a.xml": {**FINISHED, "codes": ["ALTER_OFF_GRID", "ELEMENT_NOT_MODELLED"]},
            "b.xml": FINISHED,
        }
        with tempfile.TemporaryDirectory() as folder:
            ledger = Path(folder) / "ledger-external.json"
            sidecar = Path(folder) / "ledger-external-codes.json"
            with mock.patch.object(corpus, "LEDGER_LIMIT", 100):
                corpus.write_ledger(ledger, records)
            self.assertNotIn(b"codes", ledger.read_bytes())
            self.assertEqual(
                {"files": {"a.xml": ["ALTER_OFF_GRID", "ELEMENT_NOT_MODELLED"]}},
                json.loads(sidecar.read_text(encoding="utf-8")),
            )
            self.assertEqual(records, corpus.load_ledger(ledger))

            corpus.write_ledger(ledger, records)
            self.assertFalse(sidecar.exists())
            self.assertIn(
                b'"codes": ["ALTER_OFF_GRID", "ELEMENT_NOT_MODELLED"]', ledger.read_bytes()
            )
            self.assertEqual(records, corpus.load_ledger(ledger))

    def test_an_update_drops_the_note_when_no_alternative_still_holds(self):
````


**`test/test_musicxml_corpus.py` ~267: replace**

````python
        self.assertEqual("n/a", before_the_round_trip["export_xsd"])

````

with

````python
        self.assertEqual("n/a", before_the_round_trip["export_xsd"])

    def test_a_loaded_file_has_the_sorted_codes_of_its_report_and_none_when_it_is_empty(self):
        unit = corpus.REPO_ROOT / "test/xml_examples/unit_test"
        expected = {
            "transpose_out_of_range.musicxml": ["TRANSPOSE_OUT_OF_RANGE"],
            "transpose_for_part.musicxml": ["ELEMENT_NOT_MODELLED", "FOR_PART_NOT_MODELLED"],
            "quarter_tone_tartini.xml": None,
        }
        for name, codes in expected.items():
            with self.subTest(name=name):
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    corpus_worker.examine(unit / name, analyses=False)
                final = records_in(output.getvalue())[-1]
                self.assertEqual(codes, final.get("codes"))

````


**`test/test_musicxml_corpus.py` ~319: replace**

````python
            record = corpus.run_one(SMALL_FILE, timeout=60)
        self.assertEqual({**NOTHING_FINISHED, "input": "valid", "load": "crash"}, record)

````

with

````python
            record = corpus.run_one(SMALL_FILE, timeout=60)
        self.assertEqual({**NOTHING_FINISHED, "input": "valid", "load": "crash", "exit": 3}, record)

````


**`test/test_musicxml_corpus.py` ~332: replace**

````python
                "export_xml": "crash",
            },
            record,
        )

    def test_diagnostics_give_the_exit_status_and_the_end_of_stderr(self):
        diagnostics = {}
        with stand_in_maialib(DIES_AFTER_THE_FINAL_RECORD):
            record = corpus.run_one(SMALL_FILE, timeout=60, diagnostics=diagnostics)
        # The record is complete: only the exit status shows the crash.
        self.assertEqual({**FINISHED, "export_xsd": "valid"}, record)
        self.assertEqual(
````

with

````python
                "export_xml": "crash",
                "exit": 3,
            },
            record,
        )

    def test_diagnostics_give_the_exit_status_and_the_end_of_stderr(self):
        diagnostics = {}
        with stand_in_maialib(DIES_AFTER_THE_FINAL_RECORD):
            record = corpus.run_one(SMALL_FILE, timeout=60, diagnostics=diagnostics)
        # The record is complete but for the exit status, which shows the crash.
        self.assertEqual({**FINISHED, "export_xsd": "valid", "exit": 3}, record)
        self.assertEqual(
````


**`test/test_musicxml_fuzz.py` ~2: replace**

````python

import io
````

with

````python

import contextlib
import importlib.util
import io
````


**`test/test_musicxml_fuzz.py` ~20: replace**

````python
BYTE_LEVEL = ("utf16", "byte-order-mark", "latin1-declaration", "truncate")

````

with

````python
BYTE_LEVEL = ("utf16", "byte-order-mark", "latin1-declaration", "truncate")
SCRIPTS = Path(__file__).resolve().parents[1] / "scripts"

````


**`test/test_musicxml_fuzz.py` ~282: replace**

````python

if __name__ == "__main__":
````

with

````python

def make_fuzz():
    """scripts/make-fuzz.py as a module, with scripts/ on the path for its own imports."""
    if str(SCRIPTS) not in sys.path:
        sys.path.insert(0, str(SCRIPTS))
    spec = importlib.util.spec_from_file_location("make_fuzz", SCRIPTS / "make-fuzz.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class AcceptanceTestCase(unittest.TestCase):
    """`make fuzz FUZZ_ARGS="--accept"`: the exit status and the cases it lists."""

    def run_make_fuzz(self, outcomes, *arguments):
        """make-fuzz.py's exit status and output for a run whose cases end in 'outcomes'."""
        script = make_fuzz()
        results = [
            (fuzz.Case(1, index, f"{index}.xml", "truncate", b"", ".xml"), record(), outcome)
            for index, outcome in enumerate(outcomes)
        ]
        report = fuzz.WORK / "report-seed-1.json"
        output = io.StringIO()
        with mock.patch.object(script.fuzz, "run", return_value=results), mock.patch.object(
            script.fuzz, "write_report", return_value=report
        ), mock.patch.object(sys, "argv", ["make-fuzz.py", *arguments]), contextlib.redirect_stdout(
            output
        ):
            status = script.main()
        return status, output.getvalue()

    def test_accept_fails_and_lists_each_case_worth_minimising(self):
        status, printed = self.run_make_fuzz(
            ["ok", "load:IndexError", "export:xsd-invalid", "crash:load"], "--accept"
        )
        self.assertEqual(1, status)
        self.assertIn("case 1: load:IndexError (1.xml, truncate)", printed)
        self.assertIn("case 3: crash:load (3.xml, truncate)", printed)
        self.assertNotIn("case 2:", printed)

    def test_accept_passes_when_every_outcome_is_expected(self):
        status, _ = self.run_make_fuzz(
            ["ok", "export:xsd-invalid", "roundtrip:unstable"], "--accept"
        )
        self.assertEqual(0, status)

    def test_without_accept_the_run_only_reports(self):
        status, printed = self.run_make_fuzz(["crash:load"])
        self.assertEqual(0, status)
        self.assertIn("crash:load: 1", printed)


if __name__ == "__main__":
````

- [ ] **Step 2: Run them and see them fail.** «pytest» `test_musicxml_corpus.LedgerLogicTestCase test_musicxml_corpus.WorkerStagesTestCase test_musicxml_corpus.WorkerProcessTestCase test_musicxml_fuzz.AcceptanceTestCase` → errors `AttributeError: module 'corpus' has no attribute 'ledger_after'` (and `select`, `LEDGER_LIMIT`); the codes test fails (`['TRANSPOSE_OUT_OF_RANGE'] != None`); the three exit tests fail (`'exit': 3` missing); `test_accept_*` exit with `SystemExit: 2` (`unrecognized arguments: --accept`).

- [ ] **Step 3: The tooling.** Apply, in order:

**`test/musicxml/corpus.py` ~34: replace**

````python
STDERR_TAIL = 2000

````

with

````python
STDERR_TAIL = 2000
# The size above which a ledger keeps its "codes" in a sidecar file instead, below the 500 KB
# from which the repository's pre-commit hook refuses a file.
LEDGER_LIMIT = 500_000

````


**`test/musicxml/corpus.py` ~54: replace**

````python
    return _files_under(EXTERNAL_ROOT) if EXTERNAL_ROOT.is_dir() else []

````

with

````python
    return _files_under(EXTERNAL_ROOT) if EXTERNAL_ROOT.is_dir() else []


def default_workers() -> int:
    """The number of files examined at once unless told otherwise: half the CPUs, at least 2."""
    return max(2, (os.cpu_count() or 2) // 2)


def select(
    names: Iterable[str], substring: str | None = None, skip_slow: bool = False
) -> list[str]:
    """The names that contain ``substring`` (all without one), the slow files left out with
    ``skip_slow``."""
    return [
        name
        for name in names
        if (substring is None or substring in name) and not (skip_slow and is_slow(name))
    ]

````


**`test/musicxml/corpus.py` ~98: replace**

````python
    ``diagnostics``, also fill it with the worker's ``exit_code`` (None when the timeout stopped
    it) and ``stderr_tail``, the last STDERR_TAIL characters of its standard error.
    """
````

with

````python
    ``diagnostics``, also fill it with the worker's ``exit_code`` (None when the timeout stopped
    it) and ``stderr_tail``, the last STDERR_TAIL characters of its standard error. A worker that
    ended with a status other than 0 has it in the record's ``exit``, also after its final record.
    """
````


**`test/musicxml/corpus.py` ~124: replace**

````python
    record = _last_record(stdout or b"") or corpus_worker.new_record(analyses)
    return ended(record, "timeout" if exit_code is None else "crash")


def run_corpus(
    names: Iterable[str], analyses: bool = False, workers: int | None = None
) -> dict[str, Record]:
    """Examine the files in parallel and return their records by name."""
    names = list(names)
    count = workers or max(2, (os.cpu_count() or 2) // 2)
    with ThreadPoolExecutor(max_workers=count) as pool:
        records = list(pool.map(lambda name: run_one(name, analyses), names))
    return dict(zip(names, records))


def load_ledger(path: Path) -> dict[str, Record]:
    return json.loads(path.read_text(encoding="utf-8"))["files"]


def write_ledger(path: Path, records: dict[str, Record]) -> None:
    """Write one file per line, sorted, so that a change shows as a one-line diff."""
    lines = [
        f"  {json.dumps(name)}: {json.dumps(records[name], sort_keys=True)}"
        for name in sorted(records)
    ]
    # Bytes, so the line endings are LF on every platform (Path.write_text has no newline
    # argument before Python 3.10).
    path.write_bytes(('{"files": {\n' + ",\n".join(lines) + "\n}}\n").encode("utf-8"))

````

with

````python
    record = _last_record(stdout or b"") or corpus_worker.new_record(analyses)
    if exit_code is not None and exit_code != 0:
        record["exit"] = exit_code
    return ended(record, "timeout" if exit_code is None else "crash")


def run_corpus(
    names: Iterable[str], analyses: bool = False, workers: int | None = None
) -> dict[str, Record]:
    """Examine the files in parallel and return their records by name."""
    names = list(names)
    with ThreadPoolExecutor(max_workers=workers or default_workers()) as pool:
        records = list(pool.map(lambda name: run_one(name, analyses), names))
    return dict(zip(names, records))


def codes_path(path: Path) -> Path:
    """The sidecar file of a ledger, which holds its "codes" when the ledger would be too big."""
    return path.with_name(path.stem + "-codes.json")


def load_ledger(path: Path) -> dict[str, Record]:
    """The ledger's records, with the "codes" of its sidecar file, if it has one."""
    records = json.loads(path.read_text(encoding="utf-8"))["files"]
    sidecar = codes_path(path)
    if sidecar.is_file():
        for name, codes in json.loads(sidecar.read_text(encoding="utf-8"))["files"].items():
            records.setdefault(name, {})["codes"] = codes
    return records


def _ledger_bytes(entries: dict[str, Any]) -> bytes:
    """One file per line, sorted, so that a change shows as a one-line diff; LF line endings on
    every platform (Path.write_text has no newline argument before Python 3.10)."""
    lines = [
        f"  {json.dumps(name)}: {json.dumps(entries[name], sort_keys=True)}"
        for name in sorted(entries)
    ]
    return ('{"files": {\n' + ",\n".join(lines) + "\n}}\n").encode("utf-8")


def write_ledger(path: Path, records: dict[str, Record]) -> None:
    """Write the ledger, one file per line. When it would exceed LEDGER_LIMIT bytes, the "codes"
    of its records go to its sidecar file (codes_path), in the same format; otherwise a sidecar
    left from an earlier write is removed."""
    data = _ledger_bytes(records)
    sidecar = codes_path(path)
    if len(data) > LEDGER_LIMIT:
        codes = {name: record["codes"] for name, record in records.items() if "codes" in record}
        rest = {
            name: {key: value for key, value in record.items() if key != "codes"}
            for name, record in records.items()
        }
        data = _ledger_bytes(rest)
        sidecar.write_bytes(_ledger_bytes(codes))
    elif sidecar.is_file():
        sidecar.unlink()
    path.write_bytes(data)


def ledger_after(
    old: dict[str, Record], actual: dict[str, Record], complete: bool
) -> dict[str, Record]:
    """The ledger to write after examining the files of ``actual``: their updated entries, and,
    when the run examined only some of the corpus (``complete`` false), the old entries of every
    other file."""
    ledger = {} if complete else dict(old)
    ledger.update(updated_ledger(old, actual))
    return ledger

````


**`test/musicxml/corpus_worker.py` ~12: replace**

````python
date). A stage that cannot run because of an earlier result is "n/a".
"""
````

with

````python
date). A stage that cannot run because of an earlier result is "n/a".
A file that loads also has "codes": the distinct codes of its import report, sorted, when the
report is not empty.
"""
````


**`test/musicxml/corpus_worker.py` ~124: replace**

````python
    record["load"] = "ok"
    emit(record)
````

with

````python
    record["load"] = "ok"
    codes = sorted({issue.code for issue in score.getImportIssues()})
    if codes:
        record["codes"] = codes
    emit(record)
````


**`test/musicxml/fuzz.py` ~13: replace**

````python
import json
import os
import random
````

with

````python
import json
import random
````


**`test/musicxml/fuzz.py` ~309: replace**

````python
    deadline = None if minutes is None else time.monotonic() + minutes * 60
    count = workers or max(2, (os.cpu_count() or 2) // 2)
    results: list[tuple[Case, Record, str]] = []
````

with

````python
    deadline = None if minutes is None else time.monotonic() + minutes * 60
    count = workers or corpus.default_workers()
    results: list[tuple[Case, Record, str]] = []
````


**`scripts/make-corpus.py` ~8: replace**

````python
review the diff before committing it. Both need maialib installed (`make dev`).
"""
````

with

````python
review the diff before committing it. Both need maialib installed (`make dev`).

Options go through CORPUS_ARGS, e.g. `make corpus CORPUS_ARGS="--skip-slow --workers 4"`:
--in-repo-only leaves out the external corpus, --skip-slow the files of 10 MB or more, and
--filter SUBSTRING every file whose repository-relative path does not contain SUBSTRING; --workers
sets how many files are examined at once. A run that leaves files out compares, or updates, only
the files it examined, and does not run the round trip.
"""
````


**`scripts/make-corpus.py` ~27: replace**

````python
    )
    arguments = parser.parse_args()
    failed = False
    for ledger, files in (
        (corpus.LEDGER, corpus.corpus_files()),
        (corpus.EXTERNAL_LEDGER, corpus.external_files()),
    ):
        if not files:
            continue
        print(
            f"{color.OKGREEN}Examining {len(files)} files for {ledger.name}...{color.ENDC}",
            flush=True,
        )
        actual = corpus.run_corpus(files)
        old = corpus.load_ledger(ledger) if ledger.is_file() else {}
        if arguments.update_ledger:
            corpus.write_ledger(ledger, corpus.updated_ledger(old, actual))
            print(f"{color.OKGREEN}Wrote {ledger.relative_to(REPO_ROOT)}{color.ENDC}")
            continue
        problems = corpus.compare(old, actual)
        problems += [
            f"{name}: in the ledger but not in the corpus"
            for name in sorted(set(old) - set(actual))
        ]
        for problem in problems:
            print(f"{color.FAIL}{problem}{color.ENDC}")
        failed = failed or bool(problems)
    if not arguments.update_ledger and not failed:
        # `make py-tests` skips the slow corpus files; their <transpose> round trip runs here,
````

with

````python
    )
    parser.add_argument("--in-repo-only", action="store_true", help="leave out the external corpus")
    parser.add_argument(
        "--skip-slow", action="store_true", help="leave out the files of 10 MB or more"
    )
    parser.add_argument(
        "--filter", metavar="SUBSTRING", help="only the files whose path contains SUBSTRING"
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=None,
        help=f"files examined at once (default {corpus.default_workers()})",
    )
    arguments = parser.parse_args()
    corpora = [(corpus.LEDGER, corpus.corpus_files())]
    if not arguments.in_repo_only:
        corpora.append((corpus.EXTERNAL_LEDGER, corpus.external_files()))
    failed = False
    partial = arguments.skip_slow or arguments.filter is not None
    for ledger, every_file in corpora:
        files = corpus.select(every_file, arguments.filter, arguments.skip_slow)
        if not files:
            continue
        print(
            f"{color.OKGREEN}Examining {len(files)} files for {ledger.name}...{color.ENDC}",
            flush=True,
        )
        actual = corpus.run_corpus(files, workers=arguments.workers)
        old = corpus.load_ledger(ledger) if ledger.is_file() else {}
        complete = len(files) == len(every_file)
        if arguments.update_ledger:
            corpus.write_ledger(ledger, corpus.ledger_after(old, actual, complete))
            print(f"{color.OKGREEN}Wrote {ledger.relative_to(REPO_ROOT)}{color.ENDC}")
            continue
        problems = corpus.compare(old, actual)
        if complete:
            problems += [
                f"{name}: in the ledger but not in the corpus"
                for name in sorted(set(old) - set(actual))
            ]
        for problem in problems:
            print(f"{color.FAIL}{problem}{color.ENDC}")
        failed = failed or bool(problems)
    if not arguments.update_ledger and not failed and not partial:
        # `make py-tests` skips the slow corpus files; their <transpose> round trip runs here,
````


**`scripts/make-fuzz.py` ~6: replace**

````python
each failing outcome are saved into test/musicxml/fuzz-regressions/, minimised unless
fuzz.minimize keeps them as they are. Needs maialib installed (`make dev`).
"""
````

with

````python
each failing outcome are saved into test/musicxml/fuzz-regressions/, minimised unless
fuzz.minimize keeps them as they are. With --accept the run is the acceptance test: it lists every
case whose outcome is worth minimising (fuzz.worth_minimising) and exits 1 when there is one;
without it the run only reports. Needs maialib installed (`make dev`).
"""
````


**`scripts/make-fuzz.py` ~31: replace**

````python
    parser.add_argument("--per-outcome", type=int, default=2, help="cases saved per outcome")
    arguments = parser.parse_args()
````

with

````python
    parser.add_argument("--per-outcome", type=int, default=2, help="cases saved per outcome")
    parser.add_argument(
        "--accept", action="store_true", help="fail when an outcome is worth minimising"
    )
    arguments = parser.parse_args()
````


**`scripts/make-fuzz.py` ~51: replace**

````python
            print(f"  {outcome}, case {case.index}: {path.relative_to(REPO_ROOT)}")
    return 0
````

with

````python
            print(f"  {outcome}, case {case.index}: {path.relative_to(REPO_ROOT)}")
    if arguments.accept:
        findings = [
            (case, outcome) for case, _, outcome in results if fuzz.worth_minimising(outcome)
        ]
        for case, outcome in findings:
            finding = f"case {case.index}: {outcome} ({case.source}, {case.mutation})"
            print(f"{color.FAIL}{finding}{color.ENDC}")
        if findings:
            print(f"{color.FAIL}{len(findings)} cases are not accepted.{color.ENDC}")
            return 1
    return 0
````


**`Makefile` ~126: replace**

````makefile
# Every file of the MusicXML corpus, the slow ones included, and the external corpus once
# `make corpus-fetch` has downloaded it, compared with test/musicxml/ledger*.json.
corpus:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py

# Write the current corpus results as the ledgers; review the diff before committing it.
corpus-update-ledger:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py --update-ledger

# Download the external MusicXML corpus (OpenScore, CC0) at pinned commits into
# test/musicxml/external/, which git ignores.
corpus-fetch:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus-fetch.py

# Seeded mutation fuzzing of the MusicXML reader and writer; options through FUZZ_ARGS, e.g.
# make fuzz FUZZ_ARGS="--seed 7 --cases 1000".
fuzz:
````

with

````makefile
# Every file of the MusicXML corpus, the slow ones included, and the external corpus once
# `make corpus-fetch` has downloaded it, compared with test/musicxml/ledger*.json. Options go
# through CORPUS_ARGS: --in-repo-only, --skip-slow, --filter SUBSTRING, --workers N.
corpus:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py $(CORPUS_ARGS)

# Write the current corpus results as the ledgers; review the diff before committing it. With
# CORPUS_ARGS that leave files out, only the examined files' lines are written.
corpus-update-ledger:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py --update-ledger $(CORPUS_ARGS)

# Download the external MusicXML corpus (OpenScore, CC0) at pinned commits into
# test/musicxml/external/, which git ignores.
corpus-fetch:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus-fetch.py

# Seeded mutation fuzzing of the MusicXML reader and writer; options through FUZZ_ARGS, e.g.
# make fuzz FUZZ_ARGS="--seed 7 --cases 1000"; --accept fails on a case worth minimising.
fuzz:
````

- [ ] **Step 4: The README.** In `test/musicxml/README.md` apply, in order:

**`test/musicxml/README.md` ~31: replace**

````markdown
| `roundtrip` | `stable` when the export, loaded and exported again, is identical apart from its encoding date; `unstable`; an exception type; `crash`; `timeout`; `n/a` |
| `slow` | `true` for files of 10 MB or more: `make py-tests` skips them, `make corpus` runs them |
````

with

````markdown
| `roundtrip` | `stable` when the export, loaded and exported again, is identical apart from its encoding date; `unstable`; an exception type; `crash`; `timeout`; `n/a` |
| `codes` | the distinct codes of the file's import report (`Score.getImportIssues()`), sorted; present only when the file loads and its report is not empty |
| `exit` | the worker's exit status, present only when it is not 0 -- also when the worker ended after its final record, which no stage shows. The status of a crash differs between platforms (3221226505 for 0xC0000409 on Windows, a negative signal number on Linux): a ledger line that holds one needs `{"any_of": [...]}` |
| `slow` | `true` for files of 10 MB or more: `make py-tests` skips them, `make corpus` runs them |
````


**`test/musicxml/README.md` ~43: replace**

````markdown
ledger records what maialib does with the real file, on every platform.

````

with

````markdown
ledger records what maialib does with the real file, on every platform.

A ledger that would exceed 500,000 bytes keeps its `codes` in a sidecar file, `<ledger>-codes.json`
(`ledger-external-codes.json`), one file per line in the same format; `make corpus` reads both, and
`make corpus-update-ledger` writes the sidecar, or removes it when the ledger fits again.

````


**`test/musicxml/README.md` ~93: replace**

````markdown
- `make corpus-update-ledger` writes the ledgers from the current results.
- `make corpus-fetch` downloads OpenScore Lieder and String Quartets (CC0) at pinned commits;
  `make corpus` then includes them. On Windows it fails with "Filename too long" when the path of
  the repository's root is longer than 66 characters: the deepest OpenScore file adds 193 more,
  and git there creates no file whose path is 260 characters or longer.
- `make fuzz` runs 300 cases of seed 1 and writes `fuzz-work/report-seed-1.json`; `make
  fuzz-minimize` does the same, then saves up to two cases of each outcome worth it into
  `fuzz-regressions/`, minimised where possible. Options go through `FUZZ_ARGS`, e.g.
  `make fuzz FUZZ_ARGS="--seed 7 --cases 1000"` (also `--minutes`, `--timeout`, `--per-outcome`).
- `python test/musicxml/musicxml_check.py FILE...` validates files; `python
````

with

````markdown
- `make corpus-update-ledger` writes the ledgers from the current results.
- Both take options through `CORPUS_ARGS`, e.g. `make corpus CORPUS_ARGS="--skip-slow --workers 4"`:
  `--in-repo-only` leaves out the external corpus, `--skip-slow` the files of 10 MB or more, and
  `--filter SUBSTRING` every file whose repository-relative path does not contain `SUBSTRING`;
  `--workers N` examines N files at once (default: half the CPUs, at least 2). A run that leaves
  files out compares, or writes, only the lines of the files it examined, and skips the round
  trip.
- `make corpus-fetch` downloads OpenScore Lieder and String Quartets (CC0) at pinned commits;
  `make corpus` then includes them. On Windows it fails with "Filename too long" when the path of
  the repository's root is longer than 66 characters: the deepest OpenScore file adds 193 more,
  and git there creates no file whose path is 260 characters or longer.
- `make fuzz` runs 300 cases of seed 1 and writes `fuzz-work/report-seed-1.json`; `make
  fuzz-minimize` does the same, then saves up to two cases of each outcome worth it into
  `fuzz-regressions/`, minimised where possible. Options go through `FUZZ_ARGS`, e.g.
  `make fuzz FUZZ_ARGS="--seed 7 --cases 1000"` (also `--minutes`, `--timeout`, `--per-outcome`).
  `make fuzz FUZZ_ARGS="--accept"` is the acceptance test: it lists each case whose outcome is
  worth minimising and exits 1 when there is one; without `--accept` the run only reports.
- `python test/musicxml/musicxml_check.py FILE...` validates files; `python
````

- [ ] **Step 5: Pass.** «pytest» `test_musicxml_corpus.LedgerLogicTestCase test_musicxml_corpus.WorkerStagesTestCase test_musicxml_corpus.WorkerProcessTestCase test_musicxml_fuzz` → OK. `ruff check` and `ruff format --check` on `test/musicxml/corpus.py test/musicxml/corpus_worker.py test/musicxml/fuzz.py scripts/make-corpus.py scripts/make-fuzz.py test/test_musicxml_corpus.py test/test_musicxml_fuzz.py` → clean. «build» `make "PYTHON=$py" corpus 'CORPUS_ARGS=--in-repo-only --filter unit_test/transpose_ --workers 2'; $LASTEXITCODE` → 1, every difference a missing `codes` (the ledgers are regenerated next), and no round trip runs.

- [ ] **Step 6: Mutations.** (a) In `run_one` replace `record["exit"] = exit_code` with `pass` → `test_a_worker_that_dies_is_a_crash_at_its_first_unfinished_stage`, `test_a_worker_that_dies_while_checking_the_export_is_a_crash_there` and `test_diagnostics_give_the_exit_status_and_the_end_of_stderr` fail. (b) In the worker replace `if codes:` with `if True:` → `test_a_loaded_file_has_the_sorted_codes_of_its_report_and_none_when_it_is_empty` fails (`None != []`). (c) In `write_ledger` replace `if len(data) > LEDGER_LIMIT:` with `if False:` → `test_codes_move_to_a_sidecar_only_when_the_ledger_would_exceed_the_limit` fails. (d) In `load_ledger` replace `if sidecar.is_file():` with `if False:` → the same test fails (the codes are lost on loading). (e) In `ledger_after` replace `ledger = {} if complete else dict(old)` with `ledger = {}` → `test_a_partial_update_keeps_the_entries_of_the_files_it_did_not_examine` fails. (f) In `select` replace `not (skip_slow and is_slow(name))` with `True` → `test_a_selection_keeps_the_files_with_the_substring_and_can_leave_out_slow_ones` fails. (g) In `make-fuzz.py` replace `return 1` with `return 0` → `test_accept_fails_and_lists_each_case_worth_minimising` fails. (h) In `make-fuzz.py` replace `if arguments.accept:` with `if True:` → `test_without_accept_the_run_only_reports` fails (`0 != 1`: a `crash:load` case fails the run). (i) In `make-fuzz.py` replace `if fuzz.worth_minimising(outcome)` with `if True` → `test_accept_passes_when_every_outcome_is_expected` fails (`0 != 1`). Revert each; rerun green.

- [ ] **Step 7: Regenerate the ledgers.** «build» `make "PYTHON=$py" corpus-update-ledger; $LASTEXITCODE` (every file, the slow ones and the external corpus included) → 0. Write with the Write tool `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-ledger-check.py`:

```python
"""Compare a ledger before and after an update: every difference must be a "codes" or "exit"
field. Usage: python ledger-check.py OLD NEW [NEW_SIDECAR]; NEW_SIDECAR holds NEW's codes."""
import json
import sys
from collections import Counter

old = json.load(open(sys.argv[1], encoding="utf-8"))["files"]
new = json.load(open(sys.argv[2], encoding="utf-8"))["files"]
if len(sys.argv) > 3:
    for name, codes in json.load(open(sys.argv[3], encoding="utf-8"))["files"].items():
        new.setdefault(name, {})["codes"] = codes
other = []
codes = Counter()
with_codes = 0
for name in sorted(set(old) | set(new)):
    before = {k: v for k, v in old.get(name, {}).items() if k not in ("codes", "exit")}
    after = {k: v for k, v in new.get(name, {}).items() if k not in ("codes", "exit")}
    if before != after:
        other.append((name, before, after))
    if "exit" in new.get(name, {}):
        other.append((name, "exit", new[name]["exit"]))
    if "codes" in new.get(name, {}):
        with_codes += 1
        codes.update(new[name]["codes"])
print("files:", len(new), "with codes:", with_codes)
for code, count in sorted(codes.items()):
    print(f"  {code}: {count}")
print("other differences:", len(other))
for item in other:
    print("  ", item)
```

  Then, in Bash from the repository root: `git show HEAD:test/musicxml/ledger.json > /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-ledger-old.json; git show HEAD:test/musicxml/ledger-external.json > /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-ledger-external-old.json`; `$PY /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-ledger-check.py /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-ledger-old.json test/musicxml/ledger.json` and `$PY /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-ledger-check.py /c/Users/nyck/AppData/Local/Temp/maialib-4c1a-ledger-external-old.json test/musicxml/ledger-external.json test/musicxml/ledger-external-codes.json` (with `PY=/c/Users/nyck/AppData/Local/Temp/maialib-4c1a-venv/Scripts/python.exe`) → `other differences: 0` for both. Measured on a scratch copy (the whole update took 18 minutes): ledger.json, 229 of 289 files with codes — `ELEMENT_NOT_MODELLED` 219, `PART_NAME_DUPLICATE` 5, `ACCIDENTAL_ALTER_MISMATCH` 3, `ACCIDENTAL_NAME_UNKNOWN` 3, `ALTER_OFF_GRID` 3, `TRANSPOSE_PAIR_CORRECTED` 3, `DIVISIONS_MISSING` 1, `FOR_PART_NOT_MODELLED` 1, `TRANSPOSE_CHROMATIC_NOT_INTEGER` 1, `TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER` 1, `TRANSPOSE_OUT_OF_RANGE` 1 (71,473 bytes); the external ledger, 1,587 of 1,658 files with codes — `ELEMENT_NOT_MODELLED` 1,587, `ACCIDENTAL_NAME_UNKNOWN` 13, `PART_NAME_DUPLICATE` 12. With its codes the external ledger would exceed 500,000 bytes, so `ledger-external.json` stays byte-identical (`git diff --stat test/musicxml/ledger-external.json` is empty) and the codes are in the new `test/musicxml/ledger-external-codes.json` (223,869 bytes with LF endings); record the sizes and counts you get. No line may hold `exit` (no corpus file crashes today); if one does, stop and report it. Then «build» `make "PYTHON=$py" corpus; $LASTEXITCODE` → 0 (both ledgers match, the round trip runs).

- [ ] **Step 8: Whole suites.** «build» `make "PYTHON=$py" cpp-tests` → 0, `[  PASSED  ] 1216 tests.` «build» `make "PYTHON=$py" py-tests` → OK, `Ran 718 tests` (711 + 7). «build» `make "PYTHON=$py" fuzz; $LASTEXITCODE` → 0 (report mode). «build» `make "PYTHON=$py" fuzz 'FUZZ_ARGS=--accept'; $LASTEXITCODE` → 1 until 4c-1b, the cases it lists matching the report's findings that are worth minimising; record them.

- [ ] **Step 9: Commit.** `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`; `git add test/musicxml/corpus.py test/musicxml/corpus_worker.py test/musicxml/fuzz.py scripts/make-corpus.py scripts/make-fuzz.py Makefile test/musicxml/README.md test/musicxml/ledger.json test/musicxml/ledger-external-codes.json test/test_musicxml_corpus.py test/test_musicxml_fuzz.py` (and `test/musicxml/ledger-external.json` only if Step 7 found it changed, which it must explain), message (the counts measured on a scratch copy; use Step 7's if they differ):

```
test: the ledger records report codes and exit statuses; corpus subsets; fuzz acceptance

A corpus record gains "codes", the sorted distinct codes of the file's
import report, when the file loads and the report is not empty, and
"exit", the worker's exit status, when it is not 0: a worker that ended
with an error after its final record was invisible to the ledger. A
ledger that would exceed 500,000 bytes keeps its codes in a sidecar,
<ledger>-codes.json, read and written with it.

make corpus and make corpus-update-ledger take CORPUS_ARGS: --in-repo-only,
--skip-slow, --filter SUBSTRING and --workers N (default half the CPUs,
at least 2, now defined once for the corpus and the fuzz driver); a run
that leaves files out compares or writes only their lines. make fuzz
FUZZ_ARGS="--accept" exits 1, listing the cases, when an outcome is
worth minimising.

ledger.json: every line of a file that loads with a non-empty report
gains its codes (229 of 289 files); no other field of any line changes.
ledger-external.json is unchanged: with codes it would exceed 500,000
bytes, so they are in ledger-external-codes.json (1,587 files).

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

---

### Task 8: CHANGELOG and the final verification

**Files:**
- Modify: `CHANGELOG.md` (`[Unreleased]`, relative to v1.10.3 = `origin/main` `72d1e72`: the existing lines that name the warnings, ~13, ~28, ~78, ~83, ~94; a group in `### Added` before `- Add \`Helper.getLibraryVersion()\``; entries at the end of `### Fix`, before `### Removed`; one entry at the end of `### Build and tooling`)

**Interfaces:** consumes everything above.

- [ ] **Step 1: The CHANGELOG.** Apply, in order:

**`CHANGELOG.md` ~12: replace**

````markdown
  - **Transposition** by quarter tones: `Note.transpose()`, `Chord.transpose()`, `Chord.transposeStackOnly()` and `Helper.transposePitch()` take a `float` number of semitones, which must be a multiple of 0.5 (anything else raises `RuntimeError` naming the value), and compute on exact positions: `Note("C4").transpose(0.5)` gives `C1x4`, and `C1x4` transposed by 2 gives `D1x4`. `Helper.steps2pitch()` spells an exact position, and `Helper.validateTransposeSemitones()` checks an interval
  - **MusicXML.** `Score(path)` reads a quarter tone from `<accidental>` — the Tartini names `quarter-sharp`, `three-quarters-sharp`, `quarter-flat` and `three-quarters-flat`, and the arrow glyphs `sharp-down`, `sharp-up`, `flat-up` and `flat-down` — or from a decimal `<alter>` that is exactly a multiple of 0.5 from -2 to 2, read with a `.` decimal point whatever the C or C++ locale (a near value such as `0.46` reads as natural, with a warning). `sharp-sharp`, the double sharp drawn as two sharps, is read as a double sharp. `Note.toXML()` writes a quarter tone's `<alter>` with one decimal place (`0.5`, `-1.5`) and its Tartini `<accidental>`, which no key signature can imply; a whole-tone accidental is written as before, as `<alter>` alone. What this changes for files that already held quarter-tone accidentals is under Fix
  - **Analysis.** A method whose return type can express a quarter tone computes it: `Chord.toCents()` (`["C4", "E1b4", "G4"]` gives `[350, 350]`), `Chord.isSorted()`, `Chord.sortNotes()` and `Note`'s ordering operators (exact positions), both `Chord.getHarmonicDensity()` overloads, `Chord.getMidiValueStd()`, `Helper.isEnharmonic()` (`C1x4` and `D3b4` are enharmonic, `C1x4` and `C#4` are not) and `Helper.getSemitonesDifferenceBetweenMelodies()` (a neutral third against a major third differs by 0.5). A method whose return type cannot raises `RuntimeError`, naming the offending note and a remedy the caller can apply. The harmonic analysis (`getName()`, `getQuality()`, `getRoot()`, `getBassNote()`, `getDegree()`, `stackSize()`, `getStackedHeaps()`, the stack getters, the `is*` and `have*` predicates), the `Chord` methods that build intervals (`getIntervals()`, `getIntervalsFromOriginalSortedNotes()`) and the MIDI-integer methods (`getMidiIntervals()`, `getMeanMidiValue()`, `getMeanOfExtremesMidiValue()`, `getMeanPitch()`, `getMeanOfExtremesPitch()`) name `Chord.roundQuarterTones()`, which rounds a chord's quarter tones in place, ties upward, and returns how many notes it changed; `Interval` names `Note.roundToSemitone()`; and the melody-pattern search (`Score.findMelodyPatternDataFrame()`) compares quarter tones exactly, a match whose transposition is a quarter tone having an empty `transposeInterval` and its exact `transposeSemitones`. `Chord.info()` still prints a quarter-tone chord's notes, without the analysis. `maialib.plotScorePitchEnvelope()` uses the mean methods, so it raises for a score with quarter tones until they are rounded
````

with

````markdown
  - **Transposition** by quarter tones: `Note.transpose()`, `Chord.transpose()`, `Chord.transposeStackOnly()` and `Helper.transposePitch()` take a `float` number of semitones, which must be a multiple of 0.5 (anything else raises `RuntimeError` naming the value), and compute on exact positions: `Note("C4").transpose(0.5)` gives `C1x4`, and `C1x4` transposed by 2 gives `D1x4`. `Helper.steps2pitch()` spells an exact position, and `Helper.validateTransposeSemitones()` checks an interval
  - **MusicXML.** `Score(path)` reads a quarter tone from `<accidental>` — the Tartini names `quarter-sharp`, `three-quarters-sharp`, `quarter-flat` and `three-quarters-flat`, and the arrow glyphs `sharp-down`, `sharp-up`, `flat-up` and `flat-down` — or from a decimal `<alter>` that is exactly a multiple of 0.5 from -2 to 2, read with a `.` decimal point whatever the C or C++ locale (a near value such as `0.46` reads as natural, recorded as `ALTER_OFF_GRID` in the import report). `sharp-sharp`, the double sharp drawn as two sharps, is read as a double sharp. `Note.toXML()` writes a quarter tone's `<alter>` with one decimal place (`0.5`, `-1.5`) and its Tartini `<accidental>`, which no key signature can imply; a whole-tone accidental is written as before, as `<alter>` alone. What this changes for files that already held quarter-tone accidentals is under Fix
  - **Analysis.** A method whose return type can express a quarter tone computes it: `Chord.toCents()` (`["C4", "E1b4", "G4"]` gives `[350, 350]`), `Chord.isSorted()`, `Chord.sortNotes()` and `Note`'s ordering operators (exact positions), both `Chord.getHarmonicDensity()` overloads, `Chord.getMidiValueStd()`, `Helper.isEnharmonic()` (`C1x4` and `D3b4` are enharmonic, `C1x4` and `C#4` are not) and `Helper.getSemitonesDifferenceBetweenMelodies()` (a neutral third against a major third differs by 0.5). A method whose return type cannot raises `RuntimeError`, naming the offending note and a remedy the caller can apply. The harmonic analysis (`getName()`, `getQuality()`, `getRoot()`, `getBassNote()`, `getDegree()`, `stackSize()`, `getStackedHeaps()`, the stack getters, the `is*` and `have*` predicates), the `Chord` methods that build intervals (`getIntervals()`, `getIntervalsFromOriginalSortedNotes()`) and the MIDI-integer methods (`getMidiIntervals()`, `getMeanMidiValue()`, `getMeanOfExtremesMidiValue()`, `getMeanPitch()`, `getMeanOfExtremesPitch()`) name `Chord.roundQuarterTones()`, which rounds a chord's quarter tones in place, ties upward, and returns how many notes it changed; `Interval` names `Note.roundToSemitone()`; and the melody-pattern search (`Score.findMelodyPatternDataFrame()`) compares quarter tones exactly, a match whose transposition is a quarter tone having an empty `transposeInterval` and its exact `transposeSemitones`. `Chord.info()` still prints a quarter-tone chord's notes, without the analysis. `maialib.plotScorePitchEnvelope()` uses the mean methods, so it raises for a score with quarter tones until they are rounded
````


**`CHANGELOG.md` ~27: replace**

````markdown
  - The export writes, for each staff, a `<transpose>` in measure 1 when its first pitched note is transposed or doubled, and one wherever the interval or the doubling changes: in the measure's `<attributes>` when the staff's first pitched note there brings the change, otherwise in an `<attributes>` written just before the note; a `<transpose>` at the start of a measure has no `number` when every staff has the same transposition there, and one written before a note of a part with more than one staff always has it. The interval is unfolded into `<diatonic>`, `<chromatic>` and, from an octave on, `<octave-change>`; a stored diatonic interval of 0 is written as the conventional one. A chord whose notes have different intervals or doublings cannot be written: the export raises `RuntimeError` naming the part, the measure and the staff
  - The reader corrects or ignores a `<transpose>` with one warning per correction, which starts with a code: `[transpose-chromatic-not-integer]` and `[transpose-octave-change-not-integer]`, a value that is not a whole number, ignored; `[transpose-pair-corrected]`, a `<diatonic>` that does not match `<chromatic>`, replaced by the conventional diatonic interval (a tritone accepts the augmented fourth and the diminished fifth); `[transpose-out-of-range]`, an interval with which a note of its scope would have no sounding pitch, ignored for its whole scope. An ignored `<transpose>` leaves the previous transposition in force, and a note that cannot sound with that one either is read untransposed. `<for-part>` is not modelled: `[for-part-not-modelled]` reports it and it is dropped
- **Melody search over every melodic line, `findAnyMelodyPatternDataFrame()` and `ScoreCollection` discovery**
  - `Score.findAnyMelodyPatternDataFrame(patternNumNotes=5, intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0, minOccurrences=2, ...)` gives the distinct melodic patterns of a length that have at least `minOccurrences` matches (by default, the patterns that recur exactly, transposed or not), with their matches: the columns of `findMelodyPatternDataFrame()` after `patternIdx` and `patternPitches` (the pattern's written pitches). Windows that are the same notes and rests at the same exact positions relative to their first sounding note, with the same durations, are one pattern, the first kept. In C++, `Score::findAnyMelodyPattern()` returns `std::vector<Score::FoundMelodyPattern>` (`pattern`, `matches`)
  - Durations are compared exactly, so proportional rhythms are different patterns: `C4 D4` in quarters and `E4 F#4` in halves are two patterns, with the same matches. Even at the defaults an orchestral score gives a large table, because doubled parts repeat each other's windows: the Beethoven 5 sample gives 2,164 patterns and about 489,000 rows, in about 13 s; lower thresholds can give millions of rows
  - The melody-search results have the columns `voice` (as written), `concertKey` (the score's concert key at the measure, as `getChords()` computes it), `transposeSemitones` (the exact interval from the pattern's first sounding note to the match's, quarter tones included; `NaN` when either has none) and `soundingPitches`
  - `ScoreCollection()` builds an empty collection; the directory constructors and `setDirectoriesPaths()` take `recursive=False`, which reads subdirectories at any depth when `True`
- Add `Helper.getLibraryVersion()` (C++ and Python) so the compiled `maiacore` library itself can report the version from the root `VERSION` file, matching `maialib.__version__` and `maialib.maiacore.__version__`
````

with

````markdown
  - The export writes, for each staff, a `<transpose>` in measure 1 when its first pitched note is transposed or doubled, and one wherever the interval or the doubling changes: in the measure's `<attributes>` when the staff's first pitched note there brings the change, otherwise in an `<attributes>` written just before the note; a `<transpose>` at the start of a measure has no `number` when every staff has the same transposition there, and one written before a note of a part with more than one staff always has it. The interval is unfolded into `<diatonic>`, `<chromatic>` and, from an octave on, `<octave-change>`; a stored diatonic interval of 0 is written as the conventional one. A chord whose notes have different intervals or doublings cannot be written: the export raises `RuntimeError` naming the part, the measure and the staff
  - The reader corrects or ignores a `<transpose>` with one record of the import report (see below) per correction: `TRANSPOSE_CHROMATIC_NOT_INTEGER` and `TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER`, a value that is not a whole number, ignored; `TRANSPOSE_PAIR_CORRECTED`, a `<diatonic>` that does not match `<chromatic>`, replaced by the conventional diatonic interval (a tritone accepts the augmented fourth and the diminished fifth); `TRANSPOSE_OUT_OF_RANGE`, an interval with which a note of its scope would have no sounding pitch, ignored for its whole scope. An ignored `<transpose>` leaves the previous transposition in force, and a note that cannot sound with that one either is read untransposed. `<for-part>` is not modelled: it is dropped and recorded as `FOR_PART_NOT_MODELLED`
- **Melody search over every melodic line, `findAnyMelodyPatternDataFrame()` and `ScoreCollection` discovery**
  - `Score.findAnyMelodyPatternDataFrame(patternNumNotes=5, intervalSimilarityThreshold=1.0, rhythmSimilarityThreshold=1.0, minOccurrences=2, ...)` gives the distinct melodic patterns of a length that have at least `minOccurrences` matches (by default, the patterns that recur exactly, transposed or not), with their matches: the columns of `findMelodyPatternDataFrame()` after `patternIdx` and `patternPitches` (the pattern's written pitches). Windows that are the same notes and rests at the same exact positions relative to their first sounding note, with the same durations, are one pattern, the first kept. In C++, `Score::findAnyMelodyPattern()` returns `std::vector<Score::FoundMelodyPattern>` (`pattern`, `matches`)
  - Durations are compared exactly, so proportional rhythms are different patterns: `C4 D4` in quarters and `E4 F#4` in halves are two patterns, with the same matches. Even at the defaults an orchestral score gives a large table, because doubled parts repeat each other's windows: the Beethoven 5 sample gives 2,164 patterns and about 489,000 rows, in about 13 s; lower thresholds can give millions of rows
  - The melody-search results have the columns `voice` (as written), `concertKey` (the score's concert key at the measure, as `getChords()` computes it), `transposeSemitones` (the exact interval from the pattern's first sounding note to the match's, quarter tones included; `NaN` when either has none) and `soundingPitches`
  - `ScoreCollection()` builds an empty collection; the directory constructors and `setDirectoriesPaths()` take `recursive=False`, which reads subdirectories at any depth when `True`
- **Import report**
  - `Score.getImportIssues()` returns what the MusicXML reader corrected or dropped while loading a file, one `ImportIssue` (C++ struct, Python class) per record: `code` (UPPER_SNAKE, never renamed), `kind` (`"corrected"`: a value was replaced or filled; `"dropped"`: an element was removed), `partIndex` and `partName`, `measureNumber` (as the file writes it) and `measureIndex`, `element` (its path, such as `note/pitch/alter`), `found` (the value read), `used` (the value stored) and `message`. `Score.getImportIssuesDataFrame()` has one column per field, `str` or `int64`, and keeps them when the report is empty. The report is copied with the score, emptied by `clear()` and left unchanged by edits and exports; a score built through the API has none
  - The codes: `ACCIDENTAL_ALTER_MISMATCH`, `ACCIDENTAL_NAME_UNKNOWN`, `ALTER_OFF_GRID`, `DIVISIONS_MISSING`, `PART_NAME_DUPLICATE`, `STAFF_CLAMPED`, `TUPLET_CLAMPED`, `VOICE_NOT_POSITIVE` and the four `TRANSPOSE_*` codes are corrections; `FOR_PART_NOT_MODELLED` and `ELEMENT_NOT_MODELLED` are dropped elements. `ELEMENT_NOT_MODELLED` is one record per element path outside the closed element list -- the elements the model holds, matched by path -- with the number of such elements in `found`; they are dropped on export
  - A load whose report is not empty prints one line: `[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on export); see Score.getImportIssues()`
- `ScoreCollection.getLoadErrors()` lists the files that failed to load in the collection's last load, as `(path, message)` pairs
- Add `Helper.getLibraryVersion()` (C++ and Python) so the compiled `maiacore` library itself can report the version from the root `VERSION` file, matching `maialib.__version__` and `maialib.maiacore.__version__`
````


**`CHANGELOG.md` ~77: replace**

````markdown
- **Breaking:** `Score.findMelodyPatternDataFrame()` with a list of patterns — and `ScoreCollection.findMelodyPatternDataFrame()` and the C++ `Score::findMelodyPattern()` list overload — searched only as many patterns as the machine has hardware threads, returning the rest empty, and returned a pattern empty when its search failed, printing the error to the C++ standard error. Every pattern is now searched, and a failing search raises its error, as the single-pattern search does
- **Breaking:** `Score(path)` ignored `<accidental>` and read only an `<alter>` of -2, -1, 1 or 2, so a quarter-tone accidental, or a decimal `<alter>` such as `0.5`, loaded as natural. Those notes now load as quarter tones (see Added), so the harmonic analysis and interval methods raise for such a score until its quarter tones are rounded (e.g. `Note.roundToSemitone()` on every note through `Score.forEachNote()`). An `<accidental>` also wins over a disagreeing `<alter>`, with a warning (`<alter>1</alter>` with `<accidental>natural</accidental>` loaded `C#4`, now `C4`), and is read without one (`<accidental>sharp</accidental>` alone loaded `C4`, now `C#4`)
- **Breaking:** In C++, `Helper::alterValue2Name()` gives the Tartini names for quarter tones (`0.5` is `"quarter-sharp"`, where it was the arrow name `"sharp-down"`; likewise `"three-quarters-sharp"`, `"quarter-flat"` and `"three-quarters-flat"`), and `Helper::alterValue2symbol()` and `Helper::alterValue2Name()` accept only an alter exactly on the quarter-tone grid from -2 to 2, raising, with the value, for any other: a value within 0.05 of one, such as `0.46`, was rounded to it. Neither depends on the global C++ locale any more (under a comma-decimal one they raised for every value), and `-0.0` is the natural
- **Breaking:** `Score(path)` read `<transpose>` in a part's first measure only and ignored `<octave-change>`, `<double>` and `number`, taking the first `<diatonic>` and the first `<chromatic>` of that measure even from different elements: octave-transposing parts — contrabasses, contrabassoons, piccolos, celestas, bass clarinets — sounded an octave from what the file says, and a change of transposition was ignored (W3C 72c kept its E-flat clarinet in measure 2). Sounding pitches, `Score.getChords()` and everything built on it change for such files, among them the Beethoven 5, Dvořák and Mahler 8 samples, W3C 41c and `test_getchords_poly.musicxml`
- **Breaking:** `Score.getChords()` and `Score.getChordsDataFrame()` report the concert key of each chord's measure: the most frequent written key among the pitched parts that are untransposed there or transposed by whole octaves (a tie goes to the first part), or, when every pitched part transposes, the first pitched part's written key moved by its interval and brought into -6..11 fifths. They reported the first part's written key, which disagreed with the concert-pitch chords whenever that part transposes (W3C 72a reported D major for a C major chord)
- **Breaking:** A `<transpose>` whose `<diatonic>` and `<chromatic>` disagree is corrected: the Dvořák sample's trumpets in E, `(3, 4)`, are read as `(2, 4)`, so `getTransposeDiatonic()` returns 2 where it returned 3, and the analyses at concert pitch (see Added) still relate a written `F#4` as `A#4`, the major third above it, which is also what it sounds. A `<transpose>` without `<diatonic>` stores the conventional diatonic interval, where it stored 0
- **Breaking:** A `<transpose>` that would push a note out of the representable range is ignored with a `[transpose-out-of-range]` warning (see Added), where it was applied: a written `C8` with `<chromatic>60</chromatic>` had MIDI number 168 and sounded `C13`
- **Breaking:** Exports write `<transpose>`, so `Score.__hash__` and `Part.__hash__`, which hash the export, change for scores with transpositions; an export used to lose every transposition, so a re-imported score sounded its written pitches
````

with

````markdown
- **Breaking:** `Score.findMelodyPatternDataFrame()` with a list of patterns — and `ScoreCollection.findMelodyPatternDataFrame()` and the C++ `Score::findMelodyPattern()` list overload — searched only as many patterns as the machine has hardware threads, returning the rest empty, and returned a pattern empty when its search failed, printing the error to the C++ standard error. Every pattern is now searched, and a failing search raises its error, as the single-pattern search does
- **Breaking:** `Score(path)` ignored `<accidental>` and read only an `<alter>` of -2, -1, 1 or 2, so a quarter-tone accidental, or a decimal `<alter>` such as `0.5`, loaded as natural. Those notes now load as quarter tones (see Added), so the harmonic analysis and interval methods raise for such a score until its quarter tones are rounded (e.g. `Note.roundToSemitone()` on every note through `Score.forEachNote()`). An `<accidental>` also wins over a disagreeing `<alter>`, recorded as `ACCIDENTAL_ALTER_MISMATCH` (`<alter>1</alter>` with `<accidental>natural</accidental>` loaded `C#4`, now `C4`), and is read without one (`<accidental>sharp</accidental>` alone loaded `C4`, now `C#4`)
- **Breaking:** In C++, `Helper::alterValue2Name()` gives the Tartini names for quarter tones (`0.5` is `"quarter-sharp"`, where it was the arrow name `"sharp-down"`; likewise `"three-quarters-sharp"`, `"quarter-flat"` and `"three-quarters-flat"`), and `Helper::alterValue2symbol()` and `Helper::alterValue2Name()` accept only an alter exactly on the quarter-tone grid from -2 to 2, raising, with the value, for any other: a value within 0.05 of one, such as `0.46`, was rounded to it. Neither depends on the global C++ locale any more (under a comma-decimal one they raised for every value), and `-0.0` is the natural
- **Breaking:** `Score(path)` read `<transpose>` in a part's first measure only and ignored `<octave-change>`, `<double>` and `number`, taking the first `<diatonic>` and the first `<chromatic>` of that measure even from different elements: octave-transposing parts — contrabasses, contrabassoons, piccolos, celestas, bass clarinets — sounded an octave from what the file says, and a change of transposition was ignored (W3C 72c kept its E-flat clarinet in measure 2). Sounding pitches, `Score.getChords()` and everything built on it change for such files, among them the Beethoven 5, Dvořák and Mahler 8 samples, W3C 41c and `test_getchords_poly.musicxml`
- **Breaking:** `Score.getChords()` and `Score.getChordsDataFrame()` report the concert key of each chord's measure: the most frequent written key among the pitched parts that are untransposed there or transposed by whole octaves (a tie goes to the first part), or, when every pitched part transposes, the first pitched part's written key moved by its interval and brought into -6..11 fifths. They reported the first part's written key, which disagreed with the concert-pitch chords whenever that part transposes (W3C 72a reported D major for a C major chord)
- **Breaking:** A `<transpose>` whose `<diatonic>` and `<chromatic>` disagree is corrected: the Dvořák sample's trumpets in E, `(3, 4)`, are read as `(2, 4)`, so `getTransposeDiatonic()` returns 2 where it returned 3, and the analyses at concert pitch (see Added) still relate a written `F#4` as `A#4`, the major third above it, which is also what it sounds. A `<transpose>` without `<diatonic>` stores the conventional diatonic interval, where it stored 0
- **Breaking:** A `<transpose>` that would push a note out of the representable range is ignored and recorded as `TRANSPOSE_OUT_OF_RANGE` (see Added), where it was applied: a written `C8` with `<chromatic>60</chromatic>` had MIDI number 168 and sounded `C13`
- **Breaking:** Exports write `<transpose>`, so `Score.__hash__` and `Part.__hash__`, which hash the export, change for scores with transpositions; an export used to lose every transposition, so a re-imported score sounded its written pitches
````


**`CHANGELOG.md` ~93: replace**

````markdown
- **Breaking:** `Measure.removeNote(noteId, staveId=0)` removes exactly that note: it erased the notes before it (`removeNote(0)` removed nothing), and an index or a staff outside the measure, which was undefined behaviour, raises `IndexError` (C++ `std::out_of_range`). `Measure.addNote()` with a list inserts it in list order (it was reversed), all or nothing; a position past the end or a staff outside the measure raises `IndexError` (a staff number too large raised `RuntimeError`)
- **Breaking:** `ScoreCollection.setDirectoriesPaths()` replaces the collection's scores, all or nothing, where it appended them (1, then 2, then 3 scores). Discovery matches `.xml`, `.mxl` and `.musicxml` without regard to case (`.XML` and `.MusicXML` were skipped) and loads in sorted path order (it used the file system's). `ml.ScoreCollection()` builds an empty collection, and a path that does not exist or is not a directory raises `RuntimeError` naming it — both raised `UnicodeDecodeError` from a localised message on a Windows system with a non-English locale; in C++ `ScoreCollection collection;` was ambiguous. `removeScore()` raises `IndexError` for any index outside the collection, where `removeScore(-1)` crashed the interpreter. With `recursive=True` a subdirectory the user has no permission to read is skipped; a directory that cannot be read raises `RuntimeError` naming it, a subdirectory as well as a given path; and a file that fails to load raises `RuntimeError` whose message starts with the file's path, the collection unchanged
- `Measure.clear()` empties every staff and keeps it: it removed the staves, so `getNumNotes(staveId)` and the melody search of a score holding the cleared measure raised `IndexError`

````

with

````markdown
- **Breaking:** `Measure.removeNote(noteId, staveId=0)` removes exactly that note: it erased the notes before it (`removeNote(0)` removed nothing), and an index or a staff outside the measure, which was undefined behaviour, raises `IndexError` (C++ `std::out_of_range`). `Measure.addNote()` with a list inserts it in list order (it was reversed), all or nothing; a position past the end or a staff outside the measure raises `IndexError` (a staff number too large raised `RuntimeError`)
- **Breaking:** `ScoreCollection.setDirectoriesPaths()` replaces the collection's scores, all or nothing, where it appended them (1, then 2, then 3 scores). Discovery matches `.xml`, `.mxl` and `.musicxml` without regard to case (`.XML` and `.MusicXML` were skipped) and loads in sorted path order (it used the file system's). `ml.ScoreCollection()` builds an empty collection, and a path that does not exist or is not a directory raises `RuntimeError` naming it — both raised `UnicodeDecodeError` from a localised message on a Windows system with a non-English locale; in C++ `ScoreCollection collection;` was ambiguous. `removeScore()` raises `IndexError` for any index outside the collection, where `removeScore(-1)` crashed the interpreter. With `recursive=True` a subdirectory the user has no permission to read is skipped; a directory that cannot be read raises `RuntimeError` naming it, a subdirectory as well as a given path; and a file that fails to load is skipped and listed by `getLoadErrors()` (see the import report entries below)
- `Measure.clear()` empties every staff and keeps it: it removed the staves, so `getNumNotes(staveId)` and the melody search of a score holding the cleared measure raised `IndexError`

- **Breaking:** Loading a MusicXML file prints no `[WARN]` or `[INFO]` line per correction. The accidental and `<transpose>` warnings, printed to `sys.stdout` while loading, are records of the import report (see Added), and a load with records prints one summary line; a script that read the warnings reads `getImportIssues()`, whose codes are UPPER_SNAKE (`[transpose-pair-corrected]` is `TRANSPOSE_PAIR_CORRECTED`). The reader also records what it corrected without a word: a part renamed with a suffix because another part has its name (the `[INFO]` line), a first measure without `<divisions>` read at 256 divisions, and a voice, a staff or a tuplet value that is not a positive whole number. A negative voice or tuplet value, which was kept, is read as 1
- Loading a file whose text is not UTF-8 where the reader quotes it -- an `<alter>` holding a Latin-1 byte -- ended the Python interpreter (0xC0000409, from the destructor of pybind11's redirected stream). Text from the file reaches a record, a message or the console as valid UTF-8, each invalid byte replaced by U+FFFD; the title, the composer and the part names are held that way too, where their getters raised `UnicodeDecodeError`
- **Breaking:** A file that cannot be read raises `RuntimeError` naming the file and the problem: `Score: cannot open '<path>'`; `Score: '<path>' is not well-formed XML: <description> (byte offset <n>)`, with pugixml's description and where it stopped; for an `.mxl`, `Score: '<path>' is not a readable MusicXML archive: ` and the problem (not a zip archive, no `META-INF/container.xml`, no rootfile named, the rootfile not in the archive), or the rootfile's own parse error. The message was `Unable to load the file: <path>`, or miniz's own words for an archive
- `Score(path)` opens any path on Windows, whatever its characters: `canção.xml` did not open, its UTF-8 path read in the ANSI code page. `getFileName()` and `getFilePath()` return UTF-8, where the scores of a `ScoreCollection` raised `UnicodeDecodeError` for `canção.xml` and the collection raised for a name outside the code page, such as `日本.xml`. An `.mxl` archive is recognised by its extension in any case (`SCORE.MXL` was parsed as XML) and by the zip signature its bytes start with, so a zip archive named `.xml` loads; a file named `.mxl` that is not a zip archive raises `RuntimeError` (`it is not a zip archive`). A load that prints a name the console cannot encode, as when the output is piped on Windows, writes it with backslash escapes instead of ending the interpreter
- **Breaking:** `ScoreCollection` skips a file that fails to load and lists it in `getLoadErrors()`, with one line saying how many files failed: the constructor, `setDirectoriesPaths()` and `addScore()` with paths raised `RuntimeError` for one failing file, and the collection kept its old scores. A directory that does not exist, is not a directory or cannot be read still raises before anything changes. The `addScore()` overloads print to Python's `sys.stdout`, as the other loading methods do; they printed to the C runtime's stdout

````


**`CHANGELOG.md` ~127: replace**

````markdown
- **Breaking:** The Makefile ignores an environment variable named `PYTHON`; pass the interpreter on the command line instead, e.g. `make "PYTHON=py -3.12"`

````

with

````markdown
- **Breaking:** The Makefile ignores an environment variable named `PYTHON`; pass the interpreter on the command line instead, e.g. `make "PYTHON=py -3.12"`
- The corpus ledger records `codes`, the distinct codes of a file's import report, and `exit`, a worker's exit status other than 0, which a crash after the worker's final record left invisible; a ledger that would exceed 500,000 bytes keeps its codes in `<ledger>-codes.json` (`ledger-external-codes.json`). The corpus worker loads every file from its own path, without the ASCII-named copy it made of a non-ASCII path. `make corpus` and `make corpus-update-ledger` take `CORPUS_ARGS` (`--in-repo-only`, `--skip-slow`, `--filter SUBSTRING`, `--workers N`), and `make fuzz FUZZ_ARGS="--accept"` exits 1 when a case's outcome is worth minimising

````

- [ ] **Step 2: Nothing names the old warnings.** Bash: `git -C /c/Users/nyck/Desktop/maialib grep -n -e "\[transpose-" -e "for-part-not-modelled" -e "Unrecognized <accidental>" -e "Unrepresentable <alter>" -e "Unable to load the file" -e "Adding part names index suffix" -- '*.md' '*.h' '*.cpp' '*.py' ':!docs/superpowers' ':!AI_API_CHEATSHEET.md' ':!llms-full.txt' ':!DEVELOPMENT_PLAN.md' ':!CHANGELOG.md'` → no line (`AI_API_CHEATSHEET.md` and `llms-full.txt` are regenerated at release; `docs/superpowers` holds the designs and plans that quote them; the CHANGELOG names the old messages on purpose). Commit `git add CHANGELOG.md`, message:

```
docs: changelog of the import report, Unicode paths and collection isolation

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV
```

- [ ] **Step 3: Final verification (spec §6)** from the committed tree, in a second brand-new venv:
  1. PowerShell: `py -3.12 -m venv C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-final-venv; & 'C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-final-venv\Scripts\python.exe' -m pip install -r requirements-dev.txt; $LASTEXITCODE` → 0; in «build» below use `$py = 'C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-final-venv\Scripts\python.exe'`.
  2. «build» `make "PYTHON=$py" dev` → 0; `git checkout -- AI_API_CHEATSHEET.md llms-full.txt`.
  3. «build» `make "PYTHON=$py" cpp-tests` twice → 0 both times, `[  PASSED  ] 1216 tests.` (1188 + 10 + 5 + 4 + 3 + 4 + 2).
  4. «build» `make "PYTHON=$py" py-tests` → OK, `Ran 718 tests` (690 + 6 + 1 + 1 + 7 + 4 + 2 + 7), `OK (skipped=1)`; record the duration.
  5. «build» `make "PYTHON=$py" validate` → no new findings (measured on a scratch copy of the finished tree: `Validation: no new findings (44 known).`, `21 baseline finding(s) no longer reported`).
  6. «build» `make "PYTHON=$py" corpus` (the external corpus fetched) → 0: neither ledger nor the sidecar differs, and the round trip ran.
  7. «build» `make "PYTHON=$py" msvc-gate` → 0.
  8. «build» `make "PYTHON=$py" linux-gate` → 0. If it exits 2 listing missing apt packages (this machine's WSL has no `cmake` and no password-less sudo), run the Linux route instead and report it: in Bash, `git -C /c/Users/nyck/Desktop/maialib -c core.autocrlf=false -c core.eol=lf archive HEAD` extracted under `/var/tmp/maialib-4c1a` in WSL; there a venv with `requirements-dev.txt` plus `cmake` from pip; `make "PYTHON=<venv>/bin/python" dev` and `make "PYTHON=<venv>/bin/python" py-tests` → 0, `Ran 717 tests` (the `.gitignore` of the archive keeps `test/musicxml/external` out, so the external corpus does not run there); remove `/var/tmp/maialib-4c1a` afterwards.
  9. «build» `make "PYTHON=$py" fuzz` → 0 (report mode); compare `test\musicxml\fuzz-work\report-seed-1.json` with `C:\Users\nyck\AppData\Local\Temp\maialib-4c1a-fuzz-baseline.json` by outcome, not case by case — the new fixture shifts the list of files the cases pick from (`test/musicxml/README.md`): no `crash:*` or `timeout:*` outcome the baseline does not have, and each changed count explained (a mutated negative `<voice>` or tuplet value is now read as 1; an `.mxl` mutant of 1 to 3 bytes is now `load:RuntimeError:unreadable` without reaching miniz). «build» `make "PYTHON=$py" fuzz 'FUZZ_ARGS=--accept'` → 1, as the spec expects until 4c-1b; list the outcomes it names.
  10. Import check from outside the repository (Task 0, Step 5) with the final venv, and Task 0's crash script → `exit 0`; `git status --short` → ` M .gitignore` only; `git log --oneline "$(git log -1 --format=%H -- docs/superpowers/plans/2026-10-06-import-report.md)..HEAD"` lists exactly the eight task commits (Tasks 1-8), which follow the plan's commit.
  Put every count, duration and comparison in the task report.

---

## Self-review (done while writing)

- **Spec coverage.** §1 problems → Task 1 (the three accidental warnings without part or measure; the 0xC0000409 crash), Task 2 (step 1b's five warnings as finished strings; the silent corrections), Task 4 (pugixml description and offset dropped; raw miniz messages), Task 5 (non-ASCII paths; the ASCII copy), Task 6 (all or nothing; `addScore` to the C stdout), Task 7 (`make fuzz` always 0; the invisible exit; no codes; no subset; the worker count written twice; the external ledger's size). §2 `ImportIssue` fields → Task 1 (struct, class, `==`); API: list and DataFrame with fixed dtypes and an empty frame with every column, copy/assignment, `clear()`, edits and exports, API-built empty → Task 1 (`TheReportIsCopiedKeptThroughEditsAndEmptiedByClear`, `AScoreBuiltThroughTheApiHasAnEmptyReport`, the Python tests); records: accidental codes with part and measure → Task 1; step 1b's codes with structured `found`/`used` and no bracketed prefix → Task 2; `PART_NAME_DUPLICATE`, `DIVISIONS_MISSING`, `VOICE_NOT_POSITIVE`, `STAFF_CLAMPED`, `TUPLET_CLAMPED` → Task 2; dropped elements by path with their count, one extra pass → Task 3; the summary line only when the report is not empty, no per-event `[WARN]` → Tasks 1-3; text from the file as valid UTF-8 → Task 1 (records, messages, the summary; names by Decision 10); fatal input → Task 4. §3 wide-character open on Windows, `.mxl` from the wide path, UTF-8 name and path → Task 5 (Decision 8 for the mechanism); `ScoreCollection` `u8path` → Task 5; the worker without the copy and the external ledger → Tasks 5 and 7 (Decision 14). §4 isolation, `getLoadErrors()` for the constructor, `setDirectoriesPaths` and `addScore` by path, the summary line, a bad directory still raising before anything changes, `addScore` redirected → Task 6. §5 `--accept`, `exit`, `codes`, the sidecar over 500,000 bytes, `CORPUS_ARGS` with the four options and the worker count defined once → Task 7. §6 fixtures or probes with mutations → every task's mutation step; the tests that captured warnings (C++ accidental and transpose tests, `test_score_comprehensive.py`, `test_musicxml_dump.py`, `test_musicxml_transpose.py`, `test_score_collection.py`) → Tasks 1, 2, 5, 6; step 5's all-or-nothing tests → Task 6; Doxygen and numpydoc → Tasks 1-6; CHANGELOG relative to v1.10.3 → Task 8; ledger changes explained → Tasks 3, 5, 7; verification list → Task 8 Step 3. §7 out of scope: untouched (the reader's crash sites stay, so `make fuzz --accept` fails until 4c-1b).
- **Not mapped, deliberately:** umbrella §6's policy table (`docs/musicxml-import-policy.md`), the catalogue's message templates and its meta-test, deferred to phase 4c-2 by spec §7 (Decision 2); umbrella §5's timewise conversion and `<opus>` refusal, and parts matched by `id` (umbrella §12), which the 4c-1a spec leaves to 4c-1b; the stubs (`maialib/maiacore/*.pyi`), regenerated at release.
- **Interfaces are consistent across tasks:** `ImportIssue`, `makeIssue`, `IssueLocation`, `validUtf8`, `importSummary` (1 → 2, 3, 4, 5, 6); `correctionCodes`, `alterOffGrid`, `TemporaryFile`, `minimalScore`, `kMinimalAttributes`, `kWholeC4` (1 → 2-5); `issueFields`, `ISSUE_FIELDS`, `summaryLine` (1 → 3, 5); `transposeRecords`, `transposeIssue` (2 → 3); `droppedElements` (3); `fileBytes`, `notWellFormed`, `mxlRootfile` (4 → 5); `ConsoleRedirect`, `TemporaryDirectory::utf8()`, `runChild(code, environment)` (5 → 6); `getLoadErrors`, `loadInto`, `printLoadErrors` (6); `default_workers`, `select`, `ledger_after`, `codes_path`, `LEDGER_LIMIT` (7).
- **Measured facts the plan relies on** (a scratch copy of `b620ed1` with every task applied in order, built with clang 18, the module installed in a fresh venv): the counts after each task (C++ 1198, 1203, 1207, 1210, 1214, 1216, 1216; Python 696, 697, 698, 705, 709, 711, 718); every record, message and summary line quoted in the tests; pugixml's descriptions and offsets (`Start-end tags mismatch (byte offset 29)`, `Error parsing start element tag (byte offset 20)`, `No document element found (byte offset 11)`); the crash before Task 1 (exit 3221226505) and after (exit 0); mutations Task 5 (a) and (c); `make validate`; both ledgers regenerated with codes (18 minutes; `ledger-external.json` byte-identical, the sidecar 223,869 bytes); 60 non-ASCII external files loaded from their own paths, each matching its ledger line; the code blocks above are the diffs of that copy, each block's first text unique in its file when applied in order.
