/**
  ******************************************************************************
  * @file    main.c
  * @author  Xia Qingsheng
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   �����¶ȡ�pHֵ����Һ����ʾ����ʾ
	* Ӳ�����ӣ�
  * NT35510:
  * �¶ȴ�����ģ��: VCC -> 3.3V; GND -> GND; DO -> PE3;
	*
	* PH������ģ��: VCC -> 5V; GND -> GND; PO -> PB0;
  *
	* EC������ģ��: VCC -> 5V; GND -> GND; AO -> PB1;
  *
	* ���Ƕȴ�����ģ��: VCC -> 5V; GND -> GND; AO -> PA6;

  *
  * ��������������ECֵҺ����ʾ(ADC1��PA2��DMA��ʽ��ȡ)��
             ���ڽ��ղ������õ�ECֵ��������115200����
  *           ��������λ����ʾECֵ��
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

#define RES2 820.0   //�˷ŵ��裬��Ӳ����·�й�
#define ECREF 200.0 //��Ƭ�����ѹ����Ӳ����·���

// ADCת���ĵ�ѹֵͨ��MDA��ʽ����SRAM
extern __IO uint16_t ADC_ConvertedValue[RHEOSTAT_NOFCHANEL];  
// �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
float ADC_ConvertedValueLocal[RHEOSTAT_NOFCHANEL]={0};

/****************pH���ģ���õ��ı���****************/
float PH_Value=0;
unsigned char  Tx_PH[20];   //Tx�������ڻ���pHֵ


/****************�絼�ʼ��ģ���õ��ı���************/
float EC_voltage;
float EC_value=0.0,voltage_value;
float temp_data=250;
float compensationCoefficient=1.0;//�¶�У׼ϵ��
float compensationVolatge;
float kValue=1.0;
float kValue_Low=1;  //У׼ʱ�����޸�
float kValue_High=1; //У׼ʱ�����޸�
float rawEC=0.0;
float EC_valueTemp=0.0;

unsigned char  Tx_EC[20];   //���߷��ͻ���
  
/****************���Ƕȼ��ģ���õ��ı���************/
unsigned char AD_CHANNEL=0;
unsigned long PU_V=0;
u8 tu_temp=0;
u16 tu_result=0;

float compensationVolatge;
float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
float temp_data_TU=0.0;

float K_Value_TU=3047.19;

unsigned char  Tx_TU[20];   //���߷��ͻ���




/****************Һ����ʾģ���õ��ı���************/
/*********************************/
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

	
	for(;;) 
	{	
		//1.ѭ������¶Ȳ���ʾ
		temperature=DS18B20_Get_Temp();   //��ȡ�¶ȣ�����temperature����
        
    sprintf((char*)dis_buf,"T:%0.3f degree Celsius",temperature);   //���¶�ֵ������ʾ������
    NT35510_DispStringLine_EN(LINE(1),dis_buf);    //������ʾ����
		
		//2.ѭ�����pH����ʾ
		
		ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*3.3; // ��ȡת����ADֵ
		
		PH_Value=-5.7541*ADC_ConvertedValueLocal[0]+16.654;
		
			
		if(PH_Value<=0.0){PH_Value=0.0;}
		if(PH_Value>=14.0){PH_Value=14.0;}
	
		  /*��ʾ��ѹ*/
		Tx_PH[0]=(int)(PH_Value*100)/1000+'0';	   //������ת��Ϊ�ַ�
		Tx_PH[1]=(int)(PH_Value*100)%1000/100+'0';
		Tx_PH[2]='.';
		Tx_PH[3]=(int)(PH_Value*100)%100/10+'0';
		Tx_PH[4]=(int)(PH_Value*100)%10+'0';
		Tx_PH[5]='\0';   //�ַ���������
		NT35510_DispStringLine_EN(LINE(3),"2. pH test ");
		sprintf((char*)dis_buf,"pH:%s",Tx_PH);
    NT35510_DispStringLine_EN(LINE(4),dis_buf);
		
		//3.ѭ�����絼�ʲ���ʾ
	  EC_voltage =(float) ADC_ConvertedValue[1]/4096*3300; // ��ȡת����ADֵ
		rawEC = 1000*EC_voltage/RES2/ECREF;
		EC_valueTemp=rawEC*kValue;
		/*First Range:(0,2); Second Range:(2,20)*/
		if(EC_valueTemp>2.0)
		{
			kValue=kValue_High;
		}
		else if(EC_valueTemp<=2.0)
		{
			kValue=kValue_Low;
		}
		EC_value=rawEC*kValue;
		compensationCoefficient=1.0+0.0185*((temp_data/10)-25.0); 
		EC_value=EC_value/compensationCoefficient;
		//if((EC_value<=0)){EC_value=0;}
		//if((EC_value>20)){EC_value=20;}//20mS/cm
		/*��ʾEC*/
		Tx_EC[0]=(int)(EC_value*100)/1000+'0';
		Tx_EC[1]=(int)(EC_value*100)%1000/100+'0';
		Tx_EC[2]='.';
		Tx_EC[3]=(int)(EC_value)%100/10+'0';	
		Tx_EC[4]=(int)(EC_value)%10+'0';
		Tx_EC[5]='\0';   //�ַ���������
		NT35510_DispStringLine_EN(LINE(6),"3. Electrical conductivity(EC)");
		NT35510_DispStringLine_EN(LINE(7),"test");
		sprintf((char*)dis_buf,"EC:%s",Tx_EC);
    NT35510_DispStringLine_EN(LINE(8),dis_buf);
		
		//4.ѭ�������ǶȲ���ʾ
		TU =(float) ADC_ConvertedValue[2]/4096*3.3; // ��ȡת����ADֵ
		TU_calibration=-0.0192*(temp_data/10-25)+TU;  
	  TU_value=-865.68*TU_calibration + K_Value_TU;
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}
	
		
		/*��ʾTDS*/
		Tx_TU[0]=(int)(TU_value)/1000+'0';
		Tx_TU[1]=(int)(TU_value)%1000/100+'0';	
		Tx_TU[2]=(int)(TU_value)%100/10+'0';
		Tx_TU[3]=(int)(TU_value)%10+'0';
		Tx_TU[4]='\0';   //�ַ���������

		NT35510_DispStringLine_EN(LINE(10),"4. Turbidity(TU) test");
		sprintf((char*)dis_buf,"TU:%s",Tx_TU);
    NT35510_DispStringLine_EN(LINE(11),dis_buf);
		
		
     //��ʱ
    CPU_TS_Tmr_Delay_MS(1000);
	}	 
}

/*********************************************END OF FILE**********************/

