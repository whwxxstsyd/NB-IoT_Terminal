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

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART3(UART_M5310)数据接收buffer */
uint8_t   UART_M5310_RxBuffer[128] = {0};
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
	uint8_t boot_MatchStr[2][20] = {"M5310\r\nOK\r\n", ""};	/* 指令响应完成匹配字符串 */
	uint8_t attach_MatchStr[2][20] = {"+CGATT:0", "+CGATT:1"};	/* 指令响应完成匹配字符串 */
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
	uint32_t timeslice_ms = 500;     /* 检索M5310串口响应时间精度 */
	
	memset(UART_M5310_RxBuffer, 0, sizeof(UART_M5310_RxBuffer));     /* 清空接收Buffer */
	UART_M5310_RxBufferLen = 0;
	
	_CMIOT_Uart_send(UART_M5310, AtCmd, strlen((const char *)AtCmd));    /* 发送AT指令 */
	
	_CMIOT_Debug("%s(Send->%s\r\n", __func__, AtCmd);
	
	/* 检索M5310串口返回内容是否匹配目标字符串数组，匹配成功返回数组索引号+1，超时匹配失败返回0 */
	while(1)
	{
		// _CMIOT_Debug("retryCounts: %ld\r\n", retryCounts);
		if(retryCounts * timeslice_ms >= timeout_ms)   /* 匹配超时返回 */
		{
			_CMIOT_Debug("%s(execute timeout!)\r\n", __func__);
			return 0;
		}
		
		for(i=0; i<MatchRsp_Num; i++)
		{
			if(strstr((const char *)UART_M5310_RxBuffer, (const char *)MatchRsp[i]) != NULL)  /* 检索匹配字符串 */
			{
				_CMIOT_Debug("%s(return: %d)\r\n", __func__, i+1);
				return i + 1;
			}
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
	uint8_t csq_MatchStr[2][20] = {"\r\nOK\r\n", "\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	char *csq;
	uint8_t signal = 0;
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+CSQ?\r\n"), csq_MatchStr, 2, 2000);
		
		if(result == 1)	/* 指令执行OK */
		{
			csq = strstr((const char *)UART_M5310_RxBuffer, "+CSQ:") + strlen("+CSQ:");
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
	CMIOT_UE_State ue_state = {0,0,0,0};
	char *index;
	uint8_t maxRetryCounts = 5;
	uint32_t result;
	uint8_t uestate_MatchStr[2][20] = {"\r\nOK\r\n","\r\nERROR\r\n"};	/* 指令响应完成匹配字符串 */
	bool flag = false;
	
	while(maxRetryCounts > 0)
	{
		maxRetryCounts--;
		result = _CMIOT_ExecuteAtCmd((uint8_t *)("AT+NUESTATS\r\n"), uestate_MatchStr, 2, 2000);
		
		if(result == 1)
		{
			/* 从UESTATE信息中获取NB频点 */
			index = strstr((const char *)UART_M5310_RxBuffer, "EARFCN:") + strlen("EARFCN:");
			if(index != NULL)
			{
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
			index = strstr((const char *)UART_M5310_RxBuffer, "RSRQ:") + strlen("RSRQ:");
			if(index != NULL)
			{
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
			index = strstr((const char *)UART_M5310_RxBuffer, "SNR:") + strlen("SNR:");
			if(index != NULL)
			{
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
			index = strstr((const char *)UART_M5310_RxBuffer, "ECL:") + strlen("ECL:");
			if(index != NULL)
			{
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
			break;
		}
		else
		{
			_CMIOT_Debug("%s(AT RSP ERROR!)\r\n", __func__);
		}
		
	}
	
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












