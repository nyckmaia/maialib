import os
import platform
from pathlib import Path
from terminal_colors import *

print(f"{color.OKGREEN}Building Maialib Python Module...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "module"
path.mkdir(parents=True, exist_ok=True)

# Base CMake command to build the python module
cmakeCommand = "cmake -G \"Unix Makefiles\" -B {} -S . -D PYBIND_LIB=ON".format(path)

# If 'Windows' define the MinGW 'make.exe'
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Run CMake and Make commands
os.system(cmakeCommand)
os.system("make -j8 -C {} --no-print-directory".format(path))

# Create the '__init__.py' module file
with open(f'{path}/__init__.py', 'w') as f:
    f.write('from .maialib import *\n')

print(f"{color.OKGREEN}Done!{color.ENDC}")