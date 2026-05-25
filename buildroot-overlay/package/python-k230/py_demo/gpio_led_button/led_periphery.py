from periphery import GPIO
import time

# Modify the chip path and pin number according to your K230 or other board
# Assuming LED is connected to line 20 of /dev/gpiochip1
LED_CHIP_PATH = "/dev/gpiochip1"
LED_LINE_NUM = 20

try:
    # Open GPIO and configure as output mode ("out")
    # New version of python-periphery no longer supports initial_value parameter
    led = GPIO(LED_CHIP_PATH, LED_LINE_NUM, "out")

    # Manually set initial value to low (LED off)
    led.write(False)

    print(f"Successfully initialized LED pin: {LED_CHIP_PATH} [Line {LED_LINE_NUM}]")
    print("Starting LED blink, press Ctrl+C to exit...")

    while True:
        # Output high level (True), turn on LED
        # Note: If your hardware is active-low, write(True) will turn it off,
        # and write(False) will turn it on
        print("LED ON")
        led.write(True)
        time.sleep(0.9)  # Keep on for 0.5 seconds

        # Output low level (False), turn off LED
        print("LED OFF")
        led.write(False)
        time.sleep(0.9)  # Keep off for 0.5 seconds

except KeyboardInterrupt:
    print("\nProgram terminated.")
except Exception as e:
    print(f"Error occurred: {e}")
finally:
    # Release GPIO resource properly and turn off LED before exiting
    if 'led' in locals():
        led.write(False)
        led.close()
        print("LED resource released.")
