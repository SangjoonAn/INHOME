/*
 * Iso.h
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#ifndef INC_ISO_H_
#define INC_ISO_H_

#include "project.h"

#define RX_MAX_GAIN    50
#define ISO_MAX_ATT      17

#define ISO_MSG_IDLE        0x00    // 미수행
#define ISO_MSG_LIMIT       0x01    // 제한 확보
#define ISO_MSG_FAIL        0x02    // 미확보
#define ISO_MSG_OK          0x03    // 확보
#define ISO_MSG_CHECK       0x04    // ISO 수행중
#define ISO_MSG_LIMIT_RUN   0x05    // 제한확보선택


typedef enum
{
    ISO_STATE_IDLE = 0,
    ISO_STATE_UE_CHECK,
    ISO_STATE_CHECK,
    ISO_STATE_LIMIT,
    ISO_STATE_OK,
    ISO_STATE_FAIL

} ISO_STATEe;


void Iso_Init(void);
void Iso_Task(void);
void Iso_Check(void);

void Iso_SetPwrInitFlag(u8 flag);

void Iso_SetPwrInitFlag(u8 flag);
u8 Iso_GetIsoReCheckFlag(void);
void Iso_SetIsoReCheckFlag(u8 flag);

#endif /* INC_ISO_H_ */
