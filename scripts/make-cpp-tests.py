import os
import platform
import sys
from pathlib import Path
from typing import Optional

from build_utils import normalize_build_type, run_step, usage_error
from terminal_colors import color

if len(sys.argv) != 2:
    usage_error("usage: make-cpp-tests.py <Debug|Release>")

buildType = normalize_build_type(sys.argv[1])
print(f"{color.OKGREEN}Building C++ Unit Tests on {buildType} mode...{color.ENDC}")

repoRoot = Path(__file__).resolve().parent.parent
myOS = platform.system()
path = repoRoot / "build" / myOS / "cpp-tests"


def cmake_home_directory(cache: Path) -> Optional[str]:
    """Return the source directory a CMakeCache.txt was configured from, if it records one."""
    for line in cache.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("CMAKE_HOME_DIRECTORY:"):
            return line.split("=", 1)[1]
    return None


# CMake refuses a build directory configured from another source tree (for example by an
# older checkout, in which tests-cpp was a standalone project), so such a directory is
# removed before the root project is configured into it.
cache = path / "CMakeCache.txt"
home = cmake_home_directory(cache) if cache.is_file() else None
if home is not None and Path(home).resolve() != repoRoot:
    print(
        f"{color.WARNING}{path} was configured from {home}, not from {repoRoot}: "
        f"removing it.{color.ENDC}"
    )
    run_step(
        [sys.executable, str(Path(__file__).with_name("make-clean.py")), "cpp-tests"],
        "remove the C++ test build directory",
        cwd=str(repoRoot),
    )
path.mkdir(parents=True, exist_ok=True)

cppCompiler = "clang++" if myOS == "Windows" else "g++"
cmakeCommand = [
    "cmake",
    "-G",
    "Unix Makefiles",
    "-B",
    str(path),
    "-S",
    str(repoRoot),
    "-DSTATIC_LIB=ON",
    "-DPYBIND_LIB=OFF",
    "-DMAIACORE_BUILD_TESTS=ON",
    f"-DCMAKE_BUILD_TYPE={buildType}",
    f"-DCMAKE_CXX_COMPILER={cppCompiler}",
    "-DSQLITECPP_RUN_CPPLINT=OFF",
]
if myOS == "Windows":
    cmakeCommand.append("-DCMAKE_MAKE_PROGRAM=C:/msys64/clang64/bin/mingw32-make.exe")

run_step(cmakeCommand, "CMake configure (C++ tests)")
run_step(
    ["make", "-j", str(os.cpu_count()), "-C", str(path), "--no-print-directory", "cpp-tests"],
    "build (C++ tests)",
)
print(f"{color.OKGREEN}Build C++ Tests: Done!{color.ENDC}")
