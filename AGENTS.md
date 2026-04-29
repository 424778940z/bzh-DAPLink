# Notes for AI agents

For project overview, USB identity, repository layout, build instructions, build options, forking, flashing, and porting — see [README.md](README.md).

This file collects conventions that AI coding agents working in this repo should follow.

## Conventions

- **Verify changes end-to-end:** build, flash, then check USB enumeration. Do not declare a change "done" purely on a successful compile.
- **One shell command at a time.** Do not chain destructive operations with `&&`, `;`, or pipes; the sandbox may reject the combined invocation.
- **Do not redirect to `/dev/null`** unless strictly needed — the sandbox flags it.
- **Use relative paths** for tooling.
- **Python:** set up the project venv via `bash utils/setup_venv.sh` (single entry point, also invoked by CMake's `PROJ_PYENV` target). Never call `python -m venv` directly and never install into the system Python.
