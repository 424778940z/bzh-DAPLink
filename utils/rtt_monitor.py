#!/usr/bin/env python3
"""Poll the SEGGER RTT control block over OpenOCD's tcl port and stream
the up-channel-0 ring buffer to stdout.

Workaround for the bundled WCH OpenOCD whose `rtt` Tcl command set is
stripped to just `rtt server start/stop` — no `rtt setup` / `rtt start`,
which means upstream-style clients (Cortex-Debug, telnet "rtt server"
flow) cannot configure where the control block is. We do it manually:
read WrOff/RdOff via `mdw`, drain the ring via `mdb`, advance RdOff via
`mwb`. Same protocol J-Link uses, just driven from the host side.

The default control block address (0x20000000) matches the .rtt_cb
section pinned in hal/ld/link.ld. Override with --addr if it moves.
"""

from __future__ import annotations

import argparse
import re
import socket
import sys
import time

_HEX_BYTE_RE = re.compile(r"\b[0-9a-fA-F]{2}\b")

# SEGGER_RTT_BUFFER_UP layout (offsets within aUp[0]):
#   0  sName        (char*)
#   4  pBuffer      (char*)
#   8  SizeOfBuffer (u32)
#  12  WrOff        (u32)
#  16  RdOff        (u32)
#  20  Flags        (u32)
# aUp[0] sits at offset 24 of SEGGER_RTT_CB (16 acID + 4 max-up + 4 max-down).
AUP0_OFFSET = 24


class TclClient:
    TERM = b"\x1a"

    def __init__(self, host: str, port: int, timeout: float):
        self.sock = socket.create_connection((host, port), timeout=timeout)
        self.sock.settimeout(timeout)
        self.buf = bytearray()

    def cmd(self, line: str) -> str:
        self.sock.sendall(line.encode() + self.TERM)
        while self.TERM[0] not in self.buf:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("openocd closed tcl connection")
            self.buf.extend(chunk)
        idx = self.buf.index(self.TERM[0])
        out = self.buf[:idx].decode("utf-8", errors="replace")
        del self.buf[: idx + 1]
        return out

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass


def _last_hex_word(s: str) -> int:
    # `mdw 0xADDR` returns "0xADDR: 0x12345678"; `read_memory` returns "0x12345678".
    parts = s.replace(":", " ").split()
    return int(parts[-1], 16)


def read_word(client: TclClient, addr: int) -> int:
    return _last_hex_word(client.cmd(f"mdw 0x{addr:08x}").strip())


def read_bytes(client: TclClient, addr: int, count: int) -> bytes:
    # `mdb` output format on the WCH fork: "0xADDR: HH HH HH HH ...\n"
    # — bytes are bare two-char hex (no 0x prefix), one or more lines, each
    # prefixed with the line's address. Drop the prefix, then regex-match
    # the bytes (regex avoids accidentally consuming something like a stray
    # decimal byte count in the response).
    out = client.cmd(f"mdb 0x{addr:08x} {count}")
    raw = bytearray()
    for line in out.splitlines():
        rest = line.split(":", 1)[1] if ":" in line else line
        for tok in _HEX_BYTE_RE.findall(rest):
            raw.append(int(tok, 16))
            if len(raw) >= count:
                return bytes(raw)
    return bytes(raw[:count])


def write_word(client: TclClient, addr: int, value: int) -> None:
    client.cmd(f"mww 0x{addr:08x} 0x{value:08x}")


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--host", default="localhost")
    p.add_argument(
        "--port", type=int, default=50001,
        help="OpenOCD tcl_port (Cortex-Debug uses 50001 by default; raw OpenOCD defaults to 6666)",
    )
    p.add_argument(
        "--addr", type=lambda x: int(x, 0), default=0x20000000,
        help="address of _SEGGER_RTT (default 0x20000000, matches .rtt_cb pin in hal/ld/link.ld)",
    )
    p.add_argument("--interval", type=float, default=0.02, help="poll interval in seconds (default 20 ms)")
    p.add_argument("--timeout", type=float, default=2.0)
    args = p.parse_args()

    try:
        client = TclClient(args.host, args.port, args.timeout)
    except OSError as e:
        sys.stderr.write(f"rtt_monitor: connect failed {args.host}:{args.port}: {e}\n")
        return 1

    aup0 = args.addr + AUP0_OFFSET
    p_buffer_addr = aup0 + 4
    size_addr = aup0 + 8
    wroff_addr = aup0 + 12
    rdoff_addr = aup0 + 16

    # Sanity-check the magic ID before entering the poll loop. If the firmware
    # never called SEGGER_RTT_Init() this will be all zeros and there's no
    # point polling — say so and exit.
    magic = read_bytes(client, args.addr, 16)
    if not magic.startswith(b"SEGGER RTT"):
        sys.stderr.write(
            f"rtt_monitor: no SEGGER RTT magic at 0x{args.addr:08x} "
            f"(read {magic!r}). Either the firmware has not yet called "
            f"SEGGER_RTT_Init(), or --addr is wrong.\n"
        )
        return 2

    p_buffer = read_word(client, p_buffer_addr)
    size = read_word(client, size_addr)
    sys.stderr.write(
        f"rtt_monitor: control block at 0x{args.addr:08x}, "
        f"up0 buffer at 0x{p_buffer:08x} ({size} bytes)\n"
    )
    sys.stderr.flush()

    rd = read_word(client, rdoff_addr)
    try:
        while True:
            wr = read_word(client, wroff_addr)
            if wr == rd:
                time.sleep(args.interval)
                continue
            if wr > rd:
                count = wr - rd
                data = read_bytes(client, p_buffer + rd, count)
                rd = wr
            else:
                # wrap: read from rd to end, then 0 to wr
                first = read_bytes(client, p_buffer + rd, size - rd)
                second = read_bytes(client, p_buffer, wr)
                data = first + second
                rd = wr
            write_word(client, rdoff_addr, rd)
            if data:
                sys.stdout.buffer.write(data)
                sys.stdout.flush()
    except KeyboardInterrupt:
        return 0
    finally:
        client.close()


if __name__ == "__main__":
    sys.exit(main())
