"""The Linux gate: the committed HEAD built with GCC and tested on Linux, natively or, on Windows,
inside WSL (its default distribution).

Usage: make-linux-gate.py

HEAD is exported with `git archive` into a new directory in /var/tmp (or $TMPDIR) on Linux, so
uncommitted changes are not tested, as in CI. After checking the tools it needs, the gate runs
in the exported tree: `make cpp-tests`; `python3 -m venv .venv`; `.venv/bin/python -m pip
install -r requirements-dev.txt`; `make dev PYTHON=.venv/bin/python`; `make py-tests
PYTHON=.venv/bin/python`; and an import of maialib from outside the tree. The tree is removed
when every step passes and kept for inspection when one fails.

The exit code is 0 when every step passes; otherwise it is the failing command's code, 1 when one
of the gate's own checks fails, or 2 for a usage error, another operating system, a missing WSL
or missing tools, which the gate lists with their apt packages (it installs nothing).
"""

import argparse
import json
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, NoReturn, Optional, Tuple

from build_utils import run_step, usage_error
from terminal_colors import color

REPO_ROOT = Path(__file__).resolve().parent.parent
USE_WSL = platform.system() == "Windows"

# The programs the gate needs on Linux, with the apt packages that provide them. The C compiler
# builds sqlite3 and cpptrace's C dependencies; CMake's FetchContent clones cpptrace with git.
REQUIRED_PROGRAMS = {
    "g++": "g++",
    "gcc": "gcc",
    "make": "make",
    "cmake": "cmake",
    "git": "git",
    "python3": "python3",
    "tar": "tar",
}
VERSIONED_PROGRAMS = ("g++", "cmake", "make", "python3")

# Variables through which the Linux environment could change what the gate builds or runs: CMake
# takes the C compiler and a new build directory's flags, generator and toolchain from CC, CXX,
# CFLAGS, CXXFLAGS, LDFLAGS, CMAKE_GENERATOR* and CMAKE_TOOLCHAIN_FILE; GCC searches the
# directories in CPATH, C_INCLUDE_PATH, CPLUS_INCLUDE_PATH and LIBRARY_PATH; gtest reads its flags
# (a test filter, sharding) from GTEST_*; and PYTHONPATH could shadow the installed package.
IGNORED_VARIABLES = (
    "CC",
    "CXX",
    "CFLAGS",
    "CXXFLAGS",
    "LDFLAGS",
    "CPATH",
    "C_INCLUDE_PATH",
    "CPLUS_INCLUDE_PATH",
    "LIBRARY_PATH",
    "CMAKE_TOOLCHAIN_FILE",
    "PYTHONPATH",
)
IGNORED_PREFIXES = ("CMAKE_GENERATOR", "GTEST_")
# A make that runs this script passes its options and its recursion level to its children in
# these. The exported tree is built by a top-level make, as from a terminal, so they are dropped
# as well, without a notice.
MAKE_VARIABLES = ("MAKEFLAGS", "MFLAGS", "MAKELEVEL")

HEADERS_CHECK = (
    "import os, sysconfig; "
    'include = sysconfig.get_paths()["include"]; '
    'print("headers=" + ("ok" if os.path.isfile(os.path.join(include, "Python.h")) else ""))'
)
IMPORT_CHECK = (
    "import pathlib, sys, maialib; "
    "print(maialib.__version__, maialib.__file__); "
    "sys.exit(maialib.__version__ != pathlib.Path(sys.argv[1]).read_text().strip())"
)

# One line per finished part of the gate, printed at the end (also when a later step fails).
summary: List[str] = []
# The `unset ...; ` that each step's shell runs first (see IGNORED_VARIABLES and MAKE_VARIABLES).
unset_command = ""
# The temporary directory on Linux that holds the exported tree, once it exists.
gate_dir: Optional[str] = None


def check_failed(message: str) -> NoReturn:
    """Report a failed check of the gate itself and exit with code 1."""
    print(f"{color.FAIL}Linux gate check failed: {message}{color.ENDC}")
    sys.exit(1)


def bash(script: str) -> List[str]:
    """Return the command that runs ``script`` with bash on Linux.

    In WSL it is a login shell, so PATH is the one the distribution's profile sets up (with
    ~/.local/bin, for example), as in a terminal; on Linux the script inherits this environment.
    """
    if USE_WSL:
        return ["wsl", "-e", "bash", "-lc", script]
    return ["bash", "-c", script]


