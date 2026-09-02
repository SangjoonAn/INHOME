/*
 * table.c
 *
 *  Created on: 2026. 8. 12.
 *      Author: LG
 */

#include "table.h"
#include "bsp_uart.h"
#include "utils.h"
#include <string.h>

u16 TxDetAdc[41] = { 3089, 3003, 2917, 2832, 2746, 2661, 2575, 2506, 2436, 2367, 2297, 2228, 2188, 2148, 2109, 2069, 2029, 1990, 1951, 1912, 1873, 1834, 1797, 1760, 1724, 1688, 1652, 1616, 1580, 1544, 1508, 1472, 1436, 1400, 1364, 1328, 1292, 1256, 1220, 1184, 1148 };
u16 RxDetAdc[73] =
{
    2541, 2506, 2470, 2434, 2399, 2364, 2328, 2292, 2257, 2220,
    2183, 2150, 2118, 2078, 2045, 2007, 1976, 1936, 1896, 1854,
    1813, 1778, 1743, 1706, 1680, 1652, 1625, 1598, 1570, 1543,
    1515, 1488, 1460, 1433, 1405, 1378, 1350, 1322, 1295, 1268,
    1240, 1213, 1186, 1159, 1132, 1105, 1078, 1051, 1024, 997,
    970, 943, 916, 889, 862, 835, 808, 781, 754, 727,
    700, 673, 646, 619, 592, 565, 538, 511, 484, 457,
    430, 403, 376
};
s8  TxGainAttTable[64] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };
s8  TxBalanceAttTable[64] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };
s8  RxGainAttTable[64] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };
s8  RxBalanceAttTable[64] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

s8 TxTempAttTable[23]={6, 5, 4, 3, 3, 2, 2, 2, 2, 1, 1, 0,  0, 0, 0, 0, -1, -2, -2,-3, -4, -5, -6};
s8 RxTempAttTable[23]={6, 5, 4, 3, 3, 2, 2, 2, 2, 1, 1, 0,  0, 0, 0, 0, -1, -2, -2,-3, -4, -5, -6};

//u8 ATT_Table[61]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60};

u8 ATT_Table[61] = {
     0,  2,  4,  6,  8, 10, 12, 14, 16, 18,
    20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
    40, 42, 44, 46, 48, 50, 52, 54, 56, 58,
    60, 62, 64, 66, 68, 70, 72, 74, 76, 78,
    80, 82, 84, 86, 88, 90, 92, 94, 96, 98,
   100,102,104,106,108,110,112,114,116,118,
   120
};

/*===========================================================
 * Global Variables
 *==========================================================*/

TABLE_DATA_LIST_t gTableData;

TABLE_INFO_LIST_t gTableInfo =
{
    {
        /* Start, Step, Length, StartAddress, DataSize, DataPtr */
        {30, -2, 41, mTX_DET,              DATA_SIZE_2, (u8 *)gTableData.TxDet        ,0,0,ConvTablePower10},
        {54, -2, 27, mRX_DET,              DATA_SIZE_2, (u8 *)gTableData.RxDet        ,0,0,ConvTablePower10},
        {0, 1, 0, mRESERVED3,              DATA_SIZE_2, (u8 *)0                         ,0,0,0},
        {0, 1, 0, mRESERVED4,              DATA_SIZE_2, (u8 *)0                         ,0,0,0},

        {63, -1, 64, mTX_GAIN_ATT,         DATA_SIZE_1, gTableData.TxGainAtt          ,0,0,0},
        {63, -1, 64, mTX_BALANCE_ATT,      DATA_SIZE_1, gTableData.TxBalanceAtt       ,0,0,0},
        {63, -1, 64, mRX_GAIN_ATT,         DATA_SIZE_1, gTableData.RxGainAtt          ,0,0,0},
        {63, -1, 64, mRX_BALANCE_ATT,      DATA_SIZE_1, gTableData.RxBalanceAtt       ,0,0,0},

        {-60, 10, 0, mTX_TEMP_ADJ,            DATA_SIZE_1, gTableData.Tx_TempAdj        ,0,0,0},
        {-60, 10, 0, mRX_TEMP_ADJ,            DATA_SIZE_1, gTableData.Rx_TempAdj        ,0,0,0},
        {0, 0, 0, mRESERVED10,             DATA_SIZE_1, 0                              ,0,0,0},
        {0, 0, 0, mRESERVED11,             DATA_SIZE_1, 0                              ,0,0,0},
        {0, 0, 0, mRESERVED12,             DATA_SIZE_1, 0                              ,0,0,0}
    }
};

