#include "esp8266.h"
#include "string.h"
#include "stm32f1xx_hal.h"

#define 	 WIFI_NAME   	"mango"       // wifi��
#define 	 WIFI_PASS		"12345687"		// wifi����
			 
#define 	 SERVER_IP    "192.168.234.108"  
#define 	 SERVER_PORT	"8080"		


uint8_t  esp_buff[ESPBUFF_MAX_SIZE];
uint16_t esp_cnt = 0;


extern DMA_HandleTypeDef hdma_usart2_rx;


	
void ESP8266_Init(void)
{
    HAL_UART_Receive_DMA(&huart2, esp_buff, ESPBUFF_MAX_SIZE);  // ����DMA����
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);  // �������ڵĿ����ж�
		while(ESP8266_SendCmd("AT\r\n","OK")){}
    while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK")){}
    // ����wifi�ȵ�
    while (ESP8266_SendCmd("AT+CWJAP=\""WIFI_NAME"\",\""WIFI_PASS"\"\r\n", "OK")){}
    // ���ӷ�����

    while (ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\""SERVER_IP"\","SERVER_PORT"\r\n", "CONNECT")){}

    // ����
    printf("success\r\n");
}


void ESP8266_Clear()
{
	memset(esp_buff,0,sizeof(esp_buff));
	esp_cnt = 0;
}

void ESP8266_SendString(uint8_t *str,uint8_t len)
{
	uint8_t i=0;
	for(i=0;i<len;i++)
	{
		USART2->DR = *str;
		str++;
		HAL_Delay(1);
	}
}




uint8_t ESP8266_SendCmd(uint8_t *cmd,uint8_t *res)
{
	uint8_t num = 200;
	ESP8266_Clear();
	ESP8266_SendString(cmd,strlen((const char *)cmd));
	while(num--)
	{
		if(strstr((const char*)esp_buff,(const char *)res)!=NULL)
		{
			ESP8266_Clear();
			return 0;
		}
		HAL_Delay(10);
	}
	return 1;
}

void ESP8266_SendDataToServer(uint8_t *data, uint16_t len)
{
    char cmd[50];
    sprintf(cmd, "AT+CIPSEND=%d\r\n", len);
    
    // ����AT+CIPSENDָ�����ESP8266Ҫ���͵����ݳ���
    while (ESP8266_SendCmd(cmd, ">")) {}  // �ȴ�ESP8266����">"��ʾ���Է���������
    
    // ��������
    ESP8266_SendString(data, len);
    
    // �ȴ����ݷ������
    //HAL_Delay(1000);  // ����ʵ����������ӳ�ʱ��
}

