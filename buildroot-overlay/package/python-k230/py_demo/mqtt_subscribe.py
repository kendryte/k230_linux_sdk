'''
实验名称：MQTT通信
实验平台：核桃派2B
说明：编程实现MQTT通信: 订阅者（subscribe）。
'''

#导入相关库
import paho.mqtt.client as mqtt
import time

#服务器和主题信息
host = 'mq.tongxinmao.com'
port = 18830
topic = '/public/walnutpi/1'

#客户端从服务器接收到1个CONNACK响应时执行的回调函数.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # 在on_connect()中使使用订阅主题意味着如果我们失去连接并重新连接，订阅将被更新。
    client.subscribe(topic)

# 当从服务器中收到其他设备的发布信息时，执行这个回调函数.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

#构建MQTT客户端
client = mqtt.Client()

#配置连接和接收信息的回调函数
client.on_connect = on_connect
client.on_message = on_message

#连接服务器
client.connect(host, port)

# 处理网络流量、分派回调和处理重新连接的阻塞调用。
client.loop_forever()
