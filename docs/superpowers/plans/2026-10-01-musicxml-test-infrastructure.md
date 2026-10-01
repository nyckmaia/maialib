# MusicXML Test Infrastructure (roadmap step 4a) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give maialib an offline test infrastructure for MusicXML — the official 4.0 XSD, the W3C test suite, a validator with semantic checks, a strict per-file ledger, an external corpus, a model dump and a fuzz driver — without changing any library behaviour.

**Architecture:** Python tooling under `test/musicxml/` (validator `musicxml_check.py` on `lxml`; `dump_score.py`; `corpus.py` + `corpus_worker.py`, which examine each corpus file with maialib in its own process; `fuzz.py`), vendored upstream files beside it, unittest modules in `test/`, and Makefile targets that call scripts in `scripts/`. The ledger records today's behaviour; later phases (4c-1, 4b, 4c-2) change it deliberately.

**Tech Stack:** Python 3.8+ (`from __future__ import annotations`), `lxml` 6.1.3 (test-only), `unittest`, maialib's Python API, GNU make, the existing `scripts/build_utils.py`.

**Spec:** `docs/superpowers/specs/2026-10-01-musicxml-robustness-design.md` — §7 (phase 4a) and Appendix A are binding; §2 decisions D4 and D5 apply.

## Global Constraints

- **No behaviour change:** nothing under `maiacore/` changes. The only C++ change is the output location of one test (Task 9).
- **Vendored files stay byte-identical** to upstream: the tests pin their SHA-256; `.gitattributes` marks their folders `-text` and is committed **before** the files are added (otherwise `core.autocrlf` rewrites them in the index); `.pre-commit-config.yaml` excludes them.
- Schema files: `https://raw.githubusercontent.com/w3c-cg/musicxml/v4.0/schema/<file>` (tag `v4.0` = commit `799e2defb2ece0ae7bafe08dcbcac25b2c631d53`), SHA-256 as in Appendix A of the spec. Never the `gh-pages` branch. W3C suite: `https://github.com/w3c-cg/musicxmlTestSuite` commit `77c19f7e819154c70ca1a1992e80dcda8ff82fea`. OpenScore: Lieder `38c5db510224d9facdc4b08d741fc788cfb58ea8`, StringQuartets `9be3df2ace482130fe031b9e8a647cdf112ed243`.
- **`lxml` is test-only:** pinned as `lxml==6.1.3` in `requirements-dev.txt`, installed by the MSVC gate, never imported by the `maialib` package. Runtime dependencies do not change (D5).
- **Tests run offline.** Only the vendoring steps (Tasks 1, 2) and `make corpus-fetch` download.
- **The ledger compares exception types, never messages** (C++ exception texts differ between MSVC and libstdc++).
- Python under `test/musicxml/` and `scripts/` is Python 3.8-compatible (`from __future__ import annotations`; `typing.List/Dict/Optional/Tuple/Set/Union/Any`; no `str.removeprefix`, no `dict | dict`), snake_case, and clean under the repository's ruff settings (`ruff format` and `ruff check` with `pyproject.toml`: line length 100, rules E, W, F, I, N, UP, B, C4, SIM).
- Never stage the user's uncommitted `.gitignore` change (`musescore/*`); stage files by name. Ignore rules for new folders go in `test/musicxml/.gitignore`. `git status` ends showing only ` M .gitignore`.
- Commit messages end with:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Docs, comments, docstrings in technical English; comments explain the code, never its development history (no task numbers, rulings or SHAs in code).
- Every new test is proven to fail under a targeted mutation of the code it protects (record the mutation and the failing output in the task report); commit before any mutation experiment.
- Test workflow: `make dev` only inside a brand-new `py -3.12` venv outside the repository with `pip install -r requirements-dev.txt`; `make cpp-tests`, `make py-tests`, `make validate`, `make msvc-gate` exit non-zero on failure (read exit codes directly, never through a pipe). For clang builds set, from PowerShell, `$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'` with `INCLUDE`/`LIB` unset. Python tests run from `test/` (`python -m unittest <module>`) against the installed package.
- Baselines entering 4a: C++ 1106/1106, Python 499 OK, `make validate` no new findings (65 known).

## File map

| File | Responsibility | Task |
|---|---|---|
| `.gitattributes` (new) | `-text` for the vendored folders | 1, 2 |
| `.pre-commit-config.yaml` | exclude the vendored folders from every hook | 1 |
| `test/musicxml/schema-4.0/` (new) | the 5 XSD release files + `NOTICE.md` | 1 |
| `test/musicxml/w3c-test-suite/` (new) | the W3C suite files, `LICENSE`, `README.md`, `ORIGIN.md`, `MANIFEST.sha256` | 2 |
| `test/test_musicxml_vendored.py` (new) | SHA-256 pins of the vendored files | 1, 2 |
| `test/musicxml/fixtures.py` (new) | small MusicXML documents shared by the tool tests | 3 |
| `test/musicxml/musicxml_check.py` (new) | XSD validation offline, `.mxl` reading, semantic checks, CLI | 3, 4 |
| `test/test_musicxml_check.py` (new) | tests of the validator | 3, 4 |
| `requirements-dev.txt`, `scripts/make-msvc-gate.py` | the `lxml` pin; the gate installs it | 3 |
| `test/musicxml/dump_score.py`, `test/test_musicxml_dump.py` (new) | canonical JSON dump of a `Score` | 5 |
| `test/musicxml/corpus_worker.py`, `test/musicxml/corpus.py` (new) | examine one file in a process; the corpus, runner and ledger | 6 |
| `test/musicxml/ledger.json` (new) | expected results of the in-repository corpus | 6 |
| `test/test_musicxml_corpus.py` (new) | the corpus against the ledger, inside `make py-tests` | 6 |
| `scripts/make-corpus.py` (new), `Makefile` | `make corpus`, `make corpus-update-ledger` | 6 |
| `test/musicxml/README.md` (new) | how the infrastructure is laid out and used | 6, 7, 8 |
| `scripts/make-corpus-fetch.py` (new), `test/musicxml/.gitignore` (new), `test/musicxml/ledger-external.json` (new) | external corpus | 7 |
| `test/musicxml/fuzz.py`, `scripts/make-fuzz.py`, `test/test_musicxml_fuzz.py` (new) | seeded mutation fuzzing | 8 |
| `tests-cpp/src/score-test.cpp`, `BUILDING.md`, `CHANGELOG.md` | round-trip test to the temporary directory; docs | 9 |

---

### Task 1: Vendor the MusicXML 4.0 schema

**Files:**
- Create: `.gitattributes`, `test/musicxml/schema-4.0/{musicxml.xsd,xml.xsd,xlink.xsd,catalog.xml,container.xsd,NOTICE.md}`, `test/test_musicxml_vendored.py`
- Modify: `.pre-commit-config.yaml`

