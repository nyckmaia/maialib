import os
import platform
from pathlib import Path
from terminal_colors import *

print(f"{color.OKGREEN}Running C++ Unit Tests...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

# Create a 'build' folder (if not exists)
path = Path.cwd() / "build" / myOS / "cpp-tests"

runCommand = "{}/cpp-tests".format(path)
os.system(runCommand)