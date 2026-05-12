'''
实验名称：点亮第1个LED
实验平台：核桃派
'''

#导入相关模块
import board
from digitalio import DigitalInOut, Direction
from time import sleep

#构建LED对象和初始化
led = DigitalInOut(board.LED)
led.direction = Direction.OUTPUT

#循环点亮熄灭LED
while True:
    led.value = 1 #输出高电平，点亮板载LED蓝灯
    sleep(1)
    led.value = 0 #输出低电平，熄灭板载LED蓝灯
    sleep(1)
