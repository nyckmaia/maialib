import os
import platform
from shutil import rmtree
from terminal_colors import *

def isInstalled():
    try:
        import maialib
        return True
    except:
        return False

isMaialibInstalled = isInstalled()

if (isMaialibInstalled == True):
    print(f"{color.OKGREEN}Uninstalling Maialib Python Module...{color.ENDC}")

    # Get the Operational System
    myOS = platform.system()

    # Uninstall directory in the Python 'site-packages' folder
    os.system(f"pip3 uninstall --yes maialib")

    distDir = "dist"

    # Delete dist directory
    rmtree(distDir, True)

    print(f"{color.OKGREEN}Maialib uninstalled!{color.ENDC}")
else:
    print(f"{color.OKGREEN}Maialib is not installed in your system{color.ENDC}")


# ==== OLD VERSION OF CODE ====== #

# print(f"{color.OKGREEN}Uninstalling Maialib Python Module...{color.ENDC}")

# # Get the Operational System
# myOS = platform.system()

# # Uninstall directory in the Python 'site-packages' folder
# os.system(f"pip3 uninstall --yes maialib")
# distDir = "dist"

# # Delete dist directory
# rmtree(distDir, True)
# print(f"{color.OKGREEN}Maialib uninstalled!{color.ENDC}")