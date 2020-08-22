#include "sys.h"
#include "includes.h"
//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()

/* 开始任务 
*/
#define START_TASK_PRIO									3
//任务堆栈大小	
#define START_STK_SIZE 									128
static OS_TCB 											StartTaskTCB;	
static CPU_STK 											START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

/* 系统主任务 */
#define MAIN_TASK_PRIO									6
//任务堆栈大小
#define CPU_STK_MAIN_SIZE								1000
//时间片长度
#define MAIN_TICK_LEN									0
static  OS_TCB											MainTaskTCB;
static	CPU_STK											MainTaskStk[CPU_STK_MAIN_SIZE];
static void  MainTask(void* p_arg);


/* 接收MQTT指令 */
#define MQTTReceive_PRIO								7
//任务堆栈大小32bit
#define CPU_STK_MQTTReceive_SIZE						1000
//时间片长度
#define MQTTReceive_TICK_LEN							0
static  OS_TCB											MQTTReceivetcb;
static	CPU_STK											MQTTReceivestk[CPU_STK_MQTTReceive_SIZE];
static void  MQTTReceiveTask(void* p_arg);

/* 发送MQTT指令 */
#define LTEModule_PRIO									8
//任务堆栈大小32bit
#define CPU_STK_LTEModule_SIZE							1000
//时间片长度
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

#define  SystemDatasBroadcast_PRIO                      11 // 统计任务优先级最低，我这里是12，已经低于其他任务的优先级了
#define  SystemDatasBroadcast_STK_SIZE                  200 // 任务的堆栈大小，做统计一般够了，统计结果出来后不够再加..
#define  SystemDatasBroadcast_LED_TICK_LEN              0
static   OS_TCB                                         SystemDatasBroadcast_TCB;		                // 定义统计任务的TCB
static   CPU_STK                                        SystemDatasBroadcast_STK [SystemDatasBroadcast_STK_SIZE];// 开辟数组作为任务栈给任务使用
static void  SystemDatasBroadcast (void *p_arg);


/**
 * @description: 系统软复位
 * @param {type} 
 * @return {type} 
 */
void SoftReset(void)
{
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}


// bit0:表示电脑正在向SD卡写入数据
// bit1:表示电脑正从SD卡读出数据
// bit2:SD卡写数据错误标志位
// bit3:SD卡读数据错误标志位
// bit4:1,启动ping任务
// bit5:保留.
// bit6:保留.
// bit7:保留.

struct cycle_package cycle;
struct flash_package eerom;
#define FLASH_WRITE_MODE 0
/////////////////systerm runing state//////////
vu16 watchdog_f;
vu16 function_f;
vu16 function_f2;
vu16 ec25_on_flag;
vu16 m8266_on_flag;
vu16 m8266_work_state;  // WiFi发送数据的开关
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
	
	printf("[INFO]WIFI_CONNECTION_TYPE = %d(0-UDP, 1-Client,2-Server)\r\n",TEST_CONNECTION_TYPE); //连接类型
	printf("[INFO]WIFI_LOCAL_PORT      = %d(0:updated random port)\r\n",TEST_LOCAL_PORT);			//本地端口号
	printf("[INFO]WIFI_REMOTE_IP_ADDR  = %s\r\n",TEST_REMOTE_IP_ADDR);//目标IP
	printf("[INFO]WIFI_REMOTE_PORT     = %d\r\n",TEST_REMOTE_PORT);	//目标IP端口号
	
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
			m8266_work_state =0; // 关闭发送数据的指令
			return M8266_ERROR;
		}
	}
	printf("[LOG]Succeed M8266WIFI_Module_Init_Via_SPI\r\n");
	return M8266_SUCCESS; // 成功
}


