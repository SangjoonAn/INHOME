#include "gui.h"
#include "project.h"
#include "bsp_uart.h"
#include "crc.h"
#include "table.h"
#include "utils.h"
#include "Alarm.h"
#include "Down.h"

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
                    if(pDev->MsgRecvStep == OFS_MSB_BODY_LENGTH)
                    {
                        pDev->BodyLen = ((u16)code << 8);
                    }
                    else if(pDev->MsgRecvStep == OFS_LSB_BODY_LENGTH)
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
    u16 Temp;

    Temp = GetU16BE((u8 *)&pLine->SubLen);
    pLine->SubLen = Temp;

    Temp = GetU16BE((u8 *)&pLine->BodyLen);
    pLine->BodyLen = Temp;

    //DebugPrint("\r\n %d][GUI] GUI_ParserMessage BodyLen : %d, RxLen : %d, pLine->SubLen %d", HAL_GetTick(), pLine->BodyLen, RxLen, pLine->SubLen);

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
	else if(pLine->Cmd==CMD_ALARM_LOG_CLR){
		DebugPrint("\r\n %d][GUI] CMD_ALARM_LOG_CLR", HAL_GetTick());
        Gui_ClearAlarmLogMessage(pRxMsg);
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
    MY_CONTROL_t	 *pCtrl;

    pLine = (GUI_BODY_t *)pRxMsg;

    /*
    if(pLine->SubLen != sizeof(MY_CONTROL_t)){
        DebugPrint("\r\n %d][GUI] ControlSet Error", HAL_GetTick());
        return;
    }
    */


    pCtrl = (MY_CONTROL_t *)pLine->SubData;

    //입력값 검증할것.
    // 입력값 범위 설정할것.
    if(pCtrl->Flag1.Data){
        if(pCtrl->Flag1.Bit.TxAlc){
            iMyCtrl.TxAlc = pCtrl->TxAlc;
            SystemDataItemWrite(iMyCtrl.TxAlc);
            iMySts.Flag1.Bit.TxAlc = iMyCtrl.TxAlc;
            DebugPrint("\r\n %d][GUI] TxAlc set to %d", HAL_GetTick(), iMyCtrl.TxAlc);
        }
        if(pCtrl->Flag1.Bit.RxAlc){
            iMyCtrl.RxAlc = pCtrl->RxAlc;
            SystemDataItemWrite(iMyCtrl.RxAlc);
            iMySts.Flag1.Bit.RxAlc = iMyCtrl.RxAlc;
            DebugPrint("\r\n %d][GUI] RxAlc set to %d", HAL_GetTick(), iMyCtrl.RxAlc);
        }
        if(pCtrl->Flag1.Bit.TxShutdown){
            iMyCtrl.TxShutdown = pCtrl->TxShutdown;
            SystemDataItemWrite(iMyCtrl.TxShutdown);
            iMySts.Flag1.Bit.TxShutdown = iMyCtrl.TxShutdown;
            DebugPrint("\r\n %d][GUI] TxShutdown set to %d", HAL_GetTick(), iMyCtrl.TxShutdown);
        }
        if(pCtrl->Flag1.Bit.RxShutdown){
            iMyCtrl.RxShutdown = pCtrl->RxShutdown;
            SystemDataItemWrite(iMyCtrl.RxShutdown);
            iMySts.Flag1.Bit.RxShutdown = iMyCtrl.RxShutdown;
            DebugPrint("\r\n %d][GUI] RxShutdown set to %d", HAL_GetTick(), iMyCtrl.RxShutdown);
        }
        if(pCtrl->Flag1.Bit.IsoCheck){
            iMyCtrl.IsoCheck = pCtrl->IsoCheck;
            SystemDataItemWrite(iMyCtrl.IsoCheck);
            iMySts.Flag2.Bit.IsoCheck = iMyCtrl.IsoCheck;
            DebugPrint("\r\n %d][GUI] IsoCheck set to %d", HAL_GetTick(), iMyCtrl.IsoCheck);
        }
        if(pCtrl->Flag1.Bit.IsoReCheck){
            iMyCtrl.IsoReCheck = pCtrl->IsoReCheck;
            SystemDataItemWrite(iMyCtrl.IsoReCheck);
            iMySts.Flag2.Bit.IsoReCheck = iMyCtrl.IsoReCheck;
            DebugPrint("\r\n %d][GUI] IsoReCheck set to %d", HAL_GetTick(), iMyCtrl.IsoReCheck);
        }
    }

    if(pCtrl->Flag2.Data){
        if(pCtrl->Flag2.Bit.TxPath){
            iMyCtrl.TxPath = pCtrl->TxPath;
            SystemDataItemWrite(iMyCtrl.TxPath);
            iMySts.Flag2.Bit.TxPath = iMyCtrl.TxPath;
            DebugPrint("\r\n %d][GUI] TxPath set to %d", HAL_GetTick(), iMyCtrl.TxPath);
        }
        if(pCtrl->Flag2.Bit.RxPath){
            iMyCtrl.RxPath = pCtrl->RxPath;
            SystemDataItemWrite(iMyCtrl.RxPath);
            iMySts.Flag2.Bit.RxPath = iMyCtrl.RxPath;
            DebugPrint("\r\n %d][GUI] RxPath set to %d", HAL_GetTick(), iMyCtrl.RxPath);
        }
        if(pCtrl->Flag2.Bit.SystemReset){
            if(pCtrl->SystemReset){
                Alarm_LogSave(ALARM_CODE_RESET_USER);            
                Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_MAIN_CTRL, (u8 *)&iMyCtrl, sizeof(MY_CONTROL_t));
                DebugPrint("\r\n %d][GUI] SystemReset ", HAL_GetTick());
                HAL_Delay(500);
                Init_SwReset();
            }

        }
        if(pCtrl->Flag2.Bit.IsoLimitRun){
            iMyCtrl.IsoLimitRun = pCtrl->IsoLimitRun;
            SystemDataItemWrite(iMyCtrl.IsoLimitRun);
            iMySts.Flag2.Bit.IsoLimitRun = iMyCtrl.IsoLimitRun;
            DebugPrint("\r\n %d][GUI] IsoLimitRun set to %d", HAL_GetTick(), iMyCtrl.IsoLimitRun);
        }
    }

    if(pCtrl->Flag3.Data){
        if(pCtrl->Flag3.Bit.TxGainAtt){
            iMyCtrl.TxGainAtt = pCtrl->TxGainAtt;
            SystemDataItemWrite(iMyCtrl.TxGainAtt);
            iMySts.TxGainAtt = iMyCtrl.TxGainAtt;
            DebugPrint("\r\n %d][GUI] TxGainAtt set to %d", HAL_GetTick(), iMyCtrl.TxGainAtt);
        }
        if(pCtrl->Flag3.Bit.RxGainAtt){
            iMyCtrl.RxGainAtt = pCtrl->RxGainAtt;
            SystemDataItemWrite(iMyCtrl.RxGainAtt);
            iMySts.RxGainAtt = iMyCtrl.RxGainAtt;
            DebugPrint("\r\n %d][GUI] RxGainAtt set to %d", HAL_GetTick(), iMyCtrl.RxGainAtt);
        }
        if(pCtrl->Flag3.Bit.TxLinkBalanceAtt){
            iMyCtrl.TxLinkBalanceAtt = pCtrl->TxLinkBalanceAtt;
            SystemDataItemWrite(iMyCtrl.TxLinkBalanceAtt);
            iMySts.TxLinkBalanceAtt = iMyCtrl.TxLinkBalanceAtt;
            DebugPrint("\r\n %d][GUI] TxLinkBalanceAtt set to %d", HAL_GetTick(), iMyCtrl.TxLinkBalanceAtt);
        }
        if(pCtrl->Flag3.Bit.RxLinkBalanceAtt){
            iMyCtrl.RxLinkBalanceAtt = pCtrl->RxLinkBalanceAtt;
            SystemDataItemWrite(iMyCtrl.RxLinkBalanceAtt);
            iMySts.RxLinkBalanceAtt = iMyCtrl.RxLinkBalanceAtt;
            DebugPrint("\r\n %d][GUI] RxLinkBalanceAtt set to %d", HAL_GetTick(), iMyCtrl.RxLinkBalanceAtt);
        }
        if(pCtrl->Flag3.Bit.IsoAtt){
            iMyCtrl.IsoAtt = pCtrl->IsoAtt;
            SystemDataItemWrite(iMyCtrl.IsoAtt);
            iMySts.IsoAtt = iMyCtrl.IsoAtt;
            DebugPrint("\r\n %d][GUI] IsoAtt set to %d", HAL_GetTick(), iMyCtrl.IsoAtt);
        }
    }

    if(pCtrl->Flag4.Data){
        if(pCtrl->Flag4.Bit.TxShutdownLimit){
            iMyCtrl.TxShutdownLimit = pCtrl->TxShutdownLimit;
            SystemDataItemWrite(iMyCtrl.TxShutdownLimit);
            iMySts.TxShutdownLimit = iMyCtrl.TxShutdownLimit;
            DebugPrint("\r\n %d][GUI] TxShutdownLimit set to %d", HAL_GetTick(), iMyCtrl.TxShutdownLimit);
        }
        if(pCtrl->Flag4.Bit.RxShutdownLimit){
            iMyCtrl.RxShutdownLimit = pCtrl->RxShutdownLimit;
            SystemDataItemWrite(iMyCtrl.RxShutdownLimit);
            iMySts.RxShutdownLimit = iMyCtrl.RxShutdownLimit;
            DebugPrint("\r\n %d][GUI] RxShutdownLimit set to %d", HAL_GetTick(), iMyCtrl.RxShutdownLimit);
        }
        if(pCtrl->Flag4.Bit.TxAlcHighLevel){
            iMyCtrl.TxAlcHighLevel = pCtrl->TxAlcHighLevel;
            SystemDataItemWrite(iMyCtrl.TxAlcHighLevel);
            iMySts.TxAlcHighLevel = iMyCtrl.TxAlcHighLevel;
            DebugPrint("\r\n %d][GUI] TxAlcHighLevel set to %d", HAL_GetTick(), iMyCtrl.TxAlcHighLevel);
        }
        if(pCtrl->Flag4.Bit.RxAlcHighLevel){
            iMyCtrl.RxAlcHighLevel = pCtrl->RxAlcHighLevel;
            SystemDataItemWrite(iMyCtrl.RxAlcHighLevel);
            iMySts.RxAlcHighLevel = iMyCtrl.RxAlcHighLevel;
            DebugPrint("\r\n %d][GUI] RxAlcHighLevel set to %d", HAL_GetTick(), iMyCtrl.RxAlcHighLevel);
        }
        if(pCtrl->Flag4.Bit.TxAlcLowOffset){
            iMyCtrl.TxAlcLowOffset = pCtrl->TxAlcLowOffset;
            SystemDataItemWrite(iMyCtrl.TxAlcLowOffset);
            iMySts.TxAlcLowOffset = iMyCtrl.TxAlcLowOffset;
            DebugPrint("\r\n %d][GUI] TxAlcLowOffset set to %d", HAL_GetTick(), iMyCtrl.TxAlcLowOffset);
        }
        if(pCtrl->Flag4.Bit.RxAlcLowOffset){
            iMyCtrl.RxAlcLowOffset = pCtrl->RxAlcLowOffset;
            SystemDataItemWrite(iMyCtrl.RxAlcLowOffset);
            iMySts.RxAlcLowOffset = iMyCtrl.RxAlcLowOffset;
            DebugPrint("\r\n %d][GUI] RxAlcLowOffset set to %d", HAL_GetTick(), iMyCtrl.RxAlcLowOffset);
        }
    }

    if(pCtrl->Flag5.Data){
        if(pCtrl->Flag5.Bit.TxPowerOffsetInput){
            iMyCtrl.TxPowerOffsetInput = pCtrl->TxPowerOffsetInput;
            SystemDataItemWrite(iMyCtrl.TxPowerOffsetInput);
            iMySts.TxPowerOffsetInput = iMyCtrl.TxPowerOffsetInput;
            DebugPrint("\r\n %d][GUI] TxPowerOffsetInput set to %d", HAL_GetTick(), iMyCtrl.TxPowerOffsetInput);
        }
        if(pCtrl->Flag5.Bit.TxPowerOffsetOutput){
            iMyCtrl.TxPowerOffsetOutput = pCtrl->TxPowerOffsetOutput;
            SystemDataItemWrite(iMyCtrl.TxPowerOffsetOutput);
            iMySts.TxPowerOffsetOutput = iMyCtrl.TxPowerOffsetOutput;
            DebugPrint("\r\n %d][GUI] TxPowerOffsetOutput set to %d", HAL_GetTick(), iMyCtrl.TxPowerOffsetOutput);
        }
        if(pCtrl->Flag5.Bit.RxPowerOffsetInput){
            iMyCtrl.RxPowerOffsetInput = pCtrl->RxPowerOffsetInput;
            SystemDataItemWrite(iMyCtrl.RxPowerOffsetInput);
            iMySts.RxPowerOffsetInput = iMyCtrl.RxPowerOffsetInput;
            DebugPrint("\r\n %d][GUI] RxPowerOffsetInput set to %d", HAL_GetTick(), iMyCtrl.RxPowerOffsetInput);
        }
        if(pCtrl->Flag5.Bit.RxPowerOffsetOutput){
            iMyCtrl.RxPowerOffsetOutput = pCtrl->RxPowerOffsetOutput;
            SystemDataItemWrite(iMyCtrl.RxPowerOffsetOutput);
            iMySts.RxPowerOffsetOutput = iMyCtrl.RxPowerOffsetOutput;
            DebugPrint("\r\n %d][GUI] RxPowerOffsetOutput set to %d", HAL_GetTick(), iMyCtrl.RxPowerOffsetOutput);
        }
        if(pCtrl->Flag5.Bit.TemperatureComp){
            iMyCtrl.TemperatureComp = pCtrl->TemperatureComp;
            SystemDataItemWrite(iMyCtrl.TemperatureComp);
            DebugPrint("\r\n %d][GUI] TemperatureComp set to %d", HAL_GetTick(), iMyCtrl.TemperatureComp);
        }
        if(pCtrl->Flag5.Bit.TemperatureOffset){
            iMyCtrl.TemperatureOffset = pCtrl->TemperatureOffset;
            SystemDataItemWrite(iMyCtrl.TemperatureOffset);
            DebugPrint("\r\n %d][GUI] TemperatureOffset set to %d", HAL_GetTick(), iMyCtrl.TemperatureOffset);
        }
        if(pCtrl->Flag5.Bit.TxAttGainOffset){
            iMyCtrl.TxAttGainOffset = pCtrl->TxAttGainOffset;
            SystemDataItemWrite(iMyCtrl.TxAttGainOffset);
            iMySts.TxAttGainOffset = iMyCtrl.TxAttGainOffset;
            DebugPrint("\r\n %d][GUI] TxAttGainOffset set to %d", HAL_GetTick(), iMyCtrl.TxAttGainOffset);
        }
    }

    if(pCtrl->Flag6.Data){
        if(pCtrl->Flag6.Bit.RxAttIsoOffset){
            iMyCtrl.RxAttIsoOffset = pCtrl->RxAttIsoOffset;
            SystemDataItemWrite(iMyCtrl.RxAttIsoOffset);
            iMySts.RxAttIsoOffset = iMyCtrl.RxAttIsoOffset;
            DebugPrint("\r\n %d][GUI] RxAttIsoOffset set to %d", HAL_GetTick(), iMyCtrl.RxAttIsoOffset);
        }
        if(pCtrl->Flag6.Bit.IsoSet){
            iMyCtrl.IsoSet = pCtrl->IsoSet;
            SystemDataItemWrite(iMyCtrl.IsoSet);
            iMySts.IsoSet = iMyCtrl.IsoSet;
            DebugPrint("\r\n %d][GUI] IsoSet set to %d", HAL_GetTick(), iMyCtrl.IsoSet);
        }
        if(pCtrl->Flag6.Bit.OscSet){
            iMyCtrl.OscSet = pCtrl->OscSet;
            SystemDataItemWrite(iMyCtrl.OscSet);
            iMySts.OscSet = iMyCtrl.OscSet;
            DebugPrint("\r\n %d][GUI] OscSet set to %d", HAL_GetTick(), iMyCtrl.OscSet);
        }
        if(pCtrl->Flag6.Bit.TxSdTime){
            iMyCtrl.TxSdTime = pCtrl->TxSdTime;
            SystemDataItemWrite(iMyCtrl.TxSdTime);
            iMySts.TxSdTime = iMyCtrl.TxSdTime;
            DebugPrint("\r\n %d][GUI] TxSdTime set to %d", HAL_GetTick(), iMyCtrl.TxSdTime);
        }
        if(pCtrl->Flag6.Bit.RxSdTime){
            iMyCtrl.RxSdTime = pCtrl->RxSdTime;
            SystemDataItemWrite(iMyCtrl.RxSdTime);
            iMySts.RxSdTime = iMyCtrl.RxSdTime;
            DebugPrint("\r\n %d][GUI] RxSdTime set to %d", HAL_GetTick(), iMyCtrl.RxSdTime);
        }
    }


    if(pCtrl->Flag7.Data){
        if(pCtrl->Flag7.Bit.TxAlcAtt){
            iMyCtrl.TxAlcAtt = pCtrl->TxAlcAtt;
            SystemDataItemWrite(iMyCtrl.TxAlcAtt);
            iMySts.TxAlcAtt = iMyCtrl.TxAlcAtt;
            DebugPrint("\r\n %d][GUI] TxAlcAtt set to %d", HAL_GetTick(), iMyCtrl.TxAlcAtt);
        }
        if(pCtrl->Flag7.Bit.RxAlcAtt){
            iMyCtrl.RxAlcAtt = pCtrl->RxAlcAtt;
            SystemDataItemWrite(iMyCtrl.RxAlcAtt);
            iMySts.RxAlcAtt = iMyCtrl.RxAlcAtt;
            DebugPrint("\r\n %d][GUI] RxAlcAtt set to %d", HAL_GetTick(), iMyCtrl.RxAlcAtt);
        }
        if(pCtrl->Flag7.Bit.OscOnOff){
            iMyCtrl.OscOnOff = pCtrl->OscOnOff;
            SystemDataItemWrite(iMyCtrl.OscOnOff);
            iMySts.OscOnOff = iMyCtrl.OscOnOff;
            DebugPrint("\r\n %d][GUI] OscOnOff set to %d", HAL_GetTick(), iMyCtrl.OscOnOff);
        }
    }

    if(pCtrl->Flag8.Data){
        if(pCtrl->Flag8.Bit.TxSubAtt){
            iMyCtrl.TxSubAtt = pCtrl->TxSubAtt;
            SystemDataItemWrite(iMyCtrl.TxSubAtt);
            iMySts.TxSubAtt = iMyCtrl.TxSubAtt;
            DebugPrint("\r\n %d][GUI] TxSubAtt set to %d", HAL_GetTick(), iMyCtrl.TxSubAtt);
        }
        if(pCtrl->Flag8.Bit.RxSubAtt){
            iMyCtrl.RxSubAtt = pCtrl->RxSubAtt;
            SystemDataItemWrite(iMyCtrl.RxSubAtt);
            iMySts.RxSubAtt = iMyCtrl.RxSubAtt;
            DebugPrint("\r\n %d][GUI] RxSubAtt set to %d", HAL_GetTick(), iMyCtrl.RxSubAtt);
        }
        if(pCtrl->Flag8.Bit.Test_ModeAtt){
            iMyCtrl.Test_ModeAtt = pCtrl->Test_ModeAtt;
            SystemDataItemWrite(iMyCtrl.Test_ModeAtt);
            iMySts.Test_ModeAtt = iMyCtrl.Test_ModeAtt;
            DebugPrint("\r\n %d][GUI] Test_ModeAtt set to %d", HAL_GetTick(), iMyCtrl.Test_ModeAtt);
        }
        if(pCtrl->Flag8.Bit.Test_TxAtt1){
            iMyCtrl.Test_TxAtt1 = pCtrl->Test_TxAtt1;
            SystemDataItemWrite(iMyCtrl.Test_TxAtt1);
            iMySts.Test_TxAtt1 = iMyCtrl.Test_TxAtt1;
            DebugPrint("\r\n %d][GUI] Test_TxAtt1 set to %d", HAL_GetTick(), iMyCtrl.Test_TxAtt1);
        }
        if(pCtrl->Flag8.Bit.Test_RxAtt1){
            iMyCtrl.Test_RxAtt1 = pCtrl->Test_RxAtt1;
            SystemDataItemWrite(iMyCtrl.Test_RxAtt1);
            iMySts.Test_RxAtt1 = iMyCtrl.Test_RxAtt1;
            DebugPrint("\r\n %d][GUI] Test_RxAtt1 set to %d", HAL_GetTick(), iMyCtrl.Test_RxAtt1);
        }
        if(pCtrl->Flag8.Bit.Test_RxAtt2){
            iMyCtrl.Test_RxAtt2 = pCtrl->Test_RxAtt2;
            SystemDataItemWrite(iMyCtrl.Test_RxAtt2);
            iMySts.Test_RxAtt2 = iMyCtrl.Test_RxAtt2;
            DebugPrint("\r\n %d][GUI] Test_RxAtt2 set to %d", HAL_GetTick(), iMyCtrl.Test_RxAtt2);
        }
        if(pCtrl->Flag8.Bit.SysFreq){
            iMyCtrl.SysFreq = pCtrl->SysFreq;
            SystemDataItemWrite(iMyCtrl.SysFreq);
            iMySts.SysFreq = iMyCtrl.SysFreq;
            DebugPrint("\r\n %d][GUI] SysFreq set to %d", HAL_GetTick(), iMyCtrl.SysFreq);
        }
        if(pCtrl->Flag8.Bit.IsoThreshold){
            iMyCtrl.IsoThreshold = pCtrl->IsoThreshold;
            SystemDataItemWrite(iMyCtrl.IsoThreshold);
            iMySts.IsoThreshold = iMyCtrl.IsoThreshold;
            DebugPrint("\r\n %d][GUI] IsoThreshold set to %d", HAL_GetTick(), iMyCtrl.IsoThreshold);
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
    u8 SubData[GUI_TABLE_SUBDATA_SIZE];
    


    if(!Table_LoadTable(pLine->SubData[GUI_TABLE_SUBDATA_INDEX], SubData, &SubDataLength)){
        DebugPrint("\r\n[%d][GUI][ERR] Gui_SendTableStatusMessage() Table_LoadTable Fail", HAL_GetTick());
        return;
    }

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_TABLE_STATUS, SubData, SubDataLength);
}


