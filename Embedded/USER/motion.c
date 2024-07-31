#include "motion.h"

static char now_location = 0;


int moving(char location)
{
	static int count_value = 0;
	int difference_value = 0;
	
	if (now_location == 0)
	{
		now_location = location;
		return 0;
	}
	else if (location != now_location)
	{
		difference_value = location - now_location;
		now_location = location;
	}
	count_value++;
	if(count_value == 6) now_location = count_value = 0;
	return 50 * difference_value;
}

//��ȡ������Ʒ�ķ�����뺯��
int* get_way(char location)
{
	static int out_location[2], count = 0;

	if (now_location == 0)
	{
		out_location[0] = out_location[1] = 0;
		now_location = location;
	}
	else if (location > now_location)
	{
		out_location[0] = (location - now_location) * 700;	//time
		out_location[1] = 50;								//speed
	}
	else if (location < now_location)
	{
		out_location[0] = (now_location -location) * 700;	//time
		out_location[1] = -50;								//speed
	}
	now_location = location;

	count++;
	if (count == 6) now_location = count = 0;	//ÿ���θ�λ
	return out_location;
}

void location_task(char RGB)
{
    int* location_x_y_data = NULL;
		static int flag_value = 0;
    reset_circle_location_packge();

	while(1)
	{
		SendByte('C',RGB);
		vTaskDelay(100);

		location_x_y_data = NULL;
		location_x_y_data = circle_location_decoding();	// ���㶨λx��y

		if(	location_x_y_data != NULL)//���յ�����
		{
			//�����ж�
			if(	((location_x_y_data[0] > -circle_dead_area) && (location_x_y_data[0] < circle_dead_area)) &&
				((location_x_y_data[1] > -circle_dead_area) && (location_x_y_data[1] < circle_dead_area)))
			{
				if(flag_value == 1)
					{
							change_xyw_speed_no_quene(0, 0, 0);
							flag_value = 0;
							return;
					}
				flag_value++;
			}

			//�ֶ��޷�
			for(char i = 0; i < 2; i++)
			{
				if(location_x_y_data[i] < -circle_second_area) location_x_y_data[i] = -2;
				else if(location_x_y_data[i] > circle_second_area) location_x_y_data[i] = 2;
				else if((location_x_y_data[i] > -circle_second_area) && (location_x_y_data[i] < -1)) location_x_y_data[i] = -1;
				else if((location_x_y_data[i] < circle_second_area) && (location_x_y_data[i]) > 1) location_x_y_data[i] = 1;
			}

			change_xyw_speed_no_quene(location_x_y_data[0], location_x_y_data[1], 0);
		}
	}
	
}

void Line_task()
{
	int* Line_Angle_Data = NULL;
	static int count = 0;
	
	while(1)
    {
				
			SendByte('n','A');
        delay_ms(200);
				
				Line_Angle_Data = Line_Angle_decoding();
			
				if(Line_Angle_Data != NULL)//�յ�����
				{
					if(Line_Angle_Data[0] > -Line_dead_area && Line_Angle_Data[0] < Line_dead_area)
						{	
							if(count == 3)
							{
								change_xyw_speed_no_quene(0, 0, 0);
								count = 0;
								return;
							}
							change_xyw_speed_no_quene(0, 0, 0);
							count++;
						}
					
					if(Line_Angle_Data[0] < -Line_second_area) 
						Line_Angle_Data[0] = -2;
					else if(Line_Angle_Data[0] > Line_second_area) 
						Line_Angle_Data[0] = 2;
					else if((Line_Angle_Data[0] > -Line_second_area) && (Line_Angle_Data[0] < -Line_dead_area))
						Line_Angle_Data[0] = -1;
					else if((Line_Angle_Data[0] < Line_second_area) && (Line_Angle_Data[0] > Line_dead_area))
						Line_Angle_Data[0] = 1;
					
					change_xyw_speed_no_quene(0, 0, Line_Angle_Data[0]);
					
				}
				   
    }
}

//��ȡԭ��
void Claw_Catch_material(void)
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
	ServoAngle(2,Cloude_Work);//��̨��ת��λ��90
	delay_ms(100);
}

//�����Ϸ��õ�����
void PutOn_Ground(void)
{
	ServoAngle(0,Claw_Open_Big);//�ɿ���Ƕ�
	ServoAngle(2,Clound_Reset);//��̨��ת�ӻ�����ȡ��270
	delay_ms(500);
	
	ServoAngle(0,Claw_Catch);//ץȡ�Ƕȣ�225
	
	Claw_Up_Down(0,Put_Height);//�½�

	Claw_Up_Down(1,Put_Height);//����

	ServoAngle(2,Cloude_Work);//��̨��ת��λ��90
	delay_ms(500);
	
	Claw_Up_Down(0,Placement_Height);//�½�

	ServoAngle(0,Claw_Open_Big);//�ɿ���Ƕȣ�150
	delay_ms(100);
	
	Claw_Up_Down(1,Placement_Height);//����
}

//�ӵ���ȡ������
void Catch_From_Ground(void)
{
	Claw_Up_Down(0,Placement_Height);//�½�
	
	ServoAngle(0,Claw_Catch);//ץȡ�Ƕȣ�225
	delay_ms(350);
	Claw_Up_Down(1,Placement_Height);//����
	
	ServoAngle(2,Clound_Reset);//��̨��ת�ŵ����֣�270
	delay_ms(550);
	Claw_Up_Down(0,Put_Height);//�½�
	
	ServoAngle(0,Claw_Open_Small);//�ɿ�С�Ƕȣ�200
	delay_ms(100);
	Claw_Up_Down(1,Put_Height);//����

	ServoAngle(0,Claw_Open);//�ɿ���Ƕ�
	
	ServoAngle(2,Cloude_Work);//��̨��ת��λ��90
	delay_ms(100);
}

//���
void Stacking_Material(void)
{
	ServoAngle(0,Claw_Open_Big);//�ɿ�С�Ƕȣ�200
	ServoAngle(2,Clound_Reset);//��̨��ת�ӻ�����ȡ��270
	delay_ms(500);
	
	ServoAngle(0,Claw_Catch);//ץȡ�Ƕȣ�225
	
	Claw_Up_Down(0,Put_Height);//�½�

	Claw_Up_Down(1,Put_Height);//����
	
	ServoAngle(2,Cloude_Work);//��̨��ת��λ��90
	delay_ms(500);
	
	Claw_Up_Down(0,Stack_Height);//�½�

	ServoAngle(0,Claw_Open_Big);//�ɿ���Ƕȣ�150
	delay_ms(100);
	
	Claw_Up_Down(1,Stack_Height);//����
}
