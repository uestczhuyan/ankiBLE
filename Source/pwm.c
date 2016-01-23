#include <ioCC2540.h>
#include "pwm.h"
#include "simpleBLEPeripheral.h"
#include "OSAL.h"

#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "osal_snv.h"


#define INIT_RGB 255

#define RGB_MAX 200

#define COUNTER 50
#define CHANGE_COUNTER 80


#define STATUS_POWER_LOW 1
#define STATUS_POWER_CHARGING 2
#define STATUS_POWER_HIGH 4     
#define STATUS_HAS_MSG 8
#define STATUS_CONNECTED 16
#define STATUS_SLEEPING 32


#define SWITCHQI P0_3

#define HAL_RED_LINE_PIN       6

#define DARK_RGB  255

int16 MAX_R = RGB_MAX;
int16 MAX_G = RGB_MAX;
int16 MAX_B = RGB_MAX;
int16 R_K = 0;
int16 G_K = 0;
int16 B_K = 0;

uint8 STATUS = 0;
uint8 LAST_STATUS = 0;

char updown = 1,count = 0,changed=0;
//uint8 *temp_value1;
//uint8 *temp_value2;
int16 Change_G = 0;
int16 Change_R = 0;
int16 Change_B = 0;

int16 LED1_Red = INIT_RGB;
int16 LED1_Green = INIT_RGB;
int16 LED1_Blue = INIT_RGB;

int16 LED2_Red = INIT_RGB;
int16 LED2_Green = INIT_RGB;
int16 LED2_Blue = INIT_RGB;

uint16 all_counter = 0;

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

void init_QI_Switch(int8 on){
  if(on > 0){
    SWITCHQI = 1;
    //HalLcdWriteStringValue( "pos: up", on, 10,  HAL_LCD_LINE_6 );
  }else{
    SWITCHQI = 0;
    //HalLcdWriteStringValue( "pos: down", on, 10,  HAL_LCD_LINE_6 );
  }
}

void initRedLine(){
  P1DIR &= ~BV(HAL_RED_LINE_PIN);  //输入，外设
  P1SEL &= ~BV(HAL_RED_LINE_PIN);
}

