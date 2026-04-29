#!/usr/bin/env bash
set -euo pipefail

mode="${1:-release}"
case "$mode" in
  release) preset=wch-toolchain-release ;;
  debug)   preset=wch-toolchain-debug   ;;
  *) echo "usage: $0 [debug|release]" >&2; exit 2 ;;
esac

cmake --preset "$preset"
cmake --build --preset "$preset"
