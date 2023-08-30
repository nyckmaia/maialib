import os
import platform
from shutil import copytree
from pathlib import Path

from terminal_colors import *

print(f"{color.OKGREEN}Installing Maialib module on Python Kernel v{platform.python_version()}...{color.ENDC}")

distDir = "dist"

# Link the install directory in the Python 'site-packages' folder
os.system(f"pip install {distDir}/")

stubsPath = Path.cwd() / "stubs"
print(f"{color.OKGREEN}Generating Python Module Stubs from Maiacore...{color.ENDC}")
# This pybind11-stubgen command below works until the pybind11-stubgen v0.16.2
# The next version is v1.0. The syntax was changed and stoped to generate to maiacore stubs
# Also, the WSL VS Code (Linux) was unable to access the maiacore stubs, even in the v0.16.2
genStubsCommand = f"pybind11-stubgen maialib.maiacore --output-dir={stubsPath} --ignore-invalid=all --no-setup-py"
os.system(genStubsCommand)

print(f"{color.OKGREEN}Generating Python Module Stubs from Maiapy...{color.ENDC}")
maiapyPath = Path.cwd() / "maialib" / "maiapy"
genStubsCommand = f"stubgen {maiapyPath} -o {stubsPath}"
os.system(genStubsCommand)


print(f"{color.OKGREEN}Copy stubs to dist folder...{color.ENDC}")

# Copy stubs files to the 'install package' folder
copytree("./stubs/maialib/", f"{distDir}/maialib/", dirs_exist_ok=True)
copytree("./stubs/maialib/", "./maialib/", dirs_exist_ok=True)

# Uninstall maialib
os.system(f"pip uninstall --yes maialib")

# Re - install maialib, now with Python stubs
os.system(f"pip install {distDir}/")

print(f"{color.OKGREEN}Maialib Installed on Python kernel v{platform.python_version()} {color.ENDC}")
