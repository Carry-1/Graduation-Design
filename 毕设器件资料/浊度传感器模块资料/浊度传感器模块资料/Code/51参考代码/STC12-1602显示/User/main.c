/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * 文件名  ：main.c
 * 描述    ：        
 * 硬件连接：
 * LCD1602:RS -> P2.0; RW -> P2.1; E -> P2.2;
 *         D0~D7 -> P0口
 * 浊度模块: VCC -> 5V; GND -> GND; AO -> P1.2;
 * 按键:   key1 -> P2.4; key2 -> P2.5; key3 -> P2.6;
 * LED：   LED -> P1.4
 * 功能描述：测量TU值液晶显示；
             串口接收测量所得的TU值（波特率9600）；
 *           可连接上位机显示TU值；
 * 淘宝    ：https://ilovemcu.taobao.com
**********************************************************************************/
#include <STC12C5A60S2.h>
#include <intrins.h>
#include <lcd.h>
#include "DS18B20.h"


#define	uchar	unsigned char
#define uint	unsigned int

#define MAIN_Fosc		11059200L	//定义主时钟, 模拟串口和和延时会自动适应。5~35MHZ  22118400L 
 
#define ADC_OFF()	ADC_CONTR = 0
#define ADC_ON		(1 << 7)
#define ADC_90T		(3 << 5)
#define ADC_180T	(2 << 5)
#define ADC_360T	(1 << 5)
#define ADC_540T	0
#define ADC_FLAG	(1 << 4)	//软件清0
#define ADC_START	(1 << 3)	//自动清0
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


/**************变量定义***************************/
float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
unsigned char disbuff[13]={0}; 
float temp_data=0.0;
float K_Value=3047.19;


void  DelayUs(unsigned int time);   //延时子函数
void  Delay1(unsigned int time);		 //延时主程序
uint	adc10_start(uchar channel);	//channel = 0~7
int s1;


/*
 * 函数名：PH_Value_Conversion
 * 描述  ：获取TU模块输出电压值
 *         
 * 输入  ：无
 * 输出  ：无
 * 说明  ：
 */
void TU_Value_Conversion()
{
		TU=(TU*5.0)/1024;
		TU_calibration=-0.0192*(temp_data/10-25)+TU;  
	  TU_value=-865.68*TU_calibration + K_Value;
	
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}

		
		/*显示酸碱度*/
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
 * 函数名：PH_Value_Conversion
 * 描述  ：获取TU模块输出电压值
 *         
 * 输入  ：无
 * 输出  ：无
 * 说明  ：
 */
/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 */
void main(void)
{
	 
	LCDInit(); //显示屏初始化
  Init_Ds18b20();	
	DisplayString(0x0,0,"  Welcome...  ");
	DisplayString(0x0,1,"water quality");
	Delay1(500);
	WriteCmd(LCD_CLS);
	P1ASF = 0x07;			//12C5A60AD/S2系列模拟输入(AD)选择
	ADC_CONTR = ADC_360T | ADC_ON;
	DisplayString(0x0,0,"TU:");
	DisplayString(0x0,1,"T:");
	DisplaySingleChar(0x08,0,'p');
	DisplaySingleChar(0x09,0,'p');
	DisplaySingleChar(0x0A,0,'m');
	while(1)
	{
		temp_data=Get_temp();
		//TU采集转换
		TU = adc10_start(2);
    Delay1(1000);		
		TU_Value_Conversion();
		TEMP_Value_Conversion();
						
	}
}
	

///********************* 做一次ADC转换 *******************/
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
//延时函数//
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



