"""`make corpus` and `make corpus-update-ledger`: the MusicXML corpus against its ledgers.

`make corpus` examines every file of the in-repository corpus, the slow ones included, and the
external corpus once `make corpus-fetch` has downloaded it, and fails when a result differs from
test/musicxml/ledger.json or ledger-external.json. When every file matches its ledger, it then
runs the <transpose> round trip of test/test_musicxml_transpose.py with MAIALIB_SLOW_TESTS=1, which
adds the slow corpus files. `make corpus-update-ledger` writes the results as the ledgers instead;
review the diff before committing it. Both need maialib installed (`make dev`).

Options go through CORPUS_ARGS, e.g. `make corpus CORPUS_ARGS="--skip-slow --workers 4"`:
--in-repo-only leaves out the external corpus, --skip-slow the files of 10 MB or more, and
--filter SUBSTRING every file whose repository-relative path does not contain SUBSTRING; --workers
sets how many files are examined at once, at least 1. A run that leaves files out compares, or
updates, only the files it examined, and does not run the round trip; one that leaves every file
out fails.
"""

import argparse
import os
import sys

from build_utils import REPO_ROOT, run_step
from terminal_colors import color

sys.path.insert(0, str(REPO_ROOT / "test" / "musicxml"))

import corpus  # noqa: E402


def positive_int(text: str) -> int:
    """argparse type: a whole number of at least 1."""
    try:
        value = int(text)
    except ValueError:
        raise argparse.ArgumentTypeError(f"{text!r} is not a whole number") from None
    if value < 1:
        raise argparse.ArgumentTypeError(f"{value} is less than 1")
    return value


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--update-ledger", action="store_true", help="write the results as the ledgers"
    )
    parser.add_argument("--in-repo-only", action="store_true", help="leave out the external corpus")
    parser.add_argument(
        "--skip-slow", action="store_true", help="leave out the files of 10 MB or more"
    )
    parser.add_argument(
        "--filter", metavar="SUBSTRING", help="only the files whose path contains SUBSTRING"
    )
    parser.add_argument(
        "--workers",
        type=positive_int,
        default=None,
        help=f"files examined at once (default {corpus.default_workers()})",
    )
    arguments = parser.parse_args()
    corpora = [(corpus.LEDGER, corpus.corpus_files())]
    if not arguments.in_repo_only:
        corpora.append((corpus.EXTERNAL_LEDGER, corpus.external_files()))
    failed = False
    examined = 0
    partial = arguments.skip_slow or arguments.filter is not None
    for ledger, every_file in corpora:
        files = corpus.select(every_file, arguments.filter, arguments.skip_slow)
        if not files:
            continue
        examined += len(files)
        print(
            f"{color.OKGREEN}Examining {len(files)} files for {ledger.name}...{color.ENDC}",
            flush=True,
        )
        actual = corpus.run_corpus(files, workers=arguments.workers)
        old = corpus.load_ledger(ledger) if ledger.is_file() else {}
        complete = len(files) == len(every_file)
        if arguments.update_ledger:
            corpus.write_ledger(ledger, corpus.ledger_after(old, actual, complete))
            print(f"{color.OKGREEN}Wrote {ledger.relative_to(REPO_ROOT)}{color.ENDC}")
            continue
        problems = corpus.compare(old, actual)
        if complete:
            problems += [
                f"{name}: in the ledger but not in the corpus"
                for name in sorted(set(old) - set(actual))
            ]
        for problem in problems:
            print(f"{color.FAIL}{problem}{color.ENDC}")
        failed = failed or bool(problems)
    if examined == 0:
        print(f"{color.FAIL}No corpus file is left to examine.{color.ENDC}")
        return 1
    if not arguments.update_ledger and not failed and not partial:
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
