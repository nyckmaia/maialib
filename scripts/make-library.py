import sys
import os
import platform
from pathlib import Path
from terminal_colors import *

numArgs = len(sys.argv)

if (numArgs != 3):
    print(f"{color.FAIL}[ERROR] You MUST pass 2 arguments: 'libType' and 'buildType'!{color.ENDC}")

# Get input command line arguments
libType = sys.argv[1]
buildType = sys.argv[2]

# ===== VALIDATE INPUT ARGUMENTS ===== #

print(f"{color.OKGREEN}Building a {libType} library in {buildType} mode...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "static" / buildType
path.mkdir(parents=True, exist_ok=True)

# Base CMake command to build the python module
cmakeCommand = f"cmake -G \"Unix Makefiles\" -B {path} -S ./core -DCMAKE_BUILD_TYPE={buildType}"

# If 'Windows' define the MinGW 'make.exe'
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Get CPU num threads
numThreads = os.cpu_count()

# Run CMake and Make commands
os.system(cmakeCommand)
os.system(f"make -j {numThreads} -C {path} --no-print-directory")

print(f"{color.OKGREEN}Done!{color.ENDC}")