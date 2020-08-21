/********************************************************************
 * M8266HostIf.c
 * .Description
 *     Source file of M8266WIFI Host Interface 
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"	
#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"

/***********************************************************************************
 * M8266HostIf_GPIO_SPInCS_nRESET_Pin_Init                                         *
 * Description                                                                     *
 *    To initialise the GPIOs for SPI nCS and nRESET output for M8266WIFI module   *
 *    You may update the macros of GPIO PINs usages for nRESET from brd_cfg.h      *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    None                                                                         *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_GPIO_CS_RESET_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	//Initial STM32's GPIO for M8266WIFI_SPI_nCS
	M8266WIFI_SPI_nCS_GPIO_RCC_CLOCK_EN;  								// enable nCS GPIO clock
	GPIO_OUTPUT_HIGH(M8266WIFI_SPI_nCS_GPIO,M8266WIFI_SPI_nCS_PIN);		// nCS output high initially	
	GPIO_InitStructure_AS_GPIO_OUTPUT(M8266WIFI_SPI_nCS_GPIO,M8266WIFI_SPI_nCS_PIN);
	
	//Initial STM32's GPIO for M8266WIFI_nRESET
	M8266WIFI_nRESET_GPIO_RCC_CLOCK_EN;  								  // enable nRESET GPIO clock
	GPIO_OUTPUT_HIGH(M8266WIFI_nRESET_GPIO,M8266WIFI_nRESET_PIN);		  // nRESET output high initially	
	GPIO_InitStructure_AS_GPIO_OUTPUT(M8266WIFI_nRESET_GPIO,M8266WIFI_nRESET_PIN);
}
/***********************************************************************************
 * M8266HostIf_SPI_Init                                                            *
 * Description                                                                     *
 *    To initialise the SPI Interface for M8266WIFI module                         *
 *    You may update the macros of SPI usages for nRESET from brd_cfg.h            *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    None                                                                         *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_SPI_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;
#if (M8266WIFI_SPI_INTERFACE_NO == 1)  // if use SPI1 GPIOA5/6/7 for SCK/MISO/MOSI
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 							// enable the SPI1 GPIO CLOCK
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;									// SET GPIOs for SPI1
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);							// Enable GPIO Alternate for SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);						 	//  Enable SPI1 CLOCK
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);							//  Start to Reset SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);							//  Stop reseting SPI1	

#elif (M8266WIFI_SPI_INTERFACE_NO == 2)    // if use SPI2 GPIOB13/14/15 for SCK/MISO/MOSI
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 							// enable the SPI2 GPIO CLOCK
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;									// SET GPIOs for SPI2
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);							// Enable GPIO Alternate for SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);						  	//  Enable SPI2 CLOCK
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);							//  Start to Reset SPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);							//  Stop reseting SPI2
		
#elif (M8266WIFI_SPI_INTERFACE_NO == 3)    // if use SPI3 GPIOC10/11/12 for SCK/MISO/MOSI
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 							// enable the SPI3 GPIO CLOCK
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;								 	// SET GPIOs for SPI3
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3);							// Enable GPIO Alternate for SPI3
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);						    //  Enable SPI3 CLOCK
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE);							//  Start to Reset SPI3
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, DISABLE);							//  Stop reseting SPI3	
#else
	#error Invalid M8266WIFI_SPI_INTERFACE_NO defined in brd_cfg.h 
#endif

	SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;    
	SPI_InitStructure.SPI_Mode 				= SPI_Mode_Master;									  
	SPI_InitStructure.SPI_DataSize 			= SPI_DataSize_8b;									 
	SPI_InitStructure.SPI_CPOL 				= SPI_CPOL_Low;		                   //  !!!! M8266WIFI SPI Works at CPOL=LOW, CPHA=1Edge
	SPI_InitStructure.SPI_CPHA 				= SPI_CPHA_1Edge;	
	SPI_InitStructure.SPI_NSS 				= SPI_NSS_Soft;									
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit 			= SPI_FirstBit_MSB;						//  !!!! M8266WIFI SPI Works with MSB First							
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(M8266WIFI_INTERFACE_SPI, &SPI_InitStructure);
	SPI_Cmd(M8266WIFI_INTERFACE_SPI, ENABLE); 										//	Enable SPI Interface
	//M8266HostIf_SPI_ReadWriteByte(0xff);											//  Start Trasmission	
#ifdef M8266WIFI_SPI_ACCESS_USE_DMA	
	SPI_I2S_DMACmd(M8266WIFI_INTERFACE_SPI, SPI_I2S_DMAReq_Rx, ENABLE);			//  Enable SPI Rx DMA
	SPI_I2S_DMACmd(M8266WIFI_INTERFACE_SPI, SPI_I2S_DMAReq_Tx, ENABLE);			//  Enable SPI Tx DMA
#endif


}
/***********************************************************************************
 * M8266HostIf_SPI_SetSpeed                                                        *
 * Description                                                                     *
 *    To setup the SPI Clock Speed for M8266WIFI module                            *
 * Parameter(s):                                                                   *
 *    SPI_BaudRatePrescaler: SPI BaudRate Prescaler                                *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_SPI_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));		// 判断有效性
	M8266WIFI_INTERFACE_SPI->CR1	&=0XFFC7;															// 位3-5清零，用来设置时钟
	M8266WIFI_INTERFACE_SPI->CR1	|=SPI_BaudRatePrescaler;							// 设置SPI速度 
	SPI_Cmd(M8266WIFI_INTERFACE_SPI,ENABLE); 													// 使能SPIx	
} 

#ifdef M8266WIFI_SPI_ACCESS_USE_DMA
void M8266HostIf_SPI_DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;    

	//Initialization of SPI TX DMA
	RCC_AHB1PeriphClockCmd(M8266WIFI_INTERFACE_SPI_TX_RCC_Periph_DMA, ENABLE);  
	DMA_DeInit(M8266WIFI_INTERFACE_SPI_TX_DMA_STREAM); 
	DMA_InitStructure.DMA_Channel 							= M8266WIFI_INTERFACE_SPI_TX_DMA_CHANNEL;
	DMA_InitStructure.DMA_PeripheralBaseAddr 	= (uint32_t)(&(M8266WIFI_INTERFACE_SPI->DR));  
	DMA_InitStructure.DMA_Memory0BaseAddr 			= (uint32_t)0;  // use 0 for the moment during initialization
	DMA_InitStructure.DMA_DIR 									= DMA_DIR_MemoryToPeripheral;  
	DMA_InitStructure.DMA_BufferSize 					= 1;            // use 1 for the moment during initialization
	DMA_InitStructure.DMA_PeripheralInc 				= DMA_PeripheralInc_Disable;  
	DMA_InitStructure.DMA_MemoryInc 						= DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryDataSize 			= DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_Mode 								= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority 						= DMA_Priority_Low;  
	DMA_InitStructure.DMA_FIFOMode 						= DMA_FIFOMode_Disable;      
	DMA_InitStructure.DMA_FIFOThreshold 				= DMA_FIFOThreshold_HalfFull;          
	DMA_InitStructure.DMA_MemoryBurst 					= DMA_MemoryBurst_Single;         
	DMA_InitStructure.DMA_PeripheralBurst 			= DMA_PeripheralBurst_Single;   
	DMA_Init(M8266WIFI_INTERFACE_SPI_TX_DMA_STREAM, &DMA_InitStructure);
	//DMA_Cmd(M8266WIFI_INTERFACE_SPI_TX_DMA_STREAM, ENABLE);   // Please DO NOT ENABLE the STREAM during initialization

	//Initialization of SPI RX DMA
	//SPI1_RX uses DMA2 Stream2, channel 3
	RCC_AHB1PeriphClockCmd(M8266WIFI_INTERFACE_SPI_RX_RCC_Periph_DMA, ENABLE);  
	DMA_DeInit(M8266WIFI_INTERFACE_SPI_RX_DMA_STREAM); 
	DMA_InitStructure.DMA_Channel 							= M8266WIFI_INTERFACE_SPI_RX_DMA_CHANNEL;
	DMA_InitStructure.DMA_PeripheralBaseAddr 	= (uint32_t)(&(M8266WIFI_INTERFACE_SPI->DR));  
	DMA_InitStructure.DMA_Memory0BaseAddr 			= (uint32_t)0;  // use 0 for the moment during initialization
	DMA_InitStructure.DMA_DIR 									= DMA_DIR_PeripheralToMemory;  
	DMA_InitStructure.DMA_BufferSize 					= 1;            // use 1 for the moment during initialization
	DMA_InitStructure.DMA_PeripheralInc 				= DMA_PeripheralInc_Disable;  
	DMA_InitStructure.DMA_MemoryInc 						= DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryDataSize 			= DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_Mode 								= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority 						= DMA_Priority_High;  
	DMA_InitStructure.DMA_FIFOMode 						= DMA_FIFOMode_Disable;      
	DMA_InitStructure.DMA_FIFOThreshold 				= DMA_FIFOThreshold_HalfFull;          
	DMA_InitStructure.DMA_MemoryBurst 					= DMA_MemoryBurst_Single;         
	DMA_InitStructure.DMA_PeripheralBurst 			= DMA_PeripheralBurst_Single;   
	DMA_Init(M8266WIFI_INTERFACE_SPI_RX_DMA_STREAM, &DMA_InitStructure);
	//DMA_Cmd(M8266WIFI_INTERFACE_SPI_RX_DMA_STREAM, ENABLE);  // Please DO NOT ENABLE the STREAM during initialization
}
#endif

/***********************************************************************************
 * M8266HostIf_Init                                                                *
 * Description                                                                     *
 *    To initialise the Host interface for M8266WIFI module                        *
 * Parameter(s):                                                                   *
 *    baud: baud rate to set                                                       *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/ 
void M8266HostIf_Init(void)
{
	 M8266HostIf_GPIO_CS_RESET_Init();
	 M8266HostIf_SPI_Init();
#ifdef M8266WIFI_SPI_ACCESS_USE_DMA
	 M8266HostIf_SPI_DMA_Init();
#endif
	printf("[LOG]M8266 HostIf_Init\r\n");	
}

//////////////////////////////////////////////////////////////////////////////////////
// BELOW FUNCTIONS ARE REQUIRED BY M8266WIFIDRV.LIB. 
// PLEASE IMPLEMENTE THEM ACCORDING TO YOUR HARDWARE
//////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************************
 * M8266HostIf_Set_nRESET_Pin                                                      *
 * Description                                                                     *
 *    To Outpout HIGH or LOW onto the GPIO pin for M8266WIFI nRESET                *
 *    You may update the macros of GPIO PIN usages for nRESET from brd_cfg.h       *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    1. level: LEVEL output to nRESET pin                                         *
 *              0 = output LOW  onto nRESET                                        *
 *              1 = output HIGH onto nRESET                                        *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_Set_nRESET_Pin(u8 level)
{
	if(level!=0)
		//GPIO_SetBits(M8266WIFI_nRESET_GPIO, M8266WIFI_nRESET_PIN);
		M8266WIFI_nRESET_GPIO->BSRRL = M8266WIFI_nRESET_PIN;


	else
		//GPIO_ResetBits(M8266WIFI_nRESET_GPIO, M8266WIFI_nRESET_PIN);
		M8266WIFI_nRESET_GPIO->BSRRH = M8266WIFI_nRESET_PIN;
}
/***********************************************************************************
 * M8266HostIf_Set_SPI_nCS_PIN                                                     *
 * Description                                                                     *
 *    To Outpout HIGH or LOW onto the GPIO pin for M8266WIFI SPI nCS               *
 *    You may update the macros of GPIO PIN usages for SPI nCS from brd_cfg.h      *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    1. level: LEVEL output to SPI nCS pin                                        *
 *              0 = output LOW  onto SPI nCS                                       *
 *              1 = output HIGH onto SPI nCS                                       *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_Set_SPI_nCS_Pin(u8 level)
{
	if(level!=0)
		//GPIO_SetBits(M8266WIFI_SPI_nCS_GPIO, M8266WIFI_SPI_nCS_PIN);
		M8266WIFI_SPI_nCS_GPIO->BSRRL = M8266WIFI_SPI_nCS_PIN;
	else
		M8266WIFI_SPI_nCS_GPIO->BSRRH = M8266WIFI_SPI_nCS_PIN;
}

/***********************************************************************************
 * M8266WIFIHostIf_delay_us                                                        *
 * Description                                                                     *
 *    To loop delay some micro seconds.                                            *
 * Parameter(s):                                                                   *
 *    1. nus: the micro seconds to delay                                           *
 * Return:                                                                         *
 *    none                                                                         *
 ***********************************************************************************/
