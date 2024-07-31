#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include <ctype.h>
#include "string.h"
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "qr_code_driver.h"


//接收标志位
#define rec_finish      1
#define rec_unfinish    0

//数据包头包尾
#define COLOR_PACKAGE_HEADER '@'
#define COLOR_PACKAGE_TAIL '#'

#define CIRCLE_PACKAGE_HEADER 'Q'
#define CIRCLE_PACKAGE_TAIL 'E'

#define LINE_PACKAGE_HEADER 'L'
#define LINE_PACKAGE_TAIL '%'


typedef struct usart_pack
{
    u8 package_header;  //包头
    u8 package_tail;    //包尾
		u8* packet;         //包
    u8 package_flag;    //数据包标志位 rec_finish：接收完成  rec_unfinish：接收中

    struct usart_pack *next_usart_pack; //指向下一个数据包

}_usart_pack;


//初始化
void usart1_init(u32 bound);
void usart2_init(u32 bound);
void usart3_init(u32 bound);
void uart4_init(u32 bound);
void uart5_init(u32 bound);

//数据包相关函数
void package_Init(void);
void create_usart_package(_usart_pack* packger);
void packet_scan(u8 rec_data);
void reset_circle_location_packge(void);
void Reset_Line_Angle_Packge(void);

//从树莓派获取数据
u8* get_color_data(void);
u8* get_circle_location_data(void);
u8* Get_Line_Angle_data(void);

//呼叫树莓牢底
void SendByte(char chose, u8 command);

//转换串口字符数据包
int* circle_location_decoding(void);
int* Line_Angle_decoding(void);
extern uint8_t Serial_RxPacket[4];	

#endif


