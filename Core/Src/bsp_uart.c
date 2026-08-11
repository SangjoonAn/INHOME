#include "common.h"
#include "bsp_uart.h"




// ---- UART1 Part ----
static u8 Uart1RxBuffer[UARTRX1BUFFSIZE];
static u8 Uart1Rxd;
static u16 Uart1RxHead, Uart1RxTail;
static u8  Uart1TxBusy = 0;


void bsp_uart_init(void)
{
    BSP_UART1_Init();


    BSP_UART4_Init();
}

void BSP_UART1_Init(void)
{
    memset(Uart1RxBuffer, 0, sizeof(Uart1RxBuffer));

    Uart1RxHead = 0;
    Uart1RxTail = 0;
    Uart1TxBusy = 0;

    HAL_UART_Receive_IT(&huart1, &Uart1Rxd, 1);
}


void BSP_UART1_RxClear(void)
{
    Uart1RxHead = 0;
    Uart1RxTail = 0;
    memset(Uart1RxBuffer, 0, sizeof(Uart1RxBuffer));
}

u8 BSP_UART1_GetChar(void)
{
	if(Uart1RxTail==Uart1RxHead)	return 0xff;
	else{
		u8 bTemp;
		bTemp = Uart1RxBuffer[Uart1RxTail++];
        Uart1RxTail &= UART1RXCURMASK;
		return bTemp;
	}
}

u8 BSP_UART1_RxDataExist(void)
{
    if(Uart1RxTail==Uart1RxHead)	return 0;
    else return 1;
}

void BSP_UART1_PutChar(u8 data)
{
    HAL_UART_Transmit(&huart1, &data, 1, HAL_MAX_DELAY);
}

void BSP_UART1_PutString(const char *str)
{
    if (str == NULL) return;
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}




void BSP_UART1_GetRxRingBuffer(u8 **buf, u16 **head, u16 **tail, u16 *size)
{
    *buf  = Uart1RxBuffer;
    *head = &Uart1RxHead;
    *tail = &Uart1RxTail;
    *size = UARTRX1BUFFSIZE;
}

HAL_StatusTypeDef BSP_UART1_Write(u8 *data, u16 len)
{
    if(Uart1TxBusy) return HAL_BUSY;

    Uart1TxBusy = 1;

    return HAL_UART_Transmit_IT(&huart1, data, len);
}


// ---- UART4 Part ----
static u8 Uart4RxBuffer[UARTRX4BUFFSIZE];
static u8 Uart4Rxd;
static u8  Uart4TxBusy = 0;

#if(UARTTX4BUFFSIZE > 0x100)
  u16  Uart4TxHead=0, Uart4TxTail=0;
#else
  u8  Uart4TxHead=0, Uart4TxTail=0;
#endif

#if(UARTRX4BUFFSIZE > 0x100)
  u16  Uart4RxHead=0, Uart4RxTail=0;
#else
  u8  Uart4RxHead=0, Uart4RxTail=0;
#endif

void BSP_UART4_Init(void)
{
    memset(Uart4RxBuffer, 0, sizeof(Uart4RxBuffer));

    Uart4RxHead = 0;
    Uart4RxTail = 0;
    Uart4TxBusy = 0;

    HAL_UART_Receive_IT(&huart4, &Uart4Rxd, 1);
}

void BSP_UART4_RxClear(void)
{
    Uart4RxHead = 0;
    Uart4RxTail = 0;
    memset(Uart4RxBuffer, 0, sizeof(Uart4RxBuffer));
}

u8 BSP_UART4_GetChar(void)
{
	if(Uart4RxTail==Uart4RxHead)	return 0xff;
	else{
		u8 bTemp;
		bTemp = Uart4RxBuffer[Uart4RxTail++];
        Uart4RxTail &= UART4RXCURMASK;
		return bTemp;
	}
}

u8 BSP_UART4_RxDataExist(void)
{
    if(Uart4RxTail==Uart4RxHead)	return 0;
    else return 1;
}

void BSP_UART4_PutChar(u8 data)
{
    HAL_UART_Transmit(&huart4, &data, 1, HAL_MAX_DELAY);
}

void BSP_UART4_PutString(const char *str)
{
    if (str == NULL) return;
    HAL_UART_Transmit(&huart4, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}


void BSP_UART4_Print(const char *fmt, ...)
{
    char buf[256];

    va_list args;

    va_start(args, fmt);

    vsnprintf(buf,
              sizeof(buf),
              fmt,
              args);

    va_end(args);

    BSP_UART4_PutString(buf);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Uart1RxBuffer[Uart1RxHead++] = Uart1Rxd;
        
        Uart1RxHead &= UART1RXCURMASK;
        HAL_UART_Receive_IT(&huart1, &Uart1Rxd, 1);
    }
    else if (huart->Instance == UART4)
    {
        Uart4RxBuffer[Uart4RxHead++] = Uart4Rxd;
        Uart4RxHead &= UART4RXCURMASK;
        HAL_UART_Receive_IT(&huart4, &Uart4Rxd, 1);
    }


}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Uart1TxBusy = 0;
    }
}