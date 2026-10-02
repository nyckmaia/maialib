"""Validate MusicXML 4.0 documents against the official XSD, offline.

The schema files in schema-4.0/ are the unmodified MusicXML 4.0 release. Their imports of xml.xsd
and xlink.xsd name http://www.musicxml.org/xsd/ URLs that no longer resolve, so a resolver maps
them to the local copies: validation never uses the network.

semantic_findings() checks the rules the schema cannot express. It resolves instrument references
only from <instrument> in notes and <midi-instrument> in <score-part>, and checks <staff> only on
notes: the id references of midi-device, play, instrument-change and instrument-link, the player
attributes of assess, wait, sync and other-listening, and <staff> on forward, direction and
harmony, are not checked.

Command line: ``python musicxml_check.py FILE...`` prints one line per file and exits with 1 when
a file cannot be read, is invalid against the schema, or breaks an error-level rule.
"""

from __future__ import annotations

import io
import sys
import zipfile
import zlib
from collections import Counter
from dataclasses import dataclass, field
from fractions import Fraction
from pathlib import Path

from lxml import etree

# The errors reading a damaged archive can raise: zipfile's own; a corrupt deflate stream; data
# that ends early; a missing entry; an unsupported compression method; a corrupt bzip2 stream; an
# encrypted entry or a decompressor this Python lacks; an entry name flagged UTF-8 that is not.
# CPython can be built without lzma, and Python reads Zstandard from 3.14 on, so the errors of
# those decompressors join only when their module imports.
_ARCHIVE_ERRORS: tuple[type[Exception], ...] = (
    zipfile.BadZipFile,
    zlib.error,
    EOFError,
    KeyError,
    NotImplementedError,
    OSError,
    RuntimeError,
    ValueError,
)
try:
    import lzma
except ImportError:
    pass
else:
    _ARCHIVE_ERRORS += (lzma.LZMAError,)
try:
    from compression import zstd
except ImportError:
    pass
else:
    _ARCHIVE_ERRORS += (zstd.ZstdError,)

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

    problem: str | None = None
    xsd_errors: list[str] = field(default_factory=list)
    findings: list[Finding] = field(default_factory=list)

    @property
    def readable(self) -> bool:
        return self.problem is None

    @property
    def xsd_valid(self) -> bool:
        return self.readable and not self.xsd_errors

    @property
    def errors(self) -> list[str]:
        """The names of the failed error-level checks, sorted, without repeats."""
        return sorted({finding.check for finding in self.findings if finding.severity == "error"})

    @property
    def warnings(self) -> list[str]:
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


_compiled: dict[str, etree.XMLSchema] = {}


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
    parser = etree.XMLParser(
        no_network=True, load_dtd=False, resolve_entities=False, huge_tree=True
    )
    return etree.fromstring(data, parser)


def _xsd_errors(schema: etree.XMLSchema, root: etree._Element) -> list[str]:
    if schema.validate(root):
        return []
    return [f"line {entry.line}: {entry.message}" for entry in schema.error_log]


def read_mxl(data: bytes) -> tuple[bytes | None, list[Finding]]:
    """Return the MusicXML document inside an .mxl archive and what is wrong with its layout.

    The document is None when the archive names no readable MusicXML rootfile. Bytes that are not
    a readable zip archive raise zipfile.BadZipFile or another of the errors check_mxl_bytes
    catches: a corrupt deflate, LZMA or Zstandard stream, an entry name flagged UTF-8 that is not,
    an entry compressed by an unsupported method.
    """
    findings: list[Finding] = []
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


def _number(text: str | None) -> Fraction | None:
    """The value of a decimal element text, or None when it is absent or not a number."""
    if text is None:
        return None
    try:
        return Fraction(text.strip())
    except (ValueError, ZeroDivisionError):
        return None


def _time_quarters(time: etree._Element) -> Fraction | None:
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


