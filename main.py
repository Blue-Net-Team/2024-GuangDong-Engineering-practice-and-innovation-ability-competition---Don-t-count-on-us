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
from UART import UART
import detector
import cv2
import threading


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

# 创建线程
t = threading.Thread(target=correcttion)
t.start()

while True:
    #识别二维码
    img = cap.read()[1]
    qr_msg = qr(img)
    if qr_msg is not None:
        qr_msg = map(str, qr_msg.split())       # ['red', 'green', 'blue']有序的颜色信息
        break

while ser.read() != b'\n':      # 电控发送start信号，开始第一次识别颜色
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
            ret, img = cap.read()
            # 二值化滤波
            img_color = color.filter(img)

            img_color, cycle = color.draw_cyclic(img_color)

            if cycle:       # 识别到颜色
                ser.send(1)     # 发送信号，做出抓取动作
                break


        