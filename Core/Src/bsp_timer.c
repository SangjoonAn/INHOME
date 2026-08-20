#include "main.h"
#include "bsp_timer.h"

static void SystemRunTime_Update(void);

u32 gCurTimerTick;
TIMER gLongTimer[TIMERmax];

void bsp_timer_init(void)
{
	u8 i = 0;

    gCurTimerTick = 0;

	for(i = 0; i < TIMERmax; i++)
	{
		gLongTimer[i].timerSetFlag = 0;		//0:Reset
		gLongTimer[i].Count = 0;
	}


    bsp_timer_set(TimerLed, Time1mSec);
    bsp_timer_set(TimerSystemRunTime, Time1Min);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);


}


void Timer_Task(void)
{
    static u32 old_Tick = 0;
    
    if(gCurTimerTick != old_Tick){
        old_Tick = gCurTimerTick;

        bsp_timer_check();
    }

    if(bsp_timer_TimeOverCheck(TimerSystemRunTime)){
        SystemRunTime_Update();
        bsp_timer_set(TimerSystemRunTime, Time1Min);
    }	

}

static void SystemRunTime_Update(void)
{
	u16 DayCount;

    iMySts.SystemRunTime[0]++;

    if(iMySts.SystemRunTime[0] >= 60)
    {
        iMySts.SystemRunTime[0] = 0;
        iMySts.SystemRunTime[1]++;

		/* 24시간 -> 1일 */
        if(iMySts.SystemRunTime[1] >= 24)
        {
            iMySts.SystemRunTime[1] = 0;
            iMySts.SystemRunTime[2]++;

			DayCount = ((u16)iMySts.SystemRunTime[2] << 8)	|  (u16)iMySts.SystemRunTime[3];
			DayCount++;

			/* 365일 -> 1년 */
			if(DayCount >= 365){
				DayCount = 0;
				iMySts.SystemRunTime[4]++;
			}
			iMySts.SystemRunTime[2] = (u8)(DayCount >> 8);
			iMySts.SystemRunTime[3] = (u8)(DayCount & 0xFF);

        }
    }
}

void System_GetTime(u8 *pTime)
{
    if (pTime == NULL)
    {
        return;
    }

	 memcpy(pTime, iMySts.SystemRunTime, 5U);
}


void bsp_timer_check(void)
{
	u8 i = 0;
    
    for(i = 0; i < TIMERmax; i++ )
    {
        if(gLongTimer[i].Count > 0L )
        {
            gLongTimer[i].Count--;
        }
    }

}

u8 bsp_timer_set(u8 TimeId, u32 TimeCnt)
{
	if (TimeId >= TIMERmax) return FALSE;

	gLongTimer[TimeId].Count = TimeCnt;		  
	gLongTimer[TimeId].timerSetFlag = 1; 	
	
	return(TRUE);
}

u32 bsp_timer_RemainCheck(u8 TimeId)
{
	u32 i = 0;

	if (TimeId >= TIMERmax) return FALSE;
	
	i = gLongTimer[TimeId].Count;
	
	return( i );
}



u8 bsp_timer_TimeOverCheck( u8 TimeId )
{
	u8 Ret = FALSE;

	if (TimeId >= TIMERmax) return FALSE;
	
	if ( gLongTimer[TimeId].timerSetFlag == TRUE )
	{
		if ( gLongTimer[TimeId].Count == 0 )
		{		   
			gLongTimer[TimeId].timerSetFlag = 0; 
			Ret = TRUE; //return( TRUE );
		}
		else
		{
			Ret = FALSE; //return(FALSE);
		}
	}

	return( Ret );
}

u8 bsp_timer_Cancel ( u8 TimeId )
{

	if (TimeId >= TIMERmax) return FALSE;
	
	gLongTimer[TimeId].timerSetFlag = 0;
	gLongTimer[TimeId].Count = 0;
	
	return( TRUE );
}

