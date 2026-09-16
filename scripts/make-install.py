import os
import platform
from shutil import copytree
from pathlib import Path

from terminal_colors import *

print(
    f"""{color.OKGREEN}Installing Maialib module on Python Kernel v{platform.python_version()}...{
        color.ENDC
    }"""
)

distDir = "dist"

# Link the install directory in the Python 'site-packages' folder
os.system(f"pip install {distDir}/")

stubsPath = Path.cwd() / "stubs"
print(f"{color.OKGREEN}Generating Python Module Stubs from Maiacore...{color.ENDC}")

genStubsCommand = f"""pybind11-stubgen maialib.maiacore --output-dir={
    stubsPath
} --ignore-invalid-expressions \".*\" --ignore-all-errors"""
os.system(genStubsCommand)

print(f"{color.OKGREEN}Generating Python Module Stubs from Maiapy...{color.ENDC}")
maiapyPath = Path.cwd() / "maialib" / "maiapy"
genStubsCommand = (
    f"""stubgen --no-analysis  {maiapyPath} -o {stubsPath} --include-docstrings --ignore-errors"""
)
os.system(genStubsCommand)

print(f"{color.OKGREEN}Copy stubs to dist folder...{color.ENDC}")

# Copy stubs files to the 'install package' folder
copytree("./stubs/maialib/", f"{distDir}/maialib/", dirs_exist_ok=True)
copytree("./stubs/maialib/", "./maialib/", dirs_exist_ok=True)

# The AI docs are built from the '.pyi' stubs inside './maialib/', so this must run
# only after the 'copytree' calls above have populated that folder. Building them
# earlier (e.g. right after stub generation into './stubs') reads stale/empty stubs
# from './maialib/' and silently wipes out the generated docs.
print(f"{color.OKGREEN}Building AI_API_CHEATSHEET.md from stubs...{color.ENDC}")
os.system(f"python {Path.cwd() / 'scripts' / 'build-cheatsheet.py'}")

print(f"{color.OKGREEN}Building llms-full.txt...{color.ENDC}")
os.system(f"python {Path.cwd() / 'scripts' / 'build-llms-full.py'}")

# Uninstall maialib
os.system(f"pip uninstall --yes maialib")

# Re - install maialib, now with Python stubs
os.system(f"pip install {distDir}/")

print(
    f"""{color.OKGREEN}Maialib Installed on Python kernel v{platform.python_version()} {
        color.ENDC
    }"""
)
