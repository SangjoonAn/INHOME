#ifndef BSP_INIT_H
#define BSP_INIT_H

#include "stm32f2xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define MAKER_FRTEK                     0x04
#define MU_FW_VER    				    0x03



#define INITCHECKNUM					0x01			


#define SystemDataItemWrite(CurSystemDataItem)							I2C_EE_BufferWrite((u8 *)(&CurSystemDataItem), ((u16)(&CurSystemDataItem)-(u16)(&iMyCtrl))+ExtE2pMapSYSTEMSAVEVAR, sizeof(CurSystemDataItem))

#define ExtE2pMapSYSTEMSAVEVAR							0x0010				//1776byte

void bsp_Init(void);
void Led_StatusToggle(void);
void Init_DataRestore(void);
void Init_SwReset(void);
void Init_ResetCheck(void);
void DWT_DelayUs(uint32_t us);

void Init_TxAmpOn(void);
void Init_TxAmpOff(void);
void Init_RxAmpOn(void);
void Init_RxAmpOff(void);
void Init_AllAmpOff(void);
void Init_AllAmpOn(void);
#endif