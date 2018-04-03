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
#include "lcd.h"
#include "User_Cli.h"
#include "ui.h"
#include "key.h"
#include "timers.h"
#include "adc.h"

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

extern uint8_t   UART_M5310_RxBuffer[512];

TaskHandle_t start_task;     	/* 开始任务    */
TaskHandle_t cli_task;       	/* CLI任务     */
TaskHandle_t m5310_task;     	/* M5310任务   */
TaskHandle_t lcd_task;       	/* LCD任务     */
TaskHandle_t bluetooth_task;	/* 蓝牙任务    */


TimerHandle_t watchDogTimer;	/* 看门狗定时器 */

CM_MENU_POSITION menuPosition = {0, 0, 0, 0, 0, KEYPAD_ENTER};	/* 菜单位置 */


/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------*
**                             Function Declare                               *
**----------------------------------------------------------------------------*/

void _CMIOT_CliTaskProc(void *pvParameters);			/* CLI任务入口函数    */
void _CMIOT_M5310TaskProc(void *pvParameters);			/* M5310任务入口函数  */
void _CMIOT_StartTaskProc(void *pvParameters);			/* 开启任务入口函数   */
void _CMIOT_BluetoothTaskProc(void *pvParameters);		/* 蓝牙任务入口函数   */

void IWDG_Configuration(void);	/* 初始化看门狗 */


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
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	/* 最高1位用来配置抢占优先级，低3位用来配置响应优先级 */
	
	LCD_Init();
	
	_CMIOT_UI_BootPage();
	
	/* 初始化串口 */
	_CMIOT_Uart_Init(UART_CLI_DEBUG, 115200);
	_CMIOT_Uart_Init(UART_M5310, 9600);
	_CMIOT_Uart_Init(UART_BLUETOOTH, 57600);
	_CMIOT_Debug("%s(UART Init OK!)\r\n", __func__);
	
	Adc_Init();		/* ADC初始化 */
	
	/* 检查是否被看门狗重启 */
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		_CMIOT_Debug("*****<ERROR>Reset by stm32 IWDG!<ERROR>*****\r\n");
		RCC_ClearFlag();
	}
	
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
	_CMIOT_Debug("%s...\r\n", __func__);
	
	IWDG_Configuration();	/* 初始化看门狗及喂狗定时器 */

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
				(uint16_t            )512,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&m5310_task);
	/* 创建蓝牙任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_BluetoothTaskProc,
				(const char*         )"bluetooth_task",
				(uint16_t            )512,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&bluetooth_task);
							
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
	
	_CMIOT_Debug("%s...\r\n", __func__);
	_CMIOT_CLI_Init();
	
	while(1)
	{
		notifyValue = ulTaskNotifyTake(pdTRUE, 60000);   /* 获取任务通知 */
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
			_CMIOT_Debug("%s(NotifyValue: %d)\r\n", __func__, notifyValue);
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
Function Name	:	_CMIOT_LcdTaskProc
Author			:	zhaoji
Created Time	:	2018.03.16
Description 	:	LCD显示任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_LcdTaskProc(void *pvParameters)
{
	_CMIOT_Debug("%s...\r\n", __func__);
	_CMIOT_TabIndex(&menuPosition);
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
	uint32_t notifyValue;
	
	cm_key_init();	/* 初始化按键 */
	
	_CMIOT_Debug("%s(NotifyValue: %d)\r\n", __func__, notifyValue);
	
	taskENTER_CRITICAL();   /* 进入临界区 */
	
	/* 创建LCD任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_LcdTaskProc,
				(const char*         )"lcd_task",
				(uint16_t            )512,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&lcd_task);
				
	taskEXIT_CRITICAL();   /* 退出临界区 */
	
	while(1)
	{
		notifyValue = ulTaskNotifyTake(pdTRUE, 60000);   /* 获取任务通知 */
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
			taskENTER_CRITICAL();   /* 进入临界区 */
			
			_CMIOT_Debug("%s(Recv NotifyValue)\r\n", __func__);
			
			/* 当用户通过按钮切换功能菜单时，删除重新创建线程 */
			if(lcd_task != NULL)
			{
				vTaskDelete(lcd_task);   /* 删除任务 */
				_CMIOT_Debug("%s(Delete Lcd task)\r\n", __func__);
			}
			
			/* 创建LCD任务 */
			xTaskCreate((TaskFunction_t      )_CMIOT_LcdTaskProc,
						(const char*         )"lcd_task",
						(uint16_t            )512,
						(void*               )NULL,
						(UBaseType_t         )1,
						(TaskHandle_t*       )&lcd_task);
						
			taskEXIT_CRITICAL();   /* 退出临界区 */
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BluetoothTaskProc
Author			:	zhaoji
Created Time	:	2018.04.02
Description 	:	蓝牙任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BluetoothTaskProc(void *pvParameters)
{
	
}


/*-----------------------------------------------------------------------------
Function Name	:	IWDG_Configuration
Author			:	zhaoji
Created Time	:	2018.03.31
Description 	:	独立看门狗初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void IWDG_Configuration(void)  
{
	/* 喂狗时间计算： T =  (IWDG_Prescaler*Reload)/40(ms)      */
    RCC_LSICmd(ENABLE);	//打开LSI  
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);  
  
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  
    IWDG_SetPrescaler(IWDG_Prescaler_256);  
    IWDG_SetReload(782);	/* 782*256/40 ≈ 5000ms */
    IWDG_ReloadCounter();  
    IWDG_Enable();
	
	/* 每3秒喂一次狗 */
	watchDogTimer = xTimerCreate((const char*   )"WatchDogTimer",
								(TickType_t     )3000,
								(UBaseType_t    )pdTRUE,
								(void*          )1,
								(TimerCallbackFunction_t)IWDG_ReloadCounter);
								
	xTimerStart(watchDogTimer, 0);
}



