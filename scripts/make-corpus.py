"""`make corpus` and `make corpus-update-ledger`: the MusicXML corpus against its ledgers.

`make corpus` examines every file of the in-repository corpus, the slow ones included, and the
external corpus once `make corpus-fetch` has downloaded it, and fails when a result differs from
test/musicxml/ledger.json or ledger-external.json. `make corpus-update-ledger` writes the results
as the ledgers instead; review the diff before committing it. Both need maialib installed
(`make dev`).
"""

import argparse
import os
import sys

from build_utils import REPO_ROOT, run_step
from terminal_colors import color

sys.path.insert(0, str(REPO_ROOT / "test" / "musicxml"))

import corpus  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--update-ledger", action="store_true", help="write the results as the ledgers"
    )
    arguments = parser.parse_args()
    failed = False
    for ledger, files in (
        (corpus.LEDGER, corpus.corpus_files()),
        (corpus.EXTERNAL_LEDGER, corpus.external_files()),
    ):
        if not files:
            continue
        print(
            f"{color.OKGREEN}Examining {len(files)} files for {ledger.name}...{color.ENDC}",
            flush=True,
        )
        actual = corpus.run_corpus(files)
        old = corpus.load_ledger(ledger) if ledger.is_file() else {}
        if arguments.update_ledger:
            corpus.write_ledger(ledger, corpus.updated_ledger(old, actual))
            print(f"{color.OKGREEN}Wrote {ledger.relative_to(REPO_ROOT)}{color.ENDC}")
            continue
        problems = corpus.compare(old, actual)
        problems += [
            f"{name}: in the ledger but not in the corpus"
            for name in sorted(set(old) - set(actual))
        ]
        for problem in problems:
            print(f"{color.FAIL}{problem}{color.ENDC}")
        failed = failed or bool(problems)
    if not arguments.update_ledger and not failed:
        # `make py-tests` skips the slow corpus files; their <transpose> round trip runs here,
        # once the corpus matches its ledgers, so that a ledger difference is always reported.
        os.environ["MAIALIB_SLOW_TESTS"] = "1"
        run_step(
            [
                sys.executable,
                "-m",
                "unittest",
                "test_musicxml_transpose.TransposeRoundTripTestCase",
            ],
            "the <transpose> round trip of the corpus",
            cwd=str(REPO_ROOT / "test"),
        )
    if failed:
        print(
            f"{color.FAIL}The corpus differs from its ledger: review, then `make corpus-update-ledger`.{color.ENDC}"
        )
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
