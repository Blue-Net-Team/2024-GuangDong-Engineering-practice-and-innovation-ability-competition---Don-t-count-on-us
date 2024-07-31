#ifndef __MAIN_H
#define __MAIN_H

//内核
#include "sys.h"
#include "delay.h"

//外设
#include "usart.h"
#include "tim2.h"
#include "tim4.h"

//驱动
#include "stepping_motor_driver.h"
#include "digital_tube_driver.h"
#include "qr_code_driver.h"
#include "servo_driver.h"

//用户
#include "motion.h"

//c标准库
#include "stdlib.h"

//任务
void tast_loop(void);


#endif


