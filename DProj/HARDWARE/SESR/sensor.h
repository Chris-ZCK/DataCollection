#ifndef __SENSOR_H
#define __SENSOR_H
#include "sys.h"

//extern char message[1000];

void Data_Packing_sens(char *msg);
void act_get_data(void);
u8 act_send_data(void);
#endif
