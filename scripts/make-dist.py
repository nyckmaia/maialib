from glob import glob
import platform
from shutil import copy2
from shutil import rmtree
from pathlib import Path

from terminal_colors import *

print(f"{color.OKGREEN}Generating 'dist' folder...{color.ENDC}", end="")

# Get the Operational System
myOS = platform.system()
buildDir = f'build/{myOS}/module/'
distDir = "dist"

# Clear the 'dist' folder
rmtree(distDir, True)

binaryModuleList = []

# Get the module file and set the install directory
if myOS == "Windows":
    binaryModuleList = glob(f'{buildDir}/Debug/*.pyd')
elif myOS == "Linux":
    binaryModuleList = glob(f'{buildDir}/*.so')
elif myOS == "Darwin":
    binaryModuleList = glob(f'{buildDir}/*.so')
else:
    print(f"{color.FAIL}[ERROR] Unknown OS!{color.ENDC}")

# Maialib module file path
modulePath = binaryModuleList[0]
setupFilePath = "maialib/setup.py"
readmeFilePath = "README.md"
licenseFilePath = "LICENSE.txt"

# Create the dist directory, if it not exists
Path(f'{distDir}/maialib/maiapy/').mkdir(parents=True, exist_ok=True)
Path(f'{distDir}/maialib/maiacore/').mkdir(parents=True, exist_ok=True)

copy2(readmeFilePath, distDir)
copy2(modulePath, f'{distDir}/maialib/maiacore/')
copy2(setupFilePath, distDir)
copy2(licenseFilePath, distDir)

# Copy all maialib files to the 'dist' folder
maialibAllFiles = "maialib/*.py"
for filename in glob(maialibAllFiles):
    copy2(filename, f'{distDir}/maialib/')

# Copy all maiacore files to the 'dist' folder
maiacoreAllFiles = "maialib/maiacore/*.py"
for filename in glob(maiacoreAllFiles):
    copy2(filename, f'{distDir}/maialib/maiacore/')

# Copy all maiapy files to the 'dist' folder
maiaPyAllFiles = "maialib/maiapy/*.py"
for filename in glob(maiaPyAllFiles):
    copy2(filename, f'{distDir}/maialib/maiapy/')

print(f"{color.OKGREEN}Done!{color.ENDC}")
