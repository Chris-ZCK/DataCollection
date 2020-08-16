/*
 * @Author: Howell
 * @Date: 2020-08-10 17:34:17
 * @LastEditTime: 2020-08-14 17:26:25
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit;
 * @FilePath: \USERd:\Documents\GitProject\DataCollection\DProj\HARDWARE\BATTERY\battery.h
 */
#ifndef __BATTERY_H
#define __BATTERY_H
#include "sys.h"

typedef struct{
	int SOH;  // 电池健康状态
    int total_voltage;  // 总电压,单位mv
    int charge_current;  // 充电电流，单位ma
	int discharge_current;  // 放电电流,单位ma
    int cell_temperature;  // 电池温度,单位摄氏度
	int level;  // 电池容量，单位万分之
	int max_capacity;  // 电池容量，单位mAh
	int remain_capacity;  // 当前电量，单位mAh
	u8 info[150];
} Battery_stats;
// local
void SendBatteryInstruct(u8 *instruct, u8 len);

u8 battery_data_anay(void);
u8 battery_data_get(void);

extern Battery_stats battery;

#endif
