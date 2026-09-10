/*
 * Osc.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "Osc.h"
#include "project.h"
#include "bsp_timer.h"
#include "bsp_init.h"
#include "Iso.h"
#include "Alarm.h"
#include "bsp_uart.h"

#define OscPrint(...) do { if (OscFlag) DebugPrint(__VA_ARGS__); } while (0)

u8 OscFlag = 0;
u8 OscCount = 0;

static OSC_STATEe OscState = OSC_STATE_IDLE;


 void Osc_Init(void)
 {
    OscState = OSC_STATE_IDLE;
    OscCount = 0;

    bsp_timer_set(TimerOscCheck, Time500mSec);

 }

void Osc_Task(void)
{
    if(bsp_timer_TimeOverCheck(TimerOscCheck)){
        if(iMySts.OscOnOff == ON){
            Osc_Check();
        }
        bsp_timer_set(TimerOscCheck, Time500mSec);
    }	

}

void Osc_Check(void)
{
    static u8 PreRxAlc = OFF;

    if(Iso_GetPwrInitFlag() == ON) return;
    if(Iso_GetIsoReCheckFlag() == ON) return;

    //Amp 설정이 On 인 경우에만 실행
    if(iMyCtrl.RxPath == ON && iMySts.Alarm.Bit.OscAlarm == OFF){
        switch(OscState)
        {
            case OSC_STATE_IDLE:
                OscCount = 0;
                OscState = OSC_STATE_CHECK;
                PreRxAlc = RX_ALC_BIT;
                //Alarm 설정후 Iso진행시 Alarm 언제 꺼저야 되는지 확인 필요
                break;
            case OSC_STATE_CHECK:
                if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
                    RX_ALC_BIT = OFF;
                    Init_RxAmpOff();
                    OscPrint("\r\n %d][OSC] STATE_CHECK -> STATE_FEEDBACK_CHECK", HAL_GetTick());
                    OscState = OSC_STATE_FEEDBACK_CHECK;
                }
                else{
                    Osc_ClearStatusBit();
                    iMySts.Stability = 0;
                    RX_ALC_BIT = PreRxAlc;
                }
                break;

            case OSC_STATE_FEEDBACK_CHECK:
                if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
                    OscState = OSC_STATE_RECHECK;
                    OscPrint("\r\n %d][OSC] STATE_FEEDBACK_CHECK -> STATE_RECHECK", HAL_GetTick());
                }
                else{
                    OscState = OSC_STATE_RECOVERY;
                    OscPrint("\r\n %d][OSC] STATE_FEEDBACK_CHECK -> STATE_RECOVERY", HAL_GetTick());
                }
                break;

            case OSC_STATE_RECOVERY:
                RX_ALC_BIT = PreRxAlc;
                Init_RxAmpOn();
                OscState = OSC_STATE_IDLE;
                OscPrint("\r\n %d][OSC] STATE_RECOVERY -> STATE_IDLE", HAL_GetTick());
                break;

            case OSC_STATE_RECHECK:
                if(OscCount >= OSC_MAX_COUNT){
                    OscState = OSC_STATE_IDLE;
                    Osc_SetStatusBit();
                    iMySts.Stability = 1;
                    RX_ALC_BIT = PreRxAlc;
                    Alarm_Set(ALARM_BIT_OSC);
                    Iso_SetIsoReCheckFlag(ON);
                    OscPrint("\r\n %d][OSC] STATE_RECHECK -> OSC_STATE_IDLE", HAL_GetTick());
                }
                else{
                    OscCount++;
                    OscState = OSC_STATE_CHECK;
                    Init_RxAmpOn();
                    OscPrint("\r\n %d][OSC] STATE_RECHECK -> STATE_CHECK", HAL_GetTick());
                }
                break;

            default:
                OscState = OSC_STATE_IDLE;
                break;
        }

    }


}

void Osc_SetStatusBit(void)
{
    iMySts.Alarm.Bit.OscAlarm = ON;
}
void Osc_ClearStatusBit(void)
{
    iMySts.Alarm.Bit.OscAlarm = OFF;
}
u8 Osc_GetStatusBit(void)
{
    return iMySts.Alarm.Bit.OscAlarm;
}