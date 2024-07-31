#include "main.h"

/********************
	声明：
	绝大部分移植于gitee蓝网电控库 "@D2zling"
	具体详细参照gitee蓝网电控库“2024工创电控代码”
*******************/

/*
r"""
*********************************************
? ? ? ? ? ? ? ? ? ?_ooOoo_
? ? ? ? ? ? ? ? ? o8888888o
? ? ? ? ? ? ? ? ? 88" . "88
? ? ? ? ? ? ? ? ? (| -_- |)
? ? ? ? ? ? ? ? ? O\ ?= ?/O
? ? ? ? ? ? ? ?____/`---'\____
? ? ? ? ? ? ?.' ?\\| ? ? |// ?`.
? ? ? ? ? ? / ?\\||| ?: ?|||// ?\
? ? ? ? ? ?/ ?_||||| -:- |||||- ?\
? ? ? ? ? ?| ? | \\\ ?- ?/// | ? |
? ? ? ? ? ?| \_| ?''\---/'' ?| ? |
? ? ? ? ? ?\ ?.-\__ ?`-` ?___/-. /
? ? ? ? ?___`. .' ?/--.--\ ?`. . __
? ? ? ."" '&lt; ?`.___\_&lt;|>_/___.' ?>'"".
? ? ?| | : ?`- \`.;`\ _ /`;.`/ - ` : | |
? ? ?\ ?\ `-. ? \_ __\ /__ _/ ? .-` / ?/
======`-.____`-.___\_____/___.-`____.-'======
? ? ? ? ? ? ? ? ? ?`=---='

? ? ? ? ? ? 佛祖保佑 ? ? ? 工创省一
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
"""
*/

/*
舵机      PD12	   PD13	   PD14	   
通道	    CH1	     CH2		 CH3		   
对应      爪子     货舱    云台
ServoNum   0        1       2
*/

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);			//设置系统中断优先级分组4
	delay_init(168);										  //延时初始化
	usart3_init(9600);										//树莓派通信端口
	scan_qr_code_Init();									//扫码初始化
	digital_tube_Init();									//数码管模块初始化
	MOTOR_UP_Down_Init();									//脉冲控制升降爪初始化
	usart_ctrl_steeping_motor_Init();			//串口控制步进电机初始化
	servo_Init(20000 - 1, 84 - 1);				//舵机控制初始化

	package_Init();											  //串口数据包软件初始化
	motor_soft_Init();										//电机软件初始化
	
	//舵机启动复位
	ServoAngle(0,Claw_Open); //150 228 205 130 Claw_Open Claw_Catch Claw_Open_Small Claw_Open_Big
	ServoAngle(1,Warehouse_running); //108 10 208 120 Warehouse_1 Warehouse_2 Warehouse_3 Warehouse_running
	ServoAngle(2,Clound_Reset); //90 268  Cloude_Work  Clound_Reset
}

