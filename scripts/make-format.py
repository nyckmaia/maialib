"""Format all C++ source and header files in-place using clang-format.

This replaces the old `format-cpp` Makefile recipe, which used to shell out
to the POSIX `find` utility directly -- the only target in the whole
Makefile that did so, while every other target delegates to a
`scripts/make-*.py` script. That made `format-cpp` the one target that broke
if `make` fell back to `cmd.exe` on Windows, where `find` resolves to
Microsoft's unrelated `find.exe` instead of the POSIX one. This script
collects the files with pathlib instead, so it works identically regardless
of which shell invokes `make`.
"""

import shutil
import subprocess
import sys
from pathlib import Path

from build_utils import run_step
from terminal_colors import color

# Well-known LLVM install locations, tried only if the 'clang-format' found on
# PATH is missing or non-functional (e.g. a broken pip-installed wrapper).
FALLBACK_CANDIDATES = [
    r"C:\Program Files\LLVM\bin\clang-format.exe",
    r"C:\Program Files (x86)\LLVM\bin\clang-format.exe",
    "/usr/local/opt/llvm/bin/clang-format",
    "/opt/homebrew/opt/llvm/bin/clang-format",
    "/usr/local/bin/clang-format",
    "/usr/bin/clang-format",
]


def is_working_clang_format(executable: str) -> bool:
    """Return True if 'executable' can actually be run (i.e. '--version' succeeds).

    A binary merely existing/resolving is not enough: on this project's Windows
    dev machine, the 'clang-format' found on PATH is a broken pip-installed
    wrapper that crashes with exit code 1 even on '--version'.
    """
    try:
        result = subprocess.run(
            [executable, "--version"], capture_output=True, text=True, timeout=10
        )
    except (OSError, subprocess.TimeoutExpired):
        return False
    return result.returncode == 0


def find_clang_format() -> str:
    """Locate a functional 'clang-format' executable.

    PATH is probed first via shutil.which(), since a working PATH entry is
    the common case and must not be bypassed. Only if that entry is missing
    or non-functional do we fall back to a handful of well-known LLVM install
    locations. If none of those work either, fail loudly with an actionable
    message instead of silently formatting nothing.
    """
    tried = []

    on_path = shutil.which("clang-format")
    if on_path:
        tried.append(on_path)
        if is_working_clang_format(on_path):
            return on_path

    for candidate in FALLBACK_CANDIDATES:
        if candidate in tried or not Path(candidate).is_file():
            continue
        tried.append(candidate)
        if is_working_clang_format(candidate):
            return candidate

    print(f"{color.FAIL}[ERROR] No functional 'clang-format' executable found.{color.ENDC}")
    if tried:
        print(
            f"{color.FAIL}Found but not functional (failed to run '--version'): "
            f"{', '.join(tried)}{color.ENDC}"
        )
    else:
        print(
            f"{color.FAIL}Not found on PATH or in any well-known LLVM install location."
            f"{color.ENDC}"
        )
    print(
        f"{color.FAIL}Install LLVM's clang-format (https://releases.llvm.org/) and make sure "
        f"a working copy is first on PATH, e.g. the LLVM install's 'bin' directory "
        f"(such as 'C:\\Program Files\\LLVM\\bin' on Windows).{color.ENDC}"
    )
    sys.exit(1)


def collect_files(directory: Path, patterns: list, exclude_names: frozenset = frozenset()) -> list:
    """Recursively collect files under 'directory' matching any of 'patterns',
    skipping any file whose name is in 'exclude_names'."""
    files = set()
    for pattern in patterns:
        for path in directory.rglob(pattern):
            if path.is_file() and path.name not in exclude_names:
                files.add(path)
    return sorted(files)


clangFormatExe = find_clang_format()
print(f"{color.OKGREEN}Formatting C++ code with clang-format ({clangFormatExe})...{color.ENDC}")

repoRoot = Path.cwd()
styleFile = repoRoot / ".clang-format"

# The two excluded tests-cpp headers are generated characterisation tables: the tests compare
# the library against them, so they must stay byte-identical to what was generated and are
# left unformatted.
targets = [
    (repoRoot / "maiacore" / "include" / "maiacore", ["*.h"], frozenset()),
    (repoRoot / "maiacore" / "src" / "maiacore", ["*.cpp", "*.h"], frozenset()),
    (
        repoRoot / "tests-cpp" / "src",
        ["*.cpp", "*.h"],
        frozenset({"pitch-spelling-legacy-data.h", "quarter-tone-characterization-data.h"}),
    ),
]

filesToFormat = []
for directory, patterns, excludeNames in targets:
    filesToFormat += collect_files(directory, patterns, excludeNames)

if not filesToFormat:
    print(
        f"{color.FAIL}[ERROR] No C++ files found to format under {[str(t[0]) for t in targets]}. "
        f"Run this from the repo root.{color.ENDC}"
    )
    sys.exit(1)

run_step(
    [clangFormatExe, f"--style=file:{styleFile}", "-i", *[str(f) for f in filesToFormat]],
    "clang-format",
)

print(f"{color.OKGREEN}C++ formatting complete.{color.ENDC}")
