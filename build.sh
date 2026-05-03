#!/usr/bin/env bash
set -euo pipefail

mode="${1:-release}"
case "$mode" in
  release) preset=arm-toolchain-release ;;
  debug)   preset=arm-toolchain-debug   ;;
  *) echo "usage: $0 [debug|release]" >&2; exit 2 ;;
esac

cmake --preset "$preset"
cmake --build --preset "$preset"
