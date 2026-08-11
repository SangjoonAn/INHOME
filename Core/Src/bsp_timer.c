#include "bsp_timer.h"


u32 CurTimerTick;
TIMER LongTimer[TIMERmax];

void bsp_timer_init(void)
{
	u8 i = 0;

    CurTimerTick = 0;

	for(i = 0; i < TIMERmax; i++)
	{
		LongTimer[i].timerSetFlag = 0;		//0:Reset
		LongTimer[i].Count = 0;
	}


    bsp_timer_set(TimerLed, Time1mSec);
    bsp_timer_set(TimerSystemRunTime, Time1Min);

}


void Timer_Task(void)
{
    static u32 old_Tick = 0;
    
    if(CurTimerTick != old_Tick){
        old_Tick = CurTimerTick;

        bsp_timer_check();
    }
}

void bsp_timer_check(void)
{
	u8 i = 0;
    
    for(i = 0; i < TIMERmax; i++ )
    {
        if(LongTimer[i].Count > 0L )
        {
            LongTimer[i].Count--;
        }
    }

}

u8 bsp_timer_set(u8 TimeId, u32 TimeCnt)
{
	if (TimeId >= TIMERmax) return FALSE;

	LongTimer[TimeId].Count = TimeCnt;		  
	LongTimer[TimeId].timerSetFlag = 1; 	
	
	return(TRUE);
}

u32 bsp_timer_RemainCheck(u8 TimeId)
{
	u32 i = 0;

	if (TimeId >= TIMERmax) return FALSE;
	
	i = LongTimer[TimeId].Count;
	
	return( i );
}



u8 bsp_timer_TimeOverCheck( u8 TimeId )
{
	u8 Ret = FALSE;

	if (TimeId >= TIMERmax) return FALSE;
	
	if ( LongTimer[TimeId].timerSetFlag == TRUE )
	{
		if ( LongTimer[TimeId].Count == 0 )
		{		   
			LongTimer[TimeId].timerSetFlag = 0; 
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
	
	LongTimer[TimeId].timerSetFlag = 0;
	LongTimer[TimeId].Count = 0;
	
	return( TRUE );
}

