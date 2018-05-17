/*-----------------------------------------------------------------------------
File Name   	:   cli.c
Author          :   zhaoji
Created Time    :   2018.02.23
Description     :   CLI接口
-----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "User_Cli.h"
#include "stm32f10x.h"
#include "FreeRTOS_CLI.h"
#include "common.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "usart.h"
#include "ui.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
extern volatile unsigned long long FreeRTOSRunTimeTicks;	/* FreeRTOS时间统计所用的节拍计数器 */

extern TaskHandle_t start_task;     /* 开始任务   */
extern TaskHandle_t cli_task;       /* CLI任务    */
extern TaskHandle_t m5310_task;     /* M5310任务  */
extern TaskHandle_t lcd_task;       /* LCD任务    */

extern bool DEBUG_FLAG;				/* 调试信息打印标志位，为true时将调试信息打印至调试串口 */
extern bool	NB_DEBUG_FLAG;			/* NB模组调试标志位，为true时将串口接收到的模组数据转发至单片机调试串口 */
extern bool BLE_DEBUG_FLAG;			/* BLE蓝牙模组调试标志位，为true时将串口接收到的模组数据转发至单片机调试串口 */

/* USART1(UART_BLUETOOTH)数据接收buffer */
extern uint8_t   UART_BLE_RxBuffer[1024];
extern uint32_t  UART_BLE_RxBufferLen;

extern	CM_MENU_POSITION menuPosition;	/* 菜单坐标信息 */

