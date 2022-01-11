import os
import platform
from terminal_colors import *

print(f"{color.OKGREEN}Running Python Unit Tests...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# If 'Windows'
if myOS == "Windows":
    os.system(f"python -m unittest")
else:
    os.system(f"python3 -m unittest")

print(f"{color.OKGREEN}Python Unit Tests: Done!{color.ENDC}")