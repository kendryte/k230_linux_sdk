'''
Experiment: Button Controls LED
Platform: 01studio
'''

import board
import time
from digitalio import DigitalInOut, Direction, Pull

# Initialize LED
led = DigitalInOut(board.LED)  # Define pin number
led.direction = Direction.OUTPUT  # Set as output

# Initialize button
key = DigitalInOut(board.KEY)  # Define pin number
key.direction = Direction.INPUT  # Set as input
key.pull = Pull.UP  # Enable pull-up resistor

while True:
    if key.value == 0:  # Button pressed
        led.value = 1  # Turn on LED
    else:  # Button released
        led.value = 0  # Turn off LED

    time.sleep(0.2)
