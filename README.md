# 🎵 Maialib - Music Analysis Library for Everyone

![Maialib CI/CD](https://github.com/nyckmaia/maialib/actions/workflows/wheels.yml/badge.svg)
[![PyPI version](https://badge.fury.io/py/maialib.svg)](https://badge.fury.io/py/maialib)
[![Python Versions](https://img.shields.io/pypi/pyversions/maialib.svg)](https://pypi.org/project/maialib/)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Downloads](https://static.pepy.tech/badge/maialib)](https://pepy.tech/project/maialib)

**Analyze and create musical scores using Python - designed for musicians, researchers, and composers.**

Whether you're a music student analyzing Bach chorales, a composer generating algorithmic pieces, or a researcher studying harmonic patterns across centuries of music, Maialib makes working with musical scores as easy as a few lines of code.

```python
import maialib as ml

# Create a middle C note
note = ml.Note("C#4")

# Print information about this note
print("Note Information:")
print(f"  Pitch: {note.getPitch()}")
print(f"  Pitch class: {note.getPitchClass()}")
print(f"  Pitch Step: {note.getPitchStep()}")
print(f"  Octave: {note.getOctave()}")
print(f"  Alter symbol: {note.getAlterSymbol()}" )
print(f"  Type (long name): {note.getLongType()}")
print(f"  Duration (unit: quarter notes): {note.getQuarterDuration()}")
print(f"  MIDI number: {note.getMidiNumber()}")
print(f"  Frequency: {note.getFrequency():.2f} Hz")
print(f"  Is note on: {note.isNoteOn()}")
print(f"  Is note off: {note.isNoteOff()}")
print(f"  Enharmonic Notes: {note.getEnharmonicNotes()}")
print(f"  Harmonic Spectrum (first 3 partials): {note.getHarmonicSpectrum(numPartials=3, partialsDecayExpRate=0.5)}")
```

**Output**

```
Note Information:
  Pitch: C#4
  Pitch class: C#
  Pitch Step: C
  Octave: 4
  Alter symbol: #
  Type (long name): quarter
  Duration (unit: quarter notes): 1.0
  MIDI number: 61
  Frequency: 277.18 Hz
  Is note on: True
  Is note off: False
  Ehnarmonic Notes: [<Note Db4>, <Note Bx3>]
  Harmonic Spectrum (first 3 partials): ([277.1826171875, 554.365234375, 831.5478515625], [1.0, 0.5, 0.25])
```

**No music software required. No complex APIs. Just simple, powerful Python.**

---

## 📑 Table of Contents

- [Why Maialib?](#-why-maialib)
- [Installation](#-installation)
- [Quick Start](#-quick-start)
- [Examples Gallery](#-examples-gallery)
- [What Can You Analyze?](#-what-can-you-analyze)
- [Documentation](#-documentation)
- [Included Example Scores](#-included-example-scores)
- [FAQ](#-faq)
- [Community & Support](#-community--support)
- [Contributing](#-contributing)
- [Citation](#-citation)
- [License](#%EF%B8%8F-license)

---

## ✨ Why Maialib?

### Built for Musicians Who Code (or Want to Learn)

- **🎹 Musician-Friendly**: Designed with music theory terminology, not just technical jargon
- **⚡ Lightning Fast**: C++17 core engine with Python interface - get professional performance without complexity
- **📊 Data Science Ready**: Export to pandas DataFrames for statistical analysis
- **🎨 Visualization Built-in**: Generate piano rolls, pitch envelopes, and dissonance curves
- **📝 MusicXML Native**: Read and write standard notation files from MuseScore, Finale, Sibelius
- **🔬 Research-Grade**: Used in musicology research and composition studies
- **🎓 Academic Foundation**: Developed as part of doctoral research at UNICAMP (State University of Campinas)

### Perfect For:

✅ **Music Students** - Analyze scores for theory assignments and research projects <br/>
✅ **Composers** - Generate algorithmic compositions and explore musical patterns<br/>
✅ **Musicologists** - Perform computational analysis on large musical corpora<br/>
✅ **Music Teachers** - Create customized teaching examples and exercises<br/>
✅ **Arrangers** - Batch transpose scores for different instruments<br/>
✅ **Performers** - Extract sections and practice specific passages<br/>
✅ **Researchers** - Conduct quantitative musicology studies

### Key Features

- **Full MusicXML Support**: Read and write `.xml`, `.mxl`, and `.musicxml` files
- **High Performance**: C++17 core engine for fast computation on large musical corpora
- **Built-in Visualizations**: Piano rolls, pitch envelopes, chord analysis, and dissonance curves
- **Advanced Chord Analysis**: Identify qualities, analyze complexity, and calculate harmonic density
- **Dissonance Models**: Psychoacoustic dissonance calculation using the Sethares model
- **DataFrame Export**: Seamless integration with pandas for statistical analysis
- **Beginner-Friendly**: Simple API designed for musicians without extensive programming experience
- **Comprehensive Documentation**: Interactive tutorials, examples, and complete API reference
- **Cross-Platform**: Works on Windows, Linux, and macOS with Python 3.8-3.14

### Academic Background

Maialib was developed as part of the doctoral thesis:

**"maialib: A programming library for computer-aided music analysis and composition with applications on models of dissonance"**

- **Author**: Nycholas Maia
- **Institution**: UNICAMP (Universidade Estadual de Campinas)
- **Defense Date**: November 25, 2022
- **Research Focus**: Computational musicology, dissonance models, and algorithmic composition

---

## 🔧 Installation

### Prerequisites

- **Python 3.8 or newer** ([Download here](https://www.python.org/downloads/))
- Basic command line familiarity (we'll guide you!)

### Install in 30 Seconds

Open your terminal (or Command Prompt on Windows) and run:

```bash
pip install maialib
```

**Upgrading from an older version?**

```bash
pip install maialib --upgrade
```

### Verify Installation

```python
import maialib as ml
print(ml.__version__)
```

If you see a version number, you're ready to go! 🎉

<details>
<summary>🛠️ Troubleshooting Installation Issues</summary>

**"pip: command not found"**

- Make sure Python is in your system PATH
- Try `python -m pip install maialib` or `python3 -m pip install maialib`

**"ModuleNotFoundError: No module named 'maialib'"**

- Verify installation with `pip list | grep maialib`
- Make sure you're using the same Python environment

**Still having issues?**

- [Check our Installation Guide](https://github.com/nyckmaia/maialib/wiki/Installation)
- [Ask in GitHub Discussions](https://github.com/nyckmaia/maialib/discussions)

</details>

---

## 🚀 Quick Start

### Your First Musical Analysis in 5 Minutes

Maialib includes example scores so you can start immediately - no need to find files!

```python
import maialib as ml

# Load one of the included Bach examples
sampleScore = ml.SampleScore.Bach_Cello_Suite_1
sampleScorePath = ml.getSampleScorePath(sampleScore)
score = ml.Score(sampleScorePath)

# Get basic score information
print(f"Title: {score.getTitle()}")
print(f"Composer: {score.getComposerName()}")
print(f"Number of measures: {score.getNumMeasures()}")
print(f"Number of notes: {score.getNumNotes()}")

# Extract all notes as a pandas DataFrame
df = score.toDataFrame()
print("\nFirst 5 notes:")
print(df.head())

# Count notes by pitch class
pitch_distribution = df['Note'].value_counts()
print("\nPitch class distribution:")
print(pitch_distribution)
```

**Output:**

```
Title: Cello Suite No. 1
Composer: Johann Sebastian Bach
Number of measures: 220
Number of notes: 2293

First 5 notes:
                 Part      Measure       Note
0  <Part Violoncello>  <Measure 0>  <Note G2>
1  <Part Violoncello>  <Measure 0>  <Note D3>
2  <Part Violoncello>  <Measure 0>  <Note B3>
3  <Part Violoncello>  <Measure 0>  <Note A3>
4  <Part Violoncello>  <Measure 0>  <Note B3>

Pitch class distribution:
Note
<Note A3>     241
<Note G3>     184
<Note F#3>    169
<Note D3>     156
<Note B3>     154
             ...
<Note Eb2>      1
<Note Bb2>      1
<Note Ab3>      1
<Note Eb4>      1
<Note D4>       1
Name: count, Length: 99, dtype: int64
```

### Visualize Your Score

```python
# Load one of the included Bach examples
sampleScore = ml.SampleScore.Bach_Cello_Suite_1
sampleScorePath = ml.getSampleScorePath(sampleScore)
score = ml.Score(sampleScorePath)

# Generate a piano roll visualization
fig1, df1 = ml.plotPianoRoll(score, measureStart=1, measureEnd=16)
fig1.show()

# Plot pitch envelope over time
fig2, df2 = ml.plotScorePitchEnvelope(score, measureStart=1, measureEnd=16)
fig2.show()
```

---

## 💡 Examples Gallery

### Example 1: Get all music chords as a table

```python
import maialib as ml

# Load one of the included Chopin examples
sampleScore = ml.SampleScore.Chopin_Fantasie_Impromptu
sampleScorePath = ml.getSampleScorePath(sampleScore)
score = ml.Score(sampleScorePath)

# Extract chord data as a pandas DataFrame
chords = score.getChordsDataFrame()
print(chords)
```

**Output**

```
      measure  floatMeasure      key                                   chord  \
0           1      1.000000  <Key E>                      <Chord [G#2, G#3]>
1           2      2.000000  <Key E>                      <Chord [G#2, G#3]>
2           3      3.000000  <Key E>                      <Chord [C#2, C#3]>
3           3      3.083333  <Key E>                           <Chord [G#3]>
4           3      3.166667  <Key E>                           <Chord [G#3]>
...       ...           ...      ...                                     ...
1746      101    101.812500  <Key E>                 <Chord [G#1, G#2, C#4]>
1747      101    101.875000  <Key E>                 <Chord [G#1, G#2, G#4]>
1748      101    101.937500  <Key E>                 <Chord [G#1, G#2, F#3]>
1749      102    102.000000  <Key E>  <Chord [C#2, G#2, F#3, G#3, B#3, D#4]>
1750      103    103.000000  <Key E>       <Chord [C#2, G#2, E#3, G#3, C#4]>

      isHomophonic
0             True
1             True
2             True
3             True
4            False
...            ...
1746         False
1747         False
1748         False
1749          True
1750          True

[1751 rows x 5 columns]
```

### Example 2: Analyze Melodic Intervals

```python
# Load Beethoven's Symphony No. 5 score
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Beethoven_Symphony_5th))

# Analyze melodic intervals in the first measure of the first violin part
firstMeasure = score.getPart('Violins 1').getMeasure(0) # First violin part
firstMeasureNumNotes = firstMeasure.getNumNotes()

# Collect notes in the first measure
notes = []
for i in range(firstMeasureNumNotes):
    note = firstMeasure.getNote(i)
    notes.append(note)

# Calculate and print melodic intervals
print("Melodic intervals in opening:")
for i in range(firstMeasureNumNotes - 1):
    if notes[i].isNoteOff() or notes[i+1].isNoteOff():
        continue
    interval = ml.Interval(notes[i], notes[i+1])
    print(f"{notes[i].getPitch()} → {notes[i+1].getPitch()}: {interval.getName()} ({interval.getNumSemitones()} semitones)")
```

**Output**

```
Melodic intervals in opening:
G4 → G4: P1 (0 semitones)
G4 → G4: P1 (0 semitones)
```

### Example 3: Find Melodic Patterns

```python
# Load your Beethoven 5th Symphony score (or any other sheet music)
score = ml.Score('Beethoven_5th_symphony.xml')

# Define a pattern to search for (e.g., descending scale fragment)
pattern = [
    ml.Note("G4"),
    ml.Note("F4"),
    ml.Note("E4"),
    ml.Note("D4")
]

# Find all occurrences
df_patterns = score.findMelodyPatternDataFrame(
    melodyPattern=pattern,
    totalIntervalsSimilarityThreshold=0.8,
    totalRhythmSimilarityThreshold=0.5
)

print(f"Found {len(df_patterns)} occurrences of the pattern")
print(df_patterns[['partName', 'measureId', 'transposeInterval', 'totalSimilarity']])
```

**Output**

```
Found 6 occurrences of the pattern
    partName  measureId transposeInterval  totalSimilarity
0  Violins 1        406            m3 asc              1.0
1  Violins 1        409            m7 asc              1.0
2  Violins 2        406            m3 asc              1.0
3  Violins 2        409            m7 asc              1.0
4     Violas        166           M2 desc              1.0
5     Violas        460           M6 desc              1.0
```

### Example 4: Statistical Harmonic Analysis

```python
import pandas as pd
import maialib as ml

score = ml.Score('Beethoven_Symphony_5.xml')

chords = score.getChords()

# Analyze chord quality distribution
chord_qualities = []
for _, _, _, chord, _ in chords:
    if chord.isMajorChord():
        chord_qualities.append('Major')
    elif chord.isMinorChord():
        chord_qualities.append('Minor')
    elif chord.isDiminishedChord():
        chord_qualities.append('Diminished')
    elif chord.isAugmentedChord():
        chord_qualities.append('Augmented')
    else:
        chord_qualities.append('Other')

# Create distribution
df = pd.DataFrame({'Quality': chord_qualities})
distribution = df['Quality'].value_counts(normalize=True) * 100

print("Chord quality distribution:")
print(distribution)
```

**Output**

```
Chord quality distribution:
Quality
Other         40.218878
Major         30.711354
Minor         22.229822
Diminished     6.703146
Augmented      0.136799
Name: proportion, dtype: float64
```

### Example 5: Dissonance Analysis

```python
# Calculate psychoacoustic dissonance using Sethares model
score = ml.Score('my_score.xml')
chords = score.getChords()

dissonance_values = []
for _, _, _, chord, _ in chords:
    # Calculate Sethares dissonance value
    chordDissonance = chord.getSetharesDissonance(numPartialsPerNote=6)
    dissonance_values.append(chordDissonance)

print(dissonance_values)
```

**Output**

```
[1.1458631753921509, 1.1458631753921509, 1.1458631753921509, 1.233114242553711, 0.9513687491416931,...]
```

---

## 🎹 What Can You Analyze?

Maialib provides comprehensive tools for musical analysis:

### 🎼 Score Analysis

- Load MusicXML files (`.xml`, `.mxl`, `.musicxml`)
- Extract parts, measures, notes, and chords
- Access key signatures, time signatures, and tempo markings
- Export to pandas DataFrames for data science workflows

### 🎵 Melodic Analysis

- Find melodic patterns and motifs
- Calculate intervallic contours
- Analyze pitch distributions and ranges
- Compute melodic similarity metrics

### 🎶 Harmonic Analysis

- Extract vertical chord structures
- Identify chord qualities (major, minor, diminished, augmented)
- Analyze harmonic complexity and density
- Calculate psychoacoustic dissonance (Sethares model)
- Track chord progressions

### 🎹 Rhythmic Analysis

- Extract rhythmic patterns
- Analyze duration distributions
- Calculate rhythmic similarity
- Study metric structures

### 🎨 Visualization

- Piano rolls with customizable colors
- Pitch envelope graphs
- Chord analysis heatmaps
- Dissonance curves
- Statistical distribution plots

### ⚙️ Score Manipulation

- Transpose to any key
- Extract specific sections
- Modify notes, chords, and measures
- Export modified scores to MusicXML

### 📊 Statistical Tools

- Convert scores to DataFrames
- Aggregate data across multiple scores
- Perform corpus-wide analysis
- Export data for R, MATLAB, or other tools

---

## 📚 Documentation

### For Musicians & Researchers

**🎓 Interactive Tutorials** - Learn by doing with Jupyter notebooks:

- [Create Notes and Chords](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/create_notes_chords.ipynb) - Build musical objects from scratch
- [Create a Score](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/create_score.ipynb) - Generate scores programmatically
- [Using DataFrames](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/dataframe.ipynb) - Export and analyze with pandas
- [Get Chords](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/get_chords.ipynb) - Extract harmonic structures
- [Intervals](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/intervals.ipynb) - Work with melodic and harmonic intervals
- [Plotting Functions](https://github.com/nyckmaia/maialib/blob/main/python-tutorial/plots.ipynb) - Visualize your analysis

**📖 Full Tutorial Collection**: [Browse all tutorials](https://github.com/nyckmaia/maialib/tree/main/python-tutorial)

**🌐 API Reference**: [Complete documentation](https://maialib.com/)

### For Developers

**🛠️ Build from Source**: See [BUILDING.md](BUILDING.md) for C++ compilation instructions

**💻 C++ API Documentation**: [Developer docs](https://maialib.com/)

**🤝 Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines

---

## 🎼 Included Example Scores

Maialib includes 7 classical works so you can start exploring immediately:

- **Bach** - Cello Suite No. 1 (`Bach_Cello_Suite_1.mxl`)
- **Bach** - Prelude No. 1 in C Major, BWV 846 (`Bach_Prelude_1_BWV_846.xml`)
- **Beethoven** - String Quartet Op. 133 (`Beethoven_quartet_133.xml`)
- **Beethoven** - Symphony No. 5, Movement 1 (`Beethoven_Symphony_5_mov_1.xml`)
- **Chopin** - Fantaisie-Impromptu Op. 66 (`Chopin_Fantasie_Impromptu.mxl`)
- **Dvořák** - Symphony No. 9 "New World", Movement 4 (`Dvorak_Symphony_9_mov_4.xml`)
- **Mahler** - Symphony No. 1 "Titan", Movement 1 (`Mahler_Symphony_1_mov_1.mxl`)
- **Mozart** - Requiem, Introitus (`Mozart_Requiem_Introitus.mxl`)
- **Strauss** - The Blue Danube Waltz (`Strauss_Blue_Danube.mxl`)

**Try them now:**

```python
import maialib as ml
sampleScorePath = ml.getSampleScorePath(ml.SampleScore.Beethoven_Symphony_5th)
score = ml.Score(sampleScorePath)
```

---

## ❓ FAQ

### What is MusicXML?

MusicXML is the standard file format for sheet music, like `.docx` for documents or `.pdf` for portable documents. It contains all musical information: notes, rhythms, dynamics, articulations, and layout. Most notation software (MuseScore, Finale, Sibelius) can export to MusicXML.

### Where can I find MusicXML files?

Many sources offer free MusicXML downloads:

- [MuseScore.com](https://musescore.com) - Largest free sheet music library
- [IMSLP](https://imslp.org) - Public domain classical music
- [MusicXML.com](https://www.musicxml.com/music-in-musicxml/) - Sample files
- Export from your own notation software

### What if I don't have a MusicXML file?

You have several options:

1. **Convert from MIDI**: Import MIDI files into MuseScore (free) and export as MusicXML
2. **Scan PDF scores**: Use OMR (Optical Music Recognition) software:
   - [MuseScore Import](https://musescore.com/import) - Experimental, free
   - [ACE Studio PDF to MusicXML](https://acestudio.ai/pdf-to-musicxml/) - Free converter
   - [Sibelius PhotoScore](https://www.avid.com/photoscore-and-notateme-lite) - Professional option
3. **Use included examples**: Maialib comes with 7 classical works ready to analyze!

### Do I need programming experience?

**Beginner-friendly:** If you've never programmed before, start with our [interactive tutorials](https://github.com/nyckmaia/maialib/tree/main/python-tutorial). Basic Python knowledge helps, but you can learn as you go. Most common tasks require just 3-5 lines of code.

**Intermediate users:** If you know Python basics, you'll be productive immediately.

**Advanced users:** Full C++ source code available for custom extensions.

### Can I use Maialib for commercial projects?

Yes! Maialib is licensed under GPL-3.0, which allows commercial use. However:

- ✅ You can use Maialib in commercial research, analysis, or composition
- ✅ You can sell music created or analyzed with Maialib
- ⚠️ If you distribute software that uses Maialib, you must also license it under GPL-3.0
- 📖 See [license details](#%EF%B8%8F-license) below

### How do I cite Maialib in my research?

```bibtex
@software{maialib2024,
  author = {Maia, Nycholas},
  title = {Maialib: Music Analysis Library},
  year = {2024},
  url = {https://github.com/nyckmaia/maialib},
  version = {1.0}
}
```

Text citation: "Analysis performed using Maialib (Maia, 2024), a Python library for computational musicology."

### Does Maialib work with MIDI files?

Not directly. Maialib is designed for MusicXML, which contains more musical information than MIDI (articulations, dynamics, score layout). However, you can:

1. Convert MIDI → MusicXML using MuseScore
2. Load the MusicXML file into Maialib
3. Export back to MIDI if needed

### How is this different from notation software like MuseScore or Finale?

**Notation Software** (MuseScore, Finale, Sibelius):

- Visual editing of scores
- Playback and engraving
- Manual analysis

**Maialib**:

- Programmatic analysis at scale
- Statistical and computational musicology
- Batch processing hundreds of scores
- Algorithmic composition
- Integration with data science tools

**Use both together!** Edit scores in MuseScore, analyze them with Maialib.

### What Python version do I need?

Maialib supports **Python 3.8 through 3.14**. Check your version:

```bash
python --version
```

### Can I contribute to Maialib?

Absolutely! We welcome:

- 🐛 Bug reports and fixes
- ✨ Feature requests and implementations
- 📚 Documentation improvements
- 🌍 Translation help
- 🎵 Example scores and tutorials

See [CONTRIBUTING.md](CONTRIBUTING.md) to get started.

### Is there a community?

Yes! Connect with other Maialib users:

- 💬 [GitHub Discussions](https://github.com/nyckmaia/maialib/discussions) - Ask questions, share projects
- 🐛 [Issue Tracker](https://github.com/nyckmaia/maialib/issues) - Report bugs, request features
- 📧 [Email maintainer](mailto:nyckmaia@gmail.com) - Direct contact

---

## 🤝 Community & Support

### Get Help

- 💬 **[GitHub Discussions](https://github.com/nyckmaia/maialib/discussions)** - Ask questions, share your projects
- 🐛 **[Issue Tracker](https://github.com/nyckmaia/maialib/issues)** - Report bugs or request features
- 📖 **[Documentation](https://maialib.com)** - Complete API reference
- 📧 **[Email](mailto:nyckmaia@gmail.com)** - Contact the maintainer

### Share Your Work

Built something cool with Maialib? We'd love to hear about it!

- Share in [GitHub Discussions](https://github.com/nyckmaia/maialib/discussions/categories/show-and-tell)
- Tag `#maialib` on social media
- Submit your project to our showcase

---

## 🌟 Contributing

We welcome contributions from musicians, programmers, and researchers!

**Ways to contribute:**

- 🐛 Report bugs or suggest features in [Issues](https://github.com/nyckmaia/maialib/issues)
- 📝 Improve documentation or tutorials
- 🎵 Share example scores or analysis notebooks
- 💻 Submit code improvements via Pull Requests

**Getting started:**

1. Fork the repository
2. Make your improvements
3. Submit a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

---

## 📖 Citation

If you use Maialib in your research, please cite:

```bibtex
@software{maialib2022,
  author = {Maia, Nycholas},
  title = {Maialib: Music Analysis Library for Computational Musicology},
  year = {2022},
  publisher = {GitHub},
  url = {https://github.com/nyckmaia/maialib},
  version = {1.10.0}
}
```

**Text format:**

> Maia, N. (2022). _Maialib: Music Analysis Library for Computational Musicology_ (Version 1.10.0) [Computer software]. https://github.com/nyckmaia/maialib

---

## ⚖️ License

Maialib is licensed under the **[GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html)**.

**What this means in plain language:**

✅ **You CAN:**

- Use Maialib for personal projects, research, and commercial work
- Modify the source code
- Distribute modified versions
- Use it in academic publications

⚠️ **You MUST:**

- Keep the same GPL-3.0 license if you distribute modified versions
- Provide source code if you distribute software that includes Maialib
- Give credit to the original authors

📚 **For academic use:** Simply cite Maialib in your publications (see [Citation](#-citation))

🎵 **For creative work:** Music you create or analyze with Maialib is yours - the license only applies to the software itself.

**Questions about licensing?** [Contact us](mailto:nyckmaia@gmail.com)

---

## 📖 Glossary

<details>
<summary>Click to expand - Terms for non-programmers</summary>

- **API**: Application Programming Interface - the set of functions and commands available in Maialib
- **DataFrame**: A table-like data structure (like Excel spreadsheets) used for organizing and analyzing data
- **MusicXML**: Standard file format for musical notation, readable by most music software
- **MIDI**: Musical Instrument Digital Interface - a simpler music format focusing on performance data
- **Python Package**: Pre-built software you install with one command (`pip install`)
- **pip**: Python's package installer - the tool used to install Maialib
- **Transpose**: Shift all notes up or down by a specific interval
- **Chord Quality**: The type of chord (major, minor, diminished, augmented, etc.)
- **Pitch Class**: The note name without octave (C, D, E, etc.)
- **Harmonic Analysis**: Studying chords and their relationships in music
- **Melodic Contour**: The shape of a melody (ascending, descending, etc.)
- **Dissonance**: Musical tension or roughness between simultaneous notes

</details>

---

<div align="center">

**Made with ❤️ for musicians and researchers**

[⭐ Star us on GitHub](https://github.com/nyckmaia/maialib) | [🐛 Report Issues](https://github.com/nyckmaia/maialib/issues) | [💬 Join Discussions](https://github.com/nyckmaia/maialib/discussions)

</div>
