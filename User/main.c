/*-----------------------------------------------------------------------------
File Name   	:   main.c
Author          :   zhaoji
Created Time    :   2018.02.24
Description     :   主程序入口
-----------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "delay.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "m5310.h"
#include "FreeRTOS_CLI.h"
#include "tft_lcd.h"
#include "User_Cli.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
#define	CLI_MAX_OUTPUT_LENGTH   100
#define CLI_MAX_INPUT_LENGTH    32


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART2(UART_CLI_DEBUG)数据接收buffer */
uint8_t   UART_CLI_RxBuffer[CLI_MAX_INPUT_LENGTH]    = {0};
uint32_t  UART_CLI_RxBufferLen      =  0;


TaskHandle_t start_task;     /* 开始任务   */
TaskHandle_t cli_task;       /* CLI任务    */
TaskHandle_t m5310_task;     /* M5310任务  */


/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------*
**                             Function Declare                               *
**----------------------------------------------------------------------------*/
void _CMIOT_CliTaskProc(void *pvParameters);
void _CMIOT_M5310TaskProc(void *pvParameters);
void _CMIOT_StartTaskProc(void *pvParameters);



/*-----------------------------------------------------------------------------
Function Name	:	main
Author			:	zhaoji
Created Time	:	2018.02.01
Description 	:	main函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
int main(void)
{
	delay_init();    /* 初始化systick实现普通延时 */
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* 初始化串口 */
	_CMIOT_Uart_Init(UART_CLI_DEBUG, 115200);
	_CMIOT_Uart_Init(UART_M5310, 115200);
	_CMIOT_Uart_Init(UART_BLUETOOTH, 115200);
	_CMIOT_Debug("%s(UART Init OK!)\r\n", __func__);
	
	// LCD_Init();
	
	
	/* 创建开始任务，开始任务在创建好其它任务后删除 */
	xTaskCreate((TaskFunction_t      )_CMIOT_StartTaskProc,
				(const char*         )"start_task",
				(uint16_t            )256,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&start_task);
					
	vTaskStartScheduler();    /* 开启任务调度 */
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_StartTaskProc
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	:	开启任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_StartTaskProc(void *pvParameters)
{
	taskENTER_CRITICAL();   /* 进入临界区 */
	
	/* 创建CLI任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_CliTaskProc,
				(const char*         )"cli_task",
				(uint16_t            )256,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&cli_task);
	/* 创建M5310任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_M5310TaskProc,
				(const char*         )"m5310_task",
				(uint16_t            )256,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&m5310_task);
							
	vTaskDelete(start_task);   /* 删除开始任务 */
	
	taskEXIT_CRITICAL();   /* 退出临界区 */
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_CliTaskProc
Author			:	zhaoji
Created Time	:	2018.01.12
Description 	:	CLI任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_CliTaskProc(void *pvParameters)
{
	uint32_t notifyValue;
	BaseType_t xMoreDataToFollow;
	/* The input and output buffers are declared static to keep them off the stack. */
	static uint8_t pcOutputString[ CLI_MAX_OUTPUT_LENGTH ];
	
	_CMIOT_CLI_Init();
	
	while(1)
	{
		notifyValue = ulTaskNotifyTake(pdTRUE, 60000);   /* 获取任务通知 */
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
			_CMIOT_Debug("CLI Command Receive [CR][LF]\r\n");
			memset(pcOutputString, 0, sizeof(pcOutputString));
			do
			{
				xMoreDataToFollow = FreeRTOS_CLIProcessCommand
				(
					(const char *)UART_CLI_RxBuffer,   /* The command string.*/
					(char *)pcOutputString,            /* The output buffer. */
					CLI_MAX_OUTPUT_LENGTH              /* The size of the output buffer. */
				);
				_CMIOT_Uart_send(UART_CLI_DEBUG, pcOutputString, strlen((const char*)pcOutputString));
			}
			while(xMoreDataToFollow != pdFALSE);
			
			memset(UART_CLI_RxBuffer, 0, sizeof(UART_CLI_RxBuffer));
			UART_CLI_RxBufferLen = 0;
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310TaskProc
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	:	M5310任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_M5310TaskProc(void *pvParameters)
{
	uint8_t msg[32] = {0};
	
//	sprintf((char *)msg, "Register time: %d\r\n", _CMIOT_M5310_GetRegisterTime());
//	LCD_ShowString(30,40,200,24,24,msg);
//	
//	sprintf((char *)msg, "CSQ VALUE: %d\r\n", _CMIOT_M5310_GetSignalstrength());
//	LCD_ShowString(30,70,200,24,24,msg);
	
	
	// _CMIOT_M5310_GetSignalstrength();
	_CMIOT_M5310_GetUeState();
	while(1)
	{
		//
	}
}



