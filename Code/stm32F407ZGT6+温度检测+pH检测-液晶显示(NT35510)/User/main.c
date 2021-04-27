/**
  ******************************************************************************
  * @file    main.c
  * @author  Xia Qingsheng
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   测量温度、pH值并用液晶显示屏显示
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

float PH_Value=0;
unsigned char  Tx[20];   //Tx数组用于缓存pH值
// ADC1转换的电压值通过DMA方式传到SRAM
__IO uint16_t ADC_ConvertedValue = 8;
// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal;  

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
/*显示DS18B20的序列号*/
//	while( DS18B20_Init() )	
//		NT35510_DispStringLine_EN(LINE(1),"no ds18b20 exit");
//	
//	NT35510_DispStringLine_EN(LINE(1),"ds18b20 ok");

//	DS18B20_ReadId ( ucDs18b20Id  );           // 读取 DS18B20 的序列号
//	
//	for ( uc = 0; uc < 8; uc++ )             // 打印 DS18B20 的序列号
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
		//1.循环检测温度并显示
		temperature=DS18B20_Get_Temp();
        
    sprintf((char*)dis_buf,"T:%0.3f degree Celsius",temperature);
    NT35510_DispStringLine_EN(LINE(1),dis_buf);
		
		//2.循环检测pH并显示
		
		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值
		
		PH_Value=-5.7541*ADC_ConvertedValueLocal+16.654;
		
			
		if(PH_Value<=0.0){PH_Value=0.0;}
		if(PH_Value>=14.0){PH_Value=14.0;}
	
		  /*显示电压*/
		Tx[0]=(int)(PH_Value*100)/1000+'0';	
		Tx[1]=(int)(PH_Value*100)%1000/100+'0';
		Tx[2]='.';
		Tx[3]=(int)(PH_Value*100)%100/10+'0';
		Tx[4]=(int)(PH_Value*100)%10+'0';
		Tx[5]='\0';   //字符串结束符
		NT35510_DispStringLine_EN(LINE(3),"2. pH test ");
		sprintf((char*)dis_buf,"pH:%s",Tx);
    NT35510_DispStringLine_EN(LINE(4),dis_buf);
		
	  

     //延时
    CPU_TS_Tmr_Delay_MS(1000);
	}	 
}

/*********************************************END OF FILE**********************/

