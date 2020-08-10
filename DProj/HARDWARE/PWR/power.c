#include "power.h"
#include "delay.h"
#include "iwdg.h"

/**
 * @description: Power_Ctrl_Init, 电源控制引脚初始化
 * @param 
 * @return: 
 */
void Power_Ctrl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOE, ENABLE);  // 使能GPIOE时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // 初始化
	
	// POWER_OUT & POWER_CAM
	GPIO_SetBits(GPIOE, GPIO_Pin_11 | GPIO_Pin_12 );
	// POWER_MP & POWER_MIC & POWER_USB & POWER_OUT3V3 & POWER_OUT5
	GPIO_ResetBits(GPIOE, GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	

	//INIO1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // 初始化
	GPIO_ResetBits(GPIOE, GPIO_Pin_7);
	//OUTIO1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);  // 初始化
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
	#if EN_log_print>=2
	printf("*Power_Ctrl_Init\r\n");
	#endif
	// 打开传感器电源
	POWER_OUT3V3=1;
	POWER_OUT5=1;
}

/**
 * @description: Cam_Crtl_Init, 相机控制引脚初始化
 * @param
 * @return: 
 */
void Cam_Crtl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //使能GPIOE时钟

	//GPIOE5,E6初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // 初始化

	GPIO_ResetBits(GPIOD, GPIO_Pin_5 | GPIO_Pin_6);
}

//初始化IIC
void Relay_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_GPIOC, ENABLE );	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化

	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
}

/**
 * @description: USB_Photograph
 * 相机被识别后控制拍照引脚
 * 拉高引脚通知相机拍照或录像（根据相机的设置），拉高时间0.5~1s，然后拉低
 * @param {type} 
 * @return: 
 */
void USB_Photograph(void)
{
	USB_PHO = 1;
	printf("拍摄...\r\n");
	delay_ms(1000);
	USB_PHO = 0;
	printf("ing .....\r\n");
}

/**
 * @description: USB_Connecting
 * 控制相机被USB访问引脚
 * 控制相机是否处于连接状态，当电平拉高时，告知相机外部设备要接USB了，外部设备完成USB连接后可以把输入引脚3拉低，即可恢复相机
 * @param {type} 
 * @return: 
 */
void USB_Connecting(u8 ctrl)
{
	if (ctrl == 1)
	{
		USB_CNT = 1;
		printf("连接相机...\r\n");
	}
	else
	{
		USB_CNT = 0;
		printf("恢复相机...\r\n");
	}
}

// 5V USB电源控制引脚
void USB_CONNECT_ON()
{
	POWER_USB = 1;
}
void USB_CONNECT_OFF()
{
	POWER_USB = 0;
}

// 模拟打开相机
void openUSB(void)
{
	printf("openUSB...\r\n");
	IWDG_Feed();
	USB_Connecting(1);  // 首先通知相机处于连接模式
	//delay_ms(10);
	USB_CONNECT_ON();  // 打开USB电源
	//delay_ms(100);
}

// 模拟关闭相机
void closeUSB(void)
{
	printf("closeUSB...\r\n");
	// delay_ms(10);
	USB_Connecting(0);
	// delay_ms(100);
	USB_CONNECT_OFF();
}


// 模拟打开负载
void openReLoad(void)
{
	IWDG_Feed();
	printf("*openReLoad...\r\n");
	POWER_OUTIO1=1;
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
}

// 模拟关闭负载
void closeReLoad(void)
{
	printf("*closeReLoad...\r\n");
	POWER_OUTIO1=0;
	delay_ms(100);
}

// 模拟打开
u8 openOutputLoad(void)
{
	u8 i=0;
	IWDG_Feed();
	while(i++<3)
	{
		if(Relay_IO_IN==1)
		{
			printf("*succeed openOutputLoad...\r\n");
			return 1;
		}
		else
		{
			printf("*try openOutputLoad...cnt:%d\r\n", i);
			POWER_INIO1=1;
			delay_ms(300);
			POWER_INIO1=0;
			delay_ms(1000);
			delay_ms(1000);
		}
		
	}
	printf("*!fail openOutputLoad...\r\n");
	return 0;
}

u8 closeOutputload(void)
{
	u8 i=0;
	IWDG_Feed();
	while(i++<3)
	{
		if(Relay_IO_IN==0)
		{
			printf("*succeed closeOutputload...\r\n");
			return 1;
		}
		else
		{
			printf("*try closeOutputload...cnt:%d\r\n", i);
			POWER_INIO1=1;
			delay_ms(300);
			POWER_INIO1=0;
			delay_ms(1000);
			delay_ms(1000);
		}	
	}
	printf("*!fail closeOutputload...\r\n");
	return 0;
}


