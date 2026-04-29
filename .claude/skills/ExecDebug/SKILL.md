---
name: ExecDebug
description: Debug this CH32V305 CMSIS-DAP firmware on the WCH-LinkE board via a second WCH-LinkE (or any wlink-capable probe) with the bundled WCH OpenOCD. Use when the user asks to start a GDB session, watch RTT/SDI output, set breakpoints, or investigate a HardFault dump.
---

# ExecDebug

This firmware runs on the **probe** (the WCH-LinkE board). To debug it you need a **second** debug probe on the host side — a separate WCH-LinkE in its default RV-mode firmware is the canonical setup. The bundled WCH OpenOCD fork at `wch_tools/openocd/bin/openocd` understands `wlinke` as an adapter and exposes WCH-specific Tcl commands (`wlink_sdi`, `sdi_printf`) that upstream OpenOCD does not have.

## Hardware

| Role                | Hardware                                    | Firmware                       |
|---------------------|---------------------------------------------|--------------------------------|
| Target (debugged)   | WCH-LinkE board #1                          | this project's `.bin/.hex`     |
| Probe (debugger)    | WCH-LinkE board #2, or any wlink-compatible | factory WCH-Link RV firmware   |

Wire the probe's SWD/SDI signals to the target's PA13 (SWDIO) / PA14 (SWCLK) — the same pins this firmware drives outward when it acts as a probe itself. With the toolchain SDI bridge, only those two lines plus GND are needed; no extra UART wiring.

## Build

Default Release build is enough for normal probing:

```bash
cmake --build .build
```

For fault-handler dumps you want Debug + a debug log sink:

```bash
cmake -B .build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build .build
```

`Debug` defaults to `DEBUG_OUTPUT=DEBUG_OUTPUT_RTT` (see [`cmake/compile_options.cmake`](cmake/compile_options.cmake)), which routes `debug_printf` (and the HardFault dump in [`hal/Startup/ch32v30x_it.c`](hal/Startup/ch32v30x_it.c)) into the SEGGER RTT control block in SRAM. RTT is **non-blocking**: if the host isn't draining the ring buffer the target drops the tail rather than spinning, so the firmware boots even with no debugger attached. The vendored RTT source is at [`modules/tinyusb/lib/SEGGER_RTT/RTT/SEGGER_RTT.c`](modules/tinyusb/lib/SEGGER_RTT/RTT/SEGGER_RTT.c) — the Conf header already has the RV32 `csrr/csrci mstatus` lock primitives.

The other sinks remain available in [`cmake/compile_options.cmake`](cmake/compile_options.cmake) — `DEBUG_OUTPUT_SDI` (DATA0/DATA1 polling, **blocking** on no consumer) and `DEBUG_OUTPUT_UART` (USART3, mutually exclusive with `DAP_CDC`).

## Flash

Two paths, pick whichever you have:

### A) `wchisp` (USB ISP, no probe needed)

Put the WCH-LinkE into ISP via the BOOT button on power-up, then:

```bash
wchisp flash .build/BZH_DAPLink_WchLinkE-*.elf
```

### B) WCH OpenOCD via the second probe

```bash
wch_tools/openocd/bin/openocd \
  -f wch_tools/openocd/bin/wch-riscv.cfg \
  -c "program .build/BZH_DAPLink_WchLinkE-<DATE>-<SHA>.hex verify reset exit"
```

`wch-riscv.cfg` already selects `adapter driver wlinke` + `transport select sdi` and configures the `wch_riscv` target — don't override.

## Start a debug session

Launch the GDB server in the background, leave it running:

```bash
nohup wch_tools/openocd/bin/openocd \
  -f wch_tools/openocd/bin/wch-riscv.cfg \
  -c "gdb_port 50000" \
  -c "init" \
  > /tmp/wch_ocd.log 2>&1 &
```

Then connect from gdb (use the toolchain's gdb — not `gdb-multiarch`):

```bash
wch_tools/toolchain/bin/riscv-wch-elf-gdb .build/BZH_DAPLink_WchLinkE-<DATE>-<SHA>.elf
```

```gdb
target remote localhost:50000
monitor reset halt
load                                      # only if not already flashed
monitor reset halt
```

Then set breakpoints, `continue`, etc. as normal. `monitor reset halt` is the WCH equivalent of `monitor reset 2` — it asserts nRST and re-halts at the reset vector.

## Debug log live view

The default `DEBUG_OUTPUT_RTT` produces a SEGGER-compatible control block at exactly `0x20000000` (pinned via `.rtt_cb` in [`hal/ld/link.ld`](hal/ld/link.ld) + `-DSEGGER_RTT_SECTION=".rtt_cb"` in [`cmake/compile_options.cmake`](cmake/compile_options.cmake) — verify with `nm | grep _SEGGER_RTT`).

### IMPORTANT: WCH OpenOCD has a stripped RTT command set

`wch_tools/openocd/bin/openocd` exposes only `rtt server start/stop` — there is **no** `rtt setup` and **no** `rtt start`. Run `monitor help rtt` and you'll see the full menu is the four lines under `rtt server`. That means upstream RTT clients (Cortex-Debug's `rttConfig`, plain `nc localhost 9090` after the standard setup-then-start dance) **cannot configure where the control block lives**, because the configure step doesn't exist. The C-level RTT plumbing inside this OpenOCD fork *does* work, it just isn't exposed via Tcl.

So we sidestep OpenOCD's RTT subsystem and poll the control block ourselves over its tcl port.

### A) `utils/rtt_monitor.py` — recommended

Polls `_SEGGER_RTT` at the pinned address via `mdw`/`mdb`/`mww` over the OpenOCD tcl port. Works with the bundled WCH fork **and** upstream OpenOCD; works alongside an active gdb / Cortex-Debug session because tcl is a separate connection.

