#ifndef BSP_UART_H
#define BSP_UART_H

#include "common.h"
#include "main.h"

#define DebugPrint(...)    BSP_UART4_Print(__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//UART STRUCT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define UARTRX1BUFFSIZE        0x100
#define UARTTX1BUFFSIZE        0x100
#define UART1TXCURMASK	        (UARTTX1BUFFSIZE-1)
#define UART1RXCURMASK	        (UARTRX1BUFFSIZE-1)

#define UARTRX4BUFFSIZE        125
#define UARTTX4BUFFSIZE        125
#define UART4TXCURMASK	        (UARTTX4BUFFSIZE-1)
#define UART4RXCURMASK	        (UARTRX4BUFFSIZE-1)

typedef struct
{
    u8 (*IsExist)(void);
    u8   (*GetByte)(void);
    HAL_StatusTypeDef (*Write)(u8 *, u16);

} UART_DEV_t;

void bsp_uart_init(void);

void BSP_UART1_Init(void);
void BSP_UART4_Init(void);

u8 BSP_UART1_RxDataExist(void);
u8 BSP_UART1_GetChar(void);

void BSP_UART1_GetRxRingBuffer(u8 **buf, u16 **head, u16 **tail, u16 *size);
HAL_StatusTypeDef BSP_UART1_Write(u8 *data, u16 len);
void BSP_UART4_Print(const char *fmt, ...);

#endif