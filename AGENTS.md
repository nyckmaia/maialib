# maialib — Notes for AI assistants

You are working with **maialib**, a Python music-analysis library backed by a C++17
core (`maiacore`) exposed via pybind11. Researchers use it to analyze MusicXML
scores: chord progressions, melodic patterns, rhythm, harmony, dissonance.

**Canonical import:**

```python
import maialib as ml
```

## Quick-start

```python
import maialib as ml

path = ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)
score = ml.Score(path)
chords_df = score.getChordsDataFrame()
print(chords_df.head())
```

## Where to look next

- `AI_API_CHEATSHEET.md` — full Python API on one page (paste into your system prompt).
- `RECIPES.md` *(Phase 2 — may not exist yet)* — task-oriented examples.
- `python-tutorial/` — Jupyter notebooks organized by skill level.
- `README.md` — full human-facing docs.
- `maialib/maiacore/maiacore.pyi` — authoritative type stubs for the C++ API.

## Pitfalls (read before generating code)

1. **Input format is MusicXML/MXL only.** `.xml`, `.mxl`, `.musicxml`. **Never** suggest reading `.mid` / `.midi` — maialib does not parse MIDI.
2. **camelCase, always.** Methods are `getPitchClass`, `getChordsDataFrame`, `findMelodyPattern` — never snake_case. Python-style `get_pitch_class` does not exist.
3. **Score path is a `str`, not `pathlib.Path`.** `ml.Score(str(my_path))`.
4. **Samples ship bundled.** Use `ml.getSampleScorePath(ml.SampleScore.<NAME>)`. Available samples are enumerated on `ml.SampleScore`. Do not download external files in example code.
5. **`Chord` has a large `haveAnyOctave*()` family** (~50 variants). Check `maialib/maiacore/maiacore.pyi` before calling — do not guess names.
6. **Prefer DataFrame methods for analysis.** `score.getChordsDataFrame()`, `score.toDataFrame()`, `score.findMelodyPatternDataFrame()` return pandas DataFrames — idiomatic for data-science workflows.
7. **No async API.** Everything is synchronous.

## AI tooling pipeline (for maintainers)

`make dev` regenerates, in order:

- `.pyi` stubs (`pybind11-stubgen maialib.maiacore` + `stubgen` for maiapy)
- `AI_API_CHEATSHEET.md` (via `scripts/build-cheatsheet.py`)
- `llms-full.txt` (via `scripts/build-llms-full.py`)

Run individually:

```bash
make build-cheatsheet
make build-llms-full
make build-ai-docs   # both, in one go
```

The `.pyi` stubs are the single source of truth for the cheatsheet generator. In Phase 2, C++ Doxygen comments on the headers will sync into pybind11 docstrings, which propagate into the stubs automatically.
