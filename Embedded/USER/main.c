#include "main.h"

/********************
	������
	���󲿷���ֲ��gitee������ؿ� "@D2zling"
	������ϸ����gitee������ؿ⡰2024������ش��롱
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

? ? ? ? ? ? ���汣�� ? ? ? ����ʡһ
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
"""
*/

/*
���      PD12	   PD13	   PD14	   
ͨ��	    CH1	     CH2		 CH3		   
��Ӧ      צ��     ����    ��̨
ServoNum   0        1       2
*/

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);			//����ϵͳ�ж����ȼ�����4
	delay_init(168);										  //��ʱ��ʼ��
	usart3_init(9600);										//��ݮ��ͨ�Ŷ˿�
	scan_qr_code_Init();									//ɨ���ʼ��
	digital_tube_Init();									//�����ģ���ʼ��
	MOTOR_UP_Down_Init();									//�����������צ��ʼ��
	usart_ctrl_steeping_motor_Init();			//���ڿ��Ʋ��������ʼ��
	servo_Init(20000 - 1, 84 - 1);				//������Ƴ�ʼ��

	package_Init();											  //�������ݰ������ʼ��
	motor_soft_Init();										//��������ʼ��
	
	//���������λ
	ServoAngle(0,Claw_Open); //150 228 205 130 Claw_Open Claw_Catch Claw_Open_Small Claw_Open_Big
	ServoAngle(1,Warehouse_running); //108 10 208 120 Warehouse_1 Warehouse_2 Warehouse_3 Warehouse_running
	ServoAngle(2,Clound_Reset); //90 268  Cloude_Work  Clound_Reset
}

