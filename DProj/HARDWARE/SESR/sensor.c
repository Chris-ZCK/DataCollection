#include "sensor.h"  // self

void Data_Packing_sens(char *msg)
{
	// sensor data
	u8 res;
	char now_time[30];
	float temp = 23.0;
	float hum = 50.2;
	float lus = 100;
	float BaroPressure=100000;
	float gps_longitude = 120.11611; 
	float gps_latitude = 30.26361;  
	float gps_altitude = 30.0;
	char tempdata[100];
	int ec25_csq=20;
	int relay_sta=0;
	// data updata
	// time
	calendar_get_time(&calendar);
	calendar_get_date(&calendar);
	sprintf(now_time,"%d-%d-%d %d:%d:%d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
	#if SENSOR_MODE
	SHT2x_GetTempandHumiPollAvg();
	temp=SHT20.TEMP_POLL;
	hum=SHT20.HUMI_POLL;
	lus=FluxMeasurement_get();
	BaroPressure=MS5611_Compensation_Calcu();
	battery_data_get(); 
	//closeReLoad();
	#endif
	if(ec25_on_flag==1)
	{
		
		ec25_csq = ec25_QueeryCSQ();
		// 判断是否正常获取GPS数据
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
			
			printf("*info:Qurry EC25 GPS|latitude:%f|longitude:%f|altitude:%f\r\n",gps_latitude, gps_longitude, gps_altitude);
		}
		else
		{
			gps_longitude = -9999;
			gps_latitude = -9999;
			gps_altitude = -9999;
			printf("*GPS is unlocated\r\n");
		}
	}
	else
	{
		ec25_csq = -9999;
		printf("*!not ec25 queery %d\r\n", ec25_csq);
		
		gps_longitude = -9999;
		gps_latitude = -9999;
		gps_altitude = -9999;
		printf("*!not ec25 queery GPS\r\n");
		
		
	}
	// data pack
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
	sprintf(tempdata,"|bv/%s/%d",battery.info,(int)(battery.total_voltage*2));
	strcat(msg,tempdata);

	sprintf(tempdata,"|bc//%d",(int)(battery.charge_current*10));
	strcat(msg,tempdata);

	sprintf(tempdata,"|bd//%d",(int)(battery.discharge_current*10));
	strcat(msg,tempdata);

	sprintf(tempdata,"|bt//%d",battery.cell_temperature-40);
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
	// <!强制置零>
	relay_sta = 0; 
	sprintf(tempdata,"|OUT/RLY/%d\n",relay_sta);
	strcat(msg,tempdata);	

printf("----------\r\n*info:%s\r\n----------\r\n", msg);
}

void act_get_data(void)
{	
	LED_YELLOW_ON();
	u8 bufmessages[500];
	F407USART1_SendString("->\r\n$act:act_get_data...\r\n");
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
	F407USART1_SendString("->\r\n$act:act_send_data...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	LED_YELLOW_OFF();
	mf_sensordata_send();
	return 1;
}
