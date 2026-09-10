/*
 * Alarm.c
 *
 *  Created on: 2026. 8. 17.
 *      Author: LG
 */

#include "Alarm.h"
#include "utils.h"
#include "bsp_timer.h"
#include "bsp_eep.h"
#include "bsp_init.h"
#include "bsp_uart.h"

ALARM_STATE_t gAlarm;

/*===========================================================
 * EEPROM Address
 *==========================================================*/
static const u16 g_AlarmLogAddress[ALARM_LOG_TOTAL_BLOCK] =
{
    ALARM_LOG_INDEX0_ADDR,
    ALARM_LOG_INDEX100_ADDR,
    ALARM_LOG_INDEX200_ADDR,
    ALARM_LOG_INDEX300_ADDR,
    ALARM_LOG_INDEX400_ADDR,
    ALARM_LOG_INDEX500_ADDR
};


/*===========================================================
 * Alarm Init
 *==========================================================*/

void Alarm_Init(void)
{
    memset(&gAlarm, 0, sizeof(gAlarm));

    if (I2C_EE_BufferRead((u8 *)&gAlarm.LogIndex, ALARM_LOG_WRITE_INDEX_ADDR, sizeof(gAlarm.LogIndex)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] ALARM_LOG_WRITE_INDEX_ADDR ERR ", HAL_GetTick());
        gAlarm.LogIndex = 0U;
    }

    if (gAlarm.LogIndex >= ALARM_LOG_TOTAL_COUNT)
    {
        DebugPrint("\r\n %d][ALARM][ERR] LogIndex >= ALARM_LOG_TOTAL_COUNT ERR ", HAL_GetTick());
        gAlarm.LogIndex = 0U;
    }

    Alarm_CountReadAll();

    bsp_timer_set(TimerAlarmCheck, Time100mSec);
}

void Alarm_Task(void)
{
    if(bsp_timer_TimeOverCheck(TimerAlarmCheck)){
        Alarm_Check();
        Alarm_Save();
        bsp_timer_set(TimerAlarmCheck, Time100mSec);
    }	
}


void Alarm_Check(void)
{
    static unsigned char AlarmOccurCnt[ALARM_COUNT_MAX]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static unsigned char AlarmClearCnt[ALARM_COUNT_MAX]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};




}

void Alarm_Save(void)
{
    u16 Change = 0;
    u16 Occurred;

    if(iMySts.Alarm.Data)
    {
        //LED Alarm On
    }
    else{
        //LED Alarm Off
    }

    //Change = iMyCtrl.PreAlarmSts ^ gAlarm.Status;
    //Occurred = Change & gAlarm.Status;

    //Occurred = iMyCtrl.PreAlarmSts ^ gAlarm.Status;

    if(gAlarm.Status){
        if (gAlarm.Status & ALARM_BIT_POWER_ON)
        {
            gAlarm.PowerOnCount++;
            Alarm_CountSave(ALARM_COUNT_INDEX_POWER_ON);
            Alarm_LogSave(ALARM_CODE_POWER_ON);
        }

        if (gAlarm.Status & ALARM_BIT_POWER_OFF)
        {
            Alarm_LogSave(ALARM_CODE_POWER_OFF);
        }

        if (gAlarm.Status & ALARM_BIT_FWD_SD)
        {
            if(gAlarm.FwdSDState == FWD_SD_RECOVERY){
                DebugPrint("\r\n %d][ALARM] Alarm_Save FWD_SD_RECOVERY ", HAL_GetTick());
                return;
            }
            gAlarm.FwdSDCount++;
            Alarm_CountSave(ALARM_COUNT_INDEX_FWD_SD);
            Alarm_LogSave(Alarm_GetCode(ALARM_BIT_FWD_SD));
        }

        if (gAlarm.Status & ALARM_BIT_REV_SD)
        {
            if(gAlarm.RevSDState == REV_SD_RECOVERY){
                DebugPrint("\r\n %d][ALARM] Alarm_Save FWD_SD_RECOVERY ", HAL_GetTick());
                return;
            }
            gAlarm.RevSDCount++;
            Alarm_CountSave(ALARM_COUNT_INDEX_REV_SD);
            Alarm_LogSave(Alarm_GetCode(ALARM_BIT_REV_SD));
        }

        
        if (gAlarm.Status & ALARM_BIT_OSC)
        {
            gAlarm.OscCount++;
            Alarm_CountSave(ALARM_COUNT_INDEX_OSC);
            Alarm_LogSave(Alarm_GetCode(ALARM_BIT_OSC));
        }

        
        if (gAlarm.Status & ALARM_BIT_ISO)
        {
            Alarm_LogSave(Alarm_GetCode(ALARM_BIT_ISO));
        }

        gAlarm.Status = 0;

    }



}

