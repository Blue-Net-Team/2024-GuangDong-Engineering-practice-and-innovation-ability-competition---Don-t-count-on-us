#include "stepping_motor_driver.h"


//串口控制步进电机初始化
void usart_ctrl_steeping_motor_Init()
{
	uart4_init(115200);										//底盘电机控制串口
}

//底盘电机控制函数
void usart_ctrl_chassis_motor(steeping_motor motor, char speed_or_distence)
{
	if(speed_or_distence == 0)	//距离模式
	{
		char ctrl_location_arr[13] = {	motor.motor_ID, 				//地址
										0xfd, 							//指令码
										motor.motor_direction, 			//方向
										motor.motor_speed >> 8, 		//速度高8位
										motor.motor_speed, 				//速度低8位
										motor.motor_acc_speed,			//加速度档位
										motor.motor_pulse_num >> 24,
										motor.motor_pulse_num >> 16, 	//脉冲数高16位
										motor.motor_pulse_num >> 8,
										motor.motor_pulse_num , 		//脉冲数低16位
										0x00,							//相对，绝对位置
										motor.synchronous,				//多机同步
										0x6b};							//校验

		for(char i = 0; i < 13; i++)
		{
			while((UART4->SR&0X40)==0);
			UART4->DR = ctrl_location_arr[i];
		}
	}
	else if(speed_or_distence == 1)
	{
		char ctrl_speed_arr[8] = {	motor.motor_ID, 					//地址
									0xf6, 								//指令码
									motor.motor_direction, 				//方向
									motor.motor_speed >> 8, 			//速度高8位
									motor.motor_speed, 					//速度低8位
									motor.motor_acc_speed, 				//加速度
									motor.synchronous, 					//多机同步
									0x6b};								//校验

		for(char i = 0; i < 8; i++)
		{
			while((UART4->SR&0X40)==0);
			UART4->DR = ctrl_speed_arr[i];
		}
	}
	vTaskDelay(1);
}

//底盘电机同步函数
void usart_ctrl_chassis_motor_synchronous()
{
	char synchronous_move[4] = {0x00, 0xff, 0x66, 0x6b};
	
	for(char i = 0; i < 4; i++)
	{
		while((UART4->SR&0X40)==0);
		UART4->DR = synchronous_move[i];    
	}

	vTaskDelay(1);
}

//设置回零位置ID1设置回零
void set_gimbal_motor_zero(steeping_motor motor)
{
	char set_zero[5] = {motor.motor_ID, 0x93, 0x88, 0x01, 0x6b};//设置回零指令			

	for(char i = 0; i < 5; i++)
	{
		while((USART3->SR&0X40)==0);
		USART3->DR = set_zero[i];    
	}	
}

//触发回零
void gimbal_motor_return_zero(steeping_motor motor)
{
	char set_zero[5] = {motor.motor_ID, 0x9a, 0x00, 0x00, 0x6b};//触发回零指令	

	for(char i = 0; i < 5; i++)
	{
		while((USART3->SR&0X40)==0);
		USART3->DR = set_zero[i];    
	}
	vTaskDelay(1);
}

//------------不带RTOS的方向控制----------------
static steeping_motor MOTOR[4];

void motor_soft_Init()
{
	for(char i = 0;i < 4; i++) 
	{
		MOTOR[i].motor_ID = i + 1;			//电机ID
		MOTOR[i].motor_acc_speed = 200;		//默认电机加速度
		MOTOR[i].synchronous = 0;
	}

	//电机直行方向
	MOTOR[0].motor_direction = 1;
	MOTOR[1].motor_direction = 0;
	MOTOR[2].motor_direction = 1;
	MOTOR[3].motor_direction = 0;
}

//解算速度
void change_xyw_speed_no_quene(int x, int y, int w)
{
	//速度融合
	int motor_out_speed[4];

	motor_out_speed[0] = x - y - w;
	motor_out_speed[1] = x + y + w;
	motor_out_speed[2] = x + y - w;
	motor_out_speed[3] = x - y + w;

	//方向变量复位
	MOTOR[0].motor_direction = MOTOR[2].motor_direction = 1;
	MOTOR[1].motor_direction = MOTOR[3].motor_direction = 0;

	for(char i = 0; i<4; i++) 
	{
		if(motor_out_speed[i] < 0)//方向未负，改变方向和速度变为正数
		{
			MOTOR[i].motor_direction = !MOTOR[i].motor_direction;	
			motor_out_speed[i] = motor_out_speed[i] * -1;
		}
	}

	//写入
	for(char i = 0; i<4; i++) 
	{
		MOTOR[i].motor_speed = motor_out_speed[i];
		usart_ctrl_chassis_motor(MOTOR[i], 1);
	}

	usart_ctrl_chassis_motor_synchronous();

}

//脉冲控制初始化
void MOTOR_UP_Down_Init(void) 
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); 	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;   //方向	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	//配置PE9为步进（STP）信号
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);

  TIM_Cmd(TIM1, ENABLE);
}

//方向选择
void Cloud_Turn(uint16_t dir)
{
			if(dir)
      {
				GPIO_ResetBits(GPIOE,GPIO_Pin_0);
			}
			else
			{
				GPIO_SetBits(GPIOE,GPIO_Pin_0);					
			}
}

//脉冲升降控制
void Claw_Up_Down(uint16_t dir,uint16_t dis)//0下降 1上升
{
		Cloud_Turn(dir);
		for(int i = 0; i < dis; i++)
        {
            GPIO_SetBits(GPIOE, GPIO_Pin_9);
            delay_us(50);
            GPIO_ResetBits(GPIOE, GPIO_Pin_9);
            delay_us(50);
        }
    delay_ms(500);
}
