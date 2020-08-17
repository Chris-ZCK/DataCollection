/*
 * @Author: Howell
 * @Date: 2020-08-10 17:34:17
 * @LastEditTime: 2020-08-15 22:49:18
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \DProj\HARDWARE\KEY\key.c
 */
#include "key.h"
#include "delay.h"
/**
 * @description: key IO initial
 * KEY1	PA0
 * KEY2	PE0
 * KEY3	PE1
 * @param {type}  none
 * @return: none 
 */
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**
 * @description: Key value scan
 * @param u16 divtime Scan interval
 * @return: key value
 */
u8 KEY_Scan(u16 divtime)
{
	if ((KEY1 == 0 || KEY2 == 0 || KEY3 == 0))
	{
		delay_ms(divtime); //È¥¶¶¶¯
		if (KEY1 == 0)
			return 1;
		else if (KEY2 == 0)
			return 2;
		else if (KEY3 == 0)
			return 3;
	}
	return 0;
}


uint8_t key1_down = 0;	 // key1 state
uint8_t key2_down = 0;	 // key2 state
uint8_t key3_down = 0; 	 // key3 state

/**
 * @description: ??????????
 * ??????
 * @param {type} 
 * @return {type} 
 * 1 key1
 * 2 key2
 * 3 keye
 */
void key_scan_fun(void)
{
	u8 value;

	value = KEY_Scan(10);
	switch (value)
	{
		case 0:
			break;
		case 1:
			//printf("key.c:key1_down\r\n");
			//key1_down = 1;
			break;
		case 2:
			printf("key.c:key2_down\r\n");
			key2_down = 1;
			break;
		case 3:
			printf("key.c:key3_down\r\n");
			key3_down = 1;
			break;
	}
}
