#include "gui.h"
#include "project.h"
#include "bsp_uart.h"
#include "crc.h"
#include "table.h"
#include "utils.h"
#include "Alarm.h"

GUI_DEV_t GuiProtoDev1, *pGuiProtoDev1;
static UART_DEV_t  uart1_device = {BSP_UART1_RxDataExist,	BSP_UART1_GetChar, BSP_UART1_Write};
static u8 Gui_RxMsgBuffer[UART1_FRAME_BUF_SIZE];
static u8 Gui_TxMsgBuffer[UART1_FRAME_BUF_SIZE];



MY_STATE_t      iMySts;
MY_CONTROL_t    iMyCtrl;


void Gui_Init(void)
{
    pGuiProtoDev1 = &GuiProtoDev1;

    memset(pGuiProtoDev1, 0, sizeof(GUI_DEV_t));

    pGuiProtoDev1->Uart        = &uart1_device;
    pGuiProtoDev1->pMsgBuffer  = Gui_RxMsgBuffer;
    pGuiProtoDev1->RecvState   = GUI_PROTO_STATE_SYNC;
    pGuiProtoDev1->MsgBuffSize = UART1_FRAME_BUF_SIZE; 
    pGuiProtoDev1->LastRecvTick = HAL_GetTick();

}


static void Gui_RecvMessage(GUI_DEV_t *pDev)
{
    u8  code;
    u16 RcvCrc, CalCrc;

    if (pDev->RecvState != GUI_PROTO_STATE_SYNC) 
    {
        if ((HAL_GetTick() - pDev->LastRecvTick) > GUI_FRAME_TIMEOUT_MS)
        {
            pDev->RxTimeoutCnt++;
            Gui_ResetRecvState(pDev);
        }
    }

    while (pDev->Uart->IsExist())
    {
        code = pDev->Uart->GetByte();
        pDev->LastRecvTick = HAL_GetTick();

        switch (pDev->RecvState)
        {
            case GUI_PROTO_STATE_SYNC:
                if (code == SYNC_BYTE)
                {
                    pDev->SyncCnt++;
                    if (pDev->SyncCnt >= SYNC_LEN)
                    {
                        pDev->SyncCnt = 0;
                        pDev->MsgRecvStep = 0;
                        pDev->RecvState = GUI_PROTO_STATE_BODY;
                    }
                }
                else
                {
                    pDev->SyncCnt = 0;
                }
                break;

            case GUI_PROTO_STATE_BODY:
                if(pDev->MsgRecvStep < pDev->MsgBuffSize)
                {
                    if(pDev->MsgRecvStep == 1)
                    {
                        pDev->BodyLen = ((u16)code << 8);
                    }
                    else if(pDev->MsgRecvStep == 2)
                    {
                        pDev->BodyLen |= code;

                        if((pDev->BodyLen < BODY_MIN_LEN) || (GUI_TOTAL_FRAME_LEN(pDev->BodyLen) > pDev->MsgBuffSize))
                        {
                            pDev->RxLenErrCnt++;
                            Gui_ResetRecvState(pDev);
                            break;
                        }
                    }


                    pDev->pMsgBuffer[pDev->MsgRecvStep++] = code;

                    if((pDev->MsgRecvStep > BODY_MIN_LEN) && (pDev->MsgRecvStep >= GUI_TOTAL_FRAME_LEN(pDev->BodyLen)))
                    {
                        pDev->RecvState = GUI_PROTO_STATE_END;
                    }

                }
                else
                {
                    Gui_ResetRecvState(pDev);
                }
                break;

            case GUI_PROTO_STATE_END:
                if (code == END_FLAG)
                {
                    u8  *pBody      = &pDev->pMsgBuffer[HEADER_LEN]; 
                    u16 bodyDataLen = GUI_BODY_DATA_LEN(pDev->BodyLen);

                    CalCrc = Generate_CRC(pBody, bodyDataLen);
                    RcvCrc = ((u16)pBody[bodyDataLen] << 8) | (u16)pBody[bodyDataLen + 1];

                    if (CalCrc == RcvCrc)
                    {
                        GUI_ParserMessage(pDev->pMsgBuffer, GUI_CALC_RXLEN(pDev->BodyLen));
                    }
                    else
                    {
                        pDev->RxCrcErrCnt++;
                    }
                }

                //memset(pDev->pMsgBuffer, 0, pDev->MsgBuffSize);
                Gui_ResetRecvState(pDev);
                break;
            default:
                Gui_ResetRecvState(pDev);
                break;
        }

        if (pDev->MsgRecvStep >= pDev->MsgBuffSize)
        {
            Gui_ResetRecvState(pDev);
        }
    }

}



