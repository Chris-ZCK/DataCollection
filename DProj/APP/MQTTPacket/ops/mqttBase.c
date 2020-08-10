#include "mqttBase.h"
#include "MQTTPacket.h"
#include "mqttTransport.h"

uint8_t MQTT_Receivebuf[MQTT_RECEIVEBUFF_MAXLENGTH];
uint8_t MQTT_SEND_buf[MQTT_SEND_BUFF_MAXLENGTH];  // MQTT������
uint8_t MQTT_DATA_buf[MQTT_DATA_BUFF_MAXLENGTH];  // ���ݻ�����


//char MY_TOPIC_PICDOWN[30] = "4B554B323820/picdown";
//char MY_TOPIC_PICUP[30] = "4B554B323820/picup";
//char MY_TOPIC_MSGDOWN[30] = "4B554B323820/msgdown";
//char MY_TOPIC_MSGUP[30] = "4B554B323820/msgup";
//char MY_TOPIC_CONFIG[30] = "4B554B323820/config";

//char CLIENTID[30] =  "005a003f3436510b32323535";

char MY_TOPIC_PICDOWN[30] = "559D50415283/picdown";
char MY_TOPIC_PICUP[30] = "559D50415283/picup";
char MY_TOPIC_MSGDOWN[30] = "559D50415283/msgdown";
char MY_TOPIC_MSGUP[30] = "559D50415283/msgup";
char MY_TOPIC_CONFIG[30] = "559D50415283/config";

char CLIENTID[30] =  "005a003f3436510b32323538";


void mqtt_UID_set(void)
{
	u32 mcuID[3];
	mcuID[0] = (*(volatile uint32_t *)0x1fff7a10);  //???��?ID
	mcuID[1] = (*(volatile uint32_t *)0x1fff7a14);
	mcuID[2] = (*(volatile uint32_t *)0x1fff7a18);

	sprintf(CLIENTID, "%08X%08X%08X", mcuID[0], mcuID[1], mcuID[2]);

	sprintf(MY_TOPIC_PICDOWN, "%08X%04X/picdown", mcuID[0]+mcuID[1], (u16)(mcuID[2]+(mcuID[2]>>16)));
	sprintf(MY_TOPIC_PICUP  , "%08X%04X/picup",   mcuID[0]+mcuID[1], (u16)(mcuID[2]+(mcuID[2]>>16)));
	sprintf(MY_TOPIC_MSGDOWN, "%08X%04X/msgdown", mcuID[0]+mcuID[1], (u16)(mcuID[2]+(mcuID[2]>>16)));
	sprintf(MY_TOPIC_MSGUP  , "%08X%04X/msgup",   mcuID[0]+mcuID[1], (u16)(mcuID[2]+(mcuID[2]>>16)));
	sprintf(MY_TOPIC_CONFIG , "%08X%04X/config",  mcuID[0]+mcuID[1], (u16)(mcuID[2]+(mcuID[2]>>16)));
	
//	printf("*MQTT:Clinet ID       :%s\r\n",CLIENTID);
//	printf("*MQTT:MY_TOPIC_PICDOWN:%s\r\n",MY_TOPIC_PICDOWN);
//	printf("*MQTT:MY_TOPIC_PICUP  :%s\r\n",MY_TOPIC_PICUP);
//	printf("*MQTT:MY_TOPIC_MSGDOWN:%s\r\n",MY_TOPIC_MSGDOWN);
//	printf("*MQTT:MY_TOPIC_MSGUP  :%s\r\n",MY_TOPIC_MSGUP);
//	printf("*MQTT:MY_TOPIC_CONFIG :%s\r\n",MY_TOPIC_CONFIG);
}


/**
  * @brief  �����������һ����������
  * @param  pTopic ��Ϣ���⣬����
  * @param  pMessage ��Ϣ���ݣ�����
  * @retval С��0��ʾ������Ϣʧ��
  */
