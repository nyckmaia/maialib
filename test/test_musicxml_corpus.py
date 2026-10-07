"""Every file of the MusicXML corpus behaves as test/musicxml/ledger.json records.

The slow files (marked in the ledger) run only under `make corpus`.
"""

import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import types
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import corpus  # noqa: E402
import corpus_worker  # noqa: E402
from fixtures import MINIMAL_SCORE  # noqa: E402

FINISHED = {
    "input": "valid",
    "load": "ok",
    "export": "ok",
    "export_xml": "well-formed",
    "export_xsd": "invalid",
    "export_errors": [],
    "roundtrip": "stable",
}

NOTHING_FINISHED = {
    "input": "n/a",
    "load": "n/a",
    "export": "n/a",
    "export_xml": "n/a",
    "export_xsd": "n/a",
    "export_errors": [],
    "roundtrip": "n/a",
}

# Corpus files under and over the size from which a file is slow.
SMALL_FILE = "test/xml_examples/unit_test/test_chord.xml"
SLOW_FILE = "test/xml_examples/Beethoven/big_files/Symphony_9th.xml"

# Small corpus files that load, one uncompressed and one compressed.
LOADABLE_FILES = (
    "test/musicxml/w3c-test-suite/xmlFiles/45a-SimpleRepeat.musicxml",
    "test/xml_examples/unit_test/test_compressed_file.mxl",
)

# MINIMAL_SCORE with an ampersand in its title, which maialib writes unescaped: the score is valid
# and loads, but its export is not well-formed XML.
AMPERSAND_SCORE = MINIMAL_SCORE.replace(
    b'<score-partwise version="4.0">',
    b'<score-partwise version="4.0"><work><work-title>A &amp; B</work-title></work>',
)

# A stand-in maialib whose scores load and export MINIMAL_SCORE, so that every stage succeeds; at
# exit, after the worker's final record, it writes 3,000 characters and a byte that is not UTF-8
# to stderr and ends the process with status 3, as a crash in a destructor would.
DIES_AFTER_THE_FINAL_RECORD = f"""
import atexit
import os
import sys

EXPORT = {MINIMAL_SCORE.decode()!r}


class Score:
    def __init__(self, path):
        pass

    def toXML(self):
        return EXPORT


def die():
    sys.stderr.buffer.write(b"a" * 3000 + b"\\xff the end")
    sys.stderr.buffer.flush()
    os._exit(3)


atexit.register(die)
"""

# A stand-in maialib that replaces the checks of the export, which the worker runs after
# importing maialib, by an exit with status 3.
DIES_WHILE_CHECKING_THE_EXPORT = """
import os

import musicxml_check

musicxml_check.check_bytes = lambda data: os._exit(3)


class Score:
    def __init__(self, path):
        pass

    def toXML(self):
        return "<score-partwise/>"
"""

# A stand-in for corpus_worker.py: it leaves a file in its temporary directory and names the file
# on stderr, then hangs when the file to examine is called "hang" and exits with status 3
# otherwise.
LEAVES_A_TEMPORARY_FILE = """
import os
import sys
import tempfile
import time

handle, path = tempfile.mkstemp()
os.close(handle)
print(path, file=sys.stderr, flush=True)
if os.path.basename(sys.argv[1]) == "hang":
    time.sleep(60)
sys.exit(3)
"""


def records_in(output):
    """The records a worker printed, in order."""
    prefix = corpus_worker.PREFIX
    return [
        json.loads(line[len(prefix) :]) for line in output.splitlines() if line.startswith(prefix)
    ]


def temporary_variables(folder):
    """The environment variables that point a process's temporary directory at `folder`."""
    return dict.fromkeys(("TMPDIR", "TEMP", "TMP"), str(folder))


def entries_a_bare_process_leaves():
    """The names a Python process that runs no code leaves in a fresh temporary directory.

    Software outside the test, such as an antivirus that hooks process creation, can add entries
    to the temporary directory of every new process. A test of what a worker leaves there discounts
    these names, measured here instead of listed, so that it judges only the worker's own entries.
    """
    with tempfile.TemporaryDirectory() as folder:
        subprocess.run(
            [sys.executable, "-c", "pass"],
            capture_output=True,
            timeout=60,
            check=True,
            env={**os.environ, **temporary_variables(folder)},
        )
        return set(os.listdir(folder))