```bash
# When running OpenOCD from Cortex-Debug, the launch.json command line shows
# "-c tcl_port 50001". Match that:
${PROJECT_ROOT}/.venv/bin/python3 utils/rtt_monitor.py --port 50001 | tee /tmp/rtt.log

# When running raw OpenOCD (Option below), tcl_port defaults to 6666:
python3 utils/rtt_monitor.py | tee /tmp/rtt.log
```

The script verifies the magic `"SEGGER RTT"` at `0x20000000` before entering the poll loop — if it sees zeros there, it tells you `SEGGER_RTT_Init()` never ran (or `--addr` is wrong) and exits, which is a much faster diagnostic than silent no-output.

### B) Standalone OpenOCD + raw gdb

If Cortex-Debug's halt-flooding is getting in the way (it polls aggressively and dies if the target HFs):

```bash
nohup wch_tools/openocd/bin/openocd \
  -f wch_tools/openocd/bin/wch-riscv.cfg \
  -c "gdb_port 50000" -c "tcl_port 6666" -c "init" \
  > /tmp/wch_ocd.log 2>&1 &

wch_tools/toolchain/bin/riscv-wch-elf-gdb .build_debug/BZH_DAPLink_WchLinkE-*.elf
(gdb) target remote localhost:50000
(gdb) monitor reset halt
(gdb) load
(gdb) monitor reset halt
(gdb) continue
# in a side terminal:
python3 utils/rtt_monitor.py
```

### C) Legacy SDI sink

If you build with `-DDEBUG_OUTPUT=DEBUG_OUTPUT_SDI`, the firmware writes into the WCH SDI DATA0/DATA1 registers. View via either `monitor sdi_printf enable` (gdb console, WCH fork only) or [`utils/sdi_monitor.py`](utils/sdi_monitor.py). SDI **blocks** the firmware when no consumer is attached, RTT does not — that's why RTT is the default.

## Reading the HardFault dump

The new RV32 HardFault handler in [`hal/Startup/ch32v30x_it.c`](hal/Startup/ch32v30x_it.c) prints (via `debug_printf`):

```
===== HARDFAULT =====
  cause   : 0x00000007 (Store/AMO access fault)
  mepc    : 0x00001234
  mtval   : 0xE0000380
  mstatus : 0x...
  mtvec   : 0x...
  sp      : 0x20007e00
  stack   : [0x20007000 .. 0x20008000)
stack window (post-HPE):
  0x20007e00: 0x... 0x... 0x... 0x...
  ...
addr2line -e <fw>.elf -afpi 0x00001234
===== END HARDFAULT =====
```

Pipe the SDI output into a log file, then resolve `mepc` (and any plausible return addresses fished out of the stack window) to source lines:

```bash
wch_tools/toolchain/bin/riscv-wch-elf-addr2line \
  -e .build/BZH_DAPLink_WchLinkE-*.elf -afpi 0x00001234
```

`mcause` decoding follows priv-spec 1.12; the handler labels the common cases inline. `mtval` is the faulting address for load/store faults, the offending instruction word for illegal-instruction faults, otherwise zero.

The post-HPE stack window starts after the WCH hardware-prologue push (52 bytes saving x1, x5-x7, x10-x17, x28-x31) — meaning the words you see are the live caller frame, not the trapped registers themselves. The trapped registers are above `sp` (lower addresses) and were already saved by HPE; if you need them, halt in gdb at the `while(1)` and `info registers`.

## Teardown

```bash
# 1. exit gdb
# 2. kill openocd (it does not auto-exit unless `-c "init; exit"` was used)
pkill -x openocd
```

## Recovering a stuck chip (HardFault loop / wedged USB enumeration)

After repeated `program ... verify reset` cycles the chip can fall into a state where boot reaches `main()` but USB never enumerates and the firmware enters a HardFault loop. The OpenOCD-driven reset path doesn't always fully cycle the bus on this board. **WCH-LinkE wires nRST to the target** so a hardware-pin pulse via SRST clears it cleanly:

```bash
# via OpenOCD's tcl_port (50001 when started by Cortex-Debug, 6666 for raw)
echo "halt; adapter assert_srst; sleep 50; adapter deassert_srst" | nc -q1 localhost 50001
```

or from the gdb console: `monitor adapter assert_srst; monitor adapter deassert_srst`. Hold time of ~50 ms is plenty. After deassert, `targets` should report **running** and `lsusb -d 1a86:8011` should show the device within 1–2 s.

## Troubleshooting

- **`debug_printf` hangs and the firmware appears frozen** — Debug build with no SDI consumer attached. Either run a probe + `sdi_printf enable`, or rebuild Release.
- **HardFault repeats forever / no dump appears** — the fault handler itself is faulting. Most likely cause: `debug_printf` → `vsnprintf` overruns the 4 KB stack when the trap arrives during a deep IRQ chain (USB IRQ → TinyUSB → vendor control). Increase `__stack_size` in [`hal/ld/link.ld`](hal/ld/link.ld) or shrink the 256-byte buffer in [`src/debug.c`](src/debug.c).
- **`Error: invalid command name "sdi_printf"`** — you're talking to upstream OpenOCD, not the bundled WCH fork. Use `wch_tools/openocd/bin/openocd`.
- **`Error: unable to find a matching CMSIS-DAP device`** — that error comes from running this firmware *as* a probe with upstream OpenOCD; it's unrelated to the SDI debug path. Use a separate physical WCH-LinkE in factory mode.
- **OpenOCD errors with `wlink_set_address option value`** — the bundled `wch-riscv.cfg` uses `wlink_set_address 0x00000000`. Don't change that for CH32V305.
