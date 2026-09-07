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

#define OscPrint(...) do { if (OscFlag) DebugPrint(__VA_ARGS__); } while (0)

u8 OscFlag = 0;
u8 OscCount = 0;

static OSC_STATEe OscState = OSC_STATE_IDLE;


 void Osc_Init(void)
 {
    OscState = OSC_STATE_IDLE;
    OscCount = 0;

    bsp_timer_set(TimerOscCheck, Time1Sec);

 }

void Osc_Task(void)
{
    if(bsp_timer_TimeOverCheck(TimerOscCheck)){
        Osc_Check();
        bsp_timer_set(TimerOscCheck, Time1Sec);
    }	

}

void Osc_Check(void)
{
    static u8 PreRxAlc = OFF;


    if(iMySts.OscOnOff == ON && iMySts.Flag2.Bit.RxPath == ON && Iso_GetIsoReCheckFlag() == OFF){
        switch(OscState)
        {
            case OSC_STATE_IDLE:
                OscCount = 0;
                PreRxAlc = iMySts.Flag1.Bit.RxAlc;
                OscState = OSC_STATE_CHECK;
                break;
            case OSC_STATE_CHECK:
                if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
                    iMySts.Flag1.Bit.RxAlc = OFF;
                    Init_RxAmpOff();
                    // or atten set to max
                    // iMyCtrl.RxAlcAtt = RX_ATT_MAX_NUM
                    // Atten_SetRxAtt(RX_ATT1);
                    OscState = OSC_STATE_FEEDBACK_CHECK;
                }
                else{

                    OscState = OSC_STATE_RECOVERY;
                }
                break;

            case OSC_STATE_FEEDBACK_CHECK:
                if(iMySts.RxOutputPower >= iMySts.IsoLimitLevel){
                    OscState = OSC_STATE_RECHECK;
                }
                else{
                    OscState = OSC_STATE_RECOVERY;
                }
                break;

            case OSC_STATE_RECOVERY:
                OscCount = 0;
                iMySts.Flag1.Bit.RxAlc = PreRxAlc;
                Init_RxAmpOn();
                OscState = OSC_STATE_IDLE;
                break;

            case OSC_STATE_RECHECK:
                if(OscCount >= OSC_MAX_COUNT){
                    OscState = OSC_STATE_IDLE;
                    Iso_SetIsoReCheckFlag(ON);
                }
                else{
                    OscCount++;
                    OscState = OSC_STATE_CHECK;
                    Init_RxAmpOn();
                }
                break;

            default:
                OscState = OSC_STATE_IDLE;
                break;
        }


    }


}