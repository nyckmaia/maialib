import os
import platform
import sys
from pathlib import Path

from build_utils import normalize_build_type, run_step, usage_error
from terminal_colors import color

if len(sys.argv) != 2:
    usage_error("usage: make-cpp-tests.py <Debug|Release>")

buildType = normalize_build_type(sys.argv[1])
print(f"{color.OKGREEN}Building C++ Unit Tests on {buildType} mode...{color.ENDC}")

myOS = platform.system()
path = Path.cwd() / "build" / myOS / "cpp-tests"
path.mkdir(parents=True, exist_ok=True)

cppCompiler = "clang++" if myOS == "Windows" else "g++"
cmakeCommand = [
    "cmake",
    "-G",
    "Unix Makefiles",
    "-B",
    str(path),
    "-S",
    "./tests-cpp",
    "-DPYBIND_LIB=OFF",
    f"-DCMAKE_BUILD_TYPE={buildType}",
    f"-DCMAKE_CXX_COMPILER={cppCompiler}",
    "-DSQLITECPP_RUN_CPPLINT=OFF",
    "-DLLVM_USE_CRT_DEBUG=MD",
    "-Dgtest_force_shared_crt=ON",
]
if myOS == "Windows":
    cmakeCommand.append("-DCMAKE_MAKE_PROGRAM=C:/msys64/clang64/bin/mingw32-make.exe")

run_step(cmakeCommand, "CMake configure (C++ tests)")
run_step(
    ["make", "-j", str(os.cpu_count()), "-C", str(path), "--no-print-directory"],
    "build (C++ tests)",
)
print(f"{color.OKGREEN}Build C++ Tests: Done!{color.ENDC}")
