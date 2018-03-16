/*-----------------------------------------------------------------------------
File Name   	:   m5310.h
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


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
typedef struct
{
	uint32_t earfcn;
	int32_t  rsrq;
	uint32_t snr;
	uint32_t ecl;
	uint8_t cellid[16];
	
}CMIOT_UE_State;


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ExecuteAtCmd
Author			:	zhaoji
Created Time	:	2018.02.22
Description 	: 	执行AT指令
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_ExecuteAtCmd(uint8_t *AtCmd, uint8_t MatchRsp[][20], uint8_t MatchRsp_Num, uint32_t timeout_ms);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetRegisterTime
Author			:	zhaoji
Created Time	:	2018.02.22
Description 	: 	获取M5310驻网时间
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_M5310_GetRegisterTime(void);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetSignalstrength
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取M5310 CSQ值
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_M5310_GetSignalstrength(void);




/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetUeState
Author			:	zhaoji
Created Time	:	2018.02.26
Description 	: 	获取M5310 频点、RSRQ、SNR、ECL等射频参数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
CMIOT_UE_State _CMIOT_M5310_GetUeState(void);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetModuleName
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取模组型号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetModuleName(uint8_t *ModuleName, uint32_t buffersize);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetModuleVersion
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取模组版本号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetModuleVersion(uint8_t *ModuleVersion, uint32_t buffersize);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetICCID
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取ICCID号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetICCID(uint8_t *ICCID, uint32_t buffersize);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetIMSI
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取IMSI号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetIMSI(uint8_t *IMSI, uint32_t buffersize);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetIMEI
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取IMSI号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetIMEI(uint8_t *IMEI, uint32_t buffersize);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetPLMN
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取PLMN号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetPLMN(uint8_t *plmn, uint32_t buffersize);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetNB_Band
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取频段号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_GetNB_Band(void);



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Get_PSM_TIMER_Value
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取T3412 T3324定时器内容
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_Get_PSM_TIMER_Value(uint8_t *t3324, uint8_t *t3412, uint32_t buffersize);








#endif
