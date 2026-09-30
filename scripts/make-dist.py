import platform
import sys
from shutil import copy2

from build_utils import REPO_ROOT, build_dir, remove_trees
from terminal_colors import color

print(f"{color.OKGREEN}Generating 'dist' folder...{color.ENDC}")

# Get the Operational System
myOS = platform.system()
buildDir = build_dir("module")
distDir = REPO_ROOT / "dist"

# Clear the 'dist' folder
remove_trees([distDir])

binaryModuleList = []

# Get the module file and set the install directory
if myOS == "Windows":
    binaryModuleList = sorted(buildDir.glob("*.pyd"))
elif myOS == "Linux" or myOS == "Darwin":
    binaryModuleList = sorted(buildDir.glob("*.so"))
else:
    print(f"{color.FAIL}[ERROR] Unknown OS!{color.ENDC}")

if not binaryModuleList:
    print(
        f"{color.FAIL}[ERROR] No built maiacore module found in '{buildDir}': "
        f"build it first with 'make module'.{color.ENDC}"
    )
    sys.exit(1)

# Maialib module file path
modulePath = binaryModuleList[0]
maialibDir = REPO_ROOT / "maialib"

# Create the dist directory, if it not exists
(distDir / "maialib" / "maiapy").mkdir(parents=True, exist_ok=True)
(distDir / "maialib" / "maiacore").mkdir(parents=True, exist_ok=True)
(distDir / "maialib" / "xml-scores-examples").mkdir(parents=True, exist_ok=True)

copy2(REPO_ROOT / "README.md", distDir)
copy2(modulePath, distDir / "maialib" / "maiacore")
copy2(maialibDir / "setup.py", distDir)
copy2(REPO_ROOT / "LICENSE.txt", distDir)

# Copy all maialib files to the 'dist' folder
for filename in maialibDir.glob("*.py"):
    copy2(filename, distDir / "maialib")

# Copy all maiacore files to the 'dist' folder
for filename in (maialibDir / "maiacore").glob("*.py"):
    copy2(filename, distDir / "maialib" / "maiacore")

# Copy all maiapy files to the 'dist' folder
for filename in (maialibDir / "maiapy").glob("*.py"):
    copy2(filename, distDir / "maialib" / "maiapy")

# Copy all XML and MXL sample files to the 'dist' folder
for pattern in ("*.xml", "*.mxl"):
    for filename in (maialibDir / "xml-scores-examples").glob(pattern):
        copy2(filename, distDir / "maialib" / "xml-scores-examples")

print(f"{color.OKGREEN}Done!{color.ENDC}")
