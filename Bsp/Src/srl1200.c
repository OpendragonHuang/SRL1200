#include "srl1200.h"

#include <string.h>
#include <Stdlib.h>

Data_Buf_t txDataPack;
Data_Buf_t rxDataPack;
Data_Buf_t tmpDataPack;
__IO uint8_t SRL1200_isRecvOver = 0;

#define GET_BYTE_VALUE(VAR, BYTE)  *((uint8_t *)(&VAR)+BYTE)

// SRL 1200 内部函数声明
// 引脚初始
static void SRL1200_PinInit(void);
// 数据发送函数
static SRL1200_RecvData_t * _SRL1200_SendData(UART_HandleTypeDef * huart, 
                                      uint8_t command, 
                                      Data_Buf_t * dataBuf, 
                                      uint32_t timeout);
// 原始数据封装和解析函数
static void SRL1200_ParseRecvData(Data_Buf_t * dataBuf, SRL1200_RecvData_t * recvData);
static void SRL1200_EncapSendData(SRL1200_SendData_t * sendData, Data_Buf_t * dataBuf);

// 数据初始化函数
static void SRL1200_SendData_Init(SRL1200_SendData_t * sendData);
static void SRL1200_RecvData_Init(SRL1200_RecvData_t * buf);

static void SRL1200_Log(SRL1200_RecvData_t *recvData);
static void SRL1200_WriteByteData(Data_Buf_t * dest, uint8_t * data, uint8_t len);


/**
  * @brief  初始化 SRL1200 。
  */
void SRL1200_Init(void)
{
	SRL1200_PinInit();
	SRL1200_Enable();
	SRL1200_RST();
	DataBuf_Init(&txDataPack);
	DataBuf_Init(&rxDataPack);
	DataBuf_Init(&tmpDataPack);
}

static void SRL1200_PinInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	SRL1200_EN_CLCK();
	SRL1200_RST_CLCK();
  
  HAL_GPIO_WritePin(SRL1200_EN_GPIO_Port, SRL1200_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SRL1200_RST_GPIO_Port, SRL1200_RST_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = SRL1200_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SRL1200_EN_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SRL1200_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SRL1200_RST_GPIO_Port, &GPIO_InitStruct);
}

void SRL1200_Enable(void)
{
	HAL_GPIO_WritePin(SRL1200_EN_GPIO_Port, SRL1200_EN_Pin, GPIO_PIN_SET);
}

void SRL1200_Disable(void)
{
	HAL_GPIO_WritePin(SRL1200_EN_GPIO_Port, SRL1200_EN_Pin, GPIO_PIN_RESET);
}

