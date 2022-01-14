import os
import platform
from shutil import rmtree
from pathlib import Path
from terminal_colors import *

print(f"{color.OKGREEN}Uninstalling Maialib Python Module...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Uninstall directory in the Python 'site-packages' folder
os.system(f"pip3 uninstall --yes maialib")

distDir = "dist"

# Delete dist directory
rmtree(distDir, True)

print(f"{color.OKGREEN}Maialib uninstalled! {color.ENDC}")