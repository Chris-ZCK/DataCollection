#include "rtc.h"
#include "delay.h"
#include "usart.h" 
#include "power.h"
#include "iwdg.h"

_calendar_obj calendar;	//日历结构体


NVIC_InitTypeDef   NVIC_InitStructure;

//RTC时间设置
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(0~31)
//week:星期(1~7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}

//RTC初始化
//返回值:0,初始化成功;
//       1,LSE开启失败;
//       2,进入初始化模式失败;
extern u32 last_timecount;
u8 My_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry=0X1FFF; 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
	printf("[LOG]My RTC Init\r\n");

	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//是否第一次配置?
	{
		// config LSE
		//RCC_SYSCLKSource_LSE
//		RCC_LSEConfig(RCC_LSE_ON);//LSE 开启   
//		printf("*rtc RCC_LSEConfig start\r\n");
//		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
//		{
//			retry++;
//			delay_ms(10);
//		}
//		if(retry==0)
//		{
//			printf("*!fail1 rtc RCC LSEConfig\r\n");
			// config LSI
			//RCC_SYSCLKSource_LSI
			RCC_LSICmd(ENABLE);//LSI 开启     
			printf("[LOG]rtc RCC_LSICmd config start\r\n");
			while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
			{
				retry++;
				delay_ms(10);
			}
			if(retry==0)
			{
				printf("[ERROR]fail2 rtc RCC LSICmd\r\n");
				return 1;		//LSI 开启失败. 
			}
			else
			{
				printf("[LOG]rtc RCC_LSICmd config OK\r\n");
			}
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);     //设置RTC时钟(RTCCLK),选择LSI作为RTC时钟    
//		}
//		else
//		{
//			printf("*rtc RCC_LSECmd config OK\r\n");
//			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    		
//		}
		
		RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟 
		
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式
		RTC_Init(&RTC_InitStructure);
 
		RTC_Set_Time(18,17,0,RTC_H12_AM);  //设置时间
		RTC_Set_Date(19,9,23,1);		//设置日期
		printf("[LOG]rtc RCC_LSEConfig2\r\n");
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//标记已经初始化过了
	} 
	last_timecount = RTC_GetCounter();  // 获取当前的时间(s)
	return 0;
}

int my_RTC_Set_Time(_calendar_obj calendar)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry=0X1FFF; 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
	printf("[LOG]RTC Set Time\r\n");

	// config LSE
	//RCC_SYSCLKSource_LSE
//	RCC_LSEConfig(RCC_LSE_ON);//LSE 开启   
//	printf("*rtc RCC_LSEConfig start\r\n");
//	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
//	{
//		retry++;
//		delay_ms(10);
//	}
//	if(retry==0)
//	{
//		printf("*!fail1 rtc RCC LSEConfig\r\n");
		// config LSI
		//RCC_SYSCLKSource_LSI
		RCC_LSICmd(ENABLE);//LSI 开启     
		printf("[LOG]rtc RCC_LSICmd config start\r\n");
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		{
			retry++;
			delay_ms(10);
		}
		if(retry==0)
		{
			printf("[ERROR]fail2 rtc RCC LSICmd\r\n");
			return 1;		//LSI 开启失败. 
		}
		else
		{
			printf("[LOG]rtc RCC_LSICmd config OK\r\n");
		}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);     //设置RTC时钟(RTCCLK),选择LSI作为RTC时钟    
