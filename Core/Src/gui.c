#include "gui.h"
#include "project.h"
#include "bsp_uart.h"
#include "crc.h"

GUI_DEV_t GuiProtoDev1, *pGuiProtoDev1;
static UART_DEV_t  uart1_device = {BSP_UART1_RxDataExist,	BSP_UART1_GetChar, BSP_UART1_Write};
static u8 Gui_RxMsgBuffer[UART1_FRAME_BUF_SIZE];
static u8 Gui_TxMsgBuffer[UART1_FRAME_BUF_SIZE];

static u16 GetU16BE(const u8 *p);
static u32 GetU32BE(const u8 *p);

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
	
	if(pLine->Cmd==CMD_MAIN_STATUS_REQ){
		DebugPrint("\r\n %d] CMD_MAIN_STATUS_REQ", HAL_GetTick());
        Gui_SendStatusMessage(pRxMsg);
        return;
	}		
	else if(pLine->Cmd==CMD_MAIN_CTRL){
		DebugPrint("\r\n %d] CMD_MAIN_CTRL", HAL_GetTick());
        Gui_ControlSet(pRxMsg);
        return;
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
    

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_MAIN_STATUS_REQ, subData, subLen);
}

void Gui_ControlSet(u8 *pRxMsg)
{
    GUI_BODY_t      *pLine;
    MY_CONTROL_t	 *CtrlPtr;

    pLine = (GUI_BODY_t *)pRxMsg;

    if(pLine->SubLen != sizeof(MY_CONTROL_t)){
        DebugPrint("\r\n %d] ControlSet Error", HAL_GetTick());
        return;
    }


    CtrlPtr = (MY_CONTROL_t *)pLine->SubData;

    if(CtrlPtr->Control.Flag1.Data){
        if(CtrlPtr->Control.Flag1.Bit.TxAlc){
            iMyCtrl.Control.TxAlc = CtrlPtr->Control.TxAlc;
            DebugPrint("\r\n %d] TxAlc set to %d", HAL_GetTick(), iMyCtrl.Control.TxAlc);
        }
    }

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_MAIN_CTRL, (u8 *)&iMyCtrl, sizeof(MY_CONTROL_t));
    
        
    DebugPrint("\r\n %d] ControlSet OK", HAL_GetTick());
}

void Gui_SendControlMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_MAIN_CTRL, (u8 *)&iMyCtrl, sizeof(MY_CONTROL_t));
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
static void PutU16BE(u8 *p, u16 value)
{
    p[0] = (u8)(value >> 8);
    p[1] = (u8)(value & 0xFF);
}

static void PutU32BE(u8 *p, u32 value)
{
    p[0] = (u8)(value >> 24);
    p[1] = (u8)(value >> 16);
    p[2] = (u8)(value >> 8);
    p[3] = (u8)(value & 0xFF);
}

static u16 GetU16BE(const u8 *p)
{
    return ((u16)p[0] << 8) |
           ((u16)p[1]);
}

static u32 GetU32BE(const u8 *p)
{
    return ((u32)p[0] << 24) |
           ((u32)p[1] << 16) |
           ((u32)p[2] << 8) |
           ((u32)p[3]);
}

u16 SerializeMyState(u8 *pBuf, const MY_STATE_t *pState)
{
    u16 index = 0;

    pBuf[index++] = pState->Manufacture;
    pBuf[index++] = pState->Version;

    memcpy(&pBuf[index], pState->Reserve002, sizeof(pState->Reserve002));
    index += sizeof(pState->Reserve002);

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

    memcpy(&pBuf[index], pState->SystemRunTime, sizeof(pState->SystemRunTime));
    index += sizeof(pState->SystemRunTime);

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

    PutU16BE(&pBuf[index], (u16)pState->SysTemper);
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