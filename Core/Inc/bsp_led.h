#ifndef BSP_LED_H
#define BSP_LED_H

#include "common.h"

#define LED_PWR_OFF    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
#define LED_PWR_HALF   __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 30000);
#define LED_PWR_ON     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 60000);


void LED_Task(void);
void OpLedToggle(void);


#endif