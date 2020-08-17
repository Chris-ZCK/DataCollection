/*
 * @Author: Howell
 * @Date: 2020-08-10 17:34:17
 * @LastEditTime: 2020-08-17 06:45:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \USERd:\Documents\GitProject\DataCollection\DProj\HARDWARE\BATTERY\battery.c
 */
#include "battery.h"  // self
#include "usart2.h"
#include "delay.h"
#include "string.h"
/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */

// 握手指令
u8 instructHandshake[13] = {0xDD, 0x0D, 0x00, 0x0A, 0x05, 0x00, 0x01, 0x00, 0x00, 0x0B, 0x0E, 0xB8, 0x77};
// 获取电池数据
u8 instructBatteryInfo[9] = {0xDD, 0x0D, 0x03, 0x03, 0x01, 0x00, 0x15, 0xF8, 0x77};
// 获取电池串数据
u8 instructBatteryCell[9] = {0xDD, 0x0D, 0x03, 0x04, 0x01, 0x00, 0xD4, 0x49, 0x77};

void testtest(void)
{
	SendBatteryInstruct(instructHandshake,13);
	SendBatteryInstruct(instructBatteryInfo,9);
	SendBatteryInstruct(instructBatteryCell,9);
}
/**
 * @description: 发送获取电池信息的指令0
 * @param {type} 
 * @return {type} 
 */
void SendBatteryInstruct(u8 *instruct, u8 len)
{
	u8 i;
	#if EN_LOG_PRINT>2 
	printf("[LOG]SendBatteryInstruct[%d]:", len);  // 测试时候使用
	#endif
	for(i=0;i<len;i++)
	{
		F407USART2_SendByte(instruct[i]);
		//#if EN_LOG_PRINT>2 
		printf("%02X ",instruct[i]);  // 测试时候使用
		//#endif
	}
	printf("\r\n");
	//printf("[LOG]SendBatteryInstruct\r\n");
}

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


// 电池状态结构体
Battery_stats battery;

#if BATTERY_NEW_VERSION  // 旧版本电池
u8 battery_data_anay(void)
{
	u8 len=0;	
	u8 res=0;
	// u8 buf[200];
	
	F407USART2_SendString("<15300420859E708F180>");
	delay_ms(1000);

	// 找到起始点
	while(F407USART2_buffRead(&res))
	{
		if(res=='<')
		{
			//printf("%c",res);
			battery.info[len]=res;
			len++;
			break;
		}
	}	
	// 找到终点
	while(F407USART2_buffRead(&res))
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
	// 开始解析
	if(len>10)  // 符合长度的数据，则进行解析阶段
	{
		//printf("\r\n\r\n");//插入换行
			
		battery.info[len]=0;
		
		battery.SOH=String2NumHex(battery.info+22,2);
		printf("SOH=%d\r\n",battery.SOH);
		battery.total_voltage=String2NumHex(battery.info+24,4)*2;
		printf("total_voltage=%d\r\n",battery.total_voltage);
		
		battery.charge_current=String2NumHex(battery.info+46,4)*10;
		printf("charge_current=%d\r\n",battery.charge_current);
		battery.discharge_current=String2NumHex(battery.info+50,4)*10;
		printf("discharge_current=%d\r\n",battery.discharge_current);

		battery.cell_temperature=String2NumHex(battery.info+56,2)-40;
		printf("cell_temperature=%d\r\n",battery.cell_temperature);
		
		battery.level=String2NumHex(battery.info+116,2);
		printf("level=%d\r\n",battery.level);
		battery.remain_capacity=String2NumHex(battery.info+118,4)*100;
		printf("remain_capacity=%d\r\n",battery.remain_capacity);
		battery.max_capacity=String2NumHex(battery.info+122,4)*100;
		printf("max_capacity=%d\r\n",battery.max_capacity);
		return 1;
	}
	else
	{
		battery.total_voltage = -9999;
		battery.charge_current = -9999;
		battery.discharge_current= -9999;
		battery.cell_temperature= -9999;
		battery.level = -9999;
		battery.remain_capacity= -9999;
		battery.max_capacity= -9999;
		strcpy((char*)battery.info,"error");
		return 0;
	}
}
#else  // 新版本电池


/**
 * @description: 特定函数，将2位16进制转为10进制
 * @param {type} 
 * @return {type} 
 */
