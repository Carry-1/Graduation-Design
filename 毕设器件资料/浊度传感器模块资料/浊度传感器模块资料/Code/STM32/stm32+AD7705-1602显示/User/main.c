/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * 文件名  ：main.c
 * 描述    ：        
 * 硬件连接：
 * AD7705:VCC -> 5V; GND -> GND; CS-> PA0;DRDY-> PA1;SLCK -> PA5; DOUT -> PA6; DIN ->PA7;
 * LCD1602:RS -> PA11; RW -> PA12; E -> PA15;
 *         D0~D7 -> PB3~PB10
 * DS18B20：VCC -> 3.3V; GND -> GND; DOUT -> PB12
 * 浊度传感器模块: VCC -> 5V; GND -> GND; AO -> AIN1+;
 *
 * 功能描述：测量浊度值液晶显示；
             串口接收测量所得的浊度值（波特率115200）；
 *           可连接上位机显示TDS值；
 * 淘宝    ：https://ilovemcu.taobao.com
**********************************************************************************/
 
 
#include "stm32f10x.h"
#include "bsp_usart1.h"
#include <string.h>
#include "delay.h"
#include "driver_1602.h"
#include "math.h"
#include "AD7705.h"
#include "ds18b20.h"

GPIO_InitTypeDef  GPIO_InitStructure; 
//unsigned long PH_num=0,PU_V=0;
//float PH_Value=0;
//u8 ph_temp=0;
//u16 ph_result=0;
u16 adc_1;
float adc_v_1;
float TDS=0.0,TDS_voltage;
float TDS_value=0.0,voltage_value;
//float temp_data=0.0;
//float compensationCoefficient=1.0;//温度校准系数
//float compensationVolatge;
//float kValue=1.0;

float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
//unsigned char disbuff[13]={0}; 
float temp_data=0.0;
float K_Value=3047.19;


unsigned char  Tx[9];   //无线发送缓存


/***************************************************************************
 * 描  述 : MAIN函数
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************/
 
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable the GPIO  Clock */					 		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO|RCC_APB2Periph_SPI1,ENABLE);
	
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);		//屏蔽所有作为JTAG口的GPIO口
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);		//屏蔽PB口上IO口JTAG功能
	
	
//1602 D0~D7 引脚	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|
	GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*I/O 方向 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; /*I/O 输出速度*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	
//1602 EN RS RW 引脚	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*I/O 方向 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; /*I/O 输出速度*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
   /* Configure AD7705  DRDY */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

/**************TDS值采集函数***************/
void TU_Value_Conversion()
{
	TM7705_Init(2);	
 	adc_1 = ReadAD7705(2);
	TU = adc_1*5.0/65535;
	
//		TU=(TU*5.0)/1024;
		TU_calibration=-0.0192*(temp_data/10-25)+TU;  
	  TU_value=-865.68*TU_calibration + K_Value;
	
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}

		
		/*显示浑浊度*/
		Tx[0]=(int)(TU_value)/1000+'0';
		Tx[1]=(int)(TU_value)%1000/100+'0';	
		Tx[2]=(int)(TU_value)%100/10+'0';
		Tx[3]=(int)(TU_value)%10+'0';
		
		LCD_printchar(0x5,0,Tx[0]);
		LCD_printchar(0x6,0,Tx[1]);
		LCD_printchar(0x7,0,Tx[2]);
		LCD_printchar(0x8,0,Tx[3]);
}

void TEMP_Value_Conversion()
{
	  temp_data=DS18B20_Get_Temp();
	
	  Tx[4]=(int)(temp_data)%1000/100+'0';	
	  Tx[5]=(int)(temp_data)%100/10+'0';
	  Tx[6]='.';
	  Tx[7]=(int)(temp_data)%10+'0';
	
	  LCD_printchar(0x5,1,Tx[4]);
		LCD_printchar(0x6,1,Tx[5]);
		LCD_printchar(0x7,1,Tx[6]);
		LCD_printchar(0x8,1,Tx[7]);
}

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
	
		GPIO_Configuration();
	  DS18B20_Init();
    /* 配置USART1 */
    USART1_Config();
		LCD_init(); 
		AD7705_AllInit();
	
     
	LCD_printstring(0,0,"TU:");
	LCD_printstring(0,1,"T:");
	LCD_printstring(0x09,0,"NTU");
	
  while(1)
	{	
		TEMP_Value_Conversion();
		TU_Value_Conversion();	
			
		printf("%s",Tx);
		
	}	
}
/*********************************************END OF FILE**********************/
