import cv2
import numpy as np


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

    def detect(self, img:cv2.typing.MatLike):
        """颜色识别
        * img: 传入的图像数据
        返回值：二值化过滤后的图像数据"""
        _shape = img.shape
        try:
            hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)				# 将BGR图像转换成HSV图像
        except:
            return np.zeros((_shape[0], _shape[1]), np.uint8)

        low = np.array([self.low_h, self.low_s, self.low_v])		# 低阈值
        high = np.array([self.high_h, self.high_s, self.high_v])	# 高阈值

        mask = cv2.inRange(hsv, low, high)						# 通过阈值过滤图像，将在阈值范围内的像素点设置为255，不在阈值范围内的像素点设置为0
        kernel = np.ones((5, 5), np.uint8)						# 创建一个5*5的矩阵，矩阵元素全为1
        opencal = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)	# 开运算，先腐蚀后膨胀
        # 对opencal进行腐蚀操作，去除噪声
        res = cv2.erode(opencal, kernel, iterations=1)
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
    
    def draw_cyclic(self, img:cv2.typing.MatLike):
        """在图像上绘制圆形
        * img: 传入的二值化图像数据
        * 返回值：绘制圆形后的图像数据，圆形的坐标(圆心，半径)"""
        lst = []
        for cnt in ColorDetector._get_edge(img):										# 遍历轮廓数据
            (x, y), radius = cv2.minEnclosingCircle(cnt)
            center = (int(x), int(y))
            radius = int(radius)
            if (self.minarea > radius*radius or radius*radius > self.maxarea):
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


class QRDetector(object):
    """二维码识别器"""
    def __init__(self) -> None:
        """初始化"""
        self.detector = cv2.QRCodeDetector()
        pass

    def __call__(self, img:cv2.typing.MatLike) -> str:
        """使用call方法调用识别器
        * img: 传入的图像数据
        返回值：二维码的数据"""
        # 识别二维码
        data, _, _ = self.detector.detectAndDecode(img)
        return data

    def __del__(self):
        """析构函数"""
        del self.detector
        pass


class LineDetector(object):
    """直线识别器"""
    def __init__(self) -> None:
        pass

    def get_angle(self, img:cv2.typing.MatLike) -> float|None:
        """获取直线的角度"""
        # 转换为灰度图
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        # 使用Canny算法进行边缘检测
        edges = cv2.Canny(gray, 50, 150, apertureSize=3)
        # 使用霍夫变换检测直线
        lines = cv2.HoughLines(edges, 1, np.pi / 180, 200)

        if lines is not None:
            for rho, theta in lines[0]:
                # 计算直线的斜率
                a = np.cos(theta)
                b = np.sin(theta)
                # 计算直线的角度（以度为单位）
                angle = np.arctan2(b, a) * 180 / np.pi
                return angle

        return None

    def draw_and_get_angle_difference(self, img:cv2.typing.MatLike, p1, p2) -> float|None:
        """在图像上绘制直线，并获取两条直线之间的角度"""
        # 在图像上绘制直线
        cv2.line(img, p1, p2, (0, 255, 0), 2)
        # 获取图像中的直线的角度
        angle1 = self.get_angle(img)
        # 计算绘制的直线的角度
        dx, dy = p2[0] - p1[0], p2[1] - p1[1]
        angle2 = np.arctan2(dy, dx) * 180 / np.pi
        # 返回两条直线之间的角度
        return abs(angle1 - angle2) if angle1 is not None else None