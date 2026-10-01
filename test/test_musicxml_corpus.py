"""Every file of the MusicXML corpus behaves as test/musicxml/ledger.json records.

The slow files (marked in the ledger) run only under `make corpus`.
"""

import contextlib
import io
import json
import os
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

# MINIMAL_SCORE with an ampersand in its title, which maialib writes unescaped: the score is valid
# and loads, but its export is not well-formed XML.
AMPERSAND_SCORE = MINIMAL_SCORE.replace(
    b'<score-partwise version="4.0">',
    b'<score-partwise version="4.0"><work><work-title>A &amp; B</work-title></work>',
)


def records_in(output):
    """The records a worker printed, in order."""
    prefix = corpus_worker.PREFIX
    return [
        json.loads(line[len(prefix) :]) for line in output.splitlines() if line.startswith(prefix)
    ]


def pending_stages(record):
    return sorted(key for key, value in record.items() if value == corpus_worker.PENDING)


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


class WorkerProcessTestCase(unittest.TestCase):
    def test_a_worker_out_of_time_is_a_timeout_at_its_first_unfinished_stage(self):
        # Ten milliseconds end the worker before its first record: the schema alone takes longer.
        self.assertEqual(
            {**NOTHING_FINISHED, "input": "timeout"}, corpus.run_one(SMALL_FILE, timeout=0.01)
        )

    def test_a_worker_that_dies_is_a_crash_at_its_first_unfinished_stage(self):
        # A stand-in for maialib, found first on PYTHONPATH, ends the process when the worker
        # imports it after the input stage, as a crash inside maialib would.
        with tempfile.TemporaryDirectory() as folder:
            package = Path(folder) / "maialib"
            package.mkdir()
            (package / "__init__.py").write_text("import os\n\nos._exit(3)\n", encoding="utf-8")
            with mock.patch.dict(os.environ, {"PYTHONPATH": folder}):
                record = corpus.run_one(SMALL_FILE, timeout=60)
        self.assertEqual({**NOTHING_FINISHED, "input": "valid", "load": "crash"}, record)

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
