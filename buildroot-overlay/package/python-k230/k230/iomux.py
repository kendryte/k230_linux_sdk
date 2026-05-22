# SPDX-FileCopyrightText: 2024 Kendryte
#
# SPDX-License-Identifier: MIT
"""
K230 IOMUX Pin Configuration Module

This module provides functions to read and configure IOMUX pins on the K230 SoC.
It requires root privileges to access /dev/mem.

Example Usage:
    from k230 import iomux

    # Get available functions for a pin
    funcs = iomux.get_pin_functions(38)
    print(funcs)  # {0: 'GPIO38', 1: 'UART0_TXD', ...}

    # Get current function of a pin
    func = iomux.get_current_function(38)
    print(func)  # 'alt0 (GPIO38)'

    # Get full status of a pin (function + input/output/pull-up/down/etc.)
    status = iomux.get_pin_status(38)
    print(status)  # 'alt0 (GPIO38), input, pull-up, ds=8'

    # Set a pin to a specific function
    iomux.set_pin_function(38, 'alt1')  # Set to UART0_TXD
    iomux.set_pin_function(38, iomux.ALT1)

    # Configure pin properties
    iomux.set_pin_config(38, ie=True, oe=True, pu=True, ds=8)
"""

from typing import Dict, Optional, Union
import os
import re
import mmap

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


def get_pin_functions(pin: int, iomux_idx: int = 0) -> Dict[int, str]:
    """Get function mapping for a specific pin.

    Args:
        pin: Pin number (0-63 for controller 0, 0-7 for controller 1)
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Dictionary mapping IO_SEL values to function names
    """
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
ALT0 = 0
ALT1 = 1
ALT2 = 2
ALT3 = 3
ALT4 = 4
ALT5 = 5
ALT6 = 6
ALT7 = 7

ALT_MAPPING = {
    "alt0": 0, "alt1": 1, "alt2": 2, "alt3": 3, "alt4": 4,
    "alt5": 5, "alt6": 6, "alt7": 7,
}


class Function:
    """Function constant for IOMUX pin configuration.

    This class allows using function names as constants without quotes.
    Example: set_pin_function(38, UART0_TXD) instead of set_pin_function(38, 'UART0_TXD')
    """
    def __init__(self, name: str):
        self._name = name

    def __str__(self):
        return self._name

    def __repr__(self):
        return f"Function({self._name!r})"

    def __eq__(self, other):
        if isinstance(other, Function):
            return self._name == other._name
        return self._name == other


# Function name constants - can be used without quotes
# Format: FUNCTION_PIN = Function("FUNCTION_PIN")
# Examples: UART0_TXD, I2C0_SDA, GPIO38, etc.

# GPIO functions
GPIO0 = Function("GPIO0")
GPIO1 = Function("GPIO1")
GPIO2 = Function("GPIO2")
GPIO3 = Function("GPIO3")
GPIO4 = Function("GPIO4")
GPIO5 = Function("GPIO5")
GPIO6 = Function("GPIO6")
GPIO7 = Function("GPIO7")
GPIO8 = Function("GPIO8")
GPIO9 = Function("GPIO9")
GPIO10 = Function("GPIO10")
GPIO11 = Function("GPIO11")
GPIO12 = Function("GPIO12")
GPIO13 = Function("GPIO13")
GPIO14 = Function("GPIO14")
GPIO15 = Function("GPIO15")
GPIO16 = Function("GPIO16")
GPIO17 = Function("GPIO17")
GPIO18 = Function("GPIO18")
GPIO19 = Function("GPIO19")
GPIO20 = Function("GPIO20")
GPIO21 = Function("GPIO21")
GPIO22 = Function("GPIO22")
GPIO23 = Function("GPIO23")
GPIO24 = Function("GPIO24")
GPIO25 = Function("GPIO25")
GPIO26 = Function("GPIO26")
GPIO27 = Function("GPIO27")
GPIO28 = Function("GPIO28")
GPIO29 = Function("GPIO29")
GPIO30 = Function("GPIO30")
GPIO31 = Function("GPIO31")
GPIO32 = Function("GPIO32")
GPIO33 = Function("GPIO33")
GPIO34 = Function("GPIO34")
GPIO35 = Function("GPIO35")
GPIO36 = Function("GPIO36")
GPIO37 = Function("GPIO37")
GPIO38 = Function("GPIO38")
GPIO39 = Function("GPIO39")
GPIO40 = Function("GPIO40")
GPIO41 = Function("GPIO41")
GPIO42 = Function("GPIO42")
GPIO43 = Function("GPIO43")
GPIO44 = Function("GPIO44")
GPIO45 = Function("GPIO45")
GPIO46 = Function("GPIO46")
GPIO47 = Function("GPIO47")
GPIO48 = Function("GPIO48")
GPIO49 = Function("GPIO49")
GPIO50 = Function("GPIO50")
GPIO51 = Function("GPIO51")
GPIO52 = Function("GPIO52")
GPIO53 = Function("GPIO53")
GPIO54 = Function("GPIO54")
GPIO55 = Function("GPIO55")
GPIO56 = Function("GPIO56")
GPIO57 = Function("GPIO57")
GPIO58 = Function("GPIO58")
GPIO59 = Function("GPIO59")
GPIO60 = Function("GPIO60")
GPIO61 = Function("GPIO61")
GPIO62 = Function("GPIO62")
GPIO63 = Function("GPIO63")
GPIO64 = Function("GPIO64")
GPIO65 = Function("GPIO65")
GPIO66 = Function("GPIO66")
GPIO67 = Function("GPIO67")
GPIO68 = Function("GPIO68")
GPIO69 = Function("GPIO69")
GPIO70 = Function("GPIO70")
GPIO71 = Function("GPIO71")

# Boot and JTAG functions
BOOT0 = Function("BOOT0")
BOOT1 = Function("BOOT1")
JTAG_TCK = Function("JTAG_TCK")
JTAG_TDI = Function("JTAG_TDI")
JTAG_TDO = Function("JTAG_TDO")
JTAG_TMS = Function("JTAG_TMS")
JTAG_RST = Function("JTAG_RST")

# PWM functions
PWM0 = Function("PWM0")
PWM1 = Function("PWM1")
PWM2 = Function("PWM2")
PWM3 = Function("PWM3")
PWM4 = Function("PWM4")
PWM5 = Function("PWM5")

# I2C functions
I2C0_SCL = Function("I2C0_SCL")
I2C0_SDA = Function("I2C0_SDA")
I2C1_SCL = Function("I2C1_SCL")
I2C1_SDA = Function("I2C1_SDA")
I2C2_SCL = Function("I2C2_SCL")
I2C2_SDA = Function("I2C2_SDA")
I2C3_SCL = Function("I2C3_SCL")
I2C3_SDA = Function("I2C3_SDA")
I2C4_SCL = Function("I2C4_SCL")
I2C4_SDA = Function("I2C4_SDA")

