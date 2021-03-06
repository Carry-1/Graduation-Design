/********************(C) COPRIGHT 2019 Crownto electronic **************************
 * 文件名  ：main.c
 * 描述    ：        
 * 硬件连接：
 * 浊度传感器模块: VCC -> 5V; GND -> GND; DO -> PB6;
 * LED：   LED -> PC13
 * 功能描述：测量浊度传感器模块DO口高低电平；
 *           浑浊度高于阈值时指示灯亮；浑浊度低于阈值时指示灯灭（指示灯可替换为继电器）
 * 淘宝    ：https://ilovemcu.taobao.com
**********************************************************************************/

#include "stm32f10x.h"
#include "delay.h"

/**************************************************************************************
 * 描  述 : GPIO初始化配置
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable the GPIO  Clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC , ENABLE); 						 		
	
  GPIO_DeInit(GPIOB);	 //将外设GPIOB寄存器重设为缺省值
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    //浮空输入   
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC , GPIO_Pin_13);   //初始状态，熄灭指示灯
}

/**************************************************************************************
 * 描  述 : 浑浊度高于阈值时指示灯亮；浑浊度低于阈值时指示灯灭（指示灯可替换为继电器）
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
void LiquidSensor_Scan(void)
{
	 if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)           //浊度传感器用引脚PB6为低电平
	  {
	    delay_ms(10);                                            //延时防抖动
	    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)	       //浊度传感器用引脚PB6为低电平
	    {
		    GPIO_ResetBits(GPIOC , GPIO_Pin_13);	                   //点亮LED2
	    }
	  }
		else
		{
		 	    GPIO_SetBits(GPIOC , GPIO_Pin_13);                      //熄灭LED2
		}
}

/**************************************************************************************
 * 描  述 : MAIN函数
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
int main(void)
{

	SystemInit();		            	//设置系统时钟72MHZ
	GPIO_Configuration();         //GPIO口初始化
	
  while(1)
  {
		LiquidSensor_Scan();         //浊度传感器检测，并控制用户指示灯LED2亮灭 
	}
}
/*********************************END FILE********************************************/
