/*
 * Iso.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "Iso.h"
#include "atten.h"
#include "bsp_init.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "Alarm.h"

#define IsoPrint(...) do { if (IsoFlag) DebugPrint(__VA_ARGS__); } while (0)

u8 IsoFlag = 0;
u8 IsoPwrInitFlag = OFF;

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

    Iso_CheckTimeout();

    switch(IsoState)
    {
      case ISO_STATE_IDLE:
        Iso_StateIdle();
        break;

      case ISO_STATE_UE_CHECK:
        Iso_StateUeCheck();
        break;

      case ISO_STATE_CHECK:
        Iso_StateCheck();
        break;

      case ISO_STATE_LIMIT:
        Iso_StateLimit();
        break;

      case ISO_STATE_OK:
        Iso_StateOk();
        break;

      case ISO_STATE_FAIL:
        Iso_StateFail();
        break;

      default:
        IsoState = ISO_STATE_IDLE;
        IsoPrint("\r\n %d][ISO] Iso_Check default ISO_STATE_IDLE ", HAL_GetTick());
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

void Iso_CheckTimeout(void)
{
  if(IsoState == ISO_STATE_IDLE) return;

  if(bsp_timer_TimeOverCheck(TimerISOTimeOut))
  {
    IsoState = ISO_STATE_FAIL;
    IsoPrint("\r\n %d][ISO] TimerISOTimeOut ",HAL_GetTick());
  }
}

void Iso_StateIdle(void)
{
  bsp_timer_set(TimerISOTimeOut, Time2Min);
  IsoState = ISO_STATE_UE_CHECK;
  iMySts.IsoMsg = ISO_MSG_CHECK;

  if(iMySts.SysFreq  == SYS_FREQ_900M){
      iMySts.RxMaxGain = RX_900M_MAX_GAIN;
  }
  else{
      iMySts.RxMaxGain = RX_18G_MAX_GAIN;
  }
  IsoPrint("\r\n %d][ISO] ISO_STATE_IDLE ", HAL_GetTick());
}

void Iso_StateUeCheck(void)
{
  if(iMySts.RxOutputPower < iMySts.IsoThreshold){
    Init_RxAmpOn();
    Atten_SetRxAtt(RX_ATT2, ISO_MAX_ATT);
    IsoState = ISO_STATE_CHECK;
    iMySts.IsoMsg = ISO_MSG_CHECK;
    IsoPrint("\r\n %d][ISO] ISO_STATE_UE_CHECK IsoAtt %ddB ", HAL_GetTick(), iMySts.IsoAtt);
  }
}

void Iso_StateCheck(void)
{
  if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
      if((iMySts.IsoAtt > ISO_LIMIT_ATT_MAX) || (iMySts.IsoAtt < ISO_LIMIT_ATT_MIN)){
        IsoState = ISO_STATE_FAIL;
        IsoPrint("\r\n %d][ISO] ISO_STATE_FAIL ", HAL_GetTick());
      }
      else{
        IsoState = ISO_STATE_LIMIT;
        IsoPrint("\r\n %d][ISO] ISO_STATE_LIMIT ", HAL_GetTick());
      }
    }
  else{
    if(iMySts.IsoAtt == 0){
      IsoState = ISO_STATE_OK;
      IsoPrint("\r\n %d][ISO] ISO_STATE_OK ", HAL_GetTick());
    }
    else if(iMySts.IsoAtt == 1){
      iMySts.IsoAtt --;
      Atten_SetRxAtt(RX_ATT2, iMySts.IsoAtt);
      IsoPrint("\r\n %d][ISO] IsoAtt %ddB ", HAL_GetTick(), iMySts.IsoAtt);
    }
    else{
      iMySts.IsoAtt -= 2;
      Atten_SetRxAtt(RX_ATT2, iMySts.IsoAtt);
      IsoPrint("\r\n %d][ISO] IsoAtt %d ", HAL_GetTick(), iMySts.IsoAtt);
    }
    
  }

}

void Iso_StateLimit(void)
{
  u8 Offset;

  Iso_CalGain();

  Init_AllAmpOff();

  Iso_SetEnd();

  if(iMySts.Flag2.Bit.IsoLimitRun == ON){
    iMySts.IsoMsg = ISO_MSG_LIMIT_RUN;
  }
  else{
    iMySts.IsoMsg = ISO_MSG_LIMIT;
  }

  if(iMySts.SysFreq  == SYS_FREQ_900M){
    Offset = ((RX_900M_MAX_GAIN + ISO_MAX_ATT) >> 1) - iMySts.IsoMeasure;
    if(Offset > ISO_62DB) Offset = ISO_62DB;
    Alarm_SetIso(ISO_74DB + Offset); 
  }
  else{
    Offset = ((RX_18G_MAX_GAIN + ISO_MAX_ATT) >> 1) - iMySts.IsoMeasure;
    if(Offset > ISO_62DB) Offset = ISO_62DB;
    Alarm_SetIso(ISO_77DB + Offset); 
  }
   

}

void Iso_StateOk(void)
{

  Iso_CalGain();

  Init_AllAmpOn();

  Iso_SetEnd();

  iMySts.IsoMsg = ISO_MSG_OK;
  
  Alarm_SetIso(ISO_77DB);
  

}

void Iso_StateFail(void)
{
  Init_AllAmpOff();

  Iso_SetEnd();

  iMySts.IsoMsg = ISO_MSG_FAIL;

  Alarm_SetIso(ISO_FAIL);
}

void Iso_CalGain(void)
{
  u16 cTemp;

  cTemp = iMySts.RxMaxGain + iMySts.IsoAtt;
  iMySts.IsoMeasure = cTemp >> 1;
  iMySts.RxMaxGain = cTemp - ISO_MAX_ATT;
}

void Iso_SetEnd(void)
{
  Atten_SetRxAtt(RX_ATT2, ISO_MAX_ATT);  
  IsoPwrInitFlag = OFF;
  iMySts.Flag2.Bit.IsoReCheck = OFF;
  bsp_timer_Cancel(TimerISOTimeOut);
}