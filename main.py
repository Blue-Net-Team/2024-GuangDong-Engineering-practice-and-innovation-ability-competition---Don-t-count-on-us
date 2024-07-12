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
from typing import Iterable
from UART import UART
import detector
import cv2


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


class Solution(detector.ColorDetector, detector.LineDetector):
    def __init__(self):
        self.init_part1()
        # 创建串口对象，self.ser继承了二维码识别的功能
        self.ser = UART()
        self.cap = cv2.VideoCapture(0)


    def init_part1(self):
        # TODO:测试夹爪的圆心坐标,半径
        self.circle_point = (100, 100)
        self.circle_r = 50

        # 创建识别器对象
        detector.ColorDetector.__init__(self)
        detector.LineDetector.__init__(self)

        # 颜色识别器面积初始化
        self.minarea = 0
        self.maxarea = 100000

        self.mask = None

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
        self.set_threshold(thresholds[_colorindex])       # 设置阈值
        if self.img is None:return False
        mask = self.filter(self.img)                                 # 过滤
        if mask is None:return False
        img, p = self.draw_cyclic(mask)
        if not p:return False
        return True
    
    
    def CORRECTION_angle(self) -> tuple[int, int]|None:
        """校准小车与直线的角度
        * return: 识别到的角度,如果没有识别到直线返回None"""
        angle = self.get_angle(self.img)
        if angle is not None:
            angle = int(angle)
            if abs(angle-90) > 0:
                return 1, angle


    def CORRECTION_distance(self) -> tuple[int,int]|None:
        """校准小车与直线的距离"""
        distance = self.get_distance(self.img)
        if distance is not None:
            return 2, distance


    def DETECTCOLOR(self):
        """
        颜色识别, 识别到什么颜色就发送什么信号(1,2,3)
        """
        for i in range(3):
            if self.detect_color(i):
                return 3, i


    def LOCATECOLOR(self, _colorindex:int):
        """
        色环定位
        ----
        * _colorindex: 颜色索引
        """
        self.set_threshold(thresholds[_colorindex])       # 设置阈值
        mask = self.filter(self.img)
        if mask is None:return None
        res, p = self.draw_cyclic(mask)

        if not p: return None

        dx, dy = p[0][0] - self.circle_point[0], p[0][1] - self.circle_point[1]
        return 4, dx, dy
        

    def __call__(self):
        while True:
            data = self.ser.read()
            if not data: continue
            self.img = self.cap.read()[1]

            if data == 'A':
                data = self.CORRECTION_angle()
                if data is not None:
                    self.send_msg(data)
            elif data == 'D':
                data = self.CORRECTION_distance()
                if data is not None:
                    self.send_msg(data)
            elif data == 'color':
                data = self.DETECTCOLOR()
                if data is not None:
                    self.send_msg(data)
            elif data[0] == 'C':
                if data[1] == 'R':
                    data = self.LOCATECOLOR(0)
                    if data is not None:
                        self.send_msg(data)
                elif data[1] == 'G':
                    data = self.LOCATECOLOR(1)
                    if data is not None:
                        self.send_msg(data)
                elif data[1] == 'B':
                    data = self.LOCATECOLOR(2)
                    if data is not None:
                        self.send_msg(data)
        

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