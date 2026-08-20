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

#define MAX_MIN_CLEAR_CNT                       3
#define MAX_ADC_BUFFER_CNT                      20


// VOUT​=424mV+6.25mV×T
#define TEMPER_0DO		424  
//  ADC_RBW = (3.3V / 4095(ADC 12bit))
#define ADC_RBW	        0.805664

void BspAdc_Init(void);
void BspAdc_GetValue(void);
void Adc_Task(void);
void SystemTemp_Update(void);
u16 Adc_CalcCleanAverage(u16 *pBuffer);
void PowerDet_Update(void);


#endif /* BSP_ADC_H */