**Interfaces:** Produces `test/musicxml/schema-4.0/` (consumed by Task 3's `SCHEMA_DIR`).

- [ ] **Step 1: Write the failing test** — `test/test_musicxml_vendored.py`:

```python
"""The vendored MusicXML files are byte-identical to their upstream releases."""

import hashlib
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCHEMA = ROOT / "test" / "musicxml" / "schema-4.0"
SCHEMA_SHA256 = {
    "musicxml.xsd": "bfe37ed25a9ec00e6f2591d53df260b84efe12aed209ba3ac0a76f9287665a99",
    "xml.xsd": "616a3077df5cfc954ac74a75abe9697b95eef7a85dbe09367d995a483e840eb5",
    "xlink.xsd": "6e601f8eeb41618b50e4c7f944dff754e57ea43b602755470dda24c9c2f6df92",
    "catalog.xml": "c65df54cbf1c6bd73a335d47c0ec292c4c1d7ecca20dbb6e36388bb169c71245",
    "container.xsd": "deddcc2f51e856de21397bbe25e2cf304ca9e3253b0d25dcc6349c390bc22fa6",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git_attributes() -> list:
    return (ROOT / ".gitattributes").read_text(encoding="utf-8").splitlines()


class VendoredSchemaTestCase(unittest.TestCase):
    def test_each_schema_file_is_byte_identical_to_the_release(self):
        for name, digest in SCHEMA_SHA256.items():
            with self.subTest(file=name):
                self.assertEqual(digest, sha256(SCHEMA / name))

    def test_the_folder_holds_the_release_files_and_the_notice_only(self):
        self.assertEqual(set(SCHEMA_SHA256) | {"NOTICE.md"}, {p.name for p in SCHEMA.iterdir()})

    def test_git_keeps_the_schema_bytes(self):
        self.assertIn("test/musicxml/schema-4.0/** -text", git_attributes())


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run it and see it fail** — from `test/`: `python -m unittest test_musicxml_vendored -v` → errors (`FileNotFoundError` for the files and `.gitattributes`).

- [ ] **Step 3: `.gitattributes` and the pre-commit exclusion first.** Create `.gitattributes`:

```
# Vendored files that must stay byte-identical to their upstream releases: the tests pin their
# SHA-256, and line-ending conversion would change them.
test/musicxml/schema-4.0/** -text
```

At the top of `.pre-commit-config.yaml`, before `repos:`:

```yaml
# The vendored MusicXML schema and test suite stay byte-identical to their upstream releases (the
# tests pin their SHA-256), so no hook may rewrite them.
exclude: ^test/musicxml/(schema-4\.0|w3c-test-suite)/
```

Commit these two files alone (`git add .gitattributes .pre-commit-config.yaml`), message `build: keep the vendored MusicXML files byte-identical`.

- [ ] **Step 4: Download the five files** into `test/musicxml/schema-4.0/` with `curl -L --fail -o test/musicxml/schema-4.0/<file> https://raw.githubusercontent.com/w3c-cg/musicxml/v4.0/schema/<file>` for each name in `SCHEMA_SHA256`, then verify each with `sha256sum` against the table (stop and report if any differs — do not adjust the pins).

- [ ] **Step 5: Write `test/musicxml/schema-4.0/NOTICE.md`:**

```markdown
# MusicXML 4.0 schema

These are the unmodified XML Schema files of **MusicXML 4.0**, the Final Community Group Report
of the W3C Music Notation Community Group (1 June 2021, https://www.w3.org/2021/06/musicxml40/).

- Source: https://github.com/w3c-cg/musicxml, tag `v4.0`
  (commit `799e2defb2ece0ae7bafe08dcbcac25b2c631d53`), folder `schema/`.
- `musicxml.xsd`, `xlink.xsd`, `container.xsd` and `catalog.xml`: Copyright © 2004-2021 the
  Contributors to the MusicXML Specification, published by the W3C Music Notation Community Group
  under the W3C Community Final Specification Agreement (FSA),
  https://www.w3.org/community/about/agreements/final/.
- `xml.xsd`: the W3C schema for the XML namespace, identical to
  http://www.w3.org/2007/08/xml.xsd; Copyright © World Wide Web Consortium,
  https://www.w3.org/copyright/document-license-2023/.

maialib uses these files only in its tests, to validate MusicXML documents
(`test/musicxml/musicxml_check.py`); they are not part of the maialib package. The tests pin each
file's SHA-256, so the files must stay byte-identical; `.gitattributes` marks the folder `-text`.
```

- [ ] **Step 6: Run the test** — `python -m unittest test_musicxml_vendored -v` → 3 tests OK.

- [ ] **Step 7: Prove the pins bite:** append one byte to a copy of `musicxml.xsd` in place (`printf ' ' >> …`), run the test (expect a failure naming `musicxml.xsd`), restore with `git checkout`/re-download, rerun green. Record both outputs.

- [ ] **Step 8: Commit** `git add test/musicxml/schema-4.0 test/test_musicxml_vendored.py`, message `test: vendor the official MusicXML 4.0 schema for offline validation`. Afterwards confirm `git ls-files --eol test/musicxml/schema-4.0/` shows `-text` for every file and `git diff HEAD --stat` is empty for them.

### Task 2: Vendor the W3C MusicXML test suite

**Files:**
- Create: `test/musicxml/w3c-test-suite/` (the suite's MusicXML files, `LICENSE`, `README.md`, plus `ORIGIN.md` and `MANIFEST.sha256`)
- Modify: `.gitattributes`, `test/test_musicxml_vendored.py`

**Interfaces:** Produces `test/musicxml/w3c-test-suite/` (consumed by Task 3's suite test and by Task 6's corpus).

- [ ] **Step 1: Failing tests** — append to `test/test_musicxml_vendored.py`, before the `if __name__` block:

```python
SUITE = ROOT / "test" / "musicxml" / "w3c-test-suite"
OWN_FILES = ("MANIFEST.sha256", "ORIGIN.md")


def manifest() -> dict:
    """The pinned SHA-256 of every vendored suite file, by path relative to the suite folder."""
    pins = {}
    for line in (SUITE / "MANIFEST.sha256").read_text(encoding="utf-8").splitlines():
        digest, name = line.split("  ", 1)
        pins[name] = digest
    return pins


class VendoredTestSuiteTestCase(unittest.TestCase):
    def test_every_suite_file_is_byte_identical_to_the_pinned_commit(self):
        for name, digest in manifest().items():
            with self.subTest(file=name):
                self.assertEqual(digest, sha256(SUITE / name))

    def test_the_manifest_lists_every_vendored_file(self):
        files = {
            path.relative_to(SUITE).as_posix()
            for path in SUITE.rglob("*")
            if path.is_file() and path.name not in OWN_FILES
        }
        self.assertEqual(files, set(manifest()))

    def test_git_keeps_the_suite_bytes(self):
        self.assertIn("test/musicxml/w3c-test-suite/** -text", git_attributes())
```

Run: `python -m unittest test_musicxml_vendored -v` → the three new tests error.

- [ ] **Step 2:** append `test/musicxml/w3c-test-suite/** -text` to `.gitattributes`; commit it alone (`build: keep the vendored W3C test suite byte-identical`).

- [ ] **Step 3: Download and copy.** `curl -L --fail -o <scratch>/suite.zip https://codeload.github.com/w3c-cg/musicxmlTestSuite/zip/77c19f7e819154c70ca1a1992e80dcda8ff82fea`; extract with Python's `zipfile` into the scratchpad; inspect the layout. Copy into `test/musicxml/w3c-test-suite/`, keeping their relative paths: every MusicXML test file (`*.musicxml`, `*.mxl`, and any file whose name contains `.invalid`), plus `LICENSE` and `README.md`. Copy nothing else (CI configuration, scripts, images); list what you left out in the report. Expect about 183 MusicXML files.

- [ ] **Step 4: `ORIGIN.md`:**

```markdown
# W3C MusicXML test suite

The MusicXML files here, with `LICENSE` and `README.md`, are the unmodified contents of
https://github.com/w3c-cg/musicxmlTestSuite at commit `77c19f7e819154c70ca1a1992e80dcda8ff82fea`
(retrieved 2026-10-01): the W3C Music Notation Community Group's copy of Reinhold Kainhofer's
LilyPond MusicXML test suite, released under the MIT License (see `LICENSE`).

Files whose name contains `.invalid` are deliberately invalid MusicXML. `MANIFEST.sha256` pins
every file and the tests check it, so the files must stay byte-identical; `.gitattributes` marks
the folder `-text`. maialib uses them only in its tests; they are not part of the maialib package.
```

- [ ] **Step 5: Generate `MANIFEST.sha256`** (run from the repository root):

```bash
python - <<'EOF'
import hashlib
from pathlib import Path

suite = Path("test/musicxml/w3c-test-suite")
own = ("MANIFEST.sha256", "ORIGIN.md")
lines = [
    f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.relative_to(suite).as_posix()}"
    for path in sorted(suite.rglob("*"))
    if path.is_file() and path.name not in own
]
(suite / "MANIFEST.sha256").write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
print(len(lines), "files pinned")
EOF
```

- [ ] **Step 6:** `python -m unittest test_musicxml_vendored -v` → 6 tests OK. Prove the pin bites (append a byte to one suite file → failure naming it → restore → green); record the outputs.

- [ ] **Step 7: Commit** `git add test/musicxml/w3c-test-suite test/test_musicxml_vendored.py`, message `test: vendor the W3C MusicXML test suite`. Confirm `git ls-files --eol test/musicxml/w3c-test-suite/` shows `-text` everywhere.

### Task 3: Validate MusicXML against the 4.0 schema offline, `.mxl` included

**Files:**
- Create: `test/musicxml/fixtures.py`, `test/musicxml/musicxml_check.py`, `test/test_musicxml_check.py`
- Modify: `requirements-dev.txt`, `scripts/make-msvc-gate.py`

**Interfaces:**
- Consumes: `test/musicxml/schema-4.0/` (Task 1), `test/musicxml/w3c-test-suite/` (Task 2).
- Produces (module `musicxml_check`, importable after `sys.path.insert(0, "<repo>/test/musicxml")`):
  - `Finding(check: str, severity: str, where: str, detail: str)` — frozen dataclass; `severity` is `"error"` or `"warning"`.
  - `Report(problem: Optional[str], xsd_errors: List[str], findings: List[Finding])` with properties `readable: bool`, `xsd_valid: bool`, `errors: List[str]` (sorted unique error check names), `warnings: List[str]`.
  - `parse_document(data: bytes) -> etree._Element` (raises `etree.XMLSyntaxError`).
  - `read_mxl(data: bytes) -> Tuple[Optional[bytes], List[Finding]]`.
  - `check_bytes(data: bytes) -> Report`, `check_mxl_bytes(data: bytes) -> Report`, `check_file(path: Union[str, Path]) -> Report`.
  - Constants `SCHEMA_DIR`, `MIMETYPE`, `MUSICXML_MEDIA_TYPE`.
- Produces (module `fixtures`): `MINIMAL_SCORE: bytes`, `CONTAINER: bytes`.

- [ ] **Step 1: Dependency.** Add to `requirements-dev.txt`, after the runtime block, a new block:

```
# Test-only: the MusicXML tests validate documents against the official 4.0 schema with lxml.
lxml==6.1.3
```

Install it into your venv (`pip install lxml==6.1.3`).

- [ ] **Step 2: Fixtures** — `test/musicxml/fixtures.py`:

```python
"""Small MusicXML documents shared by the tests of the MusicXML tools."""

# The MusicXML "Hello World": one whole-note C4 in 4/4, valid against the 4.0 schema. Each piece
# the tests replace appears exactly once.
MINIMAL_SCORE = (
    b'<?xml version="1.0" encoding="UTF-8"?>\n'
    b'<score-partwise version="4.0">'
    b'<part-list><score-part id="P1"><part-name>Music</part-name></score-part></part-list>'
    b'<part id="P1"><measure number="1">'
    b"<attributes><divisions>1</divisions><key><fifths>0</fifths></key>"
    b"<time><beats>4</beats><beat-type>4</beat-type></time>"
    b"<clef><sign>G</sign><line>2</line></clef></attributes>"
    b"<note><pitch><step>C</step><octave>4</octave></pitch>"
    b"<duration>4</duration><type>whole</type></note>"
    b"</measure></part>"
    b"</score-partwise>\n"
)

# The META-INF/container.xml of an .mxl archive whose score is "score.musicxml".
CONTAINER = (
    b'<?xml version="1.0" encoding="UTF-8"?>\n'
    b"<container><rootfiles>"
    b'<rootfile full-path="score.musicxml" media-type="application/vnd.recordare.musicxml+xml"/>'
    b"</rootfiles></container>\n"
)
```

- [ ] **Step 3: Failing tests** — `test/test_musicxml_check.py`:

```python
"""The MusicXML validator: the 4.0 schema offline, .mxl archives, and the semantic rules."""

import io
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import musicxml_check  # noqa: E402
from fixtures import CONTAINER, MINIMAL_SCORE  # noqa: E402

SUITE = Path(__file__).resolve().parent / "musicxml" / "w3c-test-suite"


def archive(score=MINIMAL_SCORE, container=CONTAINER, mimetype="stored") -> bytes:
    """An .mxl archive; ``mimetype`` is "stored" (first, uncompressed), "deflated" or None."""
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as zipped:
        if mimetype == "stored":
            zipped.writestr(
                zipfile.ZipInfo("mimetype"), musicxml_check.MIMETYPE, compress_type=zipfile.ZIP_STORED
            )
        elif mimetype == "deflated":
            zipped.writestr("mimetype", musicxml_check.MIMETYPE)
        if container is not None:
            zipped.writestr("META-INF/container.xml", container)
        zipped.writestr("score.musicxml", score)
    return buffer.getvalue()


class SchemaValidationTestCase(unittest.TestCase):
    def test_the_minimal_score_is_valid(self):
        report = musicxml_check.check_bytes(MINIMAL_SCORE)
        self.assertTrue(report.readable, report.problem)
        self.assertEqual([], report.xsd_errors)

    def test_an_element_out_of_schema_order_is_invalid(self):
        swapped = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>", b"<type>whole</type><duration>4</duration>"
        )
        report = musicxml_check.check_bytes(swapped)
        self.assertTrue(report.readable)
        self.assertFalse(report.xsd_valid)

    def test_xml_that_is_not_well_formed_is_unreadable(self):
        self.assertFalse(musicxml_check.check_bytes(b"<score-partwise><part>").readable)

    def test_the_imported_xml_namespace_resolves_offline(self):
        # xml:lang is declared in xml.xsd, which musicxml.xsd imports from a URL that no longer
        # resolves; the schema compiles and this validates only through the local copies.
        lyric = (
            b"<type>whole</type>"
            b'<lyric><syllabic>single</syllabic><text xml:lang="la">Ky</text></lyric>'
        )
        report = musicxml_check.check_bytes(MINIMAL_SCORE.replace(b"<type>whole</type>", lyric))
        self.assertTrue(report.xsd_valid, report.xsd_errors)

    def test_every_w3c_suite_file_is_valid_unless_named_invalid(self):
        files = [
            path
            for path in sorted(SUITE.rglob("*"))
            if path.is_file() and (".musicxml" in path.name or path.suffix == ".mxl")
        ]
        self.assertGreaterEqual(len(files), 180)
        wrong = [
            path.name
            for path in files
            if musicxml_check.check_file(path).xsd_valid == (".invalid" in path.name)
        ]
        self.assertEqual([], wrong)


class ArchiveTestCase(unittest.TestCase):
    def test_a_conforming_archive_is_valid_without_findings(self):
        report = musicxml_check.check_mxl_bytes(archive())
        self.assertTrue(report.xsd_valid, report.problem)
        self.assertEqual([], report.warnings)
        self.assertEqual([], report.errors)

    def test_an_archive_without_mimetype_is_read_with_a_warning(self):
        report = musicxml_check.check_mxl_bytes(archive(mimetype=None))
        self.assertTrue(report.readable)
        self.assertEqual(["mimetype-not-first-stored"], report.warnings)

    def test_a_compressed_mimetype_is_a_warning(self):
        report = musicxml_check.check_mxl_bytes(archive(mimetype="deflated"))
        self.assertEqual(["mimetype-not-first-stored"], report.warnings)

    def test_an_archive_without_container_is_unreadable(self):
        report = musicxml_check.check_mxl_bytes(archive(container=None))
        self.assertFalse(report.readable)
        self.assertEqual(["container-missing"], report.errors)

    def test_a_rootfile_naming_a_missing_file_is_unreadable(self):
        container = CONTAINER.replace(b"score.musicxml", b"elsewhere.musicxml")
        report = musicxml_check.check_mxl_bytes(archive(container=container))
        self.assertFalse(report.readable)
        self.assertEqual(["rootfile-missing"], report.errors)

    def test_a_first_rootfile_of_another_media_type_is_unreadable(self):
        container = CONTAINER.replace(b"application/vnd.recordare.musicxml+xml", b"application/pdf")
        report = musicxml_check.check_mxl_bytes(archive(container=container))
        self.assertFalse(report.readable)
        self.assertEqual(["rootfile-not-musicxml"], report.errors)

    def test_whitespace_inside_the_rootfile_is_read_with_a_warning(self):
        # The layout of the OpenScore and MuseScore archives: valid MusicXML, but the container
        # breaks container.xsd, which wants <rootfile> empty.
        container = CONTAINER.replace(b'+xml"/>', b'+xml">\n</rootfile>')
        report = musicxml_check.check_mxl_bytes(archive(container=container))
        self.assertTrue(report.readable)
        self.assertIn("container-not-schema-valid", report.warnings)

    def test_an_archive_is_recognised_by_its_signature_not_its_name(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "score.xml"
            path.write_bytes(archive())
            self.assertTrue(musicxml_check.check_file(path).xsd_valid)

    def test_bytes_that_are_not_a_zip_archive_are_unreadable(self):
        self.assertFalse(musicxml_check.check_mxl_bytes(b"PK\x03\x04 not an archive").readable)


if __name__ == "__main__":
    unittest.main()
```

Run `python -m unittest test_musicxml_check -v` → `ModuleNotFoundError: musicxml_check`.

- [ ] **Step 4: Implement** `test/musicxml/musicxml_check.py`:

```python
"""Validate MusicXML 4.0 documents against the official XSD, offline.

The schema files in schema-4.0/ are the unmodified MusicXML 4.0 release. Their imports of xml.xsd
and xlink.xsd name http://www.musicxml.org/xsd/ URLs that no longer resolve, so a resolver maps
them to the local copies: validation never uses the network.

Command line: ``python musicxml_check.py FILE...`` prints one line per file and exits with 1 when
a file cannot be read or is invalid against the schema.
"""

from __future__ import annotations

import io
import sys
import zipfile
import zlib
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

from lxml import etree

SCHEMA_DIR = Path(__file__).resolve().parent / "schema-4.0"
REMOTE_SCHEMA_PREFIX = "http://www.musicxml.org/xsd/"
MIMETYPE = b"application/vnd.recordare.musicxml"
MUSICXML_MEDIA_TYPE = "application/vnd.recordare.musicxml+xml"


@dataclass(frozen=True)
class Finding:
    """A broken rule: its stable check name, "error" or "warning", where it is, and what it is."""

    check: str
    severity: str
    where: str
    detail: str


@dataclass
class Report:
    """The result of checking one document.

    ``problem`` says why no MusicXML document could be read (an XML syntax error, a corrupt
    archive, a missing rootfile); it is None when the document was read and checked.
    """

    problem: Optional[str] = None
    xsd_errors: List[str] = field(default_factory=list)
    findings: List[Finding] = field(default_factory=list)

    @property
    def readable(self) -> bool:
        return self.problem is None

    @property
    def xsd_valid(self) -> bool:
        return self.readable and not self.xsd_errors

    @property
    def errors(self) -> List[str]:
        """The names of the failed error-level checks, sorted, without repeats."""
        return sorted({finding.check for finding in self.findings if finding.severity == "error"})

    @property
    def warnings(self) -> List[str]:
        """The names of the failed warning-level checks, sorted, without repeats."""
        return sorted({finding.check for finding in self.findings if finding.severity == "warning"})


class LocalSchemaResolver(etree.Resolver):
    """Resolve the schema's http://www.musicxml.org/xsd/ imports to the files in SCHEMA_DIR."""

    def resolve(self, system_url, public_id, context):
        if system_url and system_url.startswith(REMOTE_SCHEMA_PREFIX):
            local = SCHEMA_DIR / system_url[len(REMOTE_SCHEMA_PREFIX) :]
            if local.is_file():
                return self.resolve_filename(str(local), context)
        return None


_compiled: Dict[str, etree.XMLSchema] = {}


def _schema(name: str) -> etree.XMLSchema:
    if name not in _compiled:
        parser = etree.XMLParser(no_network=True)
        parser.resolvers.add(LocalSchemaResolver())
        _compiled[name] = etree.XMLSchema(etree.parse(str(SCHEMA_DIR / name), parser))
    return _compiled[name]


def musicxml_schema() -> etree.XMLSchema:
    """The compiled MusicXML 4.0 schema."""
    return _schema("musicxml.xsd")


def container_schema() -> etree.XMLSchema:
    """The compiled schema of an .mxl archive's META-INF/container.xml."""
    return _schema("container.xsd")


def parse_document(data: bytes) -> etree._Element:
    """Parse XML without loading a DTD or using the network; raises etree.XMLSyntaxError."""
    # huge_tree lifts libxml2's limits on text size and nesting, which large scores exceed.
    parser = etree.XMLParser(no_network=True, load_dtd=False, resolve_entities=False, huge_tree=True)
    return etree.fromstring(data, parser)


def _xsd_errors(schema: etree.XMLSchema, root: etree._Element) -> List[str]:
    if schema.validate(root):
        return []
    return [f"line {entry.line}: {entry.message}" for entry in schema.error_log]


def read_mxl(data: bytes) -> Tuple[Optional[bytes], List[Finding]]:
    """Return the MusicXML document inside an .mxl archive and what is wrong with its layout.

    The document is None when the archive names no readable MusicXML rootfile. Raises
    zipfile.BadZipFile (or zlib.error) when the bytes are not a readable zip archive.
    """
    findings: List[Finding] = []
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        entries = archive.infolist()
        first = entries[0] if entries else None
        if (
            first is None
            or first.filename != "mimetype"
            or first.compress_type != zipfile.ZIP_STORED
            or first.extra
            or archive.read("mimetype") != MIMETYPE
        ):
            findings.append(
                Finding(
                    "mimetype-not-first-stored",
                    "warning",
                    "archive",
                    "the first entry should be 'mimetype', stored uncompressed, holding "
                    + MIMETYPE.decode("ascii"),
                )
            )
        names = {entry.filename for entry in entries}
        if "META-INF/container.xml" not in names:
            findings.append(
                Finding("container-missing", "error", "archive", "no META-INF/container.xml")
            )
            return None, findings
        try:
            container = parse_document(archive.read("META-INF/container.xml"))
        except etree.XMLSyntaxError as error:
            findings.append(
                Finding("container-missing", "error", "archive", f"META-INF/container.xml: {error}")
            )
            return None, findings
        if not container_schema().validate(container):
            details = "; ".join(entry.message for entry in container_schema().error_log)
            findings.append(Finding("container-not-schema-valid", "warning", "archive", details))
        rootfile = container.find("rootfiles/rootfile")
        path = rootfile.get("full-path") if rootfile is not None else None
        if path is None or path not in names:
            findings.append(
                Finding(
                    "rootfile-missing",
                    "error",
                    "archive",
                    f"the first <rootfile> names {path!r}, which the archive does not hold",
                )
            )
            return None, findings
        media_type = rootfile.get("media-type")
        if media_type is not None and media_type != MUSICXML_MEDIA_TYPE:
            findings.append(
                Finding(
                    "rootfile-not-musicxml",
                    "error",
                    "archive",
                    f"the first <rootfile> has media-type {media_type}",
                )
            )
            return None, findings
        return archive.read(path), findings


def check_bytes(data: bytes) -> Report:
    """Check an uncompressed MusicXML document."""
    try:
        root = parse_document(data)
    except etree.XMLSyntaxError as error:
        return Report(problem=f"not well-formed XML: {error}")
    return Report(xsd_errors=_xsd_errors(musicxml_schema(), root))


def check_mxl_bytes(data: bytes) -> Report:
    """Check an .mxl archive: its layout, then the MusicXML document it names."""
    try:
        document, findings = read_mxl(data)
    except (zipfile.BadZipFile, zlib.error, EOFError, KeyError, NotImplementedError, OSError,
            RuntimeError) as error:
        return Report(problem=f"not a readable zip archive: {error}")
    if document is None:
        problems = "; ".join(finding.detail for finding in findings if finding.severity == "error")
        return Report(problem=problems, findings=findings)
    report = check_bytes(document)
    report.findings = findings + report.findings
    return report


def check_file(path: Union[str, Path]) -> Report:
    """Check a .xml, .musicxml or .mxl file; an archive is recognised by its PK signature."""
    data = Path(path).read_bytes()
    return check_mxl_bytes(data) if data[:2] == b"PK" else check_bytes(data)


def main(argv: List[str]) -> int:
    failed = False
    for name in argv:
        report = check_file(name)
        if not report.readable:
            print(f"{name}: unreadable: {report.problem}")
            failed = True
            continue
        state = "valid" if report.xsd_valid else f"invalid ({len(report.xsd_errors)} XSD errors)"
        errors = ", ".join(report.errors) or "none"
        warnings = ", ".join(report.warnings) or "none"
        print(f"{name}: {state}; errors: {errors}; warnings: {warnings}")
        for error in report.xsd_errors[:5]:
            print(f"    {error}")
        failed = failed or not report.xsd_valid or bool(report.errors)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
```

If `test_a_conforming_archive_is_valid_without_findings` fails because `container.xsd` requires something `CONTAINER` lacks, change `CONTAINER` to what the schema requires (not the checker) and say so in the report.

- [ ] **Step 5:** `python -m unittest test_musicxml_check -v` → all pass. `python test/musicxml/musicxml_check.py maialib/xml-scores-examples/*` from the repository root prints one line per sample (all `valid` per the spec's research); paste the output in the report.

- [ ] **Step 6: MSVC gate installs the test dependency.** In `scripts/make-msvc-gate.py` add a module-level helper:

```python
def pinned_requirement(name: str) -> str:
    """Return the requirement line pinning ``name`` in requirements-dev.txt."""
    for line in (REPO_ROOT / "requirements-dev.txt").read_text(encoding="utf-8").splitlines():
        if line.split("==")[0].strip().lower() == name:
            return line.strip()
    check_failed(f"requirements-dev.txt does not pin {name}")
```

and in `python_build_and_test`, immediately before the `run_step([str(python), "-m", "unittest"], ...)` line:

```python
    # The MusicXML tests validate documents with lxml, a test-only dependency pinned in
    # requirements-dev.txt; `pip install .` does not install it.
    run_step(
        [str(python), "-m", "pip", "install", pinned_requirement("lxml")],
        "pip install lxml (test dependency)",
    )
```

(`check_failed` already exists in that script and does not return; if its name differs, use the script's existing failure helper.)

- [ ] **Step 7: Mutations** (record each: the change, the failing tests, the restore): (a) `LocalSchemaResolver.resolve` returns `None` always → `test_the_imported_xml_namespace_resolves_offline` and the schema tests fail (the schema no longer compiles offline); (b) drop the `first.compress_type` condition → `test_a_compressed_mimetype_is_a_warning` fails; (c) drop the media-type check → `test_a_first_rootfile_of_another_media_type_is_unreadable` fails; (d) make `check_file` ignore the PK signature → the signature test fails.

- [ ] **Step 8: Commit** `git add requirements-dev.txt scripts/make-msvc-gate.py test/musicxml/fixtures.py test/musicxml/musicxml_check.py test/test_musicxml_check.py`, message `test: validate MusicXML against the official 4.0 schema offline, .mxl archives included`.

### Task 4: Check the MusicXML rules the schema cannot express

**Files:**
- Modify: `test/musicxml/musicxml_check.py`, `test/test_musicxml_check.py`

**Interfaces:**
- Consumes: Task 3's module.
- Produces: `semantic_findings(root: etree._Element) -> List[Finding]`; `check_bytes` now fills `Report.findings` with it. Check names (stable, used by the ledger): errors `duration-before-divisions`, `chord-without-anchor`, `staff-above-staves`, `position-negative`, `position-past-measure-end`, `unpaired-tie`, `unpaired-slur`, `unpaired-tuplet`, `part-without-score-part`, `duplicate-part`, `score-part-without-part`, `dangling-instrument-ref` (plus Task 3's `container-missing`, `rootfile-missing`, `rootfile-not-musicxml`); warnings `measure-length-mismatch`, `timewise-not-checked` (plus `mimetype-not-first-stored`, `container-not-schema-valid`).

- [ ] **Step 1: Failing tests** — append to `test/test_musicxml_check.py` (before `if __name__`):

```python
WHOLE_NOTE = (
    b"<note><pitch><step>C</step><octave>4</octave></pitch>"
    b"<duration>4</duration><type>whole</type></note>"
)


def note(duration: int, kind: bytes, after_duration: bytes = b"", notations: bytes = b"") -> bytes:
    """A C4 note; ``after_duration`` holds <tie> elements, ``notations`` <notations> content."""
    return (
        b"<note><pitch><step>C</step><octave>4</octave></pitch><duration>"
        + str(duration).encode()
        + b"</duration>"
        + after_duration
        + b"<type>"
        + kind
        + b"</type>"
        + (b"<notations>" + notations + b"</notations>" if notations else b"")
        + b"</note>"
    )


def with_notes(*notes: bytes) -> bytes:
    return MINIMAL_SCORE.replace(WHOLE_NOTE, b"".join(notes))


def errors(data: bytes) -> list:
    return musicxml_check.check_bytes(data).errors


def warnings(data: bytes) -> list:
    return musicxml_check.check_bytes(data).warnings


class SemanticChecksTestCase(unittest.TestCase):
    def test_the_minimal_score_breaks_no_rule(self):
        self.assertEqual([], errors(MINIMAL_SCORE))
        self.assertEqual([], warnings(MINIMAL_SCORE))

    def test_a_duration_before_any_divisions_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"<divisions>1</divisions>", b"")
        self.assertIn("duration-before-divisions", errors(data))

    def test_a_chord_note_without_a_preceding_note_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"<note><pitch>", b"<note><chord/><pitch>")
        self.assertIn("chord-without-anchor", errors(data))

    def test_a_staff_above_the_number_of_staves_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"<type>whole</type>", b"<type>whole</type><staff>2</staff>")
        self.assertIn("staff-above-staves", errors(data))

    def test_a_backup_before_the_measure_start_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"</note>", b"</note><backup><duration>8</duration></backup>")
        self.assertIn("position-negative", errors(data))

    def test_a_forward_past_the_measure_end_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"</note>", b"</note><forward><duration>4</duration></forward>")
        self.assertIn("position-past-measure-end", errors(data))

    def test_a_forward_to_the_measure_end_is_allowed(self):
        voice_two = b"</note><backup><duration>4</duration></backup><forward><duration>4</duration></forward>"
        self.assertEqual([], errors(MINIMAL_SCORE.replace(b"</note>", voice_two)))

    def test_a_tie_start_without_a_stop_is_an_error(self):
        data = with_notes(note(4, b"whole", b'<tie type="start"/>'))
        self.assertIn("unpaired-tie", errors(data))

    def test_a_tie_stop_without_a_start_is_an_error(self):
        data = with_notes(note(4, b"whole", b'<tie type="stop"/>'))
        self.assertIn("unpaired-tie", errors(data))

    def test_tied_notes_pair_including_a_note_that_ends_one_tie_and_starts_the_next(self):
        data = with_notes(
            note(2, b"half", b'<tie type="start"/>'),
            note(1, b"quarter", b'<tie type="stop"/><tie type="start"/>'),
            note(1, b"quarter", b'<tie type="stop"/>'),
        )
        self.assertEqual([], errors(data))

    def test_a_slur_start_without_a_stop_is_an_error(self):
        data = with_notes(note(4, b"whole", notations=b'<slur type="start" number="1"/>'))
        self.assertIn("unpaired-slur", errors(data))

    def test_a_slur_start_and_stop_pair(self):
        data = with_notes(
            note(2, b"half", notations=b'<slur type="start" number="1"/>'),
            note(2, b"half", notations=b'<slur type="stop" number="1"/>'),
        )
        self.assertEqual([], errors(data))

    def test_a_tuplet_start_without_a_stop_is_an_error(self):
        data = with_notes(note(4, b"whole", notations=b'<tuplet type="start"/>'))
        self.assertIn("unpaired-tuplet", errors(data))

    def test_a_part_without_its_score_part_is_an_error(self):
        found = errors(MINIMAL_SCORE.replace(b'<part id="P1">', b'<part id="P2">'))
        self.assertIn("part-without-score-part", found)
        self.assertIn("score-part-without-part", found)

    def test_a_second_part_for_one_score_part_is_an_error(self):
        start = MINIMAL_SCORE.index(b'<part id="P1">')
        end = MINIMAL_SCORE.index(b"</part>") + len(b"</part>")
        part = MINIMAL_SCORE[start:end]
        self.assertIn("duplicate-part", errors(MINIMAL_SCORE.replace(part, part + part)))

    def test_an_instrument_reference_to_no_score_instrument_is_an_error(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration>", b'<duration>4</duration><instrument id="P1-I9"/>'
        )
        self.assertIn("dangling-instrument-ref", errors(data))

    def test_a_measure_shorter_than_its_time_signature_is_a_warning_only(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>", b"<duration>3</duration><type>half</type><dot/>"
        )
        self.assertEqual([], errors(data))
        self.assertEqual(["measure-length-mismatch"], warnings(data))

    def test_an_implicit_measure_is_not_measured(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>", b"<duration>3</duration><type>half</type><dot/>"
        ).replace(b'<measure number="1">', b'<measure number="1" implicit="yes">')
        self.assertEqual([], warnings(data))

    def test_a_measure_without_meter_is_not_measured(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>", b"<duration>3</duration><type>half</type><dot/>"
        ).replace(
            b"<time><beats>4</beats><beat-type>4</beat-type></time>", b"<time><senza-misura/></time>"
        )
        self.assertEqual([], warnings(data))

    def test_composite_beats_are_summed(self):
        five_eighths = (
            MINIMAL_SCORE.replace(b"<divisions>1</divisions>", b"<divisions>2</divisions>")
            .replace(
                b"<time><beats>4</beats><beat-type>4</beat-type></time>",
                b"<time><beats>3+2</beats><beat-type>8</beat-type></time>",
            )
            .replace(b"<duration>4</duration><type>whole</type>", b"<duration>5</duration><type>half</type>")
        )
        self.assertEqual([], warnings(five_eighths))
        self.assertEqual(["measure-length-mismatch"], warnings(five_eighths.replace(b"<duration>5<", b"<duration>4<")))

    def test_a_timewise_score_is_reported_as_not_checked(self):
        timewise = (
            b'<?xml version="1.0" encoding="UTF-8"?>\n<score-timewise version="4.0">'
            b'<part-list><score-part id="P1"><part-name>Music</part-name></score-part></part-list>'
            b'<measure number="1"><part id="P1">' + WHOLE_NOTE + b"</part></measure></score-timewise>"
        )
        self.assertEqual(["timewise-not-checked"], warnings(timewise))
```

Run → the new tests fail (no findings are produced yet), the old ones still pass.

- [ ] **Step 2: Implement** — in `musicxml_check.py` add `from fractions import Fraction` and `Set` to the typing import, update the module docstring's last sentence to "…exits with 1 when a file cannot be read, is invalid against the schema, or breaks an error-level rule.", replace `check_bytes` with:

```python
def check_bytes(data: bytes) -> Report:
    """Check an uncompressed MusicXML document: the schema, then the semantic rules."""
    try:
        root = parse_document(data)
    except etree.XMLSyntaxError as error:
        return Report(problem=f"not well-formed XML: {error}")
    return Report(
        xsd_errors=_xsd_errors(musicxml_schema(), root), findings=semantic_findings(root)
    )
```

and add, after `read_mxl`:

```python
def _number(text: Optional[str]) -> Optional[Fraction]:
    """The value of a decimal element text, or None when it is absent or not a number."""
    if text is None:
        return None
    try:
        return Fraction(text.strip())
    except (ValueError, ZeroDivisionError):
        return None


def _time_quarters(time: etree._Element) -> Optional[Fraction]:
    """Quarter notes in a measure of this <time>: composite beats such as 3+2 are summed and
    several beats/beat-type pairs added; None under senza-misura or when unreadable."""
    if time.find("senza-misura") is not None:
        return None
    beats = time.findall("beats")
    beat_types = time.findall("beat-type")
    if not beats or len(beats) != len(beat_types):
        return None
    total = Fraction(0)
    for beat, beat_type in zip(beats, beat_types):
        counts = [_number(piece) for piece in (beat.text or "").split("+")]
        unit = _number(beat_type.text)
        if any(count is None for count in counts) or unit is None or unit <= 0:
            return None
        total += sum(counts) * 4 / unit
    return total


class _PartState:
    """What carries over from one measure to the next within a part."""

    def __init__(self) -> None:
        self.divisions: Optional[Fraction] = None
        self.staves = 1
        self.quarters: Optional[Fraction] = None
        self.duration_before_divisions_seen = False
        self.open_ties: Dict[Tuple[str, str, str], int] = {}
        self.open_slurs: Set[str] = set()
        self.open_tuplets: Set[str] = set()

    def measure_length(self) -> Optional[Fraction]:
        """The time signature's length in divisions, or None when either is unknown."""
        if self.quarters is None or self.divisions is None:
            return None
        return self.quarters * self.divisions


def _pitch_key(note: etree._Element) -> Optional[Tuple[str, str, str]]:
    pitch = note.find("pitch")
    if pitch is not None:
        alter = _number(pitch.findtext("alter")) or Fraction(0)
        return (pitch.findtext("step", ""), str(alter), pitch.findtext("octave", ""))
    unpitched = note.find("unpitched")
    if unpitched is not None:
        return (
            unpitched.findtext("display-step", ""),
            "unpitched",
            unpitched.findtext("display-octave", ""),
        )
    return None


def _check_ties(note, state: _PartState, where: str, findings: List[Finding]) -> None:
    key = _pitch_key(note)
    if key is None:
        return
    kinds = {tie.get("type") for tie in note.iterfind("tie")}
    if "stop" in kinds:
        if state.open_ties.get(key, 0) > 0:
            state.open_ties[key] -= 1
        else:
            findings.append(
                Finding("unpaired-tie", "error", where, f"a tie stop on {'/'.join(key)} has no start")
            )
    if "start" in kinds:
        state.open_ties[key] = state.open_ties.get(key, 0) + 1


def _pair_spanners(elements, open_numbers: Set[str], name: str, where: str,
                   findings: List[Finding]) -> None:
    """Pair the starts and stops of slurs or tuplets by number. Stops apply before starts, so a
    note can end one and begin the next with the same number."""
    numbers: Dict[str, List[str]] = {"start": [], "stop": []}
    for element in elements:
        kind = element.get("type")
        if kind in numbers:
            numbers[kind].append(element.get("number", "1"))
    for number in numbers["stop"]:
        if number in open_numbers:
            open_numbers.discard(number)
        else:
            findings.append(
                Finding(f"unpaired-{name}", "error", where, f"{name} {number} stops without a start")
            )
    for number in numbers["start"]:
        if number in open_numbers:
            findings.append(
                Finding(f"unpaired-{name}", "error", where, f"{name} {number} starts while open")
            )
        open_numbers.add(number)


def _check_note(note, where: str, state: _PartState, instrument_ids: Set[str], has_previous: bool,
                cursor: Fraction, end: Fraction,
                findings: List[Finding]) -> Tuple[Fraction, Fraction]:
    """Check one note; return the measure's cursor and furthest position after it."""
    is_grace = note.find("grace") is not None
    is_chord = note.find("chord") is not None
    duration = _number(note.findtext("duration"))
    if duration is not None and state.divisions is None and not state.duration_before_divisions_seen:
        state.duration_before_divisions_seen = True
        findings.append(
            Finding(
                "duration-before-divisions",
                "error",
                where,
                "a note has a <duration> before any <divisions> in its part",
            )
        )
    if is_chord and not has_previous:
        findings.append(
            Finding("chord-without-anchor", "error", where, "<chord/> on the first note of the measure")
        )
    staff = _number(note.findtext("staff"))
    if staff is not None and staff > state.staves:
        findings.append(
            Finding(
                "staff-above-staves",
                "error",
                where,
                f"<staff>{staff}</staff> in a part with {state.staves} staves",
            )
        )
    for instrument in note.iterfind("instrument"):
        if instrument.get("id") not in instrument_ids:
            findings.append(
                Finding(
                    "dangling-instrument-ref",
                    "error",
                    where,
                    f'<instrument id="{instrument.get("id")}"> names no <score-instrument> of the part',
                )
            )
    _check_ties(note, state, where, findings)
    _pair_spanners(note.iterfind("notations/slur"), state.open_slurs, "slur", where, findings)
    _pair_spanners(note.iterfind("notations/tuplet"), state.open_tuplets, "tuplet", where, findings)
    if not is_chord and not is_grace and duration is not None:
        cursor += duration
        end = max(end, cursor)
    return cursor, end


def _check_measure(part_id: str, measure, state: _PartState, instrument_ids: Set[str],
                   findings: List[Finding]) -> None:
    where = f"part {part_id}, measure {measure.get('number', '?')}"
    implicit = measure.get("implicit") == "yes"
    cursor = Fraction(0)
    end = Fraction(0)
    has_note = False
    for child in measure:
        if child.tag == "attributes":
            divisions = _number(child.findtext("divisions"))
            if divisions is not None and divisions > 0:
                state.divisions = divisions
            staves = _number(child.findtext("staves"))
            if staves is not None and staves >= 1:
                state.staves = int(staves)
            time = child.find("time")
            if time is not None:
                state.quarters = _time_quarters(time)
        elif child.tag == "note":
            cursor, end = _check_note(
                child, where, state, instrument_ids, has_note, cursor, end, findings
            )
            has_note = True
        elif child.tag == "backup":
            cursor -= _number(child.findtext("duration")) or Fraction(0)
            if cursor < 0:
                findings.append(
                    Finding(
                        "position-negative",
                        "error",
                        where,
                        "<backup> moves before the start of the measure",
                    )
                )
                cursor = Fraction(0)
        elif child.tag == "forward":
            cursor += _number(child.findtext("duration")) or Fraction(0)
            limit = state.measure_length()
            if limit is not None and not implicit and cursor > limit:
                findings.append(
                    Finding(
                        "position-past-measure-end",
                        "error",
                        where,
                        f"<forward> reaches {cursor} divisions in a measure of {limit}",
                    )
                )
            end = max(end, cursor)
    limit = state.measure_length()
    if limit is not None and not implicit and end != limit:
        findings.append(
            Finding(
                "measure-length-mismatch",
                "warning",
                where,
                f"the content lasts {end} divisions, the time signature {limit}",
            )
        )


def _check_part_list(root, findings: List[Finding]) -> Dict[str, Set[str]]:
    """Check part ids and MIDI instrument references; return each part's instrument ids."""
    instruments: Dict[str, Set[str]] = {}
    for score_part in root.iterfind("part-list/score-part"):
        part_id = score_part.get("id")
        ids = {instrument.get("id") for instrument in score_part.iterfind("score-instrument")}
        instruments[part_id] = ids
        for midi in score_part.iterfind("midi-instrument"):
            if midi.get("id") not in ids:
                findings.append(
                    Finding(
                        "dangling-instrument-ref",
                        "error",
                        f"score-part {part_id}",
                        f'<midi-instrument id="{midi.get("id")}"> names no <score-instrument>',
                    )
                )
    seen: Set[str] = set()
    for part in root.iterfind("part"):
        part_id = part.get("id")
        if part_id not in instruments:
            findings.append(
                Finding("part-without-score-part", "error", f"part {part_id}", "no <score-part> has this id")
            )
        elif part_id in seen:
            findings.append(
                Finding("duplicate-part", "error", f"part {part_id}", "a second <part> for one <score-part>")
            )
        seen.add(part_id)
    for part_id in instruments:
        if part_id not in seen:
            findings.append(
                Finding("score-part-without-part", "error", f"score-part {part_id}", "no <part> has this id")
            )
    return instruments


def _check_part(part, instrument_ids: Set[str], findings: List[Finding]) -> None:
    part_id = part.get("id")
    state = _PartState()
    for measure in part.iterfind("measure"):
        _check_measure(part_id, measure, state, instrument_ids, findings)
    where = f"part {part_id}"
    for key, count in sorted(state.open_ties.items()):
        if count > 0:
            findings.append(
                Finding("unpaired-tie", "error", where, f"a tie start on {'/'.join(key)} has no stop")
            )
    for number in sorted(state.open_slurs):
        findings.append(Finding("unpaired-slur", "error", where, f"slur {number} never stops"))
    for number in sorted(state.open_tuplets):
        findings.append(Finding("unpaired-tuplet", "error", where, f"tuplet {number} never stops"))


def semantic_findings(root: etree._Element) -> List[Finding]:
    """The rules of a MusicXML document that its schema cannot express."""
    if root.tag == "score-timewise":
        return [
            Finding("timewise-not-checked", "warning", "score", "score-timewise documents are not checked")
        ]
    if root.tag != "score-partwise":
        return []
    findings: List[Finding] = []
    instruments = _check_part_list(root, findings)
    for part in root.iterfind("part"):
        _check_part(part, instruments.get(part.get("id"), set()), findings)
    return findings
```

Run `ruff format` and `ruff check` on the two files (the long-line layout above is for reading; ruff decides the final wrapping).

- [ ] **Step 3:** `python -m unittest test_musicxml_check -v` → all pass (including Task 3's suite test: semantic findings do not change `xsd_valid`).

- [ ] **Step 4: Mutations** — one per check family, each failing its test(s), then restored: drop `cursor = Fraction(0)` reset AND the negative test (use `cursor < 0` → `cursor < -1`); change `"+"` split to take only the first piece (composite test fails); process starts before stops in `_check_ties` (the chained-tie test fails); remove the `implicit` condition (implicit test fails); return `[]` early in `_check_part_list` (part tests fail); skip `_pair_spanners` for tuplets (tuplet test fails).

- [ ] **Step 5: Commit** `git add test/musicxml/musicxml_check.py test/test_musicxml_check.py`, message `test: check the MusicXML rules the schema cannot express`.

### Task 5: A canonical dump of a maialib score

**Files:**
- Create: `test/musicxml/dump_score.py`, `test/test_musicxml_dump.py`

**Interfaces:** Produces `dump_score(score) -> Dict[str, Any]` (keys `title`, `composer`, `anacrusis`, `parts[]` → `name`, `staves`, `pitched`, `staff_lines`, `measures[]` → `number`, `divisions`, `key`, `time`, `barlines`, `staves[]` → `clef`, `notes[]`), `_safe(getter) -> Any` (value, or `{"error": "<type>"}`), and the CLI `python dump_score.py SCORE [OUTPUT]`. Used by 4c-1, 4b and 4c-2 to measure before and after.

- [ ] **Step 1: Failing tests** — `test/test_musicxml_dump.py`:

```python
"""dump_score: a canonical, deterministic JSON dump of a maialib Score."""

import json
import sys
import unittest
from pathlib import Path

import maialib as ml

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import dump_score  # noqa: E402

FIXTURE = Path(__file__).resolve().parent / "xml_examples" / "unit_test" / "test_chord.xml"


class DumpScoreTestCase(unittest.TestCase):
    def test_the_dump_is_deterministic_json(self):
        first = json.dumps(dump_score.dump_score(ml.Score(str(FIXTURE))), sort_keys=True)
        second = json.dumps(dump_score.dump_score(ml.Score(str(FIXTURE))), sort_keys=True)
        self.assertEqual(first, second)

    def test_the_dump_holds_every_note_of_every_measure(self):
        score = ml.Score(str(FIXTURE))
        dump = dump_score.dump_score(score)
        self.assertEqual(score.getNumParts(), len(dump["parts"]))
        for p, part_dump in enumerate(dump["parts"]):
            part = score.getPart(p)
            self.assertEqual(part.getNumMeasures(), len(part_dump["measures"]))
            for m, measure_dump in enumerate(part_dump["measures"]):
                notes = sum(len(staff["notes"]) for staff in measure_dump["staves"])
                self.assertEqual(part.getMeasure(m).getNumNotes(), notes)

    def test_a_note_is_dumped_with_its_pitch_rhythm_and_flags(self):
        record = dump_score.note_record(ml.Note("C#4"))
        self.assertEqual("C#4", record["pitch"])
        self.assertTrue(record["on"])
        self.assertFalse(record["chord"])
        self.assertEqual([0, 0], record["transpose"])

    def test_a_getter_that_raises_is_recorded_instead_of_stopping_the_dump(self):
        def broken():
            raise IndexError("out of range")

        self.assertEqual({"error": "IndexError"}, dump_score._safe(broken))


if __name__ == "__main__":
    unittest.main()
```

Run → `ModuleNotFoundError: dump_score`.

- [ ] **Step 2: Implement** `test/musicxml/dump_score.py`:

```python
"""A canonical JSON dump of a maialib Score, to compare the model before and after a change.

Command line: ``python dump_score.py SCORE [OUTPUT]`` writes the dump to OUTPUT or prints it.
Every value comes from maialib's public API; a getter that raises is recorded as
{"error": "<exception type>"} instead of stopping the dump.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any, Callable, Dict, List


def _safe(getter: Callable[[], Any]) -> Any:
    try:
        return getter()
    except Exception as error:  # the exception type is the recorded value
        return {"error": type(error).__name__}


def _clef(measure: Any, staff: int) -> Any:
    def read() -> Dict[str, Any]:
        clef = measure.getClef(staff)
        return {"sign": clef.getSign().name, "line": clef.getLine()}

    return _safe(read)


def _barline(barline: Any) -> Dict[str, str]:
    return {
        "location": barline.getLocation(),
        "style": barline.getBarStyle(),
        "repeat": barline.getDirection(),
    }


def note_record(note: Any) -> Dict[str, Any]:
    """One note as plain data: written pitch, rhythm, voice and staff, flags and notations."""
    return {
        "pitch": _safe(note.getWrittenPitch),
        "on": note.isNoteOn(),
        "pitched": note.isPitched(),
        "ticks": _safe(note.getDurationTicks),
        "divisions": _safe(note.getDivisionsPerQuarterNote),
        "type": _safe(note.getType),
        "dots": _safe(note.getNumDots),
        "voice": note.getVoice(),
        "staff": note.getStaff(),
        "chord": note.inChord(),
        "grace": note.isGraceNote(),
        "transpose": [note.getTransposeDiatonic(), note.getTransposeChromatic()],
        "ties": list(note.getTie()),
        "slur": list(note.getSlur()),
        "beams": list(note.getBeam()),
        "articulations": list(note.getArticulation()),
        "stem": note.getStem(),
        "unpitched_index": note.getUnpitchedIndex(),
    }


def _measure(measure: Any) -> Dict[str, Any]:
    staves = []
    for staff in range(measure.getNumStaves()):
        notes = [note_record(measure.getNote(index, staff)) for index in range(measure.getNumNotes(staff))]
        staves.append({"clef": _clef(measure, staff), "notes": notes})
    return {
        "number": measure.getNumber(),
        "divisions": measure.getDivisionsPerQuarterNote(),
        "key": _safe(
            lambda: {"fifths": measure.getKey().getFifthCircle(), "major": measure.getKey().isMajorMode()}
        ),
        "time": _safe(
            lambda: [measure.getTimeSignature().getUpperValue(), measure.getTimeSignature().getLowerValue()]
        ),
        "barlines": [_barline(measure.getBarlineLeft()), _barline(measure.getBarlineRight())],
        "staves": staves,
    }


def dump_score(score: Any) -> Dict[str, Any]:
    """The score as plain data: the header, then every part, measure, staff and note in order."""
    parts: List[Dict[str, Any]] = []
    for index in range(score.getNumParts()):
        part = score.getPart(index)
        parts.append(
            {
                "name": part.getName(),
                "staves": part.getNumStaves(),
                "pitched": part.isPitched(),
                "staff_lines": part.getStaffLines(),
                "measures": [_measure(part.getMeasure(m)) for m in range(part.getNumMeasures())],
            }
        )
    return {
        "title": score.getTitle(),
        "composer": score.getComposerName(),
        "anacrusis": score.haveAnacrusisMeasure(),
        "parts": parts,
    }


def main(argv: List[str]) -> int:
    if len(argv) not in (1, 2):
        print("usage: dump_score.py SCORE [OUTPUT]", file=sys.stderr)
        return 2
    import maialib as ml

    text = json.dumps(dump_score(ml.Score(argv[0])), indent=1, sort_keys=True) + "\n"
    if len(argv) == 2:
        Path(argv[1]).write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
```

If a binding name used above does not exist in the installed maialib (check with `dir(ml.Note)` etc.), use the name the bindings expose and say so in the report; do not change maiacore.

- [ ] **Step 3:** `python -m unittest test_musicxml_dump -v` → 4 tests pass. `python test/musicxml/dump_score.py maialib/xml-scores-examples/Chopin_Fantasie_Impromptu.mxl > <scratch>/chopin.json` runs; mention the dump's size.

- [ ] **Step 4: Mutations:** dump only staff 0 (`range(1)`) → the every-note test fails on a multi-staff measure of the fixture (if the fixture has one staff, use `xml_examples/unit_test/test_staves.xml` instead and say which); drop `"transpose"` → the note test fails; let `_safe` re-raise → the getter test fails.

- [ ] **Step 5: Commit** `git add test/musicxml/dump_score.py test/test_musicxml_dump.py`, message `test: a canonical dump of a maialib score`.

### Task 6: The corpus ledger

**Files:**
- Create: `test/musicxml/corpus_worker.py`, `test/musicxml/corpus.py`, `test/musicxml/ledger.json` (generated), `test/test_musicxml_corpus.py`, `scripts/make-corpus.py`, `test/musicxml/README.md`
- Modify: `Makefile`

**Interfaces:**
- Consumes: `musicxml_check.check_file`, `check_bytes`, `Report` (Tasks 3–4).
- Produces (module `corpus_worker`): `PREFIX = "CORPUS-RECORD "`, `PENDING = "pending"`, `NOT_APPLICABLE = "n/a"`, `STAGE_ORDER = ("input", "load", "analyses", "export", "export_xml", "export_xsd", "roundtrip")`, `new_record(analyses: bool = False) -> Dict[str, Any]`, CLI `python corpus_worker.py PATH [--analyses]`.
- Produces (module `corpus`): `HERE`, `REPO_ROOT`, `LEDGER`, `EXTERNAL_ROOT`, `EXTERNAL_LEDGER`, `corpus_files() -> List[str]`, `external_files() -> List[str]`, `is_slow(name) -> bool`, `timeout_for(name) -> float`, `ended(record, outcome) -> Record`, `run_one(name, analyses=False, timeout=None) -> Record`, `run_corpus(names, analyses=False, workers=None) -> Dict[str, Record]`, `load_ledger(path)`, `write_ledger(path, records)`, `updated_ledger(old, actual)`, `compare(expected, actual) -> List[str]`. Names are repository-relative POSIX paths.
- Record fields: `input` (`valid`/`invalid`/`unreadable`), `load` (`ok`/exception type/`crash`/`timeout`), `analyses` (fuzz only), `export` (`ok`/exception type/`crash`/`timeout`/`n/a`), `export_xml` (`well-formed`/`ill-formed`/`n/a`), `export_xsd` (`valid`/`invalid`/`n/a`), `export_errors` (list of check names), `roundtrip` (`stable`/`unstable`/exception type/`crash`/`timeout`/`n/a`). Ledger entries may add `"slow": true`, a `"note"`, and `{"any_of": [...]}` values.

- [ ] **Step 1: Failing tests** — `test/test_musicxml_corpus.py`:

```python
"""Every file of the MusicXML corpus behaves as test/musicxml/ledger.json records.

The slow files (marked in the ledger) run only under `make corpus`.
"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import corpus  # noqa: E402
import corpus_worker  # noqa: E402

FINISHED = {
    "input": "valid",
    "load": "ok",
    "export": "ok",
    "export_xml": "well-formed",
    "export_xsd": "invalid",
    "export_errors": [],
    "roundtrip": "stable",
}


class LedgerLogicTestCase(unittest.TestCase):
    def test_an_unfinished_stage_takes_the_outcome_and_later_stages_are_not_applicable(self):
        record = corpus_worker.new_record()
        record.update(input="valid", load="ok")
        self.assertEqual(
            {**record, "export": "crash", "export_xml": "n/a", "export_xsd": "n/a", "roundtrip": "n/a"},
            corpus.ended(dict(record), "crash"),
        )

    def test_a_finished_record_is_left_as_it_is(self):
        self.assertEqual(FINISHED, corpus.ended(dict(FINISHED), "crash"))

    def test_a_changed_field_is_a_difference(self):
        problems = corpus.compare({"a.xml": FINISHED}, {"a.xml": {**FINISHED, "load": "IndexError"}})
        self.assertEqual(1, len(problems))
        self.assertIn("load", problems[0])

    def test_alternatives_accept_any_listed_value_and_slow_and_note_are_ignored(self):
        entry = {**FINISHED, "load": {"any_of": ["ok", "crash"]}, "slow": True, "note": "why"}
        self.assertEqual([], corpus.compare({"a.xml": entry}, {"a.xml": FINISHED}))

    def test_a_file_missing_from_the_ledger_is_a_difference(self):
        self.assertEqual(["b.xml: not in the ledger"], corpus.compare({}, {"b.xml": FINISHED}))


class CorpusLedgerTestCase(unittest.TestCase):
    def test_the_ledger_lists_every_corpus_file(self):
        self.assertEqual(corpus.corpus_files(), sorted(corpus.load_ledger(corpus.LEDGER)))

    def test_every_corpus_file_matches_its_ledger_entry(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        files = [name for name in corpus.corpus_files() if not ledger.get(name, {}).get("slow")]
        problems = corpus.compare(ledger, corpus.run_corpus(files))
        self.assertEqual(
            [], problems, "\n" + "\n".join(problems) + "\nReview, then `make corpus-update-ledger`."
        )


if __name__ == "__main__":
    unittest.main()
```

Run → `ModuleNotFoundError: corpus`.

- [ ] **Step 2: Implement** `test/musicxml/corpus_worker.py`:

```python
"""Examine one MusicXML file with maialib, for the corpus ledger and the fuzz driver.

Usage: ``python corpus_worker.py PATH [--analyses]``

After each stage the worker prints a line "CORPUS-RECORD <json>" holding the record so far, so
the parent still learns the finished stages when a later one crashes or hangs. The stages, in
order: input (the file itself against the MusicXML 4.0 schema), load (maialib.Score), analyses
(only with --analyses: chords and the intervals between consecutive notes), export
(Score.toXML), the export's checks (well-formed, schema, semantic errors), and roundtrip (the
export loaded and exported again, compared without its encoding date).
"""

from __future__ import annotations

import json
import re
import sys
import tempfile
from pathlib import Path
from typing import Any, Dict, List

import musicxml_check

PREFIX = "CORPUS-RECORD "
PENDING = "pending"
NOT_APPLICABLE = "n/a"
STAGE_ORDER = ("input", "load", "analyses", "export", "export_xml", "export_xsd", "roundtrip")
_ENCODING_DATE = re.compile(r"<encoding-date>[^<]*</encoding-date>")

Record = Dict[str, Any]


def new_record(analyses: bool = False) -> Record:
    """A record with every stage pending; "analyses" only when that stage runs."""
    record: Record = {name: PENDING for name in STAGE_ORDER if analyses or name != "analyses"}
    record["export_errors"] = []
    return record


def emit(record: Record) -> None:
    print(PREFIX + json.dumps(record, sort_keys=True), flush=True)


def finish(record: Record) -> None:
    """Mark the stages that did not run as not applicable, then emit the final record."""
    for name in STAGE_ORDER:
        if record.get(name) == PENDING:
            record[name] = NOT_APPLICABLE
    emit(record)


def input_status(path: Path) -> str:
    report = musicxml_check.check_file(path)
    if not report.readable:
        return "unreadable"
    return "valid" if report.xsd_valid else "invalid"


def without_encoding_date(text: str) -> str:
    return _ENCODING_DATE.sub("", text)


def run_analyses(ml: Any, score: Any) -> str:
    """Chord extraction, and the interval between each pair of consecutive pitched notes."""
    try:
        score.getChords()
        for p in range(score.getNumParts()):
            part = score.getPart(p)
            previous = None
            for m in range(part.getNumMeasures()):
                measure = part.getMeasure(m)
                for s in range(measure.getNumStaves()):
                    for n in range(measure.getNumNotes(s)):
                        note = measure.getNote(n, s)
                        if not note.isNoteOn() or not note.isPitched():
                            continue
                        if previous is not None:
                            ml.Interval(previous, note).getName()
                        previous = note
    except Exception as error:  # the exception type is the result
        return type(error).__name__
    return "ok"


def roundtrip_status(ml: Any, exported: str) -> str:
    with tempfile.TemporaryDirectory() as folder:
        path = Path(folder) / "roundtrip.musicxml"
        path.write_text(exported, encoding="utf-8")
        try:
            again = ml.Score(str(path)).toXML()
        except Exception as error:  # the exception type is the result
            return type(error).__name__
    return "stable" if without_encoding_date(again) == without_encoding_date(exported) else "unstable"


def examine(path: Path, analyses: bool) -> None:
    record = new_record(analyses)
    record["input"] = input_status(path)
    emit(record)

    import maialib as ml

    try:
        score = ml.Score(str(path))
    except Exception as error:  # the exception type is the result
        record["load"] = type(error).__name__
        finish(record)
        return
    record["load"] = "ok"
    emit(record)

    if analyses:
        record["analyses"] = run_analyses(ml, score)
        emit(record)

    try:
        exported = score.toXML()
    except Exception as error:  # the exception type is the result
        record["export"] = type(error).__name__
        finish(record)
        return
    record["export"] = "ok"
    report = musicxml_check.check_bytes(exported.encode("utf-8"))
    record["export_xml"] = "well-formed" if report.readable else "ill-formed"
    if report.readable:
        record["export_xsd"] = "valid" if report.xsd_valid else "invalid"
        record["export_errors"] = report.errors
    emit(record)

    record["roundtrip"] = roundtrip_status(ml, exported)
    finish(record)


def main(argv: List[str]) -> int:
    if sys.platform == "win32":
        import ctypes

        # A crash ends the process at once instead of opening the Windows error-reporting
        # dialog, which would hold it until the parent's timeout.
        sem_failcriticalerrors, sem_nogpfaulterrorbox = 0x0001, 0x0002
        ctypes.windll.kernel32.SetErrorMode(sem_failcriticalerrors | sem_nogpfaulterrorbox)
    analyses = "--analyses" in argv
    paths = [argument for argument in argv if argument != "--analyses"]
    if len(paths) != 1:
        print("usage: corpus_worker.py PATH [--analyses]", file=sys.stderr)
        return 2
    examine(Path(paths[0]), analyses)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
```

and `test/musicxml/corpus.py`:

```python
"""The MusicXML corpus: which files it holds, how they are examined, and the ledger of results.

The in-repository corpus is every MusicXML file under test/xml_examples, the samples bundled in
maialib/xml-scores-examples and the vendored W3C test suite; `make corpus-fetch` adds the
external corpus under test/musicxml/external. corpus_worker.py examines each file in its own
process, so a crash or a hang becomes a result instead of ending the run. README.md describes
the ledger.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional

import corpus_worker

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parents[1]
WORKER = HERE / "corpus_worker.py"
LEDGER = HERE / "ledger.json"
EXTERNAL_ROOT = HERE / "external"
EXTERNAL_LEDGER = HERE / "ledger-external.json"
CORPUS_ROOTS = ("test/xml_examples", "maialib/xml-scores-examples", "test/musicxml/w3c-test-suite")
SUFFIXES = (".xml", ".musicxml", ".mxl", ".invalid")
SLOW_BYTES = 10_000_000
SLOW_TIMEOUT = 3600.0
IGNORED_KEYS = ("slow", "note")

Record = Dict[str, Any]


def _files_under(root: Path) -> List[str]:
    return sorted(
        path.relative_to(REPO_ROOT).as_posix()
        for path in root.rglob("*")
        if path.is_file() and ".git" not in path.parts and path.name.lower().endswith(SUFFIXES)
    )


def corpus_files() -> List[str]:
    """Repository-relative paths of the in-repository corpus, sorted."""
    return sorted(name for root in CORPUS_ROOTS for name in _files_under(REPO_ROOT / root))


def external_files() -> List[str]:
    """Repository-relative paths of the fetched external corpus; empty until it is fetched."""
    return _files_under(EXTERNAL_ROOT) if EXTERNAL_ROOT.is_dir() else []


def is_slow(name: str) -> bool:
    """Files of 10 MB or more: `make py-tests` skips them, `make corpus` runs them."""
    return (REPO_ROOT / name).stat().st_size >= SLOW_BYTES


def timeout_for(name: str) -> float:
    """Seconds allowed for one file: 30 s plus 10 s per megabyte, and an hour for a slow file."""
    if is_slow(name):
        return SLOW_TIMEOUT
    return 30 + 10 * (REPO_ROOT / name).stat().st_size / 1_000_000


def _last_record(output: bytes) -> Optional[Record]:
    last = None
    for line in output.decode("utf-8", errors="replace").splitlines():
        if line.startswith(corpus_worker.PREFIX):
            last = json.loads(line[len(corpus_worker.PREFIX) :])
    return last


def ended(record: Record, outcome: str) -> Record:
    """Mark the first unfinished stage with ``outcome`` ("crash" or "timeout"), later ones n/a."""
    marked = False
    for name in corpus_worker.STAGE_ORDER:
        if record.get(name) == corpus_worker.PENDING:
            record[name] = corpus_worker.NOT_APPLICABLE if marked else outcome
            marked = True
    return record


def run_one(name: str, analyses: bool = False, timeout: Optional[float] = None) -> Record:
    """Examine one repository-relative file in a fresh process and return its record."""
    command = [sys.executable, str(WORKER), str(REPO_ROOT / name)]
    if analyses:
        command.append("--analyses")
    limit = timeout_for(name) if timeout is None else timeout
    try:
        completed = subprocess.run(command, cwd=str(REPO_ROOT), capture_output=True, timeout=limit)
    except subprocess.TimeoutExpired as expired:
        record = _last_record(expired.stdout or b"") or corpus_worker.new_record(analyses)
        return ended(record, "timeout")
    record = _last_record(completed.stdout) or corpus_worker.new_record(analyses)
    return ended(record, "crash")


def run_corpus(names: Iterable[str], analyses: bool = False,
               workers: Optional[int] = None) -> Dict[str, Record]:
    """Examine the files in parallel and return their records by name."""
    names = list(names)
    count = workers or max(2, (os.cpu_count() or 2) // 2)
    with ThreadPoolExecutor(max_workers=count) as pool:
        records = list(pool.map(lambda name: run_one(name, analyses), names))
    return dict(zip(names, records))


def load_ledger(path: Path) -> Dict[str, Record]:
    return json.loads(path.read_text(encoding="utf-8"))["files"]


def write_ledger(path: Path, records: Dict[str, Record]) -> None:
    """Write one file per line, sorted, so that a change shows as a one-line diff."""
    lines = [
        f"  {json.dumps(name)}: {json.dumps(records[name], sort_keys=True)}" for name in sorted(records)
    ]
    path.write_text('{"files": {\n' + ",\n".join(lines) + "\n}}\n", encoding="utf-8", newline="\n")


def updated_ledger(old: Dict[str, Record], actual: Dict[str, Record]) -> Dict[str, Record]:
    """The ledger to write for these results: where the old entry allowed several values and the
    new one is among them, the alternatives and the note stay; slow files are marked."""
    ledger: Dict[str, Record] = {}
    for name, record in actual.items():
        entry = dict(record)
        previous = old.get(name, {})
        for key, allowed in previous.items():
            if isinstance(allowed, dict) and record.get(key) in allowed.get("any_of", []):
                entry[key] = allowed
        if "note" in previous:
            entry["note"] = previous["note"]
        if is_slow(name):
            entry["slow"] = True
        ledger[name] = entry
    return ledger


def _matches(allowed: Any, value: Any) -> bool:
    if isinstance(allowed, dict):
        return value in allowed.get("any_of", [])
    return allowed == value


def compare(expected: Dict[str, Record], actual: Dict[str, Record]) -> List[str]:
    """Every difference between the ledger entries and this run's records, sorted by file."""
    problems: List[str] = []
    for name in sorted(actual):
        if name not in expected:
            problems.append(f"{name}: not in the ledger")
            continue
        want = {key: value for key, value in expected[name].items() if key not in IGNORED_KEYS}
        got = actual[name]
        for key in sorted(set(want) | set(got)):
            if not _matches(want.get(key), got.get(key)):
                problems.append(f"{name}: {key} is {got.get(key)!r}, the ledger says {want.get(key)!r}")
    return problems
```

- [ ] **Step 3: `scripts/make-corpus.py`:**

```python
"""`make corpus` and `make corpus-update-ledger`: the MusicXML corpus against its ledgers.

`make corpus` examines every file of the in-repository corpus, the slow ones included, and the
external corpus once `make corpus-fetch` has downloaded it, and fails when a result differs from
test/musicxml/ledger.json or ledger-external.json. `make corpus-update-ledger` writes the results
as the ledgers instead; review the diff before committing it. Both need maialib installed
(`make dev`).
"""

import argparse
import sys

from build_utils import REPO_ROOT
from terminal_colors import color

sys.path.insert(0, str(REPO_ROOT / "test" / "musicxml"))

import corpus  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--update-ledger", action="store_true", help="write the results as the ledgers")
    arguments = parser.parse_args()
    failed = False
    for ledger, files in (
        (corpus.LEDGER, corpus.corpus_files()),
        (corpus.EXTERNAL_LEDGER, corpus.external_files()),
    ):
        if not files:
            continue
        print(f"{color.OKGREEN}Examining {len(files)} files for {ledger.name}...{color.ENDC}", flush=True)
        actual = corpus.run_corpus(files)
        old = corpus.load_ledger(ledger) if ledger.is_file() else {}
        if arguments.update_ledger:
            corpus.write_ledger(ledger, corpus.updated_ledger(old, actual))
            print(f"{color.OKGREEN}Wrote {ledger.relative_to(REPO_ROOT)}{color.ENDC}")
            continue
        problems = corpus.compare(old, actual)
        problems += [f"{name}: in the ledger but not in the corpus" for name in sorted(set(old) - set(actual))]
        for problem in problems:
            print(f"{color.FAIL}{problem}{color.ENDC}")
        failed = failed or bool(problems)
    if failed:
        print(f"{color.FAIL}The corpus differs from its ledger: review, then `make corpus-update-ledger`.{color.ENDC}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
```

In the `Makefile`: add `.PHONY: corpus` and `.PHONY: corpus-update-ledger` to the `.PHONY` list, and after the `tests:` target:

```make
# Every file of the MusicXML corpus, the slow ones included, and the external corpus once
# `make corpus-fetch` has downloaded it, compared with test/musicxml/ledger*.json.
corpus:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py

# Write the current corpus results as the ledgers; review the diff before committing it.
corpus-update-ledger:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py --update-ledger
```

- [ ] **Step 4: Generate the ledger.** With maialib freshly installed from this branch (brand-new venv, `make dev`): `make corpus-update-ledger` (runs every file, slow ones included). Then run `make corpus` twice — both must report no difference (determinism). Inspect `test/musicxml/ledger.json` and put a summary table in the report: for each field, how many files have each value; list every file whose `load` is not `ok`, and every `crash`/`timeout`. Expected from the 2026-10-01 survey: the Mozart sample loads with `IndexError`; the other in-repository fixtures and samples load. Whatever the W3C suite files do is recorded as found — 4a changes no behaviour, so a crash or a timeout is a fact for 4c-1, not something to fix here.

- [ ] **Step 5: `test/musicxml/README.md`:**

```markdown
# MusicXML test infrastructure

| Path | What |
|---|---|
| `schema-4.0/` | The official MusicXML 4.0 XSD, unmodified (see `NOTICE.md`) |
| `w3c-test-suite/` | The W3C MusicXML test suite, unmodified (see `ORIGIN.md`) |
| `musicxml_check.py` | Validation: the 4.0 schema, offline through `lxml`, and the rules the schema cannot express |
| `dump_score.py` | A canonical JSON dump of a maialib `Score` |
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
```

- [ ] **Step 6:** `python -m unittest test_musicxml_corpus -v` → all pass (note its run time in the report). Run the whole `make py-tests` in the brand-new venv → 499 + the new tests, OK.

- [ ] **Step 7: Cross-compiler check:** `make msvc-gate` (its Python step now installs lxml and runs the corpus test against the MSVC-built module). If a file's outcome differs between clang and MSVC, record both in the ledger as `{"any_of": [...]}` with a `note` naming the undefined-behaviour site (spec Appendix B) and say so in the report.

- [ ] **Step 8: Mutations:** make `ended` mark every pending stage with the outcome (the first `LedgerLogicTestCase` test fails); make `compare` ignore `export_errors` (`test_a_changed_field…` variant: change `export_errors` instead of `load` in a scratch copy of the test to see it fail — or add that case permanently if it fails); flip one ledger value by hand (e.g. a sample's `roundtrip`) → `test_every_corpus_file_matches_its_ledger_entry` fails naming it; restore.

- [ ] **Step 9: Commit** `git add Makefile scripts/make-corpus.py test/musicxml/corpus.py test/musicxml/corpus_worker.py test/musicxml/ledger.json test/musicxml/README.md test/test_musicxml_corpus.py`, message `test: a ledger of how every corpus file loads, exports, validates and round-trips`.

### Task 7: The external corpus

**Files:**
- Create: `scripts/make-corpus-fetch.py`, `test/musicxml/.gitignore`, `test/musicxml/ledger-external.json` (generated)
- Modify: `Makefile`, `test/musicxml/README.md`

**Interfaces:** Consumes `corpus.external_files`, `corpus.EXTERNAL_LEDGER` and `scripts/make-corpus.py` (Task 6), which already handle the external corpus once it exists.

- [ ] **Step 1: Ignore rule** — `test/musicxml/.gitignore`:

```
# Downloaded by `make corpus-fetch`.
external/
```

- [ ] **Step 2: `scripts/make-corpus-fetch.py`:**

```python
"""`make corpus-fetch`: download the external MusicXML corpus at pinned commits.

OpenScore Lieder and OpenScore String Quartets (both CC0) go to test/musicxml/external/, which git
ignores; only their .mxl scores and licence files are checked out. `make corpus` then examines
them against test/musicxml/ledger-external.json.
"""

from build_utils import REPO_ROOT, run_step
from terminal_colors import color

EXTERNAL = REPO_ROOT / "test" / "musicxml" / "external"
REPOSITORIES = (
    ("Lieder", "https://github.com/OpenScore/Lieder", "38c5db510224d9facdc4b08d741fc788cfb58ea8"),
    (
        "StringQuartets",
        "https://github.com/OpenScore/StringQuartets",
        "9be3df2ace482130fe031b9e8a647cdf112ed243",
    ),
)

EXTERNAL.mkdir(parents=True, exist_ok=True)
for name, url, commit in REPOSITORIES:
    target = EXTERNAL / name
    if not (target / ".git").is_dir():
        run_step(
            ["git", "clone", "--filter=blob:none", "--no-checkout", url, str(target)], f"clone {name}"
        )
    run_step(
        ["git", "-C", str(target), "sparse-checkout", "set", "--no-cone", "/**/*.mxl", "/LICENSE*"],
        f"select the scores of {name}",
    )
    run_step(["git", "-C", str(target), "checkout", "--quiet", commit], f"check out {name} at {commit[:8]}")