u8 Table_GetDataSize(u8 TableIndex)
{
    TABLE_INFO_t *pInfo;

    if (TableIndex >= TABLE_INDEX_COUNT) return FALSE;

    pInfo = &gTableInfo.Table[TableIndex];

    if(pInfo->DataSize == DATA_SIZE_1){
        return DATA_SIZE_1;
    }
    else{
        return DATA_SIZE_2;
    }
}


/*===========================================================
 * Load Header
 *
 * EEPROM:
 *
 * 0x1000 + TableIndex * 2
 *
 * [Start]
 * [Length]
 *
 * StartAddress / DataSize / DataPtr는 읽지 않는다.
 *==========================================================*/
u8 Table_LoadHeader(u8 TableIndex)
{
    TABLE_INFO_t *pInfo;
    u8 Buffer[TABLE_HEADER_SIZE];
    u16 Address;

    if (TableIndex >= TABLE_INDEX_COUNT) return FALSE;

    pInfo = &gTableInfo.Table[TableIndex];
    Address = GET_HEADER_ADDR(TableIndex);

    if (I2C_EE_BufferRead(Buffer, Address, sizeof(Buffer)) != EEPROM_OK)
    {
        return FALSE;
    }

    pInfo->Start  = Buffer[0];
    pInfo->Length = Buffer[1];

    return TRUE;
}


/*===========================================================
 * Save Header
 *
 * 지정한 TableIndex의 Header만 EEPROM에 저장
 *==========================================================*/
u8 Table_SaveHeader(u8 TableIndex)
{
    TABLE_INFO_t *pInfo;
    u8 Buffer[2];
    u16 Address;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        return FALSE;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    Buffer[0] = pInfo->Start;
    Buffer[1] = pInfo->Length;

    Address = GET_HEADER_ADDR(TableIndex);

    if (I2C_EE_BufferWrite(Buffer, Address, sizeof(Buffer)) != EEPROM_OK)
    {
        return FALSE;
    }

    return TRUE;
}


u8 Table_LoadData(u8 TableIndex)
{
    TABLE_INFO_t *pInfo;
    u16 Address;
    u16 DataLength;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        return FALSE;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    if(pInfo->Length == 0){
        DataLength = TABLE_MAX_SIZE * pInfo->DataSize;
    }
    else DataLength = pInfo->Length * pInfo->DataSize;

    Address = pInfo->StartAddress;

    if (I2C_EE_BufferRead(pInfo->DataPtr, Address, DataLength) != EEPROM_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/*===========================================================
 * Load Table
 *
 * TableIndex + Length
 *
 * Start는 사용하지 않는다.
 *
 * Length만큼 EEPROM에서 RAM으로 읽는다.
 *==========================================================*/
u8 Table_LoadTable(u8 TableIndex, u8 *pSubData, u16 *pSubDataLength)
{
    TABLE_INFO_t *pInfo;
    u16 DataLength;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n%d][TABLE][ERR] Table_LoadTable() TableIndex Error",HAL_GetTick());
        return FALSE;
    }

    if ((pSubData == NULL) || (pSubDataLength == NULL))
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_LoadTable() Parameter Error", HAL_GetTick());
        return FALSE;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    if (pInfo->DataPtr == 0U)
    {
        DebugPrint("\r\n%d][TABLE][ERR] Table_LoadTable() DataPtr Error",HAL_GetTick());
        return FALSE;
    }

    pSubData[TABLE_SUBDATA_INDEX]  = TableIndex;
    pSubData[TABLE_SUBDATA_START]  = pInfo->Start;
    pSubData[TABLE_SUBDATA_LENGTH] = pInfo->Length;
    DataLength = (u16)pInfo->Length * pInfo->DataSize;

    if (pInfo->Length != 0){
        if (pInfo->DataSize == DATA_SIZE_2)
        {
            Memcpy_U16ToBigEndian( &pSubData[TABLE_SUBDATA_DATA], (const u16 *)pInfo->DataPtr, pInfo->Length);
        }
        else
        {
            memcpy( &pSubData[TABLE_SUBDATA_DATA], pInfo->DataPtr, DataLength);
        }
    }

    *pSubDataLength = TABLE_SUBDATA_DATA_OFFSET + DataLength;

    return TRUE;
}


