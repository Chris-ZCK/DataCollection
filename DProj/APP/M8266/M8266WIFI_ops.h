/*
 * @Author: your name
 * @Date: 2020-08-20 22:28:40
 * @LastEditTime: 2020-08-21 02:24:58
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \USERc:\Users\lihao\Documents\Git Projects\DataCollection\DProj\APP\M8266\M8266WIFI_ops.h
 */
/********************************************************************
 * M8266WIFI_ops.h
 * .Description
 *     header file of M8266WIFI HAL operations
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
 
#ifndef _M8266WIFI_OPS_H_ 
#define _M8266WIFI_OPS_H_
#include "sys.h" 

#define WIFI_NO 2
#if WIFI_NO == 1
#define TARGET_SSID			"@PHICOMM_28"
#define TARGET_PASSWORD		"12345678"
#else
#define TARGET_SSID			"wynjd8802"
#define TARGET_PASSWORD		""
#endif


// SETINGS
// Single settings
#define M8266_SET_TX_MAX_POWER  0
#define M8266_SET_OPMODE        1
// Opthional settings
#define M8266_GET_STA_STATUS    1


// SYS CONFIG
#define SEND_DATA_MAX_SIZE 		1024
#define TIMEOUT_IN_S           	10     /*TCP建立链接超时时间，单位：秒 */
#define M8266_SUCCESS 1
#define M8266_ERROR   0


// Set TCP/UDP Connections
#define TEST_CONNECTION_TYPE  	1      /* 0=WIFI module as UDP, 1=WIFI module as TCP Client, 2=WIFI module as TCP Server */
#define TEST_LOCAL_PORT       	0      /* local port=0 then an updated random port will be generated each time. To avoid the rejection by some TCP server due to repeative same ip:port */
#define TEST_REMOTE_IP_ADDR   	"192.168.1.100"
#define TEST_REMOTE_PORT        8000

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/////// BELOW FUNCTIONS ARE QUERYING OR CONFIGURATION VIA SPI
/////// MORE TO BE FOUND IN M8266WIFIDrv.h
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
u8 M8266WIFI_Module_Init_Via_SPI(void);
u8 M8266WIFI_Config_Connection_via_SPI(u8 tcp_udp, u16 local_port, u8* remote_ip, u16 remote_port, u8 link_no);
u8 M8266WIFI_SPI_wait_sta_connecting_to_ap_and_get_ip(char* sta_ip, u8 max_wait_time_in_s);
u8 M8266WIFI_Sleep_module_for_some_time_and_then_wakeup_automatically(u16 time_to_wakeup_in_ms);
u8 M8266WIFI_Sleep_Module(void);
u8 M8266WIFI_Wakeup_Module(void);

u8 M8266TransportOpen(void);
u8 M8266TransportCLose(void);
u16 WiFiSendPacketBuffer(u8* buf, u16 buflen);
#endif
