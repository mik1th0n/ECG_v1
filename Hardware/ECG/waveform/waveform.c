/*
*********************************************************************************************************
*
*	ģ������ : uCOS-III
*	�ļ����� : waveform.c
*	��    �� : V1.0
*	˵    �� : ������ʾ����
*
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2017-03-25    CML       ECG�������
*
*	Copyright (C), 2015-2016, CML
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       ����ͷ�ļ�
*********************************************************************************************************
*/                                                          
#include "sys.h"
#include "delay.h"
#include "ILI93xx.h"
#include "waveform.h"
#include "adc.h"
#include "bsp_usart1.h"

/*
*********************************************************************************************************
*                                       ˽�б�������
*********************************************************************************************************
*/
u16   buf[350];
u16   table[11]  ={0,25,50,75,100,125,150,175,200,225,250};
u16		table1[41] ={0,5,10,15,20,30,35,40,45,55,60,65,70,80,85,
										90,95,105,110,115,120,130,135,140,145,155,
										160,165,170,180,185,190,195,205,210,215,220,
										230,235,240,245};
u16 Ypos1=100,Ypos2=100,Xpos1,Xpos2;
u16 adcx,dsl;
u16  t=0;
u16  h=0;
u16 d=0;
u32 tim=500;		/* ����Ƶ���޸Ĳ������ */
u32 bei=1;
u16 Yinit=12; //75
u8 a;
u8 len;										

/*
*********************************************************************************************************
*	�� �� ��: Draw_Coordinate
*	����˵��: ��������
*	�������: ��
*	�� �� ֵ: ��
* ��    ����
* ��    �ڣ�2017.4.2  11:12
*********************************************************************************************************
*/
void Draw_Coordinate(void)
{
	int x = 0;
	int y = 0;
	u32 num = 0;
	
	/* ��������ֵ��ʾ */
	for(y=6,num=0;y<200;y+=10,num+=100)
	{
		LCD_ShowxNum(255,(u8)y,num,4,12,1);
	}
	/* �����굥λ��ʾ */
	for(y=6;y<200;y+=10)
	{
		LCD_ShowString(287,y,20,24,12,"mv");
	}
	
	/* ��������ֵ��ʾ */
	for (x=0,num=100;x<251;x+=23,num-=10)
	{
		LCD_ShowxNum(x,0,num,3,12,1);
	}
	/* �����굥λ��ʾ */
	for(x=18;y<251;y+=23)
	{
		LCD_ShowString(x,0,20,24,12,"s");
	}

}

/*
*********************************************************************************************************
*	�� �� ��: Draw_Gaid
*	����˵��: ��������
*	�������: ��
*	�� �� ֵ: ��
* ��    ����
* ��    �ڣ�2017.4.2  11:12
*********************************************************************************************************
*/
/*
void Draw_Gaid(void) 
{
	u16 x,y;
	for(x=45;x<246;x=x+25)//25
		for(y=6;y<197;y=y+2)//5
		{
			LCD_Fast_DrawPoint(x,y,YELLOW);
		}
	for(y=6;y<197;y=y+10)
		for(x=45;x<246;x=x+2)//5
		{
			LCD_Fast_DrawPoint(x,y,YELLOW);
		}
}
*/
void Draw_Gaid(void) 
{
	u16 x,y;
	for(x=0;x<251;x=x+25)
		for(y=12;y<203;y=y+5)
		{
			LCD_Fast_DrawPoint(x,y,YELLOW );
		}
	for(y=12;y<203;y=y+10)
		for(x=0;x<251;x=x+2)
		{
			LCD_Fast_DrawPoint(x,y,YELLOW );
		}
}