/*===========================================================
 * Save Table
 *
 * GUI:
 *
 * TableIndex
 * Start
 * Length
 * Data
 *
 * 호출 전에 GUI 데이터가 DataPtr에 복사되어 있어야 한다.
 *
 * 처리 순서:
 *
 * 1. RAM Data → EEPROM Data
 * 2. RAM Header Start/Length 변경
 * 3. EEPROM Header 저장
 *
 * Header 저장 실패 시에도 RAM Start/Length는
 * 그대로 유지한다.
 *==========================================================*/

u8 Table_SaveTable(u8 TableIndex, u8 Start, u8 Length, const u8 *pData)
{
    TABLE_INFO_t *pInfo;
    u16 DataLength;
 
    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        return FALSE;
    }
 
    pInfo = &gTableInfo.Table[TableIndex];
 
    if (pInfo->DataPtr == NULL)
    {
        return FALSE;
    }
 
    if (Length == 0U || pData == NULL)
    {
        /* 데이터는 쓰지 않고 Header(Start/Length=0)만 갱신 */
        pInfo->Start  = Start;
        pInfo->Length = 0U;
 
        return Table_SaveHeader(TableIndex);
    }
 
    DataLength = (u16)Length * pInfo->DataSize;


    memcpy((u8 *)pInfo->DataPtr, pData, DataLength);

 
    if (I2C_EE_BufferWrite(pInfo->DataPtr, pInfo->StartAddress, DataLength) != EEPROM_OK)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_SaveTable Failed", HAL_GetTick());
        return FALSE;
    }
 
    pInfo->Start  = Start;
    pInfo->Length = Length;

    if(Table_SaveHeader(TableIndex) != TRUE){
        DebugPrint("\r\n[%d][TABLE][ERR] TableHeader_Save Failed", HAL_GetTick());
        return FALSE;
    }

    return  TRUE;
}


/*===========================================================
 * Init
 *==========================================================*/
void Table_Init(void)
{
    u8 i;

    memset(&gTableData, 0, sizeof(gTableData));

    for (i = 0U; i < TABLE_INDEX_COUNT; i++)
    {
        if(gTableInfo.Table[i].DataPtr != 0){
            if (Table_LoadHeader(i))
            {
                Table_LoadData(i);
            }
        }

    }

}


void ConvTablePower10(u8 TableIndex)
{
    TABLE_INFO_t *pInfo;
    u16 *pData;

    s16 StartNum;
    s16 TableStep;

    u16 AdResult;
    s16 i;

    s16 BaseData;
    s16 AdOffset;
    s16 TableRange;
    s16 TableOffset;
    s32 MeanData;

    /*-------------------------------------------------------
     * Table 정보 확인
     *------------------------------------------------------*/
    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] ConvTablePower10() TableIndex(%d) >= TABLE_INDEX_COUNT", HAL_GetTick(), TableIndex);
        return;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    /*-------------------------------------------------------
     * DET Table 확인
     *------------------------------------------------------*/
    if ((pInfo->DataPtr == NULL) || (pInfo->Length == 0U))
    {
        pInfo->MeanData = 0;
        DebugPrint("\r\n[%d][TABLE][ERR] ConvTablePower10() DataPtr NULL", HAL_GetTick());
        return;
    }

    /*-------------------------------------------------------
     * DET Table
     *------------------------------------------------------*/
    pData = (u16 *)pInfo->DataPtr;

    /*-------------------------------------------------------
     * 현재 ADC 값
     *------------------------------------------------------*/
    AdResult = pInfo->AdResult;

    /*-------------------------------------------------------
     * Table Start / Step
     *------------------------------------------------------*/
    StartNum  = (s16)pInfo->Start;
    TableStep = (s16)pInfo->Step;


    /*-------------------------------------------------------
     * 첫 번째 Table보다 높은 영역
     *------------------------------------------------------*/
    if (AdResult > (pData[0]))
    {
        //pInfo->MeanData = (StartNum - TableStep) * 10;
        pInfo->MeanData = (StartNum - TableStep) * 5;
        return;
    }

    /*-------------------------------------------------------
     * Table 검색
     *------------------------------------------------------*/
    for (i = 1; i < (s16)pInfo->Length; i++)
    {
        /*
         * ADC Table 범위
         *
         * 예:
         * pData[16] = 2029
         * pData[17] = 1990
         *
         * TableRange = 39
         */
        TableRange = (s32)pData[i - 1] - (s32)pData[i];

        if (TableRange <= 0)
        {
            pInfo->MeanData = 0;

            DebugPrint("\r\n[%d][TABLE][ERR]ConvTablePower10 Table Error ",HAL_GetTick());
            return;
        }


        /*
         * 현재 ADC가 현재 구간에 있는지 확인
         *
         * ADC Table이 큰 값 → 작은 값 순서이므로
         *
         * pData[i-1] >= AdResult >= pData[i]
         */
        if (AdResult >= pData[i])
        {
            /*------------------------------------------------
             * 이전 Table의 dB 값
             *
             * i = 17이면
             *
             * StartNum  = 30
             * TableStep = -2
             *
             * BaseData =
             * (30 + (-2 * 16)) * 10
             * = -20
             *
             * 즉 -2.0 dB
             *------------------------------------------------*/
            BaseData = ((s32)StartNum + ((s32)TableStep * (i - 1))) * 10;

            /*------------------------------------------------
             * 이전 ADC 값에서 현재 ADC까지 이동한 거리
             *
             * 2029 → 2000
             *
             * AdOffset = 2029 - 2000
             *          = 29
             *------------------------------------------------*/
            AdOffset = (s32)pData[i - 1] - (s32)AdResult;


            /*------------------------------------------------
             * 선형 보간
             *
             * MeanData =
             * BaseData
             * + (AdOffset / TableRange)
             *   * (TableStep * 10)
             *
             * 예:
             *
             * BaseData   = -20
             * AdOffset   = 29
             * TableRange = 39
             * TableStep  = -2
             *
             * MeanData =
             * -20 + (29 * -2 * 10) / 39
             *
             * = -20 - 14
             * = -34
             *
             * → -3.4 dB
             *------------------------------------------------*/
            MeanData = BaseData + ((AdOffset * (s32)TableStep * 10) / TableRange);
            //Start/Step이 2배로 저장되어 있으므로 MeanData를 1/2로 나누어 실제 dB 값으로 변환
            MeanData = MeanData >> 1;
            pInfo->MeanData = (s16)MeanData;

            return;
        }
    }

    /*-------------------------------------------------------
     * 마지막 Table보다 낮은 영역
     *------------------------------------------------------*/
    //pInfo->MeanData =(StartNum - TableStep + (i * TableStep)) * 10;
    pInfo->MeanData =(StartNum - TableStep + (i * TableStep)) * 5;
}

