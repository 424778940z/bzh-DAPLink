---
name: PythonEnv
description: Set up and use the project-wide Python virtual environment. Use when the user asks to run a Python script, install Python packages, set up the Python environment, or when any task requires Python execution within this project. Also use when a build or script fails due to missing Python dependencies.
---

# PythonEnv

The project uses a shared Python venv at `<repo-root>/.venv/`, defined in `cmake/project_common.cmake`.

## Setup

The venv is created and provisioned solely through the setup script:

```bash
bash utils/setup_venv.sh
```

This is the single entry point. CMake's `PROJ_PYENV` target invokes the same
script, so do not call `python -m venv` directly anywhere.

## Running Python

Always use the venv Python, never the system one:

```bash
.venv/bin/python <script.py>
```

## Installing Additional Packages

```bash
.venv/bin/pip install <package>
```

If the package is a project dependency, also add it to `utils/python_requirements.txt`.

## CMake Integration

- CMake variable `PYTHON_RUNNER` resolves to `.venv/bin/python`
- CMake target `PROJ_PYENV` creates the venv and installs requirements
- Build targets that depend on Python should depend on `PROJ_PYENV`
