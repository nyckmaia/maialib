"""The MSVC gate: maiacore and its C++ unit tests compiled by cl from Visual Studio 2022 and run
in each configuration; with --python, also the Python package built by `pip install .` through
setup.py's MSVC path (the path CI takes), tested in a fresh virtual environment.

Usage: make-msvc-gate.py [--vs-instance PATH] [--configs Debug,Release] [--python]

Windows only. The exit code is 0 when every step passes; otherwise it is the failing command's
code, 1 when one of the gate's own checks fails, or 2 for a usage error, another operating
system or a missing Visual Studio 2022.
"""

import argparse
import json
import os
import platform
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import Dict, List, NoReturn, Tuple

from build_utils import normalize_build_type, run_step, usage_error
from terminal_colors import color

REPO_ROOT = Path(__file__).resolve().parent.parent
# Kept short: MSVC's compiler checks fail (C1083) when the build tree exceeds MAX_PATH.
BUILD_DIR = REPO_ROOT / "build" / "Windows" / "msvc-gate"
VENV_DIR = BUILD_DIR / "venv"

GENERATOR = "Visual Studio 17 2022"
ARCHITECTURE = "x64"
VS_VERSIONS = "[17.0,18.0)"
VC_TOOLS = "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
PYTHON_VERSION = "3.12"
# Seconds; the timeout of the CTest registration of cpp-tests.
TEST_TIMEOUT = 1800

# Variables through which the calling shell could change what the gate builds or runs: cl and
# link take extra options from CL, _CL_, LINK and _LINK_; CMake seeds a new build directory's
# flags, generator and toolchain from CFLAGS, CXXFLAGS, LDFLAGS, CMAKE_GENERATOR* and
# CMAKE_TOOLCHAIN_FILE; setup.py reads CMAKE_ARGS and DEBUG; gtest reads its flags (a test
# filter, sharding) from GTEST_*; and PYTHONPATH could shadow the installed package.
IGNORED_VARIABLES = (
    "CL",
    "_CL_",
    "LINK",
    "_LINK_",
    "CFLAGS",
    "CXXFLAGS",
    "LDFLAGS",
    "CMAKE_TOOLCHAIN_FILE",
    "CMAKE_ARGS",
    "DEBUG",
    "PYTHONPATH",
)
IGNORED_PREFIXES = ("CMAKE_GENERATOR", "GTEST_")

# One line per finished part of the gate, printed at the end (also when a later step fails).
summary: List[str] = []


def check_failed(message: str) -> NoReturn:
    """Report a failed check of the gate itself and exit with code 1."""
    print(f"{color.FAIL}MSVC gate check failed: {message}{color.ENDC}")
    sys.exit(1)


def query(command: List[str], step_name: str) -> str:
    """Run a command and return its standard output; failures exit as in run_step()."""
    sys.stdout.flush()
    try:
        result = subprocess.run(
            command, stdout=subprocess.PIPE, encoding="utf-8", errors="replace", check=False
        )
    except FileNotFoundError:
        print(f"{color.FAIL}Step failed: {step_name} (command not found: {command[0]}){color.ENDC}")
        sys.exit(127)
    except OSError as error:
        print(f"{color.FAIL}Step failed: {step_name} (could not start: {error}){color.ENDC}")
        sys.exit(126)
    if result.returncode != 0:
        print(result.stdout, end="")
        print(f"{color.FAIL}Step failed: {step_name} (exit code {result.returncode}){color.ENDC}")
        sys.exit(result.returncode)
    return result.stdout


def same_path(first: str, second: str) -> bool:
    return os.path.normcase(os.path.abspath(first)) == os.path.normcase(os.path.abspath(second))


def is_inside(path: str, directory: str) -> bool:
    inner = os.path.normcase(os.path.abspath(path))
    return inner.startswith(os.path.normcase(os.path.abspath(directory)).rstrip(os.sep) + os.sep)


def remove_tree(path: Path) -> None:
    """Delete a directory tree, read-only files included (the git packs of fetched sources)."""

    def make_writable_and_retry(function, target, _error):
        os.chmod(target, stat.S_IWRITE)
        function(target)

    try:
        if sys.version_info >= (3, 12):
            shutil.rmtree(path, onexc=make_writable_and_retry)
        else:
            shutil.rmtree(path, onerror=make_writable_and_retry)
    except OSError as error:
        check_failed(f"could not remove {path}: {error}")


