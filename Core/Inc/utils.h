/*
 * utils.h
 *
 *  Created on: 2026. 8. 15.
 *      Author: LG
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "common.h"
#include "main.h"

void Memcpy_BigEndianToU16(u16 *pDst, const u8 *pSrc, u16 DataCount);
void Memcpy_U16ToBigEndian(u8 *pDst, const u16 *pSrc, u16 DataCount);

void PutU16BE(u8 *p, u16 value);
void PutU32BE(u8 *p, u32 value);
u16 GetU16BE(const u8 *p);
u32 GetU32BE(const u8 *p);

#endif /* INC_UTILS_H_ */
