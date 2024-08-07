import sys
import os
from shutil import rmtree
import glob
from terminal_colors import *

numArgs = len(sys.argv)

if (numArgs != 2):
    print(
        f"{color.FAIL}[ERROR] You MUST pass 1 argument: 'all', 'static', 'shared', 'module' or 'dist'!{color.ENDC}")

cleanOption = sys.argv[1]

# ===== VALIDATE INPUT ARGUMENTS ===== #


if (cleanOption == "all"):
    print(f"{color.OKGREEN}Deleting 'build' and 'dist' folders...{color.ENDC}", end="")

    rmtree("./build", True)
    rmtree("./dist", True)
    rmtree("./maialib.egg-info", True)
    rmtree("__pycache__", True)
    rmtree("tests/__pycache__", True)
    rmtree("maiapy/__pycache__", True)
    rmtree("scripts/__pycache__", True)
    rmtree("./stubs", True)
    rmtree("./code-coverage", True)
    rmtree("./wheelhouse", True)

    if os.path.exists("profile.json"):
        os.remove("profile.json")

    if os.path.exists(".coverage"):
        os.remove(".coverage")

    # Delete *.pyi files
    files = glob.glob('./maialib/**/*.pyi', recursive=True)

    for f in files:
        try:
            os.remove(f)
        except OSError as e:
            print("Error: %s : %s" % (f, e.strerror))

    print(f"{color.OKGREEN}Done!{color.ENDC}")
    sys.exit()

if (cleanOption == "dist"):
    print(f"{color.OKGREEN}Deleting 'dist' folder...{color.ENDC}", end="")
    rmtree("./dist", True)
    rmtree("./stubs", True)
    rmtree("./.coverage", True)
    rmtree("./code-coverage", True)
    print(f"{color.OKGREEN}Done!{color.ENDC}")
    sys.exit()
