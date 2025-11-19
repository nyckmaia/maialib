# 🤝 Contributing to Maialib

Thank you for your interest in contributing to Maialib! This project thrives because of contributions from musicians, researchers, and developers like you.

---

## 🎯 Ways to Contribute

### 🐛 Report Bugs

Found a bug? Help us fix it!

1. **Search existing issues** to avoid duplicates
2. **Create a new issue** with:
   - Clear, descriptive title
   - Steps to reproduce
   - Expected vs. actual behavior
   - Platform and Python version
   - Code snippet (if applicable)
   - Error messages or stack traces

[Report a bug →](https://github.com/nyckmaia/maialib/issues/new?labels=bug)

### ✨ Request Features

Have an idea for a new feature?

1. **Check existing feature requests** to avoid duplicates
2. **Create a new issue** explaining:
   - What problem it solves
   - How it would work
   - Example use cases
   - Why it's important for musicians/researchers

[Request a feature →](https://github.com/nyckmaia/maialib/issues/new?labels=enhancement)

### 📚 Improve Documentation

Documentation improvements are always welcome:

- Fix typos or unclear explanations
- Add examples to existing docs
- Write tutorials or guides
- Translate documentation
- Improve code comments

**No coding required!** Just edit the files and submit a PR.

### 🎵 Share Example Scores

Help others learn by contributing:

- Jupyter notebooks with analysis examples
- Sample scores (public domain or Creative Commons)
- Tutorial walkthroughs
- Use case demonstrations

### 💻 Contribute Code

Ready to dive into the codebase?

- Fix bugs
- Implement new features
- Improve performance
- Add tests
- Refactor code

---

## 🚀 Getting Started

### 1. Fork the Repository

Click the "Fork" button at the top of the [Maialib repository](https://github.com/nyckmaia/maialib).

### 2. Clone Your Fork

```bash
git clone https://github.com/YOUR_USERNAME/maialib.git
cd maialib
```

### 3. Set Up Development Environment

**Install dependencies:**
```bash
pip install pathlib cpplint wheel mypy pybind11-stubgen
```

**Build from source:**
```bash
make
```

See [BUILDING.md](BUILDING.md) for detailed build instructions.

### 4. Create a Branch

```bash
git checkout -b feature/my-awesome-feature
# or
git checkout -b fix/issue-123-bug-description
```

**Branch naming conventions:**
- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation changes
- `refactor/` - Code refactoring
- `test/` - Test additions/fixes

### 5. Make Your Changes

- Write clear, readable code
- Follow existing code style
- Add tests for new functionality
- Update documentation if needed

### 6. Test Your Changes

**Run all tests:**
```bash
make tests
```

**Run specific test suites:**
```bash
make cpp-tests    # C++ unit tests
make py-tests     # Python unit tests
make validate     # Code quality checks
```

### 7. Commit Your Changes

Write clear, descriptive commit messages:

```bash
git add .
git commit -m "Add feature: melodic similarity algorithm

- Implement dynamic time warping for melody comparison
- Add unit tests covering edge cases
- Update documentation with usage examples"
```

**Commit message format:**
```
<type>: <brief summary>

<detailed description>

<optional footer with issue references>
```

**Types:**
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `style:` - Code style changes (formatting, no logic change)
- `refactor:` - Code refactoring
- `test:` - Adding or updating tests
- `chore:` - Maintenance tasks

### 8. Push to Your Fork

```bash
git push origin feature/my-awesome-feature
```

### 9. Create a Pull Request

1. Go to your fork on GitHub
2. Click "New Pull Request"
3. Fill in the PR template:
   - Clear title describing the change
   - What problem does it solve?
   - How does it work?
   - Any breaking changes?
   - Related issues (use `Fixes #123` or `Closes #456`)

---

## 📋 Code Guidelines

### C++ Code Style

**Standards:**
- C++17 language features
- 100 characters maximum line length
- Use meaningful variable names
- Document all public APIs with Doxygen comments

**Example:**
```cpp
/**
 * @brief Calculates the harmonic complexity of a chord.
 * @param useEnharmony If true, considers enharmonic equivalents.
 * @return Normalized complexity value (0.0 to 1.0).
 * @details Uses intervallic analysis in closed-stack configuration
 *          to quantify harmonic tension and dissonance.
 */
float getCloseStackHarmonicComplexity(const bool useEnharmony = false);
```

**Run code validation:**
```bash
make validate  # Runs cpplint and cppcheck
```

### Python Code Style

**Standards:**
- Follow PEP 8 style guide
- Use type hints where appropriate
- Write docstrings for all public functions
- Keep functions focused and small
- Use Ruff for formatting and linting

**Example:**
```python
def calculate_melodic_similarity(
    melody1: List[Note],
    melody2: List[Note],
    threshold: float = 0.8
) -> float:
    """
    Calculate similarity between two melodic sequences.

    Args:
        melody1: First melody as list of Note objects
        melody2: Second melody to compare
        threshold: Minimum similarity threshold (0.0-1.0)

    Returns:
        Similarity score between 0.0 and 1.0

    Example:
        >>> m1 = [Note("C4"), Note("D4"), Note("E4")]
        >>> m2 = [Note("C4"), Note("D4"), Note("F4")]
        >>> calculate_melodic_similarity(m1, m2)
        0.85
    """
    # Implementation...
```

### Code Formatting

**Before submitting a pull request, format your code:**

#### Automatic Formatting

```bash
make format          # Format all C++ and Python code
make format-cpp      # Format only C++ code
make format-python   # Format only Python code
```

#### Code Linting

```bash
make validate        # Run cpplint (C++) and Ruff linting (Python)
make lint-python     # Run Ruff linter on Python code
make lint-python-fix # Auto-fix Python linting issues
```

#### Pre-commit Hooks (Optional but Recommended)

Install pre-commit hooks to automatically format code before each commit:

```bash
pip install pre-commit
pre-commit install
```

The hooks will:
- Format C++ code with clang-format
- Format Python code with Ruff
- Fix common issues (trailing whitespace, line endings)
- Validate YAML files

**Configuration:**
- C++: `.clang-format` (Google style, 100 chars, 4 spaces)
- Python: `pyproject.toml` (Ruff configuration)
- Pre-commit: `.pre-commit-config.yaml`

### Testing Requirements

**All contributions must include tests:**

**C++ tests (GoogleTest):**
- Add tests in `tests-cpp/src/`
- Cover edge cases and error conditions
- Use descriptive test names

```cpp
TEST(ChordTest, IsDiminishedReturnsTrueForDimChord) {
    Chord chord({"C4", "Eb4", "Gb4"});
    EXPECT_TRUE(chord.isDiminished());
}

TEST(ChordTest, IsDiminishedReturnsFalseForMajorChord) {
    Chord chord({"C4", "E4", "G4"});
    EXPECT_FALSE(chord.isDiminished());
}
```

**Python tests (unittest):**
- Add tests in `test/`
- Follow existing test structure
- Test both success and failure cases

```python
class TestMelodicSimilarity(unittest.TestCase):
    def test_identical_melodies_return_one(self):
        melody = [Note("C4"), Note("D4"), Note("E4")]
        similarity = calculate_melodic_similarity(melody, melody)
        self.assertEqual(similarity, 1.0)

    def test_different_melodies_return_less_than_one(self):
        melody1 = [Note("C4"), Note("D4"), Note("E4")]
        melody2 = [Note("G4"), Note("A4"), Note("B4")]
        similarity = calculate_melodic_similarity(melody1, melody2)
        self.assertLess(similarity, 1.0)
```

---

## 📝 Documentation Guidelines

### Doxygen Comments (C++)

All public APIs must have Doxygen documentation:

```cpp
/**
 * @brief One-line summary (required)
 * @param paramName Description of parameter (required for each param)
 * @return Description of return value (required if non-void)
 * @details Extended explanation with examples and use cases (optional)
 * @note Special notes or caveats (optional)
 * @warning Important warnings (optional)
 * @example
 * @code
 * Chord chord({"C4", "E4", "G4"});
 * float complexity = chord.getHarmonicComplexity();
 * @endcode
 */
```

### Docstrings (Python)

Use Google-style docstrings:

```python
def function_name(param1: Type1, param2: Type2) -> ReturnType:
    """One-line summary.

    Extended description explaining the function's purpose,
    behavior, and any important details.

    Args:
        param1: Description of param1
        param2: Description of param2

    Returns:
        Description of return value

    Raises:
        ExceptionType: When and why this exception is raised

    Example:
        >>> result = function_name(arg1, arg2)
        >>> print(result)
        expected_output
    """
```

### README and Tutorial Updates

If your PR adds features, update:
- **README.md** - Add to examples or features section
- **CHANGELOG.md** - Document the change
- **Python tutorials** - Create or update relevant notebooks

---

## 🎯 Specific Contribution Areas

### 🎵 For Musicians

**No programming required:**
- Report unclear documentation
- Suggest user-friendly feature improvements
- Share analysis workflows and use cases
- Contribute example scores (public domain)
- Write beginner tutorials

### 🔬 For Researchers

**Academic contributions welcome:**
- Implement published algorithms (cite sources!)
- Add analysis methods from musicology research
- Contribute corpus analysis tools
- Write research-focused tutorials
- Share citation information for papers using Maialib

### 💻 For Developers

**Technical contributions:**
- Performance optimizations
- New analysis algorithms
- API improvements
- Cross-platform compatibility
- Build system enhancements

---

## ⚠️ What NOT to Do

- ❌ Submit PRs without tests
- ❌ Ignore code style guidelines
- ❌ Make breaking changes without discussion
- ❌ Copy code without proper attribution
- ❌ Include copyrighted musical scores
- ❌ Submit large PRs mixing unrelated changes

**Instead:**
- ✅ Discuss major changes in an issue first
- ✅ Keep PRs focused on one change
- ✅ Write clear commit messages
- ✅ Update documentation
- ✅ Add tests for new code

---

## 🔄 Review Process

### What to Expect

1. **Automatic checks** run on your PR:
   - Code style validation
   - All tests must pass
   - No new compiler warnings

2. **Maintainer review** (usually within 1 week):
   - Code quality assessment
   - Design review
   - Test coverage verification

3. **Feedback and iteration**:
   - Address review comments
   - Update based on feedback
   - Re-request review when ready

4. **Merge**:
   - Once approved, your PR will be merged
   - You'll be added to contributors list! 🎉

### Tips for Faster Reviews

- Keep PRs small and focused
- Write clear descriptions
- Respond promptly to feedback
- Ensure all tests pass
- Update documentation

---

## 🏆 Recognition

Contributors are recognized in:
- **GitHub contributors page**
- **CHANGELOG.md** - Credit for each contribution
- **Project README** - Link to contributors
- **Release notes** - Acknowledgment of contributions

---

## 💬 Communication

### Where to Ask Questions

- **[GitHub Discussions](https://github.com/nyckmaia/maialib/discussions)** - General questions, ideas, show-and-tell
- **[Issue Comments](https://github.com/nyckmaia/maialib/issues)** - Questions about specific issues
- **[Pull Request Comments](https://github.com/nyckmaia/maialib/pulls)** - Questions about code changes
- **[Email](mailto:nyckmaia@gmail.com)** - Private or sensitive matters

### Best Practices

- Be respectful and constructive
- Search before asking (avoid duplicates)
- Provide context and examples
- Follow up on your questions
- Help others when you can

---

## 📜 License

By contributing to Maialib, you agree that your contributions will be licensed under the **GNU General Public License v3.0** (GPL-3.0).

This means:
- Your code becomes part of Maialib
- Others can use, modify, and distribute your contributions
- Your contributions must also be open source
- You retain copyright to your work

See [LICENSE](LICENSE) for full details.

---

## 🙏 Thank You!

Every contribution, big or small, makes Maialib better for musicians and researchers worldwide. We appreciate your time and effort!

**Questions about contributing?**
- 💬 [Start a discussion](https://github.com/nyckmaia/maialib/discussions)
- 📧 [Email the maintainer](mailto:nyckmaia@gmail.com)

---

<div align="center">

**Happy Contributing! 🎵**

[View Open Issues](https://github.com/nyckmaia/maialib/issues) | [See Good First Issues](https://github.com/nyckmaia/maialib/labels/good%20first%20issue) | [Join Discussions](https://github.com/nyckmaia/maialib/discussions)

</div>