def capture(
    command: List[str], step_name: str, cwd: Optional[Path] = None, stdin: Optional[bytes] = None
) -> Tuple[int, bytes]:
    """Run ``command`` with ``stdin`` as its input; return its exit code and standard output.

    A missing program ends the gate with code 127 and any other failure to start it with 126, as
    in run_step(); a non-zero exit code is left to the caller.
    """
    sys.stdout.flush()
    sys.stderr.flush()
    try:
        result = subprocess.run(command, cwd=cwd, input=stdin, stdout=subprocess.PIPE, check=False)
    except FileNotFoundError:
        print(f"{color.FAIL}Step failed: {step_name} (command not found: {command[0]}){color.ENDC}")
        sys.exit(127)
    except OSError as error:
        print(f"{color.FAIL}Step failed: {step_name} (could not start: {error}){color.ENDC}")
        sys.exit(126)
    return result.returncode, result.stdout


def step_failed(step_name: str, code: int, output: str) -> NoReturn:
    """Print a failed command's captured ``output`` and exit with its ``code``."""
    if output.strip():
        print(output.rstrip())
    print(f"{color.FAIL}Step failed: {step_name} (exit code {code}){color.ENDC}")
    sys.exit(code)


def git_output(arguments: List[str], step_name: str) -> str:
    """Run git in the repository and return its standard output; failures exit as in run_step()."""
    code, output = capture(["git", *arguments], step_name, cwd=REPO_ROOT)
    text = output.decode("utf-8", errors="replace")
    if code != 0:
        step_failed(step_name, code, text)
    return text


def linux_output(script: str, step_name: str) -> str:
    """Run ``script`` on Linux and return its standard output; failures exit as in run_step()."""
    code, output = capture(bash(script), step_name)
    text = output.decode("utf-8", errors="replace")
    if code != 0:
        step_failed(step_name, code, text)
    return text


def run_in(directory: str, command: str, step_name: str) -> None:
    """Run the shell ``command`` in ``directory`` on Linux without the ignored variables; a
    failure ends the gate with the command's exit code (see run_step())."""
    run_step(bash(f"{unset_command}cd {shlex.quote(directory)} || exit; {command}"), step_name)


def probe_linux() -> Dict[str, str]:
    """Describe Linux as {key: value}.

    The keys are "program:<name>" (its path, empty when it is missing), "venv" and "headers"
    ("ok" when python3 creates a virtual environment with pip and has its C headers),
    "version:<name>", "system", and "env:<name>" for every exported variable.
    """
    script = "\n".join(
        [
            f"for p in {' '.join(REQUIRED_PROGRAMS)}; do",
            '  echo "program:$p=$(command -v "$p")"',
            "done",
            f"for p in {' '.join(VERSIONED_PROGRAMS)}; do",
            '  command -v "$p" >/dev/null && echo "version:$p=$("$p" --version 2>&1 | head -n 1)"',
            "done",
            "if command -v python3 >/dev/null; then",
            '  scratch=$(mktemp -d) && python3 -m venv "$scratch/venv" >/dev/null 2>&1 && '
            "echo venv=ok",
            '  rm -rf "$scratch"',
            f"  python3 -c {shlex.quote(HEADERS_CHECK)}",
            "fi",
            'echo "system=$(. /etc/os-release 2>/dev/null; echo "$PRETTY_NAME")"',
            "compgen -e | sed 's/^/env:/; s/$/=/'",
            "exit 0",
        ]
    )
    code, output = capture(bash(script), "check the tools on Linux")
    text = output.decode("utf-8", errors="replace")
    if code != 0:
        if text.strip():
            print(text.rstrip())
        hint = (
            ": install a Linux distribution with `wsl --install -d Ubuntu`, or make the one to "
            "use WSL's default with `wsl --set-default <name>`"
            if USE_WSL
            else ""
        )
        usage_error(f"could not run bash on Linux (exit code {code}){hint}")
    facts = {}
    for line in text.splitlines():
        key, separator, value = line.partition("=")
        if separator:
            facts[key.strip()] = value.strip()
    return facts


def missing_prerequisites(facts: Dict[str, str]) -> List[Tuple[str, str]]:
    """Return (what is missing, apt package) for everything the gate needs that Linux lacks."""
    missing = [
        (program, package)
        for program, package in REQUIRED_PROGRAMS.items()
        if not facts.get(f"program:{program}")
    ]
    has_python = bool(facts.get("program:python3"))
    if not has_python or facts.get("venv") != "ok":
        missing.append(("python3 -m venv, a virtual environment with pip", "python3-venv"))
    if not has_python or facts.get("headers") != "ok":
        missing.append(("Python.h, the Python C headers the module is built with", "python3-dev"))
    return missing


def ignore_environment_overrides(facts: Dict[str, str]) -> None:
    """Set the `unset` that each step runs first, announcing the variables it drops."""
    global unset_command
    exported = [key[len("env:") :] for key in facts if key.startswith("env:")]
    ignored = sorted(
        name for name in exported if name in IGNORED_VARIABLES or name.startswith(IGNORED_PREFIXES)
    )
    for name in ignored:
        print(f"{color.WARNING}Ignoring {name} from the environment.{color.ENDC}")
    dropped = ignored + [name for name in exported if name in MAKE_VARIABLES]
    unset_command = f"unset {' '.join(dropped)}; " if dropped else ""


