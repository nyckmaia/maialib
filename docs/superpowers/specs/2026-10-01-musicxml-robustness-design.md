# MusicXML robustness and valid MusicXML 4.0 export (roadmap item 4) — Design

**Status:** decisions approved by the user on 2026-10-01 (recorded in §2); design sections 1–8 approved one by one.
**Roadmap:** item 4 of `C:/Users/nyck/.claude/plans/wobbly-coalescing-quill.md`, phases 4a → (1b, 5) → 4c-1 → 4b → 4c-2 → 4d-1..4d-4. This is the umbrella design; each phase gets its own implementation plan written from it, and each 4d sub-phase a short design of its own.
**User goal (2026-09-29, verbatim):** "Faça tratamentos especificos para cada tag que possa conter erro, defina os valores padrões em caso de falta ou de valor inválido, defina os valores corretos/corrigidos no objeto interno do maialib de modo que quando a partitura for exportada do Maialib para um novo arquivo XML o arquivo exportado esteja com todas as tags e valores corrigidos de acordo com o padrão MusicXML 4.0."

## 1. Problem (measured on `main` @ `7f6636d`)

**Reader** (`Score::loadXMLFile`, `maiacore/src/maiacore/score.cpp:188-866`): one absolute XPath query per part and measure (quadratic in the number of measures); attributes taken from the first match anywhere in the measure; notes stored per staff in document order with voices mixed; `<backup>`/`<forward>` never read.
- Crashes, hangs and undefined behaviour on real or slightly damaged files (16 sites, Appendix B). Examples: a part whose first measure has no `<key>` (valid MusicXML) throws `std::out_of_range`; a second `<clef>` in a measure throws `std::out_of_range` — the cause of the bundled `Mozart_Requiem_Introitus.mxl` failing to load (`IndexError` in Python); divisions that are not a power of two make the dot computation loop forever (`helper.cpp:808-811`); divisions 0 divide by zero.
- Silent wrong answers: `<time>` and `<divisions>` are not carried forward (a 3/4 piece reads as 4/4 from measure 2), clefs revert to measure 1's, non-major modes read as minor, non-traditional keys as C major, `3+2` beats as 3, part names paired with parts by position (ids ignored), a missing `<octave>` silently gives octave 0, cue notes count as sounding notes, `<forward>` time is lost.
- Errors surface as `RuntimeError`, `IndexError`, `ValueError` or `MemoryError`; the parser's error text is dropped; `.mxl` is detected by extension only; UTF-16 inside `.mxl` is cut; one bad file aborts a whole `ScoreCollection`.

**Writer** (`Score::toXML`/`toFile`, `Part`, `Measure`, `Note`, `Clef`, `Barline` `toXML`):
- Declares MusicXML 3.0; writes invented header content (rights "Copyright © ", fixed encoding date 2020-08-01, "Maialib 1.0.0", a Dolet note, false `<supports>`, fixed `<defaults>` with fonts and page layout) and an invented part abbreviation (first 4 bytes of the name, which can split a UTF-8 character).
- Invalid output: no XML escaping anywhere (an `&` in a title produces an ill-formed file); `<alter>` inside `<unpitched>`; `<accidental>` and `<time-modification>` before `<dot>`; octaves −1, 10, 11; `<backup>` written only in staff 1 with a wrong duration (multi-staff measures are overfull) and sometimes 0; no `<forward>`; `slur orientation=""`; `midi-unpitched` 0; scores built through the API have no `<divisions>`; the clef is repeated in every measure; a measure with fewer clefs than staves throws.
- `toFile` always appends `.xml` (`out.xml` → `out.xml.xml`); `.mxl` output has no `mimetype` and stores the full path as the inner file name.

**Test infrastructure:** no schema validation, no golden files, no expected-status ledger, no fuzzing; CI runs no tests. 6 of the 7 bundled samples load. Two tests pin invalid output.

## 2. Decisions (user, 2026-10-01)

