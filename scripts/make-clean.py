import sys
from shutil import rmtree
from terminal_colors import *

numArgs = len(sys.argv)

if (numArgs != 2):
    print(f"{color.FAIL}[ERROR] You MUST pass 1 argument: 'all', 'static', 'shared' or 'module'!{color.ENDC}")

libType = sys.argv[0]

# ===== VALIDATE INPUT ARGUMENTS ===== #

# At this moment, we are not using the 'libType' input argument
# In the future, we will implement a 'clear' for each lib type folder

print(f"{color.OKGREEN}Deleting 'build' and 'dist' folders...{color.ENDC}", end = "")

rmtree("./build", True)
rmtree("./dist", True)
rmtree("__pycache__", True)
rmtree("tests/__pycache__", True)

print(f"{color.OKGREEN}Done!{color.ENDC}")