def export_head() -> Tuple[str, bytes]:
    """Return the short name of HEAD and HEAD as a tar archive.

    git archive writes line endings as a checkout would, so on Windows (core.autocrlf=true, the
    Git for Windows default) the Linux build would get CRLF files. The gate asks for LF, as a
    Linux checkout, and CI, has them.
    """
    commit = git_output(["rev-parse", "--short", "HEAD"], "git rev-parse HEAD").strip()
    if git_output(["status", "--porcelain"], "git status").strip():
        print(
            f"{color.WARNING}The working tree has uncommitted changes: the gate tests the "
            f"committed HEAD ({commit}) only.{color.ENDC}"
        )
    command = ["git", "-c", "core.autocrlf=false", "-c", "core.eol=lf", "archive", "--format=tar"]
    code, archive = capture([*command, "HEAD"], "git archive HEAD", cwd=REPO_ROOT)
    if code != 0:
        step_failed("git archive HEAD", code, "")
    return commit, archive


def extract(archive: bytes) -> str:
    """Extract ``archive`` into a new temporary directory on Linux; return the tree's path.

    The directory is made in /var/tmp, which a restart keeps, unlike /tmp: WSL stops the Linux
    instance soon after its last process exits, and a tree kept after a failure must outlast it.

    The files get the time of the extraction (tar -m), not the commit's: when the Linux clock is
    behind the one that made the commit (WSL's lags the host's), a recent commit's files would be
    in the future, and make warns that the build may be incomplete.
    """
    global gate_dir
    script = (
        'dir=$(mktemp -d "${TMPDIR:-/var/tmp}/maialib-linux-gate.XXXXXX") || exit; '
        'echo "gate-dir=$dir"; mkdir "$dir/maialib" && tar -x -m -C "$dir/maialib"'
    )
    code, output = capture(bash(script), "extract HEAD on Linux", stdin=archive)
    text = output.decode("utf-8", errors="replace")
    for line in text.splitlines():
        if line.startswith("gate-dir="):
            gate_dir = line[len("gate-dir=") :].strip()
    if code != 0 or not gate_dir:
        step_failed("extract HEAD on Linux", code or 1, text)
    print(f"{color.OKGREEN}Linux gate: HEAD extracted into {gate_dir}/maialib{color.ENDC}")
    return f"{gate_dir}/maialib"


def gcc_compiler(build_dir: str) -> str:
    """Fail unless CMake found GCC as the C++ compiler for ``build_dir``; describe it."""
    text = linux_output(
        f"cat {shlex.quote(build_dir)}/CMakeFiles/*/CMakeCXXCompiler.cmake",
        f"read the C++ compiler that CMake found for {build_dir}",
    )
    values = dict(
        re.findall(r'^set\((CMAKE_CXX_COMPILER(?:_ID|_VERSION)?) "([^"]*)"\)', text, re.M)
    )
    path = values.get("CMAKE_CXX_COMPILER", "")
    compiler_id = values.get("CMAKE_CXX_COMPILER_ID", "")
    if compiler_id != "GNU":
        check_failed(
            f"{build_dir} compiles C++ with '{path}' ({compiler_id or 'unknown'}), not GCC"
        )
    return f"GCC {values.get('CMAKE_CXX_COMPILER_VERSION', '')} ({path})"


def gtest_result(report: str) -> str:
    """Summarise a gtest JSON report, e.g. "1047/1050 passed, 3 skipped"; fail if no test ran."""
    try:
        data = json.loads(linux_output(f"cat {shlex.quote(report)}", "read the C++ test report"))
        cases = [case for suite in data["testsuites"] for case in suite["testsuite"]]
    except (ValueError, KeyError, TypeError) as error:
        check_failed(f"cannot read the test report {report}: {error!r}")
    ran = [case for case in cases if case.get("status") == "RUN"]
    skipped = sum(1 for case in ran if case.get("result") == "SKIPPED")
    failed = sum(1 for case in ran if case.get("failures"))
    if not ran or failed:
        check_failed(f"{report} records {len(ran)} tests run and {failed} failed")
    result = f"{len(ran) - skipped}/{len(ran)} passed"
    return f"{result}, {skipped} skipped" if skipped else result


def unittest_result(log: str) -> str:
    """Summarise the unittest output saved in ``log``, e.g. "472 tests, OK (skipped=2)"."""
    text = linux_output(f"cat {shlex.quote(log)}", "read the Python test output")
    ran = re.findall(r"^Ran (\d+) tests? in ", text, re.M)
    status = re.findall(r"^((?:OK|FAILED)\b.*?)\s*$", text, re.M)
    if not ran or not status:
        check_failed(f"{log} holds no unittest summary")
    return f"{ran[-1]} tests, {status[-1]}"


