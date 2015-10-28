#include <ioCC2540.h>
#include "pwm.h"


//pwm pins:
//P0.0
//P0.1
#define LED1 P1_0 

int16 LED1_Red;
int16 LED1_Green;
int16 LED1_Blue;

int16 LED2_Red;
int16 LED2_Green;
int16 LED2_Blue;

void Timer1_init(){
  //T1 备用1 位置  占用P1的0、1端口，   占用P0的 6、7端口
  P1DIR|= 0x03;
  P1SEL|= 0x03;
  P0DIR|= 0xC0;
  P0SEL|= 0xC0;
  
  PERCFG |= 0x40; //U0,U1 都在备用0 位置   T1 在备用1位置
  
  // Initialize Timer 1
  T1CTL = 0x0E;               // Div = 128, CLR, MODE = Suspended          
  T1CCTL1 = 0x2C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T1CCTL2 = 0x2C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T1CCTL3 = 0x2C;             // IM = 0, CMP = Clear output on compare; Mode = Compare
  T1CCTL4 = 0x2C;
  T1CNTL = 0;                 // Reset timer to 0;

  T1CCTL0 = 0x4C;           
  T1CC0H = 0x01;             
  T1CC0L = 0x00;
  
  
  IEN1 |= 0x02;               // Enable T1 cpu interrupt
}


void Timer3_init(){
   //T3 备用1 位置  占用P1的0、1端口，   占用P0的 6、7端口
  P1DIR|= 0xC0;
  P1SEL|= 0xC0;
 
  P2SEL |= 0x20;
  
  PERCFG |= 0x20; //U0,U1 都在备用0 位置   T3 在备用1位置
  
  // Initialize Timer3
  T3CTL = 0xF8;               // Div = 128, CLR, MODE = Suspended          
  T3CCTL1 = 0x2C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  T3CCTL0 = 0x2C;             // IM = 0; CMP = Clear output on compare; Mode = Compare
  
  T3CNT =0;                 // Reset timer to 0;
  
  T3IE =1;               // Enable T3 cpu interrupt
}

void PWM_init()
{
  
  Timer1_init();
  
  Timer3_init();
  
  //Timer4_init();
  
  EA=1;
}


void pwmPulse(int16 l1red, int16 l1green, int16 l1blue,int16 l2red)
{
  int16 l1r,l1g,l1b,l2r;
  
  l1r=l1red;
  l1g=l1green;
  l1b=l1blue;
  l2r = l2red;
  
  T1CC1L = (uint8)l1r;
  T1CC1H = (uint8)0x0;
  T1CC2L = (uint8)l1g;
  T1CC2H = (uint8)0x0;
  T1CC3L = (uint8)l1b;
  T1CC3H = (uint8)0x0;
  T1CC4L = (uint8)l2r;
  T1CC4H = (uint8)0x0;
  // Reset timer
  T1CNTL = 0;
  

  // Start timer in modulo mode.
  T1CTL |= 0x02; 
  
}

void pwmPulse3(int16 green, int16 blue)
{
  int16 g,b;
  g=green;
  b=blue;
  // Set up the timer registers
  

  T3CC0 = (uint8)g;
  T3CC1 = (uint8)b;
  // Reset timer
  //T4CNTL = 0;
}

void setRGB(int16 LED1_red, int16 LED1_green, int16 LED1_blue,int16 LED2_red, int16 LED2_green, int16 LED2_blue)
{
  LED1_Red=LED1_red;
  LED1_Green=LED1_green;
  LED1_Blue=LED1_blue;
  
  LED2_Red=LED2_red;
  LED2_Green=LED2_green;
  LED2_Blue=LED2_blue; 
}

//#pragma register_bank=2
#pragma vector = T1_VECTOR
__interrupt void pwmISR (void) {
    uint8 flags = T1STAT;
    // T1 ch 0
    if (flags & 0x01){          
      pwmPulse(LED1_Red,LED1_Green,LED1_Blue,LED2_Red);
     
    }
    T1STAT = ~ flags;
}
#pragma vector = T3_VECTOR
__interrupt void pwmISR3 (void) {
  pwmPulse3(LED2_Green,LED2_Blue);
}

