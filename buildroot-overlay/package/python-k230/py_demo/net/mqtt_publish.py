'''
实验名称：MQTT通信
实验平台：核桃派2B
说明：编程实现MQTT通信:发布者（publish）。
'''

#导入相关库
import paho.mqtt.client as mqtt
import time

#服务器和主题信息
host = 'mq.tongxinmao.com'
port = 18830
topic = '/public/walnutpi/1'

#构建mqtt客户端对象
client = mqtt.Client()

#发起连接
client.connect(host,port)

while True:

    #发布信息
    client.publish(topic,'Hello WalnutPi!')

    time.sleep(2) #延时1秒，发送间隔
