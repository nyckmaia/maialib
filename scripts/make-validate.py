"""Run cpplint and cppcheck on maiacore's sources and fail only on new findings.

Findings are counted per (tool, file, category), so moving code does not create
"new" findings; a count above the committed baseline (validate-baseline.json)
fails the run. Pass --update-baseline to accept the current findings.

Both tools come from the Python that runs this script, which should have the
versions that requirements-dev.txt pins: their findings can change from one
version to the next. They run from the repository root on a relative path, every
path is written with forward slashes, and cppcheck analyses the sources for one
fixed platform, so the same sources give the same baseline on every operating
system. Exit codes: 0 no new findings, 1 new findings, 2 the check could not
run (bad arguments, a baseline that is missing or not valid, or a tool that is
not installed, fails or prints something that is not a finding).
"""

import importlib.util
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path
from typing import List, NoReturn, Tuple

from build_utils import REPO_ROOT, usage_error
from terminal_colors import color

SOURCE_DIR = "maiacore/src/maiacore/"
BASELINE = Path(__file__).with_name("validate-baseline.json")
# cpplint writes each finding to stderr as "<file>:<line>:  <message>  [<category>] [<confidence>]"
# and its progress to stdout. It exits with 1 both when it finds something and when it cannot
# run at all (bad arguments), so any other line on stderr is an error.
CPPLINT_LINE = re.compile(r"^(?P<file>.+?):\d+:\s.*\[(?P<category>[\w/+-]+)\] \[\d\]$")
# cppcheck writes findings to stderr in this format and its own errors to stdout. It exits with
# 0 whether or not it finds anything, and with 1 when it cannot run.
CPPCHECK_TEMPLATE = "{file}|{severity}|{id}|{line}|{message}"
# The platform (type sizes, predefined macros) cppcheck analyses the sources for. Its default,
# "native", is the operating system it runs on.
CPPCHECK_PLATFORM = "unix64"
# Messages about the analysis rather than the code: at its default check level, cppcheck limits
# the branches it follows in a large function and says so. Every other message is a finding.
CPPCHECK_IGNORED_IDS = {"normalCheckLevelMaxBranches"}

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


def python_tool(module: str, arguments: List[str]) -> subprocess.CompletedProcess:
    """Run ``python -m <module> <arguments>`` from the repository root and capture its output.

    The interpreter is the one running this script. It runs with -W ignore, which also
    overrides PYTHONWARNINGS and the development mode (-X dev, PYTHONDEVMODE), so none of
    Python's own warnings, which go to stderr, mixes with the tool's output there: Python 3.14
    deprecates the codecs.open() calls in cpplint 2.0.2, for example.
    """
    if importlib.util.find_spec(module) is None:
        cannot_run(
            f"{module} is not installed for {sys.executable}: "
            "install the pinned tools with 'pip install -r requirements-dev.txt'"
        )
    command = [sys.executable, "-W", "ignore", "-m", module] + arguments
    try:
        return subprocess.run(
            command, cwd=REPO_ROOT, capture_output=True, text=True, errors="replace"
        )
    except OSError as error:
        cannot_run(f"could not start {module}: {error}")


def cpplint_findings() -> List[Finding]:
    # maiacore/CPPLINT.cfg filters out every whitespace/* category, so --linelength has no
    # effect there: the 100-column limit is applied by clang-format (make format-cpp).
    result = python_tool("cpplint", ["--quiet", "--linelength=100", "--recursive", SOURCE_DIR])
    lines = [line.strip() for line in result.stderr.splitlines() if line.strip()]
    matches = [CPPLINT_LINE.match(line) for line in lines]
    unexpected = [line for line, match in zip(lines, matches) if match is None]
    if unexpected:
        cannot_run(
            f"cpplint printed output that is not a finding (exit code {result.returncode})",
            "\n".join(unexpected),
        )
    if result.returncode != (1 if lines else 0):
        cannot_run(
            f"cpplint exited with code {result.returncode} after {len(lines)} finding(s)",
            result.stdout + result.stderr,
        )
    return [
        (f"cpplint|{normalize(match['file'])}|{match['category']}", line)
        for line, match in zip(lines, matches)
    ]


def cppcheck_findings() -> List[Finding]:
    # The findings, and so the baseline, depend on this version.
    print(f"cppcheck: {python_tool('cppcheck', ['--version']).stdout.strip()}")
    result = python_tool(
        "cppcheck",
        [
            "--quiet",
            f"--template={CPPCHECK_TEMPLATE}",
            f"--platform={CPPCHECK_PLATFORM}",
            SOURCE_DIR,
        ],
    )
    fields = [line.split("|", 4) for line in result.stderr.splitlines() if line.strip()]
    if any(len(field) != 5 for field in fields):
        cannot_run(
            f"cppcheck printed output that is not a finding (exit code {result.returncode})",
            result.stdout + result.stderr,
        )
    if result.returncode != 0:
        cannot_run(f"cppcheck exited with code {result.returncode}", result.stdout + result.stderr)
    findings = []
    for file, severity, check_id, line, message in fields:
        if check_id not in CPPCHECK_IGNORED_IDS:
            path = normalize(file)
            description = f"{path}:{line}: {severity}: {message} [{check_id}]"
            findings.append((f"cppcheck|{path}|{check_id}", description))
    return findings


def read_baseline() -> Counter:
    """Return the committed finding counts; exit with code 2 if they cannot be read."""
    if not BASELINE.is_file():
        cannot_run(
            f"baseline not found: {BASELINE} (create it with 'make validate-update-baseline')"
        )
    try:
        # utf-8-sig also accepts the byte order mark that some Windows editors write.
        data = json.loads(BASELINE.read_text(encoding="utf-8-sig"))
    except (OSError, ValueError) as error:
        cannot_run(f"cannot read the baseline {BASELINE}: {error}")
    if not isinstance(data, dict) or not all(
        type(count) is int and count >= 0 for count in data.values()
    ):
        cannot_run(
            f"the baseline {BASELINE} is not a JSON object of finding counts "
            "(recreate it with 'make validate-update-baseline')"
        )
    return Counter(data)


args = sys.argv[1:]
if args not in ([], ["--update-baseline"]):
    usage_error("usage: make-validate.py [--update-baseline]")
update = args == ["--update-baseline"]

print(f"{color.OKGREEN}Validating C++ code (cpplint, cppcheck)...{color.ENDC}")
baseline = Counter() if update else read_baseline()
findings = cpplint_findings() + cppcheck_findings()
current = Counter(key for key, _ in findings)

if update:
    # Written as bytes so the file has LF line endings on every platform.
    text = json.dumps(dict(current), indent=2, sort_keys=True) + "\n"
    try:
        BASELINE.write_bytes(text.encode("utf-8"))
    except OSError as error:
        cannot_run(f"cannot write the baseline {BASELINE}: {error}")
    print(f"{color.OKGREEN}Baseline updated: {sum(current.values())} findings.{color.ENDC}")
    sys.exit(0)

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
