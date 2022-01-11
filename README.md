# Maia Project

Maia Project is multiplatform (Windows, OSX and Linux) a set of musical tools wroted in C++17 that enable musical score analisys in a easy and fast way.

This project can be used in two ways:

. Maia App: It is user-friendly application with a Graphical User Interface (GUI)
. Maia Lib: Its a C++ library (with a Python wrapper) without the GUI

So, the `Maia Lib` is the project *core* and the `Maia App` is just a Graphical User Interface that calls the `Maia Lib` functionalities.

## Advantages
* Easy to use to musicians and musical researchers
* High computer perfomance and fast calculations
* All library functionalities are avalible in a compiled binary Python module (C++ wrapper)
* Read and write MusicXML features

Now you can easily import your musical score using:
```
import maialib as maia
score = maia.Score('examples/Beethoven/Symphony_9th.xml')

ADD KNOWING ISSUES TO build
Windows
- disable Antivirus or make a exception (Cmake permissions)
- Python que vem com o MSYS2 pode conffundir o cmake e nao pegar a versao certa do Python

```
[NOTE]
====
To import musical scores using Maialib, these files must be `MusicXML` files. +
The file extensions can be:

* `*.xml`
* `*.mxl`
* `*.musicxml`

You can easily generate this files using a score editor like:

* MuseScore (free!)
* Sibelius
* Finale
* Others
====

[NOTE]
====
Many `MusicXML` files are avaliable for free in the internet for download.
But if you don't have a specific MusicXML file, you can import the MIDI file in a score editor (like MuseScore) and then export the `MusicXML` file from it. 
====
Now you can explore the Maialib functionalities as:

* x
* y
* z
====================================

# TO DO
. myScore.getPart(0).append() => auto create new measures at the score end
. Implementar o XML do Metronome Mark
. Create a new class called: Duration
. Move the implementation of `pitchRatio()` function to a new Interval method
. Add nested autocomplete feature
. How use pybind11 with pyLint?
. Create a maialib installer (put the shared library inside Python default library folder)
. The installer should call the cmake to build de project

## General
. Test and clean the `midiNote2pitch()` function
. `Note::transpose()` and/or transpose functions MUST auto update the `Note::_alterSymbol` class member
. Add overloaded methods to all methods excluding the input `config` JSON argument

## To Think:
. Pensar em algum método que quantifique a complexidade harmônica de um acorde.
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
. Improve `ticks2noteType()` to receive any duration tick possible. DEPENDS on Duration class

### Score Class
. To Fix:
* `.countNotes()`

#### Score::findPattern()
. Fix the errors when running the `test_frag2.xml`
. Add a input `vector<float>` argument field called: 'similarityPercentile' +
Use the `getPercentiles()` function to filter the output table +
After that, remove the `getPercentiles()` function from project

#### Score::getChords()
. Put the `sameAttackNotes` input argument inside the input JSON (as optional)
. Set the `min/maxStackNotes` as optional input field
. Skip/exclude percussion instruments notes
. Add a new boolean input argument called: `excludeRepeatedNotes`
. Add a new `enum` input argument: `minNoteType` => To exclude too short time chords
. Change `partNumber` input argument type from a vector part indexes to a vector of part names (`strings`)
. Implement `MusicXML::getChordsPerNoteEvent()` method

### Part Class
. Implementar:
* Move part of `toXML()` code to inside of `Measure.toXML()`

### Chord Class
. Add private members:
* std::vector<Interval> _interval;
* std::vector<Interval> _stackInterval;
. Implement:
* bool Chord::isTonal(const bool useEnharmony = false)
* std::vector<Interval> getIntervals()
* std::vector<Interval> getStackIntervals()
* bool Chord::isInRootPosition()
* bool Chord::isInFirstInversion()
* bool Chord::isInSecondInversion()
* int Chord::getInversionValue()
* void Chord::toMajor()
* void Chord::toMinor()
* void Chord::toHalfDiminished()
* void Chord::toDiminished()
* void Chord::toAugmented()

### Note Class
. Add support a quarter tone
. Rewrite the `getName()` method without do the enarmony with MIDI Notes.
. To the future => Implement method: `float getHarmonicDensity()` => Research: Something like 'Parncutt Roughness'

### Documentation
. Add a small description inside each `*.ipynb` tutorial file describing the method/function

. Add custom documentation to all functions/methods to use the `help()` Python function +
See: https://pybind11.readthedocs.io/en/master/basics.html#keyword-arguments

. When apply, add the 'Default Arguments' wroted in C++ functions definitions (*.h) in Pybind11 Wrapper +
See: https://pybind11.readthedocs.io/en/master/basics.html#default-arguments

. Write a Doxygen developer documentation (Only for C++)

. Convert all tutorials `(*.ipynb files)` to HTML using `jupyter nbconvert --to html myFile.ipynb`

### Python Plots
. Create a `maia_plot.py` file that contains the basic plot functions to easily plot the MaiaLib data outputs +
Example: MusicXML::instrumentFragmentation() method will have a match-pair Python function called: `plotInstrumentFragmentation(data, arg1, arg2, ...)`

include::docs/faq.adoc[]

include::docs/note.adoc[]

include::docs/chord.adoc[]

include::docs/score.adoc[]

include::docs/part.adoc[]

include::docs/measure.adoc[]

include::docs/maialib_v2.adoc[]

include::docs/feedback.adoc[]