| # | Decision |
|---|---|
| D1 | **Import report:** every correction is a record; `Score.getImportIssues()` (and a DataFrame) returns them; loading prints one summary line, only when there are records. |
| D2 | **No strict mode:** loading is always lenient; rigor means checking that `getImportIssues()` is empty. |
| D3 | **Octaves −1, 10, 11 on export** (MusicXML allows 0–9): the export raises, listing every offending note (part, measure, pitch); no invalid file leaves maialib. |
| D4 | **Corpus:** vendor, unmodified at pinned commits with their licence and attribution, the official MusicXML 4.0 XSD (5 files, 0.4 MB, W3C FSA) and the W3C/LilyPond test suite (183 files, 1.3 MB, MIT); `make tests` runs fully offline; OpenScore (CC0, ~60 MB) stays out of the repository, fetched on demand. |
| D5 | **Validation only in the tests:** `lxml` joins `requirements-dev.txt`; runtime dependencies do not change; no public validation function. |
| D6 | **Valid forms the model cannot hold are added to the model** (in 4c-2): `Key` keeps the mode as written (the 10 MusicXML modes), `ClefSign` gains TAB, NONE and JIANPU, `TimeSignature` keeps the written form (composite beats such as `3+2`, the common/cut symbol, senza-misura) plus an effective signature for analyses. Until then 4c-1 only prevents the crashes and records issues. |
| D7 | **All four 4d groups ship in the release:** header and parts; expression and tempo; structure and form; note notation. |
| D8 | **Architecture A:** 4c-1 patches the current reader for safety and adds the report; 4c-2 rewrites the reader as one document-order pass per part with the policy applied while reading. (Rejected: a separate DOM normaliser, which duplicates the state and timing logic; patching the XPath reader only, which keeps the quadratic walk and fragile timing.) |
| D9 | **`toFile` respects the extension:** `.xml`, `.musicxml` and `.mxl` names are used as given (`.mxl` compresses); without an extension it appends `.musicxml` or `.mxl` according to the compression flag; an extension contradicting the flag raises. |

Earlier roadmap decision (user, 2026-09-29): the model is the single source of truth and is extended gradually; content the model does not hold is dropped on export and reported.

## 3. Principles

1. The model is the single source of truth: an import corrects values in the model and records each correction; an export writes only what the model holds.
2. Every export is valid MusicXML 4.0 — it passes the official XSD and the semantic checks of §7.3 — or the export raises (e.g. D3).
3. Loading is always lenient and never changes a value silently: every replaced value is a record in the import report (§5).
4. Content the model does not hold is dropped on export and reported: one record per element name, with its count.
5. Reading tolerates real files (DTD 0.6a to 4.0 inputs, UTF-16 and byte-order marks, `.mxl` without `mimetype` or with a non-conforming `container.xml`); writing follows the standard strictly.

## 4. Phases and acceptance

Roadmap order: 4a → 1b → 5 → 4c-1 → 4b → 4c-2 → 4d-1..4d-4 (steps 1b and 5 are other roadmap items; §13).

| Phase | Delivers | Acceptance |
|---|---|---|
| 4a Test infrastructure (no behaviour change) | XSD 4.0 + `lxml` validator, semantic checker, corpus (fixtures, samples, W3C suite), strict ledger, `make fuzz`, optional `make corpus-fetch`, canonical model dump | The ledger records each file's current status; `make tests` is offline and green; the fuzz driver is reproducible from a seed |
| 4c-1 Reader safety | The 16 crash/hang/UB sites become corrections with records; the report API; `ScoreCollection` isolation; the Mozart sample loads | A fixed-seed, fixed-budget `make fuzz` finds no crash, hang or exception other than `RuntimeError` for an unreadable file; every corpus file loads or fails as the ledger says |
| 4b Writer validity | §9 | Every export of every loadable corpus file and of API-built scores passes the XSD and the semantic checks; export → import → export is byte-identical except the encoding date |
| 4c-2 Reader semantics | §10 | Model dumps of the whole corpus before (4b state) and after, every difference mapped to a policy-table row or a listed semantic fix; ledger updated |
| 4d-1..4d-4 Extensions | §11 | Each element is read into the model, exported valid, round-trips stably and leaves the "dropped" records; C++ API, binding, numpydoc and tests |

