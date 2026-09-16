import os
import platform
from shutil import rmtree

from terminal_colors import *


def isInstalled():
    try:
        # The import IS the probe. ruff reports it as unused and `ruff check
        # --fix` (make lint-python-fix) would delete it, which would make this
        # function always return True and turn `make uninstall` into a silent
        # no-op that still prints "Maialib uninstalled!".
        import maialib  # noqa: F401

        return True
    except:
        return False


isMaialibInstalled = isInstalled()

if isMaialibInstalled == True:
    print(f"{color.OKGREEN}Uninstalling Maialib Python Module...{color.ENDC}")

    # Get the Operational System
    myOS = platform.system()

    # Uninstall directory in the Python 'site-packages' folder
    os.system("pip uninstall --yes maialib")

    distDir = "dist"

    # Delete dist directory
    rmtree(distDir, True)

    print(f"{color.OKGREEN}Maialib uninstalled!{color.ENDC}")
else:
    print(f"{color.OKGREEN}Maialib is not installed in your system{color.ENDC}")