/**
 * @description: 系统初始化
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
	function_f = 0;  // 任务执行标志清零
	function_f2 = 0;
	ec25_on_flag = 0;  // 测试模式
	key_on_flag = 0; // 任务执行标志清零
	led_on_flag = 0;


	// systerm initial
	delay_init(168);  	// 时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 中断分组配置
		
	#if DEBUG_MODE
	uart_init(115200); 	// 串口初始化
	#else
	uart_init(921600); 	// 串口初始化
	#endif
	printf("\r\n\r\n\r\n>>>>>>>>>>>>>>SYSTERM START>>>>>>>>>>>>>>\r\n");
	
	KEY_Init();	  		// key init
	LED_Init();   		// LED init
	
	IWDG_Init(IWDG_Prescaler_256,4000);  // 4,000*256/32,000=32s
	
	My_RTC_Init();  //初始化RTC
	
	calendar_get_time(&calendar);
	calendar_get_date(&calendar);
	printf("[TIME]%d/%d/%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
    
	// local_time_cnt = calendar.sec;  // 用于看门狗统计
	
	#if ANAY_TASK_ON
	// updata sys parameters
	// 打开SD开
	// 读取数据
	// 解析数据buf,len
	// flash
	mf_config_data_read_flash(m_buf);
	res = analyze_config_para((char *)m_buf,m_value);
	if(res==0) // 有意义
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
	STMFLASH_Write(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);	// 将初始化参数写入寄存器
	while(1);
	#else
	//printf("[INFO]cycle data:%d,%d,%d,%d,%x\r\n",cycle.time_stamp,cycle.picture_id,cycle.task_cnt,cycle.watch_cnt,cycle.function);
	#endif
	
	// 处理休眠机制
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
	// 有效数据
	STMFLASH_Read(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);
	printf("[INFO]STMFLASH_Read:\r\n");
	printf("\t*time_stamp:%d\r\n\t*task_cnt:%d\r\n",cycle.time_stamp,cycle.task_cnt);
	// 获取时间
	now_time = get_time_cnt();
	time_delta = now_time - cycle.time_stamp;  // 正常时>0,或者now_time+3600- cycle.time_stamp>0 ，否则异常，更新时间戳时间
	printf("[INFO]count down|next statr time:{(T:%d) %d}\r\n",sensor_frequency,sensor_frequency-time_delta);
	if(!key_on_flag && !function_f2)  // 休眠 
	{
		if(time_delta <0)
			time_delta+=3600;
		if(time_delta> sensor_frequency || time_delta<0)  // 符合条件,加上小于零的判断，方便过滤错误
		{
			cycle.time_stamp=now_time;  // 更新时间戳
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
		cycle.time_stamp=now_time;  // 更新时间戳
		cycle.task_cnt ++;		
		STMFLASH_Write(FLASH_SAVE_ADDRC1,(u32 *)&cycle,sizeof(cycle)/4);
		printf("[INFO]STMFLASH_Write|time_stamp:%d,task_cnt:%d\r\n",cycle.time_stamp,cycle.task_cnt);
		printf("[INFO]force to execute the task,can't sleep!!!\r\n");
	}
	#endif

	// SD卡初始化
    if (!SD_Init())
	{
		printf("[LOG]SD_Init ok\r\n"); //判断SD卡是否存在
		// 访问文件夹
	}
	else
	{
        printf("[ERROR]SD_Init failed!!!!!!!!!!\r\n");
	}
	mymem_init(SRAMIN);      // 初始化内部内存池
	exfuns_init();           // 为fatfs相关变量申请内存
	f_mount(fs[0], "0:", 1); // 挂载SD卡
	
	// mqtt相关
	rng_Init();			// 随机数生成器初始化
	mqtt_UID_set();     // 生成唯一id
	InitQueue(&Q_stage);  	// 初始化队列 
	InitQueue(&Q_resent);  	// 初始化队列 	
	
	#if EN_LOG_PRINT > 2
	mf_scan_files((u8*)"0:");
	mf_check_dir((u8*)"0:INBOXWIFI");
	mf_check_dir((u8*)"0:INBOX");
	mf_check_dir((u8*)"0:ARCH");
	#endif

	// 打卡SD卡日志记录开关
	#if EN_log_sd
	mf_log_init();			 //初始化日志
	#endif
	
	#if USB_MODE
	usbapp_init();
	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);
	delay_ms(1000);
	f_mount(fs[1], "1:", 1);  // 挂载U盘
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
	
	Power_Ctrl_Init(); // 电源初始化	
	
	#if SENSOR_MODE
	SHT2x_Init();  			// SHT20初始化
	max44009_initialize();  // MAX44009初始化
	#if SESOR_MS5611_ON
	MS5611_Init();  		// MS5611初始化
	#endif
	USART2_init(9600); 		// 电池数据端口初始化
	Cam_Crtl_Init();   		// 相机控制引脚初始化
	printf("[LOG]SENSOR init\r\n");
	#endif

	#if ANAY_TASK_ON
	// 任务解析部分想要重新加进来，当下的过于死板
	// 任务解析
	printf("[LOG]Task analysis...........\r\n");
	printf("[INFO]Task count:%d\r\n", cycle.task_cnt);
	printf("[INFO]Task list:");
	function_f|=(0x01);  // 获取数据
	printf("\tget data\r\n");

	function_f|=(0x40);  // 打开WiFi
	printf("\topen wifi and send data\r\n");
	if(cycle.task_cnt%camera_frequency==0 || key_on_flag) 
	{
		function_f|=(0x02);  // 拍照
		printf("\ttake photo\r\n");		
	}
	if(cycle.task_cnt%transfer_photo_frequency==0 || key_on_flag) 
	{
		function_f|=(0x04);  // 转存照片	
		printf("\tstore photo\r\n");
	}
	
	if(cycle.task_cnt%upload_frequency==0 || key_on_flag)  // 发送数据
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
			function_f|=(0x10);  // 发送数据	
			printf("[INFO]send data\r\n");
			function_f|=(0x20);  // 发送图片
			printf("[INFO]end photo\r\n");	
		}			
	}
	printf("[INFO]function=%x\r\n",function_f);
	//printf("-------------------------\r\n\r\n");
	#endif
	
	// WiFi联网
	if(wifi_work_on_flag ==1)
	{
		u8 res=0;
		IWDG_Feed();//喂狗
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
	
	IWDG_Feed();//喂狗
//	while(1)
//	{
//		
//		now_timea++;
//		if(now_timea%10==0)
//		{
//			IWDG_Feed();//喂狗
//			printf("delta=%d\r\n",now_timea);
//		}
//		if(now_timea%20==0)
//		{
//			char buf[50];
//			IWDG_Feed();			
//			WiFiSendPic("0:pic1.jpg",now_timea); // 发送图片 					
//		}
//		delay_ms(1000);
//	}
}


/**
 * @description: 分析参数是否正常/来自服务器参数
 * @param {type} 
 * buf 参数缓存地址
 * val 解析结果存放数组
 * @return {type} 
 * 0      分析成功
 * 100    不需要更新
 * others 数据异常
 * 例子：600|10800|3600|3600|12500|800|8|15|1200
 *  第二版 600|10800|3600|3600|12500|800|8|15|1200|0
 */
