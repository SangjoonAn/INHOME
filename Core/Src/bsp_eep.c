/*
 * bsp_eep.c
 *
 *  Created on: 2026. 8. 11.
 *      Author: LG
 */


/*
 * bsp_eep.c
 *
 *  Created on: Mar 30, 2023
 *      Author: USER
 */
#include "main.h"
#include "bsp_eep.h"
//#include "project.h"
#include "bsp_uart.h"

/* ============================================================
 * Internal Functions
 * ============================================================ */
 
static void EEPROM_I2C_GPIO_ToGPIO(void);
static void EEPROM_I2C_GPIO_ToAF(void);
 
static uint8_t EEPROM_IsBusReleased(void);
 
static EEPROM_Status_t EEPROM_HalStatusToStatus(
        HAL_StatusTypeDef status);
 
 
/* ============================================================
 * Convert HAL status to EEPROM status
 * ============================================================ */
 
static EEPROM_Status_t EEPROM_HalStatusToStatus(
        HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK:
            return EEPROM_OK;
 
        case HAL_BUSY:
            return EEPROM_BUSY;
 
        case HAL_TIMEOUT:
            return EEPROM_TIMEOUT;
 
        case HAL_ERROR:
        default:
            return EEPROM_ERROR;
    }
}
 
 
/* ============================================================
 * Configure I2C pins as GPIO
 *
 * PB6 = SCL
 * PB7 = SDA
 * ============================================================ */
 
static void EEPROM_I2C_GPIO_ToGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
 
    /*
     * GPIOB clock should normally already be enabled by
     * MX_GPIO_Init(). Enable here as well so that bus
     * recovery is independent from initialization order.
     */
    __HAL_RCC_GPIOB_CLK_ENABLE();
 
    GPIO_InitStruct.Pin = EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN;
 
    /*
     * Open Drain is required for I2C.
     */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
 
    /*
     * External pull-up resistors are assumed to be present
     * on the board. Internal pull-up is left disabled.
     */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
 
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
 
    HAL_GPIO_Init( EEPROM_I2C_GPIO_PORT, &GPIO_InitStruct);
 
    /*
     * Release both lines (open-drain HIGH = released).
     */
    HAL_GPIO_WritePin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN, GPIO_PIN_SET);
}
 
 
/* ============================================================
 * Configure I2C pins back to Alternate Function
 * ============================================================ */
 
static void EEPROM_I2C_GPIO_ToAF(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
 
    __HAL_RCC_GPIOB_CLK_ENABLE();
 
    GPIO_InitStruct.Pin = EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN;
 
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = EEPROM_I2C_GPIO_AF;
 
    HAL_GPIO_Init( EEPROM_I2C_GPIO_PORT, &GPIO_InitStruct);
}
 
 
/* ============================================================
 * Check physical I2C bus state
 *
 * Both SCL and SDA should read HIGH when idle/released.
 * ============================================================ */
 
static uint8_t EEPROM_IsBusReleased(void)
{
    GPIO_PinState scl;
    GPIO_PinState sda;
 
    scl = HAL_GPIO_ReadPin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN);
 
    sda = HAL_GPIO_ReadPin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN);
 
    if ((scl == GPIO_PIN_SET) && (sda == GPIO_PIN_SET))
    {
        return 1U;
    }
 
    return 0U;
}
 
 
/* ============================================================
 * I2C Bus Recovery
 *
 * Sequence:
 *
 * 1. DeInit I2C
 * 2. Change SCL/SDA to GPIO Open Drain
 * 3. Release both lines
 * 4. If bus is still stuck:
 *      Generate up to 9 SCL clocks
 * 5. Generate STOP condition
 * 6. Verify SDA/SCL
 * 7. Restore Alternate Function
 * 8. HAL_I2C_Init()
 *
 * On failure, the GPIO/peripheral state is still restored
 * to Alternate Function + I2C-initialized before returning,
 * so subsequent recovery attempts start from a known state.
 * ============================================================ */
 
