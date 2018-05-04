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
#include "stdbool.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
typedef struct
{
	uint32_t earfcn;
	int32_t  rsrq;
	int32_t snr;
	uint32_t ecl;
	uint8_t cellid[16];
	
}CMIOT_UE_STATE;	/* UESTATS结果结构体 */

typedef struct
{
	uint32_t earfcn;
	uint32_t pci;
	int32_t pri_cell;
	int32_t rsrp;
	int32_t rsrq;
	int32_t rssi;
	int32_t snr;
	
}CMIOT_UE_STATE_CELL;	/* UESTATS:CELL结果结构体 */

typedef struct
{
	uint32_t RLC_UL;
	uint32_t RLC_DL;
	uint32_t MAC_UL;
	uint32_t MAC_DL;
	
}CMIOT_UE_STATE_THP;	/* UESTATS:THP结果结构体 */


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
CMIOT_UE_STATE _CMIOT_M5310_GetUeState(void);


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


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetNetworkDelay
Author			:	zhaoji
Created Time	:	2018.03.16
Description 	: 	获取PING延迟时间
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_GetNetworkDelay(uint8_t *remoteAddr, uint32_t packetSize, uint32_t timeout);


/*-----------------------------------------------------------------------------
Function Name	:	cm_IsNbModuleAlive
Author			:	zhaoji
Created Time	:	2018.03.22
Description 	: 	检测模块是否启动正常
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool cm_IsNbModuleAlive(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetUeCellStats
Author			:	zhaoji
Created Time	:	2018.05.04
Description 	: 	获取UESTATS CELL参数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
CMIOT_UE_STATE_CELL _CMIOT_GetUeCellStats(void);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetUeTHPStats
Author			:	zhaoji
Created Time	:	2018.05.04
Description 	: 	请求UESTATS THP参数（速率）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
CMIOT_UE_STATE_THP _CMIOT_GetUeTHPStats(void);


/*-----------------------------------------------------------------------------
Function Name	:	cm_getAPN
Author			:	zhaoji
Created Time	:	2018.05.04
Description 	: 	获取当前APN
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void cm_getAPN(uint8_t *apnBuf, uint32_t bufLen);








#endif



