/*
 * @Author: your name
 * @Date: 2020-08-20 22:28:40
 * @LastEditTime: 2020-08-20 23:57:02
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \USERc:\Users\lihao\Documents\Git Projects\DataCollection\DProj\APP\M8266\M8266HostIf.h
 */
/********************************************************************
 * M8266HostIf.h
 * .Description
 *     header file of M8266WIFI Host Interface 
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
#ifndef _M8266_HOST_IF_H_
#define _M8266_HOST_IF_H_
 
void M8266HostIf_Init(void);
void M8266HostIf_SPI_SetSpeed(u8 SPI_BaudRatePrescaler);
#endif