/*
*********************************************************************************************************
*	�� �� ��: Clear_Point
*	����˵��: ������ʾ����ǰ��
*	�������: ��
*	�� �� ֵ: ��
* ��    ����
* ��    �ڣ�2017.4.2  11:12
*********************************************************************************************************
*/
void Clear_Point(u16 hang)
{
	u8 index_clear_lie = 0; 
	POINT_COLOR = BLACK ;
	for(index_clear_lie = 12;index_clear_lie <203;index_clear_lie++)
	{		
		LCD_DrawPoint(hang,index_clear_lie );
	}
	if(hang==table[h])//�ж�hang�Ƿ�Ϊ25�ı���
	{
			for(index_clear_lie = 12;index_clear_lie <203;index_clear_lie=index_clear_lie+5)
			{		
				LCD_Fast_DrawPoint(hang ,index_clear_lie,YELLOW );
			}
			h++;
			if(h>10) h=0;
	}
	if(hang ==table1[d])//�ж�hang�Ƿ�Ϊ5�ı���
	{
			for(index_clear_lie = 12;index_clear_lie <203;index_clear_lie=index_clear_lie+10)//25
			{		
				LCD_Fast_DrawPoint(hang ,index_clear_lie,YELLOW );
			}
			d++;
			if(d>40) d=0;
	}
	
	POINT_COLOR=RED;	
}

/*
*********************************************************************************************************
*	�� �� ��: Draw_Oscillogram
*	����˵��: ������ͼ
*	�������: ��
*	�� �� ֵ: ��
* ��    ����
* ��    �ڣ�2017.4.2  11:12
*********************************************************************************************************
*/
void Draw_Oscillogram(void)
{
		for(t=0;t<300;t++)//�洢AD��ֵ
		{
			buf[t] = Get_Adc(ADC_Channel_1);
			if(tim>1)
				Delay_NOP_us(tim);//�ı�ADȡ�����
		}
		
		for(t=0;t<251;t++)
		{
			Clear_Point(t);	
			Ypos2=Yinit+buf[t]*100/4096; /* ת������ */
			Ypos2=Ypos2*bei;
			if(Ypos2 >200) Ypos2 =200; /* ������Χ����ʾ */
			LCD_DrawLine (t ,Ypos1 , t+1 ,Ypos2  );/* ������꣺x��y,�յ�����x,y */
			Ypos1 =Ypos2 ;
		}	
}

/*
*********************************************************************************************************
*	�� �� ��: Get_Vpp
*	����˵��: ��ȡ���ֵ
*	�������: ��
*	�� �� ֵ: ��
* ��    ����
* ��    �ڣ�2017.4.2  11:12
*********************************************************************************************************
*/
float Get_Vpp(void)	   
{
	
	u32 max_data=buf[0];
	u32 min_data=buf[0];
	u32 n=0;
	float Vpp=0;
	for(n = 1;n<201;n++)
	{
		if(buf[n]>max_data)
		{
			max_data = buf[n];
		}
		if(buf[n]<min_data)
		{
			min_data = buf[n];
		}			
	} 	
	Vpp = (float)(max_data-min_data);
	Vpp = Vpp*(3300.0/4095);
	return Vpp;	
}

/*
*********************************************************************************************************
*	�� �� ��: Usartkeyscan
*	����˵��: ��������ɨ��
*	�������: ��
*	�� �� ֵ: ��
* ��    ����
* ��    �ڣ�2017.4.2  11:12
*********************************************************************************************************
*/
/*
void Usart_Keyscan(void)  
{
		if(USART_RX_STA&0x8000)	 
		 {
				 len =USART_RX_STA &0x3fff;
				 for(t=0;t<len;t++)
				 {
					 a=USART_RX_BUF[t];
					 USART1->DR =a;
					 while((USART1->SR&0X40)==0);//�ȴ����ͽ���
				 }
				 USART_RX_STA=0;
				 if( USART_RX_BUF[0]=='K') Yinit=Yinit -25 ;
				 if( USART_RX_BUF[0]=='O') Yinit =Yinit +25 ;
				 if( USART_RX_BUF[0]=='J') 
				 {
					 tim /=2 ;
					 LCD_ShowxNum(284,220,tim ,3,16,0);
				 }
				 if( USART_RX_BUF[0]=='D') 
				 {
					 tim *=2 ;
					 LCD_ShowxNum(284,220,tim ,3,16,0);
				 }
				 if( USART_RX_BUF[0]=='S') 
				 { 
						bei =bei*2;
					 if(bei>3) bei=1;
				 }
				 
				 if(USART_RX_BUF[0]=='O'&&USART_RX_BUF[1]=='K')
				 {
					 while(USART_RX_STA==0&& d<20)
					 {
							LED0 =!LED0 ;
							d++;
							delay_ms (1000);
					 }
					 d=0;
				 }
			 
		 }
		 
}
*/
/******************************************* (END OF FILE) *********************************************/
