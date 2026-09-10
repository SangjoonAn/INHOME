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


    if(iMySts.Flag2.Bit.RxPath == ON && Iso_GetIsoReCheckFlag() == OFF){
        switch(OscState)
        {
            case OSC_STATE_IDLE:
                OscCount = 0;
                PreRxAlc = iMySts.Flag1.Bit.RxAlc;
                OscState = OSC_STATE_CHECK;
                iMySts.Alarm.Bit.OscAlarm = ON;
                iMySts.Stability = 0;
                //Alarm 설정후 Iso진행시 Alarm 언제 꺼저야 되는지 확인 필요
                break;
            case OSC_STATE_CHECK:
                if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
                    iMySts.Flag1.Bit.RxAlc = OFF;
                    Init_RxAmpOff();
                    OscPrint("\r\n %d][OSC] STATE_CHECK -> STATE_FEEDBACK_CHECK", HAL_GetTick());
                    OscState = OSC_STATE_FEEDBACK_CHECK;
                }
                else{
                    OscPrint("\r\n %d][OSC] STATE_CHECK -> STATE_RECOVERY", HAL_GetTick());
                    OscState = OSC_STATE_RECOVERY;
                }
                break;

            case OSC_STATE_FEEDBACK_CHECK:
                if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
                    OscState = OSC_STATE_RECHECK;
                    OscPrint("\r\n %d][OSC] TATE_FEEDBACK_CHECK -> TATE_RECHECK", HAL_GetTick());
                }
                else{
                    OscState = OSC_STATE_RECOVERY;
                    OscPrint("\r\n %d][OSC] STATE_FEEDBACK_CHECK -> STATE_RECOVERY", HAL_GetTick());
                }
                break;

            case OSC_STATE_RECOVERY:
                OscCount = 0;
                iMySts.Flag1.Bit.RxAlc = PreRxAlc;
                Init_RxAmpOn();
                OscState = OSC_STATE_IDLE;
                OscPrint("\r\n %d][OSC] STATE_RECOVERY -> STATE_IDLE", HAL_GetTick());

                break;

            case OSC_STATE_RECHECK:
                if(OscCount >= OSC_MAX_COUNT){
                    OscState = OSC_STATE_IDLE;
                    iMySts.Alarm.Bit.OscAlarm = ON;
                    iMySts.Stability = 1;
                    Alarm_Set(ALARM_CODE_OSC);
                    Iso_SetIsoReCheckFlag(ON);
                    OscPrint("\r\n %d][OSC] STATE_RECHECK -> STATE_IDLE", HAL_GetTick());
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