print(f"{color.OKGREEN}External corpus ready in {EXTERNAL.relative_to(REPO_ROOT)}{color.ENDC}")
```

Makefile: `.PHONY: corpus-fetch` and, after `corpus-update-ledger`:

```make
# Download the external MusicXML corpus (OpenScore, CC0) at pinned commits into
# test/musicxml/external/, which git ignores.
corpus-fetch:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus-fetch.py
```

- [ ] **Step 3:** `make corpus-fetch`; count the `.mxl` files (expect about 1,462 + 196) and report it. `git status` must not show `test/musicxml/external`.

- [ ] **Step 4:** `make corpus-update-ledger` → writes `ledger-external.json` (and rewrites `ledger.json`, which must come out identical — `git diff --stat test/musicxml/ledger.json` empty). Then `make corpus` → no difference. Summarise the external results in the report (counts per field value; every non-`ok` load). Check `ledger-external.json` stays under 500 KB (pre-commit's large-file limit); report its size.

- [ ] **Step 5:** README: add rows `ledger-external.json` (the same for the external corpus) and `external/` (downloaded by `make corpus-fetch`; ignored by git), and the command line "`make corpus-fetch` downloads OpenScore Lieder and String Quartets (CC0) at pinned commits; `make corpus` then includes them."

- [ ] **Step 6: Commit** `git add Makefile scripts/make-corpus-fetch.py test/musicxml/.gitignore test/musicxml/ledger-external.json test/musicxml/README.md`, message `build: make corpus-fetch downloads the OpenScore corpus at pinned commits`.

### Task 8: Seeded mutation fuzzing

**Files:**
- Create: `test/musicxml/fuzz.py`, `scripts/make-fuzz.py`, `test/test_musicxml_fuzz.py`
- Modify: `Makefile`, `test/musicxml/.gitignore`, `test/musicxml/README.md`

**Interfaces:**
- Consumes: `corpus.run_one`, `corpus.corpus_files`, `corpus.HERE`, `corpus.REPO_ROOT` (Task 6); `musicxml_check.parse_document`, `read_mxl` (Task 3).
- Produces (module `fuzz`): `XML_MUTATIONS`, `MXL_MUTATIONS`, `UNMINIMISABLE`, `Case(seed, index, source, mutation, data, suffix)`, `seed_files()`, `make_case(seed, index, seeds) -> Case`, `mutate_xml(data, mutation, rng) -> bytes`, `mutate_archive(data, mutation, rng) -> bytes`, `classify(record) -> str`, `worth_minimising(outcome) -> bool`, `run_case(case, timeout, tag="") -> Record`, `run(seed, cases, minutes=None, timeout=30.0, workers=None) -> List[Tuple[Case, Record, str]]`, `summary(results)`, `write_report(seed, results) -> Path`, `minimize(case, outcome, timeout, budget=200) -> bytes`, `save_regression(case, outcome, data) -> Path`.

- [ ] **Step 1: Failing tests** — `test/test_musicxml_fuzz.py`:

```python
"""The fuzz driver's mutations, case generation and outcome classes (no worker runs here)."""

