#include "bsp_adc.h"
#include "bsp_timer.h"
#include "table.h"

u16 gAdc_Value[MAX_ADC_NUM];
u16 gAdc_Buffer[MAX_ADC_NUM][MAX_ADC_BUFFER_CNT];
u16 gAdc_BufferIndex = 0;

volatile u8 gAdc_Update;


void BspAdc_Init(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)gAdc_Value, MAX_ADC_NUM);

    HAL_TIM_Base_Start_IT(&htim3);

    bsp_timer_set(TimerAdcTemperature, Time1Sec);
    bsp_timer_set(TimerAdcPowerDetect, Time100mSec);

}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    gAdc_Update = 1;
}

void BspAdc_GetValue(void)
{
    for(u8 i = 0; i < MAX_ADC_NUM; i++)
    {
        gAdc_Buffer[i][gAdc_BufferIndex] = gAdc_Value[i];
    }

    if(++gAdc_BufferIndex >= MAX_ADC_BUFFER_CNT)
    {
        gAdc_BufferIndex = 0;
    }
}

void Adc_Task(void)
{
    if(gAdc_Update)
    {
        gAdc_Update = 0;
        BspAdc_GetValue();
    }

    if(bsp_timer_TimeOverCheck(TimerAdcTemperature)){
        SystemTemp_Update();
        bsp_timer_set(TimerAdcTemperature, Time1Sec);
    }

    if(bsp_timer_TimeOverCheck(TimerAdcPowerDetect)){
        PowerDet_Update();
        bsp_timer_set(TimerAdcPowerDetect, Time100mSec);
    }


    
}


void PowerDet_Update(void)
{
    u16 AdResult;

    //Test
    TABLE_INFO_t *pInfo;
    pInfo = &gTableInfo.Table[ADC_NUM_TX_DET];

    AdResult = Adc_CalcCleanAverage(gAdc_Buffer[ADC_NUM_TX_DET]);
    iMySts.TxDetVoltage = AdResult;
    Table_SetAdResult(ADC_NUM_TX_DET, AdResult);
    iMySts.TxOutputPower = Table_GetMeanData(ADC_NUM_TX_DET) + iMyCtrl.TxPowerOffsetOutput;

    AdResult = Adc_CalcCleanAverage(gAdc_Buffer[ADC_NUM_RX_DET]);
    iMySts.RxDetVoltage = AdResult;    
    Table_SetAdResult(ADC_NUM_RX_DET, AdResult);   
    iMySts.RxOutputPower = Table_GetMeanData(ADC_NUM_RX_DET) + iMyCtrl.RxPowerOffsetOutput;

    //TxInputPower 계산할것.


}


void SystemTemp_Update(void)
{
	u32 TempSum=0;
	float TempVoltage=0;
	float Temperature=0;

    for(u16 i = 0; i < MAX_ADC_BUFFER_CNT; i++)
    {
        TempSum += gAdc_Buffer[ADC_NUM_TEMPER_DET][i];
    }

    TempVoltage = ((float)TempSum / (float)MAX_ADC_BUFFER_CNT) * ADC_RBW;
    iMySts.SysTemperVoltage=TempVoltage;


    if(TempVoltage >= TEMPER_0DO)
    {
        Temperature = (TempVoltage - TEMPER_0DO) / 6.25f;
    }
    else
    {
        Temperature = -(TEMPER_0DO - TempVoltage) / 6.25f;
    }
    iMySts.SysTemper=Temperature;


}
    

u16 Adc_CalcCleanAverage(u16 *pBuffer)
{
    u32 Sum = 0U;
    u16 Temp;
    u16 i;
    u16 j;
    u16 ValidCount;
    u16 Buffer[MAX_ADC_BUFFER_CNT];

    ValidCount = MAX_ADC_BUFFER_CNT - (MAX_MIN_CLEAR_CNT * 2);

    if ((pBuffer == NULL) || (ValidCount == 0U))
    {
        return 0U;
    }

    memcpy(Buffer, pBuffer,sizeof(Buffer));

    /*-------------------------------------------------------
     * 내림차순 정렬
     *------------------------------------------------------*/
    for (i = 0U; i < MAX_ADC_BUFFER_CNT - 1U; i++)
    {
        for (j = i + 1U; j < MAX_ADC_BUFFER_CNT; j++)
        {
            if (Buffer[i] < Buffer[j])
            {
                Temp    = Buffer[i];
                Buffer[i] = Buffer[j];
                Buffer[j] = Temp;
            }
        }
    }

    /*-------------------------------------------------------
     * 최대/최소값 제거 후 평균
     *------------------------------------------------------*/
    for (i = MAX_MIN_CLEAR_CNT; i < (MAX_ADC_BUFFER_CNT - MAX_MIN_CLEAR_CNT); i++)
    {
        Sum += (u32)Buffer[i];
    }

    return (u16)(Sum / ValidCount);
}