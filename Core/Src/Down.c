/*
 * Down.c
 *
 *  Created on: 2026. 8. 20.
 *      Author: LG
 */

#include "Down.h"
#include "project.h"
#include "bsp_init.h"
#include "bsp_uart.h"

u8 Down_DownloadStart(void)
{
    uint32_t FirstSector = 0, NbOfSectors = 0, Address = 0;
    uint32_t SectorError = 0;

    /*Variable used for Erase procedure*/
    static FLASH_EraseInitTypeDef EraseInitStruct;

    /* Unlock the Flash to enable the flash control register access *************/ 
    HAL_FLASH_Unlock();

    /* Get the 1st sector to erase */
    FirstSector = GetSector(FLASH_USER_START_ADDR);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = FirstSector;
    EraseInitStruct.NbSectors = NbOfSectors;
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){ 
        DebugPrint("\r\n %d][GUI][ERR] Down_DownloadStart HAL_FLASHEx_Erase ERR", HAL_GetTick());
        HAL_FLASH_Lock();
        return FALSE;
    }

    HAL_FLASH_Lock();
    return TRUE;


}


u8 Down_DownloadData(u16 FrameNum, u16 FrameLength, u8 *pData)
{
    u32 Address;
    u32 WriteData;
    static u16 FrameSize =0;

    if (pData == NULL){ 
        DebugPrint("\r\n %d][GUI][ERR] Down_DownloadData pData NULL ERR", HAL_GetTick());
        return FALSE; 
    }

    if (FrameLength == 0U){
        DebugPrint("\r\n %d][GUI][ERR] Down_DownloadData FrameLength ERR", HAL_GetTick());
        return FALSE; 
    }

    if(FrameNum == 0) {
        FrameSize = FrameLength;
    }

    if (FrameSize == 0U){
        DebugPrint("\r\n %d][GUI][ERR] Down_DownloadData FrameSize NULL ERR", HAL_GetTick());
        return FALSE; 
    }

    Address = FLASH_USER_START_ADDR + (FrameNum * FrameSize);

    HAL_FLASH_Unlock();

    for(u16 i = 0; i < FrameLength; i+=4)
    {
        WriteData = (u32)pData[i] | ( (u32)pData[i+1] << 8) | ((u32)pData[i+2] << 16) | ((u32)pData[i+3] << 24);
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address+i, (uint64_t)WriteData) != HAL_OK)
        {
            DebugPrint("\r\n %d][GUI][ERR] Down_DownloadData HAL_FLASH_Program ERR", HAL_GetTick());
            HAL_FLASH_Lock();
            return FALSE;
        } 

    }


    HAL_FLASH_Lock();
    return TRUE;


}



/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;  
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;  
  }

  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;
  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) ||\
     (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
  {
    sectorsize = 16 * 1024;
  }
  else if((Sector == FLASH_SECTOR_4))
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }  
  return sectorsize;
}