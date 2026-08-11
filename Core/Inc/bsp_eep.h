/*
 * bsp_eep.h
 *
 *  Created on: 2026. 8. 11.
 *      Author: LG
 */

#ifndef INC_BSP_EEP_H_
#define INC_BSP_EEP_H_

#define EEPROM_I2C_HANDLE              hi2c1



///////////////////////////////////24C64//////////////////////////////
//#define EEPROM_Block0_ADDRESS 0xA0   	
//#define EEPROM_Block1_ADDRESS 0xA2 
//#define EEPROM_Block2_ADDRESS 0xA4 
#define EEPROM_Block2_ADDRESS 0xA0							
//#define EEPROM_Block3_ADDRESS 0xA6	
#define EEPROM_ADDRESS                 EEPROM_Block2_ADDRESS

#define EEPROM_MEMADD_SIZE             I2C_MEMADD_SIZE_16BIT
 
/* EEPROM page size (byte) */
#define I2C_PageSize                   128U
 
/* I2C transaction timeout (ms) */
#define EEPROM_I2C_TIMEOUT             100U
 
/* Maximum time to wait for EEPROM internal write cycle (ms) */
#define EEPROM_WRITE_TIMEOUT           1000U
 
/* ACK polling interval during standby wait (ms) */
#define EEPROM_POLL_DELAY              1U
 
/* Number of HAL_I2C_IsDeviceReady() internal retries.
 *
 * A value of 1 can occasionally report NOT READY due to
 * bus noise; 2~3 gives more margin without meaningfully
 * increasing worst-case latency.
 */
#define EEPROM_IS_READY_TRIALS         3U
 
/* I2C bus recovery */
#define EEPROM_RECOVERY_CLOCKS         9U
#define EEPROM_RECOVERY_DELAY          1U


/* ============================================================
 * GPIO Configuration
 * ============================================================ */
 
#define EEPROM_I2C_GPIO_PORT           GPIOB
 
#define EEPROM_I2C_SCL_PIN             GPIO_PIN_6
#define EEPROM_I2C_SDA_PIN             GPIO_PIN_7
 
#define EEPROM_I2C_GPIO_AF             GPIO_AF4_I2C1
 
 
/* ============================================================
 * EEPROM Status
 * ============================================================ */
 
typedef enum
{
    EEPROM_OK = 0,
 
    EEPROM_ERROR,
    EEPROM_BUSY,
    EEPROM_TIMEOUT,
    EEPROM_BUS_ERROR
 
} EEPROM_Status_t;
 
 
/* ============================================================
 * Public API
 * ============================================================ */
 
/*
 * EEPROM initialization.
 *
 * NOTE:
 * MX_I2C1_Init() must be called BEFORE EepInit().
 */
EEPROM_Status_t EepInit(void);
 
 
/*
 * Check EEPROM device response (single check, no recovery).
 */
EEPROM_Status_t I2C_EE_IsReady(void);
 
 
/*
 * Recover I2C bus using the 9-clock SCL toggle + STOP sequence.
 */
EEPROM_Status_t I2C_BusReset(void);
 
 
/*
 * Wait until EEPROM internal write cycle is completed
 * (ACK polling).
 */
EEPROM_Status_t I2C_EE_WaitEepromStandbyState(void);
 
 
/*
 * Write one EEPROM page.
 *
 * NumByteToWrite must not exceed I2C_PageSize, and the
 * write must not cross a page boundary.
 */
EEPROM_Status_t I2C_EE_PageWrite(uint8_t *pBuffer,
                                  uint16_t WriteAddr,
                                  uint16_t NumByteToWrite);
 
 
/*
 * Write arbitrary length data.
 * Page boundary is automatically handled.
 */
EEPROM_Status_t I2C_EE_BufferWrite(uint8_t *pBuffer,
                                    uint16_t WriteAddr,
                                    uint16_t NumByteToWrite);
 
 
/*
 * Write a single byte.
 */
EEPROM_Status_t I2C_EE_ByteWrite(uint8_t *pBuffer,
                                  uint16_t WriteAddr);
 
 
/*
 * Read arbitrary length data.
 */
EEPROM_Status_t I2C_EE_BufferRead(uint8_t *pBuffer,
                                   uint16_t ReadAddr,
                                   uint16_t NumByteToRead);
 


extern I2C_HandleTypeDef hi2c1;


#endif /* INC_BSP_EEP_H_ */
