# 🛠️ Building Maialib from Source

This guide is for developers who want to build Maialib from C++ source code.

**Regular users:** You don't need this! Simply install with `pip install maialib`

---

## 📋 Prerequisites

### Required Tools

- **C++17 compatible compiler**
  - Windows: Clang 18.0+ or MSVC 2019+
  - Linux: GCC 9.3+ or Clang 10+
  - macOS: XCode 11.5+ Command Line Tools
- **CMake 3.26 or newer**
- **Python 3.8 - 3.14**
- **Make** (or equivalent build system)

### Optional Tools

- **Doxygen** - For generating C++ documentation
- **Buildcache** - Speeds up compilation significantly
- **CppCheck** - C++ static analyzer for code quality; `make validate` runs the version that
  `requirements-dev.txt` pins, installed with pip (see below)

### Python Development Dependencies

```bash
pip install pathlib
pip install cpplint
pip install wheel
pip install mypy
pip install pybind11-stubgen
```

For a reproducible environment that matches the exact versions this project's test workflow is
known to work with (runtime deps plus `setuptools`, `wheel`, `pybind11-stubgen`, `mypy`,
`cpplint`, `cppcheck`, `ruff` and the test-only `lxml`), install from the pinned
[`requirements-dev.txt`](requirements-dev.txt) instead. `make validate` needs the cpplint and
cppcheck versions pinned there:

```bash
pip install -r requirements-dev.txt
```

`setup.py`'s `install_requires` only sets version floors (no upper caps, to avoid resolution
conflicts downstream); `requirements-dev.txt` exact-pins everything so `make tests` behaves the
same from one machine or month to the next. It needs Python 3.12 or newer, which its pinned
`numpy` 2.5.3 requires, although maialib itself supports Python 3.8 to 3.14.

**Linux/macOS only:**
```bash
# For pybind11_mkdoc (documentation generation)
sudo apt install clang  # Ubuntu/Debian
brew install llvm       # macOS
```

---

## 🚀 Quick Build

### Default Build (Release + Install)

```bash
cd maialib
make
```

This will:
1. Build the release Python module
2. Uninstall any previous maialib version
3. Install the newly built module

### Available Make Commands

#### Building

```bash
make                  # Default: build release module + install
make dev              # Same as default - uninstall + build release + install
make module           # Build Python module (release)
make module-debug     # Build Python module (debug symbols)
```

#### Testing

```bash
make tests            # Run both C++ and Python tests
make cpp-tests        # Build and run C++ tests (GoogleTest)
make py-tests         # Run Python unit tests
make coverage         # Linux: run C++ tests with lcov coverage (HTML report in code-coverage/)
make msvc-gate        # Windows: build with Visual Studio 2022 (MSVC) and run both test suites
make linux-gate       # Build and test the committed HEAD with GCC on Linux (in WSL on Windows)
```

Every command exits with a non-zero code when a step fails. `make msvc-gate` builds the C++ tests
with MSVC in Debug and Release and runs them, then builds the package with `pip install .`, the
way CI builds its Windows wheels, and runs the Python tests. It builds the working tree,
uncommitted changes included, and needs Visual Studio 2022 with the C++ x64 tools and Python 3.12
through the Python launcher (`py -3.12`). `make linux-gate` exports the committed `HEAD`
(uncommitted changes are not tested) to a temporary directory and runs `make cpp-tests`, then
`make dev` and `make py-tests` in a fresh virtual environment there. When Linux lacks a tool it
needs, it lists the `apt` packages to install and exits with code 2. It needs CMake 3.25 or newer
on Linux, which the build's `add_subdirectory(... SYSTEM)` requires: Ubuntu 22.04's `apt` CMake,
3.22, fails at configure.

```bash
make corpus                # Every corpus file against its ledger, slow and fetched ones included
make corpus-update-ledger  # Write the current corpus results as the ledgers (review the diff)
make corpus-fetch          # Download the external corpus (OpenScore, CC0) at pinned commits
make fuzz                  # Fuzz the MusicXML reader and writer (FUZZ_ARGS="--seed N --cases N")
make fuzz-minimize         # The same, then up to two failing cases of each outcome saved
                           # into test/musicxml/fuzz-regressions/, minimised where possible
```

`make py-tests` includes the validator, corpus and fuzz tests (`test/test_musicxml_check.py`,
`test_musicxml_corpus.py`, `test_musicxml_fuzz.py`), which need `lxml` from `requirements-dev.txt`;
[`test/musicxml/README.md`](test/musicxml/README.md) describes the MusicXML test infrastructure.

#### Library Building (Advanced)

```bash
make static           # Build static library (release)
make shared           # Build shared library (release)
make static-debug     # Build static library (debug)
```

Each builds into `build/<OS>/<static|shared>/<Debug|Release>/`.

