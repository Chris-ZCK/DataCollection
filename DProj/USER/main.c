#include "sys.h"
#include "includes.h"
//UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
//����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
//���ȼ�0���жϷ������������� OS_IntQTask()
//���ȼ�1��ʱ�ӽ������� OS_TickTask()
//���ȼ�2����ʱ���� OS_TmrTask()
//���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
//���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()

/* ��ʼ���� 
*/
#define START_TASK_PRIO									3
//�����ջ��С	
#define START_STK_SIZE 									128
static OS_TCB 											StartTaskTCB;	
static CPU_STK 											START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

/* ϵͳ������ */
#define MAIN_TASK_PRIO									6
//�����ջ��С
#define CPU_STK_MAIN_SIZE								1000
//ʱ��Ƭ����
#define MAIN_TICK_LEN									0
static  OS_TCB											MainTaskTCB;
static	CPU_STK											MainTaskStk[CPU_STK_MAIN_SIZE];
static void  MainTask(void* p_arg);


/* ����MQTTָ�� */
#define MQTTReceive_PRIO								7
//�����ջ��С32bit
#define CPU_STK_MQTTReceive_SIZE						1000
//ʱ��Ƭ����
#define MQTTReceive_TICK_LEN							0
static  OS_TCB											MQTTReceivetcb;
static	CPU_STK											MQTTReceivestk[CPU_STK_MQTTReceive_SIZE];
static void  MQTTReceiveTask(void* p_arg);

/* ����MQTTָ�� */
#define LTEModule_PRIO									8
//�����ջ��С32bit
#define CPU_STK_LTEModule_SIZE							1000
//ʱ��Ƭ����
#define LTEModule_TICK_LEN								0
static  OS_TCB											LTEModuletcb;
static	CPU_STK											LTEModulestk[CPU_STK_LTEModule_SIZE];
static void  LTEModuleTask(void* p_arg);

/* Watch task */
#define SysWatch_PRIO									10
#define CPU_STK_SysWatch_SIZE							2000
#define SysWatch_TICK_LEN								0
static  OS_TCB											SysWatchtcb;
static	CPU_STK											SysWatchstk[CPU_STK_SysWatch_SIZE];
static void  SysWatchTask(void* p_arg);

#define  SystemDatasBroadcast_PRIO                      11 // ͳ���������ȼ���ͣ���������12���Ѿ�����������������ȼ���
#define  SystemDatasBroadcast_STK_SIZE                  200 // ����Ķ�ջ��С����ͳ��һ�㹻�ˣ�ͳ�ƽ�������󲻹��ټ�..
#define  SystemDatasBroadcast_LED_TICK_LEN              0
static   OS_TCB                                         SystemDatasBroadcast_TCB;		                // ����ͳ�������TCB
static   CPU_STK                                        SystemDatasBroadcast_STK [SystemDatasBroadcast_STK_SIZE];// ����������Ϊ����ջ������ʹ��
static void  SystemDatasBroadcast (void *p_arg);


/**
 * @description: ϵͳ��λ
 * @param {type} 
 * @return {type} 
 */
void SoftReset(void)
{
    __set_FAULTMASK(1); // �ر������ж�
    NVIC_SystemReset(); // ��λ
}


// bit0:��ʾ����������SD��д������
// bit1:��ʾ��������SD����������
// bit2:SD��д���ݴ����־λ
// bit3:SD�������ݴ����־λ
// bit4:1,����ping����
// bit5:����.
// bit6:����.
// bit7:����.

struct cycle_package cycle;
struct flash_package eerom;
#define FLASH_WRITE_MODE 0
/////////////////systerm runing state//////////
vu16 watchdog_f;
vu16 function_f;
vu16 function_f2;
vu16 ec25_on_flag;
vu16 m8266_on_flag;
vu16 m8266_work_state;  // WiFi�������ݵĿ���
vu16 key_on_flag;
vu16 led_on_flag;
////////////////////////////////////////////

/////////////////systerm parameters//////////
vu16 sensor_frequency = CYCLE_TIME;
vu16 camera_frequency = TASK_T_P_CNT;
vu16 upload_frequency = TASK_S_D_CNT;
vu16 transfer_photo_frequency = TASK_S_P_CNT;
vu16 voltage_fuse_threshold = TD_B_V_VAL;
vu16 current_fuse_threshold = TD_C_C_VAL;
vu16 hardwork_min = TD_C_H_S;
vu16 hardwork_max = TD_C_H_E;
vu16 max_work_length = MAX_RUN_TIME;
vu16 wifi_work_on_flag = WIFI_DEFAULT_WORK;
////////////////////////////////////////////

u8 network_init(void)
{
	u8 i=0;
	u8 retry=0;
	
	printf("[INFO]WIFI_CONNECTION_TYPE = %d(0-UDP, 1-Client,2-Server)\r\n",TEST_CONNECTION_TYPE); //��������
	printf("[INFO]WIFI_LOCAL_PORT      = %d(0:updated random port)\r\n",TEST_LOCAL_PORT);			//���ض˿ں�
	printf("[INFO]WIFI_REMOTE_IP_ADDR  = %s\r\n",TEST_REMOTE_IP_ADDR);//Ŀ��IP
	printf("[INFO]WIFI_REMOTE_PORT     = %d\r\n",TEST_REMOTE_PORT);	//Ŀ��IP�˿ں�
	
	POWER_D = 1;
	/*step1;initial to HOST*/
	M8266HostIf_Init();		// Initialise the MCU host interface for M8266WIFI module
                      	// Includeing GPIOs for nCS/nRESET, SPI, UART if any
	
	/*step2;initial to SPI*/
	while(M8266WIFI_Module_Init_Via_SPI() == 0)
	{
		IWDG_Feed();
		retry++;
		printf("[INFO]M8266WIFI_Module_Init_Via_SPI failed, retry %d times\r\n",retry);
		for(i=0;i<3;i++)
		{
			M8266WIFI_Module_delay_ms(250);
			M8266WIFI_Module_delay_ms(250);
		}
		if(retry>10)
		{
			printf("[WARNING]Fail M8266WIFI_Module_Init_Via_SPI\r\n");
			m8266_work_state =0; // �رշ������ݵ�ָ��
			return M8266_ERROR;
		}
	}
	printf("[LOG]Succeed M8266WIFI_Module_Init_Via_SPI\r\n");
	return M8266_SUCCESS; // �ɹ�
}


/**
 * @description: ϵͳ��ʼ��
 * @param {type} 
 * @return {type}
  */
void system_init(void)
	
