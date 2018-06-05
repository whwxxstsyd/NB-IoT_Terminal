
/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "common.h"
#include "usart.h"
#include <stdarg.h>
#include "string.h"
#include "FreeRTOS.h"
#include "stdio.h"
#include "build_version.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
bool DEBUG_FLAG = true;	/* 调试信息打印标志位，为true时将调试信息打印至调试串口 */


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Debug
Author			:	zhaoji
Created Time	:	2018.01.02
Description 	:	debug函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_Debug(const char *fmt, ...)
{
	uint8_t LogMsg[256] = {0};
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf((char *)LogMsg, (unsigned int)(sizeof(LogMsg)), (const char *)fmt, ap);
	va_end(ap);

	/* 如果标志位为TRUE，打印至调试串口 */
	if(DEBUG_FLAG)
	{
		_CMIOT_Uart_send(UART_CLI_DEBUG, LogMsg, strlen((const char*)LogMsg));
	}
}



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_atoi
Author			:	zhaoji
Created Time	:	2017.09
Description 	:	10进制字符串转整数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
int64_t _CMIOT_atoi(const uint8_t *str)
{
	int64_t s=0;
	bool flag=false;

	if(*str=='-'||*str=='+')
	{
		if(*str=='-')
		{
			flag=true;
			str++;
		}
	}

	while(1)
	{
		if(*str>='0'&&*str<='9')
		{
			s=s*10+*str-'0';
			str++;
		}
		else
		{
			break;
		}
	}
	return s*(flag?-1:1);
}



/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Str_StartWith
Author			:	zhaoji
Created Time	:	2018.01.03
Description 	:	判断字符串是否以特定子串开始
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_Str_StartWith(uint8_t *Src, uint8_t *dst)
{
	if(strlen((const char *)Src) < strlen((const char *)dst))
	{
		return false;
	}

	if(strncmp((const char *)Src, (const char *)dst, strlen((const char *)dst)) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Str_EndWith
Author			:	zhaoji
Created Time	:	2018.01.03
Description 	:	判断字符串是否以特定子串结束
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_Str_EndWith(uint8_t *Src, uint8_t *dst)
{
	if(strlen((const char *)Src) < strlen((const char *)dst))
	{
		return false;
	}
	
	if(strncmp((const char *)(Src + strlen((const char *)Src) - strlen((const char *)dst)), (const char *)dst, strlen((const char *)dst)) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}



/*-----------------------------------------------------------------------------
Function Name	:	cm_split
Author			:	zhaoji
Created Time	:	2018.02.23
Description 	:	字符串分割，将字符串按分隔符分割后，依次放到二维数据中，并返回分割后的字符串个数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
int cm_split(uint8_t dst[][32], uint8_t *str, const uint8_t *spl)
{
	int n = 0;
	int i;
	char *result = NULL;
	result = strtok((char *)str, (const char *)spl);
	while( result != NULL )
	{
		strncpy((char *)dst[n], result, sizeof(dst[n]));
		result = strtok(NULL, (const char *)spl);
		n++;
	}
	memset(str, 0, sizeof((char *)str));
	for(i=0; i<n-1; i++)
	{
		strcat((char *)str, (const char *)dst[i]);
		strcat((char *)str, (const char *)spl);
	}
	strcat((char *)str, (const char *)dst[n-1]);
	return n;
}


/*-----------------------------------------------------------------------------
Function Name	:	getbuildtime
Author			:	zhaoji
Created Time	:	2018.05.08
Description 	:	获取编译时间
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void cm_getbuildVersion(uint8_t *buf, uint32_t bufLen)
{
	strncpy((char *)buf, __CMIOT_VERSION__, bufLen);
}



