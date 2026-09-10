/*
 * Shutdown.h
 *
 *  Created on: 2026. 8. 31.
 *      Author: LG
 */

#ifndef INC_SHUTDOWN_H_
#define INC_SHUTDOWN_H_

#define SHUTDOWN_STEP_NUM           4
#define SHUTDOWN_RECHECK_NUM        3

#define SHUTDOWN_STATUS_FLAG		0x01
#define SHUTDOWN_FOREVER_FLAG		0x02
#define TX_SHUTDOWN_FLAG			0x04
#define RX_SHUTDOWN_FLAG			0x08

void Shutdown_TxCheck(void);
void Shutdown_RxCheck(void);
void Shutdown_TxClear(void);
void Shutdown_RxClear(void);

#endif /* INC_SHUTDOWN_H_ */
