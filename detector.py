import cv2
import numpy as np
import serial


class ColorDetector(object):
    """颜色识别器
    * 通过调节trackbar来调节颜色阈值
    * 使用detect方法来识别颜色，得到二值化过滤后的图像数据
    * 使用draw_rectangle方法来在图像上绘制矩形框，传入的应该是二值化过滤后的图像数据"""
    def __init__(self) -> None:
        """初始化"""
        # region 初始颜色阈值
        self.low_h = 0
        self.low_s = 0
        self.low_v = 0

        self.high_h = 180
        self.high_s = 255
        self.high_v = 255

        self.minarea = 0
        self.maxarea = 100000 # 220800
        # endregion

    def set_threshold(self, _threshold:tuple[list[int], list[int]]) -> None:
        """设置颜色阈值
        * _threshold: 传入的颜色阈值"""
        self.low_h = _threshold[0][0]
        self.low_s = _threshold[0][1]
        self.low_v = _threshold[0][2]

        self.high_h = _threshold[1][0]
        self.high_s = _threshold[1][1]
        self.high_v = _threshold[1][2]

    def __call__(self,id:int=0):
        """使用call方法调出trackbar
        * id: 识别器的id的id"""
        # region 创建trackbar
        cv2.namedWindow(f'trackbar{id}', cv2.WINDOW_NORMAL)
        cv2.createTrackbar('low_h', f'trackbar{id}', self.low_h, 180, self.call_back_low_h)
        cv2.createTrackbar('high_h', f'trackbar{id}', self.high_h, 180, self.call_back_high_h)
        cv2.createTrackbar('low_s', f'trackbar{id}', self.low_s, 255, self.call_back_low_s)
        cv2.createTrackbar('high_s', f'trackbar{id}', self.high_s, 255, self.call_back_high_s)
        cv2.createTrackbar('low_v', f'trackbar{id}', self.low_v, 255, self.call_back_low_v)
        cv2.createTrackbar('high_v', f'trackbar{id}', self.high_v, 255, self.call_back_high_v)
        cv2.createTrackbar('minarea', f'trackbar{id}', self.minarea, 100000, self.call_back_minarea)
        cv2.createTrackbar('maxarea', f'trackbar{id}', self.maxarea, 100000, self.call_back_maxarea)
        # endregion
        pass

    # region trackbar回调函数
    def callback(self, x):
        pass

    def call_back_low_h(self, low_h):
        self.low_h = low_h
    
    def call_back_high_h(self, high_h):
        self.high_h = high_h

    def call_back_low_s(self, low_s):
        self.low_s = low_s

    def call_back_high_s(self, high_s):
        self.high_s = high_s

    def call_back_low_v(self, low_v):
        self.low_v = low_v

    def call_back_high_v(self, high_v):
        self.high_v = high_v

    def call_back_minarea(self, minarea):
        self.minarea = minarea

    def call_back_maxarea(self, maxarea):
        self.maxarea = maxarea
    # endregion

    def filter(self, img:cv2.typing.MatLike, _iterations:int=1):
        """颜色识别 二值化滤波
        * img: 传入的图像数据
        * 返回值：二值化过滤后的图像数据"""
        _shape = img.shape
        try:
            hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)				# 将BGR图像转换成HSV图像
        except:
            return np.zeros((_shape[0], _shape[1]), np.uint8)

        low = np.array([self.low_h, self.low_s, self.low_v])		# 低阈值
        high = np.array([self.high_h, self.high_s, self.high_v])	# 高阈值

        mask = cv2.inRange(hsv, low, high)						# 通过阈值过滤图像，将在阈值范围内的像素点设置为255，不在阈值范围内的像素点设置为0
        kernel = np.ones((5, 5), np.uint8)						# 创建一个5*5的矩阵，矩阵元素全为1
        opencal = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)	# 对mask进行闭运算，填充小洞
        # 对opencal进行膨胀
        res = cv2.dilate(opencal, kernel, iterations=_iterations)
        # 对res进行腐蚀
        res = cv2.erode(res, kernel, iterations=_iterations)
        return res

    def draw_rectangle(self, img:cv2.typing.MatLike): 
        """在图像上绘制矩形框
        * img: 传入的二值化图像数据
        * 返回值：绘制矩形框后的图像数据，矩形框的坐标"""
        lst = []
        for cnt in ColorDetector._get_edge(img):										# 遍历轮廓数据
            x, y, w, h = cv2.boundingRect(cnt)						# 获取矩形框的坐标和宽高

            if (self.minarea > w*h or w*h > self.maxarea):
                continue
            if h/w > 1.5:
                continue
            cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 2)	# 在图像上绘制矩形框
            lst.append((x, y, x + w, y + h))
        return img, lst
    
    def draw_cyclic(self, img:cv2.typing.MatLike) -> tuple[cv2.typing.MatLike, list[tuple[int, int]]]:
        """在图像上绘制圆形
        * img: 传入的二值化图像数据
        * 返回值：绘制圆形后的图像数据，圆形的坐标(圆心，半径)"""
        # XXX:如果没有识别到原形怎么办，会返回什么
        lst = []
        for cnt in ColorDetector._get_edge(img):										# 遍历轮廓数据
            (x, y), radius = cv2.minEnclosingCircle(cnt)        # 获取最小外接圆的圆心坐标和半径
            center = (int(x), int(y))
            radius = int(radius)
            area = cv2.contourArea(cnt)
            if (self.minarea > area or area > self.maxarea):
                continue
            if radius < 10:     # 排除小圆
                continue
            cv2.circle(img, center, radius, (0, 255, 0), 2)
            lst.append((center, radius))
        return img, lst
    
    @staticmethod
    def _get_edge(img:cv2.typing.MatLike):
        """静态函数获取边缘
        * img: 传入的图像数据
        返回值：边缘图像"""
        # conuours是轮廓，hierarchy是轮廓的层级
        contours, _ = cv2.findContours(img, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        return contours
    
    def __del__(self):
        """析构函数"""
        cv2.destroyAllWindows()
        pass


class LineDetector(object):
    """直线识别器"""
    def __init__(self) -> None:
        self.minval = 0
        self.maxval = 255

    def __call__(self,id:int=0) -> None:
        cv2.namedWindow(f'trackbar{id}', cv2.WINDOW_NORMAL)
        cv2.createTrackbar('minval', f'trackbar{id}', self.minval, 255, self.minval_callback)
        cv2.createTrackbar('maxval', f'trackbar{id}', self.maxval, 255, self.maxval_callback)

    # region trackbar回调函数
    def minval_callback(self, x):
        self.minval = x

    def maxval_callback(self, x):
        self.maxval = x
    # endregion

    def get_angle(self, _img:cv2.typing.MatLike, ifdubug:bool=False) -> float|None:
        """获取直线的角度"""
        # 转换为灰度图
        gray = cv2.cvtColor(_img, cv2.COLOR_BGR2GRAY)
        # 使用Canny算法进行边缘检测
        edges = cv2.Canny(gray, self.minval, self.maxval, apertureSize=3)
        # 闭运算
        edges = cv2.morphologyEx(edges, cv2.MORPH_CLOSE, (5, 5))        # type:ignore
        # 膨胀
        edges = cv2.dilate(edges, (5, 5))                               # type:ignore
        cv2.imshow('edges', edges)
        # 使用霍夫变换检测直线
        lines = cv2.HoughLinesP(edges, 1, np.pi / 180, 200)

        if lines is not None:       # 如果检测到直线
            for rho, theta in lines[0]:
                # 计算直线的斜率
                a = np.cos(theta)
                b = np.sin(theta)
                # 计算直线的角度（以度为单位）
                angle = np.arctan2(b, a) * 180 / np.pi
                # 将识别的线画出来
                # cv2.line(_img, (0, 0), (int(a * 1000), int(b * 1000)), (0, 0, 255), 2)
                if ifdubug:
                    cv2.imshow('img', _img)
                return angle

        return None
    
    def get_distance(self, imt:cv2.typing.MatLike, y0:int=0, ifdebug:bool=False) -> int|None:
        """获取直线的距离
        * img: 传入的图像数据
        * y0: 基准点"""
        gray = cv2.cvtColor(imt, cv2.COLOR_BGR2GRAY)
        edges = cv2.Canny(gray, self.minval, self.maxval, apertureSize=3)
        # 闭运算
        edges = cv2.morphologyEx(edges, cv2.MORPH_CLOSE, (5, 5))        # type:ignore
        # 获取直线
        lines = cv2.HoughLinesP(edges, 1, np.pi / 180, 200)

        if lines is not None:
            for rho, theta in lines[0]:
                if ifdebug:
                    cv2.imshow('edges', edges)
                    # XXX:可能存在问题
                return rho-y0
        
    
class QRdetector(serial.Serial):
    def __init__(self) -> None:
        super().__init__('/dev/ttyAMA0', 9600, timeout=1)
        pass

    def qr_detect(self) -> list[str]|None:
        ori = self.read(8)
        if ori == b'':
            return None
        msg = ori[:-1].decode()
        lst = list(map(str, msg.split('+')))
        return lst


if __name__ == '__main__':
    #TODO: 测试代码
    line = LineDetector()
    cap = cv2.VideoCapture(0)

    # region 直线识别第一测试
    while True:
        img = cap.read()[1]
        angle = line.get_angle(img)
        if angle is not None:
            print(angle)
        cv2.imshow('img', img)
        if cv2.waitKey(1) == 27:
            break
    # endregion
        
    # region 直线识别第二测试
    # p1 = (0, 400)
    # p2 = (640, 400)
    # while True:
    #     img = cap.read()[1]
    #     angle = line.draw_and_get_angle_difference(img, p1, p2)
    #     if angle is not None:
    #         print(angle)
    #     cv2.imshow('img', img)
    #     if cv2.waitKey(1) == 27:
    #         break
    # endregion
    pass