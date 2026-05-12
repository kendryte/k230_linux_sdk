'''
实验名称：按键
实验平台：核桃派
'''

#导入相关模块
import board
from digitalio import DigitalInOut, Direction, Pull

#构建LED对象和初始化
led = DigitalInOut(board.LED) #定义引脚编号
led.direction = Direction.OUTPUT  #IO为输出

#构建按键对象和初始化
key = DigitalInOut(board.KEY) #定义引脚编号
key.direction = Direction.INPUT #IO为输入
key.pull = Pull.UP #开启上拉电阻

while True:

    if key.value == 0: #按键被按下
        led.value = 1 #点亮LED

    else: #松开
        led.value = 0 #关闭LED
