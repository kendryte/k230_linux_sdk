import serial
import sys
import threading


class K230UART:
    """K230 UART 类"""

    def __init__(self, port, baudrate=115200, timeout=1):
        """
        初始化 UART

        Args:
            port: 串口设备路径，例如 /dev/ttyS0
            baudrate: 波特率，默认 115200
            timeout: 超时时间（秒）
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial = None

    def open(self):
        """打开串口"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=self.timeout,
                xonxoff=False,
                rtscts=False,
                dsrdtr=False
            )
            print(f"UART opened: {self.port} @ {self.baudrate}")
            return True
        except serial.SerialException as e:
            print(f"Failed to open UART: {e}")
            return False

    def close(self):
        """关闭串口"""
        if self.serial and self.serial.is_open:
            self.serial.close()
            print(f"UART closed: {self.port}")

    def write(self, data):
        """
        写入数据

        Args:
            data: 字符串或字节数组

        Returns:
            实际写入的字节数
        """
        if isinstance(data, str):
            data = data.encode('utf-8')
        return self.serial.write(data)

    def read(self, size=1):
        """
        读取数据

        Args:
            size: 要读取的字节数

        Returns:
            读取的字节数组
        """
        return self.serial.read(size)

    def read_all(self):
        """读取所有可用数据"""
        return self.serial.read_all()

    def read_until(self, terminator=b'\n'):
        """读取直到遇到终止符"""
        return self.serial.read_until(terminator)


def uart_receiver(uart, stop_event):
    """串口接收线程函数"""
    while not stop_event.is_set():
        response = uart.read_all()
        if response:
            print(f"\n[RX] {response.decode('utf-8', errors='ignore').strip()}")


def main():
    """主函数示例"""
    port = "/dev/ttyS3"

    # 处理命令行参数
    if len(sys.argv) > 1:
        port = sys.argv[1]

    print("K230 UART Python Example")
    print("Press Ctrl+C to exit\n")

    # 创建 UART 实例
    uart = K230UART(port, baudrate=115200)

    # 打开串口
    if not uart.open():
        sys.exit(1)

    try:
        # 启动接收线程
        stop_event = threading.Event()
        receiver_thread = threading.Thread(target=uart_receiver, args=(uart, stop_event), daemon=True)
        receiver_thread.start()

        print("Type your message and press Enter to send")
        while True:
            message = input("Send> ") + "\n"
            uart.write(message)
            print(f"[TX] {message.strip()}")

    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        stop_event.set()
        uart.close()


if __name__ == "__main__":
    main()
