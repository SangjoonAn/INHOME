/*
 * Osc.h
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#ifndef INC_OSC_H_
#define INC_OSC_H_

#define OSC_MAX_COUNT    3

typedef enum
{
    OSC_STATE_IDLE = 0,
    OSC_STATE_CHECK,
    OSC_STATE_FEEDBACK_CHECK,
    OSC_STATE_RECOVERY,
    OSC_STATE_RECHECK
} OSC_STATEe;

void Osc_Task(void);
void Osc_Init(void);
void Osc_Check(void);

#endif /* INC_OSC_H_ */