class _Spanners:
    """The starts and stops of one kind of spanner in a part, counted per key: the pitch for ties,
    the number for slurs and tuplets.

    Counting ignores document order, which is not musical order: MusicXML writes the voices of a
    measure one after another, so a slur from a later voice to an earlier one stops before it
    starts, and a tuplet of one note starts and stops on that note.
    """

    def __init__(self, name: str) -> None:
        self.name = name
        self.starts: Counter[str] = Counter()
        self.stops: Counter[str] = Counter()

    def count(self, kind: str | None, key: str) -> None:
        if kind == "start":
            self.starts[key] += 1
        elif kind == "stop":
            self.stops[key] += 1

    def findings(self, where: str) -> list[Finding]:
        """One error for each key whose starts and stops differ in number."""
        found: list[Finding] = []
        for key in sorted(set(self.starts) | set(self.stops)):
            starts, stops = self.starts[key], self.stops[key]
            if starts != stops:
                larger = "starts than stops" if starts > stops else "stops than starts"
                found.append(
                    Finding(
                        f"unpaired-{self.name}",
                        "error",
                        where,
                        f"{self.name} {key}: starts {starts}, stops {stops} (more {larger})",
                    )
                )
        return found


class _PartState:
    """What carries over from one measure to the next within a part."""

    def __init__(self) -> None:
        self.divisions: Fraction | None = None
        self.staves = 1
        self.quarters: Fraction | None = None
        self.duration_before_divisions_seen = False
        self.ties = _Spanners("tie")
        self.slurs = _Spanners("slur")
        self.tuplets = _Spanners("tuplet")


def _pitch_key(note: etree._Element) -> tuple[str, str, str] | None:
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


def _quarters(element, state: _PartState, where: str, findings: list[Finding]) -> Fraction | None:
    """The element's <duration> in quarter notes, with the divisions in force where it is read;
    None when it has none. A duration before any <divisions> is reported once per part and read
    with divisions 1, the default the W3C test suite agrees on."""
    duration = _number(element.findtext("duration"))
    if duration is None:
        return None
    if state.divisions is None:
        if not state.duration_before_divisions_seen:
            state.duration_before_divisions_seen = True
            findings.append(
                Finding(
                    "duration-before-divisions",
                    "error",
                    where,
                    "a <duration> comes before any <divisions> in its part; read with divisions 1",
                )
            )
        return duration
    return duration / state.divisions


def _count_spanners(note, state: _PartState) -> None:
    key = _pitch_key(note)
    if key is not None:
        # Every <tie> counts: a note before repeat endings can start two ties (time-only="1" and
        # time-only="2"), each stopping in its own ending.
        for tie in note.iterfind("tie"):
            state.ties.count(tie.get("type"), "/".join(key))
    for slur in note.iterfind("notations/slur"):
        state.slurs.count(slur.get("type"), slur.get("number", "1"))
    for tuplet in note.iterfind("notations/tuplet"):
        state.tuplets.count(tuplet.get("type"), tuplet.get("number", "1"))


