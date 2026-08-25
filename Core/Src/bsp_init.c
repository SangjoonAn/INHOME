#include "bsp_init.h"
#include "project.h"
#include "common.h"
#include "bsp_eep.h"
#include "bsp_uart.h"
#include "table.h"
#include "Alarm.h"
#include "Down.h"

void Init_DataRestore(void)
{
    DebugPrint("\r\n %d][INIT] Init_DataRestore", HAL_GetTick());
    memset(&iMySts, 0, sizeof(iMySts)); 


    I2C_EE_BufferRead((u8 *)&iMyCtrl, (u16)ExtE2pMapSYSTEMSAVEVAR, sizeof(iMyCtrl));

    iMySts.RptMaker = MAKER_FRTEK;
    iMySts.McuSwVer = MU_FW_VER; 

    iMySts.Flag1.Bit.TxAlc          = iMyCtrl.TxAlc;
    iMySts.Flag1.Bit.RxAlc          = iMyCtrl.RxAlc;
    iMySts.Flag1.Bit.TxShutdown     = iMyCtrl.TxShutdown;
    iMySts.Flag1.Bit.RxShutdown     = iMyCtrl.RxShutdown;

    iMySts.Flag2.Bit.TxPath         = iMyCtrl.TxPath;
    iMySts.Flag2.Bit.RxPath         = iMyCtrl.RxPath;
    iMySts.Flag2.Bit.IsoCheck       = iMyCtrl.IsoCheck;
    iMySts.Flag2.Bit.IsoReCheck     = iMyCtrl.IsoReCheck;
    iMySts.Flag2.Bit.IsoLimitRun    = iMyCtrl.IsoLimitRun;

    iMySts.TxShutdownLimit          = iMyCtrl.TxShutdownLimit;
    iMySts.RxShutdownLimit          = iMyCtrl.RxShutdownLimit;
    iMySts.TxAlcHighLevel           = iMyCtrl.TxAlcHighLevel;
    iMySts.RxAlcHighLevel           = iMyCtrl.RxAlcHighLevel;
    iMySts.TxAlcLowOffset           = iMyCtrl.TxAlcLowOffset;
    iMySts.RxAlcLowOffset           = iMyCtrl.RxAlcLowOffset;

    iMySts.TxGainAtt                = iMyCtrl.TxGainAtt;
    iMySts.RxGainAtt                = iMyCtrl.RxGainAtt;
    iMySts.TxLinkBalanceAtt         = iMyCtrl.TxLinkBalanceAtt;
    iMySts.RxLinkBalanceAtt         = iMyCtrl.RxLinkBalanceAtt;
    iMySts.IsoAtt                   = iMyCtrl.IsoAtt;
    iMySts.OscOnOff                 = iMyCtrl.OscOnOff;

    Table_Init();
    Alarm_Init();

    if(iMyCtrl.InitCheckNum!=INITCHECKNUM){
        iMyCtrl.InitCheckNum=INITCHECKNUM;			SystemDataItemWrite(iMyCtrl.InitCheckNum);
        DebugPrint("\r\n %d][INIT] Factory Set", HAL_GetTick());
        Table_SetFactory();
        if(Alarm_LogClear() == FALSE){
            DebugPrint("\r\n %d][INIT][ERR] Alarm_LogClear ERR", HAL_GetTick());
        }

    }

    DebugPrint("\r\n %d][INIT] InitCheckNum = %x", HAL_GetTick(),iMyCtrl.InitCheckNum );


    

}

void Init_ResetCheck(void)
{
    u32 CheckId;

    I2C_EE_BufferRead((u8 *)&CheckId, FW_ID_ADDR, 4);
    if(CheckId == FW_VALID_ID){
        Alarm_Set(ALARM_BIT_RESET_FW);
        DebugPrint("\r\n %d][INIT] F/W Upgrade Reset ", HAL_GetTick());
        CheckId = 0;
        I2C_EE_BufferWrite((u8 *)&CheckId, FW_ID_ADDR, 4);
    }
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)){
        DebugPrint("\r\n %d][INIT] Power On Reset ", HAL_GetTick());
        Alarm_Set(ALARM_BIT_POWER_ON);
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)){
        DebugPrint("\r\n %d][INIT] External Reset ", HAL_GetTick());
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)){
        DebugPrint("\r\n %d][INIT] Watch Dog Reset ", HAL_GetTick());
    }
}


void Init_SwReset(void)
{
	HAL_NVIC_SystemReset();
}
