#!/usr/bin/env python3
"""
K230 IOMUX Pin Configuration Tool

Display and set IOMUX pin configurations for K230 SoC.
Requires root privileges to access /dev/mem.

Usage:
    # View all pin configurations (controller 0)
    python3 <script_name>

    # View specific pin
    python3 <script_name> --pin io38

    # Set pin to specific function
    python3 <script_name> --set io38 alt1

    # Set pin config options (pull up/down, drive strength, etc.)
    python3 <script_name> --set io38 alt1 --ie --oe --pu --ds 8

    # View second IOMUX controller (PMUIOMUX at 0x91000080)
    python3 <script_name> --iomux 1

    # Set pin on second controller
    python3 <script_name> --iomux 1 --set io3 alt1
"""

import os
import sys
import mmap
import argparse
import re

# Get script name for usage messages
_script_name = os.path.basename(sys.argv[0])

import os
import sys
import mmap
import argparse
import re

# K230 IOMUX Base Addresses (from TRM Section 12.9.2)
# Controller 0: Main IOMUX at 0x91105000 (64 pins)
# Controller 1: PMUIOMUX at 0x91000080 (8 pins)
IOMUX_CONTROLLERS = [
    {"name": "iomux", "base": 0x91105000, "size": 0x100, "pins": 64, "basename": "io"},
    {"name": "pmuiomux", "base": 0x91000080, "size": 0x20, "pins": 8, "basename": "io"},
]

# Register bit fields (per TRM Section 12.9.2)
# Note: Bits 31:14 are read-only or reserved
#   Bit 31   DI   - Input data (RO)
#   Bit 30:14      - Reserved (RO)
#   Bit 13:11 IO_SEL - Function select (RW)
#   Bit 10   SL   - Slew rate enable (RW)
#   Bit 9    MSC  - Voltage control (RW)
#   Bit 8    IE   - Input enable (RW)
#   Bit 7    OE   - Output enable (RW)
#   Bit 6    PU   - Pull up (RW)
#   Bit 5    PD   - Pull down (RW)
#   Bit 4:1  DS   - Drive strength select (RW)
#   Bit 0    ST   - Schmitt trigger (RW)
RW_BITS_MASK = 0x00003FFF  # Only bits 13:0 are writable

IO_SEL_SHIFT = 11
IO_SEL_MASK = 0x7
SL_SHIFT = 10
SL_MASK = 0x1
MSC_SHIFT = 9
MSC_MASK = 0x1
IE_SHIFT = 8
IE_MASK = 0x1
OE_SHIFT = 7
OE_MASK = 0x1
PU_SHIFT = 6
PU_MASK = 0x1
PD_SHIFT = 5
PD_MASK = 0x1
DS_SHIFT = 1
DS_MASK = 0xF
ST_SHIFT = 0
ST_MASK = 0x1


