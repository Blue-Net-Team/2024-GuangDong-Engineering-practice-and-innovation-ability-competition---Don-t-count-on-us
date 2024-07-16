"""在本地电脑运行"""
import json
import socket
import cv2
import numpy as np
import main


thresholds = [
    #[low_h, low_s, low_v], [high_h, high_s, high_v]
    ([0, 0, 0], [0, 0, 0]),
    ([0, 0, 0], [0, 0, 0]),
    ([0, 0, 0], [0, 0, 0]),
]

for i in range(3):
    try:
        with open(f'{main.COLOR_dict[i]}.json', 'r') as f:
            thresholds[i] = json.load(f)
    except:
        pass

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
        """ 
        初始化
        * iftrans: 是否使用图传
        * capid: 摄像头ID，默认为0
        """
        super().init_part1()

        self.r = 20
        self.color = 0  # 0红 1绿 2蓝

        if iftrans:
            # 图传
            # TODO: 修改IP地址
            self.reveiver = ReceiveImg('192.168.137.103', 8000)
        else:
            self.reveiver = cv2.VideoCapture(capid)

        self.set_threshold(thresholds[self.color])

    # region 鼠标事件
    def mouse_action_circlePoint(self, event, x, y, flags, param):
        """鼠标事件回调函数
        ====
        针对色环定位的鼠标事件回调函数"""
        if event == cv2.EVENT_LBUTTONDOWN:
            self.circle_point = (x, y)
            print(f'circle_point:{self.circle_point}')

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
        self.circle_r = x

    def callback_OK(self, x):
        if x == 1:      # 保存
            with open(f'{main.COLOR_dict[self.color]}.json', 'w') as f:
                json.dump(thresholds[self.color], f)
            
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
    # endregion

    def __createTrackbar_color_and_circle(self):
        main.detector.ColorDetector.createTrackbar(self)        # 呼出trackbar
        cv2.createTrackbar('RGB', 'Color and circle trackbar0', 0, 2, self.callback_color)
        cv2.createTrackbar('r', 'Color and circle trackbar0', self.r, 400, self.callback_circle)
        cv2.createTrackbar('OK', 'Color and circle trackbar0', 0, 1, self.callback_OK)

    def SetColorThresholds(self):
        """
        颜色识别的阈值调试
        """
        self.__createTrackbar_color_and_circle()        # 呼出trackbar
        cv2.namedWindow('img', cv2.WINDOW_NORMAL)
        cv2.setMouseCallback('img', self.mouse_action_circlePoint)    # 设置鼠标事件回调函数
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环

            # 画出色环应该在的位置和大小
            img1 = self.img.copy()
            cv2.circle(img1, self.circle_point, self.circle_r, (0, 255, 0), 2)
            mask = self.filter(self.img)        # 二值化的图像
            if mask is None: continue
            mask1, p_list = self.draw_cycle(mask)        # 画出圆形的图像

            # self.img:原始图像
            # img1:对img深拷贝然后再画圈的图像
            try:
                # cv2.circle(img1, p_list[0][0], p_list[0][1], (255,0,255), 2)        # type:ignore
                for i in p_list:
                    cv2.circle(img1, i[0], i[1], (255,0,255), 2)
            except:
                pass
            if len(p_list) == 1:
                print(p_list)
            cv2.imshow('img', img1)

            if mask is None: continue
            cv2.imshow('mask', mask)
            # cv2.imshow('mask1', mask1)

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break
        

    def SetLineThresholds(self):
        """设置直线识别相关阈值"""
        self.ypath='y.json'         # type:str
        main.detector.LineDetector.createTrackbar(self)        # 呼出trackbar
        cv2.namedWindow('img', cv2.WINDOW_NORMAL)
        cv2.setMouseCallback('img', self.mouse_action_Line)    # 设置鼠标事件回调函数
        while True:
            _, self.img = self.reveiver.read()
            if self.img is None:continue        # 如果没有读取到图像数据，继续循环
            img1 = self.img.copy()
            img1, angle = self.get_angle(self.img, True)
            distance = self.get_distance(self.img, self.ypath, True)

            print(f'angle:{angle}, distance:{distance}')
            cv2.imshow('img', img1)             # type:ignore

            if cv2.waitKey(1) == 27:        # 按下ESC键退出
                break

if __name__ == '__main__':
    debug = DEBUG(False)
    # region 阈值调试
    debug.SetColorThresholds()
    # debug.SetLineThresholds()
    # debug.SetCircleThresholds()
    # endregion

