# Maialib - Music Analysis Library {#mainpage}

![Maialib CI/CD](https://github.com/nyckmaia/maialib/actions/workflows/wheels.yml/badge.svg)

Official website: [Maialib Documentation](https://www.maialib.com/)

This library is a multiplatform set of musical tools that enable musical score analisys and composition in a easy and fast way. <br>
The project **core** was wrote in `C++17`, but it also has a `Python` wrapper that allows a greater number of people (ie musicians not trained in IT) to also have in their hands the same power and musical tools available in `maialib`.

## Advantages

- Easy to use to musicians and musical researchers
- High computer perfomance and fast calculations
- Read and write musical scores (MusicXML file format)

You can easily import your sheet music to Python environment using:

```python
from maialib import *

score = Score('examples/Beethoven/Symphony_9th.xml')
```

Now you can explore some `maialib` features like:

- Find musical patterns
- Write your own scores from your custom algorithms
- Analyse scores in a musical statistical data perspective
- And much more!

## Requirements

### To run-only

- Python 3.8 (or greater)

### To build from source

#### Applications:

- C++17 compatible compiler
- CMake 3.8
- Python 3.8
- Make
- Doxygen (Optional: To build documentation)
- Buildcache (Optional: To accelerate the build process)
- CppCheck (Optional: C++ Static Analyzer)

#### Python Dev-only dependencies

```
pip install pathlib
pip install cpplint
pip install wheel

# To generate Python stubs
pip install pybind11-stubgen
# Mac users: May be you have to add the coverage and pybind11-stubgen on your `PATH` - /etc/paths

pybind11_mkdoc (github)
sudo apt install clang (pybind11_mkdoc dependency) - Linux/Mac Only
```

## Tested Environments

| Operational System | Compilers                       |
| ------------------ | ------------------------------- |
| Windows 10 x64     | GCC 12.2 (MSYS2)                |
| Linux Ubuntu 20.04 | GCC 9.3                         |
| Apple OSX 10.15    | XCode (Command Line Tools) 11.5 |

## Documentation

This project have 2 documentation levels. One for each user type:

- **Level 1 - User documentation:** for musicians, musical researchers and non-professional IT people
- **Level 2 - Developer documentation:** A deeper information for professional C++ programmers.

## Code Coverage

Click [here](../../code-coverage/index.html) to check the `maiacore` C++ code coverage

### Level 1: Python Tutorial

You can explore `maialib` features looking at `python-tutorial` folder. <br>
There you will learn how to use and mix `maialib` classes and functions to reach your musical goals

### Level 2: Developer Documentation

You can browse the compiled `doxygen` documentation opening `docs/index.html`. <br>

## Quick Start

### Build Python module from C++ source

Open a terminal (or CMD in Windows), enter inside of the `maialib` folder. <br>
Type: `make` to build the Python module <br>
When the build process finishes, type: `make install` <br>
Done!

## Frequent Asked Questions

### 1) Where can I find the XML file of a specific musical score?

To import musical scores the file extensions must be: `*.xml`, `*.mxl` or `*.musicxml` <br>
You can easily export your music files to these file formats above from score editors, like:

- MuseScore (free!)
- Sibelius
- Finale
- Others

Many `MusicXML` files are avaliable for free in the internet for download.
But if you don't have a specific MusicXML file, you can import the MIDI file in a score editor (like MuseScore) and then export the `MusicXML` file from it.

## Known issues to build from source

### All Platforms

- Multiple Python versions installed, like: `Official Python`, `Microsoft Python`, `MSYS2 Python` and others can direct the build system to choose a wrong version to build and install the library. <br>
  To check all Python versions installed on your system, open the `Terminal` (or `CMD` on Windows) and type: - Linux or Mac: - `which python` - `which python3` - Windows: - `where.exe python` - `where.exe python3`

### Windows-Only

- Disable your antivirus or create a exception (CMake permissions)

## Contact

Nycholas Maia - nyckmaia@gmail.com

## Contributing

If you want...

## License

Maialib is licensed under (GPLv3 License) [https://www.gnu.org/licenses/gpl-3.0.html]
