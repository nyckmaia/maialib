import os
import sys
import platform
from pathlib import Path
from terminal_colors import *

numArgs = len(sys.argv)

if (numArgs != 2):
    print(
        f"{color.FAIL}[ERROR] You MUST pass 1 argument: 'buildType' -> Debug or Release{color.ENDC}")

# Get input command line arguments
buildType = sys.argv[1]

print(f"{color.OKGREEN}Building C++ Unit Tests on {buildType} mode...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "cpp-tests"
path.mkdir(parents=True, exist_ok=True)

# Base CMake command to build the python module
cmakeCommand = f"cmake -G \"Unix Makefiles\" -B {path} -S ./tests-cpp/ -DCMAKE_BUILD_TYPE={buildType}"

# If 'Windows' define the MinGW 'make.exe'
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Get CPU num threads
numThreads = os.cpu_count()

# Run CMake and Make commands
os.system(cmakeCommand)
os.system(f"make -j {numThreads} -C {path} --no-print-directory")

print(f"{color.OKGREEN}Build C++ Tests: Done!{color.ENDC}")
