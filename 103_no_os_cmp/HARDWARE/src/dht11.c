#include "dht11.h"
extern UART_HandleTypeDef huart1;
/**
  * @brief  温湿度传感器主函数
  * @param  uint8_t
  * @retval 0成功 1失败
  */
uint8_t DHT11(float *temp,float *humi)
{
    if(DHT11_READ_DATA(temp,humi) == 1)
    {
//        printf("数据校验成功!\r\n");
				return 0;
    }
    else
    {
//        printf("DHT11没有应答，请检查传感器!\r\n");
			  return 1;
    }
                       
  
}
 
/**
  * @brief  温湿度传感器启动信号发送
  * @param  void
  * @retval None
  */
void DHT11_START(void)
{
    DHT11_GPIO_MODE_SET(0);                         //  主机设置为输出模式
    
    DHT11_PIN_RESET;                                //  主机拉低电平
    
    HAL_Delay(20);                                  //  主机等待 18 < ms > 30
    
    DHT11_GPIO_MODE_SET(1);                         //  主机设置为输入模式，等待DHT11答应
}                                                   //  因为设置了上拉输入，GPIO -> 1
 
/**
  * @brief  读取一位数据 1bit
  * @param  void
  * @retval 0/1
  */
unsigned char DHT11_READ_BIT(void)
{
    while(!DHT11_READ_IO);                          //  过度数据的低电平 
    
    Coarse_delay_us(40);
 
    if(DHT11_READ_IO)                               //  此时如果还为高电平则数据为 1
    {
        while(DHT11_READ_IO);                       //  过度数据的高电平
        return 1;
    }   
    else                                            //  若此时为低则为 0
    {
        return 0;
    }
}
 
/**
  * @brief  读取一个字节数据 1byte / 8bit
  * @param  void
  * @retval temp
  */
unsigned char DHT11_READ_BYTE(void)
{
    uint8_t i,temp = 0;                             //  暂时存储数据
    
    for(i=0; i<8 ;i++)
    {
        temp <<= 1;                                 
        if(DHT11_READ_BIT())                        //  1byte -> 8bit
        {
            temp |= 1;                              //  0000 0001
        }
    }
    return temp;
}
 
/**
  * @brief  读取温湿度传感器数据 5byte / 40bit
  * @param  void
  * @retval 0/1/2
  */
unsigned char DHT11_READ_DATA(float *temp,float *humi)
{
    uint8_t i;
    uint8_t data[5] = {0};
    
    DHT11_START();                                  //  主机发送启动信号
    
    if(DHT11_Check())                               //  如果DHT11应答     
    {  
        while(!DHT11_READ_IO);                      //  过度DHT11答复信号的低电平
        while(DHT11_READ_IO);                       //  过度DHT11答复信号的高电平
        
        for(i=0; i<5; i++)
        {                        
            data[i] = DHT11_READ_BYTE();            //  读取 5byte
        }
        
        if(data[0] + data[1] + data[2] + data[3] == data[4])
        {
					  *humi = data[0] + data[1]/100;
						*temp = data[2] + data[3]/100;
//						printf("tmp = %.2f   humi = %.2f  =====================\r\n",*temp,*humi);
//            printf("当前湿度：%d.%d%%RH当前温度：%d.%d°C--\r\n",data[0],data[1],data[2],data[3]);
            return 1;                               //  数据校验通过
        }
        else
        {
            return 0;                               //  数据校验失败
        }
    }
    else                                            //  如果DHT11不应答
    {
        return 2;
    }
}
 
/**
  * @brief  检测温湿度传感器是否存在(检测DHT11的应答信号)
  * @param  void
  * @retval 0/1
  */
unsigned char DHT11_Check(void)
{
 
    Coarse_delay_us(40);
    if(DHT11_READ_IO == 0)                          //  检测到DHT11应答
    {
        return 1;
    }
    else                                            //  检测到DHT11不应答
    {
        return 0;
    }
}
 
/**
  * @brief  设置引脚模式
  * @param  mode: 0->out, 1->in
  * @retval None
  */
static void DHT11_GPIO_MODE_SET(uint8_t mode)
{
    if(mode)
    {
        /*  输入  */
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = GPIO_PIN_6;                   //  6号引脚
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;             //  输入模式
        GPIO_InitStruct.Pull = GPIO_PULLUP;                 //  上拉输入
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else 
    {
        /*  输出  */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Pin = GPIO_PIN_6;                //  6号引脚
        GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;      //  Push Pull 推挽输出模式
        GPIO_InitStructure.Pull = GPIO_PULLUP;              //  上拉输出
        GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;    //  高速
        HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
    }
}
 
/**
  * @brief  程序延时 us , 必须在 72M 主频下使用
  * @param  us: <= 4294967295
  * @retval None
  */
void Coarse_delay_us(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--)
	{
		;
	}
}