def pending_stages(record):
    return sorted(key for key, value in record.items() if value == corpus_worker.PENDING)


@contextlib.contextmanager
def stand_in_maialib(source):
    """Inside the with block, the workers import a maialib package holding ``source``: it comes
    first on PYTHONPATH."""
    with tempfile.TemporaryDirectory() as folder:
        package = Path(folder) / "maialib"
        package.mkdir()
        (package / "__init__.py").write_text(source, encoding="utf-8")
        with mock.patch.dict(os.environ, {"PYTHONPATH": folder}):
            yield


class LedgerLogicTestCase(unittest.TestCase):
    def test_an_unfinished_stage_takes_the_outcome_and_later_stages_are_not_applicable(self):
        record = corpus_worker.new_record()
        record.update(input="valid", load="ok")
        self.assertEqual(
            {
                **record,
                "export": "crash",
                "export_xml": "n/a",
                "export_xsd": "n/a",
                "roundtrip": "n/a",
            },
            corpus.ended(dict(record), "crash"),
        )

    def test_a_finished_record_is_left_as_it_is(self):
        self.assertEqual(FINISHED, corpus.ended(dict(FINISHED), "crash"))

    def test_a_changed_field_is_a_difference(self):
        problems = corpus.compare(
            {"a.xml": FINISHED}, {"a.xml": {**FINISHED, "load": "IndexError"}}
        )
        self.assertEqual(1, len(problems))
        self.assertIn("load", problems[0])

    def test_a_changed_list_of_export_errors_is_a_difference(self):
        problems = corpus.compare(
            {"a.xml": FINISHED}, {"a.xml": {**FINISHED, "export_errors": ["unpaired-tie"]}}
        )
        self.assertEqual(1, len(problems))
        self.assertIn("export_errors", problems[0])

    def test_alternatives_accept_any_listed_value_and_slow_and_note_are_ignored(self):
        entry = {**FINISHED, "load": {"any_of": ["ok", "crash"]}, "slow": True, "note": "why"}
        self.assertEqual([], corpus.compare({"a.xml": entry}, {"a.xml": FINISHED}))

    def test_a_file_missing_from_the_ledger_is_a_difference(self):
        self.assertEqual(["b.xml: not in the ledger"], corpus.compare({}, {"b.xml": FINISHED}))

    def test_an_update_keeps_alternatives_that_still_hold_and_marks_slow_files(self):
        old = {
            SMALL_FILE: {
                **FINISHED,
                "load": {"any_of": ["ok", "crash"]},
                "export": {"any_of": ["ok", "timeout"]},
                "note": "why",
            }
        }
        actual = {SMALL_FILE: {**FINISHED, "export": "IndexError"}, SLOW_FILE: FINISHED}
        self.assertEqual(
            {
                SMALL_FILE: {
                    **FINISHED,
                    "load": {"any_of": ["ok", "crash"]},
                    "export": "IndexError",
                    "note": "why",
                },
                SLOW_FILE: {**FINISHED, "slow": True},
            },
            corpus.updated_ledger(old, actual),
        )

    def test_an_update_drops_the_note_when_no_alternative_still_holds(self):
        old = {SMALL_FILE: {**FINISHED, "load": {"any_of": ["ok", "crash"]}, "note": "why"}}
        actual = {SMALL_FILE: {**FINISHED, "load": "IndexError"}}
        self.assertEqual(
            {SMALL_FILE: {**FINISHED, "load": "IndexError"}}, corpus.updated_ledger(old, actual)
        )


