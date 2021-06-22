/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * 文件名  ：main.c
 * 描述    ：        
 * 硬件连接：
 * LCD1602:RS -> PA11; RW -> PA12; E -> PA15;
 *         D0~D7 -> PB3~PB10
 * 浊度传感器模块: VCC -> 5V; GND -> GND; AO -> PA2;
 *
 * 功能描述：测量浊度值液晶显示(ADC1、PA2、DMA方式读取)；
             串口接收测量所得的浊度值（波特率115200）；
 *           可连接上位机显示浊度值；
 * 淘宝    ：https://ilovemcu.taobao.com
**********************************************************************************/
#include "stm32f10x.h"
#include "bsp_usart1.h"
#include <string.h>
#include "delay.h"
#include "driver_1602.h"
#include "math.h"
#include "bsp_adc.h"
#include "ds18b20.h"

GPIO_InitTypeDef  GPIO_InitStructure; 
unsigned char AD_CHANNEL=0;
unsigned long PH_num=0,PU_V=0;
float PH_Value=0;
u8 ph_temp=0,tu_temp=0;
u16 ph_result=0,tu_result=0;
u16 adc_1,adc_2;
u16 adc_v_1,adc_v_2;
float compensationCoefficient=1.0;//温度校准系数
float compensationVolatge;
float kValue=1.0;

float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
float temp_data=0.0;
float K_Value=3047.19;

u8 ISendByte(unsigned char sla,unsigned char c);
u8 IRcvByte(unsigned char sla);	
u8 SPIx_ReadWriteByte(u8 TxData);

unsigned char  Tx[20];   //无线发送缓存


// ADC1转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue;

// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal;  

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
	
   /* Configure  DRDY */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}


/**************浑浊度值采集函数***************/
void TU_Value_Conversion()
{
	TU =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值
	
		TU_calibration=-0.0192*(temp_data/10-25)+TU;  
	  TU_value=-865.68*TU_calibration + K_Value;
	
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}
	
		
		/*显示TDS*/
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
		// ADC 初始化
	  ADCx_Init();  
    /* 初始化LCD1602 */
		LCD_init(); 
	
     
	LCD_printstring(0,0,"TU:");
	LCD_printstring(0,1,"T:");
	LCD_printstring(0x09,0,"NTU"); 
	
  while(1)
	{	
		TEMP_Value_Conversion();
		TU_Value_Conversion();	
		
		printf("%s",Tx);
		delay_ms(1000);
		
	}	
}
/*********************************************END OF FILE**********************/
