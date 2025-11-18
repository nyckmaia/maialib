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
- **CppCheck** - C++ static analyzer for code quality

### Python Development Dependencies

```bash
pip install pathlib
pip install cpplint
pip install wheel
pip install mypy
pip install pybind11-stubgen
```

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
make coverage         # Run C++ tests with code coverage analysis
```

#### Library Building (Advanced)

```bash
make static           # Build static library (release)
make shared           # Build shared library (release)
make static-debug     # Build static library (debug)
```

#### Code Quality

```bash
make validate         # Run cpplint and cppcheck on C++ source
```

#### Documentation

```bash
make doc              # Generate Doxygen documentation (HTML)
```

#### Cleaning

```bash
make clean            # Clean all build artifacts
make dist-clean       # Clean distribution files only
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

Located in `tests-cpp/src/`, currently **754 tests** covering:

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
- **Line Length:** 100 characters maximum
- **Linter:** cpplint
- **Static Analyzer:** cppcheck

**Run validation:**
```bash
make validate
```

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

**Solution:** Activate the correct Python environment before building.

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

Maialib includes these third-party libraries in `external/`:

- **pugixml** - XML parsing (MIT License)
- **SQLiteCpp + sqlite3** - Database functionality (MIT License)
- **cpptrace** - Stack traces (MIT License)
- **pybind11** - Python bindings (BSD License)
- **nlohmann/json** - JSON handling (MIT License)
- **GoogleTest** - Unit testing framework (BSD License)

All dependencies are vendored and built automatically.

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

Maialib uses GitHub Actions for CI/CD:

- **Wheel Building:** Automated builds for multiple platforms and Python versions
- **Testing:** All tests run on every commit
- **Code Quality:** Automatic linting and static analysis

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
# Open coverage report in browser
firefox coverage_html/index.html  # Linux
open coverage_html/index.html     # macOS
start coverage_html/index.html    # Windows
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