void Gui_SendTableSaveMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 TableIndex;
    u8 Start;
    u8 Length;
    u8 DataSize;
    u8 SubData[GUI_TABLE_SUBDATA_SIZE];
    
    TableIndex = pLine->SubData[GUI_TABLE_SUBDATA_INDEX];
    Start      = pLine->SubData[GUI_TABLE_SUBDATA_START];
    Length     = pLine->SubData[GUI_TABLE_SUBDATA_LENGTH];
    DataSize   = Table_GetDataSize(TableIndex);

    if(DataSize == DATA_SIZE_2){
        Memcpy_BigEndianToU16( (u16 *)SubData, &pLine->SubData[GUI_TABLE_SUBDATA_DATA], Length);
    }
    else{
        memcpy(SubData, &pLine->SubData[GUI_TABLE_SUBDATA_DATA], Length);
    }

    if(!Table_SaveTable(TableIndex, Start, Length, SubData)){
        DebugPrint("\r\n[%d][GUI][ERR] Gui_SendTableSaveMessage() Table_SaveTable Fail", HAL_GetTick());
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


    if(Alarm_SendGuiPacket(Index, SubData, &SubDataLen) == FALSE){
        DebugPrint("\r\n %d][GUI][ERR] Gui_SendAlarmLogMessage ", HAL_GetTick());
        return;
    }

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_ALARM_LOG_REQ, SubData, SubDataLen);

}

