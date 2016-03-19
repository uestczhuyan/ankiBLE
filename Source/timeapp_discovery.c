/**************************************************************************************************
  Filename:       timeapp_discovery.c
  Revised:        $Date: 2011-12-16 15:46:52 -0800 (Fri, 16 Dec 2011) $
  Revision:       $Revision: 58 $

  Description:    Time App service and characteristic discovery routines.

  Copyright 2011 - 2013 Texas Instruments Incorporated. All rights reserved.

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
#include "gatt_profile_uuid.h"
#include "simpleBLEPeripheral.h"
#include "battservice.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Length of Characteristic declaration + handle with 16 bit UUID
#define CHAR_DESC_HDL_UUID16_LEN        7
   
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

// Attribute handle cache
uint16 timeAppHdlCache[HDL_CACHE_LEN];

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Attribute handles used during discovery
static uint16 timeAppSvcStartHdl;
static uint16 timeAppSvcEndHdl;
static uint8 timeAppEndHdlIdx;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static uint8 TimeAppDiscCurrTime( uint8 state, gattMsgEvent_t *pMsg );
static uint8 TimeAppDiscBatt( uint8 state, gattMsgEvent_t *pMsg );



/*********************************************************************
 * @fn      timeAppDiscGattMsg()
 *
 * @brief   Handle GATT messages for characteristic discovery. 
 *
 * @param   state - Discovery state.
 * @param   pMsg - GATT message.
 *
 * @return  New discovery state.
 */
uint8 timeAppDiscGattMsg( uint8 state, gattMsgEvent_t *pMsg )
{
  // Execute discovery function for service
  do
  {
    switch ( state & 0xF0 )
    {
      // Current time service
      case DISC_CURR_TIME_START:
        state = TimeAppDiscCurrTime( state, pMsg );
        if ( state == DISC_FAILED )
        {
          state = DISC_BATT_START;
        }
        else if ( state == DISC_IDLE )
        {
          state = DISC_BATT_START;
        }
        break;
      // Battery service
      case DISC_BATT_START:
        state = TimeAppDiscBatt( state, pMsg );
        if ( state == DISC_FAILED || state == DISC_IDLE )
        {
          state = DISC_IDLE;
        }
        break;
      default:
        break;
    }
  } while ( (state != 0) && ((state & 0x0F) == 0) );
  
  return state;    
}

/*********************************************************************
 * @fn      timeAppDiscStart()
 *
 * @brief   Start service discovery. 
 *
 *
 * @return  New discovery state.
 */
uint8 timeAppDiscStart( void )
{
  // Clear handle cache
  osal_memset( timeAppHdlCache, 0, sizeof(timeAppHdlCache) );
  
  // Start discovery with first service
  return timeAppDiscGattMsg( DISC_CURR_TIME_START, NULL );
}

/*********************************************************************
 * @fn      TimeAppDiscCurrTime()
 *
 * @brief   Current time service and characteristic discovery. 
 *
 * @param   state - Discovery state.
 * @param   pMsg - GATT message.
 *
 * @return  New discovery state.
 */
