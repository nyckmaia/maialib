"""The MusicXML validator: the 4.0 schema offline, .mxl archives, and the semantic rules."""

import io
import struct
import subprocess
import sys
import tempfile
import unittest
import zipfile
from fractions import Fraction
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "musicxml"))

import musicxml_check  # noqa: E402
from fixtures import CONTAINER, MINIMAL_SCORE  # noqa: E402

try:
    import lzma
except ImportError:  # CPython can be built without it
    lzma = None

MUSICXML = Path(__file__).resolve().parent / "musicxml"
SUITE = MUSICXML / "w3c-test-suite"

# A Python built without the lzma and compression.zstd modules, as CPython can be, checking a
# conforming archive: it prints whether the archive is valid.
WITHOUT_OPTIONAL_DECOMPRESSORS = """
import sys

sys.modules["lzma"] = sys.modules["compression.zstd"] = None

import io
import zipfile

import musicxml_check
from fixtures import CONTAINER, MINIMAL_SCORE

buffer = io.BytesIO()
with zipfile.ZipFile(buffer, "w") as zipped:
    zipped.writestr("META-INF/container.xml", CONTAINER)
    zipped.writestr("score.musicxml", MINIMAL_SCORE)
print(musicxml_check.check_mxl_bytes(buffer.getvalue()).xsd_valid)
"""


def suite_files() -> list:
    """The MusicXML files of the W3C suite: every .musicxml file, the .invalid ones included, and
    the .mxl archive."""
    return [
        path
        for path in sorted(SUITE.rglob("*"))
        if path.is_file() and (".musicxml" in path.name or path.suffix == ".mxl")
    ]


# A zip extra field with no data, under the header id 0xCAFE that Java's jar tool writes.
JAR_MARKER = b"\xfe\xca\x00\x00"