## 5. Import report (phase 4c-1)

**`ImportIssue`** (C++ struct with a pybind11 class): `code` (stable UPPER_SNAKE string, e.g. `KEY_MISSING_FIRST_MEASURE`, `DIVISIONS_NOT_POSITIVE`, `ELEMENT_NOT_MODELLED`; codes are never renamed), `kind` (`corrected`: a value read was replaced or filled; `dropped`: an element was removed), part index and name, measure number as written and measure index, `element` (path, e.g. `attributes/key/fifths`), `found` (value read; empty when absent), `used` (value stored in the model), `message` (English). Elements the model does not hold produce one `dropped` record per element name with the count in `found`.

**API.** `Score.getImportIssues()` returns the list; `Score.getImportIssuesDataFrame()` returns a DataFrame with one column per field (like `getChordsDataFrame`). The report describes the import: it is copied with the `Score` and does not change after edits or exports; a score built through the API has an empty report.

**Summary line.** Printed once while loading, only when the report is not empty: `[maiacore] <file>: <n> corrections, <m> element types not modelled (dropped on export); see Score.getImportIssues()`. The reader's current per-event `[WARN]` lines (accidental disagreeing with `<alter>`, off-grid alter, unknown accidental name, …) become records; tests that capture those warnings check the report instead.

**Fatal input** still raises `RuntimeError`, now with pugixml's description and offset: not XML, a corrupt `.mxl`, a root that is not a score. `score-timewise` is converted to partwise; `<opus>` is refused with a clear message.

**`ScoreCollection`.** A file that fails to load no longer aborts the collection: it is skipped and listed by `getLoadErrors()` (path, message), with one summary line.

## 6. Policy table and closed element list

**Table** — `docs/musicxml-import-policy.md`, one row per element or attribute:

| Element | Required in 4.0 | Default if absent | Invalid when | Action | Code | Phase | Fixture |
|---|---|---|---|---|---|---|---|

Actions: `default` (use the default), `infer` (derive from context), `carry` (keep the previous value), `convert` (map to the model's form), `drop` (remove), `keep` (keep as written).

**When a record is made.** Only when the file violates MusicXML 4.0 (XSD or a semantic rule) or maialib replaces a value the file states. An absent optional element takes the default the standard defines, without a record (no `<voice>` → voice 1; no `<staves>` → 1 staff; no `<key>` in measure 1 → C major).

**Initial rows (examples; each phase writes its rows):**
- `<divisions>` absent or ≤ 0 → `infer` from the first note with `<type>` and `<duration>`; with none, 1 (the default agreed by the W3C group, test 03e). Recorded.
- `key/fifths` outside −11..11 → `default` 0. Recorded.
- An extra `<clef>` in a measure → stored by `@number` at its position (4c-1: see §8 item 6).
- `pitch/octave` absent → `infer` from the previous note of the voice. Recorded.
- Parts with fewer measures than the longest part → padded with whole-measure rests. Recorded.
- `<cue/>` notes (not played by the part) → a hidden rest of the same duration (`dropped` record), so analyses do not count them and later positions do not move.
- `<forward>` → a hidden rest, written back as `<forward>`.

**Closed element list (what maialib handles), by phase:**
1. 4c-1/4c-2 — held by the model today: work title, creator, parts (matched by `id`), MIDI unpitched; divisions, key, time, staves, staff lines, transpose; clefs (by `@number`), barlines with repeats; on notes: pitch, unpitched, rest, chord, grace, duration, voice, stem, staff, time-modification, instrument, articulations, beam, tie, slur; timing structure: `<backup>`, `<forward>`, measure `number` and `implicit`.
2. 4c-2 — model extensions of D6.
3. Step 1b — `<transpose>` in any measure, with `octave-change` and `double`.
4. 4d-1..4d-4 — the four groups of §11.
5. Everything else — `dropped`, reported by element name with its count.

**Catalogue and meta-test.** Issue codes live in one C++ catalogue (code, kind, element, message template). A meta-test fails when a catalogue code is missing from the table, or when no corpus file produces a code listed in the table.

## 7. Phase 4a — test infrastructure (no behaviour change)

### 7.1 Vendored files (`test/musicxml/`)
- `schema-4.0/`: `musicxml.xsd`, `xml.xsd`, `xlink.xsd`, `catalog.xml`, `container.xsd` from `https://github.com/w3c-cg/musicxml` tag `v4.0` (commit `799e2defb2ece0ae7bafe08dcbcac25b2c631d53`), unmodified; `NOTICE.md` names the specification ("MusicXML 4.0"), its source, the W3C Community Final Specification Agreement, and the origin of `xml.xsd` (`http://www.w3.org/2007/08/xml.xsd`, W3C). A test pins each file's SHA-256 (Appendix A). Do not use the repository's `gh-pages` branch (4.1 draft, different licence).
- `w3c-test-suite/`: the files of `https://github.com/w3c-cg/musicxmlTestSuite` at a pinned commit (HEAD was `77c19f7e` on 2026-10-01; the 4a plan pins the full SHA), 183 files — the fork of Reinhold Kainhofer's LilyPond suite — unmodified, with its MIT `LICENSE` and the README attribution.
- `.gitattributes`: both folders `-text`. The schema files mix CRLF and LF, and the suite's are LF text plus one binary `.mxl`; line-ending conversion would rewrite either folder and break the SHA-256 pins (`core.autocrlf=true` stores CRLF files as LF and checks LF files out as CRLF, so it would rewrite the suite's text files on checkout).

