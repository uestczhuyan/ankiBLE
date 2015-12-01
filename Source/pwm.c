#include <ioCC2540.h>
#include "pwm.h"
#include "simpleBLEPeripheral.h"
#include "OSAL.h"


#define INIT_RGB 1

#define RGB_MAX 200

#define COUNTER 100   

#define STATUS_POWER_LOW 1
#define STATUS_POWER_CHARGING 2
#define STATUS_POWER_HIGH 4     
#define STATUS_HAS_MSG 8 

#define SWITCHQI P2_0

int16 MAX_R = RGB_MAX;
int16 MAX_G = RGB_MAX;
int16 MAX_B = RGB_MAX;
uint8 STATUS = 0;

int8 updown = 1,count = 0;

int16 LED1_Red = INIT_RGB;
int16 LED1_Green = INIT_RGB;
int16 LED1_Blue = INIT_RGB;

int16 LED2_Red = INIT_RGB;
int16 LED2_Green = INIT_RGB;
int16 LED2_Blue = INIT_RGB;

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
  
  T1CC1L = (uint8)LED1_Red;
  T1CC1H = (uint8)0x0;
  T1CC2L = (uint8)LED1_Green;
  T1CC2H = (uint8)0x0;
  T1CC3L = (uint8)LED1_Blue;
  T1CC3H = (uint8)0x0;
  T1CC4L = (uint8)LED2_Red;
  T1CC4H = (uint8)0x0;
  
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
  
  T3CC0 = (uint8)LED2_Green;
  T3CC1 = (uint8)LED2_Blue;
  
  T3IE =1;               // Enable T3 cpu interrupt
}

void init_QI_Switch(){
  //把2.0 脚设置为 QI开关电路
  P2DIR |= 0X01;
  P2SEL &=~0X01;
  
  SWITCHQI = 1;
}

void PWM_init()
{
  //init_QI_Switch();
  Timer1_init();
  
  Timer3_init();
  
 
  
  //Timer4_init();
  
  EA=1;
}


void pwmPulse()
{ 
  int16 l1r,l1g,l1b,l2r;
  
  l1r=LED1_Red;
  l1g=LED1_Green;
  l1b=LED1_Blue;
  l2r = LED2_Red;
  
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

void pwmPulse3()
{
  int16 g,b;
  g=LED2_Green;
  b=LED2_Blue;
  // Set up the timer registers
  

  T3CC0 = (uint8)g;
  T3CC1 = (uint8)b;
  // Reset timer
  //T4CNTL = 0;
}

//#pragma register_bank=2
#pragma vector = T1_VECTOR
__interrupt void pwmISR (void) {
    uint8 flags = T1STAT;
    // T1 ch 0
    /*
    if (flags & 0x01){          
      pwmPulse(LED1_Red,LED1_Green,LED1_Blue,LED2_Red);
     
    }
    */
    T1STAT = ~ flags;
}
#pragma vector = T3_VECTOR
__interrupt void pwmISR3 (void) {
  //pwmPulse3(LED2_Green,LED2_Blue);
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


void setValus(uint8 *value){
  uint8 pos = 1;
  if(value[0] & STATUS_POWER_LOW){
    pos = 4;
  }else if(value[0] & STATUS_POWER_CHARGING){
    pos = 7;
  }else if(value[0] & STATUS_POWER_HIGH){
    pos = 10;
  }else if(value[0] & STATUS_HAS_MSG){
    pos = 13;
  }else{
    pos = 1; 
  }
  MAX_R = value[pos];
  MAX_G = value[pos+1];
  MAX_B = value[pos+2];
  STATUS = value[0];
}

void LedChange(){
  //如果手机再充电 那么需要设置事件进行循环驱动变化
  if(STATUS & STATUS_POWER_LOW
     || STATUS & STATUS_POWER_CHARGING
     || STATUS & STATUS_POWER_HIGH){
       
    setLED_EVT();
    
    if(updown){
      count++;
    }else{
      count--;
    }
    
    
    LED1_Red = 1 + count*MAX_R/COUNTER;
    LED1_Green = 1 + count*MAX_G/COUNTER;
    LED1_Blue = 1 + count*MAX_B/COUNTER;
    
    LED2_Red = 10 + (50-count)*MAX_R/COUNTER;
    LED2_Green = 10 + (50-count)*MAX_G/COUNTER;
    LED2_Blue = 10 + (50-count)*MAX_B/COUNTER;
    /*
    int i = 400;
    
    LED1_Red = i ;
    LED1_Green = i ;
    LED1_Blue = i ;
    LED2_Red=i;
    LED2_Green=i;
    LED2_Blue=i;
    */
    
   
    /*
     LED2_Red = 1 + count*MAX_R/COUNTER;
    LED2_Green = 1 + count*MAX_G/COUNTER;
    LED2_Blue = 1 + count*MAX_B/COUNTER;
    */
    
    if(count >= 50){
      updown = 0;
    }
    if(count <= 0){
      updown = 1;
    }
  }else{
    LED1_Red = MAX_R;
    LED1_Green = MAX_G;
    LED1_Blue = MAX_B;
    LED2_Red = MAX_R;
    LED2_Green = MAX_G;
    LED2_Blue = MAX_B;
  }
  
  pwmPulse();
  pwmPulse3();
}
