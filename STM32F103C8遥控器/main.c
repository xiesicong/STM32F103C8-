#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "adc.h"
#include "nvic.h"
#include "timing_trigger.h"
#include "nRF24L01_API.h"


/*
ADC读取，然后通过nrf发送，ADC的读取使用过TIM计时触发的，每20ms一次，然后DMA从ADC数据寄存器将数据搬到ADC_value数组中
正常情况下LED2会疯狂闪烁
按键暂时没用
四个通道的数据直接发送出去，接收端可用NRF转TTL模块查看数据，记得开启HEX显示和自动换行，这样易于观察。
*/


#define NRF_IRQ   		PBin(1)  //IRQ主机数据输入,上拉输入


uchar rece_buf[32];
uchar key_value;

extern uchar send_buff[32];
extern uchar flag;

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Set the Vector Table base location at 0x08000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   


	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the EXTI0 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



void SPI_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_Configuration();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE); //使能时钟

	//SPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  /*GPIO口初始化*/
	//SPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  /*GPIO口初始化*/
	//MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  /*GPIO口初始化*/
	//IRQ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  /*GPIO口初始化*/
}


int main(void)
{	
	SPI_GPIO_Init();
	delay_init();
	LED_Init();
	KEY_Init();
	ADC_Config();
	timing_trigger_init();
	NVIC_config();                      //中断配置初始化.
	
	
	LED1 = 1;
	LED2 = 1;
	
	
	while(NRF24L01_Check()); // 等待检测到NRF24L01，程序才会向下执行
	NRF24L01_RT_Init();	
//	LED2 = 0;

	while(1)
	{
		if(NRF_IRQ==0)	 	// 如果无线模块接收到数据
		{		
			if(NRF24L01_RxPacket(rece_buf)==0)
			{			   
				if(	rece_buf[1]=='1')		 //第1位以后是收到的命令数据，rece_buf[0]是数据位数长度
					LED1=~LED1;
			}
		}
//		if(1 == KEY_Scan(0))
//		{
//			LED1=~LED1;
//			rece_buf[0] = 3;
//			rece_buf[1]= 0x60;			//key
//			rece_buf[2]= '\r';					//  \n
//			rece_buf[3]= '\n';					//	\r
//			SEND_BUF(rece_buf);
//		}
		if(flag == 1)
		{
			SEND_BUF(send_buff);
			flag = 0;
			send_buff[0] = 0;
		}
	}
}

