/**
  ******************************************************************************
  * @file    main.c
  * @author  Xia Qingsheng
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   测量温度、pH值并用液晶显示屏显示
	* 硬件连接：
  * NT35510:
  * 温度传感器模块: VCC -> 3.3V; GND -> GND; DO -> PE3;
	*
	* PH传感器模块: VCC -> 5V; GND -> GND; PO -> PB0;
  *
	* EC传感器模块: VCC -> 5V; GND -> GND; AO -> PB1;
  *
	* 浑浊度传感器模块: VCC -> 5V; GND -> GND; AO -> PA6;

  *
  * 功能描述：测量EC值液晶显示(ADC1、PA2、DMA方式读取)；
             串口接收测量所得的EC值（波特率115200）；
  *           可连接上位机显示EC值；
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板 
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

#define RES2 820.0   //运放电阻，与硬件电路有关
#define ECREF 200.0 //极片输入电压，与硬件电路相关

// ADC转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue[RHEOSTAT_NOFCHANEL];  
// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal[RHEOSTAT_NOFCHANEL]={0};

/****************pH检测模块用到的变量****************/
float PH_Value=0;
unsigned char  Tx_PH[20];   //Tx数组用于缓存pH值


/****************电导率检测模块用到的变量************/
float EC_voltage;
float EC_value=0.0,voltage_value;
float temp_data=250;
float compensationCoefficient=1.0;//温度校准系数
float compensationVolatge;
float kValue=1.0;
float kValue_Low=1;  //校准时进行修改
float kValue_High=1; //校准时进行修改
float rawEC=0.0;
float EC_valueTemp=0.0;

unsigned char  Tx_EC[20];   //无线发送缓存
  
/****************浑浊度检测模块用到的变量************/
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

unsigned char  Tx_TU[20];   //无线发送缓存




/****************液晶显示模块用到的变量************/
/*********************************/
/* 液晶显示屏的显示缓冲区 */
char dis_buf[1024];


/**
  * @brief  主函数
  * @param  无  
  * @retval 无
  */
int main ( void )
{
  float temperature;    //存放温度检测值
	uint8_t uc, ucDs18b20Id [ 8 ];   //存放DS18B20序列号
  uint8_t DS18B20Id_str[20];
	
	/* 配置SysTick 为1us中断一次 */
	SysTick_Init();          //系统定时器初始化
	
  NT35510_Init ();         //LCD 初始化

	// Debug_USART_Config();		 //USART初始化, 当用串口调试助手显示温度时需要用到USART，用液晶显示时不需要用到USART
	Rheostat_Init();	       // ADC初始化
/* LED 端口初始化 */
	LED_GPIO_Config();
	
	

  //其中0、3、5、6 模式适合从左至右显示文字，
  //不推荐使用其它模式显示文字	其它模式显示文字会有镜像效果			
  //其中 6 模式为大部分液晶例程的默认显示方向  
  NT35510_GramScan ( 6 );

  NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
  
	
	NT35510_DispStringLine_EN(LINE(0),"1. ds18b20 test");

	
	for(;;) 
	{	
		//1.循环检测温度并显示
		temperature=DS18B20_Get_Temp();   //读取温度，存入temperature变量
        
    sprintf((char*)dis_buf,"T:%0.3f degree Celsius",temperature);   //将温度值插入显示缓冲区
    NT35510_DispStringLine_EN(LINE(1),dis_buf);    //调用显示函数
		
		//2.循环检测pH并显示
		
		ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*3.3; // 读取转换的AD值
		
		PH_Value=-5.7541*ADC_ConvertedValueLocal[0]+16.654;
		
			
		if(PH_Value<=0.0){PH_Value=0.0;}
		if(PH_Value>=14.0){PH_Value=14.0;}
	
		  /*显示电压*/
		Tx_PH[0]=(int)(PH_Value*100)/1000+'0';	   //将数字转化为字符
		Tx_PH[1]=(int)(PH_Value*100)%1000/100+'0';
		Tx_PH[2]='.';
		Tx_PH[3]=(int)(PH_Value*100)%100/10+'0';
		Tx_PH[4]=(int)(PH_Value*100)%10+'0';
		Tx_PH[5]='\0';   //字符串结束符
		NT35510_DispStringLine_EN(LINE(3),"2. pH test ");
		sprintf((char*)dis_buf,"pH:%s",Tx_PH);
    NT35510_DispStringLine_EN(LINE(4),dis_buf);
		
		//3.循环检测电导率并显示
	  EC_voltage =(float) ADC_ConvertedValue[1]/4096*3300; // 读取转换的AD值
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
		/*显示EC*/
		Tx_EC[0]=(int)(EC_value*100)/1000+'0';
		Tx_EC[1]=(int)(EC_value*100)%1000/100+'0';
		Tx_EC[2]='.';
		Tx_EC[3]=(int)(EC_value)%100/10+'0';	
		Tx_EC[4]=(int)(EC_value)%10+'0';
		Tx_EC[5]='\0';   //字符串结束符
		NT35510_DispStringLine_EN(LINE(6),"3. Electrical conductivity(EC)");
		NT35510_DispStringLine_EN(LINE(7),"test");
		sprintf((char*)dis_buf,"EC:%s",Tx_EC);
    NT35510_DispStringLine_EN(LINE(8),dis_buf);
		
		//4.循环检测浑浊度并显示
		TU =(float) ADC_ConvertedValue[2]/4096*3.3; // 读取转换的AD值
		TU_calibration=-0.0192*(temp_data/10-25)+TU;  
	  TU_value=-865.68*TU_calibration + K_Value_TU;
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}
	
		
		/*显示TDS*/
		Tx_TU[0]=(int)(TU_value)/1000+'0';
		Tx_TU[1]=(int)(TU_value)%1000/100+'0';	
		Tx_TU[2]=(int)(TU_value)%100/10+'0';
		Tx_TU[3]=(int)(TU_value)%10+'0';
		Tx_TU[4]='\0';   //字符串结束符

		NT35510_DispStringLine_EN(LINE(10),"4. Turbidity(TU) test");
		sprintf((char*)dis_buf,"TU:%s",Tx_TU);
    NT35510_DispStringLine_EN(LINE(11),dis_buf);
		
		
     //延时
    CPU_TS_Tmr_Delay_MS(1000);
	}	 
}

/*********************************************END OF FILE**********************/

