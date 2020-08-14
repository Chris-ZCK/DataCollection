#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h"  

//===============================================
//=================Ó²¼þ³õÊ¼»¯
//===============================================
// Global use
void USART2_init(u32 bound);
void F407USART2_SendByte(uint8_t Data);
void F407USART2_SendBytes(char *Data,uint16_t leng);
void F407USART2_SendString(char *str);

// Just for UART_TCPbuff
#define F407USART2_RECEIVE_BUFF_ENABLE		1
#if (F407USART2_RECEIVE_BUFF_ENABLE==1)
#define F407USART2_RECEIVE_BUFF_SIZE		1024
uint8_t F407USART2_buffWrite(uint8_t data);
uint8_t F407USART2_buffWrites(uint8_t *data,uint16_t length);

uint8_t F407USART2_buffRead(uint8_t *data);
uint16_t F407USART2_buffReads(uint8_t *data,uint16_t length);
uint16_t F407USART2_buffLength(void);
#endif // F407USART2_RECEIVE_BUFF_ENABLE
#endif













