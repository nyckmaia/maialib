# MusicXML test infrastructure

| Path | What |
|---|---|
| `schema-4.0/` | The official MusicXML 4.0 XSD, unmodified (see `NOTICE.md`) |
| `w3c-test-suite/` | The W3C MusicXML test suite, unmodified (see `ORIGIN.md`) |
| `musicxml_check.py` | Validation: the 4.0 schema, offline through `lxml`, and the rules the schema cannot express |
| `dump_score.py` | A canonical JSON dump of a maialib `Score` |
| `golden/` | Reviewed tool outputs that the tests compare byte for byte |
| `corpus.py`, `corpus_worker.py` | The corpus, how each file is examined, and the ledger |
| `ledger.json` | The expected result of every in-repository corpus file |
| `ledger-external.json` | The same for the external corpus |
| `external/` | The external corpus, downloaded by `make corpus-fetch`; ignored by git |
| `fuzz.py` | Seeded mutation fuzzing of maialib's MusicXML reader and writer |
| `fuzz-work/` | The scratch files and reports of `make fuzz`; ignored by git |
| `fuzz-regressions/` | Findings saved by `make fuzz-minimize`, minimised where possible, to be turned into fixtures; not ignored by git, so that they can be added |
| `fixtures.py` | Small MusicXML documents for the tests of these tools |

## The ledger

One line per corpus file (a repository-relative path), with these fields:

| Field | Values |
|---|---|
| `input` | `valid`, `invalid` or `unreadable`: the file itself against the 4.0 schema; the type of the exception the validator raised (maialib still examines the file); `crash` or `timeout`. Compared like every other field, so a validator change that alters it needs a ledger update |
| `load` | `ok`, the type of the exception `maialib.Score(path)` raised (`IndexError`, `RuntimeError`, …), `crash`, `timeout`, or `n/a` after a crash or timeout in `input` |
| `export` | `ok`, the exception type `Score.toXML()` raised, `crash`, `timeout` or `n/a` |
| `export_xml` | `well-formed` or `ill-formed`; `crash` or `timeout` while the export is checked (well-formedness, schema and semantic checks are one step); `n/a` without an export |
| `export_xsd` | `valid` or `invalid`; `n/a` without a well-formed export |
| `export_errors` | the error-level semantic checks the export fails (`musicxml_check.py`); empty without a well-formed export |
| `roundtrip` | `stable` when the export, loaded and exported again, is identical apart from its encoding date; `unstable`; an exception type; `crash`; `timeout`; `n/a` |
| `slow` | `true` for files of 10 MB or more: `make py-tests` skips them, `make corpus` runs them |
| `note` | why a field lists alternatives |

A crash or a timeout is charged to the first stage that had not finished, and every later stage is
`n/a`. Exception types are compared, never messages: the text of a C++ exception differs between
compilers. A value may be `{"any_of": [...]}` with a `note` saying why, for an outcome that
depends on the compiler. `make corpus-update-ledger` keeps the alternatives while the new value is
one of them, and their note while any are kept, so alternatives never narrow by themselves: once
their cause is fixed, prune them, and the note, by hand.

maialib and the validator read every file from its own path, whatever characters it holds: the
ledger records what maialib does with the real file, on every platform.

The ledger is strict both ways: a file that gets worse fails, and so does a file that gets better
without a ledger update. `make corpus-update-ledger` writes the current results; review its diff
like code — every changed line must be explained by the change being committed.

## Fuzzing

Case N of seed S mutates one corpus file of at most 200 KB with a random generator seeded with
"S:N", so a seed and a case number give the same mutant in the same checkout of the same corpus: a
corpus file added or removed shifts the list of files the cases pick from, and files checked out
with CRLF line endings (`core.autocrlf`) give other truncated, byte-order-marked and re-encoded
mutants. The mutations delete, duplicate or reorder elements, empty texts, make numbers non-numeric,
negative, zero or huge, put invalid enumeration values, re-encode the document (UTF-16, a byte-order
mark, a declared Latin-1), truncate it, and, in an `.mxl` archive, remove `container.xml`, point the
rootfile elsewhere or corrupt the zip. Each mutant goes through `corpus_worker.py --analyses` in its
own process, and its record gets one outcome:

- `crash:<stage>` or `timeout:<stage>`, the stage the worker had not finished; `crash:exit` or
  `timeout:exit` when it crashed or hung after its final record;
- `<stage>:<exception type>` for an exception in `input` (the validator), `load`, `analyses`,
  `export` or `roundtrip`; a `RuntimeError` from `load` on a file the validator cannot read either
  (`input` is `unreadable`) is `load:RuntimeError:unreadable`;
- `export:ill-formed`, `export:xsd-invalid`, `export:semantic-errors`, `roundtrip:unstable`;
- `ok` when nothing failed.

The first that applies wins, in that order, and within each the earliest stage. The report lists
every case that is not `ok` with its record, the worker's exit code and the end of its stderr.
`make fuzz-minimize` minimises the cases worth it, every outcome but `ok` and the expected
rejections (`load:RuntimeError:unreadable`, `export:xsd-invalid`, `export:semantic-errors`,
`roundtrip:unstable`), by removing elements while the outcome stays the same. A `load:RuntimeError`
on a file the validator reads, valid against the schema or not, is a finding like any other
exception. Re-encoded, truncated and archive mutants are kept as they are, and so is every
`load:RuntimeError`: maialib also refuses a score without parts with a `RuntimeError`, so removing
elements would keep the outcome but lose its cause. For the other outcomes, too, a minimised case
keeps the outcome but not necessarily its cause: another fault of the same class can take over
while elements are removed. Its `.json` names the seed, case, source file and mutation, so the
original mutant can be generated again on the same checkout; check that both fail for the same
reason before turning a case into a fixture.

## Commands

Every command needs maialib installed (`make dev`) except two: `make corpus-fetch` needs git and
network access, and `musicxml_check.py` needs only lxml.

- `make py-tests` runs `test_musicxml_corpus.py`: every corpus file except the slow ones against
  `ledger.json`.
- `make corpus` runs every file, slow ones included, then, when every file matches its ledger,
  the `<transpose>` round trip of `test_musicxml_transpose.py` with `MAIALIB_SLOW_TESTS=1`, which
  adds the slow corpus files that `make py-tests` skips.
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
  test/musicxml/dump_score.py SCORE [OUTPUT]` dumps a score.
