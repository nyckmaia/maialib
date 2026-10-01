"""Validate MusicXML 4.0 documents against the official XSD, offline.

The schema files in schema-4.0/ are the unmodified MusicXML 4.0 release. Their imports of xml.xsd
and xlink.xsd name http://www.musicxml.org/xsd/ URLs that no longer resolve, so a resolver maps
them to the local copies: validation never uses the network.

Command line: ``python musicxml_check.py FILE...`` prints one line per file and exits with 1 when
a file cannot be read, is invalid against the schema, or breaks an error-level rule.
"""

from __future__ import annotations

import io
import sys
import zipfile
import zlib
from dataclasses import dataclass, field
from fractions import Fraction
from pathlib import Path

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

    The document is None when the archive names no readable MusicXML rootfile. Raises
    zipfile.BadZipFile (or zlib.error) when the bytes are not a readable zip archive.
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


class _PartState:
    """What carries over from one measure to the next within a part."""

    def __init__(self) -> None:
        self.divisions: Fraction | None = None
        self.staves = 1
        self.quarters: Fraction | None = None
        self.duration_before_divisions_seen = False
        self.open_ties: dict[tuple[str, str, str], int] = {}
        self.open_slurs: set[str] = set()
        self.open_tuplets: set[str] = set()

    def measure_length(self) -> Fraction | None:
        """The time signature's length in divisions, or None when either is unknown."""
        if self.quarters is None or self.divisions is None:
            return None
        return self.quarters * self.divisions


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


def _check_ties(note, state: _PartState, where: str, findings: list[Finding]) -> None:
    key = _pitch_key(note)
    if key is None:
        return
    kinds = {tie.get("type") for tie in note.iterfind("tie")}
    if "stop" in kinds:
        if state.open_ties.get(key, 0) > 0:
            state.open_ties[key] -= 1
        else:
            findings.append(
                Finding(
                    "unpaired-tie", "error", where, f"a tie stop on {'/'.join(key)} has no start"
                )
            )
    if "start" in kinds:
        state.open_ties[key] = state.open_ties.get(key, 0) + 1


def _pair_spanners(
    elements, open_numbers: set[str], name: str, where: str, findings: list[Finding]
) -> None:
    """Pair the starts and stops of slurs or tuplets by number. Stops apply before starts, so a
    note can end one and begin the next with the same number."""
    numbers: dict[str, list[str]] = {"start": [], "stop": []}
    for element in elements:
        kind = element.get("type")
        if kind in numbers:
            numbers[kind].append(element.get("number", "1"))
    for number in numbers["stop"]:
        if number in open_numbers:
            open_numbers.discard(number)
        else:
            findings.append(
                Finding(
                    f"unpaired-{name}", "error", where, f"{name} {number} stops without a start"
                )
            )
    for number in numbers["start"]:
        if number in open_numbers:
            findings.append(
                Finding(f"unpaired-{name}", "error", where, f"{name} {number} starts while open")
            )
        open_numbers.add(number)


def _check_note(
    note,
    where: str,
    state: _PartState,
    instrument_ids: set[str],
    has_previous: bool,
    cursor: Fraction,
    end: Fraction,
    findings: list[Finding],
) -> tuple[Fraction, Fraction]:
    """Check one note; return the measure's cursor and furthest position after it."""
    is_grace = note.find("grace") is not None
    is_chord = note.find("chord") is not None
    duration = _number(note.findtext("duration"))
    if (
        duration is not None
        and state.divisions is None
        and not state.duration_before_divisions_seen
    ):
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
    _check_ties(note, state, where, findings)
    _pair_spanners(note.iterfind("notations/slur"), state.open_slurs, "slur", where, findings)
    _pair_spanners(note.iterfind("notations/tuplet"), state.open_tuplets, "tuplet", where, findings)
    if not is_chord and not is_grace and duration is not None:
        cursor += duration
        end = max(end, cursor)
    return cursor, end


def _check_measure(
    part_id: str, measure, state: _PartState, instrument_ids: set[str], findings: list[Finding]
) -> None:
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
    for key, count in sorted(state.open_ties.items()):
        if count > 0:
            findings.append(
                Finding(
                    "unpaired-tie", "error", where, f"a tie start on {'/'.join(key)} has no stop"
                )
            )
    for number in sorted(state.open_slurs):
        findings.append(Finding("unpaired-slur", "error", where, f"slur {number} never stops"))
    for number in sorted(state.open_tuplets):
        findings.append(Finding("unpaired-tuplet", "error", where, f"tuplet {number} never stops"))


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
    except (
        zipfile.BadZipFile,
        zlib.error,
        EOFError,
        KeyError,
        NotImplementedError,
        OSError,
        RuntimeError,
    ) as error:
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
