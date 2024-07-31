#include "usart.h"
#include "stdlib.h"

//数据包协议
_usart_pack head;
_usart_pack tail;	

static _usart_pack color_package;
static _usart_pack circle_location_packge;
static _usart_pack Line_Angle_Packge;

static u8 color_data_packge[10] = {0};
static u8 circle_location_data_packge[20] = {0};
static u8 Line_Angle_Data_Packge[20] = {0};

//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
	USART3->DR = (u8) ch;      
	return ch;
}

//串口2
void usart2_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	USART_DeInit(USART2); 
	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2); 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(USART2, &USART_InitStructure);
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
	
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
}

//串口3
void usart3_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	USART_DeInit(USART3); 
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3); 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(USART3, &USART_InitStructure);
	
	USART_ClearFlag(USART3, USART_FLAG_TC);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE);
	
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
}

//串口4
void uart4_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	
	USART_DeInit(UART4); 

	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_UART4); 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(UART4, &USART_InitStructure);
	
	USART_ClearFlag(UART4, USART_FLAG_TC);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART4, ENABLE);
	
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
}

//串口5
void uart5_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	
	USART_DeInit(UART5); 

	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_UART5);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_UART5); 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(UART5, &USART_InitStructure);
	
	USART_ClearFlag(UART5, USART_FLAG_TC);
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART5, ENABLE);
	
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
}

//原树莓派串口
void USART1_IRQHandler(void)            
{
	char usart1_rec_data;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		usart1_rec_data = USART_ReceiveData(USART1);
		packet_scan(usart1_rec_data);
	}
}

//数码管串口
void USART2_IRQHandler(void)            
{
	char usart2_rec_data;

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		usart2_rec_data = USART_ReceiveData(USART2);
		packet_scan(usart2_rec_data);
	}
}

//串口3 树莓派备用串口
void USART3_IRQHandler(void)            
{
	char usart3_rec_data;

	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		usart3_rec_data = USART_ReceiveData(USART3);
		packet_scan(usart3_rec_data);
		
//		while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
//		USART3->DR = (u8) usart3_rec_data;
	}
}

//电机控制串口
void UART4_IRQHandler(void)            
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		USART_ReceiveData(UART4);
	}
}

//扫码模块串口
void UART5_IRQHandler(void)            
{
	
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
	{
		u8 scan_data = USART_ReceiveData(UART5);
		rec_qr_code_data(scan_data);
	}
}

//串口包初始化
void package_Init()
{
	head.next_usart_pack = &tail;
	tail.next_usart_pack = &head;

	//颜色数据包
	color_package.package_header = COLOR_PACKAGE_HEADER;
	color_package.package_tail = COLOR_PACKAGE_TAIL;
	color_package.packet = color_data_packge;
	color_package.package_flag = rec_unfinish;
	create_usart_package(&color_package);

	//圆心数据包
	circle_location_packge.package_header = CIRCLE_PACKAGE_HEADER;
	circle_location_packge.package_tail = CIRCLE_PACKAGE_TAIL;
	circle_location_packge.packet = circle_location_data_packge;
	circle_location_packge.package_flag = rec_unfinish;
	create_usart_package(&circle_location_packge);
	
	//直线数据包
	Line_Angle_Packge.package_header = LINE_PACKAGE_HEADER;
	Line_Angle_Packge.package_tail = LINE_PACKAGE_TAIL;
	Line_Angle_Packge.packet = Line_Angle_Data_Packge;
	Line_Angle_Packge.package_flag = rec_unfinish;
	create_usart_package(&Line_Angle_Packge);
}

//复原圆心包接收标志位
void reset_circle_location_packge(void)
{
	circle_location_packge.package_flag = rec_finish;	
}

//复原直线包接收标志位
void Reset_Line_Angle_Packge(void)
{
	Line_Angle_Packge.package_flag = rec_finish;	
}

//创建一个串口包
void create_usart_package(_usart_pack* packger)
{
	_usart_pack* p_packger;

	if (head.next_usart_pack == &tail)	//最开始初始化
	{
		head.next_usart_pack = packger;
		packger->next_usart_pack = &tail;
	}
	else
	{
		for (p_packger = &head; p_packger->next_usart_pack != &tail; p_packger = p_packger->next_usart_pack);	//寻址

		p_packger->next_usart_pack = packger;//插入
		packger->next_usart_pack = &tail;
	}
}

