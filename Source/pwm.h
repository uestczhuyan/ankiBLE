#include "hal_types.h"

#ifndef _PWM_H_
#define _PWM_H_

// �Ѿ���hal_types.h �������

//typedef unsigned char uint8;
//typedef unsigned short int  uint16;
/****************************************************************************
���ú궨��
****************************************************************************/


void PWM_init();
void setRGB(int16 LED1_red, int16 LED1_green, int16 LED1_blue,int16 LED2_red, int16 LED2_green, int16 LED2_blue);

//������˸
//void startBlink(

#endif