#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus
extern "C" {
#endif

// HAL 库基本头文件包括
#include "stm32f1xx_hal.h"
#include "usart.h"

#define MSG_CRC_INIT		      0xFFFF
#define MSG_CCITT_CRC_POLY		0x1021

// CRC16 计算函数
uint16_t CalcCRC(uint8_t *msgbuf,uint8_t msglen);
#ifdef __cplusplus
}
#endif

#endif





