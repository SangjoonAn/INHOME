/*
 * Alarm.h
 *
 *  Created on: 2026. 8. 17.
 *      Author: LG
 */

#ifndef INC_ALARM_H_
#define INC_ALARM_H_

#include "common.h"
/*===========================================================
 * Alarm Log
 *==========================================================*/

#define ALARM_LOG_COUNT              100U
#define ALARM_LOG_ENTRY_SIZE         6U
#define ALARM_LOG_BLOCK_SIZE         (ALARM_LOG_COUNT * ALARM_LOG_ENTRY_SIZE)

#define ALARM_COUNT_INDEX_POWER_ON       0U
#define ALARM_COUNT_INDEX_FWD_SD         1U
#define ALARM_COUNT_INDEX_REV_SD         2U
#define ALARM_COUNT_INDEX_OSC            3U


/*===========================================================
 * EEPROM Address
 *==========================================================*/

/* Alarm Count */
#define ALARM_LOG_POWER_ON_ADDR      0x1400U
#define ALARM_LOG_FWD_SD_COUNT_ADDR  (ALARM_LOG_POWER_ON_ADDR + 4U)
#define ALARM_LOG_REV_SD_COUNT_ADDR  (ALARM_LOG_FWD_SD_COUNT_ADDR + 4U)
#define ALARM_LOG_OSC_COUNT_ADDR     (ALARM_LOG_REV_SD_COUNT_ADDR + 4U)

/* Alarm Log Write Index */
#define ALARM_LOG_WRITE_INDEX_ADDR   0x13FEU

/* Alarm Log */
#define ALARM_LOG_INDEX0_ADDR        0x1500U
#define ALARM_LOG_INDEX100_ADDR      (ALARM_LOG_INDEX0_ADDR + ALARM_LOG_BLOCK_SIZE)
#define ALARM_LOG_INDEX200_ADDR      (ALARM_LOG_INDEX100_ADDR + ALARM_LOG_BLOCK_SIZE)
#define ALARM_LOG_INDEX300_ADDR      (ALARM_LOG_INDEX200_ADDR + ALARM_LOG_BLOCK_SIZE)
#define ALARM_LOG_INDEX400_ADDR      (ALARM_LOG_INDEX300_ADDR + ALARM_LOG_BLOCK_SIZE)
#define ALARM_LOG_INDEX500_ADDR      (ALARM_LOG_INDEX400_ADDR + ALARM_LOG_BLOCK_SIZE)

#define ALARM_LOG_TOTAL_BLOCK        6U
#define ALARM_LOG_TOTAL_COUNT        (ALARM_LOG_TOTAL_BLOCK * ALARM_LOG_COUNT)

/*===========================================================
 * GUI Alarm Packet Offset
 *==========================================================*/

#define ALARM_PACKET_INDEX_OFFSET        0U
#define ALARM_PACKET_POWER_ON_OFFSET     2U
#define ALARM_PACKET_FWD_SD_OFFSET       6U
#define ALARM_PACKET_REV_SD_OFFSET       10U
#define ALARM_PACKET_OSC_OFFSET          14U
#define ALARM_PACKET_LOG_OFFSET          18U
#define ALARM_PACKET_DATA_SIZE           600U
#define ALARM_PACKET_TOTAL_SIZE          618U

/*===========================================================
 * Alarm Code
 *==========================================================*/
/* System */
#define ALARM_CODE_HISTORY_CLEAR    0x00U
#define ALARM_CODE_POWER_ON         0x01U
#define ALARM_CODE_POWER_OFF        0x02U

/* Reset */
#define ALARM_CODE_RESET_USER       0x05U
#define ALARM_CODE_RESET_FW         0x06U

/* Forward S/D */
#define ALARM_CODE_FWD_SD_1ST       0x07U
#define ALARM_CODE_FWD_SD_2ND       0x08U
#define ALARM_CODE_FWD_SD_3RD       0x09U
#define ALARM_CODE_FWD_SD_END       0x0AU
#define ALARM_CODE_FWD_SD_RECOVERY  0x0BU

/* Reverse S/D */
#define ALARM_CODE_REV_SD_1ST       0x0CU
#define ALARM_CODE_REV_SD_2ND       0x0DU
#define ALARM_CODE_REV_SD_3RD       0x0EU
#define ALARM_CODE_REV_SD_END       0x0FU
#define ALARM_CODE_REV_SD_RECOVERY  0x10U

/* Reverse Oscillation */
#define ALARM_CODE_OSC              0x11U

