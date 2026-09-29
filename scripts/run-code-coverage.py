import platform
from pathlib import Path

from build_utils import run_step
from terminal_colors import color

print(f"{color.OKGREEN}Generating C++ Maiacore Code Coverage...{color.ENDC}")

# Get the Operational System
myOS = platform.system()

if myOS == "Windows":
    print(f"{color.OKGREEN}The code coverage can run only in Linux and Mac OSX{color.ENDC}")
    exit(0)

# Maiacore raw covarage files folder
path01 = (
    Path.cwd()
    / "build"
    / myOS
    / "static"
    / "Debug"
    / "CMakeFiles"
    / "maiacore.dir"
    / "maiacore"
    / "src"
    / "maiacore"
)
path01.mkdir(parents=True, exist_ok=True)

# Maiacore raw covarage files folder
path02 = path01 / "output"
path02.mkdir(parents=True, exist_ok=True)

covInfoFile = "my.info"
covInfoFullPath = f"{path02}/{covInfoFile}"
covInfoFileFiltered = "my-filtered.info"
covInfoFileFilteredFullPath = f"{path02}/{covInfoFileFiltered}"

outputDir = "code-coverage"

excludedPatterns = ["/usr/*", "*/external/*", "*/v1/*", "/Library/*"]

run_step(["lcov", "-c", "-d", str(path01), "-o", covInfoFullPath], "lcov capture")
run_step(
    ["lcov", "--remove", covInfoFullPath, "-o", covInfoFileFilteredFullPath, *excludedPatterns],
    "lcov remove excluded files",
)
run_step(["genhtml", "-o", outputDir, covInfoFileFilteredFullPath], "genhtml")
