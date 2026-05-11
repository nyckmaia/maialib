"""Generate AI_API_CHEATSHEET.md from committed .pyi stubs.

Parses maialib/maiacore/maiacore.pyi and maialib/maiapy/*.pyi via the stdlib
`ast` module and emits a single-page Markdown surface aimed at being pasted
into an LLM system prompt. Preserves a `<!-- BEGIN MANUAL -->/<!-- END MANUAL -->`
block so curated recipes/gotchas survive regeneration.

Run:
    python scripts/build-cheatsheet.py
or:
    make build-cheatsheet
"""

from __future__ import annotations

import ast
import re
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
OUT = REPO / "AI_API_CHEATSHEET.md"
STUB_INPUTS = [
    REPO / "maialib" / "maiacore" / "maiacore.pyi",
    REPO / "maialib" / "maiapy" / "other.pyi",
    REPO / "maialib" / "maiapy" / "plots.pyi",
    REPO / "maialib" / "maiapy" / "sethares_dissonance.pyi",
]

MANUAL_BEGIN = "<!-- BEGIN MANUAL -->"
MANUAL_END = "<!-- END MANUAL -->"

DEFAULT_MANUAL_BLOCK = f"""{MANUAL_BEGIN}
## Quick recipes

```python
# Count chord qualities in Beethoven's 5th
import maialib as ml
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Beethoven_Symphony_5th))
df = score.getChordsDataFrame()
print(df['quality'].value_counts())
```

```python
# Find a melodic pattern (C-E-G) across all parts
import maialib as ml
score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
matches = score.findMelodyPatternDataFrame(["C4", "E4", "G4"])
print(matches)
```

## Gotchas

- **MusicXML only** — `.xml`, `.mxl`, `.musicxml`. Not `.mid`.
- **camelCase methods** — `getPitchClass`, never `get_pitch_class`.
- **`Score(path)` takes `str`** — wrap `pathlib.Path` with `str()`.
- **Samples ship bundled** — use `ml.SampleScore` + `ml.getSampleScorePath()`.
- **No async** — everything is synchronous.
{MANUAL_END}
"""


def _annotation(node: ast.AST | None) -> str:
    if node is None:
        return ""
    try:
        return ast.unparse(node)
    except Exception:
        return "?"


def _format_args(args: ast.arguments) -> str:
    parts: list[str] = []
    pos = list(args.args)
    defaults = [None] * (len(pos) - len(args.defaults)) + list(args.defaults)
    for a, d in zip(pos, defaults):
        if a.arg == "self":
            continue
        s = a.arg
        if a.annotation is not None:
            s += f": {_annotation(a.annotation)}"
        if d is not None:
            s += f" = {_annotation(d)}"
        parts.append(s)
    return ", ".join(parts)


def _format_func(func: ast.FunctionDef | ast.AsyncFunctionDef) -> str:
    name = func.name
    args = _format_args(func.args)
    ret = _annotation(func.returns) or "None"
    return f"- `{name}({args}) -> {ret}`"


def _is_overload(func: ast.FunctionDef | ast.AsyncFunctionDef) -> bool:
    for d in func.decorator_list:
        s = _annotation(d)
        if s.endswith("overload") or s == "typing.overload":
            return True
    return False


def _public(name: str) -> bool:
    return not name.startswith("_") or name == "__init__"


def render_module(path: Path) -> str:
    if not path.exists():
        return ""
    tree = ast.parse(path.read_text(encoding="utf-8"))
    lines: list[str] = []
    mod_label = path.stem if path.parent.name != "maiacore" else "maiacore (C++ bindings)"
    lines.append(f"### `{mod_label}`")
    lines.append("")

    free_funcs: list[str] = []
    classes: list[tuple[str, list[str]]] = []

    for node in tree.body:
        if isinstance(node, ast.ClassDef):
            methods: list[str] = []
            seen_overload: set[str] = set()
            for item in node.body:
                if isinstance(item, (ast.FunctionDef, ast.AsyncFunctionDef)) and _public(item.name):
                    if _is_overload(item):
                        if item.name in seen_overload:
                            continue
                        seen_overload.add(item.name)
                        methods.append(_format_func(item) + "  *(overloaded)*")
                    else:
                        methods.append(_format_func(item))
            classes.append((node.name, methods))
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and _public(node.name):
            free_funcs.append(_format_func(node))

    if free_funcs:
        lines.append("**Module-level functions:**")
        lines.extend(free_funcs)
        lines.append("")

    for name, methods in sorted(classes):
        if not methods:
            continue
        lines.append(f"#### Class `{name}`")
        lines.extend(methods)
        lines.append("")

    return "\n".join(lines)


def load_manual_block() -> str:
    if not OUT.exists():
        return DEFAULT_MANUAL_BLOCK
    text = OUT.read_text(encoding="utf-8")
    m = re.search(
        re.escape(MANUAL_BEGIN) + r".*?" + re.escape(MANUAL_END),
        text,
        flags=re.DOTALL,
    )
    return m.group(0) if m else DEFAULT_MANUAL_BLOCK


def main() -> None:
    manual = load_manual_block()
    parts: list[str] = []
    parts.append("# maialib API cheatsheet")
    parts.append("")
    parts.append(
        "> Paste this into your AI assistant as context. Auto-generated from `.pyi` "
        "stubs by `scripts/build-cheatsheet.py`. Do not edit method signatures "
        "here — edit the C++ source or stubs."
    )
    parts.append("")
    parts.append("## Import")
    parts.append("")
    parts.append("```python")
    parts.append("import maialib as ml")
    parts.append("```")
    parts.append("")
    parts.append("## Loading scores")
    parts.append("")
    parts.append("```python")
    parts.append("score = ml.Score('path/to/file.musicxml')")
    parts.append("# or use a bundled sample:")
    parts.append("score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))")
    parts.append("```")
    parts.append("")
    parts.append("## API surface")
    parts.append("")
    for p in STUB_INPUTS:
        section = render_module(p)
        if section:
            parts.append(section)
    parts.append(manual)
    parts.append("")

    text = "\n".join(parts)
    if not text.endswith("\n"):
        text += "\n"
    OUT.write_text(text, encoding="utf-8")
    print(f"Wrote {OUT.relative_to(REPO)}")


if __name__ == "__main__":
    main()
