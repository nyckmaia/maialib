# MusicXML `<transpose>` reading and writing (roadmap step 1b) — Design

**Status:** decisions approved by the user on 2026-10-02 (answers recorded in §2; sections §3–§8
approved one by one); aligned with the code on 2026-10-02 while the implementation plan was written
(the enum's namespace, unpitched notes, a fifth warning code, multi-staff and chord rules of the writer,
the validity rule of the round-trip tests, two out-of-scope limits).
**Roadmap:** step 1b of `C:/Users/nyck/.claude/plans/wobbly-coalescing-quill.md`. It builds on step 1a
(`docs/superpowers/specs/2026-09-30-note-pitch-views-design.md`: written, sounding and internal concert
views of a `Note`) and on step 4a (the MusicXML test infrastructure in `test/musicxml/`). The item-4
umbrella design (`docs/superpowers/specs/2026-10-01-musicxml-robustness-design.md`) assigns `<transpose>`
in any measure, with `octave-change` and `double`, to this step (§6, closed element list, item 3) and
requires phase 4c-2 to preserve it (§13).

## 1. Problem (measured on `main` @ `cd11b63`)

- **The reader reads `<transpose>` in the first measure only.** `Score::loadXMLFile`
  (`score.cpp:448-476`) takes the first `<diatonic>` and the first `<chromatic>` of all `<transpose>`
  elements of the part's first measure (they can come from different elements) with `as_int()`, which
  truncates decimals, and copies that pair onto every note of the part (`score.cpp:792`).
  `octave-change`, `double`, the `number` attribute, a `<transpose>` in a later measure and a change of
  `<transpose>` in the middle of a part are ignored.
- **Octave transpositions are lost.** Contrabasses, contrabassoons, piccolos, celestas and bass
  clarinets sound an octave away from where the file says: the Beethoven 5 sample's contrabasses
  (written G3, sounding G2) are read as sounding G3; so are the contrabasses of the Dvořák sample, five
  parts of the Mahler 8 sample, four parts of W3C 41c, and `test_getchords_poly`'s contrabass.
- **Changes are lost.** W3C 72c changes the clarinet from E♭ to B♭ in measure 2; maialib keeps E♭ for
  the whole part. The slow fixture `xakypueri` changes transposition twice in one part and introduces
  one in a later measure of another.
- **The writer never writes `<transpose>`.** An export loses every transposition, so export → import
  changes what a transposing part sounds: 72a's trumpet sounds C4 before and D4 after. The 4a ledger
  records the round trip as stable because it compares the bytes of two exports, not the model.
- **`getChords` reports a written key next to concert chords.** Its key is the written key of part 0
  (`score.cpp:2936-2941`) while its chords are at concert pitch since step 1a; they disagree whenever
  part 0 transposes (72a, 72c, 72d, `test_pattern.musicxml`).
- **A transposition that pushes a pitch above B11 aborts the whole load** (`setTransposingInterval`
  throws, `note.cpp:928-942`); one below the floor loads, and every sounding getter raises later.
- **An inconsistent diatonic/chromatic pair is followed literally.** The Dvořák sample's trumpets in E
  are written `(3, 4)`, a diminished fourth, so a written F♯4 sounds "B♭4" instead of A♯4; the
  CHANGELOG lists this as a known limitation "until the MusicXML reader checks the pair".
- **Model:** only `Note` holds a transposition, as two ints (`note.h:73-74`); `Measure` and `Part`
  hold none and have no setter. Python's `Measure.getNote()` returns a copy (`py_measure.cpp:108-129`),
  so a note-level edit made through it never reaches the score.

**Corpus** (267 in-repository files, 1,658 external): 21 files have `<transpose>`, 11 of them with
`octave-change`; 2 change it after the first measure (72c, `xakypueri`), 1 introduces it in a later
measure (`xakypueri`). None uses `double`, `number`, a non-integer `chromatic`, `<for-part>` or a
`<transpose>` without `<diatonic>`. The external corpus has no `<transpose>`; 47 of its files declare
`<concert-score/>`. W3C 72b (all transposing instruments) and the Mozart sample cannot be loaded for
reasons unrelated to `<transpose>` (a first measure without `<key>`, two clefs in one measure; item-4
Appendix B 3 and 6, phase 4c-1); W3C 72d is 72b with keys and covers the same transpositions.

## 2. Decisions (user, 2026-10-02)

