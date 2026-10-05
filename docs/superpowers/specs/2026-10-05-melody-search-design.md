# Melody search, ScoreCollection and live note references (roadmap step 5) — Design

**Status:** decisions approved by the user on 2026-10-05 (§2); sections §3–§7 approved one by one.
**Roadmap:** step 5 of `C:/Users/nyck/.claude/plans/wobbly-coalescing-quill.md`. It builds on step 1a
(written, sounding and concert pitch views), step 1b (`<transpose>` read in every measure) and step 4a
(the MusicXML corpus ledger).

## 1. Problem (measured on `main` @ `210e500`)

- **The search sees one line per part.** The melody of a part keeps only notes with `voice == 1` and
  without `<chord/>` (`score.cpp:2165`), walking measure → staff → note. In every packaged sample the
  lower staves use voices 5 and up, so they are never searched; voices 2 and up on the top staff are
  skipped too. When an encoder puts voice 1 on several staves, windows jump between the hands. A chord
  enters by its first-written note (usually the lowest), not the top note the comment claims. Each
  piece of a tied note is a separate event.
- **The last window is never built** (`score.cpp:2176-2177`, and `:2436-2437` in
  `findAnyMelodyPattern`): a pattern equal to the end of a melody, or to the whole melody, is not found.
- **An unnameable transposition aborts the whole search.** `transposeInterval` builds an `Interval` for
  every window before the threshold test; the Chopin sample with pattern `C4 D4 E4` raises "Unable to
  compute the interval [C4, Cx5]", and a quarter tone on a first note raises too.
- **`findAnyMelodyPattern`** is C++ only (its binding is commented out). `findAnyMelodyPattern(0)`
  divides by zero and kills the process; `(-1)` raises "vector too long". Its
  `removeDuplicatePatterns` drops the first of k duplicates and keeps the other k−1, and it compares
  consecutive duration differences, so `[q, q]` and `[h, h]` merge. Its note-event cache
  (`_cachedNoteEventsPerPart`) is invalidated only by copying the score: adding a note after one call
  makes the next call read freed memory.
- **Result names mislead.** `writtenClefKey` is the measure's written key, not a clef; the threshold
  parameters are named differently in the Score list overload and elsewhere; `staveId`, `filename`.
  An empty `ScoreCollection` result raises `KeyError: 'scoreTitle'` or `ValueError`.
- **`ScoreCollection`.** `ml.ScoreCollection()` and a missing directory raise `UnicodeDecodeError`
  (a localised `filesystem_error` message); the C++ default construction is ambiguous;
  `setDirectoriesPaths` replaces the paths but appends the scores (1 → 2 → 3); discovery is not
  recursive and matches extensions case-sensitively (`.XML`, `.MusicXML` are skipped); file order is the
  filesystem's; `removeScore(-1)` segfaults the interpreter.
- **Python edits through `Measure.getNote*` never reach the score:** the non-const overload is
  registered first without a return policy, so `getNote`, `getNoteOn` and `getNoteOff` return copies.
- **`Measure::removeNote(noteId, staff)` erases the range `[0, noteId)`;** an out-of-range index is
  undefined behaviour, and its tests cannot fail. `addNote` with a list inserts the items in reverse
  order at a fixed position, and a position past the end is undefined behaviour.

## 2. Decisions (user, 2026-10-05)

- **D1 — every voice of every staff is a melodic line**; a chord enters by its highest sounding note;
  a window never crosses voices, staves or parts; each result row names its staff and voice.
- **D2 — `Measure.getNote`, `getNoteOn` and `getNoteOff` return live references** in Python, like
  `Score.getPart` and `Part.getMeasure`; the documentation says the reference is valid until the
  measure gains or loses notes.
- **D3 — `ScoreCollection` gains `recursive=False`** in its directory constructor and in
  `setDirectoriesPaths`; extensions match case-insensitively; `setDirectoriesPaths` replaces the scores.
- **D4 — tied notes are one event** in the melody search, with the summed duration.
- **D5 — `findAnyMelodyPattern` is fixed and exposed** in Python as a DataFrame method.
- **D6 — misleading names are corrected now** (breaking): result columns and threshold parameters are
  unified (§4.3).

## 3. Melodic lines and windows

- **Lines.** For each part, each staff and each voice that occurs on that staff, the line is the
  sequence of its events in measure order. Lines are listed by part, staff (0-based), then voice
  (as written, ascending).
