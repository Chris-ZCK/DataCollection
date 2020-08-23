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
 * @description: ��Դ���ų�ʼ��
 * @param {type} 
 * @return {type} 
 */
void Power_Ctrl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOE, ENABLE);  // ʹ��GPIOEʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // ����
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // ��ʼ��
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ��ͨ���ģʽ
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // �������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // ����
//	GPIO_Init(GPIOB, &GPIO_InitStructure);  // ��ʼ��
	
	// POWER_OUT & POWER_CAM
	GPIO_SetBits(GPIOE, GPIO_Pin_11 | GPIO_Pin_12);
	// POWER_D & POWER_LE & POWER_USB & POWER_USB & POWER_OUT3V3 & POWER_OUT5
	GPIO_ResetBits(GPIOE, GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	// Open WiFi
	//GPIO_SetBits(GPIOB, GPIO_Pin_0);
	#if EN_LOG_PRINT>=2
	printf("[LOG]Power_Ctrl_Init\r\n");
	#endif
	// �򿪴�������Դ
	POWER_OUT3V3=1;
	POWER_OUT5=1;
}

/**
 * @description: ����������ų�ʼ��
 * @param {type} 
 * @return {type} 
 */
void Cam_Crtl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //ʹ��GPIOEʱ��

	//GPIOE5,E6��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // ����
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // ��ʼ��

	GPIO_ResetBits(GPIOE, GPIO_Pin_5 | GPIO_Pin_6);
}


/**
 * @description: �����������
 * �����ʶ��������������
 * ��������֪ͨ������ջ�¼�񣨸�����������ã�������ʱ��0.5~1s��Ȼ������
 * @param {type} 
 * @return: 
 */
void USB_Photograph(void)
{
	USB_TP = 1;
	printf("[LOG]����...\r\n");
	delay_ms(900);
	USB_TP = 0;
	printf("[LOG]ing .....\r\n");
}


/**
 * @description: �������������
 * ���������USB��������
 * ��������Ƿ�������״̬������ƽ����ʱ����֪����ⲿ�豸Ҫ��USB�ˣ��ⲿ�豸���USB���Ӻ���԰���������3���ͣ����ɻָ����
 * @param {type} 
 * @return: 
 */
void USB_Connecting(u8 ctrl)
{
	if (ctrl == 1)
	{
		USB_CNT = 1;
		printf("[LOG]�������...\r\n");
	}
	else
	{
		USB_CNT = 0;
		printf("[LOG]�ָ����...\r\n");
	}
}


/**
 * @description: �������
 * @param {type} 
 * @return {type} 
 */
void USB_CONNECT_ON()
{
	POWER_USB = 1;
}


/**
 * @description: �Ͽ��������
 * @param {type} 
 * @return {type} 
 */
void USB_CONNECT_OFF()
{
	POWER_USB = 0;
}


/**
 * @description: ִ�з����������
 * 1. ��
 * @param {type} 
 * @return {type} 
 */
void openUSB(void)
{
	printf("[LOG]openUSB...\r\n");
	USB_Connecting(1);  // ����֪ͨ�����������ģʽ
	USB_CONNECT_ON();  // ��USB��Դ
}


/**
 * @description: �ر�������ӵĶ���
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
