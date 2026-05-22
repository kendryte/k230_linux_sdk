#!/usr/bin/env python3
"""
K230 PWM Python Example - Basic Control

Usage:
    python pwm_basic.py              # Default: chip=0, channel=0, 1kHz, 50%
    python pwm_basic.py -c 0 -C 2    # chip=0, channel=2
    python pwm_basic.py -f 10000     # 10kHz
    python pwm_basic.py -d 25        # 25% duty cycle
    python pwm_basic.py --help       # Show help

Options:
  -h, --help            show this help message and exit
  -c CHIP, --chip CHIP  PWM chip number (0 or 3), default: 0
  -C CHANNEL, --channel CHANNEL
                        PWM channel number (0, 1, 2), default: 2
  -f FREQUENCY, --frequency FREQUENCY
                        Frequency in Hz, default: 1000
  -d DUTY, --duty DUTY  Duty cycle percentage (0-100), default: 50
  -p POLARITY, --polarity POLARITY
                        Polarity (normal/inversed), default: inversed
"""

import os
import sys
import time
import argparse

SYSFS_PWM_DIR = "/sys/class/pwm"

class K230PWM:
    def __init__(self, chip=0, channel=0):
        """
        Initialize PWM controller

        Args:
            chip: PWM chip number (0 or 3)
            channel: PWM channel number (0, 1, 2)
        """
        self.chip = chip
        self.channel = channel
        self.pwm_path = f"{SYSFS_PWM_DIR}/pwmchip{chip}/pwm{channel}"
        self.exported = False

    def export(self):
        """Export PWM channel"""
        export_path = f"{SYSFS_PWM_DIR}/pwmchip{self.chip}/export"
        if not os.path.exists(self.pwm_path):
            with open(export_path, 'w') as f:
                f.write(str(self.channel))
            self.exported = True
            time.sleep(0.1)  # Wait for device creation

    def unexport(self):
        """Unexport PWM channel"""
        if self.exported:
            unexport_path = f"{SYSFS_PWM_DIR}/pwmchip{self.chip}/unexport"
            with open(unexport_path, 'w') as f:
                f.write(str(self.channel))
            self.exported = False

    def set_polarity(self, polarity="inversed"):
        """
        Set PWM polarity

        Args:
            polarity: "normal" or "inversed"
        """
        # Note: K230 PWM only supports inversed polarity
        with open(f"{self.pwm_path}/polarity", 'w') as f:
            f.write(polarity)

    def set_period(self, period_ns):
        """
        Set PWM period (nanoseconds)

        Args:
            period_ns: Period length in nanoseconds
        """
        with open(f"{self.pwm_path}/period", 'w') as f:
            f.write(str(period_ns))

    def set_duty_cycle(self, duty_ns):
        """
        Set PWM duty cycle (nanoseconds)

        Args:
            duty_ns: Duty cycle time in nanoseconds
        """
        with open(f"{self.pwm_path}/duty_cycle", 'w') as f:
            f.write(str(duty_ns))

    def set_frequency(self, frequency_hz):
        """
        Set PWM frequency (Hz)

        Args:
            frequency_hz: Frequency in Hz
        """
        period_ns = int(1000000000 / frequency_hz)
        self.set_period(period_ns)

    def set_duty_percent(self, duty_percent):
        """
        Set PWM duty cycle percentage

        Args:
            duty_percent: Duty cycle percentage (0-100)
        """
        # Read current period
        with open(f"{self.pwm_path}/period", 'r') as f:
            period_ns = int(f.read().strip())

        duty_ns = int(period_ns * duty_percent / 100)
        self.set_duty_cycle(duty_ns)

    def enable(self):
        """Enable PWM"""
        with open(f"{self.pwm_path}/enable", 'w') as f:
            f.write("1")

    def disable(self):
        """Disable PWM"""
        with open(f"{self.pwm_path}/enable", 'w') as f:
            f.write("0")

    def get_state(self):
        """Get current state"""
        state = {}
        try:
            with open(f"{self.pwm_path}/period", 'r') as f:
                state['period'] = int(f.read().strip())
            with open(f"{self.pwm_path}/duty_cycle", 'r') as f:
                state['duty_cycle'] = int(f.read().strip())
            with open(f"{self.pwm_path}/polarity", 'r') as f:
                state['polarity'] = f.read().strip()
            with open(f"{self.pwm_path}/enable", 'r') as f:
                state['enabled'] = bool(int(f.read().strip()))
        except Exception as e:
            state['error'] = str(e)
        return state

    def __enter__(self):
        self.export()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disable()
        self.unexport()


def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description="K230 PWM Python Test Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python pwm_basic.py              # Default: chip=0, channel=0, 1kHz, 50%
  python pwm_basic.py -c 0 -C 2  -f 10000  -d 25  # chip=0, channel=2 10kHz 25% duty
  python pwm_basic.py -f 10000     # 10kHz
  python pwm_basic.py -d 25        # 25% duty cycle
        """
    )
    parser.add_argument('-c', '--chip', type=int, default=0,
                        help='PWM chip number (0 or 3), default: 0')
    parser.add_argument('-C', '--channel', type=int, default=2,
                        help='PWM channel number (0, 1, 2), default: 2')
    parser.add_argument('-f', '--frequency', type=int, default=1000,
                        help='Frequency in Hz, default: 1000')
    parser.add_argument('-d', '--duty', type=float, default=50,
                        help='Duty cycle percentage (0-100), default: 50')
    parser.add_argument('-p', '--polarity', type=str, default='inversed',
                        choices=['normal', 'inversed'],
                        help='Polarity (normal/inversed), default: inversed')
    return parser.parse_args()


# Usage example
def main():
    args = parse_args()

    print("K230 PWM Python Test")
    print("====================\n")
    print(f"Configuration:")
    print(f"  Chip: {args.chip}")
    print(f"  Channel: {args.channel}")
    print(f"  Frequency: {args.frequency} Hz")
    print(f"  Duty: {args.duty}%")
    print(f"  Polarity: {args.polarity}")
    print()

    # Example 1: PWM with configured parameters
    print("Example 1: PWM with configured parameters")
    with K230PWM(chip=args.chip, channel=args.channel) as pwm:
        pwm.enable()
        pwm.set_frequency(args.frequency)
        pwm.set_duty_percent(args.duty)
        print(f"State: {pwm.get_state()}")
        print("Running... Press Ctrl+C to stop")
        while True:
            time.sleep(1)
        #time.sleep(5)


if __name__ == "__main__":
    main()
