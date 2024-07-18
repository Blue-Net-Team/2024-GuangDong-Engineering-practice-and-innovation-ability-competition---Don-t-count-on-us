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