//扫描串口数据，若扫描到包则存下来
void packet_scan(u8 rec_data)
{
	static _usart_pack* p_packger;
	static u8 p_rec_data = 0, rec_flag = 0;
	
	if (rec_flag == 0)	//检索所有数据包包头
	{
		p_packger = head.next_usart_pack;

		while (p_packger->package_header != rec_data)
		{
			p_packger = p_packger->next_usart_pack;
			if (p_packger == &tail)	return;		//没有检测到数据包，直接退出函数
		}

		p_packger->package_flag = rec_unfinish;	//找到数据头，切换状态为接收中
		rec_flag = 1;	 //关闭检索数据包包头
	}
	else if (p_packger->package_tail == rec_data)		//检索包尾
	{
		//复位操作
		rec_flag = p_rec_data = 0;
		p_packger->package_flag = rec_finish;
	}
	else if(rec_flag == 1)		//其他状态存储数据包
	{
		p_packger->packet[p_rec_data] = rec_data;
		p_rec_data++;
	}

}

//获取颜色数据
u8* get_color_data()
{
	if(color_package.package_flag == rec_finish) 
	{
		color_package.package_flag = rec_unfinish;
		return color_package.packet;
	}
	else return NULL;
}

//获取圆心位置数据
u8* get_circle_location_data()
{
	if(circle_location_packge.package_flag == rec_finish)
	{
		circle_location_packge.package_flag = rec_unfinish;
		return circle_location_packge.packet;
	}
	else return NULL;
}

//获取直线角度数据
u8* Get_Line_Angle_data()
{
	if(Line_Angle_Packge.package_flag == rec_finish)
	{
		Line_Angle_Packge.package_flag = rec_unfinish;
		return Line_Angle_Packge.packet;
	}
	else return NULL;
}

//呼叫树莓先生 包头包尾均为 @ 和 # 
void SendByte(char chose, u8 command)
{
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
	USART3->DR = '@';
	
	if(chose == 'c')
	{
		while((USART3->SR&0X40)==0);  USART3->DR = 'c';
		while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
		USART3->DR = command; 	
	}
	else if(chose == 'C')
	{
		while((USART3->SR&0X40)==0);  USART3->DR = 'C';
		while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
		USART3->DR = command; 
	}
	else if(chose == 'n')
	{
		while((USART3->SR&0X40)==0);  USART3->DR = command;
	}
	
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
	USART3->DR = '#';
}

//圆心包解码
int* circle_location_decoding(void)
{
	static int out_circle_location_data[2];
  static u8* circle_location_decoding_data = NULL;

  circle_location_decoding_data = get_circle_location_data();

  if(circle_location_decoding_data != NULL) 
  {
  
			//X坐标
      int sign = (circle_location_decoding_data[0] == '0') ? 1 : -1;
      out_circle_location_data[0] = sign * ((circle_location_decoding_data[1] - 48 ) * 100
                                      + (circle_location_decoding_data[2] - 48 )*10
																			+ (circle_location_decoding_data[3] - 48 ));

      //Y坐标
      sign = (circle_location_decoding_data[4] == '0') ? 1 : -1;
      out_circle_location_data[1] = sign * ((circle_location_decoding_data[5] - 48 ) * 10
                                      + (circle_location_decoding_data[6] - 48 )
																			+ (circle_location_decoding_data[7] - 48 ));

      return out_circle_location_data;
  }
	else return NULL;
}

//直线角度解码
int* Line_Angle_decoding(void)
{
		static int out_Line_Angle_data[2];
		static u8* Line_Angle_decoding_data = NULL;
	
		Line_Angle_decoding_data = Get_Line_Angle_data();
	
		if(Line_Angle_decoding_data != NULL)
	{
		int sign = (Line_Angle_decoding_data[0] == '0') ? -1 : 1;
      out_Line_Angle_data[0] = (sign * ((Line_Angle_decoding_data[1] - 48 ) * 10
                                      + (Line_Angle_decoding_data[2] - 48))) + 0;
		return out_Line_Angle_data;
	}	
		return NULL;
}