def archive(
    score=MINIMAL_SCORE, container=CONTAINER, mimetype="stored", content=musicxml_check.MIMETYPE
) -> bytes:
    """An .mxl archive; ``mimetype`` is "stored" (first, uncompressed), "deflated", "extra" (first,
    stored, with a zip extra field), "second" (stored, after a stored container) or None, and
    ``content`` is what the mimetype entry holds."""
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as zipped:
        if mimetype == "second":
            # Stored, as the mimetype entry should be, so that only the order is wrong.
            zipped.writestr(
                zipfile.ZipInfo("META-INF/container.xml"),
                container,
                compress_type=zipfile.ZIP_STORED,
            )
        if mimetype in ("stored", "second", "extra"):
            entry = zipfile.ZipInfo("mimetype")
            if mimetype == "extra":
                entry.extra = JAR_MARKER
            zipped.writestr(entry, content, compress_type=zipfile.ZIP_STORED)
        elif mimetype == "deflated":
            zipped.writestr("mimetype", content)
        if container is not None and mimetype != "second":
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
        files = suite_files()
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

    def test_a_mimetype_after_another_entry_is_a_warning(self):
        report = musicxml_check.check_mxl_bytes(archive(mimetype="second"))
        self.assertTrue(report.xsd_valid, report.problem)
        self.assertEqual(["mimetype-not-first-stored"], report.warnings)

    def test_a_mimetype_with_an_extra_field_is_a_warning(self):
        report = musicxml_check.check_mxl_bytes(archive(mimetype="extra"))
        self.assertTrue(report.xsd_valid, report.problem)
        self.assertEqual(["mimetype-not-first-stored"], report.warnings)

    def test_mimetype_content_other_than_the_media_type_is_a_warning(self):
        content = b"application/vnd.recordare.musicxml+xml"
        report = musicxml_check.check_mxl_bytes(archive(content=content))
        self.assertTrue(report.xsd_valid, report.problem)
        self.assertEqual(["mimetype-not-first-stored"], report.warnings)

    def test_an_archive_without_container_is_unreadable(self):
        report = musicxml_check.check_mxl_bytes(archive(container=None))
        self.assertFalse(report.readable)
        self.assertEqual(["container-missing"], report.errors)

    def test_an_ill_formed_container_is_unreadable(self):
        report = musicxml_check.check_mxl_bytes(archive(container=b"<container><rootfiles>"))
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

    def test_an_entry_name_flagged_utf8_that_is_not_utf8_is_unreadable(self):
        # zipfile flags a non-ASCII entry name as UTF-8; the name's bytes then stop being UTF-8.
        name = "partitura-éèê.txt"
        buffer = io.BytesIO(archive())
        with zipfile.ZipFile(buffer, "a") as zipped:
            zipped.writestr(name, b"")
        data = buffer.getvalue().replace(name.encode(), b"partitura-" + b"\xff" * 6 + b".txt")
        self.assertFalse(musicxml_check.check_mxl_bytes(data).readable)

    @unittest.skipUnless(lzma, "this Python was built without the lzma module")
    def test_a_damaged_lzma_entry_is_unreadable(self):
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w") as zipped:
            zipped.writestr("META-INF/container.xml", CONTAINER)
            zipped.writestr("score.musicxml", MINIMAL_SCORE, compress_type=zipfile.ZIP_LZMA)
        data = bytearray(buffer.getvalue())
        entry = zipfile.ZipFile(io.BytesIO(bytes(data))).getinfo("score.musicxml")
        name_length, extra_length = struct.unpack_from("<HH", data, entry.header_offset + 26)
        # The entry's data opens with zipfile's 4-byte LZMA header and then the 5 property bytes,
        # whose first (lc, lp and pb together) cannot be 0xFF.
        data[entry.header_offset + 30 + name_length + extra_length + 4] = 0xFF
        self.assertFalse(musicxml_check.check_mxl_bytes(bytes(data)).readable)

    def test_an_entry_flagged_zstandard_whose_data_is_not_zstandard_is_unreadable(self):
        # Python 3.14 decompresses Zstandard (method 93) and raises its own error on bad data;
        # earlier versions refuse the method.
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w") as zipped:
            zipped.writestr("META-INF/container.xml", CONTAINER)
            zipped.writestr("score.musicxml", MINIMAL_SCORE)
        data = bytearray(buffer.getvalue())
        entry = zipfile.ZipFile(io.BytesIO(bytes(data))).getinfo("score.musicxml")
        # The compression method: at offset 8 of the entry's local header and at offset 10 of its
        # central directory header, the last one in the archive.
        struct.pack_into("<H", data, entry.header_offset + 8, 93)
        struct.pack_into("<H", data, data.rindex(b"PK\x01\x02") + 10, 93)
        self.assertFalse(musicxml_check.check_mxl_bytes(bytes(data)).readable)

    def test_the_validator_reads_archives_without_the_optional_decompressors(self):
        done = subprocess.run(
            [sys.executable, "-c", WITHOUT_OPTIONAL_DECOMPRESSORS],
            cwd=str(MUSICXML),
            capture_output=True,
            timeout=120,
        )
        self.assertEqual(b"True", done.stdout.strip(), done.stderr.decode("utf-8", "replace"))


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


# Five quarter notes of content in the 4/4 measure, then a <backup> to its start.
OVERFULL = note(4, b"whole") + note(1, b"quarter") + b"<backup><duration>5</duration></backup>"


def errors(data: bytes) -> list:
    return musicxml_check.check_bytes(data).errors


def warnings(data: bytes) -> list:
    return musicxml_check.check_bytes(data).warnings