{
	//test value
	//u32 now_timea=0;
	
	u8 res;
	u8  m_buf[100];
	u16 m_value[10];
	#if SLEEP_MODE
	u32 now_time;
	int time_delta;
	#endif
	// global various
	// watchdog_f=0;
	function_f = 0;  // ����ִ�б�־����
	function_f2 = 0;
	ec25_on_flag = 0;  // ����ģʽ
	key_on_flag = 0; // ����ִ�б�־����
	led_on_flag = 0;


	// systerm initial
	delay_init(168);  	// ʱ�ӳ�ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �жϷ�������
		
	#if DEBUG_MODE
	uart_init(115200); 	// ���ڳ�ʼ��
	#else
	uart_init(921600); 	// ���ڳ�ʼ��
	#endif
	printf("\r\n\r\n\r\n>>>>>>>>>>>>>>SYSTERM START>>>>>>>>>>>>>>\r\n");
	
	KEY_Init();	  		// key init
	LED_Init();   		// LED init
	
	IWDG_Init(IWDG_Prescaler_256,4000);  // 4,000*256/32,000=32s
	
	My_RTC_Init();  //��ʼ��RTC
	
	calendar_get_time(&calendar);
	calendar_get_date(&calendar);
	printf("[TIME]%d/%d/%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
    
	// local_time_cnt = calendar.sec;  // ���ڿ��Ź�ͳ��
	
	#if ANAY_TASK_ON
	// updata sys parameters
	// ��SD��
	// ��ȡ����
	// ��������buf,len
	// flash
	mf_config_data_read_flash(m_buf);
	res = analyze_config_para((char *)m_buf,m_value);
	if(res==0) // ������
	{
		sensor_frequency = m_value[0];
		camera_frequency = m_value[1]/sensor_frequency;
		upload_frequency = m_value[2]/sensor_frequency;
		transfer_photo_frequency = m_value[3]/sensor_frequency;
		voltage_fuse_threshold = m_value[4];
		current_fuse_threshold = m_value[5];
		hardwork_min = m_value[6];
		hardwork_max = m_value[7];
		max_work_length = m_value[8];
		wifi_work_on_flag = m_value[9];
		printf("[LOG]analyze_config_para update finish^^^^^\r\n");
	}
	else
	{
		sensor_frequency = CYCLE_TIME;
		camera_frequency = TASK_T_P_CNT / sensor_frequency;
		upload_frequency = TASK_S_D_CNT / sensor_frequency;
		transfer_photo_frequency = TASK_S_P_CNT/sensor_frequency;
		voltage_fuse_threshold = TD_B_V_VAL;
		current_fuse_threshold = TD_C_C_VAL;
		hardwork_min = TD_C_H_S;
		hardwork_max = TD_C_H_E;
		max_work_length = MAX_RUN_TIME;
		wifi_work_on_flag = WIFI_DEFAULT_WORK;
		printf("[LOG]None Stored parameters, use default values\r\n");
	}
	#endif
	
	// sleep mode
	#if FLASH_WRITE_MODE
	cycle.time_stamp=get_time_cnt();
	cycle.task_cnt =0;
	cycle.function=0;
	cycle.picture_id=2717;
	cycle.watch_cnt=30;
	STMFLASH_Write(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);	// ����ʼ������д��Ĵ���
	while(1);
	#else
	//printf("[INFO]cycle data:%d,%d,%d,%d,%x\r\n",cycle.time_stamp,cycle.picture_id,cycle.task_cnt,cycle.watch_cnt,cycle.function);
	#endif
	
	// �������߻���
	#if SLEEP_MODE
	key_scan_fun();
	res = KEY_Scan(20);
	switch(res)
	{
		case KEY2_PRES:
			key_on_flag = 1;
			printf("[INST]force to execute the task!!!\r\n");
			break;
		case KEY3_PRES:
			function_f2=1;
			ec25_on_flag=1;
			printf("[INST]force not to sleep!!!\r\n");
			break;
		default:
			printf("[INFO]normal start systerm\r\n");
			break;
	}
	// ��Ч����
	STMFLASH_Read(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);
	printf("[INFO]STMFLASH_Read:\r\n");
	printf("\t*time_stamp:%d\r\n\t*task_cnt:%d\r\n",cycle.time_stamp,cycle.task_cnt);
	// ��ȡʱ��
	now_time = get_time_cnt();
	time_delta = now_time - cycle.time_stamp;  // ����ʱ>0,����now_time+3600- cycle.time_stamp>0 �������쳣������ʱ���ʱ��
	printf("[INFO]count down|next statr time:{(T:%d) %d}\r\n",sensor_frequency,sensor_frequency-time_delta);
	if(!key_on_flag && !function_f2)  // ���� 
	{
		if(time_delta <0)
			time_delta+=3600;
		if(time_delta> sensor_frequency || time_delta<0)  // ��������,����С������жϣ�������˴���
		{
			cycle.time_stamp=now_time;  // ����ʱ���
			cycle.task_cnt ++;		
			STMFLASH_Write(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);
			printf("[INFO]STMFLASH_Write#time_stamp:%d,task_cnt:%d\r\n",cycle.time_stamp,cycle.task_cnt);
		}
		else 
		{
			printf("[LOG]watchdog sleep\r\n");
			if((sensor_frequency-time_delta)>STANDBY_TIME)
				Sys_Enter_Standby(STANDBY_TIME);
			else
				Sys_Enter_Standby(sensor_frequency-time_delta);
		}
	}
	else
	{
		cycle.time_stamp=now_time;  // ����ʱ���
		cycle.task_cnt ++;		
		STMFLASH_Write(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);
		printf("[INFO]STMFLASH_Write|time_stamp:%d,task_cnt:%d\r\n",cycle.time_stamp,cycle.task_cnt);
		printf("[INFO]force to execute the task,can't sleep!!!\r\n");
	}
	#endif

	// SD����ʼ��
    if (!SD_Init())
	{
		printf("[LOG]SD_Init ok\r\n"); //�ж�SD���Ƿ����
		// �����ļ���
	}
	else
	{
        printf("[ERROR]SD_Init failed!!!!!!!!!!\r\n");
	}
	mymem_init(SRAMIN);      // ��ʼ���ڲ��ڴ��
	exfuns_init();           // Ϊfatfs��ر��������ڴ�
	f_mount(fs[0], "0:", 1); // ����SD��
	
	// mqtt���
	rng_Init();			// �������������ʼ��
	mqtt_UID_set();     // ����Ψһid
	InitQueue(&Q_stage);  	// ��ʼ������ 
	InitQueue(&Q_resent);  	// ��ʼ������ 	
	
	#if EN_LOG_PRINT > 2
	mf_scan_files((u8*)"0:");
	mf_check_dir((u8*)"0:INBOXWIFI");
	mf_check_dir((u8*)"0:INBOX");
	mf_check_dir((u8*)"0:ARCH");
	#endif

	// ��SD����־��¼����
	#if EN_log_sd
	mf_log_init();			 //��ʼ����־
	#endif
	
	#if USB_MODE
	usbapp_init();
	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);
	delay_ms(1000);
	f_mount(fs[1], "1:", 1);  // ����U��
	#endif	
	
 	printf("[INFO]--------sys value--------\r\n");
	printf("[INFO]sensor_frequency        :%d s\r\n",sensor_frequency);
	printf("[INFO]camera_frequency        :%d s\r\n",camera_frequency*sensor_frequency);
	printf("[INFO]upload_frequency        :%d s\r\n",upload_frequency*sensor_frequency);
	printf("[INFO]transfer_photo_frequency:%d s\r\n",transfer_photo_frequency*sensor_frequency);
	printf("[INFO]voltage_fuse_threshold  :%d mV\r\n",voltage_fuse_threshold);
	printf("[INFO]current_fuse_threshold  :%d mA\r\n",current_fuse_threshold);
	printf("[INFO]hardwork_min            :%d H\r\n",hardwork_min);
	printf("[INFO]hardwork_max            :%d H\r\n",hardwork_max);
	printf("[INFO]max_work_length         :%d s\r\n",max_work_length);
	printf("[INFO]wifi_work_on_flag       :%d s\r\n",wifi_work_on_flag);
	printf("[INFO]-------------------------\r\n");
	
	Power_Ctrl_Init(); // ��Դ��ʼ��	
	
	#if SENSOR_MODE
	SHT2x_Init();  			// SHT20��ʼ��
	max44009_initialize();  // MAX44009��ʼ��
	#if SESOR_MS5611_ON
	MS5611_Init();  		// MS5611��ʼ��
	#endif
	USART2_init(9600); 		// ������ݶ˿ڳ�ʼ��
	Cam_Crtl_Init();   		// ����������ų�ʼ��
	printf("[LOG]SENSOR init\r\n");
	#endif

	#if ANAY_TASK_ON
	// �������������Ҫ���¼ӽ��������µĹ�������
	// �������
	printf("[LOG]Task analysis...........\r\n");
	printf("[INFO]Task count:%d\r\n", cycle.task_cnt);
	printf("[INFO]Task list:");
	function_f|=(0x01);  // ��ȡ����
	printf("\tget data\r\n");

	function_f|=(0x40);  // ��WiFi
	printf("\topen wifi and send data\r\n");
	if(cycle.task_cnt%camera_frequency==0 || key_on_flag) 
	{
		function_f|=(0x02);  // ����
		printf("\ttake photo\r\n");		
	}
	if(cycle.task_cnt%transfer_photo_frequency==0 || key_on_flag) 
	{
		function_f|=(0x04);  // ת����Ƭ	
		printf("\tstore photo\r\n");
	}
	
	if(cycle.task_cnt%upload_frequency==0 || key_on_flag)  // ��������
	{
		
		printf("\tttry to send data by 4g\r\n");
		
		#if QUEERY_BATTERY_ON
		{
			u8 i=0;
			while(i++<5 && battery_data_get()==0)
			{
				printf("[WARNING]try to get battery data, cnt:{(Max:5) %d}\r\n",i);
			}
			if((battery.charge_current)*10 >=current_fuse_threshold  || (battery.total_voltage)*2 >=voltage_fuse_threshold)
			{
				ec25_on_flag=1;
				printf("[INFO]battery ok|current:%d|total_voltage:%d|send data\r\n",(int)battery.charge_current*10,(int)battery.total_voltage*2);
			}
			else
			{
				printf("[WARNING]battery error,charge_current:{(T:%d)>%d},total_voltage:{(T:%d)>%d}\r\n",\
				(int)current_fuse_threshold,(int)battery.charge_current*10,(int)voltage_fuse_threshold,(int)battery.total_voltage*2);
			}
		}
		#else
		{
			ec25_on_flag=1;
			printf("[INFO][DEBUG]FORCE to open 4G\r\n");
		}
		#endif
		calendar_get_time(&calendar);
		if(calendar.hour>=hardwork_min && calendar.hour<=hardwork_max)
		{
			ec25_on_flag=1;
			printf("[INFO]time(%d->%d) ok,calendar.hour:%d send data\r\n",hardwork_min,hardwork_max,calendar.hour);
		}
		else
		{
			printf("[WARNING]time(%d->%d) error,calendar.hour:%d\r\n",hardwork_min,hardwork_max,calendar.hour);
		}
		if(key_on_flag)
		{
			ec25_on_flag=1;
			printf("[INFO]force to execute the task|ec25_on_flag=1");
		}
		if(ec25_on_flag)
		{
			function_f|=(0x10);  // ��������	
			printf("[INFO]send data\r\n");
			function_f|=(0x20);  // ����ͼƬ
			printf("[INFO]end photo\r\n");	
		}			
	}
	printf("[INFO]function=%x\r\n",function_f);
	//printf("-------------------------\r\n\r\n");
	#endif
	
	// WiFi����
	if(wifi_work_on_flag ==1)
	{
		u8 res=0;
		IWDG_Feed();//ι��
		res = network_init();
		if(res == M8266_SUCCESS)
		{
			m8266_work_state = 1;
			printf("[LOG]Success access to WiFi\r\n");
		}
		else
		{
			m8266_work_state = 0;
			printf("[LOG]Fail access to WiFi\r\n");
			
			if(ec25_on_flag ==0)
			{
				POWER_D = 0;
				printf("[INST]Close 4G/WiFi power\r\n");
			}
			else
			{
				POWER_D = 1;
				printf("[INST]Open 4G/WiFi power\r\n");
			}
		}
	}
	
	IWDG_Feed();//ι��
//	while(1)
//	{
//		
//		now_timea++;
//		if(now_timea%10==0)
//		{
//			IWDG_Feed();//ι��
//			printf("delta=%d\r\n",now_timea);
//		}
//		if(now_timea%20==0)
//		{
//			char buf[50];
//			IWDG_Feed();			
//			WiFiSendPic("0:pic1.jpg",now_timea); // ����ͼƬ 					
//		}
//		delay_ms(1000);
//	}
}