import io
import random
import sys
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import fuzz  # noqa: E402
import musicxml_check  # noqa: E402
from fixtures import CONTAINER, MINIMAL_SCORE  # noqa: E402
from lxml import etree  # noqa: E402

BYTE_LEVEL = ("utf16", "byte-order-mark", "latin1-declaration", "truncate")


def canonical(data: bytes) -> bytes:
    return etree.tostring(musicxml_check.parse_document(data), method="c14n")


def archive() -> bytes:
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w") as zipped:
        zipped.writestr("META-INF/container.xml", CONTAINER)
        zipped.writestr("score.musicxml", MINIMAL_SCORE)
    return buffer.getvalue()


class MutationTestCase(unittest.TestCase):
    def test_every_xml_mutation_changes_the_document(self):
        for mutation in fuzz.XML_MUTATIONS:
            with self.subTest(mutation=mutation):
                mutant = fuzz.mutate_xml(MINIMAL_SCORE, mutation, random.Random(mutation))
                if mutation in BYTE_LEVEL:
                    self.assertNotEqual(MINIMAL_SCORE, mutant)
                else:
                    self.assertNotEqual(canonical(MINIMAL_SCORE), canonical(mutant))

    def test_the_same_generator_gives_the_same_mutant(self):
        for mutation in fuzz.XML_MUTATIONS:
            with self.subTest(mutation=mutation):
                first = fuzz.mutate_xml(MINIMAL_SCORE, mutation, random.Random(7))
                self.assertEqual(first, fuzz.mutate_xml(MINIMAL_SCORE, mutation, random.Random(7)))

    def test_a_case_is_reproducible_from_its_seed_and_number(self):
        seeds = fuzz.seed_files()[:30]
        first = fuzz.make_case(3, 11, seeds)
        second = fuzz.make_case(3, 11, seeds)
        self.assertEqual((first.source, first.mutation, first.data), (second.source, second.mutation, second.data))

    def test_archive_mutations(self):
        rng = random.Random(1)
        without = zipfile.ZipFile(io.BytesIO(fuzz.mutate_archive(archive(), "mxl-drop-container", rng)))
        self.assertNotIn("META-INF/container.xml", without.namelist())
        wrong = zipfile.ZipFile(io.BytesIO(fuzz.mutate_archive(archive(), "mxl-wrong-rootfile", rng)))
        self.assertIn(b'full-path="missing.musicxml"', wrong.read("META-INF/container.xml"))
        self.assertNotEqual(archive(), fuzz.mutate_archive(archive(), "mxl-corrupt-zip", rng))

    def test_byte_level_and_archive_mutants_are_not_minimised(self):
        for mutation in fuzz.UNMINIMISABLE:
            case = fuzz.Case(1, 1, "x", mutation, b"<broken", ".musicxml")
            self.assertEqual(b"<broken", fuzz.minimize(case, "crash:load", timeout=1))


