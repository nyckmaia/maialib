"""Remove build artifacts.

Usage: make-clean.py <all|dist|static|shared|module|cpp-tests>

Read-only files (e.g. the git pack files of fetched dependencies under build/)
are made writable and removed; anything that still cannot be removed is listed
and the script exits with code 1.
"""

import glob
import os
import platform
import stat
import sys
from pathlib import Path
from shutil import rmtree
from typing import List

from build_utils import usage_error
from terminal_colors import color

osBuildDir = Path("build") / platform.system()
pythonCaches = [
    Path(p)
    for p in glob.glob("__pycache__")
    + glob.glob("maialib/**/__pycache__", recursive=True)
    + glob.glob("test/**/__pycache__", recursive=True)
    + glob.glob("scripts/__pycache__")
]
stubFiles = [Path(p) for p in glob.glob("maialib/**/*.pyi", recursive=True)]

TARGETS = {
    "static": [osBuildDir / "static"],
    "shared": [osBuildDir / "shared"],
    "module": [osBuildDir / "module"],
    "cpp-tests": [osBuildDir / "cpp-tests"],
    "dist": [Path("dist"), Path("stubs"), Path("code-coverage"), Path(".coverage")],
    "all": [
        Path("build"),
        Path("dist"),
        Path("stubs"),
        Path("code-coverage"),
        Path("wheelhouse"),
        Path("maialib.egg-info"),
        Path("profile.json"),
        Path(".coverage"),
    ]
    + pythonCaches
    + stubFiles,
}

if len(sys.argv) != 2 or sys.argv[1] not in TARGETS:
    usage_error("usage: make-clean.py <" + "|".join(TARGETS) + ">")

failures: List[str] = []


def makeWritableAndRetry(function, path, _excinfo):
    os.chmod(path, stat.S_IWRITE)
    function(path)


def remove(path: Path) -> None:
    if not path.exists():
        return
    try:
        if path.is_dir():
            if sys.version_info >= (3, 12):
                rmtree(path, onexc=makeWritableAndRetry)
            else:
                rmtree(path, onerror=makeWritableAndRetry)
        else:
            os.chmod(path, stat.S_IWRITE)
            path.unlink()
    except OSError as error:
        failures.append(f"{path}: {error}")


target = sys.argv[1]
print(f"{color.OKGREEN}Cleaning '{target}'...{color.ENDC}")
for path in TARGETS[target]:
    remove(path)

if failures:
    print(f"{color.FAIL}Could not remove:{color.ENDC}")
    for failure in failures:
        print(f"  {failure}")
    sys.exit(1)
print(f"{color.OKGREEN}Done!{color.ENDC}")
