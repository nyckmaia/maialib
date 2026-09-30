import platform
import sys
import sysconfig
from shutil import copy2

from build_utils import REPO_ROOT, build_dir, remove_trees
from terminal_colors import color

print(f"{color.OKGREEN}Generating 'dist' folder...{color.ENDC}")

# The module built for the interpreter that runs this script, which `make install` installs
# into. pybind11 names it maiacore plus that interpreter's extension suffix (for example
# maiacore.cp312-win_amd64.pyd), so a module built for another Python version is never taken.
modulePath = build_dir("module") / f"maiacore{sysconfig.get_config_var('EXT_SUFFIX')}"
if not modulePath.is_file():
    print(
        f"{color.FAIL}[ERROR] {modulePath} not found: that is the maiacore module for this "
        f"Python ({platform.python_version()}, {sys.executable}); build it first with "
        f"'make module'.{color.ENDC}"
    )
    sys.exit(1)

distDir = REPO_ROOT / "dist"
maialibDir = REPO_ROOT / "maialib"

# Clear the 'dist' folder
remove_trees([distDir])

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
