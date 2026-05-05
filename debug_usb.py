#!/usr/bin/env python3
"""
USB Debug Script - Opens serial port, triggers MCU reset, and captures output
Cross-platform support for Linux and Windows.

Usage:
    python debug_usb.py [options]

Options:
    -p, --port PORT       Serial port (default: auto-detect or /dev/ttyACM0 on Linux, COM6 on Windows)
    -b, --baud BAUD       Baud rate (default: 115200)
    -s, --serial SERIAL   WCH-Link serial number for OpenOCD adapter selection
    -t, --timeout SECS    Capture timeout in seconds (default: 5)
    -r, --no-reset        Skip MCU reset, just monitor serial
    -f, --flash FILE      Flash firmware file before monitoring
    -h, --help            Show this help message
"""

import serial
import subprocess
import threading
import time
import sys
import os
import argparse
import platform

def get_default_serial_port():
    """Get default serial port based on platform"""
    if platform.system() == "Windows":
        return "COM6"
    else:
        # Linux: try to find ttyACM ports
        for port in ["/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyUSB0"]:
            if os.path.exists(port):
                return port
        return "/dev/ttyACM0"

def get_openocd_paths():
    """Get OpenOCD paths based on platform"""
    if platform.system() == "Windows":
        return (
            "wch_tools/openocd/bin/openocd.exe",
            "wch_tools/openocd/bin/wch-riscv.cfg"
        )
    else:
        return (
            "wch_tools/openocd/bin/openocd",
            "wch_tools/openocd/bin/wch-riscv.cfg"
        )

def read_serial(ser, stop_event, output_lines):
    """Read from serial port until stop event is set"""
    while not stop_event.is_set():
        try:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting).decode('utf-8', errors='replace')
                print(data, end='', flush=True)
                output_lines.append(data)
            else:
                time.sleep(0.01)
        except Exception as e:
            print(f"\nSerial read error: {e}")
            break

def trigger_reset(openocd_path, openocd_cfg, adapter_serial=None):
    """Trigger MCU reset via OpenOCD"""
    print("\n--- Triggering MCU reset via OpenOCD ---\n")
    try:
        cmd = [openocd_path, "-f", openocd_cfg]
        if adapter_serial:
            cmd.extend(["-c", f"adapter serial {adapter_serial}"])
        cmd.extend(["-c", "init; reset; exit"])
        
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=10
        )
        if result.returncode != 0:
            print(f"OpenOCD error: {result.stderr}")
        else:
            print("--- MCU reset complete ---\n")
    except subprocess.TimeoutExpired:
        print("OpenOCD timeout!")
    except Exception as e:
        print(f"OpenOCD error: {e}")

def flash_firmware(openocd_path, openocd_cfg, firmware_path, adapter_serial=None):
    """Flash firmware via OpenOCD"""
    print(f"\n--- Flashing {firmware_path} via OpenOCD ---\n")
    try:
        cmd = [openocd_path, "-f", openocd_cfg]
        if adapter_serial:
            cmd.extend(["-c", f"adapter serial {adapter_serial}"])
        cmd.extend(["-c", f"program {firmware_path} verify reset exit"])
        
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30
        )
        if result.returncode != 0:
            print(f"OpenOCD error: {result.stderr}")
            return False
        else:
            print("--- Flash complete ---\n")
            return True
    except subprocess.TimeoutExpired:
        print("OpenOCD timeout!")
        return False
    except Exception as e:
        print(f"OpenOCD error: {e}")
        return False

