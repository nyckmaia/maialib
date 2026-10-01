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
| `fixtures.py` | Small MusicXML documents for the tests of these tools |

## The ledger

One line per corpus file (a repository-relative path), with these fields:

| Field | Values |
|---|---|
| `input` | `valid`, `invalid` or `unreadable`: the file itself against the 4.0 schema (informational) |
| `load` | `ok`, the type of the exception `maialib.Score(path)` raised (`IndexError`, `RuntimeError`, …), `crash` or `timeout` |
| `export` | `ok`, the exception type `Score.toXML()` raised, `crash`, `timeout` or `n/a` |
| `export_xml`, `export_xsd` | `well-formed`/`ill-formed`, `valid`/`invalid`, or `n/a` |
| `export_errors` | the error-level semantic checks the export fails (`musicxml_check.py`) |
| `roundtrip` | `stable` when the export, loaded and exported again, is identical apart from its encoding date; `unstable`; an exception type; `crash`; `timeout`; `n/a` |
| `slow` | `true` for files of 10 MB or more: `make py-tests` skips them, `make corpus` runs them |

Exception types are compared, never messages: the text of a C++ exception differs between
compilers. A value may be `{"any_of": [...]}` with a `note` saying why, for an outcome that
depends on the compiler.

The ledger is strict both ways: a file that gets worse fails, and so does a file that gets better
without a ledger update. `make corpus-update-ledger` writes the current results; review its diff
like code — every changed line must be explained by the change being committed.

## Commands

All need maialib installed (`make dev`).

- `make py-tests` runs `test_musicxml_corpus.py`: every corpus file except the slow ones against
  `ledger.json`.
- `make corpus` runs every file, slow ones included.
- `make corpus-update-ledger` writes the ledgers from the current results.
- `python test/musicxml/musicxml_check.py FILE...` validates files; `python
  test/musicxml/dump_score.py SCORE [OUTPUT]` dumps a score.
