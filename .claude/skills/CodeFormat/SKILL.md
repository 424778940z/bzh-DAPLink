---
name: CodeFormat
description: Format C/C++ and Python source code using the project's code_format.py script. Use when the user asks to format code, run clang-format, ruff format, fix code style, or apply code formatting to a file, folder, or module. Also use after large code changes when the user wants to clean up formatting.
---

# CodeFormat

Format C/C++ (`.c`, `.h`, `.cpp`, `.hpp`) and Python (`.py`) files using the project's `code_format.py` script.

## Usage

```bash
${PROJECT_ROOT}/.venv/bin/python3 ${PROJECT_ROOT}/utils/code_format/code_format.py --lang <all|c|py> --project [--check] [--dry-run]
${PROJECT_ROOT}/.venv/bin/python3 ${PROJECT_ROOT}/utils/code_format/code_format.py --lang <all|c|py> --path <folder_path> [--check] [--dry-run]
${PROJECT_ROOT}/.venv/bin/python3 ${PROJECT_ROOT}/utils/code_format/code_format.py --lang <all|c|py> --file <file1> --file <file2> [--check] [--dry-run]
```

The script must be run with the project venv (`${PROJECT_ROOT}/.venv/bin/python3`), which is set up by `utils/setup_venv.sh` or automatically by the CMake build.

- `--lang all`: format both C/C++ and Python
- `--lang c`: format only C/C++ files (uses clang-format)
- `--lang py`: format only Python files (uses ruff)
- `--project`: format the entire project
- `--path <dir>`: format only files under that directory
- `--file <file>`: format specific files (repeatable)
- `--check`: exit non-zero if files are unformatted (for CI)
- `--dry-run`: show diffs without modifying files
- `--lang` is required
- Exactly one of `--project`, `--path`, or `--file` is required

## Arguments

- When the user specifies a module or folder, use `--path` (e.g., `--path src/usb`)
- When the user says "format this file", use `--file` with the file path
- When the user says "format this folder", use `--path` with the folder path
- Use absolute paths to avoid ambiguity
- If the user says "clangformat only" or "C only", use `--lang c`
- If the user says "ruff only" or "python only", use `--lang py`
- If the user doesn't specify a language, use `--lang all`

## Exclusions

C/C++ files matching patterns in `utils/code_format/c_cpp/style.c.exclude` are automatically skipped (vendor HAL, third-party TinyUSB ports, build/derived dirs, reference materials). Python excludes are configured in `utils/code_format/python/ruff.toml`. Each language module manages its own exclusions.

## After Formatting

Rebuild the affected target to confirm the formatting didn't break anything:

```bash
cmake --build .build
```
