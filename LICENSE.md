# License

The original code authored for this repository — everything outside `modules/` and `hal/`, except where a file's own SPDX header indicates otherwise — is licensed under the **GNU General Public License, version 3 or later (`GPL-3.0-or-later`)**.

The verbatim license text lives in [`LICENSE`](LICENSE) (the canonical GPL-3.0 text published by the Free Software Foundation, also available at <https://www.gnu.org/licenses/gpl-3.0.html>).

## Third-party components

This project bundles or links against the following upstream sources. **We respect the license of every component as published by its upstream**, and the original license text and SPDX identifiers are preserved on each file within those trees.

| Component | Path in this repo | Upstream license |
|---|---|---|
| ARM CMSIS-DAP | `modules/cmsis_dap/` | Apache-2.0 |
| TinyUSB | `modules/tinyusb/` | MIT |
| WCH CH32V30x peripheral HAL & startup | `hal/Peripheral/`, `hal/Startup/` | WCH vendor field-of-use (file `hal/Startup/system_ch32v30x.c` carries an Apache-2.0 SPDX header) |
| WCH MounRiver toolchain pack | `wch_tools/` (sourced from the orphan `toolchain-hs-wch-wchlinkE` branch of this repo) | WCH redistribution terms shipped with the SDK |
| WCH MCU reference materials & Microsoft USB spec doc | `reference/` (sourced from the separate `ref-hs-wch-wchlinkE` branch of this repo) | WCH; Microsoft Corp. (`MS_OS_2_0_desc.docx`) — kept for educational study only, see [`README.md`](https://github.com/424778940z/bzh-DAPLink/blob/ref-hs-wch-wchlinkE/README.md) on the `ref-hs-wch-wchlinkE` branch |

## License concerns

If you believe any component in this repository is included in a way that violates its upstream license, the rights of its author, or any other applicable terms, please **open an issue on this repository** so the situation can be corrected promptly.