- **Events.**
  - A note without `<chord/>` starts an event; the chord notes that follow it join that event, which
    is represented by its highest note by sounding exact position (`Note::getQuarterToneSteps()`).
  - A note tied to the previous event's note (same sounding pitch, tie stop) extends that event: its
    duration is added; the event keeps the first note's written pitch and measure.
  - A rest is an event, as today: its melodic interval is 0 and its duration counts.
  - A grace note is not an event (it has no duration).
- **Windows.** Every window of L consecutive events in one line, the last one included
  (`N − L + 1` windows for N events). No window spans two lines.
- **Pattern length.** A pattern of fewer than 2 notes raises (`RuntimeError`, Python `RuntimeError`)
  with a message naming the method and the length; this includes the empty pattern, which today returns
  an empty table, and `findAnyMelodyPattern(n)` with `n < 2`, which today crashes for 0 and raises an
  unrelated error for negatives. A pattern longer than every line gives an empty result.
- **No cache.** The note-event cache (`NoteEvent`, `_cachedNoteEventsPerPart`,
  `_isNoteEventsPerPartCached`, `collectNoteEventsPerPart`) is removed; each call builds its lines.
  Searches stay safe to run concurrently and after edits. Building the lines costs about as much as
  today's melody walk (Beethoven 5, 13,675 notes: 0.05 s per pattern).

## 4. Comparison and results

### 4.1 Comparison (unchanged)
Melodic intervals are compared at sounding exact positions (quarter tones included) and are
transposition-invariant; durations are normalised by each sequence's longest duration; the similarity
formulas, the thresholds and the five optional callbacks keep their meaning.

### 4.2 The transposition of a match
- `transposeInterval` and `transposeSemitones` are computed only for windows that pass the thresholds.
- `transposeSemitones` (float): the difference in sounding exact position between the window's first
  non-rest event and the pattern's first non-rest note (quarter tones allowed); `NaN` when either is
  all rests.
- `transposeInterval` (str): the name and direction of that interval at concert spelling, as today
  (`"M2 asc"`), with no trailing space for a unison (`"P1"`); empty when the interval has no name (an
  augmented ninth `C4 → Cx5`, any quarter-tone interval) or when either side is all rests. The search
  never raises because of it. The melody search therefore no longer rejects quarter tones (the
  rejection stays in the harmonic analyses: `Interval`, `Chord`).

### 4.3 Result columns (Score and ScoreCollection)

| Column | Type | Meaning |
|---|---|---|
| `partName` | str | the part |
| `measure` | int | 0-based measure index of the window's first event |
| `staff` | int | 0-based staff |
| `voice` | int | voice as written |
| `writtenKey` | str | the part's written key at that measure |
| `concertKey` | str | the score's concert key at that measure (the rule of `getChords`, step 1b) |
| `transposeInterval` | str | §4.2 |
| `transposeSemitones` | float | §4.2 |
| `writtenPitches` | list[str] | the window's written pitches (`"rest"` for rests) |
| `soundingPitches` | list[str] | the window's sounding pitches (`Note::getSoundingPitch()`) |
| `semitonesDiff` | list[float] | per-interval differences (L − 1) |
| `rhythmDiff` | list[float] | per-duration differences (L) |
| `intervalSimilarity`, `rhythmSimilarity`, `totalSimilarity` | float | the similarities |

The list-of-patterns overloads add `patternIdx` (int, first column). `ScoreCollection` adds `fileName`,
`composerName`, `scoreTitle` (first columns). Rows are sorted stably by (`scoreTitle` for a collection,
`patternIdx` for a list), then `measure`, part order, `staff`, `voice`. An empty result is an empty
DataFrame with every column and its dtype. Threshold parameters are named
`intervalSimilarityThreshold` and `rhythmSimilarityThreshold` in every overload (C++ and Python).

### 4.4 `findAnyMelodyPattern`
- Same lines and windows as §3; `patternNumNotes < 2` raises.
- Every distinct window of the score becomes a pattern; two windows are the same pattern when their
  melodic intervals (exact, quarter tones included) and their durations (exact) are equal. Of equal
  windows the first, in line order, is kept.
- Each pattern is searched with the given thresholds (same parameters and callbacks as
  `findMelodyPattern`).
