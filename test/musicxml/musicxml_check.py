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