#### Code Quality

```bash
make validate                  # Ruff (report only), then cpplint and cppcheck on maiacore
make validate-update-baseline  # Accept the current cpplint and cppcheck findings
```

`make validate` fails when cpplint or cppcheck reports a finding that is not in the committed
baseline, `scripts/validate-baseline.json`, and when the check cannot run: cpplint or cppcheck is
not installed for the Makefile's `PYTHON`, a tool fails or prints output that is not a finding,
or the baseline is missing or is not a JSON object of finding counts. Both tools run with the
Makefile's `PYTHON`, so install them with `pip install -r requirements-dev.txt`: the baseline
holds the findings of the versions pinned there, and another cppcheck version can report others.
cppcheck analyses the sources for the same platform (`unix64`) on every operating system, so the
baseline does not depend on where it runs.

#### Documentation

```bash
make doc              # Generate Doxygen documentation (HTML)
```

#### Cleaning

```bash
make clean            # Clean all build artifacts
make dist-clean       # Clean distribution files only
make static-clean     # Remove build/<OS>/static only
make shared-clean     # Remove build/<OS>/shared only
make module-clean     # Remove build/<OS>/module only
make cpp-tests-clean  # Remove build/<OS>/cpp-tests only
```

#### Installation Management

```bash
make install          # Build distribution and install package
make uninstall        # Remove installed package
```

---

## 🏗️ Build System Architecture

Maialib uses a hybrid build system:

```
Makefile (Developer Interface)
    ↓
Python Scripts (scripts/)
    ↓
CMake (C++ Build System)
    ↓
setuptools (Python Package Distribution)
```

### Key Files