def ignore_environment_overrides() -> None:
    for name in sorted(os.environ):
        if name.upper() in IGNORED_VARIABLES or name.upper().startswith(IGNORED_PREFIXES):
            print(f"{color.WARNING}Ignoring {name} from the environment.{color.ENDC}")
            del os.environ[name]


def find_visual_studio_2022() -> str:
    """Return the installation path of a Visual Studio 2022 instance with the x64 C++ tools."""
    installer = Path(os.environ.get("PROGRAMFILES(X86)", r"C:\Program Files (x86)"))
    vswhere = installer / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not vswhere.is_file():
        usage_error(
            f"{vswhere} not found: install Visual Studio 2022 with the C++ tools, "
            "or pass --vs-instance PATH"
        )
    # The version range keeps newer Visual Studio versions out; -sort lists the matching
    # instances newest first, so the first one is used and the others can be named.
    output = query(
        [
            str(vswhere),
            "-version",
            VS_VERSIONS,
            "-products",
            "*",
            "-requires",
            VC_TOOLS,
            "-property",
            "installationPath",
            "-sort",
            "-utf8",
        ],
        "find Visual Studio 2022",
    )
    instances = [line.strip() for line in output.splitlines() if line.strip()]
    if not instances:
        usage_error(
            f"no Visual Studio 2022 instance with the x64 C++ tools ({VC_TOOLS}) was found: "
            "install them, or pass --vs-instance PATH"
        )
    for other in instances[1:]:
        print(f"{color.WARNING}Also found {other} (select it with --vs-instance).{color.ENDC}")
    return instances[0]


def read_cmake_cache(cache: Path) -> Dict[str, str]:
    """Return the entries of a CMakeCache.txt as {name: value}."""
    entries = {}
    for line in cache.read_text(encoding="utf-8", errors="replace").splitlines():
        match = re.match(r"([^#/][^:=]*):[^=]*=(.*)", line)
        if match:
            entries[match.group(1)] = match.group(2)
    return entries


def detected_compiler(build_dir: Path) -> Tuple[str, str]:
    """Return the path and version of the C++ compiler that CMake found for ``build_dir``.

    Visual Studio generators leave CMAKE_CXX_COMPILER out of CMakeCache.txt; CMake records the
    compiler it detected in CMakeFiles/<CMake version>/CMakeCXXCompiler.cmake instead.
    """
    entries = read_cmake_cache(build_dir / "CMakeCache.txt")
    version = ".".join(
        entries.get(f"CMAKE_CACHE_{part}_VERSION", "") for part in ("MAJOR", "MINOR", "PATCH")
    )
    compiler_file = build_dir / "CMakeFiles" / version / "CMakeCXXCompiler.cmake"
    if not compiler_file.is_file():
        found = sorted(
            (build_dir / "CMakeFiles").glob("*/CMakeCXXCompiler.cmake"),
            key=lambda path: path.stat().st_mtime,
        )
        if not found:
            check_failed(f"CMake recorded no C++ compiler in {build_dir / 'CMakeFiles'}")
        compiler_file = found[-1]
    text = compiler_file.read_text(encoding="utf-8", errors="replace")
    values = dict(re.findall(r'^set\((CMAKE_CXX_COMPILER(?:_VERSION)?) "([^"]*)"\)', text, re.M))
    return values.get("CMAKE_CXX_COMPILER", ""), values.get("CMAKE_CXX_COMPILER_VERSION", "")


def check_compiler(build_dir: Path, instance: str) -> str:
    """Fail unless ``build_dir`` compiles C++ with cl.exe from ``instance``; describe it."""
    path, version = detected_compiler(build_dir)
    print(f"CMAKE_CXX_COMPILER for {build_dir}: {path} (version {version})")
    if Path(path).name.lower() != "cl.exe" or not is_inside(path, instance):
        check_failed(f"{build_dir} compiles C++ with '{path}', not with cl.exe from {instance}")
    return f"cl {version} ({path})"


