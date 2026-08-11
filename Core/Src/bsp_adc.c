#include "bsp_adc.h"
#include "bsp_timer.h"

u16 Ad_value[MAX_ADC_NUM];
u16 Ad_buffer[MAX_ADC_NUM][MAX_ADC_BUFFER_CNT];
u16 Ad_buffer_index = 0;

volatile u8 adc_update;

void bsp_adc_init(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)Ad_value, MAX_ADC_NUM);

    HAL_TIM_Base_Start_IT(&htim3);

    bsp_timer_set(TimerAdcTemperature, Time1Sec);

}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    adc_update = 1;
}

void Adc_get_value(void)
{
    for(int i = 0; i < MAX_ADC_NUM; i++)
    {
        Ad_buffer[i][Ad_buffer_index] = Ad_value[i];
    }

    if(++Ad_buffer_index >= MAX_ADC_BUFFER_CNT)
    {
        Ad_buffer_index = 0;
    }
}

void Adc_Task(void)
{
    if(adc_update)
    {
        adc_update = 0;
        Adc_get_value();

    }

    if(bsp_timer_TimeOverCheck(TimerAdcTemperature)){
        System_TempCheck();
    }
}


void PowerDetFunc(void)
{
    
}


void System_TempCheck(void)
{
	static float TempSum=0;
	float Temper=0;
	static u16 chat=0;

	if(++chat>3){		
		chat=0;
		TempSum/=3;
		
		if(TempSum==TEMPER_0DO){
			Temper=0;
		}
		else if(TempSum>TEMPER_0DO){						
			Temper=(float)((TempSum-TEMPER_0DO)/6.25);
		}
		else{																
			Temper=(float)((TEMPER_0DO-TempSum)/6.25)*(-1);
		}
        iMySts.SysTemper=Temper;
		TempSum=0;
	}
	else{
		TempSum+=Ad_value[ADC_NUM_TEMPER_DET]*ADC_RBW;		
	}
}
    