/**
 * @description: ���������Ƿ�����/���Է���������
 * @param {type} 
 * buf ���������ַ
 * val ��������������
 * @return {type} 
 * 0      �����ɹ�
 * 100    ����Ҫ����
 * others �����쳣
 * ���ӣ�600|10800|3600|3600|12500|800|8|15|1200
 *  �ڶ��� 600|10800|3600|3600|12500|800|8|15|1200|0
 */
u8 analyze_config_para(char *buf, u16 * val)
{
	u8 res;
	// �м����
	u8 offset;
	//static u16 val[9];
	
	// �ж��Ƿ�Ϊ���±���
	if(buf[0]=='0')
	{
		//config_flag = 1;
		res=100;
		printf("[INFO]analyze_config_para|the data is latest, no need to updata!\r\n");
		goto an_end;
	}
	// �������Ʋ���
	printf("[LOG]analyze_config_para .......\r\n");
	offset=0;
	val[0] = stringtoNum(buf);
	printf("%02dsensor_frequency        :%d\r\n",offset,val[0]);
	if(val[0]<60)
	{
		printf("[WARNING]analyze_config_para error:val[0]=%d\r\n",val[0]);
		res=1;
		goto an_end;
	}	
	offset += locate_character(buf+offset, '|');
	val[1] = stringtoNum(buf+offset);
	printf("%02dcamera_frequency        :%d\r\n",offset,val[1]);
	
	if(val[1]<val[0])
	{
		printf("[WARNING]analyze_config_para error:val[1]=%d\r\n",val[1]);
		res=2;
		goto an_end;
	}
	offset += locate_character(buf+offset, '|');
	val[2] = stringtoNum(buf+offset);
	printf("%02dupload_frequency        :%d\r\n",offset,val[2]);
	
	if(val[2]<val[0])
	{
		printf("[WARNING]analyze_config_para error:val[2]=%d\r\n",val[2]);
		res=3;
		goto an_end;
	}
	offset += locate_character(buf+offset, '|');
	val[3] = stringtoNum(buf+offset);
	
	printf("%02dtransfer_photo_frequency:%d\r\n",offset,val[3]);
	if(val[3]<val[0])
	{
		printf("[WARNING]analyze_config_para error:val[3]=%d\r\n",val[3]);
		res=4;
		goto an_end;
	}
	offset += locate_character(buf+offset, '|');
	val[4] = stringtoNum(buf+offset);
	
	printf("%02dvoltage_fuse_threshold  :%d\r\n",offset,val[4]);
	offset += locate_character(buf+offset, '|');
	val[5] = stringtoNum(buf+offset);
	
	printf("%02dcurrent_fuse_threshold  :%d\r\n",offset,val[5]);
	offset += locate_character(buf+offset, '|');
	val[6] = stringtoNum(buf+offset);
	
	printf("%02dhardwork_min            :%d\r\n",offset,val[6]);
	
	if(val[6]>24)
	{
		printf("[WARNING]analyze_config_para error:val[6]=%d\r\n",val[6]);
		res=7;
		goto an_end;
	}
	offset += locate_character(buf+offset, '|');
	val[7] = stringtoNum(buf+offset);
	
	printf("%02dhardwork_max            :%d\r\n",offset,val[7]);
	if(val[7]<=val[6])
	{
		printf("[WARNING]analyze_config_para error:val[7]=%d\r\n",val[7]);
		res=8;
		goto an_end;
	}
	
	offset += locate_character(buf+offset, '|');
	val[8] = stringtoNum(buf+offset);	
	printf("%02dmax_work_length         :%d\r\n",offset,val[8]);
	if(val[8]>2400 || val[8]<180)  // ���40min,��С3min
	{
		printf("[WARNING]analyze_config_para error:val[8]=%d\r\n",val[8]);
		res=9;
		goto an_end;
	}
	
	offset += locate_character(buf+offset, '|');
	val[9] = stringtoNum(buf+offset);	
	if(val[9]>1)
	{
		printf("[WARNING]analyze_config_para error:val[9]=%d\r\n",val[9]);
	}
	printf("%02dwifi work state         :%d\r\n",offset,val[9]);
	res=0;
	an_end:
	return res;
}


