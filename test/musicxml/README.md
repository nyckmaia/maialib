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

A file whose path has a character outside ASCII is loaded by maialib from an ASCII-named temporary
copy, because maialib cannot open such a path on Windows: the ledger records what maialib makes of
the MusicXML content, whatever the file's name. The validator reads the file itself.

The ledger is strict both ways: a file that gets worse fails, and so does a file that gets better
without a ledger update. `make corpus-update-ledger` writes the current results; review its diff
like code — every changed line must be explained by the change being committed.

## Commands

Every command needs maialib installed (`make dev`) except two: `make corpus-fetch` needs git and
network access, and `musicxml_check.py` needs only lxml.

- `make py-tests` runs `test_musicxml_corpus.py`: every corpus file except the slow ones against
  `ledger.json`.
- `make corpus` runs every file, slow ones included.
- `make corpus-update-ledger` writes the ledgers from the current results.
- `make corpus-fetch` downloads OpenScore Lieder and String Quartets (CC0) at pinned commits;
  `make corpus` then includes them. On Windows it fails with "Filename too long" when the path of
  the repository's root is longer than 66 characters: the deepest OpenScore file adds 193 more,
  and git there creates no file whose path is 260 characters or longer.
- `python test/musicxml/musicxml_check.py FILE...` validates files; `python
  test/musicxml/dump_score.py SCORE [OUTPUT]` dumps a score.
