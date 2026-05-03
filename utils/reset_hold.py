#!/usr/bin/env python3
# Assert RTS on /dev/ttyUSB0 (FT232RL) -> NRST low -> hold the F411 in reset.
# Stays running until killed (SIGTERM/SIGINT/Ctrl-C); on exit, de-asserts RTS
# so the F411 boots the freshly-flashed firmware.
#
# Usage:  python3 utils/reset_hold.py [tty]   (default /dev/ttyUSB0)

import signal
import sys

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0"

s = serial.Serial(PORT, 9600, rtscts=False, dsrdtr=False)
s.rts = True
print(f"Holding NRST low via RTS on {PORT}. Send SIGTERM/SIGINT to release.", flush=True)


def release(*_):
    s.rts = False
    s.close()
    sys.exit(0)


signal.signal(signal.SIGTERM, release)
signal.signal(signal.SIGINT, release)
signal.pause()