// 입력 ATT -> Table 시작 ATT와 차이 -> Table 항목 계산
// Table Start/Step은 실제 ATT 값의 2배로 저장
// 입력 ATT도 동일한 Scale
s8 Table_GetAttOffset(s8 Att, u8 TableIndex)
{
    s16 Step;
    s16 Index;
    s16 Diff;
    TABLE_INFO_t *pInfo;

    /* Table Index 범위 확인 */
    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetAttOffset() TableIndex >= TABLE_INDEX_COUNT",
                   HAL_GetTick());
        return 0;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    /* Table Length 확인 */
    if (pInfo->Length == 0U)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetAttOffset() Table Length == 0",
                   HAL_GetTick());
        return 0;
    }

    /* Data Pointer 확인 */
    if (pInfo->DataPtr == NULL)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetAttOffset() DataPtr == NULL",
                   HAL_GetTick());
        return 0;
    }

    /*
     * ATT -> Table Index
     *
     * Att * 2 : Table Scale에 맞게 변환
     *
     * 예)
     * Att   = 60 (30dB)
     * Start = -60 (-30 dB)
     * Step  = 10 (5 dB)
     *
     * Index = (60 - (-60)) / 10
     *       = 12
     */
    Diff = ((s16)Att) - (s16)pInfo->Start;
    Step = (s16)pInfo->Step;

    if (Step != 0)
    {
        Index = Diff / Step;
    }
    else
    {
        Index = 0;
    }

    /* 최소 Index 제한 */
    if (Index < 0)
    {
        Index = 0;
    }

    /* 최대 Index 제한 */
    if (Index >= (s16)pInfo->Length)
    {
        Index = (s16)pInfo->Length - 1;
    }

    return ((s8 *)pInfo->DataPtr)[Index];
}


