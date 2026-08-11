#ifndef BSP_INIT_H
#define BSP_INIT_H

#include "stm32f2xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define INITCHECKNUM					0x10			//V1030,V1031,V1032


#define SystemDataItemWrite(CurSystemDataItem)							I2C_EE_BufferWrite((u8 *)(&CurSystemDataItem), ((u16)(&CurSystemDataItem)-(u16)(&iMyCtrl))+ExtE2pMapSYSTEMSAVEVAR, sizeof(CurSystemDataItem))

#define ExtE2pMapSYSTEMSAVEVAR							0x0010				//1776byte
#define ExtE2pmapTableStartAddr							0x0700

void OpLedToggle(void);
void SystemDataRestore(void);
#endif