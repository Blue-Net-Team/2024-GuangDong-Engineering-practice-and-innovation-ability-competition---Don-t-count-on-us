from typing import Iterable
import serial
import struct


class UART(serial.Serial):
    """串口类，串口号: /dev/ttyAMA0, 波特率: 9600
    ====
    继承了pyserial库的串口类，封装了
    * send_pack_int: 发送整形的包头包尾修饰器
    * send_pack_str: 发送字符串的包头包尾修饰器
    * send_arr: 发送整形数组数组
    * send: 发送整形数据
    * write: 发送字符串数据
    * read: 读取字符串数据"""
    def __init__(self):
        super().__init__('/dev/ttyAMA0', 9600)

    @staticmethod
    def send_pack_int(func):
        """包头包尾修饰器
        * 包头：0xff
        * 包尾：0xfe"""
        HEAD = 255
        TAIL = 254
        def wrapper(self, *args, **kwargs):
            super().write(HEAD.to_bytes(1, 'big'))     # 包头
            func(self, *args, **kwargs)
            super().write(TAIL.to_bytes(1, 'big'))     # 包尾
        return wrapper

    @staticmethod
    def send_pack_str(func):
        r"""包头包尾修饰器
        * 包头：@
        * 包尾：#"""
        def wrapper(self, *args, **kwargs):
            super().write(b'@')     # 包头
            func(self, *args, **kwargs)
            super().write(b'#')     # 包尾
        return wrapper
    
    @staticmethod
    def send_pack_arr(func):
        r"""包头包尾修饰器
        * 包头：@
        * 包尾：#"""
        def wrapper(self, *args, **kwargs):
            super().write(b'Q')     # 包头
            func(self, *args, **kwargs)
            super().write(b'E')     # 包尾
        return wrapper
    
    @send_pack_arr
    def send_arr(self, args:list[int]|tuple[int, ...]):
        """发送数组,包含包头包尾数据"""
        for index, i in enumerate(args):
            msg = str(abs(i))  # 取绝对值，因为符号会单独处理
            while len(msg) < 3:
                msg = '0' + msg

            if i >= 0:
                msg = '1' + msg
            else:
                msg = '0' + msg
                    
            self.write_with_no_pack(msg)

    @send_pack_str
    def send(self, data:int):
        """发送整型数据,包含包头包尾"""
        newdata = struct.pack('>i', data)
        super().write(newdata)
    
    @send_pack_str
    def write(self, data:str) -> int | None:
        """发送字符串数据,包含包头包尾"""
        return super().write(data.encode('ascii'))
    
    def write_with_no_pack(self, data:str) -> int | None:
        """发送字符串数据,不包含包头包尾"""
        return super().write(data.encode('ascii'))
    
    def read(self, ifdecode=True, head=b'@', tail=b'#'):
        """读取字符串数据,包头包尾只能是一个字节
        * ifdecode: 是否用ascii码解码数据包
        * head: 包头
        * tail: 包尾
        返回读取到的已解码数据"""
        PACKET_HEAD = head
        PACKET_TAIL = tail

        data = b''  # 用于存储接收到的数据

        while True:
            byte = super().read()
            if byte == PACKET_HEAD:
                data = b''
                continue
            if byte == PACKET_TAIL:
                break
            data += byte
        if ifdecode:
            res = data.decode('ascii')
        else:
            res = data
        return res if res else None
    
    def ori_read(self, size:int):
        return super().read(size)
    
    def __del__(self) -> None:
        return self.close()


if __name__ == '__main__':
    ser = UART()
    # ser.send(123)
    ser.send_arr((-68, 27))
    # ser.write('123')
    # print(ser.read())
    # del ser