'''
Experiment: LED Blink
Platform: 01studio
'''

# Import modules
import board
from digitalio import DigitalInOut, Direction
from time import sleep

# Initialize LED
led = DigitalInOut(board.LED)
led.direction = Direction.OUTPUT

# Blink LED in loop
while True:
    led.value = 1  # Output high level, turn on built-in blue LED
    sleep(1)
    led.value = 0  # Output low level, turn off built-in blue LED
    sleep(1)
