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

print(f"{color.OKGREEN}Building maiacore Python module on {buildType} mode...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder(if not exists)
path = Path.cwd() / "build" / myOS / "module"
path.mkdir(parents=True, exist_ok=True)

# Set the C++ compiler
CppCompiler = "clang++" if myOS == "Windows" else "g++"

# Base CMake command to build the python module
cmakeCommand = f"cmake -G \"Unix Makefiles\" -B {path} -S . -D PYBIND_LIB=ON -DCMAKE_BUILD_TYPE={buildType} -DCMAKE_CXX_COMPILER={CppCompiler} -DSQLITECPP_RUN_CPPLINT=OFF"
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/clang64/bin/mingw32-make.exe"'

if (buildType == "Debug"):
    cmakeCommand += " -DPROFILING=ON"

# Get CPU num threads
numThreads = os.cpu_count()

# Run CMake and Make commands
os.system(cmakeCommand)

os.system(f"make -j {numThreads} -C {path} --no-print-directory")

# ===== COMPILAÇÃO COM VS 2022 ===== #
# if myOS == "Windows":
# Na raiz do maialib
# cmake -G "Visual Studio 17 2022" -A x64 -S . -B ./build -DPYBIND_LIB=ON -DSQLITECPP_RUN_CPPLINT=OFF
# msbuild ./build/maiacore.sln /p:Configuration=RelWithDebInfo /p:Platform=x64
# ================================== #

print(f"{color.OKGREEN}Done!{color.ENDC}")
