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
#define	ATTACH_TIMEOUT		(3*60*1000)

/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART3(UART_M5310)数据接收buffer */
uint8_t   UART_M5310_RxBuffer[512] = {0};
uint32_t  UART_M5310_RxBufferLen   =  0;


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
	
	result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NRB\r\n"), boot_MatchStr, 1, 5000);   /* 重启模组，并等待启动信息 */
	
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
	
	_CMIOT_Debug("%s(Send->%s\r\n", __func__, AtCmd);
	
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
		
		// _CMIOT_Debug("retryCounts: %ld\r\n", retryCounts);
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
			_CMIOT_Debug("%s(return: %d)\r\n", __func__, signal);
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
CMIOT_UE_State _CMIOT_M5310_GetUeState(void)
{
	CMIOT_UE_State ue_state = {0,0,0,0,""};
	char *index;
	uint8_t maxRetryCounts = 5;
	uint32_t result;
	uint8_t uestate_MatchStr[2][20] = {"OK","ERROR"};	/* 指令响应完成匹配字符串 */
	bool flag = false;
	char *pSta, *pEnd;
	
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
			
			return ue_state;
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
Function Name	:	_CMIOT_M5310_ICMP_Request
Author			:	zhaoji
Created Time	:	2018.03.05
Description 	: 	测试ping延时
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/







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
			p_end  = strstr((const char *)p_head, "\r\nOK");
			strncat((char *)ModuleName, p_head, p_end - p_head);
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
			p_end  = strstr((const char *)p_head, "\r\nOK\r\n");
			strncat((char *)ModuleVersion, p_head, p_end - p_head);
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
			p_end  = strstr((const char *)p_head, "\r\nOK\r\n");
			strncat((char *)ICCID, p_head, p_end - p_head);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)ICCID, "ERROR", buffersize);
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
	uint8_t imsi_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
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
			p_end  = strstr((const char *)p_head, "\r\nOK\r\n");
			strncat((char *)IMSI, p_head, p_end - p_head);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)IMSI, "ERROR", buffersize);
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
	
	memset(IMEI, 0, buffersize);
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CGSN=1\r\n"), imei_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			p_head = strstr((const char *)UART_M5310_RxBuffer, "CGSN:") + strlen("CGSN:");
			p_end  = strstr((const char *)p_head, "\r\nOK\r\n");
			strncat((char *)IMEI, p_head, p_end - p_head);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)IMEI, "ERROR", buffersize);
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
			p_head = strstr((const char *)UART_M5310_RxBuffer, "\"") + strlen("\"");
			p_end  = strstr((const char *)p_head, "\"");
			strncat((char *)plmn, p_head, p_end - p_head);
			return 1;
		}
		delay_ms(500);
	}
	/* 获取失败 */
	strncpy((char *)plmn, "ERROR", buffersize);
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
//uint32_t _CMIOT_GetNetworkDelay()
//{
//	uint8_t nping_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
//	uint8_t maxRetryCounts = 5;
//	uint8_t result;
//	char *index;
//	bool flag;
//	uint32_t band = 0;
//	
//	while(maxRetryCounts > 0)
//	{
//		maxRetryCounts--;
//		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NBAND?\r\n"), band_MatchStr, 2, 2000);
//		
//		if(result == 1)	/* 指令执行OK */
//		{
//			index = strstr((char *)UART_M5310_RxBuffer, "+NBAND:");
//			if(index != NULL)
//			{
//				index += strlen("+NBAND:");
//				flag = false;
//				while(1)
//				{
//					if(*index == ' ')	/* 忽略空格 */
//					{
//						index++;
//						continue;
//					}
//					
//					if(*index == '-')	/* 正负 */
//					{
//						flag = true;
//						index++;
//						continue;
//					}
//					
//					if(*index >= '0' && *index <= '9')	/* 计算BAND绝对值 */
//					{
//						band = band*10 + *index - '0';
//						index++;
//					}
//					else
//					{
//						break;
//					}
//				}
//				band = band * (flag ? -1 : 1);
//				return band;
//			}
//			else
//			{
//				_CMIOT_Debug("%s(BAND Not Found!)\r\n", __func__);
//			}
//		}
//		delay_ms(500);
//	}
//	/* 获取失败 */
//	_CMIOT_Debug("%s(Exe Failed!)\r\n", __func__);
//	return 0;
//}




/*-----------------------------------------------------------------------------
Function Name	:	cm_IsModuleAlive
Author			:	zhaoji
Created Time	:	2018.03.22
Description 	: 	检测模块是否启动正常
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool cm_IsModuleAlive()
{
	uint8_t at_MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint8_t maxRetryCounts = 5;
	uint8_t result;
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT\r\n"), at_MatchStr, 2, 2000);
		
		if(maxRetryCounts == 0 && result == 1)
		{
			return true;
		}
		delay_ms(1000);
	}
	return false;
}