void tast_loop(void)
{
  static char* qr_data = NULL;						//扫码数据包
	static int* move = NULL;							//半成品/成品区动作
	static u8 *color_data = NULL;						//颜色数据

	char i;												//计数变量
	int angle_warehouse = 108;
	
	delay_ms(2000);	//等待电机启动
	                                                                                                                                                  ;
	while(1)
	{

//		ServoAngle(2,Cloude_Work);
					
//			while(1);
		
		/*********第一轮**********/
		{	//**前往原料区**//
		change_xyw_speed_no_quene(0,-91,0);//-90
		delay_ms(300);

		change_xyw_speed_no_quene(157,0,0);
		delay_ms(1200);
		
		change_xyw_speed_no_quene(0,0,0);
		delay_ms(1000);
		
		while(qr_data == NULL) qr_data = get_qr_code_data();//获取二维码并显示
		digital_tube_Display(qr_data);

		delay_ms(200);
			
		ServoAngle(0,Claw_Open);//爪子开始工作
		ServoAngle(2,Cloude_Work);//云台开始工作
		
		change_xyw_speed_no_quene(146,26,0);
		delay_ms(1200);

		change_xyw_speed_no_quene(0,0,0);
		delay_ms(100);
		
		delay_ms(500);
		}
					
		//**一轮获取物料**//
		for(i = 0;i <= 2; i++)
		{
			SendByte('c',qr_data[i]-1);//呼叫树莓先生识别颜色
			//调试：可注释掉假装识别到
			while(1)
			{
				color_data = get_color_data();
				if(color_data[0] == '1')
				{
					color_data = NULL;
					break;
				}
			}
			//按顺序决定货仓舵机角度
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			if(i <= 1)
			{
				Claw_Catch_material();//获取原料
				delay_ms(500);
			}
			if(i == 2)
			{
					Claw_Up_Down(0,Material_Height);//下降
					ServoAngle(0,Claw_Catch);//抓取角度：225
					delay_ms(300);
					ServoAngle(2,Clound_Reset);//云台旋转放置：270
					Claw_Up_Down(1,Material_Height);//升高
					Claw_Up_Down(0,Put_Height);//下降
					ServoAngle(0,Claw_Open_Small);//松开小角度：200
					delay_ms(100);
					Claw_Up_Down(1,Put_Height);//升高
					ServoAngle(0,Claw_Open);
					//放置完最后一个物料后，边走边复位云台
					change_xyw_speed_no_quene(132,0,-65);ServoAngle(1,Warehouse_running);
					delay_ms(1200);
					change_xyw_speed_no_quene(0,0,0);ServoAngle(2,Cloude_Work);
					delay_ms(600);
			}

		}

		{//**一轮物料盘到暂存区**//

			if(qr_data[0] == '1')//根据二维码数字决定先到哪个位置
			{	
				change_xyw_speed_no_quene(100,0,0);
				delay_ms(1100);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			else if(qr_data[0] == '2')
			{	
				change_xyw_speed_no_quene(100,0,0);
				delay_ms(1400);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			else if(qr_data[0] == '3') 	
			{
				change_xyw_speed_no_quene(100,0,0);
				delay_ms(1800);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
		
		}
		
		//**一轮半成品区放置三个物料**//
		for(char i = 0; i <= 2; i++)
		{
			move = get_way(qr_data[i]);//获取下一个目标
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//降下摄像头
			vTaskDelay(500);
			
			for(char j =0; j<10; j++)//刷新串口数据
			{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//树莓先生的色环矫正环节

			Claw_Up_Down(1,Camera_Height);//升起摄像头
			delay_ms(500);
			
			//按顺序决定货仓舵机角度
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			PutOn_Ground();//放置物料到地上	

		}

		//**半成品区捡起三个物料**//
		for(char i = 0; i < 3; i++)
		{
			move = get_way(qr_data[i]);		//获取下一个目标
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			//按顺序决定货仓舵机角度
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203 
			
			Catch_From_Ground();//从地面取回物料放回货仓
			
			delay_ms(500);
		}
		
		ServoAngle(1,Warehouse_running);

		{//**暂存区到成品区**//
			change_xyw_speed_no_quene(0,-100,0);//-90
			delay_ms(200);
			change_xyw_speed_no_quene(0,0,0);//-90
			delay_ms(300);
			
			
			//根据二维码数字补回距离
			if(qr_data[2] == '1')  
			{change_xyw_speed_no_quene(100,0,0);delay_ms(670);}
			else if(qr_data[2] == '2') 
			{change_xyw_speed_no_quene(100,0,0);delay_ms(335);}
			else if(qr_data[2] == '3') 
			{change_xyw_speed_no_quene(0,0,0);delay_ms(100);}

			change_xyw_speed_no_quene(0,0,0);delay_ms(500);
			
			change_xyw_speed_no_quene(133,0,-65);
			delay_ms(1300);
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(500);
			
			//直线矫正
			Claw_Up_Down(0,Line_Height);//降下摄像头
			delay_ms(500);
			Line_task();
			Claw_Up_Down(1,Line_Height);//升起摄像头
			delay_ms(500);

			//根据二维码数字决定到哪个位置
			if(qr_data[0] == '1') 		
			{	
				change_xyw_speed_no_quene(95,12,0);
				delay_ms(870);
				change_xyw_speed_no_quene(0,0,0);
			}
			else if(qr_data[0] == '2')
			{	
				change_xyw_speed_no_quene(100,15,0);
				delay_ms(1150);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			else if(qr_data[0] == '3') 	
			{
				change_xyw_speed_no_quene(100,10,0);
				delay_ms(1600);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			change_xyw_speed_no_quene(0,0,0);
			vTaskDelay(500);
		
		//*成品区放置物料*//
			 for(char i = 0; i < 3; i++)
		{
			move = get_way(qr_data[i]);		//获取下一个目标
			change_xyw_speed_no_quene(move[1], 0, 0);
			delay_ms(move[0]);
			
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//降下摄像头
			delay_ms(500);
			
			for(char j =0; j<10; j++)//刷新串口数据
			{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//色环矫正

			Claw_Up_Down(1,Camera_Height);//升起摄像头
			delay_ms(500);
			
			//按顺序决定货仓舵机角度
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			PutOn_Ground();//放置物料到地上
		}
			 
		ServoAngle(1,Warehouse_running);
		
	}
		//复位count
		 for(char i = 0; i < 3; i++)
				move = get_way(qr_data[i]);
	
			if(qr_data[2] == '1') //根据二维码数字补全距离
				{change_xyw_speed_no_quene(50,0,0);delay_ms(100);}
			else if(qr_data[2] == '2') 
				{change_xyw_speed_no_quene(-100,0,0);delay_ms(350);}
			else if(qr_data[2] == '3') 
				{change_xyw_speed_no_quene(-100,0,0);delay_ms(690);}

			change_xyw_speed_no_quene(0,0,0);delay_ms(300);
		 
		 {//**回到原料区**//			 

			 change_xyw_speed_no_quene(-100,-70,0);
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(-100,15,0);
			 delay_ms(450);
		 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(-93,0,66);
			 delay_ms(1200);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(600);  
			 
			  //直线矫正
			 Claw_Up_Down(0,Line_Height);//降下摄像头
			 delay_ms(500);
			 Line_task();
			 Claw_Up_Down(1,Line_Height);//升起摄像头
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(-150,0,0);
			 delay_ms(1400);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(400);
			 
			 change_xyw_speed_no_quene(-114,0,55);
			 delay_ms(1450);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(400);
			 
			change_xyw_speed_no_quene(-100,0,0);
			 delay_ms(130);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(600);

		 }
		 
		 //********第二轮********//
		 
		 {//二轮原料获取
			for(char j = 0; j < 10 ;j++)//刷新串口数据
			 {
				 SendByte('c',qr_data[3] - 1);
				 color_data = get_color_data();
				 delay_ms(100);
			 }
			
			 
			for(i = 3;i <= 5; i++)
		{
			
			SendByte('c',qr_data[i] - 1);//颜色判断
			
			while(1)
			{
				color_data = get_color_data();
				if(color_data[0] == '1')
				{
					color_data = NULL;
					break;
				}
			}
			//按顺序决定货仓舵机角度
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
				
				if(i <= 4)
			{
				Claw_Catch_material();//获取原料
				delay_ms(500);
			}
			if(i == 5)
			{
					Claw_Up_Down(0,Material_Height);//下降
					ServoAngle(0,Claw_Catch);//抓取角度：225
					delay_ms(300);
				
					ServoAngle(2,Clound_Reset);//云台旋转放置：270
				
					Claw_Up_Down(1,Material_Height);//升高
				
					Claw_Up_Down(0,Put_Height);//下降
				
					ServoAngle(0,Claw_Open_Small);//松开小角度：200
					delay_ms(100);
				
					Claw_Up_Down(1,Put_Height);//升高
					ServoAngle(0,Claw_Open);
				
					//放置完最后一个物料后，边走边复位云台
					change_xyw_speed_no_quene(118,0,-70);ServoAngle(1,Warehouse_running);
					delay_ms(1200);
				
					change_xyw_speed_no_quene(0,0,0);ServoAngle(2,Cloude_Work);
					delay_ms(600);

			}
		}
			
		 }
		
		 {//**第二轮原料区到暂存区**//
				
			if(qr_data[3] == '1')//根据二维码数字决定先到哪个位置
			{	
				change_xyw_speed_no_quene(100,0,0);
				delay_ms(1200);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			else if(qr_data[3] == '2')
			{	
				change_xyw_speed_no_quene(100,0,0);
				delay_ms(1500);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			else if(qr_data[3] == '3') 	
			{
				change_xyw_speed_no_quene(100,0,0);
				delay_ms(1900);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
		
		}
		
		//**第二轮在半成品区放置三个物料**//
		for(char i = 3; i <= 5; i++)
		{
			move = get_way(qr_data[i]);//获取下一个目标
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//降下摄像头
			delay_ms(500);
			
			for(char j = 0; j < 5; j++)//刷新串口数据
				{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//树莓先生的色环矫正环节

			Claw_Up_Down(1,Camera_Height);//升起摄像头
			delay_ms(500);
			
			//按顺序决定货仓舵机角度
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			PutOn_Ground();//放置物料到地上	

		}

		//**第二轮半成品区捡起三个物料**//
		for(char i = 3; i <= 5; i++)
		{
			move = get_way(qr_data[i]);		//获取下一个目标
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			//按顺序决定货仓舵机角度
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203 
			
			Catch_From_Ground();//从地面取回物料放回货仓
			
			delay_ms(500);
		}
		
		ServoAngle(1,Warehouse_running);

		{//**第二轮暂存区到成品区**//
			
			change_xyw_speed_no_quene(0,-100,0);
			delay_ms(200);
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(300);
			
			//根据二维码数字补回距离
			if(qr_data[5] == '1')  
			{change_xyw_speed_no_quene(100,0,0);delay_ms(800);}
			else if(qr_data[5] == '2') 
			{change_xyw_speed_no_quene(100,0,0);delay_ms(500);}
			else if(qr_data[5] == '3') 
			{change_xyw_speed_no_quene(100,0,0);delay_ms(100);}

			change_xyw_speed_no_quene(0,0,0);delay_ms(500);
			
			change_xyw_speed_no_quene(125,0,-65);
			delay_ms(1200);
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(500);
			
			//直线矫正
			Claw_Up_Down(0,Line_Height);//降下摄像头
			delay_ms(500);
			Line_task();
			Claw_Up_Down(1,Line_Height);//升起摄像头
			delay_ms(500);
	
			//根据二维码数字决定到哪个位置
			if(qr_data[3] == '1') 		
			{	
				change_xyw_speed_no_quene(95,12,0);
				delay_ms(900);
				change_xyw_speed_no_quene(0,0,0);
			}
			else if(qr_data[3] == '2')
			{	
				change_xyw_speed_no_quene(100,10,0);
				delay_ms(1400);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			else if(qr_data[3] == '3') 	
			{
				change_xyw_speed_no_quene(100,8,0);
				delay_ms(1700);
				change_xyw_speed_no_quene(0,0,0);
				delay_ms(100);
			}
			change_xyw_speed_no_quene(0,0,0);
			vTaskDelay(500);
			
		
		//*第二轮成品区码垛*//
			 for(char i = 3; i <= 5; i++)
		{
			move = get_way(qr_data[i]);		//获取下一个目标
			change_xyw_speed_no_quene(move[1], 0, 0);
			delay_ms(move[0]);
			
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//降下摄像头
			delay_ms(500);
			
			for(char j = 0; j < 10; j++)//刷新串口数据
			{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//色环矫正

			Claw_Up_Down(1,Camera_Height);//升起摄像头
			delay_ms(500);
			
			//按顺序决定货仓舵机角度
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			Stacking_Material();//码垛
		}
			 
			ServoAngle(1,Warehouse_running);
	}
		
			//根据二维码数字补全距离
			if(qr_data[5] == '3') 
				{change_xyw_speed_no_quene(100,0,0);delay_ms(100);}
			else if(qr_data[5] == '2') 
				{change_xyw_speed_no_quene(100,0,0);delay_ms(350);}
			else if(qr_data[5] == '1') 
				{change_xyw_speed_no_quene(100,0,0);delay_ms(690);}
				
			{//**返屋企**//
			  change_xyw_speed_no_quene(0,0,0);
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(100,-70,0);
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(100,15,0);
			 delay_ms(450);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(500);
			 
			 //直线矫正
			 Claw_Up_Down(0,Line_Height);//降下摄像头
			 delay_ms(500);
			 Line_task();
			 Claw_Up_Down(1,Line_Height);//升起摄像头
			 delay_ms(500);
			 
			 ServoAngle(0,Claw_Open); //150 225 Claw_Open Claw_Catch Claw_Open_Small
			 ServoAngle(1,Warehouse_1); //108 10 208 Warehouse_1 Warehouse_2 Warehouse_3
			 ServoAngle(2,Clound_Reset); //90 268  Cloude_Work  Clound_Reset
				
			 change_xyw_speed_no_quene(130,0,-66);
			 delay_ms(1200);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(600);
			 
			 change_xyw_speed_no_quene(150,0,0);
			 delay_ms(1600);
				
			 change_xyw_speed_no_quene(150,17,0);
			 delay_ms(650);

			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(500);
		 }
		
		 
		change_xyw_speed_no_quene(0,0,0);//停车
		delay_ms(100);
		 
/*
r"""
*********************************************
? ? ? ? ? ? ? ? ? ?_ooOoo_
? ? ? ? ? ? ? ? ? o8888888o
? ? ? ? ? ? ? ? ? 88" . "88
? ? ? ? ? ? ? ? ? (| -_- |)
? ? ? ? ? ? ? ? ? O\ ?= ?/O
? ? ? ? ? ? ? ?____/`---'\____
? ? ? ? ? ? ?.' ?\\| ? ? |// ?`.
? ? ? ? ? ? / ?\\||| ?: ?|||// ?\
? ? ? ? ? ?/ ?_||||| -:- |||||- ?\
? ? ? ? ? ?| ? | \\\ ?- ?/// | ? |
? ? ? ? ? ?| \_| ?''\---/'' ?| ? |
? ? ? ? ? ?\ ?.-\__ ?`-` ?___/-. /
? ? ? ? ?___`. .' ?/--.--\ ?`. . __
? ? ? ."" '&lt; ?`.___\_&lt;|>_/___.' ?>'"".
? ? ?| | : ?`- \`.;`\ _ /`;.`/ - ` : | |
? ? ?\ ?\ `-. ? \_ __\ /__ _/ ? .-` / ?/
======`-.____`-.___\_____/___.-`____.-'======
? ? ? ? ? ? ? ? ? ?`=---='

? ? ? ? ? ? 佛祖保佑 ? ? ? 工创省一
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
"""
*/
		 while(1);
	}
}
