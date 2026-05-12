import pwmio
import board

pwm = pwmio.PWMOut(board.PWM1, frequency=50)
pwm.duty_cycle = 2 ** 15  # Cycles the pin with 50% duty cycle (half of 2 ** 16) at 50hz

# https://docs.circuitpython.org/en/latest/shared-bindings/pwmio/index.html#module-pwmio
