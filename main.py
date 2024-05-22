r"""
发送信号1：抓取
发送信号2：向左偏
发送信号3：向右偏

*********************************************
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '&lt;  `.___\_&lt;|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='

			佛祖保佑       工创省一
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
"""
import io
import math
import socket
import struct
import time
import numpy as np
from UART import UART
import detector
import cv2
import threading


class VideoStreaming(object):
    def __init__(self, host, port):

        self.server_socket = socket.socket()			# 获取socket.socket()实例
        self.server_socket.bind((host, port))			# 绑定主机IP地址和端口号
        self.server_socket.listen(5)					# 设置监听数量

    def connecting(self):
        """连接Client"""
        # print('等待连接')
        self.connection, self.client_address = self.server_socket.accept()		# 等待Client连接，返回实例和IP地址
        self.connect = self.connection.makefile('wb')							# 创建一个传输文件 写功能 写入数据时b''二进制类型数据
        self.host_name = socket.gethostname()									# 获得服务端主机名
        self.host_ip = socket.gethostbyname(self.host_name)						# 获得服务端主机IP地址
        # print('连接成功')
    
    def start(self) -> None:
        """开始传输视频流"""
        print("Client Host Name:", self.host_name)
        print("Connection from: ", self.client_address)
        print("Streaming...")
        self.stream = io.BytesIO()							# 创建一个io流，用于存放二进制数据

    def send(self, _img) -> None:
        """发送图像数据
        ----
        * _img: 传入的图像数据"""
        while True:
            try:
                try:
                    img_encode = cv2.imencode('.jpg', _img)[1]						# 编码
                except:
                    print('没有读取到图像')
                    continue
                data_encode = np.array(img_encode)								    # 将编码数据转换成二进制数据
                self.stream.write(data_encode)										# 将二进制数据存放到io流
                self.connect.write(struct.pack('<L', self.stream.tell()))			# struct.pack()将数据转换成什么格式    stream.tell()获得目前指针的位置，将数据写入io流后，数据指针跟着后移，
                                                                                    # 也就是将数据长度转换成'<L'类型（无符号长整型），写入makefile传输文件
                                                                                    # 它的作用相当于 帧头数据，单独收到这个数据表示开始传输一帧图片数据，因为图片大小确定，这个数也就定下不变
                self.connect.flush()											    # 刷新，将数据长度发送出去
                self.stream.seek(0)													# 更新io流，将指针指向0
                self.connect.write(self.stream.read())								# 指针指向0后，从头开始读数据，然后写到makefile传输文件
                self.stream.seek(0)													# 更新指针
                self.stream.truncate()												# 更新io流数据，删除指针后面的数据

                self.connect.write(struct.pack('<L', 0))						    # 发送0，相当于帧尾数据，单独收到这个数表示一帧图片传输结束
            except ConnectionResetError:
                print('连接已重置')
                self.connecting()
            

#TODO: 完善阈值表, 0红 1绿 2蓝
thresholds = {
            #[low_h, low_s, low_v], [high_h, high_s, high_v]
    'red': ([0, 0, 0], [0, 0, 0]),
    'green': ([0, 0, 0], [0, 0, 0]),
    'blue': ([0, 0, 0], [0, 0, 0]),
}

# 打开摄像头
cap = cv2.VideoCapture(0)

# 创建识别器对象
qr = detector.QRDetector()
color = detector.ColorDetector()
line = detector.LineDetector()

# 颜色识别器面积初始化
color.minarea = 0
color.maxarea = 100000

# 创建串口对象
ser = UART()


def correcttion():
    """校准小车角度"""
    #TODO: 补充代表小车自身的直线 的两点坐标
    p1 = (0, 0)
    p2 = (0, 0)

    while True:
        _img = cap.read()[1]
        angle = line.draw_and_get_angle_difference(_img, p1, p2)
        if angle is not None:
            if abs(angle) > 3:
                #TODO:测试什么情况角度是负数，然后发送正确的信号
                if angle < 0:
                    ser.send(2)
                elif angle > 0:
                    ser.send(3)

# 创建校准线程
t = threading.Thread(target=correcttion)
t.start()

class Solution:
    def __init__(self):
        pass

    def streaming(self):
        self.stream = VideoStreaming('10.0.0.2', 8000)
        self.stream.connecting()
        self.stream.start()
        while True:
            if self.img is not None:
                self.stream.send(self.img)


    def __call__(self):
        # 打开图传线程
        trans = threading.Thread(target=self.streaming)
        trans.start()

        while True:
            #识别二维码
            self.img = cap.read()[1]
            qr_msg = qr(self.img)
            if qr_msg is not None:
                qr_msg = map(str, qr_msg.split())       # ['red', 'green', 'blue']有序的颜色信息
                break

        while True:
            if ser.read() != b'\n':      # 电控发送start信号，开始第一次识别颜色
                for i in qr_msg:
                    # region 设置颜色阈值
                    color.low_h = thresholds[i][0][0]
                    color.low_s = thresholds[i][0][1]
                    color.low_v = thresholds[i][0][2]

                    color.high_h = thresholds[i][1][0]
                    color.high_s = thresholds[i][1][1]
                    color.high_v = thresholds[i][1][2]

                    # endregion

                    while True:
                        # 读取摄像头数据
                        ret, self.img = cap.read()

                        #TODO: 在图像中找到物料应该在的正确位置，用圆圈圈出来
                        center1, radius1  = (0, 0), 0
                        eara0 = radius1 ** 2 * math.pi
                        cv2.circle(self.img, center1, radius1, (255, 255, 0), 1)

                        # 二值化滤波
                        img_color = color.filter(self.img)

                        img_color, cycle = color.draw_cyclic(img_color)

                        # 在原图上画出识别到的颜色
                        cv2.circle(self.img, cycle[0], cycle[1], (0, 0, 255), 2)

                        if cycle:       # 识别到颜色
                            # 计算重叠面积
                            eara = circle_intersection_area(center1[0], center1[1], radius1, cycle[0][0], cycle[0][1], cycle[1])
                            if eara/eara > 0.8:     # 重叠面积大于85%
                                ser.send(1)     # 发送信号，做出抓取动作
                                break


def circle_intersection_area(x0, y0, r0, x1, y1, r1):
    """计算两个圆的重叠面积"""
    d = math.sqrt((x1 - x0) ** 2 + (y1 - y0) ** 2)

    if d <= abs(r0 - r1):
        # 一个圆在另一个圆内部
        return math.pi * min(r0, r1) ** 2

    if d >= r0 + r1:
        # 两个圆没有交集
        return 0

    # 计算重叠面积
    r0_sq = r0 ** 2
    r1_sq = r1 ** 2
    alpha = math.acos((d ** 2 + r0_sq - r1_sq) / (2 * d * r0))
    beta = math.acos((d ** 2 + r1_sq - r0_sq) / (2 * d * r1))
    return r0_sq * alpha + r1_sq * beta - 0.5 * (r0_sq * math.sin(2 * alpha) + r1_sq * math.sin(2 * beta))


if __name__ == '__main__':
    Solution()