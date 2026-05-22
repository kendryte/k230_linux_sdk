import pwmio
import board
import time




pwm = pwmio.PWMOut(board.pin.PWM2, frequency=50)
pwm.duty_cycle = 2 ** 15  # Cycles the pin with 50% duty cycle (half of 2 ** 16) at 50hz
while True:
    time.sleep(1)

# https://docs.circuitpython.org/en/latest/shared-bindings/pwmio/index.html#module-pwmio
