#include "data_buf.h"
#include <string.h>

/**
  * @brief  初始化 DATA BUF。
  * @param  buf: Data_Buf_t 指针。
  */
void DataBuf_Init(Data_Buf_t * buf)
{
	memset(buf->buf, 0, DATA_BUF_SIZE);
	buf->size = 0;
}

/**
  * @brief  清空 DATA BUF。
  * @param  buf: Data_Buf_t 指针。
  */
void DataBuf_Clear(Data_Buf_t * buf)
{
	buf->size = 0;
}

/**
  * @brief  判断 DATA BUF 是否已满。
  * @retval 返回 1 表示 DATA BUF 已满，返回 0 表示 DATA BUF 未满。
  */
int DataBuf_IsFull(Data_Buf_t * buf)
{
	if(buf->size == DATA_BUF_SIZE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void DataBuf_print(Data_Buf_t * buf)
{
	for(uint16_t i = 0; i < buf->size; i++)
	{
		printf("0x%X ", buf->buf[i]);
	}
	printf("\r\n");
}

