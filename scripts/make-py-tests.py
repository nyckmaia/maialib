import sys

from build_utils import run_step
from terminal_colors import color

print(f"{color.OKGREEN}Running Python Unit Tests...{color.ENDC}")

run_step([sys.executable, "-m", "unittest"], "Python unit tests", cwd="test")

print(f"{color.OKGREEN}Python Unit Tests: Done!{color.ENDC}")