def gtest_result(report: Path) -> str:
    """Summarise a gtest JSON report, e.g. "1050/1050 passed"; fail if no test ran."""
    try:
        data = json.loads(report.read_text(encoding="utf-8"))
        cases = [case for suite in data["testsuites"] for case in suite["testsuite"]]
    except (OSError, ValueError, KeyError, TypeError) as error:
        check_failed(f"cannot read the test report {report}: {error!r}")
    ran = [case for case in cases if case.get("status") == "RUN"]
    skipped = sum(1 for case in ran if case.get("result") == "SKIPPED")
    failed = sum(1 for case in ran if case.get("failures"))
    if not ran or failed:
        check_failed(f"{report} records {len(ran)} tests run and {failed} failed")
    result = f"{len(ran) - skipped}/{len(ran)} passed"
    return f"{result}, {skipped} skipped" if skipped else result


def configure(instance: str) -> None:
    """Configure BUILD_DIR for Visual Studio 2022 with the static library and the C++ tests.

    CMake cannot switch a build directory to another source tree, generator, platform or
    Visual Studio instance, so a directory configured for other ones is removed first.
    """
    cache = BUILD_DIR / "CMakeCache.txt"
    if cache.is_file():
        entries = read_cmake_cache(cache)
        if not (
            same_path(entries.get("CMAKE_HOME_DIRECTORY", ""), str(REPO_ROOT))
            and entries.get("CMAKE_GENERATOR") == GENERATOR
            and entries.get("CMAKE_GENERATOR_PLATFORM") == ARCHITECTURE
            and same_path(entries.get("CMAKE_GENERATOR_INSTANCE", ""), instance)
        ):
            print(
                f"{color.WARNING}{BUILD_DIR} was configured for another source tree, generator, "
                f"platform or Visual Studio instance: removing it.{color.ENDC}"
            )
            remove_tree(BUILD_DIR)
    # An argument list, not a shell command, so no shell rewrites the /-style MSVC options.
    run_step(
        [
            "cmake",
            "-S",
            str(REPO_ROOT),
            "-B",
            str(BUILD_DIR),
            "-G",
            GENERATOR,
            "-A",
            ARCHITECTURE,
            f"-DCMAKE_GENERATOR_INSTANCE={instance}",
            "-DSTATIC_LIB=ON",
            "-DPYBIND_LIB=OFF",
            "-DMAIACORE_BUILD_TESTS=ON",
            "-DSQLITECPP_RUN_CPPLINT=OFF",
        ],
        "CMake configure (Visual Studio 2022)",
    )


def build_and_test(config: str, instance: str) -> None:
    """Build cpp-tests in ``config``, prove the executable is new, and run it."""
    executable = BUILD_DIR / config / "cpp-tests.exe"
    report = BUILD_DIR / config / "cpp-tests-report.json"
    for stale in (executable, report):
        try:
            stale.unlink(missing_ok=True)
        except OSError as error:
            check_failed(f"could not delete {stale}: {error}")

    started = time.time()
    run_step(
        [
            "cmake",
            "--build",
            str(BUILD_DIR),
            "--config",
            config,
            "--parallel",
            "--target",
            "cpp-tests",
        ],
        f"build cpp-tests ({config})",
    )
    if not executable.is_file():
        check_failed(f"the {config} build did not produce {executable}")
    if executable.stat().st_mtime < started:
        check_failed(f"{executable} is older than the {config} build")
    compiler = check_compiler(BUILD_DIR, instance)

    # The tests open their fixtures by paths relative to the repository root.
    run_step(
        [str(executable), f"--gtest_output=json:{report}"],
        f"C++ unit tests ({config})",
        cwd=str(REPO_ROOT),
        timeout=TEST_TIMEOUT,
    )
    summary.append(f"{config}: C++ tests {gtest_result(report)}, compiled by {compiler}")


def remove_setuptools_build_dirs() -> None:
    """Delete setuptools' build directories in the source tree (build/temp.*, lib.*, bdist.*)."""
    for pattern in ("temp.*", "lib.*", "bdist.*"):
        for path in (REPO_ROOT / "build").glob(pattern):
            remove_tree(path)


