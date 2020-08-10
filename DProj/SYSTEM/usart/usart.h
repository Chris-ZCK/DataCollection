#ifndef __USART_H
#define __USART_H
#include "stm32f4xx.h" 
#include "stdio.h"
#define EN_log_sd		1
#define EN_USART1_RX   	1		//ʹ�ܣ�1��/��ֹ��0������1����
#define USART_REC_LEN  	200  	//�����������ֽ��� 200

// Global use
extern u8  USART_RX_BUF[USART_REC_LEN];
extern u16 USART_RX_STA; 

void uart_init(u32 bound);
void F407USART1_SendByte(u8 Data);
void F407USART1_SendString(char *str);
#endif


