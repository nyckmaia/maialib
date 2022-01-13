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

## Knowing issues to build from source
### On Windows
- Disable Antivirus or make a exception (CMake permissions)
- Multiple Python versions installed- Example: "Official Python", "Microsoft Python" and "MSYS2 Python" can direct the `Maialib` build system to choose a wrong version to build and install the library

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


# TO DO
- Move the implementation of `pitchRatio()` function to a new Interval method

## General
- Test and clean the `midiNote2pitch()` function
- `Note::transpose()` and/or transpose functions MUST auto update the `Note::_alterSymbol` class member
- Add overloaded methods to all methods excluding the input `config` JSON argument

## To Think:
- Pensar em algum método que quantifique a complexidade harmônica de um acorde.
Sugestao: Usar bitwise em relação a série harmônica completa (2 bytes).
Exemplo:
bit 0 = fundamental
bit 1 = 8th
bit 2 = 5th
bit 3 = 8th
bit 4 = 3th
bit 5 = 5th
etc...

### Functions
- Improve `ticks2noteType()` to receive any duration tick possible- DEPENDS on Duration class

### Score Class
- To Fix:
- `.countNotes()`

#### Score::findPattern()
- Fix the errors when running the `test_frag2.xml`
- Add a input `vector<float>` argument field called: 'similarityPercentile' +
Use the `getPercentiles()` function to filter the output table +
After that, remove the `getPercentiles()` function from project

#### Score::getChords()
- Put the `sameAttackNotes` input argument inside the input JSON (as optional)
- Set the `min/maxStackNotes` as optional input field
- Skip/exclude percussion instruments notes
- Add a new boolean input argument called: `excludeRepeatedNotes`
- Add a new `enum` input argument: `minNoteType` => To exclude too short time chords
- Change `partNumber` input argument type from a vector part indexes to a vector of part names (`strings`)
- Implement `MusicXML::getChordsPerNoteEvent()` method

### Part Class
- Implementar:
- Move part of `toXML()` code to inside of `Measure.toXML()`

### Chord Class
- Add private members:
- std::vector<Interval> _interval;
- std::vector<Interval> _stackInterval;
- Implement:
- bool Chord::isTonal(const bool useEnharmony = false)
- std::vector<Interval> getIntervals()
- std::vector<Interval> getStackIntervals()
- bool Chord::isInRootPosition()
- bool Chord::isInFirstInversion()
- bool Chord::isInSecondInversion()
- int Chord::getInversionValue()
- void Chord::toMajor()
- void Chord::toMinor()
- void Chord::toHalfDiminished()
- void Chord::toDiminished()
- void Chord::toAugmented()

### Note Class
- Add support a quarter tone
- Rewrite the `getName()` method without do the enarmony with MIDI Notes.
- To the future => Implement method: `float getHarmonicDensity()` => Research: Something like 'Parncutt Roughness'

### Documentation
- Add a small description inside each `*.ipynb` tutorial file describing the method/function

- Add custom documentation to all functions/methods to use the `help()` Python function +
See: https://pybind11.readthedocs.io/en/master/basics.html#keyword-arguments

- When apply, add the 'Default Arguments' wroted in C++ functions definitions (*.h) in Pybind11 Wrapper +
See: https://pybind11.readthedocs.io/en/master/basics.html#default-arguments

- Write a Doxygen developer documentation (Only for C++)

- Convert all tutorials `(*.ipynb files)` to HTML using `jupyter nbconvert --to html myFile.ipynb`

### Python Plots
- Create a `maia_plot.py` file that contains the basic plot functions to easily plot the MaiaLib data outputs +
Example: MusicXML::instrumentFragmentation() method will have a match-pair Python function called: `plotInstrumentFragmentation(data, arg1, arg2, ...)`

include::docs/faq.adoc[]

include::docs/note.adoc[]

include::docs/chord.adoc[]

include::docs/score.adoc[]

include::docs/part.adoc[]

include::docs/measure.adoc[]

include::docs/maialib_v2.adoc[]

include::docs/feedback.adoc[]
