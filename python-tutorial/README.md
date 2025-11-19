# 🎓 Maialib Python Tutorials

Welcome to the Maialib tutorial collection! These interactive Jupyter notebooks will guide you from complete beginner to advanced music analysis and composition.

## 📍 Start Here

**New to Maialib?** Begin with the **Getting Started** section below.

**Know Python basics?** Jump to **Beginner** tutorials.

**Ready for analysis?** Explore **Visualization** and **Analysis** sections.

---

## 🗺️ Learning Path

### Prerequisites

- **For Beginners**: Basic computer skills, interest in music
- **For Intermediate**: Python basics (variables, loops, functions)
- **For Advanced**: Python + Music theory + Data analysis concepts

### Recommended Learning Order

```
00_Getting_Started → 01_Beginner → 04_Visualization → 02_Intermediate → 05_Analysis → 03_Advanced
```

---

## 📚 Tutorial Index

### 00. Getting Started (🟢 Start Here!)

**For**: Complete beginners, first-time Python users
**Time**: 30-45 minutes total

| # | Tutorial | Topics | Time |
|---|----------|--------|------|
| 00 | Installation | Installing maialib, verifying setup | 5 min |
| 01 | Hello World | Your first note, chord, and score | 10 min |
| 02 | Library Overview | Understanding maialib structure | 15 min |

**Start with**: `00_installation.ipynb`

---

### 01. Beginner Tutorials

**For**: Musicians without programming experience
**Prerequisites**: Completed Getting Started section
**Time**: 3-4 hours total

| # | Tutorial | Topics | Difficulty |
|---|----------|--------|------------|
| 01 | Notes Basics | Creating notes, properties, rests | 🟢 Easy |
| 02 | Chords Basics | Creating chords, chord properties | 🟢 Easy |
| 03 | Intervals Basics | Musical intervals, semitones | 🟢 Easy |
| 04 | Loading Scores | Loading MusicXML files, basic info | 🟢 Easy |
| 05 | Creating Simple Scores | Building scores from scratch | 🟡 Medium |
| 06 | Helper Functions | Pitch/MIDI/frequency conversions | 🟢 Easy |
| 07 | Duration and Rhythm | RhythmFigure, Duration class | 🟡 Medium |
| 08 | Practical: Random Melody | Generate random musical ideas | 🟢 Easy |

**Music Theory Level**: Basic (no prior theory required)

**Python Level**: None (we explain everything)

---

### 02. Intermediate Tutorials

**For**: Musicians with basic Python knowledge
**Prerequisites**: Completed Beginner section
**Time**: 5-6 hours total

| # | Tutorial | Topics | Difficulty |
|---|----------|--------|------------|
| 01 | Score Navigation | Iterating parts, measures, notes | 🟡 Medium |
| 02 | DataFrame Integration | Pandas integration, data analysis | 🟡 Medium |
| 03 | Extracting Chords | getChords() method, configurations | 🟡 Medium |
| 04 | Transposed Instruments | Bb clarinet, F horn, concert pitch | 🟡 Medium |
| 05 | Time & Key Signatures | TimeSignature, Key classes | 🟡 Medium |
| 06 | Multi-Part Scores | Creating orchestral scores | 🟡 Medium |
| 07 | Score Editing | Modifying notes, barlines, repeats | 🟡 Medium |
| 08 | Advanced Chord Properties | Inversions, extended chords | 🔴 Hard |
| 09 | Melodic Analysis | Pitch contours, leap analysis | 🟡 Medium |
| 10 | Harmonic Rhythm | Chord change timing analysis | 🔴 Hard |

**Music Theory Level**: Intermediate (basic theory helpful)

**Python Level**: Beginner (variables, loops, functions)

---

### 03. Advanced Tutorials

**For**: Researchers, composers, advanced programmers
**Prerequisites**: Intermediate tutorials + strong Python/music theory
**Time**: 6-8 hours total

| # | Tutorial | Topics | Difficulty |
|---|----------|--------|------------|
| 01 | Pattern Finding | findMelodyPattern(), similarity metrics | 🔴 Hard |
| 02 | XPath Queries | Advanced XML queries on scores | 🔴 Hard |
| 03 | Score Collections | Batch processing multiple scores | 🔴 Hard |
| 04 | Statistical Analysis | scipy, hypothesis testing, correlations | 🔴 Hard |
| 05 | Algorithmic Composition | Markov chains, rule-based harmony | 🔴 Hard |
| 06 | Custom Analysis Functions | Extending maialib functionality | 🔴 Hard |

**Music Theory Level**: Advanced (theory knowledge required)

**Python Level**: Intermediate/Advanced

---

### 04. Visualization Tutorials

**For**: Anyone wanting to create graphs and plots
**Prerequisites**: Beginner tutorials recommended
**Time**: 3-4 hours total

| # | Tutorial | Topics | Difficulty |
|---|----------|--------|------------|
| 01 | Piano Rolls | plotPianoRoll(), visual score representation | 🟢 Easy |
| 02 | Pitch Envelopes | plotScorePitchEnvelope(), melodic contour | 🟢 Easy |
| 03 | Chord Analysis Plots | plotChordsNumberOfNotes(), harmonic density | 🟡 Medium |
| 04 | Parts Activity | plotPartsActivity(), orchestration analysis | 🟡 Medium |
| 05 | Combining Visualizations | Multi-panel figures, matplotlib integration | 🔴 Hard |
| 06 | Custom Plots | Seaborn, plotly interactive plots | 🔴 Hard |