int mqtt_connect_t(uint8_t *buf, unsigned short buflen)
{
	int32_t len, rc;
	uint16_t time;
	uint16_t receivelength = 0;
	OS_ERR err;
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	//
	data.clientID.cstring = CLIENTID;
	//���ӱ���ʱ��,��
	data.keepAliveInterval = 255;
	//����Ự��־��1��ÿ�����ߺ�����¶��ģ�0��ÿ�����ߺ����½���
	/*����Ự��־��ʾ������ǰ�����Ӽ�¼���ֹ�ϵ�������ǰ�ж�����Ϣ����Ҫ���¶���*/
	data.cleansession = 1;
	//��¼�û���
	data.username.cstring = C_USERNAME;
	//��¼����
	data.password.cstring = C_PASSWORD;
	if(transport_open()==0)  // ��������
		return 0;
	OSTimeDly(200, OS_OPT_TIME_DLY, &err);

	len = MQTTSerialize_connect(buf, buflen, &data); /* 1 */
	rc = transport_sendPacketBuffer(buf, len);

	return rc;
}
int mqtt_connect(void)
{
	return mqtt_connect_t(MQTT_SEND_buf, MQTT_SEND_BUFF_MAXLENGTH);
}
/**
 * @description: disconnect the mqtt link
 * @param {type} none
 * @return: 0 ʧ�ܣ� ����ɹ�
 */
int mqtt_disconnect_t(uint8_t *buf, unsigned short buflen)
{
	int32_t len, rc;
	len = MQTTSerialize_disconnect(buf, buflen); /* 3 */
	rc = transport_sendPacketBuffer(buf, len);

	transport_close(); //�رն˿�

	return rc;
}
int mqtt_disconnect(void)
{
	return mqtt_disconnect_t(MQTT_SEND_buf, MQTT_SEND_BUFF_MAXLENGTH);
}
/**
 * @description: PUBLISH һ֡��Ϣ
 * @param *pTopic ���� 
 * @param *pMessage ��Ϣ
 * @param msglen ��Ϣ����
 * @param *buf ������
 * @param buflen ����������
 * @return the length of the sent data.  <= 0 indicates error
 */
int mqtt_publish_t(char *pTopic, char *pMessage, unsigned short msglen, uint8_t *buf, unsigned short buflen)
{
	// mqtt default parameter
	unsigned char dup = 0;
	unsigned char qos = 1;
	unsigned char retained = 0;
	unsigned short packetid = 1;
	// Serialize
	int rc;
	int len;
	MQTTString topicString = MQTTString_initializer;
	// ����
	topicString.cstring = pTopic;
	len = MQTTSerialize_publish(buf, buflen, dup, qos, retained, packetid, topicString, (unsigned char *)pMessage, msglen);
	rc = transport_sendPacketBuffer(buf, len);
	return rc;
}
int mqtt_publish(char *pTopic, char *pMessage, unsigned short msglen)
{
	return mqtt_publish_t(pTopic, pMessage, msglen, MQTT_SEND_buf, MQTT_SEND_BUFF_MAXLENGTH);
}
/**
  * @description �����������һ����Ϣ���ú�������ΪTCP�������ݺ���������
  * @param pTopic ��Ϣ���⣬����
  * @param *buf ������
  * @param buflen ����������
  * @retval С��0��ʾ������Ϣʧ��
  */
int mqtt_subscribe_t(char *pTopic, uint8_t *buf, unsigned short buflen)
{
	// mqtt default parameter
	unsigned char dup = 0;		 // the MQTT dup flag
	unsigned short packetid = 1; // the MQTT packet identifier
	int count = 1;				 // number of members in the topicFilters and reqQos arrays
	int requestedQoSs = 0;		 // array of requested QoS
	// Serialize
	int len = 0;
	int rc = 0;
	MQTTString topicString = MQTTString_initializer;
	// ����
	topicString.cstring = pTopic;
	// ����Ҫ���͵Ķ��������������л���buf����׼������
	len = MQTTSerialize_subscribe(buf, buflen, dup, packetid, count, &topicString, &requestedQoSs);
	// �����л������ݷ��ͳ�ȥ
	rc = transport_sendPacketBuffer(buf, len);
	return rc;
}
int mqtt_subscribe(char *pTopic)
{
	return mqtt_subscribe_t(pTopic, MQTT_SEND_buf, MQTT_DATA_BUFF_MAXLENGTH);
}
/**
 * @description: ping mqtt server
 * @param {type} 
 * @return: 
 */
int mqtt_Ping(void)
{
	int rc = 0;
	int len;
	unsigned char buf[200];
	len = MQTTSerialize_pingreq(buf, 200);
	rc = transport_sendPacketBuffer(buf, len);
	return rc;
}