// 입력 온도 -> Table 시작 온도와 차이 -> Table 항목 계산
// Table Start/Step은 0.5℃ 단위를 1로 표현하기 위해 2배 저장
// 입력 온도도 *2하여 동일한 Scale로 계산
s8 Table_GetTempAtt(s8 Temp, u8 TableIndex)
{
    s16 Step;
    s16 Index;
    s16 Diff;
    TABLE_INFO_t *pInfo;

    /* Table Index 범위 확인 */
    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetTempAtt() TableIndex >= TABLE_INDEX_COUNT",
                   HAL_GetTick());
        return 0;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    /* Table Length 확인 */
    if (pInfo->Length == 0U)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetTempAtt() pInfo->Length == 0",
                   HAL_GetTick());
        return 0;
    }

    /* Data Pointer 확인 */
    if (pInfo->DataPtr == NULL)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetTempAtt() DataPtr == NULL",
                   HAL_GetTick());
        return 0;
    }

    /*
     * Temperature -> Table Index
     *
     * Temp     : 실제 온도
     * Temp * 2 : Table Scale에 맞게 변환
     *
     * 예)
     * Temp  = 32℃
     * Start = 63 (31.5℃)
     * Step  = 1  (0.5℃)
     *
     * Index = (64 - 63) / 1 = 1
     */
    Diff = ((s16)Temp * 2) - (s16)pInfo->Start;
    Step = (s16)pInfo->Step;

    if (Step != 0)
    {
        Index = Diff / Step;
    }
    else
    {
        Index = 0;
    }

    /* 최소 Index 제한 */
    if (Index < 0)
    {
        Index = 0;
    }

    /* 최대 Index 제한 */
    if (Index >= (s16)pInfo->Length)
    {
        Index = (s16)pInfo->Length - 1;
    }

    return ((s8 *)pInfo->DataPtr)[Index];
}


void Table_SetFactory(void)
{
    u8 TableIndex = 0;

    gTableInfo.Table[TableIndex].Start = 30;
    gTableInfo.Table[TableIndex].Length  = sizeof(TxDetAdc) / DATA_SIZE_2;    
    Table_SaveTable(TableIndex, gTableInfo.Table[TableIndex].Start, gTableInfo.Table[TableIndex].Length, (u8 *)TxDetAdc);
    TableIndex++;

    gTableInfo.Table[TableIndex].Start = 54;
    gTableInfo.Table[TableIndex].Length = sizeof(RxDetAdc) / DATA_SIZE_2;
    Table_SaveTable(TableIndex, gTableInfo.Table[TableIndex].Start, gTableInfo.Table[TableIndex].Length, (u8 *)RxDetAdc);
    TableIndex++;

    gTableInfo.Table[TableIndex].Start = 63;
    gTableInfo.Table[TableIndex].Length = sizeof(TxGainAttTable);
    Table_SaveTable(TableIndex, gTableInfo.Table[TableIndex].Start, gTableInfo.Table[TableIndex].Length, TxGainAttTable);
    TableIndex++;

    gTableInfo.Table[TableIndex].Start = 63;
    gTableInfo.Table[TableIndex].Length = sizeof(TxBalanceAttTable);
    Table_SaveTable(TableIndex, gTableInfo.Table[TableIndex].Start, gTableInfo.Table[TableIndex].Length, TxBalanceAttTable);
    TableIndex++;

    gTableInfo.Table[TableIndex].Start = 63;
    gTableInfo.Table[TableIndex].Length = sizeof(RxGainAttTable);
    Table_SaveTable(TableIndex, gTableInfo.Table[TableIndex].Start, gTableInfo.Table[TableIndex].Length, RxGainAttTable);
    TableIndex++;

    gTableInfo.Table[TableIndex].Start = 63;
    gTableInfo.Table[TableIndex].Length = sizeof(RxBalanceAttTable);
    Table_SaveTable(TableIndex, gTableInfo.Table[TableIndex].Start, gTableInfo.Table[TableIndex].Length, RxBalanceAttTable);

   

}

u8 Table_SetAdResult(u8 TableIndex, u16 AdResult)
{
    TABLE_INFO_t *pInfo;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_SetAdResult() TableIndex(%d) >= TABLE_INDEX_COUNT", HAL_GetTick(), TableIndex);
        return FALSE;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    if (pInfo->DataPtr == NULL)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_SetAdResult() DataPtr NULL", HAL_GetTick());
        return FALSE;
    }

    pInfo->AdResult = AdResult;

    if (pInfo->AdConvertMean != NULL)
    {
        pInfo->AdConvertMean(TableIndex);
    }
    else{
        DebugPrint("\r\n[%d][TABLE][ERR] Table_SetAdResult() AdConvertMean NULL", HAL_GetTick());
    }

    return TRUE;
}

s16 Table_GetMeanData(u8 TableIndex)
{
    TABLE_INFO_t *pInfo;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetMeanData() TableIndex(%d) >= TABLE_INDEX_COUNT", HAL_GetTick(), TableIndex);
        return 0;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    if (pInfo->DataPtr == NULL)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_GetMeanData() DataPtr NULL", HAL_GetTick());
        return 0;
    }

    pInfo->AdConvertMean(TableIndex);

    return pInfo->MeanData;
}