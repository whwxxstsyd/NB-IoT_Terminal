/*-----------------------------------------------------------------------------
File Name   	:   cli.c
Author          :   zhaoji
Created Time    :   2018.02.23
Description     :   CLI接口
-----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "cli.h"
#include "stm32f10x.h"
#include "FreeRTOS_CLI.h"
#include "common.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
//FreeRTOS时间统计所用的节拍计数器
extern volatile unsigned long long FreeRTOSRunTimeTicks;

extern TaskHandle_t start_task;     /* 开始任务   */
extern TaskHandle_t cli_task;       /* CLI任务    */
extern TaskHandle_t m5310_task;     /* M5310任务  */

extern bool DEBUG_FLAG;

/*----------------------------------------------------------------------------*
**                             Function Declare                               *
**----------------------------------------------------------------------------*/
BaseType_t prvDebugLevelCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvGetSysTimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
BaseType_t prvGetHeapUsageCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/
/* 支持的命令列表 */
static CLI_Command_Definition_t CliCommandList[] = 
{
	{
		"at+debuglevel",
		"debuglevel <level>, level can be: 0 or 1\r\n",
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
	
};



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_CliInit
Author			:	zhaoji
Created Time	:	2018.02.28
Description 	:	CLI命令初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_CliInit(void)
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
		strncpy(pcWriteBuffer, "\r\nLevel not support!\r\n", xWriteBufferLen);
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
	msg = pvPortMalloc(32 * sizeof(char));
	
	sprintf(msg, "\r\nFreeHeapSize: %d\r\n", xPortGetFreeHeapSize());	/* 剩余堆大小 */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	sprintf(msg, "CLI TaskHighWaterMark: %ld\r\n", uxTaskGetStackHighWaterMark(cli_task));	/* CLI任务栈空间的高水线(High Water Mark) */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	sprintf(msg, "M5310 TaskHighWaterMark: %ld\r\n", uxTaskGetStackHighWaterMark(m5310_task));	/* M5310任务栈空间的高水线(High Water Mark) */
	strncat(pcWriteBuffer, msg, strlen(msg));
	
	strncat(pcWriteBuffer, "\r\nOK\r\n", strlen("\r\nOK\r\n"));
	
	vPortFree(msg);
	return pdFALSE;
}


