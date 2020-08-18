/*
 * @Author: your name
 * @Date: 2020-08-09 10:58:08
 * @LastEditTime: 2020-08-09 14:21:18
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Aproj\HARDWARE\PWR\power.h
 */
#ifndef __POWER_H
#define __POWER_H
#include "sys.h"

// Power control Interface
#define POWER_D 		PEout(9)   // 4G, active=1,default=0
//------------------------------------
#define POWER_LE			PEout(10)  // Output,active=1,default=0
#define POWER_OUT 		PEout(11)  // !5V out, active=1,default=1
#define POWER_CAM 		PEout(12)  // !5V DC in, active=1,default=1

#define POWER_USB 		PEout(13)  // 5V USB, active=1,default=0

#define POWER_OUT3V3 	PEout(14)  // 3.3V out, active=1,default=0
#define POWER_OUT5 		PEout(15)  // 5V out, active=1,default=0

// Camera control Interface
#define USB_CNT PEout(5) 
#define USB_TP  PEout(6) 

// Power 
void Power_Ctrl_Init(void); 
// Camera
void Cam_Crtl_Init(void);

void USB_Connecting(u8 ctrl);
void USB_Photograph(void);

void USB_CONNECT_ON(void);
void USB_CONNECT_OFF(void);
void openUSB(void);
void closeUSB(void);

#endif