def record(**fields):
    base = {
        "input": "valid", "load": "ok", "analyses": "ok", "export": "ok",
        "export_xml": "well-formed", "export_xsd": "valid", "export_errors": [], "roundtrip": "stable",
    }
    base.update(fields)
    return base


class ClassifyTestCase(unittest.TestCase):
    def test_outcome_classes(self):
        cases = [
            (record(), "ok"),
            (record(load="crash", analyses="n/a", export="n/a", export_xml="n/a", export_xsd="n/a", roundtrip="n/a"), "crash:load"),
            (record(analyses="timeout"), "timeout:analyses"),
            (record(load="IndexError", analyses="n/a", export="n/a", export_xml="n/a", export_xsd="n/a", roundtrip="n/a"), "load:IndexError"),
            (record(export="UnicodeDecodeError", export_xml="n/a", export_xsd="n/a", roundtrip="n/a"), "export:UnicodeDecodeError"),
            (record(export_xml="ill-formed", export_xsd="n/a"), "export:ill-formed"),
            (record(export_xsd="invalid"), "export:xsd-invalid"),
            (record(export_errors=["unpaired-tie"]), "export:semantic-errors"),
            (record(roundtrip="unstable"), "roundtrip:unstable"),
            (record(export_xsd="invalid", roundtrip="crash"), "crash:roundtrip"),
        ]
        for fields, expected in cases:
            with self.subTest(expected=expected):
                self.assertEqual(expected, fuzz.classify(fields))

    def test_expected_rejections_are_not_minimised(self):
        for outcome in ("ok", "load:RuntimeError", "export:xsd-invalid", "export:semantic-errors", "roundtrip:unstable"):
            self.assertFalse(fuzz.worth_minimising(outcome))
        for outcome in ("crash:load", "timeout:load", "load:IndexError", "export:ill-formed"):
            self.assertTrue(fuzz.worth_minimising(outcome))


