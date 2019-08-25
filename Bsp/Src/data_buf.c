#include "data_buf.h"
#include <string.h>

/**
  * @brief  ��ʼ�� DATA BUF��
  * @param  buf: Data_Buf_t ָ�롣
  */
void DataBuf_Init(Data_Buf_t * buf)
{
	memset(buf->buf, 0, DATA_BUF_SIZE);
	buf->size = 0;
}

/**
  * @brief  ��� DATA BUF��
  * @param  buf: Data_Buf_t ָ�롣
  */
void DataBuf_Clear(Data_Buf_t * buf)
{
	buf->size = 0;
}

/**
  * @brief  �ж� DATA BUF �Ƿ�������
  * @retval ���� 1 ��ʾ DATA BUF ���������� 0 ��ʾ DATA BUF δ����
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

