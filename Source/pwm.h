#include "hal_types.h"

#ifndef _PWM_H_
#define _PWM_H_

// 已经被hal_types.h 定义过了

//typedef unsigned char uint8;
//typedef unsigned short int  uint16;
/****************************************************************************
常用宏定义
****************************************************************************/
#define BV(x) (1<<(x))


void PWM_init();
void setRGB(int16 red, int16 green, int16 blue);
void pwmPulse(int16 red, int16 green, int16 blue);

//持续闪烁
//void startBlink(

#endif