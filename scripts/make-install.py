import platform
import sys
import tempfile
from pathlib import Path
from shutil import copytree

from build_utils import run_step
from terminal_colors import color

print(
    f"""{color.OKGREEN}Installing Maialib module on Python Kernel v{platform.python_version()}...{
        color.ENDC
    }"""
)

distDir = "dist"

# Link the install directory in the Python 'site-packages' folder
run_step([sys.executable, "-m", "pip", "install", f"{distDir}/"], "pip install dist/")

stubsPath = Path.cwd() / "stubs"
print(f"{color.OKGREEN}Generating Python Module Stubs from Maiacore...{color.ENDC}")

# `python -m` puts the working directory first on sys.path, where the repository's own
# 'maialib' folder (sources only, no compiled module) would shadow the installed package,
# so the stub generator runs from an empty directory.
with tempfile.TemporaryDirectory() as emptyDir:
    run_step(
        [
            sys.executable,
            "-m",
            "pybind11_stubgen",
            "maialib.maiacore",
            f"--output-dir={stubsPath}",
            "--ignore-invalid-expressions",
            ".*",
            "--ignore-all-errors",
        ],
        "pybind11-stubgen (maiacore stubs)",
        cwd=emptyDir,
    )

print(f"{color.OKGREEN}Generating Python Module Stubs from Maiapy...{color.ENDC}")
maiapyPath = Path.cwd() / "maialib" / "maiapy"
# mypy's wheels compile stubgen with mypyc, and `python -m` cannot run a compiled module
# ("No code object available for mypy.stubgen"), so its entry point is called directly.
run_step(
    [
        sys.executable,
        "-c",
        "from mypy.stubgen import main; main()",
        "--no-analysis",
        str(maiapyPath),
        "-o",
        str(stubsPath),
        "--include-docstrings",
        "--ignore-errors",
    ],
    "stubgen (maiapy stubs)",
)

print(f"{color.OKGREEN}Copy stubs to dist folder...{color.ENDC}")

# Copy stubs files to the 'install package' folder
copytree("./stubs/maialib/", f"{distDir}/maialib/", dirs_exist_ok=True)
copytree("./stubs/maialib/", "./maialib/", dirs_exist_ok=True)

# The AI docs are built from the '.pyi' stubs inside './maialib/', so this must run
# only after the 'copytree' calls above have populated that folder. Building them
# earlier (e.g. right after stub generation into './stubs') reads stale/empty stubs
# from './maialib/' and silently wipes out the generated docs.
print(f"{color.OKGREEN}Building AI_API_CHEATSHEET.md from stubs...{color.ENDC}")
run_step(
    [sys.executable, str(Path.cwd() / "scripts" / "build-cheatsheet.py")],
    "build AI_API_CHEATSHEET.md",
)

print(f"{color.OKGREEN}Building llms-full.txt...{color.ENDC}")
run_step(
    [sys.executable, str(Path.cwd() / "scripts" / "build-llms-full.py")], "build llms-full.txt"
)

# Uninstall maialib
run_step([sys.executable, "-m", "pip", "uninstall", "--yes", "maialib"], "pip uninstall maialib")

# Re - install maialib, now with Python stubs
run_step([sys.executable, "-m", "pip", "install", f"{distDir}/"], "pip install dist/ (final)")

print(
    f"""{color.OKGREEN}Maialib Installed on Python kernel v{platform.python_version()} {
        color.ENDC
    }"""
)
