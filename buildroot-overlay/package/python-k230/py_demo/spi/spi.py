import os
import fcntl
import struct
import ctypes

# --- Linux SPI ioctl command constants ---
# If 0x40016B01 fails, some kernels require 0x40046B01 (32-bit mode write)
SPI_IOC_WR_MODE          = 0x40016B01
SPI_IOC_WR_MAX_SPEED_HZ  = 0x40046B04
SPI_IOC_MESSAGE_1        = 0x40206B00  # Transfer 1 spi_ioc_transfer structure

def read_flash_id(bus=0, device=0):
    spi_device = f"/dev/spidev{bus}.{device}"

    if not os.path.exists(spi_device):
        print(f"Error: Device file {spi_device} not found")
        return

    fd = os.open(spi_device, os.O_RDWR)

    try:
        # 1. Set SPI mode (Mode 0)
        try:
            fcntl.ioctl(fd, SPI_IOC_WR_MODE, struct.pack('B', 0))
        except OSError:
            # Try writing Mode with 4-byte unsigned int format (newer kernels)
            fcntl.ioctl(fd, 0x40046B01, struct.pack('I', 0))

        # 2. Set SPI frequency (100 kHz)
        speed_hz = 100000
        fcntl.ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, struct.pack('I', speed_hz))

        print(f"SPI initialized: Mode=0, Speed={speed_hz/1000}kHz")

        # 3. Prepare transmit and receive buffers
        tx_data = bytes([0x9F, 0x00, 0x00, 0x00])
        # Create a writable receive buffer
        rx_buf = ctypes.create_string_buffer(4)

        # Get 64-bit memory addresses of buffers
        tx_addr = ctypes.addressof(ctypes.create_string_buffer(tx_data))
        rx_addr = ctypes.addressof(rx_buf)

        # 4. Pack spi_ioc_transfer structure exactly for Linux x86_64 / RISCV64:
        # __u64 tx_buf        (Q)
        # __u64 rx_buf        (Q)
        # __u32 len           (I)
        # __u32 speed_hz      (I)
        # __u16 delay_usecs   (H)
        # __u8  bits_per_word (B)
        # __u8  cs_change     (B)
        # __u8  tx_nbits      (B)
        # __u8  rx_nbits      (B)
        # __u16 pad           (H)
        spi_ioc_transfer = struct.pack(
            "=QQIIHBBBBH",
            tx_addr,    # tx_buf
            rx_addr,    # rx_buf
            4,          # len
            speed_hz,   # speed_hz
            0,          # delay_usecs
            8,          # bits_per_word (通常是 8 位的倍数)
            0,          # cs_change
            0,          # tx_nbits
            0,          # rx_nbits
            0           # pad
        )

        # 5. Execute transfer
        fcntl.ioctl(fd, SPI_IOC_MESSAGE_1, spi_ioc_transfer)

        # 6. Parse result
        result = [b for b in rx_buf.raw]
        print("-" * 40)
        print(f"Raw response data: {[hex(x) for x in result]}")
        print(f"Manufacturer ID: 0x{result[1]:02X}")
        print(f"Memory Type:     0x{result[2]:02X}")
        print(f"Capacity ID:     0x{result[3]:02X}")
        print("-" * 40)

    except OSError as e:
        print(f"Operation failed: {e}")
    finally:
        os.close(fd)

if __name__ == "__main__":
    # If device node is /dev/spidev0.0, call with (0, 0)
    read_flash_id(0, 0)
