#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"
#include "M8266WIFI_ops.h"

#include "stdio.h"
#include "led.h"
#include "sys.h"
#include "delay.h"


#include "transport.h"


#define  RECV_DATA_MAX_SIZE  2048  

void M8266WIFI_Test(void)
{
	u16 i;
	u16 status	= 0;
	u8  link_no	=0;
	 
	//打印连接的信息
//	printf("TEST_CONNECTION_TYPE = %d\r\n",TEST_CONNECTION_TYPE);
//	printf("TEST_LOCAL_PORT = %d\r\n",TEST_LOCAL_PORT);
//	printf("TEST_REMOTE_IP_ADDR = %d\r\n",TEST_REMOTE_IP_ADDR);
//	printf("TEST_REMOTE_PORT = %d\r\n",TEST_REMOTE_PORT);
//	printf("link_no = %d\r\n",link_no);
  	//u8 M8266WIFI_Reboot_And_Config_Connection(u8 tcp_udp, u16 local_port, u8* remote_ip, u16 remote_port, u8 link_no)
	if(M8266WIFI_Config_Connection_via_SPI(TEST_CONNECTION_TYPE, TEST_LOCAL_PORT, TEST_REMOTE_IP_ADDR, TEST_REMOTE_PORT, link_no)==0)
	{		
		printf("M8266WIFI_Config_Connection_via_SPI error!!!led flash in 1Hz\r\n");
		// led flash in 1Hz when error
		while(1)
		{
			LED0=0;LED1=0; M8266WIFI_Module_delay_ms(500);
			LED0=1;LED1=1; M8266WIFI_Module_delay_ms(500);
		}
	}
	else
	{
		printf("reboot wifi module and set up connection successfully!\r\n");
	}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
	{
		u8  RecvData[RECV_DATA_MAX_SIZE];   // make sure the stack size is more than RECV_DATA_MAX_SIZE
		u16 received = 0;
		u16 sent;
		u16 cnt;
		printf("Echo test: to receive data from remote and then echo back to remote!\r\n");
		for(i=0; i<RECV_DATA_MAX_SIZE; i++) RecvData[i]=i; 
		
		link_no = 0;
		sent = M8266WIFI_SPI_Send_Data(RecvData, 1024, link_no, &status);

		while(1)
		{
			if(M8266WIFI_SPI_Has_DataReceived())
			{
				//u16 M8266WIFI_SPI_RecvData(u8 data[], u16 max_len, uint16_t max_wait_in_ms, u8* link_no, u16* status);
				received = M8266WIFI_SPI_RecvData(RecvData, RECV_DATA_MAX_SIZE, 1, &link_no, &status);
					
				if((status&0xFF)!=0)
				{	cnt=0;				 
					printf("\r\n Received data from from M8266 with an error, status = 0x%04X\r\n", status);			 
				}
				else
				{	cnt++;				 
					printf("\r\n Received data from from M8266 with no error %d\r\n",cnt);			 
				}
				
				// Step 1: receive data
				printf(" Received Data[%d] from M8266 = ", received);
				//打印接收到的数据
				for(i=0; i<received; i++)
				{
					if(i%8==0) printf("\r\n  %03d :", i);
					printf(" %c", RecvData[i]);
				}
				printf("\r\n");
				
				// Step 2: echo the data
				if(received!=0)
				{
					//u16 M8266WIFI_SPI_Send_Data(u8 data[], u16 data_len, u8 link_no, u16* status);
					sent = M8266WIFI_SPI_Send_Data(RecvData, received, link_no, &status);
					if( (sent!=received) || ( (status&0xFF)!= 0 ) )
					{
						printf("\r\n Send Data[len=%d] to M8266 failed. Only %d bytes has been sent(0x%04X).\r\n", received, sent, status);
						break;
					}
					else
					{
						printf("\r\n Send Data[len=%d] to M8266 successfully(%04X).\r\n", received, status);
					}				
				} // end of if((status&0xFF)==0)
			} // end of if(M8266WIFI_SPI_Has_DataReceived())
		} // end of while(1)
	}	 
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
} // end of M8266WIFI_Test
