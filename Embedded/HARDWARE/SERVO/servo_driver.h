#ifndef __SERVO_DRIVER_H
#define __SERVO_DRIVER_H

#include "sys.h"
#include "usart.h"


//爪子角度
#define Claw_Open        150
#define Claw_Catch       228
#define Claw_Open_Small  205
#define Claw_Open_Big    130

//云台角度
#define Clound_Reset  268
#define Cloude_Work  	88

//货仓角度
#define Warehouse_1     	108
#define Warehouse_2     	10
#define Warehouse_3 			208
#define Warehouse_running 120


void servo_Init(int psc, int arr);
void ServoAngle(int chooseServo, double angle);

#endif