**D1 — the notes are the single source of truth.** Each pitched note keeps its own transposing
interval, as today. The reader stamps every note with the `<transpose>` in force for its staff; the
writer derives the `<transpose>` elements from the notes; a `Part` setter stamps the notes of a range.
There is no transposition state on `Measure`, `Part` or `Score`, so no copy can disagree with the
notes. Accepted cost: a `<transpose>` is written before the first note that uses it, not at its
position in the original file. (The roadmap's wording, "store it as measure state", is replaced.)

**D2 — `getChords` reports the concert key** (§6.1), taken from the parts that do not transpose; the
rule uses the most frequent key among them (user-approved refinement of "the first such part", because
timpani and trumpets in C are commonly written without a key signature).

**D3 — a non-integer `<chromatic>` is not supported:** that `<transpose>` is ignored, with a warning
(§4.3). No API change.

**D4 — `<double>` is held by the model and counts in the analyses** (§3.1, §6.2): a doubled note adds,
in chord extraction and the piano roll, a note one octave below (or above) what it sounds.

## 3. Model and API

### 3.1 `Note`
- The interval stays two ints, `transposeDiatonic` and `transposeChromatic`, and is the total
  interval: `octave-change` is folded in as `(d + 7·oc, c + 12·oc)`, the convention the existing tests
  already use (a B♭ bass clarinet is `(−8, −14)`, a piccolo `(7, 12)`). Step 1a's sounding and concert
  computations already treat a folded pair exactly as MusicXML's sum.
- New enum `OctaveDoubling { NONE, BELOW, ABOVE }` (C++: global, in `constants.h`, like `ClefSign` and
  `RhythmFigure`; Python `maialib.OctaveDoubling`), with `Note::setOctaveDoubling(OctaveDoubling)` and
  `Note::getOctaveDoubling()`. On a rest the setter changes nothing and logs a warning (`LOG_WARN`), as
  the other pitch setters do; `setPitch("rest")` clears the doubling together with the interval.
- The written, sounding, concert and acoustic getters keep returning the note's own single pitch; the
  doubling is visible through `getOctaveDoubling()` and in the analyses of §6.2.
- `getTransposeDiatonic()` keeps returning the stored value; a stored 0 with a non-zero chromatic
  interval keeps step 1a's inference (D4 of the 1a design) for spelling, and the writer writes the
  inferred value (§5.2).

### 3.2 `Part`
- New `Part::setTransposingInterval(int diatonicInterval, int chromaticInterval, int measureStart = 0,
  int measureEnd = -1, int staff = -1, OctaveDoubling doubling = OctaveDoubling::NONE)`: stamps the
  interval and the doubling on every pitched note of the measures `[measureStart, measureEnd)`
  (`measureEnd = -1`: to the end of the part) and of the staff `staff` (0-based; `-1`: every staff).
  Rests and unpitched notes are left alone.
- **Atomic:** before changing anything it checks every note in the range; if one would have no
  sounding pitch under the interval (outside the representable range, as `Note::setTransposingInterval`
  defines it), it throws (Python `RuntimeError`) naming the first such note
  (measure, staff, written pitch) and changes nothing. The setter also refuses a note that would sound
  below the lowest representable pitch, `C1b-1`, which `Note::setTransposingInterval` accepts (the
  note's sounding getters raise later instead). Invalid measure or staff indices throw as the
  other `Part` accessors do.
- In Python this is the way to change transpositions in place while `Measure.getNote()` returns copies
  (roadmap step 5 changes that); `Score.forEachNote` also edits in place.
- No getter on `Measure`, `Part` or `Score`: the interval is read from the notes.

## 4. Reader

### 4.1 Scope
- `<transpose>` is read in **every measure** of every part. Its values apply to the notes that follow
  it, in that measure and in the following ones, until the next `<transpose>` for the same staff.
- Without a `number` attribute it applies to every staff of the part; with `number="s"` only to staff
  `s` (1-based in MusicXML).
- **Inside a measure, document order decides:** a `<transpose>` applies to the notes written after it
  in the measure. MusicXML defines mid-measure attribute changes in score order (musical time), which
  differs only for a `<transpose>` placed before a `<backup>`; phase 4c-2's reader, which follows
  `<backup>` and `<forward>`, applies score order. No corpus file has that case: `xakypueri`'s
  mid-measure `<transpose>` elements follow the last note of their measure and take effect in the next
  one.
- **A chord is read as a unit:** every note of a chord takes the interval and the doubling in force
  for its staff at the chord's first note, and a `<transpose>` written between the notes of a chord
  applies from the next note that is not in that chord. A chord read from a file therefore never
  mixes tuples, so every file that loads can be exported (§5.1), except one with a chord whose notes
  span staves when another staff's transposition changes at that chord: the writer groups a chord's notes per
  staff (phase 4b rewrites cross-staff chord output). No corpus file writes a `<transpose>` inside a
  chord.
- Each pitched note is stamped with the interval and the doubling in force for its staff (§3.1); rests
  and unpitched notes are not stamped (the setter and the writer leave them alone too, so stamping them
  would not survive a round trip). A part with no `<transpose>` stays untransposed.
- A `number` that is not a positive integer reads as absent (every staff); one beyond the part's staves
  applies to no note.

### 4.2 Values
- `<chromatic>` (required by the schema): the chromatic interval.
- `<diatonic>`: the diatonic interval. **Absent** (allowed by the schema): the conventional diatonic
  interval for the chromatic one, `conventionalDiatonicInterval(c)` (`note.cpp`; the table of the 1a
  design, D4), stored explicitly, with no warning. A present value is checked (§4.3).
- `<octave-change>`: folded into the pair (§3.1); default 0.
- `<double/>`: `OctaveDoubling::BELOW`; `<double above="yes"/>`: `OctaveDoubling::ABOVE`; absent:
  `NONE`.

### 4.3 Corrections
Each correction logs one warning (`LOG_WARN`) per `<transpose>` element, naming the part, the measure
and what was done. Phase 4c-1 turns each into an import-report record (`ImportIssue`, item-4 §5) with
the code given here.

| Case | Action | Code (4c-1) |
|---|---|---|
| `<chromatic>` is not an integer | The `<transpose>` is ignored; the previous interval stays in force | `transpose-chromatic-not-integer` |
| `<octave-change>` is not an integer (schema-invalid) | The same | `transpose-octave-change-not-integer` |
| The diatonic interval disagrees with the chromatic one | The diatonic interval is replaced by `conventionalDiatonicInterval(c)` and the chromatic one kept, so nothing sounds different; for a tritone (`|c| mod 12 == 6`) both the augmented fourth and the diminished fifth agree. Example: the Dvořák trumpets in E, `(3, 4)` → `(2, 4)`. An explicit `<diatonic>0</diatonic>` with a non-zero chromatic interval, and a `<diatonic>` that is not an integer, are disagreements too | `transpose-pair-corrected` |
| A note in the element's scope would have no sounding pitch (outside the representable range) | The `<transpose>` is ignored for its whole scope, checked before anything is stamped; the previous interval stays in force, and a chord with a note that the previous interval cannot sound either is read untransposed as a unit, all its notes (§4.1; the warning gives how many notes). Today the load aborts; this anticipates item-4 §8 item 10 | `transpose-out-of-range` |
| `<for-part>` (MusicXML 4.0 concert score) | Not modelled; dropped. The notes of a concert score are already written at concert pitch | `for-part-not-modelled` |

The pair check runs on the values as written in the file (some files fold octaves into `<diatonic>`
and `<chromatic>`, e.g. the Strauss sample's contrabassoon `(−7, −12)`, which agrees) before
`octave-change` is folded in. `<concert-score/>` has no effect on reading.

## 5. Writer

### 5.1 Where a `<transpose>` is written
- For each part and staff the writer walks the pitched notes in order; the interval in force for the
  staff is the tuple `(diatonic, chromatic, doubling)` of its last pitched note. Rests and unpitched
  notes do not change it. Tuples are compared with the diatonic interval the writer writes (§5.2), so a
  stored 0 and its inferred conventional value count as the same interval.
- **Measure 1:** the tuple of the staff's first pitched note, when it is not `(0, 0, NONE)`. It
  applies from the start of the part even when the first note comes later, so that notation programs
  show the concert key signatures of the opening measures correctly.
- **Later:** wherever a pitched note's tuple differs from that of the previous pitched note of its
  staff — in the `<attributes>` at the start of the measure when that note is the staff's first pitched
  note in the measure, otherwise in an `<attributes>` written just before the note. A change back to an
  untransposed staff is written as `<diatonic>0</diatonic><chromatic>0</chromatic>`.
- **`number`:** one `<transpose>` without `number` when every staff of the part has the same tuple at
  that point; otherwise one `<transpose number="s">` for each staff whose tuple changes. A mid-measure
  change in a part with more than one staff always carries `number`: the staves are written one after
  another, so an element without it would also reach the following staves' notes of that measure.
- A change brought by a chord is written before the chord's first note.
- **Chords:** a chord whose notes have different tuples cannot be written; the export throws, naming
  the measure and the staff (the spirit of item-4 D3). Such a chord arises only through note-level
  edits: the reader reads a chord as a unit (§4.1), also where it ignores a `<transpose>` or reads
  notes untransposed (§4.3). So every file that loads can be exported, except one with a chord whose
  notes span staves when another staff's transposition changes at that chord: the writer groups a chord's notes
  per staff (phase 4b rewrites cross-staff chord output).
- `<attributes>` is opened when a `<transpose>` must be written, also in measures where key, time,
  divisions and clef did not change (`Part::toXML`, `part.cpp:219-229`).

### 5.2 Values and schema position
- The total interval is unfolded: `oc = trunc(c / 12)` (toward zero), then `<diatonic>d − 7·oc</diatonic>`,
  `<chromatic>c − 12·oc</chromatic>`, and `<octave-change>oc</octave-change>` only when `oc ≠ 0`
  (the schema: octave-change "should not be present for intervals of less than an octave"). A B♭ bass
  clarinet `(−8, −14)` is written `−1 / −2 / −1`.
- `<diatonic>` is always written. When the stored diatonic interval is 0 and the chromatic one is not,
  the writer writes the inferred conventional value (§3.1), so the file states the spelling maialib
  uses.
- `<double/>` for `BELOW`, `<double above="yes"/>` for `ABOVE`.
- Position: inside `<attributes>`, after `clef` and `staff-details`, as the MusicXML 4.0 content model
  requires (`musicxml.xsd:2813-2903`; the choice `transpose* | for-part*`, 2868-2880).
- `<for-part>` is never written. The header still declares MusicXML 3.0 until phase 4b; step 4a's
  validator checks every export against the 4.0 schema.
- `Score` and `Part` hash their `toXML()`, so their hashes change for scores with transpositions.

### 5.3 Round trip
Export → import reproduces, for every note, the sounding pitch, its spelling and the doubling. The
stored pair is identical, except that a stored diatonic interval of 0 with a non-zero chromatic
interval comes back as the conventional value the speller already used.

## 6. Analyses

### 6.1 The concert key in `getChords`
- The key reported with each chord (`getChords`, `getChordsDataFrame`'s `key` column) is the concert
  key of the chord's measure: the **most frequent written key** among the pitched parts (`isPitched()`;
  percussion is excluded) whose interval at that measure is key-neutral, `7·c − 12·d == 0` (untransposed,
  or transposed by whole octaves only). A key is compared as fifths and mode. A tie goes to the first
  part in score order. All parts count, also those `getChords`' `partNames` leaves out.
- **When every pitched part transposes:** the first pitched part's written key moved by `7·c − 12·d` fifths, brought by
  multiples of 12 into the range the `Key` class accepts (an enharmonically equal key; e.g. 13 → 1).
  Here and in the key-neutral test, `d` is the diatonic interval the speller uses (the stored one, or
  the inferred conventional one when the stored one is 0 and `c` is not).
- **The interval of a part at a measure:** that of the first pitched note of the measure (staves in
  order); without one, that of the part's last pitched note before the measure; without one, that of
  its first pitched note after it; without any, untransposed.
- The mode comes from the chosen key. The documentation says "the concert key" (today: "the score's
  first part, as that part writes it", `py_score.cpp:498-499`).
- Measured on the Beethoven 5 sample: "C Trumpet" and "Timpani" are untransposed and written without a
  key signature (0 fifths); the other 8 untransposed parts have −3. The most frequent key, −3, is the
  concert key.
- `Chord::getDegree(key)` already uses the concert root, so it now agrees with the reported key.

### 6.2 The octave doubling
- In `getChords`, each doubled note adds a note one octave below (`BELOW`) or above (`ABOVE`) its
  concert pitch, with the same onset and duration; the added note is an untransposed note at that
  concert pitch. A doubled pitch outside the representable range is skipped, with one warning per note
  and call.
- Everything built on `getChords` inherits it: `getChordsDataFrame`, chord qualities, the Sethares
  dissonance functions and the chord plots of `maiapy`.
- `maiapy`'s `plotPianoRoll` also draws the doubled octave of each doubled note.
- The melody search (`findMelodyPattern`, `ScoreCollection`) ignores the doubling: it compares melodic
  lines, and a doubled line is the same melody.

### 6.3 Effects of reading `octave-change`
Octave-transposing parts sound in the right octave, so `getChords` and everything built on it change
for files with such parts: on the Beethoven 5 sample 118 of 1,462 chord names and 721 bass notes change
(Dvořák, Mahler 8, W3C 41c and `test_getchords_poly` change too). This is a deliberate behaviour change
(§8).

## 7. Tests and verification

### 7.1 Fixtures
Small MusicXML files under `test/xml_examples/unit_test/`, one per rule: a change in the middle of a
part, including a change back to untransposed; a `<transpose>` only in a later measure; a per-staff
`number`; `octave-change` (contrabass, piccolo, bass clarinet); `<double/>` and
`<double above="yes"/>`; a non-integer `<chromatic>`; an inconsistent pair; a transposition out of
range (the file loads), including a chord read untransposed as a unit; `<for-part>`; a `<transpose>`
after notes in its measure; a `<transpose>` between the notes of a chord; a `<transpose>` without
`<diatonic>`.

### 7.2 Tests (C++ and Python)
- **Reader:** the interval and doubling of each note for every fixture, and for 72a, 72c, 72d, 41c,
  the Beethoven 5 sample and the Dvořák sample (slow files only under `make corpus`).
- **Writer:** the exported `<transpose>` elements — position, values, `number`, `double`,
  `octave-change`, a mid-measure change, and the error for a chord with mixed tuples.
- **Round trip (§5.3):** for every corpus file with `<transpose>` and every fixture, export → import
  keeps each note's sounding pitch, spelling and doubling. Each such export has no schema or semantic
  error (`test/musicxml/musicxml_check.py`) involving `<transpose>` or its children, and passes the 4.0
  schema entirely where the ledger records that file's export as valid (several samples' exports are
  invalid for unrelated reasons, a phase-4b matter). The slow files' round trip runs under
  `make corpus` only.
- **`Part.setTransposingInterval`:** measure range, staff, doubling, and atomicity on a note that would
  leave the range.
- **Concert key:** 72a, 72c (its change), 72d, `test_pattern.musicxml` and the Beethoven 5 sample;
  percussion excluded; a tie; every part transposing, including the wrap into the `Key` range.
- **Doubling:** in `getChords` and in `plotPianoRoll`.
- **`OctaveDoubling` API:** the refusal on a rest, and `setPitch("rest")` clearing it.
- Every new test is proven to fail under a targeted mutation of the code it protects.

### 7.3 Corpus, ledger and model dump
- `make corpus` is expected to change neither ledger (loads, export validity and round-trip stability
  stay the same); any change is explained in the commit that causes it.
- The model dump of step 4a (`test/musicxml/dump_score.py`) records each note's octave doubling; the
  golden file `test/musicxml/golden/test_staves.dump.json` changes accordingly, deliberately.

### 7.4 Verification
A brand-new venv with `make dev`; `make cpp-tests` twice (the C++ changes); `make py-tests`;
`make validate` (no new findings); `make corpus` (with the external corpus); `make msvc-gate`; the
Python tests on Linux (GCC, WSL); a short `make fuzz` run, since the reader changes.

## 8. Behaviour changes (CHANGELOG `[Unreleased]`, relative to v1.10.3)

- `<transpose>` is read in every measure, per staff, with `octave-change` and `double`:
  octave-transposing parts now sound in the right octave, and changes of transposition are followed —
  sounding pitches, `getChords` and everything built on it change for such files.
- `getChords` reports the concert key.
- An inconsistent diatonic/chromatic pair is corrected (the known limitation about the Dvořák
  trumpets is removed); a `<transpose>` without `<diatonic>` stores the conventional diatonic interval.
- A `<transpose>` that would push a pitch out of range no longer aborts the load.
- Exports write `<transpose>`, so a re-imported score keeps its transpositions; `Score` and `Part`
  hashes change for scores with transpositions.
- New API: `OctaveDoubling`, `Note.setOctaveDoubling` / `Note.getOctaveDoubling`,
  `Part.setTransposingInterval`.
- The stubs, the generated AI documentation and the tutorials are regenerated at release (roadmap
  step 3).

## 9. Out of scope and limits

- Score order for a `<transpose>` before a `<backup>` (phase 4c-2).
- Loading W3C 72b and the Mozart sample (phase 4c-1).
- `<for-part>` and `<concert-score/>` are not modelled (a phase-4d candidate).
- Non-integer transpositions (D3).
- The import report itself (phase 4c-1 turns the warnings of §4.3 into records).
- `Measure::getNumber()` is set only by the reader (to the measure's index), so in a score built through
  the API every measure is 0 and `getChords` reports measure 0's key for each chord (phase 4b, which
  handles measure numbers).
- A score built through the API exports no `<key>` and cannot be reloaded (item-4 Appendix B 3,
  phases 4c-1 and 4b); the round-trip tests of API-built scores set a key.
- Correction to the roadmap: its note that "Beethoven 5 contrabasses and bass clarinets are read an
  octave high" is half wrong — the Beethoven 5 sample has no bass clarinet; bass clarinets with
  `octave-change` are in W3C 41c, the Mahler 8 sample and `xakypueri`.
