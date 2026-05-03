#!/usr/bin/env python3
"""Poll the CH32V SDI DATA0/DATA1 buffer over OpenOCD's tcl port and stream
chunks to stdout — works with any OpenOCD (upstream or WCH fork) without
relying on the WCH-only `sdi_printf` command.

DATA0 is the producer-busy flag: low byte holds the chunk length (1..7),
upper three bytes hold payload[0..2]. DATA1 holds payload[3..6]. The host
clears DATA0 after consuming each chunk so the chip can write the next
one.
"""

from __future__ import annotations

import argparse
import socket
import sys
import time

DATA0_DEFAULT = 0xE0000380
DATA1_DEFAULT = 0xE0000384


class TclClient:
    """Minimal client for OpenOCD's tcl_port (line-oriented, 0x1A terminator)."""

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


def read_word(client: TclClient, addr: int) -> int:
    out = client.cmd(f"mdw 0x{addr:08x}").strip()
    parts = out.split()
    return int(parts[-1], 16)


def write_word(client: TclClient, addr: int, value: int) -> None:
    client.cmd(f"mww 0x{addr:08x} 0x{value:08x}")


def parse_chunk(data0: int, data1: int) -> bytes:
    length = data0 & 0xFF
    if length == 0 or length > 7:
        return b""
    raw = bytearray(7)
    raw[0] = (data0 >> 8) & 0xFF
    raw[1] = (data0 >> 16) & 0xFF
    raw[2] = (data0 >> 24) & 0xFF
    raw[3] = data1 & 0xFF
    raw[4] = (data1 >> 8) & 0xFF
    raw[5] = (data1 >> 16) & 0xFF
    raw[6] = (data1 >> 24) & 0xFF
    return bytes(raw[:length])


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--host", default="localhost")
    p.add_argument("--port", type=int, default=6666, help="OpenOCD tcl_port (default 6666)")
    p.add_argument("--data0", type=lambda x: int(x, 0), default=DATA0_DEFAULT)
    p.add_argument("--data1", type=lambda x: int(x, 0), default=DATA1_DEFAULT)
    p.add_argument("--interval", type=float, default=0.005, help="poll interval in seconds (default 5 ms)")
    p.add_argument("--timeout", type=float, default=2.0, help="socket timeout in seconds")
    args = p.parse_args()

    try:
        client = TclClient(args.host, args.port, args.timeout)
    except OSError as e:
        sys.stderr.write(f"sdi_monitor: cannot connect to openocd tcl port {args.host}:{args.port}: {e}\n")
        return 1

    sys.stderr.write(f"sdi_monitor: polling DATA0=0x{args.data0:08x} DATA1=0x{args.data1:08x} every {args.interval*1000:.1f} ms\n")
    sys.stderr.flush()

    try:
        while True:
            d0 = read_word(client, args.data0)
            if (d0 & 0xFF) == 0:
                time.sleep(args.interval)
                continue
            d1 = read_word(client, args.data1)
            payload = parse_chunk(d0, d1)
            write_word(client, args.data0, 0)
            if payload:
                sys.stdout.buffer.write(payload)
                sys.stdout.flush()
    except KeyboardInterrupt:
        return 0
    finally:
        client.close()


if __name__ == "__main__":
    sys.exit(main())
