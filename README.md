# WCH RISC-V toolchain packs (LFS)

This orphan branch (`toolchain-hs-wch-wchlinkE`) holds the platform toolchain archives consumed by the firmware branch [`fw-hs-wch-wchlinkE`](../../tree/fw-hs-wch-wchlinkE) via cmake FetchContent (see `cmake/toolchain.cmake` on that branch).

The firmware branch clones the ref pinned by `WCH_TOOLCHAIN_GIT_TAG` and extracts the host-appropriate pack into `wch_tools/`. To pin a specific toolchain version, tag a commit on this branch and override `WCH_TOOLCHAIN_GIT_TAG` at cmake configure time.

## Layout

- `linux.zip` — RISC-V GCC toolchain for Linux (LFS)
- `windows.zip` — RISC-V GCC toolchain for Windows (LFS)
- `README.md` — this file