//	}
//	else
//	{
//		printf("*rtc RCC_LSECmd config OK\r\n");
//		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    		
//	}
	
	RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟 

	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)
	RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)
	RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式
	RTC_Init(&RTC_InitStructure);

	printf("[INFO]DATA-B:%d-%d-%d	Time:%d:%d:%d@\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
	RTC_Set_Time(calendar.hour,calendar.min,calendar.sec,RTC_H12_AM);  //设置时间
	RTC_Set_Date(calendar.w_year-2000,calendar.w_month,calendar.w_date,6);  //设置日期
	

	calendar_get_time(&calendar);
	calendar_get_date(&calendar);
	printf("[INFO]DATA-A:%d-%d-%d	Time:%d:%d:%d&\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
	
	return 0;

}

//设置闹钟时间(按星期闹铃,24小时制)
//week:星期几(1~7) @ref  RTC_Alarm_Definitions
//hour,min,sec:小时,分钟,秒钟
void RTC_Set_AlarmA(u8 week,u8 hour,u8 min,u8 sec)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);  // 关闭闹钟A 
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;  // 小时
	RTC_TimeTypeInitStructure.RTC_Minutes=min;  // 分钟
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;  // 秒
	RTC_TimeTypeInitStructure.RTC_H12=RTC_H12_AM;
  
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay=week;  // 星期
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel=RTC_AlarmDateWeekDaySel_WeekDay;  // 按星期闹
	RTC_AlarmTypeInitStructure.RTC_AlarmMask=RTC_AlarmMask_None;  // 精确匹配星期，时分秒
	RTC_AlarmTypeInitStructure.RTC_AlarmTime=RTC_TimeTypeInitStructure;
	RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_A,&RTC_AlarmTypeInitStructure);
 
	
	RTC_ClearITPendingBit(RTC_IT_ALRA);  // 清除RTC闹钟A的标志
	EXTI_ClearITPendingBit(EXTI_Line17);  // 清除LINE17上的中断标志位 
	
	RTC_ITConfig(RTC_IT_ALRA,ENABLE);  // 开启闹钟A中断
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);  // 开启闹钟A 
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;  // LINE17
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  // 中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;   // 上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;  // 使能LINE17
	EXTI_Init(&EXTI_InitStructure);  // 配置

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;  // 抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;  // 子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);  // 配置
}

//周期性唤醒定时器设置  
/*wksel:  @ref RTC_Wakeup_Timer_Definitions
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
*/
//cnt:自动重装载值.减到0,产生中断.
void RTC_Set_WakeUp(u32 wksel,u16 cnt)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RTC_WakeUpCmd(DISABLE);  // 关闭WAKE UP
	
	RTC_WakeUpClockConfig(wksel);  // 唤醒时钟选择
	
	RTC_SetWakeUpCounter(cnt);  // 设置WAKE UP自动重装载寄存器
	
	
	RTC_ClearITPendingBit(RTC_IT_WUT);  // 清除RTC WAKE UP的标志
    EXTI_ClearITPendingBit(EXTI_Line22);  // 清除LINE22上的中断标志位 
	 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);  // 开启WAKE UP定时器中断
	RTC_WakeUpCmd( ENABLE);  // 开启WAKE UP定时器　
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;  // LINE22
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  // 中断事件
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  // 上升沿触发 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;  // 使能LINE22
    EXTI_Init(&EXTI_InitStructure);  // 配置
 
 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;  // 抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;  // 子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);  // 配置
}

//char wakeup_flag;
// RTC闹钟中断服务函数
void RTC_Alarm_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A中断
	{
		RTC_ClearFlag(RTC_FLAG_ALRAF);//清除中断标志

		calendar_get_time(&calendar);
		calendar_get_date(&calendar);
		RTC_Set_AlarmA(4,calendar.hour,calendar.min,calendar.sec+20);
		//printf("DATA:%d-%d-%d	Time:%d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
  
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断线17的中断标志 											 
}

// RTC WAKE UP中断服务函数
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF)==SET)  // WK_UP中断
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);  // 清除中断标志
//		uart_init(115200);
//		printf("RTC wake up\r\n");
//		LED0 = 0;LED1 = 0;
//		delay_ms(1000);
		//LED1=!LED1; 
		//wakeup_flag = 2;
	}   
	EXTI_ClearITPendingBit(EXTI_Line22); //  清除中断线22的中断标志 								
}
 
