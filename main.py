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
import json
import math
import multiprocessing
from typing import Iterable
from UART import UART
import detector
import cv2

# --------------用于调试的库--------------
from img_trans import VideoStreaming


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

thresholds = [
    #[low_h, low_s, low_v], [high_h, high_s, high_v]
    ([0, 0, 0], [255, 255, 255]),
    ([0, 0, 0], [255, 255, 255]),
    ([0, 0, 0], [255, 255, 255]),
]
# 读取阈值
for i in range(3):
    try:
        with open(f'{COLOR_dict[i]}.json', 'r') as f:
            thresholds[i] = json.load(f)
    except:
        pass
    
CIRCLE_POINT1 = (100, 100)
CIRCLE_R1 = 50
try:        # 读取转盘基准圆心和半径
    with open('circle1.json', 'r') as f:
        data = json.load(f)
        CIRCLE_POINT1 = tuple(data['point'])
        CIRCLE_R1 = data['r']
except FileNotFoundError:
    pass

CIRCLE_POINT2 = (100, 100)
CIRCLE_R2 = 50
try:
    with open('circle2.json', 'r') as f:
        data = json.load(f)
        CIRCLE_POINT2 = tuple(data['point'])
        CIRCLE_R2 = data['r']
except FileNotFoundError:
    pass

MINVAL = 0
MAXVAL = 255
try:        # 直线canny算子的参数
    with open('line.json', 'r') as f:
        data = json.load(f)
        MINVAL = data['minval']
        MAXVAL = data['maxval']
except FileNotFoundError:
    pass

Y0 = 0
try:        # 直线的基准y坐标
    with open('y.json', 'r') as f:
        data = json.load(f)
        Y0 = data['y']
except FileNotFoundError:
    pass

# 物料识别的开闭运算参数
COLOR_COLSE = 0
COLOR_OPEN = 1
try:
    with open('color_oc.json', 'r') as f:
        data = json.load(f)
        COLOR_COLSE = data['close']
        COLOR_OPEN = data['open']
except FileNotFoundError:
    pass

CIRCLE_OPEN = 0 
CIRCLE_CLOSE = 1
try:
    with open('circle_oc.json', 'r') as f:
        data = json.load(f)
        CIRCLE_OPEN = data['open']
        CIRCLE_CLOSE = data['close']
except FileNotFoundError:
    pass

LINE_OPEN = 0
LINE_CLOSE = 1
try:
    with open('line_oc.json', 'r') as f:
        data = json.load(f)
        LINE_OPEN = data['open']
        LINE_CLOSE = data['close']
except FileNotFoundError:
    pass

