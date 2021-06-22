/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * �ļ���  ��main.c
 * ����    ��        
 * Ӳ�����ӣ�
 * �Ƕȴ�����ģ��: VCC -> 5V; GND -> GND; DO -> PB6;
 * LED��   LED -> PC13
 * ���������������Ƕȴ�����ģ��DO�ڸߵ͵�ƽ��
 *           ���Ƕȸ�����ֵʱָʾ���������Ƕȵ�����ֵʱָʾ����ָʾ�ƿ��滻Ϊ�̵�����
 * �Ա�    ��https://ilovemcu.taobao.com
**********************************************************************************/

#include "stm32f10x.h"
#include "delay.h"

/**************************************************************************************
 * ��  �� : GPIO��ʼ������
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable the GPIO  Clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC , ENABLE); 						 		
	
  GPIO_DeInit(GPIOB);	 //������GPIOB�Ĵ�������Ϊȱʡֵ
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    //��������   
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC , GPIO_Pin_13);   //��ʼ״̬��Ϩ��ָʾ��
}

/**************************************************************************************
 * ��  �� : ���Ƕȸ�����ֵʱָʾ���������Ƕȵ�����ֵʱָʾ����ָʾ�ƿ��滻Ϊ�̵�����
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void LiquidSensor_Scan(void)
{
	 if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)           //�Ƕȴ�����������PB6Ϊ�͵�ƽ
	  {
	    delay_ms(10);                                            //��ʱ������
	    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)	       //�Ƕȴ�����������PB6Ϊ�͵�ƽ
	    {
		    GPIO_ResetBits(GPIOC , GPIO_Pin_13);	                   //����LED2
	    }
	  }
		else
		{
		 	    GPIO_SetBits(GPIOC , GPIO_Pin_13);                      //Ϩ��LED2
		}
}

/**************************************************************************************
 * ��  �� : MAIN����
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
int main(void)
{

	SystemInit();		            	//����ϵͳʱ��72MHZ
	GPIO_Configuration();         //GPIO�ڳ�ʼ��
	
  while(1)
  {
		LiquidSensor_Scan();         //�Ƕȴ�������⣬�������û�ָʾ��LED2���� 
	}
}
/*********************************END FILE********************************************/
