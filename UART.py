import serial
import struct

from detector import QRdetector


class UART(QRdetector):
    def __init__(self):
        super().__init__()

    @staticmethod
    def pack_int(func):
        """包头包尾修饰器
        * 包头：0xff
        * 包尾：0xfe"""
        head = 255
        tail = 254
        def wrapper(self, *args, **kwargs):
            super().write(head.to_bytes(1, 'big'))     # 包头
            func(self, *args, **kwargs)
            super().write(tail.to_bytes(1, 'big'))     # 包尾
        return wrapper

    @staticmethod
    def pack_str(func):
        """包头包尾修饰器
        * 包头：@
        * 包尾：\r\n"""
        def wrapper(self, *args, **kwargs):
            super().write(b'@')     # 包头
            func(self, *args, **kwargs)
            super().write(b'\r\n')     # 包尾
        return wrapper
    
    @pack_int
    def send_arr(self, args:list):
        """发送数组"""
        for i in args:
            data = struct.pack('<i', i)     # 发送四个字节，端小字节序
            print(f'发送数据：{i}',end='')
            super().write(data)
        print()

    @pack_int
    def send(self, data:int):
        """发送整型数据"""
        newdata = struct.pack('<i', data)
        super().write(newdata)
    
    @pack_str
    def write(self, data:str) -> int | None:
        return super().write(data.encode('ascii'))
    
    def __del__(self) -> None:
        return self.close()
