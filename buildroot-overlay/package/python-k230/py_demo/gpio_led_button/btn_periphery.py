from periphery import GPIO

try:
    # 1. Open GPIO and configure: input with pull-up
    button = GPIO("/dev/gpiochip0", 21, "in", bias="pull_up")

    # 2. [Core Magic]: Let Linux kernel handle debouncing, set debounce time to 20ms (20000us)
    # Note: periphery's debounce parameter unit is typically microseconds (us)
    button.debounce = 20000

    # 3. Listen for both edges
    button.edge = "both"

    print("Starting button monitoring with kernel debouncing...")

    while True:
        # 4. Block waiting for clean interrupts filtered by kernel
        if button.poll(timeout=None):
            # 5. Read event to reset the flag
            try:
                _ = button.read_event()
            except Exception:
                pass

            # 6. Read and print, no more bounce issues
            val = button.read()
            print(">>> Button Pressed <<<" if val == 0 else "=== Button Released ===")

except KeyboardInterrupt:
    print("\nProgram terminated.")
except Exception as e:
    print(f"Error: {e}")
finally:
    if 'button' in locals():
        button.close()
