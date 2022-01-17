import os
from glob import glob
import platform
from shutil import copy2
from pathlib import Path

from terminal_colors import *

print(f"{color.OKGREEN}Installing Maialib module on Python Kernel v{platform.python_version()}...{color.ENDC}")

distDir = "dist"

# Link the install directory in the Python 'site-packages' folder
os.system(f"pip3 install --user {distDir}/")

print(f"{color.OKGREEN}Maialib Installed on Python kernel v{platform.python_version()} {color.ENDC}")