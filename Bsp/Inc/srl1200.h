#ifndef __SRL_1200_H
#define __SRL_1200_H

#ifdef __cplusplus
extern "C" {
#endif

// HAL �����ͷ�ļ�����
#include "stm32f1xx_hal.h"
#include "usart.h"

// ���߷���
#include "data_buf.h"
#include "crc.h"

// SRL1200 ������ض���
#define SRL1200_EN_CLCK()       __HAL_RCC_GPIOC_CLK_ENABLE();
#define SRL1200_EN_Pin          GPIO_PIN_6
#define SRL1200_EN_GPIO_Port    GPIOC
#define SRL1200_RST_CLCK()      __HAL_RCC_GPIOB_CLK_ENABLE();
#define SRL1200_RST_Pin         GPIO_PIN_6
#define SRL1200_RST_GPIO_Port   GPIOB

// ���� SRL1200 �����������
typedef struct
{
	uint8_t header;
	uint8_t dataLength;
	uint8_t command;
	uint16_t status;
	uint8_t * dataBuf;
	uint16_t crc16;
}SRL1200_RecvData_t;

typedef struct
{
	uint8_t header;
	uint8_t dataLen;
	uint8_t command;
	uint8_t * dataBuf;
	uint16_t crc16;
}SRL1200_SendData_t;

// SRL1200 recv buf data ��غ���
void SRL1200_Init(void);
void SRL1200_Enable(void);
void SRL1200_Disable(void);
void SRL1200_RST(void);
int SRL1200_BootSwitchApp(UART_HandleTypeDef * huart);

// App ���������
int SRL1200_ReadTagSingle(UART_HandleTypeDef * huart, 
													uint16_t timeout,
													uint8_t selectOption,
													uint16_t metaDataFlag,
													uint32_t selectAddress,
                          uint8_t  selectDataLen,
													uint8_t * selectData,
													uint8_t selectDataActualLen,
													Data_Buf_t * result,
													uint16_t * statusCode);
int SRL1200_ReadTagMultiple(UART_HandleTypeDef * huart, 
														uint8_t selectOption,	
														uint16_t searchFlags,
														uint16_t timeout,
														uint32_t accessPassword,
														uint32_t selectAddress,
														uint8_t  selectDataLen,
														uint8_t * selectData,
														uint8_t selectDataActualLen,
														Data_Buf_t * result,
														uint16_t * statusCode);
int SRL1200_WriteTagEPC(UART_HandleTypeDef * huart, 
												uint16_t timeout,														
												uint8_t selectOption,
												uint8_t RUF,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t selectDataActualLen,
												uint8_t * EPCTagID,
												uint8_t EPCTagIDLen,
												Data_Buf_t * result,
												uint16_t * statusCode);
int SRL1200_WriteTagData(UART_HandleTypeDef * huart,
												uint16_t timeout,
												uint8_t selectOption,
												uint32_t writeAddress,														
												uint8_t writeMemBank,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t selectDataActualLen,
												uint8_t * writeData,
												uint8_t writeDataLen,
												Data_Buf_t * result,
												uint16_t * statusCode);
int SRL1200_LockOrUnlockTag(UART_HandleTypeDef * huart,
														uint16_t timeout,
														uint8_t selectOption,
														uint32_t accessPassword,
														uint16_t maskBits,
														uint16_t actionBits,
														uint32_t selectAddress,
														uint8_t  selectDataLen,
														uint8_t * selectData,
														uint8_t selectDataActualLen,
														Data_Buf_t * result,
														uint16_t * statusCode);
int SRL1200_KillTag(UART_HandleTypeDef * huart,
														uint16_t timeout,
														uint8_t selectOption,
														uint32_t killPassword,
														uint8_t RUF,
														uint32_t selectAddress,
														uint8_t  selectDataLen,
														uint8_t * selectData,
														uint8_t selectDataActualLen,
														Data_Buf_t * result,
														uint16_t * statusCode);
int SRL1200_ReadTagData(UART_HandleTypeDef * huart,
												uint16_t timeout,
												uint8_t selectOption,
												uint16_t metaDataFlag,
												uint8_t readMemBank,
												uint32_t readAddress,
												uint8_t wordCount,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t selectDataActualLen,
												Data_Buf_t * result,
												uint16_t * statusCode);
int SRL1200_ReadSaveTagInfo(UART_HandleTypeDef * huart,
												uint16_t  metadataFlags,
												uint8_t readOption,
												Data_Buf_t * result,
												uint16_t * statusCode);
int SRL1200_ClearTagCache(UART_HandleTypeDef * huart,
													Data_Buf_t * result,
													uint16_t * statusCode);
int SRL1200_BlockWrite(UART_HandleTypeDef * huart,
												uint16_t timeout,
												uint8_t chipType,
												uint8_t selectOption,
												uint16_t subCommand,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t selectDataActualLen,
												uint8_t writeFlag,
												uint8_t memBank,
												uint32_t wordPoint,
												uint8_t wordCount,
												uint8_t * data,
												Data_Buf_t * result,
												uint16_t * statusCode);
int SRL1200_BlockClear(UART_HandleTypeDef * huart,
												uint16_t timeout,
												uint8_t chipType,
												uint8_t selectOption,
												uint16_t subCommand,
												uint32_t accessPassword,
												uint32_t selectAddress,
												uint8_t  selectDataLen,
												uint8_t * selectData,
												uint8_t selectDataActualLen,
												uint32_t wordPoint,
												uint8_t memBank,
												uint8_t wordCount,
												Data_Buf_t * result,
												uint16_t * statusCode);

// App ����Ϣ���ú���
int SRL1200_SetAntennaConfig(UART_HandleTypeDef * huart,
														 uint8_t option,
														 uint8_t txAntNum,
														 uint8_t rxAntNum,
														 uint16_t readPower,
														 uint16_t writePower,
														 uint16_t settingTime,
														 uint16_t timeout,
														 Data_Buf_t * result,
														 uint16_t * statusCode );
int SRL1200_SetCurrentTagProtocol(UART_HandleTypeDef * huart,
																	 uint16_t currentProtocol,
																	 uint16_t timeout,
																	 Data_Buf_t * result,
																	 uint16_t * statusCode );
int SRL1200_SetFrequencyHopping(UART_HandleTypeDef * huart,
													      uint32_t freq1,
															  uint32_t freq2,
															  uint32_t freq3,
														    uint16_t timeout,
													      Data_Buf_t * result,
													      uint16_t * statusCode );
int SRL1200_SetGPIOOutputValue(UART_HandleTypeDef * huart,
															 uint16_t GPIO1,
															 uint16_t GPIO1OutputValue,
															 uint16_t GPIO2,
															 uint16_t GPIO2OutputValue,
															 uint16_t timeout,
															 Data_Buf_t * result,
															 uint16_t * statusCode );
int SRL1200_SetCurrentFrequencyRegion(UART_HandleTypeDef * huart,
																			uint16_t code,
																			uint16_t timeout,
																			Data_Buf_t * result,
																			uint16_t * statusCode );
int SRL1200_SetPowerMode(UART_HandleTypeDef * huart,
												 uint8_t powerMode,
												 uint16_t timeout,
												 Data_Buf_t * result,
												 uint16_t * statusCode );
int SRL1200_SetCardReaderConfig(UART_HandleTypeDef * huart,
																uint8_t option,
																uint8_t key,
																uint8_t value,
																uint16_t timeout,
																Data_Buf_t * result,
																uint16_t * statusCode );
int SRL1200_SetProtocolConfig(UART_HandleTypeDef * huart,
															uint8_t protocolValue,
															uint8_t parameter,
															uint8_t option,
															uint8_t value,
															uint16_t timeout,
															Data_Buf_t * result,
															uint16_t * statusCode );

// App ����Ϣ��ȡ��غ���
int SRL1200_ReadTagSingle(UART_HandleTypeDef * huart, 
													uint16_t timeout,
													uint8_t selectOption,
													uint16_t metaDataFlag,
													uint32_t selectAddress,
                          uint8_t  selectDataLen,
													uint8_t * selectData,
													uint8_t len,
													Data_Buf_t * result,
													uint16_t * statusCode);
int SRL1200_GetAntennaConfig(UART_HandleTypeDef * huart,
													   uint8_t option,
														 uint16_t timeout,
													   Data_Buf_t * result,
													   uint16_t * statusCode );
int SRL1200_GetReadTransmitPowerInfo(UART_HandleTypeDef * huart,
																		 uint8_t option,
																		 uint16_t timeout,
																		 Data_Buf_t * result,
																		 uint16_t * statusCode );
int SRL1200_GetCurrentTagProtocol(UART_HandleTypeDef * huart,
																	 uint16_t timeout,
																	 Data_Buf_t * result,
																	 uint16_t * statusCode );
int SRL1200_GetWriteTransmitPowerInfo(UART_HandleTypeDef * huart,
																			 uint8_t option,
																			 uint16_t timeout,
																			 Data_Buf_t * result,
																			 uint16_t * statusCode );
int SRL1200_GetFrequencyHoppingTable(UART_HandleTypeDef * huart,
																		 uint8_t option,
																		 uint16_t timeout,
																		 Data_Buf_t * result,
																		 uint16_t * statusCode );
int SRL1200_GetGIPIOInputValue(UART_HandleTypeDef * huart,
															 uint16_t timeout,
															 Data_Buf_t * result,
															 uint16_t * statusCode );
int SRL1200_GetCurrentFrequencyRegion(UART_HandleTypeDef * huart,
																			 uint16_t timeout,
																			 Data_Buf_t * result,
																			 uint16_t * statusCode );
int SRL1200_GetPowerMode(UART_HandleTypeDef * huart,
												 uint16_t timeout,
												 Data_Buf_t * result,
												 uint16_t * statusCode );
int SRL1200_GetCardReaderConfig(UART_HandleTypeDef * huart,
																 uint8_t option,
																 uint8_t key,
																 uint16_t timeout,
																 Data_Buf_t * result,
																 uint16_t * statusCode );
int SRL1200_ProcotolConfig(UART_HandleTypeDef * huart,
													 uint8_t procotolValue,
													 uint8_t parameter,
													 uint16_t timeout,
													 Data_Buf_t * result,
													 uint16_t * statusCode );
int SRL1200_GetCanUseTagProcotol(UART_HandleTypeDef * huart,
																 uint16_t timeout,
																 Data_Buf_t * result,
																 uint16_t * statusCode );
int SRL1200_GetCanUseFrequencyRegion(UART_HandleTypeDef * huart,
																		 uint16_t timeout,
																		 Data_Buf_t * result,
																		 uint16_t * statusCode );
int SRL1200_GetCurentTemperature(UART_HandleTypeDef * huart,
																 uint16_t timeout,
																 Data_Buf_t * result,
																 uint16_t * statusCode );

int SRL1200_SendData(UART_HandleTypeDef * huart, 
										 uint8_t command, 
										 Data_Buf_t * dataBuf, 
										 uint32_t timeout,
										 Data_Buf_t * result,
										 uint16_t * statusCode);

// SRL1200 �������ݽ������жϻص�����
void SRL1200_IDEL_IRQHandler(UART_HandleTypeDef * huart, DMA_HandleTypeDef * hdma);
#ifdef __cplusplus
}
#endif

#endif

