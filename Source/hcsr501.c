/*****************************************************
** 编写作者：  326电子工作室
** 编写时间：  2015年3月28日
** 功能模块：  HC-SR501人体红外感应模块,采用可连续触发
			   单片机采用定时器0,每50MS检测一次HC-SR501
			   输出口状态，连续10次采集到高电平则判断位
			   有人体接近
** 硬件相关：  MCU使用STC89C52RC,硬件连接对应如下：
**			   OUT----->P24	  
** 网    址：  ttsemb.taobao.com
** 技术支持群：QQ427165833
** 版权声明：  本程序只供学习使用，未经作者许可，不得用
** 			   于其他任何用途，版权所有，盗版必究。
******************************************************/

#include "hcsr501.h"

#define PEOPLE_NEAR_THRESHOLD	10	//人体接近判断阀值
#define PEOPLE_LEAVE_THRESHOLD	10	//人体离开判断阀值

sbit Out = P2^4;
uchar PeopleNear = 0;	  			//人体接近判断标示，0：无人接近或人离开；1：人接近
uchar OutUpCount = 0;				//HC-SR501输出高电平计数
uchar OutDowmCount = 0;				//HC-SR501输出低电平计数


/*****************************************************
** 函数名称：Timer0Init
** 函数功能：单片机定时器0初始化，每50MS产生1次中断
** 参    数：无
** 函数返回：无
******************************************************/ 
void Timer0Init(void)		//50毫秒@11.0592MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x4C;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0 = 1;
}

/*****************************************************
** 函数名称：Timer0IntPro
** 函数功能：单片机定时器0中断处理函数
			 ①连续10次采集到HC-SR501高电平输出，判断
			   为有人体接近。
			 ②当已判断有人体接近年后，连续10次采集到
			   HC-SR501低电平输出，判断为人体离开
** 参    数：无
** 函数返回：无
******************************************************/ 
void Timer0IntPro(void)		interrupt 1	using 0
{
	if(Out)
	{
		 OutUpCount++;
		 if(OutUpCount >= PEOPLE_NEAR_THRESHOLD)
		 {
		 	PeopleNear = 1;
		 }
	}
	else
	{
		if(PeopleNear)
		{
			OutDowmCount++;
			if(OutDowmCount >= PEOPLE_LEAVE_THRESHOLD)
			{
				PeopleNear = 0;
				OutUpCount = 0;
				OutDowmCount = 0;
			}	
		}
	}
}




