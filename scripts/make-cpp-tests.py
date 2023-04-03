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

# Set the C++ compiler
CppCompiler = "clang++" if myOS == "Windows" else "g++"

# Base CMake command to build the python module
cmakeCommand = f"cmake -G \"Unix Makefiles\" -B {path} -S ./tests-cpp -D PYBIND_LIB=OFF -DCMAKE_BUILD_TYPE={buildType} -DCMAKE_CXX_COMPILER={CppCompiler} -DSQLITECPP_RUN_CPPLINT=OFF -DLLVM_USE_CRT_DEBUG=MD -Dgtest_force_shared_crt=ON"
if myOS == "Windows":
    cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/clang64/bin/mingw32-make.exe"'

if (buildType == "Debug"):
    cmakeCommand += " -DPROFILING=ON"

# # Base CMake command to build the python module
# if myOS == "Windows":
#     vcvarsallPath = "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat\""
#     os.system(f"{vcvarsallPath} x86_amd64")

#     clPath = "\"C:\\Program Files\\Microsoft Visual Studio\\2022\Community\\VC\\Tools\\MSVC\\14.34.31933\\bin\\Hostx64\\x64\\cl.exe\""
#     cmakeCommand = f"cmake -G \"Visual Studio 17 2022\" -A x64 -B {path} -S ./tests-cpp/ -DCMAKE_BUILD_TYPE={buildType} -DCMAKE_CXX_COMPILER={clPath} -DMAIALIB_VERSION_INFO=0.0.0"
# else:
#     cmakeCommand = f"cmake -G \"Unix Makefiles\" -B {path} -S ./tests-cpp/ -DCMAKE_BUILD_TYPE={buildType}"

# # # If 'Windows' define the MinGW 'make.exe'
# # if myOS == "Windows":
# #     cmakeCommand += ' -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/make.exe"'

# Get CPU num threads
numThreads = os.cpu_count()

# Run CMake and Make commands
os.system(cmakeCommand)

os.system(f"make -j {numThreads} -C {path} --no-print-directory")

# if myOS == "Windows":
#     msbuildPath = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\MSBuild\\Current\\Bin"
#     os.system(
#         f"msbuild.exe {path}\\cpp-tests.sln -verbosity:quiet -maxcpucount")

# # os.system(f "cmake --build {path} --config Debug")
# else:
#     os.system(f"make -j {numThreads} -C {path} --no-print-directory")

print(f"{color.OKGREEN}Build C++ Tests: Done!{color.ENDC}")
