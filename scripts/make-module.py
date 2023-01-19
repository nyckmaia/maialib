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

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "module"
path.mkdir(parents=True, exist_ok=True)

# Base CMake command to build the python module
if myOS == "Windows":
    vcvarsallPath = "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat\""
    os.system(f"{vcvarsallPath} x86_amd64")

    clPath = "\"C:\\Program Files\\Microsoft Visual Studio\\2022\Community\\VC\\Tools\\MSVC\\14.34.31933\\bin\\Hostx64\\x64\\cl.exe\""
    cmakeCommand = f"cmake -G \"Visual Studio 17 2022\" -A x64 -B {path} -S . -D PYBIND_LIB=ON -DCMAKE_BUILD_TYPE={buildType} -DCMAKE_CXX_COMPILER={clPath} -DMAIALIB_VERSION_INFO=0.0.0"
else:
    cmakeCommand = f"cmake -G \"Unix Makefiles\" -B {path} -S . -D PYBIND_LIB=ON -DCMAKE_BUILD_TYPE={buildType}"

if (buildType == "Debug" or buildType == "Release"):
    cmakeCommand += " -DPROFILING=ON"

# # If 'Windows' define the MinGW 'make.exe'
# if myOS == "Windows":
#     cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Get CPU num threads
numThreads = os.cpu_count()

# Run CMake and Make commands
os.system(cmakeCommand)
if myOS == "Windows":
    msbuildPath = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\MSBuild\\Current\\Bin"
    os.system(
        f"msbuild.exe {path}\\maiacore.sln -verbosity:quiet -maxcpucount")

    # os.system(f"cmake --build {path} --config Debug")
else:
    os.system(f"make -j {numThreads} -C {path} --no-print-directory")

print(f"{color.OKGREEN}Done!{color.ENDC}")
