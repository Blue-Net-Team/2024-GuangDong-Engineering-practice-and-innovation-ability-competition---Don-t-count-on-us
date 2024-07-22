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
import time
from UART import UART
import detector
import cv2

# --------------用于调试的库--------------
from img_trans import VideoStreaming

#region 参数加载
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
        CIRCLE_POINT2:tuple[int, int] = tuple(data['point'])
        CIRCLE_R2:int = data['r']
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
# endregion


class Solution(detector.ColorDetector, detector.LineDetector, detector.CircleDetector):     # type: ignore
    def __init__(self, ifdebug:bool=False):
        self.init_part1()
        self.ser = UART()
        self.cap = cv2.VideoCapture(1)

        self.debug = ifdebug

        img_test_process = multiprocessing.Process(target=self.SEND_TESTIMG)
        if self.debug:
            img_test_process.start()

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

    def send_msg(self, msg:str|int|list|tuple):
        """从串口发送信号"""
        if isinstance(msg, list) or isinstance(msg, tuple):
            self.ser.send_arr(msg)
        else:
            self.ser.write(str(msg))

        # 清除串口缓存
        while self.ser.in_waiting > 0:
            time.sleep(0.1)
            self.ser.ori_read(self.ser.in_waiting)


    def Detect_color(self, _colorindex:int):
        """颜色识别
        * _colorindex: 颜色索引，0红 1绿 2蓝
        * return: 是否识别到颜色(bool)"""
        self.set_threshold(thresholds[_colorindex])       # 设置阈值
        
        while True:
            self.img = self.cap.read()[1]
            mask = self.filter(self.img, COLOR_COLSE, COLOR_OPEN)                                 # 过滤
            if mask is None:
                print('read no img')
                continue
            img, p = self.draw_cycle(mask)
            if self.debug:
                cv2.imwrite('testimg.jpg', img)
            print(p)
            if len(p) == 1:
                print(p)
                # 物料圆区域与夹爪圆区域的重叠面积占比
                Present = circle_intersection_area(p[0][0][0], p[0][0][1], p[0][1], CIRCLE_POINT1[0], CIRCLE_POINT1[1], CIRCLE_R1)/(math.pi*CIRCLE_R1**2)
                print(Present)
                # TODO:需要调试此处Present最小值,保证物料夹取的鲁棒性
                if Present > 0.4:      # 判断物料是否在夹爪内
                    return True

            continue
    
    def CORRECTION_angle(self) -> tuple[bool, int|None]:
        """校准小车与直线的角度
        * return: 识别到的角度,如果没有识别到直线返回None"""
        _, angle = self.get_angle(self.img)
        if angle is not None:
            angle = int(angle)
            if abs(angle-90) > 0:
                return False, angle
            else:
                return True, 0
        return False, None

    def LOCATECOLOR(self, _colorindex:int):
        """
        色环定位
        ----
        * _colorindex: 颜色索引
        * return: 是否定位到色环(bool), x偏移量, y偏移量
        """
        num = 0
        self.set_threshold(thresholds[_colorindex])       # 设置阈值
        while True:
            p_average = [0, 0]
            ps = []
            self.img = self.cap.read()[1]
            mask = self.filter(self.img, CIRCLE_CLOSE, CIRCLE_OPEN)
            if mask is None:
                continue
            img1 = self.img.copy()
            img1 = cv2.bitwise_and(img1, img1, mask=mask)        # 与操作
            mask1, p_list = self.get_circle(img1)        # 画出圆形的图像

            if self.debug:
                cv2.imwrite('testimg.jpg', mask1)

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
                continue
            dx, dy = p_average[0] - CIRCLE_POINT2[0], p_average[1] - CIRCLE_POINT2[1]
            if abs(dx) <= 1 and abs(dy) <= 1:
                return True, 1, 1
            return False, dx, dy
        
    def SEND_TESTIMG(self):
        self.streaming = VideoStreaming('192.168.137.141', 8000)
        self.streaming.connecting()
        self.streaming.start()
        while True:
            try:
                self.testimg = cv2.imread('testimg.jpg')
            except FileNotFoundError:
                continue
            self.streaming.send(self.testimg)    # 发送图传

    def __call__(self):
        while True:
            data = self.ser.read()
            if not data: 
                continue
            print(f'收到信号 {data}')
            self.img = self.cap.read()[1]
            if self.img is None:
                continue
            self.img = self.img[130:370, :]

            if data == 'A':        # 校准角度
                data = self.CORRECTION_angle()
                # XXX:此处角度为1的时候会与完成信号混淆,需要进一步沟通
                if data[0]:
                    self.send_msg(1)
                else:
                    if data[1] is not None:
                        self.send_msg(data[1])
                    else:
                        self.send_msg(0)
            elif data in ['c0', 'c1', 'c2']:          # 在转盘上夹取物料,发送c0 c1 c2
                data = self.Detect_color(int(data[1]))
                if data:
                    self.send_msg(1)
                    print('send 1')
                else:
                    self.send_msg(0)
                    print('send 0')
            elif data[0] == 'C':        # 定位色环,发送CR CG CB
                data = self.LOCATECOLOR(int(data[1]))
                if data[0]:
                    self.send_msg(1)
                    print(f'sended {1}')
                else:
                    self.send_msg((data[1], data[2]))
                    print(f'sended{data[1], data[2]}')

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
    Solution(True)()

    # -------------功能测试代码-------------
    # s = Solution()
    # streaming = VideoStreaming('192.168.137.141', 8000)

    # while True:
    #     s.img = s.cap.read()[1]
    #     s.img = s.img[:300, :]

    #     # 物料的识别
    #     res, dx, dy = s.Detect_color(0, 1)

    #     # 色环的定位
    #     # res = s.LOCATECOLOR(0)

    #     streaming.send(s.testimg)    # 发送图传

    #     print(res)