### 7.2 Validator (`musicxml_check.py`)
- XSD validation with `lxml` (dev dependency, pinned in `requirements-dev.txt`): an `etree.Resolver` maps the schema's remote imports (`http://www.musicxml.org/xsd/xml.xsd`, `…/xlink.xsd`, both HTTP 404 today) to the local files; never the network, never `XML_CATALOG_FILES` (libxml2 reads it once per process).
- `.mxl`: open the zip, check `META-INF/container.xml` against `container.xsd` (input files: warning tier) and validate the first rootfile.

### 7.3 Semantic checks (what the XSD cannot express)
- **Errors:** a duration before any `<divisions>`; the musical position negative or past the measure end through `<backup>`/`<forward>`; `<chord/>` without a preceding note; a `<part>` without its `score-part` and dangling IDREFs (`lxml` does not check IDREF integrity); `<staff>` greater than `<staves>`; tie, slur and tuplet start/stop not paired per number; a missing `container.xml` or a first rootfile that is not MusicXML.
- **Warnings:** a measure whose content does not fill the time signature (skipped under senza-misura); `mimetype` missing or not the first, stored entry of an `.mxl`.

### 7.4 Ledger (`test/musicxml/ledger.json`)
One entry per corpus file (the current fixtures under `test/xml_examples/`, the 7 bundled samples, the W3C suite):
- `load`: `ok`, the exception **type** (`IndexError`, `RuntimeError`, … — never the message, which differs between MSVC and libstdc++), `crash` or `timeout`;
- the export's XSD result and semantic errors; `roundtrip`: `stable` when export → import → export is byte-identical except the encoding date;
- the input file's own validity against the 4.0 schema (`valid`, `invalid` or `unreadable`), compared like every other field, so a validator change that alters it needs a ledger update;
- `slow` for the large files (`Beethoven/big_files/Symphony_9th.xml`, 71 MB; `unit_test/xakypueri.xml`, 14 MB);
- from 4c-1 on, the report codes.

Strict in both directions: the test fails when a file gets worse and when it gets better without a ledger update. `make corpus-update-ledger` rewrites the ledger; its diff is reviewed (the same spirit as `make validate-update-baseline`). In 4a the ledger records today's state (the Mozart sample `IndexError`; most exports invalid against 4.0).

### 7.5 Corpus test (`test/test_musicxml_corpus.py`)
Part of `make py-tests` (and so of the MSVC and Linux gates). One subprocess per file, in parallel, with a timeout, so a crash or hang is recorded instead of killing the run. `slow` files run only under `make corpus`. Expected cost: about one minute.

