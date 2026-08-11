#include "main.h"
#include "bsp_led.h"
#include "bsp_timer.h"


void LED_Task(void)
{

	if(bsp_timer_TimeOverCheck(TimerLed))
	{
		bsp_timer_set(TimerLed, Time100mSec*5L);
		OpLedToggle();
	}
  

}



void OpLedToggle(void)
{
    HAL_GPIO_TogglePin(LED_STATE_GPIO_Port, LED_STATE_Pin);
}
