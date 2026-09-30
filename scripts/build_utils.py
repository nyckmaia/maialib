"""Shared helpers for the build, test and install scripts in this directory.

Every command whose failure must stop the script runs through run_step(), so a
failing compiler, test binary or tool stops the script with that command's exit
code, and `make` reports the failure instead of printing success.

Scripts that delete from the repository resolve the paths against REPO_ROOT,
never against the working directory, and remove directory trees there with
remove_tree(), which never follows a link.
"""

import os
import platform
import stat
import subprocess
import sys
from pathlib import Path
from typing import Callable, Iterable, List, NoReturn, Optional, Sequence, Union

from terminal_colors import color

Command = Union[str, Sequence[str]]

BUILD_TYPES = {"debug": "Debug", "release": "Release"}

# The root of the repository this script belongs to.
REPO_ROOT = Path(__file__).resolve().parent.parent

# Windows sets this bit in the reparse tag of a reparse point that names another path, such as
# a symbolic link or a junction (IsReparseTagNameSurrogate in the Windows SDK). os.lstat()
# reports the tag only for these; it follows every other kind of reparse point.
_NAME_SURROGATE = 0x20000000


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


def build_dir(*parts: str) -> Path:
    """Return build/<operating system>/<parts> in the repository, e.g. build/Linux/module."""
    return REPO_ROOT.joinpath("build", platform.system(), *parts)


def _is_link_status(status: os.stat_result) -> bool:
    return stat.S_ISLNK(status.st_mode) or bool(
        getattr(status, "st_reparse_tag", 0) & _NAME_SURROGATE
    )


def is_link(path: Path) -> bool:
    """Whether ``path`` is a link: a symbolic link or, on Windows, a junction (or any other
    reparse point that names another path). False if ``path`` does not exist."""
    try:
        return _is_link_status(os.lstat(path))
    except OSError:
        return False


def remove_tree(path: Path) -> List[str]:
    """Delete ``path``, a file, a link or a directory with everything in it, if it exists.

    A link is removed itself: the directory it points to is neither entered nor changed. An
    entry that cannot be removed because it is read-only, such as the git pack files under a
    build directory, is made writable and removed again; on POSIX, where removing an entry
    needs a writable directory, its directory inside ``path`` is made writable instead. An
    entry that still cannot be removed is kept, and so are the directories that contain it,
    but everything else is removed.

    Returns one "<entry>: <reason>" line for each entry that could not be removed; the list
    is empty when ``path`` is gone.
    """
    failures: List[str] = []
    _remove(str(path), True, failures)
    return failures


def remove_trees(paths: Iterable[Path]) -> None:
    """Delete each path with remove_tree(); if anything is left, list it and exit with code 1."""
    failures = [failure for path in paths for failure in remove_tree(path)]
    if failures:
        print(f"{color.FAIL}Could not remove:{color.ENDC}")
        for failure in failures:
            print(f"  {failure}")
        sys.exit(1)


def _remove(path: str, is_top: bool, failures: List[str]) -> bool:
    """Remove ``path`` as remove_tree() describes; return whether it is gone."""
    try:
        status = os.lstat(path)
    except FileNotFoundError:
        return True
    except OSError as error:
        failures.append(_failure(path, error))
        return False
    if stat.S_ISDIR(status.st_mode) and not _is_link_status(status):
        try:
            with os.scandir(path) as entries:
                names = [entry.name for entry in entries]
        except OSError as error:
            failures.append(_failure(path, error))
            return False
        emptied = True
        for name in names:
            emptied = _remove(os.path.join(path, name), False, failures) and emptied
        # A directory that still holds an entry cannot be removed; that entry is reported.
        return emptied and _remove_entry(path, os.rmdir, status, is_top, failures)
    # os.lstat() reports a Windows junction as a directory, and os.rmdir() removes the junction
    # itself; os.unlink() removes every other link, a Windows directory symlink included, and
    # every file.
    remove = os.rmdir if stat.S_ISDIR(status.st_mode) else os.unlink
    return _remove_entry(path, remove, status, is_top, failures)


def _remove_entry(
    path: str,
    remove: Callable[[str], None],
    status: os.stat_result,
    is_top: bool,
    failures: List[str],
) -> bool:
    """Call ``remove(path)``; after a permission error, make the entry removable and retry."""
    try:
        remove(path)
        return True
    except FileNotFoundError:
        return True
    except PermissionError as error:
        reason = error
    except OSError as error:
        failures.append(_failure(path, error))
        return False
    # A link is never made writable: os.chmod() would change what it points to. On POSIX, the
    # directory that holds ``path`` is inside the tree unless ``path`` is the tree itself.
    if _is_link_status(status) or (os.name != "nt" and is_top):
        failures.append(_failure(path, reason))
        return False
    try:
        if os.name == "nt":
            os.chmod(path, stat.S_IREAD | stat.S_IWRITE)
        else:
            os.chmod(os.path.dirname(path), stat.S_IRWXU)
    except OSError:
        failures.append(_failure(path, reason))
        return False
    try:
        remove(path)
        return True
    except FileNotFoundError:
        return True
    except OSError as error:
        failures.append(_failure(path, error))
        return False


def _failure(path: str, error: OSError) -> str:
    return f"{path}: {error.strerror or error}"
