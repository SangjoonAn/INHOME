/*
 * Iso.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "Iso.h"
#include "project.h"
#include "bsp_timer.h"

void Iso_Init(void)
{
  bsp_timer_set(TimerISOCheck, Time2Sec);

}

void Iso_Task(void)
{
  if(bsp_timer_TimeOverCheck(TimerISOCheck)){
      Iso_Check();
      bsp_timer_set(TimerISOCheck, Time2Sec);
  }	
}

void Iso_Check(void)
{
  if(iMyCtrl.IsoCheck == ON){
    if(iMyCtrl.IsoThreshold > iMySts.RxOutputPower){

      
    }



  }
}