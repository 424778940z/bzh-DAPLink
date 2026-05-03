#!/bin/bash
# Flash the most recent ELF onto the WeAct STM32F411CE via SEGGER J-Link.
#
#   Default ELF: latest .build/*/BZH_DAPLink_STM32F411-*.elf
#   Override:    ./utils/flash_jlink.sh path/to/firmware.elf
#   Probe SN:    JLINK_SN=12345678 ./utils/flash_jlink.sh   (default: 174310284)
#
# JLinkExe is part of the SEGGER J-Link Software & Documentation Pack
# (apt: jlink, or https://www.segger.com/downloads/jlink/).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOP_DIR="$SCRIPT_DIR/.."

if [[ $# -ge 1 ]]; then
    ELF="$1"
else
    ELF="$(ls -t "$TOP_DIR"/.build/*/BZH_DAPLink_STM32F411-*.elf 2>/dev/null | head -1)"
fi
[[ -f "$ELF" ]] || { echo "Firmware ELF not found: ${ELF:-<none>}" >&2; exit 1; }

JLINK_SN="${JLINK_SN:-174310284}"

CMD="$(mktemp)"
trap "rm -f '$CMD'" EXIT

cat > "$CMD" <<EOF
selectemubysn $JLINK_SN
device STM32F411CE
si SWD
speed 4000
loadfile $ELF
r
g
exit
EOF
# Note: the running firmware repurposes PA13/PA14 as DAP probe outputs, which
# makes the F411's own SWD interface unreachable via SWD-only attach. Press the
# WeAct NRST button when you see "Connecting to target" so the chip is held in
# bootrom long enough for J-Link to claim SWD before our firmware re-grabs it.

echo "Flashing $ELF via J-Link SN=$JLINK_SN ..."
JLinkExe -nogui 1 -ExitOnError 1 -CommanderScript "$CMD"
