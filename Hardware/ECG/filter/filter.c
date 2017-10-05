/*
*********************************************************************************************************
*
*	模块名称 : uCOS-III
*	文件名称 : filter.c
*	版    本 : V1.0
*	说    明 : 数字滤波器
*
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2017-03-25    CML       ECG程序代码
*
*	Copyright (C), 2015-2016, CML
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       包含头文件
*********************************************************************************************************
*/                                                          
#include "sys.h"
#include "bsp_usart1.h"
#include "delay.h"
#include "filter.h"
#include "adc.h"

/*
*********************************************************************************************************
*                                       宏定义
*********************************************************************************************************
*/
#define L 100
#define N 11
#define M 5

/*
*********************************************************************************************************
*                                       私有变量定义
*********************************************************************************************************
*/
/***********************************ADC采样值缓冲区***********************************/
__IO u16 BUF1[100];  //缓冲1区
__IO u16 BUF2[100];  //缓冲2区

__IO u16 i=0,j;

/*****************************************滤波器数组****************************************/
__IO float buf_N[N]={0};                   //SG平滑滤波数组
__IO float IIR_Filter_Data[100];           //缓存数组，储存滤波之后的时域信号 float类型
__IO float SG_Filter_Data[100];            //SG平滑之后数据
__IO int   Send_int_Data[100];             //转换为int类型数据
__IO char  Send_char_Data[5];              //发送数组


__IO float w0[3]={0};
__IO float w1[3]={0};

__IO float x0=0,x1=0;
__IO float y0=0,y1=0;

/**************************************************SG_Filter********************************************/
//N=11 D=2
const float B[N][N]={
  {  0.580420,  0.377622,  0.209790,  0.076923, -0.020979, -0.083916, -0.111888, -0.104895, -0.062937,  0.013986,  0.125874},
	{  0.377622,  0.278322,  0.193007,  0.121678,  0.064336,  0.020979, -0.008392, -0.023776, -0.025175, -0.012587,  0.013986},
	{  0.209790,  0.193007,  0.173893,  0.152448,  0.128671,  0.102564,  0.074126,  0.043357,  0.010256, -0.025175, -0.062937},
	{  0.076923,  0.121678,  0.152448,  0.169231,  0.172028,  0.160839,  0.135664,  0.096503,  0.043357, -0.023776, -0.104895},
	{ -0.020979,  0.064336,  0.128671,  0.172028,  0.194406,  0.195804,  0.176224,  0.135664,  0.074126, -0.008392, -0.111888},
	{ -0.083916,  0.020979,  0.102564,  0.160839,  0.195804,  0.207459,  0.195804,  0.160839,  0.102564,  0.020979, -0.083916},
	{ -0.111888, -0.008392,  0.074126,  0.135664,  0.176224,  0.195804,  0.194406,  0.172028,  0.128671,  0.064336, -0.020979},
	{ -0.104895, -0.023776,  0.043357,  0.096503,  0.135664,  0.160839,  0.172028,  0.169231,  0.152448,  0.121678,  0.076923},
	{ -0.062937, -0.025175,  0.010256,  0.043357,  0.074126,  0.102564,  0.128671,  0.152448,  0.173893,  0.193007,  0.209790},
	{  0.013986, -0.012587, -0.025175, -0.023776, -0.008392,  0.020979,  0.064336,  0.121678,  0.193007,  0.278322,  0.377622},
	{  0.125874,  0.013986, -0.062937, -0.104895, -0.111888, -0.083916, -0.020979,  0.076923,  0.209790,  0.377622,  0.580420}
};

/********************************************50hz_IIR_Fnotch_Filter******************************************/
const float IIR_50Notch_B[3] = {
    0.9023977442,
	 -0.5577124773,
	  0.9023977442
};

const float IIR_50Notch_A[3] = {
	  1,
	 -0.5577124773,
	  0.8047954884
};



/***********************************************0.3Hz_IIR__High_Filter****************************************/
const float Gain=0.99468273;
const float IIR_High_B[3]={
         1,
        -2,
         1   
};

const float IIR_High_A[3]={
         1,
        -1.9833718117,
         0.9839372834
};