**Music Theory Level**: Basic

**Python Level**: Basic (we explain plotting concepts)

---

### 05. Analysis Tutorials

**For**: Researchers, musicologists, composers
**Prerequisites**: Intermediate tutorials + Visualization
**Time**: 4-5 hours total

| # | Tutorial | Topics | Difficulty |
|---|----------|--------|------------|
| 01 | Harmonic Complexity | Chord complexity metrics, progression analysis | 🔴 Hard |
| 02 | Dissonance Curves | Sethares model, psychoacoustic analysis | 🔴 Hard |
| 03 | Corpus Statistics | Multi-score analysis, comparative studies | 🔴 Hard |

**Music Theory Level**: Advanced

**Python Level**: Intermediate (pandas, data analysis)

---

## 🎯 Learning Paths by Role

### I'm a Music Student
```
Getting Started → Beginner (01-04) → Visualization (01-02) → Intermediate (02-03)
```
**Goal**: Analyze scores for theory assignments

### I'm a Composer
```
Getting Started → Beginner (All) → Intermediate (05-07) → Advanced (05)
```
**Goal**: Generate and manipulate musical scores programmatically

### I'm a Music Teacher
```
Getting Started → Beginner (All) → Visualization (01-02) → Intermediate (02, 06)
```
**Goal**: Create teaching examples and exercises

### I'm a Researcher/Musicologist
```
All tutorials in order
```
**Goal**: Computational musicology, corpus analysis, statistical studies

### I'm a Data Scientist (Music Hobby)
```
Getting Started → Beginner (01-04) → Intermediate (02) → Visualization (All) → Analysis (All)
```
**Goal**: Apply data science to music

---

## 📖 Tutorial Format

Each tutorial includes:

- **📋 Introduction**: What you'll learn and why it matters
- **💻 Code Examples**: Step-by-step, fully commented
- **🎵 Musical Context**: Theory explained for non-experts
- **📊 Visualizations**: Graphs, tables, plots rendered inline
- **✏️ Practice Exercises**: Test your understanding (solutions included)
- **➡️ Next Steps**: Related tutorials and further reading

---

## 🛠️ Setup Instructions

### 1. Install Jupyter

```bash
pip install jupyter notebook
```

### 2. Navigate to Tutorials

```bash
cd path/to/maialib/python-tutorial
```

### 3. Launch Jupyter

```bash
jupyter notebook
```

### 4. Start Learning!

Open `00_getting_started/00_installation.ipynb` in your browser.

---

## 🔗 Additional Resources

### Official Documentation
- [Maialib API Reference](https://maialib.com) - Complete function documentation
- [GitHub Repository](https://github.com/nyckmaia/maialib) - Source code and issues

### Music Theory Resources (if needed)
- [musictheory.net](https://www.musictheory.net) - Free online lessons
- [teoria.com](https://www.teoria.com) - Exercises and reference

### Python Resources (if needed)
- [Python.org Tutorial](https://docs.python.org/3/tutorial/) - Official Python guide
- [Real Python](https://realpython.com) - Beginner-friendly tutorials

### Data Science Resources
- [Pandas Documentation](https://pandas.pydata.org/docs/) - DataFrame manipulation
- [Matplotlib Tutorials](https://matplotlib.org/stable/tutorials/index.html) - Plotting
- [Plotly Python](https://plotly.com/python/) - Interactive visualizations

---

## 💬 Getting Help

**Questions about tutorials?**
- 💬 [GitHub Discussions](https://github.com/nyckmaia/maialib/discussions)
- 🐛 [Report Issues](https://github.com/nyckmaia/maialib/issues)
- 📧 [Email](mailto:nyckmaia@gmail.com)

**Tips for Success:**
1. ✅ Run code cells in order (top to bottom)
2. ✅ Experiment! Change values and see what happens
3. ✅ Do the practice exercises
4. ✅ Don't skip the musical context sections
5. ✅ Join discussions to share your projects

---

## 🎵 Example Projects (Coming Soon)

Once you've completed the tutorials, try these projects:

- **Chord Progression Generator** - Create common progressions (I-IV-V-I, etc.)
- **Melody Harmonizer** - Add harmony to a melody automatically
- **Score Statistics Dashboard** - Analyze and visualize any score
- **Transposition Tool** - Batch transpose scores for different instruments
- **Pattern Finder** - Find recurring motifs in classical works

---

## 🤝 Contributing

Found a typo? Have an idea for a new tutorial? We welcome contributions!

1. Fork the repository
2. Make your improvements
3. Submit a Pull Request

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

---

## 📄 License

All tutorials are part of Maialib and are licensed under [GPL-3.0](../LICENSE).

You're free to:
- ✅ Use for learning and teaching
- ✅ Modify for your needs
- ✅ Share with students and colleagues

---

<div align="center">

**Happy Learning! 🎶**

*Made with ❤️ for musicians and researchers*

[⭐ Star us on GitHub](https://github.com/nyckmaia/maialib) | [💬 Join Discussions](https://github.com/nyckmaia/maialib/discussions)

</div>
