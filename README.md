# Maialib

`maialib` is a multiplatform set of musical tools wroted in that enable musical score analisys and composition in a easy and fast way.

The project *core- was wrote in `C++17`, but it also have a `Python` wrapper which enable non-professional developers to get in their hands the same power and musical tools avaliable in `maialib`.

## Advantages
- Easy to use to musicians and musical researchers
- High computer perfomance and fast calculations
- Read and write musical scores (MusicXML file format)

You can easily import your sheet music to Python environment using:
```python
from maialib import - 

score = Score('examples/Beethoven/Symphony_9th.xml')
```

Now you can explore some `maialib` features like:

- x
- y
- z

## Documentation


## Doubts
### 1) Where can I find the XML file of a specific musical score?

To import musical scores the file extensions must be: `*.xml`, `*.mxl` or `*.musicxml`

You can easily generate this files using a score editor like:

- MuseScore (free!)
- Sibelius
- Finale
- Others

Many `MusicXML` files are avaliable for free in the internet for download.
But if you don't have a specific MusicXML file, you can import the MIDI file in a score editor (like MuseScore) and then export the `MusicXML` file from it.

## Knowing issues to build from source
### On Windows
- Disable Antivirus or make a exception (CMake permissions)
- Multiple Python versions installed- Example: "Official Python", "Microsoft Python" and "MSYS2 Python" can direct the `Maialib` build system to choose a wrong version to build and install the library