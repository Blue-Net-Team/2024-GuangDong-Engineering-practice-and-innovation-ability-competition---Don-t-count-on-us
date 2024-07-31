#include "usart.h"
#include "stdlib.h"

//���ݰ�Э��
_usart_pack head;
_usart_pack tail;	

static _usart_pack color_package;
static _usart_pack circle_location_packge;
static _usart_pack Line_Angle_Packge;

static u8 color_data_packge[10] = {0};
static u8 circle_location_data_packge[20] = {0};
static u8 Line_Angle_Data_Packge[20] = {0};

//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
	USART3->DR = (u8) ch;      
	return ch;
}

//����2
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

//����3
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

//����4
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

//����5
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

//ԭ��ݮ�ɴ���
void USART1_IRQHandler(void)            
{
	char usart1_rec_data;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		usart1_rec_data = USART_ReceiveData(USART1);
		packet_scan(usart1_rec_data);
	}
}

//����ܴ���
void USART2_IRQHandler(void)            
{
	char usart2_rec_data;

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		usart2_rec_data = USART_ReceiveData(USART2);
		packet_scan(usart2_rec_data);
	}
}

//����3 ��ݮ�ɱ��ô���
void USART3_IRQHandler(void)            
{
	char usart3_rec_data;

	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		usart3_rec_data = USART_ReceiveData(USART3);
		packet_scan(usart3_rec_data);
		
//		while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
//		USART3->DR = (u8) usart3_rec_data;
	}
}

//������ƴ���
void UART4_IRQHandler(void)            
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		USART_ReceiveData(UART4);
	}
}

//ɨ��ģ�鴮��
void UART5_IRQHandler(void)            
{
	
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
	{
		u8 scan_data = USART_ReceiveData(UART5);
		rec_qr_code_data(scan_data);
	}
}

//���ڰ���ʼ��
void package_Init()
{
	head.next_usart_pack = &tail;
	tail.next_usart_pack = &head;

	//��ɫ���ݰ�
	color_package.package_header = COLOR_PACKAGE_HEADER;
	color_package.package_tail = COLOR_PACKAGE_TAIL;
	color_package.packet = color_data_packge;
	color_package.package_flag = rec_unfinish;
	create_usart_package(&color_package);

	//Բ�����ݰ�
	circle_location_packge.package_header = CIRCLE_PACKAGE_HEADER;
	circle_location_packge.package_tail = CIRCLE_PACKAGE_TAIL;
	circle_location_packge.packet = circle_location_data_packge;
	circle_location_packge.package_flag = rec_unfinish;
	create_usart_package(&circle_location_packge);
	
	//ֱ�����ݰ�
	Line_Angle_Packge.package_header = LINE_PACKAGE_HEADER;
	Line_Angle_Packge.package_tail = LINE_PACKAGE_TAIL;
	Line_Angle_Packge.packet = Line_Angle_Data_Packge;
	Line_Angle_Packge.package_flag = rec_unfinish;
	create_usart_package(&Line_Angle_Packge);
}

//��ԭԲ�İ����ձ�־λ
void reset_circle_location_packge(void)
{
	circle_location_packge.package_flag = rec_finish;	
}

//��ԭֱ�߰����ձ�־λ
void Reset_Line_Angle_Packge(void)
{
	Line_Angle_Packge.package_flag = rec_finish;	
}

//����һ�����ڰ�
void create_usart_package(_usart_pack* packger)
{
	_usart_pack* p_packger;

	if (head.next_usart_pack == &tail)	//�ʼ��ʼ��
	{
		head.next_usart_pack = packger;
		packger->next_usart_pack = &tail;
	}
	else
	{
		for (p_packger = &head; p_packger->next_usart_pack != &tail; p_packger = p_packger->next_usart_pack);	//Ѱַ

		p_packger->next_usart_pack = packger;//����
		packger->next_usart_pack = &tail;
	}
}

//ɨ�贮�����ݣ���ɨ�赽���������
void packet_scan(u8 rec_data)
{
	static _usart_pack* p_packger;
	static u8 p_rec_data = 0, rec_flag = 0;
	
	if (rec_flag == 0)	//�����������ݰ���ͷ
	{
		p_packger = head.next_usart_pack;

		while (p_packger->package_header != rec_data)
		{
			p_packger = p_packger->next_usart_pack;
			if (p_packger == &tail)	return;		//û�м�⵽���ݰ���ֱ���˳�����
		}

		p_packger->package_flag = rec_unfinish;	//�ҵ�����ͷ���л�״̬Ϊ������
		rec_flag = 1;	 //�رռ������ݰ���ͷ
	}
	else if (p_packger->package_tail == rec_data)		//������β
	{
		//��λ����
		rec_flag = p_rec_data = 0;
		p_packger->package_flag = rec_finish;
	}
	else if(rec_flag == 1)		//����״̬�洢���ݰ�
	{
		p_packger->packet[p_rec_data] = rec_data;
		p_rec_data++;
	}

}

//��ȡ��ɫ����
u8* get_color_data()
{
	if(color_package.package_flag == rec_finish) 
	{
		color_package.package_flag = rec_unfinish;
		return color_package.packet;
	}
	else return NULL;
}

//��ȡԲ��λ������
u8* get_circle_location_data()
{
	if(circle_location_packge.package_flag == rec_finish)
	{
		circle_location_packge.package_flag = rec_unfinish;
		return circle_location_packge.packet;
	}
	else return NULL;
}

//��ȡֱ�߽Ƕ�����
u8* Get_Line_Angle_data()
{
	if(Line_Angle_Packge.package_flag == rec_finish)
	{
		Line_Angle_Packge.package_flag = rec_unfinish;
		return Line_Angle_Packge.packet;
	}
	else return NULL;
}

//������ݮ���� ��ͷ��β��Ϊ @ �� # 
void SendByte(char chose, u8 command)
{
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
	USART3->DR = '@';
	
	if(chose == 'c')
	{
		while((USART3->SR&0X40)==0);  USART3->DR = 'c';
		while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
		USART3->DR = command; 	
	}
	else if(chose == 'C')
	{
		while((USART3->SR&0X40)==0);  USART3->DR = 'C';
		while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
		USART3->DR = command; 
	}
	else if(chose == 'n')
	{
		while((USART3->SR&0X40)==0);  USART3->DR = command;
	}
	
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
	USART3->DR = '#';
}

//Բ�İ�����
int* circle_location_decoding(void)
{
	static int out_circle_location_data[2];
  static u8* circle_location_decoding_data = NULL;

  circle_location_decoding_data = get_circle_location_data();

  if(circle_location_decoding_data != NULL) 
  {
  
			//X����
      int sign = (circle_location_decoding_data[0] == '0') ? 1 : -1;
      out_circle_location_data[0] = sign * ((circle_location_decoding_data[1] - 48 ) * 100
                                      + (circle_location_decoding_data[2] - 48 )*10
																			+ (circle_location_decoding_data[3] - 48 ));

      //Y����
      sign = (circle_location_decoding_data[4] == '0') ? 1 : -1;
      out_circle_location_data[1] = sign * ((circle_location_decoding_data[5] - 48 ) * 10
                                      + (circle_location_decoding_data[6] - 48 )
																			+ (circle_location_decoding_data[7] - 48 ));

      return out_circle_location_data;
  }
	else return NULL;
}

//ֱ�߽ǶȽ���
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
