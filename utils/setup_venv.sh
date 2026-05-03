#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="$SCRIPT_DIR/../.venv"

echo "Creating venv at $VENV_DIR..."
# --system-site-packages: pip may require host socks proxy support
python3 -m venv --system-site-packages "$VENV_DIR"

# Locate the venv's python (POSIX bin/, MSYS2 bin/, Windows native Scripts/)
for cand in "$VENV_DIR/bin/python" "$VENV_DIR/bin/python.exe" "$VENV_DIR/Scripts/python.exe"; do
    if [ -x "$cand" ]; then VENV_PY="$cand"; break; fi
done
if [ -z "${VENV_PY:-}" ]; then echo "Error: cannot locate venv python" >&2; exit 1; fi

echo "Installing requirements..."
"$VENV_PY" -m pip install -r "$SCRIPT_DIR/python_requirements.txt"

echo "Installing sitecustomize.py for project-wide pycache redirect..."
SITE_PACKAGES="$("$VENV_PY" -c "import sysconfig; print(sysconfig.get_path('purelib'))")"
cat > "$SITE_PACKAGES/sitecustomize.py" <<'PY'
"""Auto-loaded by Python at startup (via site.py).

Redirects this venv's __pycache__ output to <project_root>/.build/pycache
so source directories stay clean. Applies to any Python invocation through
this venv -- activated or not, including direct .venv/bin/python calls and
subprocesses spawned from them (via PYTHONPYCACHEPREFIX env var).
"""

import os
import sys
from pathlib import Path

_project_root = Path(sys.prefix).parent
_prefix = str(_project_root / ".build" / "pycache")

if not sys.pycache_prefix:
    sys.pycache_prefix = _prefix
os.environ.setdefault("PYTHONPYCACHEPREFIX", _prefix)
PY

echo "Done. Activate with: source $VENV_DIR/bin/activate"