### 7.6 Fuzz driver (`make fuzz`)
- Seeded RNG and a budget (cases or minutes). Seeds: the small corpus files.
- Mutations on the DOM: delete an element; empty a text; non-numeric, negative, zero or huge numbers; duplicate or reorder siblings; invalid enumeration values; re-encode (UTF-16, byte-order mark, a declared latin-1); truncate the file; for `.mxl`: remove `container.xml`, point the rootfile elsewhere, corrupt the zip.
- Pipeline per mutant, in a subprocess with a timeout: load → `getChords` and intervals → export → validate → reload. Outcomes are classified; failing cases are minimised by element removal; `make fuzz-minimize` writes them to `test/musicxml/fuzz-regressions/`, where they become fixtures.
- In 4a the driver only reports; 4c-1 requires a clean fixed budget.

### 7.7 External corpus and model dump
- `make corpus-fetch`: sparse checkout of OpenScore Lieder and OpenScore String Quartets (CC0) at pinned commits into `test/musicxml/external/` (git-ignored); `make corpus` includes them, with their own ledger.
- `dump_score.py`: a canonical JSON dump of a `Score` (parts, measures, attributes, notes). Each phase measures before and after with it; 4c-2's acceptance depends on it.

### 7.8 Small fixes and limits
- The C++ test that leaves `test_roundtrip.xml` in the repository root writes to the temporary directory instead.
- Out of 4a: tests in CI (roadmap 10c); any other C++ change.

## 8. Phase 4c-1 — reader safety

Targeted changes to the current reader; every site becomes a recorded correction:
1. Rhythm figure and dots from the exact fraction of the duration (no loop on truncated values); a duration with no figure takes the nearest one (`DURATION_NOT_REPRESENTABLE`).
2. Divisions 0, negative or invalid → the `infer` policy of §6, before any arithmetic.
3. Measure 1 without `<key>` → C major, no record.
4. Parts matched to `score-part` by `id`; a missing name becomes `"Part N"` (recorded); a `<part>` without its `score-part` is still read (recorded).
5. `<instrument id>` resolved by id; an invalid id is dropped (recorded) instead of undefined behaviour.
6. Clefs by `@number` (default 1). A change inside a measure: the measure keeps its first clef and the last one applies from the next measure (recorded) until 4c-2 models the position. An unknown sign → G clef (recorded) until 4c-2 adds TAB/none/jianpu.
7. `<staves>` 0, negative or huge → the highest `<staff>` used, or 1 (recorded); a note whose `<staff>` exceeds `<staves>` raises the staff count (recorded).
8. Pitch: an invalid or missing `<step>` turns the note into a rest; a missing `<octave>` takes the previous note's of the voice; an empty `<unpitched/>` is B4 (the middle line of a treble staff); each recorded.
9. `<duration>` 0, negative or missing on a non-grace note → derived from `<type>`, dots and `<time-modification>`; without `<type>` the note is dropped (recorded).
10. A `<transpose>` that puts a sounding pitch above B11 or below the floor → ignored for that part (recorded).
11. `fifths` outside −11..11 and `beat-type` outside the powers of two 1..1024 → defaults (recorded) instead of a later `std::out_of_range` in the analyses.
12. Range-checked number parsing instead of unchecked `atoi`; divisions per measure instead of an LCM that can overflow.
13. `.mxl` detected by its `PK` signature, not the extension; a tolerant `container.xml` read with a clear reason on failure; the inner file read with encoding detection (UTF-16 works); a structure check before miniz's comment scan, which reads past the buffer on a corrupt zip.
14. XML errors show pugixml's description and offset.
15. Windows paths with non-ASCII characters opened through the wide-character API.
16. `score-timewise` converted to partwise; `<opus>` refused with a clear message.

Plus the report of §5 and the `ScoreCollection` isolation. The Mozart sample loads.

## 9. Phase 4b — writer validity (MusicXML 4.0)