# UART functions
UART0_TXD = Function("UART0_TXD")
UART0_RXD = Function("UART0_RXD")
UART1_TXD = Function("UART1_TXD")
UART1_RXD = Function("UART1_RXD")
UART1_RTS = Function("UART1_RTS")
UART1_CTS = Function("UART1_CTS")
UART2_TXD = Function("UART2_TXD")
UART2_RXD = Function("UART2_RXD")
UART2_RTS = Function("UART2_RTS")
UART2_CTS = Function("UART2_CTS")
UART3_TXD = Function("UART3_TXD")
UART3_RXD = Function("UART3_RXD")
UART3_RTS = Function("UART3_RTS")
UART3_CTS = Function("UART3_CTS")
UART3_DE = Function("UART3_DE")
UART3_RE = Function("UART3_RE")
UART4_TXD = Function("UART4_TXD")
UART4_RXD = Function("UART4_RXD")

# IIS/PDM functions
IIS_CLK = Function("IIS_CLK")
IIS_WS = Function("IIS_WS")
IIS_D_IN0_PDM_IN3 = Function("IIS_D_IN0_PDM_IN3")
IIS_D_OUT0_PDM_IN1 = Function("IIS_D_OUT0_PDM_IN1")
IIS_D_IN1_PDM_IN2 = Function("IIS_D_IN1_PDM_IN2")
IIS_D_OUT1_PDM_IN0 = Function("IIS_D_OUT1_PDM_IN0")
PDM_CLK = Function("PDM_CLK")
PDM_IN0 = Function("PDM_IN0")
PDM_IN1 = Function("PDM_IN1")
PDM_IN2 = Function("PDM_IN2")
PDM_IN3 = Function("PDM_IN3")

# MMC/SDIO functions
MMC1_CLK = Function("MMC1_CLK")
MMC1_CMD = Function("MMC1_CMD")
MMC1_D0 = Function("MMC1_D0")
MMC1_D1 = Function("MMC1_D1")
MMC1_D2 = Function("MMC1_D2")
MMC1_D3 = Function("MMC1_D3")

# Pulse counter functions
PULSE_CNTR0 = Function("PULSE_CNTR0")
PULSE_CNTR1 = Function("PULSE_CNTR1")
PULSE_CNTR2 = Function("PULSE_CNTR2")
PULSE_CNTR3 = Function("PULSE_CNTR3")
PULSE_CNTR4 = Function("PULSE_CNTR4")
PULSE_CNTR5 = Function("PULSE_CNTR5")

# OSPI/QSPI functions
OSPI_CS = Function("OSPI_CS")
OSPI_CLK = Function("OSPI_CLK")
OSPI_D0 = Function("OSPI_D0")
OSPI_D1 = Function("OSPI_D1")
OSPI_D2 = Function("OSPI_D2")
OSPI_D3 = Function("OSPI_D3")
OSPI_D4 = Function("OSPI_D4")
OSPI_D5 = Function("OSPI_D5")
OSPI_D6 = Function("OSPI_D6")
OSPI_D7 = Function("OSPI_D7")
OSPI_DQS = Function("OSPI_DQS")
QSPI0_CS0 = Function("QSPI0_CS0")
QSPI0_CLK = Function("QSPI0_CLK")
QSPI0_D0 = Function("QSPI0_D0")
QSPI0_D1 = Function("QSPI0_D1")
QSPI0_D2 = Function("QSPI0_D2")
QSPI0_D3 = Function("QSPI0_D3")
QSPI0_CS4 = Function("QSPI0_CS4")
QSPI0_CS3 = Function("QSPI0_CS3")
QSPI1_CS0 = Function("QSPI1_CS0")
QSPI1_CLK = Function("QSPI1_CLK")
QSPI1_D0 = Function("QSPI1_D0")
QSPI1_D1 = Function("QSPI1_D1")
QSPI1_D2 = Function("QSPI1_D2")
QSPI1_D3 = Function("QSPI1_D3")
QSPI1_CS4 = Function("QSPI1_CS4")
QSPI1_CS3 = Function("QSPI1_CS3")
QSPI1_CS2 = Function("QSPI1_CS2")
QSPI1_CS1 = Function("QSPI1_CS1")

# 3D control functions
D_CTRL_IN = Function("3D_CTRL_IN")
D_CTRL_OUT1 = Function("3D_CTRL_OUT1")
D_CTRL_OUT2 = Function("3D_CTRL_OUT2")

# M_CLK functions
M_CLK1 = Function("M_CLK1")
M_CLK2 = Function("M_CLK2")
M_CLK3 = Function("M_CLK3")