/*----------------------------------------------------------------------------*
**                             Function Declare                               *
**----------------------------------------------------------------------------*/
BaseType_t prvDebugLevelCommand				(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvGetSysTimeCommand				(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvGetHeapUsageCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvSendAt2NbModuleCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvEnableNbDebugModeCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvSendAt2BleModuleCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvEnableBleDebugModeCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvGetWechatHistoryListCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvGetWechatHistoryDataCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);


/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/
/* 命令列表 */
static CLI_Command_Definition_t CliCommandList[] = 
{
	{
		"at+debuglevel",
		"at+debuglevel <level>, level can be: 0 or 1\r\n",
		prvDebugLevelCommand,
		1
	},
	{
		"at+getsystime",
		"at+getsystime, get system run time(ms)\r\n",
		prvGetSysTimeCommand,
		0
	},
	{
		"at+getheapusage",
		"at+getheapusage, get system total free heap and task stackHighWaterMark\r\n",
		prvGetHeapUsageCommand,
		0
	},
	{
		"[AT2NB]",
		"[AT2NB] <AT CMD>, Send AT Command to NB Module\r\n",
		prvSendAt2NbModuleCommand,
		1
	},
	{
		"at+nbenable",
		"at+nbenable <state>, state can be: 0 or 1, enable nb module serial data\r\n",
		prvEnableNbDebugModeCommand,
		1
	},
	{
		"[AT2BLE]",
		"[AT2BLE] <AT CMD>, Send AT Command to BLE Module\r\n",
		prvSendAt2BleModuleCommand,
		1
	},
	{
		"at+bleenable",
		"at+bleenable <state>, state can be: 0 or 1, enable ble module serial data\r\n",
		prvEnableBleDebugModeCommand,
		1
	},
	{
		"at+getWechatHistoryList",
		"at+getWechatHistoryList, get wechat recordlist\r\n",
		prvGetWechatHistoryListCommand,
		0
	},
	{
		"at+getWechatHistoryData",
		"at+getWechatHistoryData, get wechar record data\r\n",
		prvGetWechatHistoryDataCommand,
		1
	},
};



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_CLI_Init
Author			:	zhaoji
Created Time	:	2018.02.28
Description 	:	CLI命令初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_CLI_Init(void)
{
	uint8_t i;
	/* 注册所有支持的命令 */
	for(i=0; i<sizeof(CliCommandList)/sizeof(CliCommandList[0]); i++)
	{
		FreeRTOS_CLIRegisterCommand(&CliCommandList[i]);
	}
}




/*-----------------------------------------------------------------------------
Function Name	:	prvDebugLevelCommand
Author			:	zhaoji
Created Time	:	2018.02.28
Description 	:	配置debug等级命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvDebugLevelCommand(char *pcWriteBuffer,
                                size_t xWriteBufferLen,
                                const char *pcCommandString)
{
	BaseType_t paraLen;
	const uint8_t *debuglevel_str;
	debuglevel_str = (const uint8_t *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &paraLen);	/* 获取参数 */
	if(paraLen != 1)
	{
		strncpy(pcWriteBuffer, "\r\nParameter number not support!\r\n\r\nOK\r\n", xWriteBufferLen);
		return pdFALSE;
	}
	
	if(_CMIOT_atoi(debuglevel_str) == 0)
	{
		DEBUG_FLAG = false;
	}
	else if(_CMIOT_atoi(debuglevel_str) == 1)
	{
		DEBUG_FLAG = true;
	}
	else
	{
		strncpy(pcWriteBuffer, "\r\nLevel not support!\r\n", xWriteBufferLen);
	}
	
	strncat(pcWriteBuffer, "\r\nOK\r\n", strlen("\r\nOK\r\n"));
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvGetSysTimeCommand
Author			:	zhaoji
Created Time	:	2018.02.28
Description 	:	获取系统运行时间 命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvGetSysTimeCommand(char *pcWriteBuffer,
                                size_t xWriteBufferLen,
                                const char *pcCommandString)
{
	char *msg;
	msg = pvPortMalloc(64 * sizeof(char));
	sprintf(msg, "\r\n+Runtime: %lld ms\r\n\r\nOK\r\n", FreeRTOSRunTimeTicks);	/* 返回系统运行时间信息 */
	strncpy(pcWriteBuffer, msg, xWriteBufferLen);
	vPortFree(msg);
	return pdFALSE;
}



/*-----------------------------------------------------------------------------
Function Name	:	prvGetHeapUsageCommand
Author			:	zhaoji
Created Time	:	2018.02.28
Description 	:	获取系统堆栈信息 命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvGetHeapUsageCommand(char *pcWriteBuffer,
                                size_t xWriteBufferLen,
                                const char *pcCommandString)
{
	char *msg;
	msg = pvPortMalloc(64 * sizeof(char));
	
	sprintf(msg, "\r\nFreeHeapSize: %d\r\n", xPortGetFreeHeapSize());	/* 剩余堆大小 */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	sprintf(msg, "CLI TaskHighWaterMark: %ld\r\n", uxTaskGetStackHighWaterMark(cli_task));		/* CLI任务栈空间的高水线(High Water Mark) */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	sprintf(msg, "M5310 TaskHighWaterMark: %ld\r\n", uxTaskGetStackHighWaterMark(m5310_task));	/* M5310任务栈空间的高水线(High Water Mark) */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	sprintf(msg, "LCD TaskHighWaterMark: %ld\r\n", uxTaskGetStackHighWaterMark(lcd_task));		/* LCD任务栈空间的高水线(High Water Mark) */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	strncat(pcWriteBuffer, "\r\nOK\r\n", strlen("\r\nOK\r\n"));
	
	vPortFree(msg);
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvSendAt2ModuleCommand
Author			:	zhaoji
Created Time	:	2018.04.02
Description 	:	发送指令到模组 命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvSendAt2NbModuleCommand(char *pcWriteBuffer,
								size_t xWriteBufferLen,
								const char *pcCommandString)
{
	BaseType_t paraLen;
	const uint8_t *at_str;
	at_str = (const uint8_t *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &paraLen);	/* 获取参数 */
	/* 发送AT指令 */
	taskENTER_CRITICAL();   /* 进入临界区 */
	_CMIOT_Uart_send(UART_M5310, (uint8_t *)at_str, strlen((char *)at_str));
	_CMIOT_Uart_send(UART_M5310, (uint8_t *)"\r\n", strlen("\r\n"));
	taskEXIT_CRITICAL();   /* 退出临界区 */
	
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvEnableNbDebugModeCommand
Author			:	zhaoji
Created Time	:	2018.04.02
Description 	:	设置NB模组调试模式 命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvEnableNbDebugModeCommand(char *pcWriteBuffer,
									size_t xWriteBufferLen,
									const char *pcCommandString)
{
	BaseType_t paraLen;
	const uint8_t *state_str;
	state_str = (const uint8_t *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &paraLen);	/* 获取参数 */
	if(paraLen != 1)
	{
		strncpy(pcWriteBuffer, "\r\nParameter number not support!\r\n\r\nOK\r\n", xWriteBufferLen);
		return pdFALSE;
	}
	
	if(_CMIOT_atoi(state_str) == 0)
	{
		NB_DEBUG_FLAG = false;
	}
	else if(_CMIOT_atoi(state_str) == 1)
	{
		NB_DEBUG_FLAG = true;
	}
	else
	{
		strncpy(pcWriteBuffer, "\r\nState not support!\r\n", xWriteBufferLen);
	}
	
	strncat(pcWriteBuffer, "\r\nOK\r\n", strlen("\r\nOK\r\n"));
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvSendAt2ModuleCommand
Author			:	zhaoji
Created Time	:	2018.04.02
Description 	:	发送指令到模组 命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvSendAt2BleModuleCommand(char *pcWriteBuffer,
									size_t xWriteBufferLen,
									const char *pcCommandString)
{
	BaseType_t paraLen;
	const uint8_t *at_str;
	at_str = (const uint8_t *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &paraLen);	/* 获取参数 */
	/* 发送AT指令 */
	taskENTER_CRITICAL();   /* 进入临界区 */
	_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)at_str, strlen((char *)at_str));
	_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)"\r\n", strlen("\r\n"));
	taskEXIT_CRITICAL();   /* 退出临界区 */
	
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvEnableBleDebugModeCommand
Author			:	zhaoji
Created Time	:	2018.04.02
Description 	:	设置BLE蓝牙模组调试模式 命令回调
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvEnableBleDebugModeCommand(char *pcWriteBuffer,
										size_t xWriteBufferLen,
										const char *pcCommandString)
{
	BaseType_t paraLen;
	const uint8_t *state_str;
	state_str = (const uint8_t *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &paraLen);	/* 获取参数 */
	if(paraLen != 1)
	{
		strncpy(pcWriteBuffer, "\r\nParameter number not support!\r\n\r\nOK\r\n", xWriteBufferLen);
		return pdFALSE;
	}
	
	if(_CMIOT_atoi(state_str) == 0)
	{
		BLE_DEBUG_FLAG = false;
	}
	else if(_CMIOT_atoi(state_str) == 1)
	{
		BLE_DEBUG_FLAG = true;
	}
	else
	{
		strncpy(pcWriteBuffer, "\r\nState not support!\r\n", xWriteBufferLen);
	}
	
	strncat(pcWriteBuffer, "\r\nOK\r\n", strlen("\r\nOK\r\n"));
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvGetWechatHistoryListCommand
Author			:	zhaoji
Created Time	:	2018.05.09
Description 	:	获取微信小程序测试记录列表
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvGetWechatHistoryListCommand(char *pcWriteBuffer,
										size_t xWriteBufferLen,
										const char *pcCommandString)
{
	if((menuPosition.xPosition + menuPosition.yPosition * 3) == 5 && menuPosition.subMenu == 1)
	{
		_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)"<Request><historyList>list</historyList></Request>", \
			strlen((const char *)"<Request><historyList>list</historyList></Request>"));
	}
	else
	{
		strncpy(pcWriteBuffer, "\r\nNot Under Ble Mode\r\n\r\nERROR\r\n", xWriteBufferLen);
	}
	return pdFALSE;
}


