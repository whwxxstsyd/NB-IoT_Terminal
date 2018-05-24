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
#include "bluetooth.h"



/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
#define	CLI_MAX_OUTPUT_LENGTH   100
#define CLI_MAX_INPUT_LENGTH    256


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART2(UART_CLI_DEBUG)数据接收buffer */
uint8_t   UART_CLI_RxBuffer[CLI_MAX_INPUT_LENGTH]    = {0};
uint32_t  UART_CLI_RxBufferLen      =  0;

bool CM_UI_BUSY = false;	/* UI界面切换状态忙标识（此时不响应按键请求） */

TaskHandle_t start_task;     	/* 开始任务    */
TaskHandle_t cli_task;       	/* CLI任务     */
TaskHandle_t m5310_task;     	/* M5310任务   */
TaskHandle_t lcd_task;       	/* LCD任务     */
TaskHandle_t bluetooth_task;	/* 蓝牙任务    */
TaskHandle_t bleCmdProcess_task = NULL;	/* 蓝牙命令处理任务 */


CM_MENU_POSITION menuPosition = {0, 0, 0, 0, 0, KEYPAD_ENTER};	/* 菜单位置 */

/* 蓝牙执行AT指令标志位 */
extern bool BLE_AT_EXE_FLAG;


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

void _CMIOT_IWDG_Configuration(void);	/* 初始化看门狗 */

