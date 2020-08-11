/*
 * @Author: Howell
 * @Date: 2020-08-10 17:34:17
 * @LastEditTime: 2020-08-11 15:09:03
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \USERd:\Documents\GitProject\DataCollection\DProj\HARDWARE\IWDG\iwdg.c
 */
#include "iwdg.h"
#include "usart.h"
#include "rtc.h"

u32 local_time_cnt = 0; // 全局变量，记录下当前的时间

/**
 * @description: 看门狗初始化
 * 初始化独立看门狗
 * prer:分频数:0~7(只有低3位有效!)
 * rlr:自动重装载值,0~0XFFF.
 * 分频因子=4*2^prer.但最大值只能是256!
 * rlr:重装载寄存器值:低11位有效.
 * 时间计算(大概):Tout=((4*2^prer)*rlr)/32 (ms).
 * instance：
 * IWDG_Init(IWDG_Prescaler_256,4000);4,000*256/32,000=32s
 * @param {type} 
 * @return {type} 
 */
void IWDG_Init(u8 prer,u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //使能对IWDG->PR IWDG->RLR的写
	
	IWDG_SetPrescaler(prer); //设置IWDG分频系数

	IWDG_SetReload(rlr);   //设置IWDG装载值

	IWDG_ReloadCounter(); //reload
	
	IWDG_Enable();       //使能看门狗	
}

u32 last_timecount=0;
/**
 * @description: 喂狗程序
 * @param {type} 
 * @return {type} 
 */

void IWDG_Feed(void)
{

	u32 timecount;
	int delta;

	IWDG_ReloadCounter();  // 喂狗

	timecount = RTC_GetCounter();
	delta = timecount - last_timecount;
	printf("---------------------IWDG(Interval:%d<>Stamp=:%d)---------------------\r\n",delta, timecount);  // 日志打印，打印出两次喂狗时间的间隔
	last_timecount = timecount;
	
}

