import os
import platform
from pathlib import Path
from terminal_colors import *

print(f"{color.OKGREEN}Building C++ Unit Tests...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "cpp-tests"
path.mkdir(parents=True, exist_ok=True)

# Base CMake command to build the python module
cmakeCommand = "cmake -G \"Unix Makefiles\" -B {} -S ./tests-cpp/".format(path)

# If 'Windows' define the MinGW 'make.exe'
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Run CMake and Make commands
os.system(cmakeCommand)
os.system("make -j8 -C {} --no-print-directory".format(path))

print(f"{color.OKGREEN}Build C++ Tests: Done!{color.ENDC}")