def remove_gate_dir() -> None:
    """Delete the temporary directory on Linux; a failure only prints a warning."""
    code, _ = capture(bash(f"rm -rf {shlex.quote(str(gate_dir))}"), "remove the exported tree")
    if code != 0:
        print(f"{color.WARNING}Could not remove {gate_dir} (exit code {code}).{color.ENDC}")


def print_summary(passed: bool) -> None:
    print(f"\n{color.BOLD}Linux gate summary{color.ENDC}")
    for line in summary:
        print(f"  {line}")
    if passed:
        print(f"{color.OKGREEN}Linux gate: passed{color.ENDC}")
    else:
        print(f"{color.FAIL}Linux gate: FAILED (see the failed step above){color.ENDC}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build the committed HEAD with GCC on Linux (in WSL on Windows) and test it."
    )
    parser.parse_args()

    if platform.system() not in ("Linux", "Windows"):
        usage_error("the Linux gate runs on Linux, or on Windows inside WSL")
    if USE_WSL:
        if shutil.which("wsl") is None:
            usage_error("WSL is not installed: install it with a distribution, `wsl --install`")
        # wsl.exe writes its own messages in UTF-16 unless this is set.
        os.environ["WSL_UTF8"] = "1"

    facts = probe_linux()
    system = facts.get("system") or "Linux"
    where = f"{system} (in WSL)" if USE_WSL else system
    missing = missing_prerequisites(facts)
    if missing:
        print(f"{color.FAIL}{where} lacks what the Linux gate needs:{color.ENDC}")
        for what, package in missing:
            print(f"  {what} (apt package: {package})")
        packages = " ".join(dict.fromkeys(package for _, package in missing))
        usage_error(f"install them with `sudo apt install {packages}`, then run the gate again")
    print(f"{color.OKGREEN}Linux gate: {where}{color.ENDC}")
    ignore_environment_overrides(facts)
    summary.append(f"Linux: {where}")
    versions = [facts.get(f"version:{program}", "") for program in VERSIONED_PROGRAMS]
    summary.append(f"Tools: {'; '.join(version for version in versions if version)}")

    commit, archive = export_head()
    summary.append(f"Commit: {commit} (HEAD)")
    tree = extract(archive)

    # gtest writes a JSON report next to the tree; the summary counts the tests from it.
    report = f"{gate_dir}/cpp-tests.json"
    run_in(tree, f"GTEST_OUTPUT=json:{shlex.quote(report)} make cpp-tests", "make cpp-tests")
    cpp_compiler = gcc_compiler(f"{tree}/build/Linux/cpp-tests")
    summary.append(f"C++ tests (Debug): {gtest_result(report)}, compiled by {cpp_compiler}")

    run_in(tree, "python3 -m venv .venv", "python3 -m venv .venv")
    run_in(
        tree,
        ".venv/bin/python -m pip install -r requirements-dev.txt",
        "pip install -r requirements-dev.txt",
    )
    run_in(tree, "make dev PYTHON=.venv/bin/python", "make dev")
    module_compiler = gcc_compiler(f"{tree}/build/Linux/module")
    # tee shows the output as it comes and keeps it for the summary; pipefail keeps make's code.
    log = f"{gate_dir}/py-tests.log"
    run_in(
        tree,
        f"set -o pipefail; make py-tests PYTHON=.venv/bin/python 2>&1 | tee {shlex.quote(log)}",
        "make py-tests",
    )
    python_tests = unittest_result(log)
    # From an empty directory outside the tree, whose maialib/ would shadow the installed package.
    python = shlex.quote(f"{tree}/.venv/bin/python")
    version_file = shlex.quote(f"{tree}/VERSION")
    run_in(
        str(gate_dir),
        f"mkdir outside && cd outside && {python} -c {shlex.quote(IMPORT_CHECK)} {version_file}",
        "import maialib from outside the tree",
    )
    version = linux_output(f"cat {version_file}", "read VERSION").strip()
    summary.append(
        f"{facts.get('version:python3', 'Python')}: make dev compiled by {module_compiler}; "
        f"unit tests: {python_tests}; import OK (maialib {version})"
    )
    remove_gate_dir()


if __name__ == "__main__":
    try:
        main()
    except SystemExit as stop:
        if stop.code not in (None, 0):
            if summary:
                print_summary(passed=False)
            if gate_dir:
                remove = f"wsl -e rm -rf {gate_dir}" if USE_WSL else f"rm -rf {gate_dir}"
                print(
                    f"{color.WARNING}The exported tree is kept for inspection in {gate_dir}"
                    f"{' (in WSL)' if USE_WSL else ''}; remove it with `{remove}`.{color.ENDC}"
                )
        raise
    print_summary(passed=True)