void Gui_Task(void)
{
    Gui_RecvMessage(pGuiProtoDev1);
}

void Gui_ResetRecvState(GUI_DEV_t *pDev)
{
    pDev->RecvState   = GUI_PROTO_STATE_SYNC;
    pDev->SyncCnt     = 0;
    pDev->MsgRecvStep = 0;
    pDev->BodyLen     = 0;
    pDev->LastRecvTick = HAL_GetTick();
}


void GUI_ParserMessage(u8 *pRxMsg, u16 RxLen)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;

    pLine->BodyLen = GetU16BE(&pRxMsg[2]);
	
	if(pLine->Cmd==CMD_MAIN_STATUS){
		DebugPrint("\r\n %d][GUI] CMD_MAIN_STATUS", HAL_GetTick());
        Gui_SendStatusMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_MAIN_CTRL){
		DebugPrint("\r\n %d][GUI] CMD_MAIN_CTRL", HAL_GetTick());
        Gui_SendControlMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_TABLE_STATUS){
		DebugPrint("\r\n %d][GUI] CMD_TABLE_STATUS", HAL_GetTick());
        Gui_SendTableStatusMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_TABLE_SAVE){
		DebugPrint("\r\n %d][GUI] CMD_TABLE_SAVE", HAL_GetTick());
        Gui_SendTableSaveMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_ALARM_LOG_REQ){
		DebugPrint("\r\n %d][GUI] CMD_ALARM_LOG_REQ", HAL_GetTick());
        Gui_SendAlarmLogMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_DOWNLOAD_REQ){
		DebugPrint("\r\n %d][GUI] CMD_DOWNLOAD_REQ", HAL_GetTick());
        Gui_SendDownLoadReqMessage(pRxMsg);
        return;
    }
	else if(pLine->Cmd==CMD_DOWNLOAD_DATA){
		//DebugPrint("\r\n %d][GUI] CMD_DOWNLOAD_DATA", HAL_GetTick());
        Gui_SendDownLoadDataMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_DOWNLOAD_CONFIRM){
		DebugPrint("\r\n %d][GUI] CMD_DOWNLOAD_CONFIRM", HAL_GetTick());
        Gui_SendDownLoadConfirmMessage(pRxMsg);
        return;
	}		
    else{
        DebugPrint("\r\n %d][GUI] Another CMD", HAL_GetTick());
    }
}

void Gui_SendStatusMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
    u8 subData[sizeof(MY_STATE_t)];
    MY_STATE_t snapshot;
    u16 subLen;

    pLine = (GUI_BODY_t *)pRxMsg;

    snapshot = iMySts;


    subLen = SerializeMyState(subData, &snapshot);
    

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_MAIN_STATUS, subData, subLen);
}

void Gui_SendControlMessage(u8 *pRxMsg)
{
    GUI_BODY_t      *pLine;
    MY_CONTROL_t	 *CtrlPtr;

    pLine = (GUI_BODY_t *)pRxMsg;

    if(pLine->SubLen != sizeof(MY_CONTROL_t)){
        DebugPrint("\r\n %d][GUI] ControlSet Error", HAL_GetTick());
        return;
    }


    CtrlPtr = (MY_CONTROL_t *)pLine->SubData;

    if(CtrlPtr->Control.Flag1.Data){
        if(CtrlPtr->Control.Flag1.Bit.TxAlc){
            iMyCtrl.Control.TxAlc = CtrlPtr->Control.TxAlc;
            DebugPrint("\r\n %d][GUI] TxAlc set to %d", HAL_GetTick(), iMyCtrl.Control.TxAlc);
        }
    }

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_MAIN_CTRL, (u8 *)&iMyCtrl, sizeof(MY_CONTROL_t));
    
        
    DebugPrint("\r\n %d][GUI] ControlSet OK", HAL_GetTick());
}


