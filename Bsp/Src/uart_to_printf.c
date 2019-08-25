#include "uart_to_printf.h"
#include <stdio.h>

/**
  * @brief  把 printf 重定向到 UART2。
  * @param  ch: 待发送得字符。
  * @param  stram: 文件指针流。
  * @retval 发送的字符。
  */
int fputc(int ch, FILE * steam)
{
#ifdef __SRL1200_DEBUG__
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
#endif
	return ch;
}