# Function name hints from header file - maps IO_SEL value to function name
# Controller 0: pins 0-63, Controller 1: pins 0-7 (io0-io7)
def get_pin_funcs(pin, iomux_idx=0):
    """Get function mapping for a specific pin."""
    if iomux_idx == 0:
        # First controller: pins 0-63
        funcs = {
            0: {0: "GPIO0", 1: "BOOT0", 3: "TEST_PIN0"},
            1: {0: "GPIO1", 1: "BOOT1", 3: "TEST_PIN1"},
            2: {0: "GPIO2", 1: "JTAG_TCK", 2: "PULSE_CNTR0", 3: "TEST_PIN2"},
            3: {0: "GPIO3", 1: "JTAG_TDI", 2: "PULSE_CNTR1", 3: "UART1_TXD", 4: "TEST_PIN0"},
            4: {0: "GPIO4", 1: "JTAG_TDO", 2: "PULSE_CNTR2", 3: "UART1_RXD", 4: "TEST_PIN1"},
            5: {0: "GPIO5", 1: "JTAG_TMS", 2: "PULSE_CNTR3", 3: "UART2_TXD", 4: "TEST_PIN2"},
            6: {0: "GPIO6", 1: "JTAG_RST", 2: "PULSE_CNTR4", 3: "UART2_RXD", 4: "TEST_PIN3"},
            7: {0: "GPIO7", 1: "PWM2", 2: "I2C4_SCL", 3: "TEST_PIN3", 4: "DI0"},
            8: {0: "GPIO8", 1: "PWM3", 2: "I2C4_SDA", 3: "TEST_PIN4", 4: "DI1"},
            9: {0: "GPIO9", 1: "PWM4", 2: "UART1_TXD", 3: "I2C1_SCL", 4: "DI2"},
            10: {0: "GPIO10", 1: "3D_CTRL_IN", 2: "UART1_RXD", 3: "I2C1_SDA", 4: "DI3"},
            11: {0: "GPIO11", 1: "3D_CTRL_OUT1", 2: "UART2_TXD", 3: "I2C2_SCL", 4: "DO0"},
            12: {0: "GPIO12", 1: "3D_CTRL_OUT2", 2: "UART2_RXD", 3: "I2C2_SDA", 4: "DO1"},
            13: {0: "GPIO13", 1: "M_CLK1", 4: "DO2"},
            14: {0: "GPIO14", 1: "OSPI_CS", 2: "TEST_PIN5", 3: "QSPI0_CS0", 4: "DO3"},
            15: {0: "GPIO15", 1: "OSPI_CLK", 2: "TEST_PIN6", 3: "QSPI0_CLK", 4: "CO3"},
            16: {0: "GPIO16", 1: "OSPI_D0", 2: "QSPI1_CS4", 3: "QSPI0_D0", 4: "CO2"},
            17: {0: "GPIO17", 1: "OSPI_D1", 2: "QSPI1_CS3", 3: "QSPI0_D1", 4: "CO1"},
            18: {0: "GPIO18", 1: "OSPI_D2", 2: "QSPI1_CS2", 3: "QSPI0_D2", 4: "CO0"},
            19: {0: "GPIO19", 1: "OSPI_D3", 2: "QSPI1_CS1", 3: "QSPI0_D3", 4: "TEST_PIN4"},
            20: {0: "GPIO20", 1: "OSPI_D4", 2: "QSPI1_CS0", 3: "PULSE_CNTR0", 4: "TEST_PIN5"},
            21: {0: "GPIO21", 1: "OSPI_D5", 2: "QSPI1_CLK", 3: "PULSE_CNTR1", 4: "TEST_PIN6"},
            22: {0: "GPIO22", 1: "OSPI_D6", 2: "QSPI1_D0", 3: "PULSE_CNTR2", 4: "TEST_PIN7"},
            23: {0: "GPIO23", 1: "OSPI_D7", 2: "QSPI1_D1", 3: "PULSE_CNTR3", 4: "TEST_PIN8"},
            24: {0: "GPIO24", 1: "OSPI_DQS", 2: "QSPI1_D2", 3: "PULSE_CNTR4", 4: "TEST_PIN9"},
            25: {0: "GPIO25", 1: "PWM5", 2: "QSPI1_D3", 3: "PULSE_CNTR5", 4: "TEST_PIN10"},
            26: {0: "GPIO26", 1: "MMC1_CLK", 2: "TEST_PIN7", 3: "PDM_CLK"},
            27: {0: "GPIO27", 1: "MMC1_CMD", 2: "PULSE_CNTR5", 3: "PDM_IN0", 4: "CI0"},
            28: {0: "GPIO28", 1: "MMC1_D0", 2: "UART3_TXD", 3: "PDM_IN1", 4: "CI1"},
            29: {0: "GPIO29", 1: "MMC1_D1", 2: "UART3_RXD", 3: "3D_CTRL_IN", 4: "CI2"},
            30: {0: "GPIO30", 1: "MMC1_D2", 2: "UART3_RTS", 3: "3D_CTRL_OUT1", 4: "CI3"},
            31: {0: "GPIO31", 1: "MMC1_D3", 2: "UART3_CTS", 3: "3D_CTRL_OUT2", 4: "TEST_PIN11"},
            32: {0: "GPIO32", 1: "I2C0_SCL", 2: "IIS_CLK", 3: "UART3_TXD", 4: "TEST_PIN12"},
            33: {0: "GPIO33", 1: "I2C0_SDA", 2: "IIS_WS", 3: "UART3_RXD", 4: "TEST_PIN13"},
            34: {0: "GPIO34", 1: "I2C1_SCL", 2: "IIS_D_IN0_PDM_IN3", 3: "UART3_RTS", 4: "TEST_PIN14"},
            35: {0: "GPIO35", 1: "I2C1_SDA", 2: "IIS_D_OUT0_PDM_IN1", 3: "UART3_CTS", 4: "TEST_PIN15"},
            36: {0: "GPIO36", 1: "I2C3_SCL", 2: "IIS_D_IN1_PDM_IN2", 3: "UART4_TXD", 4: "TEST_PIN16"},
            37: {0: "GPIO37", 1: "I2C3_SDA", 2: "IIS_D_OUT1_PDM_IN0", 3: "UART4_RXD", 4: "TEST_PIN17"},
            38: {0: "GPIO38", 1: "UART0_TXD", 2: "TEST_PIN8", 3: "QSPI1_CS0", 4: "HSYNC0"},
            39: {0: "GPIO39", 1: "UART0_RXD", 2: "TEST_PIN9", 3: "QSPI1_CLK", 4: "VSYNC0"},
            40: {0: "GPIO40", 1: "UART1_TXD", 2: "I2C1_SCL", 3: "QSPI1_D0", 4: "TEST_PIN18"},
            41: {0: "GPIO41", 1: "UART1_RXD", 2: "I2C1_SDA", 3: "QSPI1_D1", 4: "TEST_PIN19"},
            42: {0: "GPIO42", 1: "UART1_RTS", 2: "PWM0", 3: "QSPI1_D2", 4: "TEST_PIN20"},
            43: {0: "GPIO43", 1: "UART1_CTS", 2: "PWM1", 3: "QSPI1_D3", 4: "TEST_PIN21"},
            44: {0: "GPIO44", 1: "UART2_TXD", 2: "I2C3_SCL", 3: "TEST_PIN10", 4: "SPI2AXI_CLK"},
            45: {0: "GPIO45", 1: "UART2_RXD", 2: "I2C3_SDA", 3: "TEST_PIN11", 4: "SPI2AXI_CS"},
            46: {0: "GPIO46", 1: "UART2_RTS", 2: "PWM2", 3: "I2C4_SCL", 4: "TEST_PIN22"},
            47: {0: "GPIO47", 1: "UART2_CTS", 2: "PWM3", 3: "I2C4_SDA", 4: "TEST_PIN23"},
            48: {0: "GPIO48", 1: "UART4_TXD", 2: "TEST_PIN12", 3: "I2C0_SCL", 4: "SPI2AXI_DIN"},
            49: {0: "GPIO49", 1: "UART4_RXD", 2: "TEST_PIN13", 3: "I2C0_SDA", 4: "SPI2AXI_DOUT"},
            50: {0: "GPIO50", 1: "UART3_TXD", 2: "I2C2_SCL", 3: "QSPI0_CS4", 4: "TEST_PIN24"},
            51: {0: "GPIO51", 1: "UART3_RXD", 2: "I2C2_SDA", 3: "QSPI0_CS3", 4: "TEST_PIN25"},
            52: {0: "GPIO52", 1: "UART3_RTS", 2: "PWM4", 3: "I2C3_SCL", 4: "TEST_PIN26"},
            53: {0: "GPIO53", 1: "UART3_CTS", 2: "PWM5", 3: "I2C3_SDA"},
            54: {0: "GPIO54", 1: "QSPI0_CS0", 2: "MMC1_CMD", 3: "PWM0", 4: "TEST_PIN27"},
            55: {0: "GPIO55", 1: "QSPI0_CLK", 2: "MMC1_CLK", 3: "PWM1", 4: "TEST_PIN28"},
            56: {0: "GPIO56", 1: "QSPI0_D0", 2: "MMC1_D0", 3: "PWM2", 4: "TEST_PIN29"},
            57: {0: "GPIO57", 1: "QSPI0_D1", 2: "MMC1_D1", 3: "PWM3", 4: "TEST_PIN30"},
            58: {0: "GPIO58", 1: "QSPI0_D2", 2: "MMC1_D2", 3: "PWM4", 4: "TEST_PIN31"},
            59: {0: "GPIO59", 1: "QSPI0_D3", 2: "MMC1_D3", 3: "PWM5"},
            60: {0: "GPIO60", 1: "PWM0", 2: "I2C0_SCL", 3: "QSPI0_CS2", 4: "HSYNC1"},
            61: {0: "GPIO61", 1: "PWM1", 2: "I2C0_SDA", 3: "QSPI0_CS1", 4: "VSYNC1"},
            62: {0: "GPIO62", 1: "M_CLK2", 2: "UART3_DE", 3: "TEST_PIN14"},
            63: {0: "GPIO63", 1: "M_CLK3", 2: "UART3_RE", 3: "TEST_PIN15"},
        }
    else:
        # Second controller: pins 0-7 (io0-io7)
        funcs = {
            0: {1: "GPIO64", 2: "INT0"},
            1: {1: "GPIO65", 2: "INT1"},
            2: {1: "GPIO66", 2: "INT2"},
            3: {1: "GPIO67", 2: "INT3"},
            4: {1: "GPIO68", 2: "INT4"},
            5: {1: "GPIO69", 2: "INT5"},
            6: {1: "GPIO70", 2: "OUT1"},
            7: {1: "GPIO71", 2: "OUT2"},
        }
    return funcs.get(pin, {})


