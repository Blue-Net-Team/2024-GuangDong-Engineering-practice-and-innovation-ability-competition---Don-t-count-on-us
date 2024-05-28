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
import math
import numpy as np
from UART import UART
import detector
import cv2
import threading
from img_trans import VideoStreaming
    

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
qr = ()
color = detector.ColorDetector()
line = detector.LineDetector()

# 颜色识别器面积初始化
color.minarea = 0
color.maxarea = 100000

# 创建串口对象
ser = UART()


def correcttion():
    """校准小车角度"""
    while True:
        _img = cap.read()[1]
        angle = line.get_angle(_img)
        if angle is not None:
            if abs(angle) > 3:
                #TODO:测试什么情况角度小于90，然后发送正确的信号
                if angle < 90:
                    ser.send(2)
                elif angle > 90:
                    ser.send(3)

# 创建校准线程
t = threading.Thread(target=correcttion)
t.start()

class Solution:
    def __init__(self):
        pass

    def streaming(self):
        self.stream = VideoStreaming('10.0.0.3', 8000)
        self.stream.connecting()
        self.stream.start()
        while True:
            if self.img is not None:
                self.stream.send(self.img)


    def __call__(self):
        # 打开图传线程
        trans = threading.Thread(target=self.streaming)
        trans.start()
        # TODO: 等待新的二维码模块到了之后补充二维码识别逻辑

        while True:
            if ser.read() != b'\n':      # 电控发送start信号，开始第一次识别颜色
                for i in qr_msg:
                    # i:“1”为红色，“2”为绿色，“3”为蓝色
                    index = i-1
                    # region 设置颜色阈值
                    color.low_h = thresholds[index][0][0]
                    color.low_s = thresholds[index][0][1]
                    color.low_v = thresholds[index][0][2]

                    color.high_h = thresholds[index][1][0]
                    color.high_s = thresholds[index][1][1]
                    color.high_v = thresholds[index][1][2]

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
                                # TODO:拉高电平
                                
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