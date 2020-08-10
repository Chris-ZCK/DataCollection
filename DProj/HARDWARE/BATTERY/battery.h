#ifndef __BATTERY_H
#define __BATTERY_H
#include "sys.h"

typedef struct{
	u16 SOH;  // 电池健康状态
    float total_voltage;  // 总电压
    float charge_current;  // 充电电流
	float discharge_current;  // 放电电流
    int cell_temperature;  // 电池温度
	int level;  // 电池容量
	int max_capacity;  // 电池容量
	int remain_capacity;  // 当前电量
	u8 info[150];
} Battery_stats;

u8 battery_data_anay(void);
u8 battery_data_get(void);

extern Battery_stats battery;
#endif
