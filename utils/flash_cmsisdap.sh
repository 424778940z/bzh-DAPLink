#!/bin/bash
# Flash the most recent ELF onto the WeAct STM32F411CE via OpenOCD + a
# CMSIS-DAP probe (default: the WCH-LinkE running this project's prior firmware).
#
#   Default ELF: latest .build/*/BZH_DAPLink_STM32F411-*.elf
#   Override:    ./utils/flash_cmsisdap.sh path/to/firmware.elf
#   Probe SN:    PROBE_SN=01A615FD ./utils/flash_cmsisdap.sh   (default: 01A615FD)
#
# OpenOCD's `connect_assert_srst` pulls NRST low during initial attach so the
# probe wins SWD even if the previously-running firmware has PA13/PA14 driven
# as outputs.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOP_DIR="$SCRIPT_DIR/.."

if [[ $# -ge 1 ]]; then
    ELF="$1"
else
    ELF="$(ls -t "$TOP_DIR"/.build/*/BZH_DAPLink_STM32F411-*.elf 2>/dev/null | head -1)"
fi
[[ -f "$ELF" ]] || { echo "Firmware ELF not found: ${ELF:-<none>}" >&2; exit 1; }

PROBE_SN="${PROBE_SN:-01A615FD}"

echo "Flashing $ELF via CMSIS-DAP SN=$PROBE_SN ..."
openocd \
    -f interface/cmsis-dap.cfg \
    -c "adapter serial $PROBE_SN" \
    -c "adapter speed 1000" \
    -c "transport select swd" \
    -c "reset_config srst_only srst_nogate connect_assert_srst" \
    -f target/stm32f4x.cfg \
    -c "program $ELF verify reset exit"
