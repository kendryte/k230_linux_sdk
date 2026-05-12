#!/usr/bin/env python3
"""
K230 IOMUX Test Script

This script demonstrates how to use the iomux module to configure pins.
"""

import sys
from k230 import iomux


def test_get_pin_functions():
    """Test get_pin_functions - get all available functions for a pin."""
    print("=== Test get_pin_functions ===")
    funcs = iomux.get_pin_functions(38)
    print(f"Pin 38 functions: {funcs}")
    # Expected: {0: 'GPIO38', 1: 'UART0_TXD', 2: 'TEST_PIN8', 3: 'QSPI1_CS0', 4: 'HSYNC0'}


def test_get_current_function():
    """Test get_current_function - get current function of a pin."""
    print("\n=== Test get_current_function ===")
    # Note: This requires root privileges and actual hardware
    try:
        func = iomux.get_current_function(38)
        print(f"Pin 38 current function: {func}")
    except Exception as e:
        print(f"Error reading pin 38: {e}")


def test_set_pin_function():
    """Test set_pin_function - set a pin to a specific function."""
    print("\n=== Test set_pin_function ===")
    # Set pin 38 to UART0_TXD (alt1)
    try:
        success = iomux.set_pin_function(38, 'alt1')
        print(f"Set pin 38 to alt1: {'SUCCESS' if success else 'FAILED'}")
    except Exception as e:
        print(f"Error setting pin 38: {e}")


def test_set_pin_config():
    """Test set_pin_config - configure pin properties."""
    print("\n=== Test set_pin_config ===")
    try:
        # Enable input, output, pull-up, drive strength 8
        success = iomux.set_pin_config(38, ie=True, oe=True, pu=True, ds=8)
        print(f"Config pin 38: {'SUCCESS' if success else 'FAILED'}")
    except Exception as e:
        print(f"Error configuring pin 38: {e}")


def test_get_pin_functions_by_name():
    """Test using function name constants."""
    print("\n=== Test using function name constants ===")
    funcs = iomux.get_pin_functions(38)
    print(f"Pin 38 available functions:")
    for io_sel, func_name in sorted(funcs.items()):
        print(f"  alt{io_sel}: {func_name}")


def test_parse_pin_name():
    """Test parse_pin_name - parse pin name string."""
    print("\n=== Test parse_pin_name ===")
    print(f"parse_pin_name('io38'): {iomux.parse_pin_name('io38')}")
    print(f"parse_pin_name('IO38'): {iomux.parse_pin_name('IO38')}")
    print(f"parse_pin_name('38'): {iomux.parse_pin_name('38')}")


def test_all_pins_info():
    """Test get_all_pins_info - get information for all pins."""
    print("\n=== Test get_all_pins_info (partial) ===")
    try:
        all_info = iomux.get_all_pins_info()
        count = 0
        for pin_name, func in list(all_info.items())[:10]:  # Show first 10
            print(f"  {pin_name}: {func}")
            count += 1
        print(f"... and {len(all_info) - count} more pins")
    except Exception as e:
        print(f"Error getting pins info: {e}")


def test_alt_constants():
    """Test using ALT constants."""
    print("\n=== Test ALT constants ===")
    print(f"ALT0 = {iomux.ALT0}")
    print(f"ALT1 = {iomux.ALT1}")
    print(f"ALT2 = {iomux.ALT2}")


def test_function_constants():
    """Test using Function constants like UART0_TXD, GPIO38, etc."""
    print("\n=== Test Function constants ===")
    print(f"UART0_TXD = {iomux.UART0_TXD}")
    print(f"GPIO38 = {iomux.GPIO38}")
    print(f"I2C0_SCL = {iomux.I2C0_SCL}")


def main():
    """Run all tests."""
    print("K230 IOMUX Module Test")
    print("=" * 50)

    test_get_pin_functions()
    test_get_pin_functions_by_name()
    test_parse_pin_name()
    test_alt_constants()
    test_function_constants()
    test_all_pins_info()

    # The following tests require actual hardware access
    print("\n" + "=" * 50)
    print("Hardware access tests (may require root):")
    print("=" * 50)
    test_get_current_function()
    test_set_pin_function()
    test_set_pin_config()


if __name__ == "__main__":
    main()
