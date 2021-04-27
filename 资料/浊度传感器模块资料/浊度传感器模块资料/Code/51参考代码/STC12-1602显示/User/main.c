/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * �ļ���  ��main.c
 * ����    ��        
 * Ӳ�����ӣ�
 * LCD1602:RS -> P2.0; RW -> P2.1; E -> P2.2;
 *         D0~D7 -> P0��
 * �Ƕ�ģ��: VCC -> 5V; GND -> GND; AO -> P1.2;
 * ����:   key1 -> P2.4; key2 -> P2.5; key3 -> P2.6;
 * LED��   LED -> P1.4
 * ��������������TUֵҺ����ʾ��
             ���ڽ��ղ������õ�TUֵ��������9600����
 *           ��������λ����ʾTUֵ��
 * �Ա�    ��https://ilovemcu.taobao.com
**********************************************************************************/
#include <STC12C5A60S2.h>
#include <intrins.h>
#include <lcd.h>
#include "DS18B20.h"


#define	uchar	unsigned char
#define uint	unsigned int

#define MAIN_Fosc		11059200L	//������ʱ��, ģ�⴮�ںͺ���ʱ���Զ���Ӧ��5~35MHZ  22118400L 
 
#define ADC_OFF()	ADC_CONTR = 0
#define ADC_ON		(1 << 7)
#define ADC_90T		(3 << 5)
#define ADC_180T	(2 << 5)
#define ADC_360T	(1 << 5)
#define ADC_540T	0
#define ADC_FLAG	(1 << 4)	//�����0
#define ADC_START	(1 << 3)	//�Զ���0
#define ADC_CH0		0
#define ADC_CH1		1
#define ADC_CH2		2
#define ADC_CH3		3
#define ADC_CH4		4
#define ADC_CH5		5
#define ADC_CH6		6
#define ADC_CH7		7

sbit k1 = P2^4;
sbit k2 = P2^5;
sbit k3 = P2^6;
sbit ph_led = P1^4;


/**************��������***************************/
float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
unsigned char disbuff[13]={0}; 
float temp_data=0.0;
float K_Value=3047.19;


void  DelayUs(unsigned int time);   //��ʱ�Ӻ���
void  Delay1(unsigned int time);		 //��ʱ������
uint	adc10_start(uchar channel);	//channel = 0~7
int s1;


/*
 * ��������PH_Value_Conversion
 * ����  ����ȡTUģ�������ѹֵ
 *         
 * ����  ����
 * ���  ����
 * ˵��  ��
 */
void TU_Value_Conversion()
{
		TU=(TU*5.0)/1024;
		TU_calibration=-0.0192*(temp_data/10-25)+TU;  
	  TU_value=-865.68*TU_calibration + K_Value;
	
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}

		
		/*��ʾ����*/
		disbuff[0]=(int)(TU_value)/1000+'0';
		disbuff[1]=(int)(TU_value)%1000/100+'0';	
		disbuff[2]=(int)(TU_value)%100/10+'0';
		disbuff[3]=(int)(TU_value)%10+'0';
		
		DisplaySingleChar(0x03,0,disbuff[0]);
		DisplaySingleChar(0x04,0,disbuff[1]);
		DisplaySingleChar(0x05,0,disbuff[2]);
		DisplaySingleChar(0x06,0,disbuff[3]);		
}
void TEMP_Value_Conversion()
{
	  disbuff[4]=(int)(temp_data)/1000+'0';
	  disbuff[5]=(int)(temp_data)%1000/100+'0';	
	  disbuff[6]=(int)(temp_data)%100/10+'0';
	  disbuff[7]='.';
	  disbuff[8]=(int)(temp_data)%10+'0';

		DisplaySingleChar(0x03,1,disbuff[4]);
		DisplaySingleChar(0x04,1,disbuff[5]);
		DisplaySingleChar(0x05,1,disbuff[6]);
		DisplaySingleChar(0x06,1,disbuff[7]);
		DisplaySingleChar(0x07,1,disbuff[8]);	
}
/*
 * ��������PH_Value_Conversion
 * ����  ����ȡTUģ�������ѹֵ
 *         
 * ����  ����
 * ���  ����
 * ˵��  ��
 */
/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 */
void main(void)
{
	 
	LCDInit(); //��ʾ����ʼ��
  Init_Ds18b20();	
	DisplayString(0x0,0,"  Welcome...  ");
	DisplayString(0x0,1,"water quality");
	Delay1(500);
	WriteCmd(LCD_CLS);
	P1ASF = 0x07;			//12C5A60AD/S2ϵ��ģ������(AD)ѡ��
	ADC_CONTR = ADC_360T | ADC_ON;
	DisplayString(0x0,0,"TU:");
	DisplayString(0x0,1,"T:");
	DisplaySingleChar(0x08,0,'p');
	DisplaySingleChar(0x09,0,'p');
	DisplaySingleChar(0x0A,0,'m');
	while(1)
	{
		temp_data=Get_temp();
		//TU�ɼ�ת��
		TU = adc10_start(2);
    Delay1(1000);		
		TU_Value_Conversion();
		TEMP_Value_Conversion();
						
	}
}
	

///********************* ��һ��ADCת�� *******************/
uint	adc10_start(uchar channel)	//channel = 0~7
{
	uint	adc;
	uchar	i;

	ADC_RES = 0;
	ADC_RESL = 0;

	ADC_CONTR = (ADC_CONTR & 0xe0) | ADC_START | channel; 

	i = 250;
	do{
		if(ADC_CONTR & ADC_FLAG)
		{
			ADC_CONTR &= ~ADC_FLAG;
			adc = (uint)ADC_RES;
			adc = (adc << 2) | (ADC_RESL & 3);
			return	adc;
		}
	}while(--i);
	return	1024;
}


//========================================================================
//��ʱ����//
void Delay1(unsigned int time)
{
    unsigned int timeCounter = 0;
	for (timeCounter = time;timeCounter > 0 ;timeCounter --)
		DelayUs(255);
}

void DelayUs(unsigned int time)
{
	unsigned int timeCounter = 0;
	for (timeCounter = 0;timeCounter < time;timeCounter ++)
		_nop_();
}



