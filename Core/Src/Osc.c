/*
 * Osc.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "Osc.h"
#include "project.h"
#include "bsp_timer.h"


 void Osc_Init(void)
 {
    bsp_timer_set(TimerOscCheck, Time100mSec);

 }

void Osc_Task(void)
{
    if(bsp_timer_TimeOverCheck(TimerOscCheck)){
        //Osc_Check();
        bsp_timer_set(TimerOscCheck, Time100mSec);
    }	

}

void Osc_Check(void)
{
    
}