/*-----------------------------------------------------------------------------
File Name   	:   ui.h
Author          :   zhaoji
Created Time    :   2018.03.12
Description     :   UI接口
-----------------------------------------------------------------------------*/

#ifndef __UI_H__
#define	__UI_H__

#include "stm32f10x.h"

/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GUI_Init
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	GUI界面初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_GUI_Init(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_TabIndex
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	菜单跳转
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_TabIndex(uint8_t newIndex, uint8_t oldIndex);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_UI_BootPage
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	开机欢迎界面
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_UI_BootPage(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_NBIOT_Measurement
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	根据当前选中菜单进行对应的测试，并展示数据到LCD
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_NBIOT_Measurement(uint8_t index);

#endif // __UI_H__


