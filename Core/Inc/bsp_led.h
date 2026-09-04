#ifndef BSP_LED_H
#define BSP_LED_H

#include "common.h"

#define LED_PWR_OFF    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0)
#define LED_PWR_HALF   __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1500)
#define LED_PWR_ON     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 2999)

#define LED_TX_SHUTDOWN_OFF    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0)
#define LED_TX_SHUTDOWN_HALF   __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 30000)
#define LED_TX_SHUTDOWN_ON     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 60000)

#define LED_RX_SHUTDOWN_OFF    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0)
#define LED_RX_SHUTDOWN_HALF   __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 30000)
#define LED_RX_SHUTDOWN_ON     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 60000)


void LED_Task(void);
void Led_StatusToggle(void);
void Led_ShutDownToggle(void);
void Led_ShutDownOn(void);
void Led_ShutDownOff(void);
void Led_AllOn(void);
void Led_AllOff(void);
void Led_Init(void);
#endif