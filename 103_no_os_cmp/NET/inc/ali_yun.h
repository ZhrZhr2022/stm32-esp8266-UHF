#ifndef _AL_YUN_H_
#define _AL_YUN_H_

#include "main.h"
#include "cJSON.h"

#define		 ALI_USERNAME		"stm32&ix25oHiHCSl"                                         // �û���
#define		 ALICLIENTLD			"ix25oHiHCSl.stm32|securemode=2\\,signmethod=hmacsha256\\,timestamp=1688993486206|"				// �ͻ�id
#define		 ALI_PASSWD			"08d7d8eb8bf44b45c81bfe0194cdb3b2d6b5ec58accfd115878efb403d0144a9"           // MQTT ����
#define		 ALI_MQTT_HOSTURL	"iot-06z00b28nanp9ew.mqtt.iothub.aliyuncs.com"			// mqtt���ӵ���ַ
#define		 ALI_PORT			"1883"				// �˿�

#define      ALI_TOPIC_SET          "/sys/ix25oHiHCSl/stm32/thing/service/property/set"
#define      ALI_TOPIC_POST         "/sys/ix25oHiHCSl/stm32/thing/event/property/post"


// �����ϴ� ��ʽ
#define JSON_FORMAT      "{\\\"params\\\":{\\\"temperature\\\":%f\\,\\\"Humidity\\\":%f\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

void Ali_Yun_Init(void);

void Ali_Yun_Topic(void);

void Ali_Yun_Send(void);

void Ali_Yun_GetRCV(void);


#endif
