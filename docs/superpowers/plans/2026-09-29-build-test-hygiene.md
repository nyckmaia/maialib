# Build and Test Hygiene (roadmap step 10a) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every local build/test command trustworthy (a failure is reported as a failure, the C++ tests always test the library just built, one cpptrace version), stop CI from publishing to PyPI on anything but a published release, and commit the true MSVC and Linux gates as scripts.

**Architecture:** One shared `scripts/build_utils.py` runs every external command and exits with its code; `tests-cpp` becomes a subdirectory of the root CMake project so the test executable links the `maiacore` target (no prebuilt library by path); `make validate` compares findings with a committed baseline; the workflow's publish job is guarded by the release event.

**Tech Stack:** Python 3 scripts (stdlib only), GNU make, CMake ≥ 3.15 (3.26 installed), clang 18 + MSVC STL locally, MSVC (cl) through the Visual Studio 2022 generator, GoogleTest 1.14, cpptrace (FetchContent), GitHub Actions.

**Spec:** Design note below; roadmap context in `C:/Users/nyck/.claude/plans/wobbly-coalescing-quill.md` (step 10a).

## Design note (replaces a separate spec for this small step)

Measured on `main` @ `cab5d60`:
- Every `scripts/*.py` except `make-install.py` and `make-format.py` runs commands through `os.system` and ignores the result, so `make tests`, `make cpp-tests`, `make py-tests`, `make validate` and the library builds exit 0 when compilation or tests fail. Scripts given wrong arguments print an error and then crash with `IndexError`.
- `make-py-tests.py` / `make-uninstall.py` / `make-install.py` call bare `python`/`pip`/`pybind11-stubgen`/`stubgen`; on this machine `python` is 3.14 while the tools live in 3.12, so stubs can come from a different environment than the one `make` installed into.
- `tests-cpp/CMakeLists.txt` is a separate project that links `maiacore` by name from `../build/${OS}/static/Debug`; the executable has no dependency on `maiacore.lib`, so a library-only change leaves a stale test binary. It fetches cpptrace **v0.6.2**, while the root fetches **v0.8.2** and also puts the vendored v0.6.2 headers (`maiacore/include/external/cpptrace/include`, 117 tracked files) on maiacore's include path — maiacore compiles against v0.6.2 headers and links v0.8.2. `log.h` (public) includes `<cpptrace/cpptrace.hpp>`; `score.h` (public) includes SQLiteCpp headers.
- `_ITERATOR_DEBUG_LEVEL=0` is forced on every Windows build of the tests but only on non-MSVC Windows builds of the root, so MSVC Debug builds of both cannot link (LNK2038).
- `make-clean.py` implements only `all` and `dist` (`static`, `shared`, `module` silently do nothing), uses wrong `__pycache__` paths, and `rmtree(..., True)` silently leaves the read-only cpptrace git pack files under `build/`.
- Makefile: target `shared-relase` (typo; `make shared` calls the missing `shared-release`), `make cmake` calls a missing script, recursion uses `@make`, `make-library.py` builds shared libraries into `static/`.
- `make validate`: cpplint currently exits 1 with ~60 pre-existing findings (45 `build/include_what_you_use`), so a plain exit-code propagation would make the target always fail.
- `.gitignore` contains `test_*.xml` without an anchor, so any new fixture named `test_*.xml` anywhere (e.g. `test/xml_examples/`) is silently not committed.
- `.github/workflows/wheels.yml` triggers on push to `main`, `workflow_dispatch` and `release: published`; its `upload_all` job publishes to PyPI with no `if:` — any push to `main` or manual run publishes.

Decisions:
- D1 All scripts use `build_utils.run_step()`; a sequence runs without a shell, a string runs through the shell. Usage errors exit 2. Python-based tools run as `sys.executable -m <module>`.
- D2 `tests-cpp` is built only through the root project (`-DSTATIC_LIB=ON -DMAIACORE_BUILD_TESTS=ON`), into `build/<OS>/cpp-tests`, with the executable kept at `build/<OS>/cpp-tests/cpp-tests(.exe)` for single-config generators. Standalone configuration of `tests-cpp/` is no longer supported.
- D3 cpptrace comes only from FetchContent v0.8.2; the vendored headers are deleted. cpptrace and SQLiteCpp become `PUBLIC` link dependencies of the static/shared library because public headers include them.
- D4 `make validate` fails only when a (tool, file, category) count exceeds `scripts/validate-baseline.json`; `--update-baseline` rewrites it.
- D5 `upload_all` runs only for `release`/`published`. A stdlib-only Python test pins the guard.
- D6 The MSVC gate and the Linux gate are scripts in `scripts/`, each exiting non-zero on any failure.

