
import time
import sys
import board
import busio



def main(bus_num=0):
    """Scan I2C devices on specified bus."""
    # SCL and SDA pins for different I2C buses

    print(f"Hello Blinka! Using I2C bus {bus_num}")

    _, scl,sda  = board.pin.i2cPorts[bus_num]
    i2c = busio.I2C(scl, sda)

    while not i2c.try_lock():
        pass

    try:
        devices = i2c.scan()
        print(f"I2C devices found on bus {bus_num}: {[hex(i) for i in devices]}")
    finally:
        i2c.unlock()


if __name__ == "__main__":
    bus = 0
    if len(sys.argv) > 1:
        if sys.argv[1] in ["-h", "--help"]:
            print("Usage: python pi_busio_i2c.py [bus_num]")
            print("  bus_num  I2C bus number (default: 0)")
            print("           0 - I2C0 (SCL/SDA)")
            print("           1 - I2C1 (SCL1/SDA1)")
            print("           2 - I2C2 (SCL2/SDA2)")
            sys.exit(0)
        bus = int(sys.argv[1])

    main(bus)
