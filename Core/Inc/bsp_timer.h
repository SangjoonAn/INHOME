#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include "common.h"
#include "project.h"

#define Time1mSec		1
#define Time10mSec		10
#define Time100mSec		100
#define Time1Sec		1000L

#define Time1Min		1000L*60L
#define Time1Hour		(Time1Min*60L)
#define Time1Day		(Time1Hour*24L)

#define ALCTime			(Time1Day*7L)

#define TIMERmax	60

typedef struct {
	u8	    timerSetFlag;	// Timer Registor Flag  0:Reset, 1:Set
	//u8	TimerID;		// Timer Identification //
	u32	    Count;			// Timer Count //
} TIMER;


// Timer Register & Specification
enum
{
	TimerReserved	= 0,
	TimerLed,
	TimerSystemRunTime,
	TimerAdcTemperature,
	TimerAdcPowerDetect,
	Timer_Test,
	TimerDn,
	TimerFrontLED,		
	
	TimerSleepOn,
	TimerLEDBlink,	//10
	/////////////////////////////////////
	TimerALC_900M,
	TimerALC_1_8G,
	TimerALC_2_1G,
	

	TimerISO_900M,
	TimerISO_1_8G,
	TimerISO_2_1G,


	TimerRxSleep_900M,			//TimertSleepFunc_900M,
	TimerRxSleep_1_8G,			//TimertSleepFunc_1_8G,			
	TimerRxSleep_2_1G,			//TimertSleepFunc_2_1G,

	TimerTxSleep_900M,			
	TimerTxSleep_1_8G,			
	TimerTxSleep_2_1G,			

	TimerALCPreiod_900M,		
	TimerALCPreiod_1_8G,
	TimerALCPreiod_2_1G,

	TimerRvsALCPreiod_900M,
	TimerRvsALCPreiod_1_8G,
	TimerRvsALCPreiod_2_1G,

	TimerTblCal_900M,
	TimerTblCal_1_8G,
	TimerTblCal_2_1G,

	TimerReset_900M,
	TimerReset_1_8G,				
	TimerReset_2_1G,

	TimertSleepFunc_900M,
	TimertSleepFunc_1_8G,				
	TimertSleepFunc_2_1G,

	TimerTxSD_900M,			
	TimerTxSD_1_8G,			
	TimerTxSD_2_1G,			

	TimerRxSD_900M,			//TimertSDFunc_900M,
	TimerRxSD_1_8G,			//TimertSDFunc_1_8G,			
	TimerRxSD_2_1G,			//TimertSDFunc_2_1G,
	
	
	/////////////////////////////////////
	
	TimerReset,

	//ReMS
	TimerRemsRx,			
	TimerRepRst,
	TimerNextStsRsps,				//50
	TimerSmsRstTx,
	TimerSmsTimeTx,
	TimerNetStsWait,
	//TimerRepRst,
	TimertSioRx,			//50
	//TimertSleepFunc,
	
	TimerSduRx,
	TimerRcuRx,
	
	TimerExtRx,					
	TimerExtTxWait,

	TimerLteStsRsps,
	
	TimerRvsALCPreiod,			//
	TimerOsc1SecTimeTick,
	
	
///////////////////////////////////////////////////
// USER DEFINE TIMER

// END Of User TIMER
//////////////////////////////////////////////////
	
};



void bsp_timer_init(void);
void bsp_timer_check(void);
void Timer_Task(void);
u8 bsp_timer_set(u8 TimeId, u32 TimeCnt);
u8 bsp_timer_TimeOverCheck( u8 TimeId );
u32 bsp_timer_RemainCheck(u8 TimeId);

#endif /* BSP_TIMER_H */