void M8266HostIf_delay_us(u8 nus)
{
	delay_us(nus);
}

/**
 * @description: To loop delay some millisecond.
 * @param {type} 
 * @return {type} 
 */
void M8266WIFI_Module_delay_ms(u16 nms)
{
	 u16 i, j;
	 for(i=0; i<nms; i++)
	   for(j=0; j<4; j++)	// delay 1ms. Call 4 times of delay_us(250), since M8266HostIf_delay_us(u8 nus), nus max 256
	      M8266HostIf_delay_us(250);
}
/***********************************************************************************
 * M8266HostIf_SPI_ReadWriteByte                                                   *
 * Description                                                                     *
 *    To write a byte onto the SPI bus from MCU MOSI to the M8266WIFI module       *
 *    and read back a byte from the SPI bus MISO meanwhile                         *
 *    You may update the macros of SPI usage from brd_cfg.h                        *
 * Parameter(s):                                                                   *
 *    1. TxdByte: the byte to be sent over MOSI                                    *
 * Return:                                                                         *
 *    1. The byte read back from MOSI meanwhile                                    *                                                                         *
 ***********************************************************************************/
u8 M8266HostIf_SPI_ReadWriteByte(u8 TxdByte)
{
	while(SPI_I2S_GetFlagStatus(M8266WIFI_INTERFACE_SPI, SPI_I2S_FLAG_TXE) == RESET){} 	//wait SPI TXD Buffer Empty
	SPI_I2S_SendData(M8266WIFI_INTERFACE_SPI, TxdByte); 								//Write the byte to the TXD buffer and then shift out via MOSI
	while(SPI_I2S_GetFlagStatus(M8266WIFI_INTERFACE_SPI, SPI_I2S_FLAG_RXNE) == RESET){} //wait the SPI RXD buffer has received the data from MISO
	return SPI_I2S_ReceiveData(M8266WIFI_INTERFACE_SPI);  								//return the data received from MISO
}
