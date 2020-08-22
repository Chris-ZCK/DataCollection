/********************************************************************
 * M8266WIFI_ops.c
 * .Description
 *     source file of M8266WIFI HAL operations
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
#include "stdio.h"
#include "string.h"
#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"
#include "M8266WIFI_ops.h"
#include "iwdg.h"

unsigned char g_link_no;  // socket 端口号全局变量|socket port global variable

////////////////////////////////////////////////////////////////////////////////////
// Functions as examples of M8266WIFI Module Operations
//    -- Fundamentals
////////////////////////////////////////////////////////////////////////////////////

/***********************************************************************************
 * M8266WIFI_Module_Hardware_Reset                                                 *
 * Description                                                                     *
 *    1. To perform a hardware reset to M8266WIFI module via the nReset Pin        *
 *       and bring M8266WIFI module to boot up from external SPI flash             *
 *    2. In order to make sure the M8266WIFI module bootup from external           *
 *       SPI flash, nCS should be low during Reset out via nRESET pin              *
 * Parameter(s):                                                                   *
 *    none                                                                         *
 * Return:                                                                         *
 *    none                                                                         *
 ***********************************************************************************/
void M8266WIFI_Module_Hardware_Reset(void) // total 800ms
{
	M8266HostIf_Set_SPI_nCS_Pin(0); // Module nCS==ESP8266 GPIO15 as well, should be low during reset in order for a normal reset
	M8266WIFI_Module_delay_ms(1);	// delay 1ms, adequate for nCS stable

	M8266HostIf_Set_nRESET_Pin(0); // Pull low the nReset Pin to bring the module into reset state
	M8266WIFI_Module_delay_ms(5);  // delay 1ms, adequate for nRESET stable.
								   // give more time especially for some board not good enough

	M8266HostIf_Set_nRESET_Pin(1);	// Pull high again the nReset Pin to bring the module exiting reset state
	M8266WIFI_Module_delay_ms(100); // at least 18ms required for reset-out-boot sampling boottrap pin
									// Here, we use 50ms for adequate abundance.
									// However, for some board GPIO needs more time for stable(especially for nRESET),
									// give more time here
	M8266HostIf_Set_SPI_nCS_Pin(1);
	//M8266WIFI_Module_delay_ms(1); 	    // delay 1ms, adequate for nCS stable

	M8266WIFI_Module_delay_ms(750 - 2); // Delay more than 500ms for M8266WIFI module bootup and initialization including bootup information print.
										// Wait time should be no less than 0.5s.
										// Here, we use wait 750ms
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/////// BELOW FUNCTIONS ARE QUERYING OR CONFIGURATION VIA SPI
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************************
 * M8266WIFI_SPI_wait_sta_connecting_to_ap_and_get_ip                              *
 * Description                                                                     *
 *    Wait M8266WIFI in STA mode connecting to AP/Router and get ip address        *
 *    via SPI API functions in M8266WIFIDrv.lib                                    *
 * Parameter(s):                                                                   *
 *    1. sta_ip: if success, sta_ip return the ip address achieved                 *
 *    2. max_wait_time_in_s: max timeout to wait in seconds                        *
 * Return:                                                                         *
 *       1 = M8266WIFI module connects AP/router and gets a valid IP               *
 *           from AP/router before the timeout succefully                          *
 *       0 = M8266WIFI module fails to connect the AP/router and get               *
 *           a valid IP before the timeout                                         *
 ***********************************************************************************/
u8 M8266WIFI_SPI_wait_sta_connecting_to_ap_and_get_ip(char *sta_ip, u8 max_wait_time_in_s)
{
	u16 i;
	u16 status = 0;

	for (i = 0; i < 1 * max_wait_time_in_s; i++) // max wait
	{
		if ((M8266WIFI_SPI_Get_STA_IP_Addr(sta_ip, &status) == 1) && (strcmp(sta_ip, "0.0.0.0") != 0))
			break;

		M8266WIFI_Module_delay_ms(1000);
		continue;
	}
	if (i >= 1 * max_wait_time_in_s)
		return 0; // false

	return 1; // true
}
/***********************************************************************************
 * M8266WIFI_Module_Init_Via_SPI(void)                                             *
 * Description                                                                     *
 *    To perform a Initialization sequency to M8266WIFI module via SPI I/F         *
 *    (1) Reboot the M8266WIFI module via nRESET pin                               *
 *    (2) Wait and Check if the M8266WIFI module gets an valid IP address          *
 *        if the module in STA or STA+AP mode                                      *
 *    Call this function after Host SPI Interface Initialised                      *
 *    if use SPI interface to config module                                        *
 * Parameter(s):                                                                   *
 *    none                                                                         *
 * Return:                                                                         *
 * Return:                                                                         *
 *       0 = failed                                                                *
 *       1 = success                                                               * 
 ***********************************************************************************/
 u8 M8266WIFI_Module_Init_Via_SPI(void)
 {
	u32  spi_clk = 40000000;
	u8   sta_ap_mode = 0;
	u8   connection_status = 0xFF;
	char sta_ip[15+1]={0};
	char ssid[32];
	s8   rssi;
	u16  status = 0;
	u8	 byte;
	
    //To hardware reset the module (with nCS=0 during reset) and wait up the module bootup
	M8266WIFI_Module_Hardware_Reset();
	
	//Try SPI clock in a fast one as possible up to 40MHz (M8266WIFI could support only upto 40MHz SPI) 
	/*
	Notes: 
	For STM32, SPI Clock = APB2_CLOCK/SPI_BaudRatePrescaler, or something similar
	For STM32F4, please see system_stm32f4xx.c, APB2_CLOCK = PCLK2 = HCLK/2 = SYSCLK/2
	Meanwhile, the PCLK2 should <= 90MHz, and SPI_CLK should <= 45MHz
	- STM32F401xx:     SYSCLK          = 168MHz, so, APB2_CLOCK = 84MHz
	- STM32F40_41xxx:  SYSCLK          = 180MHz, so, APB2_CLOCK = 90MHz
	- STM32F427_437xx, STM32F429_439xx = 180MHz, so, APB2_CLOCK = 90MHz
	*/
	#if defined(MCU_IS_STM32)
	#ifndef SPI_BaudRatePrescaler_2
	#define SPI_BaudRatePrescaler_2 		((uint32_t)0x00000000U)
	#define SPI_BaudRatePrescaler_4         ((uint32_t)0x00000008U)
	#define SPI_BaudRatePrescaler_8         ((uint32_t)0x00000010U)
	#define SPI_BaudRatePrescaler_16        ((uint32_t)0x00000018U)
	#define SPI_BaudRatePrescaler_32        ((uint32_t)0x00000020U)
	#define SPI_BaudRatePrescaler_64        ((uint32_t)0x00000028U)
	#define SPI_BaudRatePrescaler_128       ((uint32_t)0x00000030U)
	#define SPI_BaudRatePrescaler_256       ((uint32_t)0x00000038U)
	#endif
	#endif
	M8266HostIf_SPI_SetSpeed(SPI_BaudRatePrescaler_4); // Setup SPI Clock. Here 84/8 = 10.5MHz for STM32F4xx, up to 40MHz (SPI2 & 3 upto 21MHz)
	spi_clk = 10000;

	// wait clock stable
	M8266WIFI_Module_delay_ms(1);
	//It is very important to call M8266HostIf_SPI_Select() to tell the driver which SPI you used
	//and how faster the SPI clock you used. The function must be called before SPI access
	if(M8266HostIf_SPI_Select((uint32_t)M8266WIFI_INTERFACE_SPI, spi_clk, &status)==0)
	{
		printf("[WARNING]Failed to CALL M8266HostIf_SPI_Select\r\n");
		return 0;
	}
	else
		printf("[INFO]<S1/8>M8266HostIf_SPI_Select SPI1 ,spi_clk = %d\r\n",spi_clk);
	 
	#ifdef M8266WIFI_SPI_ACCESS_USE_DMA
   //void M8266HostIf_SPI_DMA_Select(uint32_t spi_tx_dam_base_addr, uint32_t spi_tx_dam_stream_base_addr, uint32_t spi_rx_dam_base_addr, uint32_t spi_rx_dam_stream_base_addr);
	 M8266HostIf_SPI_DMA_Select((uint32_t)M8266WIFI_INTERFACE_SPI_TX_DMA, 
								(uint32_t)M8266WIFI_INTERFACE_SPI_TX_DMA_STREAM, 
								(uint32_t)M8266WIFI_INTERFACE_SPI_RX_DMA, 
								(uint32_t)M8266WIFI_INTERFACE_SPI_RX_DMA_STREAM
								);
	#endif
	
	if(M8266WIFI_SPI_Interface_Communication_OK(&byte)==0) 	  									//	if SPI logical Communication failed
	{
		printf("[WARNING]Failed to CHECK M8266WIFI_SPI_Interface_Communication_OK\r\n");
		return 0;
	}
	else
		printf("[INFO]<S2/8>M8266WIFI_SPI_Interface_Communication_OK\r\n");	 
 
	#if M8266_SET_TX_MAX_POWER
	// If you hope to reduce the Max Tx power, you could enable it by change to "#if 1"
	//u8 M8266WIFI_SPI_Set_Tx_Max_Power(u8 tx_max_power, u16 *status)
	if(M8266WIFI_SPI_Set_Tx_Max_Power(82, &status)==0)   // tx_max_power=68 to set the max tx power of aroud half of manufacture default, i.e. 50mW or 17dBm
	{
		printf("[WARNING]<*S3/8>Fail to SET M8266WIFI_SPI_Set_Tx_Max_Power  =  %d\r\n",82);
		return 0;
	}
	else
		printf("[INFO]<*S3/8>M8266WIFI_SPI_Set_Tx_Max_Power=%d\r\n",82);
	#endif	

	//设置为STA模式，设置一次即可
	#if M8266_SET_OPMODE
	// If you hope to change the op_mode overrding the default loaded from flash, you could enable it by change to "#if 1"
	// BUT you are strongly recommended not to use "saved" if you call thie function each time upon bootup
	// Since it will write flash each time during bootup and the falsh might go to failure with a limited write life cycle of around tens of thousands times	
	//u8 M8266WIFI_SPI_Set_Opmode(u8 op_mode, u8 saved, u16* status);
	if(M8266WIFI_SPI_Set_Opmode(1, 0, &status)==0)  // set to STA+AP mode, not saved // 1=STA Only, 2=AP Only, 3=STA+AP
    {
    	printf("[WARNING]Failed to SET M8266WIFI_SPI_Set_Opmode");
    	return 0;
    }
	else
		printf("[INFO]<S3/8>M8266WIFI_SPI_Set_Opmode = STA Only\r\n");
	#endif

	#if 0 //当WIFI工作在AP模式，设置AP的属性,这里不需要打开
	// If you hope to change the ap info overrding the default loaded from flash, you could enable it by change to "#if 1"
	// BUT you are strongly recommended not to use "saved" if you call thie function each time upon bootup
	// Since it will write flash each time during bootup and the falsh might go to failure with a limited write life cycle of around tens of thousands times	
  	// u8 M8266WIFI_SPI_Config_AP(u8 ssid[13+1], u8 password[13+1], u8 enc, u8 channel, u8 saved, u16* status);	
	if(M8266WIFI_SPI_Config_AP("Anylinkin", "1234567890", 4, 6, 0, &status)==0)  // set to 4=WPA_WPA2_PSK, not saved // 0=OPEN, 1=WEP, 2=WPA_PSK, 3=WPA2_PSK, 4=WPA_WPA2_PSK
		return 0;
	#endif
	
  	// get current op_mode, if STA mode or STA+AP mode, then, wait connection to external AP and getting ip
	//u8 M8266WIFI_SPI_Get_Opmode(u8* op_mode, u16* status);	
	if(M8266WIFI_SPI_Get_Opmode(&sta_ap_mode, &status)==0)  
	{
		printf("[WARNING]Failed to GET M8266WIFI_SPI_Get_Opmode\r\n");
		return 0;	
	}
	else
	{
		printf("[INFO]<S4/8>M8266WIFI_SPI_Get_Opmode STA =%d\r\n",sta_ap_mode);
	}
		
	if(  (sta_ap_mode == 1)   // if STA mode
	   ||(sta_ap_mode == 3))  // if STA+AP mode
	{
		//配置WIFI和密码，一次即可
		#if 1 
		// If you hope to use the SSID and password here overriding the default loaded from on-module SPI flash, you could enable it by change to "#if 1"
		// BUT you are strongly recommended not to use "saved" if you call thie function each time upon bootup
		// Since it will write flash each time during bootup and the falsh might go to failure with a limited write life cycle of around tens of thousands times
		// Sure you could call it with "saved" once in your debug and change back to "not saved" to have a way, 
		// alternative to AT or Airkiss/WPS, to update the default in spi flash
			
		IWDG_Feed();
		//u8 M8266WIFI_SPI_STA_Connect_Ap(u8 ssid[13+1], u8 password[13+1], u8 saved, u8 timeout_in_s, u16* status);
		//if(M8266WIFI_SPI_STA_Connect_Ap("TP-LINK_AD", "12345678", 0, 20, &status) == 0) // not saved, timeout=20s
		if(M8266WIFI_SPI_STA_Connect_Ap((u8*)TARGET_SSID, (u8*)TARGET_PASSWORD, 0, 20, &status) == 0) // not saved, timeout=20s
		{
			printf("[WARNING]Failed to CONNECT to AP: %s[ssid]\r\n",TARGET_SSID);	
			return 0;
		}
		else
		{
			printf("[INFO]<S5/8>Connect to WIFI:[ssid]=%s, [password]=%s, status = 0x%04X\r\n",TARGET_SSID, TARGET_PASSWORD, status);	
		}	 			 
		#endif
		IWDG_Feed();
		//u8 M8266WIFI_SPI_wait_sta_connecting_to_ap_and_get_ip(char* sta_ip, u8 max_wait_time_in_s)
		if(M8266WIFI_SPI_wait_sta_connecting_to_ap_and_get_ip(sta_ip, 10)==0) // max wait 10s to get sta ip
		{
			printf("[WARNING]Fail M8266WIFI module GET ip address!\r\n");
			return 0; 
		}
		else
		{
			printf("[INFO]<S6/8>STA_connecting_to_ap_and_get_ip=%s\r\n",sta_ip);
		}
		
		#if 0
		//u8 SPI_Set_Web_Server(u8 open_not_shutdown, u16 server_port, u16* status)
		if(SPI_Set_Web_Server(0, 3128, &status)==0)  // to shutdown the web_server
		{
			return 0; // printf("M8266WIFI module failed to get sta ip address!\n");
		}
		//u8 SPI_Set_Web_Server(u8 open_not_shutdown, u16 server_port, u16* status)
		if(SPI_Set_Web_Server(1, 3128, &status)==0)  // to (re-)start the web_server with port = 3128
		{
			return 0; // printf("M8266WIFI module failed to get sta ip address!\n");
		}
		#endif
			 
		#if M8266_GET_STA_STATUS
		// these below two functions are just for examples on how to use them. You may or may not use them during module initialisation		 
		//u8 M8266WIFI_SPI_Get_STA_Connection_Status(u8* connection_status, u16* status)
		if(M8266WIFI_SPI_Get_STA_Connection_Status(&connection_status, &status)==0)
		{
			printf("[WARNING]Failed to GET Get_STA_Connection_Status\r\n");
			return 0;
		}
		else
		{
			printf("[INFO]<S7/8>");
			switch(connection_status)
			{
				case 0: printf("station is in idle\r\n"); break; 
				case 1: printf("station is connecting to an AP\r\n"); break; 
				case 2: printf("station is to connect to an AP but with an wrong password\r\n");break; 
				case 3: printf("station is to connect to an AP which is not found\r\n"); break; 
				case 4: printf("station failed to connect to the AP\r\n");   break; 
				case 5: printf("station is connected to the AP and has got the IP successfully\r\n"); break; 
				case 255: printf("the module is in AP-only mode\r\n"); break; 
			}
		}
		//u8 M8266WIFI_SPI_STA_Query_Current_SSID_And_RSSI(char* ssid, u8* rssi, u16* status)
		if(M8266WIFI_SPI_STA_Query_Current_SSID_And_RSSI(ssid, &rssi, &status)==0)
		{
			printf("[WARNING]Failed to M8266WIFI_SPI_STA_Query_Current_SSID_And_RSSI\r\n");
			return 0;
		}
		else
		{
			printf("[INFO]<S8/8>Current_SSID_And_RSSI: ssid=%s, rssi=%d\r\n",ssid,rssi);	
		}
		#endif
			 
	}
	printf("[LOG]Successfull connect to WIFI & get IP address.\r\n");
  	return 1;
 }


/*************************************************************************************
 * M8266WIFI_Config_Connection_Via_SPI                                               *
 * Description                                                                       *
 *    To reboot the module, wait wifi connection, and setup udp/tcp connection       *
 *    whether the response is what is expected via SPI                               *
 * Parameter(s):                                                                     *
 *    1. tcp_udp     : the type of connection to set up                              *
 *                     = 0, to set up connection of UDP                              *
 *                     = 1, to set up connection as TCP Client                       *
 *                     = 2, to set up connection as TCP Server                       *
 *    2. local_port  : to specify the local port if any                              *
 *                     if local_port=0, then a random local port will be generated   *
 *    3. remote_ip   : the string of ip address of remote server                     *
 *                     e.g. "192.168.1.2"                                            *
 *    4. remote_port : the port value of remote server                               *
 *    5. link_no     : the link_number to be used. max 8 links as per mutitiple link *
 * Return:                                                                           *
 *       0 = failed                                                                  *
 *       1 = success                                                                 * 
 *************************************************************************************/
u8 M8266WIFI_Config_Connection_via_SPI(u8 tcp_udp, u16 local_port, u8* remote_ip, u16 remote_port, u8 link_no)
{
	u16 status=0;
	// Setup the TCP/UPD link
	// u8 M8266WIFI_SPI_Setup_Connection(u8 tcp_udp, u16 local_port, u8* remote_ip, u16 remote_port, u8 link_no, u8 timeout_in_s, u16* status);
	return M8266WIFI_SPI_Setup_Connection(tcp_udp, local_port, (char *)remote_ip, remote_port, link_no, TIMEOUT_IN_S, &status);
}

/*************************************************************************************
 * M8266WIFI_Sleep_module_for_some_time_and_then_wakeup_automatically                *
 * Description                                                                       *
 *    - To (deep) sleep the module for some time via SPI                             *
 *    - All connections will get lost when entering sleep and after wakeup           *
 *      And should be re-established by the host                                     *
 *      e.g.                                                                         *
 *          	if(M8266WIFI_Sleep_M8266WIFI(5000)==0) // wakeup in 5000ms             *
 *		              return fail; // do somework here for failure                     *
 *            M8266WIFI_Module_Init_Via_SPI();       // re-initial the module        *
 *         or M8266WIFI_Config_Connection_via_SPI(); // re-establish the server      *
 * Parameter(s):                                                                     *
  *     1. time_to_wakeup_in_ms : time to wakeup from sleep, unit in ms              *
 *        - max value 4294967 ms (about 1.19hour).                                   *
 *          if a value exceed this provided, then 4294967ms will be used             *
 *        - use 0 if expect to sleep for ever without automatic wakeup               *
* Return:                                                                            *
 *       0 = failed                                                                  *
 *       1 = success                                                                 * 
 *************************************************************************************/
u8 M8266WIFI_Sleep_module_for_some_time_and_then_wakeup_automatically(u16 time_to_wakeup_in_ms)
{
	u16 status;
	//u8 M8266WIFI_SPI_Sleep_Module(u8 sleep_type, u32 time_to_wakeup_in_ms, u16 *status)
	if (M8266WIFI_SPI_Sleep_Module(3, time_to_wakeup_in_ms, &status) == 0) //sleep_type=3=deep_sleep
		return 0;

	if (time_to_wakeup_in_ms > 0) // not a forever sleep
	{
		M8266HostIf_Set_SPI_nCS_Pin(0);							// set nCS low to enaure the module will reboot from flash after wakeup
		M8266WIFI_Module_delay_ms(time_to_wakeup_in_ms + 1000); // additional 1000ms for bootup time
		M8266HostIf_Set_SPI_nCS_Pin(1);
	}

	return 1;
}


/*************************************************************************************
 * M8266WIFI_Sleep_Module                                                            *
 * Description                                                                       *
 *    To (deep) sleep the module forever via SPI and should be wakeup by host        *
 *    After calling of this function, all connection will get lost                   *
 * Parameter(s):                                                                     *
 *    None                                                                           *
* Return:                                                                            *
 *       0 = failed                                                                  *
 *       1 = success                                                                 * 
 *************************************************************************************/
u8 M8266WIFI_Sleep_Module(void)
{
	u16 status;
	//u8 M8266WIFI_SPI_Sleep_Module(u8 sleep_type, u32 time_to_wakeup_in_ms, u16 *status)
	if (M8266WIFI_SPI_Sleep_Module(3, 0, &status) == 0) //sleep_type=3=deep_sleep
		return 0;

	return 1;
}


/*************************************************************************************
 * M8266WIFI_Wakeup_Module                                                           *
 * Description                                                                       *
 *    To wakeup the module                                                           *
 *    - All connections will get lost when entering sleep and after wakeup           *
 *      And should be re-established by the host after wakeup                        *
 *      e.g.                                                                         *
 *          	if(M8266WIFI_Wakeup_Module()==0)                                       *
 *		        {      ; // do somework here for failure                               *
 *            }                                                                      *
 *            else                                                                   *
 *            {                                                                      *
 *               M8266WIFI_Config_Connection_via_SPI(); // re-establish the server   *
 *            }                                                                      *
 *                                                                                   *
 * Parameter(s):                                                                     *
 *    None                                                                           *
* Return:                                                                            *
 *       0 = failed                                                                  *
 *       1 = success                                                                 * 
 *************************************************************************************/
u8 M8266WIFI_Wakeup_Module(void)
{
	 return M8266WIFI_Module_Init_Via_SPI();
}


/**
 * @description: 
 * 	打开tcp连接端口，可以自动的尝试连接端口
 * 	Open the TCP connection port, the fun can automatic try doing it.
 * @param {type} 
 * @return {type} 
 */
u8 M8266TransportOpen(void)
{
	u8 ret;
	u8 cnt=M8266_ERROR;
	
	//新建一个Socket并绑定本地端口5000,连接TCP服务器
	#define Max_TRY_TIMES 10
	M8266WIFI_Module_delay_ms(1000);
	while(cnt<Max_TRY_TIMES)
	{
		ret = M8266WIFI_Config_Connection_via_SPI(TEST_CONNECTION_TYPE, TEST_LOCAL_PORT, (u8*)TEST_REMOTE_IP_ADDR, TEST_REMOTE_PORT, g_link_no);
		if(ret == M8266_SUCCESS)
		{
			printf("[INFO]Transport_open Successful socket=%d\r\n",g_link_no);
			return M8266_SUCCESS;
		}
		else
		{	
			printf("[WARNING]Try transport_open Failed <no | cnt : %d/%d>\r\n",g_link_no,cnt);
			g_link_no++;
			
			if(g_link_no>=4)
			{				
				g_link_no=0;
				cnt++;
			}
		}
		M8266WIFI_Module_delay_ms(1000);	
	}
	printf("[WARNING]M8266 transport open socket=%d failed\r\n",g_link_no);
	return M8266_ERROR;
}


/**
 * @description: Close the current link.
 * @param {type} 
 * @return {type} 
 */
u8 M8266TransportCLose(void)
{
	u16 ret;
	u16 status=0;	//用于回传状态
	
	ret = M8266WIFI_SPI_Delete_Connection(g_link_no, &status);
	if(ret == M8266_SUCCESS)
	{		
		printf("[INFO]M8266 succeed to close socket=%d,status=%4x\r\n", g_link_no, status);
		return M8266_SUCCESS;
	}
	else
	{
		printf("[WARNING]M8266 faile to close socket=%d, ,status=%4x\r\n",g_link_no, status);
		return M8266_ERROR;
	}
}


/**
  * @brief  通过TCP方式发送数据到TCP服务器
  * @param  buf 数据首地址
  * @param  buflen 数据长度
  * @retval 返回值为已发送的数据
*/

u16 WiFiSendPacketBuffer(u8* buf, u16 buflen)
{
	// u8 stateValeu=0;
	u16 status	= 0;	 //用于回传状态 

    int remainder_item;  // 剩余的字节数量
	int sent = 0;
	int total_sent = 0;

	// PrintProgressBarInit();
	// IWDG_Feed();
	remainder_item = buflen;
	while(remainder_item >= SEND_DATA_MAX_SIZE)
	{
		sent = M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE, g_link_no, &status);
		total_sent += sent;  // 更新待发送的位置

		if( (sent!=SEND_DATA_MAX_SIZE) || ( (status&0xFF)!= 0 ) )  // 尽量保证按1024字节发送数据
		{
			if( (status&0xFF)==0x12 )  // 0x12 = Module send buffer full, and normally sent return with 0, i.e. this packet has not been sent anyway
			{       
				// printf("Module send buffer full!\r\n");                       
				// M8266HostIf_delay_us(250);    // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				M8266WIFI_Module_delay_ms(10);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
				// if(sent<SEND_DATA_MAX_SIZE)
				// total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend it					
			}
			else if(((status&0xFF)==0x14)  	// 0x14 = connection of link_no not present
			||((status&0xFF)==0x15) )		// 0x15 = connection of link_no closed
			{
				// do some work here, including re-establish the closed connect or link here and re-send the packet if any
				// additional work here
				printf("[WARNING]Connection of socket=%d not present OR connection of socket=%d closed!\r\n",g_link_no,g_link_no);
				goto exit;	
			}
			else if	((status&0xFF)==0x18)  //  0x18 = TCP server in listening states and no tcp clients connecting to so far
			{
				printf("[WARNING]TCP server in listening states and no tcp clients connecting to so far!\r\n");
				M8266HostIf_delay_us(250);	
				goto exit;
			}				
			else  // 0x10, 0x11 here may due to spi failure during sending, and normally packet has been sent partially, i.e. sent!=0
			{
				// do some work here
				printf("[WARNING]There are some Unknown errors! status:%4x\r\n",status);
				// M8266HostIf_delay_us(250);       // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				// M8266WIFI_Module_delay_ms(25);	   // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
				// if(sent<SEND_DATA_MAX_SIZE)
				// 	total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend the left packet
				goto exit;
			}
			
		}
		remainder_item = buflen - total_sent;
		
		// printf("remainder_item:%d\r\n",remainder_item);
		// PrintProgressBar(total_sent, buflen);
		M8266WIFI_Module_delay_ms(20);
	}
	
	// Send the remaining items
	while(total_sent != buflen)//当小于1024的时候回调出
	{
		sent = M8266WIFI_SPI_Send_Data(buf+total_sent, remainder_item, g_link_no, &status);
		total_sent += sent;	//更新待发送的位置
				
		if( (sent!=remainder_item) || ( (status&0xFF)!= 0 ) ) //尽量保证按1024字节发送数据
		{
			if( (status&0xFF)==0x12 )  // 0x12 = Module send buffer full, and normally sent return with 0, i.e. this packet has not been sent anyway
			{       
				// printf("Module send buffer full!\r\n");                       
				// M8266HostIf_delay_us(250);    // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				M8266WIFI_Module_delay_ms(10);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
				// if(sent<SEND_DATA_MAX_SIZE)
				// 	total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend it					
			}
			else if(  ((status&0xFF)==0x14)  // 0x14 = connection of link_no not present
			||((status&0xFF)==0x15) ) 		 // 0x15 = connection of link_no closed
			{
				// do some work here, including re-establish the closed connect or link here and re-send the packet if any
				// additional work here
				printf("[WARNING]Connection of link_no not present OR connection of link_no %d  closed!\r\n",g_link_no);
				goto exit;
			}
			else if	((status&0xFF)==0x18)       // 0x18 = TCP server in listening states and no tcp clients connecting to so far
			{
				printf("[WARNING]TCP server in listening states and no tcp clients connecting to so far!\r\n");
				M8266HostIf_delay_us(250);	
				goto exit;
			}				
			else // 0x10, 0x11 here may due to spi failure during sending, and normally packet has been sent partially, i.e. sent!=0
			{
				// do some work here
				printf("[WARNING]There are some Unknown errors! status:%4x\r\n",status);
				goto exit;
			}
			
		}
		remainder_item = buflen - total_sent;
		// printf("remainder_item:%d\r\n",remainder_item);
		// PrintProgressBar(total_sent, buflen);
		M8266WIFI_Module_delay_ms(20);
	}
exit:	
	if(total_sent == buflen)
	{
		// printf("[LOG]Transport_sendPacketBuffer Succeed <sent-buflen:%d-%d>|Socket:%d\r\n",total_sent,buflen,g_link_no);
	}
	else
		printf("[WARNING]Transport_sendPacketBuffer Fail <sent-buflen:%d-%d>|Socket:%d\r\n",total_sent,buflen,g_link_no);
    return total_sent;
}
/*
u16 WiFiSendPacketBuffer(u8* buf, u16 buflen)
{
	// u8 stateValeu=0;
	u16 status	= 0;	 //用于回传状态 

    int remainder_item;  // 剩余的字节数量
	int sent = 0;
	int total_sent = 0;

	// PrintProgressBarInit();
	IWDG_Feed();
	remainder_item = buflen;
	while(remainder_item >= SEND_DATA_MAX_SIZE)
	{
		sent = M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE, g_link_no, &status);
		total_sent += sent;  // 更新待发送的位置

		if( (sent!=SEND_DATA_MAX_SIZE) || ( (status&0xFF)!= 0 ) )  // 尽量保证按1024字节发送数据
		{
			if( (status&0xFF)==0x12 )  // 0x12 = Module send buffer full, and normally sent return with 0, i.e. this packet has not been sent anyway
			{       
				// printf("Module send buffer full!\r\n");                       
				// M8266HostIf_delay_us(250);    // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				M8266WIFI_Module_delay_ms(10);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
				// if(sent<SEND_DATA_MAX_SIZE)
				// total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend it					
			}
			else if(((status&0xFF)==0x14)  	// 0x14 = connection of link_no not present
			||((status&0xFF)==0x15) )		// 0x15 = connection of link_no closed
			{
				// do some work here, including re-establish the closed connect or link here and re-send the packet if any
				// additional work here
				printf("[WARNING]Connection of socket=%d not present OR connection of socket=%d closed!\r\n",g_link_no,g_link_no);
				goto exit;	
			}
			else if	((status&0xFF)==0x18)  //  0x18 = TCP server in listening states and no tcp clients connecting to so far
			{
				printf("[WARNING]TCP server in listening states and no tcp clients connecting to so far!\r\n");
				M8266HostIf_delay_us(250);	
				goto exit;
			}				
			else  // 0x10, 0x11 here may due to spi failure during sending, and normally packet has been sent partially, i.e. sent!=0
			{
				// do some work here
				printf("[WARNING]There are some Unknown errors! status:%4x\r\n",status);
				// M8266HostIf_delay_us(250);       // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				// M8266WIFI_Module_delay_ms(25);	   // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
				// if(sent<SEND_DATA_MAX_SIZE)
				// 	total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend the left packet
				goto exit;
			}
			
		}
		remainder_item = buflen - total_sent;
		
		// printf("remainder_item:%d\r\n",remainder_item);
		// PrintProgressBar(total_sent, buflen);
		M8266WIFI_Module_delay_ms(20);
	}

	// Send the remaining items
	while(total_sent != buflen)//当小于1024的时候回调出
	{
		sent = M8266WIFI_SPI_Send_Data(buf+total_sent, remainder_item, g_link_no, &status);
		total_sent += sent;	//更新待发送的位置
				
		if( (sent!=remainder_item) || ( (status&0xFF)!= 0 ) ) //尽量保证按1024字节发送数据
		{
			if( (status&0xFF)==0x12 )  // 0x12 = Module send buffer full, and normally sent return with 0, i.e. this packet has not been sent anyway
			{       
				// printf("Module send buffer full!\r\n");                       
				// M8266HostIf_delay_us(250);    // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				M8266WIFI_Module_delay_ms(10);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
				// if(sent<SEND_DATA_MAX_SIZE)
				// 	total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend it					
			}
			else if(  ((status&0xFF)==0x14)  // 0x14 = connection of link_no not present
			||((status&0xFF)==0x15) ) 		 // 0x15 = connection of link_no closed
			{
				// do some work here, including re-establish the closed connect or link here and re-send the packet if any
				// additional work here
				printf("[WARNING]Connection of link_no not present OR connection of link_no %d  closed!\r\n",g_link_no);
				goto exit;
			}
			else if	((status&0xFF)==0x18)       // 0x18 = TCP server in listening states and no tcp clients connecting to so far
			{
				printf("[WARNING]TCP server in listening states and no tcp clients connecting to so far!\r\n");
				M8266HostIf_delay_us(250);	
				goto exit;
			}				
			else // 0x10, 0x11 here may due to spi failure during sending, and normally packet has been sent partially, i.e. sent!=0
			{
				// do some work here
				printf("[WARNING]There are some Unknown errors! status:%4x\r\n",status);
				goto exit;
			}
			
		}
		remainder_item = buflen - total_sent;
		// printf("remainder_item:%d\r\n",remainder_item);
		// PrintProgressBar(total_sent, buflen);
		M8266WIFI_Module_delay_ms(20);
	}
exit:	
	if(total_sent == buflen)
	{
		// printf("[LOG]Transport_sendPacketBuffer Succeed <sent-buflen:%d-%d>|Socket:%d\r\n",total_sent,buflen,g_link_no);
	}
	else
		printf("[WARNING]Transport_sendPacketBuffer Fail <sent-buflen:%d-%d>|Socket:%d\r\n",total_sent,buflen,g_link_no);
    return total_sent;
}
*/

