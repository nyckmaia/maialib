# Maialib - Music Analysis Library {#mainpage}

![Maialib CI/CD](https://github.com/nyckmaia/maialib/actions/workflows/wheels.yml/badge.svg)

This library is a multiplatform set of musical tools that enable musical score analisys and composition in a easy and fast way. <br>
The project **core** was wrote in `C++17`, but it also has a `Python` wrapper that allows a greater number of people (ie musicians not trained in IT) to also have in their hands the same power and musical tools available in `maialib`.

## Advantages

- Easy to use to musicians and musical researchers
- High computer perfomance and fast calculations
- Read and write musical scores (MusicXML file format)

## Get Python Package

```python
pip install maialib
```
Or, if you have a older `maialib` version installed on your system, please get the latest version running: `pip install maialib --upgrade`

## Get Started

You can easily import your sheet music (`*.xml` file) to Python environment using:

```python
import maialib as ml

myScore = ml.Score('./Beethoven/Symphony_9th.xml')
```

Now you can explore some `maialib` features like:

- Find musical patterns
- Write your own scores from your custom algorithms
- Analyse scores in a musical statistical data perspective
- And much more!

## Frequent Asked Questions

### 1) Where can I find the XML file of a specific musical score?

To import musical scores the file extensions must be: `*.xml`, `*.mxl` or `*.musicxml` <br>
You can easily export your music files to these file formats above from score editors, like:

- MuseScore (free!)
- Sibelius
- Finale
- Others

Many `MusicXML` files are avaliable for free in the internet for download.

### 2) What can I do if I don't have a `*.xml` file of my target music?
- First, make shure and look at different websites and online repositories trying to find the `*.xml` file
- You can find on the internet the desired MIDI file and import it in a score editor (like MuseScore, Sibelius, Finale, etc.) and then export the `MusicXML` file from it
- You can use scan the sheet music paper and get a PDF version of it, so:
  - You can use a OMR software to try to convert the PDF file into a `*.xml` file
  - You can pay for other people to type manually note-by-note the PDF into a musical software (link MuseScore, Sibelius, Finale, etc.)
- You can type manually note-by-note the music paper into a musical software (link MuseScore, Sibelius, Finale, etc.)

## Documentation (in development)

This project have 2 documentation levels. One for each user type:

- **Level 1 - User documentation:** for musicians, musical researchers and non-professional IT people (help me to do that!)
- **Level 2 - Developer documentation:** A deeper information for professional C++ programmers (Doxygen)

### Level 1: Python Tutorial

You can explore `maialib` features looking at `python-tutorial` folder ([link here](https://github.com/nyckmaia/maialib/tree/main/python-tutorial)). <br>
There you will learn how to use and mix `maialib` classes and functions to reach your musical goals
If you are starting, please check these 3 basic `maialib` Python tutorials:
- [Create Notes and Chords](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/create_notes_chords.ipynb)
- [Create a Score](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/create_score.ipynb)
- [Using Dataframes](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/dataframe.ipynb)

### Level 2: Developer Documentation

[Maialib Documentation WebSite](https://maialib.com/)

----------------------------

Would you like to improve any maialib function?
=============================================

## Are you a C++ developer?

### Requirements to build from C++ sources:

- C++17 compatible compiler
- CMake 3.26
- Python 3.8
- Make
- Doxygen (Optional: To build documentation)
- Buildcache (Optional: To accelerate the build process)
- CppCheck (Optional: C++ Static Analyzer)

## Are you a Python developer?

### Python Dev-only dependencies

```
pip install pathlib
pip install cpplint
pip install wheel
pip install mypy

# To generate Python stubs
pip install pybind11-stubgen
# Mac users: May be you have to add the coverage and pybind11-stubgen on your `PATH` - /etc/paths

pybind11_mkdoc (github)
sudo apt install clang (pybind11_mkdoc dependency) - Linux/Mac Only
```

## Tested Environments

| Operational System | Compilers                       |
| ------------------ | ------------------------------- |
| Windows 10 x64     | Clang 15.0 (MSYS2)              |
| Linux Ubuntu 20.04 | GCC 9.3                         |
| Apple OSX 10.15    | XCode 11.5 (Command Line Tools) |

## Quick Start

### Build Python module from C++ source

Open a terminal (or CMD in Windows), enter inside of the `maialib` folder. <br>
Type: `make` to build the Python module <br>
When the build process finishes, type: `make install` <br>
Done!

## Known issues to build from source

### All Platforms

- Multiple Python versions installed, like: `Official Python`, `Microsoft Python`, `MSYS2 Python` and others can direct the build system to choose a wrong version to build and install the library. <br>
  To check all Python versions installed on your system, open the `Terminal` (or `CMD` on Windows) and type: - Linux or Mac: - `which python` - `which python3` - Windows: - `where.exe python` - `where.exe python3`

### Windows-Only

- Disable your antivirus or create a exception (CMake permissions)

## Contact

Nycholas Maia - nyckmaia@gmail.com

## Contributing

- Fork this project
- Make your custumizations and improvments
- Please, send me a pull request

## License

Maialib is licensed under [GPLv3 License](https://www.gnu.org/licenses/gpl-3.0.html)
