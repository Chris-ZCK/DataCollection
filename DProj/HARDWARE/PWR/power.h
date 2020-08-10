#ifndef __POWER_H
#define __POWER_H
#include "sys.h"

//PC4 Relay read
//#define Relay_IO_IN()  {GPIOC->MODER&=~(3<<(4*2));GPIOB->MODER|=0<<4*2;}	//PC4输入模式
#define Relay_IO_IN 	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)     //PC5
// Power control Interface
#define POWER_MP 		PEout(9)   // MP1584, active=1,default=0
#define POWER_MIC       PEout(10)  // MIC30032,active=1,default=0
//------------------------------------

#define POWER_OUT 		PEout(11)  // 5V out, active=1,default=1
#define POWER_CAM 		PEout(12)  // 5V DC in, active=1,default=1

#define POWER_USB 		PEout(13)  // 5V USB, active=1,default=0

#define POWER_OUT3V3 	PEout(14)  // 3.3V out, active=1,default=0
#define POWER_OUT5 		PEout(15)  // 5V out, active=1,default=0

#define POWER_INIO1 	PCout(4)  // OUTIO1, active=1,default=0 
#define POWER_OUTIO1 	PEout(7)  // INIO1, active=1,default=0
// Camera control Interface
#define USB_CNT PEout(5) // 8TX-????3???USB
#define USB_PHO PEout(6) // 7GND????5?????

// Power 
void Power_Ctrl_Init(void); //初始化

// Camera
void Cam_Crtl_Init(void);

void USB_Connecting(u8 ctrl);
void USB_Photograph(void);

void USB_CONNECT_ON(void);
void USB_CONNECT_OFF(void);
void openUSB(void);
void closeUSB(void);

void openReLoad(void);
void closeReLoad(void);

void Relay_Init(void);
u8 openOutputLoad(void);
u8 closeOutputload(void);
#endif
