/*-----------------------------------------------------------------------------
File Name   	:   m5310.c
Author          :   zhaoji
Created Time    :   2018.02.22
Description     :   M5310接口
-----------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------*
                               Dependencies                                   *
------------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stdbool.h"
#include "common.h"
#include "usart.h"
#include "string.h"
#include "delay.h"
#include "m5310.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
#define	ATTACH_TIMEOUT		(2*60*1000)

/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART3(UART_M5310)数据接收buffer */
uint8_t   UART_M5310_RxBuffer[512] = {0};
uint32_t  UART_M5310_RxBufferLen   =  0;

// uint8_t PING_ADDR[] = "114.114.114.114";

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetRegisterTime
Author			:	zhaoji
Created Time	:	2018.02.22
Description 	: 	获取M5310驻网时间
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_M5310_GetRegisterTime(void)
{
	uint8_t boot_MatchStr[2][20] = {"OK", ""};	/* 指令响应完成匹配字符串 */
	uint8_t attach_MatchStr[2][20] = {"CGATT:0", "CGATT:1"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint32_t start_time;
	uint32_t end_time;
	
	result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NRB\r\n"), boot_MatchStr, 1, 10000);   /* 重启模组，并等待启动信息 */
	if(result == 1)
	{
		start_time = FreeRTOSRunTimeTicks;   /* M5310启动完成时间点 */
		while(1)
		{
			result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CGATT?\r\n"), attach_MatchStr, 2, 5000);
			if(result == 2)
			{
				break;
			}
			if(FreeRTOSRunTimeTicks - start_time > ATTACH_TIMEOUT)
			{
				_CMIOT_Debug("%s(Attach timeout!)\r\n", __func__);
				return 1;
			}
			delay_ms(500);
		}
		end_time = FreeRTOSRunTimeTicks;   /* M5310附着成功时间点 */
		_CMIOT_Debug("%s(return: %ld ms)\r\n", __func__, end_time - start_time);
		return end_time - start_time;   /* 返回附着网络所用时间（单位：毫秒），附着成功时间点 - 启动完成时间点 */
	}
	else   /* 等待启动信息超时 */
	{
		_CMIOT_Debug("%s(Wait boot information timeout!)\r\n", __func__);
		return 0;
	}
}




/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ExecuteAtCmd
Author			:	zhaoji
Created Time	:	2018.02.22
Description 	: 	执行AT指令
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_ExecuteAtCmd(uint8_t *AtCmd, uint8_t MatchRsp[][20], uint8_t MatchRsp_Num, uint32_t timeout_ms)
{
	uint8_t i;
	uint32_t retryCounts = 0;        /* 检索次数 */
	uint32_t timeslice_ms = 100;     /* 检索M5310串口响应时间精度 */
	
	memset(UART_M5310_RxBuffer, 0, sizeof(UART_M5310_RxBuffer));     /* 清空接收Buffer */
	UART_M5310_RxBufferLen = 0;
	
	_CMIOT_Uart_send(UART_M5310, AtCmd, strlen((const char *)AtCmd));    /* 发送AT指令 */
		
	_CMIOT_Debug("%s() %s\r\n", __func__, AtCmd);
	
	/* 检索M5310串口返回内容是否匹配目标字符串数组，匹配成功返回数组索引号+1，超时匹配失败返回0 */
	while(1)
	{
		for(i=0; i<MatchRsp_Num; i++)
		{
			if(strstr((const char *)UART_M5310_RxBuffer, (const char *)MatchRsp[i]) != NULL)  /* 检索匹配字符串 */
			{
				_CMIOT_Debug("%s(return: %d)\r\n", __func__, i+1);
				return i + 1;
			}
		}
		
		if(retryCounts * timeslice_ms >= timeout_ms)   /* 匹配超时返回 */
		{
			_CMIOT_Debug("%s(execute timeout!)\r\n", __func__);
			return 0;
		}
		
		delay_ms(timeslice_ms);
		retryCounts++;
	}
}



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetSignalstrength
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取M5310 CSQ值
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_M5310_GetSignalstrength(void)
{
	uint8_t csq_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	char *csq;
	uint8_t signal = 0;
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CSQ\r\n"), csq_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			csq = strstr((const char *)UART_M5310_RxBuffer, "CSQ:");

			csq += strlen("CSQ:");
			while(1)
			{
				if(*csq == ' ')	/* 忽略空格 */
				{
					csq++;
					continue;
				}
				
				if(*csq >= '0' && *csq <= '9')	/* 计算信号值 */
				{
					signal = signal*10 + *csq - '0';
					csq++;
				}
				else
				{
					break;
				}
			}
			/* 返回信号值 */
			_CMIOT_Debug("%s(%d)\r\n", __func__, signal);
			return signal;
		}
		delay_ms(500);
	}
	/* 获取信号 执行失败 */
	_CMIOT_Debug("%s(Failed!)\r\n", __func__);
	return 0;
}




