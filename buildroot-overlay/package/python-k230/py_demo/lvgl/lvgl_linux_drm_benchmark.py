#!/usr/bin/env python3
"""
LVGL Benchmark Demo for K230

Runs the built-in LVGL benchmark to measure rendering performance.

Usage:
    python3 lvgl_demo_benchmark.py
"""

import os
import time
import lvgl as lv


def main():
    # ---- Initialization ----
    lv.k230_init()
    lv.demo_benchmark()

    # ---- Main loop ----
    print("LVGL benchmark running... Press Ctrl+C to exit")
    try:
        while True:
            idle_time = lv.timer_handler()
            time.sleep(idle_time / 1000.0)
    except KeyboardInterrupt:
        print("\nExiting...")
        os._exit(0)


if __name__ == "__main__":
    main()
