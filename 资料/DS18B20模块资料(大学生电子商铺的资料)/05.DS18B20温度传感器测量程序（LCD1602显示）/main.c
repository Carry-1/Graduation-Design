//****************************************************
//****************************************************
//			СǿII�� ���߿���������
//				��ѧ����������
//			http://ilovemcu.taobao.com
//****************************************************
//****************************************************
#include "main.h"
#include "LCD1602.h"
#include "DS18B20.h"

//�������
unsigned int Temp_Buffer = 0;

//****************************************************
//������
//****************************************************
void main()
{
	Init_LCD1602();									//��ʼ��LCD1602
	LCD1602_write_com(0x80);						//ָ�����õ���һ�е�1��
	LCD1602_write_word("RF DEMO BORAD");

	Temp_Buffer = Get_temp();  					//��ȡDS18B20��ֵ
	Delay_ms(1000);								//�ȴ�1s�ȴ�DS18B20�����ȶ�����������85�档
	while(1)
	{
		Temp_Buffer = Get_temp();  					//��ȡDS18B20��ֵ
		LCD1602_write_com(0x80+0x40);				//����LCD1602ָ�뵽�ڶ��е�һ��
		LCD1602_write_word("TEMP = ");
		if(flag_temper == 1)						//�����¶ȱ�־λ��ʾ�¶�����
		{
			LCD1602_write_data('-');	
		}
		if( Temp_Buffer/1000 != 0 )					//�����һλΪ0��������ʾ
		{
			LCD1602_write_data(Temp_Buffer/1000+0X30);	   //��ʾ�¶Ȱ�λֵ
		}
		if( Temp_Buffer/1000 == 0 && Temp_Buffer%1000/100 == 0 )			//��λʮλ��Ϊ0��������ʾ
		{

		}
		else
		{
			LCD1602_write_data(Temp_Buffer%1000/100+0X30);	   //��ʾ�¶�ʮλֵ
		}
		LCD1602_write_data(Temp_Buffer%100/10+0X30);	   //��ʾ�¶ȸ�λֵ
		LCD1602_write_data('.');						   //��ʾС����
		LCD1602_write_data(Temp_Buffer%10+0X30);		   //��ʾ�¶�ʮ��λֵ
		LCD1602_write_word(" C  ");						   //��ʾ�ַ�C
		
		Delay_ms(500);				
	}
}

//****************************************************
//MS��ʱ����(12M�����²���)
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}