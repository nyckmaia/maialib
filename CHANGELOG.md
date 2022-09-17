# Maialib - Change Log

All notable changes to this project will be documented in this file.

## [v0.0.16] - 2022-MM-AA
### New Features
- `Note Class`:
    - `.getFrequency()`
    - `.getEnharmonicPitch()`
    - `.toEnharmonicPitch()`
    - `.getEnharmonicPitches()`
    - `.getEnharmonicNote()`
    - `.getEnharmonicNotes()`
- `Interval::getValue()`
- `Interval::setNotes()`
- `Interval::getPitchStepInterval()`
- `Helper::midiNote2octave()`
- `Chord::getStackedNotes()`
- New Unit Tests:
    - `Note::getEnharmonicPitch`
    - `Note::getEnharmonicPitches()`
    - `Note::getEnharmonicNotes()`

### Improved
- `Chord::stackInThirds()` now can use enharmony to compute the stacked chord
- `Interval::getDiatonicSteps()` added optional argument `useSingleOctave`

### Fixed

-------------------------

## [v0.0.15] - 2022-09-07
### New Features
- Add maialib Python stubs

### Improved
- `ScoreCollection` class:
    - Add overloaded constructor: vector of directories paths
    - Add `.merge(ScoreCollection& other)` method
    - Add `+` operator overload to `.merge(ScoreCollection& other)` method
    - Add `.getNumDirectories()` method
    - Add `.addDirectory()` method
    - Add `.setDirectoriesPaths()` method
    - Add oveloaded methods:
        - `.addScore(const std::string& filePath)`
        - `.addScore(const std::vector<std::string>& filePaths)`
- Replace `python3` to `python` calls
- Update `Pybind11` to v2.10.0

### Fixed
- C++ & Python unit tests are now running without any errors: 
    - `make cpp-tests`
    - `make py-tests`
    - `make tests`

-------------------------

## [v0.0.14] - 2022-07-06
### New Features
- Add New `ScoreCollection` class
- Add to all maialib classes:
    - Add Python `__hash__` method
    - Add Python `__sizeof__` method
- `Chord` Class: Add `getNotes()` method

### Improved
- Update Pybind11 to v2.9.2
- Better maialib objects representation inside the Python environment
- `Score::forEachNote()` callback contains more input parameters

### Fixed

-------------------------

## [v0.0.13] - 2022-03-05
### New Features

### Improved
- `Chord` class:
    - Add optional input argument: `.getIntervals(bool fromRoot = false)`
    - Add optional input argument: `.getStackIntervals(bool fromRoot = false)`
- Trello: Cards and status updated

### Fixed
- Fix `Score::setRepeat()` default arguments
- Fix `Note::transpose()` now works as expected

-------------------------

## [v0.0.12] - 2022-03-04
### New Features
- Add `Note::getDuration()` as an alias to `Note::getQuarterDuration()`
- Add `Duration` enum class
- Add `std::vector<std::string> Helper::midiNotes2pitches(int midiNote)`
- Add `void Score::setRepeat(int measureStart, int measureEnd)`
- `Chord` class:
    - Add `.getIntervals()`
    - Add `.getStackIntervals()`

### Improved
- `Note` constructor now receives a `Duration` enum class as second parameter
- `Helper::midiNotes2pitch()` if the user try to do something impossible, now this method returns an error

### Fixed
- Replace `Chord::getIntervalNames()` by `Chord::getIntervals()`

-------------------------

## [v0.0.11] - 2022-02-16
### New Features
- `Note` class
    - Add constructor overload to receive MIDI Numbers
    - Add `.transpose()`
- Add external `Cherno Instrumentor` profiler class. Load the output `profile.json` file inside `chrome://tracing/`

### Improved

### Fixed
- `Helper::midiNote2pitch()`

-------------------------
## [v0.0.10] - 2022-02-14
### New Features
- Add Makefile target: `make validate`
    - Run `cppcheck` static analyzer (optional dev dependency)
    - Run `cpplint` (optional dev dependency)
- Add `float Chord::getQuarterDuration()`
- `Note` class:
    - Add `float getQuarterDuration()`
    - Add `std::string getType()`
    - Add `std::string getLongType()`
    - Add `std::string getShortType()`
    - Add `int getNumDots()`
    - Add `bool isDotted()`
    - Add `bool isDoubleDotted()`

### Improved
- `Note` class: abstract the `ticks` integer values and replace it to `rhythm figures` as strings
- `Chord::getDuration()` now returns a `rhythm figure` as string
- `Helper::noteType2ticks()`
- `Helper::ticks2NoteType()`

### Fixed

-------------------------
## [v0.0.9] - 2022-02-11
### New Features
- Add `Chord::removeDuplicateNotes()`

### Improved
- `Score::getChords()`
    - Add optional boolean config input argument: `includeUnpitched`
    - Add optional boolean config input argument: ``includeDuplicates`
    - Replace optional default values:
        - `minStack`: minimum value from `2` to `1` 
        - `maxDuration`: maximum value from `quarter` to `maxima`
- Update Doxygen documentation
- Update Trello status and cards

### Fixed
- Fix `Score::getChords()` input arguments: `minDuration` and `maxDuration` 

-------------------------
## [v0.0.8] - 2022-02-10
### New Features
- Add unit test to `Helper::ticks2noteType()`
- Add `Chord::sortNotes()`
- `Measure` Class
    - Add `.divisionsPerQuarterNoteChanged()`
    - Add `.setIsDivisionsPerQuarterNoteChanged()`

### Improved
- Performance: Add optimization flags to the build system
- Performance: `Score::getChords()` add database indexes to speed up queries
- Performance: `Helper::ticks2noteType()` add GCC extension: `case range`
- Better code reading on `Score::loadXML()`
- Move external utility functions to a new file called: `utils.h`

### Fixed
- Fix `Score::getChords()` to support multiple `divisionsPerQuarterNote`
- Fix `cpp-tests` build
- Move `divisionsPerQuarterNote` from `Score` to `Measure` class
- Remove `divisionsPerQuarterNote` from `Note` class

-------------------------
## [v0.0.7] - 2022-02-08
### New Features
- Rewrite: `Score::getChords()`
    - Mode `Continuos`
    - Mode `Same Attack`
- Add `Chord::getDurationTicks()` method
- Add external dependency: `SQLiteCpp`
- Add `BuildCache` as a *optional* development tool

### Improved
- `VS Code`: Enable `pretty-printing` for `gdb`

### Fixed
- `Helper::noteType2ticks()` fix uppercase strings

------------------------
## [v0.0.6] - 2022-01-24
### New Features
- Add `Measure::getNumber()` and `Measure::setNumber()`

### Improved
- `Score::toDataFrame()`
    - Now return more data and columns for index and objects
    - Better performace
- Python default print for: `Part` and `Measure` objects 
### Fixed

------------------------
## [v0.0.5] - 2022-01-22
### New Features
- Add `Note::getStaff()`
- Add `Score::getPart(const std::string& partName)` overload
- Add `Score::getPartNames()`
- Add `Score::toDataFrame()`

### Improved
- `make dev` first uninstall the current installed Python module before start the new installation
- Better `README.md`
- Added a new test score: `test_multiple_instruments4.musicxml`

### Fixed
- Python wrapper: `Helper::ticks2noteType()` and `Helper::noteType2ticks`
- tests-cpp: `CMakeLists.txt` to include `maiacore` library
------------------------
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
------------------------
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
------------------------
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
------------------------
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
------------------------
