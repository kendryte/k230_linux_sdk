'''
Experiment: Button Event Detection
Platform: 01studio
'''

import board
import time
from digitalio import DigitalInOut, Direction, Pull

# Initialize button
key = DigitalInOut(board.KEY)  # Define pin number
key.direction = Direction.INPUT  # Set as input
key.pull = Pull.UP  # Enable pull-up resistor

# Store previous state for edge detection
prev_state = 1

print("Button event detection started. Press Ctrl+C to exit.")

while True:
    current_state = key.value

    # Detect falling edge (pressed)
    if prev_state == 1 and current_state == 0:
        print("[EVENT] Button pressed!")

    # Detect rising edge (released)
    elif prev_state == 0 and current_state == 1:
        print("[EVENT] Button released!")

    prev_state = current_state
    time.sleep(0.2)  # 10ms sleep to reduce CPU usage
