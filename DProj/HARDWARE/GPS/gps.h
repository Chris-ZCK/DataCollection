#ifndef __GPS_H
#define __GPS_H	 
#include "sys.h"
 	   
//NMEA 0183 Э����������ݴ�Žṹ��
typedef struct  
{										    
	u32 latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
	u8 nshemi;					//��γ/��γ,N:��γ;S:��γ				  
	u32 longitude;			    //���� ������100000��,ʵ��Ҫ����100000
	u8 ewhemi;					//����/����,E:����;W:����
	u8 gpssta;					//GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.				  
	int altitude;			 	//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
}nmea_msg;

int NMEA_Str2num(u8 *buf,u8*dx);
//void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf);
void EC25_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf);

extern nmea_msg gpsx;
#endif

 



