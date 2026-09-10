/*
 * bsp_debug.h
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#ifndef INC_BSP_DEBUG_H_
#define INC_BSP_DEBUG_H_

#include "project.h"

#define MAX_ARGC  8

void CMD_ShowHelpList(int argc, char *argv[]);
void AlcCmd(int argc, char *argv[]);
u8 ParseCmd(char *buf, char *argv[]);
u8 UsrCmdExc(int argc, char *argv[]);
u8 UsrGetLine(s8 *pBuf, u8 limit);
void Debug_Task(void);


#endif /* INC_BSP_DEBUG_H_ */
