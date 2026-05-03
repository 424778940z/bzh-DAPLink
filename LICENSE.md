# License

The original code authored for this repository — everything outside `modules/` and `hal/`, except where a file's own SPDX header indicates otherwise — is licensed under the **GNU General Public License, version 3 or later (`GPL-3.0-or-later`)**.

The verbatim license text lives in [`LICENSE`](LICENSE) (the canonical GPL-3.0 text published by the Free Software Foundation, also available at <https://www.gnu.org/licenses/gpl-3.0.html>).

## Third-party components

This project bundles or links against the following upstream sources. **We respect the license of every component as published by its upstream**, and the original license text and SPDX identifiers are preserved on each file within those trees.

| Component | Path in this repo | Upstream license |
|---|---|---|
| ARM CMSIS-DAP | `modules/cmsis_dap/` | Apache-2.0 |
| TinyUSB | `modules/tinyusb/` | MIT |
| SEGGER RTT (bundled inside the TinyUSB submodule) | `modules/tinyusb/lib/SEGGER_RTT/` | SEGGER source-included BSD-style; see header banner in `RTT/SEGGER_RTT.h` |
| STMicroelectronics STM32CubeF4 (HAL, CMSIS device headers, board startup) | `modules/STM32CubeF4/` | STMicroelectronics — see per-file SPDX headers and `modules/STM32CubeF4/LICENSE.md`; CMSIS Core portions are Apache-2.0 |
| F411 startup, system init, syscalls/sysmem (re-distributed STM32 HAL files) | `hal/Startup/` | STMicroelectronics — see per-file SPDX headers |
| Arm GNU Toolchain (downloaded into `arm_toolchain/`, **not** redistributed in this repo) | gitignored, fetched by `cmake/toolchain_arm.cmake` | GCC: GPL-3.0-with-GCC-Exception; binutils: GPL-3.0-or-later; newlib: BSD-3-Clause / mixed — see `share/doc/` inside the downloaded toolchain |

## License concerns

If you believe any component in this repository is included in a way that violates its upstream license, the rights of its author, or any other applicable terms, please **open an issue on this repository** so the situation can be corrected promptly.
