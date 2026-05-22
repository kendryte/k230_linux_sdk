#!/usr/bin/env python3
import fcntl
import sys

I2C_SLAVE = 0x0703

def i2c_scan(bus_num=0):
    with open(f"/dev/i2c-{bus_num}", "r+b", buffering=0) as f:
        print(f"Scanning I2C bus {bus_num}:")
        print("Address  Device")
        print("-------  ------")
        for addr in range(0x00, 0x100):
            try:
                fcntl.ioctl(f.fileno(), I2C_SLAVE, addr)
                f.read(1)  # 尝试读取一个字节
                print(f"  0x{addr:02X}   Present")
            except:
                pass

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] in ["-h", "--help"]:
        print("Usage: python i2c.py [bus_num]")
        print("  bus_num  I2C bus number (default: 0)")
        print("  -h, --help  Show this help message")
        sys.exit(0)

    bus = 0
    if len(sys.argv) > 1:
        bus = int(sys.argv[1])
    i2c_scan(bus)
