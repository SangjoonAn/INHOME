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

    if(iMySts.Flag1.Bit.TxAlc == ON){
        if(iMySts.TxOutputPower > iMySts.TxAlcHighLevel){
            PowerDiff  = iMySts.TxOutputPower - iMySts.TxAlcHighLevel;
            /* 0.1 dB 단위이므로 10 = 1.0 dB */
            if(PowerDiff  > 10){
                iMySts.TxAlcAtt++;
                if(iMySts.TxAlcAtt > TX_ALC_ATT_MAX_NUM) iMySts.TxAlcAtt = TX_ALC_ATT_MAX_NUM;

            }

            if(PreAlcAtt != iMySts.TxAlcAtt){
                Atten_SetTxAtt();
                AlcPrint("\r\n %d][ALC] Atten_SetTxAtt = %d", HAL_GetTick(), iMySts.TxAlcAtt);
                PreAlcAtt = iMySts.TxAlcAtt;
            }
        }
        else if(iMySts.TxAlcLowOffset > iMySts.TxOutputPower){
            PowerDiff  = iMySts.TxOutputPower - iMySts.TxAlcLowOffset;

            if(PowerDiff  < -10){
                if(iMySts.TxAlcAtt > TX_ALC_ATT_MIN_NUM){
                    iMySts.TxAlcAtt--;
                }
                
            }

            if(PreAlcAtt != iMySts.TxAlcAtt){
                Atten_SetTxAtt();
                AlcPrint("\r\n %d][ALC] Atten_SetTxAtt = %d", HAL_GetTick(), iMySts.TxAlcAtt);
                PreAlcAtt = iMySts.TxAlcAtt;
            }
        }



    }


}


void Alc_SetRx(void)
{
    s16 PowerDiff =0;
    static u8 PreAlcAtt = 0;

    if(iMySts.Flag1.Bit.RxAlc == ON){
        if(iMySts.RxOutputPower > iMySts.RxAlcHighLevel){
            PowerDiff  = iMySts.RxOutputPower - iMySts.RxAlcHighLevel;

            if(PowerDiff  > 10){
                iMySts.RxAlcAtt++;
                if(iMySts.RxAlcAtt > 40) iMySts.RxAlcAtt = 40;

            }

            if(PreAlcAtt != iMySts.RxAlcAtt){
                Atten_SetRxAtt(RX_ATT1);
                AlcPrint("\r\n %d][ALC] Atten_SetRxAtt = %d", HAL_GetTick(), iMySts.RxAlcAtt);
                PreAlcAtt = iMySts.RxAlcAtt;
            }
        }
        else if(iMySts.TxAlcLowOffset > iMySts.RxOutputPower){
            PowerDiff  = iMySts.RxOutputPower - iMySts.TxAlcLowOffset;

            if(PowerDiff  < -10){
                if(iMySts.RxAlcAtt > 0){
                    iMySts.RxAlcAtt--;
                }
                
            }

            if(PreAlcAtt != iMySts.RxAlcAtt){
                Atten_SetRxAtt(RX_ATT1);
                AlcPrint("\r\n %d][ALC] Atten_SetRxAtt = %d", HAL_GetTick(), iMySts.RxAlcAtt);
                PreAlcAtt = iMySts.RxAlcAtt;
            }
        }



    }


}