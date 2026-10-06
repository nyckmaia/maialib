# Import report, Unicode paths and collection isolation (phase 4c-1a) — Design

**Status:** approved by the user on 2026-10-06 (two sections). Phase 4c-1 of the MusicXML robustness
design (`docs/superpowers/specs/2026-10-01-musicxml-robustness-design.md`, "the umbrella") is split
by the user's decision of 2026-10-06 into **4c-1a** (this document: the report and the
infrastructure) and **4c-1b** (the crash, hang and undefined-behaviour sites of umbrella §8 items
1–13 and 16, plus the defects found since, each becoming a recorded correction, with the fuzz
acceptance of umbrella §4). The umbrella's §5 is binding here; this document fixes what §5 leaves open.

## 1. Problem (measured on `main` @ `782be95`)

- The reader reports what it corrects as unstructured console lines, or not at all:
  three accidental/`<alter>` warnings without part or measure (`score.cpp` ~1166, ~1189, ~1204);
  step 1b's five `<transpose>` warnings, built as finished strings inside `TransposeElement`;
  silent corrections — a duplicate part name renamed (an `[INFO]` line), `<divisions>` defaulting to
  256, voice 0 read as 1, staff and tuplet values clamped.
- A warning that quotes bytes from the file that are not valid UTF-8 (e.g. `<alter>\xE9</alter>`)
  terminates the process (exit 0xC0000409): the bytes reach pybind11's `pythonbuf`, whose
  re-synchronisation in a `noexcept` destructor throws.
- Fatal load errors drop pugixml's description and offset (`score.cpp` ~693); `.mxl` failures carry
  raw miniz messages without the file name.
- Non-ASCII paths: `Score("…canção.xml")` fails to open on Windows (narrow, ANSI-interpreted paths
  reach pugixml and miniz); through `ScoreCollection` the file loads but `getFileName`/`getFilePath`
  raise `UnicodeDecodeError`; a name such as `日本.xml` makes the collection constructor raise. The
  corpus worker loads non-ASCII paths through an ASCII-named temporary copy.
- `ScoreCollection` aborts (all or nothing, since step 5) when one file fails to load; its `addScore`
  overloads print to the C stdout instead of Python's.
- Test tooling: `make fuzz` always exits 0; a worker that exits non-zero after its final record is
  invisible to the ledger; the ledger has no report codes; `make corpus` cannot run a subset and its
  worker count (`max(2, cpu // 2)`, written twice) cannot be set. The external ledger is 472,214 bytes.

## 2. The import report (umbrella §5)

- **`ImportIssue`** (C++ struct, pybind11 class `maialib.ImportIssue`): `code` (UPPER_SNAKE string,
  never renamed), `kind` (`"corrected"`: a value was replaced or filled; `"dropped"`: an element was
  removed), `partIndex` (int, −1 when not in a part), `partName`, `measureNumber` (the measure number
  as the file writes it, string; empty when not in a measure), `measureIndex` (int, −1 when not in a
  measure), `element` (path, e.g. `attributes/transpose/diatonic`), `found` (value read, empty when
  absent), `used` (value stored in the model), `message` (English).
- **API:** `Score.getImportIssues()` (list) and `Score.getImportIssuesDataFrame()` (one column per
  field, fixed dtypes: str for strings, int64 for indices; an empty report is an empty DataFrame with
  every column). The report describes the import: it is copied with the `Score` (copy constructor and
  assignment), reset by `clear()`, unchanged by edits and exports; a score built through the API has an
  empty report.
