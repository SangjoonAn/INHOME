/*
 * utils.c
 *
 *  Created on: 2026. 8. 15.
 *      Author: LG
 */
#include "utils.h"
#include "project.h"

void Memcpy_BigEndianToU16(u16 *pDst, const u8 *pSrc, u16 DataCount)
{
    u16 i;

    if ((pDst == NULL) || (pSrc == NULL) || (DataCount == 0U))
    {
        return;
    }

    for (i = 0U; i < DataCount; i++)
    {
        pDst[i] =
            ((u16)pSrc[i * 2U] << 8) |
            (u16)pSrc[(i * 2U) + 1U];
    }
}


void Memcpy_U16ToBigEndian(u8 *pDst, const u16 *pSrc, u16 DataCount)
{
    u16 i;

    if ((pDst == NULL) || (pSrc == NULL) || (DataCount == 0U))
    {
        return;
    }

    for (i = 0U; i < DataCount; i++)
    {
        pDst[i * 2U] =
            (u8)(pSrc[i] >> 8);

        pDst[(i * 2U) + 1U] =
            (u8)(pSrc[i] & 0xFFU);
    }
}

void PutU16BE(u8 *p, u16 value)
{
    p[0] = (u8)(value >> 8);
    p[1] = (u8)(value & 0xFF);
}

void PutU32BE(u8 *p, u32 value)
{
    p[0] = (u8)(value >> 24);
    p[1] = (u8)(value >> 16);
    p[2] = (u8)(value >> 8);
    p[3] = (u8)(value & 0xFF);
}

u16 GetU16BE(const u8 *p)
{
    return ((u16)p[0] << 8) |
           ((u16)p[1]);
}

u32 GetU32BE(const u8 *p)
{
    return ((u32)p[0] << 24) |
           ((u32)p[1] << 16) |
           ((u32)p[2] << 8) |
           ((u32)p[3]);
}