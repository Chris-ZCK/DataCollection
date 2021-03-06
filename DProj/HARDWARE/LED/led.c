/*
 * @Author: Howell
 * @Date: 2020-08-10 17:34:17
 * @LastEditTime: 2020-08-15 05:30:07
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \USERd:\Documents\GitProject\DataCollection\DProj\HARDWARE\LED\led.c
 */
#include "led.h"
/**
 * @description: LED IO initial
 * @param {type} none
 * @return: none
 */
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	  //普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   //上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);			   //初始化

	GPIO_SetBits(GPIOE, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4); //GPIOE2,E3,E4设置高，灯灭
	#if EN_LOG_PRINT>2
	printf("[LOG]LED_Init\r\n");
	#endif
}


extern vu16 led_on_flag;  // 全局LED灯的开关

/**
 * @description: LED control
 * @param {type} none
 * @return: none
 */
// 绿灯
void LED_GREEN_ON(void)
{
	if(led_on_flag)
		ledGREEN = 0;
}
void LED_GREEN_OFF(void)
{
	ledGREEN = 1;
}
void LED_GREEN_NOT(void)
{
	if(led_on_flag)
		ledGREEN = !ledGREEN;
}
// 黄灯
void LED_YELLOW_ON(void)
{
	if(led_on_flag)
		ledYELLOW = 0;
}
void LED_YELLOW_OFF(void)
{
	ledYELLOW = 1;
}
void LED_YELLOW_NOT(void)
{
	if(led_on_flag)
		ledBLUE = !ledBLUE;
}
// 蓝灯
void LED_BLUE_ON(void)
{
	if(led_on_flag)
		ledBLUE = 0;
}
void LED_BLUE_OFF(void)
{
	ledBLUE = 1;
}
void LED_BLUE_NOT(void)
{
	if(led_on_flag)
		ledBLUE = !ledBLUE;
}
