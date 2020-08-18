#include "sensor.h"  // self

void Data_Packing_sens(char *msg)
{
	char now_time[30];
	char tempdata[100];  // 缓存数据

	float temp = -9999;
	float hum = -9999;
	float lus = -9999;
	float BaroPressure = -9999;
	float gps_longitude = -9999; 
	float gps_latitude = -9999;  
	float gps_altitude = -9999;
	int ec25_csq = -9999;

	int relay_sta = 0;  // 继电器数据，默认为零
	
	// 更新当前时间
	calendar_get_time(&calendar);
	calendar_get_date(&calendar);
	sprintf(now_time,"%d-%d-%d %d:%d:%d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
	
	#if SENSOR_MODE
	// 获取各类传感器数据
	SHT2x_GetTempandHumiPollAvg();
	temp=SHT20.TEMP_POLL;
	hum=SHT20.HUMI_POLL;
	lus=FluxMeasurement_get();
	BaroPressure=MS5611_Compensation_Calcu();
	#endif

	// 4G数据
	if(ec25_on_flag==1)
	{
		ec25_csq = ec25_QueeryCSQ();
		#if QUEERY_GPS_ON
		// 判断是否正常获取GPS数据
		gpsx.gpssta = ec25_QueeryGPS();
		if(gpsx.gpssta!=0) 
		{
			if(gpsx.nshemi == 'N')
				gps_latitude = gpsx.latitude/100000.0;
			else if(gpsx.ewhemi == 'S')
				gps_latitude = -gpsx.latitude/100000.0;
			else
				gps_altitude = -9999;
			
			if(gpsx.ewhemi == 'E')
				gps_longitude = gpsx.longitude/100000.0;
			else if(gpsx.ewhemi == 'W')
				gps_longitude = -gpsx.longitude/100000.0;
			else
				gps_longitude = -9999;

			gps_altitude = gpsx.altitude/10.0;
			
			printf("[INFO]Qurry EC25 GPS|latitude:%f|longitude:%f|altitude:%f\r\n",gps_latitude, gps_longitude, gps_altitude);
		}
		else
		{
			gps_longitude = -9999;
			gps_latitude = -9999;
			gps_altitude = -9999;
			printf("[INFO]GPS is unlocated\r\n");
		}
		#else
		gps_longitude = -9999;
		gps_latitude = -9999;
		gps_altitude = -9999;
		printf("[INFO]GPS is not work\r\n");
		#endif
	}
	else
	{
		ec25_csq = -9999;
		printf("[WARNING]not ec25 queery %d\r\n", ec25_csq);
		
		gps_longitude = -9999;
		gps_latitude = -9999;
		gps_altitude = -9999;
		printf("[WARNING]not ec25 queery GPS\r\n");
	}

	// Sensors
	sprintf(msg,"msgty//sdata|sid//11|time//%s",now_time);
  //SHT21-temp
  sprintf(tempdata,"|t/SHT31/%f",temp);
	strcat(msg,tempdata);
	//SHT21-humi
	sprintf(tempdata,"|h/SHT31/%f",hum);
	strcat(msg,tempdata);
	//MS5611-pres 
    sprintf(tempdata,"|p/MS5611/%f",BaroPressure);
	strcat(msg,tempdata);
	//MAX44009-lus
	sprintf(tempdata,"|lus/MAX44009/%f",lus);
	strcat(msg,tempdata);
	
	//Battery 	
	#if QUEERY_BATTERY_ON
	battery_data_get(); 
	#else
	battery.total_voltage = -9999.0/2;
	battery.charge_current = -9999.0/10;
	battery.discharge_current= -9999.0/10;
	battery.cell_temperature= -9959;
	battery.level = -9999;
	battery.remain_capacity= -9999;
	battery.max_capacity= -9999;
	printf("[WARNING]Battey is not work\r\n");
	#endif
	sprintf(tempdata,"|bv/%s/%d","work",(int)(battery.total_voltage));
	strcat(msg,tempdata);

	sprintf(tempdata,"|bc//%d",(int)(battery.charge_current));
	strcat(msg,tempdata);

	sprintf(tempdata,"|bd//%d",(int)(battery.discharge_current));
	strcat(msg,tempdata);

	sprintf(tempdata,"|bt//%d",battery.cell_temperature);
	strcat(msg,tempdata);

	sprintf(tempdata,"|bl//%d",battery.level);
	strcat(msg,tempdata);

	sprintf(tempdata,"|bm//%d",battery.max_capacity);
	strcat(msg,tempdata);

	sprintf(tempdata,"|br//%d",battery.remain_capacity);
	strcat(msg,tempdata);
	
	
	//GPS modle
	sprintf(tempdata,"|LO/gps/%f",gps_longitude);
	strcat(msg,tempdata);
	sprintf(tempdata,"|LA/gps/%f",gps_latitude);
	strcat(msg,tempdata);
	sprintf(tempdata,"|AL/gps/%f",gps_altitude);
	strcat(msg,tempdata);
	
	//4G modle
	sprintf(tempdata,"|CSQ/EC25/%d",ec25_csq);
	strcat(msg,tempdata);
	
	// relay state
	sprintf(tempdata,"|OUT/RLY/%d\n",relay_sta);
	strcat(msg,tempdata);	

	printf("----------\r\n*info:%s\r\n----------\r\n", msg);
}

void act_get_data(void)
{	
	LED_YELLOW_ON();
	u8 bufmessages[500];
	F407USART1_SendString("[INST]]act:act_get_data...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	LED_YELLOW_OFF();
	Data_Packing_sens((char *)bufmessages);
	mf_sensordata_write(bufmessages);
}

// send sensor data
u8 act_send_data(void)
{
	LED_YELLOW_ON();
	F407USART1_SendString("[INST]act:act_send_data...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	LED_YELLOW_OFF();
	mf_sensordata_send();
	return 1;
}