void Gui_SendMessage(u8 destId, u8 srcId, u8 cmd, const u8 *pSubData, u16 subLen)
{
    u16 bodyLen = BODY_MIN_LEN + subLen;
    u16 Crc, index = 0;

    memset(Gui_TxMsgBuffer, 0, sizeof(Gui_TxMsgBuffer));

    Gui_TxMsgBuffer[index++] = SYNC_BYTE;
    Gui_TxMsgBuffer[index++] = SYNC_BYTE;
    Gui_TxMsgBuffer[index++] = SYNC_BYTE;
    Gui_TxMsgBuffer[index++] = SYNC_BYTE;
 
    Gui_TxMsgBuffer[index++] = srcId;
    Gui_TxMsgBuffer[index++] = (bodyLen >> 8) & 0xFF;   // [FIX] 실제 BodyLen
    Gui_TxMsgBuffer[index++] = bodyLen & 0xFF;
    Gui_TxMsgBuffer[index++] = destId;
 
    Gui_TxMsgBuffer[index++] = 0; // Reserved
    Gui_TxMsgBuffer[index++] = 0; // Reserved
    Gui_TxMsgBuffer[index++] = 0; // Reserved
 
    Gui_TxMsgBuffer[index++] = cmd;
    Gui_TxMsgBuffer[index++] = 0; // RCode
 
    Gui_TxMsgBuffer[index++] = (subLen >> 8) & 0xFF;
    Gui_TxMsgBuffer[index++] = subLen & 0xFF;

    if (SYNC_LEN + HEADER_LEN + bodyLen + 1 > sizeof(Gui_TxMsgBuffer)) return;

    memcpy(&Gui_TxMsgBuffer[index], pSubData, subLen);
    index += subLen;

    Crc = Generate_CRC(&Gui_TxMsgBuffer[SYNC_LEN + HEADER_LEN], GUI_BODY_DATA_LEN(bodyLen));
    Gui_TxMsgBuffer[index++] = (Crc >> 8) & 0xFF; // CRC High Byte
    Gui_TxMsgBuffer[index++] = Crc & 0xFF; // CRC

    Gui_TxMsgBuffer[index++] = END_FLAG;

    pGuiProtoDev1->Uart->Write(Gui_TxMsgBuffer, index);

}


void Gui_SendTableStatusMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u16 SubDataLength;
    u8 SubData[TABLE_SUBDATA_SIZE];
    
    if(!Table_Get(pLine->SubData[TABLE_SUBDATA_INDEX], SubData, &SubDataLength)){
        DebugPrint("\r\n[%d][GUI][ERR] Gui_SendTableStatusMessage() Table_Get Fail", HAL_GetTick());
    }

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_TABLE_STATUS, SubData, SubDataLength);
}

//void Gui_SendTableStatusGui_SendTableStatusMessageMessage2(u8 *pRxMsg)
//{
//    TABLE_INFO_t *pInfo;
//	GUI_BODY_t  *pLine;
//	pLine = (GUI_BODY_t *)pRxMsg;
//    u16 SubDataLength;
//    u8 SubData[TABLE_SUBDATA_SIZE];
//    
//    pInfo = Table_Get(pLine->SubData[TABLE_SUBDATA_INDEX]);
//    if(pInfo == NULL){
//        DebugPrint("\r\n;[%d][ERR] Gui_SendTableStatusMessage pInfo NULL RETURN ", HAL_GetTick());
//        return;
//    }
//
//    SubData[TABLE_SUBDATA_INDEX] = pLine->SubData[TABLE_SUBDATA_INDEX];
//    SubData[TABLE_SUBDATA_START] = pInfo->Start;
//    SubData[TABLE_SUBDATA_LENGTH] = pInfo->Length;   
//
//    // Det Test 
//    //pInfo->Start = 30;
//    //pInfo->Length = 41;   
//    //SubData[TABLE_SUBDATA_START] = pInfo->Start;
//    //SubData[TABLE_SUBDATA_LENGTH] = pInfo->Length;   
//    //memcpy(pInfo->DataPtr, TxDetAdc, sizeof(TxDetAdc));
//    //
//
//    // Att Test 
//    //pInfo->Start = 63;
//    //pInfo->Length = 64;   
//    //SubData[TABLE_SUBDATA_START] = pInfo->Start;
//    //SubData[TABLE_SUBDATA_LENGTH] = pInfo->Length;   
//    //memcpy(pInfo->DataPtr, TxGainAttTableOffset, sizeof(TxGainAttTableOffset));
//    //
//
//    if(pInfo->Length != 0){
//        if(pInfo->DataSize == DATA_SIZE_2){
//            Memcpy_U16ToBigEndian(&SubData[TABLE_SUBDATA_DATA], pInfo->DataPtr, pInfo->Length);
//        }
//        else if(pInfo->DataSize == DATA_SIZE_1){
//            memcpy(&SubData[TABLE_SUBDATA_DATA], pInfo->DataPtr, pInfo->Length);
//        }
//    }
//
//    SubDataLength = (u16)pInfo->Length * pInfo->DataSize + TABLE_SUBDATA_DATA_OFFSET;
//
//    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_TABLE_STATUS, SubData, SubDataLength);
//
//
//}

