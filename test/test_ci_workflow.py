"""The wheel workflow must publish to PyPI only when a GitHub release is published."""

import unittest
from pathlib import Path

WORKFLOW = Path(__file__).resolve().parents[1] / ".github" / "workflows" / "wheels.yml"
GUARD = "if: github.event_name == 'release' && github.event.action == 'published'"


def jobBlock(text: str, job: str) -> str:
    lines = text.splitlines()
    start = lines.index(f"  {job}:")
    block = []
    for line in lines[start + 1 :]:
        if line.startswith("  ") and not line.startswith("    ") and line.strip():
            break
        block.append(line)
    return "\n".join(block)


class WheelWorkflowTestCase(unittest.TestCase):
    def test_upload_job_runs_only_for_a_published_release(self):
        block = jobBlock(WORKFLOW.read_text(encoding="utf-8"), "upload_all")
        self.assertIn("    " + GUARD, block.splitlines())
        self.assertIn("pypa/gh-action-pypi-publish", block)


if __name__ == "__main__":
    unittest.main()