EEPROM_Status_t I2C_BusReset(void)
{
    uint32_t i;
 
    DebugPrint("\r\n[I2C] Bus Recovery start");
 
    /*
     * Disable I2C peripheral before manipulating its pins.
     * Continue recovery even if DeInit reports an error -
     * the GPIO pins can still be controlled manually.
     */
    HAL_I2C_DeInit(&EEPROM_I2C_HANDLE);
 
    /*
     * I2C -> GPIO
     */
    EEPROM_I2C_GPIO_ToGPIO();
 
    HAL_Delay(1);
 
    /*
     * If bus is already released, no clock recovery
     * is necessary.
     */
    if (!EEPROM_IsBusReleased())
    {
        DebugPrint("\r\n[I2C] Bus stuck, toggling SCL");
 
        /*
         * Slave may be holding SDA LOW.
         * Generate up to 9 SCL clocks.
         */
        for (i = 0U; i < EEPROM_RECOVERY_CLOCKS; i++)
        {
            /* SCL LOW */
            HAL_GPIO_WritePin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN, GPIO_PIN_RESET);
 
            HAL_Delay(EEPROM_RECOVERY_DELAY);
 
            /* SCL HIGH */
            HAL_GPIO_WritePin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN, GPIO_PIN_SET);
 
            HAL_Delay(EEPROM_RECOVERY_DELAY);
 
            /*
             * If slave released SDA, stop clocking.
             */
            if (HAL_GPIO_ReadPin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN) == GPIO_PIN_SET)
            {
                break;
            }
        }
 
        if (i >= EEPROM_RECOVERY_CLOCKS)
        {
            DebugPrint("\r\n[I2C] SDA still stuck LOW after %d clocks", EEPROM_RECOVERY_CLOCKS);
        }
    }
 
    /*
     * Make sure SCL is HIGH before generating STOP.
     */
    HAL_GPIO_WritePin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN, GPIO_PIN_SET);
 
    HAL_Delay(1);
 
    /*
     * Generate STOP condition:
     *
     * SCL = HIGH
     * SDA LOW -> HIGH
     */
    HAL_GPIO_WritePin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN, GPIO_PIN_RESET);
 
    HAL_Delay(1);
 
    HAL_GPIO_WritePin( EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN, GPIO_PIN_SET);
 
    HAL_Delay(1);
 
    /*
     * Verify physical bus state.
     */
    if (!EEPROM_IsBusReleased())
    {
        DebugPrint("\r\n[I2C] Bus still stuck after recovery");
 
        /*
         * Restore AF + peripheral even on failure, so the
         * next attempt (or normal operation) starts clean.
         */
        EEPROM_I2C_GPIO_ToAF();
        HAL_I2C_Init(&EEPROM_I2C_HANDLE);
 
        return EEPROM_BUS_ERROR;
    }
 
    /*
     * GPIO -> I2C AF
     */
    EEPROM_I2C_GPIO_ToAF();
 
    /*
     * Restore I2C peripheral.
     *
     * HAL_I2C_Init() uses the configuration already set up
     * by MX_I2C1_Init() in hi2c1.Init.
     */
    if (HAL_I2C_Init(&EEPROM_I2C_HANDLE) != HAL_OK)
    {
        DebugPrint("\r\n[I2C] HAL_I2C_Init failed after recovery");
 
        return EEPROM_BUS_ERROR;
    }
 
    DebugPrint("\r\n[I2C] Bus Recovery OK");
 
    return EEPROM_OK;
}
 
 
/* ============================================================
 * Check EEPROM Ready
 *
 * Single check, no bus recovery performed here.
 * ============================================================ */
 
EEPROM_Status_t I2C_EE_IsReady(void)
{
    HAL_StatusTypeDef status;
 
    status = HAL_I2C_IsDeviceReady( &EEPROM_I2C_HANDLE, EEPROM_ADDRESS, EEPROM_IS_READY_TRIALS, EEPROM_I2C_TIMEOUT);
 
    if (status == HAL_OK)
    {
        return EEPROM_OK;
    }
 
    if (status == HAL_BUSY)
    {
        return EEPROM_BUSY;
    }
 
    if (status == HAL_TIMEOUT)
    {
        return EEPROM_TIMEOUT;
    }
 
    return EEPROM_ERROR;
}
 
 
/* ============================================================
 * EEPROM Initialization
 *
 * IMPORTANT:
 * MX_I2C1_Init() must be called BEFORE Eep_Init().
 *
 * Example:
 *
 *     HAL_Init();
 *     SystemClock_Config();
 *     MX_GPIO_Init();
 *     MX_I2C1_Init();
 *
 *     Eep_Init();
 * ============================================================ */
 
EEPROM_Status_t Eep_Init(void)
{
    EEPROM_Status_t status;
 
    DebugPrint("\r\n%d][EEPROM] Init",HAL_GetTick());
 
    /*
     * First check whether EEPROM responds normally.
     */
    status = I2C_EE_IsReady();
 
    if (status == EEPROM_OK)
    {
        DebugPrint("\r\n%d][EEPROM] Device Ready (Addr=0x%02X, Page=%d)",HAL_GetTick() , EEPROM_ADDRESS, I2C_PageSize);
        return EEPROM_OK;
    }
 
    DebugPrint("\r\n%d][EEPROM] Device not ready, trying Bus Recovery",HAL_GetTick() );
 
    /*
     * EEPROM did not respond. Try I2C bus recovery.
     */
    status = I2C_BusReset();
 
    if (status != EEPROM_OK)
    {
        DebugPrint("\r\n%d][EEPROM] Bus Recovery failed",HAL_GetTick() );
 
        return status;
    }
 
    /*
     * Check EEPROM again after recovery.
     */
    status = I2C_EE_IsReady();
 
    if (status == EEPROM_OK)
    {
        DebugPrint("\r\n%d][EEPROM] Init OK after Recovery",HAL_GetTick() );
    }
    else
    {
        DebugPrint("\r\n%d][EEPROM] Device not responding after Recovery",HAL_GetTick() );
    }
 
    return status;
}
 
 
/* ============================================================
 * Wait EEPROM Standby
 *
 * After an EEPROM WRITE, the device performs an internal
 * write cycle during which it normally does NOT ACK its
 * address. Poll until it ACKs again (or timeout).
 * ============================================================ */
 
