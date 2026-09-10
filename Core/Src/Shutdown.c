/*
 * Shutdown.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */
#include "Shutdown.h"
#include "project.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "Alarm.h"
#include "bsp_init.h"
#include "bsp_led.h"

#define ShutdownPrint(...) do { if (ShutdownFlag) DebugPrint(__VA_ARGS__); } while (0)

u8 ShutdownFlag = 0;

u8 TxShutdown_Status = 0;
u8 TxShutdownStep = 0;
u16 TxShutdownCheckCnt = 0;
u8 TxShutdownDetCnt = 0;

u8 RxShutdown_Status = 0;
u8 RxShutdownStep = 0;
u16 RxShutdownCheckCnt = 0;
u8 RxShutdownDetCnt = 0;


			          //30s, 5min,  60min
//u16 ShutdownHoldTime[SHUTDOWN_RECHECK_NUM]={60, 600, 7200};	
u16 ShutdownHoldTime[SHUTDOWN_STEP_NUM]={3,3,3};	
// 과출력 감지시간
u16 ShutdownCheckTime[SHUTDOWN_STEP_NUM]={6, 3, 3, 3};	

void Shutdown_Init(void)
{
    bsp_timer_set(TimerShutdownCheck, Time500mSec);
}


void Shutdown_Task(void)
{
    if(bsp_timer_TimeOverCheck(TimerShutdownCheck)){
        Shutdown_TxCheck();
        Shutdown_RxCheck();
        bsp_timer_set(TimerShutdownCheck, Time500mSec);
    }	
}

void Shutdown_TxCheck(void)
{
    if(iMyCtrl.TxPath == ON){
        if(iMySts.Flag1.Bit.TxShutdown == ON){

            if(TxShutdown_Status & SHUTDOWN_FOREVER_FLAG){
                return;
            }

            if(TxShutdown_Status & TX_SHUTDOWN_FLAG){
                if(TxShutdownStep > 0 && TxShutdownStep <= SHUTDOWN_RECHECK_NUM){
                    if(++TxShutdownCheckCnt >= ShutdownHoldTime[TxShutdownStep-1]){
                        TxShutdownCheckCnt = 0;
                        TxShutdown_Status&=~TX_SHUTDOWN_FLAG;
                        iMySts.Alarm.Bit.TxShutdownAlarm = OFF;
                        ShutdownPrint("\r\n %d][SHUTDOWN]Tx Shutdown Amp On, TxShutdownStep=%d, Holdtime=%d", HAL_GetTick(), TxShutdownStep, ShutdownHoldTime[TxShutdownStep-1]);
                        Init_TxAmpOn();
                        Alarm_SetFwdSD(FWD_SD_RECOVERY);                    
                    }

                }
            }
            else if(iMySts.TxOutputPower > iMySts.TxShutdownLimit){
                
                if(++TxShutdownDetCnt  >= ShutdownCheckTime[TxShutdownStep]){
                    LED_TX_SHUTDOWN_ON;
                    TxShutdownDetCnt  = 0;             
                    Init_TxAmpOff();

                    if(TxShutdownStep >= SHUTDOWN_STEP_NUM){
                        Alarm_SetFwdSD(FWD_SD_END);
                        TxShutdown_Status |= SHUTDOWN_FOREVER_FLAG;
                        TxShutdownStep = 0;
                        ShutdownPrint("\r\n %d][SHUTDOWN]Tx SHUTDOWN FOREVER", HAL_GetTick());
                    }
                    else{
                        if(TxShutdownStep == 0){
                            Alarm_SetFwdSD(FWD_SD_1ST);
                        }
                        else if(TxShutdownStep == 1){
                            Alarm_SetFwdSD(FWD_SD_2ND);
                        }
                        else if(TxShutdownStep == 2){
                            Alarm_SetFwdSD(FWD_SD_3RD);
                        }
                        else{
                            ShutdownPrint("\r\n %d][SHUTDOWN][ERR] Tx Shutdown Step Error, TxShutdownStep=%d, Checktime=%d", HAL_GetTick(), TxShutdownStep, ShutdownCheckTime[TxShutdownStep]);
                            TxShutdownStep = 0;
                        }
                        
                        ShutdownPrint("\r\n %d][SHUTDOWN]Tx Shutdown Amp Off, TxShutdownStep=%d, Checktime=%d", HAL_GetTick(), TxShutdownStep, ShutdownCheckTime[TxShutdownStep]);
                        TxShutdown_Status |= TX_SHUTDOWN_FLAG;
                        TxShutdownCheckCnt  = 0;
                        TxShutdownStep++;
                    }
                    iMySts.Alarm.Bit.TxShutdownAlarm = ON;

                }
            }
            else{
                Shutdown_TxClear();
            }
        }

    }

}



