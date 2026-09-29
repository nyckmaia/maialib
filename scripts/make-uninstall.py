import sys
from shutil import rmtree

from build_utils import run_step
from terminal_colors import color


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

    # Uninstall directory in the Python 'site-packages' folder
    run_step(
        [sys.executable, "-m", "pip", "uninstall", "--yes", "maialib"], "pip uninstall maialib"
    )

    distDir = "dist"

    # Delete dist directory
    rmtree(distDir, True)

    print(f"{color.OKGREEN}Maialib uninstalled!{color.ENDC}")
else:
    print(f"{color.OKGREEN}Maialib is not installed in your system{color.ENDC}")
