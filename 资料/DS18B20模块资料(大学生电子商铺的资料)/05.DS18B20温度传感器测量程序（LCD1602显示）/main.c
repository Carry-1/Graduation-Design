//****************************************************
//****************************************************
//			小强II号 无线开发板例程
//				大学生电子商铺
//			http://ilovemcu.taobao.com
//****************************************************
//****************************************************
#include "main.h"
#include "LCD1602.h"
#include "DS18B20.h"

//定义变量
unsigned int Temp_Buffer = 0;

//****************************************************
//主函数
//****************************************************
void main()
{
	Init_LCD1602();									//初始化LCD1602
	LCD1602_write_com(0x80);						//指针设置到第一行第1列
	LCD1602_write_word("RF DEMO BORAD");

	Temp_Buffer = Get_temp();  					//读取DS18B20的值
	Delay_ms(1000);								//等待1s等待DS18B20数据稳定。否则会出现85℃。
	while(1)
	{
		Temp_Buffer = Get_temp();  					//读取DS18B20的值
		LCD1602_write_com(0x80+0x40);				//设置LCD1602指针到第二行第一列
		LCD1602_write_word("TEMP = ");
		if(flag_temper == 1)						//根据温度标志位显示温度正负
		{
			LCD1602_write_data('-');	
		}
		if( Temp_Buffer/1000 != 0 )					//如果第一位为0，忽略显示
		{
			LCD1602_write_data(Temp_Buffer/1000+0X30);	   //显示温度百位值
		}
		if( Temp_Buffer/1000 == 0 && Temp_Buffer%1000/100 == 0 )			//百位十位都为0，忽略显示
		{

		}
		else
		{
			LCD1602_write_data(Temp_Buffer%1000/100+0X30);	   //显示温度十位值
		}
		LCD1602_write_data(Temp_Buffer%100/10+0X30);	   //显示温度个位值
		LCD1602_write_data('.');						   //显示小数点
		LCD1602_write_data(Temp_Buffer%10+0X30);		   //显示温度十分位值
		LCD1602_write_word(" C  ");						   //显示字符C
		
		Delay_ms(500);				
	}
}

//****************************************************
//MS延时函数(12M晶振下测试)
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}