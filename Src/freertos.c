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
//	printf("准备从 Boot 层切换到 App 层\r\n");
//	if(SRL1200_BootSwitchApp(&huart1) == -1)
//	{
//		printf("从 Boot 层切换到 App 层失败\r\n");
//	}
//	else
//	{
//		printf("成功从 Boot 层切换到 App 层\r\n");
//		printf("---------------------------------开始测试---------------------------------\r\n");
//		test02();
//	}
	test01();
}

void test01(void)
{
//	uint8_t buf = 0x60;
//	uint8_t buf[] = {0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD};
//	uint8_t buf[] = {0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44};
//	printf("------------------------------App层操作命令测试------------------------------\r\n");
//	printf("1.测试单标签的读操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_ReadTagSingle(&huart1, 0x01E8, 0x10, 0x0014, 0, 0, NULL, 0, NULL, NULL);	
//	printf("2.测试多标签的读操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_ReadTagMultiple(&huart1, 0x00, 0, 0x00C8, 0, 0, 0, NULL, 0, NULL, NULL);
//	printf("3.测试写 EPC 操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_WriteTagEPC(&huart1, 0x00C8, 0x00, 0, 0, 0, 0, NULL, 0, buf, 8, NULL, NULL);
//	printf("4.测试写 数据操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_WriteTagData(&huart1, 0x03E8, 00, 0x01, 0x03, 0, 0, 0, NULL, 0, buf, 8, NULL, NULL);
//	printf("5.测试锁定或解锁标签操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_LockOrUnlockTag(&huart1, 0x03E8, 00, 0x00, 0x02, 0x02, 0, 0, NULL, 0, NULL, NULL);
//	printf("6.测试杀死标签操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_KillTag(&huart1, 0x03E8, 00, 0, 0, 0, 0, NULL, 0, NULL, NULL);
//	printf("7.测试读取标签数据操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_ReadTagData(&huart1, 0x03E8, 0x00, 0, 0x02, 0x01, 0x02, 0, 0x10, 0x04, &buf, 1, NULL, NULL);
//	printf("8.测试读取标签保存的信息操作\r\n");
//	printf("----------------------------------------------------------------------------\r\n");
//	SRL1200_ReadSaveTagInfo(&huart1, 0x00BF, 0x00, NULL, NULL);
	printf("9.清楚标签缓冲操作\r\n");
	printf("----------------------------------------------------------------------------\r\n");
	SRL1200_ClearTagCache(&huart1, NULL, NULL);

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
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
