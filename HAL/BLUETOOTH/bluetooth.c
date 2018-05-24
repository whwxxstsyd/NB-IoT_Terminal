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
#include "bluetooth.h"
#include "timers.h"
#include "adc.h"


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART1(UART_BLUETOOTH)数据接收buffer */
uint8_t   UART_BLE_RxBuffer[1024] = {0};
uint32_t  UART_BLE_RxBufferLen   =  0;

/* USART3(UART_M5310)数据接收buffer */
extern uint8_t   UART_M5310_RxBuffer[512];
extern uint32_t  UART_M5310_RxBufferLen;

/* 蓝牙执行AT指令标志位 */
bool BLE_AT_EXE_FLAG = false;

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/
static uint8_t msg[64] = {0};

TimerHandle_t bleAtExeTimer;	/* 蓝牙执行AT指令时的M5310串口定时器，定时器到达时将M5310串口接收buffer数据通过蓝牙发送到APP */




/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ExecuteBLEAtCmd
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	执行BLE AT指令
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint32_t _CMIOT_ExecuteBLEAtCmd(uint8_t *AtCmd, uint8_t MatchRsp[][20], uint8_t MatchRsp_Num, uint32_t timeout_ms)
{
	uint8_t i;
	uint32_t retryCounts = 0;        /* 检索次数 */
	uint32_t timeslice_ms = 100;     /* 检索BLE串口响应时间精度 */
	
	memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));     /* 清空接收Buffer */
	UART_BLE_RxBufferLen = 0;
	
	_CMIOT_Uart_send(UART_BLUETOOTH, AtCmd, strlen((const char *)AtCmd));    /* 发送AT指令 */

	_CMIOT_Debug("%s() %s\r\n", __func__, AtCmd);
	
	/* 检索M5310串口返回内容是否匹配目标字符串数组，匹配成功返回数组索引号+1，超时匹配失败返回0 */
	while(1)
	{
		for(i=0; i<MatchRsp_Num; i++)
		{
			if(strstr((const char *)UART_BLE_RxBuffer, (const char *)MatchRsp[i]) != NULL)  /* 检索匹配字符串 */
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
Function Name	:	_CMIOT_BleResetGpioInit
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	: 	蓝牙复位引脚初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BleResetGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BleReset
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	: 	复位蓝牙模块
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BleReset(void)
{
	BLE_RESET_ENABLE;
	delay_ms(1000);
	BLE_RESET_DISABLE;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GetBleConnectedState
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	获取蓝牙连接状态
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
bool _CMIOT_GetBleConnectedState(void)
{
	uint8_t MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	char *p_head;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)("AT+LINK?\r\n"), MatchStr, 2, 2000);   /* 查询连接状态 */
		if(result == 1)
		{
			p_head = strstr((const char*)UART_BLE_RxBuffer, "+Link:");
			if(p_head != NULL)
			{
				p_head += strlen("+Link:");
				if(strncmp(p_head, "OffLine", strlen("OffLine")) == 0)
				{
					/* 离线 */
					_CMIOT_Debug("%s(OffLine)\r\n", __func__);
					return false;
				}
				else if(strncmp(p_head, "OnLine", strlen("OnLine")) == 0)
				{
					/* 在线 */
					_CMIOT_Debug("%s(OnLine)\r\n", __func__);
					return true;
				}
				else
				{
					_CMIOT_Debug("%s(unknow link state!)\r\n", __func__);
				}
			}
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return false;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_ExitPassthroughMode
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	BLE模块退出透传模式（进入命令模式）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_ExitPassthroughMode(void)
{
	uint8_t MatchStr[2][20] = {"a+ok", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)("+++a"), MatchStr, 2, 2000);   /* 退出透传模式 */
		if(result == 1)
		{
			return 1;
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_OffMaxPut
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	配置为20字节分包发送
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_OffMaxPut(void)
{
	uint8_t MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)("AT+MAXPUT=OFF\r\n"), MatchStr, 2, 2000);   /* 配置为20字节分包发送 */
		if(result == 1)
		{
			return 1;
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_SetPacketTimer
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	设置BLE打包间隔
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_SetPacketInterval(void)
{
	uint8_t MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)("AT+UARTTM=100\r\n"), MatchStr, 2, 2000);   /* 设置打包间隔 */
		if(result == 1)
		{
			return 1;
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_SetName
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	设置BLE名称（D5200_IMEI后四位） 用于被搜索时区分不同终端
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_SetName(void)
{
	uint8_t MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	uint8_t bleNameCmd[64] = {0};
	uint8_t imei[32] = {0};
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	_CMIOT_GetIMEI(imei, sizeof(imei));
	
	strcat((char *)bleNameCmd, "AT+NAME=D5200_");
	strcat((char *)bleNameCmd, (char *)(imei + strlen((const char *)imei) - 4));
	strcat((char *)bleNameCmd, "\r\n");
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)bleNameCmd, MatchStr, 2, 2000);   /* 配置BLE设备名称 */
		if(result == 1)
		{
			return 1;
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_EnterPassthroughMode
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	进入透传模式（数据模式）
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_EnterPassthroughMode(void)
{
	uint8_t MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)("AT+ENTM\r\n"), MatchStr, 2, 2000);   /* 进入透传模式 */
		if(result == 1)
		{
			return 1;
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_Reboot
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	重启BLE模块
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
uint8_t _CMIOT_BLE_Reboot(void)
{
	uint8_t MatchStr[2][20] = {"OK", "ERROR"};	/* 指令响应完成匹配字符串 */
	uint32_t result;
	uint8_t maxRetryCounts = 5;
	uint8_t waitBootInfoRetry = 10;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	while(maxRetryCounts > 0)
	{
		result = _CMIOT_ExecuteBLEAtCmd((uint8_t *)("AT+Z\r\n"), MatchStr, 2, 5000);   /* AT+Z重启模块 */
		if(result == 1)
		{
			while(waitBootInfoRetry > 0)
			{
				if(strstr((const char*)UART_BLE_RxBuffer, "USR-BLE") != NULL) {
					memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));     /* 清空接收Buffer */
					UART_BLE_RxBufferLen = 0;
					break;
				}
				waitBootInfoRetry--;
				if(waitBootInfoRetry == 0)
				{
					_CMIOT_Debug("%s(WaitBootMsg timeout)\r\n", __func__);
					return 2;
				}
				delay_ms(1000);
			}
			_CMIOT_Debug("%s(boot ok)\r\n", __func__);
			return 1;
		}
		maxRetryCounts--;
	}
	_CMIOT_Debug("%s(Execute fail!)\r\n", __func__);
	return 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_M5310Uartbuf2Ble
Author			:	zhaoji
Created Time	:	2018.04.11
Description 	: 	将M5310串口接收到的数据发送到蓝牙
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_M5310Uartbuf2Ble(void)
{
	_CMIOT_Debug("%s...\r\n", __func__);
	
	/* 发送到蓝牙模块 */
	_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)"<Response><AT>", strlen("<Response><AT>"));
	_CMIOT_Uart_send(UART_BLUETOOTH, UART_M5310_RxBuffer, UART_M5310_RxBufferLen);
	_CMIOT_Uart_send(UART_BLUETOOTH, (uint8_t *)"</AT></Response>", strlen("</AT></Response>"));
	/* 清除缓冲区 */
	memset(UART_M5310_RxBuffer, 0, sizeof(UART_M5310_RxBuffer));
	UART_M5310_RxBufferLen = 0;
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_Init
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	初始化BLE模块
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_Init(void)
{
	_CMIOT_Debug("%s...\r\n", __func__);
	
	/* 500ms串口未接收到新数据，认为本帧数据接收完成 */
	bleAtExeTimer = xTimerCreate((const char*   )"bleAtExeTimer",
								(TickType_t     )500,
								(UBaseType_t    )pdFALSE,
								(void*          )2,
								(TimerCallbackFunction_t)_CMIOT_M5310Uartbuf2Ble);
	
	_CMIOT_BLE_ExitPassthroughMode();		/* 退出透传 */
	_CMIOT_BLE_OffMaxPut();					/* 配置为20字节分包 */
	_CMIOT_BLE_SetPacketInterval();			/* 设置分包发送间隔 */
	_CMIOT_BLE_SetName();					/* 设置名称 */
	_CMIOT_BLE_Reboot();					/* 重启 */
	delay_ms(1000);
	_CMIOT_BLE_ExitPassthroughMode();		/* 退出透传模式 */
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_RSP_deviceInfo
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	返回设备信息到蓝牙
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_RSP_deviceInfo(void)
{
	uint8_t rsp[512] = {0};
	
	_CMIOT_Debug("%s() ...\r\n", __func__);
	
	strcat((char *)rsp, "<Response><deviceInfo>");
	
	/* 模组名称 */
	_CMIOT_GetModuleName(msg, sizeof(msg));
	strcat((char *)rsp, "<module>");
	strcat((char *)rsp, (const char *)msg);
	strcat((char *)rsp, "</module>");

	/* 模组版本 */
	_CMIOT_GetModuleVersion(msg, sizeof(msg));
	strcat((char *)rsp, "<module_ver>");
	strcat((char *)rsp, (const char *)msg);
	strcat((char *)rsp, "</module_ver>");

	/* ICCID */
	_CMIOT_GetICCID(msg, sizeof(msg));
	strcat((char *)rsp, "<iccid>");
	strcat((char *)rsp, (const char *)msg);
	strcat((char *)rsp, "</iccid>");

	/* IMSI */
	_CMIOT_GetIMSI(msg, sizeof(msg));
	strcat((char *)rsp, "<imsi>");
	strcat((char *)rsp, (const char *)msg);
	strcat((char *)rsp, "</imsi>");

	/* IMEI */
	_CMIOT_GetIMEI(msg, sizeof(msg));
	strcat((char *)rsp, "<imei>");
	strcat((char *)rsp, (const char *)msg);
	strcat((char *)rsp, "</imei>");

	/* MCU软件版本 */
	strcat((char *)rsp, "<firmware>");
	cm_getbuildVersion(msg, sizeof(msg));
	strcat((char *)rsp, (const char*)msg);
	strcat((char *)rsp, "</firmware>");
	
	/* 电池电压 */
	sprintf((char *)msg, "%.1f", cm_getBatteryVol());
	strcat((char *)rsp, "<voltage>");
	strcat((char *)rsp, (const char*)msg);
	strcat((char *)rsp, "</voltage>");
	
	strcat((char *)rsp, "</deviceInfo></Response>");
	
	/* 发送到蓝牙模块 */
	_CMIOT_Uart_send(UART_BLUETOOTH, rsp, strlen((const char *)rsp));
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_RSP_radioInfo
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	返回信号参数信息到蓝牙
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_RSP_radioInfo(void)
{
	uint8_t rsp[512] = {0};
	uint8_t t3324[32] = {0};
	uint8_t t3412[32] = {0};
	uint8_t plmn[32] = {0};
	uint8_t apn[32] = {0};
	CMIOT_UE_STATE ue_state;
	CMIOT_UE_STATE_CELL ue_state_cell;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	/* Ping测试防止模组进入PSM模式后射频参数不准确 */
	_CMIOT_GetNetworkDelay((uint8_t *)PING_ADDR, 100, 5000);
	
	strcat((char *)rsp, "<Response><radioInfo>");
	ue_state_cell = _CMIOT_GetUeCellStats();
	/* rssi */
	sprintf((char *)msg, "<rssi>%d</rssi>", ue_state_cell.rssi);
	strcat((char *)rsp, (const char*)msg);
	/* rsrp */
	sprintf((char *)msg, "<rsrp>%d</rsrp>", ue_state_cell.rsrp);
	strcat((char *)rsp, (const char*)msg);
	
	ue_state = _CMIOT_M5310_GetUeState();
	/* snr */
	sprintf((char *)msg, "<snr>%d</snr>", ue_state.snr);
	strcat((char *)rsp, (const char*)msg);
	/* rsrq */
	sprintf((char *)msg, "<rsrq>%d</rsrq>", ue_state.rsrq);
	strcat((char *)rsp, (const char*)msg);
	/* earfcn */
	sprintf((char *)msg, "<earfcn>%d</earfcn>", ue_state.earfcn);
	strcat((char *)rsp, (const char*)msg);
	/* ecl */
	sprintf((char *)msg, "<ecl>%d</ecl>", ue_state.ecl);
	strcat((char *)rsp, (const char*)msg);
	/* cellid */
	sprintf((char *)msg, "<cellid>%s</cellid>", ue_state.cellid);
	strcat((char *)rsp, (const char*)msg);
	/* band */
	sprintf((char *)msg, "<band>%d</band>", _CMIOT_GetNB_Band());
	strcat((char *)rsp, (const char*)msg);
	/* t3324/3412 */
	_CMIOT_Get_PSM_TIMER_Value(t3324, t3412, sizeof(t3324));
	sprintf((char *)msg, "<t3324>%s</t3324><t3412>%s</t3412>", t3324, t3412);
	strcat((char *)rsp, (const char*)msg);
	/* plmn */
	_CMIOT_GetPLMN(plmn, sizeof(plmn));
	sprintf((char *)msg, "<plmn>%s</plmn>", plmn);
	strcat((char *)rsp, (const char*)msg);
	/* apn */
	cm_getAPN(apn, sizeof(apn));
	sprintf((char *)msg, "<apn>%s</apn>", apn);
	strcat((char *)rsp, (const char*)msg);
	
	strcat((char *)rsp, "</radioInfo></Response>");
	
	/* 发送到蓝牙模块 */
	_CMIOT_Uart_send(UART_BLUETOOTH, rsp, strlen((const char *)rsp));
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_RSP_attachTime
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	返回一次驻网时长到蓝牙
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_RSP_attachTime(void)
{
	uint8_t rsp[128] = {0};
	strcat((char *)rsp, "<Response>");
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	/* attachTime */
	sprintf((char *)msg, "<attachTime>%d</attachTime>", _CMIOT_M5310_GetRegisterTime());
	strcat((char *)rsp, (const char*)msg);
	
	strcat((char *)rsp, "</Response>");
	
	/* 发送到蓝牙模块 */
	_CMIOT_Uart_send(UART_BLUETOOTH, rsp, strlen((const char *)rsp));
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_RSP_pingDelay
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	返回一次ping延时数据到蓝牙
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_RSP_pingDelay(void)
{
	uint8_t rsp[64] = {0};
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	strcat((char *)rsp, "<Response>");
	
	/* pingDelay */
	sprintf((char *)msg, "<pingDelay>%d</pingDelay>", _CMIOT_GetNetworkDelay((uint8_t *)"114.114.114.114", 100, 10000));
	strcat((char *)rsp, (const char*)msg);
	
	strcat((char *)rsp, "</Response>");
	
	/* 发送到蓝牙模块 */
	_CMIOT_Uart_send(UART_BLUETOOTH, rsp, strlen((const char *)rsp));
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_RSP_comprehensiveTest
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	返回一次综合测试数据到蓝牙
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_RSP_comprehensiveTest(void)
{
	uint8_t rsp[256] = {0};
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	strcat((char *)rsp, "<Response><comprehensive>");
	
	/* attachTime */
	sprintf((char *)msg, "<attachTime>%d</attachTime>", _CMIOT_M5310_GetRegisterTime());
	strcat((char *)rsp, (const char*)msg);
	/* pingDelay */
	sprintf((char *)msg, "<pingDelay>%d</pingDelay>", _CMIOT_GetNetworkDelay((uint8_t *)"114.114.114.114", 100, 10000));
	strcat((char *)rsp, (const char*)msg);
	
	/* csq */
	sprintf((char *)msg, "<rssi>%d</rssi>", _CMIOT_GetUeCellStats().rssi);
	strcat((char *)rsp, (const char*)msg);

	/* snr */
	sprintf((char *)msg, "<snr>%d</snr>", _CMIOT_M5310_GetUeState().snr);
	strcat((char *)rsp, (const char*)msg);
	
	strcat((char *)rsp, "</comprehensive></Response>");
	
	/* 发送到蓝牙模块 */
	_CMIOT_Uart_send(UART_BLUETOOTH, rsp, strlen((const char *)rsp));
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BLE_DataProcess
Author			:	zhaoji
Created Time	:	2018.04.10
Description 	: 	BLE数据解析处理函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BLE_DataProcess(void)
{
	uint8_t cmd[256] 	= {0};
	char *p_head;
	char *p_tail;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	if(strstr((const char*)UART_BLE_RxBuffer, "<Request>") != NULL && strstr((const char*)UART_BLE_RxBuffer, "</Request>") != NULL)
	{
		_CMIOT_Debug("%s(request data)\r\n", __func__);
		
		/* 定位并获取命令参数 */
		p_head = strstr((const char*)UART_BLE_RxBuffer, "<Request>") + strlen("<Request>");
		p_tail = strstr((const char*)UART_BLE_RxBuffer, "</Request>");
		
		if(p_tail - p_head > sizeof(cmd))
		{
			_CMIOT_Debug("%s(ble request data over length:256)\r\n", __func__);
			/* 清空BLE接收Buffer */
			memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));
			UART_BLE_RxBufferLen = 0;
			return;
		}
		
		memcpy(cmd, p_head, p_tail-p_head);
		
		/* 清空BLE接收Buffer */
		memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));
		UART_BLE_RxBufferLen = 0;
		
		/* 返回响应数据 */
		if(strcmp((const char*)cmd, "deviceInfo") == 0)
		{
			/* 返回设备信息 */
			_CMIOT_BLE_RSP_deviceInfo();
		}
		else if(strcmp((const char*)cmd, "radioInfo") == 0)
		{
			/* 返回信号参数 */
			_CMIOT_BLE_RSP_radioInfo();
		}
		else if(strcmp((const char*)cmd, "attachTime") == 0)
		{
			/* 返回一次驻网时间 */
			_CMIOT_BLE_RSP_attachTime();
		}
		else if(strcmp((const char*)cmd, "pingDelay") == 0)
		{
			/* 返回ping延时 */
			_CMIOT_BLE_RSP_pingDelay();
		}
		else if(strcmp((const char*)cmd, "comprehensiveTest") == 0)
		{
			/* 返回一次综合测试数据（包括驻网、信号、延时） */
			_CMIOT_BLE_RSP_comprehensiveTest();
		}
		else if(strcmp((const char*)cmd, "bleAtEnable") == 0)
		{
			/* 进入蓝牙AT指令模式（工程模式） */
			memset(UART_M5310_RxBuffer, 0, sizeof(UART_M5310_RxBuffer));
			UART_M5310_RxBufferLen = 0;
			BLE_AT_EXE_FLAG = true;
			_CMIOT_Debug("%s(bleAtEnable)\r\n", __func__);
		}
		else if(strcmp((const char*)cmd, "bleAtDisable") == 0)
		{
			/* 退出蓝牙AT指令模式（工程模式） */
			BLE_AT_EXE_FLAG = false;
			_CMIOT_Debug("%s(bleAtDisable)\r\n", __func__);
		}
		else if(_CMIOT_Str_StartWith(cmd, (uint8_t *)"<AT>") && _CMIOT_Str_EndWith(cmd, (uint8_t *)"</AT>"))
		{
			BLE_AT_EXE_FLAG = true;
			_CMIOT_Debug("%s(Recv AT Command)\r\n", __func__);
			/* 发送AT指令 */
			/* 定位并获取AT指令 */
			p_head = strstr((const char*)cmd, "<AT>") + strlen("<AT>");
			p_tail = strstr((const char*)cmd, "</AT>");
			_CMIOT_Uart_send(UART_M5310, (uint8_t *)p_head, p_tail-p_head);
		}
		else
		{
			/* 未知蓝牙命令 */
			_CMIOT_Debug("%s(unknow cmd)\r\n", __func__);
		}
	}
	else
	{
		_CMIOT_Debug("%s(error request data!)\r\n", __func__);
	}
	/* 清空BLE接收Buffer */
	_CMIOT_Debug("%s(Clear ble buffer)\r\n", __func__);
	memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));
	UART_BLE_RxBufferLen = 0;
}







