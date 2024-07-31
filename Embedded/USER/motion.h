#ifndef __MOTION_H
#define __MOTION_H

#include "stepping_motor_driver.h"
#include "servo_driver.h"


//������Ƹ߶�
#define Stack_Height  2700
#define Placement_Height  5350
#define Normal_Height  2600
#define Material_Height  2300
#define Put_Height 2200
#define Camera_Height  0
#define Line_Height  1000  //ֱ�߽�������ͷ�߶�

//Բ����������
#define circle_dead_area       	3
#define circle_second_area      10
#define circle_thrid_area       20

//ֱ����������
#define Line_dead_area 1
#define Line_second_area 3

//��ȡ���Ʒ/��Ʒ���ķ���;���
int moving(char location);
int* get_way(char location);

//��λ
void location_task(char RGB);
void Line_task(void);

//����ץȡ����
void Claw_Catch_material(void);
void PutOn_Ground(void);
void Catch_From_Ground(void);
void Stacking_Material(void);

#endif
