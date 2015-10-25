#include <ioCC2540.h>
#include "pwm.h"


//pwm pins:
//P0.0
//P0.1

int16 gRed;
int16 gGreen;
int16 gBlue;

void Timer1_init(){
  // Initialize Timer 1
  T1CTL = 0x0E;               // Div = 128, CLR, MODE = Suspended          
  T1CCTL1 = 0x0C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T1CCTL2 = 0x0C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T1CCTL3 = 0x0C;             // IM = 0, CMP = Clear output on compare; Mode = Compare
  T1CCTL4 = 0x0C;
  T1CNTL = 0;                 // Reset timer to 0;

  T1CCTL0 = 0x4C;           
  T1CC0H = 0x01;             
  T1CC0L = 0x00;
  
  
  IEN1 |= 0x02;               // Enable T1 cpu interrupt
}

void Timer4_init(){
  // Initialize Timer4
  T4CTL = 0xFC;               // Div = 128, CLR, MODE = Suspended          
  T4CCTL1 = 0x0C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T4CCTL0 = 0x0C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  //T4CNTL = 0;                 // Reset timer to 0;
  
  IEN1 |= 0x10;               // Enable T4 cpu interrupt
}

void PWM_init()
{
  //T1 ����1 λ��  ռ��P1��0��1�˿ڣ�   ռ��P0�� 6��7�˿�
  P1DIR|= 0x03;
  P1SEL|= 0x03;
  P0DIR|= 0xC0;
  P0SEL|= 0xC0;
  
  PERCFG |= 0x40; //U0,U1 ���ڱ���0 λ��   T1 �ڱ���1λ��
  
  Timer1_init();
  
  //Timer4_init();
  
  EA=1;
  
}


void pwmPulse(int16 red, int16 green, int16 blue)
{
  int16 r,g,b;
  
  r=red;
  g=green;
  b=blue;

  T1CC1L = (uint8)r;
  T1CC1H = (uint8)0x0;
  T1CC2L = (uint8)g;
  T1CC2H = (uint8)0x0;
  T1CC3L = (uint8)b;
  T1CC3H = (uint8)0x0;
  T1CC4L = (uint8)r;
  T1CC4H = (uint8)0x0;
  // Reset timer
  T1CNTL = 0;
  

  // Start timer in modulo mode.
  T1CTL |= 0x02; 
  
}

void pwmPulse4(int16 red, int16 green, int16 blue)
{
  int16 r,g,b;
  // stop,ע�⣬���ܼ���䣬��������ƫ��ʮ����������ԭ��δ����
  //T1CTL &= BV(0)|BV(1); 
#if 0
  r=375;
  g=1;
  b=1;
#else
  r=red;
  g=green;
  b=blue;
#endif
  // Set up the timer registers

  T4CC0 = (uint8)r;

  T4CC1 = (uint8)g;

  // Reset timer
  //T4CNTL = 0;
  
}
void setRGB(int16 red, int16 green, int16 blue)
{
  gRed=red;
  gGreen=green;
  gBlue=blue;
}

//#pragma register_bank=2
#pragma vector = T1_VECTOR
__interrupt void pwmISR (void) {
    uint8 flags = T1STAT;
    // T1 ch 0
    if (flags & 0x01){          
      pwmPulse(gRed,gGreen,gBlue);
     
    }
    T1STAT = ~ flags;
}
//#pragma register_bank=2
/*#pragma vector = T4_VECTOR
__interrupt void pwmISR4 (void) {
    uint8 flags = IRCON;
    // T1 ch 0
    if (flags & 0x10){          
      pwmPulse4(gRed,gGreen,gBlue);
     
    }
    IRCON &=0xef;
}
*/
