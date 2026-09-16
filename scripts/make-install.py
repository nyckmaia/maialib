import platform
import subprocess
import sys
from pathlib import Path
from shutil import copytree

from terminal_colors import *


def run_step(command: str, step_name: str) -> None:
    """Run a shell command, printing its output as it runs, and abort the script with a
    clear message if the command fails."""
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        print(f"{color.FAIL}Step failed: {step_name} (exit code {result.returncode}){color.ENDC}")
        sys.exit(result.returncode)


print(
    f"""{color.OKGREEN}Installing Maialib module on Python Kernel v{platform.python_version()}...{
        color.ENDC
    }"""
)

distDir = "dist"

# Link the install directory in the Python 'site-packages' folder
run_step(f"pip install {distDir}/", "pip install dist/")

stubsPath = Path.cwd() / "stubs"
print(f"{color.OKGREEN}Generating Python Module Stubs from Maiacore...{color.ENDC}")

genStubsCommand = f"""pybind11-stubgen maialib.maiacore --output-dir={
    stubsPath
} --ignore-invalid-expressions \".*\" --ignore-all-errors"""
run_step(genStubsCommand, "pybind11-stubgen (maiacore stubs)")

print(f"{color.OKGREEN}Generating Python Module Stubs from Maiapy...{color.ENDC}")
maiapyPath = Path.cwd() / "maialib" / "maiapy"
genStubsCommand = (
    f"""stubgen --no-analysis  {maiapyPath} -o {stubsPath} --include-docstrings --ignore-errors"""
)
run_step(genStubsCommand, "stubgen (maiapy stubs)")

print(f"{color.OKGREEN}Copy stubs to dist folder...{color.ENDC}")

# Copy stubs files to the 'install package' folder
copytree("./stubs/maialib/", f"{distDir}/maialib/", dirs_exist_ok=True)
copytree("./stubs/maialib/", "./maialib/", dirs_exist_ok=True)

# The AI docs are built from the '.pyi' stubs inside './maialib/', so this must run
# only after the 'copytree' calls above have populated that folder. Building them
# earlier (e.g. right after stub generation into './stubs') reads stale/empty stubs
# from './maialib/' and silently wipes out the generated docs.
print(f"{color.OKGREEN}Building AI_API_CHEATSHEET.md from stubs...{color.ENDC}")
run_step(f"python {Path.cwd() / 'scripts' / 'build-cheatsheet.py'}", "build AI_API_CHEATSHEET.md")

print(f"{color.OKGREEN}Building llms-full.txt...{color.ENDC}")
run_step(f"python {Path.cwd() / 'scripts' / 'build-llms-full.py'}", "build llms-full.txt")

# Uninstall maialib
run_step("pip uninstall --yes maialib", "pip uninstall maialib")

# Re - install maialib, now with Python stubs
run_step(f"pip install {distDir}/", "pip install dist/ (final)")

print(
    f"""{color.OKGREEN}Maialib Installed on Python kernel v{platform.python_version()} {
        color.ENDC
    }"""
)
