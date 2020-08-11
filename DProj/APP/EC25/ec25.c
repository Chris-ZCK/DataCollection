#include "ec25.h"
#include "usart3.h"
#include "UART_TCPbuff.h"
#include "MyFunction_C.h"
// foundmental
#include "power.h"

#define CHANGE_BAUD 0
/**
 * @description: ec25_Init
 * @param {type} 
 * @return: 
 */
EC25_ERR ec25_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // 使能GPIOD时钟

	// GPIOD13,GPIOD6,GPIOD5初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	  // 普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   //上拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);			   // 初始化

	GPIO_SetBits(GPIOD, GPIO_Pin_7);	// GPIOD7(W_DISABLE#) 高，启动网络；
	GPIO_ResetBits(GPIOD, GPIO_Pin_14); // GPIOD6(PEREST) 高，不重启,由于reset加入了一个三极管电路
	GPIO_ResetBits(GPIOD, GPIO_Pin_13); // GPIOD13(DTR) 设置为低，启动模块
	#if CHANGE_BAUD
	F407USART3_Init(115200); // config & fixed
	#else
	F407USART3_Init(921600); // config & fixed
	#endif
	POWER_D = 1;			 // 打开4G电源
#if (EN_log_print >= 3)
	F407USART1_SendString("POWER_4G,ec25_Init\r\n");
#endif
	delay_ms(1000);
	return ec25_tcp_Conditon_Normal();
}
/**
 * @description: 硬件重启模块
 * @param ms a low level voltage with the frame of150~460ms, 300ms recommand
 * @return: 
 */
void ec25_reset(int ms)
{
#if (EN_log_print >= 3)
	F407USART1_SendString("reset_modle\r\n");
#endif
	EC25_PERST = 1;
	delay_ms(ms);
	EC25_PERST = 0;
}
/**
 * @description: ec25休眠
 * @param {type} 
 * @return: 
 */
void ec25_DTR_Sleep(void)
{
	waitString_set("OK", 2);
	// 关闭网络，否则可能会打开失败
	F407USART3_SendString("AT+QICLOSE=0\r\n");
	F407USART1_SendString("AT+QICLOSE=0\r\n");
	WaitString_OKflag(5000);

	// Deactivate a PDP Context
	waitString_set("OK:", 2);
	F407USART3_SendString("AT+QIDEACT=1\r\n");
	F407USART1_SendString("AT+QIDEACT=1\r\n");
	WaitString_OKflag(5000);

	EC24_DTR = 1;
}
/**
 * @description: 硬件唤醒EC25
 * wake up the module
 * @param {type} 
 * @return: 
 */
void ec25_DTR_Wakeup(void)
{
	EC24_DTR = 0; // active wake up
}
/**
 * @description: 模组关机
 * turn off the module
 * @param {type} 
 * @return: 
 */
void ec25_Turn_off(void)
{
	waitString_set("POWERED DOWN", 12);
	// 关闭网络，否则可能会打开失败
	F407USART3_SendString("AT+QPOWD\r\n");
#if (EN_log_print >= 1)
	F407USART1_SendString("AT+QPOWD\r\n");
#endif
	WaitString_OKflag(5000);
	delay_ms(1000);
	POWER_D = 0; // turn off the power
	delay_ms(1000);
}
/**
 * @description: ec25重启
 * @param {type} none
 * @return: none
 */
void ec25_Restart(void)
{
	ec25_Turn_off();
	POWER_D = 0; // 打开4G电源
	delay_ms(1000);
	ec25_Init();
}

/**
 * @description: EC25正常方式建立链接
 * @param {type} 
 * @return: 
 */
