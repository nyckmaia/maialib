import os
import platform
import sys
from pathlib import Path

from build_utils import normalize_build_type, run_step, usage_error
from terminal_colors import color

if len(sys.argv) != 2:
    usage_error("usage: make-module.py <Debug|Release>")

buildType = normalize_build_type(sys.argv[1])
print(f"{color.OKGREEN}Building maiacore Python module on {buildType} mode...{color.ENDC}")

myOS = platform.system()
path = Path.cwd() / "build" / myOS / "module"
path.mkdir(parents=True, exist_ok=True)

cppCompiler = "clang++" if myOS == "Windows" else "g++"
# The module is built for the interpreter running this script, the one `make install` then
# installs it into; without the flag, CMake would keep the interpreter cached by an earlier
# configure, even one from another (or deleted) virtual environment.
cmakeCommand = [
    "cmake",
    "-G",
    "Unix Makefiles",
    "-B",
    str(path),
    "-S",
    ".",
    "-DPYBIND_LIB=ON",
    f"-DPYTHON_EXECUTABLE={sys.executable}",
    f"-DCMAKE_BUILD_TYPE={buildType}",
    f"-DCMAKE_CXX_COMPILER={cppCompiler}",
    "-DSQLITECPP_RUN_CPPLINT=OFF",
]
if myOS == "Windows":
    cmakeCommand.append("-DCMAKE_MAKE_PROGRAM=C:/msys64/clang64/bin/mingw32-make.exe")
if buildType == "Debug":
    cmakeCommand.append("-DPROFILING=ON")

run_step(cmakeCommand, "CMake configure")
run_step(["make", "-j", str(os.cpu_count()), "-C", str(path), "--no-print-directory"], "build")
print(f"{color.OKGREEN}Done!{color.ENDC}")
