"""在本地电脑运行"""
import socket
import cv2
import numpy as np
import main


#TODO: 完善阈值表, 0红 1绿 2蓝
thresholds = [
    #[low_h, low_s, low_v], [high_h, high_s, high_v]
    ([0, 0, 0], [0, 0, 0]),
    ([0, 0, 0], [0, 0, 0]),
    ([0, 0, 0], [0, 0, 0]),
]

class ReceiveImg(object):
    def __init__(self, host, port):
        """初始化
        * host: 树莓派的IP地址
        * port: 端口号，与树莓派设置的端口号一致"""
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)					# 设置创建socket服务的Client客服务的参数
        self.client_socket.connect((host, port))												# 连接的主机IP地址和端口
        self.connection = self.client_socket.makefile('rb')										# 创建一个makefile传输文件，读功能，读数据是b''二进制类型
        # need bytes here
        self.stream_bytes = b' '											# 创建一个变量，存放的数据类型是b''二进制类型数据
        
        print(" ")
        print("已连接到服务端：")
        print("Host : ", host)
        print("请按‘q’退出图像传输!")

    def read(self):
        try:
            msg = self.connection.read(1024)						# 读makefile传输文件，一次读1024个字节
            self.stream_bytes += msg
            first = self.stream_bytes.find(b'\xff\xd8')					# 检测帧头位置
            last = self.stream_bytes.find(b'\xff\xd9')					# 检测帧尾位置

            if first != -1 and last != -1:
                jpg = self.stream_bytes[first:last + 2]					# 帧头和帧尾中间的数据就是二进制图片数据（编码后的二进制图片数据，需要解码后使用）
                self.stream_bytes = self.stream_bytes[last + 2:]				# 更新stream_bytes数据
                image = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)			# 将二进制图片数据转换成numpy.uint8格式（也就是图片格式）数据，然后解码获得图片数据
                return True, image
            return False, None

        except:
            print("Error：连接出错！")
            return False, None


class DEBUG(main.Solution):
    def __init__(self, iftrans:bool=True, capid:int=0) -> None:
        super().init_part1()

        if iftrans:
            # 图传
            self.reveiver = ReceiveImg('10.0.0.3', 8000)
        else:
            self.reveiver = cv2.VideoCapture(capid)


    def SetColorThresholds(self):
        self.color()
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环

            cv2.circle(self.img, self.circle_point, self.circle_r, (0, 255, 0), 2)
            mask = self.color.filter(self.img)

            cv2.imshow('img', self.img)

            if mask is None: continue
            cv2.imshow('mask', mask)

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break
        
    def SetLineThresholds(self):
        """设置直线识别相关阈值"""
        self.line()
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环

            angle = self.line.get_angle(self.img)
            distance = self.line.get_distance(self.img)

            print(f'angle:{angle}, distance:{distance}')
            cv2.imshow('img', self.img)

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break

if __name__ == '__main__':
    debug = DEBUG()
    # region 阈值调试
    debug.SetColorThresholds()
    # debug.SetLineThresholds()
    # endregion

    # region 功能测试
    """ while True:
        _, debug.img = debug.reveiver.read()
        if debug.img is None:continue        # 如果没有读取到图像数据，继续循环
        # res = debug.CORRECTION_angle()            # 角度校准
        # res = debug.CORRECTION_distance()       # 直线识别
        # res = debug.DETECTCOLOR()         # 颜色识别
        res = debug.LOCATECOLOR(0)          # 色环定位

        if res is not None:
            print(res)
        cv2.imshow('img', debug.img)
        if cv2.waitKey(1) == 27:        # 按下ESC键退出
            break """
    # endregion