class Solution(detector.ColorDetector, detector.LineDetector, detector.CircleDetector):     # type: ignore
    def __init__(self, ifdebug:bool=False):
        self.init_part1()
        self.ser = UART()
        self.cap = cv2.VideoCapture(0)

        self.debug = ifdebug

    def init_part1(self):

        # 创建识别器对象
        detector.ColorDetector.__init__(self)
        detector.LineDetector.__init__(self)
        detector.CircleDetector.__init__(self)
        detector.LineDetector.set_threshold(self, (MINVAL, MAXVAL))

        try:
            with open('radius.json', 'r') as f:
                data = json.load(f)
                self.minR = data['minR']
                self.maxR = data['maxR']
        except:
            pass

        self.mask = None
        # 用于测试过程中发送图传的图片
        self.testimg = None

    def send_msg(self, msg:str|int|Iterable):
        """从串口发送信号"""
        if isinstance(msg, str):
            self.ser.write(msg)
        elif isinstance(msg, int):
            self.ser.send(msg)
        elif isinstance(msg, Iterable):
            self.ser.send_arr(msg)

    def Detect_color(self, _colorindex:int, time:int):
        """颜色识别
        * _colorindex: 颜色索引，0红 1绿 2蓝
        * return: 是否识别到颜色(bool), dx, dy"""
        if time == 1:
            CIRCLE_POINT = CIRCLE_POINT1
            CIRCLE_R = CIRCLE_R1
        elif time == 2:
            CIRCLE_POINT = CIRCLE_POINT2
            CIRCLE_R = CIRCLE_R2
        else:
            raise ValueError('time参数错误')
        
        self.set_threshold(thresholds[_colorindex])       # 设置阈值
        if self.img is None:return False, None, None
        mask = self.filter(self.img, COLOR_COLSE, COLOR_OPEN)                                 # 过滤
        if mask is None:return False, None, None
        img, p = self.draw_cycle(mask)
        if self.debug:
            self.testimg = img
        if len(p) == 1:
            if circle_intersection_area(p[0][0][0], p[0][0][1], p[0][1], 
                                    CIRCLE_POINT[0], CIRCLE_POINT[1], CIRCLE_R)/math.pi*CIRCLE_R1**2 > 0.8:      # 判断物料是否在夹爪内
                return True, 0, 0
            else:
                dx:int = p[0][0][0] - CIRCLE_POINT[0]
                dy:int = p[0][0][1] - CIRCLE_POINT[1]
                return False, dx, dy
            
        return False, None, None
    
    
    def CORRECTION_angle(self) -> tuple[int, int]|None:
        """校准小车与直线的角度
        * return: 识别到的角度,如果没有识别到直线返回None"""
        _, angle = self.get_angle(self.img)
        if angle is not None:
            angle = int(angle)
            if abs(angle-90) > 0:
                return 1, angle


    def CORRECTION_distance(self) -> tuple[int,int]|None:
        """校准小车与直线的距离"""
        distance = self.get_distance(self.img, Y0)
        if distance is not None:
            return 2, int(distance)


    def DETECTCOLOR(self):
        """
        物料颜色识别, 识别到什么颜色就发送什么信号(1,2,3)
        """
        for i in range(3):
            if self.Detect_color(i, 1):
                return 3, i


    def LOCATECOLOR(self, _colorindex:int):
        """
        色环定位
        ----
        * _colorindex: 颜色索引
        """
        num = 0
        while True:
            p_average = [0, 0]
            ps = []
            self.set_threshold(thresholds[_colorindex])       # 设置阈值
            mask = self.filter(self.img, CIRCLE_CLOSE, CIRCLE_OPEN)
            if mask is None:return None
            img1 = self.img.copy()
            img1 = cv2.bitwise_and(img1, img1, mask=mask)        # 与操作
            mask1, p_list = self.get_circle(img1)        # 画出圆形的图像

            if p_list.shape == (1,3):
                # print(p_list)
                ps.append((p_list[0][0], p_list[0][1]))
                num += 1
            if num % 10 == 0 and num != 0:
                for i in ps:
                    p_average[0] += i[0]
                    p_average[1] += i[1]
                p_average[0] = p_average[0] // 10
                p_average[1] = p_average[1] // 10
                p_average[0], p_average[1] = int(p_average[0]), int(p_average[1])
                # print(p_average)
            else:
                continue
            
            if p_average == [0, 0]:
                return None
            dx, dy = p_average[0] - CIRCLE_POINT2[0], p_average[1] - CIRCLE_POINT2[1]
            return 4, dx, dy
        

    def SEND_TESTIMG(self):
        streaming = VideoStreaming('192.168.137.91', 8000)
        streaming.connecting()
        streaming.start()
        while True:
            if self.testimg is not None:
                streaming.send(self.testimg)    # 发送图传

    def __call__(self):
        img_test_process = multiprocessing.Process(target=self.SEND_TESTIMG)
        img_test_process.start()
        while True:
            data = self.ser.read()
            if not data: continue
            print(f'收到信号 {data}')
            self.img = self.cap.read()[1]
            self.img = self.img[:300, :]

            if data == 'A':        # 校准角度
                data = self.CORRECTION_angle()
                if data is not None:
                    self.send_msg(data)
                else:
                    self.send_msg((255,255,255,255))
            elif data == 'D':       # 校准距离
                data = self.CORRECTION_distance()
                if data is not None:
                    self.send_msg(data)
                else:
                    self.send_msg((255,255,255,255))
            elif data == 'c1':          # 在转盘上夹取物料
                data = self.DETECTCOLOR()
                if data is not None:
                    self.send_msg(data)
                else:
                    self.send_msg((255,255,255,255))
            elif data[0] == 'c':        # 发送cR cG cB,在地上夹取物料
                res = self.Detect_color(COLOR_dict_reverse[data[1]], 2)
                if res[0]:
                    msg = 3, 1, res[1], res[2]
                    self.send_msg(msg)
                else:
                    msg = 3, 0, res[1], res[2]
                    self.send_msg(msg)     
            elif data[0] == 'C':        # 定位色环,发送CR CG CB
                if data[1] == 'R':
                    data = self.LOCATECOLOR(0)
                    if data is not None:
                        self.send_msg(data)
                    else:
                        self.send_msg((255,255,255,255))
                elif data[1] == 'G':
                    data = self.LOCATECOLOR(1)
                    if data is not None:
                        self.send_msg(data)
                    else:
                        self.send_msg((255,255,255,255))
                elif data[1] == 'B':
                    data = self.LOCATECOLOR(2)
                    if data is not None:
                        self.send_msg(data)
                    else:
                        self.send_msg((255,255,255,255))

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
    # Solution()()

    # -------------功能测试代码-------------
    s = Solution()
    streaming = VideoStreaming('192.168.137.141', 8000)

    while True:
        s.img = s.cap.read()[1]
        s.img = s.img[:300, :]

        # 物料的识别
        res:bool = s.Detect_color(0)

        # 色环的定位
        # res = s.LOCATECOLOR(0)

        streaming.send(s.testimg)    # 发送图传

        print(res)