/*===========================================================
 * Alarm Code Conversion
 *==========================================================*/

u16 Alarm_GetCode(u16 AlarmBit)
{
    if (AlarmBit & ALARM_BIT_FWD_SD)
    {
        switch (gAlarm.FwdSDState)
        {
            case FWD_SD_1ST:
                return ALARM_CODE_FWD_SD_1ST;

            case FWD_SD_2ND:
                return ALARM_CODE_FWD_SD_2ND;

            case FWD_SD_3RD:
                return ALARM_CODE_FWD_SD_3RD;

            case FWD_SD_END:
                return ALARM_CODE_FWD_SD_END;

            case FWD_SD_RECOVERY:
                return ALARM_CODE_FWD_SD_RECOVERY;

            default:
                break;
        }
    }

    if (AlarmBit & ALARM_BIT_REV_SD){
        switch (gAlarm.RevSDState)
        {
            case REV_SD_1ST:
                return ALARM_CODE_REV_SD_1ST;

            case REV_SD_2ND:
                return ALARM_CODE_REV_SD_2ND;

            case REV_SD_3RD:
                return ALARM_CODE_REV_SD_3RD;

            case REV_SD_END:
                return ALARM_CODE_REV_SD_END;

            case REV_SD_RECOVERY:
                return ALARM_CODE_REV_SD_RECOVERY;

            default:
                break;
        }
    }

    if (AlarmBit & ALARM_BIT_ISO){
        switch (gAlarm.IsoState)
        {
            case ISO_77DB:
                return ALARM_CODE_ISO_77DB;

            case ISO_76DB:
                return ALARM_CODE_ISO_76DB;

            case ISO_75DB:
                return ALARM_CODE_ISO_75DB;

            case ISO_74DB:
                return ALARM_CODE_ISO_74DB;

            case ISO_73DB:
                return ALARM_CODE_ISO_73DB;

            case ISO_72DB:
                return ALARM_CODE_ISO_72DB;

            case ISO_71DB:
                return ALARM_CODE_ISO_71DB;

            case ISO_70DB:
                return ALARM_CODE_ISO_70DB;

            case ISO_69DB:
                return ALARM_CODE_ISO_69DB;

            case ISO_68DB:
                return ALARM_CODE_ISO_68DB;

            case ISO_67DB:
                return ALARM_CODE_ISO_67DB;

            case ISO_66DB:
                return ALARM_CODE_ISO_66DB;

            case ISO_65DB:
                return ALARM_CODE_ISO_65DB;

            case ISO_64DB:
                return ALARM_CODE_ISO_64DB;

            case ISO_63DB:
                return ALARM_CODE_ISO_63DB;

            case ISO_62DB:
                return ALARM_CODE_ISO_62DB;

            case ISO_FAIL:
                return ALARM_CODE_ISO_FAIL;

            default:
                break;
        }
    }

    if (AlarmBit & ALARM_BIT_OSC){
        return ALARM_CODE_OSC;
    }


    return 0xFFU;
}



/*===========================================================
 * Alarm Set / Clear
 *==========================================================*/

void Alarm_Set(u16 AlarmBit)
{
    gAlarm.Status |= AlarmBit;
}


void Alarm_Clear(u16 AlarmBit)
{
    gAlarm.Status &= ~AlarmBit;
}


void Alarm_SetIso(u16 State)
{
    gAlarm.IsoState = State;
    Alarm_Set(ALARM_BIT_ISO);
}

void Alarm_ClearIso(u16 State)
{
    gAlarm.IsoState = 0;
}



/*===========================================================
 * Forward S/D
 *==========================================================*/
void Alarm_SetFwdSD(FWD_SD_STATE_t State)
{
    gAlarm.FwdSDState |= State;
    Alarm_Set(ALARM_BIT_FWD_SD);

}


