"""在本地电脑运行"""
import json
import math
import cv2
from img_trans import ReceiveImg
import main

thresholds = main.thresholds

class DEBUG(main.Solution):
    def __init__(self, iftrans:bool=True, capid:int=0) -> None:
        """ 
        初始化
        * iftrans: 是否使用图传
        * capid: 摄像头ID，默认为0
        """
        super().init_part1()

        # 初始化圆形信息
        self.circle_point1 = main.CIRCLE_POINT1
        self.r1 = main.CIRCLE_R1

        self.circle_point2 = main.CIRCLE_POINT2
        self.r2 = main.CIRCLE_R2

        self.maxval = main.MAXVAL
        self.minval = main.MINVAL

        self.circle_id = 0

        if self.circle_id == 0:
                self.circle_point = self.circle_point1
                self.r = self.r1
        elif self.circle_id == 1:
            self.circle_point = self.circle_point2
            self.r = self.r2
        else:
            raise ValueError('circle_id error')
        
        # 用于调整阈值表的索引
        self.color = 0  # 0红 1绿 2蓝

        if iftrans:
            # 图传
            self.reveiver = ReceiveImg('192.168.137.141', 8000)
        else:
            self.reveiver = cv2.VideoCapture(capid)

        self.set_threshold(thresholds[self.color])

        self.color_open = main.COLOR_OPEN
        self.color_close = main.COLOR_COLSE
        
        self.circle_open = main.CIRCLE_OPEN
        self.circle_close = main.CIRCLE_CLOSE

        self.line_open = main.LINE_OPEN
        self.line_close = main.LINE_CLOSE

        self.thre = 200

    # region 鼠标事件
    def mouse_action_circlePoint(self, event, x, y, flags, param):
        """鼠标事件回调函数
        ====
        针对色环定位的鼠标事件回调函数"""
        if event == cv2.EVENT_LBUTTONDOWN:
            if self.circle_id == 0:
                self.circle_point1 = (x, y)
                print(f'circle_point1:{self.circle_point1}')
            elif self.circle_id == 1:
                self.circle_point2 = (x, y)
                print(f'circle_point2:{self.circle_point2}')
            else:
                raise ValueError('circle_id error')

    def mouse_action_Line(self, event, x, y, flags, param):
        """鼠标事件回调函数
        ====
        针对直线识别的鼠标事件回调函数"""
        if event == cv2.EVENT_LBUTTONDOWN:
            self.y = y
            print(f'y:{self.y}')
            with open('y.json', 'w') as f:
                json.dump({'y':y}, f)
    # endregion

    # region 回调函数
    def callback_circle(self, x):
        self.r = x

    def call_back_low_h(self, low_h):
        super().call_back_low_h(low_h)
        thresholds[self.color][0][0] = low_h

    def call_back_low_s(self, low_s):
        super().call_back_low_s(low_s)
        thresholds[self.color][0][1] = low_s

    def call_back_low_v(self, low_v):
        super().call_back_low_v(low_v)
        thresholds[self.color][0][2] = low_v

    def call_back_high_h(self, high_h):
        super().call_back_high_h(high_h)
        thresholds[self.color][1][0] = high_h

    def call_back_high_s(self, high_s):
        super().call_back_high_s(high_s)
        thresholds[self.color][1][1] = high_s

    def call_back_high_v(self, high_v):
        super().call_back_high_v(high_v)
        thresholds[self.color][1][2] = high_v

    def callback_color_OK(self, x):
        if x == 1:      # 保存
            with open(f'{main.COLOR_dict[self.color]}.json', 'w') as f: # 保存颜色阈值
                json.dump(thresholds[self.color], f)
            with open(f'circle{self.circle_id+1}.json', 'w') as f:      # 保存夹爪基准圆的信息
                json.dump({'point':self.circle_point, 'r':self.r}, f)

            with open('radius.json', 'w') as f:                         # 保存物料最小圆半径和最大圆半径
                json.dump({'minR':self.minR, 'maxR':self.maxR}, f)

            with open('color_oc.json', 'w') as f:
                json.dump({'open':self.color_open, 'close':self.color_close}, f)

            with open('circle_oc.json', 'w') as f:
                json.dump({'open':self.circle_open, 'close':self.circle_close}, f)
            
    def callback_color(self, x):
        self.color = x
        self.set_threshold(thresholds[x])

        # 更新trackbar
        cv2.setTrackbarPos('low_h', 'Color and circle trackbar0', thresholds[x][0][0])
        cv2.setTrackbarPos('low_s', 'Color and circle trackbar0', thresholds[x][0][1])
        cv2.setTrackbarPos('low_v', 'Color and circle trackbar0', thresholds[x][0][2])
        cv2.setTrackbarPos('high_h', 'Color and circle trackbar0', thresholds[x][1][0])
        cv2.setTrackbarPos('high_s', 'Color and circle trackbar0', thresholds[x][1][1])
        cv2.setTrackbarPos('high_v', 'Color and circle trackbar0', thresholds[x][1][2])
    
    def callback_line_OK(self, x):
        if x == 1:      # 保存
            with open('line.json', 'w') as f:
                json.dump({'minval':self.minval, 'maxval':self.maxval}, f)
 
            with open('y.json', 'w') as f:
                json.dump({'y':self.y}, f)

            with open('line_oc.json', 'w') as f:
                json.dump({'open':self.line_open, 'close':self.line_close}, f)

    def callback_color_open(self, x):
        self.color_open = x

    def callback_color_close(self, x):
        self.color_close = x

    def callback_circle_open(self, x):
        self.circle_open = x

    def callback_circle_close(self, x):
        self.circle_close = x

    def callback_circle_choise(self, x):
        self.circle_id =  x

    def callback_line_open(self, x):
        self.line_open = x

    def callback_line_close(self, x):
        self.line_close = x

    def callback_line_thre(self, x):
        self.thre = x
    # endregion

    # region trackbar
    def __createTrackbar_color_and_circle(self):
        """
        创建色环和物料颜色识别的trackbar
        """
        # --------颜色阈值和物料最小圆半径和最大圆半径的调试trackbar--------
        main.detector.ColorDetector.createTrackbar(self)        # 呼出颜色阈值和外接圆半径trackbar
        cv2.createTrackbar('open', 'Color and circle trackbar0', self.color_open, 5, self.callback_color_open)        # 开运算trackbar
        cv2.createTrackbar('close', 'Color and circle trackbar0', self.color_close, 5, self.callback_color_close)      # 闭运算trackbar
        cv2.createTrackbar('RGB', 'Color and circle trackbar0', 0, 2, self.callback_color)          # 颜色选择trackbar
        cv2.createTrackbar('r', 'Color and circle trackbar0', self.r, 400, self.callback_circle)    # 基准圆半径trackbar
        cv2.createTrackbar('circle id', 'Color and circle trackbar0', 0, 1, self.callback_circle_choise)    # 圆的id
        cv2.createTrackbar('OK', 'Color and circle trackbar0', 0, 1, self.callback_color_OK)        # 保存trackbar

        # --------色环最小圆半径和最大圆半径的调试trackbar--------
        main.detector.CircleDetector.createTrackbar(self)
        cv2.createTrackbar('open', 'Circle trackbar0', self.circle_open, 5, self.callback_circle_open)        # 开运算trackbar
        cv2.createTrackbar('close', 'Circle trackbar0', self.circle_close, 15, self.callback_circle_close)      # 闭运算trackbar

    def __createTrackbar_line(self):
        """
        创建直线识别的trackbar
        """
        main.detector.LineDetector.createTrackbar(self)
        cv2.createTrackbar('open', 'Line trackbar0', self.line_open, 10, self.callback_line_open)        # 开运算trackbar
        cv2.createTrackbar('close', 'Line trackbar0', self.line_close, 10, self.callback_line_close)      # 闭运算trackbar
        cv2.createTrackbar('thre', 'Line trackbar0', self.thre, 255, self.callback_line_thre)      # 二值化阈值trackbar
        cv2.createTrackbar('OK', 'Line trackbar0', 0, 1, self.callback_line_OK)
    # endregion

    def SetCircleThresholds(self):
        """
        色环颜色识别的阈值调试
        """
        self.__createTrackbar_color_and_circle()        # 呼出trackbar
        cv2.namedWindow('img', cv2.WINDOW_AUTOSIZE)
        cv2.setMouseCallback('img', self.mouse_action_circlePoint)    # 设置鼠标事件回调函数
        num = 0
        ps = []             # points
        while True:
            p_average = [0, 0]
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环
            self.img = self.img[130:370, :]

            # 画出色环应该在的位置和大小
            img1 = self.img.copy()
            img2 = self.img.copy()
            cv2.circle(img2, self.circle_point, self.r, (0, 255, 0), 2)
            mask = self.filter(self.img, self.circle_close, self.circle_open)        # 二值化的图像
            if mask is None: continue
            img1 = cv2.bitwise_and(img1, img1, mask=mask)        # 与操作
            mask1, p_list = self.get_circle(img1)        # 画出圆形的图像

            # self.img:原始图像
            # img1:对img深拷贝然后再画圈的图像
            try:
                for i in p_list:
                    cv2.circle(img2, i[0], i[1], (255,0,255), 2)
            except:
                pass
            if p_list.shape == (1,3):
                # print(p_list)
                ps.append((p_list[0][0], p_list[0][1]))
                num += 1
            if num % 10 == 0 and num != 0:      # 每10次取平均值
                for i in ps:
                    p_average[0] += i[0]
                    p_average[1] += i[1]
                p_average[0] = p_average[0] // 10
                p_average[1] = p_average[1] // 10
                p_average[0], p_average[1] = int(p_average[0]), int(p_average[1])
                print(p_average)
                cv2.circle(img2, (p_average[0], p_average[1]), 2, (255, 255, 0), 2)
                num = 0
                ps = []
            cv2.imshow('img', img2)

            if mask is None: continue
            cv2.imshow('mask', mask)
            cv2.imshow('mask1', mask1)

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break

    def SetColorThresholds(self):
        """
        物料颜色识别的阈值调试
        """
        self.__createTrackbar_color_and_circle()        # 呼出trackbar
        cv2.namedWindow('img', cv2.WINDOW_AUTOSIZE)
        cv2.setMouseCallback('img', self.mouse_action_circlePoint)    # 设置鼠标事件回调函数
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环
            self.img = self.img[130:370, :]
            # self.img = cv2.GaussianBlur(self.img, (5, 5), 10)

            # 画出色环应该在的位置和大小
            img1 = self.img.copy()
            img2 = self.img.copy()

            if self.circle_id == 0:
                self.circle_point = self.circle_point1
                self.r = self.r1
            elif self.circle_id == 1:
                self.circle_point = self.circle_point2
                self.r = self.r2
            else:
                raise ValueError('circle_id error')
            
            cv2.circle(img2, self.circle_point, self.r, (0, 255, 0), 2)
            mask = self.filter(self.img, self.color_close, self.color_open)        # 二值化的图像
            if mask is None: continue

            res, p_list = self.draw_cycle(mask)
            print(p_list)
            if len(p_list) == 1:
                cv2.circle(img2, p_list[0][0], p_list[0][1], (255, 0, 255), 2)

                res = main.circle_intersection_area(p_list[0][0][0], p_list[0][0][1], p_list[0][1],
                                                    self.circle_point1[0], self.circle_point1[1], self.r1)/(math.pi*self.r1**2)
                print(res)
                if res > 0.4:      # 判断物料是否在夹爪内
                    print(True)

            cv2.imshow('img', img2)
            cv2.imshow('mask', mask)

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break


    def SetLineThresholds(self):
        """设置直线识别相关阈值"""
        self.ypath='y.json'         # type:str
        self.__createTrackbar_line()        # 呼出trackbar
        cv2.namedWindow('img', cv2.WINDOW_AUTOSIZE)
        cv2.setMouseCallback('img', self.mouse_action_Line)    # 设置鼠标事件回调函数
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环
            self.img = self.img[130:360, :]
            img1 = self.img.copy()
            img1, angle = self.get_angle(self.img, True, self.line_close, self.line_open, 1, self.thre)
            # distance = self.get_distance(self.img, self.y, True)

            print(f'angle:{angle}')
            cv2.imshow('img', img1)             # type:ignore

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break

    def ReadOriImg(self):
        """读取原图"""
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环
            cv2.imshow('img', self.img)
            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break


if __name__ == '__main__':
    debug = DEBUG(True)
    # region 阈值调试
    # debug.SetCircleThresholds()
    debug.SetLineThresholds()
    # debug.SetColorThresholds()
    # debug.ReadOriImg()
    # endregion