## Global Constraints
- Makefile stays portable (Windows, Linux, macOS): GNU make syntax only, no shell-specific constructs.
- Scripts use only the Python standard library.
- Docs, comments, docstrings and CHANGELOG in technical English; comments explain the code, never its development history.
- Never stage the user's uncommitted `.gitignore` line `musescore/*` (see Task 5 for the staging method). Stage files by name, never `git add -A` / `git add .`.
- Commit messages end with:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Baseline suites before any change: C++ **1050/1050**, Python **471/471**. They must be identical at the end (same test names), plus any test this plan adds.
- Build traps on this machine: set the MSVC environment for clang from PowerShell (`$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'`, `INCLUDE`/`LIB` unset), never `vswhere -latest`; delete files under `build\Windows` from Bash, not PowerShell; run the C++ tests from the repository root; `make dev` only inside a brand-new `py -3.12` venv; never read an exit code through a pipe.

---

### Task 0: Record the baseline

**Files:** none (measurements only; record them in the SDD ledger).

- [ ] **Step 1: Suites and exit codes as they are today**

In a brand-new `py -3.12` venv with `requirements-dev.txt`, from PowerShell with the MSVC environment above:
```
make clean ; rm -rf build   (rm from Bash)
make dev
make cpp-tests ; echo "exit=$LASTEXITCODE"
make py-tests ; echo "exit=$LASTEXITCODE"
```
Expected: C++ `[  PASSED  ] 1050 tests.`, Python `Ran 471 tests ... OK`, both exits 0.

- [ ] **Step 2: Prove today's exit codes lie**

Create a scratch failing test outside version control (`test/test_zz_scratch_failure.py` with `class T(unittest.TestCase): def test_fail(self): self.fail("scratch")`), run `make py-tests`, record the exit code (expected today: 0 despite the failure), then delete the file. For C++: `GTEST_ALSO_RUN_DISABLED_TESTS=1` env var makes disabled tests run; record whether any fails and `make cpp-tests`' exit code (expected today: 0). Record both results; they are the "before" evidence for Task 1.

---

### Task 1: Shared command runner; every script propagates exit codes

**Files:**
- Create: `scripts/build_utils.py`
- Modify: `scripts/make-library.py`, `scripts/make-module.py`, `scripts/make-py-tests.py`, `scripts/make-uninstall.py`, `scripts/make-install.py`, `scripts/make-format.py`, `scripts/make-dist.py`, `scripts/run-code-coverage.py`, `Makefile`

**Interfaces:**
- Produces: `build_utils.run_step(command, step_name, cwd=None, timeout=None) -> None`, `build_utils.usage_error(message) -> NoReturn`, `build_utils.normalize_build_type(value) -> str` ("Debug"/"Release"). Tasks 2–4 and 6–7 import these.

- [ ] **Step 1: Write `scripts/build_utils.py`**

```python
"""Shared helpers for the build, test and install scripts in this directory.

Every external command runs through run_step(), so a failing compiler, test
binary or tool stops the script with that command's exit code, and `make`
reports the failure instead of printing success.
"""

import subprocess
import sys
from typing import NoReturn, Optional, Sequence, Union

from terminal_colors import color

Command = Union[str, Sequence[str]]

BUILD_TYPES = {"debug": "Debug", "release": "Release"}


def run_step(
    command: Command, step_name: str, cwd: Optional[str] = None, timeout: Optional[float] = None
) -> None:
    """Run ``command``; on failure print ``step_name`` and exit with the command's code.

    A string runs through the shell (for commands that need shell syntax); a
    sequence runs directly, so arguments need no shell quoting. A command that
    exceeds ``timeout`` seconds is reported and ends the script with code 124.
    """
    try:
        result = subprocess.run(command, shell=isinstance(command, str), cwd=cwd, timeout=timeout)
    except subprocess.TimeoutExpired:
        print(f"{color.FAIL}Step timed out after {timeout} s: {step_name}{color.ENDC}")
        sys.exit(124)
    except FileNotFoundError as error:
        print(f"{color.FAIL}Step failed: {step_name} (command not found: {error.filename}){color.ENDC}")
        sys.exit(127)
    if result.returncode != 0:
        print(f"{color.FAIL}Step failed: {step_name} (exit code {result.returncode}){color.ENDC}")
        sys.exit(result.returncode)


def usage_error(message: str) -> NoReturn:
    """Print a usage error and exit with code 2."""
    print(f"{color.FAIL}[ERROR] {message}{color.ENDC}")
    sys.exit(2)


def normalize_build_type(value: str) -> str:
    """Map 'debug'/'Debug'/'release'/... to CMake's 'Debug'/'Release', or exit with a usage error."""
    try:
        return BUILD_TYPES[value.lower()]
    except KeyError:
        usage_error(f"unknown build type '{value}': expected Debug or Release")
```

