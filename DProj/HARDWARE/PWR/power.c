#include "power.h"
#include "delay.h"
#include "iwdg.h"

/**
 * @description: Power_Ctrl_Init, ��Դ�������ų�ʼ��
 * @param 
 * @return: 
 */
void Power_Ctrl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOE, ENABLE);  // ʹ��GPIOEʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // ����
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // ��ʼ��
	
	// POWER_OUT & POWER_CAM
	GPIO_SetBits(GPIOE, GPIO_Pin_11 | GPIO_Pin_12 );
	// POWER_MP & POWER_MIC & POWER_USB & POWER_OUT3V3 & POWER_OUT5
	GPIO_ResetBits(GPIOE, GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	

	//INIO1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // ��ʼ��
	GPIO_ResetBits(GPIOE, GPIO_Pin_7);
	//OUTIO1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);  // ��ʼ��
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
	#if EN_log_print>=2
	printf("*Power_Ctrl_Init\r\n");
	#endif
	// �򿪴�������Դ
	POWER_OUT3V3=1;
	POWER_OUT5=1;
}

/**
 * @description: Cam_Crtl_Init, ����������ų�ʼ��
 * @param
 * @return: 
 */
void Cam_Crtl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //ʹ��GPIOEʱ��

	//GPIOE5,E6��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // ����
	GPIO_Init(GPIOE, &GPIO_InitStructure);  // ��ʼ��

	GPIO_ResetBits(GPIOD, GPIO_Pin_5 | GPIO_Pin_6);
}

//��ʼ��IIC
void Relay_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_GPIOC, ENABLE );	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��

	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
}

/**
 * @description: USB_Photograph
 * �����ʶ��������������
 * ��������֪ͨ������ջ�¼�񣨸�����������ã�������ʱ��0.5~1s��Ȼ������
 * @param {type} 
 * @return: 
 */
void USB_Photograph(void)
{
	USB_PHO = 1;
	printf("����...\r\n");
	delay_ms(1000);
	USB_PHO = 0;
	printf("ing .....\r\n");
}

/**
 * @description: USB_Connecting
 * ���������USB��������
 * ��������Ƿ�������״̬������ƽ����ʱ����֪����ⲿ�豸Ҫ��USB�ˣ��ⲿ�豸���USB���Ӻ���԰���������3���ͣ����ɻָ����
 * @param {type} 
 * @return: 
 */
void USB_Connecting(u8 ctrl)
{
	if (ctrl == 1)
	{
		USB_CNT = 1;
		printf("�������...\r\n");
	}
	else
	{
		USB_CNT = 0;
		printf("�ָ����...\r\n");
	}
}

// 5V USB��Դ��������
void USB_CONNECT_ON()
{
	POWER_USB = 1;
}
void USB_CONNECT_OFF()
{
	POWER_USB = 0;
}

// ģ������
void openUSB(void)
{
	printf("openUSB...\r\n");
	IWDG_Feed();
	USB_Connecting(1);  // ����֪ͨ�����������ģʽ
	//delay_ms(10);
	USB_CONNECT_ON();  // ��USB��Դ
	//delay_ms(100);
}

// ģ��ر����
void closeUSB(void)
{
	printf("closeUSB...\r\n");
	// delay_ms(10);
	USB_Connecting(0);
	// delay_ms(100);
	USB_CONNECT_OFF();
}


// ģ��򿪸���
void openReLoad(void)
{
	IWDG_Feed();
	printf("*openReLoad...\r\n");
	POWER_OUTIO1=1;
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
}

// ģ��رո���
void closeReLoad(void)
{
	printf("*closeReLoad...\r\n");
	POWER_OUTIO1=0;
	delay_ms(100);
}

// ģ���
u8 openOutputLoad(void)
{
	u8 i=0;
	IWDG_Feed();
	while(i++<3)
	{
		if(Relay_IO_IN==1)
		{
			printf("*succeed openOutputLoad...\r\n");
			return 1;
		}
		else
		{
			printf("*try openOutputLoad...cnt:%d\r\n", i);
			POWER_INIO1=1;
			delay_ms(300);
			POWER_INIO1=0;
			delay_ms(1000);
			delay_ms(1000);
		}
		
	}
	printf("*!fail openOutputLoad...\r\n");
	return 0;
}

u8 closeOutputload(void)
{
	u8 i=0;
	IWDG_Feed();
	while(i++<3)
	{
		if(Relay_IO_IN==0)
		{
			printf("*succeed closeOutputload...\r\n");
			return 1;
		}
		else
		{
			printf("*try closeOutputload...cnt:%d\r\n", i);
			POWER_INIO1=1;
			delay_ms(300);
			POWER_INIO1=0;
			delay_ms(1000);
			delay_ms(1000);
		}	
	}
	printf("*!fail closeOutputload...\r\n");
	return 0;
}


