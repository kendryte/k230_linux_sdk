'''
实验名称：UART(串口通讯)
实验平台：核桃派
'''

#导入相关模块
import serial,time

# 配置串口
com = serial.Serial("/dev/ttyS3", 115200)

#发送提示字符
com.write(b'Hello WalnutPi!')

while True:

    # 获得接收缓冲区字符个数 int
    count = com.inWaiting()

    if count != 0: #收到数据

        # 读取内容并打印
        recv = com.read(count)
        print(recv)

        #发回数据
        com.write(recv)

        # 清空接收缓冲区
        com.flushInput()

    # 延时100ms,接收间隔
    time.sleep(0.1)