void Shutdown_RxCheck(void)
{
    if(iMyCtrl.RxPath == ON){
        if(iMySts.Flag1.Bit.RxShutdown == ON){

            if(RxShutdown_Status & SHUTDOWN_FOREVER_FLAG){
                return;
            }

            if(RxShutdown_Status & RX_SHUTDOWN_FLAG){
                if(RxShutdownStep > 0 && RxShutdownStep <= SHUTDOWN_RECHECK_NUM){
                    if(++RxShutdownCheckCnt >= ShutdownHoldTime[RxShutdownStep-1]){
                        RxShutdownCheckCnt = 0;
                        RxShutdown_Status&=~RX_SHUTDOWN_FLAG;
                        iMySts.Alarm.Bit.RxShutdownAlarm = OFF;
                        ShutdownPrint("\r\n %d][SHUTDOWN]Rx Shutdown Amp On, RxShutdownStep=%d, Holdtime=%d", HAL_GetTick(), RxShutdownStep, ShutdownHoldTime[RxShutdownStep-1]);
                        Init_RxAmpOn();
                        Alarm_SetRevSD(REV_SD_RECOVERY);
                    }

                }
            }
            else if(iMySts.RxOutputPower > iMySts.RxShutdownLimit){
                
                if(++RxShutdownDetCnt  >= ShutdownCheckTime[RxShutdownStep]){
                    LED_RX_SHUTDOWN_ON;
                    RxShutdownDetCnt  = 0;
                    Init_RxAmpOff();

                    if(RxShutdownStep >= SHUTDOWN_STEP_NUM){
                        Alarm_SetRevSD(REV_SD_END);
                        RxShutdown_Status |= SHUTDOWN_FOREVER_FLAG;
                        RxShutdownStep = 0;
                        ShutdownPrint("\r\n %d][SHUTDOWN]Rx SHUTDOWN FOREVER", HAL_GetTick());
                    }
                    else{
                        if(RxShutdownStep == 0){
                            Alarm_SetRevSD(REV_SD_1ST);
                        }
                        else if(RxShutdownStep == 1){
                            Alarm_SetRevSD(REV_SD_2ND);
                        }
                        else if(RxShutdownStep == 2){
                            Alarm_SetRevSD(REV_SD_3RD);
                        }
                        else{
                            ShutdownPrint("\r\n %d][SHUTDOWN][ERR] Rx Shutdown Step Error, RxShutdownStep=%d, Checktime=%d", HAL_GetTick(), RxShutdownStep, ShutdownCheckTime[RxShutdownStep]);
                            RxShutdownStep = 0;
                        }
                        
                        ShutdownPrint("\r\n %d][SHUTDOWN]Rx Shutdown Amp Off, RxShutdownStep=%d, Checktime=%d", HAL_GetTick(), RxShutdownStep, ShutdownCheckTime[RxShutdownStep]);
                        RxShutdown_Status |= RX_SHUTDOWN_FLAG;
                        RxShutdownCheckCnt  = 0;
                        RxShutdownStep++;
                    }
                    iMySts.Alarm.Bit.RxShutdownAlarm = ON;

                }
            }
            else{
                Shutdown_RxClear();           
            }
        }



    }


}

void Shutdown_TxClear(void)
{
    TxShutdown_Status = 0;
    TxShutdownStep = 0;
    TxShutdownCheckCnt = 0;
    TxShutdownDetCnt = 0;
    iMySts.Alarm.Bit.TxShutdownAlarm = OFF;
}

void Shutdown_RxClear(void)
{
    RxShutdown_Status = 0;
    RxShutdownStep = 0;
    RxShutdownCheckCnt = 0;
    RxShutdownDetCnt = 0;
    iMySts.Alarm.Bit.RxShutdownAlarm = OFF;
}
