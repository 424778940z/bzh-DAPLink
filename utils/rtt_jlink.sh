#!/bin/bash
# Stream SEGGER RTT channel 0 from the WeAct STM32F411CE via J-Link.
#
# Spins up JLinkGDBServer in the background (it provides the RTT bridge even
# without GDB attached), then attaches JLinkRTTClient on the canonical telnet
# port (19021). Ctrl-C kills both. The firmware must have called
# SEGGER_RTT_Init() and the linker must place the control block at .rtt_cb
# (hal/ld/link.ld) for the client to find it instantly.
#
#   Probe SN: JLINK_SN=12345678 ./utils/rtt_jlink.sh   (default: 174310284)

set -euo pipefail

JLINK_SN="${JLINK_SN:-174310284}"
RTT_PORT="${RTT_PORT:-19021}"
GDB_PORT="${GDB_PORT:-2331}"

echo "Starting J-Link GDB server (SN=$JLINK_SN, RTT port=$RTT_PORT)..."
JLinkGDBServer \
    -if SWD \
    -device STM32F411CE \
    -speed 4000 \
    -select usb=$JLINK_SN \
    -port $GDB_PORT \
    -RTTTelnetPort $RTT_PORT \
    -nogui \
    -silent \
    >/dev/null 2>&1 &
GDBSERVER_PID=$!
trap "kill $GDBSERVER_PID 2>/dev/null; wait $GDBSERVER_PID 2>/dev/null; exit 0" INT TERM EXIT

# Wait briefly for the server to bind
sleep 1.5

echo "Attaching JLinkRTTClient (Ctrl-C to exit)..."
JLinkRTTClient -RTTTelnetPort $RTT_PORT
