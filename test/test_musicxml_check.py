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
                zipfile.ZipInfo("mimetype"),
                musicxml_check.MIMETYPE,
                compress_type=zipfile.ZIP_STORED,
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
