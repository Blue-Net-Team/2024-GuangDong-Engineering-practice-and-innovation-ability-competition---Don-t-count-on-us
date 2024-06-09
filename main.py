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
from doctest import debug
import math
from typing import Iterable
import numpy as np
from UART import UART
import detector
import cv2
import threading
from img_trans import VideoStreaming
import RPi.GPIO as GPIO     # type: ignore

# TODO: 约定信号引脚
start_pin = 18      # 启动电平引脚
sign_pin = 17       # 信号引脚
c_pin = 27          # 角度校准引脚

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
COLOR_dict = {
    0:'R',
    1:'G',
    2:'B'
}
COLOR_dict_reverse = {
    'R':0,
    'G':1,
    'B':2
}


class Solution:
    def __init__(self, ifdebug:bool=False):
        self.debug = ifdebug
        # TODO:测试夹爪的圆心坐标,半径
        self.circle_point = (100, 100)
        self.circle_r = 50

        # 打开读取图像线程
        threading.Thread(target=self.get_img).start()

        if debug:
            # 打开远程图传线程
            threading.Thread(target=self.streaming).start()

        # 创建识别器对象
        self.color = detector.ColorDetector()
        self.line = detector.LineDetector()

        # 颜色识别器面积初始化
        self.color.minarea = 0
        self.color.maxarea = 100000

        # 创建串口对象，self.ser继承了二维码识别的功能
        self.ser = UART()
        self.cap = cv2.VideoCapture(0)

        self.mask = None
        pass

    def streaming(self):
        """远程图传"""
        if self.debug:
            self.stream = VideoStreaming('10.0.0.3', 8000)
            self.stream.connecting()
            self.stream.start()
            while True:
                if self.img is not None:
                    self.stream.send(self.img)

    def get_img(self):
        """读取摄像头图像"""
        while True:
            ret, self.img = self.cap.read()
            cv2.circle(self.img, self.circle_point, self.circle_r, (0, 0, 255), 2)
            if ret:
                break

    def send_msg(self, msg:str|int|Iterable):
        """从串口发送信号"""
        if isinstance(msg, str):
            self.ser.write(msg)
        elif isinstance(msg, int):
            self.ser.send(msg)
        elif isinstance(msg, Iterable):
            self.ser.send_arr(msg)

    def detect_color(self, _colorindex:int) -> bool:
        """颜色识别
        * _colorindex: 颜色索引，0红 1绿 2蓝
        * return: 是否识别到颜色"""
        self.color.set_threshold(thresholds[_colorindex])       # 设置阈值
        # XXX:while True是不是可以去掉，在外部封装
        while True:
            mask = self.color.filter(self.img)                                 # 过滤
            img, p = self.color.draw_cyclic(mask)
            if not p:
                continue
            return True
    
    def LOCATE_cycle(self, _color:str) -> tuple[int, int]|None:
        """色环定位
        * _color: 颜色索引
        * return: 圆心坐标差值，如果识别到多个或者没有识别到返回None"""
        color_index = COLOR_dict_reverse[_color]
        self.color.set_threshold(thresholds[color_index])       # 设置阈值
        
        mask = self.color.filter(self.img)                                 # 过滤
        img, p = self.color.draw_cyclic(mask)
        if len(p) == 1:
            # XXX:是x，y还是y，x
            x0, y0 = self.circle_point
            x, y = p[0]     # 圆心坐标
            difference = x-x0, y-y0
            self.send_msg(difference)
            return difference
        
    
    def CORRECTION_angle(self) -> int|None:
        """校准小车与直线的角度
        * return: 识别到的角度,如果没有识别到直线返回None"""
        angle = self.line.get_angle(self.img)
        if angle is not None:
            angle = int(angle)
            if abs(angle-90) > 0:
                self.send_msg((1, angle))
                return angle

    
    def CORRECTION_distance(self):
        """校准小车与直线的距离"""
        _img = self.cap.read()[1]
        distance = self.line.get_distance(self.img)
        if distance is not None:
            self.send_msg((2, distance))


    def DETECTCOLOR(self):
        """
        颜色识别, 识别到什么颜色就发送什么信号(1,2,3)
        """
        for i in range(3):
            if self.detect_color(i):
                self.send_msg((3, i))

    
    def LOCATECOLOR(self, _colorindex:int):
        """
        色环定位
        ----
        * _colorindex: 颜色索引
        """
        self.color.set_threshold(thresholds[_colorindex])       # 设置阈值
        mask = self.color.filter(self.img)
        res, p = self.color.draw_cyclic(mask)

        dx, dy = p[0][0] - self.circle_point[0], p[0][1] - self.circle_point[1]
        self.send_msg((4, dx, dy))
        


    def __call__(self):
        while True:
            data = self.ser.read()

            if data == 'A':self.CORRECTION_angle()
            elif data == 'D':self.CORRECTION_distance()
            elif data == 'color':self.DETECTCOLOR()
            elif data[0] == 'C':
                if data[1] == 'R':self.LOCATECOLOR(0)
                elif data[1] == 'G':self.LOCATECOLOR(1)
                elif data[1] == 'B':self.LOCATECOLOR(2)
        

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