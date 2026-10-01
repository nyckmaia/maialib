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


if __name__ == "__main__":
    unittest.main()