/**
 * @description: ������
 * ��������
 * @param {type} 
 * @return {type} 
 */
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	system_init();		//ϵͳ��ʼ�� 
	OSInit(&err);		//��ʼ��UCOSIII
	OS_CRITICAL_ENTER();//�����ٽ���			 
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	// �˳��ٽ���	 
	OSStart(&err);      // ����UCOSIII
}


/**
 * @description:��ʼ������ 
 * @param {type} 
 * @return {type} 
 */
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
	#if OS_CFG_STAT_TASK_EN > 0u
   	OSStatTaskCPUUsageInit(&err);  	// ͳ������                
	#endif
	
	#ifdef CPU_CFG_INT_DIS_MEAS_EN		// ���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
	#endif
	
	#if	OS_CFG_SCHED_ROUND_ROBIN_EN  	// ��ʹ��ʱ��Ƭ��ת��ʱ��
	//ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
	#endif		
	
	OS_CRITICAL_ENTER();	//�����ٽ���

    // stackMonitoring
    OSTaskCreate((OS_TCB *)&SystemDatasBroadcast_TCB,
                 (CPU_CHAR *)"SystemDatasBroadcast",
                 (OS_TASK_PTR)SystemDatasBroadcast,
                 (void *)0,
                 (OS_PRIO)SystemDatasBroadcast_PRIO,
                 (CPU_STK *)&SystemDatasBroadcast_STK[0],
                 (CPU_STK_SIZE)SystemDatasBroadcast_STK_SIZE / 10, /*ջ����ٽ�ֵ������ջ��С��90%��*/
                 (CPU_STK_SIZE)SystemDatasBroadcast_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);
	if(ec25_on_flag==1)
	{
		OSTaskCreate((OS_TCB *)&MQTTReceivetcb,
					(CPU_CHAR *)"MQTTReceive",
					(OS_TASK_PTR)MQTTReceiveTask,
					(void *)0,
					(OS_PRIO)MQTTReceive_PRIO,
					(CPU_STK *)&MQTTReceivestk[0],
					(CPU_STK_SIZE)CPU_STK_MQTTReceive_SIZE / 10,
					(CPU_STK_SIZE)CPU_STK_MQTTReceive_SIZE,
					(OS_MSG_QTY)0,
					(OS_TICK)MQTTReceive_TICK_LEN,
					(void *)0,
					(OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
					(OS_ERR *)&err);

		OSTaskCreate((OS_TCB *)&LTEModuletcb,
					(CPU_CHAR *)"LTEModule",
					(OS_TASK_PTR)LTEModuleTask,
					(void *)0,
					(OS_PRIO)LTEModule_PRIO,
					(CPU_STK *)&LTEModulestk[0],
					(CPU_STK_SIZE)CPU_STK_LTEModule_SIZE / 10,
					(CPU_STK_SIZE)CPU_STK_LTEModule_SIZE,
					(OS_MSG_QTY)0,
					(OS_TICK)LTEModule_TICK_LEN,
					(void *)0,
					(OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
					(OS_ERR *)&err);
	}

    // watchTask
    OSTaskCreate((OS_TCB *)&SysWatchtcb,
                 (CPU_CHAR *)"SysWatch",
                 (OS_TASK_PTR)SysWatchTask,
                 (void *)0,
                 (OS_PRIO)SysWatch_PRIO,
                 (CPU_STK *)&SysWatchstk[0],
                 (CPU_STK_SIZE)CPU_STK_SysWatch_SIZE / 10,
                 (CPU_STK_SIZE)CPU_STK_SysWatch_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)SysWatch_TICK_LEN,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);
				 
    OSTaskCreate((OS_TCB *)&MainTaskTCB,                              // �������ջ�Ŀ�ʼ��ַ
                 (CPU_CHAR *)"MainTask",                              // �����������
                 (OS_TASK_PTR)MainTask,                               // ָ����������ָ��
                 (void *)0,                                           // ָ�룬��һ��ִ������ʱ���ݸ�������ʵ���ָ�����*p_arg
                 (OS_PRIO)MAIN_TASK_PRIO,                             // ���ȼ�����	 ����ֵԽС���ȼ�Խ��
                 (CPU_STK *)&MainTaskStk[0],                          // �����ջ�Ļ���ַ������ַͨ���Ƿ����������Ķ�ջ������ڴ�λ��
                 (CPU_STK_SIZE)CPU_STK_MAIN_SIZE / 10,                // ���߸������ǵ�ַ��ˮӡ�� ������ջ������ָ��λ��ʱ�Ͳ�������������
                 (CPU_STK_SIZE)CPU_STK_MAIN_SIZE,                     // ����Ķ�ջ��С
                 (OS_MSG_QTY)0,                                       //
                 (OS_TICK)MAIN_TICK_LEN,                              // ��������ӵ�ж��ٸ�ʱ��Ƭ��������ʱ��Ƭ��ѯ��������ʱ��Ч
                 (void *)0,                                           //
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //
                 (OS_ERR *)&err);      
				 
	OS_CRITICAL_EXIT();			// �˳��ٽ���
	OSTaskDel((OS_TCB*)0,&err);	// ɾ��start_task��������
}