void tast_loop(void)
{
  static char* qr_data = NULL;						//ɨ�����ݰ�
	static int* move = NULL;							//���Ʒ/��Ʒ������
	static u8 *color_data = NULL;						//��ɫ����

	char i;												//��������
	int angle_warehouse = 108;
	
	delay_ms(2000);	//�ȴ��������
	                                                                                                                                                  ;
	while(1)
	{

//		ServoAngle(2,Cloude_Work);
					
//			while(1);
		
		/*********��һ��**********/
		{	//**ǰ��ԭ����**//
		change_xyw_speed_no_quene(0,-91,0);//-90
		delay_ms(300);

		change_xyw_speed_no_quene(157,0,0);
		delay_ms(1200);
		
		change_xyw_speed_no_quene(0,0,0);
		delay_ms(1000);
		
		while(qr_data == NULL) qr_data = get_qr_code_data();//��ȡ��ά�벢��ʾ
		digital_tube_Display(qr_data);

		delay_ms(200);
			
		ServoAngle(0,Claw_Open);//צ�ӿ�ʼ����
		ServoAngle(2,Cloude_Work);//��̨��ʼ����
		
		change_xyw_speed_no_quene(146,26,0);
		delay_ms(1200);

		change_xyw_speed_no_quene(0,0,0);
		delay_ms(100);
		
		delay_ms(500);
		}
					
		//**һ�ֻ�ȡ����**//
		for(i = 0;i <= 2; i++)
		{
			SendByte('c',qr_data[i]-1);//������ݮ����ʶ����ɫ
			//���ԣ���ע�͵���װʶ��
			while(1)
			{
				color_data = get_color_data();
				if(color_data[0] == '1')
				{
					color_data = NULL;
					break;
				}
			}
			//��˳��������ֶ���Ƕ�
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			if(i <= 1)
			{
				Claw_Catch_material();//��ȡԭ��
				delay_ms(500);
			}
			if(i == 2)
			{
					Claw_Up_Down(0,Material_Height);//�½�
					ServoAngle(0,Claw_Catch);//ץȡ�Ƕȣ�225
					delay_ms(300);
					ServoAngle(2,Clound_Reset);//��̨��ת���ã�270
					Claw_Up_Down(1,Material_Height);//����
					Claw_Up_Down(0,Put_Height);//�½�
					ServoAngle(0,Claw_Open_Small);//�ɿ�С�Ƕȣ�200
					delay_ms(100);
					Claw_Up_Down(1,Put_Height);//����
					ServoAngle(0,Claw_Open);
					//���������һ�����Ϻ󣬱��߱߸�λ��̨
					change_xyw_speed_no_quene(132,0,-65);ServoAngle(1,Warehouse_running);
					delay_ms(1200);
					change_xyw_speed_no_quene(0,0,0);ServoAngle(2,Cloude_Work);
					delay_ms(600);
			}

		}

		{//**һ�������̵��ݴ���**//

			if(qr_data[0] == '1')//���ݶ�ά�����־����ȵ��ĸ�λ��
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
		
		//**һ�ְ��Ʒ��������������**//
		for(char i = 0; i <= 2; i++)
		{
			move = get_way(qr_data[i]);//��ȡ��һ��Ŀ��
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//��������ͷ
			vTaskDelay(500);
			
			for(char j =0; j<10; j++)//ˢ�´�������
			{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//��ݮ������ɫ����������

			Claw_Up_Down(1,Camera_Height);//��������ͷ
			delay_ms(500);
			
			//��˳��������ֶ���Ƕ�
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			PutOn_Ground();//�������ϵ�����	

		}

		//**���Ʒ��������������**//
		for(char i = 0; i < 3; i++)
		{
			move = get_way(qr_data[i]);		//��ȡ��һ��Ŀ��
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			//��˳��������ֶ���Ƕ�
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203 
			
			Catch_From_Ground();//�ӵ���ȡ�����ϷŻػ���
			
			delay_ms(500);
		}
		
		ServoAngle(1,Warehouse_running);

		{//**�ݴ�������Ʒ��**//
			change_xyw_speed_no_quene(0,-100,0);//-90
			delay_ms(200);
			change_xyw_speed_no_quene(0,0,0);//-90
			delay_ms(300);
			
			
			//���ݶ�ά�����ֲ��ؾ���
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
			
			//ֱ�߽���
			Claw_Up_Down(0,Line_Height);//��������ͷ
			delay_ms(500);
			Line_task();
			Claw_Up_Down(1,Line_Height);//��������ͷ
			delay_ms(500);

			//���ݶ�ά�����־������ĸ�λ��
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
		
		//*��Ʒ����������*//
			 for(char i = 0; i < 3; i++)
		{
			move = get_way(qr_data[i]);		//��ȡ��һ��Ŀ��
			change_xyw_speed_no_quene(move[1], 0, 0);
			delay_ms(move[0]);
			
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//��������ͷ
			delay_ms(500);
			
			for(char j =0; j<10; j++)//ˢ�´�������
			{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//ɫ������

			Claw_Up_Down(1,Camera_Height);//��������ͷ
			delay_ms(500);
			
			//��˳��������ֶ���Ƕ�
			if(i == 0) 		angle_warehouse = Warehouse_1;
			else if(i == 1) 	angle_warehouse = Warehouse_2;
			else if(i == 2) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			PutOn_Ground();//�������ϵ�����
		}
			 
		ServoAngle(1,Warehouse_running);
		
	}
		//��λcount
		 for(char i = 0; i < 3; i++)
				move = get_way(qr_data[i]);
	
			if(qr_data[2] == '1') //���ݶ�ά�����ֲ�ȫ����
				{change_xyw_speed_no_quene(50,0,0);delay_ms(100);}
			else if(qr_data[2] == '2') 
				{change_xyw_speed_no_quene(-100,0,0);delay_ms(350);}
			else if(qr_data[2] == '3') 
				{change_xyw_speed_no_quene(-100,0,0);delay_ms(690);}

			change_xyw_speed_no_quene(0,0,0);delay_ms(300);
		 
		 {//**�ص�ԭ����**//			 

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
			 
			  //ֱ�߽���
			 Claw_Up_Down(0,Line_Height);//��������ͷ
			 delay_ms(500);
			 Line_task();
			 Claw_Up_Down(1,Line_Height);//��������ͷ
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
		 
		 //********�ڶ���********//
		 
		 {//����ԭ�ϻ�ȡ
			for(char j = 0; j < 10 ;j++)//ˢ�´�������
			 {
				 SendByte('c',qr_data[3] - 1);
				 color_data = get_color_data();
				 delay_ms(100);
			 }
			
			 
			for(i = 3;i <= 5; i++)
		{
			
			SendByte('c',qr_data[i] - 1);//��ɫ�ж�
			
			while(1)
			{
				color_data = get_color_data();
				if(color_data[0] == '1')
				{
					color_data = NULL;
					break;
				}
			}
			//��˳��������ֶ���Ƕ�
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
				
				if(i <= 4)
			{
				Claw_Catch_material();//��ȡԭ��
				delay_ms(500);
			}
			if(i == 5)
			{
					Claw_Up_Down(0,Material_Height);//�½�
					ServoAngle(0,Claw_Catch);//ץȡ�Ƕȣ�225
					delay_ms(300);
				
					ServoAngle(2,Clound_Reset);//��̨��ת���ã�270
				
					Claw_Up_Down(1,Material_Height);//����
				
					Claw_Up_Down(0,Put_Height);//�½�
				
					ServoAngle(0,Claw_Open_Small);//�ɿ�С�Ƕȣ�200
					delay_ms(100);
				
					Claw_Up_Down(1,Put_Height);//����
					ServoAngle(0,Claw_Open);
				
					//���������һ�����Ϻ󣬱��߱߸�λ��̨
					change_xyw_speed_no_quene(118,0,-70);ServoAngle(1,Warehouse_running);
					delay_ms(1200);
				
					change_xyw_speed_no_quene(0,0,0);ServoAngle(2,Cloude_Work);
					delay_ms(600);

			}
		}
			
		 }
		
		 {//**�ڶ���ԭ�������ݴ���**//
				
			if(qr_data[3] == '1')//���ݶ�ά�����־����ȵ��ĸ�λ��
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
		
		//**�ڶ����ڰ��Ʒ��������������**//
		for(char i = 3; i <= 5; i++)
		{
			move = get_way(qr_data[i]);//��ȡ��һ��Ŀ��
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//��������ͷ
			delay_ms(500);
			
			for(char j = 0; j < 5; j++)//ˢ�´�������
				{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//��ݮ������ɫ����������

			Claw_Up_Down(1,Camera_Height);//��������ͷ
			delay_ms(500);
			
			//��˳��������ֶ���Ƕ�
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			PutOn_Ground();//�������ϵ�����	

		}

		//**�ڶ��ְ��Ʒ��������������**//
		for(char i = 3; i <= 5; i++)
		{
			move = get_way(qr_data[i]);		//��ȡ��һ��Ŀ��
			change_xyw_speed_no_quene(move[1],0,0);
			delay_ms(move[0]);

			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			//��˳��������ֶ���Ƕ�
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203 
			
			Catch_From_Ground();//�ӵ���ȡ�����ϷŻػ���
			
			delay_ms(500);
		}
		
		ServoAngle(1,Warehouse_running);

		{//**�ڶ����ݴ�������Ʒ��**//
			
			change_xyw_speed_no_quene(0,-100,0);
			delay_ms(200);
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(300);
			
			//���ݶ�ά�����ֲ��ؾ���
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
			
			//ֱ�߽���
			Claw_Up_Down(0,Line_Height);//��������ͷ
			delay_ms(500);
			Line_task();
			Claw_Up_Down(1,Line_Height);//��������ͷ
			delay_ms(500);
	
			//���ݶ�ά�����־������ĸ�λ��
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
			
		
		//*�ڶ��ֳ�Ʒ�����*//
			 for(char i = 3; i <= 5; i++)
		{
			move = get_way(qr_data[i]);		//��ȡ��һ��Ŀ��
			change_xyw_speed_no_quene(move[1], 0, 0);
			delay_ms(move[0]);
			
			change_xyw_speed_no_quene(0,0,0);
			delay_ms(100);
			
			Claw_Up_Down(0,Camera_Height);//��������ͷ
			delay_ms(500);
			
			for(char j = 0; j < 10; j++)//ˢ�´�������
			{SendByte('C',qr_data[i]-1);delay_ms(100);}
			
			location_task(qr_data[i]-1);//ɫ������

			Claw_Up_Down(1,Camera_Height);//��������ͷ
			delay_ms(500);
			
			//��˳��������ֶ���Ƕ�
			if(i == 3) 		angle_warehouse = Warehouse_1;
			else if(i == 4) 	angle_warehouse = Warehouse_2;
			else if(i == 5) 	angle_warehouse = Warehouse_3;
			ServoAngle(1,angle_warehouse);//108 10 203
			
			Stacking_Material();//���
		}
			 
			ServoAngle(1,Warehouse_running);
	}
		
			//���ݶ�ά�����ֲ�ȫ����
			if(qr_data[5] == '3') 
				{change_xyw_speed_no_quene(100,0,0);delay_ms(100);}
			else if(qr_data[5] == '2') 
				{change_xyw_speed_no_quene(100,0,0);delay_ms(350);}
			else if(qr_data[5] == '1') 
				{change_xyw_speed_no_quene(100,0,0);delay_ms(690);}
				
			{//**������**//
			  change_xyw_speed_no_quene(0,0,0);
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(100,-70,0);
			 delay_ms(500);
			 
			 change_xyw_speed_no_quene(100,15,0);
			 delay_ms(450);
			 
			 change_xyw_speed_no_quene(0,0,0);
			 delay_ms(500);
			 
			 //ֱ�߽���
			 Claw_Up_Down(0,Line_Height);//��������ͷ
			 delay_ms(500);
			 Line_task();
			 Claw_Up_Down(1,Line_Height);//��������ͷ
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
		
		 
		change_xyw_speed_no_quene(0,0,0);//ͣ��
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

? ? ? ? ? ? ���汣�� ? ? ? ����ʡһ
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
"""
*/
		 while(1);
	}
}
