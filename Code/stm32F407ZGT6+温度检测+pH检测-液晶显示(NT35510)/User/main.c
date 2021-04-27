/**
  ******************************************************************************
  * @file    main.c
  * @author  Xia Qingsheng
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   �����¶ȡ�pHֵ����Һ����ʾ����ʾ
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  STM32 F407 ������ 
  * 
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./led/bsp_led.h"  
#include "./lcd/bsp_nt35510_lcd.h"
#include "./dwt_delay/core_delay.h" 
#include "DS18B20/bsp_ds18b20.h"
#include "systick/bsp_SysTick.h"
#include "math.h"
#include "./adc/bsp_adc.h"  

float PH_Value=0;
unsigned char  Tx[20];   //Tx�������ڻ���pHֵ
// ADC1ת���ĵ�ѹֵͨ��DMA��ʽ����SRAM
__IO uint16_t ADC_ConvertedValue = 8;
// �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
float ADC_ConvertedValueLocal;  

/* Һ����ʾ������ʾ������ */
char dis_buf[1024];


/**
  * @brief  ������
  * @param  ��  
  * @retval ��
  */
int main ( void )
{
  float temperature;    //����¶ȼ��ֵ
	uint8_t uc, ucDs18b20Id [ 8 ];   //���DS18B20���к�
  uint8_t DS18B20Id_str[20];
	
	/* ����SysTick Ϊ1us�ж�һ�� */
	SysTick_Init();          //ϵͳ��ʱ����ʼ��
	
  NT35510_Init ();         //LCD ��ʼ��

	// Debug_USART_Config();		 //USART��ʼ��, ���ô��ڵ���������ʾ�¶�ʱ��Ҫ�õ�USART����Һ����ʾʱ����Ҫ�õ�USART
	Rheostat_Init();	       // ADC��ʼ��
/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();
	
	

  //����0��3��5��6 ģʽ�ʺϴ���������ʾ���֣�
  //���Ƽ�ʹ������ģʽ��ʾ����	����ģʽ��ʾ���ֻ��о���Ч��			
  //���� 6 ģʽΪ�󲿷�Һ�����̵�Ĭ����ʾ����  
  NT35510_GramScan ( 6 );

  NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */
  
	
	NT35510_DispStringLine_EN(LINE(0),"1. ds18b20 test");
/*��ʾDS18B20�����к�*/
//	while( DS18B20_Init() )	
//		NT35510_DispStringLine_EN(LINE(1),"no ds18b20 exit");
//	
//	NT35510_DispStringLine_EN(LINE(1),"ds18b20 ok");

//	DS18B20_ReadId ( ucDs18b20Id  );           // ��ȡ DS18B20 �����к�
//	
//	for ( uc = 0; uc < 8; uc++ )             // ��ӡ DS18B20 �����к�
//  {    
//    sprintf((char *)&DS18B20Id_str[2*uc], "%.2x",ucDs18b20Id[uc]);  
//    
//    if(uc == 7)
//      DS18B20Id_str[17] = '\0';        
//  }

//  sprintf((char*)dis_buf,"DS18B20 serial num:0x%s",DS18B20Id_str);  
//  NT35510_DispStringLine_EN(LINE(2),dis_buf);
	
	for(;;) 
	{	
		//1.ѭ������¶Ȳ���ʾ
		temperature=DS18B20_Get_Temp();
        
    sprintf((char*)dis_buf,"T:%0.3f degree Celsius",temperature);
    NT35510_DispStringLine_EN(LINE(1),dis_buf);
		
		//2.ѭ�����pH����ʾ
		
		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // ��ȡת����ADֵ
		
		PH_Value=-5.7541*ADC_ConvertedValueLocal+16.654;
		
			
		if(PH_Value<=0.0){PH_Value=0.0;}
		if(PH_Value>=14.0){PH_Value=14.0;}
	
		  /*��ʾ��ѹ*/
		Tx[0]=(int)(PH_Value*100)/1000+'0';	
		Tx[1]=(int)(PH_Value*100)%1000/100+'0';
		Tx[2]='.';
		Tx[3]=(int)(PH_Value*100)%100/10+'0';
		Tx[4]=(int)(PH_Value*100)%10+'0';
		Tx[5]='\0';   //�ַ���������
		NT35510_DispStringLine_EN(LINE(3),"2. pH test ");
		sprintf((char*)dis_buf,"pH:%s",Tx);
    NT35510_DispStringLine_EN(LINE(4),dis_buf);
		
	  

     //��ʱ
    CPU_TS_Tmr_Delay_MS(1000);
	}	 
}

/*********************************************END OF FILE**********************/

