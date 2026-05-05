import glob
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import click

MODULE_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = MODULE_DIR.parents[2]

EXTENSIONS = (".c", ".h", ".cpp", ".hpp")

# VS Code's ms-vscode.cpptools extension bundles its own clang-format. This glob
# covers Linux (linux-x64, linux-arm64), macOS (darwin-x64, darwin-arm64), and
# Windows (win32-x64), and picks up the .exe suffix on Windows naturally.
CPPTOOLS_GLOB = os.path.expanduser("~/.vscode/extensions/ms-vscode.cpptools-*/LLVM/bin/clang-format*")


def find_tool():
    matches = sorted(glob.glob(CPPTOOLS_GLOB))
    if matches:
        return matches[-1]
    on_path = shutil.which("clang-format")
    if on_path:
        return on_path
    click.echo("Error: clang-format not found", err=True)
    sys.exit(1)


def load_excludes():
    path = MODULE_DIR / "style.c.exclude"
    if not path.exists():
        return []
    return [re.compile(p) for p in path.read_text().splitlines() if p.strip()]


def run(targets, mode="format"):
    tool = find_tool()
    style_file = MODULE_DIR / ".clang-format"
    args = [tool, f"--style=file:{style_file}", "--verbose"]
    if mode == "check":
        args += ["--dry-run", "--Werror"]
    elif mode == "dry-run":
        args.append("--dry-run")
    else:
        args.append("-i")
    args += targets

    click.echo(f"[C/C++] {mode}: {len(targets)} files")
    return subprocess.run(args).returncode