- **Header:** MusicXML 4.0 partwise DOCTYPE and `version="4.0"`; `<work-title>` only when there is a title; `<identification>` with the model's creators and a truthful `<encoding>` (`<software>maialib X.Y.Z</software>` from `VERSION`, `<encoding-date>` today); no invented rights, encoder, Dolet note, `<supports>` or `<defaults>`.
- **Text:** every text and attribute value escaped (`& < > " '`); characters illegal in XML removed.
- **Part list:** ids `P1…Pn`; no invented abbreviation (4d-1 brings the real one); `score-instrument`/`midi-instrument` only when the model holds the data; `midi-unpitched` within 1..128; unique ids; every `<instrument>` reference resolves.
- **Attributes:** measure 1 always carries `<divisions>`, key, time, `<staves>` (when more than one) and one numbered clef per staff; later measures only what changed since the previous measure. One divisions value per part; every duration an exact positive integer.
- **Notes:** schema order; `<unpitched>` without `<alter>`; `<type>` and `<dot>` from the model (grace notes with their real type, not a fixed "16th"); `<accidental>` and `<time-modification>` after the dots; `<staff>` = the staff the note is stored in; `<notations>` written for articulations even without a tie or slur; no empty `orientation`; an octave outside 0–9 makes the export raise, listing the notes (D3).
- **Voices and staves:** per staff and per voice, that voice's notes followed by `<backup>` to the measure start; gaps and hidden rests become `<forward>`; chord notes share their onset; grace notes have no duration; an empty measure becomes `<rest measure="yes"/>` with the time signature's duration.
- **Barlines:** valid 4.0 values only; no invented final barline.
- **Setters** that take free text reject values outside the 4.0 enumerations with `RuntimeError` (stem, tie type, slur type and orientation, beam value, bar-style, barline location, repeat direction, articulation name), so the model never holds an unexportable value.
- **`toFile`:** D9. `.mxl`: `mimetype` as the first entry, stored uncompressed with no extra field; `META-INF/container.xml` with `<rootfile full-path="score.musicxml" media-type="application/vnd.recordare.musicxml+xml"/>`; the inner name without directories.
- Tests that pin invalid output (`<alter>` in `<unpitched>`; `<time-modification>` before `<dot>`) change, each listed with this justification.

## 10. Phase 4c-2 — reader semantics