# Test pins
TEST_PIN0 = Function("TEST_PIN0")
TEST_PIN1 = Function("TEST_PIN1")
TEST_PIN2 = Function("TEST_PIN2")
TEST_PIN3 = Function("TEST_PIN3")
TEST_PIN4 = Function("TEST_PIN4")
TEST_PIN5 = Function("TEST_PIN5")
TEST_PIN6 = Function("TEST_PIN6")
TEST_PIN7 = Function("TEST_PIN7")
TEST_PIN8 = Function("TEST_PIN8")
TEST_PIN9 = Function("TEST_PIN9")
TEST_PIN10 = Function("TEST_PIN10")
TEST_PIN11 = Function("TEST_PIN11")
TEST_PIN12 = Function("TEST_PIN12")
TEST_PIN13 = Function("TEST_PIN13")
TEST_PIN14 = Function("TEST_PIN14")
TEST_PIN15 = Function("TEST_PIN15")
TEST_PIN16 = Function("TEST_PIN16")
TEST_PIN17 = Function("TEST_PIN17")
TEST_PIN18 = Function("TEST_PIN18")
TEST_PIN19 = Function("TEST_PIN19")
TEST_PIN20 = Function("TEST_PIN20")
TEST_PIN21 = Function("TEST_PIN21")
TEST_PIN22 = Function("TEST_PIN22")
TEST_PIN23 = Function("TEST_PIN23")
TEST_PIN24 = Function("TEST_PIN24")
TEST_PIN25 = Function("TEST_PIN25")
TEST_PIN26 = Function("TEST_PIN26")
TEST_PIN27 = Function("TEST_PIN27")
TEST_PIN28 = Function("TEST_PIN28")
TEST_PIN29 = Function("TEST_PIN29")
TEST_PIN30 = Function("TEST_PIN30")
TEST_PIN31 = Function("TEST_PIN31")
TEST_PIN32 = Function("TEST_PIN32")
TEST_PIN33 = Function("TEST_PIN33")
TEST_PIN34 = Function("TEST_PIN34")
TEST_PIN35 = Function("TEST_PIN35")
TEST_PIN36 = Function("TEST_PIN36")
TEST_PIN37 = Function("TEST_PIN37")
TEST_PIN38 = Function("TEST_PIN38")
TEST_PIN39 = Function("TEST_PIN39")
TEST_PIN40 = Function("TEST_PIN40")
TEST_PIN41 = Function("TEST_PIN41")
TEST_PIN42 = Function("TEST_PIN42")
TEST_PIN43 = Function("TEST_PIN43")
TEST_PIN44 = Function("TEST_PIN44")
TEST_PIN45 = Function("TEST_PIN45")
TEST_PIN46 = Function("TEST_PIN46")
TEST_PIN47 = Function("TEST_PIN47")
TEST_PIN48 = Function("TEST_PIN48")
TEST_PIN49 = Function("TEST_PIN49")
TEST_PIN50 = Function("TEST_PIN50")
TEST_PIN51 = Function("TEST_PIN51")
TEST_PIN52 = Function("TEST_PIN52")
TEST_PIN53 = Function("TEST_PIN53")
TEST_PIN54 = Function("TEST_PIN54")
TEST_PIN55 = Function("TEST_PIN55")
TEST_PIN56 = Function("TEST_PIN56")
TEST_PIN57 = Function("TEST_PIN57")
TEST_PIN58 = Function("TEST_PIN58")
TEST_PIN59 = Function("TEST_PIN59")
TEST_PIN60 = Function("TEST_PIN60")
TEST_PIN61 = Function("TEST_PIN61")
TEST_PIN62 = Function("TEST_PIN62")
TEST_PIN63 = Function("TEST_PIN63")
TEST_PIN64 = Function("TEST_PIN64")
TEST_PIN65 = Function("TEST_PIN65")
TEST_PIN66 = Function("TEST_PIN66")
TEST_PIN67 = Function("TEST_PIN67")
TEST_PIN68 = Function("TEST_PIN68")
TEST_PIN69 = Function("TEST_PIN69")
TEST_PIN70 = Function("TEST_PIN70")
TEST_PIN71 = Function("TEST_PIN71")
TEST_PIN72 = Function("TEST_PIN72")
TEST_PIN73 = Function("TEST_PIN73")
TEST_PIN74 = Function("TEST_PIN74")
TEST_PIN75 = Function("TEST_PIN75")
TEST_PIN76 = Function("TEST_PIN76")
TEST_PIN77 = Function("TEST_PIN77")
TEST_PIN78 = Function("TEST_PIN78")
TEST_PIN79 = Function("TEST_PIN79")
TEST_PIN80 = Function("TEST_PIN80")
TEST_PIN81 = Function("TEST_PIN81")
TEST_PIN82 = Function("TEST_PIN82")
TEST_PIN83 = Function("TEST_PIN83")
TEST_PIN84 = Function("TEST_PIN84")
TEST_PIN85 = Function("TEST_PIN85")
TEST_PIN86 = Function("TEST_PIN86")
TEST_PIN87 = Function("TEST_PIN87")
TEST_PIN88 = Function("TEST_PIN88")
TEST_PIN89 = Function("TEST_PIN89")
TEST_PIN90 = Function("TEST_PIN90")
TEST_PIN91 = Function("TEST_PIN91")
TEST_PIN92 = Function("TEST_PIN92")
TEST_PIN93 = Function("TEST_PIN93")
TEST_PIN94 = Function("TEST_PIN94")
TEST_PIN95 = Function("TEST_PIN95")
TEST_PIN96 = Function("TEST_PIN96")
TEST_PIN97 = Function("TEST_PIN97")
TEST_PIN98 = Function("TEST_PIN98")
TEST_PIN99 = Function("TEST_PIN99")
TEST_PIN100 = Function("TEST_PIN100")
TEST_PIN101 = Function("TEST_PIN101")
TEST_PIN102 = Function("TEST_PIN102")
TEST_PIN103 = Function("TEST_PIN103")
TEST_PIN104 = Function("TEST_PIN104")
TEST_PIN105 = Function("TEST_PIN105")
TEST_PIN106 = Function("TEST_PIN106")
TEST_PIN107 = Function("TEST_PIN107")
TEST_PIN108 = Function("TEST_PIN108")
TEST_PIN109 = Function("TEST_PIN109")
TEST_PIN110 = Function("TEST_PIN110")
TEST_PIN111 = Function("TEST_PIN111")
TEST_PIN112 = Function("TEST_PIN112")
TEST_PIN113 = Function("TEST_PIN113")
TEST_PIN114 = Function("TEST_PIN114")
TEST_PIN115 = Function("TEST_PIN115")
TEST_PIN116 = Function("TEST_PIN116")
TEST_PIN117 = Function("TEST_PIN117")
TEST_PIN118 = Function("TEST_PIN118")
TEST_PIN119 = Function("TEST_PIN119")
TEST_PIN120 = Function("TEST_PIN120")
TEST_PIN121 = Function("TEST_PIN121")
TEST_PIN122 = Function("TEST_PIN122")
TEST_PIN123 = Function("TEST_PIN123")
TEST_PIN124 = Function("TEST_PIN124")
TEST_PIN125 = Function("TEST_PIN125")
TEST_PIN126 = Function("TEST_PIN126")
TEST_PIN127 = Function("TEST_PIN127")
TEST_PIN128 = Function("TEST_PIN128")
TEST_PIN129 = Function("TEST_PIN129")
TEST_PIN130 = Function("TEST_PIN130")
TEST_PIN131 = Function("TEST_PIN131")
TEST_PIN132 = Function("TEST_PIN132")
TEST_PIN133 = Function("TEST_PIN133")
TEST_PIN134 = Function("TEST_PIN134")
TEST_PIN135 = Function("TEST_PIN135")
TEST_PIN136 = Function("TEST_PIN136")
TEST_PIN137 = Function("TEST_PIN137")
TEST_PIN138 = Function("TEST_PIN138")
TEST_PIN139 = Function("TEST_PIN139")
TEST_PIN140 = Function("TEST_PIN140")
TEST_PIN141 = Function("TEST_PIN141")
TEST_PIN142 = Function("TEST_PIN142")
TEST_PIN143 = Function("TEST_PIN143")
TEST_PIN144 = Function("TEST_PIN144")
TEST_PIN145 = Function("TEST_PIN145")
TEST_PIN146 = Function("TEST_PIN146")
TEST_PIN147 = Function("TEST_PIN147")
TEST_PIN148 = Function("TEST_PIN148")
TEST_PIN149 = Function("TEST_PIN149")
TEST_PIN150 = Function("TEST_PIN150")
TEST_PIN151 = Function("TEST_PIN151")
TEST_PIN152 = Function("TEST_PIN152")
TEST_PIN153 = Function("TEST_PIN153")
TEST_PIN154 = Function("TEST_PIN154")
TEST_PIN155 = Function("TEST_PIN155")
TEST_PIN156 = Function("TEST_PIN156")
TEST_PIN157 = Function("TEST_PIN157")
TEST_PIN158 = Function("TEST_PIN158")
TEST_PIN159 = Function("TEST_PIN159")
TEST_PIN160 = Function("TEST_PIN160")
TEST_PIN161 = Function("TEST_PIN161")
TEST_PIN162 = Function("TEST_PIN162")
TEST_PIN163 = Function("TEST_PIN163")
TEST_PIN164 = Function("TEST_PIN164")
TEST_PIN165 = Function("TEST_PIN165")
TEST_PIN166 = Function("TEST_PIN166")
TEST_PIN167 = Function("TEST_PIN167")
TEST_PIN168 = Function("TEST_PIN168")
TEST_PIN169 = Function("TEST_PIN169")
TEST_PIN170 = Function("TEST_PIN170")
TEST_PIN171 = Function("TEST_PIN171")
TEST_PIN172 = Function("TEST_PIN172")
TEST_PIN173 = Function("TEST_PIN173")
TEST_PIN174 = Function("TEST_PIN174")
TEST_PIN175 = Function("TEST_PIN175")
TEST_PIN176 = Function("TEST_PIN176")
TEST_PIN177 = Function("TEST_PIN177")
TEST_PIN178 = Function("TEST_PIN178")
TEST_PIN179 = Function("TEST_PIN179")
TEST_PIN180 = Function("TEST_PIN180")
TEST_PIN181 = Function("TEST_PIN181")
TEST_PIN182 = Function("TEST_PIN182")
TEST_PIN183 = Function("TEST_PIN183")
TEST_PIN184 = Function("TEST_PIN184")
TEST_PIN185 = Function("TEST_PIN185")
TEST_PIN186 = Function("TEST_PIN186")
TEST_PIN187 = Function("TEST_PIN187")
TEST_PIN188 = Function("TEST_PIN188")
TEST_PIN189 = Function("TEST_PIN189")
TEST_PIN190 = Function("TEST_PIN190")
TEST_PIN191 = Function("TEST_PIN191")
TEST_PIN192 = Function("TEST_PIN192")
TEST_PIN193 = Function("TEST_PIN193")
TEST_PIN194 = Function("TEST_PIN194")
TEST_PIN195 = Function("TEST_PIN195")
TEST_PIN196 = Function("TEST_PIN196")
TEST_PIN197 = Function("TEST_PIN197")
TEST_PIN198 = Function("TEST_PIN198")
TEST_PIN199 = Function("TEST_PIN199")
TEST_PIN200 = Function("TEST_PIN200")
TEST_PIN201 = Function("TEST_PIN201")
TEST_PIN202 = Function("TEST_PIN202")
TEST_PIN203 = Function("TEST_PIN203")
TEST_PIN204 = Function("TEST_PIN204")
TEST_PIN205 = Function("TEST_PIN205")
TEST_PIN206 = Function("TEST_PIN206")
TEST_PIN207 = Function("TEST_PIN207")
TEST_PIN208 = Function("TEST_PIN208")
TEST_PIN209 = Function("TEST_PIN209")
TEST_PIN210 = Function("TEST_PIN210")
TEST_PIN211 = Function("TEST_PIN211")
TEST_PIN212 = Function("TEST_PIN212")
TEST_PIN213 = Function("TEST_PIN213")
TEST_PIN214 = Function("TEST_PIN214")
TEST_PIN215 = Function("TEST_PIN215")
TEST_PIN216 = Function("TEST_PIN216")
TEST_PIN217 = Function("TEST_PIN217")
TEST_PIN218 = Function("TEST_PIN218")
TEST_PIN219 = Function("TEST_PIN219")
TEST_PIN220 = Function("TEST_PIN220")
TEST_PIN221 = Function("TEST_PIN221")
TEST_PIN222 = Function("TEST_PIN222")
TEST_PIN223 = Function("TEST_PIN223")
TEST_PIN224 = Function("TEST_PIN224")
TEST_PIN225 = Function("TEST_PIN225")
TEST_PIN226 = Function("TEST_PIN226")
TEST_PIN227 = Function("TEST_PIN227")
TEST_PIN228 = Function("TEST_PIN228")
TEST_PIN229 = Function("TEST_PIN229")
TEST_PIN230 = Function("TEST_PIN230")
TEST_PIN231 = Function("TEST_PIN231")
TEST_PIN232 = Function("TEST_PIN232")
TEST_PIN233 = Function("TEST_PIN233")
TEST_PIN234 = Function("TEST_PIN234")
TEST_PIN235 = Function("TEST_PIN235")
TEST_PIN236 = Function("TEST_PIN236")
TEST_PIN237 = Function("TEST_PIN237")
TEST_PIN238 = Function("TEST_PIN238")
TEST_PIN239 = Function("TEST_PIN239")
TEST_PIN240 = Function("TEST_PIN240")
TEST_PIN241 = Function("TEST_PIN241")
TEST_PIN242 = Function("TEST_PIN242")
TEST_PIN243 = Function("TEST_PIN243")
TEST_PIN244 = Function("TEST_PIN244")
TEST_PIN245 = Function("TEST_PIN245")
TEST_PIN246 = Function("TEST_PIN246")
TEST_PIN247 = Function("TEST_PIN247")
TEST_PIN248 = Function("TEST_PIN248")
TEST_PIN249 = Function("TEST_PIN249")
TEST_PIN250 = Function("TEST_PIN250")
TEST_PIN251 = Function("TEST_PIN251")
TEST_PIN252 = Function("TEST_PIN252")
TEST_PIN253 = Function("TEST_PIN253")
TEST_PIN254 = Function("TEST_PIN254")
TEST_PIN255 = Function("TEST_PIN255")

