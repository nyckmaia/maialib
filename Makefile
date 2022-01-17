all: module

.PHONY: dev
.PHONY: clean
.PHONY: dist-clean
.PHONY: static-clean
.PHONY: shared-clean
.PHONY: module-clean
.PHONY: static-debug
.PHONY: static-release
.PHONY: shared-debug
.PHONY: shared-relase
.PHONY: static
.PHONY: shared
.PHONY: cmake
.PHONY: module
.PHONY: validate
.PHONY: cpp-tests
.PHONY: run-cpp-tests
.PHONY: py-tests 
.PHONY: test
.PHONY: dist
.PHONY: install
.PHONY: uninstall
.PHONY: doc
.PHONY: logo

SCRIPTS_DIR = ./scripts

dev:
	@make clean
	@make module
	@make install

clean:
	@python3 $(SCRIPTS_DIR)/make-clean.py all

dist-clean:
	@python3 $(SCRIPTS_DIR)/make-clean.py dist

static-clean:
	@python3 $(SCRIPTS_DIR)/make-clean.py static

shared-clean:
	@python3 $(SCRIPTS_DIR)/make-clean.py shared

module-clean:
	@python3 $(SCRIPTS_DIR)/make-clean.py module

static-debug:
	@python3 $(SCRIPTS_DIR)/make-library.py static debug

static-release:
	@python3 $(SCRIPTS_DIR)/make-library.py static release

shared-debug:
	@python3 $(SCRIPTS_DIR)/make-library.py shared debug

shared-relase:
	@python3 $(SCRIPTS_DIR)/make-library.py shared release

static:
	@make static-release

shared:
	@make shared-release

cmake:
	@python3 $(SCRIPTS_DIR)/make-cmake.py

module:
	@python3 $(SCRIPTS_DIR)/make-module.py
	
validate:
	@python3 $(SCRIPTS_DIR)/make-validate.py

cpp-tests:
	@make static-debug
	@python3 $(SCRIPTS_DIR)/make-cpp-tests.py

run-cpp-tests:
	@make cpp-tests
	@python3 $(SCRIPTS_DIR)/make-run-cpp-tests.py

py-tests:
	@python3 $(SCRIPTS_DIR)/make-py-tests.py

tests:
	@make cpp-tests
	@make run-cpp-tests
	@make py-tests

dist:
	@python3 $(SCRIPTS_DIR)/make-dist.py

install:
	@make dist
	@python3 $(SCRIPTS_DIR)/make-install.py

uninstall:
	@python3 $(SCRIPTS_DIR)/make-uninstall.py

doc:
	@doxygen