// Rx Data를 수정하고 있음. 수정할것.
void Gui_SendTableSaveMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 TableIndex;
    u8 Start;
    u8 Length;


    
    TableIndex = pLine->SubData[TABLE_SUBDATA_INDEX];
    Start      = pLine->SubData[TABLE_SUBDATA_START];
    Length     = pLine->SubData[TABLE_SUBDATA_LENGTH];


    if(!Table_Save(TableIndex, Start, Length, &pLine->SubData[TABLE_SUBDATA_DATA])){
        DebugPrint("\r\n[%d][GUI][ERR] Gui_SendTableSaveMessage() Table_Save Fail", HAL_GetTick());
        return;
    }

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_TABLE_SAVE, pLine->SubData, pLine->SubLen);

}

void Gui_SendAlarmLogMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u16 Index;

    u8 SubData[ALARM_PACKET_TOTAL_SIZE];
    u16 SubDataLen;

    memset(SubData, 0, sizeof(SubData));

    Index = pLine->SubData[0] << 8 | pLine->SubData[1];

    /*
     * ==========================================
     * Alarm Log Test Data
     *
     * Log = Time 5 Byte + Code 1 Byte
     *       = 6 Byte
     * ==========================================
     */
    //PutU16BE( &SubData[ALARM_PACKET_INDEX_OFFSET], Index);

    /* Power ON Count */
    //PutU32BE( &SubData[ALARM_PACKET_POWER_ON_OFFSET], 0x00000011UL);

    /* FWD S/D Count */
    //PutU32BE( &SubData[ALARM_PACKET_FWD_SD_OFFSET], 0x00000022UL);

    /* REV S/D Count */
    //PutU32BE( &SubData[ALARM_PACKET_REV_SD_OFFSET], 0x00000033UL);

    /* OSC Count */
    //PutU32BE(&SubData[ALARM_PACKET_OSC_OFFSET], 0x00000044UL);
    /* Log 0 */
    //SubData[ALARM_PACKET_LOG_OFFSET + 0] = 0x26;
    //SubData[ALARM_PACKET_LOG_OFFSET + 1] = 0x08;
    //SubData[ALARM_PACKET_LOG_OFFSET + 2] = 0x19;
    //SubData[ALARM_PACKET_LOG_OFFSET + 3] = 0x10;
    //SubData[ALARM_PACKET_LOG_OFFSET + 4] = 0x30;
    //SubData[ALARM_PACKET_LOG_OFFSET + 5] = 0x21;

    ///* Log 1 */
    //SubData[ALARM_PACKET_LOG_OFFSET + 6] = 0x26;
    //SubData[ALARM_PACKET_LOG_OFFSET + 7] = 0x08;
    //SubData[ALARM_PACKET_LOG_OFFSET + 8] = 0x19;
    //SubData[ALARM_PACKET_LOG_OFFSET + 9] = 0x10;
    //SubData[ALARM_PACKET_LOG_OFFSET + 10] = 0x31;
    //SubData[ALARM_PACKET_LOG_OFFSET + 11] = 0x22;

    ///* Log 2 */
    //SubData[ALARM_PACKET_LOG_OFFSET + 12] = 0x26;
    //SubData[ALARM_PACKET_LOG_OFFSET + 13] = 0x08;
    //SubData[ALARM_PACKET_LOG_OFFSET + 14] = 0x19;
    //SubData[ALARM_PACKET_LOG_OFFSET + 15] = 0x10;
    //SubData[ALARM_PACKET_LOG_OFFSET + 16] = 0x32;
    //SubData[ALARM_PACKET_LOG_OFFSET + 17] = 0x23;

    ///* Log 3 */
    //SubData[ALARM_PACKET_LOG_OFFSET + 18] = 0x26;
    //SubData[ALARM_PACKET_LOG_OFFSET + 19] = 0x08;
    //SubData[ALARM_PACKET_LOG_OFFSET + 20] = 0x19;
    //SubData[ALARM_PACKET_LOG_OFFSET + 21] = 0x10;
    //SubData[ALARM_PACKET_LOG_OFFSET + 22] = 0x33;
    //SubData[ALARM_PACKET_LOG_OFFSET + 23] = 0x24;
    ////

    SubDataLen = ALARM_PACKET_TOTAL_SIZE;


    Alarm_SendGuiPacket(Index, SubData, &SubDataLen);

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_ALARM_LOG_REQ, SubData, SubDataLen);

}

