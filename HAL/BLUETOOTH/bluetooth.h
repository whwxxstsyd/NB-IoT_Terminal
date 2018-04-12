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




#endif