EC25_ERR ec25_tcp_Conditon_Normal(void)
{
	uint8_t i;
	// Power on the module, already done
	ec25_reset(460); // Reboot the module
	// 等待电源稳定，可以接收到AT指令

	for (i = 0; i < 30; i++)
	{
		waitString_set("OK", 2);
		F407USART3_SendString("AT\r\n");
		F407USART1_SendString("AT\r\n");
		if (WaitString_OKflag(2000))
			break;
	} // maxium wait 20s
	// config uart
	waitString_set("OK", 2);
	F407USART3_SendString("AT+IFC=2,2\r\n");
	F407USART1_SendString("AT+IFC=2,2\r\n");
	WaitString_OKflag(3000);
	//config gps
	
	waitString_set("OK", 2);
	F407USART3_SendString("AT+QGPS=1,1\r\n");
	F407USART1_SendString("$AT+QGPS=1,1\r\n");
	if(WaitString_OKflag(1000))
		printf("*success open gps\r\n");
	else
		printf("*!Try open gps\r\n");
	
	waitString_set("OK", 2);
	F407USART3_SendString("AT+QGPSCFG=\"autogps\",1\r\n");
	F407USART1_SendString("$AT+QGPSCFG=\"autogps\",1\r\n");
	if(WaitString_OKflag(3000))
		printf("*success autogps\r\n");
	else
		printf("*!fail autogps\r\n");
	
	/* 用于设置默认的UART波特率 */
	#if CHANGE_BAUD
	waitString_set("OK", 2);
	F407USART3_SendString("AT+IPR=921600;&W\r\n");
	F407USART1_SendString("AT+IPR=921600;&W\r\n");
	WaitString_OKflag(3000);	
	#endif
	return ec25_Act_PDP_Condition();
}
/**
 * @description: act PDP Condition
 * @param {type} none
 * @return: EC25_ERR
 */
EC25_ERR ec25_Act_PDP_Condition(void)
{
	uint8_t i, j, res;
	/* Use AT+CPIN?/AT+CREG?/AT+CGREG? to query the SIM status and network registration status */
	// Query the SIM status
	i = 1;
	while (i++)
	{
		waitString_set("+CPIN: READY", 12);
		F407USART3_SendString("AT+CPIN?\r\n");
		F407USART1_SendString("AT+CPIN?\r\n");
		res = WaitString_OKflag(2000);
		if (res)
		{
			printf("Find (U)SIM card\r\n");
			break;
		}
		else if (i > 10)
		{
			printf("failed to identify (U)SIM card\r\n");
			return EC25_ERR_CPIN;
		} // maxium wait 20s
	}
	// Query CS service
	IWDG_Feed();
	i = 1;
	while (i++)
	{
		waitString_set("+CREG: 0,1", 10);
		F407USART3_SendString("AT+CREG?\r\n");
		F407USART1_SendString("AT+CREG?\r\n");
		res = WaitString_OKflag(2000);
		if (res)
		{
			printf("Register on CS domain service\r\n");
			break;
		}
		else if (i > 20)
		{
			printf("Failed to register on CS domain service\r\n");
			return EC25_ERR_CREG;
		} // maxium wait 40s
	}
	// Query PS service
	i = 1;
	while (i++)
	{
		waitString_set("+CGREG: 0,1", 10);
		F407USART3_SendString("AT+CGREG?\r\n");
		F407USART1_SendString("AT+CGREG?\r\n");
		res = WaitString_OKflag(2000);
		if (res)
		{
			printf("Register on PS domain service\r\n");
			break;
		}
		else if (i > 20)
		{
			printf("Failed to register on PS domain service\r\n");
			return EC25_ERR_CGREG;
		} // maxium wait 40s
	}

#if In_Condition_HCTV == 1
	/* Use AT+QICSGP=1,1,"HCTV","","",3 to set APN as "HCTV",user name as "",password as ""*/
	//set APN
	waitString_set("OK", 2);
	F407USART3_SendString("AT+QICSGP=1,1,\"HCTV\",\"\",\"\",3\r\n");
	F407USART1_SendString("AT+QICSGP=1,1,\"HCTV\",\"\",\"\",3\r\n");
	// F407USART3_SendString("AT+QICSGP=1,1,\"HCTV\",\"\",\"\",0\r\n");
	// F407USART1_SendString("AT+QICSGP=1,1,\"HCTV\",\"\",\"\",0\r\n");
	WaitString_OKflag(3000);
	delay_ms(1000);
	// soft reset network
	waitString_set("OK", 2);
	F407USART3_SendString("AT+CFUN=0\r\n");
	F407USART1_SendString("AT+CFUN=0\r\n");
	WaitString_OKflag(3000);
	delay_ms(500);
	waitString_set("OK", 2);
	F407USART3_SendString("AT+CFUN=1\r\n");
	F407USART1_SendString("AT+CFUN=1\r\n");
	WaitString_OKflag(3000);
	delay_ms(1000);
#endif //In_Condition_HCTV 

	// act PDP环境
	j = 5;
	while(j--)	
	{
		waitString_set("OK", 2);
		F407USART3_SendString("AT+QIACT=1\r\n");
		F407USART1_SendString("AT+QIACT=1\r\n");
		res = WaitString_OKflag(5000);
		if (!res)
		{
			printf("Fail:AT+QIACT\r\n");
			//waitString_set("OK:", 2);
			//F407USART3_SendString("AT+QIDEACT=1\r\n");
			//F407USART1_SendString("AT+QIDEACT=1\r\n");
			//res = WaitString_OKflag(5000);
			//break;
		}			
		// Query PDP condition
		i = 3; // maxium wait 20s
		while (i--)
		{
			waitString_set("+QIACT", 6);
			F407USART3_SendString("AT+QIACT?\r\n");
			F407USART1_SendString("AT+QIACT?\r\n");
			res = WaitString_OKflag(2000);
			if (res)
			{
				printf("Success:AT+QIACT\r\n");
				return EC25_ERR_NONE;
			}
		}
		printf("Failed to register on PS domain service\r\n");
	}
	return EC25_ERR_QIACT;
}
/**
 * @description: ec25 Open Connection
 * @param uint8_t *ip_port 包含IP和PORT的指令串
 * @return: 0 成功, 非零 EC25_ERR 失败
 * @note: 前提是PDP环境已经打开
 */