- [ ] **Step 2: Convert each script**

Rules for every script: `from build_utils import run_step, usage_error, normalize_build_type` as needed; validate `sys.argv` before indexing it (`usage_error("usage: <script> <args>")`); replace each `os.system(cmd)` with `run_step([...], "<step name>")` using an argument list; replace bare `python`/`pip`/`python3` with `sys.executable`; replace `pybind11-stubgen ...` with `[sys.executable, "-m", "pybind11_stubgen", ...]` and `stubgen ...` with `[sys.executable, "-m", "mypy.stubgen", ...]`. Delete the private `run_step` copies in `make-install.py` and `make-format.py` and import the shared one (keep their call sites' behaviour).

`scripts/make-library.py` becomes (the library type now selects both the CMake option and the output directory):
```python
import os
import platform
import sys
from pathlib import Path

from build_utils import normalize_build_type, run_step, usage_error
from terminal_colors import color

LIB_OPTIONS = {"static": "-DSTATIC_LIB=ON", "shared": "-DSHARED_LIB=ON"}

if len(sys.argv) != 3 or sys.argv[1] not in LIB_OPTIONS:
    usage_error("usage: make-library.py <static|shared> <Debug|Release>")

libType = sys.argv[1]
buildType = normalize_build_type(sys.argv[2])
print(f"{color.OKGREEN}Building a {libType} library in {buildType} mode...{color.ENDC}")

myOS = platform.system()
path = Path.cwd() / "build" / myOS / libType / buildType
path.mkdir(parents=True, exist_ok=True)

cppCompiler = "clang++" if myOS == "Windows" else "g++"
cmakeCommand = [
    "cmake", "-G", "Unix Makefiles", "-B", str(path), "-S", ".",
    LIB_OPTIONS[libType], "-DPYBIND_LIB=OFF",
    f"-DCMAKE_BUILD_TYPE={buildType}", f"-DCMAKE_CXX_COMPILER={cppCompiler}",
    "-DSQLITECPP_RUN_CPPLINT=OFF",
]
if myOS == "Windows":
    cmakeCommand.append("-DCMAKE_MAKE_PROGRAM=C:/msys64/clang64/bin/mingw32-make.exe")

run_step(cmakeCommand, "CMake configure")
run_step(["make", "-j", str(os.cpu_count()), "-C", str(path), "--no-print-directory"], "build")
print(f"{color.OKGREEN}Done!{color.ENDC}")
```
Apply the same pattern to `make-module.py` (one argument, `normalize_build_type`, `-DPROFILING=ON` kept for Debug). `make-py-tests.py` becomes `run_step([sys.executable, "-m", "unittest"], "Python unit tests", cwd="test")` with no OS branch. `make-uninstall.py` uses `run_step([sys.executable, "-m", "pip", "uninstall", "--yes", "maialib"], "pip uninstall maialib")`. `make-dist.py`: if no built module is found, print which directory was searched and `sys.exit(1)` instead of `IndexError`. `run-code-coverage.py`: every `os.system` through `run_step`.

- [ ] **Step 3: Makefile — one interpreter, real recursion**

At the top of the Makefile:
```make
# The interpreter that runs every script; override with e.g. `make PYTHON=py\ -3.12`.
ifeq ($(OS),Windows_NT)
PYTHON ?= python
else
PYTHON ?= python3
endif
```
Replace every `@python $(SCRIPTS_DIR)/` with `@$(PYTHON) $(SCRIPTS_DIR)/` and every recursive `@make <target>` with `@$(MAKE) --no-print-directory <target>`.

- [ ] **Step 4: Verify the failure now propagates**

Repeat Task 0 Step 2 (scratch failing Python test; `GTEST_ALSO_RUN_DISABLED_TESTS=1` if a disabled C++ test fails — otherwise a scratch failing gtest in a file you delete afterwards). Expected: `make py-tests` and `make cpp-tests` now exit non-zero; `make tests` stops at the first failure. Also run each converted script with no/invalid arguments: exit code 2 and a usage line. Remove every scratch file; `git status` shows no scratch files.

- [ ] **Step 5: Full suites unchanged**

`make dev` (fresh venv) then `make cpp-tests` and `make py-tests`: 1050/1050 and 471/471, both exit 0.

- [ ] **Step 6: Commit**

```bash
git add scripts/build_utils.py scripts/make-library.py scripts/make-module.py scripts/make-py-tests.py scripts/make-uninstall.py scripts/make-install.py scripts/make-format.py scripts/make-dist.py scripts/run-code-coverage.py Makefile
git commit -m "build: every script propagates its commands' exit codes"
```

---

### Task 2: `make clean` removes everything it should, and the Makefile targets exist

**Files:**
- Modify: `scripts/make-clean.py`, `Makefile`

- [ ] **Step 1: Rewrite `scripts/make-clean.py`**

```python
"""Remove build artifacts.

Usage: make-clean.py <all|dist|static|shared|module|cpp-tests>

Read-only files (e.g. the git pack files of fetched dependencies under build/)
are made writable and removed; anything that still cannot be removed is listed
and the script exits with code 1.
"""

import glob
import os
import platform
import stat
import sys
from pathlib import Path
from shutil import rmtree

from build_utils import usage_error
from terminal_colors import color

osBuildDir = Path("build") / platform.system()
pythonCaches = [Path(p) for p in glob.glob("__pycache__") + glob.glob("maialib/**/__pycache__", recursive=True)
                + glob.glob("test/**/__pycache__", recursive=True) + glob.glob("scripts/__pycache__")]
stubFiles = [Path(p) for p in glob.glob("maialib/**/*.pyi", recursive=True)]

TARGETS = {
    "static": [osBuildDir / "static"],
    "shared": [osBuildDir / "shared"],
    "module": [osBuildDir / "module"],
    "cpp-tests": [osBuildDir / "cpp-tests"],
    "dist": [Path("dist"), Path("stubs"), Path("code-coverage"), Path(".coverage")],
    "all": [Path("build"), Path("dist"), Path("stubs"), Path("code-coverage"), Path("wheelhouse"),
            Path("maialib.egg-info"), Path("profile.json"), Path(".coverage")] + pythonCaches + stubFiles,
}

if len(sys.argv) != 2 or sys.argv[1] not in TARGETS:
    usage_error("usage: make-clean.py <" + "|".join(TARGETS) + ">")

failures = []


def makeWritableAndRetry(function, path, _excinfo):
    os.chmod(path, stat.S_IWRITE)
    function(path)


def remove(path: Path) -> None:
    if not path.exists():
        return
    try:
        if path.is_dir():
            if sys.version_info >= (3, 12):
                rmtree(path, onexc=makeWritableAndRetry)
            else:
                rmtree(path, onerror=makeWritableAndRetry)
        else:
            os.chmod(path, stat.S_IWRITE)
            path.unlink()
    except OSError as error:
        failures.append(f"{path}: {error}")


target = sys.argv[1]
print(f"{color.OKGREEN}Cleaning '{target}'...{color.ENDC}")
for path in TARGETS[target]:
    remove(path)

if failures:
    print(f"{color.FAIL}Could not remove:{color.ENDC}")
    for failure in failures:
        print(f"  {failure}")
    sys.exit(1)
print(f"{color.OKGREEN}Done!{color.ENDC}")
```

- [ ] **Step 2: Makefile targets**

Rename the target `shared-relase` to `shared-release` (and in `.PHONY`); delete the `cmake` target and its `.PHONY` entry (its script does not exist); add `cpp-tests-clean: ; @$(PYTHON) $(SCRIPTS_DIR)/make-clean.py cpp-tests` with a `.PHONY` entry.

- [ ] **Step 3: Verify**

After a full build (`make dev`, `make cpp-tests`), `make clean` exits 0 and `build/`, `dist/`, `stubs/` are gone — check with `ls` that no read-only pack file remains. Each partial target removes only its directory (`make static-clean` after `make static-debug`). `python scripts/make-clean.py` with no argument exits 2. `make shared` builds into `build/<OS>/shared/Release`.

- [ ] **Step 4: Commit**

```bash
git add scripts/make-clean.py Makefile
git commit -m "build: make clean removes read-only files and every target it names"
```

---

### Task 3: The C++ tests link the `maiacore` target, with one cpptrace

**Files:**
- Modify: `CMakeLists.txt`, `tests-cpp/CMakeLists.txt`, `scripts/make-cpp-tests.py`, `scripts/make-run-cpp-tests.py`, `scripts/run-code-coverage.py`, `Makefile`
- Delete: `maiacore/include/external/cpptrace/` (vendored v0.6.2 headers)

**Interfaces:**
- Produces: CMake option `MAIACORE_BUILD_TESTS` (requires `STATIC_LIB=ON`); test target `cpp-tests`, registered with CTest (working directory = repository root, timeout 1800 s); executable at `build/<OS>/cpp-tests/cpp-tests(.exe)` for single-config generators and `build/<OS>/<dir>/<Config>/cpp-tests.exe` for Visual Studio.

- [ ] **Step 1: Root `CMakeLists.txt`**

Add the option next to the others:
```cmake
option(MAIACORE_BUILD_TESTS "Build the C++ unit tests in tests-cpp (requires STATIC_LIB)" OFF)
```
Link cpptrace and SQLiteCpp publicly for the static and shared libraries, because `log.h` and `score.h` include their headers:
```cmake
if(STATIC_LIB)
    message("=====> GENERATING STATIC LIBRARY <=====")
    add_definitions(-DMAIACORE_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${maiacore_src})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
endif()
```
(same `PUBLIC` for `SHARED_LIB`; the Python module keeps `PRIVATE`). Delete the vendored include line `target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external/cpptrace/include)` and the unused `MAIACORE_LIB_DIR`, `SQLITECPP_LIB_DIR`, `SQLITE_LIB_DIR` variables. At the end of the file:
```cmake
if(MAIACORE_BUILD_TESTS)
    if(NOT STATIC_LIB)
        message(FATAL_ERROR "MAIACORE_BUILD_TESTS requires STATIC_LIB=ON: the tests link the static maiacore library.")
    endif()
    enable_testing()
    add_subdirectory(tests-cpp)
endif()
```
Also correct `message(FALTAL ...)` to `message(FATAL_ERROR ...)`.

- [ ] **Step 2: Rewrite `tests-cpp/CMakeLists.txt` as a subdirectory**

```cmake
# The C++ unit tests. Built only from the repository root with
# -DSTATIC_LIB=ON -DMAIACORE_BUILD_TESTS=ON (see scripts/make-cpp-tests.py): the
# executable links the maiacore target, so every change to the library relinks it,
# and compiler flags, runtime library and dependencies are the library's own.

if(WIN32)
    add_definitions(-DGTEST_HAS_PTHREAD=0)
endif()

# GoogleTest must use the static C runtime that the root CMakeLists.txt selects.
set(gtest_force_shared_crt OFF CACHE BOOL "Use the static C runtime" FORCE)
add_subdirectory(googletest-1.14.0)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES COMPILE_FLAGS "-static")
endif()

add_executable(cpp-tests
    src/main.cpp
    src/helpers-test.cpp
    src/note-test.cpp
    src/chord-test.cpp
    src/interval-test.cpp
    src/score-test.cpp
    src/part-test.cpp
    src/measure-test.cpp
    src/fraction-test.cpp
    src/duration-test.cpp
    src/key-test.cpp
    src/time-signature-test.cpp
    src/clef-test.cpp
    src/score-collection-test.cpp
    src/barline-test.cpp
    src/utils-test.cpp
    src/config-test.cpp
    src/pitch-test.cpp
)

target_link_libraries(cpp-tests PRIVATE maiacore gtest)
if(MSYS OR MINGW)
    target_link_libraries(cpp-tests PRIVATE gtest_main)
endif()
if(UNIX)
    target_link_libraries(cpp-tests PRIVATE dl)
endif()

# Keep the executable at the top of the build directory (per configuration for
# multi-config generators), where the scripts expect it.
set_target_properties(cpp-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# The tests open their fixtures by paths relative to the repository root.
add_test(NAME cpp-tests COMMAND cpp-tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(cpp-tests PROPERTIES TIMEOUT 1800)
```
Check every flag the old file set is still in effect through the root (`-Wall -Wextra`, Linux `--coverage` in Debug, `_ITERATOR_DEBUG_LEVEL=0` for clang on Windows); add `-pedantic` for the test target on non-MSVC compilers if the old build used it.

- [ ] **Step 3: Scripts**

`scripts/make-cpp-tests.py` configures the root project into `build/<OS>/cpp-tests` with `-DSTATIC_LIB=ON -DPYBIND_LIB=OFF -DMAIACORE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=<type> -DCMAKE_CXX_COMPILER=<clang++|g++> -DSQLITECPP_RUN_CPPLINT=OFF` (plus the mingw make program on Windows) and builds the target `cpp-tests`. Before configuring, if `build/<OS>/cpp-tests/CMakeCache.txt` exists with a `CMAKE_HOME_DIRECTORY` that is not the repository root (a cache from the old standalone project), remove that build directory and say so. `scripts/make-run-cpp-tests.py` runs the executable directly with `cwd` = repository root and `timeout=1800` through `run_step`, so gtest's own summary stays visible. `Makefile`: `build-cpp-tests` only calls `make-cpp-tests.py Debug` (no `static-debug`). `scripts/run-code-coverage.py`: object paths move from `build/<OS>/static/Debug/...` to `build/<OS>/cpp-tests/...`.

- [ ] **Step 4: Delete the vendored cpptrace headers**

Confirm nothing references them (`git grep -n "external/cpptrace"` after Step 1 must return nothing), then `git rm -r maiacore/include/external/cpptrace`.

- [ ] **Step 5: Verify**

From a clean `build/`: `make cpp-tests` → 1050/1050, exit 0; the configure log mentions only cpptrace v0.8.2. Touch one maiacore source (e.g. append a blank line to `maiacore/src/maiacore/fraction.cpp`, then restore it) and rebuild: the log shows `Linking CXX executable` and the executable's mtime changes — the stale-binary trap is gone. `make dev` still builds the Python module and `make py-tests` gives 471/471. On Linux (Task 8's gate, or WSL by hand) `make coverage` still finds its object files.

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt tests-cpp/CMakeLists.txt scripts/make-cpp-tests.py scripts/make-run-cpp-tests.py scripts/run-code-coverage.py Makefile
git commit -m "build: the C++ tests link the maiacore target and share its single cpptrace"
```
(the `git rm` from Step 4 is already staged; include it in this commit)

---

### Task 4: `make validate` fails on new findings only

**Files:**
- Modify: `scripts/make-validate.py`
- Create: `scripts/validate-baseline.json`

- [ ] **Step 1: Rewrite `scripts/make-validate.py`**

```python
"""Run cpplint and cppcheck on maiacore's sources and fail only on new findings.

Findings are counted per (tool, file, category), so moving code does not create
"new" findings; a count above the committed baseline (validate-baseline.json)
fails the run. Pass --update-baseline to accept the current findings.
"""

