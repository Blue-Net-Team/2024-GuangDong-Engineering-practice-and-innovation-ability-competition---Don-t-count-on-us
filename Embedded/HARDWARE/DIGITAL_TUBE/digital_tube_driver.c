#include "digital_tube_driver.h"
#include "usart.h"

//数码管显示模块初始化
void digital_tube_Init()
{
    usart2_init(115200);
}

//数码管显示
//协议格式(xxxxxx)
void digital_tube_Display(char* disp_arr)
{
    while((USART2->SR&0X40)==0);
    USART2->DR = '(';    

    for(char i = 0; i < 6; i++)
	{
		while((USART2->SR&0X40)==0);
		USART2->DR = disp_arr[i];    
	}

    while((USART2->SR&0X40)==0);
    USART2->DR = ')';    
}