EC25_ERR ec25_TCPConnect(char *ip, uint16_t port)
{
	uint8_t i, res;
	char ptr[50];
	// close Network
	i = 1;
	printf("*Function:ec25_TCPConnect\r\n");
	sprintf(ptr, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n", ip, port);
	while (i++)
	{
		waitString_set("OK", 2);
		// 关闭网络，否则可能会打开失败
		F407USART3_SendString("AT+QICLOSE=0\r\n");
		printf("@AT+QICLOSE=0|{(try i<10)i:%d}\r\n",i);
		res = WaitString_OKflag(3000);
		if (res)
		{
			printf("*succeed QICLOSE\r\n");
			break;
		}
		else if (i > 10)
		{
			printf("*Fail:AT+QICLOSE\r\n");
			return EC25_ERR_QICLOSE;
		} // maxium wait 40s
	}
	// Establish Network
	i = 1;
	while (i++)
	{
		waitString_set("+QIOPEN: 0,0", 12);
		F407USART3_SendString(ptr);
		printf("@%s", ptr);
		WaitString_OKflag(5000);
		delay_ms(500);
		// Querry QISTATE
		waitString_set("+QISTATE", 8);
		F407USART3_SendString("AT+QISTATE?\r\n");
		F407USART1_SendString("@AT+QISTATE?\r\n");
		res = WaitString_OKflag(2000);
		if (res)
		{
			F407USART1_SendString("*ec25_TCP_Connected\r\n");
			break;
		}
		else if (i > 3)
		{
			F407USART1_SendString("*Fail:ec25_TCP_Connect\r\n");
			return EC25_ERR_QIOPEN;
		} // maxium wait 40s
		printf("*Try open connect|{(if i>3)i=%d}\r\n",i);
	}
	//ec25_SynLocalTime();
	return EC25_ERR_NONE;
}
/**
 * @description: 断开TCP连接
 * @param {type} none
 * @return: 
 */
EC25_ERR ec25_TCPDisConnect(void)
{
	waitString_set("OK", 2);
	// 关闭网络，否则可能会打开失败
	F407USART3_SendString("AT+QICLOSE=0\r\n");
	F407USART1_SendString("AT+QICLOSE=0\r\n");
	return (EC25_ERR)WaitString_OKflag(5000);
}
#if 0
/**
 * @description: TCP Connect
 * @param {type} 
 * @return: 
 * @note: refer "Quectel EC2x TCP(IP) AT Commands" page 8
 */
	uint8_t SIM7100_TCPConnect(uint8_t *ip, uint16_t port)
{
	OS_ERR err;
	uint8_t res;
	uint8_t i;
	char ptr[50] = {0};

	waitString_set("OK", 2);
	// 关闭网络，否则可能会打开失败
	F407USART3_SendString("AT+QICLOSE=0\r\n");
	F407USART1_SendString("AT+QICLOSE=0\r\n");
	res = WaitString_OKflag(5000);
	if (!res)
		printf("Fail:AT+QICLOSE\r\n");
	delay_ms(1000);

	waitString_set("OK:", 2);
	F407USART3_SendString("AT+QIDEACT=1\r\n");
	F407USART1_SendString("AT+QIDEACT=1\r\n");
	res = WaitString_OKflag(5000);
	if (!res)
		printf("Fail:AT+QIDEACT\r\n");
	delay_ms(1000);

	waitString_set("OK", 2);
	F407USART3_SendString("AT+CFUN=0\r\n");
	F407USART1_SendString("AT+CFUN=0\r\n");
	res = WaitString_OKflag(3000);
	if (!res)
		printf("Fail:AT+CFUN=0\r\n");
	delay_ms(500);
	waitString_set("OK", 2);
	F407USART3_SendString("AT+CFUN=1\r\n");
	F407USART1_SendString("AT+CFUN=1\r\n");
	res = WaitString_OKflag(3000);
	if (!res)
		printf("Fail:AT+CFUN=0\r\n");

	for (i = 0; i < 10; i++)
	{
		waitString_set("+CREG: 0,1", 10);
		F407USART3_SendString("AT+CREG?\r\n");
		F407USART1_SendString("AT+CREG?\r\n");
		if (WaitString_OKflag(3000))
			break;
		else
		{
			printf("Fail:AT+CREG,i=%d\r\n", i);
		}
	}

	waitString_set("+CSQ", 2);
	F407USART3_SendString("AT+CSQ\r\n");
	F407USART1_SendString("AT+CSQ\r\n");
	WaitString_OKflag(3000);

	waitString_set("OK:", 2);
	// 打开网络
	F407USART3_SendString("AT+QIACT=1\r\n");
	F407USART1_SendString("AT+QIACT=1\r\n");
	res = WaitString_OKflag(5000);
	if (!res)
		printf("Fail:AT+QIACT\r\n");

	delay_ms(1000);

	waitString_set("+QIOPEN: 0,0", 12); //命令模式等待该响应
	//打开tcp连接
	sprintf(ptr, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n", ip, port);
	//F407USART1_SendString("AT+QIOPEN=1,0,\"TCP\",\"112.74.59.250\",10002,0,1\r\n");
	F407USART3_SendString(ptr);
#if (EN_log_print >= 1)
	F407USART1_SendString(ptr);
#endif // EN_log_print
	res = WaitString_OKflag(10000);
	if (res)
	{
		F407USART1_SendString("ec25_TCP_Connected\r\n");
	}
	else
	{
		F407USART1_SendString("Fail:ec25_TCP_Connect\r\n");
	}
	return res;
}

uint8_t SIM7100_TCPConnect(uint8_t *ip, uint16_t port)
{
	OS_ERR err;
	uint8_t res;
	uint8_t i;
	char ptr[50];

	waitString_set("OK", 2);
	// 关闭网络，否则可能会打开失败
	F407USART3_SendString("AT+QICLOSE=0\r\n");
	F407USART1_SendString("AT+QICLOSE=0\r\n");
	res = WaitString_OKflag(5000);
	if (!res)
		printf("Fail:AT+QICLOSE\r\n");
	delay_ms(1000);

	waitString_set("OK:", 2);
	F407USART3_SendString("AT+QIDEACT=1\r\n");
	F407USART1_SendString("AT+QIDEACT=1\r\n");
	res = WaitString_OKflag(5000);
	if (!res)
		printf("Fail:AT+QIDEACT\r\n");
	delay_ms(1000);

	waitString_set("OK", 2);
	F407USART3_SendString("AT+CFUN=0\r\n");
	F407USART1_SendString("AT+CFUN=0\r\n");
	res = WaitString_OKflag(3000);
	if (!res)
		printf("Fail:AT+CFUN=0\r\n");
	delay_ms(500);
	waitString_set("OK", 2);
	F407USART3_SendString("AT+CFUN=1\r\n");
	F407USART1_SendString("AT+CFUN=1\r\n");
	res = WaitString_OKflag(3000);
	if (!res)
		printf("Fail:AT+CFUN=0\r\n");

	for (i = 0; i < 10; i++)
	{
		waitString_set("+CREG: 0,1", 10);
		F407USART3_SendString("AT+CREG?\r\n");
		F407USART1_SendString("AT+CREG?\r\n");
		if (WaitString_OKflag(3000)) 
			break;
		else
		{
			printf("Fail:AT+CREG,i=%d\r\n",i);
		}
		
	}

	waitString_set("+CSQ", 2);
	F407USART3_SendString("AT+CSQ\r\n");
	F407USART1_SendString("AT+CSQ\r\n");
	WaitString_OKflag(3000);

	waitString_set("OK:", 2);
	// 打开网络
	F407USART3_SendString("AT+QIACT=1\r\n");
	F407USART1_SendString("AT+QIACT=1\r\n");
	res = WaitString_OKflag(5000);
	if (!res)
		printf("Fail:AT+QIACT\r\n");

	delay_ms(1000);

	waitString_set("+QIOPEN: 0,0", 12); //命令模式等待该响应
	//打开tcp连接
	sprintf(ptr, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n", ip, port);
	//F407USART1_SendString("AT+QIOPEN=1,0,\"TCP\",\"112.74.59.250\",10002,0,1\r\n");
	F407USART3_SendString(ptr);
#if (EN_log_print >= 1)
	F407USART1_SendString(ptr);
#endif // EN_log_print
	res = WaitString_OKflag(10000);
	if (res)
	{
		F407USART1_SendString("ec25_TCP_Connected\r\n");
	}
	else
	{
		F407USART1_SendString("Fail:ec25_TCP_Connect\r\n");
	}
	return res;
}

/**
 * @description: 发送失败后重新建立链接
 * @param {type} 
 * @return: 
 */
uint8_t ec25_TCPReconnect(uint8_t *ip, uint16_t port)
{
	OS_ERR err;
	uint8_t res;
	uint8_t i;
	char ptr[50] = {0};

	waitString_set("OK", 2);
	// 关闭网络，否则可能会打开失败
	F407USART3_SendString("AT+QICLOSE=0\r\n");
	F407USART1_SendString("AT+QICLOSE=0\r\n");
	res = WaitString_OKflag(10000);
	if (!res)
	{
		printf("Fail:AT+QICLOSE\r\n");
		return EC25_ERR_QICLOSE;
	}

	delay_ms(1000);

	// F407USART1_SendString("AT+QIOPEN=1,0,\"TCP\",\"112.74.59.250\",10002,0,1\r\n");
	sprintf(ptr, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n", ip, port);
	// 尝试链接5次
	for (i = 0; i < 5; i++)
	{
		waitString_set("+QIOPEN: 0,0", 12); // 命令模式等待该响应
		// 打开tcp连接
		F407USART3_SendString(ptr);
#if (EN_log_print >= 1)
		F407USART1_SendString(ptr);
#endif // EN_log_print
		res = WaitString_OKflag(10000);
		if (res)
		{
			F407USART1_SendString("ec25_TCP_Connected\r\n");
			return EC25_ERR_NONE;
		}
	}
	F407USART1_SendString("Fail:ec25_TCP_Connect,EC25_ERR_QIOPEN\r\n");
	return EC25_ERR_QIOPEN;
}
#endif // test

/**
 * @description: 发送一组数据
 * @param uint8_t *data 待发送图片地址, uint16_t length 待发送图片长度
 * @return: 
 */
uint32_t ec25_TCPSendData(char *data, uint16_t length)
{
	uint8_t res = 0;
	uint16_t len = length, index = 0;

	while (len > UART_SEND_PACK_LENGTH_MAX)
	{
		res = ec25_TCPSendDataPack(data + index, UART_SEND_PACK_LENGTH_MAX);
		if (res == 0)
		{
			printf("Fail:ec25_TCPSendDataPack-3\r\n");
			return 0;
		}
		len -= UART_SEND_PACK_LENGTH_MAX;
		index += UART_SEND_PACK_LENGTH_MAX;
	}
	if (len > 0)
	{
		res = ec25_TCPSendDataPack(data + index, len);
		if (res == 0)
		{
			printf("Fail:ec25_TCPSendDataPack-4\r\n");
			return 0;
		}
		index += len;
	}
	return index;
}

/**
 * @description: 发送一帧数据<=1460
 * @param uint8_t *data 待发送图片地址, uint16_t length 待发送图片长度
 * @return: 成功发送长度
 */
uint32_t stringtoNum(char* str);


uint16_t ec25_TCPSendDataPack(char *data, uint16_t length)
{
	uint8_t *buff;
	uint8_t res,offset,i;
	uint32_t unackedbytes;
	static char myBUF50[30];
	// check cache
	i=1;
	while(i++)
	{
		if(i>3)
		{
			F407USART1_SendString("\r\n\r\n*!(ec25_TCPSendDataPack/AT+QISEND)please check send status!\r\n\r\n");
			delay_ms(1000);
			return 0;
		}
		
		waitString_set("+QISEND: ", 9);
		buff = waitAppend_set(30);
		F407USART3_SendString("AT+QISEND=0,0\r\n");
		res = WaitAppend_OKflag(3000);
		if (res == 0)
		{
			printf("*!Fail:QISEND Queey Data, ec25_TCPSendDataPack-2：%d\r\n",i);
			delay_ms(100);
			continue;
		}
		offset = locate_character((char *)buff, ',');
		offset += locate_character((char *)(buff+offset), ',');  // 寻找第二个逗号
		unackedbytes = stringtoNum((char *)(buff+offset));
		
		// printf("unackedbytes=%d\r\n",unackedbytes);
		if(unackedbytes<5120)	
		{
			break;
		}
		else
		{
			printf(".wait.\r\n");
		}
	
		delay_ms(100);		
	}
	// send header
	waitString_set(">", 1);
	sprintf(myBUF50, "AT+QISEND=0,%d\r\n", length);
	F407USART3_SendString(myBUF50);
#if (EN_log_print >= 3)
	F407USART1_SendString(myBUF50);
#endif // EN_log_print
	res = WaitString_OKflag(5000);
	if (res == 0)
	{
		printf("Fail:AT+QISEND=0, ec25_TCPSendDataPack-1");
		return 0;
	}
	// send data
	// reveive SEND OK|SEND FAIL|ERROR	
	waitString_set("SEND ", 5);
	buff = waitAppend_set(2);
	F407USART3_SendBytes(data, length);
	res = WaitAppend_OKflag(10000);
	if (res == 0)
	{
		F407USART1_SendString("Fail:QISEND Data, ec25_TCPSendDataPack-2：");
		return 0;
	}
	if (buff[0]=='F' && buff[1]=='A') // FAIL
	{
		F407USART1_SendString("\r\n\r\n*!(ec25_TCPSendDataPack/SEND DATA)please check send status!\r\n\r\n");
		delay_ms(1000);
		return 0;
	}	
	return length;
}
// succeed:1 fail:0
uint16_t ec25_SynLocalTime(void)
{
	uint8_t *buff;
	uint8_t res, i;
	F407USART1_SendString("ec25_SynLocalTime\r\n");
	i = 1;
	while (i++)
	{
		// 循环尝试10次
		if (i > 3)
		{
			F407USART1_SendString("\r\n\r\n*!(ec25_SynLocalTime/AT+QNTP)please check send status!\r\n\r\n");
			delay_ms(3000);
			return 0;
		}
		
		waitString_set("+QNTP: ", 7);
		buff = waitAppend_set(40);
		F407USART3_SendString("AT+QNTP=1,\"120.25.115.20\",123\r\n");
		//+QNTP: 0,"2019/09/28,06:40:53+00"
		res = WaitAppend_OKflag(5000);
		if (res == 0)
		{
			F407USART1_SendString("Fail:QISEND Synchronize Local Time, error TCP\r\n");
			delay_ms(100);
			continue;
		}
		if(buff[0]=='0')
		{
			calendar_get_time(&calendar);
			calendar_get_date(&calendar);
			printf("DATA:%d-%d-%d	Time:%d:%d:%d*\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
			calendar.w_year=stringtoNum((char *)buff+3);
			calendar.w_month=stringtoNum((char *)buff+8);
			calendar.w_date=stringtoNum((char *)buff+11);
			calendar.hour=stringtoNum((char *)buff+14);
			calendar.min=stringtoNum((char *)buff+17);
			calendar.sec=stringtoNum((char *)buff+20);
			printf("DATA:%d-%d-%d	Time:%d:%d:%d#\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
			my_RTC_Set_Time(calendar);
		}
		else
		{
			F407USART1_SendString("Fail:QISEND anay Synchronize Local Time, error anay\r\n");
			continue;
		}

		delay_ms(100);
		return 1;
	}
	return 0;
}

int ec25_QueeryCSQ(void)
{
	uint8_t *buff;
	uint8_t res, i;
	u16 ec25_data_csq;
	F407USART1_SendString("#ec25_QueeryCSQ\r\n");
	i = 1;
	while (i++)
	{
		IWDG_Feed();
		// 循环尝试10次
		if (i > 5)
		{
			F407USART1_SendString("\r\n\r\n*!(ec25_QueeryCSQ/AT+CSQ)please check send status!\r\n\r\n");
			delay_ms(1000);
			return -9999;
		}
		//AT+CSQ
		//+CSQ: 28,99
		waitString_set("+CSQ: ", 6);
		buff = waitAppend_set(30);
		F407USART3_SendString("AT+CSQ\r\n");
		//+QNTP: 0,"2019/09/28,06:40:53+00"
		res = WaitAppend_OKflag(10000);
		if (res == 0)
		{
			F407USART1_SendString("*!Fail:ec25_QueeryCSQ\r\n");
			delay_ms(100);
			continue;
		}
		ec25_data_csq = stringtoNum((char *)buff);
		printf("*info:Qurry EC25 CSQ|%d\r\n",ec25_data_csq);
		delay_ms(100);
		return ec25_data_csq;
	}
	return 0;
}


#define EC25_nmeasrc_ON 0
// success:1 fail:0
int ec25_QueeryGPS(void)
{
	uint8_t *buff;
	uint8_t res, i;
	u16 ec25_data_csq;
	F407USART1_SendString("ec25_QueeryGPS\r\n");

	IWDG_Feed();
	// 可以多次打开
	waitString_set("OK", 2);
	F407USART3_SendString("AT+QGPS=1\r\n");
	F407USART1_SendString("$AT+QGPS=1\r\n");
	res = WaitString_OKflag(2000);
	if (res)
	{
		printf("*Success Turn on GNSS\r\n");
		
	}
	else
	{
		printf("*!Try Turn on GNSS\r\n");
	}
	
	waitString_set("+QGPS: 1", 8);
	F407USART3_SendString("AT+QGPS?\r\n");
	F407USART1_SendString("$AT+QGPS?\r\n");
	res = WaitString_OKflag(3000);
	if(res)
	{
		printf("$GNSS is Working\r\n");
	}
	else
	{
		waitString_set("OK", 2);
		F407USART3_SendString("AT+QGPS=1\r\n");
		F407USART1_SendString("$AT+QGPS=1\r\n");
		res = WaitString_OKflag(2000);
		if (!res)
		{
			printf("$Fail Turn on GNSS\r\n");
			return 0;
		}
		else
		{
			printf("$Success Turn on GNSS\r\n");
		}
		
	}		

	#if EC25_nmeasrc_ON
	waitString_set("OK", 2);
	F407USART3_SendString("AT+QGPSCFG=\"nmeasrc\",1\r\n");
	F407USART1_SendString("$AT+QGPSCFG=\"nmeasrc\",1\r\n");		
	if (!res)
	{
		printf("$Fail Enable <nmeasrc> functionality.\r\n");
		continue;
	}	
	
	//AT+CSQ
	//+CSQ: 28,99
	waitString_set("+QGPSGNMEA: ", 11);
	buff = waitAppend_set(80);
	F407USART3_SendString("AT+QGPSGNMEA=\"GGA\"\r\n");
	F407USART1_SendString("$AT+QGPSGNMEA=\"GGA\"\r\n");	
	res = WaitAppend_OKflag(10000);
	if (res == 0)
	{
		F407USART1_SendString("Fail:ec25_QueeryGPS\r\n");
		delay_ms(100);
		continue;
	}
	NMEA_GNGGA_Analysis(&gpsx, buff);
	#else
	
	for (i = 0; i < 5; i++)
	{
		IWDG_Feed();
		//AT+QGPSLOC?
		//+QGPSLOC: 122809.0,3015.8155N,12006.9562E,1.3,47.0,2,0.00,0.0,0.0,171019,03<CR><LF>	
		waitString_set("+QGPSLOC: ", 10);
		buff = waitAppend_set(100);
		F407USART3_SendString("AT+QGPSLOC?\r\n");
		F407USART1_SendString("$AT+QGPSLOC?\r\n");
		//+QGPSLOC: 122809.0,3015.8155N,12006.9562E,1.3,47.0,2,0.00,0.0,0.0,171019,03<CR><LF>	
		res = WaitAppend_OKflag(2000);
		if (res)
		{
			printf("Qurry EC25 successful\r\n");
			break;
		}
		else
		{
			printf("*Try:ec25_QueeryGPS cnt:(max 8)%d\r\n",i);
			delay_ms(100);
		}
		delay_ms(1000);
		//mqtt_ping_task();
	}
	#endif
	waitString_set("OK", 2);
	F407USART3_SendString("AT+QGPSEND\r\n");
	F407USART1_SendString("$AT+QGPSEND\r\n");
	WaitAppend_OKflag(2000);
	
	if (res)
	{
		/*
		EC25_GNGGA_Analysis(&gpsx, buff);
		*/
		//ec25_data_csq = stringtoNum((char *)buff);
		printf("Qurry EC25 successful\r\n");
		delay_ms(100);
		return 1;
	}
	else
	{
		printf("*Fail:ec25_QueeryGPS\r\n");
		return 0;
	}
}

