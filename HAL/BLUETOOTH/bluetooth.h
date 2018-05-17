/*-----------------------------------------------------------------------------
File Name   	:   m5310.h
Author          :   zhaoji
Created Time    :   2018.02.22
Description     :   M5310接口
-----------------------------------------------------------------------------*/

#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__



/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stdbool.h"


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_Init
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	初始化BLE模块
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_Init(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_DataProcess
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	BLE数据解析处理函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_DataProcess(void);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_ExitPassthroughMode
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	BLE模块退出透传模式（进入命令模式）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_ExitPassthroughMode(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_EnterPassthroughMode
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	进入透传模式（数据模式）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_EnterPassthroughMode(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetBleConnectedState
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	获取蓝牙连接状态
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_GetBleConnectedState(void);



#endif

