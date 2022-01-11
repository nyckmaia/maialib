import os
import platform
from pathlib import Path
from terminal_colors import *

print(f"{color.OKGREEN}Building Maia App...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "app"
path.mkdir(parents=True, exist_ok=True)

# Locate the maia app root directory
appRootDir = Path.cwd() / "../app"

# Base CMake command to build the python module
cmakeCommand = "cmake -G \"Unix Makefiles\" -B {} -S {}".format(path, appRootDir)

# If 'Windows' define the MinGW 'make.exe'
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Run CMake and Make commands
os.system(cmakeCommand)
os.system("make -j8 -C {} --no-print-directory".format(path))

print(f"{color.OKGREEN}Done!{color.ENDC}")