//u16 WiFiSendPacketBuffer(u8* buf, u16 buflen)
/*u16 WiFiSendPacketBuffer(u8* buf, u16 buflen)
{
	u16 status	= 0;	//用于回传状态 
//	u8  ret;	//用于赋值连接

    int remainder_item;//剩余的字节数量
	int sent = 0;
	int total_sent = 0;
//	remainder_item = buflen - total_sent;
//	printf("Start remainder_item:%d\r\n",remainder_item);
	remainder_item = buflen;
	while(remainder_item >= SEND_DATA_MAX_SIZE)//当小于1024的时候回调出
	{
		sent = M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE, g_link_no, &status);
		total_sent += sent;	//更新待发送的位置

		if( (sent!=SEND_DATA_MAX_SIZE) || ( (status&0xFF)!= 0 ) ) //尽量保证按1024字节发送数据
		{
			if( (status&0xFF)==0x12 )  				   // 0x12 = Module send buffer full, and normally sent return with 0, i.e. this packet has not been sent anyway
			{       
				printf("Module send buffer full!\r\n");                       
				//M8266HostIf_delay_us(250);       // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				M8266WIFI_Module_delay_ms(10);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
//				if(sent<SEND_DATA_MAX_SIZE)
//					total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend it					
			}
			else if(  ((status&0xFF)==0x14)      // 0x14 = connection of link_no not present
			||((status&0xFF)==0x15) )    // 0x15 = connection of link_no closed
			{
				// do some work here, including re-establish the closed connect or link here and re-send the packet if any
				// additional work here
				printf("Connection of link_no not present OR connection of link_no %d  closed!\r\n",g_link_no);
				goto exit;
//				g_link_no++;
//				
//				if(g_link_no>=4)
//					g_link_no=0;
//								
//				ret = M8266WIFI_Config_Connection_via_SPI(TEST_CONNECTION_TYPE, TEST_LOCAL_PORT, TEST_REMOTE_IP_ADDR, TEST_REMOTE_PORT, g_link_no);
//				
//				if(ret == ERROR)
//				{
//					printf("Socket %d  Connect Error\r\n",g_link_no);
//				}
//				else
//				{
//					printf("Socket %d Opened\r\n",g_link_no);
//					mqtt_connect();
//				}
//				M8266WIFI_Module_delay_ms(1000);	
			}
			else if	( (status&0xFF)==0x18)       // 0x18 = TCP server in listening states and no tcp clients connecting to so far
			{
				printf("TCP server in listening states and no tcp clients connecting to so far!\r\n");
				M8266HostIf_delay_us(250);	
				goto exit;
			}				
			else                                 // 0x10, 0x11 here may due to spi failure during sending, and normally packet has been sent partially, i.e. sent!=0
			{
				// do some work here
				printf("There are some Unknown errors! status:%x\r\n",status);
				//M8266HostIf_delay_us(250);       // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				//M8266WIFI_Module_delay_ms(25);	   // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
//				if(sent<SEND_DATA_MAX_SIZE)
//					total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend the left packet
				goto exit;
			}
			
		}
		remainder_item = buflen - total_sent;
		printf("remainder_item:%d\r\n",remainder_item);
		M8266WIFI_Module_delay_ms(20);
	}
	//发送余项
//	M8266WIFI_Module_delay_ms(100);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
//	sent = M8266WIFI_SPI_Send_Data(buf+total_sent, remainder_item, g_link_no, &status); 
//	printf("last remainder_item sent:%d,status:%x\r\n",sent,status);
//	M8266WIFI_Module_delay_ms(500);
//	total_sent += sent;
	//printf("total_sent=%d	buflen=%d\r\n",total_sent,buflen);
	while(total_sent != buflen)//当小于1024的时候回调出
	{
		sent = M8266WIFI_SPI_Send_Data(buf+total_sent, remainder_item, g_link_no, &status);
		total_sent += sent;	//更新待发送的位置
				
		if( (sent!=remainder_item) || ( (status&0xFF)!= 0 ) ) //尽量保证按1024字节发送数据
		{
			if( (status&0xFF)==0x12 )  				   // 0x12 = Module send buffer full, and normally sent return with 0, i.e. this packet has not been sent anyway
			{       
				printf("Module send buffer full!\r\n");                       
				//M8266HostIf_delay_us(250);       // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 250us here, may repeat query 8- times, but in a small interval
				M8266WIFI_Module_delay_ms(10);	 // if 500Kbytes/s, need 2ms for 1Kbytes packet to be sent. If use 1ms here,   may repeat query 2- times, but in a large interval
//				if(sent<SEND_DATA_MAX_SIZE)
//					total_sent += M8266WIFI_SPI_Send_Data(buf+total_sent, SEND_DATA_MAX_SIZE-sent, g_link_no, &status);  // try to resend it					
			}
			else if(  ((status&0xFF)==0x14)      // 0x14 = connection of link_no not present
			||((status&0xFF)==0x15) )    // 0x15 = connection of link_no closed
			{
				// do some work here, including re-establish the closed connect or link here and re-send the packet if any
				// additional work here
				printf("Connection of link_no not present OR connection of link_no %d  closed!\r\n",g_link_no);
				goto exit;
			}
			else if	( (status&0xFF)==0x18)       // 0x18 = TCP server in listening states and no tcp clients connecting to so far
			{
				printf("TCP server in listening states and no tcp clients connecting to so far!\r\n");
				M8266HostIf_delay_us(250);	
				goto exit;
			}				
			else // 0x10, 0x11 here may due to spi failure during sending, and normally packet has been sent partially, i.e. sent!=0
			{
				// do some work here
				printf("There are some Unknown errors! status:%x\r\n",status);
				goto exit;
			}
			
		}
		remainder_item = buflen - total_sent;
		printf("remainder_item:%d\r\n",remainder_item);
		M8266WIFI_Module_delay_ms(20);
	}
	//M8266WIFI_Module_delay_ms(12);
exit:	
	if(total_sent == buflen)
		printf("transport_sendPacketBuffer Successfully <sent-buflen:%d-%d>/no%d\r\n",total_sent,buflen,g_link_no);
	else
		printf("transport_sendPacketBuffer Failed <sent-buflen:%d-%d>\r\n",total_sent,buflen);
    return total_sent;
}
*/