/*-----------------------------------------------------------------------------
Function Name	:	prvGetWechatHistoryDataCommand
Author			:	zhaoji
Created Time	:	2018.05.09
Description 	:	获取微信小程序测试数据
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
BaseType_t prvGetWechatHistoryDataCommand(char *pcWriteBuffer,
										size_t xWriteBufferLen,
										const char *pcCommandString)
{
	BaseType_t paraLen;
	uint8_t *str;
	str = (uint8_t *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &paraLen);	/* 获取参数 */
	if(paraLen != 1)
	{
		strncpy(pcWriteBuffer, "\r\nParameter number not support!\r\n\r\nERROR\r\n", xWriteBufferLen);
		return pdFALSE;
	}
	
	if((menuPosition.xPosition + menuPosition.yPosition * 3) == 5 && menuPosition.subMenu == 1)
	{
		_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)"<Request><historyData>", strlen((const char *)"<Request><historyData>"));
		_CMIOT_Uart_send(UART_BLUETOOTH, str, strlen((const char*)str));
		_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)"</historyData></Request>", strlen((const char *)"</historyData></Request>"));
	}
	else
	{
		strncpy(pcWriteBuffer, "\r\nNot Under Ble Mode\r\n\r\nERROR\r\n", xWriteBufferLen);
	}
	return pdFALSE;
}