class SemanticChecksTestCase(unittest.TestCase):
    def test_the_minimal_score_breaks_no_rule(self):
        self.assertEqual([], errors(MINIMAL_SCORE))
        self.assertEqual([], warnings(MINIMAL_SCORE))

    def test_a_chord_in_a_full_measure_breaks_no_rule(self):
        chord_tone = (
            b"<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
            b"<duration>4</duration><type>whole</type></note>"
        )
        report = musicxml_check.check_bytes(with_notes(WHOLE_NOTE, chord_tone))
        self.assertEqual([], report.findings)

    def test_a_note_on_the_second_of_two_staves_breaks_no_rule(self):
        data = MINIMAL_SCORE.replace(b"</time>", b"</time><staves>2</staves>").replace(
            b"<type>whole</type>", b"<type>whole</type><staff>2</staff>"
        )
        self.assertEqual([], musicxml_check.check_bytes(data).findings)

    def test_backup_and_forward_durations_are_read_in_divisions(self):
        # At 2 divisions per quarter note, a <backup> and a <forward> of 8 span the 4/4 measure
        # exactly; read as 8 quarter notes, they would leave it.
        data = with_notes(
            note(8, b"whole"),
            b"<backup><duration>8</duration></backup><forward><duration>8</duration></forward>",
        ).replace(b"<divisions>1</divisions>", b"<divisions>2</divisions>")
        self.assertEqual([], musicxml_check.check_bytes(data).findings)

    def test_a_duration_before_any_divisions_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"<divisions>1</divisions>", b"")
        self.assertIn("duration-before-divisions", errors(data))

    def test_durations_without_divisions_are_read_with_one_division_per_quarter(self):
        # The default of W3C test 03e: two halves of 2 fill the 4/4 measure, and the missing
        # <divisions> is reported once although both durations precede it.
        data = with_notes(note(2, b"half"), note(2, b"half"))
        report = musicxml_check.check_bytes(data.replace(b"<divisions>1</divisions>", b""))
        self.assertEqual(
            ["duration-before-divisions"], [finding.check for finding in report.findings]
        )

    def test_a_divisions_change_inside_a_full_measure_is_no_length_mismatch(self):
        # Two quarter notes at 1 division per quarter, then two at 2 (W3C test 03c).
        data = with_notes(
            note(1, b"quarter"),
            note(1, b"quarter"),
            b"<attributes><divisions>2</divisions></attributes>",
            note(2, b"quarter"),
            note(2, b"quarter"),
        )
        self.assertEqual([], errors(data))
        self.assertEqual([], warnings(data))

    def test_a_chord_note_without_a_preceding_note_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"<note><pitch>", b"<note><chord/><pitch>")
        self.assertIn("chord-without-anchor", errors(data))

    def test_a_staff_above_the_number_of_staves_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"<type>whole</type>", b"<type>whole</type><staff>2</staff>")
        self.assertIn("staff-above-staves", errors(data))

    def test_a_backup_before_the_measure_start_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"</note>", b"</note><backup><duration>8</duration></backup>")
        self.assertIn("position-negative", errors(data))

    def test_a_backup_one_quarter_before_the_measure_start_is_an_error(self):
        data = MINIMAL_SCORE.replace(b"</note>", b"</note><backup><duration>5</duration></backup>")
        self.assertIn("position-negative", errors(data))

    def test_a_forward_past_the_measure_end_is_an_error(self):
        data = MINIMAL_SCORE.replace(
            b"</note>", b"</note><forward><duration>4</duration></forward>"
        )
        self.assertIn("position-past-measure-end", errors(data))

    def test_a_forward_to_the_measure_end_is_allowed(self):
        voice_two = b"</note><backup><duration>4</duration></backup><forward><duration>4</duration></forward>"
        self.assertEqual([], errors(MINIMAL_SCORE.replace(b"</note>", voice_two)))

    def test_a_forward_within_the_notes_of_an_overfull_measure_is_allowed(self):
        data = with_notes(OVERFULL, b"<forward><duration>5</duration></forward>")
        self.assertEqual([], errors(data))
        self.assertEqual(["measure-length-mismatch"], warnings(data))

    def test_a_forward_past_the_notes_of_an_overfull_measure_is_an_error(self):
        data = with_notes(OVERFULL, b"<forward><duration>6</duration></forward>")
        self.assertIn("position-past-measure-end", errors(data))

    def test_a_forward_is_measured_against_the_notes_written_after_it(self):
        data = with_notes(
            b"<forward><duration>5</duration></forward><backup><duration>5</duration></backup>",
            note(4, b"whole"),
            note(1, b"quarter"),
        )
        self.assertEqual([], errors(data))

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

    def test_ties_into_two_repeat_endings_pair(self):
        # The note before the endings starts one tie into each; each ending stops its own.
        data = with_notes(
            note(2, b"half", b'<tie type="start" time-only="1"/><tie type="start" time-only="2"/>'),
            note(1, b"quarter", b'<tie type="stop" time-only="1"/>'),
            note(1, b"quarter", b'<tie type="stop" time-only="2"/>'),
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

    def test_a_tuplet_of_one_note_pairs(self):
        # A tuplet that holds a single note starts and stops on it (W3C test 23e, measure 2).
        tuplet = b'<tuplet type="start"/><tuplet type="stop"/>'
        self.assertEqual([], errors(with_notes(note(4, b"whole", notations=tuplet))))

    def test_a_slur_across_voices_that_stops_first_in_the_document_pairs(self):
        # Voice 1 is written before voice 2, so a slur from beat 1 of voice 2 to beat 3 of voice 1
        # stops before it starts in document order (W3C test 33c, measure 3).
        data = with_notes(
            note(2, b"half", b"<voice>1</voice>"),
            note(2, b"half", b"<voice>1</voice>", notations=b'<slur type="stop" number="1"/>'),
            b"<backup><duration>4</duration></backup>",
            note(2, b"half", b"<voice>2</voice>", notations=b'<slur type="start" number="1"/>'),
            note(2, b"half", b"<voice>2</voice>"),
        )
        self.assertEqual([], errors(data))

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

    def test_a_midi_instrument_naming_no_score_instrument_is_an_error(self):
        def score_part(midi_id: bytes) -> bytes:
            instruments = (
                b'<score-instrument id="P1-I1"><instrument-name>Piano</instrument-name>'
                b'</score-instrument><midi-instrument id="'
                + midi_id
                + b'"><midi-channel>1</midi-channel></midi-instrument>'
            )
            return MINIMAL_SCORE.replace(b"</part-name>", b"</part-name>" + instruments)

        self.assertEqual([], errors(score_part(b"P1-I1")))
        self.assertIn("dangling-instrument-ref", errors(score_part(b"P1-I2")))

    def test_a_measure_shorter_than_its_time_signature_is_a_warning_only(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>",
            b"<duration>3</duration><type>half</type><dot/>",
        )
        self.assertEqual([], errors(data))
        self.assertEqual(["measure-length-mismatch"], warnings(data))

    def test_an_implicit_measure_is_not_measured(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>",
            b"<duration>3</duration><type>half</type><dot/>",
        ).replace(b'<measure number="1">', b'<measure number="1" implicit="yes">')
        self.assertEqual([], warnings(data))

    def test_a_measure_without_meter_is_not_measured(self):
        data = MINIMAL_SCORE.replace(
            b"<duration>4</duration><type>whole</type>",
            b"<duration>3</duration><type>half</type><dot/>",
        ).replace(
            b"<time><beats>4</beats><beat-type>4</beat-type></time>",
            b"<time><senza-misura/></time>",
        )
        self.assertEqual([], warnings(data))

    def test_composite_beats_are_summed(self):
        five_eighths = (
            MINIMAL_SCORE.replace(b"<divisions>1</divisions>", b"<divisions>2</divisions>")
            .replace(
                b"<time><beats>4</beats><beat-type>4</beat-type></time>",
                b"<time><beats>3+2</beats><beat-type>8</beat-type></time>",
            )
            .replace(
                b"<duration>4</duration><type>whole</type>",
                b"<duration>5</duration><type>half</type>",
            )
        )
        self.assertEqual([], warnings(five_eighths))
        self.assertEqual(
            ["measure-length-mismatch"],
            warnings(five_eighths.replace(b"<duration>5<", b"<duration>4<")),
        )

    def test_several_beats_and_beat_type_pairs_are_added(self):
        # 3+2/8 and 3/4 in one <time>, as in W3C test 11e: 5.5 quarter notes, 11 divisions at 2.
        mixed = MINIMAL_SCORE.replace(
            b"<divisions>1</divisions>", b"<divisions>2</divisions>"
        ).replace(
            b"<time><beats>4</beats><beat-type>4</beat-type></time>",
            b"<time><beats>3+2</beats><beat-type>8</beat-type>"
            b"<beats>3</beats><beat-type>4</beat-type></time>",
        )
        dotted_quarter = (
            b"<note><pitch><step>C</step><octave>4</octave></pitch>"
            b"<duration>3</duration><type>quarter</type><dot/></note>"
        )
        self.assertEqual(
            [], warnings(mixed.replace(WHOLE_NOTE, note(8, b"whole") + dotted_quarter))
        )
        self.assertEqual(
            ["measure-length-mismatch"], warnings(mixed.replace(WHOLE_NOTE, note(8, b"whole")))
        )

    def test_a_timewise_score_is_reported_as_not_checked(self):
        timewise = (
            b'<?xml version="1.0" encoding="UTF-8"?>\n<score-timewise version="4.0">'
            b'<part-list><score-part id="P1"><part-name>Music</part-name></score-part></part-list>'
            b'<measure number="1"><part id="P1">'
            + WHOLE_NOTE
            + b"</part></measure></score-timewise>"
        )
        self.assertEqual(["timewise-not-checked"], warnings(timewise))


def with_duration(text: bytes) -> bytes:
    """MINIMAL_SCORE with ``text`` as the duration of its whole note."""
    return MINIMAL_SCORE.replace(b"<duration>4</duration>", b"<duration>" + text + b"</duration>")


class NumberTestCase(unittest.TestCase):
    def test_an_exponent_infinity_nan_or_a_digit_outside_ascii_is_not_a_number(self):
        for text in ("1e5", "1E5", "2.5e-3", "inf", "-Infinity", "NaN", "\u0661\u0662", "\u00a05"):
            with self.subTest(text=ascii(text)):
                self.assertIsNone(musicxml_check._number(text))

    def test_a_decimal_is_read_with_its_sign_and_the_whitespace_around_it(self):
        self.assertEqual(Fraction(-3, 2), musicxml_check._number(" -1.5 "))
        self.assertEqual(Fraction(1, 2), musicxml_check._number(".5"))
        self.assertEqual(Fraction(3), musicxml_check._number("\t+3.\n"))

    def test_a_number_of_more_than_100_digits_is_not_read(self):
        self.assertEqual(int("9" * 100), musicxml_check._number("9" * 100))
        self.assertIsNone(musicxml_check._number("9" * 101))
        self.assertIsNone(musicxml_check._number("0." + "9" * 100))

    def test_a_duration_too_long_to_read_is_left_out_without_an_exception(self):
        # 1e5000 is ten to the 5,000th; Python refuses to write an integer of more than 4,300
        # digits as text, which a finding about the measure's length would do.
        for duration in (b"1e5000", b"1" * 5000):
            with self.subTest(duration=duration[:8]):
                report = musicxml_check.check_bytes(with_duration(duration))
                self.assertEqual(["measure-length-mismatch"], report.warnings)

    def test_a_measure_whose_positions_grow_too_long_is_not_measured(self):
        # Fifty quarter notes, each after a <divisions> of its own, a different odd number of 100
        # digits: the denominator of their sum has thousands of digits, and each addition costs
        # more than the one before.
        notes = b"".join(
            b"<attributes><divisions>%d</divisions></attributes>" % (10**99 + 2 * i + 1)
            + note(1, b"quarter")
            for i in range(50)
        )
        self.assertEqual([], musicxml_check.check_bytes(with_notes(notes)).findings)

    def test_a_time_signature_too_long_to_compute_is_not_read(self):
        # Fifty beats of one, each over a different odd beat type of 100 digits.
        pairs = b"".join(
            b"<beats>1</beats><beat-type>%d</beat-type>" % (10**99 + 2 * i + 1) for i in range(50)
        )
        data = MINIMAL_SCORE.replace(b"<beats>4</beats><beat-type>4</beat-type>", pairs)
        self.assertEqual([], musicxml_check.check_bytes(data).findings)


# The findings of every W3C suite file that has any, as (errors, warnings); every other suite file
# has none. The suite is reference MusicXML, so a rule change that alters this needs a reason for
# each file it touches.
W3C_SUITE_FINDINGS = {
    "03b-Rhythm-Backup.musicxml": ([], ["measure-length-mismatch"]),
    "03d-Rhythm-DottedDurations-Factors.musicxml": ([], ["measure-length-mismatch"]),
    "03e-Rhythm-No-Divisions.musicxml": (["duration-before-divisions"], []),
    "03f-Rhythm-Forward.musicxml": ([], ["measure-length-mismatch"]),
    "11b-TimeSignatures-NoTime.musicxml": (["position-negative"], []),
    "13b-KeySignatures-ChurchModes.musicxml": ([], ["measure-length-mismatch"]),
    "21a-Chord-Basic.musicxml": ([], ["measure-length-mismatch"]),
    "21h-Chord-Accidentals.musicxml": ([], ["measure-length-mismatch"]),
    "21i-Chord-DifferentVoices.musicxml": ([], ["measure-length-mismatch"]),
    "23b-Tuplets-Styles.musicxml": ([], ["measure-length-mismatch"]),
    "24g-GraceNote-Dynamics.musicxml": ([], ["measure-length-mismatch"]),
    "33i-Ties-NotEnded.musicxml": (["unpaired-tie"], []),
    "41g-PartNoId.invalid.musicxml": (
        ["duration-before-divisions", "part-without-score-part", "score-part-without-part"],
        [],
    ),
    "41h-TooManyParts.musicxml": (["duration-before-divisions", "part-without-score-part"], []),
    "45h-Repeats-Partial.musicxml": ([], ["measure-length-mismatch"]),
    "46c-Midmeasure-Clef.musicxml": ([], ["measure-length-mismatch"]),
    "46d-PickupMeasure-ImplicitMeasures.musicxml": ([], ["measure-length-mismatch"]),
    "46f-IncompleteMeasures.musicxml": ([], ["measure-length-mismatch"]),
    "46i-OverfullMeasures.musicxml": ([], ["measure-length-mismatch"]),
    "46j-EmptyMeasure.musicxml": ([], ["measure-length-mismatch"]),
    "51a-Header-Credits.musicxml": (["duration-before-divisions"], []),
    "51b-Header-Quotes.musicxml": (["duration-before-divisions"], []),
    "51c-MultipleMetadata.musicxml": (["duration-before-divisions"], []),
    "51c-MultipleRights.musicxml": (["duration-before-divisions"], []),
    "51d-EmptyTitle.musicxml": (["duration-before-divisions"], []),
    "61i-Lyrics-Chords.musicxml": ([], ["measure-length-mismatch"]),
    "71f-AllChordTypes.musicxml": ([], ["measure-length-mismatch"]),
    "74a-FiguredBass.musicxml": ([], ["measure-length-mismatch"]),
    "90a-Compressed-MusicXML.mxl": ([], ["mimetype-not-first-stored"]),
    "99a-Sibelius5-IgnoreBeaming.musicxml": ([], ["measure-length-mismatch"]),
}


class SuiteFindingsTestCase(unittest.TestCase):
    def test_the_findings_of_every_w3c_suite_file(self):
        found = {}
        for path in suite_files():
            report = musicxml_check.check_file(path)
            if report.errors or report.warnings:
                found[path.name] = (report.errors, report.warnings)
        self.assertEqual(W3C_SUITE_FINDINGS, found)


if __name__ == "__main__":
    unittest.main()
