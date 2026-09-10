/*
 * Osc.h
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#ifndef INC_OSC_H_
#define INC_OSC_H_
#include "common.h"


#define OSC_MAX_COUNT    3

typedef enum
{
    OSC_STATE_IDLE = 0,
    OSC_STATE_CHECK,
    OSC_STATE_FEEDBACK_CHECK,
    OSC_STATE_RECOVERY,
    OSC_STATE_RECHECK,
    OSC_STATE_ISO_CHECK
} OSC_STATEe;

void Osc_Task(void);
void Osc_Init(void);
void Osc_Check(void);
void Osc_SetStatusBit(void);
void Osc_ClearStatusBit(void);
u8 Osc_GetStatusBit(void);
#endif /* INC_OSC_H_ */
