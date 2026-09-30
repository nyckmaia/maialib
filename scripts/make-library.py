import os
import platform
import sys
from pathlib import Path

from build_utils import normalize_build_type, run_step, usage_error
from terminal_colors import color

LIB_OPTIONS = {"static": "-DSTATIC_LIB=ON", "shared": "-DSHARED_LIB=ON"}

if len(sys.argv) != 3 or sys.argv[1] not in LIB_OPTIONS:
    usage_error("usage: make-library.py <static|shared> <Debug|Release>")

libType = sys.argv[1]
buildType = normalize_build_type(sys.argv[2])
print(f"{color.OKGREEN}Building a {libType} library in {buildType} mode...{color.ENDC}")

myOS = platform.system()
path = Path.cwd() / "build" / myOS / libType / buildType
path.mkdir(parents=True, exist_ok=True)

cppCompiler = "clang++" if myOS == "Windows" else "g++"
cmakeCommand = [
    "cmake",
    "-G",
    "Unix Makefiles",
    "-B",
    str(path),
    "-S",
    ".",
    LIB_OPTIONS[libType],
    "-DPYBIND_LIB=OFF",
    f"-DCMAKE_BUILD_TYPE={buildType}",
    f"-DCMAKE_CXX_COMPILER={cppCompiler}",
    "-DSQLITECPP_RUN_CPPLINT=OFF",
    "-DSQLITECPP_RUN_CPPCHECK=OFF",
]
if myOS == "Windows":
    cmakeCommand.append("-DCMAKE_MAKE_PROGRAM=C:/msys64/clang64/bin/mingw32-make.exe")

run_step(cmakeCommand, "CMake configure")
run_step(["make", "-j", str(os.cpu_count()), "-C", str(path), "--no-print-directory"], "build")
print(f"{color.OKGREEN}Done!{color.ENDC}")
