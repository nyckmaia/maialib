# Maialib - Change Log

All notable changes to this project will be documented in this file.

## [v0.0.4] - 2022-01-17
### New Features
- Part Class:
    - `.getNumNotesOn()`
    - `.getNumNotesOff()`
- Measure Class:
    - `.getNumNotesOn()`
    - `.getNumNotesOff()`
    - `.getNoteOff()`
- Note Class:
    - `.isNoteOff()`
- New `dist` folder structure
- Added `maiapy` to this project

### Improved
- Rename the compiled C++ output module from `maialib` to `maiacore`
- Parallel build: dynamically count the max number of threads (`make jobs`)
- Rename `Measure::getNumElements()` to `.getNumNotes()`
- Rename `Measure::getNote()` to `.getNoteOn()`

### Fixed

## [v0.0.3] - 2022-01-13
### New Features

### Improved
- Folder structure: Create a `core` folder separate the C++ side
- Better `README.md`
- `Makefile`: Added the new `make dev` - build and install the Python module
- Update `pybin11_json.hpp` to `v0.2.12`

### Fixed
- `Score::Score`: Added a `std::vector` constructor overload
- `Makefile`: Renamed `make test` to `make tests`

## [v0.0.2] - 2022-01-10
### New Features
- New build system using `make` commands
- Parallel build: `make -j8`
- Created a `test` folder for store Python unit tests
- Maialib cross-platform installer: `make install`
- Created VS Code Tasks to `build`, `install` and `debug` code

### Improved
- Add full support to enharmonic pitch values to `midiNote2pitch()` function
- Update `pybind11` library to `v2.9.0`
- Update `nlohmann JSON` library from `v3.9.1` to `v3.10.5`
- Better folder tree structure
- Move pure maialib functions to `Helper` class static members
- Move old `orq_functions.cpp` content to `Score.cpp`
- Remove unnecessary files
- Update doxygen file
- Add `make doc` to the `Makefile` options
- Moved old `mxml_files` folder to `tests/xml_examples` folder

### Fixed
- `Score::forEachNote()` now works as expected
- `midiNote2pitch()` python wrapper default argument

## [v0.0.1] - 2021-08-14
### New Features
- A optional *callback function* to the `Chord::isTonal()` method
- `Score::forEachNote()` method

### Improved
- Update `nlohmann::json` library to the v3.9.1
- Replace all `size_t` variables/arguments to `int` to improve performance
- `Score` constructor now can receive a list of parts as a `std::initializer_list` 
- `Score::getChords()` include default arguments

### Fixed
- `Score::setMetronomeMark()` now works as expected
- `Note` constructor works for rest
- `tutorial/create_note_chords.ipynb` correct variable names
- `Measure::getNote()` now return only notes and skip all rests
- `Measure::getElement()` now return notes and rests