import json
import re
import shutil
import subprocess
import sys
from collections import Counter
from pathlib import Path

from build_utils import usage_error
from terminal_colors import color

SOURCE_DIR = "maiacore/src/maiacore/"
BASELINE = Path(__file__).with_name("validate-baseline.json")
CPPLINT_LINE = re.compile(r"^(?P<file>.+?):\d+:\s.*\[(?P<category>[\w/+-]+)\] \[\d\]$")


def normalize(path: str) -> str:
    return Path(path).as_posix()


def cpplintFindings() -> Counter:
    result = subprocess.run(
        [sys.executable, "-m", "cpplint", "--quiet", "--linelength=100", "--recursive", SOURCE_DIR],
        capture_output=True, text=True,
    )
    if result.returncode not in (0, 1):
        print(result.stderr)
        sys.exit(f"cpplint failed to run (exit code {result.returncode})")
    counts = Counter()
    for line in (result.stdout + result.stderr).splitlines():
        match = CPPLINT_LINE.match(line.strip())
        if match:
            counts[f"cpplint|{normalize(match['file'])}|{match['category']}"] += 1
    return counts


def cppcheckFindings() -> Counter:
    if shutil.which("cppcheck") is None:
        sys.exit("cppcheck not found on PATH")
    result = subprocess.run(
        ["cppcheck", "--quiet", "--template={file}|{id}", SOURCE_DIR], capture_output=True, text=True
    )
    if result.returncode != 0:
        print(result.stderr)
        sys.exit(f"cppcheck failed to run (exit code {result.returncode})")
    counts = Counter()
    for line in result.stderr.splitlines():
        if "|" in line:
            file, category = line.rsplit("|", 1)
            counts[f"cppcheck|{normalize(file)}|{category.strip()}"] += 1
    return counts


