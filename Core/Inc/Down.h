/*
 * Down.h
 *
 *  Created on: 2026. 8. 20.
 *      Author: LG
 */

#ifndef INC_DOWN_H_
#define INC_DOWN_H_

#include "stm32f2xx_hal.h"
#include "project.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

#define FLASH_APP_START_ADDR        ADDR_FLASH_SECTOR_1   /* Start @ of user Flash area */
#define FLASH_APP_END_ADDR          ADDR_FLASH_SECTOR_6   /* Start @ of user Flash area */

#define FLASH_DOWNLOAD_START_ADDR   ADDR_FLASH_SECTOR_9   /* Start @ of user Flash area */
#define FLASH_DOWNLOAD_END_ADDR     ADDR_FLASH_SECTOR_11  +  GetSectorSize(ADDR_FLASH_SECTOR_11) -1 /* End @ of user Flash area : sector start address + sector size - 1 */


//EEPROM ADDRESS
#define FW_ID_ADDR											0x2F00				//4byte
#define FW_SIZE_ADDR				  						0x2F04				//4byte
#define FW_PATH_ADDR				  						0x2F08				//1byte
#define FW_FRAME_SIZE_ADDR		  							0x2F10				//4byte

#define FW_VALID_ID                     0x7F557F55U
#define FW_UPGRADE_ID                   0x00050401U
#define FW_INVALID_ID                   0x00000000U


uint32_t GetSector(uint32_t Address);
uint32_t GetSectorSize(uint32_t Sector);
u8 Down_DownloadStart(void);
u8 Down_DownloadData(u16 FrameNum, u16 FrameLength, u8 *pData);
u8 Down_CheckData(u16 RcvCrc);
#endif /* INC_DOWN_H_ */