- **Records produced in 4c-1a:**
  - the accidental/`<alter>` warnings (codes `ACCIDENTAL_ALTER_MISMATCH`, `ALTER_OFF_GRID`,
    `ACCIDENTAL_NAME_UNKNOWN`, with part and measure);
  - step 1b's warnings, with the same meaning and new codes `TRANSPOSE_CHROMATIC_NOT_INTEGER`,
    `TRANSPOSE_OCTAVE_CHANGE_NOT_INTEGER`, `TRANSPOSE_PAIR_CORRECTED`, `TRANSPOSE_OUT_OF_RANGE`,
    `FOR_PART_NOT_MODELLED` (the bracketed kebab-case prefixes of step 1b's log lines disappear);
    `TransposeElement` keeps structured `found`/`used` values instead of finished strings;
  - the silent corrections: `PART_NAME_DUPLICATE`, `DIVISIONS_MISSING` (the 256 default),
    `VOICE_NOT_POSITIVE`, and each staff or tuplet clamp (`STAFF_CLAMPED`, `TUPLET_CLAMPED`);
  - **dropped elements:** one `dropped` record per element path not held by the model (the closed
    element list of umbrella §6, matched by path because names such as `type` and `staff` are
    ambiguous), with `found` holding the count and `element` the path; computed by one extra pass over
    the document.
  4c-1b adds the records of its corrections with the same mechanism.
- **Output while loading:** no per-event `[WARN]` line remains; one summary line is printed only when
  the report is not empty:
  `[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on export); see Score.getImportIssues()`.
  No message quotes raw file bytes unsanitised: any text taken from the file is converted to valid
  UTF-8 (invalid bytes replaced by U+FFFD) before it reaches a stream or a record.
- **Fatal input** raises `RuntimeError` with pugixml's description and byte offset, or, for an `.mxl`,
  with the file name and the archive problem (no readable `container.xml`, rootfile not found, not a
  zip).

## 3. Unicode paths
- `Score` opens files through `std::ifstream` on `std::filesystem::u8path` (wide paths on Windows) and
  parses them with `load_buffer`; `.mxl` is recognised by its extension in any case and by its `PK`
  signature, and an archive too short to hold a zip end record is refused before miniz reads it. `Score`
  keeps its file name and path as UTF-8 (`getFileName`, `getFilePath` never raise for a non-ASCII name);
  title, composer and part names that are not valid UTF-8 are stored with U+FFFD replacements.
- Console output that Python's stream cannot encode (a valid UTF-8 name on a console whose code page lacks
  its characters) must not end the process: the redirect to Python's streams is guarded so an encoding
  failure degrades the text instead of terminating.
- `ScoreCollection` builds its paths with `std::filesystem::u8path` and passes UTF-8 to `Score`.
- The corpus worker (`test/musicxml/corpus_worker.py`) loads every file from its own path; the
  ASCII-named copy is removed. The external ledger's records with non-ASCII paths are expected to stay
  identical (measured: the ASCII copy behaved like the real file); any change is explained.

## 4. ScoreCollection isolation
- A file that fails to load is skipped; `ScoreCollection.getLoadErrors()` returns `(path, message)`
  pairs for the last load (constructor, `setDirectoriesPaths`, `addScore` by path), and one summary line
  is printed when it is not empty. This replaces step 5's all-or-nothing reload; a directory that does
  not exist, or is not a directory, still raises before anything changes.
- `addScore` overloads redirect their output to Python's `sys.stdout`/`sys.stderr`.

## 5. Test tooling
- `make fuzz FUZZ_ARGS="--accept"`: exits non-zero when any case's outcome is worth minimising
  (`fuzz.worth_minimising`), and prints those cases; without `--accept` it keeps reporting only.
- Ledger records gain `exit` (the worker's exit code) only when it is non-zero, and `codes` (the sorted
  list of distinct report codes) only when non-empty. The external ledger's codes live in a sidecar file
  `ledger-external-codes.json` (measured at about 224 KB), compared the same way.
- `make corpus` and `make corpus-update-ledger` accept `CORPUS_ARGS` with `--in-repo-only`,
  `--skip-slow`, `--filter <substring>` and `--workers N` (default `max(2, cpu // 2)`, defined once).

## 6. Tests, docs and verification
- Every rule above has a fixture or a probe test, each proven to fail under a targeted mutation; the
  tests that capture today's warnings (about 25 C++ assertions, the Python tests of
  `test_score_comprehensive.py`, `test_musicxml_dump.py`, `test_musicxml_transpose.py`,
  `test_score_collection.py`) assert the report instead; step 5's all-or-nothing tests become isolation
  tests.
- Doxygen and numpydoc for every new API; CHANGELOG `[Unreleased]` relative to v1.10.3.
- Ledger changes: the external ledger's non-ASCII records and the added `codes`/`exit` fields; every
  changed line explained in its commit.
- Verification: brand-new venv `make dev`; `make cpp-tests` twice; `make py-tests`; `make validate`;
  `make corpus`; `make msvc-gate`; the Python tests on Linux; `make fuzz` (report mode; the acceptance
  mode is expected to fail until 4c-1b).

## 7. Out of scope (4c-1b and later)
- The crash/hang/undefined-behaviour sites (umbrella §8 items 1–13, 16) and the defects found since:
  `<beats>` 0 and a non-numeric `<beat-type>` breaking `getChords`, a huge `<duration>` hanging the load,
  a `<part>` without its `score-part`, W3C 33a-Spanners hanging in `getChords`, `<divisions>3` triplets
  hanging the load (4c-1b). Timewise conversion and `<opus>` refusal (umbrella §8 item 16) also go to
  4c-1b.
- A `<divisions>` change in the middle of a part (4c-2, attribute inheritance).