/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_GetUeState
Author			:	zhaoji
Created Time	:	2018.02.26
Description 	: 	获取M5310 频点、RSRQ、SNR、ECL等射频参数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
CMIOT_UE_STATE _CMIOT_M5310_GetUeState(void)
{
	char *index;
	uint8_t maxRetryCounts = 5;
	uint32_t result;
	uint8_t uestate_MatchStr[2][20] = {"OK","ERROR"};	/* 指令响应完成匹配字符串 */
	bool flag = false;
	char *pSta, *pEnd;
	CMIOT_UE_STATE ue_state = {0,0,0,0,""};
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NUESTATS\r\n"), uestate_MatchStr, 2, 2000);
		
		if(result == 1)
		{
			/* 从UESTATE信息中获取NB频点 */
			index = strstr((const char *)UART_M5310_RxBuffer, "EARFCN:");
			if(index != NULL)
			{
				index += strlen("EARFCN:");
				flag = false;
				while(1)
				{
					if(*index == ' ')	/* 忽略空格 */
					{
						index++;
						continue;
					}
					
					if(*index == '-')	/* 正负 */
					{
						flag = true;
						index++;
						continue;
					}
					
					if(*index >= '0' && *index <= '9')	/* 计算频点值 */
					{
						ue_state.earfcn = ue_state.earfcn*10 + *index - '0';
						index++;
					}
					else
					{
						break;
					}
				}
				ue_state.earfcn = ue_state.earfcn * (flag ? -1 : 1);
				_CMIOT_Debug("%s(EARFCN: %ld)\r\n", __func__, ue_state.earfcn);
			}
			else
			{
				_CMIOT_Debug("%s(EARFCN Not Found!)\r\n", __func__);
			}
			
			/* 从UESTATE信息中获取RSRQ */
			index = strstr((const char *)UART_M5310_RxBuffer, "RSRQ:");
			if(index != NULL)
			{
				index += strlen("RSRQ:");
				flag = false;
				while(1)
				{
					if(*index == ' ')	/* 忽略空格 */
					{
						index++;
						continue;
					}
					
					if(*index == '-')	/* 正负 */
					{
						flag = true;
						index++;
						continue;
					}
					
					if(*index >= '0' && *index <= '9')	/* 计算RSRQ绝对值 */
					{
						ue_state.rsrq = ue_state.rsrq*10 + *index - '0';
						index++;
					}
					else
					{
						break;
					}
				}
				ue_state.rsrq = ue_state.rsrq * (flag ? -1 : 1);
				_CMIOT_Debug("%s(RSRQ: %ld)\r\n", __func__, ue_state.rsrq);
			}
			else
			{
				_CMIOT_Debug("%s(RSRQ Not Found!)\r\n", __func__);
			}
			
			/* 从UESTATE信息中获取SNR */
			index = strstr((const char *)UART_M5310_RxBuffer, "SNR:");
			if(index != NULL)
			{
				index += strlen("SNR:");
				flag = false;
				while(1)
				{
					if(*index == ' ')	/* 忽略空格 */
					{
						index++;
						continue;
					}
					
					if(*index == '-')	/* 正负 */
					{
						flag = true;
						index++;
						continue;
					}
					
					if(*index >= '0' && *index <= '9')	/* 计算SNR绝对值 */
					{
						ue_state.snr = ue_state.snr*10 + *index - '0';
						index++;
					}
					else
					{
						break;
					}
				}
				ue_state.snr = ue_state.snr * (flag ? -1 : 1);
				_CMIOT_Debug("%s(SNR: %ld)\r\n", __func__, ue_state.snr);
			}
			else
			{
				_CMIOT_Debug("%s(SNR Not Found!)\r\n", __func__);
			}
			
			/* 从UESTATE信息中获取ECL */
			index = strstr((const char *)UART_M5310_RxBuffer, "ECL:");
			if(index != NULL)
			{
				index += strlen("ECL:");
				flag = false;
				while(1)
				{
					if(*index == ' ')	/* 忽略空格 */
					{
						index++;
						continue;
					}
					
					if(*index == '-')	/* 正负 */
					{
						flag = true;
						index++;
						continue;
					}
					
					if(*index >= '0' && *index <= '9')	/* 计算ECL绝对值 */
					{
						ue_state.ecl = ue_state.ecl*10 + *index - '0';
						index++;
					}
					else
					{
						break;
					}
				}
				ue_state.ecl = ue_state.ecl * (flag ? -1 : 1);
				_CMIOT_Debug("%s(ECL: %ld)\r\n", __func__, ue_state.ecl);
			}
			else
			{
				_CMIOT_Debug("%s(ECL Not Found!)\r\n", __func__);
			}
			
			/* CELL ID*/
			pSta = strstr((const char *)UART_M5310_RxBuffer, "Cell ID:") + strlen("Cell ID:");
			if(pSta != NULL)
			{
				pEnd = strstr(pSta, "\r\n");
				memcpy(ue_state.cellid, pSta, pEnd-pSta);
				_CMIOT_Debug("%s(CELLID: %s)\r\n", __func__, ue_state.cellid);
			}
			else
			{
				memcpy(ue_state.cellid, "Invalid", strlen("Invalid"));
				_CMIOT_Debug("%s(CELLID Not Found!)\r\n", __func__);
			}
			return ue_state;;
		}
		else
		{
			_CMIOT_Debug("%s(AT RSP ERROR!)\r\n", __func__);
		}
	}
	strncpy((char *)ue_state.cellid, "ERROR", sizeof(ue_state.cellid));
	return ue_state;
}



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetModuleName
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取模组型号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetModuleName(uint8_t *ModuleName, uint32_t buffersize)
{
	uint8_t cgmm_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	
	memset(ModuleName, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CGMM\r\n"), cgmm_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = (char *)UART_M5310_RxBuffer;
			while(1)
			{
				if(*p_head == ' ' || *p_head == '\r' || *p_head == '\n')
				{
					p_head++;
				}
				else{break;}
			}
			p_end  = strstr((const char *)p_head, "\r\n\r\nOK");
			strncat((char *)ModuleName, p_head, p_end - p_head);
			_CMIOT_Debug("%s(%s)\r\n", __func__, ModuleName);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)ModuleName, "ERROR", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetModuleVersion
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取模组版本号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetModuleVersion(uint8_t *ModuleVersion, uint32_t buffersize)
{
	uint8_t cmver_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	
	memset(ModuleVersion, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CMVER\r\n"), cmver_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = (char *)UART_M5310_RxBuffer;
			while(1)
			{
				if(*p_head == ' ' || *p_head == '\r' || *p_head == '\n')
				{
					p_head++;
				}
				else{break;}
			}
			p_end  = strstr((const char *)p_head, "\r\n\r\nOK\r\n");
			strncat((char *)ModuleVersion, p_head, p_end - p_head);
			_CMIOT_Debug("%s(%s)\r\n", __func__, ModuleVersion);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)ModuleVersion, "ERROR", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetICCID
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取ICCID号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetICCID(uint8_t *ICCID, uint32_t buffersize)
{
	uint8_t ccid_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	
	memset(ICCID, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NCCID\r\n"), ccid_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, "NCCID:") + strlen("NCCID:");
			p_end  = strstr((const char *)p_head, "\r\n\r\nOK\r\n");
			strncat((char *)ICCID, p_head, p_end - p_head);
			_CMIOT_Debug("%s(%s)\r\n", __func__, ICCID);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)ICCID, "NO SIM", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetIMSI
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取IMSI号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetIMSI(uint8_t *IMSI, uint32_t buffersize)
{
	uint8_t imsi_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	
	memset(IMSI, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CIMI\r\n"), imsi_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = (char *)UART_M5310_RxBuffer;
			while(1)
			{
				if(*p_head == ' ' || *p_head == '\r' || *p_head == '\n')
				{
					p_head++;
				}
				else{break;}
			}
			p_end  = strstr((const char *)p_head, "\r\n");
			strncat((char *)IMSI, p_head, p_end - p_head);
			_CMIOT_Debug("%s(%s)\r\n", __func__, IMSI);
			return 1;
		}
		delay_ms(2000);
	}
	/* 获取失败 */
	strncpy((char *)IMSI, "NO SIM", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetIMEI
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	: 	获取IMEI号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetIMEI(uint8_t *IMEI, uint32_t buffersize)
{
	uint8_t imei_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	
	while(maxRetryCounts > 0)
	{
		memset(IMEI, 0, buffersize);
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CGSN=1\r\n"), imei_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, "CGSN:") + strlen("CGSN:");
			p_end  = strstr((const char *)p_head, "\r\n\r\nOK\r\n");
			strncat((char *)IMEI, p_head, p_end - p_head);
			_CMIOT_Debug("%s(%s)\r\n", __func__, IMEI);
			if(_CMIOT_Str_StartWith(IMEI, (uint8_t *)"8658"))
			{
				_CMIOT_Debug("%s(OK)\r\n", __func__);
				return 1;
			}
		}
		delay_ms(1000);
	}
	/* 获取失败 */
	strncpy((char *)IMEI, "NO Module", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetPLMN
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取PLMN号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_GetPLMN(uint8_t *plmn, uint32_t buffersize)
{
	uint8_t cops_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	
	memset(plmn, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+COPS?\r\n"), cops_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, "\"");
			if(p_head == NULL)
			{
				return 0;
			}
			p_head++;
			p_end  = strstr((const char *)p_head, "\"");
			/* 当前PLMN信息为空 */
			if(p_end == p_head)
			{
				return 0;
			}
			/* 提取plmn内容 */
			strncat((char *)plmn, p_head, p_end - p_head);
			_CMIOT_Debug("%s(%s)\r\n", __func__, plmn);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	// strncpy((char *)plmn, "ERROR", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetNB_Band
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取频段号
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_GetNB_Band(void)
{
	uint8_t band_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *index;
	bool flag;
	uint32_t band = 0;
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NBAND?\r\n"), band_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			index = strstr((char *)UART_M5310_RxBuffer, "+NBAND:");
			if(index != NULL)
			{
				index += strlen("+NBAND:");
				flag = false;
				while(1)
				{
					if(*index == ' ')	/* 忽略空格 */
					{
						index++;
						continue;
					}
					
					if(*index == '-')	/* 正负 */
					{
						flag = true;
						index++;
						continue;
					}
					
					if(*index >= '0' && *index <= '9')	/* 计算BAND绝对值 */
					{
						band = band*10 + *index - '0';
						index++;
					}
					else
					{
						break;
					}
				}
				band = band * (flag ? -1 : 1);
				_CMIOT_Debug("%s(%d)\r\n", __func__, band);
				return band;
			}
			else
			{
				_CMIOT_Debug("%s(BAND Not Found!)\r\n", __func__);
			}
		}
		delay_ms(500);
	}
	/* 获取失败 */
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Get_PSM_TIMER_Value
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取T3412 T3324定时器内容
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_Get_PSM_TIMER_Value(uint8_t *t3324, uint8_t *t3412, uint32_t buffersize)
{
	uint8_t cpsms_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head, *p_end;
	char unit[10] = {0};
	uint32_t timerValue;
	memset(t3324, 0, buffersize);
	memset(t3412, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CPSMS?\r\n"), cpsms_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			_CMIOT_Debug("%s(%s)\r\n", __func__, UART_M5310_RxBuffer);
			/* T3412 */
			p_head = strstr((const char *)UART_M5310_RxBuffer, ",") + strlen(",");
			p_head = strstr((const char *)p_head, ",") + strlen(",");
			p_head = strstr((const char *)p_head, ",") + strlen(",");
			p_end  = strstr((const char *)p_head, ",");
			
			if(strncmp(p_head, "000", strlen("000")) == 0) { strncpy(unit, "10min", sizeof(unit)); }
			else if(strncmp(p_head, "001", strlen("001")) == 0) { strncpy(unit, "1h", sizeof(unit)); }
			else if(strncmp(p_head, "010", strlen("010")) == 0) { strncpy(unit, "10h", sizeof(unit)); }
			else if(strncmp(p_head, "011", strlen("011")) == 0) { strncpy(unit, "2s", sizeof(unit)); }
			else if(strncmp(p_head, "100", strlen("100")) == 0) { strncpy(unit, "30s", sizeof(unit)); }
			else if(strncmp(p_head, "101", strlen("101")) == 0) { strncpy(unit, "1min", sizeof(unit)); }
			else if(strncmp(p_head, "110", strlen("110")) == 0) { strncpy(unit, "320h", sizeof(unit)); }
			else if(strncmp(p_head, "111", strlen("111")) == 0) { strncpy(unit, "Invalid", sizeof(unit)); }
			else { strncpy(unit, "Invalid", sizeof("unknow")); };
			
			p_head += 3;
			timerValue = 0;
			while(p_head < p_end)
			{
				timerValue = timerValue * 2 + *p_head - '0';
				p_head++;
			}
			
			sprintf((char *)t3412, "%d * %s", timerValue, unit);
			
			/* T3324 */
			p_head = strstr((const char *)p_head, ",") + strlen(",");
			p_end  = strstr((const char *)p_head, "\r\n");
			
			if(strncmp(p_head, "000", strlen("000")) == 0) { strncpy(unit, "2s", sizeof(unit)); }
			else if(strncmp(p_head, "001", strlen("001")) == 0) { strncpy(unit, "1min", sizeof(unit)); }
			else if(strncmp(p_head, "010", strlen("010")) == 0) { strncpy(unit, "decihours", sizeof(unit)); }
			else if(strncmp(p_head, "111", strlen("111")) == 0) { strncpy(unit, "Invalid", sizeof(unit)); }
			else { strncpy(unit, "Invalid", sizeof("unknow")); };
			
			timerValue = 0;
			p_head += 3;
			while(p_head < p_end)
			{
				timerValue = timerValue * 2 + *p_head - '0';
				p_head++;
			}
			sprintf((char *)t3324, "%d * %s", timerValue, unit);
			
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)t3412, "ERROR", buffersize);
	strncpy((char *)t3324, "ERROR", buffersize);
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetNetworkDelay
Author			:	zhaoji
Created Time	:	2018.03.16
Description 	: 	获取PING延迟时间
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_GetNetworkDelay(uint8_t *remoteAddr, uint32_t packetSize, uint32_t timeout)
{
	uint8_t nping_MatchStr[3][20] = {"NPING:", "NPINGERR","ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *index;
	uint32_t pingDelay = 0;
	uint8_t pingAtCmd[128] = {0};
	
	sprintf((char *)pingAtCmd, "AT+NPING=%s,%d,%d,1\r\n",remoteAddr, packetSize, timeout);
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd(pingAtCmd, nping_MatchStr, 3, 12000);
		
		if(result == 1)	/* 指令执行OK */
		{
			delay_ms(1000);
			index = strstr((const char *)UART_M5310_RxBuffer, "=") + strlen("=");
			while(1)
			{
				if(*index == ' ')
				{
					index++;
					continue;
				}
				
				if(*index >= '0' && *index <= '9')
				{
					pingDelay = pingDelay*10 + *index - '0';
					index++;
					continue;
				}
				else
				{
					break;
				}
			}
			_CMIOT_Debug("%s(%d ms)\r\n", __func__, pingDelay);
			return pingDelay;
		}
		
		if(result ==2)
		{
			_CMIOT_Debug("%s(NPINGERR!)\r\n", __func__);
			return 0;
		}
		
		delay_ms(500);
	}
	/* 获取失败 */
	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	cm_IsNbModuleAlive
Author			:	zhaoji
Created Time	:	2018.03.22
Description 	: 	检测模块是否启动正常
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool cm_IsNbModuleAlive(void)
{
	uint8_t at_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT\r\n"), at_MatchStr, 2, 2000);
		
		if(result == 1)
		{
			/* 检测通过 */
			_CMIOT_Debug("%s(alive!)\r\n", __func__);
			return true;
		}
		delay_ms(1000);
	}
	_CMIOT_Debug("%s(execute fail!)\r\n", __func__);
	return false;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetUeCellStats
Author			:	zhaoji
Created Time	:	2018.05.04
Description 	: 	获取UESTATS CELL参数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
CMIOT_UE_STATE_CELL _CMIOT_GetUeCellStats(void)
{
	uint8_t at_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	CMIOT_UE_STATE_CELL ue_state_cell = {0,0,0,0,0,0};
	char *cellStateStr;
	char *p_head;
	char *p_tail;
	uint8_t res_array[7][32] = {0};
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NUESTATS=CELL\r\n"), at_MatchStr, 2, 5000);
		
		if(result == 1)
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, "NUESTATS:CELL,");
			if(p_head != NULL)  /* 检索匹配字符串 */
			{
				_CMIOT_Debug("%s(Get cell uestats ok)\r\n", __func__);
				p_head += strlen("NUESTATS:CELL,");
				p_tail = strstr(p_head, "\r\n");
				cellStateStr = pvPortMalloc((p_tail-p_head+1) * sizeof(char));	/* 申请内存 */
				memcpy(cellStateStr, p_head, p_tail - p_head);
				if(cm_split(res_array, (uint8_t *)cellStateStr, (uint8_t *)(",")) == 7)	/* 判断参数数量是否正确 */
				{
					ue_state_cell.earfcn	= _CMIOT_atoi(res_array[0]);	/* 频点 */
					ue_state_cell.pci		= _CMIOT_atoi(res_array[1]);	/* PCI */
					ue_state_cell.pri_cell	= _CMIOT_atoi(res_array[2]);	/* primary cell */
					ue_state_cell.rsrp		= _CMIOT_atoi(res_array[3]);	/* RSRP */
					ue_state_cell.rsrq		= _CMIOT_atoi(res_array[4]);	/* RSRQ */
					ue_state_cell.rssi		= _CMIOT_atoi(res_array[5]);	/* RSSI */
					ue_state_cell.snr		= _CMIOT_atoi(res_array[6]);	/* SNR */
				}
				else
				{
					_CMIOT_Debug("%s(cell para num error)\r\n", __func__);
				}
				vPortFree(cellStateStr); /* 释放内存 */
			}
			else
			{
				_CMIOT_Debug("%s(Get cell uestats fail)\r\n", __func__);
			}
			_CMIOT_Debug("%s(earfcn: %d, pci: %d, primary cell: %d, rsrp: %d, rsrq: %d, rssi: %d, snr: %d)\r\n",\
							__func__, ue_state_cell.earfcn, ue_state_cell.pci, ue_state_cell.pri_cell,\
							ue_state_cell.rsrp, ue_state_cell.rsrq, ue_state_cell.rssi, ue_state_cell.snr);
			return ue_state_cell;
		}
		delay_ms(1000);
	}
	return ue_state_cell;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetUeTHPStats
Author			:	zhaoji
Created Time	:	2018.05.04
Description 	: 	请求UESTATS THP参数（速率）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
CMIOT_UE_STATE_THP _CMIOT_GetUeTHPStats(void)
{
	uint8_t at_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head;
	char *p_tail;
	uint8_t res_array[4][32]	= {0};
	uint8_t res1_array[3][32]	= {0};
	CMIOT_UE_STATE_THP ue_state_thp = {0,0,0,0};
	char *str;
	uint8_t i = 0;
	
	while(maxRetryCounts > 0)
	{
		ue_state_thp.MAC_DL = 0;
		ue_state_thp.MAC_UL = 0;
		ue_state_thp.RLC_DL = 0;
		ue_state_thp.RLC_UL = 0;
		
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NUESTATS=THP\r\n"), at_MatchStr, 2, 5000);
		
		if(result == 1)
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, "NUESTATS");
			if(p_head != NULL)
			{
				_CMIOT_Debug("%s(Get thp uestats ok)\r\n", __func__);
				p_tail = strstr(p_head, "\r\n\r\nOK\r\n");
				str = pvPortMalloc((p_tail-p_head+1) * sizeof(char));	/* 申请内存 */
				memset(str, 0, p_tail-p_head+1);
				memcpy(str, p_head, p_tail - p_head);
				
				if(cm_split(res_array, (uint8_t *)str, (uint8_t *)("\r\n")) == 4)	/* 判断参数数量是否正确 */
				{
					for(i=0; i<4; i++)
					{
						_CMIOT_Debug("%s(line[%d]: %s)\r\n", __func__, i+1, res_array[i]);
						if(cm_split(res1_array, (uint8_t *)res_array[i], (uint8_t *)(",")) == 3)
						{
							if(strcmp((char *)res1_array[1], "RLC UL") == 0)
							{
								_CMIOT_Debug("%s(RLC UL -> String(%s))\r\n", __func__, res1_array[2]);
								ue_state_thp.RLC_UL = _CMIOT_atoi(res1_array[2]);	/* RLC上行速率 */
							}
							else if(strcmp((char *)res1_array[1], "RLC DL") == 0)
							{
								_CMIOT_Debug("%s(RLC DL -> String(%s))\r\n", __func__, res1_array[2]);
								ue_state_thp.RLC_DL = _CMIOT_atoi(res1_array[2]);	/* RLC下行速率 */
							}
							else if(strcmp((char *)res1_array[1], "MAC UL") == 0)
							{
								_CMIOT_Debug("%s(MAC UL -> String(%s))\r\n", __func__, res1_array[2]);
								ue_state_thp.MAC_UL = _CMIOT_atoi(res1_array[2]);	/* MAC上行速率 */
							}
							else if(strcmp((char *)res1_array[1], "MAC DL") == 0)
							{
								_CMIOT_Debug("%s(MAC DL -> String(%s))\r\n", __func__, res1_array[2]);
								ue_state_thp.MAC_DL = _CMIOT_atoi(res1_array[2]);	/* MAC下行速率 */
							}
							else
							{
								_CMIOT_Debug("%s(unknow THP Type)\r\n", __func__);
							}
						}
						else
						{
							_CMIOT_Debug("%s(line para num error!)\r\n", __func__);
						}
					}
				}
				else
				{
					_CMIOT_Debug("%s(line num error!)\r\n", __func__);
				}
				vPortFree(str);	/* 释放内存 */
				_CMIOT_Debug("%s(rlc_ul: %d, rlc_dl: %d, mac_ul: %d, mac_dl: %d)\r\n", __func__, ue_state_thp.RLC_UL, ue_state_thp.RLC_DL, ue_state_thp.MAC_UL, ue_state_thp.MAC_DL);
				return ue_state_thp;
			}
			else {
				_CMIOT_Debug("%s(get uestats thp error)\r\n", __func__);
			}
		}
		delay_ms(1000);
	}
	return ue_state_thp;
}


/*-----------------------------------------------------------------------------
Function Name	:	cm_getAPN
Author			:	zhaoji
Created Time	:	2018.05.04
Description 	: 	获取当前APN
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void cm_getAPN(uint8_t *apnBuf, uint32_t bufLen)
{
	uint8_t at_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	char *p_head;
	char *p_tail;
	
	memset(apnBuf, 0, bufLen); /* 清空buffer */
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CGDCONT?\r\n"), at_MatchStr, 2, 2000);
		
		if(result == 1)
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, ",\"IP\",");
			if(p_head != NULL)
			{
				p_head += strlen(",\"IP\",");
				if(*(p_head) == ',')
				{
					_CMIOT_Debug("%s(empty apn)\r\n", __func__);
					return;
				}
				p_tail = strstr(p_head, ",");
				/* 检查apn长度是否超过 */
				if(p_tail - p_head < bufLen)
				{
					memcpy(apnBuf, p_head + 1, p_tail - p_head - 2);
				}
				else
				{
					memcpy(apnBuf, p_head + 1, bufLen);
				}
				_CMIOT_Debug("%s(apn: %s)\r\n", __func__, apnBuf);
				return;
			}
			_CMIOT_Debug("%s(find apn type fail)\r\n", __func__);
			return;
		}
		delay_ms(1000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_IsPdpAttached
Author			:	zhaoji
Created Time	:	2018.05.05
Description 	: 	获取PDP附着状态
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_IsPdpAttached(void)
{
	uint8_t attach_MatchStr[2][20] = {"CGATT:0", "CGATT:1"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;

	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CGATT?\r\n"), attach_MatchStr, 2, 2000);
		if(result == 2)
		{
			_CMIOT_Debug("%s(Attached!)\r\n", __func__);
			return true;
		}
		if(result == 1)
		{
			_CMIOT_Debug("%s(unattached!)\r\n", __func__);
			return false;
		}
	}
	_CMIOT_Debug("%s(execute fail)\r\n", __func__);
	return false;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_SetAutoConnect
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	配置自动入网参数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_SetAutoConnect(bool state)
{
	uint8_t matchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	
	if(_CMIOT_SetMinFunctionalityState(true))
	{
		while(maxRetryCounts > 0)
		{
			maxRetryCounts--;
			if(state)
			{
				/* 配置自动入网模式 */
				result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NCONFIG=AUTOCONNECT,TRUE\r\n"), matchStr, 2, 5000);
			}
			else
			{
				/* 配置非自动入网模式 */
				result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NCONFIG=AUTOCONNECT,FALSE\r\n"), matchStr, 2, 5000);
			}
			
			if(result == 2)
			{
				_CMIOT_Debug("%s(error!)\r\n", __func__);
				// return false;
			}
			if(result == 1)
			{
				_CMIOT_Debug("%s(ok!)\r\n", __func__);
				return true;
			}
		}
		_CMIOT_Debug("%s(execute fail)\r\n", __func__);
		return false;
	}
	else
	{
		return false;
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_SetAutoConnect
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	配置自动入网参数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_SetScramblingState(bool state)
{
	uint8_t matchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	
	if(_CMIOT_SetMinFunctionalityState(true))
	{
		while(maxRetryCounts > 0)
		{
			maxRetryCounts--;
			if(state)
			{
				/* 打开扰码 */
				result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NCONFIG=CR_0354_0338_SCRAMBLING,TRUE\r\n"), matchStr, 2, 5000);
			}
			else
			{
				/* 关闭扰码 */
				result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NCONFIG=CR_0354_0338_SCRAMBLING,FALSE\r\n"), matchStr, 2, 5000);
			}
			
			if(result == 2)
			{
				_CMIOT_Debug("%s(error!)\r\n", __func__);
				// return false;
			}
			if(result == 1)
			{
				_CMIOT_Debug("%s(ok!)\r\n", __func__);
				return true;
			}
		}
		_CMIOT_Debug("%s(execute fail)\r\n", __func__);
		return false;
	}
	else
	{
		return false;
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_SetMinFunctionalityState
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	配置工作等级（全功能、最小功能）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_SetMinFunctionalityState(bool state)
{
	uint8_t matchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;

	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		if(state)
		{
			/* 配置最小工作模式 */
			result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CFUN=0\r\n"), matchStr, 2, 5000);
		}
		else
		{
			/* 配置全功能模式 */
			result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CFUN=1\r\n"), matchStr, 2, 5000);
		}
		
		if(result == 2)
		{
			_CMIOT_Debug("%s(error!)\r\n", __func__);
			// return false;
		}
		if(result == 1)
		{
			_CMIOT_Debug("%s(ok!)\r\n", __func__);
			return true;
		}
	}
	_CMIOT_Debug("%s(execute fail)\r\n", __func__);
	return false;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_NbModule_Reboot
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	重启模组
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_NbModule_Reboot(void)
{
	uint8_t matchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;

	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NRB\r\n"), matchStr, 2, 10000);
		if(result == 2)
		{
			_CMIOT_Debug("%s(reboot cmd error!)\r\n", __func__);
		}
		if(result == 1)
		{
			_CMIOT_Debug("%s(reboot success!)\r\n", __func__);
			return;
		}
	}
	_CMIOT_Debug("%s(execute fail)\r\n", __func__);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310PowerGpioInit
Author			:	zhaoji
Created Time	:	2018.05.18
Description 	: 	初始化NB模组电源控制GPIO
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_M5310PowerGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310_Init
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	: 	初始化NB模组
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_M5310_Init(void)
{
	uint8_t maxRetryCounts = 5;
	_CMIOT_M5310PowerGpioInit();
	M5310_POWER_OFF;	/* 关机 */
	delay_ms(1000);
	M5310_POWER_ON;		/* 开机 */
	
	while(maxRetryCounts > 0)
	{
		if(cm_IsNbModuleAlive())	/* 检测模组是否启动成功 */
		{
			break;
		}
		delay_ms(1000);
	}
	/* 打开扰码 */
	_CMIOT_SetScramblingState(true);
	/* 打开自动入网 */
	_CMIOT_SetAutoConnect(true);
	/* 重启模组 */
	_CMIOT_NbModule_Reboot();
	/* 检测模组是否启动成功 */
	maxRetryCounts = 5;
	while(maxRetryCounts > 0)
	{
		if(cm_IsNbModuleAlive())
		{
			break;
		}
		delay_ms(1000);
	}
}