void Gui_ClearAlarmLogMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u16 Index;

    u8 SubData[4];
    u16 SubDataLen;

    memset(SubData, 0, sizeof(SubData));
    SubDataLen = 0;

    Alarm_LogClear();
    

    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_ALARM_LOG_CLR, SubData, SubDataLen);

}


void Gui_SendDownLoadReqMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 SubData[4];
    u16 SubDataLen;
    u16 FrameNum;
    u32 FwId  = FW_UPGRADE_ID;

    SubDataLen = 4;

    SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_ACK;
    SubData[GUI_DOWNLOAD_REQ_CNT_MSB_OFFSET] = pLine->SubData[1];
    SubData[GUI_DOWNLOAD_REQ_CNT_LSB_OFFSET] = pLine->SubData[2];

    FrameNum = pLine->SubData[1] << 8 | pLine->SubData[2];

    /*
    if(Down_DownloadStart() == FALSE){
        DebugPrint("\r\n %d][GUI][ERR] Gui_SendDownLoadReqMessage Down_DownloadStart ERR", HAL_GetTick());
        // GUI Send 추가
        return;
    }
    */

    I2C_EE_BufferWrite((u8 *)&FwId, FW_ID_ADDR, 4);
    I2C_EE_BufferWrite((u8 *)&FrameNum, FW_FRAME_SIZE_ADDR, 2);
    DebugPrint("\r\n %d][GUI] Gui_SendDownLoadReqMessage - FrameNum = %d", HAL_GetTick(), FrameNum);

    Init_SwReset();
   


    //Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_REQ, SubData, SubDataLen);

}



