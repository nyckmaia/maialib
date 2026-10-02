"""The fuzz driver's mutations, case generation, outcome classes and report (no worker runs here)."""

import io
import json
import random
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest import mock

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

    def test_reordering_changes_the_order_whatever_the_shuffle_gives(self):
        # Twenty of these fifty generators shuffle the siblings they pick back into their order.
        for seed in range(50):
            with self.subTest(seed=seed):
                mutant = fuzz.mutate_xml(MINIMAL_SCORE, "reorder-siblings", random.Random(seed))
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
        self.assertEqual(
            (first.source, first.mutation, first.data),
            (second.source, second.mutation, second.data),
        )

    def test_another_seed_gives_other_cases(self):
        seeds = fuzz.seed_files()[:30]

        def cases(seed):
            return [
                (case.source, case.mutation, case.data)
                for case in (fuzz.make_case(seed, index, seeds) for index in range(10))
            ]

        self.assertNotEqual(cases(3), cases(4))

    def test_archive_mutations(self):
        rng = random.Random(1)
        data = archive()
        without = zipfile.ZipFile(io.BytesIO(fuzz.mutate_archive(data, "mxl-drop-container", rng)))
        self.assertNotIn("META-INF/container.xml", without.namelist())
        wrong = zipfile.ZipFile(io.BytesIO(fuzz.mutate_archive(data, "mxl-wrong-rootfile", rng)))
        self.assertIn(b'full-path="missing.musicxml"', wrong.read("META-INF/container.xml"))
        self.assertNotEqual(data, fuzz.mutate_archive(data, "mxl-corrupt-zip", rng))

    def test_byte_level_and_archive_mutants_are_not_minimised(self):
        # Even a mutant that parses is returned as it is, before any worker runs.
        with mock.patch.object(fuzz, "run_case", side_effect=AssertionError("a worker ran")):
            for mutation in fuzz.UNMINIMISABLE:
                for data in (b"<broken", MINIMAL_SCORE):
                    with self.subTest(mutation=mutation, data=data[:20]):
                        case = fuzz.Case(1, 1, "x", mutation, data, ".musicxml")
                        self.assertEqual(data, fuzz.minimize(case, "crash:load", timeout=1))

    def test_minimising_keeps_only_what_the_outcome_needs_in_its_order(self):
        def run_case(case, timeout, tag=""):
            # A stand-in for the worker: the load fails while the step precedes the octave.
            failing = b"<step>C</step><octave>4</octave>" in case.data
            return record(load="IndexError" if failing else "ok")

        case = fuzz.Case(1, 1, "x", "delete-element", MINIMAL_SCORE, ".musicxml")
        with mock.patch.object(fuzz, "run_case", side_effect=run_case):
            minimal = fuzz.minimize(case, "load:IndexError", timeout=1)
        self.assertEqual(
            b'<score-partwise version="4.0"><part id="P1"><measure number="1"><note><pitch>'
            b"<step>C</step><octave>4</octave></pitch></note></measure></part></score-partwise>",
            canonical(minimal),
        )


def record(**fields):
    base = {
        "input": "valid",
        "load": "ok",
        "analyses": "ok",
        "export": "ok",
        "export_xml": "well-formed",
        "export_xsd": "valid",
        "export_errors": [],
        "roundtrip": "stable",
    }
    base.update(fields)
    return base


# The stages after one that ended the worker.
AFTER_INPUT = {
    "load": "n/a",
    "analyses": "n/a",
    "export": "n/a",
    "export_xml": "n/a",
    "export_xsd": "n/a",
    "roundtrip": "n/a",
}
AFTER_LOAD = {key: value for key, value in AFTER_INPUT.items() if key != "load"}


def exited(code):
    """Diagnostics of a worker that ended with ``code`` (None: stopped at the timeout)."""
    return {"exit_code": code, "stderr_tail": ""}


