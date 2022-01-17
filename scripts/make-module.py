import os
import platform
from pathlib import Path
from terminal_colors import *

print(f"{color.OKGREEN}Building maiacore Python module...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "module"
path.mkdir(parents=True, exist_ok=True)

# Base CMake command to build the python module
cmakeCommand = "cmake -G \"Unix Makefiles\" -B {} -S ./core -D PYBIND_LIB=ON".format(path)

# If 'Windows' define the MinGW 'make.exe'
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Get CPU num threads
numThreads = os.cpu_count()

# Run CMake and Make commands
os.system(cmakeCommand)
os.system(f"make -j {numThreads} -C {path} --no-print-directory")

print(f"{color.OKGREEN}Done!{color.ENDC}")