#!/usr/bin/env python3

import importlib
import os
import sys
from pathlib import Path

import click

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]

LANG_MODULES = {"c": "c_cpp.formatter", "py": "python.formatter"}

sys.path.insert(0, str(SCRIPT_DIR))


def load_formatter(lang_key):
    return importlib.import_module(LANG_MODULES[lang_key])


def is_excluded(filepath, patterns):
    # Normalize separators so forward-slash patterns in style.c.exclude match
    # both POSIX and Windows paths.
    s = str(filepath).replace(os.sep, "/")
    return any(p.search(s) for p in patterns)


def collect_files(base_path, exclude_patterns, extensions):
    files = []
    for root, _, names in os.walk(base_path):
        for name in names:
            if not name.endswith(extensions):
                continue
            full = os.path.join(root, name)
            if not is_excluded(full, exclude_patterns):
                files.append(full)
    return sorted(files)


@click.command()
@click.option("--project", is_flag=True, help="Format the entire project.")
@click.option("--path", type=click.Path(exists=True), help="Format files under a specific path.")
@click.option("--file", multiple=True, type=click.Path(exists=True), help="Format specific files.")
@click.option("--lang", type=click.Choice(["all", "c", "py"]), required=True, help="Language to format.")
@click.option("--check", is_flag=True, help="Exit non-zero if files are unformatted.")
@click.option("--dry-run", is_flag=True, help="Show diffs without modifying files.")
def main(project, path, file, lang, check, dry_run):
    modes = sum([project, bool(path), bool(file)])
    if modes == 0:
        raise click.UsageError("Must specify --project, --path, or --file")
    if modes > 1:
        raise click.UsageError("Use only one of --project, --path, or --file")
    if check and dry_run:
        raise click.UsageError("Use only one of --check or --dry-run")

    run_mode = "check" if check else "dry-run" if dry_run else "format"
    langs = LANG_MODULES.keys() if lang == "all" else [lang]
    rc = 0
    has_targets = False

    for lang_key in langs:
        fmt = load_formatter(lang_key)
        excludes = fmt.load_excludes()

        if file:
            targets = [f for f in file if f.endswith(fmt.EXTENSIONS) and not is_excluded(f, excludes)]
        else:
            base = PROJECT_ROOT if project else Path(path).resolve()
            targets = collect_files(base, excludes, fmt.EXTENSIONS)

        if targets:
            has_targets = True
            rc |= fmt.run(targets, run_mode)

    if not has_targets:
        click.echo("No matching files found")

    sys.exit(rc)


if __name__ == "__main__":
    main()
