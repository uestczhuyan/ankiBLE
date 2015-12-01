/*****************************************************
** 编写作者：  326电子工作室
** 编写时间：  2015年3月28日
** 功能模块：  HC-SR501人体红外感应模块
** 硬件相关：  MCU使用STC89C52RC,硬件连接对应如下：
**			   OUT----->P24
** 网    址：  ttsemb.taobao.com
** 技术支持群：QQ427165833
** 版权声明：  本程序只供学习使用，未经作者许可，不得用
** 			   于其他任何用途，版权所有，盗版必究。
******************************************************/
#ifndef _HCSR501_H
#define _HCSR501_H
#define uchar		unsigned char
#define uint		unsigned int

extern void Timer0Init(void);

extern uchar PeopleNear;
#endif


