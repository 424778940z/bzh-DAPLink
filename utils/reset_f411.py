#!/usr/bin/env python3
# Pulse RTS on /dev/ttyUSB0 (FT232RL) to reset the F411's NRST line.
#
# Wiring: FT232RL RTS -> F411 NRST (direct, no inverter). Pyserial s.rts=True
# asserts RTS, which on FTDI drives the pin low -> NRST low -> F411 reset.
# Setting s.rts=False releases the line high -> F411 runs.
#
# Usage:  python3 utils/reset_f411.py [tty]   (default /dev/ttyUSB0)

import sys
import time
import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0"

s = serial.Serial(PORT, 9600, rtscts=False, dsrdtr=False)
# Hold reset for 50 ms, then release.
s.rts = True
time.sleep(0.05)
s.rts = False
s.close()
