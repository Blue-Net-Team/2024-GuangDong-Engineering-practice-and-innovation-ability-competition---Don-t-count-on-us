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
MSG_SEND_dict = {
    0:'R',
    1:'G',
    2:'B'
}


class Solution:
    def __init__(self):
        # TODO:测试夹爪的圆心坐标,半径
        self.circle_point = (100, 100)
        self.circle_r = 50
        self.circle_area = math.pi * self.circle_r ** 2

        # 打开读取图像线程
        threading.Thread(target=self.get_img).start()

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
        self.sending_start = False
        pass


    def correcttion(self):
        """校准小车角度"""
        while True:
            if GPIO.input(c_pin):
                _img = self.cap.read()[1]
                angle = self.line.get_angle(_img)
                if angle is not None:
                    if abs(angle-90) > 2:
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


    def send_msg(self, msg:str|int|list):
        """从串口发送信号"""
        if isinstance(msg, str):
            self.ser.write(msg)
        elif isinstance(msg, int):
            self.ser.send(msg)
        elif isinstance(msg, tuple):
            self.ser.send_arr(msg)

    
    def SENG(self, msg):
        """发送信号"""
        while self.sending_start:
            self.send_msg(msg)


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
        p = None
        def UPDATE():
            """更新"""
            global last_p
            while True:
                last_p = p
                time.sleep(0.5)
        threading.Thread(target=UPDATE).start()
        for msg in qr_lst:
            # region 第一次到原料区拿取物料
            """先停车，然后确定物料停止，再校准位置(持续发送x轴距离，直到重叠面积达到95%)
            拉高17引脚电平，等待0.5秒，拉低17引脚电平，从而触发物料夹持"""
            for i in msg:
                # XXX: 可能有问题
                p = None
                while True:
                    threshold = thresholds[int(i)]      # 获取阈值
                    self.color.set_threshold(threshold)     # 设置阈值
                    self.mask = self.color.filter(self.img)    # 过滤
                    self.mask, p = self.color.draw_cyclic(self.mask)    # 画出色环

                    # region 判断是否停止
                    if last_p is not None:
                        change_p = last_p[0][0]-p[0][0], last_p[0][1]-p[0][1]
                    else:
                        change_p = p[0]
                    
                    change = change_p[0] ** 2 + change_p[1] ** 2
                    change = change ** 0.5

                    if change < 5:      #TODO: 确定是不是5,物料半秒移动的距离
                        break
                    # endregion

                # region 校准位置
                defence_x = None
                self.sending_start = True
                threading.Thread(target=self.SENG, args=(defence_x,)).start()

                while True:
                    self.mask = self.color.filter(self.img)    # 过滤
                    self.mask, p = self.color.draw_cyclic(self.mask)    # 画出色环
                    defence_x = p[0][0]-self.circle_point[0]        # 计算色环与圆心的x轴距
                    # 计算重叠面积
                    area = circle_intersection_area(p[0][0], p[0][1], p[1], self.circle_point[0], self.circle_point[1], self.circle_r)

                    if area/self.circle_area > 0.95:
                        GPIO.output(sign_pin, 1)
                        time.sleep(0.5)
                        GPIO.output(sign_pin, 0)
                        self.sending_start = False
                        break
                # endregion

            # endregion
                    
            # region 将物料放到粗加工区 无序
            """
            定位色环   ->   停车信号 
                ^              |
                |              v
             继续向前   <-  做出动作
            定位红绿蓝色环，发出RGB信号到UART
            """
            for i in range(3):
                threshold = thresholds[i]
                self.color.set_threshold(threshold)
                while True:
                    self.mask = self.color.filter(self.img)
                    self.mask, p = self.color.draw_cyclic(self.mask)
                    
                    # 计算重叠面积
                    area = circle_intersection_area(p[0][0], p[0][1], p[1], self.circle_point[0], self.circle_point[1], self.circle_r)
                    if area/self.circle_area > 0.95:
                        self.send_msg(MSG_SEND_dict[i])
                        break
            # endregion
                    
            # region 将物料从粗加工取出 有序
            for i in msg:
                # TODO:讨论部分
                """放完物料之后小车在区域尾部，电控读取了二维码的顺序信息后，
                在此处是不是应该根据信息直接来到对应的颜色区域，然后我在进行调整，
                因为颜色区域是红绿蓝顺序的。
                
                是不是应该拉高一个引脚电平（到了大概位置之后）然后我在开始调整呢？"""
                # XXX: 未讨论而且可能有问题
                # region 校准位置
                defence_x = None
                self.sending_start = True
                threading.Thread(target=self.SENG, args=(defence_x,)).start()

                while True:
                    self.mask = self.color.filter(self.img)    # 过滤
                    self.mask, p = self.color.draw_cyclic(self.mask)    # 画出色环
                    defence_x = p[0][0]-self.circle_point[0]        # 计算色环与圆心的x轴距
                    # 计算重叠面积
                    area = circle_intersection_area(p[0][0], p[0][1], p[1], self.circle_point[0], self.circle_point[1], self.circle_r)

                    if area/self.circle_area > 0.95:
                        GPIO.output(sign_pin, 1)
                        time.sleep(0.5)
                        GPIO.output(sign_pin, 0)
                        self.sending_start = False
                        break
                # endregion
            # endregion
                    
            # region 将物料放到暂存区 有序
            # TODO:需要讨论,与上面相同
            for i in msg:
                defence_x = None
                self.sending_start = True
                threading.Thread(target=self.SENG, args=(defence_x,)).start()
                
                while True:
                    if GPIO.input(start_pin):   # 启动引脚被拉高电平
                        self.mask = self.color.filter(self.img)
                        self.mask, p = self.color.draw_cyclic(self.mask)

                        # 计算重叠面积
                        area = circle_intersection_area(p[0][0], p[0][1], p[1], self.circle_point[0], self.circle_point[1], self.circle_r)

                        if area/self.circle_area > 0.95:
                            GPIO.output(sign_pin, 1)
                            time.sleep(0.5)
                            GPIO.output(sign_pin, 0)
                            self.sending_start = False
                            break
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