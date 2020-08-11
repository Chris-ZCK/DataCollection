/*
 * @Author: Howwell
 * @Date: 2020-08-10 17:34:17
 * @LastEditTime: 2020-08-11 10:03:56
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \USERd:\Documents\GitProject\DataCollection\DProj\HARDWARE\LED\led.h
 */
#ifndef __LED_H
#define __LED_H
#include "sys.h"
// Local use
#define ledGREEN    PEout(2)
#define ledYELLOW   PEout(3)
#define ledBLUE     PEout(4)

// Global use
void LED_Init(void);

void LED_GREEN_ON(void);
void LED_GREEN_OFF(void);
void LED_GREEN_NOT(void);

void LED_YELLOW_ON(void);
void LED_YELLOW_OFF(void);
void LED_YELLOW_NOT(void);

void LED_BLUE_ON(void);
void LED_BLUE_OFF(void);
void LED_BLUE_NOT(void);
#endif
