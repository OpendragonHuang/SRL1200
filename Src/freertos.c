/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>

#include "usart.h"
#include "srl1200.h"
#include "data_buf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
Data_Buf_t sendDataPack;
Data_Buf_t recvDataPack;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void test00(void);
void test01(void);
void test02(void);
void test03(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  /* Infinite loop */
	
	DataBuf_Init(&sendDataPack);
	DataBuf_Init(&recvDataPack);
	SRL1200_Init();
	test00();
  for(;;)
  {
		osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void test00(void)
{
	printf("准备从 Boot 层切换到 App 层\r\n");
	if(SRL1200_BootSwitchApp(&huart1) == -1)
	{
		printf("从 Boot 层切换到 App 层失败\r\n");
	}
	else
	{
		printf("成功从 Boot 层切换到 App 层\r\n");
		printf("---------------------------------开始测试---------------------------------\r\n");
		test02();
	}
}

void test01(void)
{
	printf("------------------------------App层操作命令测试------------------------------\r\n");
	printf("1.测试单标签的读操作\r\n");
	printf("----------------------------------------------------------------------------\r\n");
	SRL1200_ReadTagSingle(&huart1, 0x01E8, 0x10, 0x0014, 0, 0, NULL, 0, NULL, NULL);
	
}

void test02(void)
{
	printf("------------------------------App层设置命令测试------------------------------\r\n");
	printf("1.设置天线的配置\r\n");
	SRL1200_SetAntennaConfig(&huart1, 0x04,0x01,0x00, 0x03E8,0x0BB8, 0x01F4, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("2.设置当前标签的协议\r\n");
	SRL1200_SetCurrentTagProtocol(&huart1, 0x0005, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("3.设置 GPIO 的输出值\r\n");
	SRL1200_SetGPIOOutputValue(&huart1, 0x01, 0x01, 0x02, 0x01, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("4.设置功率模式\r\n");
	SRL1200_SetPowerMode(&huart1, 0x00, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("6.设置阅读器配置\r\n");
	SRL1200_SetCardReaderConfig(&huart1, 0x01, 0x00, 0x00, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("7.设置协议配置。\r\n");
	SRL1200_SetProtocolConfig(&huart1,  0x05, 0x12, 0x01, 0x03, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");	
}

void test03(void)
{
	printf("------------------------------App层获取命令测试------------------------------\r\n");
	printf("1.获取天线的配置信息\r\n");
	SRL1200_GetAntennaConfig(&huart1, 0x01, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("2.获取读发射功率信息。\r\n");
	SRL1200_GetReadTransmitPowerInfo(&huart1, 0x01, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("3.获取当前工作标签的协议\r\n");
	SRL1200_GetCurrentTagProtocol(&huart1, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("4.获取写发射功率信息\r\n");
	SRL1200_GetWriteTransmitPowerInfo(&huart1, 0x01, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("5.获取跳频表\r\n");
	SRL1200_GetFrequencyHoppingTable(&huart1, 0x00, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("6.获取当前GPIO的输入值\r\n");
	SRL1200_GetGIPIOInputValue(&huart1, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("7.获取当前频率的区域\r\n");
	SRL1200_GetCurrentFrequencyRegion(&huart1, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("8.功率的模式\r\n");
	SRL1200_GetPowerMode(&huart1, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("9.可用的标签协议\r\n");
	SRL1200_GetCanUseTagProcotol(&huart1, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	printf("10.获取芯片当前的温度\r\n");
	SRL1200_GetCurentTemperature(&huart1, 1000, NULL, NULL);
	printf("--------------------------------------------------------------------------\r\n");
	
}

//void test02(void)
//{
//	printf("APP 层操作命令\r\n");
//	printf("--------------------------------------------------------------------------\r\n");
//	printf("1.单标签存盘命令\r\n");
//	sendDataPack.buf[0] = 0x03;
//	sendDataPack.buf[1] = 0xE8;
//	
//	sendDataPack.buf[2] = 0x03;
//	
//	sendDataPack.buf[3] = 0x00;
//	sendDataPack.buf[4] = 0x00;
//	sendDataPack.buf[6] = 0x00;
//	sendDataPack.buf[7] = 0x20;
//	
//	sendDataPack.buf[8] = 0x10;
//	
//	sendDataPack.buf[9] = 0x12;
//	sendDataPack.buf[10] = 0x34;
//	sendDataPack.size = 0x0A;
//	SRL1200_SendData(&huart1, 0x21, &sendDataPack, 0x03E8, &recvDataPack);
//	DataBuf_Clear(&recvDataPack);
//	printf("--------------------------------------------------------------------------\r\n");
//	printf("2.多标签存盘命令\r\n");
//	sendDataPack.buf[0] = 0x00;
//	
//	sendDataPack.buf[1] = 0x00;
//	sendDataPack.buf[2] = 0x00;
//	
//	sendDataPack.buf[3] = 0x00;
//	sendDataPack.buf[4] = 0xC8;
//	
//	sendDataPack.size = 0x05;
//	SRL1200_SendData(&huart1, 0x22, &sendDataPack, 0x00C8, &recvDataPack);
//	DataBuf_Clear(&recvDataPack);
//	printf("--------------------------------------------------------------------------\r\n");
//	printf("3.写标签 EPC 命令\r\n");
//	sendDataPack.buf[0] = 0x03;
//	sendDataPack.buf[1] = 0xE8;
//	
//	sendDataPack.buf[2] = 0x00;
//	
//	sendDataPack.buf[3] = 0x00;
//	
//	sendDataPack.buf[4] = 0x11;
//	sendDataPack.buf[5] = 0x11;
//	sendDataPack.buf[6] = 0x22;
//	sendDataPack.buf[7] = 0x22;
//	sendDataPack.buf[8] = 0x33;
//	sendDataPack.buf[9] = 0x33;
//	sendDataPack.buf[10] = 0x44;
//	sendDataPack.buf[11] = 0x44;
//	
//	sendDataPack.size = 0x0C;
//	SRL1200_SendData(&huart1, 0x23, &sendDataPack, 0x03E8, &recvDataPack);
//	DataBuf_Clear(&recvDataPack);
//	printf("--------------------------------------------------------------------------\r\n");
//	printf("4.写标签存储区命令\r\n");
//	sendDataPack.buf[0] = 0x03;
//	sendDataPack.buf[1] = 0xE8;
//	
//	sendDataPack.buf[2] = 0x00;
//	
//	sendDataPack.buf[3] = 0x00;
//	sendDataPack.buf[4] = 0x00;
//	sendDataPack.buf[5] = 0x00;
//	sendDataPack.buf[6] = 0x01;
//	
//	sendDataPack.buf[7] = 0x03;
//	
//	sendDataPack.buf[8] = 0xAA;
//	sendDataPack.buf[9] = 0xAA;
//	sendDataPack.buf[10] = 0xAA;
//	sendDataPack.buf[11] = 0xAA;
//	sendDataPack.buf[12] = 0xBB;
//	sendDataPack.buf[13] = 0xBB;
//	sendDataPack.buf[14] = 0xBB;
//	sendDataPack.buf[15] = 0xBB;
//	sendDataPack.buf[16] = 0xCC;
//	sendDataPack.buf[17] = 0xCC;
//	sendDataPack.buf[18] = 0xCC;
//	sendDataPack.buf[19] = 0xCC;
//	sendDataPack.buf[20] = 0xDD;
//	sendDataPack.buf[21] = 0xDD;
//	sendDataPack.buf[22] = 0xDD;
//	sendDataPack.buf[23] = 0xDD;
//	
//	sendDataPack.size = 0x14;
//	SRL1200_SendData(&huart1, 0x24, &sendDataPack, 0x03E8, &recvDataPack);
//	DataBuf_Clear(&recvDataPack);
//		printf("--------------------------------------------------------------------------\r\n");
//	printf("4.写标签存储区命令\r\n");
//	sendDataPack.buf[0] = 0x03;
//	sendDataPack.buf[1] = 0xE8;
//	
//	sendDataPack.buf[2] = 0x00;
//	
//	sendDataPack.buf[3] = 0x00;
//	sendDataPack.buf[4] = 0x00;
//	sendDataPack.buf[5] = 0x00;
//	sendDataPack.buf[6] = 0x01;
//	
//	sendDataPack.buf[7] = 0x03;
//	
//	sendDataPack.buf[8] = 0xAA;
//	sendDataPack.buf[9] = 0xAA;
//	sendDataPack.buf[10] = 0xAA;
//	sendDataPack.buf[11] = 0xAA;
//	sendDataPack.buf[12] = 0xBB;
//	sendDataPack.buf[13] = 0xBB;
//	sendDataPack.buf[14] = 0xBB;
//	sendDataPack.buf[15] = 0xBB;
//	sendDataPack.buf[16] = 0xCC;
//	sendDataPack.buf[17] = 0xCC;
//	sendDataPack.buf[18] = 0xCC;
//	sendDataPack.buf[19] = 0xCC;
//	sendDataPack.buf[20] = 0xDD;
//	sendDataPack.buf[21] = 0xDD;
//	sendDataPack.buf[22] = 0xDD;
//	sendDataPack.buf[23] = 0xDD;
//	
//	sendDataPack.size = 0x14;
//	SRL1200_SendData(&huart1, 0x24, &sendDataPack, 0x03E8, &recvDataPack);
//	DataBuf_Clear(&recvDataPack);
//}

//void test04(void)
//{
//	printf("APP 层设置命令\r\n");
//	printf("--------------------------------------------------------------------------\r\n");
//	printf("1.单标签存盘命令\r\n");
//}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
