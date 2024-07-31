#ifndef __STEPPING_MOTOR_DRIVER
#define __STEPPING_MOTOR_DRIVER

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"
#include "math.h"


//张大头步进电机串口函数库
#define scan_time   10   //速度响应时间
#define x_correct	0
#define y_correct	0
#define w_correct	0

typedef struct 
{
	char motor_ID;			//电机ID
	char motor_direction;	//电机运动方向
	char synchronous;		//同步设定
	int motor_speed;		//速度
	int motor_acc_speed;	//加速度
	int motor_pulse_num;	//总脉冲数
}
steeping_motor;


/*底盘电机设置*/
void usart_ctrl_steeping_motor_Init(void);
void usart_ctrl_chassis_motor(steeping_motor motor, char speed_or_distence);
void usart_ctrl_chassis_motor_synchronous(void);

/*速度解算*/
void speed_mix(void);
void speed_x(void);
void speed_y(void);
void speed_w(void);
void change_xyw_speed(float x, float y, float w);

void motor_soft_Init(void);
void change_xyw_speed_no_quene(int x, int y, int w);

//脉冲控制步进电机
void MOTOR_UP_Down_Init(void);
void Cloud_Turn(uint16_t dir);
void Claw_Up_Down(uint16_t dir,uint16_t dis);//0下降 1上升

#endif
