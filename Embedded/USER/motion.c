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

//获取放置物品的方向距离函数
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
	if (count == 6) now_location = count = 0;	//每三次复位
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
		location_x_y_data = circle_location_decoding();	// 解算定位x和y

		if(	location_x_y_data != NULL)//接收到数据
		{
			//死区判断
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

			//分段限幅
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
			
				if(Line_Angle_Data != NULL)//收到数据
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

//获取原料
void Claw_Catch_material(void)
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
	ServoAngle(2,Cloude_Work);//云台旋转归位：90
	delay_ms(100);
}

//将物料放置到地上
void PutOn_Ground(void)
{
	ServoAngle(0,Claw_Open_Big);//松开大角度
	ServoAngle(2,Clound_Reset);//云台旋转从货仓拿取：270
	delay_ms(500);
	
	ServoAngle(0,Claw_Catch);//抓取角度：225
	
	Claw_Up_Down(0,Put_Height);//下降

	Claw_Up_Down(1,Put_Height);//升高

	ServoAngle(2,Cloude_Work);//云台旋转归位：90
	delay_ms(500);
	
	Claw_Up_Down(0,Placement_Height);//下降

	ServoAngle(0,Claw_Open_Big);//松开大角度：150
	delay_ms(100);
	
	Claw_Up_Down(1,Placement_Height);//升高
}

//从地上取回物料
void Catch_From_Ground(void)
{
	Claw_Up_Down(0,Placement_Height);//下降
	
	ServoAngle(0,Claw_Catch);//抓取角度：225
	delay_ms(350);
	Claw_Up_Down(1,Placement_Height);//升高
	
	ServoAngle(2,Clound_Reset);//云台旋转放到货仓：270
	delay_ms(550);
	Claw_Up_Down(0,Put_Height);//下降
	
	ServoAngle(0,Claw_Open_Small);//松开小角度：200
	delay_ms(100);
	Claw_Up_Down(1,Put_Height);//升高

	ServoAngle(0,Claw_Open);//松开大角度
	
	ServoAngle(2,Cloude_Work);//云台旋转归位：90
	delay_ms(100);
}

//码垛
void Stacking_Material(void)
{
	ServoAngle(0,Claw_Open_Big);//松开小角度：200
	ServoAngle(2,Clound_Reset);//云台旋转从货仓拿取：270
	delay_ms(500);
	
	ServoAngle(0,Claw_Catch);//抓取角度：225
	
	Claw_Up_Down(0,Put_Height);//下降

	Claw_Up_Down(1,Put_Height);//升高
	
	ServoAngle(2,Cloude_Work);//云台旋转归位：90
	delay_ms(500);
	
	Claw_Up_Down(0,Stack_Height);//下降

	ServoAngle(0,Claw_Open_Big);//松开大角度：150
	delay_ms(100);
	
	Claw_Up_Down(1,Stack_Height);//升高
}