# Display interface functions
DI0 = Function("DI0")
DI1 = Function("DI1")
DI2 = Function("DI2")
DI3 = Function("DI3")
DO0 = Function("DO0")
DO1 = Function("DO1")
DO2 = Function("DO2")
DO3 = Function("DO3")
CO3 = Function("CO3")
CO2 = Function("CO2")
CO1 = Function("CO1")
CO0 = Function("CO0")
CI0 = Function("CI0")
CI1 = Function("CI1")
CI2 = Function("CI2")
CI3 = Function("CI3")
HSYNC0 = Function("HSYNC0")
VSYNC0 = Function("VSYNC0")
HSYNC1 = Function("HSYNC1")
VSYNC1 = Function("VSYNC1")

# PMUIOMUX functions
INT0 = Function("INT0")
INT1 = Function("INT1")
INT2 = Function("INT2")
INT3 = Function("INT3")
INT4 = Function("INT4")
INT5 = Function("INT5")
OUT1 = Function("OUT1")
OUT2 = Function("OUT2")

# SPI2AXI functions
SPI2AXI_CLK = Function("SPI2AXI_CLK")
SPI2AXI_CS = Function("SPI2AXI_CS")
SPI2AXI_DIN = Function("SPI2AXI_DIN")
SPI2AXI_DOUT = Function("SPI2AXI_DOUT")


