# BZH_DAPLink_STM32F411

CMSIS-DAP debug probe firmware for the **WeAct STM32F411CE Black Pill** board (STM32F411CEU6, Cortex-M4F @ 96 MHz; see *Porting* for other STM32F411 designs).

## Repository layout

| Path                              | Contents                                                                              |
|-----------------------------------|---------------------------------------------------------------------------------------|
| `src/`                            | Project firmware sources                                                              |
| `src/usb/tinyusb_port/`           | TinyUSB device callbacks, descriptors, vendor/HID/CDC class glue                      |
| `src/DAP_config.h`                | CMSIS-DAP target config: pinout, clock, LED                                           |
| `src/dap_config_checker.c`        | Compile-time enforcement of the F411 OTG_FS endpoint budget (max 2 of 3 functions)    |
| `hal/Startup/`                    | F411 startup, system init, vector table, syscalls/sysmem                              |
| `hal/Peripheral/`                 | STM32 HAL peripheral wrapper build glue                                               |
| `hal/ld/link.ld`                  | Linker script (RAM places SEGGER RTT control block at 0x20000000)                     |
| `modules/cmsis_dap/`              | ARM CMSIS-DAP submodule                                                               |
| `modules/tinyusb/`                | TinyUSB submodule (also provides SEGGER RTT bundled under `lib/SEGGER_RTT/`)          |
| `modules/STM32CubeF4/`            | STMicroelectronics STM32CubeF4 (HAL + CMSIS device headers)                           |
| `cmake/`                          | CMake project glue, compile flags, two toolchain selectors                            |
| `arm_toolchain/`                  | Derived (gitignored). Populated by cmake when an `arm-toolchain-*` preset is used.    |
| `utils/`                          | Flash / RTT / reset helpers (J-Link, FT232 RTS reset), code formatters, venv setup    |

There is no companion LFS toolchain branch — the Arm GNU Toolchain is fetched directly from `developer.arm.com` by `cmake/toolchain_arm.cmake` on first configure (when the `arm-toolchain-*` preset is selected). `system-toolchain-*` instead uses whatever `arm-none-eabi-gcc` is on your `$PATH`.

## Building

Requires `cmake >= 3.28`, `ninja`, `git`. The `arm-toolchain-*` presets additionally need network access on first configure (the Arm GNU Toolchain 14.2.rel1 is downloaded into `arm_toolchain/`).

Four CMake presets are exposed in [`CMakePresets.json`](CMakePresets.json):

| Preset                       | Toolchain source                                                | Build type |
|------------------------------|-----------------------------------------------------------------|------------|
| `system-toolchain-debug`     | Host-installed `arm-none-eabi-gcc` (your `$PATH`)               | Debug      |
| `system-toolchain-release`   | Host-installed `arm-none-eabi-gcc`                              | Release    |
| `arm-toolchain-debug`        | Auto-fetched Arm GNU Toolchain 14.2.rel1 → `arm_toolchain/`     | Debug      |
| `arm-toolchain-release`      | Auto-fetched Arm GNU Toolchain 14.2.rel1                        | Release    |

```sh
cmake --preset arm-toolchain-release
cmake --build --preset arm-toolchain-release
```

[`build.sh`](build.sh) is a thin wrapper that selects the `arm-toolchain-*` preset for the requested mode (used by the top-level CI workflow on `main`):

```sh
bash ./build.sh release   # -> arm-toolchain-release
bash ./build.sh debug     # -> arm-toolchain-debug
```

Outputs land in `.build/<preset>/` as `BZH_DAPLink_STM32F411-<YYYYMMDD>-<short-sha>.{elf,hex,bin,lst,map}`.

## Build options

Set in [`cmake/compile_options.cmake`](cmake/compile_options.cmake) under `PROJ_WIDE_DEFINES`:

| Macro       | Effect when defined                                  |
|-------------|------------------------------------------------------|
| `DAP_FW_V1` | CMSIS-DAP v1 (HID) interface                         |
| `DAP_FW_V2` | CMSIS-DAP v2 (Vendor / WinUSB) interface             |
| `DAP_CDC`   | USB CDC ACM bridged to USART1                        |

**At most TWO** of `{DAP_FW_V1, DAP_FW_V2, DAP_CDC}` may be enabled at once: F411's OTG_FS hardware exposes only 4 IN endpoints (incl. EP0) and each function claims one IN slot. `src/dap_config_checker.c` enforces this with `#error`. Default ships `V1 + CDC`. Other valid combos: `{V1+V2}`, `{V2+CDC}`.

`DEBUG_OUTPUT` (debug-log sink) is wired to the build configuration via a generator expression: `Debug` → `DEBUG_OUTPUT_RTT`, `Release` → `DEBUG_OUTPUT_NONE`. `DEBUG_OUTPUT_UART` is left as a commented-out swap in the same block — it routes through USART1 and is mutually exclusive with `DAP_CDC` (also enforced at compile time by `src/dap_config_checker.c`).

## Flashing

The running firmware repurposes PA13/PA14 (the F411's default SWD pins) as DAP outputs, so SWD attach to the F411 itself requires the WeAct's **NRST button held during connect**. Easiest flow is an external probe (e.g. J-Link, ST-Link) on a shared SWD breakout switched to F411 mode:

```sh
bash utils/flash_jlink.sh        # J-Link via segger-jlink
bash utils/flash_cmsisdap.sh     # OpenOCD via another CMSIS-DAP probe
```

`utils/reset_f411.py` (50 ms RTS pulse) and `utils/reset_hold.py` (asserts RTS until SIGTERM) drive an FT232RL adapter (default `/dev/ttyUSB0`) whose RTS line is wired to the F411's NRST — useful for scripting a reset between OpenOCD invocations or holding the chip in bootrom while another probe claims SWD.

Verify enumeration after flashing:

- Linux: `lsusb -d c251:f001 -v`
- Windows: USB Tree View, or `Get-PnpDevice -Class USB`

## USB identity

| Field      | Value                                                |
|------------|------------------------------------------------------|
| VID:PID    | `0xC251:0xF001` (Keil community CMSIS-DAP)           |
| Class      | Composite (IAD)                                      |
| iProduct   | `BZH STM32F411 CMSIS-DAP`                            |
| iSerial    | 8 hex chars derived from STM32 96-bit UID (hashed)   |
| DAP v1     | HID interface, 64-byte report                        |
| DAP v2     | Vendor (WinUSB) bulk endpoints                       |
| CDC        | USART1 bridge (gated by `DAP_CDC`)                   |

On Windows, WinUSB binds **only to the Vendor interface** via MS OS 2.0 subset descriptors (advertised through BOS); HID and CDC use the standard class drivers. An MS OS 1.0 (0xEE) fallback is also served — Win7 auto-bind is best-effort and may need a one-time Zadig / INF install for the Vendor interface.

## Porting

These are hard-coded for the WeAct V3.x board and need adjusting for other STM32F411 designs:

- **SWD / nRESET pins, status LED** — `src/DAP_config.h` (default: SWCLK=PA14, SWDIO=PA13, nRESET=PA8, LED=PC13)
- **HSE crystal value** — `cmake/compile_options.cmake` `HSE_VALUE` (WeAct V3.x = 25 MHz Y2 crystal)
- **Clock tree** — `src/main.c` `SystemClock_Config`. The PLL must give exact 48 MHz on PLLQ for OTG_FS: `PLLM=25, PLLN=192, PLLP=2, PLLQ=4` → SYSCLK=96 MHz. **Do not "restore" 100 MHz SYSCLK** — `PLLN=200` gives 50 MHz USB which sneaks past short FS exchanges then fails on long bulk responses with `urb status=-84` / EILSEQ.
- **Board init / pin remap** — `src/main.c` (the firmware repurposes PA13/PA14 as DAP I/O pins)