def _check_note(
    note,
    where: str,
    state: _PartState,
    instrument_ids: set[str],
    has_previous: bool,
    findings: list[Finding],
) -> Fraction | None:
    """Check one note; return its duration in quarter notes, or None when it has none."""
    duration = _quarters(note, state, where, findings)
    if note.find("chord") is not None and not has_previous:
        findings.append(
            Finding(
                "chord-without-anchor", "error", where, "<chord/> on the first note of the measure"
            )
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
    _count_spanners(note, state)
    return duration


def _check_measure(
    part_id: str, measure, state: _PartState, instrument_ids: set[str], findings: list[Finding]
) -> None:
    """Check one measure, following the musical position in quarter notes."""
    where = f"part {part_id}, measure {measure.get('number', '?')}"
    implicit = measure.get("implicit") == "yes"
    cursor = Fraction(0)
    # A chord tone is never longer than the note it joins, so the furthest note end is reached by
    # a note that is not a chord tone.
    note_end = Fraction(0)
    forward_end = Fraction(0)  # the furthest position a <forward> reaches
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
            duration = _check_note(child, where, state, instrument_ids, has_note, findings)
            has_note = True
            if duration is not None and child.find("chord") is None and child.find("grace") is None:
                cursor += duration
                note_end = max(note_end, cursor)
        elif child.tag == "backup":
            cursor -= _quarters(child, state, where, findings) or Fraction(0)
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
            cursor += _quarters(child, state, where, findings) or Fraction(0)
            forward_end = max(forward_end, cursor)
    limit = state.quarters
    if limit is None or implicit:
        return
    # Notes may run past the time signature (an overfull measure, rounded tuplet durations), and a
    # <forward> may follow them there; it is an error only beyond every note of the measure,
    # including the notes written after it.
    if forward_end > limit and forward_end > note_end:
        findings.append(
            Finding(
                "position-past-measure-end",
                "error",
                where,
                f"a <forward> reaches {forward_end} quarter notes, past the time signature "
                f"({limit}) and the last note end ({note_end})",
            )
        )
    end = max(note_end, forward_end)
    if end != limit:
        findings.append(
            Finding(
                "measure-length-mismatch",
                "warning",
                where,
                f"the content lasts {end} quarter notes, the time signature {limit}",
            )
        )


def _check_part_list(root, findings: list[Finding]) -> dict[str, set[str]]:
    """Check part ids and MIDI instrument references; return each part's instrument ids."""
    instruments: dict[str, set[str]] = {}
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
    seen: set[str] = set()
    for part in root.iterfind("part"):
        part_id = part.get("id")
        if part_id not in instruments:
            findings.append(
                Finding(
                    "part-without-score-part",
                    "error",
                    f"part {part_id}",
                    "no <score-part> has this id",
                )
            )
        elif part_id in seen:
            findings.append(
                Finding(
                    "duplicate-part",
                    "error",
                    f"part {part_id}",
                    "a second <part> for one <score-part>",
                )
            )
        seen.add(part_id)
    for part_id in instruments:
        if part_id not in seen:
            findings.append(
                Finding(
                    "score-part-without-part",
                    "error",
                    f"score-part {part_id}",
                    "no <part> has this id",
                )
            )
    return instruments


def _check_part(part, instrument_ids: set[str], findings: list[Finding]) -> None:
    part_id = part.get("id")
    state = _PartState()
    for measure in part.iterfind("measure"):
        _check_measure(part_id, measure, state, instrument_ids, findings)
    where = f"part {part_id}"
    for spanners in (state.ties, state.slurs, state.tuplets):
        findings.extend(spanners.findings(where))


def semantic_findings(root: etree._Element) -> list[Finding]:
    """The rules of a MusicXML document that its schema cannot express."""
    if root.tag == "score-timewise":
        return [
            Finding(
                "timewise-not-checked",
                "warning",
                "score",
                "score-timewise documents are not checked",
            )
        ]
    if root.tag != "score-partwise":
        return []
    findings: list[Finding] = []
    instruments = _check_part_list(root, findings)
    for part in root.iterfind("part"):
        _check_part(part, instruments.get(part.get("id"), set()), findings)
    return findings


def check_bytes(data: bytes) -> Report:
    """Check an uncompressed MusicXML document: the schema, then the semantic rules."""
    try:
        root = parse_document(data)
    except etree.XMLSyntaxError as error:
        return Report(problem=f"not well-formed XML: {error}")
    return Report(xsd_errors=_xsd_errors(musicxml_schema(), root), findings=semantic_findings(root))


def check_mxl_bytes(data: bytes) -> Report:
    """Check an .mxl archive: its layout, then the MusicXML document it names."""
    try:
        document, findings = read_mxl(data)
    except _ARCHIVE_ERRORS as error:
        return Report(problem=f"not a readable zip archive: {error}")
    if document is None:
        problems = "; ".join(finding.detail for finding in findings if finding.severity == "error")
        return Report(problem=problems, findings=findings)
    report = check_bytes(document)
    report.findings = findings + report.findings
    return report


def check_file(path: str | Path) -> Report:
    """Check a .xml, .musicxml or .mxl file; an archive is recognised by its PK signature."""
    data = Path(path).read_bytes()
    return check_mxl_bytes(data) if data[:2] == b"PK" else check_bytes(data)


def main(argv: list[str]) -> int:
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