void PWM_init()
{
  
  Timer1_init();
  
  //Timer3_init();
    
  //把2.0 脚设置为 QI开关电路
  P0DIR |= 0X08;
  P0SEL &=~0X08;
  
  
  initRedLine();
 
  
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

void changeColorRightNow(uint8 *value,uint8 *value2,uint8 isChange){
  
  uint8 pos = 2;
  uint8 *thisValue;
  if(value[0] & STATUS_POWER_LOW){
    thisValue = value;
    pos = 8;
  }else if(value[0] & STATUS_POWER_CHARGING){
    thisValue = value;
    pos = 14;
  }else if(value[0] & STATUS_POWER_HIGH){
    pos = 0;
    thisValue = value2;
  }else if(value[0] & STATUS_HAS_MSG){
    pos = 6;
    thisValue = value2;
  }else{
    thisValue = value;
    pos = 2; 
  }
  STATUS = value[0];
  
  MAX_R = thisValue[pos];
  MAX_G = thisValue[pos+1];
  MAX_B = thisValue[pos+2];
  R_K = (thisValue[pos+3] - thisValue[pos]) ;
  G_K = (thisValue[pos+4] - thisValue[pos+1]);
  B_K = (thisValue[pos+5] - thisValue[pos+2]);
  

  HalLcdWriteStringValue( "change:", MAX_R, 10,  HAL_LCD_LINE_4 );
  HalLcdWriteStringValue( "change:", R_K, 10,  HAL_LCD_LINE_5 );
  HalLcdWriteStringValue( "pos: ", value[0], 10,  HAL_LCD_LINE_6 );

  
  if(isChange != 0 ){
    // 1 表示 当前颜色变暗 然后变亮
    // 2 表示 有0变亮
    // 3 表示 由目前变暗
    changed = isChange;
    count = 1;
    updown = 1; 
    all_counter=0;
  }
}

//状态转移机  状态转移在此完成
void setValus(uint8 *value,uint8 *value2,uint8 isChange){
  //if(value[0]==92){
  //  return;
  //}

  //当前状态不能在此转移为本身的状态
  if(value[0] == STATUS){
    return;
  }
  
  //状态0 只能往 STATUS_CONNECTED 状态转移
  if(STATUS == 0 && value[0] & STATUS_CONNECTED){
     LAST_STATUS = STATUS;
     changeColorRightNow(value,value2,isChange);
     HalLcdWriteStringValue( "connected1:", value[0], 10,  HAL_LCD_LINE_1 );
     return;
  }else{
     HalLcdWriteStringValue( "connected1-:", value[0], 10,  HAL_LCD_LINE_1 );
  }
  
  //来的任何数据 都不改变灯光 等待迎宾灯完成后在进行灯光切换
  if(STATUS & STATUS_CONNECTED){
    HalLcdWriteStringValue( "connected2:", value[0], 10,  HAL_LCD_LINE_8 );
    return;
  }
  //处于灯光熄灭状态  可以被任何活动状态唤醒
  if(STATUS & STATUS_SLEEPING && value[0] > 0 && value[0] <16){
    //当前是活动状态，可以被任何状态替换。
    LAST_STATUS = STATUS;
    changeColorRightNow(value,value2,isChange);
  }
 
}

void LedChange(){
  
  all_counter++;
  
  if(STATUS & STATUS_CONNECTED){
    if(all_counter > 1000/SBP_PERIODIC_EVT_PERIOD * 5){
      //迎宾灯跑完5s 之后，切换到上一个状态
      
      HalLcdWriteStringValue( "BTS:", getBlueToothStatus(), 10,  HAL_LCD_LINE_7 );
      if(getBlueToothStatus() >0){
        uint8 *temp_value1=(uint8 *)osal_mem_alloc( sizeof( uint8 ) * 20);
        uint8 *temp_value2=(uint8 *)osal_mem_alloc( sizeof( uint8 ) * 20);
        osal_snv_read(0x80,20,temp_value1);
        osal_snv_read(0x95,20,temp_value2);
        STATUS = temp_value1[0];
        //HalLcdWriteStringValue( "ssstatus:", STATUS, 10,  HAL_LCD_LINE_7 );
        if(STATUS > 0){
          changed = 1;
          changeColorRightNow(temp_value1,temp_value2,changed);
          //HalLcdWriteStringValue( "statuxxs:", STATUS, 10,  HAL_LCD_LINE_7 );
        }else{
          changed = 3;
        }
      }else if(getBlueToothStatus() == -1){
         STATUS = STATUS_SLEEPING;
      }else{
        STATUS = 0;
      }
      LAST_STATUS = STATUS_CONNECTED;
    }
  }else if( STATUS > 0 && all_counter > 1000/SBP_PERIODIC_EVT_PERIOD * 20){
      //计时20 s 后关闭灯光
      LAST_STATUS = STATUS;
      if(getBlueToothStatus() == -1 || getBlueToothStatus() == 1){
         STATUS = STATUS_SLEEPING;
      }else{
        STATUS = 0;
      }
      changed = 3;
      count = 1;
      updown = 1; 
      all_counter=0;
  }
  
  //HalLcdWriteStringValue( "xxxxxxx:", STATUS, 10,  HAL_LCD_LINE_8 );
  
  //如果正在状态切换。那么就进行淡入淡出效果
  if(changed == 0){
    //如果手机再充电 那么需要设置事件进行循环驱动变化
    if(STATUS & STATUS_POWER_LOW
       || STATUS & STATUS_POWER_CHARGING
       || STATUS & STATUS_POWER_HIGH
       || STATUS & STATUS_CONNECTED){
         
         
      LED1_Red = MAX_R + count*R_K/COUNTER;
      LED1_Green = MAX_G + count*G_K/COUNTER;
      LED1_Blue = MAX_B + count*B_K/COUNTER;
      
      LED2_Red = LED1_Red;
      LED2_Green = LED1_Green;
      LED2_Blue = LED1_Blue;
      
      //HalLcdWriteStringValue( "xxxxxxx:", LED1_Green, 10,  HAL_LCD_LINE_8 );

     
      /*
      LED2_Red = MAX_R + (COUNTER -count)*R_K/COUNTER;
      LED2_Green = MAX_G + (COUNTER-count)*G_K/COUNTER;
      LED2_Blue = MAX_B + (COUNTER-count)*B_K/COUNTER;
      */

      
      if(updown)
        count++;
      else
        count--;
      
      if(count >= COUNTER)
        updown=0;
        //count = 0;
      if(count <= 0)
        updown=1;
      
      
    }else{
      LED1_Red = DARK_RGB;
      LED1_Green = DARK_RGB;
      LED1_Blue = DARK_RGB;
      LED2_Red = DARK_RGB;
      LED2_Green = DARK_RGB;
      LED2_Blue = DARK_RGB;
    }
  }else{
    //过渡效果中 那么计时器不开始
    all_counter = 0;
    
    //过渡效果
    if(changed == 1 || changed == 3){
      if(count == 1){
        Change_R = LED1_Red;
        Change_G = LED1_Green;
        Change_B = LED1_Blue;
      }
      //当前颜色变暗
      LED1_Red = Change_R + count*(DARK_RGB - Change_R)/CHANGE_COUNTER;
      LED1_Green = Change_G + count*(DARK_RGB - Change_G)/CHANGE_COUNTER;
      LED1_Blue = Change_B + count*(DARK_RGB - Change_B)/CHANGE_COUNTER;
      LED2_Red = Change_R + count*(DARK_RGB - Change_R)/CHANGE_COUNTER;
      LED2_Green = Change_G + count*(DARK_RGB - Change_G)/CHANGE_COUNTER;
      LED2_Blue = Change_B + count*(DARK_RGB - Change_B)/CHANGE_COUNTER;
    }else if(changed == 2){
      //当前颜色变到明亮
      LED1_Red = MAX_R + (CHANGE_COUNTER - count)*(DARK_RGB - MAX_R)/CHANGE_COUNTER;
      LED1_Green = MAX_G + (CHANGE_COUNTER - count)*(DARK_RGB - MAX_G)/CHANGE_COUNTER;
      LED1_Blue = MAX_B + (CHANGE_COUNTER - count)*(DARK_RGB - MAX_B)/CHANGE_COUNTER;
      LED2_Red = MAX_R + (CHANGE_COUNTER - count)*(DARK_RGB - MAX_R)/CHANGE_COUNTER;
      LED2_Green = MAX_G + (CHANGE_COUNTER - count)*(DARK_RGB - MAX_G)/CHANGE_COUNTER;
      LED2_Blue = MAX_B + (CHANGE_COUNTER - count)*(DARK_RGB - MAX_B)/CHANGE_COUNTER;
    }
    
    count++;
    if(count > CHANGE_COUNTER){
      if(changed != 2){
        count = 1;
        if(changed == 3){
          changed = 0;
        }else{
          changed = 2;
        }
        pwmPulse();
        pwmPulse3();
        setLED_EVT(SBP_PERIODIC_EVT_PERIOD*5);
        return;
      }else{
        count = 1;
        updown = 1;
        changed = 0;
      }
    }
    
  }
  
  pwmPulse();
  pwmPulse3();
  
  setLED_EVT(SBP_PERIODIC_EVT_PERIOD);
}
