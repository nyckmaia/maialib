"""Shared helpers for the build, test and install scripts in this directory.

Every external command runs through run_step(), so a failing compiler, test
binary or tool stops the script with that command's exit code, and `make`
reports the failure instead of printing success.
"""

import os
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
    A missing working directory ends the script with code 2, a missing program
    with 127, and any other failure to start the command with 126.
    """
    if cwd is not None and not os.path.isdir(cwd):
        where = os.path.abspath(cwd)
        print(
            f"{color.FAIL}Step failed: {step_name} (working directory not found: {where}){color.ENDC}"
        )
        sys.exit(2)
    # The child writes straight to the inherited handles, so this script's own buffered
    # messages must be flushed first to precede the child's output in a redirected log.
    sys.stdout.flush()
    sys.stderr.flush()
    try:
        result = subprocess.run(command, shell=isinstance(command, str), cwd=cwd, timeout=timeout)
    except subprocess.TimeoutExpired:
        print(f"{color.FAIL}Step timed out after {timeout} s: {step_name}{color.ENDC}")
        sys.exit(124)
    except FileNotFoundError as error:
        # Windows leaves error.filename unset, so fall back to the program that was run.
        program = error.filename or (command if isinstance(command, str) else command[0])
        print(f"{color.FAIL}Step failed: {step_name} (command not found: {program}){color.ENDC}")
        sys.exit(127)
    except OSError as error:
        print(f"{color.FAIL}Step failed: {step_name} (could not start: {error}){color.ENDC}")
        sys.exit(126)
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
