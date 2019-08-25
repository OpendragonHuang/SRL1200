#include "srl1200.h"

#include <string.h>
#include <Stdlib.h>

Data_Buf_t txDataPack;
Data_Buf_t rxDataPack;
Data_Buf_t tmpDataPack;
__IO uint8_t SRL1200_isRecvOver = 0;

#define GET_BYTE_VALUE(VAR, BYTE)  *((uint8_t *)(&VAR)+BYTE)

// SRL 1200 �ڲ���������
// ���ų�ʼ
static void SRL1200_PinInit(void);
// ���ݷ��ͺ���
static SRL1200_RecvData_t * _SRL1200_SendData(UART_HandleTypeDef * huart, 
                                      uint8_t command, 
                                      Data_Buf_t * dataBuf, 
                                      uint32_t timeout);
// ԭʼ���ݷ�װ�ͽ�������
static void SRL1200_ParseRecvData(Data_Buf_t * dataBuf, SRL1200_RecvData_t * recvData);
static void SRL1200_EncapSendData(SRL1200_SendData_t * sendData, Data_Buf_t * dataBuf);

// ���ݳ�ʼ������
static void SRL1200_SendData_Init(SRL1200_SendData_t * sendData);
static void SRL1200_RecvData_Init(SRL1200_RecvData_t * buf);

static void SRL1200_Log(SRL1200_RecvData_t *recvData);
static void SRL1200_WriteByteData(Data_Buf_t * dest, uint8_t * data, uint8_t len);


/**
  * @brief  ��ʼ�� SRL1200 ��
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
  * @brief  �� SRL1200 �� Boot ���л��� App �㡣
  */
int SRL1200_BootSwitchApp(UART_HandleTypeDef * huart)
{
	int ret = SRL1200_SendData(huart, 0x04, NULL, 1000, NULL, NULL);
	return ret;
}

/**
  * @brief  ��ȡ������ǩ����Ϣ��
  * @param  huart: ���ھ����
	* @param  selectOption: ��ͬ��ֵ����Ҫ��ͬ�Ĳ���������Ҫ�Ĳ����� 0 ��伴�ɡ�
	* @param  metaDataFlag: �ο� SRL1200 �����ֲᡣ 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData ���ݵ�ʵ�ʳ���  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���ǩ�������
  * @param  huart: ���ھ����
	* @param  selectOption: ��ͬ��ֵ����Ҫ��ͬ�Ĳ���������Ҫ�Ĳ����� 0 ��伴�ɡ�
	* @param  metaDataFlag: �ο� SRL1200 �����ֲᡣ 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData ���ݵ�ʵ�ʳ���  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���±�ǩ�� EPC��
  * @param  huart: ���ھ����
	* @param  selectOption: ��ͬ��ֵ����Ҫ��ͬ�Ĳ���������Ҫ�Ĳ����� 0 ��伴�ɡ�
	* @param  metaDataFlag: �ο� SRL1200 �����ֲᡣ 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData ���ݵ�ʵ�ʳ���  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���±�ǩ�� EPC��
  * @param  huart: ���ھ����
	* @param  selectOption: ��ͬ��ֵ����Ҫ��ͬ�Ĳ���������Ҫ�Ĳ����� 0 ��伴�ɡ�
	* @param  metaDataFlag: �ο� SRL1200 �����ֲᡣ 
	* @param  selectAddress: 
	* @param  selectDataLen: 
	* @param  selectData: 
	* @param  len: selectData ���ݵ�ʵ�ʳ���  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  �������ߵ����á�
  * @param  huart: ���ھ����
	* @param  option: ��ͬ��ֵ����Ҫ��ͬ�Ĳ���������Ҫ�Ĳ����� 0 ��伴�ɡ�
	* @param  txAntNum: �ο� SRL1200 �����ֲᡣ 
	* @param  rxAntNum: �ο� SRL1200 �����ֲᡣ
	* @param  readPower: �ο� SRL1200 �����ֲᡣ
	* @param  writePower: �ο� SRL1200 �����ֲᡣ
	* @param  settingTime: �ο� SRL1200 �����ֲᡣ  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���õ�ǰ��ǩ��Э�顣
  * @param  huart: ���ھ����
	* @param  currentProtocol: Ŀǰֵֻ��Ϊ 0x0005, ��ʾ����Ϊ GEN2��18K-6CЭ�顣  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ������Ƶ��
  * @param  huart: ���ھ����
	* @param  freq1: 1 ��Ƶ��ֵ��
	* @param  freq2: 2 ��Ƶ��ֵ��
	* @param  freq3: 3 ��Ƶ��ֵ��
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���� GPIO �����ֵ��
  * @param  huart: ���ھ����
	* @param  GPIO1: Ϊ 0x01 ��ʾ���� GPIO 1������ֵ�����á�
	* @param  GPIO1OutputValue: 0x01 ����ߵ�ƽ��0x00 ����͵�ƽ��
	* @param  GPIO2: Ϊ 0x02 ��ʾ���� GPIO 2������ֵ������
	* @param  GPIO2OutputValue: 0x01 ����ߵ�ƽ��0x00 ����͵�ƽ��
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���õ�ǰ��Ƶ������
  * @param  huart: ���ھ����
	* @param  code: �ο� SRL1200 �����ֲᡣ  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ���ù���ģʽ��
  * @param  huart: ���ھ����
	* @param  powerMode: �ο� SRL1200 �����ֲᡣ  
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  �����Ķ������á�
  * @param  huart: ���ھ����
	* @param  option: �ο� SRL1200 �����ֲᡣ
	* @param  key: �ο� SRL1200 �����ֲᡣ
	* @param  value: �ο� SRL1200 �����ֲᡣ
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ����Э�����á�
  * @param  huart: ���ھ����
	* @param  protocolValue: �ο� SRL1200 �����ֲᡣ
	* @param  parameter: �ο� SRL1200 �����ֲᡣ
	* @param  option: �ο� SRL1200 �����ֲᡣ
	* @param  value: �ο� SRL1200 �����ֲᡣ
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡ���ߵ�������Ϣ��
  * @param  huart: ���ھ����
	* @param  option: ѡ�����Ϊ 0x01 ~ 0x05��
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡ�����书����Ϣ��
  * @param  huart: ���ھ����
	* @param  option: ѡ���ֻ��Ϊ 0x00 �� 0x01����Ч��һ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡ��ǰ������ǩ��Э�顣
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetCurrentTagProtocol(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x63, NULL, timeout, result, statusCode);
}

/**
  * @brief  ��ȡд���书����Ϣ��
  * @param  huart: ���ھ����
	* @param  option: ѡ���ֻ��Ϊ 0x00 �� 0x01����Ч��һ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡ��Ƶ��
  * @param  huart: ���ھ����
	* @param  option: ѡ��, 0x00 ��ʾ��ȡ��Ƶ��0x01 ��ȡ��Ƶ�����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡ��ǰ GPIO ������ֵ��
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetGIPIOInputValue(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x66, NULL, timeout, result, statusCode);
}

/**
  * @brief  ��ȡ��ǰƵ�ʵ�����
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetCurrentFrequencyRegion(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x67, NULL, timeout, result, statusCode);
}

/**
  * @brief  ��ȡ���ʵ�ģʽ��
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetPowerMode(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x68, NULL, timeout, result, statusCode);
}

/**
  * @brief  ��ȡ��д��������Ϣ��
  * @param  huart: ���ھ����
	* @param  option: �� 0x9A ���õ�ֵһ�¡�
	* @param  key: �� 0x9A ���õ�ֵһ��
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡЭ��������Ϣ��
  * @param  huart: ���ھ����
	* @param  procotolValue: �� 0x9B ����һ�¡�
	* @param  parameter: �� 0x9B ����һ��
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
  * @brief  ��ȡ���õı�ǩЭ�顣
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetCanUseTagProcotol(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x70, NULL, timeout, result, statusCode);
}

/**
  * @brief  ��ȡ���õ�Ƶ������
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetCanUseFrequencyRegion(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x71, NULL, timeout, result, statusCode);
}

/**
  * @brief  ��ȡоƬ��ǰ���¶ȡ�
  * @param  huart: ���ھ����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
  */
