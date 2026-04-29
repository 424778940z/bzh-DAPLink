# Notes for AI agents

This branch (`toolchain-hs-wch-wchlinkE`) is an **orphan branch holding LFS-tracked WCH MounRiver toolchain pack archives** consumed at configure time by the firmware on [`fw-hs-wch-wchlinkE`](../../tree/fw-hs-wch-wchlinkE) via cmake FetchContent. See [README.md](README.md) for the layout and the consumer contract.

## What goes here

- `linux.zip`, `windows.zip` — LFS-tracked host platform toolchain archives (RISC-V GCC + binutils + newlib).
- `.gitattributes` — declares the `*.zip` LFS filter.
- `README.md` — describes the branch and its consumer.

## Editing rules

- **Do not add unrelated content** to this branch. It exists solely to host the toolchain blobs the firmware fetches; no firmware sources, build glue, IDE/agent config, etc.
- **When updating the toolchain version**, tag the new commit on this branch with a stable identifier (e.g. `wch-mrs-vX.Y.Z`) and update `WCH_TOOLCHAIN_GIT_TAG` on `fw-hs-wch-wchlinkE` to match. Existing tags must remain reachable so older firmware revisions stay buildable.
- **LFS objects** must be pushed to the remote (`git lfs push`) before the firmware branch's CI is pointed at the new tag — otherwise the cmake bootstrap will fall back to `WCH_TOOLCHAIN_CANONICAL_URL` (or fail entirely on a private fork without that fallback).
- **Editable on this branch:** [README.md](README.md), this file, `.gitattributes`, and the LFS pack archives. Everything else is out of scope.