void SRL1200_RST(void)
{
	HAL_GPIO_WritePin(SRL1200_RST_GPIO_Port, SRL1200_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(SRL1200_RST_GPIO_Port, SRL1200_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
}

/**
  * @brief  让 SRL1200 从 Boot 层切换到 App 层。
  */
int SRL1200_BootSwitchApp(UART_HandleTypeDef * huart)
{
	int ret = SRL1200_SendData(huart, 0x04, NULL, 1000, NULL, NULL);
	return ret;
}

/**
  * @brief  读取单个标签的信息。
  * @param  huart: 串口句柄。
	* @param  selectOption: 不同的值，需要不同的参数，不需要的参数用 0 填充即可。
	* @param  metaDataFlag: 参考 SRL1200 数据手册。 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData 数据的实际长度  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_ReadTagSingle(UART_HandleTypeDef * huart, 
													uint16_t timeout,
													uint8_t selectOption,
													uint16_t metaDataFlag,
													uint32_t selectAddress,
                          uint8_t  selectDataLen,
													uint8_t * selectData,
													uint8_t len,
													Data_Buf_t * result,
													uint16_t * statusCode)
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = timeout/256;
	tmpDataPack.buf[tmpDataPack.size++] = timeout%256;

	tmpDataPack.buf[tmpDataPack.size++] = selectOption;
	
	if((selectOption & 0x10) != 0)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&metaDataFlag)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&metaDataFlag)+0);
	}
	
	if((selectOption & 0x03) == 0x03)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+3);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+2);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress));
	}
	
	if((selectOption & 0x03) > 0)
	{
		tmpDataPack.buf[tmpDataPack.size++] = selectDataLen;
		memcpy(&tmpDataPack.buf[tmpDataPack.size], selectData, len);
	}
	
	tmpDataPack.size = tmpDataPack.size + len;
	
	return SRL1200_SendData(huart, 0x21, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  多标签存盘命令。
  * @param  huart: 串口句柄。
	* @param  selectOption: 不同的值，需要不同的参数，不需要的参数用 0 填充即可。
	* @param  metaDataFlag: 参考 SRL1200 数据手册。 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData 数据的实际长度  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_ReadTagMultiple(UART_HandleTypeDef * huart, 
														uint8_t selectOption,	
														uint16_t searchFlags,
														uint16_t timeout,
														uint32_t accessPassword,
														uint32_t selectAddress,
														uint8_t  selectDataLen,
														uint8_t * selectData,
														uint8_t len,
														Data_Buf_t * result,
														uint16_t * statusCode)
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = selectOption;
	tmpDataPack.buf[tmpDataPack.size++] = searchFlags/256;
	tmpDataPack.buf[tmpDataPack.size++] = searchFlags%256;
	tmpDataPack.buf[tmpDataPack.size++] = timeout/256;
	tmpDataPack.buf[tmpDataPack.size++] = timeout%256;
	
	if(selectOption != 0x00)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword)+3);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword)+2);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword));
	}
	
	if((selectOption & 0x03) == 0x03)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+3);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+2);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress));
	}
	
	if((selectOption & 0x03) > 0)
	{
		tmpDataPack.buf[tmpDataPack.size++] = selectDataLen;
		memcpy(&tmpDataPack.buf[tmpDataPack.size], selectData, len);
	}
	
	tmpDataPack.size = tmpDataPack.size + len;
	
	return SRL1200_SendData(huart, 0x22, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  更新标签的 EPC。
  * @param  huart: 串口句柄。
	* @param  selectOption: 不同的值，需要不同的参数，不需要的参数用 0 填充即可。
	* @param  metaDataFlag: 参考 SRL1200 数据手册。 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData 数据的实际长度  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_WriteTagEPC(UART_HandleTypeDef * huart, 
												uint16_t timeout,														
												uint8_t selectOption,
												uint8_t RUF,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t len,
												uint8_t * EPCTagID,
												uint8_t EPCTagIDLen,
												Data_Buf_t * result,
												uint16_t * statusCode)
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = timeout/256;
	tmpDataPack.buf[tmpDataPack.size++] = timeout%256;
	tmpDataPack.buf[tmpDataPack.size++] = selectOption;

	if(selectOption == 0x00)
	{
		tmpDataPack.buf[tmpDataPack.size++] = RUF;
	}
	
	if(selectOption != 0x00)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword)+3);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword)+2);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&accessPassword));
	}
	
	if((selectOption & 0x03) == 0x03)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+3);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+2);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&selectAddress));
	}
	
	if((selectOption & 0x03) > 0)
	{
		tmpDataPack.buf[tmpDataPack.size++] = selectDataLen;
		memcpy(&tmpDataPack.buf[tmpDataPack.size], selectData, len);
		tmpDataPack.size = tmpDataPack.size + len;
	}
	
	memcpy(&tmpDataPack.buf[tmpDataPack.size], EPCTagID, EPCTagIDLen);
	tmpDataPack.size = tmpDataPack.size + EPCTagIDLen;
	
	return SRL1200_SendData(huart, 0x23, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  更新标签的 EPC。
  * @param  huart: 串口句柄。
	* @param  selectOption: 不同的值，需要不同的参数，不需要的参数用 0 填充即可。
	* @param  metaDataFlag: 参考 SRL1200 数据手册。 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData 数据的实际长度  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_WriteTagData(UART_HandleTypeDef * huart,
												uint16_t timeout,
												uint8_t option,
												uint32_t writeAddress,														
												uint8_t writeMemBank,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t len,
												uint8_t * writeData,
												uint8_t writeDataLen,
												Data_Buf_t * result,
												uint16_t * statusCode)
{
	DataBuf_Clear(&tmpDataPack);

	SRL1200_WriteByteData(&tmpDataPack, (uint8_t *)&timeout, 2);
	tmpDataPack.buf[tmpDataPack.size++] = option;
	
	SRL1200_WriteByteData(&tmpDataPack, (uint8_t *)&writeAddress, 4);
	tmpDataPack.buf[tmpDataPack.size++] = writeMemBank;
	
	memcpy(&tmpDataPack.buf[tmpDataPack.size], writeData, writeDataLen);
	tmpDataPack.size = tmpDataPack.size + writeDataLen;
	
	return SRL1200_SendData(huart, 0x23, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  设置天线的配置。
  * @param  huart: 串口句柄。
	* @param  option: 不同的值，需要不同的参数，不需要的参数用 0 填充即可。
	* @param  txAntNum: 参考 SRL1200 数据手册。 
	* @param  rxAntNum: 参考 SRL1200 数据手册。
	* @param  readPower: 参考 SRL1200 数据手册。
	* @param  writePower: 参考 SRL1200 数据手册。
	* @param  settingTime: 参考 SRL1200 数据手册。  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetAntennaConfig(UART_HandleTypeDef * huart,
														 uint8_t option,
														 uint8_t txAntNum,
														 uint8_t rxAntNum,
														 uint16_t readPower,
														 uint16_t writePower,
														 uint16_t settingTime,
														 uint16_t timeout,
														 Data_Buf_t * result,
														 uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	
	tmpDataPack.buf[tmpDataPack.size++] = txAntNum;
	if(option == 0x00)
	{
		tmpDataPack.buf[tmpDataPack.size++] = rxAntNum;
	}
	else if(option == 0x03)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&readPower)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&readPower));
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&writePower)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&writePower));
	}
	else if(option == 0x04)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&readPower)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&readPower));
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&writePower)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&writePower));
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&settingTime)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&settingTime));
	}
	
	return SRL1200_SendData(huart, 0x91, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  设置当前标签的协议。
  * @param  huart: 串口句柄。
	* @param  currentProtocol: 目前值只能为 0x0005, 表示设置为 GEN2，18K-6C协议。  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetCurrentTagProtocol(UART_HandleTypeDef * huart,
																	 uint16_t currentProtocol,
																	 uint16_t timeout,
																	 Data_Buf_t * result,
																	 uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&currentProtocol)+1);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&currentProtocol));
	
	return SRL1200_SendData(huart, 0x93, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  设置跳频。
  * @param  huart: 串口句柄。
	* @param  freq1: 1 的频率值。
	* @param  freq2: 2 的频率值。
	* @param  freq3: 3 的频率值。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetFrequencyHopping(UART_HandleTypeDef * huart,
													      uint32_t freq1,
															  uint32_t freq2,
															  uint32_t freq3,
														    uint16_t timeout,
													      Data_Buf_t * result,
													      uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq1)+3);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq1)+2);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq1)+1);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq1));
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq2)+3);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq2)+2);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq2)+1);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq2));
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq3)+3);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq3)+2);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq3)+1);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&freq3));
	return SRL1200_SendData(huart, 0x95, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  设置 GPIO 的输出值。
  * @param  huart: 串口句柄。
	* @param  GPIO1: 为 0x01 表示设置 GPIO 1，其他值不设置。
	* @param  GPIO1OutputValue: 0x01 输出高电平，0x00 输出低电平。
	* @param  GPIO2: 为 0x02 表示设置 GPIO 2，其他值不设置
	* @param  GPIO2OutputValue: 0x01 输出高电平，0x00 输出低电平。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetGPIOOutputValue(UART_HandleTypeDef * huart,
															 uint16_t GPIO1,
															 uint16_t GPIO1OutputValue,
															 uint16_t GPIO2,
															 uint16_t GPIO2OutputValue,
															 uint16_t timeout,
															 Data_Buf_t * result,
															 uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	
	if(GPIO1 == 0x01)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO1)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO1));
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO1OutputValue)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO1OutputValue));
	}
	if(GPIO1 == 0x01)
	{
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO2)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO2));
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO2OutputValue)+1);
		tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&GPIO2OutputValue));
	}
	return SRL1200_SendData(huart, 0x96, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  设置当前的频率区域。
  * @param  huart: 串口句柄。
	* @param  code: 参考 SRL1200 数据手册。  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetCurrentFrequencyRegion(UART_HandleTypeDef * huart,
																			uint16_t code,
																			uint16_t timeout,
																			Data_Buf_t * result,
																			uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&code)+1);
	tmpDataPack.buf[tmpDataPack.size++] = *((uint8_t *)(&code));
	
	return SRL1200_SendData(huart, 0x97, &tmpDataPack, timeout, result, statusCode);

}

/**
  * @brief  设置功率模式。
  * @param  huart: 串口句柄。
	* @param  powerMode: 参考 SRL1200 数据手册。  
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetPowerMode(UART_HandleTypeDef * huart,
												 uint8_t powerMode,
												 uint16_t timeout,
												 Data_Buf_t * result,
												 uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = powerMode;
	
	return SRL1200_SendData(huart, 0x98, &tmpDataPack, timeout, result, statusCode);

}

/**
  * @brief  设置阅读器配置。
  * @param  huart: 串口句柄。
	* @param  option: 参考 SRL1200 数据手册。
	* @param  key: 参考 SRL1200 数据手册。
	* @param  value: 参考 SRL1200 数据手册。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetCardReaderConfig(UART_HandleTypeDef * huart,
																uint8_t option,
																uint8_t key,
																uint8_t value,
																uint16_t timeout,
																Data_Buf_t * result,
																uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = option;
	tmpDataPack.buf[tmpDataPack.size++] = key;
	tmpDataPack.buf[tmpDataPack.size++] = value;
	
	return SRL1200_SendData(huart, 0x9A, &tmpDataPack, timeout, result, statusCode);

}


/**
  * @brief  设置协议配置。
  * @param  huart: 串口句柄。
	* @param  protocolValue: 参考 SRL1200 数据手册。
	* @param  parameter: 参考 SRL1200 数据手册。
	* @param  option: 参考 SRL1200 数据手册。
	* @param  value: 参考 SRL1200 数据手册。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SetProtocolConfig(UART_HandleTypeDef * huart,
															uint8_t protocolValue,
															uint8_t parameter,
															uint8_t option,
															uint8_t value,
															uint16_t timeout,
															Data_Buf_t * result,
															uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = protocolValue;
	tmpDataPack.buf[tmpDataPack.size++] = parameter;
	if(parameter == 0x00 || parameter == 0x02)
	{
		tmpDataPack.buf[tmpDataPack.size++] = option;
	}
	tmpDataPack.buf[tmpDataPack.size++] = value;
	
	return SRL1200_SendData(huart, 0x9B, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取天线的配置信息。
  * @param  huart: 串口句柄。
	* @param  option: 选项，可以为 0x01 ~ 0x05。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetAntennaConfig(UART_HandleTypeDef * huart,
													   uint8_t option,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = option;

	return SRL1200_SendData(huart, 0x61, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取读发射功率信息。
  * @param  huart: 串口句柄。
	* @param  option: 选项，可只能为 0x00 和 0x01，但效果一样。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetReadTransmitPowerInfo(UART_HandleTypeDef * huart,
													   uint8_t option,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = option;

	return SRL1200_SendData(huart, 0x62, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取当前工作标签的协议。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetCurrentTagProtocol(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x63, NULL, timeout, result, statusCode);
}

/**
  * @brief  获取写发射功率信息。
  * @param  huart: 串口句柄。
	* @param  option: 选项，可只能为 0x00 和 0x01，但效果一样。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetWriteTransmitPowerInfo(UART_HandleTypeDef * huart,
													   uint8_t option,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = option;

	return SRL1200_SendData(huart, 0x64, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取跳频表。
  * @param  huart: 串口句柄。
	* @param  option: 选项, 0x00 表示获取跳频表，0x01 获取跳频间隔。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetFrequencyHoppingTable(UART_HandleTypeDef * huart,
													   uint8_t option,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	if(option == 0x01)
	{
			tmpDataPack.buf[tmpDataPack.size++] = option;
	}
	
	return SRL1200_SendData(huart, 0x65, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取当前 GPIO 的输入值。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetGIPIOInputValue(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x66, NULL, timeout, result, statusCode);
}

/**
  * @brief  获取当前频率的区域。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetCurrentFrequencyRegion(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x67, NULL, timeout, result, statusCode);
}

/**
  * @brief  获取功率的模式。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetPowerMode(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x68, NULL, timeout, result, statusCode);
}

/**
  * @brief  获取读写器配置信息。
  * @param  huart: 串口句柄。
	* @param  option: 与 0x9A 设置的值一致。
	* @param  key: 与 0x9A 设置的值一致
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetCardReaderConfig(UART_HandleTypeDef * huart,
													   uint8_t option,
                             uint8_t key,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = option;
	tmpDataPack.buf[tmpDataPack.size++] = key;

	return SRL1200_SendData(huart, 0x6A, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取协议配置信息。
  * @param  huart: 串口句柄。
	* @param  procotolValue: 与 0x9B 命令一致。
	* @param  parameter: 与 0x9B 命令一致
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_ProcotolConfig(UART_HandleTypeDef * huart,
													   uint8_t procotolValue,
                             uint8_t parameter,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	DataBuf_Clear(&tmpDataPack);
	tmpDataPack.buf[tmpDataPack.size++] = procotolValue;
	tmpDataPack.buf[tmpDataPack.size++] = parameter;

	return SRL1200_SendData(huart, 0x6B, &tmpDataPack, timeout, result, statusCode);
}

/**
  * @brief  获取可用的标签协议。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetCanUseTagProcotol(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x70, NULL, timeout, result, statusCode);
}

/**
  * @brief  获取可用的频率区域。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetCanUseFrequencyRegion(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x71, NULL, timeout, result, statusCode);
}

/**
  * @brief  获取芯片当前的温度。
  * @param  huart: 串口句柄。
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_GetCurentTemperature(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x72, NULL, timeout, result, statusCode);
}



/**
  * @brief  向 SRL1200 发送数据包。
  * @param  huart: 串口句柄。
	* @param  command: 命令。
	* @param  dataBuf: 数据缓冲区
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
	* @param  result：保存模块回复的数据。
  * @param  statusCode：，模块回复的状态码。
	* @retval 返回 0 表示接收模块数据成功，返回 -1 表示接收失败。
  */
int SRL1200_SendData(UART_HandleTypeDef * huart, 
										 uint8_t command, 
										 Data_Buf_t * dataBuf, 
										 uint32_t timeout,
										 Data_Buf_t * result,
										 uint16_t * statusCode)
{
	SRL1200_RecvData_t * recvData = _SRL1200_SendData(huart, command, dataBuf, timeout);
	
	int ret = -1;
	if(recvData != NULL)
	{
		// 输出接收到的数据
		SRL1200_Log(recvData);
		if(recvData->command == command)
		{
			if(statusCode != NULL)
			{
				 *statusCode = recvData->status; 
			}
			if(recvData->status == 0x0000)
			{
				ret = 0;
			}
			if(recvData->dataLength > 0 && result != NULL)
			{
				memcpy(result->buf,recvData->dataBuf, recvData->dataLength);
				result->size =  recvData->dataLength;
			}
		}
		free(recvData->dataBuf);
		free(recvData);
	}
	return ret;
}

/**
  * @brief  向 SRL1200 发送数据包。
  * @param  huart: 串口句柄。
	* @param  command: 命令。
	* @param  dataBuf: 数据缓冲区
	* @param  timeout：超过此时间，还没有收到模块回复，表示发送失败。
  */
static SRL1200_RecvData_t * _SRL1200_SendData(UART_HandleTypeDef * huart, 
                                      uint8_t command, 
                                      Data_Buf_t * dataBuf, 
                                      uint32_t timeout)
{
	SRL1200_SendData_t sendData;
	SRL1200_SendData_Init(&sendData);
	
	sendData.header = 0xFF;
	sendData.command = command;
	
	if(dataBuf != NULL)
	{
		sendData.dataLen = dataBuf->size;
		sendData.dataBuf = (uint8_t *)malloc(dataBuf->size);
		memcpy(sendData.dataBuf, dataBuf->buf, dataBuf->size);
	}
	else
	{
		sendData.dataLen = 0;
		sendData.dataBuf = NULL;
	}
	
	// 开始接受数据
	DataBuf_Clear(&rxDataPack);
	HAL_UART_Receive_DMA(huart, rxDataPack.buf, DATA_BUF_SIZE);
	
	DataBuf_Clear(&txDataPack);
	SRL1200_EncapSendData(&sendData, &txDataPack);
	
	HAL_UART_Transmit_DMA(huart, txDataPack.buf, txDataPack.size);
	
	//等待接收数据完成
	uint32_t i = 0;
	while(SRL1200_isRecvOver == 0 && i < timeout)
	{
		HAL_Delay(1);
		i++;
	}
	SRL1200_isRecvOver = 0;
	
	SRL1200_RecvData_t * dataPack = NULL;
	if(i < timeout)
	{
		dataPack = (SRL1200_RecvData_t*)malloc(sizeof(SRL1200_RecvData_t));
		SRL1200_RecvData_Init(dataPack);
		SRL1200_ParseRecvData(&rxDataPack, dataPack);	
	}
	
	free(sendData.dataBuf);
	return dataPack;
}

void SRL1200_IDEL_IRQHandler(UART_HandleTypeDef * huart, DMA_HandleTypeDef * hdma)
{
	uint32_t flag = 0;
	uint32_t temp;
	flag =__HAL_UART_GET_FLAG(huart ,UART_FLAG_IDLE);
	if((flag != RESET))
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		temp = huart->Instance->SR;
		temp = huart->Instance->DR;
		temp = hdma->Instance->CNDTR;
		HAL_UART_DMAStop(huart);
		rxDataPack.size = DATA_BUF_SIZE - temp;
		SRL1200_isRecvOver = 1;
	}
}

/**
  * @brief  封装 SRL1200 发送的数据。
	* @param  sendData: 待发送给 SRL1200 的数据。
	* @param  dataBuf: 封装完成的数据。
  */
static void SRL1200_EncapSendData(SRL1200_SendData_t * sendData, Data_Buf_t * dataBuf)
{
	dataBuf->buf[0] = sendData->header;
	dataBuf->buf[1] = sendData->dataLen;
	dataBuf->buf[2] = sendData->command;
	
	if(sendData->dataLen > 0)
	{
		memcpy(&dataBuf->buf[3], sendData->dataBuf, sendData->dataLen);
	}
	uint16_t crc16 = CalcCRC(&dataBuf->buf[1], sendData->dataLen+2);
	dataBuf->buf[sendData->dataLen+3] = *(((uint8_t *)&crc16)+1);
	dataBuf->buf[sendData->dataLen+4] = *(((uint8_t *)&crc16));
	dataBuf->size = sendData->dataLen+5;
}

/**
  * @brief  解析来自 SRL1200 回复的数据。
	* @param  dataBuf: 存放解析成功的数据。
	* @param  recvData: 待被解析的数据。
  */
static void SRL1200_ParseRecvData(Data_Buf_t * dataBuf, SRL1200_RecvData_t * recvData)
{
	uint8_t * p = dataBuf->buf;
	
	recvData->header = p[0];
	recvData->dataLength = p[1];
	recvData->command = p[2];
	recvData->status = (p[3]<< 8) + p[4];
	recvData->crc16 = (p[recvData->dataLength+5]<<8) + p[recvData->dataLength+6];
	
	recvData->dataBuf = (uint8_t *)malloc(recvData->dataLength);
	memcpy(recvData->dataBuf, &p[5], recvData->dataLength);
}


/**
  * @brief  初始化 SRL1200_RecvData_t
  * @param  buf: SRL1200_RecvData_t 指针。
  */
static void SRL1200_SendData_Init(SRL1200_SendData_t * sendData)
{
	sendData->header = 0xFF;
	sendData->dataLen = 0;
	sendData->command = 0;
	sendData->dataBuf = NULL;
	sendData->crc16 = 0;
}

/**
  * @brief  初始化 SRL1200_RecvData_t
  * @param  buf: SRL1200_RecvData_t 指针。
  */
static void SRL1200_RecvData_Init(SRL1200_RecvData_t * buf)
{
	buf->header = 0xFF;
	buf->dataLength = 0;
	buf->command = 0;
	buf->dataBuf = NULL;
	buf->status = 0;
	buf->crc16 = 0;
}

static void SRL1200_Log(SRL1200_RecvData_t *recvData)
{
	printf("数据长度：0x%X\r\n", recvData->dataLength);
	printf("命令：0x%X\r\n", recvData->command);
	printf("状态；0x%X\r\n", recvData->status);
	printf("CRC16 校验：0x%X\r\n", recvData->crc16);
	if(recvData->dataLength > 0)
	{
		printf("接收的数据为：");
		for(int i = 0; i < recvData->dataLength; i++)
		{
		 printf("0x%X ", recvData->dataBuf[i]);
		}
		printf("\r\n");
	}
}

static void SRL1200_WriteByteData(Data_Buf_t * dest, uint8_t * data, uint8_t len)
{
	for(uint8_t i = len-1; i >= 0; i--)
	{
		dest->buf[dest->size++] = *(data+i);
	}
}

