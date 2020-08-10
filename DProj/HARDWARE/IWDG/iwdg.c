#include "iwdg.h"
#include "usart.h"
#include "rtc.h"

int local_time_cnt=0;
//��ʼ���������Ź�
//prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)
//rlr:�Զ���װ��ֵ,0~0XFFF.
//��Ƶ����=4*2^prer.�����ֵֻ����256!
//rlr:��װ�ؼĴ���ֵ:��11λ��Ч.
//ʱ�����(���):Tout=((4*2^prer)*rlr)/32 (ms).
void IWDG_Init(u8 prer,u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //ʹ�ܶ�IWDG->PR IWDG->RLR��д
	
	IWDG_SetPrescaler(prer); //����IWDG��Ƶϵ��

	IWDG_SetReload(rlr);   //����IWDGװ��ֵ

	IWDG_ReloadCounter(); //reload
	
	IWDG_Enable();       //ʹ�ܿ��Ź�	
}

//ι�������Ź�
void IWDG_Feed(void)
{
	int delta;
	
	IWDG_ReloadCounter();//reload
	calendar_get_time(&calendar);
	if(local_time_cnt<=calendar.sec)
		delta =  calendar.sec -local_time_cnt;
	else
		delta = calendar.sec +60 -local_time_cnt ;

	printf("*IWDG(%d)\r\n",delta);
	local_time_cnt = calendar.sec;
}
