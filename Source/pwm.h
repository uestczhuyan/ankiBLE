#include "hal_types.h"

#ifndef _PWM_H_
#define _PWM_H_

// 已经被hal_types.h 定义过了

//typedef unsigned char uint8;
//typedef unsigned short int  uint16;
/****************************************************************************
常用宏定义
****************************************************************************/


void PWM_init();
void setRGB(int16 LED1_red, int16 LED1_green, int16 LED1_blue,int16 LED2_red, int16 LED2_green, int16 LED2_blue);

void setValus(uint8 *value,uint8 *value2,uint8 isChange);

void LedChange();

void init_QI_Switch(int8 on);

void init_RedLine_Switch(int8 on);

#endif