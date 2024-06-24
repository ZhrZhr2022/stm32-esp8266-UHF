#include "ali_yun.h"
#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

//======================wifi======================
extern uint8_t  esp_buff[ESPBUFF_MAX_SIZE];
extern uint16_t esp_cnt;
// ================================================


//=====================数据=======================
extern double temp_value;
extern double humi_value;
//================================================


// 设置阿里账号,上云
void Ali_Yun_Init(void)
{
	//设置用户名，密码
	while(ESP8266_SendCmd("AT+MQTTUSERCFG=0,1,\"NULL\",\""ALI_USERNAME"\",\""ALI_PASSWD"\",0,0,\"\"\r\n","OK")){}
	HAL_Delay(10);
	// 设置客服id
	while(ESP8266_SendCmd("AT+MQTTCLIENTID=0,\""ALICLIENTLD"\"\r\n","OK")){}
		
	// 连接腾讯云  AT+MQTTCONN=0,"iot-06z00b28nanp9ew.mqtt.iothub.aliyuncs.com",1883,1
	while(ESP8266_SendCmd("AT+MQTTCONN=0,\""ALI_MQTT_HOSTURL"\",1883,1\r\n","OK")){}
		
	Ali_Yun_Topic();
}

void Ali_Yun_Topic(void)
{
	//"AT+MQTTPUB=0,\"发布的主题\",\"";
	while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_SET"\",0\r\n","OK")){}
		
	while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_POST"\",0\r\n","OK")){}
}





//阿里云数据上传
void Ali_Yun_Send(void)
{
	uint8_t msg_buf[1024];
	uint8_t params_buf[1024];
	uint8_t data_value_buf[24];
	uint16_t move_num = 0;
	cJSON *send_cjson = NULL;
//	char str[1024];
//	char *str = (char*)malloc(1024);
	char *str = NULL;
	int i=0;
	
	printf("str = %p\r\n",&str);
	
	cJSON *params_cjson = NULL;
//	memset(str,0,sizeof(str));
	memset(msg_buf,0,sizeof(msg_buf));
	memset(params_buf,0,sizeof(params_buf));
	memset(data_value_buf,0,sizeof(data_value_buf));
	// "{\\\"params\\\":{\\\"temperature\\\":%f\\,\\\"Humidity\\\":%f\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

	send_cjson = cJSON_CreateObject();   // 创建cjson
	
	// 构建发送的json
	params_cjson = cJSON_CreateObject();
	
//============================================== 发送的数据================================================
//	cJSON_AddStringToObject(params_cjson, "temperature", "China");
	printf("cjson发送数据 temp_value = %f\r\n",temp_value);
	printf("cjson发送数据 humi_value = %f\r\n",humi_value);
	
//	memset(data_value_buf,0,sizeof(data_value_buf));
//	snprintf((char *)data_value_buf,sizeof(temp_value),"%1.3f",temp_value++);	
//	cJSON_AddItemToObject(params_cjson,"temperature",cJSON_CreateString((char *)data_value_buf));
//	
//	memset(data_value_buf,0,sizeof(data_value_buf));
//	snprintf((char *)data_value_buf,sizeof(humi_value),"%1.3f",humi_value++);	
//	cJSON_AddItemToObject(params_cjson,"Humidity",cJSON_CreateString((char *)data_value_buf));
	
	cJSON_AddNumberToObject(params_cjson,"temperature",temp_value++);
	cJSON_AddNumberToObject(params_cjson,"Humidity",humi_value++);
	
//============================================== 发送的数据================================================
	// 加入主的json数据中
	cJSON_AddItemToObject(send_cjson, "params", params_cjson);
	cJSON_AddItemToObject(send_cjson,"version",cJSON_CreateString("1.0.0"));
	
	
	
	str = cJSON_PrintUnformatted(send_cjson);
	
	printf("json格式 = %s\r\n",str);
	
	// 加转义字符
	for(i=0;*str!='\0';i++)
	{
		params_buf[i] = *str;
		if(*(str+1)=='"'||*(str+1)==',')
		{
			params_buf[++i] = '\\';
		}
		str++;
		move_num++;
//		free(str++);
		
	}
	str = str - move_num;
	printf("params_buf = %s\r\n",params_buf);
	
	// 整理所有数据
	sprintf((char *)msg_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST"\",\"%s\",0,0\r\n",params_buf);
	
	printf("开始发送数据:%s\r\n",msg_buf);
	ESP8266_SendCmd(msg_buf,"OK");

	
	ESP8266_Clear();
	cJSON_Delete(send_cjson);
	if(str!=NULL){
		free(str);
		str = NULL;
		printf("释放str空间成功\r\n");
	}
		
//	cJSON_free(send_cjson);
//	cJSON_Delete(params_cjson);
	
//	free(send_cjson);
	
	

	

}

uint8_t cjson_err_num = 0;  //cjson 解析错误的次数
void Ali_Yun_GetRCV(void)
{
	cJSON *cjson = NULL;
	int num;
	char topic_buff[256];
	char recv_buffer[ESPBUFF_MAX_SIZE];
	char *ptr_recv = strstr((const char *)esp_buff,"+MQTTSUBRECV");
	
	
	// "/sys/ix25oHiHCSl/stm32/thing/service/property/set"

	if(ptr_recv!=NULL)  // 存在
	{
		memset(topic_buff,0,sizeof(topic_buff));
		sscanf((char *)esp_buff,"+MQTTSUBRECV:0,%[^,],%d,%s",topic_buff,&num,recv_buffer);

		if(strstr(topic_buff,ALI_TOPIC_SET))      // 判断主题
		{
			printf("========================数据解析开始===========================\r\n");
			
			printf("接收数据成功，开始解析  %s\r\n",recv_buffer);
			cjson = cJSON_Parse(recv_buffer);
			
			
			if(cjson==NULL)
			{
				printf("cjson 解析错误\r\n");
				cjson_err_num++;
				if(cjson_err_num>3){
					ESP8266_Clear();
					cjson_err_num = 0;
				}			
				printf("========================数据解析失败===========================\r\n");
			}
			else
			{
				cJSON *json_data = NULL;
				json_data = cJSON_GetObjectItem(cjson,"params");
				cjson_err_num = 0;
				if(json_data==NULL){
					printf("cjson  没有数据\r\n");
					return;
				}else
				{
					printf("cjson 内存大小为 = %d\r\n",sizeof(cjson));
//					printf("数据接收：%s\r\n",esp_buff);
					// ====================================解析数据=========================================
					if(cJSON_GetObjectItem(json_data,"temperature")!=NULL)
					{
						temp_value = cJSON_GetObjectItem(json_data,"temperature")->valuedouble;
						printf("csjon解析成功 temp_value = %f\r\n",temp_value);
					}
					
					if(cJSON_GetObjectItem(json_data,"Humidity")!=NULL)
					{
						humi_value = cJSON_GetObjectItem(json_data,"Humidity")->valuedouble;
						printf("csjon解析成功 Humidity = %f\r\n",humi_value);
					}
					
					//======================================================================================
				}
				
				ESP8266_Clear();
				cJSON_Delete(cjson);
				printf("========================数据解析成功===========================\r\n");
				
			}
			
			
			
			
			
		}
		
		
		
		

	}

	
	
}
