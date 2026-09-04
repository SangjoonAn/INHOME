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
u8 Shutdown_Recovery_Num = 3;

			          //30s, 5min,  60min
u16 ShutdownHoldTime[SHUTDOWN_RECHECK_NUM]={60, 600, 7200, 0};	
//u16 ShutdownHoldTime[SHUTDOWN_RECHECK_NUM]={7, 7, 7};	
// 과출력 감지시간
u16 ShutdownCheckTime[SHUTDOWN_RECHECK_NUM]={6, 3, 3, 3};	

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
    static u8 step = 0;
    static u16 RecheckCnt = 0;
    static u8  ShutdownDetectCnt = 0;

    if(iMyCtrl.TxShutdown == ON){

        if(TxShutdown_Status & SHUTDOWN_FOREVER_FLAG){
            return;
        }

        if(TxShutdown_Status & TX_SHUTDOWN_FLAG){
            if(step > 0 && step <= 3){
                if(++RecheckCnt >= ShutdownHoldTime[step-1]){
                    RecheckCnt = 0;
                    TxShutdown_Status&=~TX_SHUTDOWN_FLAG;
                    ShutdownPrint("\r\n %d][SHUTDOWN] Shutdown Amp On, step=%d, Holdtime=%d", HAL_GetTick(), step, ShutdownHoldTime[step-1]);
                    Init_TxAmpOn();
                }

            }
        }
        else if(iMySts.TxOutputPower > iMyCtrl.TxShutdownLimit){
            
            if(++ShutdownDetectCnt  >= ShutdownCheckTime[step]){
                LED_TX_SHUTDOWN_ON;
                ShutdownDetectCnt  = 0;
                Alarm_Set(ALARM_BIT_FWD_SD);
                Init_TxAmpOff();

                if(step >= Shutdown_Recovery_Num){
                    TxShutdown_Status |= SHUTDOWN_FOREVER_FLAG;
                    step = 0;
                    ShutdownPrint("\r\n %d][SHUTDOWN] SHUTDOWN FOREVER", HAL_GetTick());
                }
                else{
                    ShutdownPrint("\r\n %d][SHUTDOWN] Shutdown Amp Off, step=%d, Checktime=%d", HAL_GetTick(), step, ShutdownCheckTime[step]);
                    TxShutdown_Status |= TX_SHUTDOWN_FLAG;
                    RecheckCnt  = 0;
                    step++;
                }

            }
        }
        else{
            ShutdownDetectCnt = 0;
        }
    }

}

void Shutdown_RxCheck(void)
{

}