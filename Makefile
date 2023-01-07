all: dev

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
.PHONY: module-debug
.PHONY: module-release
.PHONY: module
.PHONY: validate
.PHONY: build-cpp-tests
.PHONY: cpp-tests
.PHONY: py-tests 
.PHONY: tests
.PHONY: dist
.PHONY: install
.PHONY: uninstall
.PHONY: doc
.PHONY: logo

SCRIPTS_DIR = ./scripts

dev:
#	@make clean
	@make uninstall
	@make module-release
	@make install

clean:
	@python $(SCRIPTS_DIR)/make-clean.py all

dist-clean:
	@python $(SCRIPTS_DIR)/make-clean.py dist

static-clean:
	@python $(SCRIPTS_DIR)/make-clean.py static

shared-clean:
	@python $(SCRIPTS_DIR)/make-clean.py shared

module-clean:
	@python $(SCRIPTS_DIR)/make-clean.py module

static-debug:
	@python $(SCRIPTS_DIR)/make-library.py static Debug

static-release:
	@python $(SCRIPTS_DIR)/make-library.py static release

shared-debug:
	@python $(SCRIPTS_DIR)/make-library.py shared debug

shared-relase:
	@python $(SCRIPTS_DIR)/make-library.py shared release

static:
	@make static-release

shared:
	@make shared-release

cmake:
	@python $(SCRIPTS_DIR)/make-cmake.py

module-debug:
	@python $(SCRIPTS_DIR)/make-module.py Debug

module-release:
	@python $(SCRIPTS_DIR)/make-module.py Release

module:
	@make module-release
	
validate:
	@python $(SCRIPTS_DIR)/make-validate.py

build-cpp-tests:
	@make static-debug
	@python $(SCRIPTS_DIR)/make-cpp-tests.py Debug

cpp-tests:
	@make build-cpp-tests
	@python $(SCRIPTS_DIR)/make-run-cpp-tests.py
	@python $(SCRIPTS_DIR)/run-code-coverage.py

py-tests:
	@python $(SCRIPTS_DIR)/make-py-tests.py

tests:
	@make cpp-tests
	@make py-tests

dist:
	@python $(SCRIPTS_DIR)/make-dist.py

install:
	@make dist
	@python $(SCRIPTS_DIR)/make-install.py

uninstall:
	@python $(SCRIPTS_DIR)/make-uninstall.py

doc:
	@doxygen
