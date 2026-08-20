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
s8  TxGainAttTableOffset[64] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

/*===========================================================
 * Global Variables
 *==========================================================*/

TABLE_DATA_LIST_t gTableData;

TABLE_INFO_LIST_t gTableInfo =
{
    {
        /* Start, Step, Length, StartAddress, DataSize, DataPtr */
        {0, 1, 0, mTX_DET,              DATA_SIZE_2, (u8 *)gTableData.TxDet        ,0,0,ConvTablePower10},
        {0, 1, 0, mRX_DET,              DATA_SIZE_2, (u8 *)gTableData.RxDet        ,0,0,ConvTablePower10},
        {0, 1, 0, mSLEEP_DET,           DATA_SIZE_2, (u8 *)gTableData.SleepDet     ,0,0,ConvTablePower10},
        {0, 1, 0, mRESERVED3,           DATA_SIZE_2, (u8 *)gTableData.Reserved3    ,0,0,ConvTablePower10},

        {0, 1, 0, mTX_GAIN_ATT,         DATA_SIZE_1, gTableData.TxGainAtt          ,0,0,0},
        {0, 1, 0, mTX_BALANCE_ATT,      DATA_SIZE_1, gTableData.TxBalanceAtt       ,0,0,0},
        {0, 1, 0, mRX_GAIN_ATT,         DATA_SIZE_1, gTableData.RxGainAtt          ,0,0,0},
        {0, 1, 0, mRX_BALANCE_ATT,      DATA_SIZE_1, gTableData.RxBalanceAtt       ,0,0,0},

        {0, 0, 0, mRESERVED8,           DATA_SIZE_1, 0                              ,0,0,0},
        {0, 0, 0, mRESERVED9,           DATA_SIZE_1, 0                              ,0,0,0},
        {0, 0, 0, mRESERVED10,          DATA_SIZE_1, 0                              ,0,0,0},
        {0, 0, 0, mRESERVED11,          DATA_SIZE_1, 0                              ,0,0,0},
        {0, 0, 0, mRESERVED12,          DATA_SIZE_1, 0                              ,0,0,0}
    }
};

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
u8 Table_Get(u8 TableIndex, u8 *pSubData, u16 *pSubDataLength)
{
    TABLE_INFO_t *pInfo;
    u16 DataLength;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        DebugPrint("\r\n%d][TABLE][ERR] Table_Get() TableIndex Error",HAL_GetTick());
        return FALSE;
    }

    if ((pSubData == NULL) || (pSubDataLength == NULL))
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_Get() Parameter Error", HAL_GetTick());
        return FALSE;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    if (pInfo->DataPtr == 0U)
    {
        DebugPrint("\r\n%d][TABLE][ERR] Table_Get() DataPtr Error",HAL_GetTick());
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

//u8 Table_Save2(u8 *SubData)
//{
//    TABLE_INFO_t *pInfo;
//    u8  TableIndex;
//    u8  Start;
//    u8  Length;
//    u16 WriteSize;
//
//    /*-------------------------------------------------------
//     * 최소 Header 3 Byte 확인
//     *------------------------------------------------------*/
//    if (SubData == NULL) return FALSE;
//
//    /*-------------------------------------------------------
//     * SUB_DATA Header
//     *------------------------------------------------------*/
//    TableIndex = SubData[TABLE_SUBDATA_INDEX];
//    Start      = SubData[TABLE_SUBDATA_START];
//    Length     = SubData[TABLE_SUBDATA_LENGTH];
//
//    /*-------------------------------------------------------
//     * Table Index 검사
//     *------------------------------------------------------*/
//    if (TableIndex >= TABLE_INDEX_COUNT || Length == 0U) return FALSE;
//
//
//    pInfo = &gTableInfo.Table[TableIndex];
//
//    if (pInfo->DataPtr == NULL) return FALSE;
//
//
//    /*-------------------------------------------------------
//     * 실제 Data 크기 계산
//     *
//     * DET : Length * 2
//     * ATT : Length * 1
//     *------------------------------------------------------*/
//    WriteSize = (u16)Length * pInfo->DataSize;
//
//    /*-------------------------------------------------------
//     * Data를 RAM Table에 복사
//     *
//     * SUB_DATA[3]부터 실제 Table Data
//     *------------------------------------------------------*/
//    memcpy(pInfo->DataPtr, &SubData[TABLE_SUBDATA_DATA], WriteSize);
//
//    /*-------------------------------------------------------
//     * RAM -> EEPROM Data 저장
//     *------------------------------------------------------*/
//    if (I2C_EE_BufferWrite(pInfo->DataPtr, pInfo->StartAddress, WriteSize) != EEPROM_OK)
//    {
//        return FALSE;
//    }
//
//    /*-------------------------------------------------------
//     * EEPROM Data 저장 성공 후 Header 갱신
//     *------------------------------------------------------*/
//    pInfo->Start  = Start;
//    pInfo->Length = Length;
//
//    if (Table_SaveHeader(TableIndex) == FALSE)
//    {
//        return FALSE;
//    }
//
//    return TRUE;
//}

 /* pData 데이터는 BigEndian으로 들어옴 */
u8 Table_Save(u8 TableIndex, u8 Start, u8 Length, const u8 *pData)
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

    if (pInfo->DataSize == DATA_SIZE_2)
    {
        Memcpy_BigEndianToU16( (u16 *)pInfo->DataPtr, pData, Length);
    }
    else
    {
        memcpy(pInfo->DataPtr, pData, Length);
    }

 
    if (I2C_EE_BufferWrite(pInfo->DataPtr, pInfo->StartAddress, DataLength) != EEPROM_OK)
    {
        DebugPrint("\r\n[%d][TABLE][ERR] Table_Save Failed", HAL_GetTick());
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

    /*
     * 2. RAM Data 초기화
     */
    memset(&gTableData, 0, sizeof(gTableData));

    /*
     * 3. EEPROM Header Load
     *
     * Start / Length만 읽는다.
     */
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

    /*-------------------------------------------------------
     * Table 정보 확인
     *------------------------------------------------------*/
    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        return;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    /*-------------------------------------------------------
     * DET Table 확인
     *------------------------------------------------------*/
    if ((pInfo->DataPtr == NULL) || (pInfo->Length == 0U))
    {
        pInfo->MeanData = 0;
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
        pInfo->MeanData = (StartNum - TableStep) * 10;
        return;
    }

    /*-------------------------------------------------------
     * Table 검색
     *------------------------------------------------------*/
    for (i = 1; i < (s16)pInfo->Length; i++)
    {
        TableOffset =  (s16)((pData[i - 1] - pData[i]) / 2);

        TableRange = (s16)pData[i - 1] - (s16)pData[i];
        if (TableRange <= 0)
        {
            pInfo->MeanData = 0;
            return;
        }
        TableOffset = TableRange / 2;

        if (AdResult > ((s16)pData[i] - TableOffset))
        {
            BaseData = (StartNum + (TableStep * (i - 1))) * 10;

            AdOffset = (s16)AdResult - ((s16)pData[i - 1] - TableOffset);

            pInfo->MeanData = BaseData - (((s32)AdOffset * TableStep * 10) / TableRange);

            return;
        }
    }

    /*-------------------------------------------------------
     * 마지막 Table보다 낮은 영역
     *------------------------------------------------------*/
    pInfo->MeanData =(StartNum - TableStep + (i * TableStep)) * 10;
}

u8 Table_SetAdResult(u8 TableIndex, u16 AdResult)
{
    TABLE_INFO_t *pInfo;

    if (TableIndex >= TABLE_INDEX_COUNT)
    {
        return FALSE;
    }

    pInfo = &gTableInfo.Table[TableIndex];

    if (pInfo->DataPtr == NULL)
    {
        return FALSE;
    }

    pInfo->AdResult = AdResult;

    if (pInfo->AdConvertMean != NULL)
    {
        pInfo->AdConvertMean(TableIndex);
    }

    return TRUE;
}