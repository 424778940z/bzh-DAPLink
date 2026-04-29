# BZH_DAPLink_WchLinkE

CMSIS-DAP debug probe firmware for the **WCH-LinkE** reference board (CH32V305FBP6, RV32 @ 144 MHz; see *Porting* for other CH32V305FBP6 designs). 

## Repository layout

| Path                         | Contents                                                                |
|------------------------------|-------------------------------------------------------------------------|
| `src/`                       | Project firmware sources                                                |
| `src/usb/tinyusb_port/`      | TinyUSB device callbacks, descriptors, vendor/HID/CDC class glue        |
| `src/usb/tinyusb_port/wch/`  | TinyUSB DCD ports for CH32 USBHS / USBFS controllers                    |
| `src/DAP_config.h`           | CMSIS-DAP target config: pinout, clock, LED                             |
| `hal/`                       | WCH CH32V30x peripheral HAL, startup, linker script                     |
| `modules/cmsis_dap/`         | ARM CMSIS-DAP submodule                                                 |
| `modules/tinyusb/`           | TinyUSB submodule                                                       |
| `cmake/`                     | CMake toolchain bootstrap, project glue, compile flags                  |
| `wch_tools/`                 | Derived (gitignored). Populated by cmake on first configure.            |

Two related trees live on separate branches: `toolchain-hs-wch-wchlinkE` (orphan, holds the LFS-tracked WCH MounRiver toolchain pack zips fetched by cmake), and `ref-hs-wch-wchlinkE` (WCH MCU / USB reference material for study; see that branch's own `README.md`).

## Forking

When forking on GitHub, **deselect "Copy the `main` branch only"** in the fork dialog so the `toolchain-hs-wch-wchlinkE` branch and the `wch-mrs-v2.3.0` tag come along.

GitHub doesn't replicate LFS storage across forks. The cmake bootstrap handles that: on LFS miss against your fork's `origin`, it falls back to `WCH_TOOLCHAIN_CANONICAL_URL` (defaults to the canonical public URL) over unauthenticated HTTPS — no extra flags needed for a routine fork build. To carry the blobs in your fork instead:

```sh
git lfs fetch origin wch-mrs-v2.3.0
git lfs push origin wch-mrs-v2.3.0
```

Configure-time toolchain overrides: `-DWCH_TOOLCHAIN_GIT_REPO=<url>` (replaces origin, disables canonical fallback), `-DWCH_TOOLCHAIN_GIT_TAG=<ref>`, `-DWCH_TOOLCHAIN_CANONICAL_URL=<url>`.

## Building

Requires `cmake >= 3.28`, `ninja`, `git`, `git-lfs`. Network access on first configure.

```sh
cmake -B .build -G Ninja
cmake --build .build
```

First configure fetches the `toolchain-hs-wch-wchlinkE` branch and extracts the host pack zip into `wch_tools/`. Outputs land in `.build/` as `BZH_DAPLink_WchLinkE-<YYYYMMDD>-<short-sha>.{elf,hex,bin,lst,map}`.

## Build options

Set in [`cmake/compile_options.cmake`](cmake/compile_options.cmake) under `PROJ_WIDE_DEFINES`:

| Macro | Effect when defined |
|---|---|
| `DAP_FW_V1` | CMSIS-DAP v1 (HID) interface |
| `DAP_FW_V2` | CMSIS-DAP v2 (Vendor / WinUSB) interface |
| `DAP_CDC` | USB CDC ACM bridged to UART3 |
| `DAP_PWR_OUT` | Drive PA5 / PB12 as power-output pins during board init |

`DEBUG_OUTPUT` (debug-log sink) is wired to the build configuration via a generator expression: `Debug` → `DEBUG_OUTPUT_SDI`, `Release` → `DEBUG_OUTPUT_NONE`. The `DEBUG_OUTPUT_UART` alternative is left as a commented-out swap in the same block — it routes through UART3 and is mutually exclusive with `DAP_CDC` (enforced at compile time by `src/uart3_conflict_checker.c`).

## Flashing

Use any WCH-compatible flasher (MounRiver Studio flash tool, `wchisp`, OpenOCD with a WCH-RISC-V config, etc.) targeting the built `.elf` or `.hex`. The cmake-managed toolchain pack ships GCC + binutils only — bring your own flasher.

Verify enumeration after flashing:
- Linux: `lsusb -d 1A86:8011 -v`
- Windows: USB Tree View, or `Get-PnpDevice -Class USB`

## USB identity

| Field   | Value                                          |
|---------|------------------------------------------------|
| VID:PID | `0x1A86:0x8011` (official WCH-LinkE DAP)       |
| Class   | Composite (IAD)                                |
| DAP v1  | HID interface, 64-byte report                  |
| DAP v2  | Vendor (WinUSB) bulk endpoints                 |
| CDC     | UART bridge (gated by `DAP_CDC`, on by default) |

On Windows, WinUSB binds **only to the Vendor interface** via MS OS 2.0 subset descriptors (advertised through BOS); HID and CDC use the standard class drivers. An MS OS 1.0 (0xEE) fallback is also served — Win7 auto-bind is best-effort and may need a one-time Zadig / INF install for the Vendor interface.

## Porting

These are hard-coded for the WCH-LinkE reference board and need adjusting for other CH32V305FBP6 designs:

- **SWD / JTAG pins**, **status LED** — `src/DAP_config.h`
- **Board init / pin remap** — `src/main.c` (e.g. PB13→PA14 SWCLK routing, PC6→SWDIO via 1k, optional power switching on PA5/PB12 via `DAP_PWR_OUT`)
