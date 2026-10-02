"""`make corpus-fetch`: download the external MusicXML corpus at pinned commits.

OpenScore Lieder and OpenScore String Quartets (both CC0) go to test/musicxml/external/, which git
ignores; only their .mxl scores and licence files are checked out. `make corpus` then examines
them against test/musicxml/ledger-external.json.
"""

from build_utils import REPO_ROOT, run_step
from terminal_colors import color

EXTERNAL = REPO_ROOT / "test" / "musicxml" / "external"
REPOSITORIES = (
    ("Lieder", "https://github.com/OpenScore/Lieder", "38c5db510224d9facdc4b08d741fc788cfb58ea8"),
    (
        "StringQuartets",
        "https://github.com/OpenScore/StringQuartets",
        "9be3df2ace482130fe031b9e8a647cdf112ed243",
    ),
)

EXTERNAL.mkdir(parents=True, exist_ok=True)
for name, url, commit in REPOSITORIES:
    target = EXTERNAL / name
    if not (target / ".git").is_dir():
        run_step(
            ["git", "clone", "--filter=blob:none", "--no-checkout", url, str(target)],
            f"clone {name}",
        )
    run_step(
        ["git", "-C", str(target), "sparse-checkout", "set", "--no-cone", "/**/*.mxl", "/LICENSE*"],
        f"select the scores of {name}",
    )
    run_step(
        ["git", "-C", str(target), "checkout", "--quiet", commit],
        f"check out {name} at {commit[:8]}",
    )
print(f"{color.OKGREEN}External corpus ready in {EXTERNAL.relative_to(REPO_ROOT)}{color.ENDC}")