void _CMIOT_BatteryCheckInit(void);


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
	/* 初始化延时 */
	delay_init();
	
	/* 最高1位用来配置抢占优先级，低3位用来配置响应优先级 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* 初始化串口 */
	_CMIOT_Uart_Init(UART_CLI_DEBUG, 921600);
	_CMIOT_Uart_Init(UART_M5310, 9600);
	_CMIOT_Uart_Init(UART_BLUETOOTH, 57600);
	_CMIOT_Debug("%s(UART Init OK!)\r\n", __func__);
	
	/* 检查是否被看门狗重启 */
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		_CMIOT_Debug("\r\n*****<ERROR>REBOOT BECAUSE STM32 IWDG!<ERROR>*****\r\n");
		RCC_ClearFlag();
		
		_CMIOT_BleResetGpioInit();				/* 初始BLE复位GPIO */
		_CMIOT_BleReset();						/* 复位BLE模组，后续重新初始化模组 */
	}
	
	/* ADC初始化 */
	Adc_Init();
	
	/* 初始化LCD */
	LCD_Init();
	
	/* 显示启动界面 */
	_CMIOT_UI_BootPage();
	
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	LCD_ShowString(10, 300, 16, (u8 *)"Usart Init OK!");
	delay_ms(500);
	
	/* M5310初始化 */
	LCD_ShowString(10, 300, 16, (u8 *)"NB-IoT Module Init ...");
	_CMIOT_M5310_Init();
	LCD_ShowString(10, 300, 16, (u8 *)"NB-IoT Module Init OK!");

	/* 初始化蓝牙模块 */
	LCD_ShowString(10, 300, 16, (u8 *)"Bluetooth Init ...    ");
	_CMIOT_BLE_Init();
	LCD_ShowString(10, 300, 16, (u8 *)"Bluetooth Init OK!    ");
	
	/* FreeRTOS初始化 */
	LCD_ShowString(10, 300, 16, (u8 *)"FreeRTOS System starting...");

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
	
	_CMIOT_IWDG_Configuration();	/* 初始化看门狗及喂狗定时器 */
	
	/* 初始化电池检测 */
	_CMIOT_BatteryCheckInit();
	
	taskENTER_CRITICAL();   /* 进入临界区 */
	
	/* 创建CLI任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_CliTaskProc,
				(const char*         )"cli_task",
				(uint16_t            )512,
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
		notifyValue = ulTaskNotifyTake(pdTRUE, 10000);   /* 获取任务通知 */
		_CMIOT_Debug("%s(notifyValue: %d)\r\n", __func__, notifyValue);
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
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
	CM_UI_BUSY = false;
	_CMIOT_Debug("%s...\r\n", __func__);
	/* 更新UI界面 */
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
		notifyValue = ulTaskNotifyTake(pdTRUE, 10000);   /* 获取任务通知 */
		
		_CMIOT_Debug("%s(notifyValue: %d)\r\n", __func__, notifyValue);
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
			taskENTER_CRITICAL();   /* 进入临界区 */
			
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
Function Name	:	_CMIOT_BleCmdProcessTaskProc
Author			:	zhaoji
Created Time	:	2018.05.21
Description 	:	蓝牙命令处理任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BleCmdProcessTaskProc(void *pvParameters)
{
	_CMIOT_Debug("%s() ...\r\n", __func__);
	/* 处理接收到的蓝牙数据 */
	_CMIOT_BLE_DataProcess();
	
	while(1)
	{
		if(!BLE_AT_EXE_FLAG)
		{
			_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
			delay_ms(5000);
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
	uint32_t notifyValue;
	/* 接收通知 */
	while(1)
	{
		notifyValue = ulTaskNotifyTake(pdTRUE, 10000);   /* 获取任务通知 */
		
		_CMIOT_Debug("%s(notifyValue: %d)\r\n", __func__, notifyValue);
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
			taskENTER_CRITICAL();   /* 进入临界区 */
			
			/* 删除创建线程 */
			if(bleCmdProcess_task != NULL)
			{
				vTaskDelete(bleCmdProcess_task);   /* 删除任务 */
				bleCmdProcess_task = NULL;
				_CMIOT_Debug("%s(Delete bleCmdProcess task)\r\n", __func__);
			}
			
			/* 创建蓝牙命令处理任务 */
			xTaskCreate((TaskFunction_t      )_CMIOT_BleCmdProcessTaskProc,
						(const char*         )"bleCmdProcess_task",
						(uint16_t            )1024,
						(void*               )NULL,
						(UBaseType_t         )1,
						(TaskHandle_t*       )&bleCmdProcess_task);
						
			taskEXIT_CRITICAL();   /* 退出临界区 */
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_IWDG_ReloadCounter
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	:	复位看门狗计数器
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_IWDG_ReloadCounter(void)
{
	_CMIOT_Debug("%s()\r\n", __func__);
	IWDG_ReloadCounter();
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
void _CMIOT_IWDG_Configuration(void)
{
	TimerHandle_t watchDogTimer;
	/* 喂狗时间计算： T =  (IWDG_Prescaler*Reload)/40(ms)      */
    RCC_LSICmd(ENABLE);	//打开LSI
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);
	
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(1600);	/* 1600*256/40 ≈ 10000ms */
    IWDG_ReloadCounter();
    IWDG_Enable();
	
	/* 每3秒喂一次狗 */
	watchDogTimer = xTimerCreate((const char*   )"WatchDogTimer",
								(TickType_t     )5000,
								(UBaseType_t    )pdTRUE,
								(void*          )1,
								(TimerCallbackFunction_t)_CMIOT_IWDG_ReloadCounter);
								
	xTimerStart(watchDogTimer, 0);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_StartBatteryStateShow
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	:	显示电池电量、充电状态
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_StartBatteryStateShow()
{
	/* 每3秒喂一次狗 */
	TimerHandle_t batteryTimer = xTimerCreate((const char*   )"batteryTimer",
								(TickType_t     )2000,
								(UBaseType_t    )pdTRUE,
								(void*          )3,
								(TimerCallbackFunction_t)_CMIOT_ShowBatteryTips);
								
	xTimerStart(batteryTimer, 0);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BatteryChargeGpioInit
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	:	电池充电管理芯片充电状态IO初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BatteryChargeGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BatteryCheckInit
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	:	初始化电池检测
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BatteryCheckInit(void)
{
	/* 初始化IO */
	_CMIOT_BatteryChargeGpioInit();
	/* 开启检测 */
	// _CMIOT_StartBatteryStateShow();
}

