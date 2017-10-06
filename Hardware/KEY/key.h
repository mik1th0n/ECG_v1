#ifndef __KEY_H__
#define __KEY_H__
#include "sys.h"

//#define KEY0 PCin(5)   	//PC5
//#define KEY1 PAin(15)	//PA15 


#define KEY0  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5) //读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15) //读取按键1

#define KEY0_PRES 	1	// KEY0按下
#define KEY1_PRES	  2	// KEY1按下

void KEY_Init(void);// IO初始化
u8 KEY_Scan(u8);  	// 按键扫描函数	

#endif