def _build_func_to_pins_map() -> Dict[str, list]:
    """Build a reverse map from function names to (pin, io_sel) pairs."""
    func_map = {}
    # Get the full pin functions dictionary for controller 0
    all_pin_funcs = {
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
    for pin, pin_funcs in all_pin_funcs.items():
        for io_sel, func_name in pin_funcs.items():
            if func_name not in func_map:
                func_map[func_name] = []
            func_map[func_name].append((pin, io_sel))

    # Build lowercase map for case-insensitive lookup
    func_map_lower = {}
    for func_name, pins in func_map.items():
        func_map_lower[func_name.lower()] = pins

    return func_map_lower


FUNC_TO_PINS_MAP = _build_func_to_pins_map()


def get_function_number(func_name: Union[str, int, Function], pin: Optional[int] = None, iomux_idx: int = 0) -> Optional[int]:
    """Convert function name to IO_SEL number.

    Args:
        func_name: Function name (alt0, alt1, or specific like GPIO38, JTAG_TDI, UART0_TXD)
        pin: Pin number for specific function lookup
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        IO_SEL number (0-7) or None if invalid

    Example:
        >>> get_function_number("alt1")
        1
        >>> get_function_number("GPIO38")
        0
        >>> get_function_number("UART0_TXD", 38)
        1
        >>> get_function_number(UART0_TXD, 38)  # Use constant directly
        1
    """
    # Handle Function constant objects
    if isinstance(func_name, Function):
        func_name_str = func_name._name
        # If pin is specified, look up the function for that specific pin
        if pin is not None:
            pin_funcs = get_pin_functions(pin, iomux_idx)
            for io_sel, fn in pin_funcs.items():
                if fn == func_name_str:
                    return io_sel
        # If no pin specified, we can't determine the IO_SEL
        # Return None and let the caller handle the error
        return None

    if isinstance(func_name, int):
        if 0 <= func_name <= 7:
            return func_name
        return None

    func_name = func_name.lower().strip()

    # First try alt mapping
    if func_name in ALT_MAPPING:
        return ALT_MAPPING[func_name]

    # If pin is specified, check if func_name is a specific function for that pin
    if pin is not None:
        pin_funcs = get_pin_functions(pin, iomux_idx)
        for io_sel, fn in pin_funcs.items():
            if fn.lower() == func_name:
                return io_sel

    return None


def get_io_sel_name(io_sel: int, pin_funcs: Dict[int, str]) -> str:
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


def parse_pin_name(pin_name: str, iomux_idx: int = 0) -> Optional[int]:
    """Parse pin name like 'io38' to pin number.

    Args:
        pin_name: Pin name string (e.g., 'io38', 'IO38')
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Pin number (0-63) or None if invalid
    """
    controller = IOMUX_CONTROLLERS[iomux_idx]
    basename = controller["basename"]
    match = re.match(rf'^{basename}(\d+)$', pin_name.lower())
    if match:
        pin_num = int(match.group(1))
        num_pins = controller["pins"]
        if 0 <= pin_num < num_pins:
            return pin_num
    return None


def get_pin_name(pin: int, iomux_idx: int = 0) -> str:
    """Get the pin name string for specified controller.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Pin name string (e.g., 'io38')
    """
    controller = IOMUX_CONTROLLERS[iomux_idx]
    basename = controller["basename"]
    return f"{basename}{pin}"


def read_pin(pin: int, iomux_idx: int = 0) -> int:
    """Read a pin's register value.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Register value

    Raises:
        PermissionError: If not running as root
        RuntimeError: If /dev/mem is not accessible
    """
    mem_path = "/dev/mem"
    controller = IOMUX_CONTROLLERS[iomux_idx]
    offset_base = controller["base"]

    if not os.path.exists(mem_path):
        raise RuntimeError(f"{mem_path} does not exist")

    with open(mem_path, "r+b") as mem_file:
        page_size = os.sysconf("SC_PAGE_SIZE")
        base_aligned = offset_base & ~(page_size - 1)
        page_offset = offset_base - base_aligned
        map_size = max(page_size, page_offset + controller["size"])

        mm = mmap.mmap(
            mem_file.fileno(),
            map_size,
            mmap.MAP_SHARED,
            mmap.PROT_READ | mmap.PROT_WRITE,
            offset=base_aligned
        )

        try:
            offset = page_offset + pin * 4
            return int.from_bytes(mm[offset:offset + 4], byteorder='little')
        finally:
            mm.close()


def _get_register_info(pin: int, iomux_idx: int = 0) -> dict:
    """Get register address and offset for a pin."""
    controller = IOMUX_CONTROLLERS[iomux_idx]
    offset_base = controller["base"]
    page_size = os.sysconf("SC_PAGE_SIZE")

    base_aligned = offset_base & ~(page_size - 1)
    page_offset = offset_base - base_aligned
    reg_offset = pin * 4
    phys_addr = offset_base + reg_offset

    return {
        'phys_addr': phys_addr,
        'reg_offset': reg_offset,
        'page_offset': page_offset,
        'base_aligned': base_aligned,
        'map_size': max(page_size, page_offset + controller["size"]),
    }


def read_pin(pin: int, iomux_idx: int = 0) -> int:
    """Read a pin's register value.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Register value

    Raises:
        RuntimeError: If /dev/mem is not accessible
    """
    mem_path = "/dev/mem"
    if not os.path.exists(mem_path):
        raise RuntimeError(f"{mem_path} does not exist")

    info = _get_register_info(pin, iomux_idx)

    with open(mem_path, "r+b") as mem_file:
        mm = mmap.mmap(
            mem_file.fileno(),
            info['map_size'],
            mmap.MAP_SHARED,
            mmap.PROT_READ | mmap.PROT_WRITE,
            offset=info['base_aligned']
        )

        try:
            offset = info['page_offset'] + pin * 4
            return int.from_bytes(mm[offset:offset + 4], byteorder='little')
        finally:
            mm.close()


def write_pin(pin: int, value: int, iomux_idx: int = 0, confirm: bool = True) -> bool:
    """Write a pin's register value.

    Args:
        pin: Pin number
        value: Value to write
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)
        confirm: If True, prompt user for confirmation before writing

    Returns:
        True if written successfully

    Note:
        This function uses devmem to write directly to avoid mmap caching issues.
    """
    import subprocess
    info = _get_register_info(pin, iomux_idx)

    if confirm:
        current_value = read_pin(pin, iomux_idx)
        print(f"Pin:              {pin}")
        print(f"Physical address: 0x{info['phys_addr']:08x}")
        print(f"Value to write:   0x{value:08x}")
        print(f"Current value:    0x{current_value:08x}")
        print(f"Current status:   {get_pin_status(pin, iomux_idx)}")
        print(f"Register offset:  0x{info['reg_offset']:04x}")

        while True:
            user_input = input("Confirm write? [y/N]: ").strip().lower()
            if user_input in ['y', 'yes']:
                break
            elif user_input in ['', 'n', 'no']:
                print("Write cancelled by user")
                return False
            else:
                print("Please enter 'y' or 'n'")

    # Use devmem to write directly
    subprocess.run(['devmem', hex(info['phys_addr']), '32', hex(value)], capture_output=True)

    # Use devmem to read back for verification
    result = subprocess.run(['devmem', hex(info['phys_addr']), '32'], capture_output=True, text=True)
    verified = int(result.stdout.strip(), 0)
    print(f"After write read: 0x{verified:08x}")
    print(f"After status:     {get_pin_status(pin, iomux_idx)}")

    # Only verify writable bits (bits 13:0)
    if (verified & RW_BITS_MASK) != (value & RW_BITS_MASK):
        print(f"  Warning: Verification mismatch (expected: 0x{value:08x})")
        return False
    return True


def set_pin_function(pin: int, func: Union[str, int], iomux_idx: int = 0) -> bool:
    """Set a pin's function.

    Args:
        pin: Pin number
        func: Function to set (alt0-alt7 or function name like 'UART0_TXD')
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        True if successful

    Example:
        >>> set_pin_function(38, 'alt1')  # Set to UART0_TXD
        >>> set_pin_function(38, ALT1)
        >>> set_pin_function(38, 'UART0_TXD')
    """
    func_num = get_function_number(func, pin, iomux_idx)
    if func_num is None:
        raise ValueError(f"Invalid function '{func}' for pin {pin}")

    # Read current value
    reg_value = read_pin(pin, iomux_idx)

    # Clear IO_SEL bits and set new function
    new_value = reg_value & ~(IO_SEL_MASK << IO_SEL_SHIFT)
    new_value |= (func_num << IO_SEL_SHIFT)

    # Get function names for display
    pin_funcs = get_pin_functions(pin, iomux_idx)
    current_func = get_io_sel_name((reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK, pin_funcs)
    new_func = get_io_sel_name(func_num, pin_funcs)

    pin_name_str = get_pin_name(pin, iomux_idx)
    print(f"Setting {pin_name_str}:")
    print(f"  Current:  {current_func}")
    print(f"  New:      {new_func}")
    print(f"  Status:   {get_pin_status(pin, iomux_idx)}")
    print(f"  Register: 0x{reg_value:08x} -> 0x{new_value:08x}")

    return write_pin(pin, new_value, iomux_idx, confirm=True)


def set_pin_config(
    pin: int,
    iomux_idx: int = 0,
    ie: Optional[bool] = None,
    oe: Optional[bool] = None,
    pu: Optional[bool] = None,
    pd: Optional[bool] = None,
    ds: Optional[int] = None,
    sl: Optional[bool] = None,
    st: Optional[bool] = None,
) -> bool:
    """Configure pin properties.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)
        ie: Enable input (True/False)
        oe: Enable output (True/False)
        pu: Enable pull-up (True/False)
        pd: Enable pull-down (True/False)
        ds: Drive strength (1-15)
        sl: Enable slew rate (True/False)
        st: Enable schmitt trigger (True/False)

    Returns:
        True if successful
    """
    reg_value = read_pin(pin, iomux_idx)
    new_value = reg_value

    # Handle input enable
    if ie is not None:
        if ie:
            new_value &= ~(IE_MASK << IE_SHIFT)
            new_value |= (1 << IE_SHIFT)
        else:
            new_value &= ~(IE_MASK << IE_SHIFT)

    # Handle output enable
    if oe is not None:
        if oe:
            new_value &= ~(OE_MASK << OE_SHIFT)
            new_value |= (1 << OE_SHIFT)
        else:
            new_value &= ~(OE_MASK << OE_SHIFT)

    # Handle pull up/down
    if pu is not None:
        if pu:
            new_value &= ~(PU_MASK << PU_SHIFT)
            new_value |= (1 << PU_SHIFT)
        else:
            new_value &= ~(PU_MASK << PU_SHIFT)

    if pd is not None:
        if pd:
            new_value &= ~(PD_MASK << PD_SHIFT)
            new_value |= (1 << PD_SHIFT)
        else:
            new_value &= ~(PD_MASK << PD_SHIFT)

    # Handle drive strength
    if ds is not None:
        if not (1 <= ds <= 15):
            raise ValueError("Drive strength must be between 1 and 15")
        new_value &= ~(DS_MASK << DS_SHIFT)
        new_value |= ((ds & DS_MASK) << DS_SHIFT)

    # Handle slew rate
    if sl is not None:
        if sl:
            new_value &= ~(SL_MASK << SL_SHIFT)
            new_value |= (1 << SL_SHIFT)
        else:
            new_value &= ~(SL_MASK << SL_SHIFT)

    # Handle schmitt trigger
    if st is not None:
        if st:
            new_value &= ~(ST_MASK << ST_SHIFT)
            new_value |= (1 << ST_SHIFT)
        else:
            new_value &= ~(ST_MASK << ST_SHIFT)

    if new_value == reg_value:
        print("No changes to apply")
        return True

    pin_name_str = get_pin_name(pin, iomux_idx)
    print(f"Configuring {pin_name_str}:")
    print(f"  Status:   {get_pin_status(pin, iomux_idx)}")
    print(f"  Register: 0x{reg_value:08x} -> 0x{new_value:08x}")

    return write_pin(pin, new_value, iomux_idx, confirm=True)


def get_pin_status(pin: int, iomux_idx: int = 0) -> str:
    """Get the current function and status of a pin.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        String with function and status info (e.g., 'alt0 (GPIO38), input, pull-up')
    """
    reg_value = read_pin(pin, iomux_idx)
    pin_funcs = get_pin_functions(pin, iomux_idx)
    io_sel = (reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK
    func_name = get_io_sel_name(io_sel, pin_funcs)

    # Extract status bits
    is_ie = (reg_value >> IE_SHIFT) & IE_MASK  # Input enable
    is_oe = (reg_value >> OE_SHIFT) & OE_MASK  # Output enable
    is_pu = (reg_value >> PU_SHIFT) & PU_MASK  # Pull up
    is_pd = (reg_value >> PD_SHIFT) & PD_MASK  # Pull down
    ds = (reg_value >> DS_SHIFT) & DS_MASK     # Drive strength
    is_sl = (reg_value >> SL_SHIFT) & SL_MASK  # Slew rate
    is_st = (reg_value >> ST_SHIFT) & ST_MASK  # Schmitt trigger

    # Build status string
    status_parts = []

    # Input/Output
    if is_ie and not is_oe:
        status_parts.append("ie")
    elif is_oe and not is_ie:
        status_parts.append("oe")
    elif is_ie and is_oe:
        status_parts.append("ie oe")

    # Pull up/down
    if is_pu:
        status_parts.append("pu")
    elif is_pd:
        status_parts.append("pd")

    # Drive strength
    status_parts.append(f"ds={ds}")

    # Slew rate and Schmitt trigger
    if is_sl:
        status_parts.append("sl")
    if is_st:
        status_parts.append("st")

    return f"{func_name}, {', '.join(status_parts)}"


def get_current_function(pin: int, iomux_idx: int = 0) -> str:
    """Get the current function of a pin.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Current function name (e.g., 'alt0 (GPIO38)')
    """
    reg_value = read_pin(pin, iomux_idx)
    pin_funcs = get_pin_functions(pin, iomux_idx)
    io_sel = (reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK
    return get_io_sel_name(io_sel, pin_funcs)


def get_all_pins_info(iomux_idx: int = 0) -> Dict[str, tuple]:
    """Get information for all pins.

    Args:
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        Dictionary mapping pin names to (current_function, all_functions) tuples
    """
    controller = IOMUX_CONTROLLERS[iomux_idx]
    result = {}

    for pin in range(controller["pins"]):
        pin_name = get_pin_name(pin, iomux_idx)
        current_func = get_current_function(pin, iomux_idx)
        all_funcs = get_pin_functions(pin, iomux_idx)
        result[pin_name] = (current_func, all_funcs)

    return result


def get_pin_status(pin: int, iomux_idx: int = 0) -> str:
    """Get the current function and status of a pin.

    Args:
        pin: Pin number
        iomux_idx: IOMUX controller index (0=main, 1=pmuiomux)

    Returns:
        String with function and status info (e.g., 'alt0 (GPIO38), input, pull-up')
    """
    reg_value = read_pin(pin, iomux_idx)
    pin_funcs = get_pin_functions(pin, iomux_idx)
    io_sel = (reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK
    func_name = get_io_sel_name(io_sel, pin_funcs)

    # Extract status bits
    is_ie = (reg_value >> IE_SHIFT) & IE_MASK  # Input enable
    is_oe = (reg_value >> OE_SHIFT) & OE_MASK  # Output enable
    is_pu = (reg_value >> PU_SHIFT) & PU_MASK  # Pull up
    is_pd = (reg_value >> PD_SHIFT) & PD_MASK  # Pull down
    ds = (reg_value >> DS_SHIFT) & DS_MASK     # Drive strength
    is_sl = (reg_value >> SL_SHIFT) & SL_MASK  # Slew rate
    is_st = (reg_value >> ST_SHIFT) & ST_MASK  # Schmitt trigger

    # Build status string
    status_parts = []

    # Input/Output
    if is_ie and not is_oe:
        status_parts.append("input")
    elif is_oe and not is_ie:
        status_parts.append("output")
    elif is_ie and is_oe:
        status_parts.append("bidirectional")
    else:
        status_parts.append("no input/output")

    # Pull up/down
    if is_pu:
        status_parts.append("pull-up")
    elif is_pd:
        status_parts.append("pull-down")

    # Drive strength
    status_parts.append(f"ds={ds}")

    # Slew rate and Schmitt trigger
    if is_sl:
        status_parts.append("slew")
    if is_st:
        status_parts.append("schmitt")

    return f"{func_name}, {', '.join(status_parts)}"


def print_help():
    """Print help message for command-line usage."""
    help_text = """
K230 IOMUX Pin Configuration Tool
Usage: python iomux.py [OPTIONS] <COMMAND>

Commands:
  --info [all|<pin>|<func>]   Show pin information
                              - 'all'     : Show all pins information
                              - <pin>     : Show specific pin (e.g., io38, 38)
                              - <func>    : Show pins using this function (e.g., UART0_TXD)

  --set <pin> --func <func>   Set pin function
                              [--ie] [--no-ie]     Enable/disable input
                              [--oe] [--no-oe]     Enable/disable output
                              [--pu] [--no-pu]     Enable/disable pull-up
                              [--pd] [--no-pd]     Enable/disable pull-down
                              [--ds <val>]         Drive strength (1-15)
                              [--sl] [--no-sl]     Enable/disable slew rate
                              [--st] [--no-st]     Enable/disable schmitt trigger

Options:
  -c, --controller <idx>    IOMUX controller index (0=main, 1=pmuiomux) [default: 0]
  -h, --help                Show this help message

Examples:
  python iomux.py -c 0 --info all              # Show all pins
  python iomux.py -c 0 --info io38             # Show pin 38 info
  python iomux.py -c 0 --info UART0_TXD        # Show pins using UART0_TXD

  python iomux.py -c 0 --set io38 --func alt1              # Set to alt1
  python iomux.py -c 0 --set io38 --func UART0_TXD         # Set to UART0_TXD
  python iomux.py -c 0 --set io38 --func alt1 --ie --pu    # Set with config
  python iomux.py -c 0 --set io38 --func alt1 --ds 8       # Set drive strength

Function Names:
  - alt0, alt1, ... alt7     # Alternative function selection
  - GPIO0-71                 # GPIO functions
  - UART0_TXD, UART0_RXD...  # UART functions
  - I2C0_SCL, I2C0_SDA...    # I2C functions
  - PWM0-PWM5                # PWM functions
  - And more...

Note: This tool requires root privileges to access /dev/mem
"""
    print(help_text.strip())


def parse_pin_arg(pin_arg: str, iomux_idx: int) -> Optional[int]:
    """Parse pin argument that could be 'io38', '38'.

    Returns the pin number if valid, or None if it's a function name or invalid.
    """
    # Try as pin name first (io38)
    pin = parse_pin_name(pin_arg, iomux_idx)
    if pin is not None:
        return pin

    # Try as pin number
    try:
        pin = int(pin_arg)
        controller = IOMUX_CONTROLLERS[iomux_idx]
        if 0 <= pin < controller["pins"]:
            return pin
    except ValueError:
        pass

    # Check if it's a function name - don't raise, let caller handle
    func_lower = pin_arg.lower()
    if func_lower in FUNC_TO_PINS_MAP:
        return None  # It's a function name, not a pin

    return None  # Invalid


def main():
    """Main entry point for command-line usage."""
    import sys
    import argparse

    parser = argparse.ArgumentParser(
        prog='iomux',
        description='K230 IOMUX Pin Configuration Tool',
        add_help=False
    )
    parser.add_argument('-c', '--controller', type=int, default=0, choices=[0, 1],
                        help='IOMUX controller index')
    parser.add_argument('-h', '--help', action='store_true', help='Show help')

    # Primary actions
    parser.add_argument('--info', nargs='?', const='all', metavar='PIN_OR_FUNC',
                        help='Show pin information')
    parser.add_argument('--set', nargs='?', metavar='PIN',
                        help='Set pin function (requires --func)')

    # Set options
    parser.add_argument('--func', dest='func', help='Function to set (alt0-alt7 or function name)')
    parser.add_argument('--ie', action='store_true', dest='ie', help='Enable input')
    parser.add_argument('--no-ie', action='store_true', dest='no_ie', help='Disable input')
    parser.add_argument('--oe', action='store_true', dest='oe', help='Enable output')
    parser.add_argument('--no-oe', action='store_true', dest='no_oe', help='Disable output')
    parser.add_argument('--pu', action='store_true', dest='pu', help='Enable pull-up')
    parser.add_argument('--no-pu', action='store_true', dest='no_pu', help='Disable pull-up')
    parser.add_argument('--pd', action='store_true', dest='pd', help='Enable pull-down')
    parser.add_argument('--no-pd', action='store_true', dest='no_pd', help='Disable pull-down')
    parser.add_argument('--ds', type=int, dest='ds', help='Drive strength (1-15)')
    parser.add_argument('--no-ds', action='store_true', dest='no_ds', help='Reset drive strength')
    parser.add_argument('--sl', action='store_true', dest='sl', help='Enable slew rate')
    parser.add_argument('--no-sl', action='store_true', dest='no_sl', help='Disable slew rate')
    parser.add_argument('--st', action='store_true', dest='st', help='Enable schmitt trigger')
    parser.add_argument('--no-st', action='store_true', dest='no_st', help='Disable schmitt trigger')

    # Handle no arguments
    if len(sys.argv) == 1:
        print_help()
        return

    args = parser.parse_args()

    try:
        # Show help
        if args.help:
            print_help()
            return

        # --info command
        if args.info is not None:
            if args.info.lower() == 'all':
                print("All pins information:")
                info = get_all_pins_info(args.controller)
                for pin_name, (current_func, all_funcs) in info.items():
                    status = get_pin_status(pin, args.controller)
                    func_list = ", ".join(f"alt{io_sel}:{func_name}" for io_sel, func_name in sorted(all_funcs.items()))
                    print(f"  {pin_name}: {status}")
                    print(f"      [{func_list}]")
            else:
                # First check if it's a function name (case-insensitive)
                func_lower = args.info.lower()
                if func_lower in FUNC_TO_PINS_MAP:
                    # It's a function name - show all pins using this function
                    print(f"Pins can be {args.info}:")
                    for pin, io_sel in FUNC_TO_PINS_MAP[func_lower]:
                        # Check if this pin is in the specified controller
                        if pin < IOMUX_CONTROLLERS[args.controller]["pins"]:
                            status = get_pin_status(pin, args.controller)
                            print(f"  {get_pin_name(pin, args.controller)}: {status}")
                    return

                # Not a function name, try as pin
                try:
                    pin = parse_pin_arg(args.info, args.controller)
                    status = get_pin_status(pin, args.controller)
                    print(f"Pin {get_pin_name(pin, args.controller)}:")
                    print(f"  {status}")
                    funcs = get_pin_functions(pin, args.controller)
                    if funcs:
                        print("  Available functions:")
                        for io_sel, func_name in sorted(funcs.items()):
                            print(f"    alt{io_sel}: {func_name}")
                except ValueError as e:
                    print(f"Error: {e}", file=sys.stderr)
                    sys.exit(1)
            return

        # --set command
        if args.set is not None:
            if not args.func:
                print("Error: --func is required for --set", file=sys.stderr)
                sys.exit(1)

            pin = parse_pin_arg(args.set, args.controller)
            if pin is None:
                # Check if it's a function name
                func_lower = args.set.lower()
                if func_lower in FUNC_TO_PINS_MAP:
                    print(f"Error: '{args.set}' is a function name, not a pin. Use it with --func.", file=sys.stderr)
                else:
                    print(f"Error: Invalid pin '{args.set}'", file=sys.stderr)
                sys.exit(1)

            # Check if --func is a valid function
            try:
                func_num = get_function_number(args.func, pin, args.controller)
                if func_num is None:
                    print(f"Error: Invalid function '{args.func}' for pin {pin}", file=sys.stderr)
                    sys.exit(1)
            except ValueError as e:
                print(f"Error: {e}", file=sys.stderr)
                sys.exit(1)

            # Read current value
            reg_value = read_pin(pin, args.controller)
            pin_funcs = get_pin_functions(pin, args.controller)
            current_func = get_io_sel_name((reg_value >> IO_SEL_SHIFT) & IO_SEL_MASK, pin_funcs)

            # Calculate new value
            new_value = reg_value

            # Set function
            new_value &= ~(IO_SEL_MASK << IO_SEL_SHIFT)
            new_value |= (func_num << IO_SEL_SHIFT)

            # Handle config options
            def clear_bit(val, shift):
                return val & ~(1 << shift)

            def set_bit(val, shift):
                return val | (1 << shift)

            if args.no_ie:
                new_value = clear_bit(new_value, IE_SHIFT)
            if args.ie:
                new_value = set_bit(new_value, IE_SHIFT)

            if args.no_oe:
                new_value = clear_bit(new_value, OE_SHIFT)
            if args.oe:
                new_value = set_bit(new_value, OE_SHIFT)

            if args.no_pu:
                new_value = clear_bit(new_value, PU_SHIFT)
            if args.pu:
                new_value = set_bit(new_value, PU_SHIFT)

            if args.no_pd:
                new_value = clear_bit(new_value, PD_SHIFT)
            if args.pd:
                new_value = set_bit(new_value, PD_SHIFT)

            if args.no_ds:
                new_value &= ~(DS_MASK << DS_SHIFT)
            if args.ds is not None:
                if not (1 <= args.ds <= 15):
                    print("Error: Drive strength must be between 1 and 15", file=sys.stderr)
                    sys.exit(1)
                new_value &= ~(DS_MASK << DS_SHIFT)
                new_value |= ((args.ds & DS_MASK) << DS_SHIFT)

            if args.no_sl:
                new_value = clear_bit(new_value, SL_SHIFT)
            if args.sl:
                new_value = set_bit(new_value, SL_SHIFT)

            if args.no_st:
                new_value = clear_bit(new_value, ST_SHIFT)
            if args.st:
                new_value = set_bit(new_value, ST_SHIFT)

            new_func = get_io_sel_name(func_num, pin_funcs)

            pin_name_str = get_pin_name(pin, args.controller)
            print(f"Setting {pin_name_str}:")
            print(f"  Current:  {current_func}")
            print(f"  New:      {new_func}")
            print(f"  Status:   {get_pin_status(pin, args.controller)}")
            print(f"  Register: 0x{reg_value:08x} -> 0x{new_value:08x}")

            write_pin(pin, new_value, args.controller, confirm=True)
            return

        # If no action specified
        print_help()

    except PermissionError:
        print("Error: Permission denied. Please run as root.", file=sys.stderr)
        sys.exit(1)
    except RuntimeError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nCancelled by user")
        sys.exit(0)


if __name__ == "__main__":
    main()
