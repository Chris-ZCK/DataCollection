/*
 * @Author: Howell
 * @Date: 2020-08-09 10:58:08
 * @LastEditTime: 2020-08-16 01:31:08
 * @LastEditors: Please set LastEditors
 * @Description: 
 * @FilePath: \Aproj\HARDWARE\PWR\power.c
 */
#include "stdio.h"
#include "power.h"
#include "delay.h"
/**
 * @description: 电源引脚初始化
 * @param {type} 
 * @return {type} 
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
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 普通输出模式
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // 推挽输出
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉
//	GPIO_Init(GPIOB, &GPIO_InitStructure);  // 初始化
	
	// POWER_OUT & POWER_CAM
	GPIO_SetBits(GPIOE, GPIO_Pin_11 | GPIO_Pin_12);
	// POWER_D & POWER_LE & POWER_USB & POWER_USB & POWER_OUT3V3 & POWER_OUT5
	GPIO_ResetBits(GPIOE, GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	// Open WiFi
	//GPIO_SetBits(GPIOB, GPIO_Pin_0);
	#if EN_LOG_PRINT>=2
	printf("[LOG]Power_Ctrl_Init\r\n");
	#endif
	// 打开传感器电源
	POWER_OUT3V3=1;
	POWER_OUT5=1;
}

/**
 * @description: 相机控制引脚初始化
 * @param {type} 
 * @return {type} 
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

	GPIO_ResetBits(GPIOE, GPIO_Pin_5 | GPIO_Pin_6);
}


/**
 * @description: 控制相机拍照
 * 相机被识别后控制拍照引脚
 * 拉高引脚通知相机拍照或录像（根据相机的设置），拉高时间0.5~1s，然后拉低
 * @param {type} 
 * @return: 
 */
void USB_Photograph(void)
{
	USB_TP = 1;
	printf("[LOG]拍摄...\r\n");
	delay_ms(900);
	USB_TP = 0;
	printf("[LOG]ing .....\r\n");
}


/**
 * @description: 控制器链接相机
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
		printf("[LOG]连接相机...\r\n");
	}
	else
	{
		USB_CNT = 0;
		printf("[LOG]恢复相机...\r\n");
	}
}


/**
 * @description: 连接相机
 * @param {type} 
 * @return {type} 
 */
void USB_CONNECT_ON()
{
	POWER_USB = 1;
}


/**
 * @description: 断开相机连接
 * @param {type} 
 * @return {type} 
 */
void USB_CONNECT_OFF()
{
	POWER_USB = 0;
}


/**
 * @description: 执行访问相机动作
 * 1. 连
 * @param {type} 
 * @return {type} 
 */
void openUSB(void)
{
	printf("[LOG]openUSB...\r\n");
	USB_Connecting(1);  // 首先通知相机处于连接模式
	USB_CONNECT_ON();  // 打开USB电源
}


/**
 * @description: 关闭相机连接的动作
 * @param {type} 
 * @return {type} 
 */
void closeUSB(void)
{
	printf("[LOG]closeUSB...\r\n");
	// delay_ms(10);
	USB_Connecting(0);
	// delay_ms(100);
	USB_CONNECT_OFF();
}
