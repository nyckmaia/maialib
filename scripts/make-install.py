import os
from glob import glob
import platform
from shutil import copy2
from pathlib import Path

from terminal_colors import *

print(f"{color.OKGREEN}Installing Maialib Python Module...{color.ENDC}")

# Get the Operational System
myOS = platform.system()
buildDir = f'build/{myOS}/module/'
distDir = "dist"

binaryModuleList = []

# Get the module file and set the install directory
if myOS == "Windows":
    binaryModuleList = glob(f'{buildDir}/*.pyd')
elif myOS == "Linux":
    binaryModuleList = glob(f'{buildDir}/*.so')
elif myOS == "Darwin":
    binaryModuleList = glob(f'{buildDir}/*.so')
else:
    print(f"{color.FAIL}[ERROR] Unknown OS!{color.ENDC}")

# Maialib module file path
modulePath = binaryModuleList[0]
initFilePath = f'{buildDir}/__init__.py'
setupFilePath = str(Path.cwd() / "setup.py")
readmeFilePath = "README.md"
licenseFilePath = "LICENSE.txt"

# Create the dist directory, if it not exists
Path(f'{distDir}/maialib').mkdir(parents=True, exist_ok=True)

copy2(readmeFilePath, distDir)
copy2(modulePath, f'{distDir}/maialib')
copy2(initFilePath, f'{distDir}/maialib')
copy2(setupFilePath, distDir)
copy2(licenseFilePath, distDir)

# Link the install directory in the Python 'site-packages' folder
os.system(f"pip3 install {distDir}/")

print(f"{color.OKGREEN}Maialib Installed! {color.ENDC}")