# Notes for AI agents

This branch (`main`) is an **index only**. It carries no firmware sources, HAL, build system, or IDE/agent config — only [README.md](README.md), this file, [LICENSE](LICENSE), [LICENSE.md](LICENSE.md), and the firmware build CI under [.github/workflows/](.github/workflows/).

To do real work, switch to the appropriate branch listed in [README.md](README.md):

- **`fw-*`** — firmware ports. Each carries its own AGENTS.md, README, `.claude/` config, and a `build.sh [debug|release]` script tailored to that target's toolchain (ARM vs RISC-V), HAL, and skills.
- **`toolchain-*`** — orphan branches holding LFS-tracked vendor toolchain packs, fetched at configure time by the matching firmware branch.
- **`ref-*`** — vendor MCU / USB reference materials.

Branch naming: `{fw|toolchain|ref}-{fs|hs}-{brand}-{boardname}`.

## CI

[.github/workflows/build.yml](.github/workflows/build.yml) is a `workflow_dispatch`-triggered job that takes a `ref` input (branch, tag, or commit SHA), checks it out, runs `bash ./build.sh release` from that ref, and uploads the resulting firmware bundle as a workflow artifact named `<ref>-<short-sha>`. The per-branch `build.sh` is the contract — `main` does not know how each fw is built. To add a new buildable branch, add a `build.sh [debug|release]` to that branch's root; no workflow change required.

## Working on this branch

Valid edits on `main` are limited to README.md, AGENTS.md, LICENSE, LICENSE.md, and `.github/workflows/`. Adding sources, build files, or IDE/agent config to `main` is out of scope — they belong on a `fw-*` branch.
