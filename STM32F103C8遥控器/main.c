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
ADC��ȡ��Ȼ��ͨ��nrf���ͣ�ADC�Ķ�ȡʹ�ù�TIM��ʱ�����ģ�ÿ20msһ�Σ�Ȼ��DMA��ADC���ݼĴ��������ݰᵽADC_value������
���������LED2������˸
������ʱû��
�ĸ�ͨ��������ֱ�ӷ��ͳ�ȥ�����ն˿���NRFתTTLģ��鿴���ݣ��ǵÿ���HEX��ʾ���Զ����У��������ڹ۲졣
*/


#define NRF_IRQ   		PBin(1)  //IRQ������������,��������


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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE); //ʹ��ʱ��

	//SPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  /*GPIO�ڳ�ʼ��*/
	//SPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  /*GPIO�ڳ�ʼ��*/
	//MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  /*GPIO�ڳ�ʼ��*/
	//IRQ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  /*GPIO�ڳ�ʼ��*/
}


int main(void)
{	
	SPI_GPIO_Init();
	delay_init();
	LED_Init();
	KEY_Init();
	ADC_Config();
	timing_trigger_init();
	NVIC_config();                      //�ж����ó�ʼ��.
	
	
	LED1 = 1;
	LED2 = 1;
	
	
	while(NRF24L01_Check()); // �ȴ���⵽NRF24L01������Ż�����ִ��
	NRF24L01_RT_Init();	
//	LED2 = 0;

	while(1)
	{
		if(NRF_IRQ==0)	 	// �������ģ����յ�����
		{		
			if(NRF24L01_RxPacket(rece_buf)==0)
			{			   
				if(	rece_buf[1]=='1')		 //��1λ�Ժ����յ����������ݣ�rece_buf[0]������λ������
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

