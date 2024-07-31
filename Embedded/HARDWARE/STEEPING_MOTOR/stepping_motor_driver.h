#ifndef __STEPPING_MOTOR_DRIVER
#define __STEPPING_MOTOR_DRIVER

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"
#include "math.h"


//�Ŵ�ͷ����������ں�����
#define scan_time   10   //�ٶ���Ӧʱ��
#define x_correct	0
#define y_correct	0
#define w_correct	0

typedef struct 
{
	char motor_ID;			//���ID
	char motor_direction;	//����˶�����
	char synchronous;		//ͬ���趨
	int motor_speed;		//�ٶ�
	int motor_acc_speed;	//���ٶ�
	int motor_pulse_num;	//��������
}
steeping_motor;


/*���̵������*/
void usart_ctrl_steeping_motor_Init(void);
void usart_ctrl_chassis_motor(steeping_motor motor, char speed_or_distence);
void usart_ctrl_chassis_motor_synchronous(void);

/*�ٶȽ���*/
void speed_mix(void);
void speed_x(void);
void speed_y(void);
void speed_w(void);
void change_xyw_speed(float x, float y, float w);

void motor_soft_Init(void);
void change_xyw_speed_no_quene(int x, int y, int w);

//������Ʋ������
void MOTOR_UP_Down_Init(void);
void Cloud_Turn(uint16_t dir);
void Claw_Up_Down(uint16_t dir,uint16_t dis);//0�½� 1����

#endif