/*
*********************************************************************************************************
*                                       函数定义
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*	函 数 名: Get_Adc_Val
*	功能说明: 复位IIR滤波器参数
*	输入参数: 无
*	返 回 值: 无
* 其    它：
* 日    期：2017.4.9  10:40
*********************************************************************************************************
*/
void Get_Adc_Val(void)
{
		for (i=0;i<100;i++)
		{
				BUF1[i] = Get_Adc(0);
		}
		
		for (i=0;i<100;i++)
		{
				BUF2[i] = Get_Adc(0);
		}
}	

/*
*********************************************************************************************************
*	函 数 名: IIR_Reset
*	功能说明: 复位IIR滤波器参数
*	输入参数: 无
*	返 回 值: 无
* 其    它：
* 日    期：2017.4.2  11:12
*********************************************************************************************************
*/
void IIR_Reset(void)
{
	u16 i;
   for(i=0;i<3;i--)
 	 {
     w0[i]=0;
		 w1[i]=0;
   }
	  x0=0;
	  y0=0;
	  x1=0;
	  y1=0;	 
}

/*
*********************************************************************************************************
*	函 数 名: First_filter_buf1
*	功能说明: 一阶滤波
*	输入参数: 无
*	返 回 值: 无
* 其    它：
* 日    期：2017.4.9  10:19
*********************************************************************************************************
*/
void First_filter_buf1(void)
{
	 for(i=0;i<100;i++)
	 {
			x0=(float)BUF1[i]/4096*3.3-1.23;   // 读取转换的AD值	
			x0=-x0;   //电压数值翻转
		 
			w0[0]=IIR_50Notch_A[0]*x0-IIR_50Notch_A[1]*w0[1]-IIR_50Notch_A[2]*w0[2];
			y0=IIR_50Notch_B[0]*w0[0]+IIR_50Notch_B[1]*w0[1]+IIR_50Notch_B[2]*w0[2];
			 
			w1[0]=IIR_High_A[0]*y0-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
			y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);
			 
			IIR_Filter_Data[i]=y1;         //缓存数组储存滤波之后的信号 
			 
			w0[2]=w0[1];
			w0[1]=w0[0];
			w1[2]=w1[1];
			w1[1]=w1[0];             
	}
	for(i=0;i<=M;i++)            //第0到M，一共M+1个点
  {
		for(j=0;j<N;j++)
		{
      SG_Filter_Data[i]=SG_Filter_Data[i]+B[j][i]*IIR_Filter_Data[j];
		}
  }
	for(i=0;i<N;i++)            //将数据放入缓存数组中
	{
     buf_N[0]=buf_N[1];
		 buf_N[1]=buf_N[2];
		 buf_N[2]=buf_N[3];
		 buf_N[3]=buf_N[4];
		 buf_N[4]=buf_N[5];
		 buf_N[5]=buf_N[6];
		 buf_N[6]=buf_N[7];
		 buf_N[7]=buf_N[8];
		 buf_N[8]=buf_N[9];      //数据移位
		 buf_N[9]=buf_N[10];
	   buf_N[10]=IIR_Filter_Data[i];	
  }
	for(i=M+1;i<L-M;i++)            //第M+1到L-1，一共L-M-1个点
  {
		
    for(j=0;j<N;j++)
		{
      SG_Filter_Data[i]=SG_Filter_Data[i]+B[j][M]*buf_N[j];
		}
		buf_N[0]=buf_N[1];
		buf_N[1]=buf_N[2];
		buf_N[2]=buf_N[3];
		buf_N[3]=buf_N[4];
		buf_N[4]=buf_N[5];
		buf_N[5]=buf_N[6];
		buf_N[6]=buf_N[7];
		buf_N[7]=buf_N[8];
		buf_N[8]=buf_N[9];     //数据移位
		buf_N[9]=buf_N[10];
		buf_N[10]=IIR_Filter_Data[i+M]; 
	}
}	

