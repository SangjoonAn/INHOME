#ifndef CRC_H
#define CRC_H

#include "common.h"


#define CRC_16_POLYNOMIAL	0X1021
#define CRC_16_SEED			  0X0000

u16 Generate_CRC (u8 *buf_ptr, u16 len);


#endif /* CRC_H */