# Alt name mapping: alt0->0, alt1->1, etc.
ALT_MAPPING = {
    "alt0": 0, "alt1": 1, "alt2": 2, "alt3": 3, "alt4": 4,
    "alt5": 5, "alt6": 6, "alt7": 7,
}


def get_function_number(func_name, pin=None):
    """Convert function name to IO_SEL number.

    Args:
        func_name: Function name (alt0, alt1, or specific like GPIO38, JTAG_TDI)
        pin: Pin number for specific function lookup

    Returns:
        IO_SEL number (0-7) or None if invalid
    """
    func_name = func_name.lower().strip()

    # First try alt mapping
    if func_name in ALT_MAPPING:
        return ALT_MAPPING[func_name]

    # If pin is specified, check if func_name is a specific function for that pin
    if pin is not None:
        pin_funcs = get_pin_funcs(pin, 0)
        for io_sel, fn in pin_funcs.items():
            if fn.lower() == func_name:
                return io_sel

    return None


def get_io_sel_name(io_sel, pin_funcs):
    """Get the function name for a given IO_SEL value."""
    name_map = {
        0: ("FUNC1", "alt0"),
        1: ("FUNC2", "alt1"),
        2: ("FUNC3", "alt2"),
        3: ("FUNC4", "alt3"),
        4: ("FUNC5", "alt4"),
        5: ("FUNC6", "alt5"),
        6: ("FUNC7", "alt6"),
        7: ("FUNC8", "alt7"),
    }

    base_name, alt_name = name_map.get(io_sel, (f"FUNC{io_sel + 1}", f"alt{io_sel}"))
    func_spec = pin_funcs.get(io_sel)

    if func_spec:
        return f"{alt_name} ({func_spec})"
    return alt_name


