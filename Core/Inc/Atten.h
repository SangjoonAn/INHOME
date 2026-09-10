/*
 * Atten.h
 *
 *  Created on: 2026. 8. 26.
 *      Author: LG
 */

#ifndef INC_ATTEN_H_
#define INC_ATTEN_H_

#define RX_ATT1     0
#define RX_ATT2     1
#define TX_ATT1     2
#define ATT_MAX_CNT 3

#define RX_ATT_MIN_NUM 0
#define RX_ATT_MAX_NUM 40
#define TX_ATT_MIN_NUM 0
#define TX_ATT_MAX_NUM 40


#define SUB_ATT_MIN_NUM 0
#define SUB_ATT_MAX_NUM 10

#define ATT_MAX_NUM     127

void Atten_ClockOnOff(u8 Mode);
void Atten_DataOnOff(u8 Mode);
void Atten_ClockDelay(void);
void Atten_SendProc(u8 SerialData8Bit, u8 TargetAtten);
void Atten_RxAtt1_OnOff(u8 Mode);
void Atten_RxAtt2_OnOff(u8 Mode);
void Atten_RxAtt3_OnOff(u8 Mode);
void Atten_TxAtt1_OnOff(u8 Mode);
void Atten_TxAtt2_OnOff(u8 Mode);
void Atten_TxAtt3_OnOff(u8 Mode);
void Atten_SetTxAtt(u8 val);
void Atten_SetRxAtt(u8 TargetAtten, u8 val);

void Atten_TestAtt(u8 TargetAtten, s8 TestAtt);

#endif /* INC_ATTEN_H_ */
