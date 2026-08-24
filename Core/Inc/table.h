#ifndef __TABLE_H
#define __TABLE_H

#include "common.h"
#include "main.h"
#include "bsp_eep.h"
#include <stdint.h>

/*===========================================================
 * EEPROM Address
 *==========================================================*/

#define TABLE_HEADER_BASE_ADDR      0x0700U
#define TABLE_DATA_BASE_ADDR        0x0800U
#define TABLE_MAX_SIZE              255U


/*===========================================================
 * Table Index
 *==========================================================*/

#define TABLE_IDX_TX_DET            0U
#define TABLE_IDX_RX_DET            1U
#define TABLE_IDX_SLEEP_DET         2U
#define TABLE_IDX_RESERVED3         3U
#define TABLE_IDX_TX_GAIN_ATT       4U
#define TABLE_IDX_TX_BALANCE_ATT    5U
#define TABLE_IDX_RX_GAIN_ATT       6U
#define TABLE_IDX_RX_BALANCE_ATT    7U
#define TABLE_IDX_RESERVED8         8U
#define TABLE_IDX_RESERVED9         9U
#define TABLE_IDX_RESERVED10        10U
#define TABLE_IDX_RESERVED11        11U
#define TABLE_IDX_RESERVED12        12U
#define TABLE_INDEX_COUNT           13U


#define TABLE_SUBDATA_INDEX             0U
#define TABLE_SUBDATA_START             1U
#define TABLE_SUBDATA_LENGTH            2U
#define TABLE_SUBDATA_DATA              3U
#define TABLE_SUBDATA_DATA_OFFSET       3U

/*===========================================================
 * Element Size
 *==========================================================*/

#define DATA_SIZE_2      2U
#define DATA_SIZE_1      1U


/*===========================================================
 * EEPROM Header
 *
 * EEPROM에는 Start / Length만 저장
 *
 * StartAddress
 * DataSize
 * DataPtr
 *
 * 위 3개는 Firmware에서 고정
 *==========================================================*/

#define TABLE_HEADER_SIZE       2U
#define TABLE_HEADER_DATA_SIZE  (TABLE_INDEX_COUNT * TABLE_HEADER_SIZE)
#define GET_HEADER_ADDR(idx)    (TABLE_HEADER_BASE_ADDR + ((u16)(idx) * TABLE_HEADER_SIZE))

/*===========================================================
 * EEPROM Data Address
 *==========================================================*/
#define mTX_DET         (TABLE_DATA_BASE_ADDR)
#define mRX_DET         (mTX_DET + (TABLE_MAX_SIZE * DATA_SIZE_2))
#define mTX_TEMP_ADJ      (mRX_DET + (TABLE_MAX_SIZE * DATA_SIZE_2))
#define mRX_TEMP_ADJ      (mTX_TEMP_ADJ + (TABLE_MAX_SIZE * DATA_SIZE_2))
#define mTX_GAIN_ATT    (mRX_TEMP_ADJ + (TABLE_MAX_SIZE * DATA_SIZE_2))

#define mTX_BALANCE_ATT (mTX_GAIN_ATT + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRX_GAIN_ATT    (mTX_BALANCE_ATT + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRX_BALANCE_ATT (mRX_GAIN_ATT + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRESERVED8      (mRX_BALANCE_ATT + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRESERVED9      (mRESERVED8 + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRESERVED10     (mRESERVED9 + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRESERVED11     (mRESERVED10 + (TABLE_MAX_SIZE * DATA_SIZE_1))
#define mRESERVED12     (mRESERVED11 + (TABLE_MAX_SIZE * DATA_SIZE_1))


/*===========================================================
 * Table Size
 *==========================================================*/

#define TABLE_DET_COUNT             4U
#define TABLE_DET_TOTAL_SIZE       (TABLE_MAX_SIZE * DATA_SIZE_2 * TABLE_DET_COUNT)

#define TABLE_ATT_COUNT             9U

#define TABLE_ATT_TOTAL_SIZE       (TABLE_MAX_SIZE * DATA_SIZE_1 * TABLE_ATT_COUNT)
#define TABLE_TOTAL_DATA_SIZE      (TABLE_DET_TOTAL_SIZE + TABLE_ATT_TOTAL_SIZE)
#define TABLE_TOTAL_EEPROM_SIZE    ((TABLE_DATA_BASE_ADDR - TABLE_HEADER_BASE_ADDR) + TABLE_TOTAL_DATA_SIZE)


#define TABLE_SUBDATA_SIZE  765

/*===========================================================
 * RAM Table
 *==========================================================*/

typedef struct
{
    u16 TxDet[TABLE_MAX_SIZE];
    u16 RxDet[TABLE_MAX_SIZE];
    u16 Tx_TempAdj[TABLE_MAX_SIZE];
    u16 Rx_TempAdj[TABLE_MAX_SIZE];

    u8 TxGainAtt[TABLE_MAX_SIZE];
    u8 TxBalanceAtt[TABLE_MAX_SIZE];
    u8 RxGainAtt[TABLE_MAX_SIZE];
    u8 RxBalanceAtt[TABLE_MAX_SIZE];

    u8 Reserved8[TABLE_MAX_SIZE];
    u8 Reserved9[TABLE_MAX_SIZE];
    u8 Reserved10[TABLE_MAX_SIZE];
    u8 Reserved11[TABLE_MAX_SIZE];
    u8 Reserved12[TABLE_MAX_SIZE];

} TABLE_DATA_LIST_t;


/*===========================================================
 * Table Information
 *==========================================================*/

typedef struct
{
    s8  Start;          // Table Start
    s8  Step;           // Table Step
    u8  Length;         // Valid data length
    u16 StartAddress;   // EEPROM data address
    u8  DataSize;       // 1 or 2 bytes
    void *DataPtr;        // RAM data
    s16 MeanData;
    u16 AdResult;
    void 	(*AdConvertMean)(u8 TableIndex);
} TABLE_INFO_t;


typedef struct
{
    TABLE_INFO_t Table[TABLE_INDEX_COUNT];

} TABLE_INFO_LIST_t;


/*===========================================================
 * Global
 *==========================================================*/
extern TABLE_DATA_LIST_t gTableData;
extern TABLE_INFO_LIST_t gTableInfo;


/*===========================================================
 * API
 *==========================================================*/

void Table_Init(void);
u8 Table_GetDataSize(u8 TableIndex);
u8 Table_LoadHeader(u8 TableIndex);
u8 Table_LoadData(u8 TableIndex);
u8 Table_SaveTable(u8 TableIndex, u8 Start, u8 Length, const u8 *pData);
u8 Table_SaveHeader(u8 TableIndex);
u8 Table_SetAdResult(u8 TableIndex, u16 AdResult);
void Table_SetFactory(void);
u8 Table_LoadTable(u8 TableIndex, u8 *pSubData, u16 *pSubDataLength);
void ConvTablePower10(u8 TableIndex);


extern u16 TxDetAdc[41];
extern s8 TxGainAttTableOffset[64];

#endif