static u8 local_memdevflag=0;  // 
/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
void  SysWatchTask(void *pdata)
{
	OS_ERR err;
	u16 t;
	printf("[TASK]SysWatchTask run\r\n");
	delay_ms(500);
	while(1)
	{
		t++;
		#if USB_MODE
		// USB���ش򿪺󣬷�ִ��������USBɨ�����
		if(usbConnectSwitchGet())
		{
			if((t%60)==0)
			{
				if(usbx.hdevclass==1)
				{
					if(local_memdevflag==0)
					{
						fs[1]->drv=2;  			// ��ʱ��ΪҲ���Բ���,����Ϊ������������
						f_mount(fs[1],"1:",1); 	// ���¹���U��
						usbapp_user_app();

						usbConnectStateSet(1);						
						local_memdevflag=1;
					}  
				}
				else 
					local_memdevflag=0; // U�̱��γ���
			}
			
			while((usbx.bDeviceState&0XC0)==0X40) // USB�豸������,���ǻ�û���ӳɹ�,�Ͳ�ѯ.
			{
				usbapp_pulling();  // ��ѯ����USB����
				delay_ms(1);  // ������HID��ô��...,U�̱Ƚ���
				#if (EN_LOG_PRINT >= 2)
				printf(".");
				#endif // EN_LOG_PRINT
			}
			usbapp_pulling();  // ���USB
			#if (EN_LOG_PRINT >= 2)
			printf("\\");
			#endif // EN_LOG_PRINT
		}
		#endif
		UART_TCPbuff_Run(F407USART3_buffRead);  // ѭ����ȡU3�л�������
		//key_scan_fun();
		OSTimeDly(5,OS_OPT_TIME_DLY,&err);
	}									 
}


/**
 * @description: ɨ��ת������е��ļ�
 * @param {type} 
 * @return {type} 
 */
