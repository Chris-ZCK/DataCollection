#include "SHT2x.h"
#include "delay.h"
#include "myiic_sht20.h"
#include<stdio.h>

SHT2x_data SHT20;
u8 SHT2x_Init(void)
{
	u8 err;
	IIC_SHT20_Init();
	err = SHT2x_SoftReset();
	return err;
}

u8 SHT2x_SoftReset(void) //SHT20软件复位
{
	u8 err=0;
	
	delay_ms(250);
	IIC_SHT20_Start();
	IIC_SHT20_Send_Byte(0x88);
	err = IIC_SHT20_Wait_Ack();
	IIC_SHT20_Send_Byte(0x21);
	err = IIC_SHT20_Wait_Ack();
	IIC_SHT20_Send_Byte(0x30);
	err = IIC_SHT20_Wait_Ack();
	IIC_SHT20_Stop();
	delay_ms(150);
	return err;
}
static double shtTEMP,shtHUMI;
void SHT2x_GetTempandHumiPoll(void)
{
	double TEMP,HUMI;
	u8 ack, tmp1, tmp2,tmp3,tmp4;
	u16 SRH;
	u16 ST;
	u16 i=0;
	IIC_SHT20_Start();				//发送IIC开始信号
	IIC_SHT20_Send_Byte(I2C_ADR_W);			//IIC发送一个字节 
	ack = IIC_SHT20_Wait_Ack();	
	IIC_SHT20_Send_Byte(TRIG_TEMP_MEASUREMENT_POLL);
	ack = IIC_SHT20_Wait_Ack();
	IIC_SHT20_Send_Byte(0x00);
	ack = IIC_SHT20_Wait_Ack();

	do {
		delay_ms(100);               
		IIC_SHT20_Start();				//发送IIC开始信号
		IIC_SHT20_Send_Byte(I2C_ADR_R);	
		i++;
		ack = IIC_SHT20_Wait_Ack();
		if(i==10)break;
	}while(ack!=0);
	
	tmp1 = IIC_SHT20_Read_Byte(1);
	tmp2 = IIC_SHT20_Read_Byte(1);
	IIC_SHT20_Read_Byte(1);
	tmp3 = IIC_SHT20_Read_Byte(1);
	tmp4 = IIC_SHT20_Read_Byte(1);
	IIC_SHT20_Read_Byte(0);
	IIC_SHT20_Stop();

	SRH = (tmp3 << 8) | (tmp4 << 0);
	SRH &= ~0x0003;
	HUMI = ((double)SRH * 0.00190735) - 6;

	ST = (tmp1 << 8) | (tmp2 << 0);
	ST &= ~0x0003;
	TEMP = ((double)ST * 0.00268127) - 46.85;

	shtTEMP = TEMP;
	shtHUMI = HUMI;
}

u8 SHT2x_GetTempandHumiPollAvg(void)
{
	u8 i=0;
	while(i++<10)
	{
		SHT2x_GetTempandHumiPoll();
		if(shtTEMP>100 || shtTEMP<-100)
		{
			SHT2x_Init();
			printf("*!Sht31 error TEMP:%f,HUMI:%f\r\n",shtTEMP, shtHUMI);
			delay_ms(500);
			continue;
		}

		SHT20.TEMP_POLL = shtTEMP;
		SHT20.HUMI_POLL = shtHUMI;
		printf("*Sht31 woking well, temp:%f, humi:%f\r\n",shtTEMP,shtHUMI);
		return 1;
	}
	SHT20.TEMP_POLL = -9999;
	SHT20.HUMI_POLL = -9999;
	printf("*!Sht31 woking error\r\n");
	return 0;
	
	
}