/*
*********************************************************************************************************
*	函 数 名: Else_filter_buf1
*	功能说明: 滤波
*	输入参数: 无
*	返 回 值: 无
* 其    它：
* 日    期：2017.4.9  10:19
*********************************************************************************************************
*/
void Else_filter_buf1(void)
{
	  for(i=0;i<100;i++)
	 {
			x0=(float)BUF1[i]/4096*3.3-1.23;   // 读取转换的AD值	
			x0=-x0;   //电压数值翻转
			 
			w0[0]=IIR_50Notch_A[0]*x0-IIR_50Notch_A[1]*w0[1]-IIR_50Notch_A[2]*w0[2];
			y0=IIR_50Notch_B[0]*w0[0]+IIR_50Notch_B[1]*w0[1]+IIR_50Notch_B[2]*w0[2];
			 
			w1[0]=IIR_High_A[0]*y0-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
			y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);
			 
			IIR_Filter_Data[i]=y1;         //缓存数组储存滤波之后的信号 
			 
			w0[2]=w0[1];
			w0[1]=w0[0];
			w1[2]=w1[1];
			w1[1]=w1[0];             
	 }
	for(i=0;i<L;i++)            //第0到M，一共M+1个点
  {
		  buf_N[0]=buf_N[1];
		  buf_N[1]=buf_N[2];
		  buf_N[2]=buf_N[3];
		  buf_N[3]=buf_N[4];
		  buf_N[4]=buf_N[5];
		  buf_N[5]=buf_N[6];
		  buf_N[6]=buf_N[7];
		  buf_N[7]=buf_N[8];
		  buf_N[8]=buf_N[9];    //数据移位
		  buf_N[9]=buf_N[10];
	    buf_N[10]=IIR_Filter_Data[i];	
      for(j=0;j<N;j++)
		  {
        SG_Filter_Data[i]=SG_Filter_Data[i]+B[j][M]*buf_N[j];
		  } 
	}
}	

/*
*********************************************************************************************************
*	函 数 名: First_filter_buf2
*	功能说明: 滤波
*	输入参数: 无
*	返 回 值: 无
* 其    它：
* 日    期：2017.4.9  10:19
*********************************************************************************************************
*/
void First_filter_buf2(void)
{
	 for(i=0;i<100;i++)
	 {
			x0=(float)BUF2[i]/4096*3.3-1.23;   // 读取转换的AD值	
			x0=-x0;   //电压数值翻转
		 
			w0[0]=IIR_50Notch_A[0]*x0-IIR_50Notch_A[1]*w0[1]-IIR_50Notch_A[2]*w0[2];
			y0=IIR_50Notch_B[0]*w0[0]+IIR_50Notch_B[1]*w0[1]+IIR_50Notch_B[2]*w0[2];
			 
			w1[0]=IIR_High_A[0]*y0-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
			y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);
			 
			IIR_Filter_Data[i]=y1;         //缓存数组储存滤波之后的信号 
			 
			w0[2]=w0[1];
			w0[1]=w0[0];
			w1[2]=w1[1];
			w1[1]=w1[0];             
	}
	for(i=0;i<=M;i++)            //第0到M，一共M+1个点
  {
		for(j=0;j<N;j++)
		{
      SG_Filter_Data[i]=SG_Filter_Data[i]+B[j][i]*IIR_Filter_Data[j];
		}
  }
	for(i=0;i<N;i++)            //将数据放入缓存数组中
	{
     buf_N[0]=buf_N[1];
		 buf_N[1]=buf_N[2];
		 buf_N[2]=buf_N[3];
		 buf_N[3]=buf_N[4];
		 buf_N[4]=buf_N[5];
		 buf_N[5]=buf_N[6];
		 buf_N[6]=buf_N[7];
		 buf_N[7]=buf_N[8];
		 buf_N[8]=buf_N[9];      //数据移位
		 buf_N[9]=buf_N[10];
	   buf_N[10]=IIR_Filter_Data[i];	
  }
	for(i=M+1;i<L-M;i++)            //第M+1到L-1，一共L-M-1个点
  {
		
    for(j=0;j<N;j++)
		{
      SG_Filter_Data[i]=SG_Filter_Data[i]+B[j][M]*buf_N[j];
		}
		buf_N[0]=buf_N[1];
		buf_N[1]=buf_N[2];
		buf_N[2]=buf_N[3];
		buf_N[3]=buf_N[4];
		buf_N[4]=buf_N[5];
		buf_N[5]=buf_N[6];
		buf_N[6]=buf_N[7];
		buf_N[7]=buf_N[8];
		buf_N[8]=buf_N[9];     //数据移位
		buf_N[9]=buf_N[10];
		buf_N[10]=IIR_Filter_Data[i+M]; 
	}
}	