int SRL1200_GetCurentTemperature(UART_HandleTypeDef * huart,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode )
{
	return SRL1200_SendData(huart, 0x72, NULL, timeout, result, statusCode);
}



/**
  * @brief  �� SRL1200 �������ݰ���
  * @param  huart: ���ھ����
	* @param  command: ���
	* @param  dataBuf: ���ݻ�����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
	* @param  result������ģ��ظ������ݡ�
  * @param  statusCode����ģ��ظ���״̬�롣
	* @retval ���� 0 ��ʾ����ģ�����ݳɹ������� -1 ��ʾ����ʧ�ܡ�
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
		// ������յ�������
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
  * @brief  �� SRL1200 �������ݰ���
  * @param  huart: ���ھ����
	* @param  command: ���
	* @param  dataBuf: ���ݻ�����
	* @param  timeout��������ʱ�䣬��û���յ�ģ��ظ�����ʾ����ʧ�ܡ�
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
	
	// ��ʼ��������
	DataBuf_Clear(&rxDataPack);
	HAL_UART_Receive_DMA(huart, rxDataPack.buf, DATA_BUF_SIZE);
	
	DataBuf_Clear(&txDataPack);
	SRL1200_EncapSendData(&sendData, &txDataPack);
	
	HAL_UART_Transmit_DMA(huart, txDataPack.buf, txDataPack.size);
	
	//�ȴ������������
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
  * @brief  ��װ SRL1200 ���͵����ݡ�
	* @param  sendData: �����͸� SRL1200 �����ݡ�
	* @param  dataBuf: ��װ��ɵ����ݡ�
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
  * @brief  �������� SRL1200 �ظ������ݡ�
	* @param  dataBuf: ��Ž����ɹ������ݡ�
	* @param  recvData: �������������ݡ�
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
  * @brief  ��ʼ�� SRL1200_RecvData_t
  * @param  buf: SRL1200_RecvData_t ָ�롣
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
  * @brief  ��ʼ�� SRL1200_RecvData_t
  * @param  buf: SRL1200_RecvData_t ָ�롣
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
	printf("���ݳ��ȣ�0x%X\r\n", recvData->dataLength);
	printf("���0x%X\r\n", recvData->command);
	printf("״̬��0x%X\r\n", recvData->status);
	printf("CRC16 У�飺0x%X\r\n", recvData->crc16);
	if(recvData->dataLength > 0)
	{
		printf("���յ�����Ϊ��");
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

