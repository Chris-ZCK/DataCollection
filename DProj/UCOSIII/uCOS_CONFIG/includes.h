/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           MASTER INCLUDES
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  INCLUDES_MODULES_PRESENT
#define  INCLUDES_MODULES_PRESENT


/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/


#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <os.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>

/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <bsp.h>
//#include  <bsp_int.h>

/*
*********************************************************************************************************
*                                                 USER
*********************************************************************************************************
*/
struct cycle_package
{
	uint32_t time_stamp;

	uint16_t watch_cnt;	  // 看门狗时间
	uint16_t task_cnt;    // 任务时间
	uint16_t function; 	  // 功能函数
	uint16_t picture_id;  // 照片ID
	uint8_t  buf[100];
};

struct flash_package
{
	uint32_t id_in_flash; // #special loop;
	uint8_t  buf[100];
};


extern vu16 ec25_on_flag;
extern struct cycle_package cycle;
extern struct flash_package eerom;


// ec25 flag，1：use HCTV; 0:use normal network
#define In_Condition_HCTV 0
// global switch
// 调试模式
#define DEBUG_MODE    1
// 睡眠模式
#define SLEEP_MODE    0
// USB开关
#define USB_MODE      0
// 传感器开关
#define SENSOR_MODE   0
// 解析任务
#define ANAY_TASK_ON  0
// 按键控制测试
#define KEY_SCAN_ON 1
// 是否询问GPS数据
#define QUEERY_GPS_ON 0
// 是否询问电池数据
#define QUEERY_BATTERY_ON 0
// 串口控制
#define UART_CMD_MODE 1
// 串口EC25中间信息
#define EC25_LOG_PRINT 0

#define WIFI_TRANSFORM_ON 1




// 使用485通讯
#define BATTERY_485 1
// 获取电池数据据版本：1 老版本 0 新版本
#define BATTERY_OLD_VERSION   0
// 1 老版本
// 0 新版本
// Global macro definition

#define EN_LOG_PRINT  3
// Global use
// 0 都不打印 
// 1 重要信息
// 2 一般信息
// 3 调试信息
// 4 其他信息

//////////////////////////////
//////////////////////////////
// 重要系统参数
#define TEST_PARA 0

#if TEST_PARA
// Sys cycle, unit:s
#define CYCLE_TIME 		60

// store photo, unit:s
#define TASK_S_P_CNT	  CYCLE_TIME*2
// send data, unit:s
#define TASK_S_D_CNT    CYCLE_TIME*2
// take photo, unit:s
#define TASK_T_P_CNT		CYCLE_TIME*10

// 单位1mA
#define TD_C_C_VAL		500
// 单位mv
#define TD_B_V_VAL		12000

#define TD_C_H_S		0
#define TD_C_H_E		24

// 待机时间
#define STANDBY_TIME	30
#define MAX_RUN_TIME	1200
#else
// Sys cycle, unit:s
#define CYCLE_TIME 		600

// store photo, unit:s
#define TASK_S_P_CNT	CYCLE_TIME*12
// send data, unit:s
#define TASK_S_D_CNT    CYCLE_TIME*12
// take photo, unit:s
#define TASK_T_P_CNT	CYCLE_TIME*36

// 单位1mA
#define TD_C_C_VAL		500
// 单位mv
#define TD_B_V_VAL		12000

#define TD_C_H_S		0
#define TD_C_H_E		23

// 待机时间
#define STANDBY_TIME	30
#define MAX_RUN_TIME	1200
#endif
//////////////////////////////
//////////////////////////////


#include "MyFunction_C.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"

#include "stmflash.h"
#include "rtc.h"
#include "iwdg.h"

#include "power.h"

#include "UART_TCPbuff.h"
#include "usart3.h"

// MQTT
#include "mqttApp.h"
#include "mqttBase.h"
#include "MQTTPacket.h"
#include "mqttTransport.h"

// WiFi
#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"
#include "M8266WIFI_ops.h"

#include "ec25.h"

#include "sensor.h"  // self
#include "myiic_sht20.h"
#include "myiic_max44009.h"
#include "SHT2x.h"
#include "max44009.h" 
#include "ms5611.h"
#include "sensor.h"
#include "usart2.h"
#include "battery.h" 
#include "gps.h" 

// gloabl virable

#include "malloc.h"

#include "sdio_sdcard.h"
#include "ff.h"  
#include "exfuns.h"  
#include "fattester.h"	

#include "usbh_usr.h" 
#include "usb_app.h"

#include "queue.h" 
#include "rng.h"


extern	vu16 function_f;
u8 analyze_config_para(char *buf, u16 * val);
#endif
