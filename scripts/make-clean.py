"""Remove build artifacts from the repository.

Usage: make-clean.py <all|dist|static|shared|module|cpp-tests>

Every path is in the repository this script belongs to, whatever the working
directory. A link (a symbolic link or a Windows junction) is removed itself, and
what it points to is left alone. Read-only files (e.g. the git pack files of
fetched dependencies under build/) and read-only directories are removed too.
Every entry that still cannot be removed is listed, the rest is removed anyway,
and the script exits with code 1.
"""

import os
import sys
from pathlib import Path
from typing import Callable, List

from build_utils import REPO_ROOT, build_dir, is_link, remove_trees, usage_error
from terminal_colors import color


def find(top: Path, wanted: Callable[[str], bool]) -> List[Path]:
    """Return the entries below ``top`` whose names ``wanted`` accepts, without entering links."""
    found = []
    for directory, subdirectories, files in os.walk(top):
        found += [Path(directory, name) for name in subdirectories + files if wanted(name)]
        subdirectories[:] = [name for name in subdirectories if not is_link(Path(directory, name))]
    return found


def is_python_cache(name: str) -> bool:
    return name == "__pycache__"


def is_stub(name: str) -> bool:
    return name.endswith(".pyi")


pythonCaches = (
    [REPO_ROOT / "__pycache__", REPO_ROOT / "scripts" / "__pycache__"]
    + find(REPO_ROOT / "maialib", is_python_cache)
    + find(REPO_ROOT / "test", is_python_cache)
)
stubFiles = find(REPO_ROOT / "maialib", is_stub)

TARGETS = {
    "static": [build_dir("static")],
    "shared": [build_dir("shared")],
    "module": [build_dir("module")],
    "cpp-tests": [build_dir("cpp-tests")],
    "dist": [REPO_ROOT / name for name in ("dist", "stubs", "code-coverage", ".coverage")],
    "all": [
        REPO_ROOT / name
        for name in (
            "build",
            "dist",
            "stubs",
            "code-coverage",
            "wheelhouse",
            "maialib.egg-info",
            "profile.json",
            ".coverage",
        )
    ]
    + pythonCaches
    + stubFiles,
}

if len(sys.argv) != 2 or sys.argv[1] not in TARGETS:
    usage_error("usage: make-clean.py <" + "|".join(TARGETS) + ">")

target = sys.argv[1]
print(f"{color.OKGREEN}Cleaning '{target}'...{color.ENDC}")
remove_trees(TARGETS[target])
print(f"{color.OKGREEN}Done!{color.ENDC}")