void Gui_SendDownLoadDataMessage(u8 *pRxMsg)
{
	GUI_BODY_t  *pLine;
	pLine = (GUI_BODY_t *)pRxMsg;
    u8 SubData[3];
    u16 SubDataLen;
    u16 FrameNum = 0;
    u16 FrameLength = 0;

    SubDataLen = 3;
    

    FrameNum = pLine->SubData[0] << 8 | pLine->SubData[1];
    FrameLength = pLine->SubLen - CRC_LEN;

    
    if(Down_DownloadData(FrameNum, FrameLength, &pLine->SubData[2]) == FALSE){
        DebugPrint("\r\n %d][GUI][ERR] Gui_SendDownLoadDataMessage - Down_DownloadData Error ", HAL_GetTick());
        SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_NACK;
        SubData[GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET] = pLine->SubData[0];
        SubData[GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET] = pLine->SubData[1];
        Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_DATA, SubData, SubDataLen);
        return;
    }
    
    /*
    // Test
    u8  ReadBuffer[512];
    memcpy(ReadBuffer, (u8 *)&pLine->SubData[2], FrameLength);
    TestCrc = Generate_CRC_Update(TestCrc, ReadBuffer, FrameLength);
    DebugPrint("\r\n %d][GUI] Generate_CRC_Update TestCrc = %d", HAL_GetTick(), TestCrc);
    */


    SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_ACK;
    SubData[GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET] = pLine->SubData[0];
    SubData[GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET] = pLine->SubData[1];

    //DebugPrint("\r\n %d][GUI] Gui_SendDownLoadDataMessage - BodyLen = %d, FrameNum = %d , FrameLength = %d", HAL_GetTick(), pLine->BodyLen ,FrameNum, FrameLength);


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

    if(Down_CheckData(Crc) == FALSE){
        DebugPrint("\r\n %d][GUI][ERR] Gui_SendDownLoadConfirmMessage - Down_CheckData Error ", HAL_GetTick());
        SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_NACK;
        SubData[GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET] = pLine->SubData[0];
        SubData[GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET] = pLine->SubData[1];
        SubData[3] = pLine->SubData[2];
        SubData[4] = pLine->SubData[3];
        Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_CONFIRM, SubData, SubDataLen);
        return;
    }



    // Test
    SubData[GUI_DOWNLOAD_ACK_OFFSET] = GUI_DOWNLOAD_ACK;
    SubData[GUI_DOWNLOAD_DATA_CNT_MSB_OFFSET] = pLine->SubData[0];
    SubData[GUI_DOWNLOAD_DATA_CNT_LSB_OFFSET] = pLine->SubData[1];
    SubData[3] = pLine->SubData[2];
    SubData[4] = pLine->SubData[3];

    DebugPrint("\r\n %d][GUI] Gui_SendDownLoadConfirmMessage - CNT = %d , Crc = %d ", HAL_GetTick(), Cnt , Crc);


    Gui_SendMessage(pLine->SourceID, pLine->DestID, CMD_DOWNLOAD_CONFIRM, SubData, SubDataLen);

    HAL_Delay(200);
    Init_SwReset();

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

    pBuf[index++] = pState->Flag1.Data;
    pBuf[index++] = pState->Flag2.Data;

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

    pBuf[index++] = pState->SystemRunTime[0];
    pBuf[index++] = pState->SystemRunTime[1];
    pBuf[index++] = pState->SystemRunTime[2];
    pBuf[index++] = pState->SystemRunTime[3];
    pBuf[index++] = pState->SystemRunTime[4];

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

    pBuf[index++] = pState->TemperatureComp;

    pBuf[index++] = pState->TemperatureOffset;

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

    pBuf[index++] = pState->UlRfSw;

    pBuf[index++] = (u8)pState->IsoSet;
    pBuf[index++] = (u8)pState->OscSet;

    PutU16BE(&pBuf[index], (u16)pState->TxSdTime); index += 2;
    PutU16BE(&pBuf[index], (u16)pState->RxSdTime); index += 2;
    PutU16BE(&pBuf[index], (u16)pState->UlFbDet); index += 2;
    pBuf[index++] = (u8)pState->SleepReleaseOffset;
    pBuf[index++] = pState->TxAlcAtt;
    pBuf[index++] = pState->RxAlcAtt;
    pBuf[index++] = pState->SubVersion;

    pBuf[index++] = pState->TxSubAtt;
    pBuf[index++] = pState->RxSubAtt;
    pBuf[index++] = pState->Test_ModeAtt;
    pBuf[index++] = pState->Test_TxAtt1;
    pBuf[index++] = pState->Test_RxAtt1;
    pBuf[index++] = pState->Test_RxAtt2;
    pBuf[index++] = pState->SysFreq;
    PutU16BE(&pBuf[index], (u16)pState->IsoThreshold); index += 2;

    memcpy(&pBuf[index], pState->Reserve149, sizeof(pState->Reserve149)); 
    index += sizeof(pState->Reserve149);

    return index;
}
