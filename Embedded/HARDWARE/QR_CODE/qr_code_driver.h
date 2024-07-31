#ifndef __QR_CODE_DRIVER_H
#define __QR_CODE_DRIVER_H

#include "usart.h"

#define rec_finish1      0
#define rec_unfinish1    1

void scan_qr_code_Init(void);
void rec_qr_code_data(u8 rec_qr_data);
char* get_qr_code_data(void);

#endif
