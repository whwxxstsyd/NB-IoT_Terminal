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

bool _CM_GUI_BUSY_FLAG_ = false;	/* UI界面切换状态忙标识（此时不响应按键请求） */

TaskHandle_t start_task;				/* 开始任务    */
TaskHandle_t cli_task;					/* CLI任务     */
TaskHandle_t keylisten_task;			/* 按键侦听任务   */
TaskHandle_t gui_task;					/* GUI任务     */
TaskHandle_t bluetooth_task;			/* 蓝牙任务    */
TaskHandle_t bleCmdProcess_task = NULL;	/* 蓝牙命令处理任务 */

CM_MENU_POSITION menuPosition = {0, 0, 0, 0, 0, KEYPAD_ENTER};	/* 菜单位置 */

extern bool BLE_AT_EXE_FLAG;			/* 蓝牙执行AT指令标志位 */


/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
**                             Function Declare                               *
**----------------------------------------------------------------------------*/

void _CMIOT_CliTaskProc(void *pvParameters);			/* CLI任务入口函数			*/
void _CMIOT_KeyPressListenTaskProc(void *pvParameters);	/* 按键侦听任务入口函数		*/
void _CMIOT_StartTaskProc(void *pvParameters);			/* 开启任务入口函数			*/
void _CMIOT_BluetoothTaskProc(void *pvParameters);		/* 蓝牙任务入口函数			*/
void _CMIOT_IWDG_Configuration(void);					/* 初始化看门狗				*/
void _CMIOT_BatteryCheckInit(void);						/* 电池检测初始化			*/


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
	/* 按键1S后打开LCD背光，达到长按开机的使用效果 */
	TimerHandle_t blkTimer = xTimerCreate((const char*   )"blkTimer",
										 (TickType_t     )1000,
										 (UBaseType_t    )pdFALSE,
										 (void*          )4,
										 (TimerCallbackFunction_t)_CMIOT_LCD_BlkInit);
	xTimerStart(blkTimer, 0);
	
	/* 初始化开关机按键IO */
	_CMIOT_PowerKey_Init();
	
	/* 初始化看门狗及喂狗定时器 */
	_CMIOT_IWDG_Configuration();
	
	/* 初始Ble蓝牙控制引脚IO */
	_CMIOT_BleCtrlGpioInit();
	
	/* 初始化NB模组电源控制引脚GPIO */
	_CMIOT_M5310PowerGpioInit();
	
	/* 初始化串口 */
	_CMIOT_Uart_Init(UART_CLI_DEBUG, 921600);		/* CLI串口（MCU调试串口） */
	_CMIOT_Uart_Init(UART_M5310, 9600);				/* M5310模组通信串口 */
	_CMIOT_Uart_Init(UART_BLUETOOTH, 57600);		/* 蓝牙模组通信串口 */
	_CMIOT_Debug("%s(UART Init OK!)\r\n", __func__);
	
	/* 检查是否被看门狗重启 */
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		_CMIOT_Debug("\r\n*****<ERROR>REBOOT BECAUSE STM32 IWDG!<ERROR>*****\r\n");
		RCC_ClearFlag();
	}
	
	/* ADC初始化，用于锂电池电压检测 */
	Adc_Init();
	
	/* 初始化LCD */
	LCD_Init();
	
	/* 显示启动界面 */
	_CMIOT_UI_BootPage();
	
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	
	/* M5310初始化 */
	LCD_ShowString(10, 300, 16, (u8 *)"NB-IoT Module Init ...");
	_CMIOT_M5310_Init();
	LCD_ShowString(10, 300, 16, (u8 *)"NB-IoT Module Init OK!");
	
	/* 蓝牙模块初始化 */
	LCD_ShowString(10, 300, 16, (u8 *)"Bluetooth Init ...    ");
	_CMIOT_BLE_Init();
	LCD_ShowString(10, 300, 16, (u8 *)"Bluetooth Init OK!    ");
	
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
	/* 创建按键消息处理任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_KeyPressListenTaskProc,
				(const char*         )"keylisten_task",
				(uint16_t            )512,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&keylisten_task);
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
	/* The output buffers are declared static to keep them off the stack. */
	static uint8_t pcOutputString[ CLI_MAX_OUTPUT_LENGTH ];
	
	_CMIOT_Debug("%s...\r\n", __func__);
	/* 初始化CLI命令行 */
	_CMIOT_Cli_Init();
	
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
			/* 清空当前CLI Buffer */
			memset(UART_CLI_RxBuffer, 0, sizeof(UART_CLI_RxBuffer));
			UART_CLI_RxBufferLen = 0;
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GuiTaskProc
Author			:	zhaoji
Created Time	:	2018.03.16
Description 	:	GUI任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_GuiTaskProc(void *pvParameters)
{
	_CM_GUI_BUSY_FLAG_ = false;
	_CMIOT_Debug("%s...\r\n", __func__);
	/* 根据按键选择情况更新UI界面 */
	_CMIOT_TabIndex(&menuPosition);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_KeyPressListenTaskProc
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	:	按键侦听任务入口函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_KeyPressListenTaskProc(void *pvParameters)
{
	uint32_t notifyValue;
	
	_CMIOT_UserKeypad_Init();	/* 初始化方向、确认、返回按键 */
	
	taskENTER_CRITICAL();   /* 进入临界区 */
	
	/* 创建GUI任务 */
	xTaskCreate((TaskFunction_t      )_CMIOT_GuiTaskProc,
				(const char*         )"gui_task",
				(uint16_t            )512,
				(void*               )NULL,
				(UBaseType_t         )1,
				(TaskHandle_t*       )&gui_task);
				
	taskEXIT_CRITICAL();   /* 退出临界区 */
	
	while(1)
	{
		notifyValue = ulTaskNotifyTake(pdTRUE, 10000);   /* 获取任务通知 */
		
		_CMIOT_Debug("%s(notifyValue: %d)\r\n", __func__, notifyValue);
		
		if(notifyValue == 1)   /* 获取到任务通知 */
		{
			taskENTER_CRITICAL();   /* 进入临界区 */
			
			/* 当用户通过按钮切换功能菜单时，删除重新创建GUI任务 */
			if(gui_task != NULL)
			{
				vTaskDelete(gui_task);   /* 删除任务 */
				_CMIOT_Debug("%s(Delete Lcd task)\r\n", __func__);
			}
			
			/* 创建GUI任务 */
			xTaskCreate((TaskFunction_t      )_CMIOT_GuiTaskProc,
						(const char*         )"gui_task",
						(uint16_t            )512,
						(void*               )NULL,
						(UBaseType_t         )1,
						(TaskHandle_t*       )&gui_task);
						
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
	
	/* 每5秒喂一次狗 */
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
	/* 定时查询电池状态 */
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
	/* 初始化充电芯片状态IO */
	_CMIOT_BatteryChargeGpioInit();
	/* 开启检测 */
	// _CMIOT_StartBatteryStateShow();
}

