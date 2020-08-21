#ifndef _BRG_CFG_H_
#define _BRG_CFG_H_

#define MCU_IS_STM32F4XX
#define M8266WIFI_CONFIG_VIA_SPI
//#define M8266WIFI_SPI_ACCESS_USE_DMA

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// M8266WIFI Module Interface Board Usages Macros
//      -- nRESET, nCS
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// nRESET
#define  M8266WIFI_nRESET_GPIO_RCC_CLOCK_EN				    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define  M8266WIFI_nRESET_GPIO								GPIOA
#define  M8266WIFI_nRESET_PIN								GPIO_Pin_8

/// nCS
#define  M8266WIFI_SPI_nCS_GPIO_RCC_CLOCK_EN				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define  M8266WIFI_SPI_nCS_GPIO								GPIOA
#define  M8266WIFI_SPI_nCS_PIN								GPIO_Pin_4


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// M8266WIFI Module Interface Board Usages Macros
//     - MOSI - MISO - CLK
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      PIN   GND - nRST - nCS - MOSI - MISO - CLK   
////For STM32
//- If uses SPI1, SPI1_MOSI=GPIOA7,  SPI1_MISO=GPIOA6,  SPI1_SCK=GPIOA5   -- use this currently
//            or  SPI1_MOSI=GPIOB5,  SPI1_MISO=GPIOB4,  SPI1_SCK=GPIOB3                 
//- If uses SPI2, SPI2_MOSI=GPIOB15, SPI3_MISO=GPIOB14, SPI3_SCK=GPIOB13  -- use this currently
//            or  SPI2_MOSI=GPIOC3,  SPI1_MISO=GPIOC2,  SPI1_SCK=GPIOB10,                  
//- If uses SPI3, SPI3_MOSI=GPIOC12, SPI3_MISO=GPIOC11, SPI3_SCK=GPIOC10  -- use this currently
//            or  SPI3_MOSI=GPIOB5,  SPI3_MISO=GPIOB4,  SPI3_SCK=GPIOB3,                  
#define  M8266WIFI_SPI_INTERFACE_NO							1   // for STM32F4xx, try to use SPI1 other than SPI2 and SPI3, since according to datasheet, SPI1 is much better than SPI2 and SPI3

//- If uses USART1, USART1_TXD=PA9, USART1_RXD=PA10
//- If uses USART2, USART2_TXD=PA2, USART2_RXD=PA3   
#define  M8266WIFI_USART_INTERFACE_NO						1 // 1 for USART1, 2 for USART2
#define  M8266WIFI_USART_RX_BUF_SIZE						512

#define  USART_FOR_PRINTF												USART1

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// M8266WIFI Module Interface Board Usages Macros
//     - SPI TX DMA   if DMA used 
//     - SPI RX DMA   if DMA used
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
#ifdef M8266WIFI_SPI_ACCESS_USE_DMA
	#define M8266WIFI_INTERFACE_SPI_TX_DMA										DMA2
	#define M8266WIFI_INTERFACE_SPI_TX_RCC_Periph_DMA 				RCC_AHB1Periph_DMA2
	#define M8266WIFI_INTERFACE_SPI_TX_DMA_CHANNEL 						DMA_Channel_3
	#define M8266WIFI_INTERFACE_SPI_TX_DMA_STREAM							DMA2_Stream3
	#define M8266WIFI_INTERFACE_SPI_RX_DMA										DMA2
	#define M8266WIFI_INTERFACE_SPI_RX_RCC_Periph_DMA 				RCC_AHB1Periph_DMA2
	#define M8266WIFI_INTERFACE_SPI_RX_DMA_CHANNEL 						DMA_Channel_3
	#define M8266WIFI_INTERFACE_SPI_RX_DMA_STREAM							DMA2_Stream2
#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// PLEASE DO NOT MODIFY BELOW UNLESS NECESSITY
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#define MCU_IS_STM32
#define GPIO_InitStructure_AS_GPIO_OUTPUT(gpio,pin)						\
GPIO_InitStructure.GPIO_Pin 	= pin;												\
GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;							\
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							\
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					\
GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;								\
GPIO_Init(gpio, &GPIO_InitStructure)	

#define GPIO_OUTPUT_HIGH(gpio,pin)	(gpio)->BSRRL=pin
#define GPIO_OUTPUT_LOW(gpio,pin)		(gpio)->BSRRH=pin

#define GPIO_InitStructure_AS_AF(gpio,pins)										\
GPIO_InitStructure.GPIO_Pin 	= pins;												\
GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;								\
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							\
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					\
GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;								\
GPIO_Init(gpio, &GPIO_InitStructure)	

//To define GPIO_PinRemapSPI1
#define GPIO_PinRemapSPI1																																	\
GPIO_PinAFConfig(M8266WIFI_SPI_GPIO,M8266WIFI_SPI_MOSI_GPIO_PinSource,GPIO_AF_SPI1);		\
GPIO_PinAFConfig(M8266WIFI_SPI_GPIO,M8266WIFI_SPI_MISO_GPIO_PinSource,GPIO_AF_SPI1); 		\
GPIO_PinAFConfig(M8266WIFI_SPI_GPIO,M8266WIFI_SPI_CLK_GPIO_PinSource,GPIO_AF_SPI1); 


//To define GPIO_PinRemapSPI2
#define GPIO_PinRemapSPI2																																	\
GPIO_PinAFConfig(M8266WIFI_SPI_GPIO,M8266WIFI_SPI_MOSI_GPIO_PinSource,GPIO_AF_SPI2);		\
GPIO_PinAFConfig(M8266WIFI_SPI_GPIO,M8266WIFI_SPI_MISO_GPIO_PinSource,GPIO_AF_SPI2); 		\
GPIO_PinAFConfig(M8266WIFI_SPI_GPIO,M8266WIFI_SPI_CLK_GPIO_PinSource,GPIO_AF_SPI2); 


//To define GPIO_PinRemapSPI3
#define GPIO_PinRemapSPI3																				\
GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_SPI3);					\
GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_SPI3); 				\
GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_SPI3);


#if   defined(MCU_IS_STM32)  // for STM32
	#if   (M8266WIFI_SPI_INTERFACE_NO == 1)
	  	#define  M8266WIFI_INTERFACE_SPI								SPI1
	#elif (M8266WIFI_SPI_INTERFACE_NO == 2)
	 	 #define  M8266WIFI_INTERFACE_SPI								SPI2
	#elif (M8266WIFI_SPI_INTERFACE_NO == 3)
	 	 #define  M8266WIFI_INTERFACE_SPI								SPI3
	#elif (M8266WIFI_SPI_INTERFACE_NO == 4)
	  	#define  M8266WIFI_INTERFACE_SPI								SPI4
	#else
	#error Wrong M8266WIFI_SPI_INTERFACE_NO defined near line 104 for STM32 in brd_cfg.h
	#endif
#else
	#error YOU SHOULD DEFINED MCU_IS_XXX near line 17 in brd_cfg.h
#endif

#endif/*brd_cfg*/