//系统进入待机模式// 休眠时长
void Sys_Enter_Standby(u32 time_seconed)
{	
	u32 now_time;
	int time_delta;	
	IWDG_Feed();
	now_time = get_time_cnt();
	time_delta = now_time - cycle.time_stamp;  // 正常时>0,或者now_time+3600- cycle.time_stamp>0 ，否则异常，更新时间戳时间
	if(time_delta <0)
		time_delta+=3600;
	printf("[SYS]Sys_Enter_Standby %d |fun:%d|run:%d\r\n\r\n",time_seconed, function_f, time_delta);
	
	printf("^^^^^^^^^^^^^^normal sleep^^^^^^^^^^^^^^^\r\n");
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n\r\n\r\n\r\n");
	if(sd_ready_flag ==0xAA)
		mf_sync();
	RCC_AHB1PeriphResetCmd(0X04FF,ENABLE);//复位所有IO口
	/*关闭所有外设*/
	POWER_D =0;
	POWER_USB=0;
	POWER_CAM=0;
	POWER_OUT3V3=0;
	POWER_OUT5=0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟		 
	 
	PWR_BackupAccessCmd(ENABLE);//后备区域访问使能

	//这里我们就直接关闭相关RTC中断
	RTC_ITConfig(RTC_IT_TS|RTC_IT_WUT|RTC_IT_ALRB|RTC_IT_ALRA,DISABLE);//关闭RTC相关中断，可能在RTC实验打开了。/*1*/
	RTC_ClearITPendingBit(RTC_IT_TS|RTC_IT_WUT|RTC_IT_ALRB|RTC_IT_ALRA);//清楚RTC相关中断标志位。/*2*/
	
	PWR_ClearFlag(PWR_FLAG_WU);//清除Wake-up 标志/*3*/
	RTC_ITConfig(RTC_IT_WUT,ENABLE);//开启自动唤醒单元/*4*/
	PWR_WakeUpPinCmd(ENABLE);//进用于设置WKUP用于唤醒
	
	//calendar_get_time(&calendar);
	//calendar_get_date(&calendar);
	//RTC_Set_AlarmA(4,calendar.hour,calendar.min,calendar.sec+20);
	//printf("DATA:%d-%d-%d	Time:%d:%d:%d\r\n",calendar.w_year,calend
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,time_seconed);
	PWR_EnterSTANDBYMode();	//进入待机模式/*5*/

}

//获取RTC时间
//*hour,*min,*sec:小时,分钟,秒钟 
//*ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM.
void RTC_Get_Time(u8 *hour,u8 *min,u8 *sec,u8 *ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_GetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
	*hour=RTC_TimeTypeInitStructure.RTC_Hours;
	*min=RTC_TimeTypeInitStructure.RTC_Minutes;
	*sec=RTC_TimeTypeInitStructure.RTC_Seconds;
	*ampm=RTC_TimeTypeInitStructure.RTC_H12;
}
//获取RTC日期
//*year,*mon,*date:年,月,日
//*week:星期
void RTC_Get_Date(u8 *year,u8 *month,u8 *date,u8 *week)
{
	RTC_DateTypeDef  RTC_DateStruct;
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

	*year=RTC_DateStruct.RTC_Year;
	*month=RTC_DateStruct.RTC_Month;
	*date=RTC_DateStruct.RTC_Date;
	*week=RTC_DateStruct.RTC_WeekDay; 
}

 //得到时间
//calendarx:日历结构体
void calendar_get_time(_calendar_obj *calendarx)
{
	u8 ampm;
	RTC_Get_Time(&calendarx->hour,&calendarx->min,&calendarx->sec,&ampm);//得到时间	
}
//得到日期
//calendarx:日历结构体
void calendar_get_date(_calendar_obj *calendarx)
{
	u8 year;
	RTC_Get_Date(&year,&calendarx->w_month,&calendarx->w_date,&calendarx->week);
	calendar.w_year=year+2000;//从2000年开始算起
}


//平年的月份日期表
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};


//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	


//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码
u32 RTC_Get_C(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(u32)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(u32)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(u32)hour*3600;//小时秒钟数
    seccount+=(u32)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去
 
 	return seccount;	
}    


/**
 * @description: 获取秒计数
 * @param {type} 
 * @return {type} 
 */
u32 RTC_GetCounter(void)
{
	u16 seccount=0;
	u16 syear, smon, sday, hour, min, sec;

	RTC_DateTypeDef RTC_DateStruct;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	syear=RTC_DateStruct.RTC_Year;
	smon=RTC_DateStruct.RTC_Month;
	sday=RTC_DateStruct.RTC_Date;

	RTC_GetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	hour=RTC_TimeTypeInitStructure.RTC_Hours;
	min=RTC_TimeTypeInitStructure.RTC_Minutes;
	sec=RTC_TimeTypeInitStructure.RTC_Seconds;

	seccount = RTC_Get_C(syear+2000, smon, sday, hour, min, sec);
	// printf("seccount=%d,%d,%d,%d,%d,%d,%d,", seccount, syear, smon, sday, hour, min, sec);	
	return seccount;
}
