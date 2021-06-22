/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * �ļ���  ��main.c
 * ����    ��        
 * Ӳ�����ӣ�
 * PCF8591:VCC -> 5V; GND -> GND; SCL -> P1.6;SDA -> P1.7; 
 *         OUT1~OUT8 -> P3��
 * LCD1602:RS -> P2.5; RW -> P2.6; E -> P2.7;
 *         D0~D7 -> P0��
 * �Ƕȴ�����ģ��: VCC -> 5V; GND -> GND; AO -> AIN0;
 *
 * ���������������Ƕ�ֵҺ����ʾ��
             ���ڽ��ղ������õ��Ƕ�ֵ��������9600����
 *           ��������λ����ʾ�Ƕ�ֵ��
 * �Ա�    ��https://ilovemcu.taobao.com
**********************************************************************************/

#include <reg52.h>
#include <include.h>

typedef unsigned char uchar;
typedef unsigned char uint;

/**************�ⲿ��������************************/
int num=0; 
/**************��������***************************/
//double PH=0.0;
//unsigned char disbuff[5]={0};
double ADC_Voltage=0.0;
float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
unsigned char disbuff[13]={0}; 
float temp_data=25.0;
float K_Value=3047.19;


/*
 * ��������Get_Voltage_Value
 * ����  ����ȡADC0809��ת����ѹֵ
 *         
 * ����  ����
 * ���  ����
 * ˵��  ��
 *						AIN0(0X40):��������
 *						AIN1(0X41):����
 *					  AIN2(0X42):����������
 *					  AIN3(0X43):��������
 */
void Get_Voltage_Value()
{
			ISendByte(PCF8591,0x40);
      num=IRcvByte(PCF8591);  //ADC0 ģ��ת��1      PH�缫
//		  TU =num * 5.0/ 255;  //ת��Ϊ��ѹֵ
}
/*
 * ��������Character_Conversion
 * ����  ����ʱ����
 *         
 * ����  ��unsigned char n����ʱ��
 * ���  ����
 * ˵��  ��
 */
void TU_Value_Conversion()
{	
		Get_Voltage_Value();
		TU=(num*5.0)/256;
		TU_calibration=-0.0192*(temp_data-25)+TU;  
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
void StartUART( void )
{  							//������9600
     SCON = 0x50;
     TMOD = 0x20;
     TH1 = 0xFD;
     TL1 = 0xFD; 
     PCON = 0x00;
     TR1 = 1;
}
//**********ͨ�����ڽ����յ����ݷ��͸�PC��**************
void R_S_Byte(uchar R_Byte)
{	
	 SBUF = R_Byte;  
     while( TI == 0 );				//��ѯ��
  	 TI = 0;    
}
/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 */
void main(void)
{
	int i;
	LCDInit();
	StartUART();
	
  DisplayString(0x0,0,"  Welcome  TU  ");
	DisplayString(0x0,1,"Detection system");
	delay_1s();
	WriteCmd(LCD_CLS);
	
	DisplayString(0x0,0,"TU=");
	DisplaySingleChar(0x08,0,'p');
	DisplaySingleChar(0x09,0,'p');
	DisplaySingleChar(0x0A,0,'m');
	
	
	while(1)
	{		
				TU_Value_Conversion();
//				display_value(); 
			for(i=0;i<5;i++)
				{
					R_S_Byte(disbuff[i]);
				}	
//				R_S_Byte('\r');//���ͻس����з�
//				R_S_Byte('\n');
				delay_1s();
  }	
}
