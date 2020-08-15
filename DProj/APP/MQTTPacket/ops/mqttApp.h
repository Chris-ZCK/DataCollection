/*
 * @Author: your name
 * @Date: 2020-08-14 21:16:53
 * @LastEditTime: 2020-08-16 01:39:16
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \USERc:\Users\lihao\Documents\Git Projects\DataCollection\DProj\APP\MQTTPacket\ops\mqttApp.h
 */
#ifndef __MQTTAPP_H
#define __MQTTAPP_H
#include "sys.h"
//B129FB112111
//4B554B323820
//#1
//#define MY_TOPIC_PICDOWN "6D756E313D31/picdown"
//#define MY_TOPIC_PICUP "6D756E313D31/picup"
//#define MY_TOPIC_MSGDOWN "6D756E313D31/msgdown"
//#define MY_TOPIC_MSGUP "6D756E313D31/msgup"
//#2
//#define MY_TOPIC_PICDOWN "4B554B323820/picdown"
//#define MY_TOPIC_PICUP "4B554B323820/picup"
//#define MY_TOPIC_MSGDOWN "4B554B323820/msgdown"
//#define MY_TOPIC_MSGUP "4B554B323820/msgup"


#define SDCARD_BUFFER 8192
#define PACKAGE_LEN 10220

// 每次发送完图片后需要check一次缓存
#define RESEND_FUN_ON 1

struct pick_package
{
	uint16_t start_byte;		//起始字节，固定为0xa596
	uint16_t picture_id;		//图片序列，重新传输需要自动加1
	uint16_t packet_total_num;  //一共分成多少包
	uint16_t data_length;		//本包数据长度
	uint16_t packet_id;			//本包图片的ID
	uint32_t data_total_length; //图片数据总长度
								//unsigned char 	data[data_len];		   //图片数据
};

typedef enum
{
    MY_MQTT_ERR_NONE = 0,  // 无错误
    MY_MQTT_ERR_MEMOVER,  // 内存溢出
    MY_MQTT_ERR_DATA_ERR,  // 数据错误
    MY_MQTT_ERR_GETDATA_FAIL,  // 获取数据错误
    MY_MQTT_ERR_SEND_FAIL,  // 发送错误
    MY_MQTT_ERR_ACK_TIMEOUT  // 应答时间错误哦
} MY_MQTT_ERR;

unsigned char mqtt_ping_state_get(void);

MY_MQTT_ERR mysend_data(char *msg);
MY_MQTT_ERR mysend_config(char *msg);
MY_MQTT_ERR mysend_picture(uint8_t *file_path, uint32_t picture_id);

#if RESEND_FUN_ON 
MY_MQTT_ERR mycheck_Queue(void);
#endif

unsigned char mqtt_state_get(void);
void mqtt_state_set(unsigned char state);
void mqtt_ping_state_set(unsigned char state);
uint8_t Wait_mqttping_OKflag(uint16_t timeout);
void mqtt_ping_task(void);
#endif