u16 MyHex2Dec2(u8* addr, u8 n)
{
	return addr[0]*256+addr[n-1];
}
// * example:
// * send: DD 0D 03 03 01 00 15 F8 77
// * recv: DD 0D 03 41 13 06 55 20 01 00 00 16 00 0A 0C AC 0C AF 0C B0 00 33 0C AC 11 B8 77
u8 battery_data_anay(void)
{
	u8 t=0;
	u8 len=0;	
	u8 res=0;

	SendBatteryInstruct(instructBatteryInfo,9);  // 发送指令
	delay_ms(1000);
	
	printf("[LOG]Battery datas:");
	while(F407USART2_buffRead(&res))  // 寻找帧头
	{
		if(res==0xDD)
		{
			printf("%02X ",res);
			battery.info[len++]=res;
			break;
		}
	}		

	while(F407USART2_buffRead(&res) && (t++)<200)  // 寻找帧尾
	{
		printf("%02X ",res);
		battery.info[len++]=res;
		if(res==0x77)
		{
			printf("\r\n");
			battery.info[len]=0;
			break;
		}	
	}

	// 解析
	if(battery.info[1]==0x0D)  // 判断帧头
	{
		u8  versionCode=0;
		u16 instructCode=0;
		u16 len_cal=0;  // 帧头1+版本1+指令码2+长度1+负荷n+crc2+帧尾1,即len_rcv+8
		u8  len_rcv=0; 
		u16 crc_cal=0;
		u16 crc_rcv=0;
	
		versionCode = battery.info[1];
		instructCode = (battery.info[2] << 8) + battery.info[3];

		len_cal = battery.info[4] + 8;
		len_rcv = len;

		crc_cal = ModBusCRC((uint8_t *)battery.info+1, len-4);  // 总长度减去帧头帧尾和crc
		crc_rcv = (battery.info[len-3]<<8) + battery.info[len-2];
		#if IGNORE_CHECK_CRC
		crc_cal = crc_rcv;	
		#endif

		if(versionCode==0x0D)
		{
			if(len_cal==len_rcv)
			{
				if(crc_cal==crc_rcv)
				{
					switch (instructCode)
					{
					case 0x0331:
							battery.SOH=-9999;  // 无此参数
							//printf("SOH=%d\r\n",battery.SOH);
							battery.total_voltage=MyHex2Dec2(battery.info+12,2)*10;
							printf("[INFO]total_voltage=%dmV\r\n",battery.total_voltage);
							
							battery.charge_current=MyHex2Dec2(battery.info+14,2)*10;
							printf("[INFO]charge_current=%dmA\r\n",battery.charge_current);
							battery.discharge_current=MyHex2Dec2(battery.info+14,2)*10;
							printf("[INFO]discharge_current=%dmA\r\n",battery.discharge_current);

							battery.cell_temperature=-9999;
							//printf("[WARNING]cell_temperature=%dC\r\n",battery.cell_temperature);
							
							battery.level=MyHex2Dec2(battery.info+18,2);
							printf("[INFO]level=%d/10000\r\n",battery.level);
							battery.remain_capacity=MyHex2Dec2(battery.info+16,2);
							printf("[INFO]remain_capacvity=%dmAh\r\n",battery.remain_capacity);
							battery.max_capacity=-9999;
							//printf("[WARNING]max_capacity=%d\r\n",battery.max_capacity);
						
							printf("[LOG]Anay battery successfully.\r\n");
							return 1;					
					default:
						printf("[WARNING]The instruct is not extist\r\n");
						break;
					}
					
				}	
				else
				{
					printf("[WARNING]CRC check wrong\r\n");
				}
				
			}
			else
			{
				printf("[WARNING]length wrong\r\n");
			}
			
		}
		else
		{
			printf("[WARNING]versionCode wrong\r\n");
		}
	}	
	// 如果未能成功解析，则退出
	battery.total_voltage = -9999;
	battery.charge_current = -9999;
	battery.discharge_current= -9999;
	battery.cell_temperature= -9999;
	battery.level = -9999;
	battery.remain_capacity= -9999;
	battery.max_capacity= -9999;
	strcpy((char*)battery.info,"error");
	return 0;
}
#endif

u8 battery_data_get(void)
{
	u8 i=0;
	IWDG_Feed();
	while(i++<3)
	{
		IWDG_Feed();//喂狗
		if(battery_data_anay())
		{
			printf("[LOG]Successd to get Battery data\r\n");
			return 1;
		}
	}
	printf("[WARN]Fail to get Battery data\r\n");
	return 0;
}


