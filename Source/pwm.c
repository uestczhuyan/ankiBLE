#include <ioCC2540.h>
#include "pwm.h"


//pwm pins:
//P0.0
//P0.1

int16 gRed;
int16 gGreen;
int16 gBlue;

void PWM_init()
{
  //设置pwm端口为输出
  P1DIR|= BV(0)|BV(1);
  //设置pwm端口为外设端口，非gpio
  P1SEL|= BV(0)|BV(1);
  //由于uart等会占用我们当前使用的pwm端口，因此需要将uart等重映射到别的端口去。
  PERCFG |= 0x40;             // Move USART1&2 to alternate2 location so that T1 is visible

  // Initialize Timer 1
  T1CTL = 0x0E;               // Div = 128, CLR, MODE = Suspended          
  T1CCTL1 = 0x0C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T1CCTL2 = 0x0C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T1CCTL3 = 0x0C;             // IM = 0, CMP = Clear output on compare; Mode = Compare
  T1CNTL = 0;                 // Reset timer to 0;

  T1CCTL0 = 0x4C;           
  T1CC0H = 0x01;             
  T1CC0L = 0x00;            
            

  EA=1;
  IEN1 |= 0x02;               // Enable T1 cpu interrupt
}


void pwmPulse(int16 red, int16 green, int16 blue)
{
  int16 r,g,b;
  // stop,注意，不能加这句，加了周期偏差十几倍，具体原因未查明
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

  T1CC1L = (uint8)r;
  T1CC1H = (uint8)0x0;
  T1CC2L = (uint8)g;
  T1CC2H = (uint8)0x0;
  T1CC3L = (uint8)b;
  T1CC3H = (uint8)0x0;

  // Reset timer
  T1CNTL = 0;
  

  // Start timer in modulo mode.
  T1CTL |= 0x02; 
  
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
