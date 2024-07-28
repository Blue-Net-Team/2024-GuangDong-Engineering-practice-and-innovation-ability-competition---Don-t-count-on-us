# 2024年工创智慧物流搬运赛道——不要指望我们队

## 1. 项目简介

本项目是针对**2024年工程实践创新大赛 智慧物流搬运**的视觉识别代码

## 2. 小队成员

- 吉泧均——结构设计
- [林哲涵](https://gitee.com/purebrandy)——电控设计
- [何云峰](https://gitee.com/iven_he)——视觉识别

## 3. 文件说明

本项目中包含了一下主要文件

- `main.py`：项目主文件，封装了所有回应电控stm32的需求的函数
- `img_trans.py`:远程图传文件，其中类
  - `VideoStreaming`是用于传输图像的类，在树莓派上运行
  - `ReceiveImg`是用于接受远程图传图像的类，在本地电脑运行
- `detector.py`：存放识别器的文件，其中类
  - `ColorDetector`是用于识别颜色的颜色识别器
  - `LineDetector`是用于识别直线状态的识别器，包含识别直线角度以及直线距离
- `DEBUG.py`：用于调试的文件，其中类
  - `Debug`是用于调试的类，继承了主文件的类，可以在调试时使用，直接修改主文件的代码就可以在调试时使用
- `UART.py`：串口通信文件，其中类
  - `UART`是用于串口通信的类，继承了`pyserial`库的`Serial`类，添加了**包头包尾**的发送方式以及**整形数据**和**整形数组**的发送方式

## 4. 使用说明

本项目需要在本地电脑和树莓派上都克隆此项目

```bash
git clone https://gitee.com/blue-net-vision/physical-distribution-car.git
```

在远程调试过程中，在树莓派上运行`img_trans.py`文件打开图传服务器，需要注意的是

- **树莓派和本地电脑需要在同一个局域网下**
- **先运行`img_trans.py`文件，再运行`DEBUG.py`文件**
- **`img_trans,py`已经将相关IO输出(print)关闭，如果需要开启需要自行取消注释**

然后打开`DEBUG.py`文件，运行相关的阈值调试函数，在图传调试的时候，将DEBUG实例化的参数改为True;如果不使用图传摄像头，就将DEBUG实例化的参数改为False

### 调试的过程中会产生以下文件

- `R.json`：存放红色阈值的文件
- `G.json`：存放绿色阈值的文件
- `B.json`：存放蓝色阈值的文件
- `line.json`：存放直线canny阈值的文件
- `color_oc.json`：存放物料颜色识别的开闭运算参数的文件
- `line_oc.json`：存放直线识别的开闭运算参数的文件
- `circle1.json`：存放物料盘上夹取物料的基准圆信息的文件
- `radius.json`：在物料盘上夹取物料的时候的最小和最大外接圆识别半径

## 代码解读

在此代码中，以下代码是用于调整图像的范围的，即裁剪图像，过滤无关的区域，使得后续的识别更加准确。

```python
self.img = self.read_cap()
# [130:370, :]是裁剪图像的范围，需要根据具体情况调整
self.img = self.img[130:370, :]
```

### 1. `main.py`——在树莓派上直接运行的主文件

#### 1.1 region 参数加载

在参数加载的region区域，设置了各种识别过程中的参数，包括阈值，开闭运算参数，基准圆信息等。前两个以大写开头的字典视为常量字典，用于在后续的识别过程中使用。

#### 1.2 类Solution

这是一个包含代码顶层抽象逻辑的**解决方案类**，继承了`detector.py`文件中的`ColorDetector`和`LineDetector`类，使后续的代码更加灵活。以大写字母开头的函数在该类中属于最顶层接口，反之为次级接口。

在该类中，可以注意到`__init__`函数中有两个部分，一个是类自身的init，另一个是在`__init__`函数之外编写的`init_part1`,其中，`init_part1`函数是便于后续`DEBUG.py`文件中**调试类**继承时候的超类初始化。

`read_cap`函数是用于读取图传图像的函数，这个函数的设计是为了解决摄像头的ID号突然发生改变导致无法读取图像的问题，并且返回值不再包含`cv2.VideoCapture.read()`返回值中的bool标签，而是直接返回图像。

`send_msg`函数是用于发送串口数据的函数，只需要向里面传递整型、整型数组、字符串等数据即可以从串口发送对应的字符ascii码。其中，整型数组(list/tuple)是针对圆环坐标矫正的数据发送，整型是针对物料夹取信号和直线角度矫正的数据发送。此函数在初期框架搭建的时候具有缺陷，只能使用单一的包头包尾，否则需要针对不同的包头包尾设计不同的发送函数和修饰器。

`Detect_color`函数是用于识别颜色的函数，需要电控提供相关的颜色索引 (0红 1绿 2蓝),返回值是是否识别到这个颜色，只会返回True，如果没有匹配的颜色，会继续循环进行识别。在此处识别的逻辑是，先对图像进行hsv的图像二值化，然后将白色(符合颜色阈值的区域)区域做最小外接圆匹配(相关参数文件位于`radius.json`)，然后圈出的圆再与基准圆(相关参数文件位于`circle1.json`)进行比较，如果两个圆的重叠面积比例(以基准圆面积为分母)大于225行的0.48，就认为匹配到颜色。

`CORRECTION_angle`函数是用于矫正直线角度的，使用此函数需要确保摄像头的视野范围可以**看清楚**基准直线，如果看不清或者看不见的话就不会有返回数据。在此函数中，首先对图像进行canny边缘检测，然后对边缘检测的图像进行霍夫直线检测，最后对检测到的直线进行角度的计算，返回值是直线的角度。244行的90是基准直线的角度，这个数值取决于摄像头摆放的角度。

`LOCATECOLOR`函数是用于矫正色环的函数，使用此函数需要确保摄像头的视野范围可以**看清楚**圆环，如果看不清或者看不见的话就不会有返回数据，而是循环识别，知道识别到圆环。在此函数中，首先对图像进行hsv的图像二值化，进行相关的开闭运算(相关参数文件位于`circle_oc.json`)，然后对图像进行霍夫圆检测，这个过程循环10次，如果没识别到不计入，然后计算平均值，并且用基准圆(相关参数文件位于`circle2.json`)的圆心坐标做差，返回值是圆心坐标的差值。可以直接对基准圆文件进行修改，这样比较方便，其中，X调大的时候，物料摆放偏左， y调小的时候，物料摆放偏下

`__call__`是类的主函数调用，调用方法比较特殊，属于类的特殊函数

```python
# 用以下方法调用__call__函数
Solution()()
```

`circle_intersection_area`函数是用于计算两个圆的重叠面积的函数，用于在`Detect_color`函数中计算两个圆的重叠面积比例，传入尾缀为0的参数是第一个圆的圆心坐标和半径，传入尾缀为1的参数是第二个圆的圆心坐标和半径，返回值是两个圆的重叠面积。

### 2.`detector.py`——存放识别器的文件

本文件识别逻辑比较基础，霍夫算法检测查看csdn或者百度，此处不再解读

### 3.`DEBUG.py`——用于调试的文件

此类有4个顶层接口，用于外界直接调用，`SetCircleThresholds` `SetColorThresholds` `SetLineThresholds`分别用于设置圆环阈值，颜色阈值，直线阈值

底层函数接口简单，不再解读

### 4.`UART.py`——串口通信文件

此代码框架成型早，没有考虑太多问题，一开始将包头包尾封装进修饰器，然后发现不同的条件发送的数据包头包尾应该不同，，所以在后续代码中添加了过多的针对性修饰器和发送函数，导致代码冗余，不够简洁，但是功能齐全，可以直接使用。`read`函数比较完美，重写了父类`sreial`的`read`函数，可以通过包头包尾来读取不定长度的数据，但是每次只能读一个字节，对于`\r\n`的读取不够完美，需要后续优化。

## 获奖情况

实时是，此代码并未获奖，分析前后原因，鲁棒性不够高，并且代码冗余，不够简洁，后续需要优化。与此同时，应该将main文件中225行的最小重叠面积比例调小，以及`ciecle1.json`文件中的基准圆半径调大，这样可以提高识别的鲁棒性。在设备方面，升降机构的电机存在不明确的问题，有时候回出现堵转的情况，有时候回直接断电，导致整个设备无法正常运行，这个问题原因仍不明确。在后续的比赛中，视觉方面决定放弃hsv识别方案，转而使用深度学习的方案，这样可以提高识别的准确性，鲁棒性，以及适应性。
