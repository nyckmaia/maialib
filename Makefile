# The interpreter that runs every script. Override it on the command line, e.g.
# `make "PYTHON=py -3.12"`; a PYTHON environment variable does not change it.
ifeq ($(OS),Windows_NT)
PYTHON = python
else
PYTHON = python3
endif

all: dev

.PHONY: dev
.PHONY: clean
.PHONY: dist-clean
.PHONY: static-clean
.PHONY: shared-clean
.PHONY: module-clean
.PHONY: cpp-tests-clean
.PHONY: static-debug
.PHONY: static-release
.PHONY: shared-debug
.PHONY: shared-release
.PHONY: static
.PHONY: shared
.PHONY: module-debug
.PHONY: module-release
.PHONY: module
.PHONY: validate
.PHONY: validate-update-baseline
.PHONY: coverage
.PHONY: build-cpp-tests
.PHONY: cpp-tests
.PHONY: py-tests
.PHONY: tests
.PHONY: corpus
.PHONY: corpus-update-ledger
.PHONY: corpus-fetch
.PHONY: fuzz
.PHONY: fuzz-minimize
.PHONY: msvc-gate
.PHONY: linux-gate
.PHONY: dist
.PHONY: install
.PHONY: uninstall
.PHONY: doc
.PHONY: logo
.PHONY: format-cpp
.PHONY: format-python
.PHONY: format
.PHONY: lint-python
.PHONY: lint-python-fix
.PHONY: build-cheatsheet
.PHONY: build-llms-full
.PHONY: build-ai-docs

SCRIPTS_DIR = ./scripts

dev:
#	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory uninstall
	@$(MAKE) --no-print-directory module-release
	@$(MAKE) --no-print-directory install

clean:
	@$(PYTHON) $(SCRIPTS_DIR)/make-clean.py all

dist-clean:
	@$(PYTHON) $(SCRIPTS_DIR)/make-clean.py dist

static-clean:
	@$(PYTHON) $(SCRIPTS_DIR)/make-clean.py static

shared-clean:
	@$(PYTHON) $(SCRIPTS_DIR)/make-clean.py shared

module-clean:
	@$(PYTHON) $(SCRIPTS_DIR)/make-clean.py module

cpp-tests-clean:
	@$(PYTHON) $(SCRIPTS_DIR)/make-clean.py cpp-tests

static-debug:
	@$(PYTHON) $(SCRIPTS_DIR)/make-library.py static Debug

static-release:
	@$(PYTHON) $(SCRIPTS_DIR)/make-library.py static release

shared-debug:
	@$(PYTHON) $(SCRIPTS_DIR)/make-library.py shared debug

shared-release:
	@$(PYTHON) $(SCRIPTS_DIR)/make-library.py shared release

static:
	@$(MAKE) --no-print-directory static-release

shared:
	@$(MAKE) --no-print-directory shared-release

module-debug:
	@$(PYTHON) $(SCRIPTS_DIR)/make-module.py Debug

module-release:
	@$(PYTHON) $(SCRIPTS_DIR)/make-module.py Release

module:
	@$(MAKE) --no-print-directory module-release

build-cpp-tests:
	@$(PYTHON) $(SCRIPTS_DIR)/make-cpp-tests.py Debug

coverage:
	@$(MAKE) --no-print-directory cpp-tests
	@$(PYTHON) $(SCRIPTS_DIR)/run-code-coverage.py

cpp-tests:
	@$(MAKE) --no-print-directory build-cpp-tests
	@$(PYTHON) $(SCRIPTS_DIR)/make-run-cpp-tests.py

py-tests:
	@$(PYTHON) $(SCRIPTS_DIR)/make-py-tests.py

tests:
	@$(MAKE) --no-print-directory cpp-tests
	@$(MAKE) --no-print-directory py-tests

# Every file of the MusicXML corpus, the slow ones included, and the external corpus once
# `make corpus-fetch` has downloaded it, compared with test/musicxml/ledger*.json.
corpus:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py

# Write the current corpus results as the ledgers; review the diff before committing it.
corpus-update-ledger:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus.py --update-ledger

# Download the external MusicXML corpus (OpenScore, CC0) at pinned commits into
# test/musicxml/external/, which git ignores.
corpus-fetch:
	@$(PYTHON) $(SCRIPTS_DIR)/make-corpus-fetch.py

# Seeded mutation fuzzing of the MusicXML reader and writer; options through FUZZ_ARGS, e.g.
# make fuzz FUZZ_ARGS="--seed 7 --cases 1000".
fuzz:
	@$(PYTHON) $(SCRIPTS_DIR)/make-fuzz.py $(FUZZ_ARGS)

# The same, then up to two failing cases of each outcome saved, minimised where possible, into
# test/musicxml/fuzz-regressions/ (FUZZ_ARGS="--per-outcome N" sets how many).
fuzz-minimize:
	@$(PYTHON) $(SCRIPTS_DIR)/make-fuzz.py --minimize $(FUZZ_ARGS)

# Windows only: maiacore and its C++ tests compiled by MSVC (Visual Studio 2022) in Debug and
# Release, then the package built by `pip install .` as CI builds it, with the Python tests.
msvc-gate:
	@$(PYTHON) $(SCRIPTS_DIR)/make-msvc-gate.py --python

# Linux, or Windows through WSL: the committed HEAD, exported to a temporary directory, built with
# GCC; its C++ tests, then `make dev` and the Python tests in a fresh virtual environment.
linux-gate:
	@$(PYTHON) $(SCRIPTS_DIR)/make-linux-gate.py

dist:
	@$(PYTHON) $(SCRIPTS_DIR)/make-dist.py

install:
	@$(MAKE) --no-print-directory dist
	@$(PYTHON) $(SCRIPTS_DIR)/make-install.py

uninstall:
	@$(PYTHON) $(SCRIPTS_DIR)/make-uninstall.py

doc:
	@doxygen

# ====================
# Code Formatting
# ====================

format-cpp:
	@$(PYTHON) $(SCRIPTS_DIR)/make-format.py

format-python:
	@echo "Formatting Python code with Ruff..."
	@ruff format maialib/ test/ scripts/
	@echo "Python formatting complete."

format: format-cpp format-python
	@echo "All code formatted successfully."

# ====================
# Linting
# ====================

lint-python:
	@echo "Linting Python code with Ruff..."
	@ruff check maialib/ test/ scripts/
	@echo "Python linting complete."

lint-python-fix:
	@echo "Linting and auto-fixing Python code with Ruff..."
	@ruff check --fix maialib/ test/ scripts/
	@echo "Python linting and fixes complete."

validate:
#	Run the Python linter for its output/side-effects only: pre-existing ruff
#	errors must not stop 'make validate' from reaching the C++ static analysis
#	below. The standalone 'lint-python' target stays strict when invoked directly.
	-@$(MAKE) --no-print-directory lint-python
	@$(PYTHON) $(SCRIPTS_DIR)/make-validate.py

validate-update-baseline:
	@$(PYTHON) $(SCRIPTS_DIR)/make-validate.py --update-baseline

# ====================
# AI-friendly docs
# ====================

build-cheatsheet:
	@$(PYTHON) $(SCRIPTS_DIR)/build-cheatsheet.py

build-llms-full:
	@$(PYTHON) $(SCRIPTS_DIR)/build-llms-full.py

build-ai-docs: build-cheatsheet build-llms-full
	@echo "AI-friendly docs regenerated."