u8 analyze_config_para(char *buf, u16 * val)
{
	u8 res;
	// 中间变量
	u8 offset;
	//static u16 val[9];
	
	// 判断是否为更新变量
	if(buf[0]=='0')
	{
		//config_flag = 1;
		res=100;
		printf("[INFO]analyze_config_para|the data is latest, no need to updata!\r\n");
		goto an_end;
	}
	// 分析控制参数
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
	if(val[8]>2400 || val[8]<180)  // 最大40min,最小3min
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
 * @description: 主函数
 * 启动任务
 * @param {type} 
 * @return {type} 
 */
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	system_init();		//系统初始化 
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区			 
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	// 退出临界区	 
	OSStart(&err);      // 开启UCOSIII
}


/**
 * @description:开始任务函数 
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
   	OSStatTaskCPUUsageInit(&err);  	// 统计任务                
	#endif
	
	#ifdef CPU_CFG_INT_DIS_MEAS_EN		// 如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
	#endif
	
	#if	OS_CFG_SCHED_ROUND_ROBIN_EN  	// 当使用时间片轮转的时候
	//使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
	#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区

    // stackMonitoring
    OSTaskCreate((OS_TCB *)&SystemDatasBroadcast_TCB,
                 (CPU_CHAR *)"SystemDatasBroadcast",
                 (OS_TASK_PTR)SystemDatasBroadcast,
                 (void *)0,
                 (OS_PRIO)SystemDatasBroadcast_PRIO,
                 (CPU_STK *)&SystemDatasBroadcast_STK[0],
                 (CPU_STK_SIZE)SystemDatasBroadcast_STK_SIZE / 10, /*栈溢出临界值设置在栈大小的90%处*/
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
				 
    OSTaskCreate((OS_TCB *)&MainTaskTCB,                              // 该任务堆栈的开始地址
                 (CPU_CHAR *)"MainTask",                              // 任务分配名字
                 (OS_TASK_PTR)MainTask,                               // 指向任务代码的指针
                 (void *)0,                                           // 指针，第一次执行任务时传递给，任务实体的指针参数*p_arg
                 (OS_PRIO)MAIN_TASK_PRIO,                             // 优先级设置	 参数值越小优先级越高
                 (CPU_STK *)&MainTaskStk[0],                          // 任务堆栈的基地址。基地址通常是分配给该任务的堆栈的最低内存位置
                 (CPU_STK_SIZE)CPU_STK_MAIN_SIZE / 10,                // 第七个参数是地址“水印” ，当堆栈生长到指定位置时就不再允许其生长
                 (CPU_STK_SIZE)CPU_STK_MAIN_SIZE,                     // 任务的堆栈大小
                 (OS_MSG_QTY)0,                                       //
                 (OS_TICK)MAIN_TICK_LEN,                              // 设置任务拥有多少个时间片，当采用时间片轮询调度任务时有效
                 (void *)0,                                           //
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //
                 (OS_ERR *)&err);      
				 
	OS_CRITICAL_EXIT();			// 退出临界区
	OSTaskDel((OS_TCB*)0,&err);	// 删除start_task任务自身
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
		// USB开关打开后，方执行下属的USB扫描程序
		if(usbConnectSwitchGet())
		{
			if((t%60)==0)
			{
				if(usbx.hdevclass==1)
				{
					if(local_memdevflag==0)
					{
						fs[1]->drv=2;  			// 暂时认为也可以不加,我认为可能是重命名
						f_mount(fs[1],"1:",1); 	// 重新挂载U盘
						usbapp_user_app();

						usbConnectStateSet(1);						
						local_memdevflag=1;
					}  
				}
				else 
					local_memdevflag=0; // U盘被拔出了
			}
			
			while((usbx.bDeviceState&0XC0)==0X40) // USB设备插入了,但是还没连接成功,猛查询.
			{
				usbapp_pulling();  // 轮询处理USB事务
				delay_ms(1);  // 不能像HID那么猛...,U盘比较慢
				#if (EN_LOG_PRINT >= 2)
				printf(".");
				#endif // EN_LOG_PRINT
			}
			usbapp_pulling();  // 检测USB
			#if (EN_LOG_PRINT >= 2)
			printf("\\");
			#endif // EN_LOG_PRINT
		}
		#endif
		UART_TCPbuff_Run(F407USART3_buffRead);  // 循环读取U3中缓存数据
		//key_scan_fun();
		OSTimeDly(5,OS_OPT_TIME_DLY,&err);
	}									 
}


