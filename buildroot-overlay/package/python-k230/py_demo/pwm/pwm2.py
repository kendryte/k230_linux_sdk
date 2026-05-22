#!/usr/bin/env python3
"""
K230 PWM Motor Speed Control Example - Console Version
"""

import argparse
import os
import sys
import time


class MotorController:
    def __init__(self, chip=0, channel=0, frequency=2000, min_duty=10, max_duty=100):
        self.chip = chip
        self.channel = channel
        self.frequency = frequency
        self.min_duty = min_duty
        self.max_duty = max_duty
        self.running = False
        self.duty = 0
        self.period_ns = int(1e9 / frequency)

        # Export PWM
        self._export()

    def _unexport(self):
        """Unexport PWM channel"""
        unexport_path = f"/sys/class/pwm/pwmchip{self.chip}/unexport"
        if os.path.exists(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}"):
            with open(unexport_path, 'w') as f:
                f.write(str(self.channel))

    def _export(self):
        self._unexport()  # Clean up first
        time.sleep(0.1)

        export_path = f"/sys/class/pwm/pwmchip{self.chip}/export"
        with open(export_path, 'w') as f:
            f.write(str(self.channel))

        # Configure - set period, duty, polarity
        with open(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}/period", 'w') as f:
            f.write(str(self.period_ns))
        with open(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}/duty_cycle", 'w') as f:
            f.write(str(int(self.period_ns * self.min_duty / 100)))
        with open(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}/polarity", 'w') as f:
            f.write("inversed")

    def set_speed(self, percent):
        """Set motor speed (0-100%)"""
        if percent < 0:
            percent = 0
        if percent > 100:
            percent = 100

        # Calculate duty cycle with dead zone
        # Map 0-100% input to min_duty-max_duty actual range
        duty = self.min_duty + (self.max_duty - self.min_duty) * percent / 100
        duty_ns = int(self.period_ns * duty / 100)

        with open(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}/duty_cycle", 'w') as f:
            f.write(str(duty_ns))

        self.duty = duty
        return duty

    def start(self):
        """Start motor"""
        with open(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}/enable", 'w') as f:
            f.write("1")

    def stop(self):
        """Stop motor"""
        with open(f"/sys/class/pwm/pwmchip{self.chip}/pwm{self.channel}/enable", 'w') as f:
            f.write("0")

    def ramp_up(self, target_percent, duration=2):
        """Ramp up speed"""
        steps = 20
        interval = duration / steps
        current = 0

        for _ in range(steps):
            if current >= target_percent:
                break
            current = min(current + (target_percent - current) / 2, target_percent)
            self.set_speed(current)
            time.sleep(interval)

        self.set_speed(target_percent)

    def ramp_down(self, target_percent, duration=2):
        """Ramp down speed"""
        steps = 20
        interval = duration / steps
        current = self.duty if self.duty > 0 else 100

        for _ in range(steps):
            if current <= target_percent:
                break
            current = max(current - (current - target_percent) / 2, target_percent)
            self.set_speed(current)
            time.sleep(interval)

        self.set_speed(target_percent)

    def cleanup(self):
        """Cleanup"""
        self.stop()
        unexport_path = f"/sys/class/pwm/pwmchip{self.chip}/unexport"
        with open(unexport_path, 'w') as f:
            f.write(str(self.channel))


# Usage example
def main():
    parser = argparse.ArgumentParser(
        description="K230 Motor Speed Control Example",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python pwm2.py              # Default: chip=0, channel=2, freq=2000Hz
  python pwm2.py -c 0 -C 2    # Use chip0 channel2
  python pwm2.py --min-duty 5 --max-duty 100  # Wider speed range
  python pwm2.py --help       # Show help

Configuration:
  Frequency: 2000 Hz (2kHz) default, max ~2200Hz
  Duty range: 10-100% (adjustable via --min-duty and --max-duty)
        """
    )
    parser.add_argument('-c', '--chip', type=int, default=0,
                        help='PWM chip number (0 or 3), default: 0')
    parser.add_argument('-C', '--channel', type=int, default=2,
                        help='PWM channel number (0, 1, 2), default: 2')
    parser.add_argument('-f', '--frequency', type=int, default=50,
                        help='PWM frequency in Hz, default: 50')
    parser.add_argument('--min-duty', type=float, default=0,
                        help='Minimum duty cycle %% (start speed), default: 0')
    parser.add_argument('--max-duty', type=float, default=100,
                        help='Maximum duty cycle %% (max speed), default: 100')
    args = parser.parse_args()

    print("K230 Motor Control Example")
    print("==========================\n")
    print(f"Configuration: chip={args.chip}, channel={args.channel}, frequency={args.frequency}Hz\n")
    print(f"Duty range: {args.min_duty}% - {args.max_duty}%\n")

    motor = MotorController(chip=args.chip, channel=args.channel, frequency=args.frequency,
                           min_duty=args.min_duty, max_duty=args.max_duty)

    try:
        print("Starting motor...")
        # Start with min_duty (10%)
        motor.start()
        time.sleep(1)

        print("Accelerating to 100%...")
        motor.ramp_up(100, duration=1)
        time.sleep(1)

        print("Decelerating to 0%...")
        motor.ramp_down(0, duration=1)
        time.sleep(1)

        print("Stopping...")
        motor.stop()

    finally:
        motor.cleanup()
        print("Cleanup done.")


if __name__ == "__main__":
    main()