- One document-order pass per part over the pugixml DOM, no XPath inside loops: linear time.
- **State carried across measures:** divisions (per measure); key with its mode; time signature with its written form and effective signature; clefs by `@number` **with their position inside the measure** (the model gains clef changes within a measure); number of staves; transposition (read in any measure since step 1b).
- **Timing:** a cursor per measure — a note advances it, a chord note or grace note does not, `<backup>` moves it back, `<forward>` ahead. Model invariant: within each measure, staff and voice, notes are in time order and contiguous from the measure start; a gap, including a voice that enters mid-measure, becomes a hidden rest (`isHidden()`), written back as `<forward>`; a position that goes negative or past the measure end is corrected (recorded); grace notes have duration 0.
- **Notes:** the figure comes from `<type>` and `<dot>` when present; when they disagree with `<duration>`, the duration wins (recorded). Cue notes become hidden rests (§6).
- **Measures:** the number is kept as written; `implicit="yes"` marks an anacrusis (today only `number="0"` is detected); parts matched by `id` and padded (§6).
- **Model extensions (D6):** `Key` gains the mode (`getName()` unchanged for major and minor; e.g. "D dorian" for the others); `ClefSign` gains TAB, NONE and JIANPU; `TimeSignature` gains the written form and the effective signature (composite beats are summed, `3+2/8` → 5/8; under senza-misura the effective signature is the length of the measure's content).
- The policy table is completed, with the meta-test of §6.
- **Acceptance:** model dumps of the whole corpus before (4b state) and after; every difference mapped to a policy-table row or a listed semantic fix (time, divisions and clef carry-forward; timing); the ledger updated; the load time of `xakypueri.xml` recorded before and after.

## 11. Phase 4d — model extensions (four sub-phases)

Each sub-phase has a short design of its own (the API shapes, e.g. how directions and dynamics are modelled), a plan and an implementation. Acceptance for each element: read into the model, exported valid, round trip stable, no longer reported as dropped; C++ API, binding, numpydoc and tests.
- **4d-1 Header and parts:** movement title and number, credits, rights, real part abbreviations, part groups, instrument names, MIDI program and channel.
- **4d-2 Expression and tempo:** dynamics, wedges (crescendo/diminuendo), words, tempo and metronome marks (the model already has fields the reader never fills), fermatas, ornaments, technical indications.
- **4d-3 Structure and form:** repeats with `times`, endings, segno and coda, `measure-style` (multi-measure rests, slashes), clef octave change, invisible notes (`print-object`).
- **4d-4 Note notation:** lyrics, tuplet brackets, numbered slurs and ties, noteheads, cautionary/editorial accidentals, type and dots as written.

## 12. Breaking changes (planned)

Each lands in `CHANGELOG.md` `[Unreleased]` (relative to the last release, v1.10.3) with a migration hint, in the phase that makes it:
- **4c-1:** the reader's `[WARN]` lines are replaced by the report and one summary line; `ScoreCollection` skips files that fail (`getLoadErrors()`); parts are matched by `id`; files that failed to load now load with recorded corrections.
- **4b:** exports are MusicXML 4.0 with a truthful header and no invented `<defaults>`/`<supports>`; `toFile` extension handling (D9); free-text setters reject invalid values; `__hash__` of `Score`, `Part` and `Measure` (a hash of `toXML()`) changes value; octaves outside 0–9 prevent the export; the part abbreviation is no longer invented.
- **4c-2:** carried-forward time signatures, divisions and clefs change positions and durations in analyses of pieces that change them mid-piece; modes change `getName()` and `isMajorMode()` for modal keys; new `ClefSign` values; rhythm figures from `<type>`; measures can contain hidden rests (`isHidden()`) where the file had `<forward>` gaps or cue notes.

## 13. Relationship with roadmap steps 1b and 5

- Step 1b (`<transpose>` reading and writing) runs right after 4a, on the current reader: it reads `<transpose>` in any measure; 4c-2 must preserve that behaviour (the ledger and the model dumps catch a regression). Its `<transpose>` output is checked by 4a's validator from day one.
- Step 5 (melody search, `ScoreCollection` semantics, `Measure` bindings) uses the corpus and ledger of 4a; `ScoreCollection` load isolation belongs to 4c-1 (§5).

## 14. Verification rules and process

- Measure before changing (ledger and model dumps); every new test proven to fail under a targeted mutation; every changed test expectation listed old → new with its justification; Doxygen and numpydoc in the same commit as the behaviour; comments explain the code, never its history; documentation in technical English.
- Gates: clang `make cpp-tests`; `make py-tests` in a brand-new venv (includes the corpus test); `make validate`; `make msvc-gate`; `make linux-gate` when WSL has `cmake`; a fixed-budget `make fuzz` where the acceptance asks for it.
- Process: 4a, 4c-1, 4b and 4c-2 each get an implementation plan written from this design, executed by subagent-driven development with a final review; each 4d sub-phase gets a short design, a plan and the same execution. The first plan is 4a's.

## 15. Out of scope

Running the tests in CI (roadmap 10c); a public validation API (D5); MusicXML 4.1 draft features; layout and formatting elements (`<print>`, `<defaults>` contents, positions) beyond what §11 lists; MIDI-only (`<sound>`/`<listening>`) features beyond tempo; reading MuseScore, Finale or Sibelius native formats.

## Appendix A — external facts (verified 2026-10-01)

| File (tag `v4.0`) | Bytes | SHA-256 |
|---|---|---|
| `musicxml.xsd` | 379,505 | `bfe37ed25a9ec00e6f2591d53df260b84efe12aed209ba3ac0a76f9287665a99` |
| `xml.xsd` | 5,695 | `616a3077df5cfc954ac74a75abe9697b95eef7a85dbe09367d995a483e840eb5` |
| `xlink.xsd` | 2,280 | `6e601f8eeb41618b50e4c7f944dff754e57ea43b602755470dda24c9c2f6df92` |
| `catalog.xml` | 11,596 | `c65df54cbf1c6bd73a335d47c0ec292c4c1d7ecca20dbb6e36388bb169c71245` |
| `container.xsd` | 7,887 | `deddcc2f51e856de21397bbe25e2cf304ca9e3253b0d25dcc6349c390bc22fa6` |

- Licence of the schema files: W3C Community Final Specification Agreement (reproduction and distribution allowed; derivative works must name the specification and version). `xml.xsd`: W3C document. The GPL-3.0 maialib repository holds them as a separate aggregate (GPLv3 §5); MuseScore (GPL-3.0) ships the same files.
- `lxml` 6.1.3 (BSD-3-Clause): wheels for CPython 3.8–3.15 (3.8 on x86/x86-64 only; lxml 7 drops 3.8); validates a 4.8 MB score in ~0.08 s; needs the remote imports redirected; does not check IDREF integrity. `xmlschema` (MIT, pure Python) is ~80× slower and needs Python ≥ 3.10 for its current version — not used.
- W3C/LilyPond suite (`w3c-cg/musicxmlTestSuite`, MIT): 183 files, 1.27 MB; intentionally XSD-invalid files carry a `.invalid` suffix (41g, 74b, 75b); XSD-valid but semantically bad: 41h, 99a–c, 45f/g, 46f/i/j, 03e (no `<divisions>`; agreed default 1).
- OpenScore Lieder (CC0, 1,462 `.mxl`, 18.6 MB; HEAD `38c5db51` on 2026-10-01) and String Quartets (CC0, 196 `.mxl`, 40 MB; HEAD `9be3df2a`): fetched by sparse checkout at pinned commits (the 4a plan pins the full SHAs); their `.mxl` files have no `mimetype` and whitespace inside `<rootfile>` (the reader must tolerate both). The MakeMusic example set has no open licence and stays out; PDMX (1.9 GB) is out of tree.

## Appendix B — crash, hang and undefined-behaviour sites (at `7f6636d`)

1. Infinite dot loop, `helper.cpp:808-811` (divisions not a power of two with small durations; divisions 0).
2. Hardware divide-by-zero, `fraction.h:26-28` (`gcd(0,0)` with divisions 0 and a zero/absent duration).
3. `getMeasure(-1)` (`score.cpp:555-558` → `part.cpp:103`) when measure 1 has no `<key>`.
4. `partsNameVec[p]` out of bounds (`score.cpp:401`) with fewer `part-name`s than parts.
5. `splitString(id, '-')[1]` / `substr` (`score.cpp:699`) on an instrument id without `-`.
6. `getClef(c)` (`score.cpp:593-594` → `measure.cpp:224`) with more clefs than measure 1 had (Mozart).
7. `<staves>` 0 / negative / huge: `std::out_of_range`, `std::length_error`, `std::bad_alloc` (`part.cpp:114-128`, `measure.cpp:79-84`).
8. `<staff>` above the staff count: `RuntimeError` (`measure.cpp:117-119`).
9. Unknown clef sign (TAB, none, jianpu): `RuntimeError` (`clef.cpp:113-115`).
10. Pitch parsing: missing/invalid step, `<unpitched/>`, octave outside −1..11 (`helper.cpp:1343-1379`).
11. Duration 0 / negative / too small (`helper.cpp:798-805`).
12. Transposition above B11 (`note.cpp:62-73`, via `setTransposingInterval`).
13. Deferred `std::out_of_range`: beat-type outside the table (`measure.cpp:195-196`), extreme fifths (`key.cpp:34-35`).
14. Undefined behaviour: `std::lcm` overflow (`score.cpp:361-364`), `atoi` overflow, float-to-int conversion of huge tuplet ratios (`helper.cpp:745-751`).
15. miniz `remove_comment` reads past the buffer on a corrupt or tiny `.mxl` (`zip_file.hpp:6384-6411`).
16. A hang cannot be interrupted from Python, and stalls a whole `ScoreCollection`.
