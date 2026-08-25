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
#include "crc.h"
#include "bsp_eep.h"

u16 FrameSize =0;
u32 TatalDataSize =0;
u16 CurFrameNum;



u8 Down_DownloadStart(void)
{
  uint32_t FirstSector = 0, NbOfSectors = 0, Address = 0;
  uint32_t SectorError = 0;

  /*Variable used for Erase procedure*/
  static FLASH_EraseInitTypeDef EraseInitStruct;

  FrameSize = 0;
  TatalDataSize = 0;
  CurFrameNum = 0;


  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();

  /* Get the 1st sector to erase */
  FirstSector = GetSector(FLASH_DOWNLOAD_START_ADDR);
  /* Get the number of sector to erase from 1st sector*/
  NbOfSectors = GetSector(FLASH_DOWNLOAD_END_ADDR) - FirstSector + 1;

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


  if(FrameNum != CurFrameNum){
    DebugPrint("\r\n %d][GUI][ERR] Down_DownloadData - FrameNum(%d) != CurFrameNum(%d) ERR ", HAL_GetTick(), FrameNum, CurFrameNum);
    return FALSE; 
  }

  TatalDataSize += FrameLength;

  Address = FLASH_DOWNLOAD_START_ADDR + (FrameNum * FrameSize);
  //DebugPrint("\r\n %d][GUI]Down_DownloadData Address = 0x%x ", HAL_GetTick(), Address);


  if ((Address < FLASH_DOWNLOAD_START_ADDR) || ((Address + FrameLength) > (FLASH_DOWNLOAD_END_ADDR + 1U))){
    DebugPrint("\r\n %d][GUI][ERR] Down_DownloadData Address Range ERR", HAL_GetTick());
    return FALSE; 
  }

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

  CurFrameNum++;


  return TRUE;


}


u8 Down_CheckData(u16 RcvCrc)
{
  u32 Address = FLASH_DOWNLOAD_START_ADDR;
  u8  ReadBuffer[128];
  u16 ReadSize;
  u32 DataSize;
  u16 Crc = CRC_16_SEED;
  u32 FwId=FW_UPGRADE_ID;
  
  DataSize = TatalDataSize;


  if(DataSize == 0) {
    DebugPrint("\r\n %d][GUI][ERR] Down_CheckData DataSize NULL ERR", HAL_GetTick());
    return FALSE; 
  }

  while(DataSize){
    if(DataSize >= sizeof(ReadBuffer)){
      ReadSize = sizeof(ReadBuffer);
    }
    else{
      ReadSize = DataSize;
    }

    memcpy(ReadBuffer, (u8 *)Address, ReadSize);
    Crc = Generate_CRC_Update(Crc, ReadBuffer, ReadSize);

    Address += ReadSize;
    DataSize -= ReadSize;
    //watchdog 추가
  }

  
  if(Crc!=RcvCrc){
    DebugPrint("\r\n %d][GUI][ERR] Down_CheckData Crc!=RcvCrc ERR", HAL_GetTick());
    return FALSE;
  }
  
  DebugPrint("\r\n %d][GUI] Down_CheckData CRC OK = 0x%04X, TatalDataSize = %lu", HAL_GetTick(), Crc, TatalDataSize);

  I2C_EE_BufferWrite((u8 *)&FwId, FW_ID_ADDR, 4);
  I2C_EE_BufferWrite((u8 *)&TatalDataSize, FW_SIZE_ADDR, 4);

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