#include "bsp_init.h"
#include "project.h"
#include "common.h"
#include "bsp_eep.h"
#include "bsp_uart.h"

void SystemDataRestore(void)
{
    memset(&iMySts, 0, sizeof(iMySts)); 


    I2C_EE_BufferRead((u8 *)&iMyCtrl, (u16)ExtE2pMapSYSTEMSAVEVAR, sizeof(iMyCtrl));


    iMySts.Manufacture = 0x02;
    iMySts.Version = 0x14; 

    if(iMyCtrl.InitCheckNum!=INITCHECKNUM){
        iMyCtrl.InitCheckNum=INITCHECKNUM;			SystemDataItemWrite(iMyCtrl.InitCheckNum);
    }

    DebugPrint("\r\n %d] InitCheckNum = %x", HAL_GetTick(),iMyCtrl.InitCheckNum );

}