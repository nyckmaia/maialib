import platform
from pathlib import Path

from build_utils import run_step
from terminal_colors import color

print(f"{color.OKGREEN}Running C++ Unit Tests...{color.ENDC}")

myOS = platform.system()
testBinary = Path.cwd() / "build" / myOS / "cpp-tests" / "cpp-tests"

# The tests open their fixtures by relative path, so they run from the repository root.
run_step([str(testBinary)], "C++ unit tests")