def parse_pin_name(pin_name, iomux_idx=0):
    """Parse pin name like 'io38' to pin number for specified controller."""
    controller = IOMUX_CONTROLLERS[iomux_idx]
    basename = controller["basename"]
    match = re.match(r'^%s(\d+)$' % basename, pin_name.lower())
    if match:
        pin_num = int(match.group(1))
        num_pins = controller["pins"]
        if 0 <= pin_num < num_pins:
            return pin_num
    return None


def get_pin_name(pin, iomux_idx=0):
    """Get the pin name string for specified controller."""
    controller = IOMUX_CONTROLLERS[iomux_idx]
    basename = controller["basename"]
    return f"{basename}{pin}"


def print_header():
    """Print the table header."""
    print("=" * 100)
    print(f"{'Pin':<6} {'Function':<25} {'Reg Value':<12} {'Config'}")
    print("=" * 100)


def print_pin_info(pin, reg_value, pin_funcs, iomux_idx=0, show_funcs=False):
    """Print information for a single pin."""
    io_sel = (reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK
    func_name = get_io_sel_name(io_sel, pin_funcs)

    # Extract config bits
    ie = (reg_value >> IE_SHIFT) & IE_MASK
    oe = (reg_value >> OE_SHIFT) & OE_MASK
    pu = (reg_value >> PU_SHIFT) & PU_MASK
    pd = (reg_value >> PD_SHIFT) & PD_MASK
    ds = (reg_value >> DS_SHIFT) & DS_MASK
    sl = (reg_value >> SL_SHIFT) & SL_MASK
    st = (reg_value >> ST_SHIFT) & ST_MASK

    # Build config string
    config_str = []
    if ie:
        config_str.append("IE")
    if oe:
        config_str.append("OE")
    if pu:
        config_str.append("PU")
    if pd:
        config_str.append("PD")
    if sl:
        config_str.append("SL")
    if st:
        config_str.append("ST")
    config_str.append(f"DS={ds}")

    config_str = " ".join(config_str) if config_str else "none"

    controller = IOMUX_CONTROLLERS[iomux_idx]
    io_addr = controller["base"] + 4 * pin
    print(f"io{pin:<5} {func_name:<25} {io_addr:#010x}={reg_value:#010x} {config_str:<20}")

    if show_funcs:
        print(f"        funcs[", end="")
        for sel, name in pin_funcs.items():
            alt = ["alt0", "alt1", "alt2", "alt3", "alt4", "alt5", "alt6", "alt7"][sel]
            print(f"{name}/", end="")
        print("]")
        print()


def read_pin(mm, pin, offset_base=0):
    """Read a pin's register value."""
    offset = offset_base + pin * 4
    return int.from_bytes(mm[offset:offset + 4], byteorder='little')


def write_pin(mm, pin, value, offset_base=0, iomux_idx=0, confirm=True):
    """Write a pin's register value.

    Args:
        mm: mmap object
        pin: Pin number
        value: Value to write
        offset_base: Base offset for the register
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)
        confirm: If True, prompt user for confirmation before writing

    Returns:
        True if written successfully, False if cancelled
    """
    offset = offset_base + pin * 4
    controller = IOMUX_CONTROLLERS[iomux_idx]
    phys_addr = controller["base"] + (pin * 4)

    print(f"Pin:              {pin}")
    print(f"Physical address: 0x{phys_addr:08x}")
    print(f"Value to write:   0x{value:08x}")

    if confirm:
        # Read current value for display
        current_value = int.from_bytes(mm[offset:offset + 4], byteorder='little')
        print(f"Current value:    0x{current_value:08x}")

        # Calculate register offset from base for display
        reg_offset = phys_addr - controller["base"]
        print(f"Register offset:  0x{reg_offset:04x}")

        while True:
            user_input = input("Confirm write? [y/N]: ").strip().lower()
            if user_input in ['y', 'yes']:
                break
            elif user_input in ['', 'n', 'no']:
                print("Write cancelled by user")
                return False
            else:
                print("Please enter 'y' or 'n'")

    # Use devmem to write directly - this is proven to work
    import subprocess
    subprocess.run(['devmem', hex(phys_addr), '32', hex(value)], capture_output=True)
    # Use devmem to read back for verification (avoids mmap caching issues)
    result = subprocess.run(['devmem', hex(phys_addr), '32'], capture_output=True, text=True)
    verified = int(result.stdout.strip(), 0)
    print(f"After write read: 0x{verified:08x}")
    # Only verify writable bits (bits 13:0), ignore read-only bits (DI and reserved)
    if (verified & RW_BITS_MASK) != (value & RW_BITS_MASK):
        print(f"  Warning: Verification mismatch (expected: 0x{value:08x})")
        return False
    return True


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="K230 IOMUX Pin Configuration Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=f"""
Examples:
  # View all pin configurations (controller 0)
  python3 {_script_name}

  # View specific pin
  python3 {_script_name} --pin io38

  # View pin with all available functions
  python3 {_script_name} --pin io38 --funcs

  # Set pin to alt1 function
  python3 {_script_name} --set io38 alt1

  # Set pin with configuration options
  python3 {_script_name} --set io38 alt1 --ie --oe --pu --ds 8

  # Set pin using specific function name (must specify pin)
  python3 {_script_name} --set io38 UART0_TXD

  # View controller 1 (PMUIOMUX at 0x91000080)
  python3 {_script_name} --iomux 1

  # Set pin on controller 1
  python3 {_script_name} --iomux 1 --set io3 alt1
        """
    )

    parser.add_argument('--pin', '-p', type=str, help='Specific pin to view (e.g., io38)')
    parser.add_argument('--funcs', '-f', action='store_true', help='Show all available functions for a pin')
    parser.add_argument('--set', '-s', nargs=2, metavar=('PIN', 'FUNC'), help='Set pin to function (e.g., io38 alt1)')
    parser.add_argument('--iomux', '-i', type=int, default=0, choices=[0, 1], help='IOMUX controller index (0=main, 1=pmuiomux)')
    parser.add_argument('--ie', action='store_true', help='Enable input')
    parser.add_argument('--no-ie', action='store_true', help='Disable input')
    parser.add_argument('--oe', action='store_true', help='Enable output')
    parser.add_argument('--no-oe', action='store_true', help='Disable output')
    parser.add_argument('--pu', action='store_true', help='Enable pull-up')
    parser.add_argument('--pd', action='store_true', help='Enable pull-down')
    parser.add_argument('--no-pull', action='store_true', help='Disable pull-up/down')
    parser.add_argument('--ds', type=int, choices=range(1, 16), metavar='1-15', help='Drive strength (1-15)')
    parser.add_argument('--sl', action='store_true', help='Enable slew rate')
    parser.add_argument('--no-sl', action='store_true', help='Disable slew rate')
    parser.add_argument('--st', action='store_true', help='Enable schmitt trigger')
    parser.add_argument('--no-st', action='store_true', help='Disable schmitt trigger')

    return parser.parse_args()


def main():
    """Main function to dump/set IOMUX configurations."""
    mem_path = "/dev/mem"
    args = parse_args()
    iomux_idx = args.iomux
    controller = IOMUX_CONTROLLERS[iomux_idx]

    if os.geteuid() != 0:
        print("Error: root privileges required")
        print(f"Usage: python3 {_script_name}")
        sys.exit(1)

    if not os.path.exists(mem_path):
        print(f"Error: {mem_path} does not exist")
        sys.exit(1)

    # Check if we need to set a pin
    if args.set:
        pin_name, func_name = args.set
        pin = parse_pin_name(pin_name, iomux_idx)
        if pin is None:
            print(f"Error: invalid pin name '{pin_name}'")
            num_pins = controller["pins"]
            basename = controller["basename"]
            print(f"Format: {basename}0 to {basename}{num_pins - 1}")
            sys.exit(1)

        func_num = get_function_number(func_name, pin)
        if func_num is None:
            print(f"Error: invalid function '{func_name}' for pin {pin_name}")
            # Show available functions
            pin_funcs = get_pin_funcs(pin, iomux_idx)
            print(f"Available functions for {pin_name}:")
            for io_sel, fn in sorted(pin_funcs.items(), key=lambda x: x[1]):
                alt = ["alt0", "alt1", "alt2", "alt3", "alt4", "alt5", "alt6", "alt7"][io_sel]
                print(f"  {alt} ({fn})")
            sys.exit(1)

        if not args.ie and not args.no_ie and not args.oe and not args.no_oe and \
           not args.pu and not args.pd and not args.no_pull and \
           not args.ds and not args.sl and not args.no_sl and \
           not args.st and not args.no_st:
            # No config changes, just function select
            pass

    try:
        # Open /dev/mem with read-write access
        with open(mem_path, "r+b") as mem_file:
            # Get page size for mmap alignment
            page_size = os.sysconf("SC_PAGE_SIZE")
            # Align base address to page boundary
            base_aligned = controller["base"] & ~(page_size - 1)
            # Calculate offset within the page
            page_offset = controller["base"] - base_aligned
            # Map at least one full page to ensure proper access
            map_size = max(page_size, page_offset + controller["size"])

            # Memory map the IOMUX region
            mm = mmap.mmap(
                mem_file.fileno(),
                map_size,
                mmap.MAP_SHARED,
                mmap.PROT_READ | mmap.PROT_WRITE,
                offset=base_aligned
            )

            if args.set:
                # Set mode
                pin = parse_pin_name(args.set[0], iomux_idx)
                func_num = get_function_number(args.set[1], pin)

                if pin is None or func_num is None:
                    print("Error: invalid arguments")
                    sys.exit(1)

                # Read current value
                reg_value = read_pin(mm, pin, page_offset)

                # Build new value
                # Clear IO_SEL bits
                new_value = reg_value & ~(IO_SEL_MASK << IO_SEL_SHIFT)
                # Set new function
                new_value |= (func_num << IO_SEL_SHIFT)

                # Handle input enable
                if args.ie:
                    new_value &= ~(IE_MASK << IE_SHIFT)
                    new_value |= (1 << IE_SHIFT)
                elif args.no_ie:
                    new_value &= ~(IE_MASK << IE_SHIFT)

                # Handle output enable
                if args.oe:
                    new_value &= ~(OE_MASK << OE_SHIFT)
                    new_value |= (1 << OE_SHIFT)
                elif args.no_oe:
                    new_value &= ~(OE_MASK << OE_SHIFT)

                # Handle pull up/down
                if args.pu:
                    new_value &= ~(PU_MASK << PU_SHIFT)
                    new_value |= (1 << PU_SHIFT)
                elif args.pd:
                    new_value &= ~(PD_MASK << PD_SHIFT)
                    new_value |= (1 << PD_SHIFT)
                elif args.no_pull:
                    new_value &= ~((PU_MASK << PU_SHIFT) | (PD_MASK << PD_SHIFT))

                # Handle drive strength
                if args.ds is not None:
                    new_value &= ~(DS_MASK << DS_SHIFT)
                    new_value |= ((args.ds & DS_MASK) << DS_SHIFT)

                # Handle slew rate
                if args.sl:
                    new_value &= ~(SL_MASK << SL_SHIFT)
                    new_value |= (1 << SL_SHIFT)
                elif args.no_sl:
                    new_value &= ~(SL_MASK << SL_SHIFT)

                # Handle schmitt trigger
                if args.st:
                    new_value &= ~(ST_MASK << ST_SHIFT)
                    new_value |= (1 << ST_SHIFT)
                elif args.no_st:
                    new_value &= ~(ST_MASK << ST_SHIFT)

                # Get pin functions for display
                pin_funcs = get_pin_funcs(pin, iomux_idx)
                current_func = get_io_sel_name((reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK, pin_funcs)
                new_func = get_io_sel_name(func_num, pin_funcs)

                pin_name_str = get_pin_name(pin, iomux_idx)
                print(f"Setting {pin_name_str}:")
                print(f"  Current:  {current_func}")
                print(f"  New:      {new_func}")
                print(f"  Register: {reg_value:#010x} -> {new_value:#010x}")

                # Write the new value (with user confirmation)
                print()
                if write_pin(mm, pin, new_value, page_offset, iomux_idx, confirm=True):
                    print(f"  Status:   Success")
                else:
                    print("  Status:   Write failed")
                    sys.exit(1)

            else:
                # View mode
                print("=" * 80)
                print("K230 IOMUX Pin Configuration")
                print(f"Controller: {controller['name']} (0x{controller['base']:x})")
                print(f"Base:       0x{controller['base']:x}")
                print(f"Size:       0x{controller['size']:x} ({controller['pins']} pins)")
                print("=" * 80)
                # print()
                # print("Register bit layout (ref TRM Section 12.9.2):")
                # print("  Bit 31   DI   - Input data (RO)")
                # print("  Bit 13:11 IO_SEL - Function select (000=func1, 001=func2, etc.)")
                # print("  Bit 10   SL   - Slew rate enable")
                # print("  Bit 9    MSC  - Voltage control")
                # print("  Bit 8    IE   - Input enable (1=enable)")
                # print("  Bit 7    OE   - Output enable (1=enable)")
                # print("  Bit 6    PU   - Pull up (1=enable)")
                # print("  Bit 5    PD   - Pull down (1=enable)")
                # print("  Bit 4:1  DS   - Drive strength select")
                # print("  Bit 0    ST   - Schmitt trigger (1=enable)")
                # print()
                # print("Function mapping:")
                # for i in range(8):
                #     alt = ["alt0", "alt1", "alt2", "alt3", "alt4", "alt5", "alt6", "alt7"][i]
                #     print(f"  IO_SEL={i} ({alt}): Function {i + 1}")
                # print()
                print()

                if args.pin:
                    # Specific pin
                    pin = parse_pin_name(args.pin, iomux_idx)
                    if pin is None:
                        print(f"Error: invalid pin name '{args.pin}'")
                        num_pins = controller["pins"]
                        basename = controller["basename"]
                        print(f"Format: {basename}0 to {basename}{num_pins - 1}")
                        sys.exit(1)

                    pin_funcs = get_pin_funcs(pin, iomux_idx)
                    reg_value = read_pin(mm, pin, page_offset)
                    print_pin_info(pin, reg_value, pin_funcs, iomux_idx, show_funcs=args.funcs)

                    # if args.funcs:
                    #     print()
                    #     print(f"  Available functions for {get_pin_name(pin, iomux_idx)}:")
                    #     for sel, name in sorted(pin_funcs.items()):
                    #         alt = ["alt0", "alt1", "alt2", "alt3", "alt4", "alt5", "alt6", "alt7"][sel]
                    #         print(f"    {alt}: {name}")
                else:
                    # All pins
                    print("Pin Configuration:")
                    print("-" * 80)

                    for pin in range(controller["pins"]):
                        reg_value = read_pin(mm, pin, page_offset)
                        pin_funcs = get_pin_funcs(pin, iomux_idx)
                        print_pin_info(pin, reg_value, pin_funcs, iomux_idx, show_funcs=args.funcs)

                    print("-" * 80)
                    print()

                    # Summary
                    print("Summary:")
                    for pin in range(controller["pins"]):
                        reg_value = read_pin(mm, pin, page_offset)
                        io_sel = (reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK
                        pin_name = get_pin_name(pin, iomux_idx)
                        func_name = get_io_sel_name(io_sel, get_pin_funcs(pin, iomux_idx))
                        print(f"  {pin_name}: {func_name}")

                print()
                print("=" * 80)

            mm.close()

    except PermissionError:
        print("Error: cannot access /dev/mem (need root privileges)")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
