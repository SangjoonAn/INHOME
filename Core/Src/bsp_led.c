#include "main.h"
#include "bsp_led.h"
#include "bsp_timer.h"

u8 Led_InitFlag, Led_InitMode, Led_InitCnt;

void LED_Task(void)
{

	if(bsp_timer_TimeOverCheck(TimerLed))
	{
		bsp_timer_set(TimerLed, Time100mSec*5L);
		Led_StatusToggle();

		if(Led_InitFlag == 0){
			if(++Led_InitCnt > 6){
				Led_InitFlag = 1;
				Led_InitCnt = 0;
				LED_PWR_OFF;
			}
			else{
				if(Led_InitMode == 0){
					Led_AllOn();
					Led_InitMode = 1;
				}
				else if(Led_InitMode == 1){
					Led_AllOff();
					Led_InitMode = 0;
				}

			}
		}

	}



}


void Led_Init(void)
{
	LED_PWR_OFF;
	LED_TX_SHUTDOWN_OFF;
	LED_RX_SHUTDOWN_OFF;

	Led_InitFlag = 0;
	Led_InitMode = 0;
	Led_InitCnt = 0;
}

void Led_StatusToggle(void)
{
    HAL_GPIO_TogglePin(LED_STATE_GPIO_Port, LED_STATE_Pin);
}


void Led_AllOff(void)
{
	LED_PWR_OFF;
	LED_TX_SHUTDOWN_OFF;
	LED_RX_SHUTDOWN_OFF;
}

void Led_AllOn(void)
{
	LED_PWR_ON;
	LED_TX_SHUTDOWN_ON;
	LED_RX_SHUTDOWN_ON;
}
