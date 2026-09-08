/*
 * Iso.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "Iso.h"
#include "bsp_timer.h"
#include "atten.h"
#include "bsp_init.h"
#include "bsp_timer.h"
#include "bsp_uart.h"

#define IsoPrint(...) do { if (IsoFlag) DebugPrint(__VA_ARGS__); } while (0)

u8 IsoFlag = 0;
u8 IsoPwrInitFlag = ON;
//u8 IsoPwrInitFlag = OFF;

static ISO_STATEe IsoState = ISO_STATE_IDLE;

void Iso_Init(void)
{
  Iso_SetPwrInitFlag(ON);
  bsp_timer_set(TimerISOCheck, Time1Sec);

}

void Iso_Task(void)
{
  if(bsp_timer_TimeOverCheck(TimerISOCheck)){
      if(iMySts.Flag2.Bit.IsoCheck){
        Iso_Check();
      }
      bsp_timer_set(TimerISOCheck, Time1Sec);
  }	
}

void Iso_Check(void)
{
  if(IsoPwrInitFlag == ON || iMySts.Flag2.Bit.IsoReCheck == ON){

    if(IsoState != ISO_STATE_IDLE){
      if(bsp_timer_TimeOverCheck(TimerISOTimeOut)) {
        IsoState = ISO_STATE_FAIL;
        IsoPrint("\r\n %d][ISO] TimerISOTimeOut ", HAL_GetTick());
      }
    }

    switch(IsoState)
    {
      case ISO_STATE_IDLE:
        bsp_timer_set(TimerISOTimeOut, Time2Min);
        //bsp_timer_set(TimerISOTimeOut, Time5Sec);
        IsoState = ISO_STATE_UE_CHECK;
        iMySts.IsoMsg = ISO_MSG_CHECK;
        IsoPrint("\r\n %d][ISO] ISO_STATE_IDLE ", HAL_GetTick());
        break;

      case ISO_STATE_UE_CHECK:
        if(iMySts.RxOutputPower < iMySts.IsoThreshold){
          Init_RxAmpOn();
          iMySts.RxMaxGain = RX_MAX_GAIN;
          iMySts.IsoAtt = 17;
          Atten_SetRxAtt(RX_ATT2);
          IsoState = ISO_STATE_CHECK;
          iMySts.IsoMsg = ISO_MSG_CHECK;
          IsoPrint("\r\n %d][ISO] ISO_STATE_UE_CHECK ", HAL_GetTick());
        }
        break;

      case ISO_STATE_CHECK:
        if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
            if(iMySts.IsoAtt > 15){
              IsoState = ISO_STATE_FAIL;
              IsoPrint("\r\n %d][ISO] ISO_STATE_FAIL ", HAL_GetTick());
            }
            else if(iMySts.IsoAtt > 2){
              IsoState = ISO_STATE_LIMIT;
              IsoPrint("\r\n %d][ISO] ISO_STATE_LIMIT ", HAL_GetTick());
            }
            else{
              IsoState = ISO_STATE_FAIL;
              IsoPrint("\r\n %d][ISO] ISO_STATE_FAIL ", HAL_GetTick());
            }
          }
        else{
          if(iMySts.IsoAtt == 0){
            IsoState = ISO_STATE_OK;
            IsoPrint("\r\n %d][ISO] ISO_STATE_OK ", HAL_GetTick());
          }
          else{
            iMySts.IsoAtt--;
            Atten_SetRxAtt(RX_ATT2);
            IsoPrint("\r\n %d][ISO] IsoAtt %ddB ", HAL_GetTick(), iMySts.IsoAtt);
          }
          
        }
        break;

      case ISO_STATE_LIMIT:
        Init_TxAmpOff();
        Init_RxAmpOff();
        if(iMySts.Flag2.Bit.IsoLimitRun == ON){
          iMySts.IsoMeasure = RX_MAX_GAIN + iMySts.IsoAtt;
          iMySts.RxMaxGain = iMySts.IsoMeasure - ISO_MAX_ATT;
          iMySts.IsoAtt = ISO_MAX_ATT;
          Atten_SetRxAtt(RX_ATT2);  
          IsoPwrInitFlag = OFF;
          iMySts.Flag2.Bit.IsoReCheck = OFF;
          iMySts.IsoMsg = ISO_MSG_LIMIT;
          bsp_timer_Cancel(TimerISOTimeOut);
        }
        else{
          IsoState = ISO_STATE_FAIL;
        }
        break;

      case ISO_STATE_OK:
        iMySts.IsoMeasure = RX_MAX_GAIN + ISO_MAX_ATT;
        iMySts.RxMaxGain = iMySts.IsoMeasure;
        Init_TxAmpOn();
        Init_RxAmpOn();
        IsoPwrInitFlag = OFF;
        iMySts.Flag2.Bit.IsoReCheck = OFF;
        iMySts.IsoMsg = ISO_MSG_OK;
        bsp_timer_Cancel(TimerISOTimeOut);
        break;

      case ISO_STATE_FAIL:
        Init_TxAmpOff();
        Init_RxAmpOff();

        IsoPwrInitFlag = OFF;
        iMySts.Flag2.Bit.IsoReCheck = OFF;
        iMySts.IsoMsg = ISO_MSG_FAIL;

        bsp_timer_Cancel(TimerISOTimeOut);
        break;

      default:
        IsoState = ISO_STATE_IDLE;
        iMySts.IsoMsg = ISO_MSG_IDLE;
        break;
    }
  }
}


void Iso_SetPwrInitFlag(u8 flag)
{
  IsoPwrInitFlag = flag;
  IsoState = ISO_STATE_IDLE;
}

void Iso_SetIsoReCheckFlag(u8 flag)
{
  iMySts.Flag2.Bit.IsoReCheck = flag;
  if(flag == ON) IsoState = ISO_STATE_IDLE;
}

u8 Iso_GetIsoReCheckFlag(void)
{
  return iMySts.Flag2.Bit.IsoReCheck;
}