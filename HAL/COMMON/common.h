#ifndef __COMMON_H__
#define __COMMON_H__

/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stdbool.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
#define	VERSION_STRING	"NB_Terminal_V01"


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Debug
Author			  :	zhaoji
Created Time	:	2018.01.02
Description 	:	debug函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_Debug(const char *fmt, ...);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_atoi
Author			:	zhaoji
Created Time	:	2017.09
Description 	:	10进制字符串转整数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
int64_t _CMIOT_atoi(const uint8_t *str);




/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Str_StartWith
Author			:	zhaoji
Created Time	:	2018.01.03
Description 	:	判断字符串是否以特定子串开始
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_Str_StartWith(uint8_t *Src, uint8_t *dst);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Str_EndWith
Author			:	zhaoji
Created Time	:	2018.01.03
Description 	:	判断字符串是否以特定子串结束
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_Str_EndWith(uint8_t *Src, uint8_t *dst);




/*-----------------------------------------------------------------------------
Function Name	:	cm_split
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	:	字符串分割，将字符串按分隔符分割后，依次放到二维数据中，并返回分割后的字符串个数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
int cm_split(uint8_t dst[][128], uint8_t *str, const uint8_t *spl);



#endif

