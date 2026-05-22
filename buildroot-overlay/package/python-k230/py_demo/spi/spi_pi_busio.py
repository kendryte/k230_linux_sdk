import time
import board
import busio

# 1. Initialize hardware SPI bus (auto CS is configured at bottom level,
#    or using default hardware CS pin)
_,sck,mosi,miso = board.pin.spiPorts[0]
spi = busio.SPI(sck,mosi,miso)

print("--- Reading SPI NOR Flash ID (hardware auto CS control) ---")

# 2. Acquire SPI bus lock
spi.try_lock()

# 3. Configure SPI settings (Mode 0, 1MHz clock)
spi.configure(baudrate=1000000, polarity=0, phase=0)

# 4. Prepare buffers
# 4 bytes total: 1 command byte + 3 bytes for receiving ID
tx_buf = bytes([0x9F, 0x00, 0x00, 0x00])
rx_buf = bytearray(4) # length must match tx_buf

# 5. Simultaneous full-duplex read/write (within single CS active cycle)
# Hardware automatically pulls CS low -> sends 4 bytes while receiving 4 bytes -> CS high
spi.write_readinto(tx_buf, rx_buf)

# 6. Release bus lock
spi.unlock()

# 7. Parse and print result
# Byte 0 in rx_buf is dummy data (sent 0x9F), JEDEC ID follows in bytes 1-3
manufacturer_id = rx_buf[1]
memory_type     = rx_buf[2]
capacity_id     = rx_buf[3]

print("-" * 40)
print(f"Raw response data (RAW): {[hex(x) for x in rx_buf]}")
print(f"Manufacturer ID: 0x{manufacturer_id:02X}")
print(f"Memory Type:     0x{memory_type:02X}")
print(f"Capacity ID:     0x{capacity_id:02X}")
print("-" * 40)