static uint8 TimeAppDiscCurrTime( uint8 state, gattMsgEvent_t *pMsg )
{
  uint8 newState = state;
  
  switch ( state )
  {
    case DISC_CURR_TIME_START:  
      {
        uint8 uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(CURRENT_TIME_SERV_UUID),
                                         HI_UINT16(CURRENT_TIME_SERV_UUID) };

        // Initialize service discovery variables
        timeAppSvcStartHdl = timeAppSvcEndHdl = 0;
        timeAppEndHdlIdx = 0;
        
        // Discover service by UUID
        GATT_DiscPrimaryServiceByUUID( timeAppConnHandle, uuid,
                                       ATT_BT_UUID_SIZE, simpleBLEPeripheral_TaskID );      

        newState = DISC_CURR_TIME_SVC;
      } 
      break;

    case DISC_CURR_TIME_SVC:
      // Service found, store handles
      if ( pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP &&
           pMsg->msg.findByTypeValueRsp.numInfo > 0 )
      {
        timeAppSvcStartHdl = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;
        timeAppSvcEndHdl = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;
      }
      
      // If procedure complete
      if ( ( pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP  && 
             pMsg->hdr.status == bleProcedureComplete ) ||
           ( pMsg->method == ATT_ERROR_RSP ) )
      {
        // If service found
        if ( timeAppSvcStartHdl != 0 )
        {
          // Discover all characteristics
          GATT_DiscAllChars( timeAppConnHandle, timeAppSvcStartHdl,
                             timeAppSvcEndHdl, simpleBLEPeripheral_TaskID );
          
          newState = DISC_CURR_TIME_CHAR;
        }
        else
        {
          // Service not found
          newState = DISC_FAILED;
        }
      }    
      break;

    case DISC_CURR_TIME_CHAR:
      {
        uint8   i;
        uint8   *p;
        uint16  handle;
        uint16  uuid;
        
        // Characteristics found
        if ( pMsg->method == ATT_READ_BY_TYPE_RSP &&
             pMsg->msg.readByTypeRsp.numPairs > 0 && 
             pMsg->msg.readByTypeRsp.len == CHAR_DESC_HDL_UUID16_LEN)
        {
          // For each characteristic declaration
          p = pMsg->msg.readByTypeRsp.dataList;
          for ( i = pMsg->msg.readByTypeRsp.numPairs; i > 0; i-- )
          {
            // Parse characteristic declaration
            handle = BUILD_UINT16(p[3], p[4]);
            uuid = BUILD_UINT16(p[5], p[6]);
                   
            // If looking for end handle
            if ( timeAppEndHdlIdx != 0 )
            {
              // End handle is one less than handle of characteristic declaration
              timeAppHdlCache[timeAppEndHdlIdx] = BUILD_UINT16(p[0], p[1]) - 1;
              
              timeAppEndHdlIdx = 0;
            }

            // If UUID is of interest, store handle
            switch ( uuid )
            {
              case CURRENT_TIME_UUID:
                timeAppHdlCache[HDL_CURR_TIME_CT_TIME_START] = handle;
                timeAppEndHdlIdx = HDL_CURR_TIME_CT_TIME_END;
                break;
                
              case LOCAL_TIME_INFO_UUID:
                timeAppHdlCache[HDL_CURR_TIME_LOC_INFO] = handle;
                break;
                
              case REF_TIME_INFO_UUID:
                timeAppHdlCache[HDL_CURR_TIME_REF_INFO] = handle;
                break;
                
              default:
                break;
            }
            
            p += CHAR_DESC_HDL_UUID16_LEN;
          }
          
        }
          
        // If procedure complete
        if ( ( pMsg->method == ATT_READ_BY_TYPE_RSP  && 
               pMsg->hdr.status == bleProcedureComplete ) ||
             ( pMsg->method == ATT_ERROR_RSP ) )
        {
          // Special case of end handle at end of service
          if ( timeAppEndHdlIdx != 0 )
          {
            timeAppHdlCache[timeAppEndHdlIdx] = timeAppSvcEndHdl;
            timeAppEndHdlIdx = 0;
          }
          
          // If didn't find time characteristic
          if ( timeAppHdlCache[HDL_CURR_TIME_CT_TIME_START] == 0 )
          {
            newState = DISC_FAILED;
          }
          else if ( timeAppHdlCache[HDL_CURR_TIME_CT_TIME_START] <
                   timeAppHdlCache[HDL_CURR_TIME_CT_TIME_END] )
          {
            // Discover characteristic descriptors
            GATT_DiscAllCharDescs( timeAppConnHandle,
                                   timeAppHdlCache[HDL_CURR_TIME_CT_TIME_START] + 1,
                                   timeAppHdlCache[HDL_CURR_TIME_CT_TIME_END],
                                   simpleBLEPeripheral_TaskID );
                                        
            newState = DISC_CURR_TIME_CT_TIME_CCCD;
          }
          else
          {
            newState = DISC_IDLE;
          }
        }
      }      
      break;

    case DISC_CURR_TIME_CT_TIME_CCCD:
      {
        uint8 i;
        
        // Characteristic descriptors found
        if ( pMsg->method == ATT_FIND_INFO_RSP &&
             pMsg->msg.findInfoRsp.numInfo > 0 && 
             pMsg->msg.findInfoRsp.format == ATT_HANDLE_BT_UUID_TYPE )
        {
          // For each handle/uuid pair
          for ( i = 0; i < pMsg->msg.findInfoRsp.numInfo; i++ )
          {
            // Look for CCCD
            if ( (pMsg->msg.findInfoRsp.info.btPair[i].uuid[0] ==
                  LO_UINT16(GATT_CLIENT_CHAR_CFG_UUID)) &&
                 (pMsg->msg.findInfoRsp.info.btPair[i].uuid[1] ==
                  HI_UINT16(GATT_CLIENT_CHAR_CFG_UUID)) )
            {
              // CCCD found
              timeAppHdlCache[HDL_CURR_TIME_CT_TIME_CCCD] =
                pMsg->msg.findInfoRsp.info.btPair[i].handle;
              
              break;
            }
          }
        }
        
        // If procedure complete
        if ( ( pMsg->method == ATT_FIND_INFO_RSP  && 
               pMsg->hdr.status == bleProcedureComplete ) ||
             ( pMsg->method == ATT_ERROR_RSP ) )
        {
          newState = DISC_IDLE;
        }
      }
      break;

    default:
      break;
  }
  
  return newState;
}