/*===========================================================
 * Reverse S/D
 *==========================================================*/

void Alarm_SetRevSD(REV_SD_STATE_t State)
{
    gAlarm.RevSDState |= State;
    Alarm_Set(ALARM_BIT_REV_SD);
}


/*===========================================================
 * Log Address
 *==========================================================*/
u8 Alarm_GetLogBlockAddress(u16 Index, u16 *pAddress)
{
    u16 Block;

    if (pAddress == NULL)
    {
        return FALSE;
    }

    if (Index >= ALARM_LOG_TOTAL_COUNT)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_GetLogBlockAddress Block(%d) >= ALARM_LOG_TOTAL_BLOCK", HAL_GetTick(), Block);

        return FALSE;
    }

    Block = Index / ALARM_LOG_COUNT;

    if (Block >= ALARM_LOG_TOTAL_BLOCK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_GetLogBlockAddress Block(%d) >= ALARM_LOG_TOTAL_BLOCK", HAL_GetTick(), Block);
        return FALSE;
    }

    *pAddress = g_AlarmLogAddress[Block];

    *pAddress += (Index % ALARM_LOG_COUNT) * ALARM_LOG_ENTRY_SIZE;

    return TRUE;
}


/*===========================================================
 * Alarm Log Save
 *==========================================================*/
void Alarm_LogSave(u8 AlarmCode)
{
    ALARM_LOG_t Log;
    u16 Address;

    if (AlarmCode == 0xFFU)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogSave AlarmCode = 0xFF", HAL_GetTick());
        return;
    }

    memset(&Log, 0, sizeof(Log));

    System_GetTime(Log.Time);
    Log.Code = AlarmCode;

    if (!Alarm_GetLogBlockAddress(gAlarm.LogIndex, &Address))
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogSave gAlarm.LogIndex = %d , Addr = %x", HAL_GetTick(), gAlarm.LogIndex, Address);
        return;
    }

    if (I2C_EE_BufferWrite((u8 *)&Log, Address, ALARM_LOG_ENTRY_SIZE) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogSave I2C_EE_BufferWrite ", HAL_GetTick());
        return;
    }

    gAlarm.LogIndex++;

    if (gAlarm.LogIndex >= ALARM_LOG_TOTAL_COUNT)
    {
        gAlarm.LogIndex = 0U;
    }

    I2C_EE_BufferWrite( (u8 *)&gAlarm.LogIndex, ALARM_LOG_WRITE_INDEX_ADDR, sizeof(gAlarm.LogIndex));
}


/*===========================================================
 * Alarm Log Read
 *==========================================================*/

u8 Alarm_LogRead(u16 Index, ALARM_LOG_t *pLog)
{
    u16 Address;

    if (pLog == NULL)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogRead pLog NULL Error ", HAL_GetTick());
        return FALSE;
    }

    if (!Alarm_GetLogBlockAddress(Index, &Address))
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogRead Alarm_GetLogBlockAddress Error ", HAL_GetTick());
        return FALSE;
    }

    if (I2C_EE_BufferRead((u8 *)pLog, Address, ALARM_LOG_ENTRY_SIZE) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogRead I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    return TRUE;
}


/*===========================================================
 * Alarm Log Clear
 *==========================================================*/