args = sys.argv[1:]
if args not in ([], ["--update-baseline"]):
    usage_error("usage: make-validate.py [--update-baseline]")

print(f"{color.OKGREEN}Validating C++ code (cpplint, cppcheck)...{color.ENDC}")
current = cpplintFindings() + cppcheckFindings()

if args == ["--update-baseline"]:
    BASELINE.write_text(json.dumps(dict(sorted(current.items())), indent=2) + "\n", encoding="utf-8")
    print(f"{color.OKGREEN}Baseline updated: {sum(current.values())} findings.{color.ENDC}")
    sys.exit(0)

baseline = Counter(json.loads(BASELINE.read_text(encoding="utf-8")))
new = {key: count - baseline[key] for key, count in current.items() if count > baseline[key]}
fixed = sum((baseline - current).values())

if new:
    print(f"{color.FAIL}New findings (tool|file|category: count above baseline):{color.ENDC}")
    for key, extra in sorted(new.items()):
        print(f"  {key}: +{extra}")
    sys.exit(1)
if fixed:
    print(f"{color.OKGREEN}{fixed} baseline findings are gone; run with --update-baseline to lock that in.{color.ENDC}")
print(f"{color.OKGREEN}Validation: no new findings ({sum(current.values())} in baseline).{color.ENDC}")
```
Add `validate-update-baseline` to the Makefile (`@$(PYTHON) $(SCRIPTS_DIR)/make-validate.py --update-baseline`, with `.PHONY`).

- [ ] **Step 2: Generate and commit the baseline, then prove a new finding fails**

`make validate-update-baseline` writes `scripts/validate-baseline.json`; `make validate` exits 0. Add a scratch cpplint violation to a maiacore source (e.g. a line over 100 characters), run `make validate`: exit 1 naming that file and `whitespace/line_length`; revert the scratch change; `make validate` exits 0 again.

- [ ] **Step 3: Commit**

```bash
git add scripts/make-validate.py scripts/validate-baseline.json Makefile
git commit -m "build: make validate fails on findings that are not in the baseline"
```

---

### Task 5: New fixtures named `test_*.xml` are no longer ignored

**Files:**
- Modify: `.gitignore` (only the `test_*.xml` line — the user's uncommitted `musescore/*` line must stay uncommitted)

- [ ] **Step 1: Measure what the suites write**

On a clean tree run `make cpp-tests` and `make py-tests`; list new files with `git status --porcelain --ignored` before and after. Record which `test_*.xml` files are test outputs and where (the C++ suite writes `test_roundtrip.xml` into the repository root).

- [ ] **Step 2: Replace the unanchored pattern**

Replace `test_*.xml` with root-anchored patterns covering exactly the measured output locations (expected: `/test_*.xml`, plus `/test/test_*.xml` only if the Python suite writes such files into `test/`). Verify: `git check-ignore -v test/xml_examples/unit_test/test_new_fixture.xml` prints nothing (not ignored); `git check-ignore -v test_roundtrip.xml` names the new rule.

- [ ] **Step 3: Stage only this change and commit**

```bash
S=<scratchpad>
git show HEAD:.gitignore > "$S/gitignore.head"
# write "$S/gitignore.ours" = gitignore.head with only the test_*.xml line replaced
blob=$(git hash-object -w "$S/gitignore.ours")
git update-index --cacheinfo 100644,"$blob",.gitignore
git diff --cached .gitignore     # must show only the test_*.xml change
git commit -m "build: ignore test output files only where the suites write them"
git diff .gitignore              # must still show only the user's '+musescore/*'
```
The working-tree `.gitignore` must contain both the new patterns and the user's `musescore/*` line.

---

### Task 6: CI publishes to PyPI only for a published release

**Files:**
- Modify: `.github/workflows/wheels.yml`
- Create: `test/test_ci_workflow.py`

- [ ] **Step 1: Write the failing test**

```python
"""The wheel workflow must publish to PyPI only when a GitHub release is published."""

import unittest
from pathlib import Path

WORKFLOW = Path(__file__).resolve().parents[1] / ".github" / "workflows" / "wheels.yml"
GUARD = "if: github.event_name == 'release' && github.event.action == 'published'"


def jobBlock(text: str, job: str) -> str:
    lines = text.splitlines()
    start = lines.index(f"  {job}:")
    block = []
    for line in lines[start + 1:]:
        if line.startswith("  ") and not line.startswith("    ") and line.strip():
            break
        block.append(line)
    return "\n".join(block)


class WheelWorkflowTestCase(unittest.TestCase):
    def test_upload_job_runs_only_for_a_published_release(self):
        block = jobBlock(WORKFLOW.read_text(encoding="utf-8"), "upload_all")
        self.assertIn(GUARD, block)
        self.assertIn("pypa/gh-action-pypi-publish", block)


if __name__ == "__main__":
    unittest.main()
```
Run: `cd test && python -m unittest test_ci_workflow -v` → FAIL (no guard yet).

- [ ] **Step 2: Add the guard**

In `.github/workflows/wheels.yml`, under `upload_all:` after `needs: build_wheels`:
```yaml
    # Publish to PyPI only when a GitHub release is published; pushes to main and
    # manual runs still build the wheels but never upload them.
    if: github.event_name == 'release' && github.event.action == 'published'
```
Run the test again → PASS. If `actionlint` is available, run it on the workflow.

- [ ] **Step 3: Commit**

```bash
git add .github/workflows/wheels.yml test/test_ci_workflow.py
git commit -m "ci: publish to PyPI only for a published release"
```

---

### Task 7: The true MSVC gate as a script

**Files:**
- Create: `scripts/make-msvc-gate.py`
- Modify: `Makefile` (target `msvc-gate`, Windows only)

- [ ] **Step 1: Write `scripts/make-msvc-gate.py`**

Behaviour (Windows only; exit 2 elsewhere):
1. Find a Visual Studio 2022 instance with the C++ tools: run `vswhere.exe` from `%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\` with `-version [17.0,18.0) -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath` (never `-latest`); `--vs-instance PATH` overrides.
2. Configure `build/Windows/msvc-gate` with `-G "Visual Studio 17 2022" -A x64 -DCMAKE_GENERATOR_INSTANCE=<instance> -DSTATIC_LIB=ON -DPYBIND_LIB=OFF -DMAIACORE_BUILD_TESTS=ON -DSQLITECPP_RUN_CPPLINT=OFF` (arguments as a list — no shell, so `/`-options are never rewritten).
3. For each configuration in `--configs` (default `Debug,Release`): `cmake --build <dir> --config <cfg> --parallel --target cpp-tests`; print `CMAKE_CXX_COMPILER` from the cache (it must be `cl.exe` under the chosen instance); delete `<dir>/<cfg>/cpp-tests.exe` before building and fail if it is missing or older than the build start afterwards; run it with `cwd` = repository root and `timeout=1800`.
4. With `--python`: create a fresh venv (`py -3.12 -m venv build/Windows/msvc-gate/venv`), run `<venv>\Scripts\python -m pip install .` from the repository root with `CMAKE_GENERATOR="Visual Studio 17 2022"` and `CMAKE_GENERATOR_INSTANCE=<instance>` in the environment (setup.py's own MSVC path, as in CI), run `python -m unittest` in `test/`, and `python -c "import maialib; print(maialib.__version__)"` from a temporary directory outside the repository.
5. Every step through `run_step`; print a final summary.

- [ ] **Step 2: Makefile**

```make
msvc-gate:
	@$(PYTHON) $(SCRIPTS_DIR)/make-msvc-gate.py --python
```
(with `.PHONY`).

- [ ] **Step 3: Verify**

`make msvc-gate` → Debug 1050/1050 and Release 1050/1050 with `cl.exe` 19.40 (VS 2022 Community, toolset 14.40.33807), Python 471/471, import OK, exit 0. Prove it can fail: a scratch failing gtest (deleted afterwards) makes it exit non-zero.

- [ ] **Step 4: Commit**

```bash
git add scripts/make-msvc-gate.py Makefile
git commit -m "build: add the MSVC gate — maiacore and its tests compiled by cl, plus the CI-path Python build"
```

---

### Task 8: The Linux (GCC) gate as a script

**Files:**
- Create: `scripts/make-linux-gate.py`
- Modify: `Makefile` (target `linux-gate`)

- [ ] **Step 1: Write `scripts/make-linux-gate.py`**

Behaviour:
1. Export the committed `HEAD` with `git archive --format=tar HEAD` (uncommitted changes are deliberately excluded, as in CI).
2. On Windows, run everything inside WSL (`wsl -e bash -lc "<script>"`, the archive piped to `tar -x -C <dir>` through `input=`); on Linux, run natively in a temporary directory.
3. Check the tools first (`g++`, `cmake`, `make`, `python3`, `python3 -m venv`) and exit 2 listing any that are missing, with the `apt` package names.
4. In the extracted tree: `make cpp-tests`, then `python3 -m venv .venv`, `.venv/bin/python -m pip install -r requirements-dev.txt`, `make dev PYTHON=.venv/bin/python`, `make py-tests PYTHON=.venv/bin/python`, and an import check from outside the tree.
5. Exit with the first failing step's code; print the test summaries.

- [ ] **Step 2: Makefile**

```make
linux-gate:
	@$(PYTHON) $(SCRIPTS_DIR)/make-linux-gate.py
```

- [ ] **Step 3: Verify**

On this machine (WSL Ubuntu): `make linux-gate` → C++ and Python suites pass with GCC, exit 0; or, if WSL lacks tools, a clear list of what to install (report it — installing packages needs the user's sudo).

- [ ] **Step 4: Commit**

```bash
git add scripts/make-linux-gate.py Makefile
git commit -m "build: add the Linux gate — HEAD built and tested with GCC through WSL or natively"
```

---

### Task 9: CHANGELOG and final verification

**Files:**
- Modify: `CHANGELOG.md` (`[Unreleased]`, a build/tooling subsection)

- [ ] **Step 1: CHANGELOG**

Short factual bullets: build and test commands exit non-zero on failure; the C++ tests link the library target (no stale binary) and use one cpptrace (v0.8.2; vendored headers removed); `make clean` targets; `make validate` baseline; `make msvc-gate` / `make linux-gate`; fixtures named `test_*.xml` are committed; CI publishes only for a published release. **Breaking** for contributors only: `tests-cpp` can no longer be configured on its own; `make shared` builds into `build/<OS>/shared/`.

- [ ] **Step 2: Final verification**

From a clean clone state (`make clean`, `rm -rf build` from Bash), brand-new `py -3.12` venv with `requirements-dev.txt`: `make dev`, `make cpp-tests` (1050/1050 + nothing new unless added), `make py-tests` (471 + Task 6's test = 472), `make validate` (exit 0), `make msvc-gate`, `make linux-gate` (or its reported prerequisites). Record every exit code read directly.

- [ ] **Step 3: Commit**

```bash
git add CHANGELOG.md
git commit -m "docs: CHANGELOG entries for the build and test hygiene work"
```