- Python `Score.findAnyMelodyPatternDataFrame(patternNumNotes=5, intervalSimilarityThreshold=0.5,
  rhythmSimilarityThreshold=0.5, …)` returns the §4.3 columns plus `patternIdx` and `patternPitches`
  (the pattern's written pitches), one row per occurrence.

## 5. ScoreCollection
- `ScoreCollection()` builds an empty collection (C++ and Python); constructors taking a directory or a
  list of directories take `recursive = false`.
- A directory that does not exist, or a path that is not a directory, raises `RuntimeError` with a
  message in English naming the path (never a decoding error).
- Discovery: files whose extension is `.xml`, `.mxl` or `.musicxml`, compared case-insensitively;
  subdirectories only when `recursive`; scores are loaded in sorted path order.
- `setDirectoriesPaths(paths, recursive = false)` replaces both the directories and the scores.
  `addDirectory` keeps adding a path without loading; `clear()` keeps its meaning.
- `removeScore(i)` raises `IndexError` (C++ `std::out_of_range`) for any index outside the collection.
- Searches follow §4 and prefix `fileName`, `composerName`, `scoreTitle`.
- Loading isolation (a file that fails to load) stays with phase 4c-1.

## 6. Live note references and Measure edits
- Python `Measure.getNote`, `getNoteOn`, `getNoteOff` return live references (`reference_internal`):
  an edit through them reaches the score. Their numpydoc says the reference is valid until the measure
  gains or loses notes (`addNote`, `removeNote`, …), after which it must be fetched again.
- `Chord.getNote` keeps returning a copy (a live reference would desynchronise the chord's stack
  cache); `Part.getMeasures` keeps returning a copy. Texts that say "`Measure.getNote()` returns a copy"
  are updated (the `Part.setTransposingInterval` numpydoc, the CHANGELOG).
- `Measure::removeNote(noteId, staff = 0)` removes exactly the note at `noteId` of `staff`; an index or
  staff outside the measure raises `std::out_of_range` (Python `IndexError`).
- `Measure::addNote` with a list of notes at a position inserts them in list order; a position beyond
  the end or a negative staff raises `std::out_of_range`.

## 7. Tests, documentation and behaviour changes
- **Fixtures** (small, under `test/xml_examples/unit_test/`, each with its line in the strict 4a
  ledger in the same commit): two staves with several voices; a chord whose highest note is written
  after the lowest; a tie across a barline; a pattern at the last window; duplicate patterns with the
  same and with different rhythms; a transposing instrument (written/sounding columns, concert key); an
  unnameable interval and a quarter tone (the search goes on).
- **Tests** in C++ and Python, each proven to fail under a targeted mutation. Tests that cannot fail
  today (`EXPECT_GE` on a `size_t`, `removeNote(0)` with `<=`) assert exact values. The duplicate count
  of `melody_patterns_quarter_tone_apart.xml` goes from 3 to 4; the quarter-tone rejection tests of the
  melody search change deliberately; live references, every §5 rule and empty results with their
  columns are tested.
- **Docs:** Doxygen and numpydoc of every function touched; the README's Beethoven example is re-run
  and updated if it changes; the two tutorial notebooks that call the unbound `Score.findMelodyPattern`
  call `findMelodyPatternDataFrame`. Stubs, `AI_API_CHEATSHEET.md` and `llms-full.txt` are regenerated
  at release.
- **CHANGELOG `[Unreleased]`**, relative to v1.10.3, lists as breaking: what the search treats as a
  melody (every voice and staff, chord top note, ties merged), the last window, the renamed columns and
  parameters, empty results as empty DataFrames, `transposeInterval` no longer aborting or rejecting
  quarter tones, live `getNote*`, `setDirectoriesPaths` replacing scores, `removeNote` removing one note,
  `addNote`'s list order; and adds `findAnyMelodyPatternDataFrame`, `recursive`, `transposeSemitones`,
  `soundingPitches`, `concertKey`, `voice`.
- **Verification:** a brand-new venv with `make dev`; `make cpp-tests` twice; `make py-tests`;
  `make validate`; `make corpus` (no ledger change beyond the added lines); `make msvc-gate`; the Python
  tests on Linux; a short `make fuzz`.

## 8. Out of scope
- Loading isolation in `ScoreCollection` and non-ASCII paths on Windows (phase 4c-1).
- `Part.addStaves` not updating its measures' staff count (backlog).
- `Chord.removeNote` (backlog).
