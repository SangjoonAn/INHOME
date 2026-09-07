/*
 * Atten.c
 *
 *  Created on: 2026. 8. 26.
 *      Author: LG
 */

#include "main.h"
#include "common.h"
#include "project.h"
#include "Atten.h"
#include "bsp_init.h"
#include "bsp_uart.h"
#include "table.h"

#define AttPrint(...) do { if (AttFlag) DebugPrint(__VA_ARGS__); } while (0)


u8 AttFlag; 

typedef struct
{
    GPIO_TypeDef *Port;
    uint16_t Pin;
} ATT_EN_GPIO_t;

static const ATT_EN_GPIO_t AttEnGpio[] =
{
    {RX_ATT_EN1_GPIO_Port, RX_ATT_EN1_Pin},
    {RX_ATT_EN2_GPIO_Port, RX_ATT_EN2_Pin},
    {TX_ATT_EN1_GPIO_Port, TX_ATT_EN1_Pin},
};


void Atten_SetTxAtt(void)
{
    u8  bTemp = 0;
    s8  SubAtten;
    s8  Offset;

    if (iMySts.Test_ModeAtt == ON) return;

    /* Temperature Compensation */
    if (iMySts.TemperatureComp == ON)
    {
        SubAtten = iMySts.TxSubAtt + Table_GetTempAtt(iMySts.SysTemper, TABLE_IDX_TX_TEMP_ADJ);
    }
    else
    {
        SubAtten = iMySts.TxSubAtt;
    }

    /* Sub ATT 제한 */
    if (SubAtten < SUB_ATT_MIN_NUM) SubAtten = SUB_ATT_MIN_NUM;      
    if (SubAtten > SUB_ATT_MAX_NUM) SubAtten = SUB_ATT_MAX_NUM;
        
    if (iMySts.TxAlcAtt > TX_ATT_MAX_NUM) iMySts.TxAlcAtt = RX_ATT_MAX_NUM;
    SubAtten += iMySts.TxAlcAtt;

    /* ATT Offset */
    bTemp = Table_GetAttOffset( SubAtten,TABLE_IDX_TX_GAIN_ATT);

    /* ATT 범위 제한 */
    if (bTemp > ATT_MAX_NUM) bTemp = ATT_MAX_NUM;
    Atten_SendProc(bTemp, TX_ATT1);
}



void Atten_SetRxAtt(u8 TargetAtten)
{
    u8  bTemp = 0;
    s8  SubAtten;
    s8  Offset;

    if (iMySts.Test_ModeAtt == ON) return;

    /* Temperature Compensation */
    if (iMySts.TemperatureComp == ON)
    {
        SubAtten = iMySts.RxSubAtt + Table_GetTempAtt(iMySts.SysTemper, TABLE_IDX_RX_TEMP_ADJ);
    }
    else
    {
        SubAtten = iMySts.RxSubAtt;
    }

    /* Sub ATT 제한 */
    if (SubAtten < SUB_ATT_MIN_NUM) SubAtten = SUB_ATT_MIN_NUM;      
    if (SubAtten > SUB_ATT_MAX_NUM) SubAtten = SUB_ATT_MAX_NUM;
        
    /* Target별 추가 ATT */
    if (TargetAtten == RX_ATT1)
    {
        if (iMySts.RxAlcAtt > RX_ATT_MAX_NUM) iMySts.RxAlcAtt = RX_ATT_MAX_NUM;
        SubAtten += iMySts.RxAlcAtt;
    }
    else if (TargetAtten == RX_ATT2)
    {
        if (iMySts.IsoAtt > RX_ATT_MAX_NUM) iMySts.IsoAtt = RX_ATT_MAX_NUM;
        SubAtten += iMySts.IsoAtt;
    }
    else
    {
        return;
    }

    /* ATT Offset */
    Offset = Table_GetAttOffset( SubAtten,TABLE_IDX_TX_GAIN_ATT);

    bTemp = (u8)(SubAtten + Offset);

    /* ATT 범위 제한 */
    if (bTemp > ATT_MAX_NUM) bTemp = ATT_MAX_NUM;
    Atten_SendProc(bTemp, TargetAtten);
}

