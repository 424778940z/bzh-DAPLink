import re
import subprocess
import sys
from pathlib import Path

import click

MODULE_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = MODULE_DIR.parents[2]

EXTENSIONS = (".py",)


def find_tool():
    venv = PROJECT_ROOT / ".venv"
    candidates = [
        venv / "bin" / "ruff",          # POSIX (Linux, macOS)
        venv / "bin" / "ruff.exe",      # MSYS2 / Git Bash Python on Windows
        venv / "Scripts" / "ruff.exe",  # python.org installer on Windows
    ]
    for path in candidates:
        if path.exists():
            return str(path)
    click.echo("Error: ruff not found in .venv", err=True)
    sys.exit(1)


def load_excludes():
    path = MODULE_DIR / "style.py.exclude"
    if not path.exists():
        return []
    return [re.compile(p) for p in path.read_text().splitlines() if p.strip()]


def run(targets, mode="format"):
    tool = find_tool()
    config_file = MODULE_DIR / "ruff.toml"
    args = [tool, "format", f"--config={config_file}"]
    if mode == "check":
        args.append("--check")
    elif mode == "dry-run":
        args.append("--diff")
    args += targets

    click.echo(f"[Python] {mode}: {len(targets)} files")
    return subprocess.run(args).returncode