if __name__ == "__main__":
    unittest.main()
```

Run → `ModuleNotFoundError: fuzz`.

- [ ] **Step 2: Implement** `test/musicxml/fuzz.py`:

```python
"""Seeded mutation fuzzing of maialib's MusicXML reader and writer.

Case N of seed S picks a small corpus file and one mutation with a random generator seeded with
"S:N", so the same seed and case number always give the same mutant. Each mutant runs through
corpus_worker.py --analyses in its own process: load, chords and intervals, export, the export's
checks and the round trip.
"""

from __future__ import annotations

import copy
import io
import json
import os
import random
import re
import time
import zipfile
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

from lxml import etree

import corpus
import musicxml_check

SEED_MAX_BYTES = 200_000
WORK = corpus.HERE / "fuzz-work"
REGRESSIONS = corpus.HERE / "fuzz-regressions"
XML_MUTATIONS = (
    "delete-element",
    "empty-text",
    "non-numeric-text",
    "negative-number",
    "zero-number",
    "huge-number",
    "duplicate-element",
    "reorder-siblings",
    "invalid-enum",
    "utf16",
    "byte-order-mark",
    "latin1-declaration",
    "truncate",
)
MXL_MUTATIONS = ("mxl-drop-container", "mxl-wrong-rootfile", "mxl-corrupt-zip")
# Mutants that are not a well-formed document, so removing elements cannot minimise them.
UNMINIMISABLE = ("utf16", "byte-order-mark", "latin1-declaration", "truncate") + MXL_MUTATIONS
ENUM_ELEMENTS = {"step", "type", "mode", "sign", "bar-style", "stem", "accidental", "notehead", "beam", "syllabic"}
ENUM_ATTRIBUTES = ("type", "location", "direction", "placement", "orientation")
NUMBER = re.compile(r"\s*-?\d+(\.\d+)?\s*")
EXPECTED_OUTCOMES = ("ok", "load:RuntimeError", "export:xsd-invalid", "export:semantic-errors", "roundtrip:unstable")