void Gui_SendDownLoadReqMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 SubData[4];
    u16 SubDataLen;

    SubDataLen = 4;

    SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_ACK;
    SubData[GUI_DOWNLOAD_REQ_CNT_MSB_OFFSET] = pLine->SubData[1];
    SubData[GUI_DOWNLOAD_REQ_CNT_LSB_OFFSET] = pLine->SubData[2];

    DebugPrint("\r\n %d][GUI] Gui_SendDownLoadReqMessage", HAL_GetTick());


    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_REQ, SubData, SubDataLen);

}



void Gui_SendDownLoadDataMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 SubData[3];
    u16 SubDataLen;
    u16 Cnt = 0;

    SubDataLen = 3;

    Cnt = pLine->SubData[0] << 8 | pLine->SubData[1];

    SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_ACK;
    SubData[GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET] = pLine->SubData[0];
    SubData[GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET] = pLine->SubData[1];

    DebugPrint("\r\n %d][GUI] Gui_SendDownLoadDataMessage - CNT = %d ", HAL_GetTick(), Cnt);


    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_DATA, SubData, SubDataLen);

}


void Gui_SendDownLoadConfirmMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 SubData[5];
    u16 SubDataLen;
    u16 Cnt = 0;
    u16 Crc = 0;

    SubDataLen = 5;

    Cnt = pLine->SubData[0] << 8 | pLine->SubData[1];
    Crc = pLine->SubData[2] << 8 | pLine->SubData[3];


    // Test
    SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_ACK;
    SubData[GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET] = pLine->SubData[0];
    SubData[GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET] = pLine->SubData[1];
    SubData[3] = pLine->SubData[2];
    SubData[4] = pLine->SubData[3];

    DebugPrint("\r\n %d][GUI] Gui_SendDownLoadConfirmMessage - CNT = %d ", HAL_GetTick(), Cnt);


    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_CONFIRM, SubData, SubDataLen);

}




