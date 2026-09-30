"""Run cpplint and cppcheck on maiacore's sources and fail only on new findings.

Findings are counted per (tool, file, category), so moving code does not create
"new" findings; a count above the committed baseline (validate-baseline.json)
fails the run. Pass --update-baseline to accept the current findings.

Both tools run from the repository root on a relative path and every path is
written with forward slashes, so the same findings give the same baseline on
every platform. Exit codes: 0 no new findings, 1 new findings, 2 the check could
not run (bad arguments, no baseline, or a tool that is missing, fails or prints
something that is not a finding).
"""

import json
import re
import shutil
import subprocess
import sys
from collections import Counter
from pathlib import Path
from typing import List, NoReturn, Tuple

from build_utils import usage_error
from terminal_colors import color

ROOT = Path(__file__).resolve().parent.parent
SOURCE_DIR = "maiacore/src/maiacore/"
BASELINE = Path(__file__).with_name("validate-baseline.json")
# cpplint writes each finding to stderr as "<file>:<line>:  <message>  [<category>] [<confidence>]"
# and its progress to stdout. It exits with 1 both when it finds something and when it cannot
# run at all (bad arguments, cpplint not installed), so any other line on stderr is an error.
# Python prints its own warnings to stderr as well (Python 3.14 deprecates the codecs.open()
# calls in cpplint 2.0.2), so cpplint runs with -W ignore, which overrides PYTHONWARNINGS and
# the development mode (-X dev, PYTHONDEVMODE).
CPPLINT_LINE = re.compile(r"^(?P<file>.+?):\d+:\s.*\[(?P<category>[\w/+-]+)\] \[\d\]$")
# cppcheck writes findings to stderr in this format and its own errors to stdout. It exits with
# 0 whether or not it finds anything, and with 1 when it cannot run.
CPPCHECK_TEMPLATE = "{file}|{severity}|{id}|{line}|{message}"

# (the "tool|file|category" key, the tool's own description of the finding)
Finding = Tuple[str, str]


def normalize(path: str) -> str:
    """Return ``path`` with forward slashes; both tools print backslashes on Windows."""
    return Path(path).as_posix()


def cannot_run(message: str, output: str = "") -> NoReturn:
    """Print a tool's ``output`` and ``message``, then exit with code 2."""
    if output.strip():
        print(output.rstrip())
    print(f"{color.FAIL}[ERROR] {message}{color.ENDC}")
    sys.exit(2)


def run(command: List[str]) -> subprocess.CompletedProcess:
    return subprocess.run(command, cwd=ROOT, capture_output=True, text=True, errors="replace")


def cpplint_findings() -> List[Finding]:
    # maiacore/CPPLINT.cfg filters out every whitespace/* category, so --linelength has no
    # effect there: the 100-column limit is applied by clang-format (make format-cpp).
    result = run(
        [sys.executable, "-W", "ignore", "-m", "cpplint"]
        + ["--quiet", "--linelength=100", "--recursive", SOURCE_DIR]
    )
    lines = [line.strip() for line in result.stderr.splitlines() if line.strip()]
    matches = [CPPLINT_LINE.match(line) for line in lines]
    unexpected = [line for line, match in zip(lines, matches) if match is None]
    if unexpected or result.returncode != (1 if lines else 0):
        cannot_run(
            f"cpplint did not run cleanly (exit code {result.returncode})",
            "\n".join(unexpected) or result.stdout + result.stderr,
        )
    return [
        (f"cpplint|{normalize(match['file'])}|{match['category']}", line)
        for line, match in zip(lines, matches)
    ]


def cppcheck_findings() -> List[Finding]:
    cppcheck = shutil.which("cppcheck")
    if cppcheck is None:
        cannot_run("cppcheck not found on PATH")
    # The baseline depends on the cppcheck version, which PATH decides.
    print(f"cppcheck: {cppcheck} ({run([cppcheck, '--version']).stdout.strip()})")
    result = run([cppcheck, "--quiet", f"--template={CPPCHECK_TEMPLATE}", SOURCE_DIR])
    fields = [line.split("|", 4) for line in result.stderr.splitlines() if line.strip()]
    if result.returncode != 0 or any(len(field) != 5 for field in fields):
        cannot_run(
            f"cppcheck did not run cleanly (exit code {result.returncode})",
            result.stdout + result.stderr,
        )
    findings = []
    for file, severity, check_id, line, message in fields:
        # "information" messages describe the analysis rather than the code (for example the
        # branches it skipped at the default check level) and vary between cppcheck versions.
        if severity != "information":
            path = normalize(file)
            description = f"{path}:{line}: {severity}: {message} [{check_id}]"
            findings.append((f"cppcheck|{path}|{check_id}", description))
    return findings


args = sys.argv[1:]
if args not in ([], ["--update-baseline"]):
    usage_error("usage: make-validate.py [--update-baseline]")

print(f"{color.OKGREEN}Validating C++ code (cpplint, cppcheck)...{color.ENDC}")
findings = cpplint_findings() + cppcheck_findings()
current = Counter(key for key, _ in findings)

if args == ["--update-baseline"]:
    # Written as bytes so the file has LF line endings on every platform.
    text = json.dumps(dict(current), indent=2, sort_keys=True) + "\n"
    BASELINE.write_bytes(text.encode("utf-8"))
    print(f"{color.OKGREEN}Baseline updated: {sum(current.values())} findings.{color.ENDC}")
    sys.exit(0)

if not BASELINE.is_file():
    cannot_run(f"baseline not found: {BASELINE} (create it with 'make validate-update-baseline')")
baseline = Counter(json.loads(BASELINE.read_text(encoding="utf-8")))
new = {key: count - baseline[key] for key, count in current.items() if count > baseline[key]}
fixed = sum((baseline - current).values())

if new:
    print(
        f"{color.FAIL}New findings (tool|file|category: count above the baseline, followed by "
        f"every current finding of that kind):{color.ENDC}"
    )
    for key, extra in sorted(new.items()):
        print(f"  {key}: +{extra}")
        for finding_key, description in findings:
            if finding_key == key:
                print(f"      {description}")
    sys.exit(1)
if fixed:
    print(
        f"{color.OKGREEN}{fixed} baseline finding(s) no longer reported; run "
        f"'make validate-update-baseline' to lock that in.{color.ENDC}"
    )
print(f"{color.OKGREEN}Validation: no new findings ({sum(current.values())} known).{color.ENDC}")
