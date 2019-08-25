#ifndef __SRL_1200_H
#define __SRL_1200_H

#ifdef __cplusplus
extern "C" {
#endif

// HAL 库基本头文件包括
#include "stm32f1xx_hal.h"
#include "usart.h"

// 工具方法
#include "data_buf.h"
#include "crc.h"

// SRL1200 引进相关定义
#define SRL1200_EN_CLCK()       __HAL_RCC_GPIOC_CLK_ENABLE();
#define SRL1200_EN_Pin          GPIO_PIN_6
#define SRL1200_EN_GPIO_Port    GPIOC
#define SRL1200_RST_CLCK()      __HAL_RCC_GPIOB_CLK_ENABLE();
#define SRL1200_RST_Pin         GPIO_PIN_6
#define SRL1200_RST_GPIO_Port   GPIOB

// 定义 SRL1200 相关数据类型
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

// SRL1200 recv buf data 相关函数
void SRL1200_Init(void);
void SRL1200_Enable(void);
void SRL1200_Disable(void);
void SRL1200_RST(void);
int SRL1200_BootSwitchApp(UART_HandleTypeDef * huart);

// App 层信息设置函数
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

// App 层信息获取相关函数
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

// SRL1200 接收数据结束的中断回调函数
void SRL1200_IDEL_IRQHandler(UART_HandleTypeDef * huart, DMA_HandleTypeDef * hdma);
#ifdef __cplusplus
}
#endif

#endif

