"""`make fuzz` and `make fuzz-minimize`: seeded mutation fuzzing of the MusicXML reader and writer.

Options go through FUZZ_ARGS, e.g. `make fuzz FUZZ_ARGS="--seed 7 --cases 1000"`. The run prints
how many cases ended in each outcome and writes every case that is not ok to
test/musicxml/fuzz-work/report-seed-<seed>.json; with --minimize, up to --per-outcome cases of
each failing outcome are saved into test/musicxml/fuzz-regressions/, minimised unless
fuzz.minimize keeps them as they are. Needs maialib installed (`make dev`).
"""

import argparse
import sys
from typing import Dict

from build_utils import REPO_ROOT
from terminal_colors import color

sys.path.insert(0, str(REPO_ROOT / "test" / "musicxml"))

import fuzz  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--cases", type=int, default=300)
    parser.add_argument(
        "--minutes", type=float, default=None, help="stop starting cases after this"
    )
    parser.add_argument("--timeout", type=float, default=30.0, help="seconds per case")
    parser.add_argument("--minimize", action="store_true")
    parser.add_argument("--per-outcome", type=int, default=2, help="cases saved per outcome")
    arguments = parser.parse_args()

    results = fuzz.run(arguments.seed, arguments.cases, arguments.minutes, arguments.timeout)
    print(f"{color.OKGREEN}{len(results)} cases of seed {arguments.seed}:{color.ENDC}")
    for outcome, count in fuzz.summary(results).items():
        print(f"  {outcome}: {count}")
    print(f"Report: {fuzz.write_report(arguments.seed, results).relative_to(REPO_ROOT)}")
    if arguments.minimize:
        minimised: Dict[str, int] = {}
        for case, _, outcome in results:
            if (
                not fuzz.worth_minimising(outcome)
                or minimised.get(outcome, 0) >= arguments.per_outcome
            ):
                continue
            minimised[outcome] = minimised.get(outcome, 0) + 1
            path = fuzz.save_regression(
                case, outcome, fuzz.minimize(case, outcome, arguments.timeout)
            )
            print(f"  {outcome}, case {case.index}: {path.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
