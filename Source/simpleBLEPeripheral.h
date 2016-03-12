/**************************************************************************************************
  Filename:       simpleBLEperipheral.h
  Revised:        $Date: 2010-08-01 14:03:16 -0700 (Sun, 01 Aug 2010) $
  Revision:       $Revision: 23256 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  definitions and prototypes.

  Copyright 2010 - 2011 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef SIMPLEBLEPERIPHERAL_H
#define SIMPLEBLEPERIPHERAL_H

#include "gatt.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */


// Simple BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT                              0x0001
// LED Ïà¹ØÊÂ¼þ
#define SBP_PERIODIC_EVT                                  0x0002
// ºìÍâÊÂ¼þ
#define SBP_REDLINE_EVT                                  0x0004
//IOS ANSC ÊÂ¼þ
#define START_DISCOVERY_EVT                           0x0008
  
//LED change interval , 50fps
#define SBP_PERIODIC_EVT_PERIOD                   20 
// ºìÍâµÆ¹âÑÓÊ±
#define SBP_LINE_EVT_PERIOD                   10000

  
  
  enum
{
  DISC_IDLE = 0x00,                       // Idle state
  
  DISC_CURR_TIME_START = 0x10,            // Current time service
  DISC_CURR_TIME_SVC,                     // Discover service
  DISC_CURR_TIME_CHAR,                    // Discover all characteristics
  DISC_CURR_TIME_CT_TIME_CCCD,            // Discover CT time CCCD


  DISC_BATT_START = 0x20,                 // Battery service
  DISC_BATT_SVC,                          // Discover service
  DISC_BATT_CHAR,                         // Discover all characteristics
  DISC_BATT_LVL_CCCD,                     // Discover battery level CCCD
  
  DISC_FAILED = 0xFF                      // Discovery failed
};

// Time App handle cache indexes
enum
{
  HDL_CURR_TIME_CT_TIME_START,            // Current time start handle
  HDL_CURR_TIME_CT_TIME_END,              // Current time end handle
  HDL_CURR_TIME_CT_TIME_CCCD,             // Current time CCCD

  HDL_BATT_LEVEL_START,                   // Battery level start handle
  HDL_BATT_LEVEL_END,                     // Battery level end handle
  HDL_BATT_LEVEL_CCCD,                    // Battery level CCCD


  HDL_CACHE_LEN
};


// Configuration states
#define TIMEAPP_CONFIG_START              0x00
#define TIMEAPP_CONFIG_CMPL               0xFF

#define TIMEAPP_CONFIG_CONN_START         15
/*********************************************************************
 * MACROS
 */
 /*********************************************************************
 * LOCAL VARIABLES
 */
extern  uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

//À¶ÑÀÊÇ·ñÊÇÁ¬½Ó×´Ì¬ 1±íÊ¾À¶ÑÀÁ¬½Ó£¬0 ±íÊ¾À¶ÑÀ¶Ï¿ª£¬-1 ±íÊ¾À¶ÑÀÔÚÓ­±öµÆ·¶Î§ÍâÃæ
static int8 isBlueToothConnected = 0;


// Connection handle
extern uint16 timeAppConnHandle;

// Handle cache
extern uint16 timeAppHdlCache[HDL_CACHE_LEN];



/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void SimpleBLEPeripheral_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events );
   
void setLED_EVT(uint8 value);

extern int8 getBlueToothStatus();


/* 
 * Time App characteristic configuration functions
 */
extern uint8 timeAppConfigNext( uint8 state );
extern uint8 timeAppConfigGattMsg( uint8 state, gattMsgEvent_t *pMsg );

/* 
 * Time App indication and notification handling functions
 */
extern void timeAppIndGattMsg( gattMsgEvent_t *pMsg );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEBLEPERIPHERAL_H */