EEPROM_Status_t I2C_EE_WaitEepromStandbyState(void)
{
    uint32_t tickstart;
    HAL_StatusTypeDef status;
 
    tickstart = HAL_GetTick();
 
    while (1)
    {
        status = HAL_I2C_IsDeviceReady( &EEPROM_I2C_HANDLE, EEPROM_ADDRESS, 1U, EEPROM_I2C_TIMEOUT);
 
        if (status == HAL_OK)
        {
            /*
             * EEPROM internal write cycle completed.
             */
            return EEPROM_OK;
        }
 
        /*
         * Maximum write-cycle timeout.
         */
        if ((HAL_GetTick() - tickstart) >= EEPROM_WRITE_TIMEOUT)
        {
            DebugPrint("\r\n%d][EEPROM] Standby wait timeout",HAL_GetTick() );
 
            return EEPROM_TIMEOUT;
        }
 
        /*
         * Small delay. No RTOS is used, so this is a plain
         * busy-wait with HAL_Delay().
         */
        HAL_Delay(EEPROM_POLL_DELAY);
    }
}
 
 
/* ============================================================
 * Page Write
 *
 * One call must NEVER cross an EEPROM page boundary.
 * NumByteToWrite <= I2C_PageSize.
 * ============================================================ */
 
EEPROM_Status_t I2C_EE_PageWrite( uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
    HAL_StatusTypeDef status;
    uint32_t error;
 
    /*
     * Parameter validation.
     */
    if (pBuffer == NULL)
    {
        return EEPROM_ERROR;
    }
 
    if (NumByteToWrite == 0U)
    {
        return EEPROM_OK;
    }
 
    if (NumByteToWrite > I2C_PageSize)
    {
        DebugPrint("\r\n%d][EEPROM] PageWrite size error (%d > %d)",HAL_GetTick() , NumByteToWrite, I2C_PageSize);
 
        return EEPROM_ERROR;
    }
 
    /*
     * Page boundary check.
     */
    if ((WriteAddr % I2C_PageSize) + NumByteToWrite > I2C_PageSize)
    {
        DebugPrint("\r\n%d][EEPROM] PageWrite boundary error @0x%04X, size=%d",HAL_GetTick() , WriteAddr, NumByteToWrite);
 
        return EEPROM_ERROR;
    }
 
    /*
     * Perform one page write.
     */
    status = HAL_I2C_Mem_Write( &EEPROM_I2C_HANDLE, EEPROM_ADDRESS, WriteAddr, EEPROM_MEMADD_SIZE, pBuffer, NumByteToWrite, EEPROM_I2C_TIMEOUT);
 
    if (status == HAL_OK)
    {
        /*
         * Wait until EEPROM internal write cycle completes.
         */
        return I2C_EE_WaitEepromStandbyState();
    }
 
    /*
     * Read HAL error flags for diagnosis and logging.
     */
    error = HAL_I2C_GetError(&EEPROM_I2C_HANDLE);
 
    DebugPrint("\r\n%d][I2C] PageWrite ERR @0x%04X, HAL=%d, ERR=0x%08lX",HAL_GetTick() , WriteAddr, status, (unsigned long)error);
 
    /*
     * Recover only when the error looks like a physical /
     * transaction-level bus problem. A plain NACK (AF) can
     * be a normal, transient condition, so it is not treated
     * as a bus-stuck situation here.
     */
    if ((status == HAL_BUSY) || (status == HAL_TIMEOUT) || (error & HAL_I2C_ERROR_BERR))
    {
        if (I2C_BusReset() == EEPROM_OK)
        {
            /*
             * Retry the page write once after recovery.
             */
            status = HAL_I2C_Mem_Write( &EEPROM_I2C_HANDLE, EEPROM_ADDRESS, WriteAddr, EEPROM_MEMADD_SIZE, pBuffer, NumByteToWrite, EEPROM_I2C_TIMEOUT);
 
            if (status == HAL_OK)
            {
                return I2C_EE_WaitEepromStandbyState();
            }
 
            error = HAL_I2C_GetError(&EEPROM_I2C_HANDLE);
 
            DebugPrint("\r\n%d][I2C] PageWrite retry failed @0x%04X, HAL=%d, ERR=0x%08lX",HAL_GetTick() , WriteAddr, status, (unsigned long)error);
        }
    }
 
    return EEPROM_HalStatusToStatus(status);
}
 
 
/* ============================================================
 * Buffer Write
 *
 * Automatically handles EEPROM page boundaries.
 *
 * Example (PageSize = 128):
 *
 *   WriteAddr = 100, Length = 100
 *
 *   1st write: 100 ~ 127 (28 bytes)
 *   2nd write: 128 ~ 199 (72 bytes)
 * ============================================================ */
 
