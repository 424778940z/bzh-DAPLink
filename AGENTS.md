# Notes for AI agents

This branch (`main`) is an **index only**. It carries no firmware, build system, sources, or IDE/agent config — only [README.md](README.md), this file, [LICENSE](LICENSE), and [LICENSE.md](LICENSE.md).

To do real work, switch to the appropriate branch listed in [README.md](README.md):

- **`fw-*`** — firmware ports. Each carries its own AGENTS.md, README, and `.claude/` config tailored to that target's toolchain (ARM vs RISC-V), HAL, and skills.
- **`toolchain-*`** — orphan branches holding LFS-tracked vendor toolchain packs, fetched at configure time by the matching firmware branch.
- **`ref-*`** — vendor MCU / USB reference materials.

Branch naming: `{fw|toolchain|ref}-{fs|hs}-{brand}-{boardname}`.

## Working on this branch

Valid edits on `main` are limited to README.md, AGENTS.md, LICENSE, and LICENSE.md. Adding sources, build files, or IDE/agent config to `main` is out of scope — they belong on a `fw-*` branch.
