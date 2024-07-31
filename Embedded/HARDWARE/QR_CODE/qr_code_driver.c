#include "qr_code_driver.h"

char p_qr_data = 0; 
char qr_data_packge[10];

//扫码模块初始化
void scan_qr_code_Init()
{
    uart5_init(9600);
    qr_data_packge[7] = rec_unfinish1;
}

//扫码数据协议实现
void rec_qr_code_data(u8 rec_qr_data)
{
    qr_data_packge[p_qr_data] = rec_qr_data;
   
    if(p_qr_data == 7)
    {
        qr_data_packge[7] = rec_finish1;
    }

    p_qr_data++;
}

//解码
char* get_qr_code_data()
{
    if((qr_data_packge[7] == rec_finish1))
    {
        qr_data_packge[7] = rec_unfinish1;
        p_qr_data = 0;


        for (char i = 3; i < 7; i++) qr_data_packge[i] = qr_data_packge[i+1];

        return qr_data_packge;
    }
    else return NULL;
}
