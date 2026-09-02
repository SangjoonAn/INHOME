/*
 * bsp_debug.c
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#include "bsp_debug.h"
#include "bsp_uart.h"

typedef struct
{
    const char *command;
    void (*function)(int argc, char *argv[]);
} COMMAND_LIST_t;

COMMAND_LIST_t  help_cmd_list[] = {
{"help",   CMD_ShowHelpList },
{"?",      CMD_ShowHelpList },
{NULL,   NULL}
};

void Debug_Task(void)
{
	s8 cmd_buf[50];
	char *argv[MAX_ARGC];
	u8 argc;

	if(BSP_UART4_RxDataExist()){
		if (UsrGetLine(cmd_buf, sizeof(cmd_buf)) != 0){

			argc = ParseCmd(cmd_buf, argv);
			UsrCmdExc(argc, argv);
		}
        DebugPrint("\r\n<MCU> ");
	}
}


void CMD_ShowHelpList(int argc, char *argv[])
{
	DebugPrint("\r\n help or ? : Show this help message");			
	DebugPrint("\r\n dtu [dls|uls|dlm|ulm] [att] [value]");			
	DebugPrint("\r\n du [dls|uls|dlm|ulm] [att] [value]");			
	DebugPrint("\r\n su [dls|uls|dlm|ulm] [det] ");		
	
}


u8 UsrCmdExc(int argc, char *argv[])
{
    u16 index;

    if(argc == 0 || argv == NULL || argv[0] == NULL){
        return 0;
    }
        

    for(index = 0; help_cmd_list[index].command != NULL; index++)
    {
        if(strcmp(argv[0], help_cmd_list[index].command) == 0)
        {
            help_cmd_list[index].function(argc, argv);
            return 1;
        }
    }

    DebugPrint("\r\nERR: bad command");
    return 0;
}

u8 UsrGetLine(s8 *pBuf, u8 limit)
{
	u8  c;
	u32  i, RcvTime=0;
    u32 StartTick;

    if(pBuf == NULL || limit < 2) return 0;

    StartTick = HAL_GetTick();


    for (i = 0; i < limit - 1; )
    {
        while(!BSP_UART4_RxDataExist())
        {
            /* UART 입력 대기 */
            if((HAL_GetTick() - StartTick) > 5000)
            {
                pBuf[0] = '\0';
                return 0;
            }
        }

        c = BSP_UART4_GetChar();

        switch(c)
        {
            case 0x7F:      /* DEL */
            case 0x08:      /* BS */

                if(i > 0)
                {
                    i--;
                    BSP_UART4_PutChar('\b');
                    BSP_UART4_PutChar(' ');
                    BSP_UART4_PutChar('\b');
                }
                break;
            case 0x0D:      /* CR */
            case 0x0A:      /* LF */
                if(i == 0) return 0;
                pBuf[i] = '\0';
                return i;
            default:
                if(i < (limit - 1))
                {
                    pBuf[i++] = c;
                    BSP_UART4_PutChar(c);
                }
                else
                {
                    /*
                     * Command Buffer Overflow
                     */
                    pBuf[0] = '\0';
                    DebugPrint("\r\nERR: command too long");
                    /*
                     * 현재 입력 라인의 나머지 데이터 제거
                     * CR 또는 LF가 나올 때까지 버림
                     */
                    while(1)
                    {
                        while(!BSP_UART4_RxDataExist())
                        {
                        }
                        c = BSP_UART4_GetChar();

                        if((c == 0x0D) || (c == 0x0A))
                        break;
                    }

                    return 0;
                }

                break;
        }

        StartTick = HAL_GetTick();
    }
}

u8 ParseCmd(char *buf, char *argv[])
{
    u8 argc = 0;

    while (*buf && argc < MAX_ARGC)
    {
        // 공백 스킵
        while (*buf == ' ') buf++;

        if (*buf == '\0') break;

        argv[argc++] = buf;

        // 다음 공백까지 이동
        while (*buf && *buf != ' ') buf++;

        if (*buf)
        {
            *buf = '\0';   // 문자열 분리
            buf++;
        }
    }
    return argc;
}
