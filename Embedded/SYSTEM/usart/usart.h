#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include <ctype.h>
#include "string.h"
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "qr_code_driver.h"


//���ձ�־λ
#define rec_finish      1
#define rec_unfinish    0

//���ݰ�ͷ��β
#define COLOR_PACKAGE_HEADER '@'
#define COLOR_PACKAGE_TAIL '#'

#define CIRCLE_PACKAGE_HEADER 'Q'
#define CIRCLE_PACKAGE_TAIL 'E'

#define LINE_PACKAGE_HEADER 'L'
#define LINE_PACKAGE_TAIL '%'


typedef struct usart_pack
{
    u8 package_header;  //��ͷ
    u8 package_tail;    //��β
		u8* packet;         //��
    u8 package_flag;    //���ݰ���־λ rec_finish���������  rec_unfinish��������

    struct usart_pack *next_usart_pack; //ָ����һ�����ݰ�

}_usart_pack;


//��ʼ��
void usart1_init(u32 bound);
void usart2_init(u32 bound);
void usart3_init(u32 bound);
void uart4_init(u32 bound);
void uart5_init(u32 bound);

//���ݰ���غ���
void package_Init(void);
void create_usart_package(_usart_pack* packger);
void packet_scan(u8 rec_data);
void reset_circle_location_packge(void);
void Reset_Line_Angle_Packge(void);

//����ݮ�ɻ�ȡ����
u8* get_color_data(void);
u8* get_circle_location_data(void);
u8* Get_Line_Angle_data(void);

//������ݮ�ε�
void SendByte(char chose, u8 command);

//ת�������ַ����ݰ�
int* circle_location_decoding(void);
int* Line_Angle_decoding(void);
extern uint8_t Serial_RxPacket[4];	

#endif