def analyze_output(full_output):
    """Analyze captured output for USB enumeration patterns"""
    print("\n=== ANALYSIS ===")
    
    # Check for BOS descriptor request
    if "BOS!" in full_output or "BOS requested" in full_output:
        print("✓ BOS descriptor was requested")
    else:
        print("✗ BOS descriptor was NOT requested")
    
    # Check for MS OS 2.0 descriptor request
    if "MS OS 2.0" in full_output:
        print("✓ MS OS 2.0 descriptor was requested")
    else:
        print("✗ MS OS 2.0 descriptor was NOT requested")
    
    # Check for MS OS 1.0 string request
    if "MS OS 1.0 string" in full_output:
        print("✓ MS OS 1.0 string was requested")
    else:
        print("✗ MS OS 1.0 string was NOT requested (may be disabled)")
    
    # Check for vendor control requests
    if "VendorCtrl" in full_output:
        print("✓ Vendor control request received")
        # Try to extract details
        for line in full_output.split('\n'):
            if "VendorCtrl" in line:
                print(f"    {line.strip()}")
    else:
        print("✗ Vendor control request NOT received")
    
    # Check for device configuration
    if "Set Config" in full_output or "configured" in full_output.lower():
        print("✓ Device was configured")
    else:
        print("? Device configuration status unknown")
    
    # Check for suspend
    if "Suspend" in full_output:
        print("! Device suspended (enumeration may have failed)")

def main():
    parser = argparse.ArgumentParser(
        description="USB Debug Script - Monitor serial output during USB enumeration",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    # Monitor with auto-detected serial port
    python debug_usb.py
    
    # Specify serial port and adapter
    python debug_usb.py -p /dev/ttyACM0 -s D9A48F065E98
    
    # Flash firmware then monitor
    python debug_usb.py -f .build/BZH_DAPLink_WchLinkE-<date>-<sha>.elf -s D9A48F065E98
    
    # Just monitor without reset
    python debug_usb.py -p /dev/ttyACM0 -r
"""
    )
    parser.add_argument("-p", "--port", help="Serial port", default=None)
    parser.add_argument("-b", "--baud", type=int, help="Baud rate", default=115200)
    parser.add_argument("-s", "--serial", help="WCH-Link adapter serial number")
    parser.add_argument("-t", "--timeout", type=int, help="Capture timeout in seconds", default=5)
    parser.add_argument("-r", "--no-reset", action="store_true", help="Skip MCU reset")
    parser.add_argument("-f", "--flash", help="Flash firmware file before monitoring")
    
    args = parser.parse_args()
    
    # Get platform-specific defaults
    serial_port = args.port or get_default_serial_port()
    openocd_path, openocd_cfg = get_openocd_paths()
    
    print(f"Platform: {platform.system()}")
    print(f"Serial port: {serial_port}")
    print(f"Baud rate: {args.baud}")
    if args.serial:
        print(f"Adapter serial: {args.serial}")
    print()
    
    # Open serial port
    print(f"Opening serial port {serial_port}...")
    try:
        ser = serial.Serial(serial_port, args.baud, timeout=0.1)
        print(f"Serial port opened successfully.\n")
    except Exception as e:
        print(f"Failed to open serial port: {e}")
        sys.exit(1)
    
    # Start serial reader thread
    stop_event = threading.Event()
    output_lines = []
    reader_thread = threading.Thread(target=read_serial, args=(ser, stop_event, output_lines))
    reader_thread.start()
    
    # Wait a bit for serial to settle
    time.sleep(0.5)
    
    # Flash if requested
    if args.flash:
        if not flash_firmware(openocd_path, openocd_cfg, args.flash, args.serial):
            print("Flash failed, continuing anyway...")
    elif not args.no_reset:
        # Trigger reset
        trigger_reset(openocd_path, openocd_cfg, args.serial)
    
    # Wait for enumeration to complete
    print(f"Capturing USB enumeration output for {args.timeout} seconds...\n")
    print("=" * 60)
    time.sleep(args.timeout)
    print("=" * 60)
    
    # Stop reader
    stop_event.set()
    reader_thread.join(timeout=1)
    
    # Close serial port
    ser.close()
    print("\n--- Serial port closed ---")
    
    # Analyze output
    full_output = ''.join(output_lines)
    analyze_output(full_output)

if __name__ == "__main__":
    main()