class WorkerStagesTestCase(unittest.TestCase):
    def test_each_stage_is_followed_by_a_record_and_an_ill_formed_export_has_no_schema_result(
        self,
    ):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "score.musicxml"
            path.write_bytes(AMPERSAND_SCORE)
            done = subprocess.run(
                [sys.executable, str(corpus.WORKER), str(path)], capture_output=True, timeout=120
            )
        records = records_in(done.stdout.decode("utf-8"))
        # A crash or a hang is charged to the first stage still pending in the last record.
        self.assertEqual(
            [
                ["export", "export_xml", "export_xsd", "load", "roundtrip"],
                ["export", "export_xml", "export_xsd", "roundtrip"],
                ["export_xml", "export_xsd", "roundtrip"],
                ["roundtrip"],
                [],
            ],
            [pending_stages(record) for record in records],
        )
        before_the_round_trip = records[-2]
        self.assertEqual("ill-formed", before_the_round_trip["export_xml"])
        self.assertEqual("n/a", before_the_round_trip["export_xsd"])

    def test_an_exception_in_the_validator_is_the_input_result_and_maialib_still_runs(self):
        output = io.StringIO()
        with mock.patch.object(
            corpus_worker.musicxml_check, "check_file", side_effect=ValueError("forced")
        ), contextlib.redirect_stdout(output):
            corpus_worker.examine(corpus.REPO_ROOT / SMALL_FILE, analyses=False)
        final = records_in(output.getvalue())[-1]
        self.assertEqual(("ValueError", "ok"), (final["input"], final["load"]))

    def test_the_round_trip_reads_exactly_the_utf8_bytes_of_the_export(self):
        exported = "<a>\u00e9</a>\n<b/>\n"
        read = []

        class Score:
            def __init__(self, path):
                read.append(Path(path).read_bytes())

            def toXML(self):
                return exported

        maialib = types.SimpleNamespace(Score=Score)
        self.assertEqual("stable", corpus_worker.roundtrip_status(maialib, exported))
        self.assertEqual([exported.encode("utf-8")], read)

    def test_the_round_trip_comparison_leaves_out_only_the_encoding_date(self):
        dated = (
            "<identification>\n"
            "    <encoding>\n"
            "      <encoding-date>2026-10-02</encoding-date>\n"
            "      <software>Maialib 1.0.0</software>\n"
            "      <encoding-description>Maialib / MusicXML 3.0</encoding-description>\n"
            "    </encoding>\n"
            "  </identification>\n"
        )
        undated = dated.replace("<encoding-date>2026-10-02</encoding-date>", "")
        self.assertEqual(undated, corpus_worker.without_encoding_date(dated))
        self.assertEqual(undated, corpus_worker.without_encoding_date(undated))


