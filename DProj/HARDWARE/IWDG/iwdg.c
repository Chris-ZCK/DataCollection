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

u32 local_time_cnt = 0; // ȫ�ֱ�������¼�µ�ǰ��ʱ��

/**
 * @description: ���Ź���ʼ��
 * ��ʼ���������Ź�
 * prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)
 * rlr:�Զ���װ��ֵ,0~0XFFF.
 * ��Ƶ����=4*2^prer.�����ֵֻ����256!
 * rlr:��װ�ؼĴ���ֵ:��11λ��Ч.
 * ʱ�����(���):Tout=((4*2^prer)*rlr)/32 (ms).
 * instance��
 * IWDG_Init(IWDG_Prescaler_256,4000);4,000*256/32,000=32s
 * @param {type} 
 * @return {type} 
 */
void IWDG_Init(u8 prer,u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //ʹ�ܶ�IWDG->PR IWDG->RLR��д
	
	IWDG_SetPrescaler(prer); //����IWDG��Ƶϵ��

	IWDG_SetReload(rlr);   //����IWDGװ��ֵ

	IWDG_ReloadCounter(); //reload
	
	IWDG_Enable();       //ʹ�ܿ��Ź�	
}

u32 last_timecount=0;
/**
 * @description: ι������
 * @param {type} 
 * @return {type} 
 */

void IWDG_Feed(void)
{

	u32 timecount;
	int delta;

	IWDG_ReloadCounter();  // ι��

	timecount = RTC_GetCounter();
	delta = timecount - last_timecount;
	printf("---------------------IWDG(Interval:%d<>Stamp=:%d)---------------------\r\n",delta, timecount);  // ��־��ӡ����ӡ������ι��ʱ��ļ��
	last_timecount = timecount;
	
}

