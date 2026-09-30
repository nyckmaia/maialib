# Tests That Can Fail (roadmap step 2) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the 22 assertions in `test/test_helpers.py` that can never fail into assertions that fail when the library stops rejecting an impossible spelling, and clear the formatting drift and the compiler warnings in the C++ test sources that step 10a made visible.

**Architecture:** Three independent, behaviour-neutral changes to test code and formatting: a formatting-only commit (`make format-cpp`), the 22 Python assertions rewritten as `assertRaisesRegex(RuntimeError, …)`, and the C++ test-source warnings removed without changing what the tests check.

**Tech Stack:** Python `unittest`, GoogleTest 1.14, clang-format (LLVM 18.1.6 via `make format-cpp`), clang 18 / GCC 13 / MSVC warnings.

**Spec:** Design note below; roadmap context in `C:/Users/nyck/.claude/plans/wobbly-coalescing-quill.md` (step 2) and the final review of step 10a (which scheduled the formatting commit and the warnings for this step).

## Design note (replaces a separate spec for this small step)

Measured on `main` @ `8d98f5f`:
- `test/test_helpers.py`, class `midiNote2pitch`, method `testTwelveTonesOctave4` (about lines 151-386) contains 22 blocks of the form
  `with self.assertRaises(Exception):` / `self.assertRaises(ml.Helper.midiNote2pitch(<midi>, "<acc>"), "")`.
  Python evaluates `ml.Helper.midiNote2pitch(...)` first: if it raises, the outer block catches it and the test passes; if it returns a string, the inner `assertRaises("<str>", "")` raises `TypeError`, which the outer block also catches — green either way.
- The 22 calls are rejected by the feasibility check in `Helper::midiNote2pitch` (`maiacore/src/maiacore/helper.cpp` ~60-102: `canBeDoubleFlat` / `canBeFlat` / `canBeSharp` / `canBeDoubleSharp`), which calls `LOG_ERROR` → `std::runtime_error` → Python `RuntimeError`, with the message `The MIDI Note '<midi>' cannot be wrote using '<acc>' accident type ...` (note the typo "wrote"; this step does not change library messages).
- The same 22 rejections are asserted correctly in C++ (`tests-cpp/src/helpers-test.cpp` ~169-256); only the Python binding path is unchecked.
- `make format-cpp` rewrites 10 tracked C++ files today (pre-existing drift from the project's `.clang-format`).
- With `-Wall -Wextra` now reaching the tests (step 10a), the C++ test build prints 13-14 warnings; the 10a reviews located them at `tests-cpp/src/measure-test.cpp:961`, `tests-cpp/src/score-test.cpp:446/460/476/507/777`, `tests-cpp/src/score-collection-test.cpp:78` (plus the GCC equivalents).

Decisions:
- D1 Each of the 22 blocks becomes `with self.assertRaisesRegex(RuntimeError, r"MIDI Note '<midi>' cannot be .* using '<acc>' accident type"):` followed by the bare call `ml.Helper.midiNote2pitch(<midi>, "<acc>")` — the pattern skips the misspelled word so a future spelling fix does not break the tests.
- D2 The formatting commit contains only `make format-cpp` output; nothing else.
- D3 Warnings are removed by the smallest change that keeps each test's meaning (e.g. `[[maybe_unused]]`, `static_cast`, removing an unused variable, matching signedness in comparisons); no warning is silenced with a pragma or a compiler flag.

## Global Constraints
- Behaviour-neutral for the library: no file under `maiacore/src` or `maiacore/include` changes except through `make format-cpp` in Task 1.
- Docs, comments, docstrings in technical English; comments explain the code, never its development history.
- Never stage the user's uncommitted `.gitignore` line `musescore/*`. Stage files by name, never `git add -A` / `git add .`.
- Commit messages end with:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01PZ1fS7HaQBBTCqJoCrbcqV`
- Baseline suites: C++ **1050/1050**, Python **472/472**; they must be identical at the end (same test names).
- Test workflow: `make dev` only inside a brand-new `py -3.12` venv outside the repository (session scratchpad) with `requirements-dev.txt`; `make cpp-tests`, `make py-tests`, `make validate` exit non-zero on failure (read exit codes directly, never through a pipe). For clang builds set, from PowerShell, `$env:VCToolsInstallDir = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\'` with `INCLUDE`/`LIB` unset.

---

### Task 1: One formatting-only commit

**Files:** whatever `make format-cpp` rewrites (expected: 10 tracked C++ files under `maiacore/` and `tests-cpp/`).

- [ ] **Step 1:** Run `make format-cpp` on a clean tree; list the changed files (`git status --short`).
- [ ] **Step 2:** Run it a second time: nothing changes (idempotent). If the second run changes anything, stop and report.
- [ ] **Step 3:** Build and run both suites: C++ 1050/1050, Python 472/472 (`make dev` in a brand-new venv, `make cpp-tests`, `make py-tests`), and `make validate` (exit 0 — if the formatting moved findings between lines only, the baseline still matches because it counts per file and category; if it does not, report).
- [ ] **Step 4:** Commit only those files: `style: apply the project's clang-format to the C++ sources`.

### Task 2: The 22 Python assertions can fail

**Files:** `test/test_helpers.py`

- [ ] **Step 1: Record today's behaviour.** For each of the 22 calls, record the exception type and message raised through the binding (a short script in the scratchpad, not in the repo).
- [ ] **Step 2: Rewrite the 22 blocks** per D1, e.g.:
```python
        # Flat
        with self.assertRaisesRegex(RuntimeError, r"MIDI Note '60' cannot be .* using 'b' accident type"):
            ml.Helper.midiNote2pitch(60, "b")
```
Remove the commented-out stderr-redirection snippet at the top of the method (dead code that no longer matches the library's behaviour).
- [ ] **Step 3: Prove they can fail.** Temporarily disable the feasibility check in `Helper::midiNote2pitch` (e.g. make the `if` at helper.cpp ~96 never true), rebuild the module in a brand-new venv, run `cd test && python -m unittest test_helpers.midiNote2pitch -v`: all 22 rewritten assertions must fail (and the method's other assertions must be unaffected). Restore the file (`git diff --stat maiacore` empty) and rebuild; then run the full Python suite: 472/472.
- [ ] **Step 4: Commit:** `test: the 22 midiNote2pitch rejections are asserted, so a regression fails the test`.

### Task 3: No compiler warnings in the C++ test sources

**Files:** `tests-cpp/src/*.cpp` (the files the build reports)

- [ ] **Step 1: Measure.** Clean build of the tests (`make cpp-tests-clean`, then `make cpp-tests`), list every warning in `tests-cpp/src` with file:line and flag (clang). If WSL has `cmake` (`make linux-gate` prerequisites), also collect GCC's list; otherwise say so.
- [ ] **Step 2: Fix each warning** per D3.
- [ ] **Step 3: Verify:** the clang test build reports 0 warnings from `tests-cpp/src`; C++ 1050/1050 with identical test names; `make msvc-gate` builds with no new MSVC warnings from `tests-cpp/src` and passes.
- [ ] **Step 4: Commit:** `test: the C++ test sources compile without warnings`.
