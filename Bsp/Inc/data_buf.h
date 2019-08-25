#ifndef __DATA_BUF_H
#define __DATA_BUF_H

#ifdef __cplusplus
extern "C" {
#endif

// HAL 库基本头文件包括
#include "stm32f1xx_hal.h"

// Uart2 缓冲区长度
#define DATA_BUF_SIZE 255
// 定义数据类型
typedef struct
{
	uint8_t buf[DATA_BUF_SIZE];
	uint16_t size;
}Data_Buf_t;


// UART BUF 相关操作函数
void DataBuf_Init(Data_Buf_t * buf);
void DataBuf_Clear(Data_Buf_t * buf);
int DataBuf_IsFull(Data_Buf_t * buf);
void DataBuf_print(Data_Buf_t * buf);
#ifdef __cplusplus
}
#endif

#endif

