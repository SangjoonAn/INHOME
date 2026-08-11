#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "common.h"
#include "main.h"

#define ADC_NUM_TX_DET					        0	
#define ADC_NUM_RX_DET					        1
#define ADC_NUM_SLEEP_DET				        2
#define ADC_NUM_RESERVE					        3
#define ADC_NUM_TEMPER_DET					    4	
#define MAX_ADC_NUM                             5

#define MAX_ADC_BUFFER_CNT                       20


#define TEMPER_0DO		424  
#define ADC_RBW	        0.805664

void bsp_adc_init(void);
void Adc_get_value(void);
void Adc_Task(void);
void System_TempCheck(void);

#endif /* BSP_ADC_H */