void act_scan_camera(void)
{
	u8 res;
	// ��ʼ��USB
	// ��������أ���Ӧ��watch_task����ʼɨ��USB
	LED_YELLOW_ON();
	printf("[INST]act:act_scan_camera...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	LED_YELLOW_OFF();
	local_memdevflag=0;
	usbConnectSwitchSet(1);  
	usbapp_mode_set();  // ��������USBģʽ����λUSB
	usbConnectStateSet(0);  // ��ձ�־λ
	openUSB();  // �������USB�ɵ�Դ
	// �ȴ����ӳɹ�
	IWDG_Feed();
	res = waitUsbConnectFlag(10000);
	if (res == 1) // ���������
	{
		printf("[LOG]successful find usb��open camera!\r\n");
	}
	else
	{
		F407USART1_SendString("[WARNING]Fail WaitDistinguishflag...\r\n");
	}
	
	printf("[LOG]try to scan usb��open camera!\r\n");
	delay_ms(2000);  // �ȴ�����ȶ�
	//#if WIFI_TRANSFORM_ON
	if(wifi_work_on_flag)
	{
		mf_dcopy("1:DCIM/100IMAGE","0:INBOXWIFI",1,1);  // save
		mf_scan_files("0:INBOXWIFI");
	}
	else
	{
		printf("[LOG]wifi_work_on_flag=%d, WIFI no need to mf_dcopy\r\n", wifi_work_on_flag);
	}
	//#endif
	mf_dcopy("1:DCIM/100IMAGE","0:INBOX",1,0);  // don't save
	mf_scan_files("0:INBOX");
	mf_scan_files("1:DCIM/100IMAGE");
	closeUSB();  // close usb power
	IWDG_Feed();
	res = waitUsbDisonnectFlag(5000);
	if (res == 1) // �����ر����
	{
		F407USART1_SendString("[LOG]success closeUSB...\r\n");
	}
	else
	{
		F407USART1_SendString("[LOG]closeUSB..fail\r\n");
	}
	IWDG_Feed();
	usbapp_mode_stop();
	usbConnectSwitchSet(0);
	delay_ms(1000);
}


/**
 * @description: ����
 * @param {type} 
 * @return {type} 
 */
void act_take_photo(void)
{
	// LED_YELLOW_ON();
	// �ж����״̬,��������������״̬��������
	F407USART1_SendString("[INST]act:act_take_photo...\r\n");
	delay_ms(1000);
	// delay_ms(1000);
	// LED_YELLOW_OFF();
	if(usbConnectSwitchGet() == 0)
	{
		USB_Photograph();
	}
	else
	{
		printf("[WARNING]Error, Camera is connected\r\n");
	}
	delay_ms(1000);
}


/**
 * @description: ����ͼƬ
 * @param {type} 
 * @return {type} 
 */
u8 act_send_picture(void)
{
	F407USART1_SendString("[INST]act:act_send_picture...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);	
	printf("[INFO]mf_scan_files-B: \r\n");
	mf_scan_files("0:INBOX");  // ɨ���ļ���
	mf_send_pics("0:INBOX","0:ARCH",1,0);  // ����ͼƬ
	printf("[INFO]mf_scan_files-A\r\n");
	mf_scan_files("0:INBOX");  // ɨ���ļ���
	return 1;
}

u8 act_send_picture_wifi(void)
{
	F407USART1_SendString("[INST]act:act_send_picture_wifi...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);	
	printf("[INFO]mf_scan_files-B: \r\n");
	mf_scan_files("0:INBOXWIFI");  // ɨ���ļ���
	mf_send_pics("0:INBOXWIFI","0:ARCH",1,1);  // ����ͼƬ
	printf("[INFO]mf_scan_files-A\r\n");
	mf_scan_files("0:INBOXWIFI");  // ɨ���ļ���
	return 1;
}


/**
 * @description:
 * @param {type} 
 * @return {type} 0 ��ʾ�ɹ�
 */
u8 check_uart_commamd(u8*buf)
{
	//u8 command;
	u8 res=0;
	printf("[INFO]check_uart_commamd:\r\n");
	if(buf[0]=='{' && buf[2]==':' && buf[4]=='}')
	{
		switch(buf[1])
		{
			case 'C':
				switch(buf[3])	
				{
					case '0':
						function_f=0;  // ǿ������
						printf("[INFO]#$ins:force to sleep\r\n");	
						break;
					case '1':
						function_f|=(0x01);  // ��ȡ����
						printf("[INFO]#$ins:get data\r\n");
						break;
					case '2':
						function_f|=(0x02);  // ����
						printf("[INFO]#$ins:take photo\r\n");	
						break;
					case '3':
						function_f|=(0x04);  // ת����Ƭ
						printf("[INFO]#$ins:store photo\r\n");
						break;
					case '4':
						function_f|=(0x10);  // ��������	
						printf("[INFO]#$ins:send data\r\n");
						break;
					case '5':
						function_f|=(0x20);  // ����ͼƬ
						printf("[INFO]#$ins:send photo\r\n");	
						break;
					default:
						res=1;
						printf("[WARNING]!!!check_uart_commamd error parameter\r\n");	
						break;
				}
				break;
				
			case 'K':
				switch(buf[3])	
				{
					case '0':
						function_f2=0;  // ��������
						printf("[INFO]#$ins:ready to sleep\r\n");	
						break;
						
					case '1':
						function_f2=1;  // �ر�����
						printf("[INFO]#$ins:not sleep\r\n");	
						break;
					case '2':
						printf("[INFO]#$ins:Request to get parameters\r\n");
						if (mqtt_state_get() == 1)
						{
							printf("[LOG]$mysend_config-QUERRY CONFIG~~~~~\r\n");
							mysend_config("0");  // ��ȡ����
						}
						else
						{
							printf("[WARNING]!!!network error\r\n");
						}
						break;
						case '3':
						printf("[INFO]#$ins:open indicator led\r\n");
						led_on_flag=1;
						break;
					case '4':
						printf("[INFO]#$ins:close indicator led\r\n");
						led_on_flag=0;
						LED_BLUE_OFF();
						LED_YELLOW_OFF();
						LED_GREEN_OFF();
						break;
					default:
						res=2;
						printf("[WARNING]!!!check_uart_commamd error parameter\r\n");	
						break;
				}
				break;
			default:
				res=3;
				printf("[WARNING]!!!check_uart_commamd error command\r\n");	
				break;
		}
	}
	else
	{
		res=4;
		printf("[WARNING]!!!check_uart_commamd error format\r\n");	
	}
	return res;
}


void openUSB(void);
void closeUSB(void);
/**
 * @description: ��ִ������
 * �ϲ��װ���������ڵ�����������ִ������ָ��
 * @param {type} 
 * @return {type} 
 */
static void MainTask(void *p_arg) // test fun
{	
    OS_ERR err;
    //u8 res;
	printf("[TASK]MainTask run\r\n");
	delay_ms(500);  
	while (1)
    {
		#if KEY_SCAN_ON
		key_scan_fun();
		
		if(key2_down==1)
		{
			act_get_data();
			key2_down = 0;
		}
		if(key3_down==1)
		{
			act_scan_camera();
			key3_down = 0;
		}
		if(key1_down==1)
		{
			act_send_picture();
			key1_down = 0;
		}
		#endif
		#if UART_CMD_MODE
		if(USART_RX_STA &= 0x8000)  // ���ܴ��ڵ�ָ��
		{
			check_uart_commamd(USART_RX_BUF);
			USART_RX_STA = 0;
		}
		#endif
		// ����Ҫ���� ���� �����ɹ�
		if((ec25_on_flag==0) || (mqtt_state_get() == 1)) 
		{
			if(function_f&0x01 )  // ��ȡ����
			{
				IWDG_Feed();				
				act_get_data();
				function_f&=(~0x01);
				printf("[INFO]finish act_get_data , fun:%x~~~~~\r\n",function_f);
			}
			else if(function_f&0x02 )  // ����
			{
				IWDG_Feed();
				act_take_photo();
				delay_ms(1000);
				function_f&=(~0x02);
				printf("[LOG]finish act_take_photo, fun:%x\r\n",function_f);
			}
			else if(function_f&0x04 )  // ת��
			{
				IWDG_Feed();				
				act_scan_camera();
				delay_ms(1000);
				function_f&=(~0x04);
				printf("[LOG]finish act_scan_camera, fun:%x~~~~~\r\n",function_f);
			}		
			else if(function_f&0x40)	// ����WIFI����
			{	
				if(m8266_work_state==1)  
				{
					IWDG_Feed();
					// act_send_picture_wifi();
					WiFiSendPic((u8*)"0:pic1.jpg",111); // test send one picture.	
					
					delay_ms(1000);
					function_f&=(~0x40); 
					printf("[LOG]finish act_send_picture_wifi, fun:%x~~~~~\r\n",function_f);
				}
				else
				{	
					function_f&=(~0x40); 
					printf("[LOG]wifi anomaly,skip act_send_picture_wifi, fun:%x~~~~~\r\n",function_f);
				}
			}
		}
		// �����ɹ�����ִ��
		if(mqtt_state_get() == 1 && !(function_f&0x0F))  // ��ִ�л���������ִ�з�������
		{	
			if(function_f&0x10)  // ���ʹ�����
			{
				IWDG_Feed();
				act_send_data();
				printf("[LOG]finish act_send_data, fun:%x~~~~~\r\n",function_f);
				delay_ms(1000);
				mf_send_log();
				delay_ms(1000);
				function_f&=(~0x10); 
				printf("[LOG]finish act_send_log, fun:%x~~~~~\r\n",function_f);
			}
			
			else if(function_f&0x20)  // ����ͼƬ
			{
				IWDG_Feed();
				act_send_picture();
				delay_ms(1000);
				function_f&=(~0x20); 
				printf("[LOG]finish act_send_picture, fun:%x~~~~~\r\n",function_f);
			}
		}
        OSTimeDly(10, OS_OPT_TIME_DLY, &err);
    }
}


/**
 * @description: ��ȡ����
 * У�� ����
 * instant:"600|10800|3600|3600|12500|800|8|15|1200"
 * @param {type} 
 * @return {type} 
 */
u8 check_config(u8 *load, u16 len)
{
	u8 res=0;
	// �м����
	u16 crc_cal = 0;
	u16 crc_rcv = 0;
	u16 msg_len=0;
	u16 value1[9];
	printf("[LOG]Check server parameters:\r\n");
	msg_len = (load[6] << 8) + load[7];
	if(msg_len < len)
	{
		crc_cal = ModBusCRC((uint8_t *)load+10, msg_len);
		crc_rcv = (load[8]<<8) + load[9];				
		if(crc_cal == crc_rcv)
		{
			// ��ò����������
			res = analyze_config_para((char*)load+10, value1);

			if(res==0)
			{
				// �洢������Ϣ���ַ�����ʽ
				// ��û�н���updata,�´�������Ч
				mf_config_data_write_flash(load+10);
				printf("[INFO]mf_config_data_write data={%s}\r\n",load+10);
				res=0;
			}
			else if(res==100)
			{
				printf("[INFO]config_flag|the config is latest, don't need to upgrade\r\n");
			}
			else
			{
				printf("[WARNING]check_config|analyze_config_para error!\r\n");
			}
		}
		else
		{
			printf("[WARNING]check_config|crc_cal error!crc_cal=%d,crc_rcv=%d\r\n",crc_cal,crc_rcv);
			res=101;
		}
	}
	else
	{
		printf("[WARNING]check_config|msg_len error,msg_len=%d,len=%d\r\n",msg_len,len);
		res=102;
	}	
	return res;
}


/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
u8 check_sever_config(u8 *load, u16 len)
{
	u8 res;
	// �м����
	u16 crc_cal = 0;
	u16 crc_rcv = 0;
	u16 msg_len=0;
	
	msg_len = (load[6] << 8) + load[7];
	//#test{
	len=msg_len+1;
	//#test}
	if(msg_len < len)
	{
		crc_cal = ModBusCRC((uint8_t *)load+10, msg_len);
		crc_rcv = (load[8]<<8) + load[9];				
		//#test{
		crc_cal = crc_rcv;
		//#test}
		if(crc_cal == crc_rcv)
		{
			res=check_uart_commamd(load+10);
			if(res==0)
				printf("[INFO]check_sever_config|succeed receive command\r\n");
			else
				printf("[WARNING]check_sever_config|parameter error\r\n");
		}
		else
		{
			printf("[WARNING]check_sever_config|crc_cal error!crc_cal=%d,crc_rcv=%d\r\n",crc_cal,crc_rcv);
			res=101;
		}
	}
	else
	{
		printf("[WARNING]check_sever_config|msg_len error,msg_len=%d,len=%d\r\n",msg_len,len);
		res=102;
	}
	return res;
}

//	InitQueue(&Q_stage);  // ��ʼ������ 
//	InitQueue(&Q_resent);  // ��ʼ������
/**
 * @description: ����������������Ϣ
 * a5
 * 		96 ������У����
 * 		90 
 * @param {type} 
 * @return {type} 
 */
void check_response(u8* load, int len)
{
	u8 res;
	
	u8 i,cnt;
	u32 uid=0;
	QElemType elem;
	if(load[0]==0xa5)
	{
		if(load[1]==0x96)  // ������У����
		{
			uid = (load[2]<<24) +  (load[3]<<16) + (load[4]<<8) +  load[5];
			// printf(">>>>>>>>>>>>>>>>>>>>>>>>R*UID:%0X\r\n",uid);
			printf(">>>>>RUID:[%3d,%3d,%3d],Q-UID:%08X,pid:%04X\r\n", 0, 0, 0, uid, 0XFFFF);
			// visit Q_stage squeue
			cnt=0;
			i=Q_stage.front;
			while(i!=Q_stage.rear && cnt++<90)
			{
				// printf("<VUID:%0X,%3d,pic=%3d\r\n",Q_stage.data[i].uid, i, Q_stage.data[i].pic_id);
				printf("<<<<<VUID:[%3d,%3d,%3d],Q-UID:%08X,pid:%04X\r\n", i,Q_stage.rear,(Q_stage.rear-i+MAXSIZE)%MAXSIZE,Q_stage.data[i].uid,Q_stage.data[i].pack_id);
				if(Q_stage.data[i].uid==uid)
				{
					while(--cnt)  //�������еĶ����Ļ���
					{
						DeQueue(&Q_stage, &elem);
						printf("*-Q_stage:[%3d,%3d,%3d],Q-UID:%08X,pid:%04X\r\n",Q_stage.front,Q_stage.rear,QueueLength(Q_stage),elem.uid,elem.pack_id);
						EnQueue(&Q_resent,elem);
						//printf("*+Q_resnt:[%3d,%3d,%3d],Q+UID:%08X,pid:%04X\r\n", Q_resent.front, Q_resent.rear, QueueLength(Q_resent), elem.uid,elem.pack_id);	
						printf("* Q_resnt:[%3d,%3d,%3d],Q*UID:%08X,pid:%04X\r\n", Q_resent.front, Q_resent.rear, QueueLength(Q_resent), Q_resent.data[Q_resent.front].uid, Q_resent.data[Q_resent.front].pack_id);						
					}
					
					DeQueue(&Q_stage, &elem);
					printf("- Q_stage:[%3d,%3d,%3d],Q-UID:%08X,pid:%04X\r\n",Q_stage.front,Q_stage.rear,QueueLength(Q_stage),elem.uid,elem.pack_id);
					// printf("* Q_resnt:[%3d,%3d,%3d],Q+UID:%08X,pid:%04X\r\n",Q_resent.front,Q_resent.rear,QueueLength(Q_resent),elem.uid,elem.pack_id);
					printf("* Q_resnt:[%3d,%3d,%3d],Q*UID:%08X,pid:%04X\r\n", Q_resent.front, Q_resent.rear, QueueLength(Q_resent), Q_resent.data[Q_resent.front].uid, Q_resent.data[Q_resent.front].pack_id);
					break;		
				}
				i=(i+1)%MAXSIZE;			
			}

		}
		else if(load[1]==0x90)  // ͨ���жϵڶ����ֽ���ʶ�����ָ��
		{
			printf("[LOG]check_config\r\n");
			res=check_config(load,len);
			if(res==0) // �ɹ��洢
			{
				printf("[INFO]mysend_config--UPDATA~~~~~\r\n");
				mysend_config("1");
			}
			else if(res==100)
			{
				printf("[INFO]mysend_config--NO_CHANGE~~~~~\r\n");
				mysend_config("1");
			}
			else
			{
				printf("[INFO]mysend_config--PARA_ERROR res:%d\r\n",res);
				mysend_config("2");  // ʧ��
			}
		}
		else if(load[1]==0x92)
		{	
			char buf[10];

			printf("[LOG]check_sever_config\r\n");
			res=check_sever_config(load,len);
			if(res==0)
			{
				printf("[LOG]check_sever_config--UPDATA~~~~~\r\n");
			}
			else
			{
				printf("[LOG]check_sever_config--ERROR\r\n");
			}
			sprintf(buf,"R%03d",res);
			mysend_config(buf);  // ��ȡ����	
		}
	}
}


/**
 * @description: ����MQTT��������ָ��
 * @param {type} 
 * @return {type} 
 */
static void MQTTReceiveTask(void *p_arg)
{
    OS_ERR err;
	int type; // �������յ�����ֵ
    //===========================
    unsigned char dup;
    int qos;
    // ������־
    unsigned char retained;
    // ��id
    unsigned short packetid;
    // ������
    MQTTString topicName;
    // ����
    unsigned char *payload;
    // ���ݳ���
    int payloadlen;
	//
	// int test_cnt=0;
    //==============================
    u8 flag=1;  // ��������һ������
	printf("[TASK]MQTTReceiveTask run\r\n");
	delay_ms(500);  

	while (1)
    {
        if (UART_TCP_buffLength() != 0)
        {
			#if (EN_LOG_PRINT >= 3)
            F407USART1_SendString("[LOG]+UART_TCP\r\n");
			#endif // EN_LOG_PRINT	
            //������յ���MQTT��Ϣ�������ݲ�ͬ����Ϣ��������ͬ�Ĵ���
            type = MQTTPacket_read(MQTT_Receivebuf, MQTT_RECEIVEBUFF_MAXLENGTH, transport_getdata);
            switch (type)
            {
				case CONNECT:
					break;
				case CONNACK:          //����������Ӧ
					// mqtt_state_set(1); // �������ӳɹ�
					mqtt_subscribe(MY_TOPIC_MSGDOWN);
					printf("[INFO]my topic msgdown=\"%s\"\r\n",MY_TOPIC_MSGDOWN);
					F407USART1_SendString("[LOG]MQTT Connect CONNACK\r\n");
					break;
				case PUBLISH: //���ĵ���Ϣ,�ɱ��˷���
					if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen,
												MQTT_Receivebuf, MQTT_RECEIVEBUFF_MAXLENGTH) == 1)
					{					
						#if (EN_LOG_PRINT >= 3)
						int i;
						F407USART1_SendString("[LOG]payload:");
						for (i = 0; i < payloadlen; i++)
						{
							printf("%0X",payload[i]);
							// ��ӡ�������������
						}
						F407USART1_SendString("\r\n");
						#endif // EN_LOG_PRINT	
						// ������������Ҫ����һ���ļ���
						check_response(payload,payloadlen);
					}
					break;
				case PUBACK: //������Ϣ��Ӧ����ʾ�����ɹ�
					break;
				case PUBREC:
					break;
				case PUBREL:
					break;
				case PUBCOMP:
					break;
				case SUBSCRIBE:
					break;
				case SUBACK: //������Ϣack	
					printf("[LOG]MQTT subscrible SUBACK\r\n");
					mqtt_state_set(1); // �������ӳɹ�
					// querry data
					if(flag)
					{
						printf("[LOG]mysend_config-QUERRY CONFIG~~~~~\r\n");
						mysend_config("0");  // ��ȡ����
						flag=0;
					}
					break;
				case UNSUBSCRIBE:
					break;
				case UNSUBACK:
					break;
				case PINGREQ:
					break;
				case PINGRESP: //Ping���ء�
					mqtt_ping_state_set(1);
					break;
				case DISCONNECT:
					break; //�ɿͻ��˷��͸�����������ʾ�����Ͽ�����
				default:
					break;
            }
        }
        OSTimeDly(5, OS_OPT_TIME_DLY, &err);
    }
}


/**
 * @description: 4Gģ������MQTT������
 * �Ǳ�Ҫ���񣬵���MQTT�������ӷ�����
 * @param {type} 
 * @return {type} 
 */
static void LTEModuleTask(void *p_arg)
{
    OS_ERR err;
	EC25_ERR res;
    uint16_t time = 0;
	static u8 mqtt_connect_flag=0;
	static u8 f_reconnect=0; 
	
    printf("[TASK]LTEModuleTask run\r\n");
	delay_ms(500);
		
	res = ec25_Init();  // ��ʼ��4Gģ�鲢����
	if(res == EC25_ERR_NONE)
	{ 
		printf("[LOG]EC25 ec25_Init succeed\r\n");
		ec25_SynLocalTime();
		ec25_QueeryCSQ();
		/* gpsx.gpssta = ec25_QueeryGPS();*/
	}
	else
	{
		F407USART1_SendString("[WARMING]EC25 ec25_Init error\r\n");
	}
	
	// init M8266
	IWDG_Feed();//ι��
	
    while (1)
	{    // ? ��������������뷢��ʱ���������ͻ    
        if (time % 5 == 0 && f_reconnect == 0)
        { //�������ʧ�ܣ�ÿ��5������³�������һ��
            if (mqtt_state_get() == 0 && ec25_on_flag == 1)  // ǰ����4G�����ɹ�
            {
				if(mqtt_connect_flag>=3)
				{
					printf("[LOG]Try mqtt_connect f:%d, restart ec25\r\n",mqtt_connect_flag);
					ec25_Restart();
					
				}
				else if(mqtt_connect_flag>6)
				{
					ec25_on_flag = 0;
					function_f&=(~0x10); 
					function_f&=(~0x20); 
					printf("[LOG]forcus to close network\r\n");
				}
                mqtt_connect();
				printf("[LOG]Try mqtt_connect f:%d\r\n",mqtt_connect_flag);
				
				mqtt_connect_flag++;  // �ۼƳ������Ӵ���				
            }
			else
			{

				mqtt_connect_flag=0;
			}
        }
		if (time % 10 == 1)  // mqtt ping
		{
			if (mqtt_state_get() == 1 && (function_f&0xf0) == 0)
            {
				u16 cnt=0;
				mqtt_Ping();
				printf("[LOG]ping mqtt .....\r\n");
				while(!mqtt_ping_state_get())
				{
					cnt++;
					OSTimeDly(10,OS_OPT_TIME_DLY,&err);
					if(cnt>=500) // 5s
					{
						printf("[LOG]none ping back .....\r\n");
						
						mqtt_state_set(0);

						break;
					}
				}
				printf("[LOG]receive ping back .....\r\n");
				//mycheck_Queue();
				mqtt_ping_state_set(0);  // ��ձ�־λ
            }
		}
        if (mqtt_state_get() == 1)
        {
			mqtt_connect_flag=0; // һ���ɹ�������

        }
        else
        {

        }
		time++;
        time %= 600;
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}


u16 time_cnt=1;
char up_down=1;
/**
 * @description: ��ʱ�������
 * ���Ҫ������ͣ��
 * 10sһ��ѭ��
 * @param {type} 
 * @return {type} 
 */
void  SystemDatasBroadcast (void *p_arg)
{
	u32 now_time;
	int time_delta;
	OS_ERR err;

	CPU_STK_SIZE free,used;
	(void)p_arg;

	F407USART1_SendString("[TASK]SystemDatasBroadcast run\r\n");
	delay_ms(500);  


	while(DEF_TRUE)
	{
		u8 sta;
		IWDG_Feed();//ι��
		#if DEBUG_MODE 
		#if USART1_DEBUG
		OSTaskStkChk (&SystemDatasBroadcast_TCB,&free,&used,&err);
		printf("SystemDatasBroadcast    used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		#endif
		OSTaskStkChk (&MainTaskTCB,&free,&used,&err);
		#if USART1_DEBUG
		printf("MainTaskTCB             used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		#endif	
		OSTaskStkChk (&MQTTReceivetcb,&free,&used,&err);
		#if USART1_DEBUG
		printf("MQTTReceive             used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		#endif	
		OSTaskStkChk (&LTEModuletcb,&free,&used,&err);
		#if USART1_DEBUG
		printf("LTEModuletcb             used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		#endif	
		OSTaskStkChk (&SysWatchtcb,&free,&used,&err);
		#if USART1_DEBUG
		printf("SysWatchtcb             used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		#endif
		if(watchdog_f*10>7200) //1h,SoftReset
		{
			printf("\r\n\r\n\r\nsleep\r\n\r\n");
			SoftReset();
		}
		if((watchdog_f*10)%60 == 30)  // 3min��60s��ʼ����������
		{
			function_f|=(0x04);  // ���ʹ�����
		}

		if((watchdog_f*10)%1800 == 70) // 30min��70s ��ʼɨ�����
		{
			function_f|=(0x02);  // ����
			//key_wkup_down = 1;
		}
		if((watchdog_f*10)%1800 == 100) // 30min��70s ת����Ƭ
		{
			function_f|=(0x01);  // ת��
			//key1_down = 1;
		}
		if((watchdog_f*10)%1800 == 150) // 30min��130s ��ʼ�ϴ�ͼƬ
		{
			function_f|=(0x08);  // ����ͼƬ
			//key2_down = 1;
		}
		#endif	
		now_time = get_time_cnt();
		time_delta = now_time - cycle.time_stamp;  // ����ʱ>0,����now_time+3600- cycle.time_stamp>0 �������쳣������ʱ���ʱ��
		if(time_delta <0)
			time_delta+=3600;
		sta = mqtt_state_get();
		printf("[BEAT]fun:%x|fun2:%x|netsta:%d|running time:{(max %d) %d}\r\n", function_f,function_f2, sta, max_work_length, time_delta);
		#if EN_log_sd
		if(sd_ready_flag ==0xAA)
			mf_sync();
		#endif
		if(function_f==0 && function_f2==0)  // �������420s�����Զ�����
		{
			#if SLEEP_MODE
			printf("[INFO]Well done, task accomplished!!!!!!\r\n");
			Sys_Enter_Standby(30);
			#endif
		}
		if(time_delta> max_work_length || time_delta<0)
		{
			#if SLEEP_MODE
			printf("$!Timeout, forced standby!!!!!!");
			Sys_Enter_Standby(30);
			#endif		
		}
		OSTimeDlyHMSM(0,0,10,0,(OS_OPT)OS_OPT_TIME_DLY,(OS_ERR*)&err);  
	}
}
