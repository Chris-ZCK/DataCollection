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

// power flag
// define POWER_MP  MP1584, active=1,default=0
// define POWER_MIC     PEout(10)  // MIC30032,active=1,default=0
#define POWER_4G		POWER_MP   

// global switch
#define DEBUG_MODE    1
#define SLEEP_MODE    0 
#define USB_MODE      1
#define SENSOR_MODE   0

#define UART_CMD_MODE 0
// Global macro definition
#define EN_log_print  2
// Global use
// 0 都不打印 
// 1 重要信息
// 2 一般信息
// 3 调试信息

// Sys cycle, unit:s
#define CYCLE_TIME 		1800

// store photo, unit:s
#define TASK_S_P_CNT	3600
// send data, unit:s
#define TASK_S_D_CNT    3600
// take photo, unit:s
#define TASK_T_P_CNT	14400

// 单位1mA
#define TD_C_C_VAL		500
// 单位mv
#define TD_B_V_VAL		12000

#define TD_C_H_S		0
#define TD_C_H_E		8

#define STANDBY_TIME	30
#define MAX_RUN_TIME	1200



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

#include "mqttApp.h"
#include "mqttBase.h"
#include "MQTTPacket.h"
#include "mqttTransport.h"

#include "ec25.h"
#if SENSOR_MODE
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
#endif
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
