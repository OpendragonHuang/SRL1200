#include "uart_to_printf.h"
#include <stdio.h>

/**
  * @brief  �� printf �ض��� UART2��
  * @param  ch: �����͵��ַ���
  * @param  stram: �ļ�ָ������
  * @retval ���͵��ַ���
  */
int fputc(int ch, FILE * steam)
{
#ifdef __SRL1200_DEBUG__
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
#endif
	return ch;
}




