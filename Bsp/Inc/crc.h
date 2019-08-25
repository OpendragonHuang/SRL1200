#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus
extern "C" {
#endif

// HAL �����ͷ�ļ�����
#include "stm32f1xx_hal.h"
#include "usart.h"

#define MSG_CRC_INIT		      0xFFFF
#define MSG_CCITT_CRC_POLY		0x1021

// CRC16 ���㺯��
uint16_t CalcCRC(uint8_t *msgbuf,uint8_t msglen);
#ifdef __cplusplus
}
#endif

#endif





