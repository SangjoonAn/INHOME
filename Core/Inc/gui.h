#ifndef GUI_H
#define GUI_H

#include "bsp_init.h"
#include "bsp_uart.h"
#include "project.h"

#define UART1_FRAME_BUF_SIZE    0x300

#define SYNC_BYTE       0x16

#define SYNC_LEN        4
#define HEADER_LEN      4      // SourceID(1) + Length(2) + DestID(1)
#define CRC_LEN                       2

#define BODY_MIN_LEN    9      // Reserved(3)+Type(1)+RCODE(1)+LEN(2)+CRC(2)
#define END_FLAG        0xF5

#define GUI_FRAME_TIMEOUT_MS    50
#define GUI_BODYLEN_MIN                 (BODY_MIN_LEN)                 // = 9, SubData 없을 때 최소값
#define GUI_CALC_BODYLEN(subLen)        (BODY_MIN_LEN + (subLen))      // 송신 시 Length 필드에 채울 값

#define GUI_BODY_DATA_LEN(bodyLen)    ((bodyLen) - CRC_LEN)          // "BodyLen - 2"
#define GUI_CALC_RXLEN(bodyLen)       (GUI_TOTAL_FRAME_LEN(bodyLen) - CRC_LEN)  // "HEADER_LEN+BodyLen-2"
#define GUI_TOTAL_FRAME_LEN(bodyLen)    (HEADER_LEN + (bodyLen))

// Body  오프셋
#define OFS_SOURCE_ID               0
#define OFS_MSB_BODY_LENGTH         1
#define OFS_LSB_BODY_LENGTH         2
#define OFS_DEST_ID                 3
#define OFS_CMD                     7
#define OFS_RCODE                   8
#define OFS_MSB_SUB_DATA_LENGTH     9
#define OFS_LSB_SUB_DATA_LENGTH     10
// Body 내부 오프셋
#define OFS_RESERVED    0      // 3 byte
#define OFS_CMD         3      // 1 byte (Type)
#define OFS_RCODE       4      // 1 byte
#define OFS_SUBDATA     7      // 가변

// CMD Define
#define CMD_MAIN_STATUS         0x00
#define CMD_MAIN_CTRL          0x01
#define CMD_ALARM_LOG_REQ      0x02
#define CMD_ALARM_LOG_CLR      0x03
#define CMD_TABLE_STATUS       0x04
#define CMD_TABLE_SAVE         0x05
#define CMD_DOWNLOAD_REQ       0x06
#define CMD_DOWNLOAD_DATA      0x08
#define CMD_DOWNLOAD_CONFIRM   0x09
#define CMD_ERR_RESPONSE       0xFF

// Table 관련
#define GUI_TABLE_SUBDATA_SIZE              765
#define GUI_TABLE_SUBDATA_INDEX             0U 
#define GUI_TABLE_SUBDATA_START             1U
#define GUI_TABLE_SUBDATA_LENGTH            2U
#define GUI_TABLE_SUBDATA_DATA              3U
#define GUI_TABLE_SUBDATA_DATA_OFFSET       3U

// Download 관련 
#define GUI_DOWNLOAD_ACK 0x00U 
#define GUI_DOWNLOAD_NACK 0x01U

#define GUI_DOWNLOAD_ACK_OFFSET       0x00
#define GUI_DOWNLOAD_REQ_CNT_MSB_OFFSET   0x02
#define GUI_DOWNLOAD_REQ_CNT_LSB_OFFSET   0x03

#define GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET   0x01
#define GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET   0x02

typedef enum {
    GUI_PROTO_STATE_SYNC = 0,
    GUI_PROTO_STATE_BODY,
    GUI_PROTO_STATE_END
} eGuiProtoRecvState;

#pragma pack(push,1)
typedef struct {
    UART_DEV_t *Uart;
    eGuiProtoRecvState RecvState;

    u8  SyncCnt;
    u16 BodyLen;
    u8  *pMsgBuffer;
    u16 MsgBuffSize;

    u16 MsgRecvStep;
    u32 LastRecvTick;

    u16 RxTimeoutCnt;
    u16 RxLenErrCnt;
    u16 RxSubLenErrCnt;
    u16 RxCrcErrCnt;
} GUI_DEV_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    u8 Start[4];
    u8 SourceID;
    u16 BodyLen;
    u8 DestID;
    u8 Reserved[3];
    u8 Cmd;
    u8 RCode;
    u16 SubLen;
    u16 Crc;
    u8 End;
} GUI_PROTOCOL_HEADER_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    u8 SourceID;
    u16 BodyLen;
    u8 DestID;    
    u8 Reserved[3];
    u8 Cmd;
    u8 RCode;
    u16 SubLen;
    u8 SubData[];
} GUI_BODY_t;
#pragma pack(pop)


void Gui_Init(void);
void Gui_Task(void);
void Gui_SendMessage(u8 destId, u8 srcId, u8 cmd, const u8 *pSubData, u16 subLen);
void GUI_ParserMessage(u8 *pRxMsg, u16 RxLen);
void Gui_ResetRecvState(GUI_DEV_t *pDev);
void Gui_SendStatusMessage(u8 *pData);
void Gui_SendControlMessage(u8 *pData);
void Gui_ControlSet(u8 *pData);
u16 SerializeMyState(u8 *pBuf, const MY_STATE_t *pState);
void Gui_SendTableStatusMessage(u8 *pRxMsg);
void Gui_SendTableSaveMessage(u8 *pRxMsg);
void Gui_SendAlarmLogMessage(u8 *pRxMsg);
void Gui_ClearAlarmLogMessage(u8 *pRxMsg);
void Gui_SendDownLoadReqMessage(u8 *pRxMsg);
void Gui_SendDownLoadDataMessage(u8 *pRxMsg);
void Gui_SendDownLoadConfirmMessage(u8 *pRxMsg);

#endif /* GUI_H */