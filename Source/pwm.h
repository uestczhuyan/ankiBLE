#include "hal_types.h"

#ifndef _PWM_H_
#define _PWM_H_

// �Ѿ���hal_types.h �������

//typedef unsigned char uint8;
//typedef unsigned short int  uint16;
/****************************************************************************
���ú궨��
****************************************************************************/
#define BV(x) (1<<(x))


void PWM_init();
void setRGB(int16 red, int16 green, int16 blue);
void pwmPulse(int16 red, int16 green, int16 blue);

//������˸
//void startBlink(

#endif