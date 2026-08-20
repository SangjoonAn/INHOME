#include "bsp_init.h"
#include "project.h"
#include "common.h"
#include "bsp_eep.h"
#include "bsp_uart.h"
#include "table.h"
#include "Alarm.h"

void SystemDataRestore(void)
{
    DebugPrint("\r\n %d][INIT] SystemDataRestore", HAL_GetTick());
    memset(&iMySts, 0, sizeof(iMySts)); 


    I2C_EE_BufferRead((u8 *)&iMyCtrl, (u16)ExtE2pMapSYSTEMSAVEVAR, sizeof(iMyCtrl));

    iMySts.RptMaker = MAKER_FRTEK;
    iMySts.McuSwVer = MU_FW_VER; 

    Table_LoadHeader(0);
    Table_LoadData(0);
    Table_LoadHeader(4);
    Table_LoadData(4);

    if(iMyCtrl.InitCheckNum!=INITCHECKNUM){
        iMyCtrl.InitCheckNum=INITCHECKNUM;			SystemDataItemWrite(iMyCtrl.InitCheckNum);
    }

    DebugPrint("\r\n %d] InitCheckNum = %x", HAL_GetTick(),iMyCtrl.InitCheckNum );



    Alarm_Set(ALARM_BIT_POWER_ON);

}