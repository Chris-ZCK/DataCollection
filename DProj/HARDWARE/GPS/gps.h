#ifndef __GPS_H
#define __GPS_H	 
#include "sys.h"
 	   
//NMEA 0183 协议解析后数据存放结构体
typedef struct  
{										    
	u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
	u8 nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
	u8 ewhemi;					//东经/西经,E:东经;W:西经
	u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
}nmea_msg;

int NMEA_Str2num(u8 *buf,u8*dx);
//void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf);
void EC25_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf);

extern nmea_msg gpsx;
#endif

 



