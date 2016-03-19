/**************************************************************************************************
  Filename:       timeapp_config.c
  Revised:        $Date: 2012-01-20 14:33:21 -0800 (Fri, 20 Jan 2012) $
  Revision:       $Revision: 63 $

  Description:    Time App characteristic configuration routines.

  Copyright 2011 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "hal_lcd.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "simpleBLEPeripheral.h"

/*********************************************************************
 * MACROS
 */

// Used to determine the end of timeAppConfigList[]
#define TIMEAPP_CONFIG_MAX      ( sizeof(timeAppConfigList) / sizeof(uint8) )

/*********************************************************************
 * CONSTANTS
 */

// Array of handle cache indexes.  This list determines the
// characteristics that are read or written during configuration.
const uint8 timeAppConfigList[] =
{
  HDL_CURR_TIME_CT_TIME_START,            // Current time
  HDL_BATT_LEVEL_START,                   // Battery level
  HDL_CURR_TIME_CT_TIME_CCCD,             // Current time CCCD
  HDL_BATT_LEVEL_CCCD                    // Battery level CCCD
};

// start index of alert notification control point in config list
#define TIMEAPP_ALERT_NTF_CTRL_START    18

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      timeAppConfigNext()
 *
 * @brief   Perform the characteristic configuration read or
 *          write procedure.
 *
 * @param   state - Configuration state.
 *
 * @return  New configuration state.
 */
uint8 timeAppConfigNext( uint8 state )
{
  attReadReq_t  readReq;
  attWriteReq_t writeReq;
  bool          read;
  //static uint8  alertNtfCtrlCmd;
  
  // Find next non-zero cached handle of interest
  while ( state < TIMEAPP_CONFIG_MAX &&
          timeAppHdlCache[timeAppConfigList[state]] == 0 )
  {
    state++;
  }

  // Return if reached end of list
  if ( state == TIMEAPP_CONFIG_MAX )
  {
    return TIMEAPP_CONFIG_CMPL;
  }

  // Determine what to do with characteristic
  switch ( timeAppConfigList[state] )
  {
    // Set notification for these characteristics
    case HDL_CURR_TIME_CT_TIME_CCCD:
    case HDL_BATT_LEVEL_CCCD:
      read = FALSE;
      writeReq.len = 2;
      writeReq.value[0] = LO_UINT16(GATT_CLIENT_CFG_NOTIFY);
      writeReq.value[1] = HI_UINT16(GATT_CLIENT_CFG_NOTIFY);
      writeReq.sig = 0;
      writeReq.cmd = 0;
      break;
    default:
      break;
  }

  // Do a GATT read or write
  if ( read )
  {
    readReq.handle = timeAppHdlCache[timeAppConfigList[state]];
    GATT_ReadCharValue( timeAppConnHandle, &readReq, simpleBLEPeripheral_TaskID );
  }
  else
  {
    writeReq.handle = timeAppHdlCache[timeAppConfigList[state]];
    GATT_WriteCharValue( timeAppConnHandle, &writeReq, simpleBLEPeripheral_TaskID );
  }

  return state;
}

/*********************************************************************
 * @fn      timeAppConfigGattMsg()
   *
 * @brief   Handle GATT messages for characteristic configuration.
 *
 * @param   state - Discovery state.
 * @param   pMsg - GATT message.
 *
 * @return  New configuration state.
 */
uint8 timeAppConfigGattMsg( uint8 state, gattMsgEvent_t *pMsg )
{
  //HalLcdWriteStringValue( "gatt msg ",state, 10,  HAL_LCD_LINE_5 );
  if ( state > TIMEAPP_CONFIG_MAX )
  {
    return TIMEAPP_CONFIG_CMPL;
  }
  
  if ( (pMsg->method == ATT_READ_RSP || pMsg->method == ATT_WRITE_RSP) &&
       (pMsg->hdr.status == SUCCESS) )
  {
    // Process response
    switch ( timeAppConfigList[state] )
    {
      case HDL_CURR_TIME_CT_TIME_START:
        HalLcdWriteStringValue( "time start ",state, 10,  HAL_LCD_LINE_5 );
        // Set clock to time read from time server
        //timeAppClockSet( pMsg->msg.readRsp.value );
        break;

      case HDL_BATT_LEVEL_START:
        // Display battery level
        HalLcdWriteStringValue( "batt start ",state, 10,  HAL_LCD_LINE_4 );
        HalLcdWriteStringValue( "S_Battery%", pMsg->msg.readRsp.value[0], 10, HAL_LCD_LINE_1 );
        break;

      case HDL_CURR_TIME_CT_TIME_CCCD:
        HalLcdWriteStringValue( "time ccd ",state, 10,  HAL_LCD_LINE_2 );
        break;

      default:
        break;
    }
  }

  return timeAppConfigNext( state + 1 );
}