u8 Alarm_LogClear(void)
{
    static u8 ClearBuffer[ALARM_LOG_BLOCK_SIZE];

    memset(ClearBuffer, 0xFF, sizeof(ClearBuffer));

    for (u8 i = 0U; i < ALARM_LOG_TOTAL_BLOCK; i++)
    {
        if (I2C_EE_BufferWrite(ClearBuffer, g_AlarmLogAddress[i], ALARM_LOG_BLOCK_SIZE) != EEPROM_OK)
        {
            DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogClear I2C_EE_BufferWrite Error ", HAL_GetTick());
            return FALSE;
        }
    }

    gAlarm.LogIndex = 0U;
    if (I2C_EE_BufferWrite((u8 *)&gAlarm.LogIndex, ALARM_LOG_WRITE_INDEX_ADDR, sizeof(gAlarm.LogIndex)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogClear I2C_EE_BufferWrite Error ", HAL_GetTick());
        return FALSE;
    }

    gAlarm.PowerOnCount = 0U;
    if (I2C_EE_BufferWrite((u8 *)&gAlarm.PowerOnCount, ALARM_LOG_POWER_ON_ADDR, sizeof(gAlarm.PowerOnCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogClear I2C_EE_BufferWrite Error ", HAL_GetTick());
        return FALSE;
    }

    gAlarm.FwdSDCount = 0U;
    if (I2C_EE_BufferWrite((u8 *)&gAlarm.FwdSDCount, ALARM_LOG_FWD_SD_COUNT_ADDR, sizeof(gAlarm.FwdSDCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogClear I2C_EE_BufferWrite Error ", HAL_GetTick());
        return FALSE;
    }

    gAlarm.RevSDCount = 0U;
    if (I2C_EE_BufferWrite((u8 *)&gAlarm.RevSDCount, ALARM_LOG_REV_SD_COUNT_ADDR, sizeof(gAlarm.RevSDCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogClear I2C_EE_BufferWrite Error ", HAL_GetTick());
        return FALSE;
    }

    gAlarm.OscCount = 0U;
    if (I2C_EE_BufferWrite((u8 *)&gAlarm.OscCount, ALARM_LOG_OSC_COUNT_ADDR, sizeof(gAlarm.OscCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_LogClear I2C_EE_BufferWrite Error ", HAL_GetTick());
        return FALSE;
    }

    Alarm_LogSave(ALARM_CODE_HISTORY_CLEAR);




    return TRUE;
}


/*===========================================================
 * Count Read
 *==========================================================*/
u8 Alarm_CountReadAll(void)
{

    if(I2C_EE_BufferRead((u8 *)&gAlarm.PowerOnCount, ALARM_LOG_POWER_ON_ADDR, sizeof(gAlarm.PowerOnCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountReadAll I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    if(I2C_EE_BufferRead((u8 *)&gAlarm.FwdSDCount, ALARM_LOG_FWD_SD_COUNT_ADDR, sizeof(gAlarm.FwdSDCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountReadAll I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    if(I2C_EE_BufferRead((u8 *)&gAlarm.RevSDCount, ALARM_LOG_REV_SD_COUNT_ADDR, sizeof(gAlarm.RevSDCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountReadAll I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    if(I2C_EE_BufferRead((u8 *)&gAlarm.OscCount, ALARM_LOG_OSC_COUNT_ADDR, sizeof(gAlarm.OscCount)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountReadAll I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    return TRUE;
}


/*===========================================================
 * Count Read
 *==========================================================*/
u8 Alarm_CountRead(u8 Index)
{
    u16 Address;
    u32 *pData;

    switch (Index)
    {
        case ALARM_COUNT_INDEX_POWER_ON:
            Address = ALARM_LOG_POWER_ON_ADDR;
            pData = &gAlarm.PowerOnCount;
            break;

        case ALARM_COUNT_INDEX_FWD_SD:
            Address = ALARM_LOG_FWD_SD_COUNT_ADDR;
            pData = &gAlarm.FwdSDCount;
            break;

        case ALARM_COUNT_INDEX_REV_SD:
            Address = ALARM_LOG_REV_SD_COUNT_ADDR;
            pData = &gAlarm.RevSDCount;
            break;

        case ALARM_COUNT_INDEX_OSC:
            Address = ALARM_LOG_OSC_COUNT_ADDR;
            pData = &gAlarm.OscCount;
            break;

        default:
            return FALSE;
    }

    if (I2C_EE_BufferRead((u8 *)pData, Address, sizeof(u32)) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountRead I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    return TRUE;
}

/*===========================================================
 * Count Save
 *==========================================================*/
u8 Alarm_CountSave(u8 Index)
{
    u16 Address;

    switch (Index)
    {
        case ALARM_COUNT_INDEX_POWER_ON:
            Address = ALARM_LOG_POWER_ON_ADDR;
            if(I2C_EE_BufferWrite((u8 *)&gAlarm.PowerOnCount, Address, sizeof(gAlarm.PowerOnCount)) != EEPROM_OK){
                DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountSave I2C_EE_BufferWrite Error ", HAL_GetTick());
                return FALSE;
            }
            break;

        case ALARM_COUNT_INDEX_FWD_SD:
            Address = ALARM_LOG_FWD_SD_COUNT_ADDR;
            if(I2C_EE_BufferWrite((u8 *)&gAlarm.FwdSDCount, Address, sizeof(gAlarm.FwdSDCount)) != EEPROM_OK){
                DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountSave I2C_EE_BufferWrite Error ", HAL_GetTick());
                return FALSE;
            }
            break;

        case ALARM_COUNT_INDEX_REV_SD:
            Address = ALARM_LOG_REV_SD_COUNT_ADDR;
            if(I2C_EE_BufferWrite((u8 *)&gAlarm.RevSDCount, Address, sizeof(gAlarm.RevSDCount)) != EEPROM_OK){
                DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountSave I2C_EE_BufferWrite Error ", HAL_GetTick());
                return FALSE;
            }
            break;

        case ALARM_COUNT_INDEX_OSC:
            Address = ALARM_LOG_OSC_COUNT_ADDR;
            if(I2C_EE_BufferWrite((u8 *)&gAlarm.OscCount, Address, sizeof(gAlarm.OscCount)) != EEPROM_OK){
                DebugPrint("\r\n %d][ALARM][ERR] Alarm_CountSave I2C_EE_BufferWrite Error ", HAL_GetTick());
                return FALSE;
            }
            break;

        default:
            return FALSE;
    }


    return TRUE;
}



u32 Alarm_CountGet(u32 Index)
{
    switch (Index)
    {
        case ALARM_COUNT_INDEX_POWER_ON:
            return gAlarm.PowerOnCount;
            break;

        case ALARM_COUNT_INDEX_FWD_SD:
            return gAlarm.FwdSDCount;
            break;

        case ALARM_COUNT_INDEX_REV_SD:
            return gAlarm.RevSDCount;
            break;

        case ALARM_COUNT_INDEX_OSC:
            return gAlarm.OscCount;
            break;

        default:
            return FALSE;
    }

}

u8 Alarm_SendGuiPacket(u16 Index, u8 *pData, u16 *pLength)
{
    u16 Address; 
    u16 LogCount; 
    u16 LogDataLength; 
    u16 DataLength;

    if ((pData == NULL) || (pLength == NULL))
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_SendGuiPacket NULL data", HAL_GetTick());
        return FALSE;
    }

    if ((Index % ALARM_LOG_COUNT) != 0U){ 
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_SendGuiPacket Index Error = %d", HAL_GetTick(), Index);
        return FALSE; 
    }

    if(Index >= gAlarm.LogIndex){
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_SendGuiPacket Index(%d) >= gAlarm.LogIndex(%d) Error ", HAL_GetTick(), Index, gAlarm.LogIndex);
        return FALSE;
    }

   if (!Alarm_GetLogBlockAddress(Index, &Address)) 
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_SendGuiPacket Alarm_GetLogBlockAddress Error ", HAL_GetTick());
        return FALSE;
    }

    LogCount = gAlarm.LogIndex - Index;
    if (LogCount > ALARM_LOG_COUNT){ 
        LogCount = ALARM_LOG_COUNT; 
    }

    LogDataLength = (gAlarm.LogIndex - Index) * ALARM_LOG_TOTAL_BLOCK;    

    PutU16BE(&pData[ALARM_PACKET_INDEX_OFFSET], Index);

    PutU32BE(&pData[ALARM_PACKET_POWER_ON_OFFSET], gAlarm.PowerOnCount);

    PutU32BE(&pData[ALARM_PACKET_FWD_SD_OFFSET], gAlarm.FwdSDCount);

    PutU32BE(&pData[ALARM_PACKET_REV_SD_OFFSET], gAlarm.RevSDCount);

    PutU32BE(&pData[ALARM_PACKET_OSC_OFFSET], gAlarm.OscCount);

    if (I2C_EE_BufferRead( &pData[ALARM_PACKET_LOG_OFFSET], Address, LogDataLength) != EEPROM_OK)
    {
        DebugPrint("\r\n %d][ALARM][ERR] Alarm_SendGuiPacket I2C_EE_BufferRead Error ", HAL_GetTick());
        return FALSE;
    }

    DataLength = LogDataLength + ALARM_PACKET_LOG_OFFSET;    

    *pLength = DataLength;

    return TRUE ;

}