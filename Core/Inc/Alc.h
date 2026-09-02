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

void Alg_AlcInit(void);
void Alg_AlcTask(void);
void Alg_SetTxAlc(void);
void Alg_SetRxAlc(void);

#endif /* INC_ALC_H_ */