/*********************************************************************
 * @fn      TimeAppDiscBatt()
 *
 * @brief   Battery service and characteristic discovery. 
 *
 * @param   state - Discovery state.
 * @param   pMsg - GATT message.
 *
 * @return  New discovery state.
 */
static uint8 TimeAppDiscBatt( uint8 state, gattMsgEvent_t *pMsg )
{
  uint8 newState = state;
  
  switch ( state )
  {
    case DISC_BATT_START:  
      {
        uint8 uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(BATT_SERV_UUID),
                                         HI_UINT16(BATT_SERV_UUID) };

        // Initialize service discovery variables
        timeAppSvcStartHdl = timeAppSvcEndHdl = 0;
        timeAppEndHdlIdx = 0;
        
        // Discover service by UUID
        GATT_DiscPrimaryServiceByUUID( timeAppConnHandle, uuid,
                                       ATT_BT_UUID_SIZE, simpleBLEPeripheral_TaskID );      

        newState = DISC_BATT_SVC;
      } 
      break;

    case DISC_BATT_SVC:
      HalLcdWriteString("DIS EVT SVC",HAL_LCD_LINE_8);
      // Service found, store handles
      if ( pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP &&
           pMsg->msg.findByTypeValueRsp.numInfo > 0 )
      {
        timeAppSvcStartHdl = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;
        timeAppSvcEndHdl = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;
      }
      
      // If procedure complete
      if ( ( pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP  && 
             pMsg->hdr.status == bleProcedureComplete ) ||
           ( pMsg->method == ATT_ERROR_RSP ) )
      {
        // If service found
        if ( timeAppSvcStartHdl != 0 )
        {
          // Discover all characteristics
          GATT_DiscAllChars( timeAppConnHandle, timeAppSvcStartHdl,
                             timeAppSvcEndHdl, simpleBLEPeripheral_TaskID );
          
          newState = DISC_BATT_CHAR;
        }
        else
        {
          // Service not found
          newState = DISC_FAILED;
        }
      }    
      break;

    case DISC_BATT_CHAR:
      {
        HalLcdWriteString("DIS EVT CHAR",HAL_LCD_LINE_7);
        uint8   i;
        uint8   *p;
        uint16  handle;
        uint16  uuid;
        
        // Characteristics found
        if ( pMsg->method == ATT_READ_BY_TYPE_RSP &&
             pMsg->msg.readByTypeRsp.numPairs > 0 && 
             pMsg->msg.readByTypeRsp.len == CHAR_DESC_HDL_UUID16_LEN )
        {
          // For each characteristic declaration
          p = pMsg->msg.readByTypeRsp.dataList;
          for ( i = pMsg->msg.readByTypeRsp.numPairs; i > 0; i-- )
          {
            // Parse characteristic declaration
            handle = BUILD_UINT16(p[3], p[4]);
            uuid = BUILD_UINT16(p[5], p[6]);
                   
            // If looking for end handle
            if ( timeAppEndHdlIdx != 0 )
            {
              // End handle is one less than handle of characteristic declaration
              timeAppHdlCache[timeAppEndHdlIdx] = BUILD_UINT16(p[0], p[1]) - 1;
              
              timeAppEndHdlIdx = 0;
            }

            // If UUID is of interest, store handle
            switch ( uuid )
            {
              case BATT_LEVEL_UUID:
                timeAppHdlCache[HDL_BATT_LEVEL_START] = handle;
                timeAppEndHdlIdx = HDL_BATT_LEVEL_END;
                break;
                                
              default:
                break;
            }
            
            p += CHAR_DESC_HDL_UUID16_LEN;
          }
          
        }
          
        // If procedure complete
        if ( ( pMsg->method == ATT_READ_BY_TYPE_RSP  && 
               pMsg->hdr.status == bleProcedureComplete ) ||
             ( pMsg->method == ATT_ERROR_RSP ) )
        {
          // Special case of end handle at end of service
          if ( timeAppEndHdlIdx != 0 )
          {
            timeAppHdlCache[timeAppEndHdlIdx] = timeAppSvcEndHdl;
            timeAppEndHdlIdx = 0;
          }
          
          // If didn't find mandatory characteristic
          if ( timeAppHdlCache[HDL_BATT_LEVEL_START] == 0 )
          {
            newState = DISC_FAILED;
          }
          else if ( timeAppHdlCache[HDL_BATT_LEVEL_START] <
                    timeAppHdlCache[HDL_BATT_LEVEL_END] )
          {
            // Discover characteristic descriptors
            GATT_DiscAllCharDescs( timeAppConnHandle,
                                   timeAppHdlCache[HDL_BATT_LEVEL_START] + 1,
                                   timeAppHdlCache[HDL_BATT_LEVEL_END],
                                   simpleBLEPeripheral_TaskID );
                                        
            newState = DISC_BATT_LVL_CCCD;
          }
          else
          {
            newState = DISC_IDLE;
          }
        }
      }      
      break;

    case DISC_BATT_LVL_CCCD:
      {
        HalLcdWriteString("DIS EVT CCD",HAL_LCD_LINE_6);
        uint8 i;
        
        // Characteristic descriptors found
        if ( pMsg->method == ATT_FIND_INFO_RSP &&
             pMsg->msg.findInfoRsp.numInfo > 0 && 
             pMsg->msg.findInfoRsp.format == ATT_HANDLE_BT_UUID_TYPE )
        {
          // For each handle/uuid pair
          for ( i = 0; i < pMsg->msg.findInfoRsp.numInfo; i++ )
          {
            // Look for CCCD
            if ( (pMsg->msg.findInfoRsp.info.btPair[i].uuid[0] ==
                  LO_UINT16(GATT_CLIENT_CHAR_CFG_UUID)) &&
                 (pMsg->msg.findInfoRsp.info.btPair[i].uuid[1] ==
                  HI_UINT16(GATT_CLIENT_CHAR_CFG_UUID)) )
            {
              // CCCD found
              timeAppHdlCache[HDL_BATT_LEVEL_CCCD] =
                pMsg->msg.findInfoRsp.info.btPair[i].handle;
              
              break;
            }
          }
        }
        
        // If procedure complete
        if ( ( pMsg->method == ATT_FIND_INFO_RSP  && 
               pMsg->hdr.status == bleProcedureComplete ) ||
             ( pMsg->method == ATT_ERROR_RSP ) )
        {
          newState = DISC_IDLE;
        }
      }
      break;

    default:
      break;
  }
  
  return newState;
}
