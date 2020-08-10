#include "sys.h"
#include "usart2.h"	  



#if (F407USART2_RECEIVE_BUFF_ENABLE == 1)
uint8_t F407USART2_buff[F407USART2_RECEIVE_BUFF_SIZE];  //UART3������
volatile uint16_t F407USART2_buffHead = 0; // ͷָ��
volatile uint16_t F407USART2_buffEnd = 0;  // βָ��
volatile uint8_t F407USART2_buffOverFlag = 0;  // ���Ǳ�־
///////////////////////////д��
/**
 * @description: ��F407USART2_buffд��һ���ֽ�
 * @param uint8_t data ��д���ֽ�
 * @return: 0 д��ʧ�ܣ�1 д��ɹ�
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
 * @description: ��F407USART2_buffд��ָ�������ַ�
 * @param uint8_t *data ��д���ַ���ַ, uint16_t length ��д���ַ�����
 * @return:  �ɹ���F407USART2_buffд���ַ�����
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

///////////////////////////��ȡ
/**
 * @description: ��F407USART2_buff�ж�ȡһ���ֽڵ�����
 * @param uint8_t *data ���ݵ�ַ
 * @return: 1 �ɹ���0 ʧ��
 * @note ѭ������
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
 * @description: ��F407USART2_buff�ж�ȡָ�����ȵ����ݵ�ָ����ַ
 * @param uint8_t *data ָ����ַ, uint16_t length ָ������
 * @return: uint16_t ��ȡ�ĳ���
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
 * @description: �񻺴�������
 * @param {type} none
 * @return: uint16_t ����������
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

	//������󣬱��봦��,����ᵼ������
	if (0x0008 & USART2->SR)
	{
		data = USART2->DR;
		step = 0;
	}
	else if (0x0020 & USART2->SR)
	{
		//�����ݻ��Զ�����жϱ�־λ
		data = USART2->DR;
		F407USART2_buffWrite(data);
	}
}

//��ʼ��IO ����3
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������ 
void USART2_init(u32 bound)
{  	
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 
	USART_DeInit(USART2);  //��λ����2
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2��GPIOA3��ʼ��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��GPIOA2����GPIOA3
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3����ΪUSART2	  
	
	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������3
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�  
		
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	
 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	//USART2_RX_STA=0;				//���� 
}

/**
 * @description: �򴮿�2����һ���ֽ�����
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
 * @description: �򴮿�2����ָ�����ȵ��ֽ�
 * @param uint8_t * Data ���ݵ�ַ; uint16_t leng ���ݾֳ���  
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
 * @description: �����ַ���
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
