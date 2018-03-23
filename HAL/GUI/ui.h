/*-----------------------------------------------------------------------------
File Name   	:   ui.h
Author          :   zhaoji
Created Time    :   2018.03.12
Description     :   UI接口
-----------------------------------------------------------------------------*/

#ifndef __UI_H__
#define	__UI_H__

#include "stm32f10x.h"


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
typedef struct
{
	int8_t xIndex;	/* 菜单横坐标 */
	int8_t yIndex;	/* 菜单纵坐标 */
	int8_t zIndex;	/* 子菜单深度 */
	
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
void _CMIOT_GUI_Init(int8_t x, int8_t y, int8_t z);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_TabIndex
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	菜单跳转
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_TabIndex(int8_t xNew, int8_t yNew, int8_t zNew, int8_t xOld, int8_t yOld, int8_t zOld);


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



#endif // __UI_H__


