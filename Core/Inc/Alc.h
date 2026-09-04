/*
 * Alc.h
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#ifndef INC_ALC_H_
#define INC_ALC_H_

#define RX_ALC_ATT_MIN_NUM     0
#define RX_ALC_ATT_MAX_NUM     40

#define TX_ALC_ATT_MIN_NUM     0
#define TX_ALC_ATT_MAX_NUM     40

void Alc_Init(void);
void Alc_Task(void);
void Alc_SetTx(void);
void Alc_SetRx(void);

#endif /* INC_ALC_H_ */