class WorkerProcessTestCase(unittest.TestCase):
    def test_a_worker_out_of_time_is_a_timeout_at_its_first_unfinished_stage(self):
        # Ten milliseconds end the worker before its first record: the schema alone takes longer.
        self.assertEqual(
            {**NOTHING_FINISHED, "input": "timeout"}, corpus.run_one(SMALL_FILE, timeout=0.01)
        )

    def test_a_worker_that_dies_is_a_crash_at_its_first_unfinished_stage(self):
        # The stand-in ends the process when the worker imports it after the input stage, as a
        # crash inside maialib would.
        with stand_in_maialib("import os\n\nos._exit(3)\n"):
            record = corpus.run_one(SMALL_FILE, timeout=60)
        self.assertEqual({**NOTHING_FINISHED, "input": "valid", "load": "crash"}, record)

    def test_a_worker_that_dies_while_checking_the_export_is_a_crash_there(self):
        # The record that says the export succeeded comes before the export's checks.
        with stand_in_maialib(DIES_WHILE_CHECKING_THE_EXPORT):
            record = corpus.run_one(SMALL_FILE, timeout=60)
        self.assertEqual(
            {
                **NOTHING_FINISHED,
                "input": "valid",
                "load": "ok",
                "export": "ok",
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
            {"exit_code": 3, "stderr_tail": "a" * 1991 + "\ufffd the end"}, diagnostics
        )

    def test_the_stderr_tail_keeps_every_character_the_worker_wrote(self):
        # On Windows a pipe would otherwise take the ANSI code page: the parent would get a byte
        # that is not UTF-8 for the first character and an escape sequence for the second.
        diagnostics = {}
        source = 'import sys\n\nprint("partitura \\u00e9 \\u4e00", file=sys.stderr)\n'
        with stand_in_maialib(source):
            corpus.run_one(SMALL_FILE, timeout=60, diagnostics=diagnostics)
        self.assertIn("partitura \u00e9 \u4e00", diagnostics["stderr_tail"])

    def test_a_worker_keeps_its_temporary_files_in_a_folder_removed_when_it_ends(self):
        with tempfile.TemporaryDirectory() as folder:
            worker = Path(folder) / "worker.py"
            worker.write_text(LEAVES_A_TEMPORARY_FILE, encoding="utf-8")
            temporary = Path(folder) / "temporary"
            temporary.mkdir()
            for name, timeout in (("crash", 60), ("hang", 3)):
                with self.subTest(name=name), mock.patch.object(
                    corpus, "WORKER", worker
                ), mock.patch.object(tempfile, "tempdir", str(temporary)):
                    diagnostics = {}
                    corpus.run_one(name, timeout=timeout, diagnostics=diagnostics)
                    left = Path(diagnostics["stderr_tail"].strip())
                    # The worker's file was in a folder of its own, which is gone, file and all.
                    self.assertEqual(temporary, left.parent.parent)
                    self.assertEqual([], os.listdir(str(temporary)))

    def test_a_warning_the_windows_code_page_cannot_encode_leaves_the_worker_running(self):
        # maialib warns about an <alter> it cannot spell and quotes it; this one is a CJK
        # character, which the ANSI code page of a pipe on Windows cannot encode.
        step = b"<step>C</step>"
        score = MINIMAL_SCORE.replace(step, step + "<alter>\u4e00</alter>".encode())
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "score.musicxml"
            path.write_bytes(score)
            record = corpus.run_one(str(path), timeout=60)
        self.assertEqual("ok", record["load"])

    def test_a_file_whose_path_is_not_ascii_has_the_record_of_its_content(self):
        # maialib opens such a path itself on every platform, a name outside the Windows ANSI
        # code page included; the worker loads the file from its own path.
        ledger = corpus.load_ledger(corpus.LEDGER)
        environmental = entries_a_bare_process_leaves()
        for name in LOADABLE_FILES:
            with self.subTest(name=name), tempfile.TemporaryDirectory() as folder:
                path = Path(folder) / ("partitura_\u00e9_\u65e5\u672c" + Path(name).suffix)
                shutil.copyfile(str(corpus.REPO_ROOT / name), str(path))
                temporary = Path(folder) / "temporary"
                temporary.mkdir()
                done = subprocess.run(
                    [sys.executable, str(corpus.WORKER), str(path)],
                    capture_output=True,
                    timeout=120,
                    env={**os.environ, **temporary_variables(temporary)},
                )
                record = records_in(done.stdout.decode("utf-8"))[-1]
                self.assertEqual("ok", record["load"])
                self.assertEqual([], corpus.compare({name: ledger[name]}, {name: record}))
                # The worker ends without an error and leaves none of its own entries in its
                # temporary directory; entries that any new process gets there from its
                # environment are not the worker's.
                self.assertEqual(0, done.returncode, done.stderr.decode("utf-8", "replace"))
                left = sorted(set(os.listdir(str(temporary))) - environmental)
                self.assertEqual([], left)


class CorpusLedgerTestCase(unittest.TestCase):
    def test_the_ledger_lists_every_corpus_file(self):
        self.assertEqual(corpus.corpus_files(), sorted(corpus.load_ledger(corpus.LEDGER)))

    def test_the_ledger_marks_as_slow_exactly_the_files_of_10_mb_or_more(self):
        # The slow flag decides which files `make py-tests` skips.
        ledger = corpus.load_ledger(corpus.LEDGER)
        self.assertEqual(
            [name for name in corpus.corpus_files() if corpus.is_slow(name)],
            sorted(name for name, entry in ledger.items() if entry.get("slow")),
        )

    def test_every_corpus_file_matches_its_ledger_entry(self):
        ledger = corpus.load_ledger(corpus.LEDGER)
        files = [name for name in corpus.corpus_files() if not ledger.get(name, {}).get("slow")]
        problems = corpus.compare(ledger, corpus.run_corpus(files))
        self.assertEqual(
            [], problems, "\n" + "\n".join(problems) + "\nReview, then `make corpus-update-ledger`."
        )


if __name__ == "__main__":
    unittest.main()