def python_build_and_test(instance: str) -> None:
    """Install the package with pip into a fresh virtual environment, then test and import it."""
    python = VENV_DIR / "Scripts" / "python.exe"
    run_step(
        ["py", f"-{PYTHON_VERSION}", "-m", "venv", "--clear", str(VENV_DIR)],
        f"create a fresh Python {PYTHON_VERSION} virtual environment",
    )
    # pip builds in the source tree, where setup.py's CMake build lands in build/temp.*. CI
    # starts without it, and CMake refuses one made by an earlier build with another generator.
    remove_setuptools_build_dirs()
    os.environ["CMAKE_GENERATOR"] = GENERATOR
    os.environ["CMAKE_GENERATOR_INSTANCE"] = instance
    try:
        run_step(
            [str(python), "-m", "pip", "install", "."],
            "pip install . (setup.py with Visual Studio 2022)",
            cwd=str(REPO_ROOT),
        )
        pip_builds = [
            cache.parent
            for cache in (REPO_ROOT / "build").glob("temp.*/**/CMakeCache.txt")
            if same_path(read_cmake_cache(cache).get("CMAKE_HOME_DIRECTORY", ""), str(REPO_ROOT))
        ]
        if not pip_builds:
            check_failed("found no CMake build of the module under build/temp.*")
        compilers = [check_compiler(build, instance) for build in pip_builds]
    finally:
        # Also removed afterwards, or a later `pip install .` would reuse this build and its
        # Visual Studio 2022 generator.
        remove_setuptools_build_dirs()

    run_step([str(python), "-m", "unittest"], "Python unit tests", cwd=str(REPO_ROOT / "test"))

    version = (REPO_ROOT / "VERSION").read_text(encoding="utf-8").strip()
    import_check = (
        "import sys, maialib; print(maialib.__version__, maialib.__file__); "
        f"sys.exit(maialib.__version__ != {version!r})"
    )
    # From outside the repository, whose maialib/ directory would shadow the installed package.
    with tempfile.TemporaryDirectory() as outside:
        run_step([str(python), "-c", import_check], "import maialib", cwd=outside)
    summary.append(
        f"Python {venv_python_version()}: pip install . compiled by {', '.join(compilers)}; "
        f"unit tests passed; import OK (maialib {version})"
    )


def venv_python_version() -> str:
    """Return the full Python version recorded in the virtual environment's pyvenv.cfg."""
    for line in (VENV_DIR / "pyvenv.cfg").read_text(encoding="utf-8").splitlines():
        key, _, value = line.partition("=")
        if key.strip() == "version":
            return value.strip()
    return PYTHON_VERSION


def print_summary(passed: bool) -> None:
    print(f"\n{color.BOLD}MSVC gate summary{color.ENDC}")
    for line in summary:
        print(f"  {line}")
    if passed:
        print(f"{color.OKGREEN}MSVC gate: passed{color.ENDC}")
    else:
        print(f"{color.FAIL}MSVC gate: FAILED (see the failed step above){color.ENDC}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build maiacore and its C++ tests with Visual Studio 2022 (cl) and run them."
    )
    parser.add_argument(
        "--vs-instance",
        metavar="PATH",
        help="Visual Studio 2022 installation to use (default: found with vswhere)",
    )
    parser.add_argument(
        "--configs",
        default="Debug,Release",
        help="comma-separated configurations to build and test (default: Debug,Release)",
    )
    parser.add_argument(
        "--python",
        action="store_true",
        help="also build the package with `pip install .` and run the Python tests",
    )
    args = parser.parse_args()

    if platform.system() != "Windows":
        usage_error("the MSVC gate runs only on Windows")
    configs = list(
        dict.fromkeys(
            normalize_build_type(item.strip()) for item in args.configs.split(",") if item.strip()
        )
    )
    if not configs:
        usage_error("--configs names no configuration")

    ignore_environment_overrides()
    if args.vs_instance:
        instance = os.path.abspath(args.vs_instance)
        if not os.path.isdir(instance):
            usage_error(f"--vs-instance: {instance} is not a directory")
    else:
        instance = find_visual_studio_2022()
    print(f"{color.OKGREEN}MSVC gate: Visual Studio at {instance}{color.ENDC}")
    summary.append(f"Visual Studio: {instance}")

    configure(instance)
    for config in configs:
        build_and_test(config, instance)
    if args.python:
        python_build_and_test(instance)


if __name__ == "__main__":
    try:
        main()
    except SystemExit as stop:
        if stop.code not in (None, 0) and summary:
            print_summary(passed=False)
        raise
    print_summary(passed=True)
