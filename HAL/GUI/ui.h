/*-----------------------------------------------------------------------------
File Name   	:   ui.h
Author          :   zhaoji
Created Time    :   2018.03.12
Description     :   UI接口
-----------------------------------------------------------------------------*/

#ifndef __UI_H__
#define	__UI_H__

#include "stm32f10x.h"
#include "stdio.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/

typedef enum
{
	KEYPAD_UP = 0,
	KEYPAD_DOWN,
	KEYPAD_LEFT,
	KEYPAD_RIGHT,
	KEYPAD_ENTER,
}CM_KEY;


typedef struct
{
	uint8_t xPosition;			/* 首页菜单横坐标 */
	uint8_t yPosition;			/* 首页菜单纵坐标 */
	uint8_t xSubPosition;		/* 子功能菜单纵坐标 */
	uint8_t ySubPosition;		/* 子功能菜单纵坐标 */
	uint8_t	subMenu;			/* 子菜单深度 */
	CM_KEY	pressKey;			/* 当前按下的按键 */
	
}CM_MENU_POSITION;


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GUI_Init
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	GUI界面初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_GUI_Init(int8_t xIndex, int8_t yIndex);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_TabIndex
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	菜单跳转
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_TabIndex(CM_MENU_POSITION *position);


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


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowSignalStrength
Author			:	zhaoji
Created Time	:	2018.03.23
Description 	: 	显示信号强度图标
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowSignalStrength(uint8_t csqValue);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowBatteryLevel
Author			:	zhaoji
Created Time	:	2018.03.29
Description 	: 	显示电池电量
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowBatteryLevel(uint8_t percentValue);



#endif // __UI_H__


