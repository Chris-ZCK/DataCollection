#ifndef __MQTTBASE_H
#define __MQTTBASE_H
#include "sys.h"
//#1
//#define CLIENTID "005a003f3436510b32323534"
//#2
//#define CLIENTID "005a003f3436510b32323535"

extern char MY_TOPIC_PICDOWN[30];
extern char MY_TOPIC_PICUP[30];
extern char MY_TOPIC_MSGDOWN[30];
extern char MY_TOPIC_MSGUP[30];
extern char MY_TOPIC_CONFIG[30];
extern char CLIENTID[30] ;
#define KEEP_ALIVE_INTERVAL 180
#define CLEANSESSION 1

#define C_USERNAME 					"admin"
#define C_PASSWORD      			"qiaojiaoshou123"
//#define MQTT_SEERVER_IP				"47.106.99.188"
#define MQTT_SEERVER_IP             "210.77.68.221"
//#define MQTT_SEERVER_IP             "159.226.67.10"
#define MQTT_SEERVER_PORT			10020

#define MQTT_RECEIVEBUFF_MAXLENGTH		2048
extern uint8_t MQTT_Receivebuf[MQTT_RECEIVEBUFF_MAXLENGTH];
#define MQTT_SEND_BUFF_MAXLENGTH		12000
extern uint8_t MQTT_SEND_buf[MQTT_SEND_BUFF_MAXLENGTH];
#define MQTT_DATA_BUFF_MAXLENGTH		12000
extern uint8_t MQTT_DATA_buf[MQTT_DATA_BUFF_MAXLENGTH];

//#define C_PASSWORD "qiaojiaoshou123"
//#define MQTT_SEERVER_IP "210.77.68.221"
//#define MQTT_SEERVER_PORT 10020

//订阅
int mqtt_subscrib(char *pTopic, char *pMessage);
//建立连接
int mqtt_connect(void);
//断开连接
int mqtt_disconnect(void);
//发布,前提是要先建立好了链接
int mqtt_publish(char *pTopic, char *pMessage, unsigned short msglen);
//订阅
int mqtt_subscribe(char *pTopic);
//ping
int mqtt_Ping(void);

void mqtt_UID_set(void);
#endif
