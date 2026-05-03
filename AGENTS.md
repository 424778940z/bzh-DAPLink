# Notes for AI agents

For project overview, USB identity, repository layout, build instructions, build options, forking, flashing, and porting — see [README.md](README.md).

This file collects conventions that AI coding agents working in this repo should follow.

## Conventions

- **Verify changes end-to-end:** build, flash, then check USB enumeration. Do not declare a change "done" purely on a successful compile.
- **One shell command at a time.** Do not chain destructive operations with `&&`, `;`, or pipes; the sandbox may reject the combined invocation.
- **Do not redirect to `/dev/null`** unless strictly needed — the sandbox flags it.
- **Use relative paths** for tooling.
- **Python:** set up the project venv via `bash utils/setup_venv.sh` (single entry point, also invoked by CMake's `PROJ_PYENV` target). Never call `python -m venv` directly and never install into the system Python.
- **DAP_config.h is a minimal-modify of the upstream CMSIS-DAP template** at `modules/cmsis_dap/Firmware/Source/DAP_config.h`. Doxygen comments and upstream structure are kept verbatim so a future re-import or re-port stays legible against `git diff`. Only the `#include`s, `#define` values, and function bodies are board-specific. Do not strip or rewrap the template comments.
- **No autonomous `sudo`** — ask before any privileged action (apt install, systemctl, udev edits, file ownership change).

## Port-specific verification (STM32F411 WeAct Black Pill)

- Probes on a shared SWD breakout, switched manually by the user:
  - `J-Link Pro` (SN `174310284`) → flashes / debugs the WeAct's STM32F411
  - WeAct STM32F411 (USB iSerial = device_id hashed UID, e.g. `CC98473F`) → acts as CMSIS-DAP probe to a downstream target (currently an STM32F103C8T6)
- Build + flash flow:
  - `cmake --preset system-toolchain-debug` (uses host `arm-none-eabi-gcc`) or `cmake --preset arm-toolchain-debug` (auto-fetched 14.2.rel1 to `arm_toolchain/`)
  - `cmake --build .build/<preset>`
  - `bash utils/flash_jlink.sh` — needs J-Link → F411 mode on the breakout. The running firmware repurposes PA13/PA14 as DAP outputs, so SWD attach to the F411 itself requires the WeAct NRST button held during connect.
- USB / RTT verification:
  - `lsusb -d c251:f001 -v` → expect VID:PID `0xC251:0xF001`, `iProduct = "BZH STM32F411 CMSIS-DAP"`, `iSerial = <8 hex chars>`.
  - `bash utils/rtt_jlink.sh` → expect `=== DAP STARTUP ===` plus a tick heartbeat; uses J-Link's GDB server RTT bridge on TCP 19021.
- ModemManager probes /dev/ttyACMx with AT commands, which collides with the CDC bridge at runtime AND can race libusb's interface claim mid-OpenOCD attach. Disable with `sudo systemctl disable --now ModemManager` (one-shot acceptable on a development workstation; the project's udev rules already include `ID_MM_DEVICE_IGNORE` for the relevant VID:PIDs).
- F411 OTG_FS only has 4 IN endpoints incl. EP0; enable AT MOST TWO of `{DAP_FW_V1, DAP_FW_V2, DAP_CDC}` in `cmake/compile_options.cmake`. `src/dap_config_checker.c` enforces this with `#error`.
- F411 has a single main PLL feeding both SYSCLK and OTG_FS; with the WeAct's 25 MHz HSE the only PLL config that gives exact 48 MHz on PLLQ has VCO=192 (PLLN=192 → SYSCLK=96 MHz). PLLN=200 gives 50 MHz USB which sneaks past short USB exchanges then fails on long FS bulk responses with `urb status=-84` / EILSEQ. Do not "restore" 100 MHz SYSCLK — see `src/main.c` `SystemClock_Config` and `src/DAP_config.h` `CPU_CLOCK`.
- `utils/reset_f411.py` (50 ms RTS pulse) and `utils/reset_hold.py` (asserts RTS until SIGTERM) drive an FT232RL adapter (default `/dev/ttyUSB0`) whose RTS line is wired to the F411's NRST. Use them to script a reset between OpenOCD invocations or to hold the chip in bootrom while J-Link claims SWD (the running firmware repurposes PA13/PA14 as DAP outputs, so SWD attach to the F411 itself otherwise needs the WeAct NRST button held by hand).
