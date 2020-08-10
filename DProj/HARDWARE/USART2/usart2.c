#include "sys.h"
#include "usart2.h"	  



#if (F407USART2_RECEIVE_BUFF_ENABLE == 1)
uint8_t F407USART2_buff[F407USART2_RECEIVE_BUFF_SIZE];  //UART3缓存区
volatile uint16_t F407USART2_buffHead = 0; // 头指针
volatile uint16_t F407USART2_buffEnd = 0;  // 尾指针
volatile uint8_t F407USART2_buffOverFlag = 0;  // 覆盖标志
///////////////////////////写入
/**
 * @description: 向F407USART2_buff写入一个字节
 * @param uint8_t data 待写入字节
 * @return: 0 写入失败，1 写入成功
 */
uint8_t F407USART2_buffWrite(uint8_t data)
{
	if (F407USART2_buffHead != F407USART2_buffEnd || F407USART2_buffOverFlag == 0)
	{
		F407USART2_buff[F407USART2_buffEnd] = data;
		F407USART2_buffEnd++;
		if (F407USART2_buffEnd >= F407USART2_RECEIVE_BUFF_SIZE)
		{
			F407USART2_buffEnd = 0;
			F407USART2_buffOverFlag = 1;
		}
		return 1;
	}
	return 0;
}
/**
 * @description: 向F407USART2_buff写入指定长度字符
 * @param uint8_t *data 待写入字符地址, uint16_t length 待写入字符长度
 * @return:  成功向F407USART2_buff写入字符长度
 */
uint8_t F407USART2_buffWrites(uint8_t *data, uint16_t length)
{
	uint16_t i = 0;
	for (i = 0; i < length; i++)
	{
		if (F407USART2_buffWrite(*(data + i)) == 0)
		{
			return 0;
		}
	}
	return i;
}

///////////////////////////读取
/**
 * @description: 从F407USART2_buff中读取一个字节的数据
 * @param uint8_t *data 数据地址
 * @return: 1 成功，0 失败
 * @note 循环覆盖
 */
uint8_t F407USART2_buffRead(uint8_t *data)
{
	if (F407USART2_buffHead != F407USART2_buffEnd || F407USART2_buffOverFlag == 1)
	{
		*data = F407USART2_buff[F407USART2_buffHead];
		F407USART2_buffHead++;
		if (F407USART2_buffHead >= F407USART2_RECEIVE_BUFF_SIZE)
		{
			F407USART2_buffHead = 0;
			F407USART2_buffOverFlag = 0;
		}
		return 1;
	}
	return 0;
}
/**
 * @description: 从F407USART2_buff中读取指定长度的数据到指定地址
 * @param uint8_t *data 指定地址, uint16_t length 指定长度
 * @return: uint16_t 读取的长度
 */
uint16_t F407USART2_buffReads(uint8_t *data, uint16_t length)
{
	uint16_t i = 0;
	for (i = 0; i < length; i++)
	{
		if (F407USART2_buffRead(data + i) == 0)
			return 0;
	}
	return length;
}
/**
 * @description: 获缓存区长度
 * @param {type} none
 * @return: uint16_t 缓存区长度
 */
uint16_t F407USART2_buffLength(void)
{
	return F407USART2_RECEIVE_BUFF_SIZE * F407USART2_buffOverFlag + F407USART2_buffEnd - F407USART2_buffHead;
}
#endif // F407USART2_RECEIVE_BUFF_ENABLE

void USART2_IRQHandler(void)
{
	static uint8_t data = 0;
	static uint8_t step = 0;

	//溢出错误，必须处理,否则会导致死机
	if (0x0008 & USART2->SR)
	{
		data = USART2->DR;
		step = 0;
	}
	else if (0x0020 & USART2->SR)
	{
		//读数据会自动清除中断标志位
		data = USART2->DR;
		F407USART2_buffWrite(data);
	}
}

//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率 
void USART2_init(u32 bound)
{  	
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 
	USART_DeInit(USART2);  //复位串口2
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2和GPIOA3初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化GPIOA2，和GPIOA3
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3复用为USART2	  
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口3
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断  
		
	USART_Cmd(USART2, ENABLE);                    //使能串口 
	
 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	//USART2_RX_STA=0;				//清零 
}

/**
 * @description: 向串口2发送一个字节数据
 * @param uint8_t Data
 * @return: none
 */
void F407USART2_SendByte(uint8_t Data)
{
	while (!(USART2->SR & USART_FLAG_TXE))
		;
	USART_SendData(USART2, Data);
}

/**
 * @description: 向串口2发送指定长度的字节
 * @param uint8_t * Data 数据地址; uint16_t leng 数据局长度  
 * @return: 
 */
void F407USART2_SendBytes(char *Data, uint16_t leng)
{
	uint16_t i = 0;
	for (i = 0; i < leng; i++)
	{
		F407USART2_SendByte(*(Data + i));
	}
}

/**
 * @description: 发送字符串
 * @param char *str string address
 * @return: 
 */
void F407USART2_SendString(char *str)
{
	u16 i = 0;
	while (*(str + i) != 0)
	{
		F407USART2_SendByte(*(str + i));
		i++;
	}
}
