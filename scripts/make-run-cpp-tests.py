import platform
from pathlib import Path

from build_utils import run_step
from terminal_colors import color

print(f"{color.OKGREEN}Running C++ Unit Tests...{color.ENDC}")

repoRoot = Path(__file__).resolve().parent.parent
testBinary = repoRoot / "build" / platform.system() / "cpp-tests" / "cpp-tests"

# The tests open their fixtures by paths relative to the repository root. Running the binary
# directly, not through CTest, keeps gtest's own summary as the output; the timeout is the one
# the CTest registration uses.
run_step([str(testBinary)], "C++ unit tests", cwd=str(repoRoot), timeout=1800)
