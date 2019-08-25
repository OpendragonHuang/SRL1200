#include "crc.h"

void CRC_calcCrc8(uint16_t * crcReg, uint16_t poly, uint16_t u8Data)
{
	uint16_t i;
	uint16_t xorFlag;
	uint16_t bit;
	uint16_t dcdBitMask = 0x80;
	for(i=0; i<8; i++)
	{
		xorFlag = *crcReg & 0x8000;
		*crcReg <<= 1;
		bit = ((u8Data & dcdBitMask) == dcdBitMask);
		*crcReg |= bit;
		if(xorFlag)
		{
			*crcReg = *crcReg ^ poly;
		}
		dcdBitMask >>= 1;	
	}
}


uint16_t CalcCRC(uint8_t *msgbuf,uint8_t msglen)
{
	uint16_t calcCrc = MSG_CRC_INIT;
	uint8_t  i;
	for (i = 0; i < msglen; ++i)
		CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, msgbuf[i]);
	return calcCrc;
}

