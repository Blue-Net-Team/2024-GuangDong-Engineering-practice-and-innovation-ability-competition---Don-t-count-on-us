r"""
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
import time
import numpy as np
from UART import UART
import detector
import cv2
import threading
from img_trans import VideoStreaming
import RPi.GPIO as GPIO
    
start_pin = 18      # 启动电平引脚
sign_pin = 17       # 信号引脚
c_pin = 27          # 校准引脚

GPIO.setmode(GPIO.BCM)
GPIO.setup(start_pin, GPIO.IN)
GPIO.setup(sign_pin, GPIO.OUT)
GPIO.setup(c_pin, GPIO.IN)

#TODO: 完善阈值表, 0红 1绿 2蓝
thresholds = [
    #[low_h, low_s, low_v], [high_h, high_s, high_v]
    ([0, 0, 0], [0, 0, 0]),
    ([0, 0, 0], [0, 0, 0]),
    ([0, 0, 0], [0, 0, 0]),
] 


class Solution:
    def __init__(self):
        # 打开读取图像线程
        img = threading.Thread(target=self.get_img)
        img.start()

        # 创建识别器对象
        self.color = detector.ColorDetector()
        self.line = detector.LineDetector()

        # 颜色识别器面积初始化
        self.color.minarea = 0
        self.color.maxarea = 100000

        # 创建串口对象，self.ser继承了二维码识别的功能
        self.ser = UART()
        self.cap = cv2.VideoCapture(0)

        # TODO:测试夹爪的圆心坐标,半径
        self.circle_point = (100, 100)
        self.circle_r = 50
        pass


    def correcttion(self):
        """校准小车角度"""
        while GPIO.input(c_pin):
            _img = self.cap.read()[1]
            angle = self.line.get_angle(_img)
            if angle is not None:
                if abs(angle) > 3:
                    self.ser.send(int(angle))



    def streaming(self):
        self.stream = VideoStreaming('10.0.0.3', 8000)
        self.stream.connecting()
        self.stream.start()
        while True:
            if self.img is not None:
                self.stream.send(self.img)


    def get_img(self):
        while True:
            ret, self.img = self.cap.read()
            cv2.circle(self.img, self.circle_point, self.circle_r, (0, 0, 255), 2)
            if ret:
                break


    def __call__(self):
        # 创建校准线程
        c = threading.Thread(target=self.correcttion)
        c.start()

        # 打开图传线程
        trans = threading.Thread(target=self.streaming)
        trans.start()

        # 二维码识别
        while True:
            qr_lst = self.ser.qr_detect()
            if qr_lst is not None:
                break

        for msg in qr_lst:
            # region 第一次到原料区拿取物料
            """定位色环 <---> 做出动作"""
            for index in msg:
                while True:
                    if GPIO.input(start_pin) :      # 检测到启动信号
                        # region 阈值表初始化
                        self.color.low_h, self.color.low_s, self.color.low_v = thresholds[int(index)-1][0]
                        self.color.high_h, self.color.high_s, self.color.high_v = thresholds[int(index)-1][1]
                        # endregion

                        mask = self.color.filter(self.img)
                        res, p = self.color.draw_cyclic(mask)

                        overlap_area = circle_intersection_area(self.circle_point[0], self.circle_point[1], self.circle_r,
                                                                p[0][0], p[0][1], p[1])
                        area = np.pi * self.circle_r ** 2

                        if overlap_area/area > 0.95:
                            # 重叠面积大于95%，拉高17引脚电平0.5s
                            GPIO.output(sign_pin, 1)
                            time.sleep(0.5)
                            GPIO.output(sign_pin, 0)
                            break
                    else:continue
            # endregion
                    
            # region 将物料放到粗加工区 无序
            """
            定位色环 -> 停车信号 
                ^           |
                |           v
            运动   <-  做出动作
            """
            for i in range(3):
                while True:
                    if GPIO.input(start_pin) :      # 检测到启动信号
                        # region 阈值表初始化
                        self.color.low_h, self.color.low_s, self.color.low_v = thresholds[i][0]
                        self.color.high_h, self.color.high_s, self.color.high_v = thresholds[i][1]
                        # endregion

                        mask = self.color.filter(self.img)
                        res, p = self.color.draw_cyclic(mask)

                        overlap_area = circle_intersection_area(self.circle_point[0], self.circle_point[1], self.circle_r,
                                                                p[0][0], p[0][1], p[1])
                        area = np.pi * self.circle_r ** 2

                        if overlap_area/area > 0.95:
                            # 重叠面积大于95%，拉高17引脚电平0.5s
                            GPIO.output(sign_pin, 1)
                            time.sleep(0.5)
                            GPIO.output(sign_pin, 0)
                            break
                    else:continue
            # endregion
                    
            # region 将物料从粗加工取出 有序
            for index in msg:
                while True:
                    if GPIO.input(start_pin) :      # 检测到启动信号
                        # region 阈值表初始化
                        self.color.low_h, self.color.low_s, self.color.low_v = thresholds[int(index)-1][0]
                        self.color.high_h, self.color.high_s, self.color.high_v = thresholds[int(index)-1][1]
                        # endregion

                        mask = self.color.filter(self.img)
                        res, p = self.color.draw_cyclic(mask)

                        overlap_area = circle_intersection_area(self.circle_point[0], self.circle_point[1], self.circle_r,
                                                                p[0][0], p[0][1], p[1])
                        area = np.pi * self.circle_r ** 2

                        if overlap_area/area > 0.95:
                            # 重叠面积大于95%，拉高17引脚电平0.5s
                            GPIO.output(sign_pin, 1)
                            time.sleep(0.5)
                            GPIO.output(sign_pin, 0)
                            break
                    else:continue
            # endregion
                    
            # region 将物料放到暂存区 有序
            # TODO:添加代码
            # endregion



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
    Solution()()