EEPROM_Status_t I2C_EE_BufferWrite( uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite )
{
    EEPROM_Status_t status;
 
    uint16_t pageOffset;
    uint16_t bytesToPageEnd;
    uint16_t bytesToWrite;
 
    if (pBuffer == NULL)
    {
        return EEPROM_ERROR;
    }
 
    if (NumByteToWrite == 0U)
    {
        return EEPROM_OK;
    }
 
    while (NumByteToWrite > 0U)
    {
        /*
         * Position inside the current EEPROM page.
         */
        pageOffset = WriteAddr % I2C_PageSize;
 
        /*
         * Remaining bytes until the page boundary.
         */
        bytesToPageEnd = I2C_PageSize - pageOffset;
 
        /*
         * Write only what fits into the current page.
         */
        if (NumByteToWrite < bytesToPageEnd)
        {
            bytesToWrite = NumByteToWrite;
        }
        else
        {
            bytesToWrite = bytesToPageEnd;
        }
 
        status = I2C_EE_PageWrite( pBuffer, WriteAddr, bytesToWrite);
 
        if (status != EEPROM_OK)
        {
            DebugPrint("\r\n%d][ERR][EEPROM] BufferWrite failed @0x%04X",HAL_GetTick() , WriteAddr);
 
            return status;
        }
 
        pBuffer += bytesToWrite;
        WriteAddr += bytesToWrite;
        NumByteToWrite -= bytesToWrite;
    }
 
    return EEPROM_OK;
}
 
 
/* ============================================================
 * Byte Write
 * ============================================================ */
 
EEPROM_Status_t I2C_EE_ByteWrite( uint8_t *pBuffer, uint16_t WriteAddr)
{
    return I2C_EE_PageWrite( pBuffer, WriteAddr, 1U);
}
 
 
/* ============================================================
 * Buffer Read
 *
 * HAL_I2C_Mem_Read() automatically generates:
 *
 *   START
 *   Slave Address + WRITE
 *   Memory Address High
 *   Memory Address Low
 *   RESTART
 *   Slave Address + READ
 *   Data
 *   STOP
 * ============================================================ */
 
EEPROM_Status_t I2C_EE_BufferRead( uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
    HAL_StatusTypeDef status;
    uint32_t error;
 
    if (pBuffer == NULL)
    {
        return EEPROM_ERROR;
    }
 
    if (NumByteToRead == 0U)
    {
        return EEPROM_OK;
    }
 
    status = HAL_I2C_Mem_Read( &EEPROM_I2C_HANDLE, EEPROM_ADDRESS, ReadAddr, EEPROM_MEMADD_SIZE, pBuffer, NumByteToRead, EEPROM_I2C_TIMEOUT);
 
    if (status == HAL_OK)
    {
        return EEPROM_OK;
    }
 
    /*
     * Read HAL error flags for diagnosis and logging.
     */
    error = HAL_I2C_GetError(&EEPROM_I2C_HANDLE);
 
    DebugPrint("\r\n%d][I2C] BufferRead ERR @0x%04X, HAL=%d, ERR=0x%08lX",HAL_GetTick() , ReadAddr, status, (unsigned long)error);
 
    /*
     * Recover physical bus errors only (see PageWrite comment
     * above for rationale).
     */
    if ((status == HAL_BUSY) || (status == HAL_TIMEOUT) || (error & HAL_I2C_ERROR_BERR))
    {
        if (I2C_BusReset() == EEPROM_OK)
        {
            /*
             * Retry once after recovery.
             */
            status = HAL_I2C_Mem_Read( &EEPROM_I2C_HANDLE, EEPROM_ADDRESS, ReadAddr, EEPROM_MEMADD_SIZE, pBuffer, NumByteToRead, EEPROM_I2C_TIMEOUT);
 
            if (status == HAL_OK)
            {
                return EEPROM_OK;
            }
 
            error = HAL_I2C_GetError(&EEPROM_I2C_HANDLE);
 
            DebugPrint("\r\n%d][I2C] BufferRead retry failed @0x%04X, HAL=%d, ERR=0x%08lX",HAL_GetTick() , ReadAddr, status, (unsigned long)error);
        }
    }
 
    return EEPROM_HalStatusToStatus(status);
}
