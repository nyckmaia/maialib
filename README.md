# Maialib - Music Analysis Library {#mainpage}

![Maialib CI/CD](https://github.com/nyckmaia/maialib/actions/workflows/wheels.yml/badge.svg)

This library is a multiplatform set of musical tools that enable musical score analysis and composition in an easy and fast way. <br>
The project **maiacore** was written in `C++17`, but it also has a `Python` wrapper that allows a greater number of people (i.e. musicians not trained in IT) to also have in their hands the same power and musical tools available in `maialib`.<br><br>
**IMPORTANT: The maialib Python package includes all maiacore classes and functions and some additional other pure python functions. This website covers the maiacore-only classes and functions documentation, which are also present inside the maialib python package.<br>We hope to add documentation of functions written in Python to this site in the future, so that we can have complete documentation of maialib.**

## Advantages

- Easy to use to by musicians and musical researchers
- High computer perfomance and fast calculations
- Read and write musical scores (MusicXML file format)

## Get Python Package

```python
pip install maialib
```

Or, if you have an older `maialib` version installed on your system, please get the latest version running: `pip install maialib --upgrade`

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

Many `MusicXML` files are avaliable for free in the Internet for download.

### 2) What can I do if I don't have a `*.xml` file of my target music?

- First, make sure, and look at different websites and online repositories trying to find the `*.xml` (or `*.musicxml` or `*.mxl`) file.
- After that, you can try these 4 options below:
  - Find on the internet the desired MIDI file and import it in a score editor (like MuseScore, Sibelius, Finale, etc.) and then export the `MusicXML` file from it
  - Scan the sheet music paper and get a PDF version of it, and after that, use a OMR software to try to convert the PDF file into a `*.xml` file:
    - [MuseScore Import - Experimental](https://musescore.com/import)
    - [ACE Studio PDF to MusicXML Converter](https://acestudio.ai/pdf-to-musicxml/)
    - [Sibelius PhotoScore](https://www.avid.com/photoscore-and-notateme-lite)
    - [ScanScore](https://scan-score.com/en/)
    - [SmartScore 64 NE](https://www.musitek.com/)
    - [Melogen AI](https://melogenai.com/)
    - [PDFtoMusic](https://www.myriad-online.com/en/products/pdftomusic.htm)
  - Pay for other people to type manually note-by-note the PDF into a musical software (link MuseScore, Sibelius, Finale, etc.)
  - Type manually note-by-note the music paper into a musical software (link MuseScore, Sibelius, Finale, etc.)

## Documentation (in development)

This project have 2 documentation levels. One for each user type:

- **Level 1 - User documentation:** for musicians, musical researchers and non-professional IT people (help me to do that!)
- **Level 2 - Developer documentation:** A deeper information for professional C++ programmers (Doxygen)

### Level 1: Python Tutorial

You can explore `maialib` features looking at `python-tutorial` folder ([link here](https://github.com/nyckmaia/maialib/tree/main/python-tutorial)). <br>
There you will learn how to use and mix `maialib` classes and functions to reach your musical goals
If you are starting out, please check these 3 basic `maialib` Python tutorials:

- [Create Notes and Chords](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/create_notes_chords.ipynb)
- [Create a Score](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/create_score.ipynb)
- [Using Dataframes](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/dataframe.ipynb)

### Level 2: Developer Documentation

[Maialib Documentation WebSite](https://maialib.com/)

## VS Code Users

- You can write your Python scripts using `*.py` or `*.ipynb` file extensions.
- If you decide to use `*.ipynb` extension, make sure to install `nbformat` Python package to enable visualize maialib graphs on VS Code editor.
  To do that: `pip install nbformat --upgrade`

---

# Would you like to improve any maialib function?

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
| Windows 10 x64     | Clang 18.0                      |
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
- If `maialib` Python stubs and autocomplete are not working good on VS Code. Run: "Pylance: Clear Persisted Indices"

### Windows-Only

- Disable your antivirus or create a exception (CMake permissions)

## Contact

Nycholas Maia - nyckmaia@gmail.com

## Contributing

- Fork this project
- Make your custumizations and improvements
- Please, send me a pull request

## License

Maialib is licensed under [GPLv3 License](https://www.gnu.org/licenses/gpl-3.0.html)
