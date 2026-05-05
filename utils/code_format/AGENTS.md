# Code Formatting System

This directory contains all code formatting configuration and tooling for the project. It serves two consumers: the CLI script (`code_format.py`) and VS Code local extensions (`.vscode/extensions/`).

## Directory Structure

```
utils/code_format/
├── code_format.py              # CLI entry point, dispatches to language modules
├── c_cpp/
│   ├── formatter.py            # C/C++ formatting module
│   ├── .clang-format           # clang-format style config
│   ├── style.c.exclude         # regex patterns for files to skip
│   └── style.c.include         # file include patterns
└── python/
    ├── formatter.py            # Python formatting module
    └── ruff.toml               # ruff format config

.vscode/extensions/
└── code-format-c-cpp/          # VS Code extension: C/C++ formatting
    ├── package.json
    └── extension.js
```

## Architecture

### Two consumers, one config

Each language has a single config file that is the source of truth. Both the CLI and the VS Code extensions point to the same file:

| Language   | Config File                | CLI Tool       | VS Code Extension         |
|------------|----------------------------|----------------|---------------------------|
| C/C++      | `c_cpp/.clang-format`      | clang-format   | `code-format-c-cpp`       |
| Python     | `python/ruff.toml`         | ruff           | ruff (marketplace ext)    |

Python does not have a local VS Code extension — it uses the marketplace `charliermarsh.ruff` extension directly, configured via `.vscode/settings.json`.

### Formatter module interface

Each `formatter.py` exports three things:

- `EXTENSIONS: tuple` — file extensions this formatter handles (e.g. `(".c", ".h", ".cpp", ".hpp")`)
- `load_excludes() -> list` — returns compiled regex patterns for files to skip
- `run(targets: list, dry_run: bool) -> int` — runs the formatter, returns exit code

### VS Code extensions

The local extensions live in `.vscode/extensions/`. VS Code discovers them and shows them under the **Recommended** section in the Extensions view, where they can be clicked to install for the workspace.

Each extension reads its config from `utils/code_format/<lang>/` using a `CONFIG_RELATIVE_PATH` constant resolved against the workspace root. They use `execFileSync` (not `execSync`) to avoid shell injection.

Extension IDs follow the pattern `local.<name>` where `local` is the publisher field in `package.json`.

## CLI Usage

```bash
# format all languages across the entire project
python utils/code_format/code_format.py --lang all --project

# format only C/C++ files under a path
python utils/code_format/code_format.py --lang c --path src/

# dry-run check specific files
python utils/code_format/code_format.py --lang py --file debug_usb.py --dry-run

# CI-style check: exit non-zero if anything would be reformatted
python utils/code_format/code_format.py --lang all --project --check
```

Valid `--lang` values: `all`, `c`, `py`

## Adding a New Language

1. Create a new directory under `utils/code_format/<lang>/`
2. Add the formatter config file(s) to that directory
3. Create `formatter.py` implementing `EXTENSIONS`, `load_excludes()`, and `run(targets, dry_run)`
4. Register it in `code_format.py` `LANG_MODULES` dict and the `--lang` choice list
5. If VS Code integration is needed, create a new extension in `.vscode/extensions/code-format-<lang>/` with `package.json` and `extension.js` following the existing pattern
6. Update `.vscode/settings.json` with the new formatter's `editor.defaultFormatter`

## Maintaining Config Files

When modifying a config file (e.g. `.clang-format`, `ruff.toml`):
- The change applies to both CLI and VS Code automatically since both point to the same file
- No rebuild or repackage is needed for CLI usage
- VS Code extensions must be repackaged and reinstalled if `extension.js` or `package.json` changed, but config-only changes take effect on next format invocation

## Updating VS Code Extensions

When `extension.js` or `package.json` is modified, reload the VS Code window to pick up changes. Config-only changes take effect on the next format invocation without a reload.
