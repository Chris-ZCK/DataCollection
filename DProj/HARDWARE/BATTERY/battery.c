#include "battery.h"  // self
#include "usart2.h"
#include "delay.h"
#include "string.h"
int String2NumHex(u8 *str, int length)
{ 
	int  count = 1;
	int  result = 0;
	int  tmp = 0;
	int  i ;
	
	for (i = length - 1; i >= 0; i--)
	{
		if ((str[i] >= '0') && (str[i] <= '9'))
			tmp = str[i] - 48;
		else if ((str[i] >= 'a') && (str[i] <= 'f'))
			tmp = str[i] - 'a' + 10;
		else if ((str[i] >= 'A') && (str[i] <= 'F'))			
			tmp = str[i] - 'A' + 10;
		else
			tmp = 0;
		result = result + tmp * count;
		count = count * 16;
	}
	return result;
}

Battery_stats battery;

u8 battery_data_anay(void)
{
	u8 t=0;
	u8 len=0;	
	u8 res=0;
	// u8 buf[200];
	
	F407USART2_SendString("<15300420859E708F180>");
	delay_ms(1000);
	t=0;
	while(F407USART2_buffRead(&res) && (t++)<200)
	{
		if(res=='<')
		{
			//printf("%c",res);
			battery.info[len]=res;
			len++;
			break;
		}
	}	
	t=0;	
	while(F407USART2_buffRead(&res) && (t++)<200)
	{
		//printf("%c",res);
		battery.info[len]=res;
		len++;
		if(res=='>')
		{
			//printf("\r\n");
			break;
		}	
	}
	if(len>10)  // 符合长度的数据，则进行解析阶段
	{
		//printf("\r\n\r\n");//插入换行
			
		battery.info[len]=0;
		
		battery.SOH=String2NumHex(battery.info+22,2);
		//printf("SOH=%d\r\n",battery.SOH);
		battery.total_voltage=String2NumHex(battery.info+24,4);
		//printf("total_voltage=%f\r\n",battery.total_voltage);
		
		battery.charge_current=String2NumHex(battery.info+46,4);
		//printf("charge_current=%d\r\n",battery.charge_current);
		battery.discharge_current=String2NumHex(battery.info+50,4);
		//printf("discharge_current=%d\r\n",battery.discharge_current);

		battery.cell_temperature=String2NumHex(battery.info+56,2);
		//printf("cell_temperature=%d\r\n",battery.cell_temperature);
		
		battery.level=String2NumHex(battery.info+116,2);
		//printf("level=%d\r\n",battery.level);
		battery.remain_capacity=String2NumHex(battery.info+118,4);
		//printf("remain_capacity=%d\r\n",battery.remain_capacity);
		battery.max_capacity=String2NumHex(battery.info+122,4);
		//printf("max_capacity=%d\r\n",battery.max_capacity);
		return 1;
	}
	else
	{
		battery.total_voltage = -9999.0/2;
		battery.charge_current = -9999.0/10;
		battery.discharge_current= -9999.0/10;
		battery.cell_temperature= -9959;
		battery.level = -9999;
		battery.remain_capacity= -9999;
		battery.max_capacity= -9999;
		strcpy((char*)battery.info,"error");
		return 0;
	}
	
}

u8 battery_data_get(void)
{
	u8 i=0;
	IWDG_Feed();
	while(i++<3)
	{
		IWDG_Feed();//喂狗
		if(battery_data_anay())
		{
			printf("*Successd to get Battery data\r\n");
			return 1;
		}
	}
	return 0;
	printf("*Fail to get Battery data\r\n");
}