/* Isolation */
#define ALARM_CODE_ISO_74DB          0x12U
#define ALARM_CODE_ISO_73DB          0x13U
#define ALARM_CODE_ISO_72DB          0x14U
#define ALARM_CODE_ISO_71DB          0x15U
#define ALARM_CODE_ISO_70DB          0x16U
#define ALARM_CODE_ISO_69DB          0x17U
#define ALARM_CODE_ISO_68DB          0x18U
#define ALARM_CODE_ISO_67DB          0x19U
#define ALARM_CODE_ISO_66DB          0x1AU
#define ALARM_CODE_ISO_65DB          0x1BU
#define ALARM_CODE_ISO_64DB          0x1CU
#define ALARM_CODE_ISO_63DB          0x1DU
#define ALARM_CODE_ISO_62DB          0x1EU
#define ALARM_CODE_ISO_61DB          0x1FU
#define ALARM_CODE_ISO_60DB          0x20U
#define ALARM_CODE_ISO_59DB          0x21U
#define ALARM_CODE_ISO_FAIL          0x22U


/*===========================================================
 * Runtime Alarm Bitmap
 *
 * 실제 세부 Alarm Code를 모두 bit로 만들지 않고
 * 기능 그룹별 발생 여부만 1개의 u32로 관리한다.
 *==========================================================*/

#define ALARM_BIT_HISTORY_CLEAR      (1UL << 0)
#define ALARM_BIT_POWER_ON           (1UL << 1)
#define ALARM_BIT_POWER_OFF          (1UL << 2)
#define ALARM_BIT_RESET_USER         (1UL << 3)
#define ALARM_BIT_RESET_FW           (1UL << 4)

#define ALARM_BIT_FWD_SD             (1UL << 5)
#define ALARM_BIT_REV_SD             (1UL << 6)
#define ALARM_BIT_OSC                (1UL << 7)
#define ALARM_BIT_ISO                (1UL << 8)

#define ALARM_COUNT_MAX                 16

#define ALARM_FWD_SD_COUNT              30

/*===========================================================
 * S/D State
 *==========================================================*/

typedef enum
{
    FWD_SD_NONE = 0,
    FWD_SD_1ST,
    FWD_SD_2ND,
    FWD_SD_3RD,
    FWD_SD_END,
    FWD_SD_RECOVERY
} FWD_SD_STATE_t;


typedef enum
{
    REV_SD_NONE = 0,
    REV_SD_1ST,
    REV_SD_2ND,
    REV_SD_3RD,
    REV_SD_END,
    REV_SD_RECOVERY

} REV_SD_STATE_t;


/*===========================================================
 * Isolation State
 *==========================================================*/

typedef enum
{
    ISO_NONE = 0,
    ISO_74DB,
    ISO_73DB,
    ISO_72DB,
    ISO_71DB,
    ISO_70DB,
    ISO_69DB,
    ISO_68DB,
    ISO_67DB,
    ISO_66DB,
    ISO_65DB,
    ISO_64DB,
    ISO_63DB,
    ISO_62DB,
    ISO_61DB,
    ISO_60DB,
    ISO_59DB,
    ISO_FAIL
} ISO_STATE_t;


/*===========================================================
 * Alarm State
 *==========================================================*/

typedef struct
{
    u16 Status;
    u16 Confirmed;

    FWD_SD_STATE_t FwdSDState;
    REV_SD_STATE_t RevSDState;
    ISO_STATE_t    IsoState;

    u32 PowerOnCount;
    u32 FwdSDCount;
    u32 RevSDCount;
    u32 OscCount;

    u16 LogIndex;

} ALARM_STATE_t;


/*===========================================================
 * Alarm Log Entry
 *==========================================================*/

typedef struct
{
    u8 Time[5];
    u8 Code;

} ALARM_LOG_t;


/*===========================================================
 * Function
 *==========================================================*/

void Alarm_Init(void);
void Alarm_Task(void);

void Alarm_Set(u16 AlarmBit);
void Alarm_Clear(u16 AlarmBit);
u8   Alarm_IsSet(u16 AlarmBit);

void Alarm_SetFwdSD(FWD_SD_STATE_t State);
void Alarm_SetRevSD(REV_SD_STATE_t State);
void Alarm_SetISO(ISO_STATE_t State);

void Alarm_LogSave(u8 AlarmCode);

u8 Alarm_LogRead(u16 Index, ALARM_LOG_t *pLog);
u8 Alarm_LogClear(void);

u8 Alarm_CountRead(u8 Index);
u8 Alarm_CountSave(u8 Index);

u8 Alarm_GetLogBlockAddress(u16 Index, u16 *pAddress);

void Alarm_Save(void);
void Alarm_Check(void);
u16 Alarm_GetCode(u16 Occurred);
u8 Alarm_CountReadAll(void);
u8 Alarm_SendGuiPacket(u16 OffSet, u8 *pData, u16 *pLength);
u32 Alarm_CountGet(u32 Index);

#endif /* INC_ALARM_H_ */