Record = Dict[str, Any]


@dataclass
class Case:
    """One mutant: the seed and case number that made it, its source file, mutation and bytes."""

    seed: int
    index: int
    source: str
    mutation: str
    data: bytes
    suffix: str


def seed_files() -> List[str]:
    """The corpus files small enough to fuzz quickly."""
    return [
        name
        for name in corpus.corpus_files()
        if (corpus.REPO_ROOT / name).stat().st_size <= SEED_MAX_BYTES
    ]


def make_case(seed: int, index: int, seeds: List[str]) -> Case:
    rng = random.Random(f"{seed}:{index}")
    source = seeds[rng.randrange(len(seeds))]
    raw = (corpus.REPO_ROOT / source).read_bytes()
    is_archive = raw[:2] == b"PK"
    mutation = rng.choice(XML_MUTATIONS + (MXL_MUTATIONS if is_archive else ()))
    if mutation in MXL_MUTATIONS:
        return Case(seed, index, source, mutation, mutate_archive(raw, mutation, rng), ".mxl")
    document = musicxml_check.read_mxl(raw)[0] if is_archive else raw
    return Case(seed, index, source, mutation, mutate_xml(document or raw, mutation, rng), ".musicxml")


def _declared(text: str, encoding: str) -> str:
    """The document with an XML declaration naming ``encoding``."""
    if text.startswith("<?xml"):
        end = text.index("?>")
        declaration = re.sub(r"encoding\s*=\s*[\"'][^\"']*[\"']", f'encoding="{encoding}"', text[:end])
        if "encoding" not in declaration:
            declaration += f' encoding="{encoding}"'
        return declaration + text[end:]
    return f'<?xml version="1.0" encoding="{encoding}"?>\n' + text


def _serialize(root: etree._Element) -> bytes:
    return etree.tostring(root.getroottree(), xml_declaration=True, encoding="UTF-8")