class ClassifyTestCase(unittest.TestCase):
    def test_outcome_classes(self):
        cases = [
            (record(), "ok"),
            (record(load="crash", **AFTER_LOAD), "crash:load"),
            (record(analyses="timeout"), "timeout:analyses"),
            (record(load="IndexError", **AFTER_LOAD), "load:IndexError"),
            (
                record(
                    export="UnicodeDecodeError", export_xml="n/a", export_xsd="n/a", roundtrip="n/a"
                ),
                "export:UnicodeDecodeError",
            ),
            (record(export_xml="ill-formed", export_xsd="n/a"), "export:ill-formed"),
            (record(export_xsd="invalid"), "export:xsd-invalid"),
            (record(export_errors=["unpaired-tie"]), "export:semantic-errors"),
            (record(roundtrip="unstable"), "roundtrip:unstable"),
            (record(export_xsd="invalid", roundtrip="crash"), "crash:roundtrip"),
            # The input stage comes first.
            (record(input="crash", **AFTER_INPUT), "crash:input"),
            (record(input="timeout", **AFTER_INPUT), "timeout:input"),
            (record(input="ValueError"), "input:ValueError"),
            (record(input="ValueError", load="IndexError", **AFTER_LOAD), "input:ValueError"),
            (record(input="unreadable", load="RuntimeError", **AFTER_LOAD), "load:RuntimeError"),
            (record(input="invalid", export_xsd="invalid"), "export:xsd-invalid"),
            # A crash or hang anywhere comes before any exception.
            (record(input="ValueError", load="crash", **AFTER_LOAD), "crash:load"),
            (record(export_xml="crash", export_xsd="n/a", roundtrip="n/a"), "crash:export_xml"),
            # A worker that ended badly after its final record.
            (record(diagnostics=exited(0)), "ok"),
            (record(diagnostics=exited(3)), "crash:exit"),
            (record(diagnostics=exited(None)), "timeout:exit"),
            (record(load="IndexError", **AFTER_LOAD, diagnostics=exited(3)), "crash:exit"),
            (record(load="crash", **AFTER_LOAD, diagnostics=exited(3221225477)), "crash:load"),
        ]
        for fields, expected in cases:
            with self.subTest(expected=expected, fields=fields):
                self.assertEqual(expected, fuzz.classify(fields))

    def test_expected_rejections_are_not_minimised(self):
        for outcome in (
            "ok",
            "load:RuntimeError",
            "export:xsd-invalid",
            "export:semantic-errors",
            "roundtrip:unstable",
        ):
            self.assertFalse(fuzz.worth_minimising(outcome))
        for outcome in (
            "crash:load",
            "timeout:load",
            "load:IndexError",
            "export:ill-formed",
            "crash:exit",
            "input:ValueError",
        ):
            self.assertTrue(fuzz.worth_minimising(outcome))


class RunTestCase(unittest.TestCase):
    def test_a_case_record_carries_the_worker_diagnostics(self):
        def run_one(name, analyses=False, timeout=None, diagnostics=None):
            self.assertEqual(b"<score-partwise/>", Path(name).read_bytes())
            diagnostics.update(exited(3))
            return record()

        case = fuzz.Case(1, 2, "x", "truncate", b"<score-partwise/>", ".musicxml")
        with tempfile.TemporaryDirectory() as folder, mock.patch.object(
            fuzz, "WORK", Path(folder)
        ), mock.patch.object(fuzz.corpus, "run_one", side_effect=run_one):
            result = fuzz.run_case(case, timeout=1)
            self.assertEqual([], list(Path(folder).iterdir()))
        self.assertEqual(record(diagnostics=exited(3)), result)
        self.assertEqual("crash:exit", fuzz.classify(result))

    def test_the_report_holds_every_case_that_is_not_ok_with_its_record(self):
        crashed = record(diagnostics={"exit_code": 3, "stderr_tail": "the end"})
        results = [
            (fuzz.Case(5, 0, "a.xml", "truncate", b"", ".musicxml"), record(), "ok"),
            (fuzz.Case(5, 1, "b.xml", "utf16", b"", ".musicxml"), crashed, "crash:exit"),
        ]
        with tempfile.TemporaryDirectory() as folder, mock.patch.object(fuzz, "WORK", Path(folder)):
            report = json.loads(fuzz.write_report(5, results).read_text(encoding="utf-8"))
        self.assertEqual(
            {
                "seed": 5,
                "cases": 2,
                "outcomes": {"crash:exit": 1, "ok": 1},
                "findings": [
                    {
                        "case": 1,
                        "source": "b.xml",
                        "mutation": "utf16",
                        "outcome": "crash:exit",
                        "record": crashed,
                    }
                ],
            },
            report,
        )


if __name__ == "__main__":
    unittest.main()