u16 SerializeMyState(u8 *pBuf, const MY_STATE_t *pState)
{
    u16 index = 0;

    pBuf[index++] = pState->RptMaker;
    pBuf[index++] = pState->McuSwVer;
    pBuf[index++] = pState->SysTemper;

    memcpy(&pBuf[index], pState->Reserve003, sizeof(pState->Reserve003));
    index += sizeof(pState->Reserve003);

    pBuf[index++] = pState->Alarm.Data;

    pBuf[index++] = pState->Reserve006;

    pBuf[index++] = pState->Control1.Data;
    pBuf[index++] = pState->Control2.Data;

    pBuf[index++] = pState->Reserve009;

    PutU16BE(&pBuf[index], (u16)pState->TxInputPower);
    index += 2;

    PutU16BE(&pBuf[index], (u16)pState->TxOutputPower);
    index += 2;

    pBuf[index++] = pState->Reserve014;
    pBuf[index++] = pState->Reserve015;

    PutU16BE(&pBuf[index], (u16)pState->RxOutputPower);
    index += 2;

    pBuf[index++] = pState->TxGain;
    pBuf[index++] = pState->RxGain;

    memcpy(&pBuf[index], pState->Reserve020, sizeof(pState->Reserve020));
    index += sizeof(pState->Reserve020);

    pBuf[index++] = (u8)pState->TxShutdownLimit;
    pBuf[index++] = (u8)pState->RxShutdownLimit;

    pBuf[index++] = (u8)pState->TxAlcHighLevel;
    pBuf[index++] = (u8)pState->RxAlcHighLevel;

    pBuf[index++] = (u8)pState->TxAlcLowOffset;
    pBuf[index++] = (u8)pState->RxAlcLowOffset;

    pBuf[index++] = (u8)pState->SleepModeLimit;

    memcpy(&pBuf[index], pState->Reserve034, sizeof(pState->Reserve034));
    index += sizeof(pState->Reserve034);

    pBuf[index++] = pState->TxGainAtt;
    pBuf[index++] = pState->RxGainAtt;
    pBuf[index++] = pState->TxLinkBalanceAtt;
    pBuf[index++] = pState->RxLinkBalanceAtt;
    pBuf[index++] = pState->IsoAtt;

    pBuf[index++] = pState->TxMaxGain;
    pBuf[index++] = pState->RxMaxGain;
    pBuf[index++] = pState->IsoLevel;
    pBuf[index++] = pState->IsoMsg;

    pBuf[index++] = pState->SystemRunTime[4];
    pBuf[index++] = pState->SystemRunTime[3];
    pBuf[index++] = pState->SystemRunTime[2];
    pBuf[index++] = pState->SystemRunTime[1];
    pBuf[index++] = pState->SystemRunTime[0];

    pBuf[index++] = pState->Stability;

    memcpy(&pBuf[index], pState->Reserve060, sizeof(pState->Reserve060));
    index += sizeof(pState->Reserve060);

    pBuf[index++] = pState->PLLAlarm; 
    pBuf[index++] = pState->OscOnOff; 
    pBuf[index++] = pState->Reserve088;

    PutU16BE(&pBuf[index], (u16)pState->UlfbOffset); index += 2;

    memcpy(&pBuf[index], pState->Reserve091, sizeof(pState->Reserve091));
    index += sizeof(pState->Reserve091);             // 091~092

    pBuf[index++] = pState->OscCheckCount;
    pBuf[index++] = pState->SdBySd;
    pBuf[index++] = pState->SdByIso;
    pBuf[index++] = pState->SdByUe;
    pBuf[index++] = pState->SignalDetect;

    PutU16BE(&pBuf[index], (u16)pState->SleepValue); 
    index += 2;   

    PutU16BE(&pBuf[index], pState->TxDetVoltage);
    index += 2;

    PutU16BE(&pBuf[index], pState->RxDetVoltage);
    index += 2;

    PutU16BE(&pBuf[index], pState->SleepModeVoltage);
    index += 2;

    PutU16BE(&pBuf[index], (u16)pState->SysTemperVoltage);
    index += 2;

    pBuf[index++] = (u8)pState->TxPowerOffsetInput;
    pBuf[index++] = (u8)pState->TxPowerOffsetOutput;
    pBuf[index++] = (u8)pState->RxPowerOffsetInput;
    pBuf[index++] = (u8)pState->RxPowerOffsetOutput;

    pBuf[index++] = pState->TxAttGainOffset;
    pBuf[index++] = (u8)pState->TxAttBalanceOffset;
    pBuf[index++] = (u8)pState->RxAttGainOffset;
    pBuf[index++] = (u8)pState->RxAttBalanceOffset;

    pBuf[index++] = pState->RxAttIsoOffset;

    pBuf[index++] = pState->Reserve119;

    pBuf[index++] = pState->Reserve120;

    pBuf[index++] = (u8)pState->TxAttGainStep;
    pBuf[index++] = (u8)pState->TxAttBalanceStep;

    pBuf[index++] = (u8)pState->RxAttGainStep;
    pBuf[index++] = (u8)pState->RxAttBalanceStep;

    pBuf[index++] = pState->RxAttIsoStep;

    pBuf[index++] = (u8)pState->TxAttGainTemp;
    pBuf[index++] = (u8)pState->TxAttBalanceTemp;
    pBuf[index++] = (u8)pState->RxAttGainTemp;
    pBuf[index++] = (u8)pState->RxAttBalanceTemp;

    pBuf[index++] = (u8)pState->RxAttIsoTemp;

    pBuf[index++] = (u8)pState->TxAttGainTotal;
    pBuf[index++] = (u8)pState->TxAttBalanceTotal;

    pBuf[index++] = (u8)pState->RxAttGainTotal;
    pBuf[index++] = (u8)pState->RxAttBalanceTotal;
    pBuf[index++] = (u8)pState->RxAttIsoTotal;

    pBuf[index++] = pState->UlRfSwOnOff;

    pBuf[index++] = (u8)pState->IsolationSetting;
    pBuf[index++] = (u8)pState->OscSetting;

    PutU16BE(&pBuf[index], (u16)pState->TxSdTTime); index += 2;
    PutU16BE(&pBuf[index], (u16)pState->RxSdTTime); index += 2;
    PutU16BE(&pBuf[index], (u16)pState->UlFbDet); index += 2;
    pBuf[index++] = (u8)pState->SleepReleaseOffset;
    pBuf[index++] = pState->TxAlcAtt;
    pBuf[index++] = pState->RxAlcAtt;
    pBuf[index++] = pState->SubVersion;

    memcpy(&pBuf[index], pState->Reserve149, sizeof(pState->Reserve149)); 
    index += sizeof(pState->Reserve149);

    return index;
}
