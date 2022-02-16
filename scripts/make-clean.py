import sys
from shutil import rmtree
from terminal_colors import *

numArgs = len(sys.argv)

if (numArgs != 2):
    print(f"{color.FAIL}[ERROR] You MUST pass 1 argument: 'all', 'static', 'shared', 'module' or 'dist'!{color.ENDC}")

cleanOption = sys.argv[1]

# ===== VALIDATE INPUT ARGUMENTS ===== #


if (cleanOption == "all"):
    print(f"{color.OKGREEN}Deleting 'build' and 'dist' folders...{color.ENDC}", end = "")

    rmtree("./build", True)
    rmtree("./dist", True)
    rmtree("./maialib.egg-info", True)
    rmtree("__pycache__", True)
    rmtree("tests/__pycache__", True)
    rmtree("maiapy/__pycache__", True)
    rmtree("scripts/__pycache__", True)
    rmtree("profile.json", True)
    print(f"{color.OKGREEN}Done!{color.ENDC}")
    sys.exit()

if (cleanOption == "dist"):
    print(f"{color.OKGREEN}Deleting 'dist' folder...{color.ENDC}", end = "")
    rmtree("./dist", True)
    print(f"{color.OKGREEN}Done!{color.ENDC}")
    sys.exit()


