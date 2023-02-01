import os
from terminal_colors import *

print(f"{color.OKGREEN}Validating C++ Code...{color.ENDC}")

coreSrcFilesDir = "./maiacore/src/maiacore/"

# ===== CPPLINT ===== #
print(f"{color.OKGREEN}=====> CPPLINT <====={color.ENDC}")
runCommand = f"cpplint --quiet --linelength=100 --recursive {coreSrcFilesDir}"
os.system(runCommand)

# ===== CPPCHECK: STATIC ANALYZER ===== #
print(f"{color.OKGREEN}=====> CPPCHECK <====={color.ENDC}")
runCommand = f"cppcheck --quiet {coreSrcFilesDir}"
os.system(runCommand)

print(f"{color.OKGREEN}Validation: Done!{color.ENDC}")