- **Makefile** - Main developer interface
- **CMakeLists.txt** - C++ build configuration
- **setup.py** - Python package distribution
- **VERSION** - Single source of truth for version number
- **scripts/** - Build automation scripts

---

## 🧪 Testing

### C++ Tests (GoogleTest)

Located in `tests-cpp/src/`, covering:

- Note class (100+ tests)
- Chord class (200+ tests)
- Interval, Score, Part, Measure
- Fraction, Duration, Key, TimeSignature, Clef
- ScoreCollection, Barline, Utils, Config
- And more...

**Run C++ tests:**
```bash
make cpp-tests
```

**With coverage:**
```bash
make coverage
```

### Python Tests (unittest)

Located in `test/`, covering:

- Note, Chord, Score functionality
- DataFrame exports
- Helper functions
- Integration tests

**Run Python tests:**
```bash
cd test
python -m unittest                                      # All tests
python -m unittest test_chord                           # Single module
python -m unittest test_chord.ChordTestCase.testMethod # Single test
```

---

## 📦 CMake Build Options

The CMakeLists.txt provides several build modes:

- **STATIC_LIB** - Build static library (`.a` / `.lib`)
- **SHARED_LIB** - Build shared library (`.so` / `.dylib` / `.dll`)
- **PYBIND_LIB** - Build Python module (default for `make`)
- **MAIACORE_BUILD_TESTS** - Build the C++ unit tests in `tests-cpp/` (requires `STATIC_LIB`;
  `tests-cpp` cannot be configured on its own)
- **PROFILING** - Enable function profiling

**Direct CMake usage:**
```bash
mkdir build
cd build
cmake .. -DPYBIND_LIB=ON
cmake --build . --config Release
```

---

## 🔍 Code Quality Standards

### C++ Style Guidelines

- **Standard:** C++17
- **Line Length:** 100 characters maximum, applied by clang-format (`make format-cpp`)
- **Linter:** cpplint, configured by `maiacore/CPPLINT.cfg` (which filters out `whitespace/*`,
  so cpplint does not check line length)
- **Static Analyzer:** cppcheck 2.17.1 (the `cppcheck` 1.5.1 package that `requirements-dev.txt`
  pins)

**Run validation:**
```bash
make validate
```

It fails when cpplint or cppcheck reports a finding that is not in `scripts/validate-baseline.json`,
or when the check cannot run (see [Code Quality](#code-quality)). Once findings are fixed, or when
new ones are accepted deliberately, rewrite the baseline with `make validate-update-baseline`,
with the tool versions that `requirements-dev.txt` pins, and commit it.

### Testing Requirements

- All new features must include unit tests
- Maintain or improve code coverage
- Tests must pass on all platforms (Windows, Linux, macOS)

---

## 🖥️ Platform-Specific Notes

### Windows

**Tested Environment:**
- Windows 10 x64
- Clang 18.0

**Common Issues:**
- Antivirus may block CMake - add exception or temporarily disable
- Multiple Python installations can confuse build system - check with `where.exe python`

### Linux

**Tested Environment:**
- Ubuntu 20.04
- GCC 9.3

**Install dependencies:**
```bash
sudo apt update
sudo apt install build-essential cmake python3-dev
```

### macOS

**Tested Environment:**
- macOS 10.15+
- XCode 11.5 Command Line Tools

**Install dependencies:**
```bash
xcode-select --install
brew install cmake python@3.11
```

---

## 🐛 Troubleshooting

### Multiple Python Versions

If you have multiple Python installations, the build system might choose the wrong one.

**Check installed versions:**

```bash
# Linux/macOS
which python
which python3

# Windows
where.exe python
where.exe python3
```

**Solution:** Activate the correct Python environment before building. `make` runs every script,
and builds the module, with `python` (Windows) or `python3` (Linux, macOS) from PATH. To use
another interpreter, pass it on the command line, e.g. `make "PYTHON=py -3.12"`; an environment
variable named `PYTHON` is ignored.

### Autocomplete Not Working in VS Code

If Python stubs aren't providing autocomplete:

1. Open VS Code Command Palette (Ctrl+Shift+P / Cmd+Shift+P)
2. Run: "Pylance: Clear Persisted Indices"
3. Restart VS Code

### Build Cache Issues

If experiencing strange build errors:

```bash
make clean
make
```

### Permission Errors (Windows)

- Disable antivirus temporarily
- Run terminal as Administrator
- Add CMake directory to antivirus exceptions

---

## 📊 Build Performance

**Typical build times** (Release mode, clean build):

| Platform | Compiler | Time | With Buildcache |
|----------|----------|------|----------------|
| Windows 10 | Clang 18 | ~8 min | ~2 min |
| Ubuntu 20.04 | GCC 9.3 | ~6 min | ~90 sec |
| macOS 10.15 | XCode 11.5 | ~7 min | ~2 min |

**Tip:** Install buildcache for significant speedups on incremental builds!

---

## 🔗 External Dependencies

Maialib builds these third-party libraries:

- **pugixml** - XML parsing (MIT License)
- **SQLiteCpp + sqlite3** - Database functionality (MIT License)
- **cpptrace** - Stack traces (MIT License)
- **pybind11** - Python bindings (BSD License)
- **nlohmann/json** - JSON handling (MIT License)
- **GoogleTest** - Unit testing framework (BSD License)

All are vendored in the repository except cpptrace, which CMake fetches (FetchContent, v0.8.2)
when it configures a build directory, so the first configure needs network access. All are built
automatically.

---

## 📝 Version Management

The project uses a single `VERSION` file as the source of truth.

**Update version:**
1. Edit `VERSION` file (e.g., `1.2.3`)
2. Version is automatically injected into:
   - Python package (`setup.py`)
   - C++ code (`CMakeLists.txt`)
   - Documentation

---

## 🌐 Continuous Integration

Maialib uses GitHub Actions to build and publish its wheels:

- **Wheel Building:** on every push to `main`, on every manual run and for every published
  release, cibuildwheel builds the wheels on Linux, Windows and macOS
- **Publishing:** the wheels are uploaded to PyPI only for a published GitHub release
- **Tests and code quality:** CI runs no tests and no linters yet. Run `make tests` and
  `make validate` before a pull request, and the gates, which build with the compilers of CI's
  Windows and Linux wheels: `make msvc-gate` (MSVC) on Windows and `make linux-gate` (GCC) on
  Linux or in WSL

See `.github/workflows/wheels.yml` for pipeline configuration.

---

## 💡 Development Tips

### Fast Iteration Workflow

```bash
# Edit C++ code
vim maiacore/src/maiacore/chord.cpp

# Quick rebuild + test
make module && make cpp-tests
```

### Debug Build

```bash
make module-debug
# Now use a Python debugger or gdb/lldb with the module
```

### Coverage Report

```bash
make coverage
# Open the coverage report in a browser (Linux only: the Debug build has coverage only there)
firefox code-coverage/index.html
```

### Profiling

```bash
# Build with profiling enabled
cd build
cmake .. -DPROFILING=ON
cmake --build . --config Release

# Run your profiling workload
# Analyze with gprof or similar tool
```

---

## 📚 Additional Resources

- **C++ API Documentation:** https://maialib.com
- **Python Tutorials:** [python-tutorial/](python-tutorial/)
- **Issue Tracker:** https://github.com/nyckmaia/maialib/issues
- **Discussions:** https://github.com/nyckmaia/maialib/discussions

---

## 🤝 Contributing

Found a build issue? Want to improve the build system?

1. Check [existing issues](https://github.com/nyckmaia/maialib/issues)
2. Open a new issue with:
   - Platform and compiler version
   - Full build output
   - Steps to reproduce
3. Submit a PR with fixes if possible

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines.

---

**Need help?** Contact: nyckmaia@gmail.com
