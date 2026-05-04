# bzh-DAPLink

Index for CMSIS-DAP debug-probe firmware ports. Each row is one target board; each branch column links to the branch carrying that artifact for that board. Switch to a firmware branch for sources, build system, and per-branch README.

| Board brand | Board name | Firmware | Toolchain | Reference |
|---|---|---|---|---|
| WCH | WCH-LinkE | [`fw-hs-wch-wchlinkE`](../../tree/fw-hs-wch-wchlinkE) | [`toolchain-hs-wch-wchlinkE`](../../tree/toolchain-hs-wch-wchlinkE) | [`ref-hs-wch-wchlinkE`](../../tree/ref-hs-wch-wchlinkE) |
| WeAct | STM32F411 | [`fw-fs-weact-stm32f411`](../../tree/fw-fs-weact-stm32f411) | — | — |

Branch naming: `{fw|toolchain|ref}-{fs|hs}-{brand}-{boardname}`.

`main` carries no sources or build system — it serves only as this index.
