/*-----------------------------------------------------------------------------
File Name   	  :   m5310.h
Author          :   zhaoji
Created Time    :   2018.02.22
Description     :   M5310接口
-----------------------------------------------------------------------------*/

#ifndef __M5310_H__
#define __M5310_H__



/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ExecuteAtCmd
Author			  :	zhaoji
Created Time	:	2018.02.22
Description 	: 执行AT指令
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_ExecuteAtCmd(uint8_t *AtCmd, uint8_t MatchRsp[][20], uint8_t MatchRsp_Num, uint32_t timeout_ms);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetRegisterTime
Author			  :	zhaoji
Created Time	:	2018.02.22
Description 	: 获取M5310驻网时间
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_M5310_GetRegisterTime(void);






#endif
