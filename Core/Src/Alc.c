/*
 * Alc.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "Alc.h"
#include "project.h"
#include "Atten.h"
#include "bsp_uart.h"
#include "bsp_timer.h"

#define AlcPrint(...) do { if (AlcFlag) DebugPrint(__VA_ARGS__); } while (0)

u8 AlcFlag = 0;


void Alc_Init(void)
{
    bsp_timer_set(TimerTxAlcCheck, Time2Sec);
    bsp_timer_set(TimerRxAlcCheck, Time500mSec);
}


void Alc_Task(void)
{
    if(bsp_timer_TimeOverCheck(TimerTxAlcCheck)){
        Alc_SetTx();
        bsp_timer_set(TimerTxAlcCheck, Time2Sec);
    }	
    if(bsp_timer_TimeOverCheck(TimerRxAlcCheck)){
        Alc_SetRx();
        bsp_timer_set(TimerRxAlcCheck, Time500mSec);
    }	
}

void Alc_SetTx(void)
{
    s16 PowerDiff =0;
    static u8 PreAlcAtt = 0;

    if(iMyCtrl.TxAlc == ON){
        if(iMySts.TxOutputPower > iMyCtrl.TxAlcHighLevel){
            PowerDiff  = iMySts.TxOutputPower - iMyCtrl.TxAlcHighLevel;
            /* 0.1 dB 단위이므로 10 = 1.0 dB */
            if(PowerDiff  > 10){
                iMyCtrl.TxAlcAtt++;
                if(iMyCtrl.TxAlcAtt > TX_ALC_ATT_MAX_NUM) iMyCtrl.TxAlcAtt = TX_ALC_ATT_MAX_NUM;

            }

            if(PreAlcAtt != iMyCtrl.TxAlcAtt){
                Atten_SetTxAtt();
                AlcPrint("\r\n %d][ALC] Atten_SetTxAtt = %d", HAL_GetTick(), iMyCtrl.TxAlcAtt);
                PreAlcAtt = iMyCtrl.TxAlcAtt;
            }
        }
        else if(iMyCtrl.TxAlcLowOffset > iMySts.TxOutputPower){
            PowerDiff  = iMySts.TxOutputPower - iMyCtrl.TxAlcLowOffset;

            if(PowerDiff  < -10){
                if(iMyCtrl.TxAlcAtt > TX_ALC_ATT_MIN_NUM){
                    iMyCtrl.TxAlcAtt--;
                }
                
            }

            if(PreAlcAtt != iMyCtrl.TxAlcAtt){
                Atten_SetTxAtt();
                AlcPrint("\r\n %d][ALC] Atten_SetTxAtt = %d", HAL_GetTick(), iMyCtrl.TxAlcAtt);
                PreAlcAtt = iMyCtrl.TxAlcAtt;
            }
        }



    }


}


void Alc_SetRx(void)
{
    s16 PowerDiff =0;
    static u8 PreAlcAtt = 0;

    if(iMyCtrl.RxAlc == ON){
        if(iMySts.RxOutputPower > iMyCtrl.RxAlcHighLevel){
            PowerDiff  = iMySts.RxOutputPower - iMyCtrl.RxAlcHighLevel;

            if(PowerDiff  > 10){
                iMyCtrl.RxAlcAtt++;
                if(iMyCtrl.RxAlcAtt > 40) iMyCtrl.RxAlcAtt = 40;

            }

            if(PreAlcAtt != iMyCtrl.RxAlcAtt){
                Atten_SetRxAtt(RX_ATT1);
                AlcPrint("\r\n %d][ALC] Atten_SetRxAtt = %d", HAL_GetTick(), iMyCtrl.RxAlcAtt);
                PreAlcAtt = iMyCtrl.RxAlcAtt;
            }
        }
        else if(iMyCtrl.TxAlcLowOffset > iMySts.RxOutputPower){
            PowerDiff  = iMySts.RxOutputPower - iMyCtrl.TxAlcLowOffset;

            if(PowerDiff  < -10){
                if(iMyCtrl.RxAlcAtt > 0){
                    iMyCtrl.RxAlcAtt--;
                }
                
            }

            if(PreAlcAtt != iMyCtrl.RxAlcAtt){
                Atten_SetRxAtt(RX_ATT1);
                AlcPrint("\r\n %d][ALC] Atten_SetRxAtt = %d", HAL_GetTick(), iMyCtrl.RxAlcAtt);
                PreAlcAtt = iMyCtrl.RxAlcAtt;
            }
        }



    }


}