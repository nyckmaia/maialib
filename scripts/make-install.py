import os
import platform
from shutil import copy2, copytree

from terminal_colors import *

print(f"{color.OKGREEN}Installing Maialib module on Python Kernel v{platform.python_version()}...{color.ENDC}")

distDir = "dist"

# Link the install directory in the Python 'site-packages' folder
os.system(f"pip install --user {distDir}/")

print(f"{color.OKGREEN}Generating Python Module Stubs...{color.ENDC}")
# genStubsCommand = "coverage run -m pybind11_stubgen maialib --output-dir=\"./stubs\" --root-module-suffix="" --ignore-invalid=all --no-setup-py"
genStubsCommand = "pybind11-stubgen maialib --output-dir=\"./stubs\" --root-module-suffix="" --ignore-invalid=all --no-setup-py"
os.system(genStubsCommand)
genStubsCommand = "pybind11-stubgen maialib.maiacore --output-dir=\"./stubs\" --root-module-suffix="" --ignore-invalid=all --no-setup-py"
os.system(genStubsCommand)
genStubsCommand = "pybind11-stubgen maialib.maiapy --output-dir=\"./stubs\" --root-module-suffix="" --ignore-invalid=all --no-setup-py"
os.system(genStubsCommand)

# Copy stubs files to the 'dist' folder
copy2("./stubs/maialib/__init__.pyi", f"{distDir}/maialib/__init__.pyi")
copytree("./stubs/maialib/maiacore/",
         f"{distDir}/maialib/maiacore/", dirs_exist_ok=True)
copytree("./stubs/maialib/maiapy/",
         f"{distDir}/maialib/maiapy/", dirs_exist_ok=True)

# Uninstall maialib
os.system(f"pip uninstall --yes maialib")

# Re - install maialib, now with Python stubs
os.system(f"pip install --user {distDir}/")

print(f"{color.OKGREEN}Maialib Installed on Python kernel v{platform.python_version()} {color.ENDC}")