// RX_ATT1, 호출불, att 값
void Atten_TestAtt(u8 TargetAtten, s8 TestAtt)
{
    u8 bTemp;

    if (iMySts.Test_ModeAtt == OFF)
        return;

    AttPrint("\r\n Test Att: %d[dB]", TestAtt);

    bTemp = (u8)(TestAtt * 2);

    if (bTemp > ATT_MAX_NUM)
        bTemp = ATT_MAX_NUM;

    Atten_SendProc(bTemp, TargetAtten);
}

void Atten_SendProc(u8 SerialData8Bit, u8 TargetAtten)
{
    u8 i;

    if (TargetAtten >= ATT_MAX_CNT)
        return;

    for (i = 0; i < 8; i++)
    {
        Atten_ClockOnOff(0);

        if (SerialData8Bit & 0x01)
            Atten_DataOnOff(1);
        else
            Atten_DataOnOff(0);

        Atten_ClockDelay();

        Atten_ClockOnOff(1);
        Atten_ClockDelay();

        SerialData8Bit >>= 1;
    }

    Atten_ClockOnOff(0);

    HAL_GPIO_WritePin(
        AttEnGpio[TargetAtten].Port,
        AttEnGpio[TargetAtten].Pin,
        ON);

    Atten_ClockDelay();

    HAL_GPIO_WritePin(
        AttEnGpio[TargetAtten].Port,
        AttEnGpio[TargetAtten].Pin,
        OFF);

    Atten_ClockDelay();
}


void Atten_ClockOnOff(u8 Mode)
{
	if(Mode){							//1: ON
        HAL_GPIO_WritePin(TX_ATT_CLK_GPIO_Port, TX_ATT_CLK_Pin, ON);
	}
	else{
		HAL_GPIO_WritePin(TX_ATT_CLK_GPIO_Port, TX_ATT_CLK_Pin, OFF);
	}
}

void Atten_DataOnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(TX_ATT_DATA_GPIO_Port, TX_ATT_DATA_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(TX_ATT_DATA_GPIO_Port, TX_ATT_DATA_Pin, OFF);
	}
}


void Atten_ClockDelay(void)
{
	DWT_DelayUs(10);
}


void Atten_RxAtt1_OnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(RX_ATT_EN1_GPIO_Port, RX_ATT_EN1_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(RX_ATT_EN1_GPIO_Port, RX_ATT_EN1_Pin, OFF);  
	}
}

void Atten_RxAtt2_OnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(RX_ATT_EN2_GPIO_Port, RX_ATT_EN2_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(RX_ATT_EN2_GPIO_Port, RX_ATT_EN2_Pin, OFF);  
	}
}

void Atten_RxAtt3_OnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(RX_ATT_EN3_GPIO_Port, RX_ATT_EN3_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(RX_ATT_EN3_GPIO_Port, RX_ATT_EN3_Pin, OFF);  
	}
}

void Atten_TxAtt1_OnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(TX_ATT_EN1_GPIO_Port, TX_ATT_EN1_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(TX_ATT_EN1_GPIO_Port, TX_ATT_EN1_Pin, OFF);  
	}
}

void Atten_TxAtt2_OnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(TX_ATT_EN2_GPIO_Port, TX_ATT_EN2_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(TX_ATT_EN2_GPIO_Port, TX_ATT_EN2_Pin, OFF);  
	}
}

void Atten_TxAtt3_OnOff(u8 Mode)
{
	if(Mode){							//1: ON
		HAL_GPIO_WritePin(TX_ATT_EN3_GPIO_Port, TX_ATT_EN3_Pin, ON);  
	}
	else{
		HAL_GPIO_WritePin(TX_ATT_EN3_GPIO_Port, TX_ATT_EN3_Pin, OFF);  
	}
}