def mutate_xml(data: bytes, mutation: str, rng: random.Random) -> bytes:
    if mutation == "truncate":
        return data[: rng.randrange(1, max(2, len(data)))]
    if mutation == "byte-order-mark":
        return b"\xef\xbb\xbf" + data
    if mutation == "utf16":
        return _declared(data.decode("utf-8", errors="replace"), "UTF-16").encode("utf-16")
    if mutation == "latin1-declaration":
        return _declared(data.decode("utf-8", errors="replace"), "ISO-8859-1").encode("utf-8")
    try:
        root = musicxml_check.parse_document(data)
    except etree.XMLSyntaxError:
        return data[: max(1, len(data) // 2)]
    _mutate_tree(root, mutation, rng)
    return _serialize(root)


def _mutate_tree(root: etree._Element, mutation: str, rng: random.Random) -> None:
    elements = [element for element in root.iter() if isinstance(element.tag, str) and element is not root]
    numbers = [element for element in elements if element.text is not None and NUMBER.fullmatch(element.text)]
    pools = {
        "empty-text": [element for element in elements if element.text and element.text.strip()],
        "non-numeric-text": numbers,
        "negative-number": numbers,
        "zero-number": [element for element in numbers if float(element.text) != 0],
        "huge-number": numbers,
        "invalid-enum": [
            element
            for element in elements
            if element.tag in ENUM_ELEMENTS or any(name in element.attrib for name in ENUM_ATTRIBUTES)
        ],
        "reorder-siblings": [element for element in [root, *elements] if len(element) >= 2],
    }
    pool = pools.get(mutation, elements)
    if not pool:  # nothing this mutation applies to: delete an element instead
        mutation, pool = "delete-element", elements
    if not pool:
        return
    target = rng.choice(pool)
    if mutation == "delete-element":
        target.getparent().remove(target)
    elif mutation == "empty-text":
        target.text = ""
    elif mutation == "non-numeric-text":
        target.text = "abc"
    elif mutation == "negative-number":
        target.text = "-" + target.text.strip().lstrip("-")
    elif mutation == "zero-number":
        target.text = "0"
    elif mutation == "huge-number":
        target.text = "99999999999999999999"
    elif mutation == "duplicate-element":
        target.addnext(copy.deepcopy(target))
    elif mutation == "reorder-siblings":
        children = list(target)
        shuffled = children[:]
        rng.shuffle(shuffled)
        if shuffled == children:
            shuffled.reverse()
        for child in shuffled:
            target.append(child)  # appending an existing child moves it to the end
    elif mutation == "invalid-enum":
        names = [name for name in ENUM_ATTRIBUTES if name in target.attrib]
        if target.tag in ENUM_ELEMENTS and (not names or rng.random() < 0.5):
            target.text = "zzz"
        else:
            target.set(rng.choice(names), "zzz")


def mutate_archive(data: bytes, mutation: str, rng: random.Random) -> bytes:
    if mutation == "mxl-corrupt-zip":
        position = rng.randrange(len(data))
        flipped = bytes(255 - byte for byte in data[position : position + 16])
        return data[:position] + flipped + data[position + 16 :]
    with zipfile.ZipFile(io.BytesIO(data)) as source:
        entries = [(info, source.read(info.filename)) for info in source.infolist()]
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as target:
        for info, content in entries:
            if info.filename == "META-INF/container.xml":
                if mutation == "mxl-drop-container":
                    continue
                content = re.sub(rb'full-path="[^"]*"', b'full-path="missing.musicxml"', content, count=1)
            target.writestr(info, content)
    return buffer.getvalue()


def classify(record: Record) -> str:
    """The outcome class of a record: a crash or timeout first, then an exception, then the
    export's validity, then an unstable round trip; "ok" when nothing failed."""
    for stage in ("load", "analyses", "export", "export_xml", "export_xsd", "roundtrip"):
        if record.get(stage) in ("crash", "timeout"):
            return f"{record[stage]}:{stage}"
    for stage in ("load", "analyses", "export", "roundtrip"):
        value = record.get(stage, "n/a")
        if value not in ("ok", "stable", "unstable", "n/a"):
            return f"{stage}:{value}"
    if record.get("export_xml") == "ill-formed":
        return "export:ill-formed"
    if record.get("export_xsd") == "invalid":
        return "export:xsd-invalid"
    if record.get("export_errors"):
        return "export:semantic-errors"
    if record.get("roundtrip") == "unstable":
        return "roundtrip:unstable"
    return "ok"


def worth_minimising(outcome: str) -> bool:
    """Crashes, hangs, unexpected exceptions and ill-formed exports; not the expected rejections."""
    return outcome not in EXPECTED_OUTCOMES


def run_case(case: Case, timeout: float, tag: str = "") -> Record:
    WORK.mkdir(exist_ok=True)
    path = WORK / f"case-{case.seed}-{case.index}{tag}{case.suffix}"
    path.write_bytes(case.data)
    try:
        name = path.relative_to(corpus.REPO_ROOT).as_posix()
        return corpus.run_one(name, analyses=True, timeout=timeout)
    finally:
        path.unlink()


def run(seed: int, cases: int, minutes: Optional[float] = None, timeout: float = 30.0,
        workers: Optional[int] = None) -> List[Tuple[Case, Record, str]]:
    """Run cases 0..cases-1 of ``seed``; with ``minutes``, stop starting batches after that."""
    seeds = seed_files()
    deadline = None if minutes is None else time.monotonic() + minutes * 60
    count = workers or max(2, (os.cpu_count() or 2) // 2)
    results: List[Tuple[Case, Record, str]] = []
    index = 0
    with ThreadPoolExecutor(max_workers=count) as pool:
        while index < cases and (deadline is None or time.monotonic() < deadline):
            batch = [make_case(seed, i, seeds) for i in range(index, min(cases, index + count))]
            for case, record in zip(batch, pool.map(lambda c: run_case(c, timeout), batch)):
                results.append((case, record, classify(record)))
            index += len(batch)
    return results


def summary(results: List[Tuple[Case, Record, str]]) -> Dict[str, int]:
    return dict(sorted(Counter(outcome for _, _, outcome in results).items()))


def write_report(seed: int, results: List[Tuple[Case, Record, str]]) -> Path:
    WORK.mkdir(exist_ok=True)
    path = WORK / f"report-seed-{seed}.json"
    findings = [
        {"case": case.index, "source": case.source, "mutation": case.mutation, "outcome": outcome, "record": record}
        for case, record, outcome in results
        if outcome != "ok"
    ]
    report = {"seed": seed, "cases": len(results), "outcomes": summary(results), "findings": findings}
    path.write_text(json.dumps(report, indent=1) + "\n", encoding="utf-8")
    return path


def _attached(element: etree._Element, root: etree._Element) -> bool:
    while element is not None:
        if element is root:
            return True
        element = element.getparent()
    return False


def minimize(case: Case, outcome: str, timeout: float, budget: int = 200) -> bytes:
    """Remove elements, larger subtrees first, while the mutant keeps its outcome."""
    if case.mutation in UNMINIMISABLE:
        return case.data
    try:
        root = musicxml_check.parse_document(case.data)
    except etree.XMLSyntaxError:
        return case.data
    trials = 0
    progress = True
    while progress and trials < budget:
        progress = False
        candidates = sorted(
            (element for element in root.iter() if isinstance(element.tag, str) and element is not root),
            key=lambda element: -sum(1 for _ in element.iter()),
        )
        for element in candidates:
            if trials >= budget:
                break
            parent = element.getparent()
            if parent is None or not _attached(element, root):
                continue
            position = parent.index(element)
            parent.remove(element)
            trials += 1
            trial = Case(case.seed, case.index, case.source, case.mutation, _serialize(root), case.suffix)
            if classify(run_case(trial, timeout, tag=f"-min{trials}")) == outcome:
                progress = True
            else:
                parent.insert(position, element)
    return _serialize(root)


def save_regression(case: Case, outcome: str, data: bytes) -> Path:
    REGRESSIONS.mkdir(exist_ok=True)
    stem = re.sub(r"[^a-z0-9]+", "-", outcome.lower()).strip("-") + f"-seed{case.seed}-case{case.index}"
    path = REGRESSIONS / (stem + case.suffix)
    path.write_bytes(data)
    description = {"seed": case.seed, "case": case.index, "source": case.source, "mutation": case.mutation, "outcome": outcome}
    path.with_suffix(".json").write_text(json.dumps(description, indent=1) + "\n", encoding="utf-8")
    return path
```

- [ ] **Step 3: `scripts/make-fuzz.py`:**

```python
"""`make fuzz` and `make fuzz-minimize`: seeded mutation fuzzing of the MusicXML reader and writer.

Options go through FUZZ_ARGS, e.g. `make fuzz FUZZ_ARGS="--seed 7 --cases 1000"`. The run prints
how many cases ended in each outcome and writes every case that is not ok to
test/musicxml/fuzz-work/report-seed-<seed>.json; with --minimize, up to --per-outcome cases of
each failing outcome are minimised into test/musicxml/fuzz-regressions/. Needs maialib installed
(`make dev`).
"""

import argparse
import sys
from typing import Dict

from build_utils import REPO_ROOT
from terminal_colors import color

sys.path.insert(0, str(REPO_ROOT / "test" / "musicxml"))

import fuzz  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--cases", type=int, default=300)
    parser.add_argument("--minutes", type=float, default=None, help="stop starting cases after this")
    parser.add_argument("--timeout", type=float, default=30.0, help="seconds per case")
    parser.add_argument("--minimize", action="store_true")
    parser.add_argument("--per-outcome", type=int, default=2, help="cases minimised per outcome")
    arguments = parser.parse_args()

    results = fuzz.run(arguments.seed, arguments.cases, arguments.minutes, arguments.timeout)
    print(f"{color.OKGREEN}{len(results)} cases of seed {arguments.seed}:{color.ENDC}")
    for outcome, count in fuzz.summary(results).items():
        print(f"  {outcome}: {count}")
    print(f"Report: {fuzz.write_report(arguments.seed, results).relative_to(REPO_ROOT)}")
    if arguments.minimize:
        minimised: Dict[str, int] = {}
        for case, _, outcome in results:
            if not fuzz.worth_minimising(outcome) or minimised.get(outcome, 0) >= arguments.per_outcome:
                continue
            minimised[outcome] = minimised.get(outcome, 0) + 1
            path = fuzz.save_regression(case, outcome, fuzz.minimize(case, outcome, arguments.timeout))
            print(f"  {outcome}, case {case.index}: {path.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

Makefile: `.PHONY: fuzz`, `.PHONY: fuzz-minimize`, and after `corpus-fetch`:

```make
# Seeded mutation fuzzing of the MusicXML reader and writer; options through FUZZ_ARGS, e.g.
# make fuzz FUZZ_ARGS="--seed 7 --cases 1000".
fuzz:
	@$(PYTHON) $(SCRIPTS_DIR)/make-fuzz.py $(FUZZ_ARGS)

# The same, then the failing cases minimised into test/musicxml/fuzz-regressions/.
fuzz-minimize:
	@$(PYTHON) $(SCRIPTS_DIR)/make-fuzz.py --minimize $(FUZZ_ARGS)
```

Append to `test/musicxml/.gitignore`:

```
# Scratch files of `make fuzz`.
fuzz-work/
```

- [ ] **Step 4:** `python -m unittest test_musicxml_fuzz -v` → all pass.

- [ ] **Step 5: A reported run.** `make fuzz FUZZ_ARGS="--seed 1 --cases 300"` and `make fuzz-minimize FUZZ_ARGS="--seed 1 --cases 100 --per-outcome 1"`: put the outcome counts of both and the list of minimised files (with their sizes) in the report. Do **not** commit `fuzz-work/` or `fuzz-regressions/` — 4a only reports; 4c-1 turns findings into fixtures. Delete `test/musicxml/fuzz-regressions/` afterwards.

- [ ] **Step 6:** README: add rows `fuzz.py` (seeded mutation fuzzing) and `fuzz-work/`, `fuzz-regressions/` (scratch, ignored by git; minimised findings that later become fixtures), and the commands `make fuzz` / `make fuzz-minimize` with the `FUZZ_ARGS` example.

- [ ] **Step 7: Mutations:** seed `make_case`'s generator with `random.Random()` (no seed) → the reproducibility test fails; let `classify` check exceptions before crashes → the `crash:roundtrip` case fails; remove the `shuffled == children` fallback and use a generator for which the shuffle is the identity (or drop the reorder entirely) → the changes-the-document test fails for `reorder-siblings`; return `True` from `worth_minimising` → its test fails.

- [ ] **Step 8: Commit** `git add Makefile scripts/make-fuzz.py test/musicxml/fuzz.py test/musicxml/.gitignore test/musicxml/README.md test/test_musicxml_fuzz.py`, message `test: seeded mutation fuzzing of the MusicXML reader and writer`.

### Task 9: Clean round-trip output, documentation, final verification

**Files:**
- Modify: `tests-cpp/src/score-test.cpp` (`TEST(ScoreComplex, LoadAndExportRoundTrip)`, ~609-629), `BUILDING.md`, `CHANGELOG.md`

- [ ] **Step 1: The C++ round trip writes to the temporary directory.** Replace the test body with:

```cpp
TEST(ScoreComplex, LoadAndExportRoundTrip) {
    Score score1("./test/xml_examples/unit_test/test_chord.xml");
    EXPECT_TRUE(score1.isValid());

    int originalNotes = score1.getNumNotes();
    int originalMeasures = score1.getNumMeasures();
    int originalParts = score1.getNumParts();

    // toFile appends the .xml extension. The export goes to the temporary directory, not to the
    // working directory, which is the repository root when the tests run.
    const std::filesystem::path exportBase =
        std::filesystem::temp_directory_path() / "maialib_load_export_round_trip";
    const std::string exportedFile = exportBase.string() + ".xml";
    score1.toFile(exportBase.string(), false);

    Score score2(exportedFile);
    EXPECT_TRUE(score2.isValid());

    EXPECT_EQ(score2.getNumNotes(), originalNotes);
    EXPECT_EQ(score2.getNumMeasures(), originalMeasures);
    EXPECT_EQ(score2.getNumParts(), originalParts);

    std::filesystem::remove(exportedFile);
}
```

Delete the stray `test_roundtrip.xml` at the repository root (generated output, git-ignored). Run `make cpp-tests` → 1106/1106, and confirm no `test_roundtrip.xml` reappears in the root. Commit `git add tests-cpp/src/score-test.cpp`, message `test: the load/export round-trip test writes to the temporary directory`.

- [ ] **Step 2: BUILDING.md** — in the list of make targets, next to `make py-tests`/`make validate`, add:

```
make corpus                # Every MusicXML corpus file, slow ones and the fetched external corpus included, against its ledger
make corpus-update-ledger  # Write the current corpus results as the ledgers (review the diff)
make corpus-fetch          # Download the external MusicXML corpus (OpenScore, CC0) at pinned commits
make fuzz                  # Seeded mutation fuzzing of the MusicXML reader and writer (FUZZ_ARGS="--seed N --cases N")
make fuzz-minimize         # The same, then the failing cases minimised into test/musicxml/fuzz-regressions/
```

and one sentence: "`make py-tests` includes the corpus test, which needs `lxml` from `requirements-dev.txt`; `test/musicxml/README.md` describes the MusicXML test infrastructure."

- [ ] **Step 3: CHANGELOG.md** `[Unreleased]` → `### Build and tooling`, add:

```markdown
- MusicXML test infrastructure (`test/musicxml/`, described in its README): the official MusicXML 4.0 XSD and the W3C MusicXML test suite, vendored unmodified with their licences; offline validation against the schema plus the rules it cannot express; a strict ledger of how every corpus file loads, exports, validates and round-trips, checked by `make py-tests`; `make corpus`, `make corpus-update-ledger`, `make corpus-fetch` (OpenScore, CC0) and `make fuzz`/`make fuzz-minimize` (seeded mutation fuzzing). `lxml` joins `requirements-dev.txt` as a test-only dependency; the maialib package does not use it
- The C++ load/export round-trip test writes its file to the temporary directory, not to the repository root
```

Commit `git add BUILDING.md CHANGELOG.md`, message `docs: the MusicXML test infrastructure and its make targets`.

- [ ] **Step 4: Final verification** from the clean tree in a brand-new `py -3.12` venv (`pip install -r requirements-dev.txt`, `make dev`): `make cpp-tests` (1106/1106), `make py-tests` (499 + every new test, OK; report the duration), `make validate` (no new findings), `make corpus` (no difference), import from outside the repository, `make msvc-gate` (passes). `make linux-gate` is not run (WSL has no cmake). `git status` shows only ` M .gitignore`. Put every count and duration in the report.

---

## Self-review (done while writing)

- Spec coverage: §7.1 → Tasks 1, 2; §7.2 → Task 3; §7.3 → Task 4; §7.4 and §7.5 → Task 6; §7.6 → Task 8; §7.7 → Tasks 5, 7; §7.8 → Task 9; D4 (corpus, offline) → Tasks 1, 2, 6, 7; D5 (lxml test-only) → Task 3. The spec's `slow` ledger field is written by `updated_ledger` from the file size (10 MB or more) and read by the test.
- Interfaces are consistent across tasks: `check_file`/`check_bytes`/`Report.errors` (3, 4 → 6), `parse_document`/`read_mxl` (3 → 8), `corpus.run_one(name, analyses, timeout)` (6 → 8), `STAGE_ORDER`/`PENDING`/`NOT_APPLICABLE` (6 → 6, 8), `fixtures.MINIMAL_SCORE`/`CONTAINER` (3 → 4, 8).