/**
 * @description: 扫描转存相机中的文件
 * @param {type} 
 * @return {type} 
 */
void act_scan_camera(void)
{
	u8 res;
	// 初始化USB
	// 打开相机开关，对应于watch_task任务开始扫描USB
	LED_YELLOW_ON();
	printf("[INST]act:act_scan_camera...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	LED_YELLOW_OFF();
	local_memdevflag=0;
	usbConnectSwitchSet(1);  
	usbapp_mode_set();  // 设置设置USB模式，复位USB
	usbConnectStateSet(0);  // 清空标志位
	openUSB();  // 打开相机的USB可电源
	// 等待连接成功
	IWDG_Feed();
	res = waitUsbConnectFlag(10000);
	if (res == 1) // 正常打开相机
	{
		printf("[LOG]successful find usb，open camera!\r\n");
	}
	else
	{
		F407USART1_SendString("[WARNING]Fail WaitDistinguishflag...\r\n");
	}
	
	printf("[LOG]try to scan usb，open camera!\r\n");
	delay_ms(2000);  // 等待相机稳定
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
	if (res == 1) // 正常关闭相机
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
 * @description: 拍照
 * @param {type} 
 * @return {type} 
 */
void act_take_photo(void)
{
	// LED_YELLOW_ON();
	// 判断相机状态,如果相机处于连接状态，则跳过
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
 * @description: 发送图片
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
	mf_scan_files("0:INBOX");  // 扫描文件夹
	mf_send_pics("0:INBOX","0:ARCH",1,0);  // 发送图片
	printf("[INFO]mf_scan_files-A\r\n");
	mf_scan_files("0:INBOX");  // 扫描文件架
	return 1;
}

u8 act_send_picture_wifi(void)
{
	F407USART1_SendString("[INST]act:act_send_picture_wifi...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);	
	printf("[INFO]mf_scan_files-B: \r\n");
	mf_scan_files("0:INBOXWIFI");  // 扫描文件夹
	mf_send_pics("0:INBOXWIFI","0:ARCH",1,1);  // 发送图片
	printf("[INFO]mf_scan_files-A\r\n");
	mf_scan_files("0:INBOXWIFI");  // 扫描文件架
	return 1;
}


/**
 * @description:
 * @param {type} 
 * @return {type} 0 表示成功
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
						function_f=0;  // 强制休眠
						printf("[INFO]#$ins:force to sleep\r\n");	
						break;
					case '1':
						function_f|=(0x01);  // 获取数据
						printf("[INFO]#$ins:get data\r\n");
						break;
					case '2':
						function_f|=(0x02);  // 拍照
						printf("[INFO]#$ins:take photo\r\n");	
						break;
					case '3':
						function_f|=(0x04);  // 转存照片
						printf("[INFO]#$ins:store photo\r\n");
						break;
					case '4':
						function_f|=(0x10);  // 发送数据	
						printf("[INFO]#$ins:send data\r\n");
						break;
					case '5':
						function_f|=(0x20);  // 发送图片
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
						function_f2=0;  // 启动休眠
						printf("[INFO]#$ins:ready to sleep\r\n");	
						break;
						
					case '1':
						function_f2=1;  // 关闭休眠
						printf("[INFO]#$ins:not sleep\r\n");	
						break;
					case '2':
						printf("[INFO]#$ins:Request to get parameters\r\n");
						if (mqtt_state_get() == 1)
						{
							printf("[LOG]$mysend_config-QUERRY CONFIG~~~~~\r\n");
							mysend_config("0");  // 获取参数
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
 * @description: 主执行任务
 * 上层封装的任务，用于调用任务函数，执行任务指令
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
		if(USART_RX_STA &= 0x8000)  // 接受串口的指令
		{
			check_uart_commamd(USART_RX_BUF);
			USART_RX_STA = 0;
		}
		#endif
		// 不需要联网 或者 联网成功
		if((ec25_on_flag==0) || (mqtt_state_get() == 1)) 
		{
			if(function_f&0x01 )  // 获取数据
			{
				IWDG_Feed();				
				act_get_data();
				function_f&=(~0x01);
				printf("[INFO]finish act_get_data , fun:%x~~~~~\r\n",function_f);
			}
			else if(function_f&0x02 )  // 拍照
			{
				IWDG_Feed();
				act_take_photo();
				delay_ms(1000);
				function_f&=(~0x02);
				printf("[LOG]finish act_take_photo, fun:%x\r\n",function_f);
			}
			else if(function_f&0x04 )  // 转存
			{
				IWDG_Feed();				
				act_scan_camera();
				delay_ms(1000);
				function_f&=(~0x04);
				printf("[LOG]finish act_scan_camera, fun:%x~~~~~\r\n",function_f);
			}		
			else if(function_f&0x40)	// 发送WIFI数据
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
		// 联网成功方可执行
		if(mqtt_state_get() == 1 && !(function_f&0x0F))  // 先执行基础任务再执行发送任务
		{	
			if(function_f&0x10)  // 发送传感器
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
			
			else if(function_f&0x20)  // 发送图片
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
 * @description: 提取参数
 * 校验 参数
 * instant:"600|10800|3600|3600|12500|800|8|15|1200"
 * @param {type} 
 * @return {type} 
 */
u8 check_config(u8 *load, u16 len)
{
	u8 res=0;
	// 中间变量
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
			// 获得参数分析结果
			res = analyze_config_para((char*)load+10, value1);

			if(res==0)
			{
				// 存储参数信息，字符串格式
				// 并没有进行updata,下次启动有效
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
	// 中间变量
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

//	InitQueue(&Q_stage);  // 初始化队列 
//	InitQueue(&Q_resent);  // 初始化队列
/**
 * @description: 检查服务器反馈的消息
 * a5
 * 		96 反馈的校验码
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
		if(load[1]==0x96)  // 反馈的校验码
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
					while(--cnt)  //弹出所有的多多余的缓存
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
		else if(load[1]==0x90)  // 通过判断第二个字节来识别控制指令
		{
			printf("[LOG]check_config\r\n");
			res=check_config(load,len);
			if(res==0) // 成功存储
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
				mysend_config("2");  // 失败
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
			mysend_config(buf);  // 获取参数	
		}
	}
}


/**
 * @description: 接收MQTT服务器的指令
 * @param {type} 
 * @return {type} 
 */
static void MQTTReceiveTask(void *p_arg)
{
    OS_ERR err;
	int type; // 解析接收的数据值
    //===========================
    unsigned char dup;
    int qos;
    // 保留标志
    unsigned char retained;
    // 包id
    unsigned short packetid;
    // 主题名
    MQTTString topicName;
    // 数据
    unsigned char *payload;
    // 数据长度
    int payloadlen;
	//
	// int test_cnt=0;
    //==============================
    u8 flag=1;  // 仅仅发送一次数据
	printf("[TASK]MQTTReceiveTask run\r\n");
	delay_ms(500);  

	while (1)
    {
        if (UART_TCP_buffLength() != 0)
        {
			#if (EN_LOG_PRINT >= 3)
            F407USART1_SendString("[LOG]+UART_TCP\r\n");
			#endif // EN_LOG_PRINT	
            //处理接收到的MQTT消息，并根据不同的消息类型做不同的处理
            type = MQTTPacket_read(MQTT_Receivebuf, MQTT_RECEIVEBUFF_MAXLENGTH, transport_getdata);
            switch (type)
            {
				case CONNECT:
					break;
				case CONNACK:          //连接请求响应
					// mqtt_state_set(1); // 设置连接成功
					mqtt_subscribe(MY_TOPIC_MSGDOWN);
					printf("[INFO]my topic msgdown=\"%s\"\r\n",MY_TOPIC_MSGDOWN);
					F407USART1_SendString("[LOG]MQTT Connect CONNACK\r\n");
					break;
				case PUBLISH: //订阅的消息,由别人发布
					if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen,
												MQTT_Receivebuf, MQTT_RECEIVEBUFF_MAXLENGTH) == 1)
					{					
						#if (EN_LOG_PRINT >= 3)
						int i;
						F407USART1_SendString("[LOG]payload:");
						for (i = 0; i < payloadlen; i++)
						{
							printf("%0X",payload[i]);
							// 打印缓存区域的内容
						}
						F407USART1_SendString("\r\n");
						#endif // EN_LOG_PRINT	
						// 理论上这里需要进行一定的检验
						check_response(payload,payloadlen);
					}
					break;
				case PUBACK: //发布消息响应，表示发布成功
					break;
				case PUBREC:
					break;
				case PUBREL:
					break;
				case PUBCOMP:
					break;
				case SUBSCRIBE:
					break;
				case SUBACK: //订阅消息ack	
					printf("[LOG]MQTT subscrible SUBACK\r\n");
					mqtt_state_set(1); // 设置连接成功
					// querry data
					if(flag)
					{
						printf("[LOG]mysend_config-QUERRY CONFIG~~~~~\r\n");
						mysend_config("0");  // 获取参数
						flag=0;
					}
					break;
				case UNSUBSCRIBE:
					break;
				case UNSUBACK:
					break;
				case PINGREQ:
					break;
				case PINGRESP: //Ping返回·
					mqtt_ping_state_set(1);
					break;
				case DISCONNECT:
					break; //由客户端发送给服务器，表示正常断开连接
				default:
					break;
            }
        }
        OSTimeDly(5, OS_OPT_TIME_DLY, &err);
    }
}


/**
 * @description: 4G模组连接MQTT服务器
 * 非必要任务，调用MQTT函数连接服务器
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
		
	res = ec25_Init();  // 初始化4G模组并联网
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
	IWDG_Feed();//喂狗
	
    while (1)
	{    // ? 这里的重连可能与发送时候的重连冲突    
        if (time % 5 == 0 && f_reconnect == 0)
        { //如果连接失败，每隔5秒会重新尝试连接一次
            if (mqtt_state_get() == 0 && ec25_on_flag == 1)  // 前提是4G联网成功
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
				
				mqtt_connect_flag++;  // 累计尝试连接次数				
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
				mqtt_ping_state_set(0);  // 清空标志位
            }
		}
        if (mqtt_state_get() == 1)
        {
			mqtt_connect_flag=0; // 一旦成功则清零

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
 * @description: 定时监控任务
 * 务必要，可以停用
 * 10s一个循环
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
		IWDG_Feed();//喂狗
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
		if((watchdog_f*10)%60 == 30)  // 3min过60s开始发送起数据
		{
			function_f|=(0x04);  // 发送传感器
		}

		if((watchdog_f*10)%1800 == 70) // 30min过70s 开始扫描相机
		{
			function_f|=(0x02);  // 拍照
			//key_wkup_down = 1;
		}
		if((watchdog_f*10)%1800 == 100) // 30min过70s 转存照片
		{
			function_f|=(0x01);  // 转存
			//key1_down = 1;
		}
		if((watchdog_f*10)%1800 == 150) // 30min过130s 开始上传图片
		{
			function_f|=(0x08);  // 发送图片
			//key2_down = 1;
		}
		#endif	
		now_time = get_time_cnt();
		time_delta = now_time - cycle.time_stamp;  // 正常时>0,或者now_time+3600- cycle.time_stamp>0 ，否则异常，更新时间戳时间
		if(time_delta <0)
			time_delta+=3600;
		sta = mqtt_state_get();
		printf("[BEAT]fun:%x|fun2:%x|netsta:%d|running time:{(max %d) %d}\r\n", function_f,function_f2, sta, max_work_length, time_delta);
		#if EN_log_sd
		if(sd_ready_flag ==0xAA)
			mf_sync();
		#endif
		if(function_f==0 && function_f2==0)  // 如果超过420s，则自动休眠
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
